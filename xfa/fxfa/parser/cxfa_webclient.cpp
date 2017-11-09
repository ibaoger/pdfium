// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_webclient.h"

// static
bool CXFA_WebClient::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_WebClient::CXFA_WebClient(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::WebClient,
                L"webClient") {}

CXFA_WebClient::~CXFA_WebClient() {}

WideStringView CXFA_WebClient::GetName() const {
  return L"webClient";
}

XFA_Element CXFA_WebClient::GetElementType() const {
  return XFA_Element::WebClient;
}
