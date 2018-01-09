// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "public/fpdf_ppo.h"

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_object.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fpdfapi/parser/cpdf_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/fsdk_define.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

enum class Axis {
  X,
  Y,
};
enum Position {
  CENTER = 0,
  LEFT = -1,
  RIGHT = 1,
  TOP = 1,
  BOTTOM = -1,
};

struct NupPageSettings {
  CFX_PointF point;
  float xScale;
  float yScale;
  CFX_RectF subPageRect;
};

/*
 * Calculates the N-up parameters.
 */
class NupState {
 public:
  NupState(size_t numPagesPerSheet);
  static bool IsPossible(size_t numPagesPerSheet);
  void PresetParameters(size_t numPagesPerSheet);
  void setWidth(size_t width) { newPageWidth = width; };
  void setHeight(size_t height) { newPageHeight = height; };
  bool isLandscape() { return isPageLandscape; };

  // returns true, if a new output page needs to be created.
  bool CreateNewPage(float in_width, float in_height, NupPageSettings& ret);

 private:
  std::pair<size_t, size_t> ConvertPageOrder(size_t subPage) const;
  void CalculatePageEdit(size_t subx, size_t suby, NupPageSettings& ret) const;

 private:
  size_t numPagesOnXAxis;
  size_t numPagesOnYAxis;
  float newPageWidth;
  float newPageHeight;
  bool isPageLandscape;
  Axis first;
  Position xStart;
  Position yStart;

  size_t inPages;
  size_t outPages;
  size_t numPagesPerSheet;
  size_t subPage;
};

NupState::NupState(size_t numPagesPerSheet)
    : newPageWidth(NAN),
      newPageHeight(NAN),
      isPageLandscape(false),
      first(Axis::X),
      xStart(LEFT),
      yStart(TOP),
      inPages(0),
      outPages(0),
      numPagesPerSheet(numPagesPerSheet),
      subPage(numPagesPerSheet) {
  PresetParameters(numPagesPerSheet);
  ASSERT((numPagesOnXAxis > 0) && (numPagesOnYAxis > 0));
}

bool NupState::IsPossible(size_t numPagesPerSheet) {
  // Supports 1 2 3 4 6 8 9 10 12 15 16

  return numPagesPerSheet != 14 && numPagesPerSheet > 0 &&
         numPagesPerSheet <= 16 &&
         (numPagesPerSheet == 1 || (numPagesPerSheet % 2) == 0 ||
          (numPagesPerSheet % 3) == 0);
}

void NupState::PresetParameters(size_t numPagesPerSheet) {
  switch (numPagesPerSheet) {
    case 1:
      numPagesOnXAxis = 1;
      numPagesOnYAxis = 1;
      break;
    case 2:
      numPagesOnXAxis = 2;
      numPagesOnYAxis = 1;
      isPageLandscape = true;
      break;
    case 3:
      numPagesOnXAxis = 3;
      numPagesOnYAxis = 1;
      isPageLandscape = true;
      break;
    case 4:
      numPagesOnXAxis = 2;
      numPagesOnYAxis = 2;
      break;
    case 6:
      numPagesOnXAxis = 3;
      numPagesOnYAxis = 2;
      isPageLandscape = true;
      break;
    case 8:
      numPagesOnXAxis = 4;
      numPagesOnYAxis = 2;
      isPageLandscape = true;
      break;
    case 9:
      numPagesOnXAxis = 3;
      numPagesOnYAxis = 3;
      break;
    case 10:
      numPagesOnXAxis = 5;
      numPagesOnYAxis = 2;
      isPageLandscape = true;
      break;
    case 12:
      numPagesOnXAxis = 3;
      numPagesOnYAxis = 4;
      break;
    case 15:
      numPagesOnXAxis = 5;
      numPagesOnYAxis = 3;
      isPageLandscape = true;
      break;
    case 16:
      numPagesOnXAxis = 4;
      numPagesOnYAxis = 4;
      break;
    default:
      numPagesOnXAxis = 1;
      numPagesOnYAxis = 1;
      break;
  }
}

