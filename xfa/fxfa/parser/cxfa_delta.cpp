// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_delta.h"

namespace {

const wchar_t* kName = L"delta";

}  // namespace

CXFA_Delta::CXFA_Delta(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_XDPPACKET_Form,
                XFA_ObjectType::Object,
                XFA_Element::Delta,
                nullptr,
                nullptr,
                kName) {}

CXFA_Delta::~CXFA_Delta() {}
