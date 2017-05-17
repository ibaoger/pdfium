// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/fm2js/cxfa_fmsimpleexpression.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"

namespace {

const wchar_t* const gs_lpStrExpFuncName[] = {
    L"pfm_rt.asgn_val_op", L"pfm_rt.log_or_op",  L"pfm_rt.log_and_op",
    L"pfm_rt.eq_op",       L"pfm_rt.neq_op",     L"pfm_rt.lt_op",
    L"pfm_rt.le_op",       L"pfm_rt.gt_op",      L"pfm_rt.ge_op",
    L"pfm_rt.plus_op",     L"pfm_rt.minus_op",   L"pfm_rt.mul_op",
    L"pfm_rt.div_op",      L"pfm_rt.pos_op",     L"pfm_rt.neg_op",
    L"pfm_rt.log_not_op",  L"pfm_rt.",           L"pfm_rt.dot_acc",
    L"pfm_rt.dotdot_acc",  L"pfm_rt.concat_obj", L"pfm_rt.is_obj",
    L"pfm_rt.is_ary",      L"pfm_rt.get_val",    L"pfm_rt.get_jsobj",
    L"pfm_rt.var_filter",
};

struct XFA_FMBuildInFunc {
  uint32_t m_uHash;
  const wchar_t* m_buildinfunc;
};

const XFA_FMBuildInFunc g_BuildInFuncs[] = {
    {0x0001f1f5, L"At"},           {0x00020b9c, L"FV"},
    {0x00021aef, L"If"},           {0x00023ee6, L"PV"},
    {0x04b5c9ee, L"Encode"},       {0x08e96685, L"DateFmt"},
    {0x09f99db6, L"Abs"},          {0x09f9e583, L"Apr"},
    {0x09fa043e, L"Avg"},          {0x0a9782a0, L"Get"},
    {0x0b1b09df, L"Len"},          {0x0b3543a6, L"Max"},
    {0x0b356ca4, L"Min"},          {0x0b358b60, L"Mod"},
    {0x0b4fded4, L"NPV"},          {0x0b846bf1, L"Pmt"},
    {0x0b8494f9, L"Put"},          {0x0bb8df5d, L"Ref"},
    {0x0bd37a99, L"Str"},          {0x0bd37fb5, L"Sum"},
    {0x1048469b, L"Cterm"},        {0x11e03660, L"Exists"},
    {0x126236e6, L"Post"},         {0x127c6661, L"PPmt"},
    {0x193ade3e, L"Right"},        {0x1ec8ab2c, L"Rate"},
    {0x20e476dc, L"IsoTime2Num"},  {0x23eb6816, L"TimeFmt"},
    {0x24fb17b0, L"LocalDateFmt"}, {0x28dee6e9, L"Format"},
    {0x2d0890b8, L"Term"},         {0x2d71b00f, L"Time"},
    {0x2f890fb1, L"Num2Time"},     {0x3767511d, L"Ceil"},
    {0x3ffd1941, L"LocalTimeFmt"}, {0x442f68c8, L"Round"},
    {0x46fd1128, L"Eval"},         {0x4d629440, L"Date2Num"},
    {0x4dcf25f8, L"Concat"},       {0x4e00255d, L"UnitValue"},
    {0x55a5cc29, L"Lower"},        {0x5e43e04c, L"WordNum"},
    {0x620ce6ba, L"Ipmt"},         {0x6f544d49, L"Count"},
    {0x7e241013, L"Within"},       {0x9b9a6e2b, L"IsoDate2Num"},
    {0xb2c941c2, L"UnitType"},     {0xb598a1f7, L"Uuid"},
    {0xbde9abde, L"Date"},         {0xc0010b80, L"Num2Date"},
    {0xc1f6144c, L"Upper"},        {0xc44028f7, L"Oneof"},
    {0xc62c1b2c, L"Space"},        {0xd0ff50f9, L"HasValue"},
    {0xd1537042, L"Floor"},        {0xd2ac9cf1, L"Time2Num"},
    {0xd907aee5, L"Num2GMTime"},   {0xdf24f7c4, L"Decode"},
    {0xe2664803, L"Substr"},       {0xe3e7b528, L"Stuff"},
    {0xe6792d4e, L"Rtrim"},        {0xe8c23f5b, L"Parse"},
    {0xea18d121, L"Choose"},       {0xebfef69c, L"Replace"},
    {0xf5ad782b, L"Left"},         {0xf7bb2248, L"Ltrim"},
};

struct XFA_FMSOMMethod {
  uint32_t m_uHash;
  const wchar_t* m_wsSomMethodName;
  uint32_t m_dParameters;
};
const XFA_FMSOMMethod gs_FMSomMethods[] = {
    {0x00000068, L"h", 0x01},
    {0x00000077, L"w", 0x01},
    {0x00000078, L"x", 0x01},
    {0x00000079, L"y", 0x01},
    {0x05eb5b0f, L"pageSpan", 0x01},
    {0x10f1b1bd, L"page", 0x01},
    {0x3bf1c2a5, L"absPageSpan", 0x01},
    {0x3c752495, L"verify", 0x0d},
    {0x44c352ad, L"formNodes", 0x01},
    {0x5775c2cc, L"absPageInBatch", 0x01},
    {0x5ee00996, L"setElement", 0x01},
    {0x7033bfd5, L"insert", 0x03},
    {0x8c5feb32, L"sheetInBatch", 0x01},
    {0x8f3a8379, L"sheet", 0x01},
    {0x92dada4f, L"saveFilteredXML", 0x01},
    {0x9cab7cae, L"remove", 0x01},
    {0xa68635f1, L"sign", 0x61},
    {0xaac241c8, L"isRecordGroup", 0x01},
    {0xd8ed1467, L"clear", 0x01},
    {0xda12e518, L"append", 0x01},
    {0xe74f0653, L"absPage", 0x01},
};

}  // namespace

