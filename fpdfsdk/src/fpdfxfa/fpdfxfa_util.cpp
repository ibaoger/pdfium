// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/include/fsdk_define.h"
#include "fpdfsdk/include/fsdk_mgr.h"
#include "fpdfsdk/include/fpdfxfa/fpdfxfa_util.h"

std::vector<CFWL_TimerInfo*> CXFA_FWLAdapterTimerMgr::s_TimerArray;

FWL_ERR CXFA_FWLAdapterTimerMgr::Start(IFWL_Timer* pTimer,
                                       FX_DWORD dwElapse,
                                       FWL_HTIMER& hTimer,
                                       FX_BOOL bImmediately) {
  if (!m_pEnv)
    return FWL_ERR_Indefinite;

  uint32_t uIDEvent = m_pEnv->FFI_SetTimer(dwElapse, TimerProc);
  s_TimerArray.push_back(new CFWL_TimerInfo(uIDEvent, pTimer));
  hTimer = reinterpret_cast<FWL_HTIMER>(s_TimerArray.back());
  return FWL_ERR_Succeeded;
}

FWL_ERR CXFA_FWLAdapterTimerMgr::Stop(FWL_HTIMER hTimer) {
  if (!hTimer || !m_pEnv)
    return FWL_ERR_Indefinite;

  CFWL_TimerInfo* pInfo = reinterpret_cast<CFWL_TimerInfo*>(hTimer);
  m_pEnv->FFI_KillTimer(pInfo->uIDEvent);
  auto it = std::find(s_TimerArray.begin(), s_TimerArray.end(), pInfo);
  if (it != s_TimerArray.end()) {
    s_TimerArray.erase(it);
    delete pInfo;
  }
  return FWL_ERR_Succeeded;
}

void CXFA_FWLAdapterTimerMgr::TimerProc(int32_t idEvent) {
  for (CFWL_TimerInfo* pInfo : s_TimerArray) {
    if (pInfo->uIDEvent == idEvent) {
      pInfo->pTimer->Run(reinterpret_cast<FWL_HTIMER>(pInfo));
      break;
    }
  }
}
