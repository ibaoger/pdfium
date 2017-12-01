// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cfx_cttgsubtable.h"

#include <utility>

#include "core/fxge/fx_freetype.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

CFX_CTTGSUBTable::CFX_CTTGSUBTable() : m_bFeautureMapLoad(false) {}

CFX_CTTGSUBTable::~CFX_CTTGSUBTable() {}

bool CFX_CTTGSUBTable::LoadGSUBTable(FT_Bytes gsub) {
  if ((gsub[0] << 24u | gsub[1] << 16u | gsub[2] << 8u | gsub[3]) != 0x00010000)
    return false;

  return Parse(&gsub[gsub[4] << 8 | gsub[5]], &gsub[gsub[6] << 8 | gsub[7]],
               &gsub[gsub[8] << 8 | gsub[9]]);
}

bool CFX_CTTGSUBTable::GetVerticalGlyph(uint32_t glyphnum,
                                        uint32_t* vglyphnum) {
  uint32_t tag[] = {
      (uint8_t)'v' << 24 | (uint8_t)'r' << 16 | (uint8_t)'t' << 8 |
          (uint8_t)'2',
      (uint8_t)'v' << 24 | (uint8_t)'e' << 16 | (uint8_t)'r' << 8 |
          (uint8_t)'t',
  };
  if (!m_bFeautureMapLoad) {
    for (const TScriptRecord& script : ScriptList) {
      for (const auto& record : script.LangSysRecords) {
        for (const auto& index : record.FeatureIndices) {
          if (FeatureList[index].FeatureTag == tag[0] ||
              FeatureList[index].FeatureTag == tag[1]) {
            m_featureSet.insert(index);
          }
        }
      }
    }
    if (m_featureSet.empty()) {
      int i = 0;
      for (const TFeatureRecord& feature : FeatureList) {
        if (feature.FeatureTag == tag[0] || feature.FeatureTag == tag[1])
          m_featureSet.insert(i);
        ++i;
      }
    }
    m_bFeautureMapLoad = true;
  }
  for (const auto& item : m_featureSet) {
    if (GetVerticalGlyphSub(glyphnum, vglyphnum, &FeatureList[item])) {
      return true;
    }
  }
  return false;
}

bool CFX_CTTGSUBTable::GetVerticalGlyphSub(uint32_t glyphnum,
                                           uint32_t* vglyphnum,
                                           TFeatureRecord* Feature) {
  for (int index : Feature->LookupListIndices) {
    if (!pdfium::IndexInBounds(LookupList, index))
      continue;
    if (LookupList[index].LookupType == 1 &&
        GetVerticalGlyphSub2(glyphnum, vglyphnum, &LookupList[index])) {
      return true;
    }
  }
  return false;
}

bool CFX_CTTGSUBTable::GetVerticalGlyphSub2(uint32_t glyphnum,
                                            uint32_t* vglyphnum,
                                            TLookup* Lookup) {
  for (const auto& subTable : Lookup->SubTables) {
    switch (subTable->SubstFormat) {
      case 1: {
        if (GetCoverageIndex(subTable->Coverage.get(), glyphnum) >= 0) {
          *vglyphnum = glyphnum + subTable->DeltaGlyphID;
          return true;
        }
        break;
      }
      case 2: {
        int index = GetCoverageIndex(subTable->Coverage.get(), glyphnum);
        if (pdfium::IndexInBounds(subTable->Substitutes, index)) {
          *vglyphnum = subTable->Substitutes[index];
          return true;
        }
        break;
      }
    }
  }
  return false;
}

int CFX_CTTGSUBTable::GetCoverageIndex(TCoverageFormat* Coverage,
                                       uint32_t g) const {
  if (!Coverage)
    return -1;

  switch (Coverage->CoverageFormat) {
    case 1: {
      int i = 0;
      for (const auto& glyph : Coverage->GlyphArray) {
        if (static_cast<uint32_t>(glyph) == g)
          return i;
        ++i;
      }
      return -1;
    }
    case 2: {
      for (const auto& rangeRec : Coverage->RangeRecords) {
        uint32_t s = rangeRec.Start;
        uint32_t e = rangeRec.End;
        uint32_t si = rangeRec.StartCoverageIndex;
        if (s <= g && g <= e)
          return si + g - s;
      }
      return -1;
    }
  }
  return -1;
}

