// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdftext/cpdf_linkextract.h"

#include <vector>

#include "core/fpdftext/cpdf_textpage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

namespace {

// Find the end of a web link starting from offset |start|.
// The purpose of this function is to separate url from the surrounding context
// characters, we do not intend to fully validate the url.
// |str| contains lower case characters only.
FX_STRSIZE FindWebLinkEnding(const CFX_WideString& str, FX_STRSIZE start) {
  FX_STRSIZE end = str.GetLength() - 1;
  if (str.Find(L'/', start) == -1) {
    // When there is no path, it only has host name, or with port.
    // According to RFC1123, host name only has alphanumeric chars, hyphens,
    // and periods. Hyphen should not at the end though.
    // Non-ASCII chars are ignored during checking.
    while (end > start && str[end] < 0x80) {
      if ((str[end] >= L'0' && str[end] <= L'9') ||
          (str[end] >= L'a' && str[end] <= L'z') || str[end] == L'.')
        break;
      end--;
    }
    return end;
  }

  // When there is a path and query after '/', most ASCII chars are allowed.
  // We don't sanitize in this case.
  return end;
}

}  // namespace

CPDF_LinkExtract::CPDF_LinkExtract(const CPDF_TextPage* pTextPage)
    : m_pTextPage(pTextPage) {}

CPDF_LinkExtract::~CPDF_LinkExtract() {}

void CPDF_LinkExtract::ExtractLinks() {
  m_LinkArray.clear();
  if (!m_pTextPage->IsParsed())
    return;

  m_strPageText = m_pTextPage->GetPageText(0, -1);
  if (m_strPageText.IsEmpty())
    return;

  ParseLink();
}

void CPDF_LinkExtract::ParseLink() {
  int start = 0;
  int pos = 0;
  int nTotalChar = m_pTextPage->CountChars();
  bool bAfterHyphen = false;
  bool bLineBreak = false;
  while (pos < nTotalChar) {
    FPDF_CHAR_INFO pageChar;
    m_pTextPage->GetCharInfo(pos, &pageChar);
    if (pageChar.m_Flag == FPDFTEXT_CHAR_GENERATED ||
        pageChar.m_Unicode == TEXT_SPACE_CHAR || pos == nTotalChar - 1) {
      int nCount = pos - start;
      if (pos == nTotalChar - 1) {
        nCount++;
      } else if (bAfterHyphen && (pageChar.m_Unicode == TEXT_LINEFEED_CHAR ||
                                  pageChar.m_Unicode == TEXT_RETURN_CHAR)) {
        // Handle text breaks with a hyphen to the next line.
        bLineBreak = true;
        pos++;
        continue;
      }
      CFX_WideString strBeCheck;
      strBeCheck = m_pTextPage->GetPageText(start, nCount);
      if (bLineBreak) {
        strBeCheck.Remove(TEXT_LINEFEED_CHAR);
        strBeCheck.Remove(TEXT_RETURN_CHAR);
        bLineBreak = false;
      }
      // Replace the generated code with the hyphen char.
      strBeCheck.Replace(L"\xfffe", TEXT_HYPHEN);

      if (strBeCheck.GetLength() > 5) {
        while (strBeCheck.GetLength() > 0) {
          wchar_t ch = strBeCheck.GetAt(strBeCheck.GetLength() - 1);
          if (ch == L')' || ch == L',' || ch == L'>' || ch == L'.') {
            strBeCheck = strBeCheck.Mid(0, strBeCheck.GetLength() - 1);
            nCount--;
          } else {
            break;
          }
        }
        // Check for potential web URLs and email addresses.
        // Ftp address, file system links, data, blob etc. are not checked.
        if (nCount > 5 &&
            (CheckWebLink(strBeCheck) || CheckMailLink(strBeCheck))) {
          m_LinkArray.push_back({start, nCount, strBeCheck});
        }
      }
      start = ++pos;
    } else {
      bAfterHyphen = (pageChar.m_Flag == FPDFTEXT_CHAR_HYPHEN ||
                      (pageChar.m_Flag == FPDFTEXT_CHAR_NORMAL &&
                       pageChar.m_Unicode == TEXT_HYPHEN_CHAR));
      pos++;
    }
  }
}

