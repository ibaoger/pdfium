// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_JAVASCRIPT_FIELD_H_
#define FPDFSDK_JAVASCRIPT_FIELD_H_

#include <string>
#include <vector>

#include "core/fxcrt/observable.h"
#include "core/fxge/cfx_color.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/javascript/JS_Define.h"

class CPDFSDK_Widget;
class Document;

enum FIELD_PROP {
  FP_BORDERSTYLE,
  FP_CURRENTVALUEINDICES,
  FP_DISPLAY,
  FP_HIDDEN,
  FP_LINEWIDTH,
  FP_RECT,
  FP_VALUE
};

struct CJS_DelayData {
  CJS_DelayData(FIELD_PROP prop, int idx, const WideString& name);
  ~CJS_DelayData();

  FIELD_PROP eProp;
  int nControlIndex;
  WideString sFieldName;
  int32_t num;
  bool b;
  ByteString string;
  WideString widestring;
  CFX_FloatRect rect;
  CFX_Color color;
  std::vector<uint32_t> wordarray;
  std::vector<WideString> widestringarray;
};

class Field : public CJS_EmbedObj {
 public:
  static void DoDelay(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                      CJS_DelayData* pData);

  explicit Field(CJS_Object* pJSObject);
  ~Field() override;

  bool get_alignment(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_alignment(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_border_style(CJS_Runtime* pRuntime,
                        CJS_PropValue* vp,
                        WideString* sError);
  bool set_border_style(CJS_Runtime* pRuntime,
                        const CJS_PropValue& vp,
                        WideString* sError);

  bool get_button_align_x(CJS_Runtime* pRuntime,
                          CJS_PropValue* vp,
                          WideString* sError);
  bool set_button_align_x(CJS_Runtime* pRuntime,
                          const CJS_PropValue& vp,
                          WideString* sError);

  bool get_button_align_y(CJS_Runtime* pRuntime,
                          CJS_PropValue* vp,
                          WideString* sError);
  bool set_button_align_y(CJS_Runtime* pRuntime,
                          const CJS_PropValue& vp,
                          WideString* sError);

  bool get_button_fit_bounds(CJS_Runtime* pRuntime,
                             CJS_PropValue* vp,
                             WideString* sError);
  bool set_button_fit_bounds(CJS_Runtime* pRuntime,
                             const CJS_PropValue& vp,
                             WideString* sError);

  bool get_button_position(CJS_Runtime* pRuntime,
                           CJS_PropValue* vp,
                           WideString* sError);
  bool set_button_position(CJS_Runtime* pRuntime,
                           const CJS_PropValue& vp,
                           WideString* sError);

  bool get_button_scale_how(CJS_Runtime* pRuntime,
                            CJS_PropValue* vp,
                            WideString* sError);
  bool set_button_scale_how(CJS_Runtime* pRuntime,
                            const CJS_PropValue& vp,
                            WideString* sError);

  bool get_button_scale_when(CJS_Runtime* pRuntime,
                             CJS_PropValue* vp,
                             WideString* sError);
  bool set_button_scale_when(CJS_Runtime* pRuntime,
                             const CJS_PropValue& vp,
                             WideString* sError);

  bool get_calc_order_index(CJS_Runtime* pRuntime,
                            CJS_PropValue* vp,
                            WideString* sError);
  bool set_calc_order_index(CJS_Runtime* pRuntime,
                            const CJS_PropValue& vp,
                            WideString* sError);

  bool get_char_limit(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError);
  bool set_char_limit(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError);

  bool get_comb(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_comb(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  bool get_commit_on_sel_change(CJS_Runtime* pRuntime,
                                CJS_PropValue* vp,
                                WideString* sError);
  bool set_commit_on_sel_change(CJS_Runtime* pRuntime,
                                const CJS_PropValue& vp,
                                WideString* sError);

  bool get_current_value_indices(CJS_Runtime* pRuntime,
                                 CJS_PropValue* vp,
                                 WideString* sError);
  bool set_current_value_indices(CJS_Runtime* pRuntime,
                                 const CJS_PropValue& vp,
                                 WideString* sError);

  bool get_default_style(CJS_Runtime* pRuntime,
                         CJS_PropValue* vp,
                         WideString* sError);
  bool set_default_style(CJS_Runtime* pRuntime,
                         const CJS_PropValue& vp,
                         WideString* sError);

  bool get_default_value(CJS_Runtime* pRuntime,
                         CJS_PropValue* vp,
                         WideString* sError);
  bool set_default_value(CJS_Runtime* pRuntime,
                         const CJS_PropValue& vp,
                         WideString* sError);

  bool get_do_not_scroll(CJS_Runtime* pRuntime,
                         CJS_PropValue* vp,
                         WideString* sError);
  bool set_do_not_scroll(CJS_Runtime* pRuntime,
                         const CJS_PropValue& vp,
                         WideString* sError);

  bool get_do_not_spell_check(CJS_Runtime* pRuntime,
                              CJS_PropValue* vp,
                              WideString* sError);
  bool set_do_not_spell_check(CJS_Runtime* pRuntime,
                              const CJS_PropValue& vp,
                              WideString* sError);

  bool get_delay(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_delay(CJS_Runtime* pRuntime,
                 const CJS_PropValue& vp,
                 WideString* sError);

  bool get_display(CJS_Runtime* pRuntime,
                   CJS_PropValue* vp,
                   WideString* sError);
  bool set_display(CJS_Runtime* pRuntime,
                   const CJS_PropValue& vp,
                   WideString* sError);

  bool get_doc(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_doc(CJS_Runtime* pRuntime,
               const CJS_PropValue& vp,
               WideString* sError);

  bool get_editable(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_editable(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_export_values(CJS_Runtime* pRuntime,
                         CJS_PropValue* vp,
                         WideString* sError);
  bool set_export_values(CJS_Runtime* pRuntime,
                         const CJS_PropValue& vp,
                         WideString* sError);

  bool get_file_select(CJS_Runtime* pRuntime,
                       CJS_PropValue* vp,
                       WideString* sError);
  bool set_file_select(CJS_Runtime* pRuntime,
                       const CJS_PropValue& vp,
                       WideString* sError);

  bool get_fill_color(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError);
  bool set_fill_color(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError);

  bool get_hidden(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_hidden(CJS_Runtime* pRuntime,
                  const CJS_PropValue& vp,
                  WideString* sError);

  bool get_highlight(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_highlight(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_line_width(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError);
  bool set_line_width(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError);

  bool get_multiline(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_multiline(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_multiple_selection(CJS_Runtime* pRuntime,
                              CJS_PropValue* vp,
                              WideString* sError);
  bool set_multiple_selection(CJS_Runtime* pRuntime,
                              const CJS_PropValue& vp,
                              WideString* sError);

  bool get_name(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_name(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  bool get_num_items(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_num_items(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_page(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_page(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  bool get_password(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_password(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_print(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_print(CJS_Runtime* pRuntime,
                 const CJS_PropValue& vp,
                 WideString* sError);

  bool get_radios_in_unison(CJS_Runtime* pRuntime,
                            CJS_PropValue* vp,
                            WideString* sError);
  bool set_radios_in_unison(CJS_Runtime* pRuntime,
                            const CJS_PropValue& vp,
                            WideString* sError);

  bool get_readonly(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_readonly(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_rect(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_rect(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  bool get_required(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_required(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_rich_text(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_rich_text(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_rich_value(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError);
  bool set_rich_value(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError);

  bool get_rotation(CJS_Runtime* pRuntime,
                    CJS_PropValue* vp,
                    WideString* sError);
  bool set_rotation(CJS_Runtime* pRuntime,
                    const CJS_PropValue& vp,
                    WideString* sError);

  bool get_stroke_color(CJS_Runtime* pRuntime,
                        CJS_PropValue* vp,
                        WideString* sError);
  bool set_stroke_color(CJS_Runtime* pRuntime,
                        const CJS_PropValue& vp,
                        WideString* sError);

  bool get_style(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_style(CJS_Runtime* pRuntime,
                 const CJS_PropValue& vp,
                 WideString* sError);

  bool get_submit_name(CJS_Runtime* pRuntime,
                       CJS_PropValue* vp,
                       WideString* sError);
  bool set_submit_name(CJS_Runtime* pRuntime,
                       const CJS_PropValue& vp,
                       WideString* sError);

  bool get_text_color(CJS_Runtime* pRuntime,
                      CJS_PropValue* vp,
                      WideString* sError);
  bool set_text_color(CJS_Runtime* pRuntime,
                      const CJS_PropValue& vp,
                      WideString* sError);

  bool get_text_font(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_text_font(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_text_size(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_text_size(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_type(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_type(CJS_Runtime* pRuntime,
                const CJS_PropValue& vp,
                WideString* sError);

  bool get_user_name(CJS_Runtime* pRuntime,
                     CJS_PropValue* vp,
                     WideString* sError);
  bool set_user_name(CJS_Runtime* pRuntime,
                     const CJS_PropValue& vp,
                     WideString* sError);

  bool get_value(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_value(CJS_Runtime* pRuntime,
                 const CJS_PropValue& vp,
                 WideString* sError);

  bool get_value_as_string(CJS_Runtime* pRuntime,
                           CJS_PropValue* vp,
                           WideString* sError);
  bool set_value_as_string(CJS_Runtime* pRuntime,
                           const CJS_PropValue& vp,
                           WideString* sError);

  bool get_source(CJS_Runtime* pRuntime, CJS_PropValue* vp, WideString* sError);
  bool set_source(CJS_Runtime* pRuntime,
                  const CJS_PropValue& vp,
                  WideString* sError);

  bool browseForFileToSubmit(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError);
  bool buttonGetCaption(CJS_Runtime* pRuntime,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        WideString& sError);
  bool buttonGetIcon(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool buttonImportIcon(CJS_Runtime* pRuntime,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        WideString& sError);
  bool buttonSetCaption(CJS_Runtime* pRuntime,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        WideString& sError);
  bool buttonSetIcon(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool checkThisBox(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool clearItems(CJS_Runtime* pRuntime,
                  const std::vector<CJS_Value>& params,
                  CJS_Value& vRet,
                  WideString& sError);
  bool defaultIsChecked(CJS_Runtime* pRuntime,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        WideString& sError);
  bool deleteItemAt(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool getArray(CJS_Runtime* pRuntime,
                const std::vector<CJS_Value>& params,
                CJS_Value& vRet,
                WideString& sError);
  bool getItemAt(CJS_Runtime* pRuntime,
                 const std::vector<CJS_Value>& params,
                 CJS_Value& vRet,
                 WideString& sError);
  bool getLock(CJS_Runtime* pRuntime,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               WideString& sError);
  bool insertItemAt(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool isBoxChecked(CJS_Runtime* pRuntime,
                    const std::vector<CJS_Value>& params,
                    CJS_Value& vRet,
                    WideString& sError);
  bool isDefaultChecked(CJS_Runtime* pRuntime,
                        const std::vector<CJS_Value>& params,
                        CJS_Value& vRet,
                        WideString& sError);
  bool setAction(CJS_Runtime* pRuntime,
                 const std::vector<CJS_Value>& params,
                 CJS_Value& vRet,
                 WideString& sError);
  bool setFocus(CJS_Runtime* pRuntime,
                const std::vector<CJS_Value>& params,
                CJS_Value& vRet,
                WideString& sError);
  bool setItems(CJS_Runtime* pRuntime,
                const std::vector<CJS_Value>& params,
                CJS_Value& vRet,
                WideString& sError);
  bool setLock(CJS_Runtime* pRuntime,
               const std::vector<CJS_Value>& params,
               CJS_Value& vRet,
               WideString& sError);
  bool signatureGetModifications(CJS_Runtime* pRuntime,
                                 const std::vector<CJS_Value>& params,
                                 CJS_Value& vRet,
                                 WideString& sError);
  bool signatureGetSeedValue(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError);
  bool signatureInfo(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool signatureSetSeedValue(CJS_Runtime* pRuntime,
                             const std::vector<CJS_Value>& params,
                             CJS_Value& vRet,
                             WideString& sError);
  bool signatureSign(CJS_Runtime* pRuntime,
                     const std::vector<CJS_Value>& params,
                     CJS_Value& vRet,
                     WideString& sError);
  bool signatureValidate(CJS_Runtime* pRuntime,
                         const std::vector<CJS_Value>& params,
                         CJS_Value& vRet,
                         WideString& sError);

  bool AttachField(Document* pDocument, const WideString& csFieldName);

 private:
  static void SetBorderStyle(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                             const WideString& swFieldName,
                             int nControlIndex,
                             const ByteString& string);
  static void SetCurrentValueIndices(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                     const WideString& swFieldName,
                                     int nControlIndex,
                                     const std::vector<uint32_t>& array);
  static void SetDisplay(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                         const WideString& swFieldName,
                         int nControlIndex,
                         int number);
  static void SetHidden(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                        const WideString& swFieldName,
                        int nControlIndex,
                        bool b);
  static void SetLineWidth(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                           const WideString& swFieldName,
                           int nControlIndex,
                           int number);
  static void SetMultiline(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                           const WideString& swFieldName,
                           int nControlIndex,
                           bool b);
  static void SetRect(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                      const WideString& swFieldName,
                      int nControlIndex,
                      const CFX_FloatRect& rect);
  static void SetValue(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                       const WideString& swFieldName,
                       int nControlIndex,
                       const std::vector<WideString>& strArray);

  static void UpdateFormField(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                              CPDF_FormField* pFormField,
                              bool bChangeMark,
                              bool bResetAP,
                              bool bRefresh);
  static void UpdateFormControl(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                CPDF_FormControl* pFormControl,
                                bool bChangeMark,
                                bool bResetAP,
                                bool bRefresh);

  static CPDFSDK_Widget* GetWidget(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                   CPDF_FormControl* pFormControl);
  static std::vector<CPDF_FormField*> GetFormFields(
      CPDFSDK_FormFillEnvironment* pFormFillEnv,
      const WideString& csFieldName);

  void SetDelay(bool bDelay);
  void ParseFieldName(const std::wstring& strFieldNameParsed,
                      std::wstring& strFieldName,
                      int& iControlNo);
  std::vector<CPDF_FormField*> GetFormFields(
      const WideString& csFieldName) const;
  CPDF_FormControl* GetSmartFieldControl(CPDF_FormField* pFormField);
  bool ValueIsOccur(CPDF_FormField* pFormField, WideString csOptLabel);

  void AddDelay_Int(FIELD_PROP prop, int32_t n);
  void AddDelay_Bool(FIELD_PROP prop, bool b);
  void AddDelay_String(FIELD_PROP prop, const ByteString& string);
  void AddDelay_Rect(FIELD_PROP prop, const CFX_FloatRect& rect);
  void AddDelay_WordArray(FIELD_PROP prop, const std::vector<uint32_t>& array);
  void AddDelay_WideStringArray(FIELD_PROP prop,
                                const std::vector<WideString>& array);

  void DoDelay();

  Document* m_pJSDoc;
  CPDFSDK_FormFillEnvironment::ObservedPtr m_pFormFillEnv;
  WideString m_FieldName;
  int m_nFormControlIndex;
  bool m_bCanSet;
  bool m_bDelay;
};

class CJS_Field : public CJS_Object {
 public:
  explicit CJS_Field(v8::Local<v8::Object> pObject) : CJS_Object(pObject) {}
  ~CJS_Field() override {}

  void InitInstance(IJS_Runtime* pIRuntime) override;

  DECLARE_JS_CLASS();
  JS_STATIC_PROP(alignment, Field);
  JS_STATIC_NAMED_PROP(borderStyle, border_style, Field);
  JS_STATIC_NAMED_PROP(buttonAlignX, button_align_x, Field);
  JS_STATIC_NAMED_PROP(buttonAlignY, button_align_y, Field);
  JS_STATIC_NAMED_PROP(buttonFitBounds, button_fit_bounds, Field);
  JS_STATIC_NAMED_PROP(buttonPosition, button_position, Field);
  JS_STATIC_NAMED_PROP(buttonScaleHow, button_scale_how, Field);
  JS_STATIC_NAMED_PROP(ButtonScaleWhen, button_scale_when, Field);
  JS_STATIC_NAMED_PROP(calcOrderIndex, calc_order_index, Field);
  JS_STATIC_NAMED_PROP(charLimit, char_limit, Field);
  JS_STATIC_PROP(comb, Field);
  JS_STATIC_NAMED_PROP(commitOnSelChange, commit_on_sel_change, Field);
  JS_STATIC_NAMED_PROP(currentValueIndices, current_value_indices, Field);
  JS_STATIC_NAMED_PROP(defaultStyle, default_style, Field);
  JS_STATIC_NAMED_PROP(defaultValue, default_value, Field);
  JS_STATIC_NAMED_PROP(doNotScroll, do_not_scroll, Field);
  JS_STATIC_NAMED_PROP(doNotSpellCheck, do_not_spell_check, Field);
  JS_STATIC_PROP(delay, Field);
  JS_STATIC_PROP(display, Field);
  JS_STATIC_PROP(doc, Field);
  JS_STATIC_PROP(editable, Field);
  JS_STATIC_NAMED_PROP(exportValues, export_values, Field);
  JS_STATIC_NAMED_PROP(fileSelect, file_select, Field);
  JS_STATIC_NAMED_PROP(fillColor, fill_color, Field);
  JS_STATIC_PROP(hidden, Field);
  JS_STATIC_PROP(highlight, Field);
  JS_STATIC_NAMED_PROP(lineWidth, line_width, Field);
  JS_STATIC_PROP(multiline, Field);
  JS_STATIC_NAMED_PROP(multipleSelection, multiple_selection, Field);
  JS_STATIC_PROP(name, Field);
  JS_STATIC_NAMED_PROP(numItems, num_items, Field);
  JS_STATIC_PROP(page, Field);
  JS_STATIC_PROP(password, Field);
  JS_STATIC_PROP(print, Field);
  JS_STATIC_NAMED_PROP(radiosInUnison, radios_in_unison, Field);
  JS_STATIC_PROP(readonly, Field);
  JS_STATIC_PROP(rect, Field);
  JS_STATIC_PROP(required, Field);
  JS_STATIC_NAMED_PROP(richText, rich_text, Field);
  JS_STATIC_NAMED_PROP(richValue, rich_value, Field);
  JS_STATIC_PROP(rotation, Field);
  JS_STATIC_NAMED_PROP(strokeColor, stroke_color, Field);
  JS_STATIC_PROP(style, Field);
  JS_STATIC_NAMED_PROP(submitName, submit_name, Field);
  JS_STATIC_NAMED_PROP(textColor, text_color, Field);
  JS_STATIC_NAMED_PROP(textFont, text_font, Field);
  JS_STATIC_NAMED_PROP(textSize, text_size, Field);
  JS_STATIC_PROP(type, Field);
  JS_STATIC_NAMED_PROP(userName, user_name, Field);
  JS_STATIC_PROP(value, Field);
  JS_STATIC_NAMED_PROP(valueAsString, value_as_string, Field);
  JS_STATIC_PROP(source, Field);

  JS_STATIC_METHOD(browseForFileToSubmit, Field);
  JS_STATIC_METHOD(buttonGetCaption, Field);
  JS_STATIC_METHOD(buttonGetIcon, Field);
  JS_STATIC_METHOD(buttonImportIcon, Field);
  JS_STATIC_METHOD(buttonSetCaption, Field);
  JS_STATIC_METHOD(buttonSetIcon, Field);
  JS_STATIC_METHOD(checkThisBox, Field);
  JS_STATIC_METHOD(clearItems, Field);
  JS_STATIC_METHOD(defaultIsChecked, Field);
  JS_STATIC_METHOD(deleteItemAt, Field);
  JS_STATIC_METHOD(getArray, Field);
  JS_STATIC_METHOD(getItemAt, Field);
  JS_STATIC_METHOD(getLock, Field);
  JS_STATIC_METHOD(insertItemAt, Field);
  JS_STATIC_METHOD(isBoxChecked, Field);
  JS_STATIC_METHOD(isDefaultChecked, Field);
  JS_STATIC_METHOD(setAction, Field);
  JS_STATIC_METHOD(setFocus, Field);
  JS_STATIC_METHOD(setItems, Field);
  JS_STATIC_METHOD(setLock, Field);
  JS_STATIC_METHOD(signatureGetModifications, Field);
  JS_STATIC_METHOD(signatureGetSeedValue, Field);
  JS_STATIC_METHOD(signatureInfo, Field);
  JS_STATIC_METHOD(signatureSetSeedValue, Field);
  JS_STATIC_METHOD(signatureSign, Field);
  JS_STATIC_METHOD(signatureValidate, Field);
};

#endif  // FPDFSDK_JAVASCRIPT_FIELD_H_
