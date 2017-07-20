// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmparse.h"

#include <memory>
#include <utility>
#include <vector>

#include "third_party/base/ptr_util.h"

namespace {

const int kMaxAssignmentChainLength = 12;

}  // namespace

CXFA_FMParse::CXFA_FMParse(const CFX_WideString& wsFormcalc)
    : m_ParserError(false) {
  m_lexer = pdfium::MakeUnique<CXFA_FMLexer>(wsFormcalc);
  m_Token = m_lexer->NextToken();
}

CXFA_FMParse::~CXFA_FMParse() {}

std::unique_ptr<CXFA_FMFunctionDefinition> CXFA_FMParse::Parse() {
  if (HasError())
    return nullptr;

  auto expressions = ParseTopExpression();
  if (HasError())
    return nullptr;

  std::vector<CFX_WideStringC> arguments;
  return pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
      1, true, L"", std::move(arguments), std::move(expressions));
}

void CXFA_FMParse::NextToken() {
  m_Token = m_lexer->NextToken();
  while (!HasError() && m_Token->m_type == TOKreserver)
    m_Token = m_lexer->NextToken();
}

void CXFA_FMParse::Check(XFA_FM_TOKEN op) {
  if (HasError())
    return;

  if (m_Token->m_type != op) {
    m_ParserError = true;
    return;
  }
  NextToken();
}

