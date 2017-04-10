// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/xfa_expression.h"

#include <utility>

#include "core/fxcrt/fx_basic.h"

namespace {

const wchar_t RUNTIMEBLOCKTEMPARRAY[] =
    L"foxit_xfa_formcalc_runtime_block_temp_array";

const wchar_t RUNTIMEBLOCKTEMPARRAYINDEX[] =
    L"foxit_xfa_formcalc_runtime_block_temp_array_index";

}  // namespace

CXFA_FMExpression::CXFA_FMExpression(uint32_t line)
    : m_type(XFA_FM_EXPTYPE_UNKNOWN), m_line(line) {}

CXFA_FMExpression::CXFA_FMExpression(uint32_t line, XFA_FM_EXPTYPE type)
    : m_type(type), m_line(line) {}

void CXFA_FMExpression::ToJavaScript(CFX_WideTextBuf& javascript) {}

void CXFA_FMExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {}

CXFA_FMFunctionDefinition::CXFA_FMFunctionDefinition(
    uint32_t line,
    bool isGlobal,
    const CFX_WideStringC& wsName,
    std::vector<CFX_WideStringC>&& arguments,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& expressions)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_FUNC),
      m_wsName(wsName),
      m_pArguments(std::move(arguments)),
      m_pExpressions(std::move(expressions)),
      m_isGlobal(isGlobal) {}

CXFA_FMFunctionDefinition::~CXFA_FMFunctionDefinition() {}

void CXFA_FMFunctionDefinition::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  if (m_isGlobal && m_pExpressions.empty()) {
    tempJS << L"// comments only";
    javascript << tempJS;
    m_wsJavascript = tempJS.AsStringC();
    return;
  }
  if (m_isGlobal)
    tempJS << L"(\n";
  tempJS << L"function ";
  if (m_wsName.GetAt(0) == L'!') {
    CFX_WideString tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
    tempJS << tempName;
  } else {
    tempJS << m_wsName;
  }
  tempJS << L"(";
  bool bNeedComma = false;
  for (const auto& identifier : m_pArguments) {
    if (bNeedComma)
      tempJS << L", ";
    if (identifier.GetAt(0) == L'!') {
      CFX_WideString tempIdentifier =
          EXCLAMATION_IN_IDENTIFIER + identifier.Mid(1);
      tempJS << tempIdentifier;
    } else {
      tempJS << identifier;
    }
    bNeedComma = true;
  }
  tempJS << L")\n{\n";
  tempJS << L"var ";
  tempJS << RUNTIMEFUNCTIONRETURNVALUE;
  tempJS << L" = null;\n";
  for (const auto& expr : m_pExpressions) {
    if (expr == m_pExpressions.back())
      expr->ToImpliedReturnJS(tempJS);
    else
      expr->ToJavaScript(tempJS);
  }
  tempJS << L"return ";
  if (m_isGlobal) {
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    tempJS << RUNTIMEFUNCTIONRETURNVALUE;
    tempJS << L")";
  } else {
    tempJS << RUNTIMEFUNCTIONRETURNVALUE;
  }
  tempJS << L";\n}\n";
  if (m_isGlobal)
    tempJS << L").call(this);\n";
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMFunctionDefinition::ToImpliedReturnJS(CFX_WideTextBuf&) {}

CXFA_FMVarExpression::CXFA_FMVarExpression(
    uint32_t line,
    const CFX_WideStringC& wsName,
    std::unique_ptr<CXFA_FMExpression> pInit)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_VAR),
      m_wsName(wsName),
      m_pInit(std::move(pInit)) {}

CXFA_FMVarExpression::~CXFA_FMVarExpression() {}

