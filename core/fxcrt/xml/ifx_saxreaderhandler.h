// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_IFX_SAXREADERHANDLER_H_
#define CORE_FXCRT_XML_IFX_SAXREADERHANDLER_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_saxcontext.h"
#include "core/fxcrt/xml/cfx_saxreader.h"

class IFX_SAXReaderHandler {
 public:
  IFX_SAXReaderHandler() {}
  virtual ~IFX_SAXReaderHandler() {}

  virtual CFX_SAXContext* OnTagEnter(const CFX_ByteStringC& bsTagName,
                                     CFX_SAXItem::Type eType,
                                     uint32_t dwStartPos) = 0;
  virtual void OnTagAttribute(CFX_SAXContext* pTag,
                              const CFX_ByteStringC& bsAttri,
                              const CFX_ByteStringC& bsValue) = 0;
  virtual void OnTagBreak(CFX_SAXContext* pTag) = 0;
  virtual void OnTagData(CFX_SAXContext* pTag,
                         CFX_SAXItem::Type eType,
                         const CFX_ByteStringC& bsData,
                         uint32_t dwStartPos) = 0;
  virtual void OnTagClose(CFX_SAXContext* pTag, uint32_t dwEndPos) = 0;
  virtual void OnTagEnd(CFX_SAXContext* pTag,
                        const CFX_ByteStringC& bsTagName,
                        uint32_t dwEndPos) = 0;

  virtual void OnTargetData(CFX_SAXContext* pTag,
                            CFX_SAXItem::Type eType,
                            const CFX_ByteStringC& bsData,
                            uint32_t dwStartPos) = 0;
};

#endif  // CORE_FXCRT_XML_IFX_SAXREADERHANDLER_H_
