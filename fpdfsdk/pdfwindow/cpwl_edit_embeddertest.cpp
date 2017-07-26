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
    EXPECT_TRUE(OpenDocument("text_form_multiple.pdf"));
    m_page = LoadPage(0);
    ASSERT_TRUE(m_page);

    m_pFormFillEnv = static_cast<CPDFSDK_FormFillEnvironment*>(form_handle());
    CBA_AnnotIterator iter(m_pFormFillEnv->GetPageView(0),
                           CPDF_Annot::Subtype::WIDGET);
    // Normal text field.
    m_pAnnot = iter.GetFirstAnnot();
    ASSERT_TRUE(m_pAnnot);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, m_pAnnot->GetAnnotSubtype());

    // Read-only text field.
    CPDFSDK_Annot* pAnnotReadOnly = iter.GetNextAnnot(m_pAnnot);

    // Pre-filled text field with char limit of 10.
    m_pAnnotCharLimit = iter.GetNextAnnot(pAnnotReadOnly);
    ASSERT_TRUE(m_pAnnotCharLimit);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              m_pAnnotCharLimit->GetAnnotSubtype());
    CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
    ASSERT_EQ(m_pAnnotCharLimit, pLastAnnot);
  }

  void FormFillerAndWindowSetup(CPDFSDK_Annot* pAnnotTextField) {
    CFFL_InteractiveFormFiller* pInteractiveFormFiller =
        m_pFormFillEnv->GetInteractiveFormFiller();
    {
      CPDFSDK_Annot::ObservedPtr pObserved(pAnnotTextField);
      EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(&pObserved, 0));
    }

    m_pFormFiller =
        pInteractiveFormFiller->GetFormFiller(pAnnotTextField, false);
    ASSERT_TRUE(m_pFormFiller);

    CPWL_Wnd* pWindow =
        m_pFormFiller->GetPDFWindow(m_pFormFillEnv->GetPageView(0), false);
    ASSERT_TRUE(pWindow);
    ASSERT_EQ(PWL_CLASSNAME_EDIT, pWindow->GetClassName());

    m_pEdit = static_cast<CPWL_Edit*>(pWindow);
  }

  FPDF_PAGE GetPage() { return m_page; }
  CPWL_Edit* GetCPWLEdit() { return m_pEdit; }
  CFFL_FormFiller* GetCFFLFormFiller() { return m_pFormFiller; }
  CPDFSDK_Annot* GetCPDFSDKAnnot() { return m_pAnnot; }
  CPDFSDK_Annot* GetCPDFSDKAnnotCharLimit() { return m_pAnnotCharLimit; }

 private:
  FPDF_PAGE m_page;
  CPWL_Edit* m_pEdit;
  CFFL_FormFiller* m_pFormFiller;
  CPDFSDK_Annot* m_pAnnot;
  CPDFSDK_Annot* m_pAnnotCharLimit;
  CPDFSDK_FormFillEnvironment* m_pFormFillEnv;
};

TEST_F(CPWLEditEmbeddertest, TypeText) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', 0));

  EXPECT_STREQ(L"abc", GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, GetSelectedTextEmptyAndBasic) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  // Attempt to set selection before text has been typed to test that
  // selection is identified as empty.
  //
  // Select from character index [0, 3) within form text field.
  GetCPWLEdit()->SetSelection(0, 3);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', 0));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', 0));
  GetCPWLEdit()->SetSelection(0, 2);

  EXPECT_STREQ(L"ab", GetCPWLEdit()->GetSelectedText().c_str());
}

TEST_F(CPWLEditEmbeddertest, GetSelectedTextFragments) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(0, 0);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  GetCPWLEdit()->SetSelection(0, 1);
  EXPECT_STREQ(L"A", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSelection(-8, -1);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  GetCPWLEdit()->SetSelection(23, 12);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSelection(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->SetSelection(49, 50);
  EXPECT_STREQ(L"r", GetCPWLEdit()->GetSelectedText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEntireTextSelection) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->DeleteSelectedText();
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->DeleteSelectedText();
  EXPECT_STREQ(L"ABCDEFGHIJKLXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(0, 5);
  EXPECT_STREQ(L"ABCDE", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->DeleteSelectedText();
  EXPECT_STREQ(L"FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteTextSelectionRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(45, 50);
  EXPECT_STREQ(L"nopqr", GetCPWLEdit()->GetSelectedText().c_str());

  GetCPWLEdit()->DeleteSelectedText();
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, DeleteEmptyTextSelection) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->DeleteSelectedText();
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLEdit()->GetText().c_str());
}

TEST_F(CPWLEditEmbeddertest, InsertTextInEmptyTextField) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"Hello");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInPopulatedTextFieldLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  // Move cursor to beginning of text field.
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Left, 0));
  }

  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"HelloABCDEFGHIJ");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInPopulatedTextFieldMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  // Move cursor to middle of text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Left, 0));
  }

  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ABCDEHelloFGHIJ");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInPopulatedTextFieldRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ABCDEFGHIJHello");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldWhole) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJ", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"Hello");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(0, 5);
  EXPECT_STREQ(L"ABCDE", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"HelloFGHIJ");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(2, 7);
  EXPECT_STREQ(L"CDEFG", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ABHelloHIJ");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  for (int i = 0; i < 10; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', 0));
  }

  GetCPWLEdit()->SetSelection(5, 10);
  EXPECT_STREQ(L"FGHIJ", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hello"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ABCDEHello");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInEmptyCharLimitTextFieldOverflow) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_STREQ(L"Elephant", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->DeleteSelectedText();

  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"Hippopotam");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInEmptyCharLimitTextFieldFit) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_STREQ(L"Elephant", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->DeleteSelectedText();

  GetCPWLEdit()->InsertText(CFX_WideString(L"Zebra"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"Zebra");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInPopulatedCharLimitTextFieldLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"HiElephant");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInPopulatedCharLimitTextFieldMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  // Move cursor to middle of text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Right, 0));
  }

  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ElephHiant");
}

TEST_F(CPWLEditEmbeddertest, InsertTextInPopulatedCharLimitTextFieldRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  // Move cursor to end of text field.
  for (int i = 0; i < 8; ++i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnKeyDown(GetCPDFSDKAnnot(), FWL_VKEY_Right, 0));
  }

  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ElephantHi");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldWhole) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_STREQ(L"Elephant", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"Hippopotam");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, 4);
  EXPECT_STREQ(L"Elep", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"Hippophant");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(2, 6);
  EXPECT_STREQ(L"epha", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ElHippopnt");
}

TEST_F(CPWLEditEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(4, 8);
  EXPECT_STREQ(L"hant", GetCPWLEdit()->GetSelectedText().c_str());
  GetCPWLEdit()->InsertText(CFX_WideString(L"Hippopotamus"));
  EXPECT_STREQ(GetCPWLEdit()->GetText().c_str(), L"ElepHippop");
}
