// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_content.h"

#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"

CJX_Content::CJX_Content(CXFA_Object* obj) : CJX_Object(obj) {}

CJX_Content::~CJX_Content() {}

bool CJX_Content::SetContent(const WideString& wsContent,
                             const WideString& wsXMLValue,
                             bool bNotify,
                             bool bScriptModify,
                             bool bSyncData) {
  WideString wsContentType;
  if (ToNode(GetXFAObject())->GetElementType() == XFA_Element::ExData) {
    pdfium::Optional<WideString> ret =
        TryAttribute(XFA_Attribute::ContentType, false);
    if (ret)
      wsContentType = *ret;
    if (wsContentType == L"text/html") {
      wsContentType = L"";
      SetAttribute(XFA_Attribute::ContentType, wsContentType.AsStringView(),
                   false);
    }
  }

  CXFA_Node* pContentRawDataNode =
      ToNode(GetXFAObject())->GetNodeItem(XFA_NODEITEM_FirstChild);
  if (!pContentRawDataNode) {
    pContentRawDataNode =
        ToNode(GetXFAObject())
            ->CreateSamePacketNode((wsContentType == L"text/xml")
                                       ? XFA_Element::Sharpxml
                                       : XFA_Element::Sharptext);
    ToNode(GetXFAObject())->InsertChild(pContentRawDataNode, nullptr);
  }
  return pContentRawDataNode->JSObject()->SetContent(
      wsContent, wsXMLValue, bNotify, bScriptModify, bSyncData);
}
