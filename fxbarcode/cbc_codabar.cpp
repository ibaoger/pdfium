// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
/*
 * Copyright 2011 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "fxbarcode/cbc_codabar.h"

#include <memory>

#include "fxbarcode/oned/BC_OnedCodaBarWriter.h"

CBC_Codabar::CBC_Codabar() : CBC_OneCode(new CBC_OnedCodaBarWriter) {}

CBC_Codabar::~CBC_Codabar() {}

bool CBC_Codabar::SetStartChar(char start) {
  return GetOnedCodaBarWriter()->SetStartChar(start);
}

bool CBC_Codabar::SetEndChar(char end) {
  return GetOnedCodaBarWriter()->SetEndChar(end);
}

bool CBC_Codabar::SetTextLocation(BC_TEXT_LOC location) {
  return GetOnedCodaBarWriter()->SetTextLocation(location);
}

bool CBC_Codabar::SetWideNarrowRatio(int8_t ratio) {
  return GetOnedCodaBarWriter()->SetWideNarrowRatio(ratio);
}

bool CBC_Codabar::Encode(const CFX_WideStringC& contents, bool isDevice) {
  if (contents.IsEmpty())
    return false;

  BCFORMAT format = BCFORMAT_CODABAR;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  CFX_WideString filtercontents =
      GetOnedCodaBarWriter()->FilterContents(contents);
  CFX_ByteString byteString = filtercontents.UTF8Encode();
  m_renderContents = filtercontents;
  auto* pWriter = GetOnedCodaBarWriter();
  std::unique_ptr<uint8_t, FxFreeDeleter> data(
      pWriter->Encode(byteString, format, outWidth, outHeight));
  if (!data)
    return false;

  return pWriter->RenderResult(filtercontents.AsStringC(), data.get(), outWidth,
                               isDevice);
}

bool CBC_Codabar::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matrix) {
  auto* pWriter = GetOnedCodaBarWriter();
  CFX_WideString renderCon =
      pWriter->encodedContents(m_renderContents.AsStringC());
  return pWriter->RenderDeviceResult(device, matrix, renderCon.AsStringC());
}

BC_TYPE CBC_Codabar::GetType() {
  return BC_CODABAR;
}

CBC_OnedCodaBarWriter* CBC_Codabar::GetOnedCodaBarWriter() {
  return static_cast<CBC_OnedCodaBarWriter*>(m_pBCWriter.get());
}
