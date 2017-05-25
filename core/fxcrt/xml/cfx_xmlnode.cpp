// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlnode.h"

#include <vector>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmlchardata.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlinstruction.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "third_party/base/stl_util.h"

CFX_XMLNode::CFX_XMLNode()
    : m_pParent(nullptr),
      m_pChild(nullptr),
      m_pPrior(nullptr),
      m_pNext(nullptr) {}

FX_XMLNODETYPE CFX_XMLNode::GetType() const {
  return FX_XMLNODE_Unknown;
}

CFX_XMLNode::~CFX_XMLNode() {
  DeleteChildren();
}

void CFX_XMLNode::DeleteChildren() {
  CFX_XMLNode* pChild = m_pChild.Get();
  while (pChild) {
    CFX_XMLNode* pNext = pChild->m_pNext.Get();
    delete pChild;
    pChild = pNext;
  }
  m_pChild = nullptr;
}

int32_t CFX_XMLNode::CountChildNodes() const {
  int32_t iCount = 0;
  CFX_XMLNode* pChild = m_pChild.Get();
  while (pChild) {
    iCount++;
    pChild = pChild->m_pNext.Get();
  }
  return iCount;
}

CFX_XMLNode* CFX_XMLNode::GetChildNode(int32_t index) const {
  CFX_XMLNode* pChild = m_pChild.Get();
  while (pChild) {
    if (index == 0) {
      return pChild;
    }
    index--;
    pChild = pChild->m_pNext.Get();
  }
  return nullptr;
}

int32_t CFX_XMLNode::GetChildNodeIndex(CFX_XMLNode* pNode) const {
  int32_t index = 0;
  CFX_XMLNode* pChild = m_pChild.Get();
  while (pChild) {
    if (pChild == pNode) {
      return index;
    }
    index++;
    pChild = pChild->m_pNext.Get();
  }
  return -1;
}

CFX_XMLNode* CFX_XMLNode::GetPath(const wchar_t* pPath,
                                  int32_t iLength,
                                  bool bQualifiedName) const {
  ASSERT(pPath);
  if (iLength < 0) {
    iLength = FXSYS_wcslen(pPath);
  }
  if (iLength == 0) {
    return nullptr;
  }
  CFX_WideString csPath;
  const wchar_t* pStart = pPath;
  const wchar_t* pEnd = pPath + iLength;
  wchar_t ch;
  while (pStart < pEnd) {
    ch = *pStart++;
    if (ch == L'/') {
      break;
    } else {
      csPath += ch;
    }
  }
  iLength -= pStart - pPath;
  CFX_XMLNode* pFind = nullptr;
  if (csPath.GetLength() < 1) {
    pFind = GetNodeItem(CFX_XMLNode::Root);
  } else if (csPath.Compare(L"..") == 0) {
    pFind = m_pParent.Get();
  } else if (csPath.Compare(L".") == 0) {
    pFind = (CFX_XMLNode*)this;
  } else {
    CFX_WideString wsTag;
    CFX_XMLNode* pNode = m_pChild.Get();
    while (pNode) {
      if (pNode->GetType() == FX_XMLNODE_Element) {
        if (bQualifiedName)
          wsTag = static_cast<CFX_XMLElement*>(pNode)->GetName();
        else
          wsTag = static_cast<CFX_XMLElement*>(pNode)->GetLocalTagName();

        if (wsTag.Compare(csPath) == 0) {
          if (iLength < 1)
            pFind = pNode;
          else
            pFind = pNode->GetPath(pStart, iLength, bQualifiedName);

          if (pFind)
            return pFind;
        }
      }
      pNode = pNode->m_pNext.Get();
    }
  }
  if (!pFind || iLength < 1)
    return pFind;
  return pFind->GetPath(pStart, iLength, bQualifiedName);
}

int32_t CFX_XMLNode::InsertChildNode(CFX_XMLNode* pNode, int32_t index) {
  pNode->m_pParent = this;
  if (!m_pChild) {
    m_pChild = pNode;
    pNode->m_pPrior = nullptr;
    pNode->m_pNext = nullptr;
    return 0;
  }
  if (index == 0) {
    pNode->m_pNext = m_pChild;
    pNode->m_pPrior = nullptr;
    m_pChild->m_pPrior = pNode;
    m_pChild = pNode;
    return 0;
  }
  int32_t iCount = 0;
  CFX_XMLNode* pFind = m_pChild.Get();
  while (++iCount != index && pFind->m_pNext) {
    pFind = pFind->m_pNext.Get();
  }
  pNode->m_pPrior = pFind;
  pNode->m_pNext = pFind->m_pNext;
  if (pFind->m_pNext)
    pFind->m_pNext->m_pPrior = pNode;
  pFind->m_pNext = pNode;
  return iCount;
}

