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

#include "fxbarcode/cbc_code128.h"

#include <memory>

#include "fxbarcode/oned/BC_OnedCode128Writer.h"
#include "third_party/base/ptr_util.h"

CBC_Code128::CBC_Code128(BC_TYPE type)
    : CBC_OneCode(pdfium::MakeUnique<CBC_OnedCode128Writer>(type)) {}

CBC_Code128::~CBC_Code128() {}

bool CBC_Code128::SetTextLocation(BC_TEXT_LOC location) {
  return GetOnedCode128Writer()->SetTextLocation(location);
}

bool CBC_Code128::Encode(const CFX_WideStringC& contents, bool isDevice) {
  if (contents.IsEmpty())
    return false;

  BCFORMAT format = BCFORMAT_CODE_128;
  int32_t outWidth = 0;
  int32_t outHeight = 0;
  auto* pWriter = GetOnedCode128Writer();
  CFX_WideString content(contents);
  if (contents.GetLength() % 2 && pWriter->GetType() == BC_CODE128_C)
    content += '0';

  CFX_WideString encodeContents = pWriter->FilterContents(content.AsStringC());
  m_renderContents = encodeContents;
  CFX_ByteString byteString = encodeContents.UTF8Encode();
  std::unique_ptr<uint8_t, FxFreeDeleter> data(
      pWriter->Encode(byteString, format, outWidth, outHeight));
  if (!data)
    return false;
  return pWriter->RenderResult(encodeContents.AsStringC(), data.get(), outWidth,
                               isDevice);
}

bool CBC_Code128::RenderDevice(CFX_RenderDevice* device,
                               const CFX_Matrix* matrix) {
  return GetOnedCode128Writer()->RenderDeviceResult(
      device, matrix, m_renderContents.AsStringC());
}

BC_TYPE CBC_Code128::GetType() {
  return BC_CODE128;
}

CBC_OnedCode128Writer* CBC_Code128::GetOnedCode128Writer() {
  return static_cast<CBC_OnedCode128Writer*>(m_pBCWriter.get());
}
