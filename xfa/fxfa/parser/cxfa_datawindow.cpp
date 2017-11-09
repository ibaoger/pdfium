// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_datawindow.h"

// static
bool CXFA_DataWindow::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Datasets & packet);
}

CXFA_DataWindow::CXFA_DataWindow(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Object,
                XFA_Element::DataWindow,
                L"dataWindow") {}

CXFA_DataWindow::~CXFA_DataWindow() {}

WideStringView CXFA_DataWindow::GetName() const {
  return L"dataWindow";
}

XFA_Element CXFA_DataWindow::GetElementType() const {
  return XFA_Element::DataWindow;
}
