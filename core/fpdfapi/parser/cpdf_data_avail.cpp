// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/parser/cpdf_data_avail.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "core/fpdfapi/cpdf_modulemgr.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_cross_ref_avail.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_hint_tables.h"
#include "core/fpdfapi/parser/cpdf_linearized_header.h"
#include "core/fpdfapi/parser/cpdf_name.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fpdfapi/parser/cpdf_page_object_avail.h"
#include "core/fpdfapi/parser/cpdf_read_validator.h"
#include "core/fpdfapi/parser/cpdf_reference.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_safe_types.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

// static
CPDF_Object* GetResourceObject(CPDF_Dictionary* pDict) {
  constexpr size_t kMaxHierarchyDepth = 64;
  size_t depth = 0;

  CPDF_Dictionary* dictionary_to_check = pDict;
  while (dictionary_to_check) {
    CPDF_Object* result = dictionary_to_check->GetObjectFor("Resources");
    if (result)
      return result;
    const CPDF_Object* parent = dictionary_to_check->GetObjectFor("Parent");
    dictionary_to_check = parent ? parent->GetDict() : nullptr;

    if (++depth > kMaxHierarchyDepth) {
      // We have cycle in parents hierarchy.
      return nullptr;
    }
  }
  return nullptr;
}

class HintsScope {
 public:
  HintsScope(CPDF_ReadValidator* validator,
             CPDF_DataAvail::DownloadHints* hints)
      : validator_(validator) {
    ASSERT(validator_);
    validator_->SetDownloadHints(hints);
  }

  ~HintsScope() { validator_->SetDownloadHints(nullptr); }

 private:
  UnownedPtr<CPDF_ReadValidator> validator_;
};

}  // namespace

CPDF_DataAvail::FileAvail::~FileAvail() {}

CPDF_DataAvail::DownloadHints::~DownloadHints() {}

CPDF_DataAvail::CPDF_DataAvail(
    FileAvail* pFileAvail,
    const RetainPtr<IFX_SeekableReadStream>& pFileRead,
    bool bSupportHintTable)
    : m_pFileAvail(pFileAvail) {
  ASSERT(pFileRead);
  m_pFileRead = pdfium::MakeRetain<CPDF_ReadValidator>(pFileRead, m_pFileAvail);
  m_dwFileLen = m_pFileRead->GetSize();
  m_PagesObjNum = 0;
  m_dwAcroFormObjNum = 0;
  m_dwInfoObjNum = 0;
  m_pDocument = 0;
  m_dwEncryptObjNum = 0;
  m_bDocAvail = false;
  m_bMainXRefLoadTried = false;
  m_bDocAvail = false;
  m_bPagesLoad = false;
  m_bPagesTreeLoad = false;
  m_bMainXRefLoadedOK = false;
  m_bAnnotsLoad = false;
  m_bHaveAcroForm = false;
  m_bAcroFormLoad = false;
  m_bPageLoadedOK = false;
  m_bNeedDownLoadResource = false;
  m_bLinearizedFormParamLoad = false;
  m_pCurrentParser = nullptr;
  m_pPageDict = nullptr;
  m_pPageResource = nullptr;
  m_docStatus = PDF_DATAAVAIL_HEADER;
  m_bTotalLoadPageTree = false;
  m_bCurPageDictLoadOK = false;
  m_bLinearedDataOK = false;
  m_bSupportHintTable = bSupportHintTable;
  m_bHeaderAvail = false;
}

CPDF_DataAvail::~CPDF_DataAvail() {
  m_pHintTables.reset();
}

void CPDF_DataAvail::SetDocument(CPDF_Document* pDoc) {
  m_pDocument = pDoc;
}

