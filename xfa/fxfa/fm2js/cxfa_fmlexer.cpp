// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmlexer.h"

#include <algorithm>
#include <iostream>

#include "core/fxcrt/fx_extension.h"
#include "third_party/base/ptr_util.h"
#include "third_party/icu/source/common/unicode/uchar.h"

namespace {

bool IsFormCalcCharacter(const wchar_t& c) {
  return (c >= 0x09 && c <= 0x0D) || (c >= 0x20 && c <= 0xd7FF) ||
         (c >= 0xE000 && c <= 0xFFFD);
}

bool IsIdentifierCharacter(const wchar_t& c) {
  return u_isalnum(c) || c == 0x005F ||  // '_'
         c == 0x0024;                    // '$'
}

bool IsInitialIdentifierCharacter(const wchar_t& c) {
  return u_isalpha(c) || c == 0x005F ||  // '_'
         c == 0x0024 ||                  // '$'
         c == 0x0021;                    // '!'
}

bool IsWhitespaceCharacter(const wchar_t& c) {
  return c == 0x0009 ||  // Horizontal tab
         c == 0x000B ||  // Vertical tab
         c == 0x000C ||  // Form feed
         c == 0x0020;    // Space
}

const XFA_FMKeyword keyWords[] = {
    {TOKand, 0x00000026, L"&"},
    {TOKlparen, 0x00000028, L"("},
    {TOKrparen, 0x00000029, L")"},
    {TOKmul, 0x0000002a, L"*"},
    {TOKplus, 0x0000002b, L"+"},
    {TOKcomma, 0x0000002c, L","},
    {TOKminus, 0x0000002d, L"-"},
    {TOKdot, 0x0000002e, L"."},
    {TOKdiv, 0x0000002f, L"/"},
    {TOKlt, 0x0000003c, L"<"},
    {TOKassign, 0x0000003d, L"="},
    {TOKgt, 0x0000003e, L">"},
    {TOKlbracket, 0x0000005b, L"["},
    {TOKrbracket, 0x0000005d, L"]"},
    {TOKor, 0x0000007c, L"|"},
    {TOKdotscream, 0x0000ec11, L".#"},
    {TOKdotstar, 0x0000ec18, L".*"},
    {TOKdotdot, 0x0000ec1c, L".."},
    {TOKle, 0x000133f9, L"<="},
    {TOKne, 0x000133fa, L"<>"},
    {TOKeq, 0x0001391a, L"=="},
    {TOKge, 0x00013e3b, L">="},
    {TOKdo, 0x00020153, L"do"},
    {TOKkseq, 0x00020676, L"eq"},
    {TOKksge, 0x000210ac, L"ge"},
    {TOKksgt, 0x000210bb, L"gt"},
    {TOKif, 0x00021aef, L"if"},
    {TOKin, 0x00021af7, L"in"},
    {TOKksle, 0x00022a51, L"le"},
    {TOKkslt, 0x00022a60, L"lt"},
    {TOKksne, 0x00023493, L"ne"},
    {TOKksor, 0x000239c1, L"or"},
    {TOKnull, 0x052931bb, L"null"},
    {TOKbreak, 0x05518c25, L"break"},
    {TOKksand, 0x09f9db33, L"and"},
    {TOKend, 0x0a631437, L"end"},
    {TOKeof, 0x0a63195a, L"eof"},
    {TOKfor, 0x0a7d67a7, L"for"},
    {TOKnan, 0x0b4f91dd, L"nan"},
    {TOKksnot, 0x0b4fd9b1, L"not"},
    {TOKvar, 0x0c2203e9, L"var"},
    {TOKthen, 0x2d5738cf, L"then"},
    {TOKelse, 0x45f65ee9, L"else"},
    {TOKexit, 0x4731d6ba, L"exit"},
    {TOKdownto, 0x4caadc3b, L"downto"},
    {TOKreturn, 0x4db8bd60, L"return"},
    {TOKinfinity, 0x5c0a010a, L"infinity"},
    {TOKendwhile, 0x5c64bff0, L"endwhile"},
    {TOKforeach, 0x67e31f38, L"foreach"},
    {TOKendfunc, 0x68f984a3, L"endfunc"},
    {TOKelseif, 0x78253218, L"elseif"},
    {TOKwhile, 0x84229259, L"while"},
    {TOKendfor, 0x8ab49d7e, L"endfor"},
    {TOKthrow, 0x8db05c94, L"throw"},
    {TOKstep, 0xa7a7887c, L"step"},
    {TOKupto, 0xb5155328, L"upto"},
    {TOKcontinue, 0xc0340685, L"continue"},
    {TOKfunc, 0xcdce60ec, L"func"},
    {TOKendif, 0xe0e8fee6, L"endif"},
};

const XFA_FM_TOKEN KEYWORD_START = TOKdo;
const XFA_FM_TOKEN KEYWORD_END = TOKendif;

const char* tokenStrings[]{
    "TOKand",        "TOKlparen",     "TOKrparen",   "TOKmul",
    "TOKplus",       "TOKcomma",      "TOKminus",    "TOKdot",
    "TOKdiv",        "TOKlt",         "TOKassign",   "TOKgt",
    "TOKlbracket",   "TOKrbracket",   "TOKor",       "TOKdotscream",
    "TOKdotstar",    "TOKdotdot",     "TOKle",       "TOKne",
    "TOKeq",         "TOKge",         "TOKdo",       "TOKkseq",
    "TOKksge",       "TOKksgt",       "TOKif",       "TOKin",
    "TOKksle",       "TOKkslt",       "TOKksne",     "TOKksor",
    "TOKnull",       "TOKbreak",      "TOKksand",    "TOKend",
    "TOKeof",        "TOKfor",        "TOKnan",      "TOKksnot",
    "TOKvar",        "TOKthen",       "TOKelse",     "TOKexit",
    "TOKdownto",     "TOKreturn",     "TOKinfinity", "TOKendwhile",
    "TOKforeach",    "TOKendfunc",    "TOKelseif",   "TOKwhile",
    "TOKendfor",     "TOKthrow",      "TOKstep",     "TOKupto",
    "TOKcontinue",   "TOKfunc",       "TOKendif",    "TOKstar",
    "TOKidentifier", "TOKunderscore", "TOKdollar",   "TOKexclamation",
    "TOKcall",       "TOKstring",     "TOKnumber",   "TOKreserver",
};

}  // namespace

