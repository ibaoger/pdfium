// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_TOOLTIPDATA_H_
#define XFA_FXFA_PARSER_CXFA_TOOLTIPDATA_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_ToolTipData : public CXFA_DataData {
 public:
  explicit CXFA_ToolTipData(CXFA_Node* pNode);

  bool GetTip(WideString& wsTip);
};

#endif  // XFA_FXFA_PARSER_CXFA_TOOLTIPDATA_H_
