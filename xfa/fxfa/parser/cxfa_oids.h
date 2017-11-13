// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_OIDS_H_
#define XFA_FXFA_PARSER_CXFA_OIDS_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Oids : public CXFA_Node {
 public:
  CXFA_Oids(CXFA_Document* doc, XFA_XDPPACKET packet);
  ~CXFA_Oids() override;
};

#endif  // XFA_FXFA_PARSER_CXFA_OIDS_H_
