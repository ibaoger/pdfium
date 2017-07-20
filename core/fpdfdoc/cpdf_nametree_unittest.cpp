// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfdoc/cpdf_nametree.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(cpdf_nametree, GetUnicodeNameWithBOM) {
  // Set up the root dictionary with a Names array.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pNames = pRootDict->SetNewFor<CPDF_Array>("Names");

  // Add the key "1" (with BOM) and value 100 into the array.
  std::ostringstream buf;
  buf << static_cast<unsigned char>(254) << static_cast<unsigned char>(255)
      << static_cast<unsigned char>(0) << static_cast<unsigned char>(49);
  pNames->AddNew<CPDF_String>(CFX_ByteString(buf), true);
  pNames->AddNew<CPDF_Number>(100);

  // Check that the key is as expected.
  CPDF_NameTree nameTree(pRootDict.get());
  CFX_WideString storedName;
  nameTree.LookupValueAndName(0, &storedName);
  EXPECT_STREQ(L"1", storedName.c_str());

  // Check that the correct value object can be obtained by looking up "1".
  CFX_WideString matchName = L"1";
  CPDF_Object* pObj = nameTree.LookupValue(matchName);
  ASSERT_TRUE(pObj->IsNumber());
  EXPECT_EQ(100, pObj->AsNumber()->GetInteger());
}

TEST(cpdf_nametree, AddIntoNames) {
  // Set up a name tree with a single Names array.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pNames = pRootDict->SetNewFor<CPDF_Array>("Names");
  pNames->AddNew<CPDF_String>("2.txt", false);
  pNames->AddNew<CPDF_Number>(222);
  pNames->AddNew<CPDF_String>("7.txt", false);
  pNames->AddNew<CPDF_Number>(777);

  CPDF_NameTree nameTree(pRootDict.get());
  pNames = nameTree.GetRoot()->GetArrayFor("Names");

  // Insert a name that already exists in the names array.
  EXPECT_FALSE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(111), L"2.txt"));

  // Insert in the beginning of the names array.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(111), L"1.txt"));

  // Insert in the middle of the names array.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(555), L"5.txt"));

  // Insert at the end of the names array.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(999), L"9.txt"));

  EXPECT_STREQ("1.txt", pNames->GetStringAt(0).c_str());
  EXPECT_EQ(111, pNames->GetIntegerAt(1));
  EXPECT_STREQ("2.txt", pNames->GetStringAt(2).c_str());
  EXPECT_EQ(222, pNames->GetIntegerAt(3));
  EXPECT_STREQ("5.txt", pNames->GetStringAt(4).c_str());
  EXPECT_EQ(555, pNames->GetIntegerAt(5));
  EXPECT_STREQ("7.txt", pNames->GetStringAt(6).c_str());
  EXPECT_EQ(777, pNames->GetIntegerAt(7));
  EXPECT_STREQ("9.txt", pNames->GetStringAt(8).c_str());
  EXPECT_EQ(999, pNames->GetIntegerAt(9));
}

