// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/ttgsubtable.h"

#include "core/fxge/fx_freetype.h"
#include "third_party/base/stl_util.h"

CFX_GlyphMap::CFX_GlyphMap() {}

CFX_GlyphMap::~CFX_GlyphMap() {}

extern "C" {
static int _CompareInt(const void* p1, const void* p2) {
  return (*(uint32_t*)p1) - (*(uint32_t*)p2);
}
};

struct _IntPair {
  int32_t key;
  int32_t value;
};

void CFX_GlyphMap::SetAt(int key, int value) {
  uint32_t count = m_Buffer.GetSize() / sizeof(_IntPair);
  _IntPair* buf = (_IntPair*)m_Buffer.GetBuffer();
  _IntPair pair = {key, value};
  if (count == 0 || key > buf[count - 1].key) {
    m_Buffer.AppendBlock(&pair, sizeof(_IntPair));
    return;
  }
  int low = 0, high = count - 1;
  while (low <= high) {
    int mid = (low + high) / 2;
    if (buf[mid].key < key) {
      low = mid + 1;
    } else if (buf[mid].key > key) {
      high = mid - 1;
    } else {
      buf[mid].value = value;
      return;
    }
  }
  m_Buffer.InsertBlock(low * sizeof(_IntPair), &pair, sizeof(_IntPair));
}

bool CFX_GlyphMap::Lookup(int key, int& value) {
  void* pResult = FXSYS_bsearch(&key, m_Buffer.GetBuffer(),
                                m_Buffer.GetSize() / sizeof(_IntPair),
                                sizeof(_IntPair), _CompareInt);
  if (!pResult) {
    return false;
  }
  value = ((uint32_t*)pResult)[1];
  return true;
}

CFX_CTTGSUBTable::CFX_CTTGSUBTable()
    : m_bFeautureMapLoad(false), loaded(false) {}

CFX_CTTGSUBTable::CFX_CTTGSUBTable(FT_Bytes gsub)
    : m_bFeautureMapLoad(false), loaded(false) {
  LoadGSUBTable(gsub);
}

CFX_CTTGSUBTable::~CFX_CTTGSUBTable() {}

bool CFX_CTTGSUBTable::IsOk() const {
  return loaded;
}

bool CFX_CTTGSUBTable::LoadGSUBTable(FT_Bytes gsub) {
  header.Version = gsub[0] << 24 | gsub[1] << 16 | gsub[2] << 8 | gsub[3];
  if (header.Version != 0x00010000) {
    return false;
  }
  header.ScriptList = gsub[4] << 8 | gsub[5];
  header.FeatureList = gsub[6] << 8 | gsub[7];
  header.LookupList = gsub[8] << 8 | gsub[9];
  return Parse(&gsub[header.ScriptList], &gsub[header.FeatureList],
               &gsub[header.LookupList]);
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
    for (const auto& script : ScriptList.ScriptRecord) {
      for (const auto& record : script.Script.LangSysRecord) {
        for (const auto& index : record.LangSys.FeatureIndex) {
          if (FeatureList.FeatureRecord[index].FeatureTag == tag[0] ||
              FeatureList.FeatureRecord[index].FeatureTag == tag[1]) {
            if (!pdfium::ContainsKey(m_featureMap, index)) {
              m_featureMap[index] = index;
            }
          }
        }
      }
    }
    if (m_featureMap.empty()) {
      int i = 0;
      for (const auto& feature : FeatureList.FeatureRecord) {
        if (feature.FeatureTag == tag[0] || feature.FeatureTag == tag[1])
          m_featureMap[i] = i;
        ++i;
      }
    }
    m_bFeautureMapLoad = true;
  }
  for (const auto& pair : m_featureMap) {
    if (GetVerticalGlyphSub(glyphnum, vglyphnum,
                            &FeatureList.FeatureRecord[pair.second].Feature)) {
      return true;
    }
  }
  return false;
}

bool CFX_CTTGSUBTable::GetVerticalGlyphSub(uint32_t glyphnum,
                                           uint32_t* vglyphnum,
                                           TFeature* Feature) {
  for (int index : Feature->LookupListIndex) {
    if (index < 0 || pdfium::CollectionSize<int>(LookupList.Lookup) < index)
      continue;

    if (LookupList.Lookup[index].LookupType == 1 &&
        GetVerticalGlyphSub2(glyphnum, vglyphnum, &LookupList.Lookup[index])) {
      return true;
    }
  }
  return false;
}

