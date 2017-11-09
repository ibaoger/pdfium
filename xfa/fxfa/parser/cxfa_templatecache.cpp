// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_templatecache.h"

// static
bool CXFA_TemplateCache::IsValidPacket(XFA_XDPPACKET packet) {
  return !!(XFA_XDPPACKET_Config & packet);
}

CXFA_TemplateCache::CXFA_TemplateCache(CXFA_Document* doc, XFA_XDPPACKET packet)
    : CXFA_Node(doc,
                packet,
                XFA_ObjectType::Node,
                XFA_Element::TemplateCache,
                L"templateCache") {}

CXFA_TemplateCache::~CXFA_TemplateCache() {}

WideStringView CXFA_TemplateCache::GetName() const {
  return L"templateCache";
}

XFA_Element CXFA_TemplateCache::GetElementType() const {
  return XFA_Element::TemplateCache;
}