std::pair<size_t, size_t> NupState::ConvertPageOrder(size_t subPage) const {
  size_t subX, subY;
  if (first == Axis::X) {
    subX = subPage % numPagesOnXAxis;
    subY = subPage / numPagesOnXAxis;
  } else {
    subX = subPage / numPagesOnYAxis;
    subY = subPage % numPagesOnYAxis;
  }

  subX = (numPagesOnXAxis - 1) * (xStart + 1) / 2 - xStart * subX;
  subY = (numPagesOnYAxis - 1) * (yStart + 1) / 2 - yStart * subY;

  return std::make_pair(subX, subY);
}

void NupState::CalculatePageEdit(size_t subXPos,
                                 size_t subYPos,
                                 NupPageSettings& pageEdit) const {
  const float subPageWidth = newPageWidth / numPagesOnXAxis,
              subPageHeight = newPageHeight / numPagesOnYAxis;

  pageEdit.point.x = subXPos * subPageWidth;
  pageEdit.point.y = subYPos * subPageHeight;

  pageEdit.xScale = subPageWidth / pageEdit.subPageRect.width;
  pageEdit.yScale = subPageHeight / pageEdit.subPageRect.height;
}

bool NupState::CreateNewPage(float inWidth,
                             float inHeight,
                             NupPageSettings& pageEdit) {
  ++inPages;
  ++subPage;
  if (subPage >= numPagesPerSheet) {
    subPage = 0;
    ++outPages;
  }
  pageEdit.subPageRect.width = inWidth;
  pageEdit.subPageRect.height = inHeight;
  size_t subX;
  size_t subY;
  std::tie(subX, subY) = ConvertPageOrder(subPage);
  CalculatePageEdit(subX, subY, pageEdit);
  return subPage == 0;
}

CFX_FloatRect GetMediaBox(CPDF_Dictionary* pPageDict) {
  return pPageDict->GetRectFor("MediaBox");
}

CFX_FloatRect GetCropBox(CPDF_Dictionary* pPageDict) {
  if (pPageDict->KeyExist("CropBox"))
    return pPageDict->GetRectFor("CropBox");
  return GetMediaBox(pPageDict);
}

CFX_FloatRect GetTrimBox(CPDF_Dictionary* pPageDict) {
  if (pPageDict->KeyExist("TrimBox"))
    return pPageDict->GetRectFor("TrimBox");
  return GetCropBox(pPageDict);
}

CPDF_Object* GetPageContent(CPDF_Dictionary* pPageDict) {
  return pPageDict ? pPageDict->GetDirectObjectFor("Contents") : nullptr;
}

CPDF_Object* PageDictGetInheritableTag(CPDF_Dictionary* pDict,
                                       const ByteString& bsSrcTag) {
  if (!pDict || bsSrcTag.IsEmpty())
    return nullptr;
  if (!pDict->KeyExist("Parent") || !pDict->KeyExist("Type"))
    return nullptr;

  CPDF_Object* pType = pDict->GetObjectFor("Type")->GetDirect();
  if (!ToName(pType))
    return nullptr;
  if (pType->GetString().Compare("Page"))
    return nullptr;

  CPDF_Dictionary* pp =
      ToDictionary(pDict->GetObjectFor("Parent")->GetDirect());
  if (!pp)
    return nullptr;

  if (pDict->KeyExist(bsSrcTag))
    return pDict->GetObjectFor(bsSrcTag);

  while (pp) {
    if (pp->KeyExist(bsSrcTag))
      return pp->GetObjectFor(bsSrcTag);
    if (!pp->KeyExist("Parent"))
      break;
    pp = ToDictionary(pp->GetObjectFor("Parent")->GetDirect());
  }
  return nullptr;
}