void CXFA_FMVarExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"var ";
  CFX_WideString tempName(m_wsName);
  if (m_wsName.GetAt(0) == L'!')
    tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
  tempJS << tempName;
  tempJS << L" = ";
  if (m_pInit) {
    m_pInit->ToJavaScript(tempJS);
    tempJS << tempName;
    tempJS << L" = ";
    tempJS << XFA_FM_EXPTypeToString(VARFILTER);
    tempJS << L"(";
    tempJS << tempName;
    tempJS << L");\n";
  } else {
    tempJS << L"\"\";\n";
  }
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMVarExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << L"var ";
  CFX_WideString tempName(m_wsName);
  if (m_wsName.GetAt(0) == L'!') {
    tempName = EXCLAMATION_IN_IDENTIFIER + m_wsName.Mid(1);
  }
  javascript << tempName;
  javascript << L" = ";
  if (m_pInit) {
    m_pInit->ToJavaScript(javascript);
    javascript << tempName;
    javascript << L" = ";
    javascript << XFA_FM_EXPTypeToString(VARFILTER);
    javascript << L"(";
    javascript << tempName;
    javascript << L");\n";
  } else {
    javascript << L"\"\";\n";
  }
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = ";
  javascript << tempName;
  javascript << L";\n";
}

CXFA_FMExpExpression::CXFA_FMExpExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_EXP),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMExpExpression::~CXFA_FMExpExpression() {}

void CXFA_FMExpExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (m_pExpression->GetOperatorToken() == TOKassign) {
    m_pExpression->ToJavaScript(javascript);
  } else {
    m_pExpression->ToJavaScript(javascript);
    javascript << L";\n";
  }
}

void CXFA_FMExpExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (m_pExpression->GetOperatorToken() == TOKassign) {
    m_pExpression->ToImpliedReturnJS(javascript);
  } else {
    if (m_pExpression->GetOperatorToken() == TOKstar ||
        m_pExpression->GetOperatorToken() == TOKdotstar ||
        m_pExpression->GetOperatorToken() == TOKdotscream ||
        m_pExpression->GetOperatorToken() == TOKdotdot ||
        m_pExpression->GetOperatorToken() == TOKdot) {
      javascript << RUNTIMEFUNCTIONRETURNVALUE;
      javascript << L" = ";
      javascript << XFA_FM_EXPTypeToString(GETFMVALUE);
      javascript << L"(";
      m_pExpression->ToJavaScript(javascript);
      javascript << L");\n";
    } else {
      javascript << RUNTIMEFUNCTIONRETURNVALUE;
      javascript << L" = ";
      m_pExpression->ToJavaScript(javascript);
      javascript << L";\n";
    }
  }
}

CXFA_FMBlockExpression::CXFA_FMBlockExpression(
    uint32_t line,
    std::vector<std::unique_ptr<CXFA_FMExpression>>&& pExpressionList)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BLOCK),
      m_ExpressionList(std::move(pExpressionList)) {}

CXFA_FMBlockExpression::~CXFA_FMBlockExpression() {}

void CXFA_FMBlockExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"{\n";
  for (const auto& expr : m_ExpressionList)
    expr->ToJavaScript(tempJS);
  tempJS << L"}\n";
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMBlockExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (!m_wsImpliedReturnJS.IsEmpty()) {
    javascript << m_wsImpliedReturnJS;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"{\n";
  for (const auto& expr : m_ExpressionList) {
    if (expr == m_ExpressionList.back())
      expr->ToImpliedReturnJS(tempJS);
    else
      expr->ToJavaScript(tempJS);
  }
  tempJS << L"}\n";
  javascript << tempJS;
  m_wsImpliedReturnJS = tempJS.AsStringC();
}

CXFA_FMDoExpression::CXFA_FMDoExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMExpression(line), m_pList(std::move(pList)) {}

CXFA_FMDoExpression::~CXFA_FMDoExpression() {}

void CXFA_FMDoExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  m_pList->ToJavaScript(javascript);
}

void CXFA_FMDoExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  m_pList->ToImpliedReturnJS(javascript);
}

CXFA_FMIfExpression::CXFA_FMIfExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExpression,
    std::unique_ptr<CXFA_FMExpression> pIfExpression,
    std::unique_ptr<CXFA_FMExpression> pElseExpression)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_IF),
      m_pExpression(std::move(pExpression)),
      m_pIfExpression(std::move(pIfExpression)),
      m_pElseExpression(std::move(pElseExpression)) {}