CFX_WideStringC XFA_FM_EXPTypeToString(
    XFA_FM_SimpleExpressionType simpleExpType) {
  return gs_lpStrExpFuncName[simpleExpType];
}

CXFA_FMSimpleExpression::CXFA_FMSimpleExpression(uint32_t line, XFA_FM_TOKEN op)
    : m_line(line), m_op(op) {}

bool CXFA_FMSimpleExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  return true;
}

bool CXFA_FMSimpleExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  return true;
}

XFA_FM_TOKEN CXFA_FMSimpleExpression::GetOperatorToken() const {
  return m_op;
}

CXFA_FMNullExpression::CXFA_FMNullExpression(uint32_t line)
    : CXFA_FMSimpleExpression(line, TOKnull) {}

bool CXFA_FMNullExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"null";
  return true;
}

CXFA_FMNumberExpression::CXFA_FMNumberExpression(uint32_t line,
                                                 CFX_WideStringC wsNumber)
    : CXFA_FMSimpleExpression(line, TOKnumber), m_wsNumber(wsNumber) {}

CXFA_FMNumberExpression::~CXFA_FMNumberExpression() {}

bool CXFA_FMNumberExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << m_wsNumber;
  return true;
}

CXFA_FMStringExpression::CXFA_FMStringExpression(uint32_t line,
                                                 CFX_WideStringC wsString)
    : CXFA_FMSimpleExpression(line, TOKstring), m_wsString(wsString) {}

CXFA_FMStringExpression::~CXFA_FMStringExpression() {}

bool CXFA_FMStringExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CFX_WideString tempStr(m_wsString);
  if (tempStr.GetLength() <= 2) {
    javascript << tempStr;
    return true;
  }
  javascript.AppendChar(L'\"');
  for (int32_t i = 1; i < tempStr.GetLength() - 1; i++) {
    wchar_t oneChar = tempStr[i];
    switch (oneChar) {
      case L'\"':
        i++;
        javascript << L"\\\"";
        break;
      case 0x0d:
        break;
      case 0x0a:
        javascript << L"\\n";
        break;
      default:
        javascript.AppendChar(oneChar);
        break;
    }
  }
  javascript.AppendChar(L'\"');
  return true;
}

CXFA_FMIdentifierExpression::CXFA_FMIdentifierExpression(
    uint32_t line,
    CFX_WideStringC wsIdentifier)
    : CXFA_FMSimpleExpression(line, TOKidentifier),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMIdentifierExpression::~CXFA_FMIdentifierExpression() {}