bool CopyInheritable(CPDF_Dictionary* pCurPageDict,
                     CPDF_Dictionary* pSrcPageDict,
                     const ByteString& key) {
  if (pCurPageDict->KeyExist(key))
    return true;

  CPDF_Object* pInheritable = PageDictGetInheritableTag(pSrcPageDict, key);
  if (!pInheritable)
    return false;

  pCurPageDict->SetFor(key, pInheritable->Clone());
  return true;
}

bool ParserPageRangeString(ByteString rangstring,
                           std::vector<uint32_t>* pageArray,
                           uint32_t nCount) {
  if (rangstring.IsEmpty())
    return true;

  rangstring.Remove(' ');
  size_t nLength = rangstring.GetLength();
  ByteString cbCompareString("0123456789-,");
  for (size_t i = 0; i < nLength; ++i) {
    if (!cbCompareString.Contains(rangstring[i]))
      return false;
  }

  ByteString cbMidRange;
  size_t nStringFrom = 0;
  Optional<size_t> nStringTo = 0;
  while (nStringTo < nLength) {
    nStringTo = rangstring.Find(',', nStringFrom);
    if (!nStringTo.has_value())
      nStringTo = nLength;
    cbMidRange = rangstring.Mid(nStringFrom, nStringTo.value() - nStringFrom);
    auto nMid = cbMidRange.Find('-');
    if (!nMid.has_value()) {
      uint32_t pageNum =
          pdfium::base::checked_cast<uint32_t>(atoi(cbMidRange.c_str()));
      if (pageNum <= 0 || pageNum > nCount)
        return false;
      pageArray->push_back(pageNum);
    } else {
      uint32_t nStartPageNum = pdfium::base::checked_cast<uint32_t>(
          atoi(cbMidRange.Left(nMid.value()).c_str()));
      if (nStartPageNum == 0)
        return false;

      nMid = nMid.value() + 1;
      size_t nEnd = cbMidRange.GetLength() - nMid.value();
      if (nEnd == 0)
        return false;

      uint32_t nEndPageNum = pdfium::base::checked_cast<uint32_t>(
          atoi(cbMidRange.Mid(nMid.value(), nEnd).c_str()));
      if (nStartPageNum < 0 || nStartPageNum > nEndPageNum ||
          nEndPageNum > nCount) {
        return false;
      }
      for (uint32_t i = nStartPageNum; i <= nEndPageNum; ++i) {
        pageArray->push_back(i);
      }
    }
    nStringFrom = nStringTo.value() + 1;
  }
  return true;
}

}  // namespace

class CPDF_Object;

class CPDF_PageOrganizer {
 public:
  CPDF_PageOrganizer(CPDF_Document* pDestPDFDoc, CPDF_Document* pSrcPDFDoc);
  ~CPDF_PageOrganizer();

  bool PDFDocInit();
  bool ExportPage(const std::vector<uint32_t>& pageNums, int nIndex);
  bool ExportNPagesToOne(const std::vector<uint32_t>& pageNums,
                         size_t nIndex,
                         size_t numPagesPerSheet);
  void AddSubPage(CPDF_Dictionary* pPageDict,
                  CFX_PointF position,
                  float xScale,
                  float yScale,
                  bool rotate,
                  bool isPageLandscape,
                  ByteString& content,
                  const CFX_FloatRect* clipRect);
  CPDF_Object* MakeXObject(CPDF_Dictionary* pSrcPageDict,
                           CPDF_Document* pDestDoc);
  void FinishPage(CPDF_Dictionary* pCurPageDict, const ByteString& content);

 private:
  using ObjectNumberMap = std::map<uint32_t, uint32_t>;

  bool UpdateReference(CPDF_Object* pObj, ObjectNumberMap* pObjNumberMap);
  uint32_t GetNewObjId(ObjectNumberMap* pObjNumberMap, CPDF_Reference* pRef);