uint8_t CFX_CTTGSUBTable::GetUInt8(FT_Bytes& p) const {
  uint8_t ret = p[0];
  p += 1;
  return ret;
}

int16_t CFX_CTTGSUBTable::GetInt16(FT_Bytes& p) const {
  uint16_t ret = p[0] << 8 | p[1];
  p += 2;
  return *(int16_t*)&ret;
}

uint16_t CFX_CTTGSUBTable::GetUInt16(FT_Bytes& p) const {
  uint16_t ret = p[0] << 8 | p[1];
  p += 2;
  return ret;
}

int32_t CFX_CTTGSUBTable::GetInt32(FT_Bytes& p) const {
  uint32_t ret = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
  p += 4;
  return *(int32_t*)&ret;
}

uint32_t CFX_CTTGSUBTable::GetUInt32(FT_Bytes& p) const {
  uint32_t ret = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
  p += 4;
  return ret;
}

bool CFX_CTTGSUBTable::Parse(FT_Bytes scriptlist,
                             FT_Bytes featurelist,
                             FT_Bytes lookuplist) {
  ParseScriptList(scriptlist);
  ParseFeatureList(featurelist);
  ParseLookupList(lookuplist);
  return true;
}

void CFX_CTTGSUBTable::ParseScriptList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  ScriptList = std::vector<TScriptRecord>(GetUInt16(sp));
  for (auto& scriptRec : ScriptList) {
    scriptRec.ScriptTag = GetUInt32(sp);
    ParseScript(&raw[GetUInt16(sp)], &scriptRec);
  }
}

void CFX_CTTGSUBTable::ParseScript(FT_Bytes raw, TScriptRecord* rec) {
  FT_Bytes sp = raw;
  rec->DefaultLangSys = GetUInt16(sp);
  rec->LangSysRecords = std::vector<TLangSysRecord>(GetUInt16(sp));
  for (auto& sysRecord : rec->LangSysRecords) {
    sysRecord.LangSysTag = GetUInt32(sp);
    ParseLangSys(&raw[GetUInt16(sp)], &sysRecord);
  }
}

void CFX_CTTGSUBTable::ParseLangSys(FT_Bytes raw, TLangSysRecord* rec) {
  FT_Bytes sp = raw;
  rec->LookupOrder = GetUInt16(sp);
  rec->ReqFeatureIndex = GetUInt16(sp);
  rec->FeatureIndices = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& element : rec->FeatureIndices)
    element = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseFeatureList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  FeatureList = std::vector<TFeatureRecord>(GetUInt16(sp));
  for (auto& featureRec : FeatureList) {
    featureRec.FeatureTag = GetUInt32(sp);
    ParseFeature(&raw[GetUInt16(sp)], &featureRec);
  }
}

void CFX_CTTGSUBTable::ParseFeature(FT_Bytes raw, TFeatureRecord* rec) {
  FT_Bytes sp = raw;
  rec->FeatureParams = GetUInt16(sp);
  rec->LookupListIndices = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& listIndex : rec->LookupListIndices)
    listIndex = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseLookupList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  LookupList = std::vector<TLookup>(GetUInt16(sp));
  for (auto& lookup : LookupList)
    ParseLookup(&raw[GetUInt16(sp)], &lookup);
}

void CFX_CTTGSUBTable::ParseLookup(FT_Bytes raw, TLookup* rec) {
  FT_Bytes sp = raw;
  rec->LookupType = GetUInt16(sp);
  rec->LookupFlag = GetUInt16(sp);
  rec->SubTables = std::vector<std::unique_ptr<TSubTable>>(GetUInt16(sp));
  if (rec->LookupType != 1)
    return;

  for (auto& subTable : rec->SubTables)
    ParseSingleSubst(&raw[GetUInt16(sp)], &subTable);
}