CXFA_FMIfExpression::~CXFA_FMIfExpression() {}

void CXFA_FMIfExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"if (";
  if (m_pExpression) {
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pExpression->ToJavaScript(tempJS);
    tempJS << L")";
  }
  tempJS << L")\n";
  if (m_pIfExpression) {
    m_pIfExpression->ToJavaScript(tempJS);
  }
  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      tempJS << L"else\n";
      tempJS << L"{\n";
      m_pElseExpression->ToJavaScript(tempJS);
      tempJS << L"}\n";
    } else {
      tempJS << L"else\n";
      m_pElseExpression->ToJavaScript(tempJS);
    }
  }
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMIfExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (!m_wsImpliedReturnJS.IsEmpty()) {
    javascript << m_wsImpliedReturnJS;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << RUNTIMEFUNCTIONRETURNVALUE;
  tempJS << L" = 0;\n";
  tempJS << L"if (";
  if (m_pExpression) {
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pExpression->ToJavaScript(tempJS);
    tempJS << L")";
  }
  tempJS << L")\n";
  if (m_pIfExpression) {
    m_pIfExpression->ToImpliedReturnJS(tempJS);
  }
  if (m_pElseExpression) {
    if (m_pElseExpression->GetExpType() == XFA_FM_EXPTYPE_IF) {
      tempJS << L"else\n";
      tempJS << L"{\n";
      m_pElseExpression->ToImpliedReturnJS(tempJS);
      tempJS << L"}\n";
    } else {
      tempJS << L"else\n";
      m_pElseExpression->ToImpliedReturnJS(tempJS);
    }
  }
  javascript << tempJS;
  m_wsImpliedReturnJS = tempJS.AsStringC();
}

CXFA_FMLoopExpression::~CXFA_FMLoopExpression() {}

void CXFA_FMLoopExpression::ToJavaScript(CFX_WideTextBuf& javascript) {}

void CXFA_FMLoopExpression::ToImpliedReturnJS(CFX_WideTextBuf&) {}

CXFA_FMWhileExpression::CXFA_FMWhileExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pCondition,
    std::unique_ptr<CXFA_FMExpression> pExpression)
    : CXFA_FMLoopExpression(line),
      m_pCondition(std::move(pCondition)),
      m_pExpression(std::move(pExpression)) {}

CXFA_FMWhileExpression::~CXFA_FMWhileExpression() {}

void CXFA_FMWhileExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"while (";
  m_pCondition->ToJavaScript(tempJS);
  tempJS << L")\n";
  m_pExpression->ToJavaScript(tempJS);
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMWhileExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (!m_wsImpliedReturnJS.IsEmpty()) {
    javascript << m_wsImpliedReturnJS;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << RUNTIMEFUNCTIONRETURNVALUE;
  tempJS << L" = 0;\n";
  tempJS << L"while (";
  m_pCondition->ToJavaScript(tempJS);
  tempJS << L")\n";
  m_pExpression->ToImpliedReturnJS(tempJS);
  javascript << tempJS;
  m_wsImpliedReturnJS = tempJS.AsStringC();
}

CXFA_FMBreakExpression::CXFA_FMBreakExpression(uint32_t line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_BREAK) {}

CXFA_FMBreakExpression::~CXFA_FMBreakExpression() {}

void CXFA_FMBreakExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"break;\n";
}

void CXFA_FMBreakExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"break;\n";
}

CXFA_FMContinueExpression::CXFA_FMContinueExpression(uint32_t line)
    : CXFA_FMExpression(line, XFA_FM_EXPTYPE_CONTINUE) {}

CXFA_FMContinueExpression::~CXFA_FMContinueExpression() {}

void CXFA_FMContinueExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"continue;\n";
}

void CXFA_FMContinueExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = 0;\n";
  javascript << L"continue;\n";
}