  UnownedPtr<CPDF_Document> m_pDestPDFDoc;
  UnownedPtr<CPDF_Document> m_pSrcPDFDoc;
  uint32_t xobjectNum = 0;
  CFX_SizeF pageSize;
  std::map<ByteString, UnownedPtr<CPDF_Object>> xobjs;
};

CPDF_PageOrganizer::CPDF_PageOrganizer(CPDF_Document* pDestPDFDoc,
                                       CPDF_Document* pSrcPDFDoc)
    : m_pDestPDFDoc(pDestPDFDoc), m_pSrcPDFDoc(pSrcPDFDoc) {}

CPDF_PageOrganizer::~CPDF_PageOrganizer() {}

bool CPDF_PageOrganizer::PDFDocInit() {
  ASSERT(m_pDestPDFDoc);
  ASSERT(m_pSrcPDFDoc);

  CPDF_Dictionary* pNewRoot = m_pDestPDFDoc->GetRoot();
  if (!pNewRoot)
    return false;

  CPDF_Dictionary* pDocInfoDict = m_pDestPDFDoc->GetInfo();
  if (!pDocInfoDict)
    return false;

  CPDF_Dictionary* pSrcDict = m_pSrcPDFDoc->GetPage(0);
  if (!pSrcDict)
    return false;

  // TODO(xlou): calculate the maximum page size.
  CPDF_Page page(m_pSrcPDFDoc.Get(), pSrcDict, true);
  pageSize.width = page.GetPageWidth();
  pageSize.height = page.GetPageHeight();

  pDocInfoDict->SetNewFor<CPDF_String>("Producer", "PDFium", false);

  ByteString cbRootType = pNewRoot->GetStringFor("Type", "");
  if (cbRootType.IsEmpty())
    pNewRoot->SetNewFor<CPDF_Name>("Type", "Catalog");

  CPDF_Object* pElement = pNewRoot->GetObjectFor("Pages");
  CPDF_Dictionary* pNewPages =
      pElement ? ToDictionary(pElement->GetDirect()) : nullptr;
  if (!pNewPages) {
    pNewPages = m_pDestPDFDoc->NewIndirect<CPDF_Dictionary>();
    pNewRoot->SetNewFor<CPDF_Reference>("Pages", m_pDestPDFDoc.Get(),
                                        pNewPages->GetObjNum());
  }

  ByteString cbPageType = pNewPages->GetStringFor("Type", "");
  if (cbPageType.IsEmpty())
    pNewPages->SetNewFor<CPDF_Name>("Type", "Pages");

  if (!pNewPages->GetArrayFor("Kids")) {
    pNewPages->SetNewFor<CPDF_Number>("Count", 0);
    pNewPages->SetNewFor<CPDF_Reference>(
        "Kids", m_pDestPDFDoc.Get(),
        m_pDestPDFDoc->NewIndirect<CPDF_Array>()->GetObjNum());
  }

  return true;
}

void CPDF_PageOrganizer::AddSubPage(CPDF_Dictionary* pPageDict,
                                    CFX_PointF position,
                                    float xScale,
                                    float yScale,
                                    bool rotate,
                                    bool isPageLandscape,
                                    ByteString& content,
                                    const CFX_FloatRect* clipRect) {
  ++xobjectNum;
  ByteString xobjectName = ByteString::Format("X%d", xobjectNum);

  CFX_Matrix matrix;
  CPDF_Object* pObj = PageDictGetInheritableTag(pPageDict, "Rotate");
  if (pObj)
    matrix.Rotate(pObj->GetInteger());

  matrix.Scale(xScale, yScale);
  matrix.Translate(position.x, position.y);
  if (isPageLandscape) {
    matrix.Rotate(FX_PI / 2);
    matrix.Translate(pageSize.width, 0.0);
  }
  xobjs[xobjectName] = MakeXObject(pPageDict, m_pDestPDFDoc.Get());
  content += ByteString::Format("q\n");
  content += ByteString::Format("%f %f %f %f %f %f cm\n", matrix.a, matrix.b,
                                matrix.c, matrix.d, matrix.e, matrix.f);
  content += ByteString::Format("/%s Do Q\n", xobjectName.c_str());
}

