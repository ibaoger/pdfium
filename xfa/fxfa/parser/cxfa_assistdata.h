// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_ASSISTDATA_H_
#define XFA_FXFA_PARSER_CXFA_ASSISTDATA_H_

#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_tooltipdata.h"

class CXFA_Node;

class CXFA_AssistData : public CXFA_DataData {
 public:
  explicit CXFA_AssistData(CXFA_Node* pNode);

  CXFA_ToolTipData GetToolTipData();
};

#endif  // XFA_FXFA_PARSER_CXFA_ASSISTDATA_H_