bool CXFA_FMIdentifierExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CFX_WideString tempStr(m_wsIdentifier);
  if (tempStr == L"$") {
    tempStr = L"this";
  } else if (tempStr == L"!") {
    tempStr = L"xfa.datasets";
  } else if (tempStr == L"$data") {
    tempStr = L"xfa.datasets.data";
  } else if (tempStr == L"$event") {
    tempStr = L"xfa.event";
  } else if (tempStr == L"$form") {
    tempStr = L"xfa.form";
  } else if (tempStr == L"$host") {
    tempStr = L"xfa.host";
  } else if (tempStr == L"$layout") {
    tempStr = L"xfa.layout";
  } else if (tempStr == L"$template") {
    tempStr = L"xfa.template";
  } else if (tempStr[0] == L'!') {
    tempStr = EXCLAMATION_IN_IDENTIFIER + tempStr.Mid(1);
  }
  javascript << tempStr;
  return true;
}

CXFA_FMUnaryExpression::CXFA_FMUnaryExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMSimpleExpression(line, op), m_pExp(std::move(pExp)) {}

CXFA_FMUnaryExpression::~CXFA_FMUnaryExpression() {}

bool CXFA_FMUnaryExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  return true;
}

CXFA_FMBinExpression::CXFA_FMBinExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMSimpleExpression(line, op),
      m_pExp1(std::move(pExp1)),
      m_pExp2(std::move(pExp2)) {}

CXFA_FMBinExpression::~CXFA_FMBinExpression() {}

bool CXFA_FMBinExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  return true;
}

CXFA_FMAssignExpression::CXFA_FMAssignExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMAssignExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"if (";
  javascript << gs_lpStrExpFuncName[ISFMOBJECT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1))
    return false;
  javascript << tempExp1;
  javascript << L"))\n{\n";
  javascript << gs_lpStrExpFuncName[ASSIGN];
  javascript << L"(";
  javascript << tempExp1;
  javascript << L", ";
  if (CFXA_IsTooBig(javascript))
    return false;

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2))
    return false;
  javascript << tempExp2;
  javascript << L");\n}\n";
  if (m_pExp1->GetOperatorToken() == TOKidentifier &&
      tempExp1.AsStringC() != L"this") {
    javascript << L"else\n{\n";
    javascript << tempExp1;
    javascript << L" = ";
    javascript << gs_lpStrExpFuncName[ASSIGN];
    javascript << L"(";
    javascript << tempExp1;
    javascript << L", ";
    javascript << tempExp2;
    javascript << L");\n}\n";
  }
  return !CFXA_IsTooBig(javascript);
}

