// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFDOC_CPDF_FORMFIELD_H_
#define CORE_FPDFDOC_CPDF_FORMFIELD_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfdoc/cpdf_aaction.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "third_party/base/stl_util.h"

#define FIELDTYPE_UNKNOWN 0
#define FIELDTYPE_PUSHBUTTON 1
#define FIELDTYPE_CHECKBOX 2
#define FIELDTYPE_RADIOBUTTON 3
#define FIELDTYPE_COMBOBOX 4
#define FIELDTYPE_LISTBOX 5
#define FIELDTYPE_TEXTFIELD 6
#define FIELDTYPE_SIGNATURE 7

#define FORMFLAG_READONLY 0x01
#define FORMFLAG_REQUIRED 0x02
#define FORMFLAG_NOEXPORT 0x04

class CPDF_Dictionary;
class CPDF_Font;
class CPDF_FormControl;
class CPDF_InterForm;
class CPDF_String;

CPDF_Object* FPDF_GetFieldAttr(const CPDF_Dictionary* pFieldDict,
                               const char* name,
                               int nLevel = 0);
WideString FPDF_GetFullName(CPDF_Dictionary* pFieldDict);

class CPDF_FormField {
 public:
  enum Type {
    Unknown,
    PushButton,
    RadioButton,
    CheckBox,
    Text,
    RichText,
    File,
    ListBox,
    ComboBox,
    Sign
  };

  CPDF_FormField(CPDF_InterForm* pForm, CPDF_Dictionary* pDict);
  ~CPDF_FormField();

  WideString GetFullName() const;

  Type GetType() const { return m_Type; }
  uint32_t GetFlags() const { return m_Flags; }

  CPDF_Dictionary* GetFieldDict() const { return m_pDict.Get(); }
  void SetFieldDict(CPDF_Dictionary* pDict) { m_pDict = pDict; }

  bool ResetField(bool bNotify = false);

  int CountControls() const {
    return pdfium::CollectionSize<int>(m_ControlList);
  }

  CPDF_FormControl* GetControl(int index) const {
    return m_ControlList[index].Get();
  }

  int GetControlIndex(const CPDF_FormControl* pControl) const;
  int GetFieldType() const;

  CPDF_AAction GetAdditionalAction() const;
  WideString GetAlternateName() const;
  WideString GetMappingName() const;

  uint32_t GetFieldFlags() const;
  ByteString GetDefaultStyle() const;
  WideString GetRichTextString() const;

  WideString GetValue() const;
  WideString GetDefaultValue() const;
  bool SetValue(const WideString& value, bool bNotify = false);

  int GetMaxLen() const;
  int CountSelectedItems() const;
  int GetSelectedIndex(int index) const;

  bool ClearSelection(bool bNotify = false);
  bool IsItemSelected(int index) const;
  bool SetItemSelection(int index, bool bSelected, bool bNotify = false);

  bool IsItemDefaultSelected(int index) const;

  int GetDefaultSelectedItem() const;
  int CountOptions() const;

  WideString GetOptionLabel(int index) const;
  WideString GetOptionValue(int index) const;

  int FindOption(WideString csOptLabel) const;
  int FindOptionValue(const WideString& csOptValue) const;

  bool CheckControl(int iControlIndex, bool bChecked, bool bNotify = false);

  int GetTopVisibleIndex() const;
  int CountSelectedOptions() const;

  int GetSelectedOptionIndex(int index) const;
  bool IsOptionSelected(int iOptIndex) const;

  bool SelectOption(int iOptIndex, bool bSelected, bool bNotify = false);

  bool ClearSelectedOptions(bool bNotify = false);

#ifdef PDF_ENABLE_XFA
  bool ClearOptions(bool bNotify = false);

  int InsertOption(WideString csOptLabel, int index = -1, bool bNotify = false);
#endif  // PDF_ENABLE_XFA

  float GetFontSize() const { return m_FontSize; }
  CPDF_Font* GetFont() const { return m_pFont.Get(); }

  const CPDF_Dictionary* GetDict() const { return m_pDict.Get(); }
  const CPDF_InterForm* GetForm() const { return m_pForm.Get(); }

  WideString GetCheckValue(bool bDefault) const;

  void AddFormControl(CPDF_FormControl* pFormControl) {
    m_ControlList.emplace_back(pFormControl);
  }

  void SetOpt(std::unique_ptr<CPDF_Object> pOpt) {
    m_pDict->SetFor("Opt", std::move(pOpt));
  }

 private:
  WideString GetValue(bool bDefault) const;
  bool SetValue(const WideString& value, bool bDefault, bool bNotify);

  void SyncFieldFlags();
  int FindListSel(CPDF_String* str);
  WideString GetOptionText(int index, int sub_index) const;

  void LoadDA();
  bool SetCheckValue(const WideString& value, bool bDefault, bool bNotify);

  bool NotifyBeforeSelectionChange(const WideString& value);
  void NotifyAfterSelectionChange();

  bool NotifyBeforeValueChange(const WideString& value);
  void NotifyAfterValueChange();

  bool NotifyListOrComboBoxBeforeChange(const WideString& value);
  void NotifyListOrComboBoxAfterChange();

  CPDF_FormField::Type m_Type;
  uint32_t m_Flags;
  UnownedPtr<CPDF_InterForm> const m_pForm;
  UnownedPtr<CPDF_Dictionary> m_pDict;
  // Owned by InterForm parent.
  std::vector<UnownedPtr<CPDF_FormControl>> m_ControlList;
  float m_FontSize;
  UnownedPtr<CPDF_Font> m_pFont;
};

#endif  // CORE_FPDFDOC_CPDF_FORMFIELD_H_
