// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_IMAGEDATA_H_
#define XFA_FXFA_PARSER_CXFA_IMAGEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_data.h"

class CXFA_Node;

class CXFA_ImageData : public CXFA_Data {
 public:
  CXFA_ImageData(CXFA_Node* pNode, bool bDefValue);

  int32_t GetAspect();
  bool GetContentType(WideString& wsContentType);
  bool GetHref(WideString& wsHref);
  XFA_ATTRIBUTEENUM GetTransferEncoding();
  bool GetContent(WideString& wsText);
  bool SetContentType(const WideString& wsContentType);
  bool SetHref(const WideString& wsHref);
  bool SetTransferEncoding(XFA_ATTRIBUTEENUM iTransferEncoding);

 private:
  bool m_bDefValue;
};

#endif  // XFA_FXFA_PARSER_CXFA_IMAGEDATA_H_