bool CPDF_DataAvail::AreObjectsAvailable(std::vector<CPDF_Object*>& obj_array,
                                         bool bParsePage,
                                         std::vector<CPDF_Object*>& ret_array) {
  if (obj_array.empty())
    return true;

  uint32_t count = 0;
  std::vector<CPDF_Object*> new_obj_array;
  for (CPDF_Object* pObj : obj_array) {
    if (!pObj)
      continue;

    int32_t type = pObj->GetType();
    switch (type) {
      case CPDF_Object::ARRAY: {
        CPDF_Array* pArray = pObj->AsArray();
        for (size_t k = 0; k < pArray->GetCount(); ++k)
          new_obj_array.push_back(pArray->GetObjectAt(k));
        break;
      }
      case CPDF_Object::STREAM:
        pObj = pObj->GetDict();
      case CPDF_Object::DICTIONARY: {
        CPDF_Dictionary* pDict = pObj->GetDict();
        if (pDict && pDict->GetStringFor("Type") == "Page" && !bParsePage)
          continue;

        for (const auto& it : *pDict) {
          if (it.first != "Parent")
            new_obj_array.push_back(it.second.get());
        }
        break;
      }
      case CPDF_Object::REFERENCE: {
        const CPDF_ReadValidator::Session read_session(GetValidator().Get());

        CPDF_Reference* pRef = pObj->AsReference();
        const uint32_t dwNum = pRef->GetRefObjNum();

        if (pdfium::ContainsKey(m_ObjectSet, dwNum))
          break;

        CPDF_Object* pReferred = m_pDocument->GetOrParseIndirectObject(dwNum);
        if (GetValidator()->has_read_problems()) {
          ASSERT(!pReferred);
          ret_array.push_back(pObj);
          ++count;
          break;
        }
        m_ObjectSet.insert(dwNum);
        if (pReferred)
          new_obj_array.push_back(pReferred);

        break;
      }
    }
  }

  if (count > 0) {
    for (CPDF_Object* pObj : new_obj_array) {
      CPDF_Reference* pRef = pObj->AsReference();
      if (pRef && pdfium::ContainsKey(m_ObjectSet, pRef->GetRefObjNum()))
        continue;
      ret_array.push_back(pObj);
    }
    return false;
  }

  obj_array = new_obj_array;
  return AreObjectsAvailable(obj_array, false, ret_array);
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::IsDocAvail(
    DownloadHints* pHints) {
  if (!m_dwFileLen)
    return DataError;

  const HintsScope hints_scope(m_pFileRead.Get(), pHints);

  while (!m_bDocAvail) {
    if (!CheckDocStatus())
      return DataNotAvailable;
  }

  return DataAvailable;
}

bool CPDF_DataAvail::CheckAcroFormSubObject() {
  if (m_objs_array.empty()) {
    std::vector<CPDF_Object*> obj_array(m_Acroforms.size());
    std::transform(
        m_Acroforms.begin(), m_Acroforms.end(), obj_array.begin(),
        [](const std::unique_ptr<CPDF_Object>& pObj) { return pObj.get(); });

    m_ObjectSet.clear();
    if (!AreObjectsAvailable(obj_array, false, m_objs_array))
      return false;

    m_objs_array.clear();
    return true;
  }

  std::vector<CPDF_Object*> new_objs_array;
  if (!AreObjectsAvailable(m_objs_array, false, new_objs_array)) {
    m_objs_array = new_objs_array;
    return false;
  }

  m_Acroforms.clear();
  return true;
}

bool CPDF_DataAvail::CheckAcroForm() {
  bool bExist = false;
  std::unique_ptr<CPDF_Object> pAcroForm =
      GetObject(m_dwAcroFormObjNum, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_PAGETREE;
    return true;
  }

  if (!pAcroForm) {
    if (m_docStatus != PDF_DATAAVAIL_ERROR)
      return false;

    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return true;
  }

  m_Acroforms.push_back(std::move(pAcroForm));
  m_docStatus = PDF_DATAAVAIL_PAGETREE;
  return true;
}

bool CPDF_DataAvail::CheckDocStatus() {
  switch (m_docStatus) {
    case PDF_DATAAVAIL_HEADER:
      return CheckHeader();
    case PDF_DATAAVAIL_FIRSTPAGE:
      return CheckFirstPage();
    case PDF_DATAAVAIL_HINTTABLE:
      return CheckHintTables();
    case PDF_DATAAVAIL_LOADALLCROSSREF:
      return CheckAndLoadAllXref();
    case PDF_DATAAVAIL_LOADALLFILE:
      return LoadAllFile();
    case PDF_DATAAVAIL_ROOT:
      return CheckRoot();
    case PDF_DATAAVAIL_INFO:
      return CheckInfo();
    case PDF_DATAAVAIL_ACROFORM:
      return CheckAcroForm();
    case PDF_DATAAVAIL_PAGETREE:
      if (m_bTotalLoadPageTree)
        return CheckPages();
      return LoadDocPages();
    case PDF_DATAAVAIL_PAGE:
      if (m_bTotalLoadPageTree)
        return CheckPage();
      m_docStatus = PDF_DATAAVAIL_PAGE_LATERLOAD;
      return true;
    case PDF_DATAAVAIL_ERROR:
      return LoadAllFile();
    case PDF_DATAAVAIL_PAGE_LATERLOAD:
      m_docStatus = PDF_DATAAVAIL_PAGE;
    default:
      m_bDocAvail = true;
      return true;
  }
}

bool CPDF_DataAvail::CheckPageStatus() {
  switch (m_docStatus) {
    case PDF_DATAAVAIL_PAGETREE:
      return CheckPages();
    case PDF_DATAAVAIL_PAGE:
      return CheckPage();
    case PDF_DATAAVAIL_ERROR:
      return LoadAllFile();
    default:
      m_bPagesTreeLoad = true;
      m_bPagesLoad = true;
      return true;
  }
}

bool CPDF_DataAvail::LoadAllFile() {
  if (GetValidator()->CheckWholeFileAndRequestIfUnavailable()) {
    m_docStatus = PDF_DATAAVAIL_DONE;
    return true;
  }
  return false;
}

bool CPDF_DataAvail::CheckAndLoadAllXref() {
  if (!m_pCrossRefAvail) {
    const CPDF_ReadValidator::Session read_session(GetValidator().Get());
    const FX_FILESIZE last_xref_offset = m_parser.ParseStartXRef();
    if (GetValidator()->has_read_problems())
      return false;

    if (last_xref_offset <= 0) {
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return false;
    }

    m_pCrossRefAvail = pdfium::MakeUnique<CPDF_CrossRefAvail>(GetSyntaxParser(),
                                                              last_xref_offset);
  }

  switch (m_pCrossRefAvail->CheckAvail()) {
    case DocAvailStatus::DataAvailable:
      break;
    case DocAvailStatus::DataNotAvailable:
      return false;
    case DocAvailStatus::DataError:
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return false;
    default:
      NOTREACHED();
      return false;
  }

  if (!m_parser.LoadAllCrossRefV4(m_pCrossRefAvail->last_crossref_offset()) &&
      !m_parser.LoadAllCrossRefV5(m_pCrossRefAvail->last_crossref_offset())) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return false;
  }

  m_dwRootObjNum = m_parser.GetRootObjNum();
  m_dwInfoObjNum = m_parser.GetInfoObjNum();
  m_pCurrentParser = &m_parser;
  m_docStatus = PDF_DATAAVAIL_ROOT;
  return true;
}

