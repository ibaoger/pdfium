// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/fx_codec_def.h"

bool IsFXCodecErrorStatus(FXCODEC_STATUS status) {
  return (status == FXCODEC_STATUS_ERROR ||
#ifdef PDF_ENABLE_XFA
          status == FXCODEC_STATUS_ERR_MEMORY ||
#endif  // PDF_ENABLE_XFA
          status == FXCODEC_STATUS_ERR_READ ||
          status == FXCODEC_STATUS_ERR_FLUSH ||
          status == FXCODEC_STATUS_ERR_FORMAT ||
          status == FXCODEC_STATUS_ERR_PARAMS);
}
