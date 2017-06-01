// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pdfwindow/PWL_TimerHandler.h"

#include "fpdfsdk/pdfwindow/PWL_Timer.h"
#include "third_party/base/ptr_util.h"

IPWL_TimerHandler::IPWL_TimerHandler() {}

IPWL_TimerHandler::~IPWL_TimerHandler() {}

void IPWL_TimerHandler::BeginTimer(int32_t nElapse) {
  if (!m_pTimer)
    m_pTimer = pdfium::MakeUnique<CPWL_Timer>(this, GetSystemHandler());
  m_pTimer->SetPWLTimer(nElapse);
}

void IPWL_TimerHandler::EndTimer() {
  if (m_pTimer)
    m_pTimer->KillPWLTimer();
}

void IPWL_TimerHandler::TimerProc() {}
