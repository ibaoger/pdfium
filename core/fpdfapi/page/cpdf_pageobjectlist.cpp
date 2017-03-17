// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/page/cpdf_pageobjectlist.h"

#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/pageint.h"
#include "third_party/base/stl_util.h"

CPDF_PageObject* CPDF_PageObjectList::GetPageObjectByIndex(int index) {
  return pdfium::IndexInBounds(*this, index) ? (*this)[index].get() : nullptr;
}

// Releases ownership of the object at index and updates the deque so that it
// is no longer accessible.
bool CPDF_PageObjectList::RemovePageObjectByIndex(int index) {
  if (!pdfium::IndexInBounds(*this, index))
    return false;
  (*this)[index].release();
  this->erase((*this).begin() + index);
  return true;
}