CPDF_Object* CPDF_PageOrganizer::MakeXObject(CPDF_Dictionary* pSrcPageDict,
                                             CPDF_Document* pDestDoc) {
  auto pObjNumberMap = pdfium::MakeUnique<ObjectNumberMap>();
  if (!pSrcPageDict)
    return nullptr;

  CPDF_Object* pSrcContentObj = GetPageContent(pSrcPageDict);
  CPDF_Stream* pNewXObject = pDestDoc->NewIndirect<CPDF_Stream>(
      nullptr, 0,
      pdfium::MakeUnique<CPDF_Dictionary>(pDestDoc->GetByteStringPool()));
  CPDF_Dictionary* pNewXObjectDict = pNewXObject->GetDict();
  const ByteString resourceString = "Resources";
  if (!CopyInheritable(pNewXObjectDict, pSrcPageDict, resourceString)) {
    // Use a default empty resources if it does not exist.
    pNewXObjectDict->SetNewFor<CPDF_Dictionary>(resourceString);
  }
  uint32_t dwSrcPageResourcesObj =
      pSrcPageDict->GetDictFor(resourceString)->GetObjNum();
  uint32_t dwNewXobjectResourcesObj =
      pNewXObjectDict->GetDictFor(resourceString)->GetObjNum();
  (*pObjNumberMap)[dwSrcPageResourcesObj] = dwNewXobjectResourcesObj;
  CPDF_Dictionary* pNewXORes = pNewXObjectDict->GetDictFor(resourceString);
  UpdateReference(pNewXORes, pObjNumberMap.get());
  pNewXObjectDict->SetNewFor<CPDF_Name>("Type", "XObject");
  pNewXObjectDict->SetNewFor<CPDF_Name>("Subtype", "Form");
  pNewXObjectDict->SetNewFor<CPDF_Number>("FormType", 1);
  CFX_FloatRect rcBBox = GetTrimBox(pSrcPageDict);
  pNewXObjectDict->SetRectFor("BBox", rcBBox);
  // TODO(xlou): add matrix field.
  CPDF_Stream* pStream;
  std::ostringstream textBuf;

  if (CPDF_Array* pSrcContentArray = ToArray(pSrcContentObj)) {
    ByteString srcContentStream;
    for (size_t i = 0; i < pSrcContentArray->GetCount(); i++) {
      pStream = pSrcContentArray->GetStreamAt(i);
      auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
      pAcc->LoadAllDataFiltered();
      ByteString sStream(pAcc->GetData(), pAcc->GetSize());
      srcContentStream += sStream;
      srcContentStream += "\n";
    }
    pNewXObject->SetDataAndRemoveFilter(srcContentStream.raw_str(),
                                        srcContentStream.GetLength());
  } else {
    pStream = pSrcContentObj->AsStream();
    auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
    pAcc->LoadAllDataFiltered();
    ByteString sStream(pAcc->GetData(), pAcc->GetSize());
    pNewXObject->SetDataAndRemoveFilter(sStream.raw_str(), sStream.GetLength());
  }

  return pNewXObject;
}