bool CXFA_FMAssignExpression::ToImpliedReturnJS(CFX_WideTextBuf& javascript) {
  javascript << L"if (";
  javascript << gs_lpStrExpFuncName[ISFMOBJECT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1))
    return false;
  javascript << tempExp1;
  javascript << L"))\n{\n";
  javascript << RUNTIMEFUNCTIONRETURNVALUE;
  javascript << L" = ";
  javascript << gs_lpStrExpFuncName[ASSIGN];
  javascript << L"(";
  javascript << tempExp1;
  javascript << L", ";
  if (CFXA_IsTooBig(javascript))
    return false;

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2))
    return false;
  javascript << tempExp2;
  javascript << L");\n}\n";
  if (m_pExp1->GetOperatorToken() == TOKidentifier &&
      tempExp1.AsStringC() != L"this") {
    javascript << L"else\n{\n";
    javascript << RUNTIMEFUNCTIONRETURNVALUE;
    javascript << L" = ";
    javascript << tempExp1;
    javascript << L" = ";
    javascript << gs_lpStrExpFuncName[ASSIGN];
    javascript << L"(";
    javascript << tempExp1;
    javascript << L", ";
    javascript << tempExp2;
    javascript << L");\n}\n";
  }
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMLogicalOrExpression::CXFA_FMLogicalOrExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMLogicalOrExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[LOGICALOR];
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMLogicalAndExpression::CXFA_FMLogicalAndExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMLogicalAndExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[LOGICALAND];
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMEqualityExpression::CXFA_FMEqualityExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMEqualityExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  switch (m_op) {
    case TOKeq:
    case TOKkseq:
      javascript << gs_lpStrExpFuncName[EQUALITY];
      break;
    case TOKne:
    case TOKksne:
      javascript << gs_lpStrExpFuncName[NOTEQUALITY];
      break;
    default:
      ASSERT(false);
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMRelationalExpression::CXFA_FMRelationalExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMRelationalExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  switch (m_op) {
    case TOKlt:
    case TOKkslt:
      javascript << gs_lpStrExpFuncName[LESS];
      break;
    case TOKgt:
    case TOKksgt:
      javascript << gs_lpStrExpFuncName[GREATER];
      break;
    case TOKle:
    case TOKksle:
      javascript << gs_lpStrExpFuncName[LESSEQUAL];
      break;
    case TOKge:
    case TOKksge:
      javascript << gs_lpStrExpFuncName[GREATEREQUAL];
      break;
    default:
      ASSERT(false);
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMAdditiveExpression::CXFA_FMAdditiveExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMAdditiveExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  switch (m_op) {
    case TOKplus:
      javascript << gs_lpStrExpFuncName[PLUS];
      break;
    case TOKminus:
      javascript << gs_lpStrExpFuncName[MINUS];
      break;
    default:
      ASSERT(false);
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMMultiplicativeExpression::CXFA_FMMultiplicativeExpression(
    uint32_t line,
    XFA_FM_TOKEN op,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp2)
    : CXFA_FMBinExpression(line, op, std::move(pExp1), std::move(pExp2)) {}

bool CXFA_FMMultiplicativeExpression::ToJavaScript(
    CFX_WideTextBuf& javascript) {
  switch (m_op) {
    case TOKmul:
      javascript << gs_lpStrExpFuncName[MULTIPLE];
      break;
    case TOKdiv:
      javascript << gs_lpStrExpFuncName[DIVIDE];
      break;
    default:
      ASSERT(false);
      break;
  }
  javascript << L"(";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMPosExpression::CXFA_FMPosExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKplus, std::move(pExp)) {}

bool CXFA_FMPosExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[POSITIVE];
  javascript << L"(";
  if (!m_pExp->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return true;
}

CXFA_FMNegExpression::CXFA_FMNegExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKminus, std::move(pExp)) {}

bool CXFA_FMNegExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[NEGATIVE];
  javascript << L"(";
  if (!m_pExp->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return true;
}

CXFA_FMNotExpression::CXFA_FMNotExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp)
    : CXFA_FMUnaryExpression(line, TOKksnot, std::move(pExp)) {}

bool CXFA_FMNotExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[NOT];
  javascript << L"(";
  if (!m_pExp->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return true;
}

CXFA_FMCallExpression::CXFA_FMCallExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pExp,
    std::vector<std::unique_ptr<CXFA_FMSimpleExpression>>&& pArguments,
    bool bIsSomMethod)
    : CXFA_FMUnaryExpression(line, TOKcall, std::move(pExp)),
      m_bIsSomMethod(bIsSomMethod),
      m_Arguments(std::move(pArguments)) {}

CXFA_FMCallExpression::~CXFA_FMCallExpression() {}

bool CXFA_FMCallExpression::IsBuildInFunc(CFX_WideTextBuf* funcName) {
  uint32_t uHash = FX_HashCode_GetW(funcName->AsStringC(), true);
  const XFA_FMBuildInFunc* pEnd = g_BuildInFuncs + FX_ArraySize(g_BuildInFuncs);
  const XFA_FMBuildInFunc* pFunc =
      std::lower_bound(g_BuildInFuncs, pEnd, uHash,
                       [](const XFA_FMBuildInFunc& func, uint32_t hash) {
                         return func.m_uHash < hash;
                       });
  if (pFunc < pEnd && uHash == pFunc->m_uHash) {
    funcName->Clear();
    *funcName << pFunc->m_buildinfunc;
    return true;
  }
  return false;
}

uint32_t CXFA_FMCallExpression::IsMethodWithObjParam(
    const CFX_WideStringC& methodName) {
  uint32_t uHash = FX_HashCode_GetW(methodName, false);
  XFA_FMSOMMethod somMethodWithObjPara;
  uint32_t parameters = 0x00;
  int32_t iStart = 0,
          iEnd = (sizeof(gs_FMSomMethods) / sizeof(gs_FMSomMethods[0])) - 1;
  int32_t iMid = (iStart + iEnd) / 2;
  do {
    iMid = (iStart + iEnd) / 2;
    somMethodWithObjPara = gs_FMSomMethods[iMid];
    if (uHash == somMethodWithObjPara.m_uHash) {
      parameters = somMethodWithObjPara.m_dParameters;
      break;
    } else if (uHash < somMethodWithObjPara.m_uHash) {
      iEnd = iMid - 1;
    } else {
      iStart = iMid + 1;
    }
  } while (iStart <= iEnd);
  return parameters;
}