const char* XFA_FM_TokenToString(XFA_FM_TOKEN tok) {
  return tokenStrings[tok];
}

const wchar_t* XFA_FM_KeywordToString(XFA_FM_TOKEN op) {
  if (op < KEYWORD_START || op > KEYWORD_END)
    return L"";
  return keyWords[op].m_keyword;
}

XFA_FM_TOKEN TokenizeIdentifier(const CFX_WideStringC& identifier) {
  uint32_t key = FX_HashCode_GetW(identifier, true);
  auto cmpFunc = [](const XFA_FMKeyword& iter, const uint32_t& val) {
    return iter.m_uHash < val;
  };

  const XFA_FMKeyword* result = std::lower_bound(
      std::begin(keyWords) + KEYWORD_START, std::end(keyWords), key, cmpFunc);
  if (result <= keyWords + KEYWORD_END && result->m_uHash == key) {
    return result->m_type;
  }
  return TOKidentifier;
}

CXFA_FMToken::CXFA_FMToken() : m_type(TOKreserver), m_uLinenum(1) {}

CXFA_FMToken::CXFA_FMToken(uint32_t uLineNum)
    : m_type(TOKreserver), m_uLinenum(uLineNum) {}

CXFA_FMToken::~CXFA_FMToken() {}

CXFA_FMLexer::CXFA_FMLexer(const CFX_WideString& wsFormCalc)
    : m_input(wsFormCalc),
      m_uCurrentLine(1),
      m_Token(pdfium::MakeUnique<CXFA_FMToken>(1)) {
  m_ptr = m_input.c_str();
}