std::unique_ptr<CPDF_Object> CPDF_DataAvail::GetObject(uint32_t objnum,
                                                       bool* pExistInFile) {
  CPDF_Parser* pParser = nullptr;

  if (pExistInFile)
    *pExistInFile = true;

  pParser = m_pDocument ? m_pDocument->GetParser() : &m_parser;

  std::unique_ptr<CPDF_Object> pRet;
  if (pParser) {
    const CPDF_ReadValidator::Session read_session(GetValidator().Get());
    pRet = pParser->ParseIndirectObject(nullptr, objnum);
    if (GetValidator()->has_read_problems())
      return nullptr;
  }

  if (!pRet && pExistInFile)
    *pExistInFile = false;

  return pRet;
}

bool CPDF_DataAvail::CheckInfo() {
  bool bExist = false;
  std::unique_ptr<CPDF_Object> pInfo = GetObject(m_dwInfoObjNum, &bExist);
  if (bExist && !pInfo) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return true;
    }
    return false;
  }
  m_docStatus =
      m_bHaveAcroForm ? PDF_DATAAVAIL_ACROFORM : PDF_DATAAVAIL_PAGETREE;
  return true;
}

bool CPDF_DataAvail::CheckRoot() {
  bool bExist = false;
  m_pRoot = GetObject(m_dwRootObjNum, &bExist);
  if (!bExist) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return true;
  }

  if (!m_pRoot) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return true;
    }
    return false;
  }

  CPDF_Dictionary* pDict = m_pRoot->GetDict();
  if (!pDict) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  CPDF_Reference* pRef = ToReference(pDict->GetObjectFor("Pages"));
  if (!pRef) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  m_PagesObjNum = pRef->GetRefObjNum();
  CPDF_Reference* pAcroFormRef =
      ToReference(m_pRoot->GetDict()->GetObjectFor("AcroForm"));
  if (pAcroFormRef) {
    m_bHaveAcroForm = true;
    m_dwAcroFormObjNum = pAcroFormRef->GetRefObjNum();
  }

  if (m_dwInfoObjNum) {
    m_docStatus = PDF_DATAAVAIL_INFO;
  } else {
    m_docStatus =
        m_bHaveAcroForm ? PDF_DATAAVAIL_ACROFORM : PDF_DATAAVAIL_PAGETREE;
  }
  return true;
}

bool CPDF_DataAvail::PreparePageItem() {
  const CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  CPDF_Reference* pRef =
      ToReference(pRoot ? pRoot->GetObjectFor("Pages") : nullptr);
  if (!pRef) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  m_PagesObjNum = pRef->GetRefObjNum();
  m_pCurrentParser = m_pDocument->GetParser();
  m_docStatus = PDF_DATAAVAIL_PAGETREE;
  return true;
}

bool CPDF_DataAvail::IsFirstCheck(uint32_t dwPage) {
  return m_pageMapCheckState.insert(dwPage).second;
}

void CPDF_DataAvail::ResetFirstCheck(uint32_t dwPage) {
  m_pageMapCheckState.erase(dwPage);
}

bool CPDF_DataAvail::CheckPage() {
  std::vector<uint32_t> UnavailObjList;
  for (uint32_t dwPageObjNum : m_PageObjList) {
    bool bExists = false;
    std::unique_ptr<CPDF_Object> pObj = GetObject(dwPageObjNum, &bExists);
    if (!pObj) {
      if (bExists)
        UnavailObjList.push_back(dwPageObjNum);
      continue;
    }
    CPDF_Array* pArray = ToArray(pObj.get());
    if (pArray) {
      for (const auto& pArrayObj : *pArray) {
        if (CPDF_Reference* pRef = ToReference(pArrayObj.get()))
          UnavailObjList.push_back(pRef->GetRefObjNum());
      }
    }
    if (!pObj->IsDictionary())
      continue;

    ByteString type = pObj->GetDict()->GetStringFor("Type");
    if (type == "Pages") {
      m_PagesArray.push_back(std::move(pObj));
      continue;
    }
  }
  m_PageObjList.clear();
  if (!UnavailObjList.empty()) {
    m_PageObjList = std::move(UnavailObjList);
    return false;
  }
  size_t iPages = m_PagesArray.size();
  for (size_t i = 0; i < iPages; ++i) {
    std::unique_ptr<CPDF_Object> pPages = std::move(m_PagesArray[i]);
    if (pPages && !GetPageKids(m_pCurrentParser, pPages.get())) {
      m_PagesArray.clear();
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return false;
    }
  }
  m_PagesArray.clear();
  if (m_PageObjList.empty())
    m_docStatus = PDF_DATAAVAIL_DONE;

  return true;
}

