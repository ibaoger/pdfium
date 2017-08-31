// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/cfx_folderfontinfo.h"

#include <limits>
#include <utility>

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_stream.h"
#include "core/fxge/cfx_fontmapper.h"
#include "core/fxge/fx_font.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const struct {
  const char* m_pName;
  const char* m_pSubstName;
} Base14Substs[] = {
    {"Courier", "Courier New"},
    {"Courier-Bold", "Courier New Bold"},
    {"Courier-BoldOblique", "Courier New Bold Italic"},
    {"Courier-Oblique", "Courier New Italic"},
    {"Helvetica", "Arial"},
    {"Helvetica-Bold", "Arial Bold"},
    {"Helvetica-BoldOblique", "Arial Bold Italic"},
    {"Helvetica-Oblique", "Arial Italic"},
    {"Times-Roman", "Times New Roman"},
    {"Times-Bold", "Times New Roman Bold"},
    {"Times-BoldItalic", "Times New Roman Bold Italic"},
    {"Times-Italic", "Times New Roman Italic"},
};

CFX_ByteString FPDF_ReadStringFromFile(FILE* pFile, uint32_t size) {
  CFX_ByteString buffer;
  if (!fread(buffer.GetBuffer(size), size, 1, pFile))
    return CFX_ByteString();
  buffer.ReleaseBuffer(size);
  return buffer;
}

CFX_ByteString FPDF_LoadTableFromTT(FILE* pFile,
                                    const uint8_t* pTables,
                                    uint32_t nTables,
                                    uint32_t tag) {
  for (uint32_t i = 0; i < nTables; i++) {
    const uint8_t* p = pTables + i * 16;
    if (GET_TT_LONG(p) == tag) {
      uint32_t offset = GET_TT_LONG(p + 8);
      uint32_t size = GET_TT_LONG(p + 12);
      fseek(pFile, offset, SEEK_SET);
      return FPDF_ReadStringFromFile(pFile, size);
    }
  }
  return CFX_ByteString();
}

uint32_t GetCharset(int charset) {
  switch (charset) {
    case FX_CHARSET_ShiftJIS:
      return CHARSET_FLAG_SHIFTJIS;
    case FX_CHARSET_ChineseSimplified:
      return CHARSET_FLAG_GB;
    case FX_CHARSET_ChineseTraditional:
      return CHARSET_FLAG_BIG5;
    case FX_CHARSET_Hangul:
      return CHARSET_FLAG_KOREAN;
    case FX_CHARSET_Symbol:
      return CHARSET_FLAG_SYMBOL;
    case FX_CHARSET_ANSI:
      return CHARSET_FLAG_ANSI;
    default:
      break;
  }
  return 0;
}

int32_t GetSimilarValue(int weight,
                        bool bItalic,
                        int pitch_family,
                        uint32_t style) {
  int32_t iSimilarValue = 0;
  if (!!(style & FXFONT_BOLD) == (weight > 400))
    iSimilarValue += 16;
  if (!!(style & FXFONT_ITALIC) == bItalic)
    iSimilarValue += 16;
  if (!!(style & FXFONT_SERIF) == !!(pitch_family & FXFONT_FF_ROMAN))
    iSimilarValue += 16;
  if (!!(style & FXFONT_SCRIPT) == !!(pitch_family & FXFONT_FF_SCRIPT))
    iSimilarValue += 8;
  if (!!(style & FXFONT_FIXED_PITCH) ==
      !!(pitch_family & FXFONT_FF_FIXEDPITCH)) {
    iSimilarValue += 8;
  }
  return iSimilarValue;
}

}  // namespace

CFX_FolderFontInfo::CFX_FolderFontInfo() {}

CFX_FolderFontInfo::~CFX_FolderFontInfo() {}

void CFX_FolderFontInfo::AddPath(const CFX_ByteString& path) {
  m_PathList.push_back(path);
}

bool CFX_FolderFontInfo::EnumFontList(CFX_FontMapper* pMapper) {
  m_pMapper = pMapper;
  for (const auto& path : m_PathList)
    ScanPath(path);
  return true;
}

void CFX_FolderFontInfo::ScanPath(const CFX_ByteString& path) {
  FX_FileHandle* handle = FX_OpenFolder(path.c_str());
  if (!handle)
    return;

  CFX_ByteString filename;
  bool bFolder;
  while (FX_GetNextFile(handle, &filename, &bFolder)) {
    if (bFolder) {
      if (filename == "." || filename == "..")
        continue;
    } else {
      CFX_ByteString ext = filename.Right(4);
      ext.MakeUpper();
      if (ext != ".TTF" && ext != ".OTF" && ext != ".TTC")
        continue;
    }

    CFX_ByteString fullpath = path;
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    fullpath += "\\";
#else
    fullpath += "/";
#endif

    fullpath += filename;
    bFolder ? ScanPath(fullpath) : ScanFile(fullpath);
  }
  FX_CloseFolder(handle);
}

