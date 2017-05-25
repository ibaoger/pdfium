// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_LINK_H_
#define CORE_FPDFDOC_CPDF_LINK_H_

#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_dest.h"
#include "core/fxcrt/fx_coordinates.h"

class CPDF_Dictionary;

class CPDF_Link {
 public:
  CPDF_Link();
  explicit CPDF_Link(CPDF_Dictionary* pDict);
  CPDF_Link(const CPDF_Link& that);
  ~CPDF_Link();

  CPDF_Dictionary* GetDict() const { return m_pDict.Get(); }
  CFX_FloatRect GetRect();
  CPDF_Dest GetDest(CPDF_Document* pDoc);
  CPDF_Action GetAction();

 private:
  CFX_UnownedPtr<CPDF_Dictionary> m_pDict;
};

#endif  // CORE_FPDFDOC_CPDF_LINK_H_
