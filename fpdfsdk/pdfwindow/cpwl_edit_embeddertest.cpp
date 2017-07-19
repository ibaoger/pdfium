// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cba_annotiterator.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPWLEditEmbeddertest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    CreateAndInitializeFormPDF();
  }

  void TearDown() override {
    UnloadPage(GetPage());
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormPDF() {
    EXPECT_TRUE(OpenDocument("text_form.pdf"));
    m_page = LoadPage(0);
    ASSERT_TRUE(m_page);

    CPDFSDK_FormFillEnvironment* pFormFillEnv =
        static_cast<CPDFSDK_FormFillEnvironment*>(form_handle());

    {
      CBA_AnnotIterator iter(pFormFillEnv->GetPageView(0),
                             CPDF_Annot::Subtype::WIDGET);
      m_pAnnot = iter.GetFirstAnnot();
      CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
      ASSERT_EQ(m_pAnnot, pLastAnnot);
      ASSERT_TRUE(m_pAnnot);
      ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, m_pAnnot->GetAnnotSubtype());
    }

    CFFL_InteractiveFormFiller* pInteractiveFormFiller =
        pFormFillEnv->GetInteractiveFormFiller();
    {
      CPDFSDK_Annot::ObservedPtr pObserved(m_pAnnot);
      EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(&pObserved, 0));
    }

    m_pFormFiller = pInteractiveFormFiller->GetFormFiller(m_pAnnot, false);
    ASSERT_TRUE(m_pFormFiller);

    CPWL_Wnd* pWindow =
        m_pFormFiller->GetPDFWindow(pFormFillEnv->GetPageView(0), false);
    ASSERT_TRUE(pWindow);
    ASSERT_EQ(PWL_CLASSNAME_EDIT, pWindow->GetClassName());

    m_pEdit = static_cast<CPWL_Edit*>(pWindow);
  }

  FPDF_PAGE GetPage() { return m_page; }
  CPWL_Edit* GetCPWLEdit() { return m_pEdit; }
  CFFL_FormFiller* GetCFFLFormFiller() { return m_pFormFiller; }
  CPDFSDK_Annot* GetCPDFSDKAnnot() { return m_pAnnot; }

 private:
  FPDF_PAGE m_page;
  CPWL_Edit* m_pEdit;
  CFFL_FormFiller* m_pFormFiller;
  CPDFSDK_Annot* m_pAnnot;
};

TEST_F(CPWLEditEmbeddertest, TypeText) {
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', 0));

  EXPECT_STREQ(L"abc", GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, GetSelectedTextEmptyAndBasic) {
  // Attempt to set selection before text has been typed to test that
  // selection is identified as empty.
  //
  // Select from character index [0, 3) within form text field.
  GetCPWLEdit()->SetSel(0, 3);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', 0));
  GetCPWLEdit()->SetSel(0, 2);

  EXPECT_STREQ(L"ab", GetCPWLEdit()->GetSelectedText().c_str());
}

TEST_F(CPWLEditEmbeddertest, GetSelectedTextFragments) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSel(0, 0);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  GetCPWLEdit()->SetSel(0, 1);
  EXPECT_STREQ(L"A", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSel(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSel(-8, -1);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  GetCPWLEdit()->SetSel(23, 12);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSel(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSel(49, 50);
  EXPECT_STREQ(L"r", GetCPWLEdit()->GetSelectedText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelection) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSel(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->ExtendSelectionAndDelete(0, 0);
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionMiddlePlus1Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSel(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->ExtendSelectionAndDelete(1, 1);
  EXPECT_STREQ(L"ABCDEFGHIJKYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionLeftPlus1Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSel(0, 5);
  EXPECT_STREQ(L"ABCDE", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->ExtendSelectionAndDelete(1, 1);
  EXPECT_STREQ(L"GHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionRightPlus1Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSel(45, 50);
  EXPECT_STREQ(L"nopqr", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->ExtendSelectionAndDelete(1, 1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijkl",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionMiddlePlus2Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSel(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->ExtendSelectionAndDelete(2, 1);
  EXPECT_STREQ(L"ABCDEFGHIJYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEmptyTextSelection) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->ExtendSelectionAndDelete(0, 0);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEmptyTextSelectionMiddlePlus1Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  // Move caret to middle of text.
  for (int i = 49; i >= 25; --i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Left, 0));
  }

  GetCPWLEdit()->ExtendSelectionAndDelete(1, 1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWX[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEmptyTextSelectionLeftPlus1Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  // Move caret to beginning of text.
  EXPECT_TRUE(
      GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Home, 0));

  GetCPWLEdit()->ExtendSelectionAndDelete(1, 1);
  EXPECT_STREQ(L"BCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEmptyTextSelectionRightPlus1Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->ExtendSelectionAndDelete(1, 1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopq",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEmptyTextSelectionMiddlePlus2Before1After) {
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  // Move caret to middle of text.
  for (int i = 49; i >= 25; --i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Left, 0));
  }

  GetCPWLEdit()->ExtendSelectionAndDelete(2, 1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVW[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}
