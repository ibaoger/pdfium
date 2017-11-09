// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_window.h"

// static
bool CXFA_Window::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_Window::CXFA_Window(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::NodeV,
                XFA_Element::Window,
                L"window") {}

CXFA_Window::~CXFA_Window() {}

WideStringView CXFA_Window::GetName() const {
  return L"window";
}

XFA_Element CXFA_Window::GetElementType() const {
  return XFA_Element::Window;
}
