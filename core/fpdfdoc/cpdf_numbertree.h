// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_NUMBERTREE_H_
#define CORE_FPDFDOC_CPDF_NUMBERTREE_H_

#include "core/fxcrt/retain_ptr.h"

class CPDF_Dictionary;
class CPDF_Object;

class CPDF_NumberTree {
 public:
  explicit CPDF_NumberTree(RetainPtr<const CPDF_Dictionary> root);
  ~CPDF_NumberTree();

  RetainPtr<const CPDF_Object> LookupValue(int num) const;

 protected:
  RetainPtr<const CPDF_Dictionary> const root_;
};

#endif  // CORE_FPDFDOC_CPDF_NUMBERTREE_H_