bool CFX_CTTGSUBTable::GetVerticalGlyphSub2(uint32_t glyphnum,
                                            uint32_t* vglyphnum,
                                            TLookup* Lookup) {
  for (const auto& subTable : Lookup->SubTable) {
    switch (subTable->SubstFormat) {
      case 1: {
        TSingleSubstFormat1* tbl1 = (TSingleSubstFormat1*)subTable;
        if (GetCoverageIndex(tbl1->Coverage.get(), glyphnum) >= 0) {
          *vglyphnum = glyphnum + tbl1->DeltaGlyphID;
          return true;
        }
        break;
      }
      case 2: {
        TSingleSubstFormat2* tbl2 = (TSingleSubstFormat2*)subTable;
        int index = GetCoverageIndex(tbl2->Coverage.get(), glyphnum);
        if (index >= 0 &&
            index < pdfium::CollectionSize<int>(tbl2->Substitute)) {
          *vglyphnum = tbl2->Substitute[index];
          return true;
        }
        break;
      }
    }
  }
  return false;
}

int CFX_CTTGSUBTable::GetCoverageIndex(TCoverageFormatBase* Coverage,
                                       uint32_t g) const {
  if (!Coverage)
    return -1;

  switch (Coverage->CoverageFormat) {
    case 1: {
      int i = 0;
      TCoverageFormat1* c1 = (TCoverageFormat1*)Coverage;
      for (const auto& glyph : c1->GlyphArray) {
        if ((uint32_t)glyph == g)
          return i;
        ++i;
      }
      return -1;
    }
    case 2: {
      TCoverageFormat2* c2 = (TCoverageFormat2*)Coverage;
      for (const auto& rangeRec : c2->RangeRecord) {
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
  ParseScriptList(scriptlist, &ScriptList);
  ParseFeatureList(featurelist, &FeatureList);
  ParseLookupList(lookuplist, &LookupList);
  return true;
}

void CFX_CTTGSUBTable::ParseScriptList(FT_Bytes raw, TScriptList* rec) {
  FT_Bytes sp = raw;
  rec->ScriptRecord = std::vector<TScriptRecord>(GetUInt16(sp));
  for (auto& scriptRec : rec->ScriptRecord) {
    scriptRec.ScriptTag = GetUInt32(sp);
    ParseScript(&raw[GetUInt16(sp)], &scriptRec.Script);
  }
}

void CFX_CTTGSUBTable::ParseScript(FT_Bytes raw, TScript* rec) {
  FT_Bytes sp = raw;
  rec->DefaultLangSys = GetUInt16(sp);
  rec->LangSysRecord = std::vector<TLangSysRecord>(GetUInt16(sp));
  for (auto& sysRecord : rec->LangSysRecord) {
    sysRecord.LangSysTag = GetUInt32(sp);
    ParseLangSys(&raw[GetUInt16(sp)], &sysRecord.LangSys);
  }
}

void CFX_CTTGSUBTable::ParseLangSys(FT_Bytes raw, TLangSys* rec) {
  FT_Bytes sp = raw;
  rec->LookupOrder = GetUInt16(sp);
  rec->ReqFeatureIndex = GetUInt16(sp);
  rec->FeatureIndex = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& element : rec->FeatureIndex)
    element = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseFeatureList(FT_Bytes raw, TFeatureList* rec) {
  FT_Bytes sp = raw;
  rec->FeatureRecord = std::vector<TFeatureRecord>(GetUInt16(sp));
  for (auto& featureRec : rec->FeatureRecord) {
    featureRec.FeatureTag = GetUInt32(sp);
    ParseFeature(&raw[GetUInt16(sp)], &featureRec.Feature);
  }
}

void CFX_CTTGSUBTable::ParseFeature(FT_Bytes raw, TFeature* rec) {
  FT_Bytes sp = raw;
  rec->FeatureParams = GetUInt16(sp);
  rec->LookupListIndex = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& listIndex : rec->LookupListIndex)
    listIndex = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseLookupList(FT_Bytes raw, TLookupList* rec) {
  FT_Bytes sp = raw;
  rec->Lookup = std::vector<TLookup>(GetUInt16(sp));
  for (auto& look : rec->Lookup)
    ParseLookup(&raw[GetUInt16(sp)], &look);
}

void CFX_CTTGSUBTable::ParseLookup(FT_Bytes raw, TLookup* rec) {
  FT_Bytes sp = raw;
  rec->LookupType = GetUInt16(sp);
  rec->LookupFlag = GetUInt16(sp);
  rec->SubTable = std::vector<TSubTableBase*>(GetUInt16(sp));
  if (rec->LookupType != 1)
    return;

  for (auto& subTable : rec->SubTable)
    ParseSingleSubst(&raw[GetUInt16(sp)], &subTable);
}

CFX_CTTGSUBTable::TCoverageFormatBase* CFX_CTTGSUBTable::ParseCoverage(
    FT_Bytes raw) {
  FT_Bytes sp = raw;
  uint16_t format = GetUInt16(sp);
  TCoverageFormatBase* rec = nullptr;
  if (format == 1) {
    rec = new TCoverageFormat1();
    ParseCoverageFormat1(raw, static_cast<TCoverageFormat1*>(rec));
  } else if (format == 2) {
    rec = new TCoverageFormat2();
    ParseCoverageFormat2(raw, static_cast<TCoverageFormat2*>(rec));
  }
  return rec;
}

void CFX_CTTGSUBTable::ParseCoverageFormat1(FT_Bytes raw,
                                            TCoverageFormat1* rec) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  rec->GlyphArray = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& glyph : rec->GlyphArray)
    glyph = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseCoverageFormat2(FT_Bytes raw,
                                            TCoverageFormat2* rec) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  rec->RangeRecord = std::vector<TRangeRecord>(GetUInt16(sp));
  for (auto& rangeRec : rec->RangeRecord) {
    rangeRec.Start = GetUInt16(sp);
    rangeRec.End = GetUInt16(sp);
    rangeRec.StartCoverageIndex = GetUInt16(sp);
  }
}

void CFX_CTTGSUBTable::ParseSingleSubst(FT_Bytes raw, TSubTableBase** rec) {
  FT_Bytes sp = raw;
  uint16_t Format = GetUInt16(sp);
  switch (Format) {
    case 1:
      *rec = new TSingleSubstFormat1();
      ParseSingleSubstFormat1(raw, (TSingleSubstFormat1*)*rec);
      break;
    case 2:
      *rec = new TSingleSubstFormat2();
      ParseSingleSubstFormat2(raw, (TSingleSubstFormat2*)*rec);
      break;
  }
}

void CFX_CTTGSUBTable::ParseSingleSubstFormat1(FT_Bytes raw,
                                               TSingleSubstFormat1* rec) {
  FT_Bytes sp = raw;
  GetUInt16(sp);
  uint16_t offset = GetUInt16(sp);
  rec->Coverage.reset(ParseCoverage(&raw[offset]));
  rec->DeltaGlyphID = GetInt16(sp);
}

void CFX_CTTGSUBTable::ParseSingleSubstFormat2(FT_Bytes raw,
                                               TSingleSubstFormat2* rec) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  uint16_t offset = GetUInt16(sp);
  rec->Coverage.reset(ParseCoverage(&raw[offset]));
  rec->Substitute = std::vector<uint16_t>(GetUInt16(sp));
  for (auto& subs : rec->Substitute)
    subs = GetUInt16(sp);
}

