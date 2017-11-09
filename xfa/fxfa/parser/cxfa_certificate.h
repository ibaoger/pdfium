// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_CERTIFICATE_H_
#define XFA_FXFA_PARSER_CXFA_CERTIFICATE_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Certificate : public CXFA_Node {
 public:
  static bool IsValidPacket(XFA_XDPPACKET packet);

  CXFA_Certificate(CXFA_Document* doc, XFA_XDPPACKET packet);
  ~CXFA_Certificate() override;

  // CXFA_Node
  WideStringView GetName() const override;
  XFA_Element GetElementType() const override;
};

#endif  // XFA_FXFA_PARSER_CXFA_CERTIFICATE_H_
