// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_APP_H_
#define XFA_FWL_CFWL_APP_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "xfa/fwl/cfwl_notedriver.h"
#include "xfa/fwl/cfwl_widgetmgr.h"

class CXFA_FFApp;
class CXFA_FWLAdapterWidgetMgr;
class CFWL_Widget;

enum FWL_KeyFlag {
  FWL_KEYFLAG_Ctrl = 1 << 0,
  FWL_KEYFLAG_Alt = 1 << 1,
  FWL_KEYFLAG_Shift = 1 << 2,
  FWL_KEYFLAG_Command = 1 << 3,
  FWL_KEYFLAG_LButton = 1 << 4,
  FWL_KEYFLAG_RButton = 1 << 5,
  FWL_KEYFLAG_MButton = 1 << 6
};

class CFWL_App {
 public:
  explicit CFWL_App(CXFA_FFApp* pAdapter,
                    CXFA_FWLAdapterWidgetMgr* pAdapterMgr);
  ~CFWL_App();

  CXFA_FFApp* GetAdapterNative() const { return m_pAdapterNative.Get(); }
  CFWL_WidgetMgr* GetWidgetMgr() { return &m_WidgetMgr; }
  CFWL_NoteDriver* GetNoteDriver() { return &m_NoteDriver; }

 private:
  UnownedPtr<CXFA_FFApp> const m_pAdapterNative;
  CFWL_WidgetMgr m_WidgetMgr;
  CFWL_NoteDriver m_NoteDriver;
};

#endif  // XFA_FWL_CFWL_APP_H_
