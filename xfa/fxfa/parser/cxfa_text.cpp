// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_text.h"

#include "xfa/fxfa/parser/cxfa_node.h"

CXFA_Text::CXFA_Text(CXFA_Node* pNode) : CXFA_Data(pNode) {}

void CXFA_Text::GetContent(WideString& wsText) {
  m_pNode->JSNode()->TryContent(wsText, false, true);
}
