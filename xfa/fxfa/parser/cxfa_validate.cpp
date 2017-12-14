// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_validate.h"

#include "fxjs/xfa/cjx_validate.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_message.h"
#include "xfa/fxfa/parser/cxfa_picture.h"
#include "xfa/fxfa/parser/cxfa_script.h"

namespace {

const CXFA_Node::PropertyData kPropertyData[] = {{XFA_Element::Message, 1, 0},
                                                 {XFA_Element::Picture, 1, 0},
                                                 {XFA_Element::Script, 1, 0},
                                                 {XFA_Element::Extras, 1, 0},
                                                 {XFA_Element::Unknown, 0, 0}};
const CXFA_Node::AttributeData kAttributeData[] = {
    {XFA_Attribute::Id, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Use, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::ScriptTest, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Error},
    {XFA_Attribute::NullTest, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Disabled},
    {XFA_Attribute::Usehref, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::Desc, XFA_AttributeType::CData, nullptr},
    {XFA_Attribute::FormatTest, XFA_AttributeType::Enum,
     (void*)XFA_AttributeEnum::Warning},
    {XFA_Attribute::Lock, XFA_AttributeType::Integer, (void*)0},
    {XFA_Attribute::Unknown, XFA_AttributeType::Integer, nullptr}};

constexpr wchar_t kName[] = L"validate";
constexpr wchar_t kFormatTest[] = L"formatTest";
constexpr wchar_t kNullTest[] = L"nullTest";
constexpr wchar_t kScriptTest[] = L"scriptTest";

}  // namespace

CXFA_Validate::CXFA_Validate(CXFA_Document* doc, XFA_PacketType packet)
    : CXFA_Node(
          doc,
          packet,
          (XFA_XDPPACKET_Config | XFA_XDPPACKET_Template | XFA_XDPPACKET_Form),
          XFA_ObjectType::ContentNode,
          XFA_Element::Validate,
          kPropertyData,
          kAttributeData,
          kName,
          pdfium::MakeUnique<CJX_Validate>(this)) {}

CXFA_Validate::~CXFA_Validate() {}

XFA_AttributeEnum CXFA_Validate::GetFormatTest() {
  return JSObject()->GetEnum(XFA_Attribute::FormatTest);
}

void CXFA_Validate::SetNullTest(const WideString& wsValue) {
  pdfium::Optional<XFA_AttributeEnum> item =
      CXFA_Node::NameToAttributeEnum(wsValue.AsStringView());
  JSObject()->SetEnum(XFA_Attribute::NullTest,
                      item ? *item : XFA_AttributeEnum::Disabled, false);
}

XFA_AttributeEnum CXFA_Validate::GetNullTest() {
  return JSObject()->GetEnum(XFA_Attribute::NullTest);
}

XFA_AttributeEnum CXFA_Validate::GetScriptTest() {
  return JSObject()->GetEnum(XFA_Attribute::ScriptTest);
}

WideString CXFA_Validate::GetMessageText(const WideString& wsMessageType) {
  CXFA_Message* pNode =
      JSObject()->GetProperty<CXFA_Message>(0, XFA_Element::Message, false);
  if (!pNode)
    return L"";

  for (CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetElementType() != XFA_Element::Text)
      continue;

    WideString wsName = pItemNode->JSObject()->GetCData(XFA_Attribute::Name);
    if (wsName.IsEmpty() || wsName == wsMessageType)
      return pItemNode->JSObject()->GetContent(false);
  }
  return L"";
}

void CXFA_Validate::SetFormatMessageText(const WideString& wsMessage) {
  SetMessageText(kFormatTest, wsMessage);
}

WideString CXFA_Validate::GetFormatMessageText() {
  return GetMessageText(kFormatTest);
}

void CXFA_Validate::SetNullMessageText(const WideString& wsMessage) {
  SetMessageText(kNullTest, wsMessage);
}

WideString CXFA_Validate::GetNullMessageText() {
  return GetMessageText(kNullTest);
}

WideString CXFA_Validate::GetScriptMessageText() {
  return GetMessageText(kScriptTest);
}

void CXFA_Validate::SetScriptMessageText(const WideString& wsMessage) {
  SetMessageText(kScriptTest, wsMessage);
}

void CXFA_Validate::SetMessageText(const WideString& wsMessageType,
                                   const WideString& wsMessage) {
  CXFA_Message* pNode =
      JSObject()->GetProperty<CXFA_Message>(0, XFA_Element::Message, true);
  if (!pNode)
    return;

  for (CXFA_Node* pItemNode = pNode->GetNodeItem(XFA_NODEITEM_FirstChild);
       pItemNode;
       pItemNode = pItemNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pItemNode->GetElementType() != XFA_Element::Text)
      continue;

    WideString wsName = pItemNode->JSObject()->GetCData(XFA_Attribute::Name);
    if (wsName.IsEmpty() || wsName == wsMessageType) {
      pItemNode->JSObject()->SetContent(wsMessage, wsMessage, false, false,
                                        true);
      return;
    }
  }

  CXFA_Node* pTextNode = pNode->CreateSamePacketNode(XFA_Element::Text);
  pNode->InsertChild(pTextNode, nullptr);
  pTextNode->JSObject()->SetCData(XFA_Attribute::Name, wsMessageType, false,
                                  false);
  pTextNode->JSObject()->SetContent(wsMessage, wsMessage, false, false, true);
}

WideString CXFA_Validate::GetPicture() {
  CXFA_Picture* pNode = GetChild<CXFA_Picture>(0, XFA_Element::Picture, false);
  return pNode ? pNode->JSObject()->GetContent(false) : L"";
}

CXFA_Script* CXFA_Validate::GetScript() {
  return GetChild<CXFA_Script>(0, XFA_Element::Script, false);
}