CXFA_FMLexer::~CXFA_FMLexer() {}

CXFA_FMToken* CXFA_FMLexer::NextToken() {
  // Do not scan anymore, once an error has occured.
  if (!m_Token)
    return nullptr;

  // Make sure we don't walk off the end of the string.
  if (!*m_ptr) {
    m_Token = pdfium::MakeUnique<CXFA_FMToken>(m_uCurrentLine);
    m_Token->m_type = TOKeof;
  } else {
    ScanForNextToken();
  }
  return m_Token.get();
}

void CXFA_FMLexer::ScanForNextToken() {
  m_Token = pdfium::MakeUnique<CXFA_FMToken>(m_uCurrentLine);
  if (!IsFormCalcCharacter(*m_ptr)) {
    m_Token = nullptr;
    return;
  }
  while (1) {
    // Make sure we don't walk off the end of the string. If we don't currently
    // have a token type then mark it EOF.
    if (!*m_ptr) {
      if (m_Token->m_type == TOKreserver)
        m_Token->m_type = TOKeof;
      return;
    }

    if (!IsFormCalcCharacter(*m_ptr)) {
      m_Token = nullptr;
    }

    if (IsWhitespaceCharacter(*m_ptr)) {
      ++m_ptr;
      continue;
    }

    switch (*m_ptr) {
      case 0x0A:
        ++m_uCurrentLine;
        m_Token->m_uLinenum = m_uCurrentLine;
        ++m_ptr;
        break;
      case 0x0D:
        ++m_ptr;
        break;
      case ';': {
        AdvanceForComment();
        break;
      }
      case '"': {
        m_Token->m_type = TOKstring;
        AdvanceForString();
        return;
      }
      case '0':
      case '1':
      case '2':
      case '3':
      case '4':
      case '5':
      case '6':
      case '7':
      case '8':
      case '9': {
        m_Token->m_type = TOKnumber;
        AdvanceForNumber();
        return;
      }
      case '=':
        ++m_ptr;
        if (!*m_ptr) {
          m_Token->m_type = TOKassign;
          return;
        }

        if (IsFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '=') {
            m_Token->m_type = TOKeq;
            ++m_ptr;
          } else {
            m_Token->m_type = TOKassign;
          }
        } else {
          m_Token = nullptr;
        }
        return;
      case '<':
        ++m_ptr;
        if (!*m_ptr) {
          m_Token->m_type = TOKlt;
          return;
        }

        if (IsFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '=') {
            m_Token->m_type = TOKle;
            ++m_ptr;
          } else if (*m_ptr == '>') {
            m_Token->m_type = TOKne;
            ++m_ptr;
          } else {
            m_Token->m_type = TOKlt;
          }
        } else {
          m_Token = nullptr;
        }
        return;
      case '>':
        ++m_ptr;
        if (!*m_ptr) {
          m_Token->m_type = TOKgt;
          return;
        }

        if (IsFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '=') {
            m_Token->m_type = TOKge;
            ++m_ptr;
          } else {
            m_Token->m_type = TOKgt;
          }
        } else {
          m_Token = nullptr;
        }
        return;
      case ',':
        m_Token->m_type = TOKcomma;
        ++m_ptr;
        return;
      case '(':
        m_Token->m_type = TOKlparen;
        ++m_ptr;
        return;
      case ')':
        m_Token->m_type = TOKrparen;
        ++m_ptr;
        return;
      case '[':
        m_Token->m_type = TOKlbracket;
        ++m_ptr;
        return;
      case ']':
        m_Token->m_type = TOKrbracket;
        ++m_ptr;
        return;
      case '&':
        m_Token->m_type = TOKand;
        ++m_ptr;
        return;
      case '|':
        m_Token->m_type = TOKor;
        ++m_ptr;
        return;
      case '+':
        m_Token->m_type = TOKplus;
        ++m_ptr;
        return;
      case '-':
        m_Token->m_type = TOKminus;
        ++m_ptr;
        return;
      case '*':
        m_Token->m_type = TOKmul;
        ++m_ptr;
        return;
      case '/': {
        ++m_ptr;
        if (!*m_ptr) {
          m_Token->m_type = TOKdiv;
          return;
        }

        if (!IsFormCalcCharacter(*m_ptr)) {
          m_Token = nullptr;
          return;
        }

        if (*m_ptr != '/') {
          m_Token->m_type = TOKdiv;
          return;
        }

        AdvanceForComment();
        break;
      }
      case '.':
        ++m_ptr;
        if (!*m_ptr) {
          m_Token->m_type = TOKdot;
          return;
        }

        if (IsFormCalcCharacter(*m_ptr)) {
          if (*m_ptr == '.') {
            m_Token->m_type = TOKdotdot;
            ++m_ptr;
          } else if (*m_ptr == '*') {
            m_Token->m_type = TOKdotstar;
            ++m_ptr;
          } else if (*m_ptr == '#') {
            m_Token->m_type = TOKdotscream;
            ++m_ptr;
          } else if (*m_ptr <= '9' && *m_ptr >= '0') {
            m_Token->m_type = TOKnumber;
            --m_ptr;
            AdvanceForNumber();
          } else {
            m_Token->m_type = TOKdot;
          }
        } else {
          m_Token = nullptr;
        }
        return;
      default: {
        if (!IsInitialIdentifierCharacter(*m_ptr)) {
          m_Token = nullptr;
          return;
        }
        AdvanceForIdentifier();
        return;
      }
    }
  }
}

