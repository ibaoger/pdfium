// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlparser.h"

#include <algorithm>
#include <cwctype>
#include <iterator>
#include <utility>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/ptr_util.h"

namespace {

const uint32_t kMaxCharRange = 0x10ffff;

bool IsXMLWhiteSpace(wchar_t ch) {
  return ch == L' ' || ch == 0x0A || ch == 0x0D || ch == 0x09;
}

struct FX_XMLNAMECHAR {
  uint16_t wStart;
  uint16_t wEnd;
  bool bStartChar;
};

const FX_XMLNAMECHAR g_XMLNameChars[] = {
    {L'-', L'.', false},    {L'0', L'9', false},     {L':', L':', false},
    {L'A', L'Z', true},     {L'_', L'_', true},      {L'a', L'z', true},
    {0xB7, 0xB7, false},    {0xC0, 0xD6, true},      {0xD8, 0xF6, true},
    {0xF8, 0x02FF, true},   {0x0300, 0x036F, false}, {0x0370, 0x037D, true},
    {0x037F, 0x1FFF, true}, {0x200C, 0x200D, true},  {0x203F, 0x2040, false},
    {0x2070, 0x218F, true}, {0x2C00, 0x2FEF, true},  {0x3001, 0xD7FF, true},
    {0xF900, 0xFDCF, true}, {0xFDF0, 0xFFFD, true},
};

}  // namespace

// static
bool CFX_XMLParser::IsXMLNameChar(wchar_t ch, bool bFirstChar) {
  auto* it = std::lower_bound(
      std::begin(g_XMLNameChars), std::end(g_XMLNameChars), ch,
      [](const FX_XMLNAMECHAR& arg, wchar_t ch) { return arg.wEnd < ch; });
  return it != std::end(g_XMLNameChars) && ch >= it->wStart &&
         (!bFirstChar || it->bStartChar);
}

CFX_XMLParser::CFX_XMLParser(CFX_XMLNode* pParent,
                             const RetainPtr<IFX_SeekableReadStream>& pStream)
    : m_pParent(pParent) {
  ASSERT(m_pParent);
  ASSERT(pStream);

  auto proxy = pdfium::MakeRetain<CFX_SeekableStreamProxy>(pStream);
  uint16_t wCodePage = proxy->GetCodePage();
  if (wCodePage != FX_CODEPAGE_UTF16LE && wCodePage != FX_CODEPAGE_UTF16BE &&
      wCodePage != FX_CODEPAGE_UTF8) {
    proxy->SetCodePage(FX_CODEPAGE_UTF8);
  }
  m_pStream = proxy;

  m_NodeStack.push(m_pParent);

  m_iXMLPlaneSize =
      std::min(m_iXMLPlaneSize,
               pdfium::base::checked_cast<size_t>(m_pStream->GetSize()));

  FX_SAFE_SIZE_T alloc_size_safe = m_iXMLPlaneSize;
  alloc_size_safe += 1;  // For NUL.
  if (!alloc_size_safe.IsValid() || alloc_size_safe.ValueOrDie() <= 0) {
    m_syntaxParserResult = FX_XmlSyntaxResult::Error;
    return;
  }

  m_Buffer.resize(pdfium::base::ValueOrDieForType<size_t>(alloc_size_safe));

  m_BlockBuffer.InitBuffer();
  std::tie(m_pCurrentBlock, m_iIndexInBlock) =
      m_BlockBuffer.GetAvailableBlock();
}

CFX_XMLParser::~CFX_XMLParser() = default;

