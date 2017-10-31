// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_RESOLVEPROCESSOR_H_
#define XFA_FXFA_PARSER_CXFA_RESOLVEPROCESSOR_H_

#include <memory>
#include <vector>

#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

class CXFA_NodeHelper;
class CFXJSE_Engine;

class CXFA_ResolveNodesData {
 public:
  explicit CXFA_ResolveNodesData(CFXJSE_Engine* pSC = nullptr);
  ~CXFA_ResolveNodesData();

  CFXJSE_Engine* m_pSC;
  CXFA_Object* m_CurObject;
  WideString m_wsName;
  XFA_HashCode m_uHashName;
  WideString m_wsCondition;
  int32_t m_nLevel;
  std::vector<CXFA_Object*> m_Objects;  // Not owned.
  uint32_t m_dwStyles;
  const XFA_SCRIPTATTRIBUTEINFO* m_pScriptAttribute;
  XFA_RESOVENODE_RSTYPE m_dwFlag;
};

class CXFA_ResolveProcessor {
 public:
  CXFA_ResolveProcessor();
  ~CXFA_ResolveProcessor();

  int32_t Resolve(CXFA_ResolveNodesData& rnd);
  int32_t GetFilter(const WideStringView& wsExpression,
                    int32_t nStart,
                    CXFA_ResolveNodesData& rnd);
  int32_t SetResultCreateNode(XFA_RESOLVENODE_RS& resolveNodeRS,
                              WideString& wsLastCondition);
  void SetIndexDataBind(WideString& wsNextCondition,
                        int32_t& iIndex,
                        int32_t iCount);
  void SetCurStart(int32_t start) { m_iCurStart = start; }

  CXFA_NodeHelper* GetNodeHelper() const { return m_pNodeHelper.get(); }

 private:
  int32_t ResolveForAttributeRs(CXFA_Object* curNode,
                                CXFA_ResolveNodesData& rnd,
                                const WideStringView& strAttr);
  int32_t ResolveAnyChild(CXFA_ResolveNodesData& rnd);
  int32_t ResolveDollar(CXFA_ResolveNodesData& rnd);
  int32_t ResolveExcalmatory(CXFA_ResolveNodesData& rnd);
  int32_t ResolveNumberSign(CXFA_ResolveNodesData& rnd);
  int32_t ResolveAsterisk(CXFA_ResolveNodesData& rnd);
  int32_t ResolveNormal(CXFA_ResolveNodesData& rnd);
  int32_t ResolvePopStack(std::vector<int32_t>* stack);
  void SetStylesForChild(uint32_t dwParentStyles, CXFA_ResolveNodesData& rnd);

  void ConditionArray(int32_t iCurIndex,
                      WideString wsCondition,
                      int32_t iFoundCount,
                      CXFA_ResolveNodesData& rnd);
  void DoPredicateFilter(int32_t iCurIndex,
                         WideString wsCondition,
                         int32_t iFoundCount,
                         CXFA_ResolveNodesData& rnd);
  void FilterCondition(CXFA_ResolveNodesData& rnd, WideString wsCondition);

  int32_t m_iCurStart;
  std::unique_ptr<CXFA_NodeHelper> m_pNodeHelper;
};

#endif  // XFA_FXFA_PARSER_CXFA_RESOLVEPROCESSOR_H_