bool CPDF_DataAvail::GetPageKids(CPDF_Parser* pParser, CPDF_Object* pPages) {
  if (!pParser) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  CPDF_Dictionary* pDict = pPages->GetDict();
  CPDF_Object* pKids = pDict ? pDict->GetObjectFor("Kids") : nullptr;
  if (!pKids)
    return true;

  switch (pKids->GetType()) {
    case CPDF_Object::REFERENCE:
      m_PageObjList.push_back(pKids->AsReference()->GetRefObjNum());
      break;
    case CPDF_Object::ARRAY: {
      CPDF_Array* pKidsArray = pKids->AsArray();
      for (size_t i = 0; i < pKidsArray->GetCount(); ++i) {
        if (CPDF_Reference* pRef = ToReference(pKidsArray->GetObjectAt(i)))
          m_PageObjList.push_back(pRef->GetRefObjNum());
      }
      break;
    }
    default:
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return false;
  }
  return true;
}

bool CPDF_DataAvail::CheckPages() {
  bool bExists = false;
  std::unique_ptr<CPDF_Object> pPages = GetObject(m_PagesObjNum, &bExists);
  if (!bExists) {
    m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
    return true;
  }

  if (!pPages) {
    if (m_docStatus == PDF_DATAAVAIL_ERROR) {
      m_docStatus = PDF_DATAAVAIL_LOADALLFILE;
      return true;
    }
    return false;
  }

  if (!GetPageKids(m_pCurrentParser, pPages.get())) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  m_docStatus = PDF_DATAAVAIL_PAGE;
  return true;
}

bool CPDF_DataAvail::CheckHeader() {
  switch (CheckHeaderAndLinearized()) {
    case DocAvailStatus::DataAvailable:
      m_docStatus = m_pLinearized ? PDF_DATAAVAIL_FIRSTPAGE
                                  : PDF_DATAAVAIL_LOADALLCROSSREF;
      return true;
    case DocAvailStatus::DataNotAvailable:
      return false;
    case DocAvailStatus::DataError:
      m_docStatus = PDF_DATAAVAIL_ERROR;
      return true;
    default:
      NOTREACHED();
      return false;
  }
}

bool CPDF_DataAvail::CheckFirstPage() {
  if (!m_pLinearized->GetFirstPageEndOffset() ||
      !m_pLinearized->GetFileSize() ||
      !m_pLinearized->GetMainXRefTableFirstEntryOffset()) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  uint32_t dwEnd = m_pLinearized->GetFirstPageEndOffset();
  dwEnd += 512;
  if ((FX_FILESIZE)dwEnd > m_dwFileLen)
    dwEnd = (uint32_t)m_dwFileLen;

  const FX_FILESIZE start_pos = m_dwFileLen > 1024 ? 1024 : m_dwFileLen;
  const size_t data_size = dwEnd > 1024 ? static_cast<size_t>(dwEnd - 1024) : 0;
  if (!GetValidator()->CheckDataRangeAndRequestIfUnavailable(start_pos,
                                                             data_size))
    return false;

  m_docStatus =
      m_bSupportHintTable ? PDF_DATAAVAIL_HINTTABLE : PDF_DATAAVAIL_DONE;
  return true;
}

bool CPDF_DataAvail::CheckHintTables() {
  if (m_pLinearized->GetPageCount() <= 1) {
    m_docStatus = PDF_DATAAVAIL_DONE;
    return true;
  }
  if (!m_pLinearized->HasHintTable()) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  const FX_FILESIZE szHintStart = m_pLinearized->GetHintStart();
  const uint32_t szHintLength = m_pLinearized->GetHintLength();

  if (!GetValidator()->CheckDataRangeAndRequestIfUnavailable(szHintStart,
                                                             szHintLength))
    return false;

  auto pHintTables = pdfium::MakeUnique<CPDF_HintTables>(GetValidator().Get(),
                                                         m_pLinearized.get());
  std::unique_ptr<CPDF_Object> pHintStream =
      ParseIndirectObjectAt(szHintStart, 0);
  CPDF_Stream* pStream = ToStream(pHintStream.get());
  if (pStream && pHintTables->LoadHintStream(pStream))
    m_pHintTables = std::move(pHintTables);

  m_docStatus = PDF_DATAAVAIL_DONE;
  return true;
}

std::unique_ptr<CPDF_Object> CPDF_DataAvail::ParseIndirectObjectAt(
    FX_FILESIZE pos,
    uint32_t objnum,
    CPDF_IndirectObjectHolder* pObjList) {
  const FX_FILESIZE SavedPos = GetSyntaxParser()->GetPos();
  GetSyntaxParser()->SetPos(pos);
  std::unique_ptr<CPDF_Object> result = GetSyntaxParser()->GetIndirectObject(
      pObjList, CPDF_SyntaxParser::ParseType::kLoose);
  GetSyntaxParser()->SetPos(SavedPos);
  return (result && (!objnum || result->GetObjNum() == objnum))
             ? std::move(result)
             : nullptr;
}

