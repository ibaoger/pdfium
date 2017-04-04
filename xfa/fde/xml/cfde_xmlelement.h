// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLELEMENT_H_
#define XFA_FDE_XML_CFDE_XMLELEMENT_H_

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmlnode.h"

class CFDE_XMLElement : public CFDE_XMLNode {
 public:
  explicit CFDE_XMLElement(const CFX_WideString& wsTag);
  ~CFDE_XMLElement() override;

  // CFDE_XMLNode
  FDE_XMLNODETYPE GetType() const override;
  CFDE_XMLNode* Clone(bool bRecursive) override;

  CFX_WideString GetTagName() const { return m_wsTag; }
  CFX_WideString GetLocalTagName() const;

  CFX_WideString GetNamespacePrefix() const;
  CFX_WideString GetNamespaceURI() const;

  int32_t CountAttributes() const;
  bool GetAttribute(int32_t index,
                    CFX_WideString* wsAttriName,
                    CFX_WideString* wsAttriValue) const;
  bool HasAttribute(const wchar_t* pwsAttriName) const;
  void RemoveAttribute(const wchar_t* pwsAttriName);

  CFX_WideString GetString(const wchar_t* pwsAttriName,
                           const CFX_WideString& pwsDefValue) const;
  void SetString(const CFX_WideString& wsAttriName,
                 const CFX_WideString& wsAttriValue);

  int32_t GetInteger(const wchar_t* pwsAttriName, int32_t iDefValue) const;
  void SetInteger(const wchar_t* pwsAttriName, int32_t iAttriValue);

  float GetFloat(const wchar_t* pwsAttriName, float fDefValue) const;
  void SetFloat(const wchar_t* pwsAttriName, float fAttriValue);

  CFX_WideString GetTextData() const;
  void SetTextData(const CFX_WideString& wsText);

 private:
  CFX_WideString m_wsTag;
  std::vector<CFX_WideString> m_Attributes;
};

#endif  // XFA_FDE_XML_CFDE_XMLELEMENT_H_
