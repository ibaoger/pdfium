// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_BOOKMARK_H_
#define CORE_FPDFDOC_CPDF_BOOKMARK_H_

#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_dest.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_string.h"

class CPDF_Dictionary;
class CPDF_Document;

class CPDF_Bookmark {
 public:
  CPDF_Bookmark();
  CPDF_Bookmark(const CPDF_Bookmark& that);
  explicit CPDF_Bookmark(CPDF_Dictionary* pDict);
  ~CPDF_Bookmark();

  CPDF_Dictionary* GetDict() const { return m_pDict.Get(); }
  uint32_t GetColorRef() const;
  uint32_t GetFontStyle() const;
  CFX_WideString GetTitle() const;
  CPDF_Dest GetDest(CPDF_Document* pDocument) const;
  CPDF_Action GetAction() const;

 private:
  CFX_UnownedPtr<CPDF_Dictionary> m_pDict;
};

#endif  // CORE_FPDFDOC_CPDF_BOOKMARK_H_
