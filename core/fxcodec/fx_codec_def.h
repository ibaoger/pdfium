// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCODEC_FX_CODEC_DEF_H_
#define CORE_FXCODEC_FX_CODEC_DEF_H_

enum FXCODEC_STATUS {
  FXCODEC_STATUS_ERROR = -1,
  FXCODEC_STATUS_FRAME_READY,
  FXCODEC_STATUS_FRAME_TOBECONTINUE,
  FXCODEC_STATUS_DECODE_READY,
  FXCODEC_STATUS_DECODE_TOBECONTINUE,
  FXCODEC_STATUS_DECODE_FINISH,
#ifdef PDF_ENABLE_XFA
  FXCODEC_STATUS_ERR_MEMORY,
#endif  // PDF_ENABLE_XFA
  FXCODEC_STATUS_ERR_READ,
  FXCODEC_STATUS_ERR_FLUSH,
  FXCODEC_STATUS_ERR_FORMAT,
  FXCODEC_STATUS_ERR_PARAMS
};

#ifdef PDF_ENABLE_XFA
enum FXCODEC_IMAGE_TYPE {
  FXCODEC_IMAGE_UNKNOWN = 0,
  FXCODEC_IMAGE_BMP,
  FXCODEC_IMAGE_JPG,
  FXCODEC_IMAGE_PNG,
  FXCODEC_IMAGE_GIF,
  FXCODEC_IMAGE_TIF,
  FXCODEC_IMAGE_MAX
};
enum FXCODEC_RESUNIT {
  FXCODEC_RESUNIT_NONE = 0,
  FXCODEC_RESUNIT_INCH,
  FXCODEC_RESUNIT_CENTIMETER,
  FXCODEC_RESUNIT_METER
};
#endif  // PDF_ENABLE_XFA

#endif  // CORE_FXCODEC_FX_CODEC_DEF_H_
