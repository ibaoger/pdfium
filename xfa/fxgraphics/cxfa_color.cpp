// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cxfa_color.h"

CXFA_Color::CXFA_Color() : m_type(FX_COLOR_None) {}

CXFA_Color::CXFA_Color(const FX_ARGB argb) : m_type(FX_COLOR_Solid) {
  m_info.argb = argb;
  m_info.pattern = nullptr;
}

CXFA_Color::CXFA_Color(CXFA_Pattern* pattern, const FX_ARGB argb)
    : m_type(FX_COLOR_Pattern) {
  m_info.argb = argb;
  m_info.pattern = pattern;
}

CXFA_Color::CXFA_Color(CXFA_Shading* shading) : m_type(FX_COLOR_Shading) {
  m_shading = shading;
}

CXFA_Color::~CXFA_Color() {}

CXFA_Color& CXFA_Color::operator=(const CXFA_Color& that) {
  if (this != &that) {
    m_type = that.m_type;
    switch (m_type) {
      case FX_COLOR_Solid:
        m_info.argb = that.m_info.argb;
        m_info.pattern = nullptr;
        break;
      case FX_COLOR_Pattern:
        m_info.argb = that.m_info.argb;
        m_info.pattern = that.m_info.pattern;
        break;
      case FX_COLOR_Shading:
        m_shading = that.m_shading;
      default:
        break;
    }
  }
  return *this;
}
