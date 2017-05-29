// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FDRM_CRYPTO_SHA256_H_
#define CORE_FDRM_CRYPTO_SHA256_H_

#include "core/fdrm/crypto/sha2.h"

namespace crypto {

class SHA256 : public SHA2 {
 public:
  static void Generate(const uint8_t* data, uint32_t size, uint8_t digest[32]);

  SHA256();
  ~SHA256() override;

  void Start();
  void Update(const uint8_t* data, uint32_t size);
  void Finish(uint8_t digest[32]);

 private:
  void Process(const uint8_t data[64]);
};

}  // namespace crypto

#endif  // CORE_FDRM_CRYPTO_SHA256_H_
