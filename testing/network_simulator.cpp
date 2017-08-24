// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/network_simulator.h"

#include <algorithm>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_system.h"
#include "third_party/base/ptr_util.h"

namespace {

using Range = std::pair<size_t, size_t>;

Range FixDirection(const Range& range) {
  if (range.first <= range.second)
    return range;
  return Range(range.second + 1, range.first + 1);
}

bool IsEmptyRange(const Range& range) {
  return range.first == range.second;
}

class FileAccessWrapper : public FPDF_FILEACCESS {
 public:
  explicit FileAccessWrapper(NetworkSimulator* simulator)
      : simulator_(simulator) {
    m_FileLen = simulator_->GetFileSize();
    m_GetBlock = &GetBlockImpl;
    m_Param = this;
  }

  static int GetBlockImpl(void* param,
                          unsigned long position,
                          unsigned char* pBuf,
                          unsigned long size) {
    return static_cast<FileAccessWrapper*>(param)->simulator_->GetBlock(
        position, pBuf, size);
  }

 private:
  NetworkSimulator* simulator_;
};

class FileAvailImpl : public FX_FILEAVAIL {
 public:
  explicit FileAvailImpl(NetworkSimulator* simulator) : simulator_(simulator) {
    version = 1;
    IsDataAvail = &IsDataAvailImpl;
  }

  static FPDF_BOOL IsDataAvailImpl(FX_FILEAVAIL* pThis,
                                   size_t offset,
                                   size_t size) {
    return static_cast<FileAvailImpl*>(pThis)->simulator_->IsDataAvail(offset,
                                                                       size);
  }

 private:
  NetworkSimulator* simulator_;
};

class DownloadHintsImpl : public FX_DOWNLOADHINTS {
 public:
  explicit DownloadHintsImpl(NetworkSimulator* simulator)
      : simulator_(simulator) {
    version = 1;
    AddSegment = &AddSegmentImpl;
  }

  static void AddSegmentImpl(FX_DOWNLOADHINTS* pThis,
                             size_t offset,
                             size_t size) {
    return static_cast<DownloadHintsImpl*>(pThis)->simulator_->AddSegment(
        offset, size);
  }

 private:
  NetworkSimulator* simulator_;
};

}  // namespace

class NetworkSimulator::RangeSet {
 public:
  RangeSet() {}
  ~RangeSet() {}

  bool Contains(const Range& range) const {
    if (IsEmptyRange(range))
      return false;
    const Range fixed_range = FixDirection(range);
    auto it = ranges().upper_bound(fixed_range);

    if (it == ranges().begin())
      return false;  // No ranges includes range.first.
    --it;            // Now it starts equal or before range.first.
    return it->second >= fixed_range.second;
  }

  void Union(const Range& range) {
    if (IsEmptyRange(range))
      return;

    Range fixed_range = FixDirection(range);
    if (IsEmpty()) {
      ranges_.insert(fixed_range);
      return;
    }

    auto start = ranges_.upper_bound(fixed_range);
    if (start != ranges_.begin())
      --start;  // start now points to the key equal or lower than offset.

    if (start->second < fixed_range.first)
      ++start;  // start element is entirely before current range, skip it.

    auto end =
        ranges_.upper_bound(Range(fixed_range.second, fixed_range.second));

    if (start == end) {  // No ranges to merge.
      ranges_.insert(fixed_range);
      return;
    }

    --end;

    const int new_start = std::min<size_t>(start->first, fixed_range.first);
    const int new_end = std::max(end->second, fixed_range.second);

    ranges_.erase(start, ++end);
    ranges_.insert(Range(new_start, new_end));
  }

  void Union(const RangeSet& range_set) {
    if (&range_set == this)
      return;

    for (const auto& it : range_set.ranges()) {
      Union(it);
    }
  }

  bool IsEmpty() const { return ranges().empty(); }

  void Clear() { ranges_.clear(); }

  struct range_compare {
    bool operator()(const Range& lval, const Range& rval) const {
      return lval.first < rval.first;
    }
  };

  using RangesContainer = std::set<Range, range_compare>;
  const RangesContainer& ranges() const { return ranges_; }

 private:
  RangesContainer ranges_;
};

NetworkSimulator::NetworkSimulator(FPDF_FILEACCESS* file_access)
    : file_access_(file_access),
      file_access_wrapper_(pdfium::MakeUnique<FileAccessWrapper>(this)),
      file_avail_(pdfium::MakeUnique<FileAvailImpl>(this)),
      download_hints_(pdfium::MakeUnique<DownloadHintsImpl>(this)),
      available_data_(pdfium::MakeUnique<RangeSet>()),
      requested_data_(pdfium::MakeUnique<RangeSet>()) {
  ASSERT(file_access_);
}

NetworkSimulator::~NetworkSimulator() {}

FPDF_BOOL NetworkSimulator::IsDataAvail(size_t offset, size_t size) {
  return available_data_->Contains(Range(offset, offset + size));
}

void NetworkSimulator::AddSegment(size_t offset, size_t size) {
  requested_data_->Union(Range(offset, offset + size));
}

unsigned long NetworkSimulator::GetFileSize() {
  return file_access_->m_FileLen;
}

int NetworkSimulator::GetBlock(unsigned long position,
                               unsigned char* pBuf,
                               unsigned long size) {
  if (!IsDataAvail(static_cast<size_t>(position), static_cast<size_t>(size)))
    return false;
  return file_access_->m_GetBlock(file_access_->m_Param, position, pBuf, size);
}

void NetworkSimulator::Flush() {
  available_data_->Union(*requested_data_);
  requested_data_->Clear();
}

void NetworkSimulator::FlushWholeFile() {
  available_data_->Union(Range(0, static_cast<size_t>(GetFileSize())));
  requested_data_->Clear();
}