bool CPDF_PageOrganizer::ExportPage(const std::vector<uint32_t>& pageNums,
                                    int nIndex) {
  int curpage = nIndex;
  auto pObjNumberMap = pdfium::MakeUnique<ObjectNumberMap>();
  for (size_t i = 0; i < pageNums.size(); ++i) {
    CPDF_Dictionary* pCurPageDict = m_pDestPDFDoc->CreateNewPage(curpage);
    CPDF_Dictionary* pSrcPageDict = m_pSrcPDFDoc->GetPage(pageNums[i] - 1);
    if (!pSrcPageDict || !pCurPageDict)
      return false;

    // Clone the page dictionary
    for (const auto& it : *pSrcPageDict) {
      const ByteString& cbSrcKeyStr = it.first;
      if (cbSrcKeyStr == "Type" || cbSrcKeyStr == "Parent")
        continue;

      CPDF_Object* pObj = it.second.get();
      pCurPageDict->SetFor(cbSrcKeyStr, pObj->Clone());
    }

    // inheritable item
    // Even though some entries are required by the PDF spec, there exist
    // PDFs that omit them. Set some defaults in this case.
    // 1 MediaBox - required
    if (!CopyInheritable(pCurPageDict, pSrcPageDict, "MediaBox")) {
      // Search for "CropBox" in the source page dictionary.
      // If it does not exist, use the default letter size.
      CPDF_Object* pInheritable =
          PageDictGetInheritableTag(pSrcPageDict, "CropBox");
      if (pInheritable) {
        pCurPageDict->SetFor("MediaBox", pInheritable->Clone());
      } else {
        // Make the default size letter size (8.5"x11")
        CPDF_Array* pArray = pCurPageDict->SetNewFor<CPDF_Array>("MediaBox");
        pArray->AddNew<CPDF_Number>(0);
        pArray->AddNew<CPDF_Number>(0);
        pArray->AddNew<CPDF_Number>(612);
        pArray->AddNew<CPDF_Number>(792);
      }
    }

    // 2 Resources - required
    if (!CopyInheritable(pCurPageDict, pSrcPageDict, "Resources")) {
      // Use a default empty resources if it does not exist.
      pCurPageDict->SetNewFor<CPDF_Dictionary>("Resources");
    }

    // 3 CropBox - optional
    CopyInheritable(pCurPageDict, pSrcPageDict, "CropBox");
    // 4 Rotate - optional
    CopyInheritable(pCurPageDict, pSrcPageDict, "Rotate");

    // Update the reference
    uint32_t dwOldPageObj = pSrcPageDict->GetObjNum();
    uint32_t dwNewPageObj = pCurPageDict->GetObjNum();
    (*pObjNumberMap)[dwOldPageObj] = dwNewPageObj;
    UpdateReference(pCurPageDict, pObjNumberMap.get());
    ++curpage;
  }

  return true;
}

void CPDF_PageOrganizer::FinishPage(CPDF_Dictionary* pCurPageDict,
                                    const ByteString& content) {
  if (!pCurPageDict)
    return;

  CPDF_Dictionary* pRes = pCurPageDict->GetDictFor("Resources");
  if (!pRes)
    pRes = pCurPageDict->SetNewFor<CPDF_Dictionary>("Resources");

  CPDF_Dictionary* pPageXObject = pRes->GetDictFor("XObject");
  if (!pPageXObject)
    pPageXObject = pRes->SetNewFor<CPDF_Dictionary>("XObject");

  for (auto& it : xobjs) {
    CPDF_Object* pObj = it.second.Get();
    pPageXObject->SetNewFor<CPDF_Reference>(it.first, m_pDestPDFDoc.Get(),
                                            pObj->GetObjNum());
  }

  auto pDict = pdfium::MakeUnique<CPDF_Dictionary>(
      m_pDestPDFDoc.Get()->GetByteStringPool());
  CPDF_Stream* pStream = m_pDestPDFDoc.Get()->NewIndirect<CPDF_Stream>(
      nullptr, 0, std::move(pDict));
  std::ostringstream textBuf;
  textBuf << ByteString::Format("%s", content.c_str());
  pStream->SetData(&textBuf);
  pCurPageDict->SetNewFor<CPDF_Reference>("Contents", m_pDestPDFDoc.Get(),
                                          pStream->GetObjNum());
}

