// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_XFA_UTILS_H_
#define XFA_FXFA_PARSER_XFA_UTILS_H_

#include <stack>

#include "xfa/fde/xml/fde_xml.h"
#include "xfa/fgas/crt/fgas_stream.h"
#include "xfa/fgas/crt/fgas_utils.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFDE_XMLElement;
class CFDE_XMLNode;
class CXFA_LocaleValue;
class CXFA_Node;
class CXFA_WidgetData;

bool XFA_FDEExtension_ResolveNamespaceQualifier(
    CFDE_XMLElement* pNode,
    const CFX_WideStringC& wsQualifier,
    CFX_WideString& wsNamespaceURI);

template <class NodeType, class TraverseStrategy>
class CXFA_NodeIteratorTemplate {
 public:
  explicit CXFA_NodeIteratorTemplate(NodeType* pRootNode = nullptr)
      : m_pRoot(pRootNode) {
    if (pRootNode)
      m_NodeStack.push(pRootNode);
  }
  bool Init(NodeType* pRootNode) {
    if (!pRootNode)
      return false;
    m_pRoot = pRootNode;
    Clear();
    m_NodeStack.push(pRootNode);
    return true;
  }
  void Clear() { m_NodeStack = std::stack<NodeType*>(); }
  void Reset() {
    Clear();
    if (m_pRoot)
      m_NodeStack.push(m_pRoot);
  }
  bool SetCurrent(NodeType* pCurNode) {
    Clear();
    if (pCurNode) {
      std::stack<NodeType*> revStack;
      NodeType* pNode;
      for (pNode = pCurNode; pNode && pNode != m_pRoot;
           pNode = TraverseStrategy::GetParent(pNode)) {
        revStack.push(pNode);
      }
      if (!pNode)
        return false;

      revStack.push(m_pRoot);
      while (!revStack.empty()) {
        m_NodeStack.push(revStack.top());
        revStack.pop();
      }
    }
    return true;
  }
  NodeType* GetCurrent() const {
    return !m_NodeStack.empty() ? m_NodeStack.top() : nullptr;
  }
  NodeType* GetRoot() const { return m_pRoot; }
  NodeType* MoveToPrev() {
    size_t nStackLength = m_NodeStack.size();
    if (nStackLength == 1)
      return nullptr;
    if (nStackLength > 1) {
      NodeType* pCurItem = m_NodeStack.top();
      m_NodeStack.pop();
      NodeType* pParentItem = m_NodeStack.top();
      NodeType* pParentFirstChildItem =
          TraverseStrategy::GetFirstChild(pParentItem);
      if (pCurItem == pParentFirstChildItem)
        return pParentItem;
      NodeType* pPrevItem = pParentFirstChildItem;
      NodeType* pPrevItemNext = nullptr;
      for (; pPrevItem; pPrevItem = pPrevItemNext) {
        pPrevItemNext = TraverseStrategy::GetNextSibling(pPrevItem);
        if (!pPrevItemNext || pPrevItemNext == pCurItem)
          break;
      }
      m_NodeStack.push(pPrevItem);
    } else {
      Reset();
    }
    if (!m_NodeStack.empty()) {
      NodeType* pChildItem = m_NodeStack.top();
      while ((pChildItem = TraverseStrategy::GetFirstChild(pChildItem))) {
        while (NodeType* pNextItem =
                   TraverseStrategy::GetNextSibling(pChildItem)) {
          pChildItem = pNextItem;
        }
        m_NodeStack.push(pChildItem);
      }
      return m_NodeStack.top();
    }
    return nullptr;
  }
  NodeType* MoveToNext() {
    NodeType* pNode = nullptr;
    NodeType* pCurrent = GetCurrent();
    while (!m_NodeStack.empty()) {
      while ((pNode = m_NodeStack.top()) != nullptr) {
        if (pCurrent != pNode)
          return pNode;
        NodeType* pChild = TraverseStrategy::GetFirstChild(pNode);
        if (!pChild)
          break;
        m_NodeStack.push(pChild);
      }
      while ((pNode = m_NodeStack.top()) != nullptr) {
        NodeType* pNext = TraverseStrategy::GetNextSibling(pNode);
        m_NodeStack.pop();
        if (m_NodeStack.empty())
          break;
        if (pNext) {
          m_NodeStack.push(pNext);
          break;
        }
      }
    }
    return nullptr;
  }
  NodeType* SkipChildrenAndMoveToNext() {
    NodeType* pNode = nullptr;
    while ((pNode = m_NodeStack.top()) != nullptr) {
      NodeType* pNext = TraverseStrategy::GetNextSibling(pNode);
      m_NodeStack.pop();
      if (m_NodeStack.empty())
        break;
      if (pNext) {
        m_NodeStack.push(pNext);
        break;
      }
    }
    return GetCurrent();
  }