CXFA_FMForExpression::CXFA_FMForExpression(
    uint32_t line,
    const CFX_WideStringC& wsVariant,
    std::unique_ptr<CXFA_FMSimpleExpression> pAssignment,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    int32_t iDirection,
    std::unique_ptr<CXFA_FMSimpleExpression> pStep,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMLoopExpression(line),
      m_wsVariant(wsVariant),
      m_pAssignment(std::move(pAssignment)),
      m_pAccessor(std::move(pAccessor)),
      m_iDirection(iDirection),
      m_pStep(std::move(pStep)),
      m_pList(std::move(pList)) {}

CXFA_FMForExpression::~CXFA_FMForExpression() {}

void CXFA_FMForExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"{\nvar ";
  CFX_WideString tempVariant;
  if (m_wsVariant.GetAt(0) == L'!') {
    tempVariant = EXCLAMATION_IN_IDENTIFIER + m_wsVariant.Mid(1);
    tempJS << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    tempJS << m_wsVariant;
  }
  tempJS << L" = null;\n";
  tempJS << L"for (";
  tempJS << tempVariant;
  tempJS << L" = ";
  tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
  tempJS << L"(";
  m_pAssignment->ToJavaScript(tempJS);
  tempJS << L"); ";
  tempJS << tempVariant;
  if (m_iDirection == 1) {
    tempJS << L" <= ";
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pAccessor->ToJavaScript(tempJS);
    tempJS << L"); ";
    tempJS << tempVariant;
    tempJS << L" += ";
  } else {
    tempJS << L" >= ";
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pAccessor->ToJavaScript(tempJS);
    tempJS << L"); ";
    tempJS << tempVariant;
    tempJS << L" -= ";
  }
  if (m_pStep) {
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pStep->ToJavaScript(tempJS);
    tempJS << L")";
  } else {
    tempJS << L"1";
  }
  tempJS << L")\n";
  m_pList->ToJavaScript(tempJS);
  tempJS << L"}\n";
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMForExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (!m_wsImpliedReturnJS.IsEmpty()) {
    javascript << m_wsImpliedReturnJS;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << RUNTIMEFUNCTIONRETURNVALUE;
  tempJS << L" = 0;\n";
  tempJS << L"{\nvar ";
  CFX_WideString tempVariant;
  if (m_wsVariant.GetAt(0) == L'!') {
    tempVariant = EXCLAMATION_IN_IDENTIFIER + m_wsVariant.Mid(1);
    tempJS << tempVariant;
  } else {
    tempVariant = m_wsVariant;
    tempJS << m_wsVariant;
  }
  tempJS << L" = null;\n";
  tempJS << L"for (";
  tempJS << tempVariant;
  tempJS << L" = ";
  tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
  tempJS << L"(";
  m_pAssignment->ToJavaScript(tempJS);
  tempJS << L"); ";
  tempJS << tempVariant;
  if (m_iDirection == 1) {
    tempJS << L" <= ";
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pAccessor->ToJavaScript(tempJS);
    tempJS << L"); ";
    tempJS << tempVariant;
    tempJS << L" += ";
  } else {
    tempJS << L" >= ";
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pAccessor->ToJavaScript(tempJS);
    tempJS << L"); ";
    tempJS << tempVariant;
    tempJS << L" -= ";
  }
  if (m_pStep) {
    tempJS << XFA_FM_EXPTypeToString(GETFMVALUE);
    tempJS << L"(";
    m_pStep->ToJavaScript(tempJS);
    tempJS << L")";
  } else {
    tempJS << L"1";
  }
  tempJS << L")\n";
  m_pList->ToImpliedReturnJS(tempJS);
  tempJS << L"}\n";
  javascript << tempJS;
  m_wsImpliedReturnJS = tempJS.AsStringC();
}

CXFA_FMForeachExpression::CXFA_FMForeachExpression(
    uint32_t line,
    const CFX_WideStringC& wsIdentifier,
    std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pAccessors,
    std::unique_ptr<CXFA_FMExpression> pList)
    : CXFA_FMLoopExpression(line),
      m_wsIdentifier(wsIdentifier),
      m_pAccessors(std::move(pAccessors)),
      m_pList(std::move(pList)) {}

CXFA_FMForeachExpression::~CXFA_FMForeachExpression() {}

void CXFA_FMForeachExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  if (!m_wsJavascript.IsEmpty()) {
    javascript << m_wsJavascript;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << L"{\n";
  tempJS << L"var ";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    tempJS << tempIdentifier;
  } else {
    tempJS << m_wsIdentifier;
  }
  tempJS << L" = null;\n";
  tempJS << L"var ";
  tempJS << RUNTIMEBLOCKTEMPARRAY;
  tempJS << L" = ";
  tempJS << XFA_FM_EXPTypeToString(CONCATFMOBJECT);
  tempJS << L"(";

  for (const auto& expr : m_pAccessors) {
    expr->ToJavaScript(tempJS);
    if (expr != m_pAccessors.back())
      tempJS << L", ";
  }
  tempJS << L");\n";
  tempJS << L"var ";
  tempJS << RUNTIMEBLOCKTEMPARRAYINDEX;
  tempJS << (L" = 0;\n");
  tempJS << L"while(";
  tempJS << RUNTIMEBLOCKTEMPARRAYINDEX;
  tempJS << L" < ";
  tempJS << RUNTIMEBLOCKTEMPARRAY;
  tempJS << L".length)\n{\n";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    tempJS << tempIdentifier;
  } else {
    tempJS << m_wsIdentifier;
  }
  tempJS << L" = ";
  tempJS << RUNTIMEBLOCKTEMPARRAY;
  tempJS << L"[";
  tempJS << RUNTIMEBLOCKTEMPARRAYINDEX;
  tempJS << L"++];\n";
  m_pList->ToJavaScript(tempJS);
  tempJS << L"}\n";
  tempJS << L"}\n";
  javascript << tempJS;
  m_wsJavascript = tempJS.AsStringC();
}

