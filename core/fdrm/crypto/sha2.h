// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FDRM_CRYPTO_SHA2_H_
#define CORE_FDRM_CRYPTO_SHA2_H_

#include <stdint.h>

namespace crypto {

class SHA2 {
 public:
  SHA2();
  virtual ~SHA2();

 protected:
  uint64_t total_[2];
  uint64_t state_[8];
  uint8_t buffer_[128];
};

}  // namespace crypto

#endif  // CORE_FDRM_CRYPTO_SHA2_H_
