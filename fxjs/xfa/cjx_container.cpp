// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_container.h"

#include <vector>

#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_field.h"
#include "xfa/fxfa/parser/xfa_utils.h"

const CJX_MethodSpec CJX_Container::MethodSpecs[] = {
    {"getDelta", getDelta_static},
    {"getDeltas", getDeltas_static}};

CJX_Container::CJX_Container(CXFA_Node* node) : CJX_Node(node) {
  DefineMethods(MethodSpecs, FX_ArraySize(MethodSpecs));
}

CJX_Container::~CJX_Container() {}

CJS_Return CJX_Container::getDelta(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  return CJS_Return(true);
}

CJS_Return CJX_Container::getDeltas(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(GetDocument());
  return CJS_Return(runtime->NewXFAObject(
      pFormNodes,
      GetDocument()->GetScriptContext()->GetJseNormalClass()->GetTemplate()));
}

bool CJX_Container::SetContent(const WideString& wsContent,
                               const WideString& wsXMLValue,
                               bool bNotify,
                               bool bScriptModify,
                               bool bSyncData) {
  if (XFA_FieldIsMultiListBox(ToNode(GetXFAObject()))) {
    CXFA_Node* pValue = GetProperty(0, XFA_Element::Value, true);
    if (!pValue)
      return false;

    CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
    ASSERT(pChildValue);
    pChildValue->JSObject()->SetCData(XFA_Attribute::ContentType, L"text/xml",
                                      false, false);
    pChildValue->JSObject()->SetContent(wsContent, wsContent, bNotify,
                                        bScriptModify, false);
    CXFA_Node* pBind = ToNode(GetXFAObject())->GetBindData();
    if (!bSyncData || !pBind)
      return false;

    std::vector<WideString> wsSaveTextArray;
    size_t iSize = 0;
    if (!wsContent.IsEmpty()) {
      size_t iStart = 0;
      size_t iLength = wsContent.GetLength();
      auto iEnd = wsContent.Find(L'\n', iStart);
      iEnd = !iEnd.has_value() ? iLength : iEnd;
      while (iEnd.value() >= iStart) {
        wsSaveTextArray.push_back(wsContent.Mid(iStart, iEnd.value() - iStart));
        iStart = iEnd.value() + 1;
        if (iStart >= iLength)
          break;

        iEnd = wsContent.Find(L'\n', iStart);
        if (!iEnd.has_value()) {
          wsSaveTextArray.push_back(wsContent.Mid(iStart, iLength - iStart));
        }
      }
      iSize = wsSaveTextArray.size();
    }
    if (iSize == 0) {
      while (CXFA_Node* pChildNode =
                 pBind->GetNodeItem(XFA_NODEITEM_FirstChild)) {
        pBind->RemoveChild(pChildNode, true);
      }
    } else {
      std::vector<CXFA_Node*> valueNodes =
          pBind->GetNodeList(XFA_NODEFILTER_Children, XFA_Element::DataValue);
      size_t iDatas = valueNodes.size();
      if (iDatas < iSize) {
        size_t iAddNodes = iSize - iDatas;
        CXFA_Node* pValueNodes = nullptr;
        while (iAddNodes-- > 0) {
          pValueNodes = pBind->CreateSamePacketNode(XFA_Element::DataValue);
          pValueNodes->JSObject()->SetCData(XFA_Attribute::Name, L"value",
                                            false, false);
          pValueNodes->CreateXMLMappingNode();
          pBind->InsertChild(pValueNodes, nullptr);
        }
        pValueNodes = nullptr;
      } else if (iDatas > iSize) {
        size_t iDelNodes = iDatas - iSize;
        while (iDelNodes-- > 0) {
          pBind->RemoveChild(pBind->GetNodeItem(XFA_NODEITEM_FirstChild), true);
        }
      }
      int32_t i = 0;
      for (CXFA_Node* pValueNode = pBind->GetNodeItem(XFA_NODEITEM_FirstChild);
           pValueNode;
           pValueNode = pValueNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
        pValueNode->JSObject()->SetAttributeValue(
            wsSaveTextArray[i], wsSaveTextArray[i], false, false);
        i++;
      }
    }
    for (const auto& pArrayNode : *(pBind->GetBindItems())) {
      if (pArrayNode.Get() != ToNode(GetXFAObject())) {
        pArrayNode->JSObject()->SetContent(wsContent, wsContent, bNotify,
                                           bScriptModify, false);
      }
    }
    return false;
  }

  CXFA_Node* pNode = nullptr;
  if (ToNode(GetXFAObject())->GetElementType() == XFA_Element::ExclGroup) {
    pNode = ToNode(GetXFAObject());
  } else {
    CXFA_Node* pValue = GetProperty(0, XFA_Element::Value, true);
    if (!pValue)
      return false;

    CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
    ASSERT(pChildValue);
    pChildValue->JSObject()->SetContent(wsContent, wsContent, bNotify,
                                        bScriptModify, false);
  }

  CXFA_Node* pBindNode = ToNode(GetXFAObject())->GetBindData();
  if (pBindNode && bSyncData) {
    pBindNode->JSObject()->SetContent(wsContent, wsXMLValue, bNotify,
                                      bScriptModify, false);
    for (const auto& pArrayNode : *(pBindNode->GetBindItems())) {
      if (pArrayNode.Get() != ToNode(GetXFAObject())) {
        pArrayNode->JSObject()->SetContent(wsContent, wsContent, bNotify, true,
                                           false);
      }
    }
  }

  if (!pNode)
    return false;

  SetAttributeValue(wsContent, wsXMLValue, bNotify, bScriptModify);
  return true;
}
