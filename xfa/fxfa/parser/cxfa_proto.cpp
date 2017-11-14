// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_proto.h"

namespace {

const wchar_t* kName = L"proto";

}  // namespace

CXFA_Proto::CXFA_Proto(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                (XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
                XFA_ObjectType::Node,
                XFA_Element::Proto,
                nullptr,
                nullptr,
                kName) {}

CXFA_Proto::~CXFA_Proto() {}