bool CPDF_PageOrganizer::ExportNPagesToOne(
    const std::vector<uint32_t>& pageNums,
    size_t nIndex,
    size_t numPagesPerSheet) {
  ByteString content;
  size_t inPages = 0;
  CPDF_Dictionary* pCurPageDict = nullptr;

  if (!NupState::IsPossible(numPagesPerSheet))
    return false;

  if (numPagesPerSheet == 1)
    return ExportPage(pageNums, nIndex);

  NupState nupState(numPagesPerSheet);
  nupState.setWidth(pageSize.width);
  nupState.setHeight(pageSize.height);
  bool rotate = false;
  if (nupState.isLandscape()) {
    nupState.setWidth(pageSize.height);
    nupState.setHeight(pageSize.width);
    rotate = true;
  }

  size_t curpage = nIndex;
  NupPageSettings pgEdit;
  for (size_t i = 0; i < pageNums.size(); ++i) {
    // If number of pages on a page = nup, create a new page.
    bool newPage =
        nupState.CreateNewPage(pageSize.width, pageSize.height, pgEdit);
    if (newPage) {
      if (i > 0) {
        // Finish up the current page, and create a new page.
        FinishPage(pCurPageDict, content);
        // Create a new page
        ++curpage;
        content.clear();
        xobjs.clear();
        inPages = 0;
      }
      pCurPageDict = m_pDestPDFDoc->CreateNewPage(curpage);
    }

    CPDF_Dictionary* pSrcPageDict = m_pSrcPDFDoc->GetPage(pageNums[i] - 1);

    if (!pSrcPageDict || !pCurPageDict)
      return false;

    AddSubPage(pSrcPageDict, CFX_PointF(pgEdit.point.x, pgEdit.point.y),
               pgEdit.xScale, pgEdit.yScale, rotate, nupState.isLandscape(),
               content, nullptr);
    ++inPages;
  }

  // Finish the last page.
  FinishPage(pCurPageDict, content);
  return true;
}

bool CPDF_PageOrganizer::UpdateReference(CPDF_Object* pObj,
                                         ObjectNumberMap* pObjNumberMap) {
  switch (pObj->GetType()) {
    case CPDF_Object::REFERENCE: {
      CPDF_Reference* pReference = pObj->AsReference();
      uint32_t newobjnum = GetNewObjId(pObjNumberMap, pReference);
      if (newobjnum == 0)
        return false;
      pReference->SetRef(m_pDestPDFDoc.Get(), newobjnum);
      break;
    }
    case CPDF_Object::DICTIONARY: {
      CPDF_Dictionary* pDict = pObj->AsDictionary();
      auto it = pDict->begin();
      while (it != pDict->end()) {
        const ByteString& key = it->first;
        CPDF_Object* pNextObj = it->second.get();
        ++it;
        if (key == "Parent" || key == "Prev" || key == "First")
          continue;
        if (!pNextObj)
          return false;
        if (!UpdateReference(pNextObj, pObjNumberMap))
          pDict->RemoveFor(key);
      }
      break;
    }
    case CPDF_Object::ARRAY: {
      CPDF_Array* pArray = pObj->AsArray();
      for (size_t i = 0; i < pArray->GetCount(); ++i) {
        CPDF_Object* pNextObj = pArray->GetObjectAt(i);
        if (!pNextObj)
          return false;
        if (!UpdateReference(pNextObj, pObjNumberMap))
          return false;
      }
      break;
    }
    case CPDF_Object::STREAM: {
      CPDF_Stream* pStream = pObj->AsStream();
      CPDF_Dictionary* pDict = pStream->GetDict();
      if (!pDict)
        return false;
      if (!UpdateReference(pDict, pObjNumberMap))
        return false;
      break;
    }
    default:
      break;
  }

  return true;
}

