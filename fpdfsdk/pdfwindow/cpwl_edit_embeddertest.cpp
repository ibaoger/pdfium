// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// DELETE
#include <iostream>

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
  void CreateAndInitializeFormPDF() {
    EXPECT_TRUE(OpenDocument("text_form.pdf"));
    page_ = LoadPage(0);
    ASSERT_TRUE(page_);

    CPDFSDK_FormFillEnvironment* pFormFillEnv =
        static_cast<CPDFSDK_FormFillEnvironment*>(form_handle());

    {
      CBA_AnnotIterator iter(pFormFillEnv->GetPageView(0),
                             CPDF_Annot::Subtype::WIDGET);
      pAnnot_ = iter.GetFirstAnnot();
      CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
      ASSERT_EQ(pAnnot_, pLastAnnot);
      ASSERT_TRUE(pAnnot_);
      ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, pAnnot_->GetAnnotSubtype());
    }

    CFFL_InteractiveFormFiller* pInteractiveFormFiller =
        pFormFillEnv->GetInteractiveFormFiller();
    {
      CPDFSDK_Annot::ObservedPtr pObserved(pAnnot_);
      EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(&pObserved, 0));
    }

    pFormFiller_ = pInteractiveFormFiller->GetFormFiller(pAnnot_, false);
    ASSERT_TRUE(pFormFiller_);

    CPWL_Wnd* pWindow =
        pFormFiller_->GetPDFWindow(pFormFillEnv->GetPageView(0), false);
    ASSERT_TRUE(pWindow);
    ASSERT_EQ(PWL_CLASSNAME_EDIT, pWindow->GetClassName());

    pEdit_ = static_cast<CPWL_Edit*>(pWindow);
  }

  FPDF_PAGE GetPage() { return page_; }
  CPWL_Edit* GetCPWL_Edit() { return pEdit_; }
  CFFL_FormFiller* GetCFFL_FormFiller() { return pFormFiller_; }
  CPDFSDK_Annot* GetCPDFSDK_Annot() { return pAnnot_; }

 private:
  FPDF_PAGE page_;
  CPWL_Edit* pEdit_;
  CFFL_FormFiller* pFormFiller_;
  CPDFSDK_Annot* pAnnot_;
};

TEST_F(CPWLEditEmbeddertest, TypeText) {
  CreateAndInitializeFormPDF();

  EXPECT_TRUE(GetCPWL_Edit()->GetText().IsEmpty());
  EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), 'a', 0));
  EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), 'b', 0));
  EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), 'c', 0));

  EXPECT_STREQ(L"abc", GetCPWL_Edit()->GetText().c_str());

  UnloadPage(GetPage());
}

TEST_F(CPWLEditEmbeddertest, GetSelectedTextEmptyAndBasic) {
  CreateAndInitializeFormPDF();

  GetCPWL_Edit()->SetSel(0, 3);
  EXPECT_TRUE(GetCPWL_Edit()->GetSelectedText().IsEmpty());

  EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), 'a', 0));
  EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), 'b', 0));
  EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), 'c', 0));
  GetCPWL_Edit()->SetSel(0, 2);

  EXPECT_STREQ(L"ab", GetCPWL_Edit()->GetSelectedText().c_str());

  UnloadPage(GetPage());
}

TEST_F(CPWLEditEmbeddertest, GetSelectedTextFragments) {
  CreateAndInitializeFormPDF();

  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(GetCFFL_FormFiller()->OnChar(GetCPDFSDK_Annot(), i + 'A', 0));
  }

  GetCPWL_Edit()->SetSel(0, 0);
  EXPECT_TRUE(GetCPWL_Edit()->GetSelectedText().IsEmpty());

  GetCPWL_Edit()->SetSel(0, 1);
  EXPECT_STREQ(L"A", GetCPWL_Edit()->GetSelectedText().c_str());

  GetCPWL_Edit()->SetSel(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWL_Edit()->GetSelectedText().c_str());

  GetCPWL_Edit()->SetSel(-8, -1);
  EXPECT_TRUE(GetCPWL_Edit()->GetSelectedText().IsEmpty());

  GetCPWL_Edit()->SetSel(23, 12);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWL_Edit()->GetSelectedText().c_str());

  GetCPWL_Edit()->SetSel(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWL_Edit()->GetSelectedText().c_str());

  GetCPWL_Edit()->SetSel(49, 50);
  EXPECT_STREQ(L"r", GetCPWL_Edit()->GetSelectedText().c_str());

  UnloadPage(GetPage());
}