CFX_CTTGSUBTable::TCoverageFormat1::TCoverageFormat1()
    : TCoverageFormatBase(1) {}

CFX_CTTGSUBTable::TCoverageFormat1::~TCoverageFormat1() {}

CFX_CTTGSUBTable::TRangeRecord::TRangeRecord()
    : Start(0), End(0), StartCoverageIndex(0) {}

CFX_CTTGSUBTable::TCoverageFormat2::TCoverageFormat2()
    : TCoverageFormatBase(2) {}

CFX_CTTGSUBTable::TCoverageFormat2::~TCoverageFormat2() {}

CFX_CTTGSUBTable::TSingleSubstFormat1::TSingleSubstFormat1()
    : TSubTableBase(1), DeltaGlyphID(0) {}

CFX_CTTGSUBTable::TSingleSubstFormat1::~TSingleSubstFormat1() {}

CFX_CTTGSUBTable::TSingleSubstFormat2::TSingleSubstFormat2()
    : TSubTableBase(2) {}

CFX_CTTGSUBTable::TSingleSubstFormat2::~TSingleSubstFormat2() {}

CFX_CTTGSUBTable::TLookup::TLookup() : LookupType(0), LookupFlag(0) {}

CFX_CTTGSUBTable::TLookup::~TLookup() {
  for (auto& subTable : SubTable)
    delete subTable;
}

CFX_CTTGSUBTable::TScript::TScript() : DefaultLangSys(0) {}

CFX_CTTGSUBTable::TScript::~TScript() {}

CFX_CTTGSUBTable::TScriptList::TScriptList() {}

CFX_CTTGSUBTable::TScriptList::~TScriptList() {}

CFX_CTTGSUBTable::TFeature::TFeature() : FeatureParams(0) {}

CFX_CTTGSUBTable::TFeature::~TFeature() {}

CFX_CTTGSUBTable::TFeatureList::TFeatureList() {}

CFX_CTTGSUBTable::TFeatureList::~TFeatureList() {}

CFX_CTTGSUBTable::TLookupList::TLookupList() {}

CFX_CTTGSUBTable::TLookupList::~TLookupList() {}

CFX_CTTGSUBTable::TLangSys::TLangSys() : LookupOrder(0), ReqFeatureIndex(0) {}

CFX_CTTGSUBTable::TLangSys::~TLangSys() {}