std::unique_ptr<CFX_CTTGSUBTable::TCoverageFormat>
CFX_CTTGSUBTable::ParseCoverage(FT_Bytes raw) {
  FT_Bytes sp = raw;
  uint16_t format = GetUInt16(sp);
  if (format == 1) {
    auto rec = pdfium::MakeUnique<TCoverageFormat>(1);
    ParseCoverageFormat1(raw, rec.get());
    return rec;
  }
  if (format == 2) {
    auto rec = pdfium::MakeUnique<TCoverageFormat>(2);
    ParseCoverageFormat2(raw, rec.get());
    return rec;
  }
  return nullptr;
}

void CFX_CTTGSUBTable::ParseCoverageFormat1(FT_Bytes raw,
                                            TCoverageFormat* rec) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  rec->GlyphArray = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& glyph : rec->GlyphArray)
    glyph = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseCoverageFormat2(FT_Bytes raw,
                                            TCoverageFormat* rec) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  rec->RangeRecords = std::vector<TRangeRecord>(GetUInt16(sp));
  for (auto& rangeRec : rec->RangeRecords) {
    rangeRec.Start = GetUInt16(sp);
    rangeRec.End = GetUInt16(sp);
    rangeRec.StartCoverageIndex = GetUInt16(sp);
  }
}

void CFX_CTTGSUBTable::ParseSingleSubst(FT_Bytes raw,
                                        std::unique_ptr<TSubTable>* rec) {
  FT_Bytes sp = raw;
  uint16_t Format = GetUInt16(sp);
  switch (Format) {
    case 1:
      *rec = pdfium::MakeUnique<TSubTable>(1);
      ParseSingleSubstFormat1(raw, rec->get());
      break;
    case 2:
      *rec = pdfium::MakeUnique<TSubTable>(2);
      ParseSingleSubstFormat2(raw, rec->get());
      break;
  }
}

void CFX_CTTGSUBTable::ParseSingleSubstFormat1(FT_Bytes raw, TSubTable* rec) {
  FT_Bytes sp = raw;
  GetUInt16(sp);
  uint16_t offset = GetUInt16(sp);
  rec->Coverage = ParseCoverage(&raw[offset]);
  rec->DeltaGlyphID = GetInt16(sp);
}

void CFX_CTTGSUBTable::ParseSingleSubstFormat2(FT_Bytes raw, TSubTable* rec) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  uint16_t offset = GetUInt16(sp);
  rec->Coverage = ParseCoverage(&raw[offset]);
  rec->Substitutes = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& substitute : rec->Substitutes)
    substitute = GetUInt16(sp);
}

CFX_CTTGSUBTable::TLangSysRecord::TLangSysRecord()
    : LangSysTag(0), LookupOrder(0), ReqFeatureIndex(0) {}

CFX_CTTGSUBTable::TLangSysRecord::~TLangSysRecord() {}

CFX_CTTGSUBTable::TScriptRecord::TScriptRecord()
    : ScriptTag(0), DefaultLangSys(0) {}

CFX_CTTGSUBTable::TScriptRecord::~TScriptRecord() {}

CFX_CTTGSUBTable::TFeatureRecord::TFeatureRecord()
    : FeatureTag(0), FeatureParams(0) {}

CFX_CTTGSUBTable::TFeatureRecord::~TFeatureRecord() {}

CFX_CTTGSUBTable::TRangeRecord::TRangeRecord()
    : Start(0), End(0), StartCoverageIndex(0) {}

CFX_CTTGSUBTable::TSubTable::TSubTable(uint16_t format) : SubstFormat(format) {}

CFX_CTTGSUBTable::TSubTable::~TSubTable() {}

CFX_CTTGSUBTable::TLookup::TLookup() : LookupType(0), LookupFlag(0) {}

CFX_CTTGSUBTable::TLookup::~TLookup() {}
