// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_FM2JS_CXFA_FMLEXER_H_
#define XFA_FXFA_FM2JS_CXFA_FMLEXER_H_

#include <memory>
#include <utility>

#include "core/fxcrt/fx_string.h"

enum XFA_FM_TOKEN {
  TOKand,
  TOKlparen,
  TOKrparen,
  TOKmul,
  TOKplus,
  TOKcomma,
  TOKminus,
  TOKdot,
  TOKdiv,
  TOKlt,
  TOKassign,
  TOKgt,
  TOKlbracket,
  TOKrbracket,
  TOKor,
  TOKdotscream,
  TOKdotstar,
  TOKdotdot,
  TOKle,
  TOKne,
  TOKeq,
  TOKge,
  TOKdo,
  TOKkseq,
  TOKksge,
  TOKksgt,
  TOKif,
  TOKin,
  TOKksle,
  TOKkslt,
  TOKksne,
  TOKksor,
  TOKnull,
  TOKbreak,
  TOKksand,
  TOKend,
  TOKeof,
  TOKfor,
  TOKnan,
  TOKksnot,
  TOKvar,
  TOKthen,
  TOKelse,
  TOKexit,
  TOKdownto,
  TOKreturn,
  TOKinfinity,
  TOKendwhile,
  TOKforeach,
  TOKendfunc,
  TOKelseif,
  TOKwhile,
  TOKendfor,
  TOKthrow,
  TOKstep,
  TOKupto,
  TOKcontinue,
  TOKfunc,
  TOKendif,
  TOKstar,
  TOKidentifier,
  TOKunderscore,
  TOKdollar,
  TOKexclamation,
  TOKcall,
  TOKstring,
  TOKnumber,
  TOKreserver
};

const char* XFA_FM_TokenToString(XFA_FM_TOKEN tok);

struct XFA_FMKeyword {
  XFA_FM_TOKEN m_type;
  uint32_t m_uHash;
  const wchar_t* m_keyword;
};

const wchar_t* XFA_FM_KeywordToString(XFA_FM_TOKEN op);

XFA_FM_TOKEN TokenizeIdentifier(const CFX_WideStringC& identifier);

class CXFA_FMToken {
 public:
  CXFA_FMToken();
  explicit CXFA_FMToken(uint32_t uLineNum);
  ~CXFA_FMToken();

  CFX_WideStringC m_wstring;
  XFA_FM_TOKEN m_type;
  uint32_t m_uLinenum;
};

class CXFA_FMLexer {
 public:
  explicit CXFA_FMLexer(const CFX_WideString& wsFormcalc);
  ~CXFA_FMLexer();

  CXFA_FMToken* NextToken();
  void SetCurrentLine(uint32_t line) { m_uCurrentLine = line; }
  void SetToken(std::unique_ptr<CXFA_FMToken> pToken) {
    m_Token = std::move(pToken);
  }

  const wchar_t* GetPos() { return m_ptr; }
  void SetPos(const wchar_t* pPos) { m_ptr = pPos; }

 private:
  void ScanForNextToken();
  void AdvanceForNumber();
  void AdvanceForString();
  void AdvanceForIdentifier();
  void AdvanceForComment();

  CFX_WideString m_input;
  const wchar_t* m_ptr;
  uint32_t m_uCurrentLine;
  std::unique_ptr<CXFA_FMToken> m_Token;
};

#endif  // XFA_FXFA_FM2JS_CXFA_FMLEXER_H_