void CFX_XMLNode::RemoveChildNode(CFX_XMLNode* pNode) {
  ASSERT(m_pChild && pNode);
  if (m_pChild == pNode) {
    m_pChild = pNode->m_pNext;
  } else {
    pNode->m_pPrior->m_pNext = pNode->m_pNext;
  }
  if (pNode->m_pNext)
    pNode->m_pNext->m_pPrior = pNode->m_pPrior;
  pNode->m_pParent = nullptr;
  pNode->m_pNext = nullptr;
  pNode->m_pPrior = nullptr;
}

CFX_XMLNode* CFX_XMLNode::GetNodeItem(CFX_XMLNode::NodeItem eItem) const {
  switch (eItem) {
    case CFX_XMLNode::Root: {
      CFX_XMLNode* pParent = (CFX_XMLNode*)this;
      while (pParent->m_pParent)
        pParent = pParent->m_pParent.Get();
      return pParent;
    }
    case CFX_XMLNode::Parent:
      return m_pParent.Get();
    case CFX_XMLNode::FirstSibling: {
      CFX_XMLNode* pItem = (CFX_XMLNode*)this;
      while (pItem->m_pPrior)
        pItem = pItem->m_pPrior.Get();
      return pItem == this ? nullptr : pItem;
    }
    case CFX_XMLNode::PriorSibling:
      return m_pPrior.Get();
    case CFX_XMLNode::NextSibling:
      return m_pNext.Get();
    case CFX_XMLNode::LastSibling: {
      CFX_XMLNode* pItem = const_cast<CFX_XMLNode*>(this);
      while (pItem->m_pNext)
        pItem = pItem->m_pNext.Get();
      return pItem == this ? nullptr : pItem;
    }
    case CFX_XMLNode::FirstNeighbor: {
      CFX_XMLNode* pParent = const_cast<CFX_XMLNode*>(this);
      while (pParent->m_pParent)
        pParent = pParent->m_pParent.Get();
      return pParent == this ? nullptr : pParent;
    }
    case CFX_XMLNode::PriorNeighbor: {
      if (!m_pPrior)
        return m_pParent.Get();
      CFX_XMLNode* pItem = m_pPrior.Get();
      while (pItem->m_pChild) {
        pItem = pItem->m_pChild.Get();
        while (pItem->m_pNext)
          pItem = pItem->m_pNext.Get();
      }
      return pItem;
    }
    case CFX_XMLNode::NextNeighbor: {
      if (m_pChild)
        return m_pChild.Get();
      if (m_pNext)
        return m_pNext.Get();
      CFX_XMLNode* pItem = m_pParent.Get();
      while (pItem) {
        if (pItem->m_pNext)
          return pItem->m_pNext.Get();
        pItem = pItem->m_pParent.Get();
      }
      return nullptr;
    }
    case CFX_XMLNode::LastNeighbor: {
      CFX_XMLNode* pItem = const_cast<CFX_XMLNode*>(this);
      while (pItem->m_pParent)
        pItem = pItem->m_pParent.Get();
      while (true) {
        while (pItem->m_pNext)
          pItem = pItem->m_pNext.Get();
        if (!pItem->m_pChild)
          break;
        pItem = pItem->m_pChild.Get();
      }
      return pItem == (CFX_XMLNode*)this ? nullptr : pItem;
    }
    case CFX_XMLNode::FirstChild:
      return m_pChild.Get();
    case CFX_XMLNode::LastChild: {
      if (!m_pChild)
        return nullptr;

      CFX_XMLNode* pChild = m_pChild.Get();
      while (pChild->m_pNext)
        pChild = pChild->m_pNext.Get();
      return pChild;
    }
    default:
      break;
  }
  return nullptr;
}

int32_t CFX_XMLNode::GetNodeLevel() const {
  int32_t iLevel = 0;
  const CFX_XMLNode* pItem = m_pParent.Get();
  while (pItem) {
    iLevel++;
    pItem = pItem->m_pParent.Get();
  }
  return iLevel;
}

bool CFX_XMLNode::InsertNodeItem(CFX_XMLNode::NodeItem eItem,
                                 CFX_XMLNode* pNode) {
  switch (eItem) {
    case CFX_XMLNode::NextSibling: {
      pNode->m_pParent = m_pParent;
      pNode->m_pNext = m_pNext;
      pNode->m_pPrior = this;
      if (m_pNext) {
        m_pNext->m_pPrior = pNode;
      }
      m_pNext = pNode;
      return true;
    }
    case CFX_XMLNode::PriorSibling: {
      pNode->m_pParent = m_pParent;
      pNode->m_pNext = this;
      pNode->m_pPrior = m_pPrior;
      if (m_pPrior) {
        m_pPrior->m_pNext = pNode;
      } else if (m_pParent) {
        m_pParent->m_pChild = pNode;
      }
      m_pPrior = pNode;
      return true;
    }
    default:
      return false;
  }
}

