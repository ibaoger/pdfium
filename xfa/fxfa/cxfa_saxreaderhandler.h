// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_SAXREADERHANDLER_H_
#define XFA_FXFA_CXFA_SAXREADERHANDLER_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_saxcontext.h"
#include "core/fxcrt/xml/cfx_saxreader.h"
#include "core/fxcrt/xml/ifx_saxreaderhandler.h"

class CXFA_ChecksumContext;

class CXFA_SAXReaderHandler : public IFX_SAXReaderHandler {
 public:
  explicit CXFA_SAXReaderHandler(CXFA_ChecksumContext* pContext);
  ~CXFA_SAXReaderHandler() override;

  // IFX_SAXReaderHandler
  CFX_SAXContext* OnTagEnter(const CFX_ByteStringC& bsTagName,
                             CFX_SAXItem::Type eType,
                             uint32_t dwStartPos) override;
  void OnTagAttribute(CFX_SAXContext* pTag,
                      const CFX_ByteStringC& bsAttri,
                      const CFX_ByteStringC& bsValue) override;
  void OnTagBreak(CFX_SAXContext* pTag) override;
  void OnTagData(CFX_SAXContext* pTag,
                 CFX_SAXItem::Type eType,
                 const CFX_ByteStringC& bsData,
                 uint32_t dwStartPos) override;
  void OnTagClose(CFX_SAXContext* pTag, uint32_t dwEndPos) override;
  void OnTagEnd(CFX_SAXContext* pTag,
                const CFX_ByteStringC& bsTagName,
                uint32_t dwEndPos) override;
  void OnTargetData(CFX_SAXContext* pTag,
                    CFX_SAXItem::Type eType,
                    const CFX_ByteStringC& bsData,
                    uint32_t dwStartPos) override;

 private:
  void UpdateChecksum(bool bCheckSpace);

  CXFA_ChecksumContext* m_pContext;
  CFX_SAXContext m_SAXContext;
};

#endif  // XFA_FXFA_CXFA_SAXREADERHANDLER_H_
