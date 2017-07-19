// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_DOCJSACTIONS_H_
#define CORE_FPDFDOC_CPDF_DOCJSACTIONS_H_

#include "core/fpdfdoc/cpdf_action.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_string.h"

class CPDF_Document;

class CPDF_DocJSActions {
 public:
  explicit CPDF_DocJSActions(CPDF_Document* pDoc);
  ~CPDF_DocJSActions();

  int CountJSActions() const;
  CPDF_Action GetJSActionAndName(int index, CFX_WideString* csName) const;
  CPDF_Action GetJSAction(const CFX_WideString& csName) const;
  int FindJSAction(const CFX_WideString& csName) const;
  CPDF_Document* GetDocument() const { return m_pDocument.Get(); }

 private:
  CFX_UnownedPtr<CPDF_Document> const m_pDocument;
};

#endif  // CORE_FPDFDOC_CPDF_DOCJSACTIONS_H_
