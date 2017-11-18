// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_EVENTHANDLER_H_
#define FXJS_CJS_EVENTHANDLER_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"

class CJS_EventContext;
class CPDFSDK_Annot;
class CPDF_Bookmark;
class CPDF_FormField;
class Field;

enum JS_EVENT_T {
  JET_UNKNOWN,
  JET_APP_INIT,
  JET_DOC_OPEN,
  JET_DOC_WILLPRINT,
  JET_DOC_DIDPRINT,
  JET_DOC_WILLSAVE,
  JET_DOC_DIDSAVE,
  JET_DOC_WILLCLOSE,
  JET_PAGE_OPEN,
  JET_PAGE_CLOSE,
  JET_PAGE_INVIEW,
  JET_PAGE_OUTVIEW,
  JET_FIELD_MOUSEDOWN,
  JET_FIELD_MOUSEUP,
  JET_FIELD_MOUSEENTER,
  JET_FIELD_MOUSEEXIT,
  JET_FIELD_FOCUS,
  JET_FIELD_BLUR,
  JET_FIELD_KEYSTROKE,
  JET_FIELD_VALIDATE,
  JET_FIELD_CALCULATE,
  JET_FIELD_FORMAT,
  JET_SCREEN_FOCUS,
  JET_SCREEN_BLUR,
  JET_SCREEN_OPEN,
  JET_SCREEN_CLOSE,
  JET_SCREEN_MOUSEDOWN,
  JET_SCREEN_MOUSEUP,
  JET_SCREEN_MOUSEENTER,
  JET_SCREEN_MOUSEEXIT,
  JET_SCREEN_INVIEW,
  JET_SCREEN_OUTVIEW,
  JET_BATCH_EXEC,
  JET_MENU_EXEC,
  JET_CONSOLE_EXEC,
  JET_EXTERNAL_EXEC,
  JET_BOOKMARK_MOUSEUP,
  JET_LINK_MOUSEUP
};

class CJS_EventHandler {
 public:
  explicit CJS_EventHandler(CJS_EventContext* pContext);
  virtual ~CJS_EventHandler();

  void OnApp_Init();

  void OnDoc_Open(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                  const WideString& strTargetName);
  void OnDoc_WillPrint(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnDoc_DidPrint(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnDoc_WillSave(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnDoc_DidSave(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnDoc_WillClose(CPDFSDK_FormFillEnvironment* pFormFillEnv);

  void OnPage_Open(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnPage_Close(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnPage_InView(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnPage_OutView(CPDFSDK_FormFillEnvironment* pFormFillEnv);

  void OnField_Calculate(CPDF_FormField* pSource,
                         CPDF_FormField* pTarget,
                         WideString& Value,
                         bool& bRc);
  void OnField_Format(CPDF_FormField* pTarget,
                      WideString& Value,
                      bool bWillCommit);
  void OnField_Keystroke(WideString& strChange,
                         const WideString& strChangeEx,
                         bool KeyDown,
                         bool bModifier,
                         int& nSelEnd,
                         int& nSelStart,
                         bool bShift,
                         CPDF_FormField* pTarget,
                         WideString& Value,
                         bool bWillCommit,
                         bool bFieldFull,
                         bool& bRc);
  void OnField_Validate(WideString& strChange,
                        const WideString& strChangeEx,
                        bool bKeyDown,
                        bool bModifier,
                        bool bShift,
                        CPDF_FormField* pTarget,
                        WideString& Value,
                        bool& bRc);

  void OnField_MouseDown(bool bModifier, bool bShift, CPDF_FormField* pTarget);
  void OnField_MouseEnter(bool bModifier, bool bShift, CPDF_FormField* pTarget);
  void OnField_MouseExit(bool bModifier, bool bShift, CPDF_FormField* pTarget);
  void OnField_MouseUp(bool bModifier, bool bShift, CPDF_FormField* pTarget);
  void OnField_Blur(bool bModifier,
                    bool bShift,
                    CPDF_FormField* pTarget,
                    const WideString& Value);
  void OnField_Focus(bool bModifier,
                     bool bShift,
                     CPDF_FormField* pTarget,
                     const WideString& Value);

  void OnScreen_Focus(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_Blur(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_Open(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_Close(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_MouseDown(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_MouseUp(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_MouseEnter(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_MouseExit(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_InView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);
  void OnScreen_OutView(bool bModifier, bool bShift, CPDFSDK_Annot* pScreen);

  void OnBookmark_MouseUp(CPDF_Bookmark* pBookMark);
  void OnLink_MouseUp(CPDFSDK_FormFillEnvironment* pFormFillEnv);

  void OnMenu_Exec(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                   const WideString& strTargetName);
  void OnBatchExec(CPDFSDK_FormFillEnvironment* pFormFillEnv);
  void OnConsole_Exec();
  void OnExternal_Exec();

  void Initial(JS_EVENT_T type);
  void Destroy();
  bool IsValid() const;

  WideString& Change();
  const WideString& ChangeEx();
  int CommitKey() const;
  bool FieldFull() const;
  bool KeyDown() const;
  bool Modifier() const;
  const wchar_t* Name() const;
  const wchar_t* Type() const;
  bool& Rc();
  int SelEnd() const;
  int SelStart() const;
  void SetSelEnd(int value);
  void SetSelStart(int value);
  bool Shift() const;
  Field* Source();
  Field* Target_Field();
  WideString& Value();
  bool WillCommit() const;
  const WideString& TargetName() const;

  JS_EVENT_T EventType() const { return m_eEventType; }

  UnownedPtr<CJS_EventContext> const m_pJSEventContext;
  JS_EVENT_T m_eEventType;
  bool m_bValid;

  WideString m_strTargetName;
  WideString m_strSourceName;
  UnownedPtr<WideString> m_pWideStrChange;
  WideString m_WideStrChangeDu;
  WideString m_WideStrChangeEx;
  int m_nCommitKey;
  bool m_bKeyDown;
  bool m_bModifier;
  bool m_bShift;
  int* m_pISelEnd;
  int m_nSelEndDu;
  int* m_pISelStart;
  int m_nSelStartDu;
  bool m_bWillCommit;
  UnownedPtr<WideString> m_pValue;
  bool m_bFieldFull;
  bool* m_pbRc;
  bool m_bRcDu;

  UnownedPtr<CPDF_Bookmark> m_pTargetBookMark;
  CPDFSDK_FormFillEnvironment::ObservedPtr m_pTargetFormFillEnv;
  CPDFSDK_Annot::ObservedPtr m_pTargetAnnot;
};

#endif  // FXJS_CJS_EVENTHANDLER_H_