bool CFX_XMLParser::Parse() {
  int32_t iCount = 0;
  while (true) {
    FX_XmlSyntaxResult result = DoSyntaxParse();
    if (result == FX_XmlSyntaxResult::Error)
      return false;
    if (result == FX_XmlSyntaxResult::EndOfString)
      break;

    switch (result) {
      case FX_XmlSyntaxResult::InstructionClose:
        if (m_pChild && m_pChild->GetType() != FX_XMLNODE_Instruction)
          return false;

        m_pChild = m_pParent;
        break;
      case FX_XmlSyntaxResult::ElementClose:
        if (m_pChild->GetType() != FX_XMLNODE_Element)
          return false;

        m_ws1 = GetTextData();
        if (m_ws1.GetLength() > 0 &&
            m_ws1 != static_cast<CFX_XMLElement*>(m_pChild)->GetName()) {
          return false;
        }

        if (!m_NodeStack.empty())
          m_NodeStack.pop();
        if (m_NodeStack.empty())
          return false;

        m_pParent = m_NodeStack.top();
        m_pChild = m_pParent;
        iCount++;
        break;
      case FX_XmlSyntaxResult::TargetName:
        m_ws1 = GetTextData();
        if (m_ws1 == L"originalXFAVersion" || m_ws1 == L"acrobat") {
          auto child = pdfium::MakeUnique<CFX_XMLInstruction>(m_ws1);
          m_pChild = child.get();
          m_pParent->AppendChild(std::move(child));
        } else {
          m_pChild = nullptr;
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::TagName: {
        m_ws1 = GetTextData();
        auto child = pdfium::MakeUnique<CFX_XMLElement>(m_ws1);
        m_pChild = child.get();
        m_pParent->AppendChild(std::move(child));
        m_NodeStack.push(m_pChild);
        m_pParent = m_pChild;
        break;
      }
      case FX_XmlSyntaxResult::AttriName:
        m_ws1 = GetTextData();
        break;
      case FX_XmlSyntaxResult::AttriValue:
        if (m_pChild && m_pChild->GetType() == FX_XMLNODE_Element) {
          static_cast<CFX_XMLElement*>(m_pChild)->SetAttribute(m_ws1,
                                                               GetTextData());
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::Text: {
        m_ws1 = GetTextData();
        auto child = pdfium::MakeUnique<CFX_XMLText>(m_ws1);
        m_pChild = child.get();
        m_pParent->AppendChild(std::move(child));
        m_pChild = m_pParent;
        break;
      }
      case FX_XmlSyntaxResult::CData: {
        m_ws1 = GetTextData();
        auto child = pdfium::MakeUnique<CFX_XMLCharData>(m_ws1);
        m_pChild = child.get();
        m_pParent->AppendChild(std::move(child));
        m_pChild = m_pParent;
        break;
      }
      case FX_XmlSyntaxResult::TargetData:
        if (m_pChild) {
          if (m_pChild->GetType() != FX_XMLNODE_Instruction)
            return false;

          auto* instruction = static_cast<CFX_XMLInstruction*>(m_pChild);
          if (!m_ws1.IsEmpty())
            instruction->AppendData(m_ws1);

          instruction->AppendData(GetTextData());
        }
        m_ws1.clear();
        break;
      case FX_XmlSyntaxResult::ElementOpen:
      case FX_XmlSyntaxResult::ElementBreak:
      case FX_XmlSyntaxResult::InstructionOpen:
      default:
        break;
    }
  }
  return m_NodeStack.size() != 1 ? false : GetStatus();
}

FX_XmlSyntaxResult CFX_XMLParser::DoSyntaxParse() {
  if (m_syntaxParserResult == FX_XmlSyntaxResult::Error ||
      m_syntaxParserResult == FX_XmlSyntaxResult::EndOfString) {
    return m_syntaxParserResult;
  }

  FX_XmlSyntaxResult syntaxParserResult = FX_XmlSyntaxResult::None;
  while (true) {
    if (m_Start >= m_End) {
      if (m_pStream->IsEOF()) {
        m_syntaxParserResult = FX_XmlSyntaxResult::EndOfString;
        return m_syntaxParserResult;
      }

      size_t buffer_chars =
          m_pStream->ReadBlock(m_Buffer.data(), m_iXMLPlaneSize);
      if (buffer_chars == 0) {
        m_syntaxParserResult = FX_XmlSyntaxResult::EndOfString;
        return m_syntaxParserResult;
      }
      m_Start = 0;
      m_End = buffer_chars;
    }

    while (m_Start < m_End) {
      wchar_t ch = m_Buffer[m_Start];
      switch (m_syntaxParserState) {
        case FDE_XmlSyntaxState::Text:
          if (ch == L'<') {
            if (!m_BlockBuffer.IsEmpty()) {
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_iEntityStart = -1;
              syntaxParserResult = FX_XmlSyntaxResult::Text;
            } else {
              m_Start++;
              m_syntaxParserState = FDE_XmlSyntaxState::Node;
            }
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XmlSyntaxState::Node:
          if (ch == L'!') {
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::SkipCommentOrDecl;
          } else if (ch == L'/') {
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::CloseElement;
          } else if (ch == L'?') {
            m_CurNodeType = FX_XMLNODE_Instruction;
            m_XMLNodeTypeStack.push(m_CurNodeType);
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::Target;
            syntaxParserResult = FX_XmlSyntaxResult::InstructionOpen;
          } else {
            m_CurNodeType = FX_XMLNODE_Element;
            m_XMLNodeTypeStack.push(m_CurNodeType);
            m_syntaxParserState = FDE_XmlSyntaxState::Tag;
            syntaxParserResult = FX_XmlSyntaxResult::ElementOpen;
          }
          break;
        case FDE_XmlSyntaxState::Target:
        case FDE_XmlSyntaxState::Tag:
          if (!IsXMLNameChar(ch, m_BlockBuffer.IsEmpty())) {
            if (m_BlockBuffer.IsEmpty()) {
              m_syntaxParserResult = FX_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }

            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            if (m_syntaxParserState != FDE_XmlSyntaxState::Target)
              syntaxParserResult = FX_XmlSyntaxResult::TagName;
            else
              syntaxParserResult = FX_XmlSyntaxResult::TargetName;

            m_syntaxParserState = FDE_XmlSyntaxState::AttriName;
          } else {
            if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FX_XmlSyntaxResult::Error;
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriName:
          if (m_BlockBuffer.IsEmpty() && IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (!IsXMLNameChar(ch, m_BlockBuffer.IsEmpty())) {
            if (m_BlockBuffer.IsEmpty()) {
              if (m_CurNodeType == FX_XMLNODE_Element) {
                if (ch == L'>' || ch == L'/') {
                  m_syntaxParserState = FDE_XmlSyntaxState::BreakElement;
                  break;
                }
              } else if (m_CurNodeType == FX_XMLNODE_Instruction) {
                if (ch == L'?') {
                  m_syntaxParserState = FDE_XmlSyntaxState::CloseInstruction;
                  m_Start++;
                } else {
                  m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
                }
                break;
              }
              m_syntaxParserResult = FX_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            } else {
              if (m_CurNodeType == FX_XMLNODE_Instruction) {
                if (ch != '=' && !IsXMLWhiteSpace(ch)) {
                  m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
                  break;
                }
              }
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_syntaxParserState = FDE_XmlSyntaxState::AttriEqualSign;
              syntaxParserResult = FX_XmlSyntaxResult::AttriName;
            }
          } else {
            if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FX_XmlSyntaxResult::Error;
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriEqualSign:
          if (IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (ch != L'=') {
            if (m_CurNodeType == FX_XMLNODE_Instruction) {
              m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
              break;
            }
            m_syntaxParserResult = FX_XmlSyntaxResult::Error;
            return m_syntaxParserResult;
          } else {
            m_syntaxParserState = FDE_XmlSyntaxState::AttriQuotation;
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::AttriQuotation:
          if (IsXMLWhiteSpace(ch)) {
            m_Start++;
            break;
          }
          if (ch != L'\"' && ch != L'\'') {
            m_syntaxParserResult = FX_XmlSyntaxResult::Error;
            return m_syntaxParserResult;
          }

          m_wQuotationMark = ch;
          m_syntaxParserState = FDE_XmlSyntaxState::AttriValue;
          m_Start++;
          break;
        case FDE_XmlSyntaxState::AttriValue:
          if (ch == m_wQuotationMark) {
            if (m_iEntityStart > -1) {
              m_syntaxParserResult = FX_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_wQuotationMark = 0;
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            m_Start++;
            m_syntaxParserState = FDE_XmlSyntaxState::AttriName;
            syntaxParserResult = FX_XmlSyntaxResult::AttriValue;
          } else {
            ParseTextChar(ch);
          }
          break;
        case FDE_XmlSyntaxState::CloseInstruction:
          if (ch != L'>') {
            if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FX_XmlSyntaxResult::Error;
            }

            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_syntaxParserState = FDE_XmlSyntaxState::TargetData;
          } else if (!m_BlockBuffer.IsEmpty()) {
            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            syntaxParserResult = FX_XmlSyntaxResult::TargetData;
          } else {
            m_Start++;
            if (m_XMLNodeTypeStack.empty()) {
              m_syntaxParserResult = FX_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
            m_XMLNodeTypeStack.pop();

            if (!m_XMLNodeTypeStack.empty())
              m_CurNodeType = m_XMLNodeTypeStack.top();
            else
              m_CurNodeType = FX_XMLNODE_Unknown;

            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
            syntaxParserResult = FX_XmlSyntaxResult::InstructionClose;
          }
          break;
        case FDE_XmlSyntaxState::BreakElement:
          if (ch == L'>') {
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
            syntaxParserResult = FX_XmlSyntaxResult::ElementBreak;
          } else if (ch == L'/') {
            m_syntaxParserState = FDE_XmlSyntaxState::CloseElement;
          } else {
            m_syntaxParserResult = FX_XmlSyntaxResult::Error;
            return m_syntaxParserResult;
          }
          m_Start++;
          break;
        case FDE_XmlSyntaxState::CloseElement:
          if (!IsXMLNameChar(ch, m_BlockBuffer.IsEmpty())) {
            if (ch == L'>') {
              if (m_XMLNodeTypeStack.empty()) {
                m_syntaxParserResult = FX_XmlSyntaxResult::Error;
                return m_syntaxParserResult;
              }
              m_XMLNodeTypeStack.pop();

              if (!m_XMLNodeTypeStack.empty())
                m_CurNodeType = m_XMLNodeTypeStack.top();
              else
                m_CurNodeType = FX_XMLNODE_Unknown;

              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_syntaxParserState = FDE_XmlSyntaxState::Text;
              syntaxParserResult = FX_XmlSyntaxResult::ElementClose;
            } else if (!IsXMLWhiteSpace(ch)) {
              m_syntaxParserResult = FX_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
          } else {
            if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FX_XmlSyntaxResult::Error;
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
          }
          m_Start++;
          break;
        case FDE_XmlSyntaxState::SkipCommentOrDecl:
          if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"--", 2) == 0) {
            m_Start += 2;
            m_syntaxParserState = FDE_XmlSyntaxState::SkipComment;
          } else if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"[CDATA[", 7) ==
                     0) {
            m_Start += 7;
            m_syntaxParserState = FDE_XmlSyntaxState::SkipCData;
          } else {
            m_syntaxParserState = FDE_XmlSyntaxState::SkipDeclNode;
            m_SkipChar = L'>';
            m_SkipStack.push(L'>');
          }
          break;
        case FDE_XmlSyntaxState::SkipCData: {
          if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"]]>", 3) == 0) {
            m_Start += 3;
            syntaxParserResult = FX_XmlSyntaxResult::CData;
            m_iTextDataLength = m_BlockBuffer.GetDataLength();
            m_BlockBuffer.Reset(true);
            std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                m_BlockBuffer.GetAvailableBlock();
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
          } else {
            if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FX_XmlSyntaxResult::Error;
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        }
        case FDE_XmlSyntaxState::SkipDeclNode:
          if (m_SkipChar == L'\'' || m_SkipChar == L'\"') {
            m_Start++;
            if (ch != m_SkipChar)
              break;

            m_SkipStack.pop();
            if (m_SkipStack.empty())
              m_syntaxParserState = FDE_XmlSyntaxState::Text;
            else
              m_SkipChar = m_SkipStack.top();
          } else {
            switch (ch) {
              case L'<':
                m_SkipChar = L'>';
                m_SkipStack.push(L'>');
                break;
              case L'[':
                m_SkipChar = L']';
                m_SkipStack.push(L']');
                break;
              case L'(':
                m_SkipChar = L')';
                m_SkipStack.push(L')');
                break;
              case L'\'':
                m_SkipChar = L'\'';
                m_SkipStack.push(L'\'');
                break;
              case L'\"':
                m_SkipChar = L'\"';
                m_SkipStack.push(L'\"');
                break;
              default:
                if (ch == m_SkipChar) {
                  m_SkipStack.pop();
                  if (m_SkipStack.empty()) {
                    m_iTextDataLength = m_BlockBuffer.GetDataLength();
                    m_BlockBuffer.Reset(true);
                    std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                        m_BlockBuffer.GetAvailableBlock();
                    m_syntaxParserState = FDE_XmlSyntaxState::Text;
                  } else {
                    m_SkipChar = m_SkipStack.top();
                  }
                }
                break;
            }
            if (!m_SkipStack.empty()) {
              if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
                std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                    m_BlockBuffer.GetAvailableBlock();
                if (!m_pCurrentBlock) {
                  return FX_XmlSyntaxResult::Error;
                }
              }
              m_pCurrentBlock[m_iIndexInBlock++] = ch;
              m_BlockBuffer.IncrementDataLength();
            }
            m_Start++;
          }
          break;
        case FDE_XmlSyntaxState::SkipComment:
          if (FXSYS_wcsnicmp(m_Buffer.data() + m_Start, L"-->", 3) == 0) {
            m_Start += 2;
            m_syntaxParserState = FDE_XmlSyntaxState::Text;
          }

          m_Start++;
          break;
        case FDE_XmlSyntaxState::TargetData:
          if (IsXMLWhiteSpace(ch)) {
            if (m_BlockBuffer.IsEmpty()) {
              m_Start++;
              break;
            }
            if (m_wQuotationMark == 0) {
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_wQuotationMark = 0;
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_Start++;
              syntaxParserResult = FX_XmlSyntaxResult::TargetData;
              break;
            }
          }
          if (ch == '?') {
            m_syntaxParserState = FDE_XmlSyntaxState::CloseInstruction;
            m_Start++;
          } else if (ch == '\"') {
            if (m_wQuotationMark == 0) {
              m_wQuotationMark = ch;
              m_Start++;
            } else if (ch == m_wQuotationMark) {
              m_iTextDataLength = m_BlockBuffer.GetDataLength();
              m_wQuotationMark = 0;
              m_BlockBuffer.Reset(true);
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              m_Start++;
              syntaxParserResult = FX_XmlSyntaxResult::TargetData;
            } else {
              m_syntaxParserResult = FX_XmlSyntaxResult::Error;
              return m_syntaxParserResult;
            }
          } else {
            if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
              std::tie(m_pCurrentBlock, m_iIndexInBlock) =
                  m_BlockBuffer.GetAvailableBlock();
              if (!m_pCurrentBlock)
                return FX_XmlSyntaxResult::Error;
            }
            m_pCurrentBlock[m_iIndexInBlock++] = ch;
            m_BlockBuffer.IncrementDataLength();
            m_Start++;
          }
          break;
        default:
          break;
      }
      if (syntaxParserResult != FX_XmlSyntaxResult::None)
        return syntaxParserResult;
    }
  }
  return FX_XmlSyntaxResult::Text;
}

bool CFX_XMLParser::GetStatus() const {
  return m_pStream && m_syntaxParserResult != FX_XmlSyntaxResult::Error;
}

void CFX_XMLParser::ParseTextChar(wchar_t character) {
  if (m_iIndexInBlock == m_BlockBuffer.GetAllocStep()) {
    std::tie(m_pCurrentBlock, m_iIndexInBlock) =
        m_BlockBuffer.GetAvailableBlock();
    if (!m_pCurrentBlock)
      return;
  }

  m_pCurrentBlock[m_iIndexInBlock++] = character;
  m_BlockBuffer.IncrementDataLength();
  if (m_iEntityStart > -1 && character == L';') {
    WideString csEntity = m_BlockBuffer.GetTextData(
        m_iEntityStart + 1,
        m_BlockBuffer.GetDataLength() - 1 - m_iEntityStart - 1);
    int32_t iLen = csEntity.GetLength();
    if (iLen > 0) {
      if (csEntity[0] == L'#') {
        uint32_t ch = 0;
        wchar_t w;
        if (iLen > 1 && csEntity[1] == L'x') {
          for (int32_t i = 2; i < iLen; i++) {
            w = csEntity[i];
            if (std::iswdigit(w))
              ch = (ch << 4) + w - L'0';
            else if (w >= L'A' && w <= L'F')
              ch = (ch << 4) + w - 55;
            else if (w >= L'a' && w <= L'f')
              ch = (ch << 4) + w - 87;
            else
              break;
          }
        } else {
          for (int32_t i = 1; i < iLen; i++) {
            w = csEntity[i];
            if (!std::iswdigit(w))
              break;

            ch = ch * 10 + w - L'0';
          }
        }
        if (ch > kMaxCharRange)
          ch = ' ';

        character = static_cast<wchar_t>(ch);
        if (character != 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, character);
          m_iEntityStart++;
        }
      } else {
        if (csEntity.Compare(L"amp") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'&');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"lt") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'<');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"gt") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'>');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"apos") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'\'');
          m_iEntityStart++;
        } else if (csEntity.Compare(L"quot") == 0) {
          m_BlockBuffer.SetTextChar(m_iEntityStart, L'\"');
          m_iEntityStart++;
        }
      }
    }
    if (m_iEntityStart >= 0 &&
        m_BlockBuffer.GetDataLength() > static_cast<size_t>(m_iEntityStart)) {
      m_BlockBuffer.DeleteTextChars(m_BlockBuffer.GetDataLength() -
                                    m_iEntityStart);
    }
    std::tie(m_pCurrentBlock, m_iIndexInBlock) =
        m_BlockBuffer.GetAvailableBlock();
    m_iEntityStart = -1;
  } else if (m_iEntityStart < 0 && character == L'&') {
    m_iEntityStart = m_BlockBuffer.GetDataLength() - 1;
  }
  m_Start++;
}

WideString CFX_XMLParser::GetTextData() const {
  return m_BlockBuffer.GetTextData(0, m_iTextDataLength);
}