std::vector<std::unique_ptr<CXFA_FMExpression>>
CXFA_FMParse::ParseTopExpression() {
  std::unique_ptr<CXFA_FMExpression> expr;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  if (HasError())
    return expressions;

  while (!HasError()) {
    if (m_Token->m_type == TOKeof || m_Token->m_type == TOKendfunc ||
        m_Token->m_type == TOKendif || m_Token->m_type == TOKelseif ||
        m_Token->m_type == TOKelse || m_Token->m_type == TOKreserver) {
      return expressions;
    }

    expr = m_Token->m_type == TOKfunc ? ParseFunction() : ParseExpression();
    if (HasError() || !expr)
      break;
    expressions.push_back(std::move(expr));
  }
  return expressions;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseFunction() {
  if (HasError())
    return nullptr;

  CFX_WideStringC ident;
  std::vector<CFX_WideStringC> arguments;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;
  uint32_t line = m_Token->m_uLinenum;

  NextToken();
  if (HasError())
    return nullptr;

  if (m_Token->m_type != TOKidentifier) {
    m_ParserError = true;
    return nullptr;
  } else {
    ident = m_Token->m_wstring;
    NextToken();
    if (HasError())
      return nullptr;
  }
  Check(TOKlparen);
  if (HasError())
    return nullptr;

  if (m_Token->m_type == TOKrparen) {
    NextToken();
    if (HasError())
      return nullptr;
  } else {
    while (!HasError()) {
      if (m_Token->m_type == TOKidentifier) {
        arguments.push_back(m_Token->m_wstring);
        if (HasError())
          return nullptr;
        if (m_Token->m_type == TOKcomma) {
          NextToken();
          if (HasError())
            return nullptr;
          continue;
        }
        if (m_Token->m_type == TOKrparen)
          NextToken();
        if (HasError())
          return nullptr;
        else
          Check(TOKrparen);
        if (HasError())
          return nullptr;
      } else {
        m_ParserError = true;
        return nullptr;
      }
      break;
    }
  }
  Check(TOKdo);
  if (HasError())
    return nullptr;

  if (m_Token->m_type == TOKendfunc) {
    NextToken();
    if (HasError())
      return nullptr;
  } else {
    expressions = ParseTopExpression();
    Check(TOKendfunc);
    if (HasError())
      return nullptr;
  }

  return pdfium::MakeUnique<CXFA_FMFunctionDefinition>(
      line, false, ident, std::move(arguments), std::move(expressions));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseExpression() {
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr;
  uint32_t line = m_Token->m_uLinenum;
  switch (m_Token->m_type) {
    case TOKvar:
      expr = ParseVarExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKnull:
    case TOKnumber:
    case TOKstring:
    case TOKplus:
    case TOKminus:
    case TOKksnot:
    case TOKidentifier:
    case TOKlparen:
      expr = ParseExpExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKif:
      expr = ParseIfExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKwhile:
      expr = ParseWhileExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKfor:
      expr = ParseForExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKforeach:
      expr = ParseForeachExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKdo:
      expr = ParseDoExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKbreak:
      expr = pdfium::MakeUnique<CXFA_FMBreakExpression>(line);
      NextToken();
      if (HasError())
        return nullptr;
      break;
    case TOKcontinue:
      expr = pdfium::MakeUnique<CXFA_FMContinueExpression>(line);
      NextToken();
      if (HasError())
        return nullptr;
      break;
    default:
      m_ParserError = true;
      return nullptr;
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseVarExpression() {
  if (HasError())
    return nullptr;

  CFX_WideStringC ident;
  uint32_t line = m_Token->m_uLinenum;
  NextToken();
  if (HasError())
    return nullptr;

  if (m_Token->m_type != TOKidentifier) {
    m_ParserError = true;
    return nullptr;
  } else {
    ident = m_Token->m_wstring;
    NextToken();
    if (HasError())
      return nullptr;
  }
  std::unique_ptr<CXFA_FMExpression> expr;
  if (m_Token->m_type == TOKassign) {
    NextToken();
    if (HasError())
      return nullptr;
    expr = ParseExpExpression();
    if (HasError())
      return nullptr;
  }

  return pdfium::MakeUnique<CXFA_FMVarExpression>(line, ident, std::move(expr));
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseSimpleExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseLogicalOrExpression();
  if (HasError())
    return nullptr;

  int level = 1;
  while (m_Token->m_type == TOKassign) {
    NextToken();
    if (HasError())
      return nullptr;

    std::unique_ptr<CXFA_FMSimpleExpression> pExp2 = ParseLogicalOrExpression();
    if (HasError())
      return nullptr;

    if (level++ == kMaxAssignmentChainLength) {
      m_ParserError = true;
      return nullptr;
    }
    pExp1 = pdfium::MakeUnique<CXFA_FMAssignExpression>(
        line, TOKassign, std::move(pExp1), std::move(pExp2));
  }
  return pExp1;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseExpExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseSimpleExpression();
  if (HasError())
    return nullptr;

  return pdfium::MakeUnique<CXFA_FMExpExpression>(line, std::move(pExp1));
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseLogicalOrExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseLogicalAndExpression();
  if (HasError())
    return nullptr;

  for (;;) {
    switch (m_Token->m_type) {
      case TOKor:
      case TOKksor: {
        NextToken();
        if (HasError())
          return nullptr;

        std::unique_ptr<CXFA_FMSimpleExpression> e2(
            ParseLogicalAndExpression());
        if (HasError())
          return nullptr;

        e1 = pdfium::MakeUnique<CXFA_FMLogicalOrExpression>(
            line, TOKor, std::move(e1), std::move(e2));
        continue;
      }
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseLogicalAndExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseEqualityExpression();
  if (HasError())
    return nullptr;

  for (;;) {
    switch (m_Token->m_type) {
      case TOKand:
      case TOKksand: {
        NextToken();
        std::unique_ptr<CXFA_FMSimpleExpression> e2 = ParseEqualityExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMLogicalAndExpression>(
            line, TOKand, std::move(e1), std::move(e2));
        continue;
      }
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseEqualityExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseRelationalExpression();
  if (HasError())
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_Token->m_type) {
      case TOKeq:
      case TOKkseq:
        NextToken();
        if (HasError())
          return nullptr;

        e2 = ParseRelationalExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMEqualityExpression>(
            line, TOKeq, std::move(e1), std::move(e2));

        continue;
      case TOKne:
      case TOKksne:
        NextToken();
        if (HasError())
          return nullptr;

        e2 = ParseRelationalExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMEqualityExpression>(
            line, TOKne, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseRelationalExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseAddtiveExpression();
  if (HasError())
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_Token->m_type) {
      case TOKlt:
      case TOKkslt:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseAddtiveExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKlt, std::move(e1), std::move(e2));
        continue;
      case TOKgt:
      case TOKksgt:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseAddtiveExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKgt, std::move(e1), std::move(e2));
        continue;
      case TOKle:
      case TOKksle:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseAddtiveExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKle, std::move(e1), std::move(e2));
        continue;
      case TOKge:
      case TOKksge:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseAddtiveExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMRelationalExpression>(
            line, TOKge, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseAddtiveExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseMultiplicativeExpression();
  if (HasError())
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_Token->m_type) {
      case TOKplus:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseMultiplicativeExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMAdditiveExpression>(
            line, TOKplus, std::move(e1), std::move(e2));
        continue;
      case TOKminus:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseMultiplicativeExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMAdditiveExpression>(
            line, TOKminus, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseMultiplicativeExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> e1 = ParseUnaryExpression();
  if (HasError())
    return nullptr;

  for (;;) {
    std::unique_ptr<CXFA_FMSimpleExpression> e2;
    switch (m_Token->m_type) {
      case TOKmul:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseUnaryExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMMultiplicativeExpression>(
            line, TOKmul, std::move(e1), std::move(e2));
        continue;
      case TOKdiv:
        NextToken();
        if (HasError())
          return nullptr;
        e2 = ParseUnaryExpression();
        if (HasError())
          return nullptr;
        e1 = pdfium::MakeUnique<CXFA_FMMultiplicativeExpression>(
            line, TOKdiv, std::move(e1), std::move(e2));
        continue;
      default:
        break;
    }
    break;
  }
  return e1;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseUnaryExpression() {
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  uint32_t line = m_Token->m_uLinenum;
  switch (m_Token->m_type) {
    case TOKplus:
      NextToken();
      if (HasError())
        return nullptr;
      expr = ParseUnaryExpression();
      if (HasError())
        return nullptr;
      expr = pdfium::MakeUnique<CXFA_FMPosExpression>(line, std::move(expr));
      break;
    case TOKminus:
      NextToken();
      if (HasError())
        return nullptr;
      expr = ParseUnaryExpression();
      if (HasError())
        return nullptr;
      expr = pdfium::MakeUnique<CXFA_FMNegExpression>(line, std::move(expr));
      break;
    case TOKksnot:
      NextToken();
      if (HasError())
        return nullptr;
      expr = ParseUnaryExpression();
      if (HasError())
        return nullptr;
      expr = pdfium::MakeUnique<CXFA_FMNotExpression>(line, std::move(expr));
      break;
    default:
      expr = ParsePrimaryExpression();
      if (HasError())
        return nullptr;
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParsePrimaryExpression() {
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  uint32_t line = m_Token->m_uLinenum;
  switch (m_Token->m_type) {
    case TOKnumber:
      expr =
          pdfium::MakeUnique<CXFA_FMNumberExpression>(line, m_Token->m_wstring);
      NextToken();
      if (HasError())
        return nullptr;
      break;
    case TOKstring:
      expr =
          pdfium::MakeUnique<CXFA_FMStringExpression>(line, m_Token->m_wstring);
      NextToken();
      if (HasError())
        return nullptr;
      break;
    case TOKidentifier: {
      CFX_WideStringC wsIdentifier(m_Token->m_wstring);
      NextToken();
      if (HasError())
        return nullptr;
      if (m_Token->m_type == TOKlbracket) {
        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (HasError())
          return nullptr;
        if (s) {
          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              line, nullptr, TOKdot, wsIdentifier, std::move(s));
        }
        NextToken();
        if (HasError())
          return nullptr;
      } else {
        expr =
            pdfium::MakeUnique<CXFA_FMIdentifierExpression>(line, wsIdentifier);
      }
      break;
    }
    case TOKif:
      expr = pdfium::MakeUnique<CXFA_FMIdentifierExpression>(
          line, m_Token->m_wstring);
      NextToken();
      if (HasError())
        return nullptr;
      break;
    case TOKnull:
      expr = pdfium::MakeUnique<CXFA_FMNullExpression>(line);
      NextToken();
      if (HasError())
        return nullptr;
      break;
    case TOKlparen:
      expr = ParseParenExpression();
      if (HasError())
        return nullptr;
      break;
    default:
      m_ParserError = true;
      NextToken();
      if (HasError())
        return nullptr;
      break;
  }
  if (HasError())
    return nullptr;
  return ParsePostExpression(std::move(expr));
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParsePostExpression(
    std::unique_ptr<CXFA_FMSimpleExpression> expr) {
  uint32_t line = m_Token->m_uLinenum;
  while (1) {
    switch (m_Token->m_type) {
      case TOKlparen: {
        NextToken();
        std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> expressions;
        if (m_Token->m_type != TOKrparen) {
          while (m_Token->m_type != TOKrparen) {
            if (std::unique_ptr<CXFA_FMSimpleExpression> expr =
                    ParseSimpleExpression())
              expressions.push_back(std::move(expr));
            if (HasError())
              return nullptr;
            if (m_Token->m_type == TOKcomma) {
              NextToken();
              if (HasError())
                return nullptr;
            } else if (m_Token->m_type == TOKeof ||
                       m_Token->m_type == TOKreserver) {
              break;
            }
          }
          if (m_Token->m_type != TOKrparen) {
            m_ParserError = true;
            return nullptr;
          }
        }
        if (HasError())
          return nullptr;
        expr = pdfium::MakeUnique<CXFA_FMCallExpression>(
            line, std::move(expr), std::move(expressions), false);
        NextToken();
        if (HasError())
          return nullptr;
        if (m_Token->m_type != TOKlbracket)
          continue;
        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (HasError())
          return nullptr;

        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            line, std::move(expr), TOKcall, L"", std::move(s));
        break;
      }
      case TOKdot:
        NextToken();
        if (HasError())
          return nullptr;
        if (m_Token->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_Token->m_wstring;
          uint32_t tempLine = m_Token->m_uLinenum;
          NextToken();
          if (HasError())
            return nullptr;
          if (m_Token->m_type == TOKlparen) {
            std::unique_ptr<CXFA_FMSimpleExpression> pExpCall;
            NextToken();
            if (HasError())
              return nullptr;
            std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> expressions;
            if (m_Token->m_type != TOKrparen) {
              while (m_Token->m_type != TOKrparen) {
                std::unique_ptr<CXFA_FMSimpleExpression> exp =
                    ParseSimpleExpression();
                if (HasError())
                  return nullptr;
                expressions.push_back(std::move(exp));
                if (m_Token->m_type == TOKcomma) {
                  NextToken();
                  if (HasError())
                    return nullptr;
                } else if (m_Token->m_type == TOKeof ||
                           m_Token->m_type == TOKreserver) {
                  break;
                }
              }
              if (m_Token->m_type != TOKrparen) {
                m_ParserError = true;
                return nullptr;
              }
            }
            if (HasError())
              return nullptr;

            std::unique_ptr<CXFA_FMSimpleExpression> pIdentifier =
                pdfium::MakeUnique<CXFA_FMIdentifierExpression>(tempLine,
                                                                tempStr);
            pExpCall = pdfium::MakeUnique<CXFA_FMCallExpression>(
                line, std::move(pIdentifier), std::move(expressions), true);
            expr = pdfium::MakeUnique<CXFA_FMMethodCallExpression>(
                line, std::move(expr), std::move(pExpCall));
            NextToken();
            if (HasError())
              return nullptr;
            if (m_Token->m_type != TOKlbracket)
              continue;

            std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
            if (HasError())
              return nullptr;
            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                line, std::move(expr), TOKcall, L"", std::move(s));
          } else if (m_Token->m_type == TOKlbracket) {
            std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
            if (HasError())
              return nullptr;

            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                tempLine, std::move(expr), TOKdot, tempStr, std::move(s));
          } else {
            std::unique_ptr<CXFA_FMSimpleExpression> s =
                pdfium::MakeUnique<CXFA_FMIndexExpression>(
                    tempLine, ACCESSOR_NO_INDEX, nullptr, false);
            expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
                line, std::move(expr), TOKdot, tempStr, std::move(s));
            continue;
          }
        } else {
          m_ParserError = true;
          return nullptr;
        }
        break;
      case TOKdotdot:
        NextToken();
        if (HasError())
          return nullptr;
        if (m_Token->m_type == TOKidentifier) {
          CFX_WideStringC tempStr = m_Token->m_wstring;
          uint32_t tempLine = m_Token->m_uLinenum;
          NextToken();
          if (m_Token->m_type == TOKlbracket) {
            std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
            if (HasError())
              return nullptr;

            expr = pdfium::MakeUnique<CXFA_FMDotDotAccessorExpression>(
                tempLine, std::move(expr), TOKdotdot, tempStr, std::move(s));
          } else {
            std::unique_ptr<CXFA_FMSimpleExpression> s =
                pdfium::MakeUnique<CXFA_FMIndexExpression>(
                    tempLine, ACCESSOR_NO_INDEX, nullptr, false);
            expr = pdfium::MakeUnique<CXFA_FMDotDotAccessorExpression>(
                line, std::move(expr), TOKdotdot, tempStr, std::move(s));
            continue;
          }
        } else {
          m_ParserError = true;
          return nullptr;
        }
        break;
      case TOKdotscream: {
        NextToken();
        if (HasError())
          return nullptr;
        if (m_Token->m_type != TOKidentifier) {
          m_ParserError = true;
          return nullptr;
        }
        CFX_WideStringC tempStr = m_Token->m_wstring;
        uint32_t tempLine = m_Token->m_uLinenum;
        NextToken();
        if (HasError())
          return nullptr;
        if (m_Token->m_type != TOKlbracket) {
          std::unique_ptr<CXFA_FMSimpleExpression> s =
              pdfium::MakeUnique<CXFA_FMIndexExpression>(
                  tempLine, ACCESSOR_NO_INDEX, nullptr, false);
          expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
              line, std::move(expr), TOKdotscream, tempStr, std::move(s));
          continue;
        }
        std::unique_ptr<CXFA_FMSimpleExpression> s = ParseIndexExpression();
        if (HasError())
          return nullptr;

        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            tempLine, std::move(expr), TOKdotscream, tempStr, std::move(s));
        break;
      }
      case TOKdotstar: {
        std::unique_ptr<CXFA_FMSimpleExpression> s =
            pdfium::MakeUnique<CXFA_FMIndexExpression>(line, ACCESSOR_NO_INDEX,
                                                       nullptr, false);
        expr = pdfium::MakeUnique<CXFA_FMDotAccessorExpression>(
            line, std::move(expr), TOKdotstar, L"*", std::move(s));
        break;
      }
      default:
        return expr;
    }
    NextToken();
    if (HasError())
      return nullptr;
  }
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseIndexExpression() {
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pExp;
  uint32_t line = m_Token->m_uLinenum;
  NextToken();
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMSimpleExpression> s;
  XFA_FM_AccessorIndex accessorIndex = ACCESSOR_NO_RELATIVEINDEX;
  if (m_Token->m_type == TOKmul) {
    pExp = pdfium::MakeUnique<CXFA_FMIndexExpression>(line, accessorIndex,
                                                      std::move(s), true);
    NextToken();
    if (HasError())
      return nullptr;
    if (m_Token->m_type != TOKrbracket) {
      m_ParserError = true;
      return nullptr;
    }
    return pExp;
  }
  if (m_Token->m_type == TOKplus) {
    accessorIndex = ACCESSOR_POSITIVE_INDEX;
    NextToken();
    if (HasError())
      return nullptr;
  } else if (m_Token->m_type == TOKminus) {
    accessorIndex = ACCESSOR_NEGATIVE_INDEX;
    NextToken();
    if (HasError())
      return nullptr;
  }
  s = ParseSimpleExpression();
  if (HasError())
    return nullptr;
  if (m_Token->m_type != TOKrbracket) {
    m_ParserError = true;
    return nullptr;
  }
  return pdfium::MakeUnique<CXFA_FMIndexExpression>(line, accessorIndex,
                                                    std::move(s), false);
}

std::unique_ptr<CXFA_FMSimpleExpression> CXFA_FMParse::ParseParenExpression() {
  Check(TOKlparen);
  if (HasError())
    return nullptr;

  if (m_Token->m_type == TOKrparen) {
    m_ParserError = true;
    return nullptr;
  }

  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMSimpleExpression> pExp1 = ParseLogicalOrExpression();
  if (HasError())
    return nullptr;

  int level = 1;
  while (m_Token->m_type == TOKassign) {
    NextToken();
    if (HasError())
      return nullptr;
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2 = ParseLogicalOrExpression();
    if (HasError())
      return nullptr;
    if (level++ == kMaxAssignmentChainLength) {
      m_ParserError = true;
      return nullptr;
    }
    if (HasError())
      return nullptr;
    pExp1 = pdfium::MakeUnique<CXFA_FMAssignExpression>(
        line, TOKassign, std::move(pExp1), std::move(pExp2));
  }
  Check(TOKrparen);
  if (HasError())
    return nullptr;
  return pExp1;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseBlockExpression() {
  if (HasError())
    return nullptr;
  uint32_t line = m_Token->m_uLinenum;
  std::unique_ptr<CXFA_FMExpression> expr;
  std::vector<std::unique_ptr<CXFA_FMExpression>> expressions;

  while (1) {
    switch (m_Token->m_type) {
      case TOKeof:
      case TOKendif:
      case TOKelseif:
      case TOKelse:
      case TOKendwhile:
      case TOKendfor:
      case TOKend:
      case TOKendfunc:
      case TOKreserver:
        break;
      case TOKfunc:
        expr = ParseFunction();
        if (HasError())
          return nullptr;
        expressions.push_back(std::move(expr));
        continue;
      default:
        expr = ParseExpression();
        if (HasError())
          return nullptr;
        expressions.push_back(std::move(expr));
        continue;
    }
    break;
  }
  return pdfium::MakeUnique<CXFA_FMBlockExpression>(line,
                                                    std::move(expressions));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseIfExpression() {
  // TODO(rharrison): Clean up how if expressions are parsed. This whole
  // reaching in to the lexer and moving the current position/token is error
  // prone and makes it hard to reason about the parsing process.
  // See https://crbug.com/pdfium/823 for details.
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  const wchar_t* pStartPos = m_lexer->GetPos();
  NextToken();
  if (HasError())
    return nullptr;
  Check(TOKlparen);
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMSimpleExpression> pExpression;
  while (m_Token->m_type != TOKrparen) {
    pExpression = ParseSimpleExpression();
    if (HasError())
      return nullptr;
    if (m_Token->m_type != TOKcomma)
      break;
    NextToken();
    if (HasError())
      return nullptr;
  }
  Check(TOKrparen);
  if (HasError())
    return nullptr;
  if (m_Token->m_type != TOKthen) {
    m_lexer->SetCurrentLine(line);
    auto pNewToken = pdfium::MakeUnique<CXFA_FMToken>(line);
    m_Token = pNewToken.get();
    m_Token->m_type = TOKidentifier;
    m_Token->m_wstring = L"if";
    m_lexer->SetToken(std::move(pNewToken));
    m_lexer->SetPos(pStartPos);
    return ParseExpExpression();
  }
  Check(TOKthen);
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMExpression> pIfExpression = ParseBlockExpression();
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMExpression> pElseExpression;
  switch (m_Token->m_type) {
    case TOKeof:
    case TOKendif:
      Check(TOKendif);
      if (HasError())
        return nullptr;
      break;
    case TOKif:
      pElseExpression = ParseIfExpression();
      if (HasError())
        return nullptr;
      Check(TOKendif);
      if (HasError())
        return nullptr;
      break;
    case TOKelseif:
      pElseExpression = ParseIfExpression();
      if (HasError())
        return nullptr;
      break;
    case TOKelse:
      NextToken();
      if (HasError())
        return nullptr;
      pElseExpression = ParseBlockExpression();
      if (HasError())
        return nullptr;
      Check(TOKendif);
      if (HasError())
        return nullptr;
      break;
    default:
      m_ParserError = true;
      return nullptr;
      break;
  }
  if (HasError())
    return nullptr;
  return pdfium::MakeUnique<CXFA_FMIfExpression>(line, std::move(pExpression),
                                                 std::move(pIfExpression),
                                                 std::move(pElseExpression));
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseWhileExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  NextToken();
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMSimpleExpression> pCondition = ParseParenExpression();
  Check(TOKdo);
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMExpression> pExpression = ParseBlockExpression();
  Check(TOKendwhile);
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr =
      pdfium::MakeUnique<CXFA_FMWhileExpression>(line, std::move(pCondition),
                                                 std::move(pExpression));
  return expr;
}

std::unique_ptr<CXFA_FMSimpleExpression>
CXFA_FMParse::ParseSubassignmentInForExpression() {
  std::unique_ptr<CXFA_FMSimpleExpression> expr;
  switch (m_Token->m_type) {
    case TOKidentifier:
      expr = ParseSimpleExpression();
      if (HasError())
        return nullptr;
      break;
    default:
      m_ParserError = true;
      return nullptr;
      break;
  }
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseForExpression() {
  if (HasError())
    return nullptr;

  CFX_WideStringC wsVariant;
  uint32_t line = m_Token->m_uLinenum;
  NextToken();
  if (HasError())
    return nullptr;
  if (m_Token->m_type != TOKidentifier) {
    m_ParserError = true;
    return nullptr;
  }

  wsVariant = m_Token->m_wstring;
  NextToken();
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMSimpleExpression> pAssignment;
  if (m_Token->m_type == TOKassign) {
    NextToken();
    if (HasError())
      return nullptr;
    pAssignment = ParseSimpleExpression();
    if (HasError())
      return nullptr;
  } else {
    m_ParserError = true;
    return nullptr;
  }

  int32_t iDirection = 0;
  if (m_Token->m_type == TOKupto) {
    iDirection = 1;
  } else if (m_Token->m_type == TOKdownto) {
    iDirection = -1;
  } else {
    m_ParserError = true;
    return nullptr;
  }

  NextToken();
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMSimpleExpression> pAccessor = ParseSimpleExpression();
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMSimpleExpression> pStep;
  if (m_Token->m_type == TOKstep) {
    NextToken();
    if (HasError())
      return nullptr;
    pStep = ParseSimpleExpression();
    if (HasError())
      return nullptr;
  }
  Check(TOKdo);
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> pList = ParseBlockExpression();
  if (HasError())
    return nullptr;
  Check(TOKendfor);
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr =
      pdfium::MakeUnique<CXFA_FMForExpression>(
          line, wsVariant, std::move(pAssignment), std::move(pAccessor),
          iDirection, std::move(pStep), std::move(pList));
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseForeachExpression() {
  if (HasError())
    return nullptr;

  std::unique_ptr<CXFA_FMExpression> expr;
  CFX_WideStringC wsIdentifier;
  std::vector<std::unique_ptr<CXFA_FMSimpleExpression>> pAccessors;
  std::unique_ptr<CXFA_FMExpression> pList;
  uint32_t line = m_Token->m_uLinenum;
  NextToken();
  if (HasError())
    return nullptr;
  if (m_Token->m_type != TOKidentifier) {
    m_ParserError = true;
    return nullptr;
  }

  wsIdentifier = m_Token->m_wstring;
  NextToken();
  if (HasError())
    return nullptr;
  Check(TOKin);
  if (HasError())
    return nullptr;
  Check(TOKlparen);
  if (HasError())
    return nullptr;

  if (m_Token->m_type == TOKrparen) {
    m_ParserError = true;
    return nullptr;
  } else {
    while (m_Token->m_type != TOKrparen) {
      std::unique_ptr<CXFA_FMSimpleExpression> s = ParseSimpleExpression();
      if (HasError())
        return nullptr;
      pAccessors.push_back(std::move(s));
      if (m_Token->m_type != TOKcomma)
        break;
      NextToken();
      if (HasError())
        return nullptr;
    }
    Check(TOKrparen);
    if (HasError())
      return nullptr;
  }
  Check(TOKdo);
  if (HasError())
    return nullptr;

  pList = ParseBlockExpression();
  if (HasError())
    return nullptr;
  Check(TOKendfor);
  if (HasError())
    return nullptr;

  expr = pdfium::MakeUnique<CXFA_FMForeachExpression>(
      line, wsIdentifier, std::move(pAccessors), std::move(pList));
  return expr;
}

std::unique_ptr<CXFA_FMExpression> CXFA_FMParse::ParseDoExpression() {
  if (HasError())
    return nullptr;

  uint32_t line = m_Token->m_uLinenum;
  NextToken();
  if (HasError())
    return nullptr;
  std::unique_ptr<CXFA_FMExpression> expr = ParseBlockExpression();
  if (HasError())
    return nullptr;
  Check(TOKend);
  if (HasError())
    return nullptr;

  return pdfium::MakeUnique<CXFA_FMDoExpression>(line, std::move(expr));
}

bool CXFA_FMParse::HasError() const {
  return m_ParserError || !m_Token;
}