bool CXFA_FMCallExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  CFX_WideTextBuf funcName;
  if (!m_pExp->ToJavaScript(funcName))
    return false;
  if (m_bIsSomMethod) {
    javascript << funcName;
    javascript << L"(";
    uint32_t methodPara = IsMethodWithObjParam(funcName.AsStringC());
    if (methodPara > 0) {
      for (size_t i = 0; i < m_Arguments.size(); ++i) {
        // Currently none of our expressions use objects for a parameter over
        // the 6th. Make sure we don't overflow the shift when doing this
        // check. If we ever need more the 32 object params we can revisit.
        if (i < 32 && (methodPara & (0x01 << i)) > 0) {
          javascript << gs_lpStrExpFuncName[GETFMJSOBJ];
        } else {
          javascript << gs_lpStrExpFuncName[GETFMVALUE];
        }
        javascript << L"(";
        const auto& expr = m_Arguments[i];
        if (!expr->ToJavaScript(javascript))
          return false;
        javascript << L")";
        if (i + 1 < m_Arguments.size()) {
          javascript << L", ";
        }
        if (CFXA_IsTooBig(javascript))
          return false;
      }
    } else {
      for (const auto& expr : m_Arguments) {
        javascript << gs_lpStrExpFuncName[GETFMVALUE];
        javascript << L"(";
        if (!expr->ToJavaScript(javascript))
          return false;
        javascript << L")";
        if (expr != m_Arguments.back())
          javascript << L", ";
        if (CFXA_IsTooBig(javascript))
          return false;
      }
    }
    javascript << L")";
  } else {
    bool isEvalFunc = false;
    bool isExistsFunc = false;
    if (IsBuildInFunc(&funcName)) {
      if (funcName.AsStringC() == L"Eval") {
        isEvalFunc = true;
        javascript << L"eval.call(this, ";
        javascript << gs_lpStrExpFuncName[CALL];
        javascript << L"Translate";
      } else if (funcName.AsStringC() == L"Exists") {
        isExistsFunc = true;
        javascript << gs_lpStrExpFuncName[CALL];
        javascript << funcName;
      } else {
        javascript << gs_lpStrExpFuncName[CALL];
        javascript << funcName;
      }
    } else {
      javascript << funcName;
    }
    javascript << L"(";
    if (isExistsFunc) {
      javascript << L"\n(\nfunction ()\n{\ntry\n{\n";
      if (!m_Arguments.empty()) {
        const auto& expr = m_Arguments[0];
        javascript << L"return ";
        if (!expr->ToJavaScript(javascript))
          return false;
        javascript << L";\n}\n";
      } else {
        javascript << L"return 0;\n}\n";
      }
      javascript << L"catch(accessExceptions)\n";
      javascript << L"{\nreturn 0;\n}\n}\n).call(this)\n";
    } else {
      for (const auto& expr : m_Arguments) {
        if (!expr->ToJavaScript(javascript))
          return false;
        if (expr != m_Arguments.back())
          javascript << L", ";
        if (CFXA_IsTooBig(javascript))
          return false;
      }
    }
    javascript << L")";
    if (isEvalFunc) {
      javascript << L")";
    }
  }
  return true;
}

CXFA_FMDotAccessorExpression::CXFA_FMDotAccessorExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    XFA_FM_TOKEN op,
    CFX_WideStringC wsIdentifier,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp)
    : CXFA_FMBinExpression(line,
                           op,
                           std::move(pAccessor),
                           std::move(pIndexExp)),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMDotAccessorExpression::~CXFA_FMDotAccessorExpression() {}