bool CPDF_LinkExtract::CheckWebLink(CFX_WideString& strBeCheck) {
  static const wchar_t kWebScheme[] = L"http";
  static const FX_STRSIZE kWebSchemeLen = FXSYS_len(kWebScheme);
  static const wchar_t kUrlStart[] = L"www.";
  static const FX_STRSIZE kUrlStartLen = FXSYS_len(kUrlStart);

  CFX_WideString str = strBeCheck;
  str.MakeLower();

  FX_STRSIZE len = str.GetLength();
  // First, try to find the scheme.
  FX_STRSIZE start = str.Find(kWebScheme);
  if (start != -1) {
    FX_STRSIZE off = start + kWebSchemeLen;  // move after "http".
    if (len > off + 4) {                     // At least "://<char>" follows.
      if (str[off] == L's')
        off++;
      if (str[off] == L':' && str[off + 1] == L'/' && str[off + 2] == L'/') {
        FX_STRSIZE end = FindWebLinkEnding(str, off + 3);
        if (end > off + 3) {  // Non-empty host name.
          strBeCheck = strBeCheck.Mid(start, end - start + 1);
          return true;
        }
      }
    }
  }

  // When there is no scheme, try to find url starting with "www.".
  start = str.Find(kUrlStart);
  if (start != -1 && len > start + kUrlStartLen) {
    FX_STRSIZE end = FindWebLinkEnding(str, start);
    if (end > start + kUrlStartLen) {
      strBeCheck = L"http://" + strBeCheck.Mid(start, end - start + 1);
      return true;
    }
  }
  return false;
}

bool CPDF_LinkExtract::CheckMailLink(CFX_WideString& str) {
  int aPos = str.Find(L'@');
  // Invalid when no '@'.
  if (aPos < 1)
    return false;

  // Check the local part.
  int pPos = aPos;  // Used to track the position of '@' or '.'.
  for (int i = aPos - 1; i >= 0; i--) {
    wchar_t ch = str.GetAt(i);
    if (ch == L'_' || ch == L'-' || FXSYS_iswalnum(ch))
      continue;

    if (ch != L'.' || i == pPos - 1 || i == 0) {
      if (i == aPos - 1) {
        // There is '.' or invalid char before '@'.
        return false;
      }
      // End extracting for other invalid chars, '.' at the beginning, or
      // consecutive '.'.
      int removed_len = i == pPos - 1 ? i + 2 : i + 1;
      str = str.Right(str.GetLength() - removed_len);
      break;
    }
    // Found a valid '.'.
    pPos = i;
  }

  // Check the domain name part.
  aPos = str.Find(L'@');
  if (aPos < 1)
    return false;

  str.TrimRight(L'.');
  // At least one '.' in domain name, but not at the beginning.
  // TODO(weili): RFC5322 allows domain names to be a local name without '.'.
  // Check whether we should remove this check.
  int ePos = str.Find(L'.', aPos + 1);
  if (ePos == -1 || ePos == aPos + 1)
    return false;

  // Validate all other chars in domain name.
  int nLen = str.GetLength();
  pPos = 0;  // Used to track the position of '.'.
  for (int i = aPos + 1; i < nLen; i++) {
    wchar_t wch = str.GetAt(i);
    if (wch == L'-' || FXSYS_iswalnum(wch))
      continue;

    if (wch != L'.' || i == pPos + 1) {
      // Domain name should end before invalid char.
      int host_end = i == pPos + 1 ? i - 2 : i - 1;
      if (pPos > 0 && host_end - aPos >= 3) {
        // Trim the ending invalid chars if there is at least one '.' and name.
        str = str.Left(host_end + 1);
        break;
      }
      return false;
    }
    pPos = i;
  }

  if (str.Find(L"mailto:") == -1)
    str = L"mailto:" + str;

  return true;
}

CFX_WideString CPDF_LinkExtract::GetURL(size_t index) const {
  return index < m_LinkArray.size() ? m_LinkArray[index].m_strUrl : L"";
}

std::vector<CFX_FloatRect> CPDF_LinkExtract::GetRects(size_t index) const {
  if (index >= m_LinkArray.size())
    return std::vector<CFX_FloatRect>();

  return m_pTextPage->GetRectArray(m_LinkArray[index].m_Start,
                                   m_LinkArray[index].m_Count);
}