uint32_t CPDF_PageOrganizer::GetNewObjId(ObjectNumberMap* pObjNumberMap,
                                         CPDF_Reference* pRef) {
  if (!pRef)
    return 0;

  uint32_t dwObjnum = pRef->GetRefObjNum();
  uint32_t dwNewObjNum = 0;
  const auto it = pObjNumberMap->find(dwObjnum);
  if (it != pObjNumberMap->end())
    dwNewObjNum = it->second;
  if (dwNewObjNum)
    return dwNewObjNum;

  CPDF_Object* pDirect = pRef->GetDirect();
  if (!pDirect)
    return 0;

  std::unique_ptr<CPDF_Object> pClone = pDirect->Clone();
  if (CPDF_Dictionary* pDictClone = pClone->AsDictionary()) {
    if (pDictClone->KeyExist("Type")) {
      ByteString strType = pDictClone->GetStringFor("Type");
      if (!FXSYS_stricmp(strType.c_str(), "Pages"))
        return 4;
      if (!FXSYS_stricmp(strType.c_str(), "Page"))
        return 0;
    }
  }
  CPDF_Object* pUnownedClone =
      m_pDestPDFDoc->AddIndirectObject(std::move(pClone));
  dwNewObjNum = pUnownedClone->GetObjNum();
  (*pObjNumberMap)[dwObjnum] = dwNewObjNum;
  if (!UpdateReference(pUnownedClone, pObjNumberMap))
    return 0;

  return dwNewObjNum;
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV FPDF_ImportPages(FPDF_DOCUMENT dest_doc,
                                                     FPDF_DOCUMENT src_doc,
                                                     FPDF_BYTESTRING pagerange,
                                                     int index) {
  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest_doc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  std::vector<uint32_t> pageArray;
  uint32_t nCount = pSrcDoc->GetPageCount();
  if (pagerange) {
    if (!ParserPageRangeString(pagerange, &pageArray, nCount))
      return false;
  } else {
    for (uint32_t i = 1; i <= nCount; ++i) {
      pageArray.push_back(i);
    }
  }

  CPDF_PageOrganizer pageOrg(pDestDoc, pSrcDoc);
  return pageOrg.PDFDocInit() && pageOrg.ExportPage(pageArray, index);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_ImportNPagesToOne(FPDF_DOCUMENT dest_doc,
                       FPDF_DOCUMENT src_doc,
                       FPDF_BYTESTRING pagerange,
                       int index,
                       int num_pages_per_page) {
  CPDF_Document* pDestDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!dest_doc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  std::vector<uint32_t> pageArray;
  int nCount = pSrcDoc->GetPageCount();
  if (pagerange) {
    if (!ParserPageRangeString(pagerange, &pageArray, nCount))
      return false;
  } else {
    for (int i = 1; i <= nCount; ++i)
      pageArray.push_back(i);
  }

  CPDF_PageOrganizer pageOrg(pDestDoc, pSrcDoc);
  return pageOrg.PDFDocInit() &&
         pageOrg.ExportNPagesToOne(pageArray, index, num_pages_per_page);
}

FPDF_EXPORT FPDF_BOOL FPDF_CALLCONV
FPDF_CopyViewerPreferences(FPDF_DOCUMENT dest_doc, FPDF_DOCUMENT src_doc) {
  CPDF_Document* pDstDoc = CPDFDocumentFromFPDFDocument(dest_doc);
  if (!pDstDoc)
    return false;

  CPDF_Document* pSrcDoc = CPDFDocumentFromFPDFDocument(src_doc);
  if (!pSrcDoc)
    return false;

  CPDF_Dictionary* pSrcDict = pSrcDoc->GetRoot();
  pSrcDict = pSrcDict->GetDictFor("ViewerPreferences");
  if (!pSrcDict)
    return false;

  CPDF_Dictionary* pDstDict = pDstDoc->GetRoot();
  if (!pDstDict)
    return false;

  pDstDict->SetFor("ViewerPreferences", pSrcDict->CloneDirectObject());
  return true;
}
