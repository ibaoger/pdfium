// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_EVENTDATA_H_
#define XFA_FXFA_PARSER_CXFA_EVENTDATA_H_

#include <stdint.h>

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;
class CXFA_Script;
class CXFA_Submit;

class CXFA_EventData : public CXFA_DataData {
 public:
  explicit CXFA_EventData(CXFA_Node* pNode);

  XFA_AttributeEnum GetActivity();
  XFA_Element GetEventType() const;
  CXFA_Script* GetScript() const;
  CXFA_Submit* GetSubmit() const;
  WideString GetRef() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_EVENTDATA_H_