CFX_XMLNode* CFX_XMLNode::RemoveNodeItem(CFX_XMLNode::NodeItem eItem) {
  CFX_XMLNode* pNode = nullptr;
  switch (eItem) {
    case CFX_XMLNode::NextSibling:
      if (m_pNext) {
        pNode = m_pNext.Get();
        m_pNext = pNode->m_pNext;
        if (m_pNext)
          m_pNext->m_pPrior = this;
        pNode->m_pParent = nullptr;
        pNode->m_pNext = nullptr;
        pNode->m_pPrior = nullptr;
      }
      break;
    default:
      break;
  }
  return pNode;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLNode::Clone() {
  return nullptr;
}

void CFX_XMLNode::SaveXMLNode(
    const CFX_RetainPtr<CFX_SeekableStreamProxy>& pXMLStream) {
  CFX_XMLNode* pNode = (CFX_XMLNode*)this;
  switch (pNode->GetType()) {
    case FX_XMLNODE_Instruction: {
      CFX_WideString ws;
      CFX_XMLInstruction* pInstruction = (CFX_XMLInstruction*)pNode;
      if (pInstruction->GetName().CompareNoCase(L"xml") == 0) {
        ws = L"<?xml version=\"1.0\" encoding=\"";
        uint16_t wCodePage = pXMLStream->GetCodePage();
        if (wCodePage == FX_CODEPAGE_UTF16LE) {
          ws += L"UTF-16";
        } else if (wCodePage == FX_CODEPAGE_UTF16BE) {
          ws += L"UTF-16be";
        } else {
          ws += L"UTF-8";
        }
        ws += L"\"?>";
        pXMLStream->WriteString(ws.AsStringC());
      } else {
        ws.Format(L"<?%s", pInstruction->GetName().c_str());
        pXMLStream->WriteString(ws.AsStringC());

        for (auto it : pInstruction->GetAttributes()) {
          CFX_WideString wsValue = it.second;
          wsValue.Replace(L"&", L"&amp;");
          wsValue.Replace(L"<", L"&lt;");
          wsValue.Replace(L">", L"&gt;");
          wsValue.Replace(L"\'", L"&apos;");
          wsValue.Replace(L"\"", L"&quot;");

          ws = L" ";
          ws += it.first;
          ws += L"=\"";
          ws += wsValue;
          ws += L"\"";
          pXMLStream->WriteString(ws.AsStringC());
        }

        for (auto target : pInstruction->GetTargetData()) {
          ws = L" \"";
          ws += target;
          ws += L"\"";
          pXMLStream->WriteString(ws.AsStringC());
        }
        ws = L"?>";
        pXMLStream->WriteString(ws.AsStringC());
      }
      break;
    }
    case FX_XMLNODE_Element: {
      CFX_WideString ws;
      ws = L"<";
      ws += static_cast<CFX_XMLElement*>(pNode)->GetName();
      pXMLStream->WriteString(ws.AsStringC());

      for (auto it : static_cast<CFX_XMLElement*>(pNode)->GetAttributes()) {
        CFX_WideString wsValue = it.second;
        wsValue.Replace(L"&", L"&amp;");
        wsValue.Replace(L"<", L"&lt;");
        wsValue.Replace(L">", L"&gt;");
        wsValue.Replace(L"\'", L"&apos;");
        wsValue.Replace(L"\"", L"&quot;");

        ws = L" ";
        ws += it.first;
        ws += L"=\"";
        ws += wsValue;
        ws += L"\"";
        pXMLStream->WriteString(ws.AsStringC());
      }
      if (pNode->m_pChild) {
        ws = L"\n>";
        pXMLStream->WriteString(ws.AsStringC());
        CFX_XMLNode* pChild = pNode->m_pChild.Get();
        while (pChild) {
          pChild->SaveXMLNode(pXMLStream);
          pChild = pChild->m_pNext.Get();
        }
        ws = L"</";
        ws += static_cast<CFX_XMLElement*>(pNode)->GetName();
        ws += L"\n>";
      } else {
        ws = L"\n/>";
      }
      pXMLStream->WriteString(ws.AsStringC());
      break;
    }
    case FX_XMLNODE_Text: {
      CFX_WideString ws = static_cast<CFX_XMLText*>(pNode)->GetText();
      ws.Replace(L"&", L"&amp;");
      ws.Replace(L"<", L"&lt;");
      ws.Replace(L">", L"&gt;");
      ws.Replace(L"\'", L"&apos;");
      ws.Replace(L"\"", L"&quot;");
      pXMLStream->WriteString(ws.AsStringC());
      break;
    }
    case FX_XMLNODE_CharData: {
      CFX_WideString ws = L"<![CDATA[";
      ws += static_cast<CFX_XMLCharData*>(pNode)->GetText();
      ws += L"]]>";
      pXMLStream->WriteString(ws.AsStringC());
      break;
    }
    case FX_XMLNODE_Unknown:
    default:
      break;
  }
}
