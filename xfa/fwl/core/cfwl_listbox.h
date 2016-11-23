// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_LISTBOX_H_
#define XFA_FWL_CORE_CFWL_LISTBOX_H_

#include <memory>
#include <vector>

#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/fwl_error.h"
#include "xfa/fwl/core/ifwl_listbox.h"
#include "xfa/fwl/core/ifwl_widget.h"

class CFWL_ListBox : public CFWL_Widget, public IFWL_ListBox::DataProvider {
 public:
  explicit CFWL_ListBox(const CFWL_App* pApp);
  ~CFWL_ListBox() override;

  void Initialize();

  CFWL_ListItem* GetItem(const IFWL_Widget* pWidget, int32_t nIndex) const;
  void GetItemText(IFWL_Widget* pWidget,
                   CFWL_ListItem* pItem,
                   CFX_WideString& wsText);

  CFWL_ListItem* AddString(const CFX_WideStringC& wsAdd, bool bSelect = false);
  bool DeleteString(CFWL_ListItem* pItem);
  void DeleteAll();
  int32_t CountSelItems();
  void SetSelItem(CFWL_ListItem* pItem, bool bSelect = true);
  CFWL_ListItem* GetSelItem(int32_t nIndexSel);
  int32_t GetSelIndex(int32_t nIndex);

  uint32_t GetItemStates(CFWL_ListItem* pItem);
};

#endif  // XFA_FWL_CORE_CFWL_LISTBOX_H_