void CFX_FolderFontInfo::ScanFile(const CFX_ByteString& path) {
  FILE* pFile = fopen(path.c_str(), "rb");
  if (!pFile)
    return;

  fseek(pFile, 0, SEEK_END);

  uint32_t filesize = ftell(pFile);
  uint8_t buffer[16];
  fseek(pFile, 0, SEEK_SET);

  size_t readCnt = fread(buffer, 12, 1, pFile);
  if (readCnt != 1) {
    fclose(pFile);
    return;
  }

  if (GET_TT_LONG(buffer) == kTableTTCF) {
    uint32_t nFaces = GET_TT_LONG(buffer + 8);
    if (nFaces > std::numeric_limits<uint32_t>::max() / 4) {
      fclose(pFile);
      return;
    }
    uint32_t face_bytes = nFaces * 4;
    uint8_t* offsets = FX_Alloc(uint8_t, face_bytes);
    readCnt = fread(offsets, 1, face_bytes, pFile);
    if (readCnt != face_bytes) {
      FX_Free(offsets);
      fclose(pFile);
      return;
    }
    for (uint32_t i = 0; i < nFaces; i++) {
      uint8_t* p = offsets + i * 4;
      ReportFace(path, pFile, filesize, GET_TT_LONG(p));
    }
    FX_Free(offsets);
  } else {
    ReportFace(path, pFile, filesize, 0);
  }
  fclose(pFile);
}

void CFX_FolderFontInfo::ReportFace(const CFX_ByteString& path,
                                    FILE* pFile,
                                    uint32_t filesize,
                                    uint32_t offset) {
  fseek(pFile, offset, SEEK_SET);
  char buffer[16];
  if (!fread(buffer, 12, 1, pFile))
    return;

  uint32_t nTables = GET_TT_SHORT(buffer + 4);
  CFX_ByteString tables = FPDF_ReadStringFromFile(pFile, nTables * 16);
  if (tables.IsEmpty())
    return;

  CFX_ByteString names =
      FPDF_LoadTableFromTT(pFile, tables.raw_str(), nTables, 0x6e616d65);
  if (names.IsEmpty())
    return;

  CFX_ByteString facename =
      GetNameFromTT(names.raw_str(), names.GetLength(), 1);
  if (facename.IsEmpty())
    return;

  CFX_ByteString style = GetNameFromTT(names.raw_str(), names.GetLength(), 2);
  if (style != "Regular")
    facename += " " + style;

  if (pdfium::ContainsKey(m_FontList, facename))
    return;

  auto pInfo = pdfium::MakeUnique<CFX_FontFaceInfo>(path, facename, tables,
                                                    offset, filesize);
  CFX_ByteString os2 =
      FPDF_LoadTableFromTT(pFile, tables.raw_str(), nTables, 0x4f532f32);
  if (os2.GetLength() >= 86) {
    const uint8_t* p = os2.raw_str() + 78;
    uint32_t codepages = GET_TT_LONG(p);
    if (codepages & (1 << 17)) {
      m_pMapper->AddInstalledFont(facename, FX_CHARSET_ShiftJIS);
      pInfo->m_Charsets |= CHARSET_FLAG_SHIFTJIS;
    }
    if (codepages & (1 << 18)) {
      m_pMapper->AddInstalledFont(facename, FX_CHARSET_ChineseSimplified);
      pInfo->m_Charsets |= CHARSET_FLAG_GB;
    }
    if (codepages & (1 << 20)) {
      m_pMapper->AddInstalledFont(facename, FX_CHARSET_ChineseTraditional);
      pInfo->m_Charsets |= CHARSET_FLAG_BIG5;
    }
    if ((codepages & (1 << 19)) || (codepages & (1 << 21))) {
      m_pMapper->AddInstalledFont(facename, FX_CHARSET_Hangul);
      pInfo->m_Charsets |= CHARSET_FLAG_KOREAN;
    }
    if (codepages & (1 << 31)) {
      m_pMapper->AddInstalledFont(facename, FX_CHARSET_Symbol);
      pInfo->m_Charsets |= CHARSET_FLAG_SYMBOL;
    }
  }
  m_pMapper->AddInstalledFont(facename, FX_CHARSET_ANSI);
  pInfo->m_Charsets |= CHARSET_FLAG_ANSI;
  pInfo->m_Styles = 0;
  if (style.Contains("Bold"))
    pInfo->m_Styles |= FXFONT_BOLD;
  if (style.Contains("Italic") || style.Contains("Oblique"))
    pInfo->m_Styles |= FXFONT_ITALIC;
  if (facename.Contains("Serif"))
    pInfo->m_Styles |= FXFONT_SERIF;

  m_FontList[facename] = std::move(pInfo);
}

