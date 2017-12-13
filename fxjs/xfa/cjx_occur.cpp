// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_occur.h"

#include <algorithm>

#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_occurdata.h"

CJX_Occur::CJX_Occur(CXFA_Occur* node) : CJX_Node(node) {}

CJX_Occur::~CJX_Occur() = default;

void CJX_Occur::max(CFXJSE_Value* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute) {
  CXFA_OccurData occurData(GetXFANode());
  if (!bSetting) {
    pValue->SetInteger(occurData.GetMax());
    return;
  }
  occurData.SetMax(pValue->ToInteger());
}

void CJX_Occur::min(CFXJSE_Value* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute) {
  CXFA_OccurData occurData(GetXFANode());
  if (!bSetting) {
    pValue->SetInteger(occurData.GetMin());
    return;
  }
  occurData.SetMin(pValue->ToInteger());
}

void CJX_Occur::use(CFXJSE_Value* pValue,
                    bool bSetting,
                    XFA_Attribute eAttribute) {
  Script_Attribute_String(pValue, bSetting, eAttribute);
}

void CJX_Occur::initial(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  Script_Attribute_String(pValue, bSetting, eAttribute);
}

void CJX_Occur::usehref(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  Script_Attribute_String(pValue, bSetting, eAttribute);
}