TEST(cpdf_nametree, AddIntoKids) {
  // Set up a name tree with five nodes of three levels.
  auto pRootDict = pdfium::MakeUnique<CPDF_Dictionary>();
  CPDF_Array* pKids = pRootDict->SetNewFor<CPDF_Array>("Kids");
  CPDF_Dictionary* pKids1 = pKids->AddNew<CPDF_Dictionary>();

  CPDF_Array* pLimits = pKids1->SetNewFor<CPDF_Array>("Limits");
  pLimits->AddNew<CPDF_String>("1.txt", false);
  pLimits->AddNew<CPDF_String>("9.txt", false);
  pKids = pKids1->SetNewFor<CPDF_Array>("Kids");
  CPDF_Dictionary* pKids2 = pKids->AddNew<CPDF_Dictionary>();
  CPDF_Dictionary* pKids3 = pKids->AddNew<CPDF_Dictionary>();

  pLimits = pKids2->SetNewFor<CPDF_Array>("Limits");
  pLimits->AddNew<CPDF_String>("1.txt", false);
  pLimits->AddNew<CPDF_String>("5.txt", false);
  pKids = pKids2->SetNewFor<CPDF_Array>("Kids");
  CPDF_Dictionary* pKids4 = pKids->AddNew<CPDF_Dictionary>();
  CPDF_Dictionary* pKids5 = pKids->AddNew<CPDF_Dictionary>();

  pLimits = pKids3->SetNewFor<CPDF_Array>("Limits");
  pLimits->AddNew<CPDF_String>("9.txt", false);
  pLimits->AddNew<CPDF_String>("9.txt", false);
  CPDF_Array* pNames = pKids3->SetNewFor<CPDF_Array>("Names");
  pNames->AddNew<CPDF_String>("9.txt", false);
  pNames->AddNew<CPDF_Number>(999);

  pLimits = pKids4->SetNewFor<CPDF_Array>("Limits");
  pLimits->AddNew<CPDF_String>("1.txt", false);
  pLimits->AddNew<CPDF_String>("2.txt", false);
  pNames = pKids4->SetNewFor<CPDF_Array>("Names");
  pNames->AddNew<CPDF_String>("1.txt", false);
  pNames->AddNew<CPDF_Number>(111);
  pNames->AddNew<CPDF_String>("2.txt", false);
  pNames->AddNew<CPDF_Number>(222);

  pLimits = pKids5->SetNewFor<CPDF_Array>("Limits");
  pLimits->AddNew<CPDF_String>("3.txt", false);
  pLimits->AddNew<CPDF_String>("5.txt", false);
  pNames = pKids5->SetNewFor<CPDF_Array>("Names");
  pNames->AddNew<CPDF_String>("3.txt", false);
  pNames->AddNew<CPDF_Number>(333);
  pNames->AddNew<CPDF_String>("5.txt", false);
  pNames->AddNew<CPDF_Number>(555);

  CPDF_NameTree nameTree(pRootDict.get());

  // Check that adding an existing name would fail.
  EXPECT_FALSE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(444), L"9.txt"));

  // Add a name within the limits of a leaf node.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(444), L"4.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"4.txt"));
  EXPECT_EQ(444, nameTree.LookupValue(L"4.txt")->GetInteger());

  // Add a name that requires changing the limits of two bottom levels.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(666), L"6.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"6.txt"));
  EXPECT_EQ(666, nameTree.LookupValue(L"6.txt")->GetInteger());

  // Add a name that requires changing the limits of two top levels.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(99), L"99.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"99.txt"));
  EXPECT_EQ(99, nameTree.LookupValue(L"99.txt")->GetInteger());

  // Add a name that requires changing the lower limit of all levels.
  EXPECT_TRUE(
      nameTree.AddValueAndName(pdfium::MakeUnique<CPDF_Number>(-5), L"0.txt"));
  ASSERT_TRUE(nameTree.LookupValue(L"0.txt"));
  EXPECT_EQ(-5, nameTree.LookupValue(L"0.txt")->GetInteger());

  // Check that the node on the first level has the expected limits.
  pKids1 = nameTree.GetRoot()->GetArrayFor("Kids")->GetDictAt(0);
  pLimits = pKids1->GetArrayFor("Limits");
  EXPECT_STREQ("0.txt", pLimits->GetStringAt(0).c_str());
  EXPECT_STREQ("99.txt", pLimits->GetStringAt(1).c_str());

  // Check that the nodes on the second level has the expected limits and names.
  pKids2 = pKids1->GetArrayFor("Kids")->GetDictAt(0);
  pLimits = pKids2->GetArrayFor("Limits");
  EXPECT_STREQ("0.txt", pLimits->GetStringAt(0).c_str());
  EXPECT_STREQ("6.txt", pLimits->GetStringAt(1).c_str());

  pKids3 = pKids1->GetArrayFor("Kids")->GetDictAt(1);
  pLimits = pKids3->GetArrayFor("Limits");
  EXPECT_STREQ("9.txt", pLimits->GetStringAt(0).c_str());
  EXPECT_STREQ("99.txt", pLimits->GetStringAt(1).c_str());
  pNames = pKids3->GetArrayFor("Names");
  EXPECT_STREQ("9.txt", pNames->GetStringAt(0).c_str());
  EXPECT_EQ(999, pNames->GetIntegerAt(1));
  EXPECT_STREQ("99.txt", pNames->GetStringAt(2).c_str());
  EXPECT_EQ(99, pNames->GetIntegerAt(3));

  // Check that the nodes on the third level has the expected limits and names.
  pKids4 = pKids2->GetArrayFor("Kids")->GetDictAt(0);
  pLimits = pKids4->GetArrayFor("Limits");
  EXPECT_STREQ("0.txt", pLimits->GetStringAt(0).c_str());
  EXPECT_STREQ("2.txt", pLimits->GetStringAt(1).c_str());
  pNames = pKids4->GetArrayFor("Names");
  EXPECT_STREQ("0.txt", pNames->GetStringAt(0).c_str());
  EXPECT_EQ(-5, pNames->GetIntegerAt(1));
  EXPECT_STREQ("1.txt", pNames->GetStringAt(2).c_str());
  EXPECT_EQ(111, pNames->GetIntegerAt(3));
  EXPECT_STREQ("2.txt", pNames->GetStringAt(4).c_str());
  EXPECT_EQ(222, pNames->GetIntegerAt(5));

  pKids5 = pKids2->GetArrayFor("Kids")->GetDictAt(1);
  pLimits = pKids5->GetArrayFor("Limits");
  EXPECT_STREQ("3.txt", pLimits->GetStringAt(0).c_str());
  EXPECT_STREQ("6.txt", pLimits->GetStringAt(1).c_str());
  pNames = pKids5->GetArrayFor("Names");
  EXPECT_STREQ("3.txt", pNames->GetStringAt(0).c_str());
  EXPECT_EQ(333, pNames->GetIntegerAt(1));
  EXPECT_STREQ("4.txt", pNames->GetStringAt(2).c_str());
  EXPECT_EQ(444, pNames->GetIntegerAt(3));
  EXPECT_STREQ("5.txt", pNames->GetStringAt(4).c_str());
  EXPECT_EQ(555, pNames->GetIntegerAt(5));
  EXPECT_STREQ("6.txt", pNames->GetStringAt(6).c_str());
  EXPECT_EQ(666, pNames->GetIntegerAt(7));
}
