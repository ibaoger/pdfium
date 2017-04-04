// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLCHARDATA_H_
#define XFA_FDE_XML_CFDE_XMLCHARDATA_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmldeclaration.h"

class CFDE_XMLCharData : public CFDE_XMLDeclaration {
 public:
  explicit CFDE_XMLCharData(const CFX_WideString& wsCData);
  ~CFDE_XMLCharData() override;

  FDE_XMLNODETYPE GetType() const override;
  CFDE_XMLNode* Clone(bool bRecursive) override;

  CFX_WideString GetCharData() const { return m_wsCharData; }
  void SetCharData(const CFX_WideString& wsCData) { m_wsCharData = wsCData; }

 private:
  CFX_WideString m_wsCharData;
};

#endif  // XFA_FDE_XML_CFDE_XMLCHARDATA_H_