void CXFA_FMLexer::AdvanceForNumber() {
  const wchar_t* start = m_ptr;
  wchar_t* end = nullptr;

  wcstod(start, &end);
  if (end == nullptr) {
    m_Token = nullptr;
    return;
  }

  m_Token->m_wstring = CFX_WideStringC(start, (end - start));
  m_ptr = end;
}

void CXFA_FMLexer::AdvanceForString() {
  const wchar_t* start = m_ptr;
  ++m_ptr;

  while (*m_ptr) {
    if (!IsFormCalcCharacter(*m_ptr)) {
      m_Token = nullptr;
      return;
    }

    // Need to determine if this is the end of the string or not
    if (*m_ptr == '"') {
      // Checking for double "s, since this another way of escaping them
      ++m_ptr;
      if (*m_ptr != '"') {
        m_Token->m_wstring = CFX_WideStringC(start, (m_ptr - start));
        return;
      }
    }
    ++m_ptr;
  }
  m_Token = nullptr;
}

void CXFA_FMLexer::AdvanceForIdentifier() {
  const wchar_t* start = m_ptr;
  ++m_ptr;

  while (*m_ptr) {
    if (!IsFormCalcCharacter(*m_ptr)) {
      m_Token = nullptr;
      return;
    }

    if (!IsIdentifierCharacter(*m_ptr)) {
      break;
    }
    m_ptr++;
  }

  m_Token->m_wstring = CFX_WideStringC(start, (m_ptr - start));
  m_Token->m_type = TokenizeIdentifier(m_Token->m_wstring);
}

void CXFA_FMLexer::AdvanceForComment() {
  ++m_ptr;

  while (*m_ptr) {
    if (!IsFormCalcCharacter(*m_ptr)) {
      m_Token = nullptr;
      return;
    }

    if (*m_ptr == L'\r')
      break;

    if (*m_ptr == L'\n') {
      ++m_uCurrentLine;
      break;
    }
    ++m_ptr;
  }
}