void CXFA_FMForeachExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  if (!m_wsImpliedReturnJS.IsEmpty()) {
    javascript << m_wsImpliedReturnJS;
    return;
  }
  CFX_WideTextBuf tempJS;
  tempJS << RUNTIMEFUNCTIONRETURNVALUE;
  tempJS << L" = 0;\n";
  tempJS << L"{\n";
  tempJS << L"var ";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    tempJS << tempIdentifier;
  } else {
    tempJS << m_wsIdentifier;
  }
  tempJS << L" = null;\n";
  tempJS << L"var ";
  tempJS << RUNTIMEBLOCKTEMPARRAY;
  tempJS << L" = ";
  tempJS << XFA_FM_EXPTypeToString(CONCATFMOBJECT);
  tempJS << L"(";
  for (const auto& expr : m_pAccessors) {
    expr->ToJavaScript(tempJS);
    if (expr != m_pAccessors.back())
      tempJS << L", ";
  }
  tempJS << L");\n";
  tempJS << L"var ";
  tempJS << RUNTIMEBLOCKTEMPARRAYINDEX;
  tempJS << L" = 0;\n";
  tempJS << L"while(";
  tempJS << RUNTIMEBLOCKTEMPARRAYINDEX;
  tempJS << L" < ";
  tempJS << RUNTIMEBLOCKTEMPARRAY;
  tempJS << L".length)\n{\n";
  if (m_wsIdentifier.GetAt(0) == L'!') {
    CFX_WideString tempIdentifier =
        EXCLAMATION_IN_IDENTIFIER + m_wsIdentifier.Mid(1);
    tempJS << tempIdentifier;
  } else {
    tempJS << m_wsIdentifier;
  }
  tempJS << L" = ";
  tempJS << RUNTIMEBLOCKTEMPARRAY;
  tempJS << L"[";
  tempJS << RUNTIMEBLOCKTEMPARRAYINDEX;
  tempJS << L"++];\n";
  m_pList->ToImpliedReturnJS(tempJS);
  tempJS << L"}\n";
  tempJS << L"}\n";
  javascript << tempJS;
  m_wsImpliedReturnJS = tempJS.AsStringC();
}