void* CFX_FolderFontInfo::GetSubstFont(const CFX_ByteString& face) {
  for (size_t iBaseFont = 0; iBaseFont < FX_ArraySize(Base14Substs);
       iBaseFont++) {
    if (face == Base14Substs[iBaseFont].m_pName)
      return GetFont(Base14Substs[iBaseFont].m_pSubstName);
  }
  return nullptr;
}

void* CFX_FolderFontInfo::FindFont(int weight,
                                   bool bItalic,
                                   int charset,
                                   int pitch_family,
                                   const char* family,
                                   bool bMatchName) {
  CFX_FontFaceInfo* pFind = nullptr;
  if (charset == FX_CHARSET_ANSI && (pitch_family & FXFONT_FF_FIXEDPITCH))
    return GetFont("Courier New");

  uint32_t charset_flag = GetCharset(charset);
  int32_t iBestSimilar = 0;
  for (const auto& it : m_FontList) {
    const CFX_ByteString& bsName = it.first;
    CFX_FontFaceInfo* pFont = it.second.get();
    if (!(pFont->m_Charsets & charset_flag) && charset != FX_CHARSET_Default)
      continue;

    if (bMatchName && !bsName.Contains(family))
      continue;

    int32_t iSimilarValue =
        GetSimilarValue(weight, bItalic, pitch_family, pFont->m_Styles);
    if (iSimilarValue > iBestSimilar) {
      iBestSimilar = iSimilarValue;
      pFind = pFont;
    }
  }
  return pFind;
}

void* CFX_FolderFontInfo::MapFont(int weight,
                                  bool bItalic,
                                  int charset,
                                  int pitch_family,
                                  const char* family,
                                  int& iExact) {
  return nullptr;
}

#ifdef PDF_ENABLE_XFA
void* CFX_FolderFontInfo::MapFontByUnicode(uint32_t dwUnicode,
                                           int weight,
                                           bool bItalic,
                                           int pitch_family) {
  return nullptr;
}
#endif  // PDF_ENABLE_XFA

void* CFX_FolderFontInfo::GetFont(const char* face) {
  auto it = m_FontList.find(face);
  return it != m_FontList.end() ? it->second.get() : nullptr;
}

uint32_t CFX_FolderFontInfo::GetFontData(void* hFont,
                                         uint32_t table,
                                         uint8_t* buffer,
                                         uint32_t size) {
  if (!hFont)
    return 0;

  const CFX_FontFaceInfo* pFont = static_cast<CFX_FontFaceInfo*>(hFont);
  uint32_t datasize = 0;
  uint32_t offset = 0;
  if (table == 0) {
    datasize = pFont->m_FontOffset ? 0 : pFont->m_FileSize;
  } else if (table == kTableTTCF) {
    datasize = pFont->m_FontOffset ? pFont->m_FileSize : 0;
  } else {
    uint32_t nTables = pFont->m_FontTables.GetLength() / 16;
    for (uint32_t i = 0; i < nTables; i++) {
      const uint8_t* p = pFont->m_FontTables.raw_str() + i * 16;
      if (GET_TT_LONG(p) == table) {
        offset = GET_TT_LONG(p + 8);
        datasize = GET_TT_LONG(p + 12);
      }
    }
  }

  if (!datasize || size < datasize)
    return datasize;

  FILE* pFile = fopen(pFont->m_FilePath.c_str(), "rb");
  if (!pFile)
    return 0;

  if (fseek(pFile, offset, SEEK_SET) < 0 ||
      fread(buffer, datasize, 1, pFile) != 1) {
    datasize = 0;
  }
  fclose(pFile);
  return datasize;
}

void CFX_FolderFontInfo::DeleteFont(void* hFont) {}

bool CFX_FolderFontInfo::GetFaceName(void* hFont, CFX_ByteString* name) {
  if (!hFont)
    return false;
  *name = static_cast<CFX_FontFaceInfo*>(hFont)->m_FaceName;
  return true;
}

bool CFX_FolderFontInfo::GetFontCharset(void* hFont, int* charset) {
  return false;
}