CPDF_DataAvail::DocLinearizationStatus CPDF_DataAvail::IsLinearizedPDF() {
  switch (CheckHeaderAndLinearized()) {
    case DocAvailStatus::DataAvailable:
      return m_pLinearized ? DocLinearizationStatus::Linearized
                           : DocLinearizationStatus::NotLinearized;
    case DocAvailStatus::DataNotAvailable:
      return DocLinearizationStatus::LinearizationUnknown;
    case DocAvailStatus::DataError:
      return DocLinearizationStatus::NotLinearized;
    default:
      NOTREACHED();
      return DocLinearizationStatus::LinearizationUnknown;
  }
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckHeaderAndLinearized() {
  if (m_bHeaderAvail)
    return DocAvailStatus::DataAvailable;

  const CPDF_ReadValidator::Session read_session(GetValidator().Get());
  const int32_t header_offset = GetHeaderOffset(GetValidator());
  if (GetValidator()->has_read_problems())
    return DocAvailStatus::DataNotAvailable;

  if (header_offset == kInvalidHeaderOffset)
    return DocAvailStatus::DataError;

  m_parser.m_pSyntax->InitParserWithValidator(GetValidator(), header_offset);
  m_pLinearized = m_parser.ParseLinearizedHeader();
  if (GetValidator()->has_read_problems())
    return DocAvailStatus::DataNotAvailable;

  m_bHeaderAvail = true;
  return DocAvailStatus::DataAvailable;
}

bool CPDF_DataAvail::CheckPage(uint32_t dwPage) {
  while (true) {
    switch (m_docStatus) {
      case PDF_DATAAVAIL_PAGETREE:
        if (!LoadDocPages())
          return false;
        break;
      case PDF_DATAAVAIL_PAGE:
        if (!LoadDocPage(dwPage))
          return false;
        break;
      case PDF_DATAAVAIL_ERROR:
        return LoadAllFile();
      default:
        m_bPagesTreeLoad = true;
        m_bPagesLoad = true;
        m_bCurPageDictLoadOK = true;
        m_docStatus = PDF_DATAAVAIL_PAGE;
        return true;
    }
  }
}

bool CPDF_DataAvail::CheckArrayPageNode(uint32_t dwPageNo,
                                        PageNode* pPageNode) {
  bool bExists = false;
  std::unique_ptr<CPDF_Object> pPages = GetObject(dwPageNo, &bExists);
  if (!bExists) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  if (!pPages)
    return false;

  CPDF_Array* pArray = pPages->AsArray();
  if (!pArray) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  pPageNode->m_type = PDF_PAGENODE_PAGES;
  for (size_t i = 0; i < pArray->GetCount(); ++i) {
    CPDF_Reference* pKid = ToReference(pArray->GetObjectAt(i));
    if (!pKid)
      continue;

    auto pNode = pdfium::MakeUnique<PageNode>();
    pNode->m_dwPageNo = pKid->GetRefObjNum();
    pPageNode->m_ChildNodes.push_back(std::move(pNode));
  }
  return true;
}

bool CPDF_DataAvail::CheckUnknownPageNode(uint32_t dwPageNo,
                                          PageNode* pPageNode) {
  bool bExists = false;
  std::unique_ptr<CPDF_Object> pPage = GetObject(dwPageNo, &bExists);
  if (!bExists) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  if (!pPage)
    return false;

  if (pPage->IsArray()) {
    pPageNode->m_dwPageNo = dwPageNo;
    pPageNode->m_type = PDF_PAGENODE_ARRAY;
    return true;
  }

  if (!pPage->IsDictionary()) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  pPageNode->m_dwPageNo = dwPageNo;
  CPDF_Dictionary* pDict = pPage->GetDict();
  const ByteString type = pDict->GetStringFor("Type");
  if (type == "Page") {
    pPageNode->m_type = PDF_PAGENODE_PAGE;
    return true;
  }

  if (type != "Pages") {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }

  pPageNode->m_type = PDF_PAGENODE_PAGES;
  CPDF_Object* pKids = pDict->GetObjectFor("Kids");
  if (!pKids) {
    m_docStatus = PDF_DATAAVAIL_PAGE;
    return true;
  }

  switch (pKids->GetType()) {
    case CPDF_Object::REFERENCE: {
      CPDF_Reference* pKid = pKids->AsReference();
      auto pNode = pdfium::MakeUnique<PageNode>();
      pNode->m_dwPageNo = pKid->GetRefObjNum();
      pPageNode->m_ChildNodes.push_back(std::move(pNode));
      break;
    }
    case CPDF_Object::ARRAY: {
      CPDF_Array* pKidsArray = pKids->AsArray();
      for (size_t i = 0; i < pKidsArray->GetCount(); ++i) {
        CPDF_Reference* pKid = ToReference(pKidsArray->GetObjectAt(i));
        if (!pKid)
          continue;

        auto pNode = pdfium::MakeUnique<PageNode>();
        pNode->m_dwPageNo = pKid->GetRefObjNum();
        pPageNode->m_ChildNodes.push_back(std::move(pNode));
      }
      break;
    }
    default:
      break;
  }
  return true;
}

bool CPDF_DataAvail::CheckPageNode(const CPDF_DataAvail::PageNode& pageNode,
                                   int32_t iPage,
                                   int32_t& iCount,
                                   int level) {
  if (level >= kMaxPageRecursionDepth)
    return false;

  int32_t iSize = pdfium::CollectionSize<int32_t>(pageNode.m_ChildNodes);
  if (iSize <= 0 || iPage >= iSize) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }
  for (int32_t i = 0; i < iSize; ++i) {
    PageNode* pNode = pageNode.m_ChildNodes[i].get();
    if (!pNode)
      continue;

    if (pNode->m_type == PDF_PAGENODE_UNKNOWN) {
      // Updates the type for the unknown page node.
      if (!CheckUnknownPageNode(pNode->m_dwPageNo, pNode))
        return false;
    }
    if (pNode->m_type == PDF_PAGENODE_ARRAY) {
      // Updates a more specific type for the array page node.
      if (!CheckArrayPageNode(pNode->m_dwPageNo, pNode))
        return false;
    }
    switch (pNode->m_type) {
      case PDF_PAGENODE_PAGE:
        iCount++;
        if (iPage == iCount && m_pDocument)
          m_pDocument->SetPageObjNum(iPage, pNode->m_dwPageNo);
        break;
      case PDF_PAGENODE_PAGES:
        if (!CheckPageNode(*pNode, iPage, iCount, level + 1))
          return false;
        break;
      case PDF_PAGENODE_UNKNOWN:
      case PDF_PAGENODE_ARRAY:
        // Already converted above, error if we get here.
        return false;
    }
    if (iPage == iCount) {
      m_docStatus = PDF_DATAAVAIL_DONE;
      return true;
    }
  }
  return true;
}