bool CXFA_FMDotAccessorExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[DOT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (m_pExp1) {
    if (!m_pExp1->ToJavaScript(tempExp1))
      return false;
    javascript << tempExp1;
  } else {
    javascript << L"null";
  }
  javascript << L", ";
  javascript << L"\"";
  if (CFXA_IsTooBig(javascript))
    return false;

  if (m_pExp1 && m_pExp1->GetOperatorToken() == TOKidentifier)
    javascript << tempExp1;
  javascript << L"\", ";
  if (m_op == TOKdotscream) {
    javascript << L"\"#";
    javascript << m_wsIdentifier;
    javascript << L"\", ";
  } else if (m_op == TOKdotstar) {
    javascript << L"\"*\", ";
  } else if (m_op == TOKcall) {
    javascript << L"\"\", ";
  } else {
    javascript << L"\"";
    javascript << m_wsIdentifier;
    javascript << L"\", ";
  }
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMIndexExpression::CXFA_FMIndexExpression(
    uint32_t line,
    XFA_FM_AccessorIndex accessorIndex,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp,
    bool bIsStarIndex)
    : CXFA_FMUnaryExpression(line, TOKlbracket, std::move(pIndexExp)),
      m_accessorIndex(accessorIndex),
      m_bIsStarIndex(bIsStarIndex) {}

bool CXFA_FMIndexExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  switch (m_accessorIndex) {
    case ACCESSOR_NO_INDEX:
      javascript << L"0";
      break;
    case ACCESSOR_NO_RELATIVEINDEX:
      javascript << L"1";
      break;
    case ACCESSOR_POSITIVE_INDEX:
      javascript << L"2";
      break;
    case ACCESSOR_NEGATIVE_INDEX:
      javascript << L"3";
      break;
    default:
      javascript << L"0";
  }
  if (!m_bIsStarIndex) {
    javascript << L", ";
    if (m_pExp) {
      if (!m_pExp->ToJavaScript(javascript))
        return false;
    } else {
      javascript << L"0";
    }
  }
  return true;
}

CXFA_FMDotDotAccessorExpression::CXFA_FMDotDotAccessorExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessor,
    XFA_FM_TOKEN op,
    CFX_WideStringC wsIdentifier,
    std::unique_ptr<CXFA_FMSimpleExpression> pIndexExp)
    : CXFA_FMBinExpression(line,
                           op,
                           std::move(pAccessor),
                           std::move(pIndexExp)),
      m_wsIdentifier(wsIdentifier) {}

CXFA_FMDotDotAccessorExpression::~CXFA_FMDotDotAccessorExpression() {}

bool CXFA_FMDotDotAccessorExpression::ToJavaScript(
    CFX_WideTextBuf& javascript) {
  javascript << gs_lpStrExpFuncName[DOTDOT];
  javascript << L"(";
  CFX_WideTextBuf tempExp1;
  if (!m_pExp1->ToJavaScript(tempExp1))
    return false;
  javascript << tempExp1;
  javascript << L", ";
  javascript << L"\"";
  if (CFXA_IsTooBig(javascript))
    return false;

  if (m_pExp1->GetOperatorToken() == TOKidentifier)
    javascript << tempExp1;
  javascript << L"\", ";
  javascript << L"\"";
  javascript << m_wsIdentifier;
  javascript << L"\", ";
  if (!m_pExp2->ToJavaScript(javascript))
    return false;
  javascript << L")";
  return !CFXA_IsTooBig(javascript);
}

CXFA_FMMethodCallExpression::CXFA_FMMethodCallExpression(
    uint32_t line,
    std::unique_ptr<CXFA_FMSimpleExpression> pAccessorExp1,
    std::unique_ptr<CXFA_FMSimpleExpression> pCallExp)
    : CXFA_FMBinExpression(line,
                           TOKdot,
                           std::move(pAccessorExp1),
                           std::move(pCallExp)) {}

bool CXFA_FMMethodCallExpression::ToJavaScript(CFX_WideTextBuf& javascript) {
  javascript << L"(\nfunction ()\n{\n";
  javascript << L"var method_return_value = null;\n";
  javascript << L"var accessor_object = ";
  if (!m_pExp1->ToJavaScript(javascript))
    return false;
  javascript << L";\n";
  javascript << L"if (";
  javascript << gs_lpStrExpFuncName[ISFMARRAY];
  javascript << L"(accessor_object))\n{\n";
  javascript << L"for(var index = accessor_object.length - 1; index > 1; "
                L"index--)\n{\n";
  javascript << L"method_return_value = accessor_object[index].";
  if (CFXA_IsTooBig(javascript))
    return false;

  CFX_WideTextBuf tempExp2;
  if (!m_pExp2->ToJavaScript(tempExp2))
    return false;
  javascript << tempExp2;
  javascript << L";\n}\n}\n";
  javascript << L"else\n{\nmethod_return_value = accessor_object.";
  javascript << tempExp2;
  javascript << L";\n}\n";
  javascript << L"return method_return_value;\n";
  javascript << L"}\n).call(this)";
  return !CFXA_IsTooBig(javascript);
}

bool CFXA_IsTooBig(const CFX_WideTextBuf& javascript) {
  return javascript.GetSize() > 256 * 1024 * 1024;
}