 protected:
  NodeType* m_pRoot;
  std::stack<NodeType*> m_NodeStack;
};

CXFA_LocaleValue XFA_GetLocaleValue(CXFA_WidgetData* pWidgetData);
FX_DOUBLE XFA_ByteStringToDouble(const CFX_ByteStringC& szStringVal);
int32_t XFA_MapRotation(int32_t nRotation);

bool XFA_RecognizeRichText(CFDE_XMLElement* pRichTextXMLNode);
void XFA_GetPlainTextFromRichText(CFDE_XMLNode* pXMLNode,
                                  CFX_WideString& wsPlainText);
bool XFA_FieldIsMultiListBox(CXFA_Node* pFieldNode);

void XFA_DataExporter_DealWithDataGroupNode(CXFA_Node* pDataNode);
void XFA_DataExporter_RegenerateFormFile(
    CXFA_Node* pNode,
    const CFX_RetainPtr<IFGAS_Stream>& pStream,
    const FX_CHAR* pChecksum = nullptr,
    bool bSaveXML = false);

const XFA_NOTSUREATTRIBUTE* XFA_GetNotsureAttribute(
    XFA_Element eElement,
    XFA_ATTRIBUTE eAttribute,
    XFA_ATTRIBUTETYPE eType = XFA_ATTRIBUTETYPE_NOTSURE);

const XFA_SCRIPTATTRIBUTEINFO* XFA_GetScriptAttributeByName(
    XFA_Element eElement,
    const CFX_WideStringC& wsAttributeName);

const XFA_PROPERTY* XFA_GetPropertyOfElement(XFA_Element eElement,
                                             XFA_Element eProperty,
                                             uint32_t dwPacket);
const XFA_PROPERTY* XFA_GetElementProperties(XFA_Element eElement,
                                             int32_t& iCount);
const uint8_t* XFA_GetElementAttributes(XFA_Element eElement, int32_t& iCount);
const XFA_ELEMENTINFO* XFA_GetElementByID(XFA_Element eName);
XFA_Element XFA_GetElementTypeForName(const CFX_WideStringC& wsName);
CXFA_Measurement XFA_GetAttributeDefaultValue_Measure(XFA_Element eElement,
                                                      XFA_ATTRIBUTE eAttribute,
                                                      uint32_t dwPacket);
bool XFA_GetAttributeDefaultValue(void*& pValue,
                                  XFA_Element eElement,
                                  XFA_ATTRIBUTE eAttribute,
                                  XFA_ATTRIBUTETYPE eType,
                                  uint32_t dwPacket);
const XFA_ATTRIBUTEINFO* XFA_GetAttributeByName(const CFX_WideStringC& wsName);
const XFA_ATTRIBUTEINFO* XFA_GetAttributeByID(XFA_ATTRIBUTE eName);
const XFA_ATTRIBUTEENUMINFO* XFA_GetAttributeEnumByName(
    const CFX_WideStringC& wsName);
const XFA_PACKETINFO* XFA_GetPacketByIndex(XFA_PACKET ePacket);
const XFA_PACKETINFO* XFA_GetPacketByID(uint32_t dwPacket);

#endif  // XFA_FXFA_PARSER_XFA_UTILS_H_