bool CPDF_DataAvail::LoadDocPage(uint32_t dwPage) {
  FX_SAFE_INT32 safePage = pdfium::base::checked_cast<int32_t>(dwPage);
  int32_t iPage = safePage.ValueOrDie();
  if (m_pDocument->GetPageCount() <= iPage ||
      m_pDocument->IsPageLoaded(iPage)) {
    m_docStatus = PDF_DATAAVAIL_DONE;
    return true;
  }
  if (m_PageNode.m_type == PDF_PAGENODE_PAGE) {
    m_docStatus = iPage == 0 ? PDF_DATAAVAIL_DONE : PDF_DATAAVAIL_ERROR;
    return true;
  }
  int32_t iCount = -1;
  return CheckPageNode(m_PageNode, iPage, iCount, 0);
}

bool CPDF_DataAvail::CheckPageCount() {
  bool bExists = false;
  std::unique_ptr<CPDF_Object> pPages = GetObject(m_PagesObjNum, &bExists);
  if (!bExists) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }
  if (!pPages)
    return false;

  CPDF_Dictionary* pPagesDict = pPages->GetDict();
  if (!pPagesDict) {
    m_docStatus = PDF_DATAAVAIL_ERROR;
    return false;
  }
  if (!pPagesDict->KeyExist("Kids"))
    return true;

  return pPagesDict->GetIntegerFor("Count") > 0;
}

bool CPDF_DataAvail::LoadDocPages() {
  if (!CheckUnknownPageNode(m_PagesObjNum, &m_PageNode))
    return false;

  if (CheckPageCount()) {
    m_docStatus = PDF_DATAAVAIL_PAGE;
    return true;
  }

  m_bTotalLoadPageTree = true;
  return false;
}

