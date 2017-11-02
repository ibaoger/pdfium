// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_exdata.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_ExData::CXFA_ExData(CXFA_Node* pNode) : CXFA_Data(pNode) {}

bool CXFA_ExData::SetContentType(const WideString& wsContentType) {
  return m_pNode->JSNode()->SetCData(XFA_ATTRIBUTE_ContentType, wsContentType,
                                     false, false);
}