bool CPDF_DataAvail::LoadPages() {
  while (!m_bPagesTreeLoad) {
    if (!CheckPageStatus())
      return false;
  }

  if (m_bPagesLoad)
    return true;

  m_pDocument->LoadPages();
  return false;
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedData() {
  if (m_bLinearedDataOK)
    return DataAvailable;
  ASSERT(m_pLinearized);
  if (!m_pLinearized->GetMainXRefTableFirstEntryOffset())
    return DataError;

  if (!m_bMainXRefLoadTried) {
    ASSERT(m_pDocument->GetParser()->GetTrailer());
    const FX_SAFE_FILESIZE main_xref_offset =
        m_pDocument->GetParser()->GetTrailer()->GetIntegerFor("Prev");
    if (!main_xref_offset.IsValid())
      return DataError;

    if (main_xref_offset.ValueOrDie() == 0)
      return DataAvailable;

    FX_SAFE_SIZE_T data_size = m_dwFileLen;
    data_size -= main_xref_offset.ValueOrDie();
    if (!data_size.IsValid())
      return DataError;

    if (!GetValidator()->CheckDataRangeAndRequestIfUnavailable(
            main_xref_offset.ValueOrDie(), data_size.ValueOrDie()))
      return DataNotAvailable;

    CPDF_Parser::Error eRet =
        m_pDocument->GetParser()->LoadLinearizedMainXRefTable();
    m_bMainXRefLoadTried = true;
    if (eRet != CPDF_Parser::SUCCESS)
      return DataError;

    if (!PreparePageItem())
      return DataNotAvailable;

    m_bMainXRefLoadedOK = true;
    m_bLinearedDataOK = true;
  }

  return m_bLinearedDataOK ? DataAvailable : DataNotAvailable;
}

bool CPDF_DataAvail::CheckPageAnnots(uint32_t dwPage) {
  if (m_objs_array.empty()) {
    m_ObjectSet.clear();

    FX_SAFE_INT32 safePage = pdfium::base::checked_cast<int32_t>(dwPage);
    CPDF_Dictionary* pPageDict = m_pDocument->GetPage(safePage.ValueOrDie());
    if (!pPageDict)
      return true;

    CPDF_Object* pAnnots = pPageDict->GetObjectFor("Annots");
    if (!pAnnots)
      return true;

    std::vector<CPDF_Object*> obj_array;
    obj_array.push_back(pAnnots);
    if (!AreObjectsAvailable(obj_array, false, m_objs_array))
      return false;

    m_objs_array.clear();
    return true;
  }

  std::vector<CPDF_Object*> new_objs_array;
  if (!AreObjectsAvailable(m_objs_array, false, new_objs_array)) {
    m_objs_array = new_objs_array;
    return false;
  }
  m_objs_array.clear();
  return true;
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::CheckLinearizedFirstPage(
    uint32_t dwPage) {
  if (!m_bAnnotsLoad) {
    if (!CheckPageAnnots(dwPage))
      return DataNotAvailable;
    m_bAnnotsLoad = true;
  }
  if (!ValidatePage(dwPage))
    return DataError;
  return DataAvailable;
}

CPDF_DataAvail::DocAvailStatus CPDF_DataAvail::IsPageAvail(
    uint32_t dwPage,
    DownloadHints* pHints) {
  if (!m_pDocument)
    return DataError;

  if (IsFirstCheck(dwPage)) {
    m_bCurPageDictLoadOK = false;
    m_bPageLoadedOK = false;
    m_bAnnotsLoad = false;
    m_bNeedDownLoadResource = false;
    m_objs_array.clear();
    m_ObjectSet.clear();
  }

  if (pdfium::ContainsKey(m_pagesLoadState, dwPage))
    return DataAvailable;

  const HintsScope hints_scope(GetValidator().Get(), pHints);

  if (m_pLinearized) {
    if (dwPage == m_pLinearized->GetFirstPageNo()) {
      DocAvailStatus nRet = CheckLinearizedFirstPage(dwPage);
      if (nRet == DataAvailable)
        m_pagesLoadState.insert(dwPage);
      return nRet;
    }

    DocAvailStatus nResult = CheckLinearizedData();
    if (nResult != DataAvailable)
      return nResult;

    if (m_pHintTables) {
      nResult = m_pHintTables->CheckPage(dwPage);
      if (nResult != DataAvailable)
        return nResult;
      if (GetPage(dwPage)) {
        m_pagesLoadState.insert(dwPage);
        return DataAvailable;
      }
    }

    if (!m_bMainXRefLoadedOK) {
      if (!LoadAllFile())
        return DataNotAvailable;
      m_pDocument->GetParser()->RebuildCrossRef();
      ResetFirstCheck(dwPage);
      return DataAvailable;
    }
    if (m_bTotalLoadPageTree) {
      if (!LoadPages())
        return DataNotAvailable;
    } else {
      if (!m_bCurPageDictLoadOK && !CheckPage(dwPage))
        return DataNotAvailable;
    }
  } else {
    if (!m_bTotalLoadPageTree && !m_bCurPageDictLoadOK && !CheckPage(dwPage)) {
      return DataNotAvailable;
    }
  }

  if (m_bHaveAcroForm && !m_bAcroFormLoad) {
    if (!CheckAcroFormSubObject())
      return DataNotAvailable;
    m_bAcroFormLoad = true;
  }

  if (!m_bPageLoadedOK) {
    if (m_objs_array.empty()) {
      m_ObjectSet.clear();

      FX_SAFE_INT32 safePage = pdfium::base::checked_cast<int32_t>(dwPage);
      m_pPageDict = m_pDocument->GetPage(safePage.ValueOrDie());
      if (!m_pPageDict) {
        ResetFirstCheck(dwPage);
        // This is XFA page.
        return DataAvailable;
      }

      std::vector<CPDF_Object*> obj_array;
      obj_array.push_back(m_pPageDict);
      if (!AreObjectsAvailable(obj_array, true, m_objs_array))
        return DataNotAvailable;

      m_objs_array.clear();
    } else {
      std::vector<CPDF_Object*> new_objs_array;
      if (!AreObjectsAvailable(m_objs_array, false, new_objs_array)) {
        m_objs_array = new_objs_array;
        return DataNotAvailable;
      }
    }
    m_objs_array.clear();
    m_bPageLoadedOK = true;
  }

  if (!m_bAnnotsLoad) {
    if (!CheckPageAnnots(dwPage))
      return DataNotAvailable;
    m_bAnnotsLoad = true;
  }

  if (m_pPageDict && !m_bNeedDownLoadResource) {
    m_pPageResource = GetResourceObject(m_pPageDict);
    m_bNeedDownLoadResource = !!m_pPageResource;
  }

  if (m_bNeedDownLoadResource) {
    if (!CheckResources())
      return DataNotAvailable;
    m_bNeedDownLoadResource = false;
  }

  m_bPageLoadedOK = false;
  m_bAnnotsLoad = false;
  m_bCurPageDictLoadOK = false;

  ResetFirstCheck(dwPage);
  m_pagesLoadState.insert(dwPage);
  if (!ValidatePage(dwPage))
    return DataError;
  return DataAvailable;
}

bool CPDF_DataAvail::CheckResources() {
  if (m_objs_array.empty()) {
    std::vector<CPDF_Object*> obj_array;
    obj_array.push_back(m_pPageResource);
    if (!AreObjectsAvailable(obj_array, true, m_objs_array))
      return false;

    m_objs_array.clear();
    return true;
  }
  std::vector<CPDF_Object*> new_objs_array;
  if (!AreObjectsAvailable(m_objs_array, false, new_objs_array)) {
    m_objs_array = new_objs_array;
    return false;
  }
  m_objs_array.clear();
  return true;
}

RetainPtr<IFX_SeekableReadStream> CPDF_DataAvail::GetFileRead() const {
  return m_pFileRead;
}

RetainPtr<CPDF_ReadValidator> CPDF_DataAvail::GetValidator() const {
  return m_pFileRead;
}

CPDF_SyntaxParser* CPDF_DataAvail::GetSyntaxParser() const {
  return m_pDocument ? m_pDocument->GetParser()->m_pSyntax.get()
                     : m_parser.m_pSyntax.get();
}

int CPDF_DataAvail::GetPageCount() const {
  if (m_pLinearized)
    return m_pLinearized->GetPageCount();
  return m_pDocument ? m_pDocument->GetPageCount() : 0;
}

CPDF_Dictionary* CPDF_DataAvail::GetPage(int index) {
  if (!m_pDocument || index < 0 || index >= GetPageCount())
    return nullptr;
  CPDF_Dictionary* page = m_pDocument->GetPage(index);
  if (page)
    return page;
  if (!m_pLinearized || !m_pHintTables)
    return nullptr;

  if (index == static_cast<int>(m_pLinearized->GetFirstPageNo()))
    return nullptr;
  FX_FILESIZE szPageStartPos = 0;
  FX_FILESIZE szPageLength = 0;
  uint32_t dwObjNum = 0;
  const bool bPagePosGot = m_pHintTables->GetPagePos(index, &szPageStartPos,
                                                     &szPageLength, &dwObjNum);
  if (!bPagePosGot || !dwObjNum)
    return nullptr;
  // We should say to the document, which object is the page.
  m_pDocument->SetPageObjNum(index, dwObjNum);
  // Page object already can be parsed in document.
  if (!m_pDocument->GetIndirectObject(dwObjNum)) {
    m_pDocument->ReplaceIndirectObjectIfHigherGeneration(
        dwObjNum, ParseIndirectObjectAt(szPageStartPos, dwObjNum, m_pDocument));
  }
  if (!ValidatePage(index))
    return nullptr;
  return m_pDocument->GetPage(index);
}

CPDF_DataAvail::DocFormStatus CPDF_DataAvail::IsFormAvail(
    DownloadHints* pHints) {
  if (!m_pDocument)
    return FormAvailable;

  const HintsScope hints_scope(GetValidator().Get(), pHints);
  if (m_pLinearized) {
    DocAvailStatus nDocStatus = CheckLinearizedData();
    if (nDocStatus == DataError)
      return FormError;
    if (nDocStatus == DataNotAvailable)
      return FormNotAvailable;
  }

  if (!m_bLinearizedFormParamLoad) {
    const CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
    if (!pRoot)
      return FormAvailable;

    CPDF_Object* pAcroForm = pRoot->GetObjectFor("AcroForm");
    if (!pAcroForm)
      return FormNotExist;

    m_objs_array.push_back(pAcroForm->GetDict());
    m_bLinearizedFormParamLoad = true;
  }

  std::vector<CPDF_Object*> new_objs_array;
  if (!AreObjectsAvailable(m_objs_array, false, new_objs_array)) {
    m_objs_array = new_objs_array;
    return FormNotAvailable;
  }

  m_objs_array.clear();
  if (!ValidateForm())
    return FormError;
  return FormAvailable;
}

bool CPDF_DataAvail::ValidatePage(uint32_t dwPage) {
  FX_SAFE_INT32 safePage = pdfium::base::checked_cast<int32_t>(dwPage);
  CPDF_Dictionary* pPageDict = m_pDocument->GetPage(safePage.ValueOrDie());
  if (!pPageDict)
    return false;
  CPDF_PageObjectAvail obj_avail(GetValidator().Get(), m_pDocument, pPageDict);
  return obj_avail.CheckAvail() == DocAvailStatus::DataAvailable;
}

bool CPDF_DataAvail::ValidateForm() {
  const CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  if (!pRoot)
    return true;
  CPDF_Object* pAcroForm = pRoot->GetObjectFor("AcroForm");
  if (!pAcroForm)
    return false;
  CPDF_PageObjectAvail obj_avail(GetValidator().Get(), m_pDocument, pAcroForm);
  return obj_avail.CheckAvail() == DocAvailStatus::DataAvailable;
}

CPDF_DataAvail::PageNode::PageNode() : m_type(PDF_PAGENODE_UNKNOWN) {}

CPDF_DataAvail::PageNode::~PageNode() {}
