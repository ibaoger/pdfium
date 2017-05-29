// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Originally from chromium's /src/base/md5_unittest.cc.

#include "core/fdrm/crypto/sha256.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

namespace crypto {

TEST(SHA256, TestB1) {
  // Example B.1 from FIPS 180-2: one-block message.
  const char* input = "abc";
  const uint8_t expected[32] = {0xba, 0x78, 0x16, 0xbf, 0x8f, 0x01, 0xcf, 0xea,
                                0x41, 0x41, 0x40, 0xde, 0x5d, 0xae, 0x22, 0x23,
                                0xb0, 0x03, 0x61, 0xa3, 0x96, 0x17, 0x7a, 0x9c,
                                0xb4, 0x10, 0xff, 0x61, 0xf2, 0x00, 0x15, 0xad};
  uint8_t actual[32];
  SHA256::Generate(reinterpret_cast<const uint8_t*>(input), strlen(input),
                   actual);
  for (size_t i = 0; i < 32; ++i)
    EXPECT_EQ(expected[i], actual[i]) << " at byte " << i;
}

TEST(SHA256, TestB2) {
  // Example B.2 from FIPS 180-2: multi-block message.
  const char* input =
      "abcdbcdecdefdefgefghfghighijhijkijkljklmklmnlmnomnopnopq";
  const uint8_t expected[32] = {0x24, 0x8d, 0x6a, 0x61, 0xd2, 0x06, 0x38, 0xb8,
                                0xe5, 0xc0, 0x26, 0x93, 0x0c, 0x3e, 0x60, 0x39,
                                0xa3, 0x3c, 0xe4, 0x59, 0x64, 0xff, 0x21, 0x67,
                                0xf6, 0xec, 0xed, 0xd4, 0x19, 0xdb, 0x06, 0xc1};
  uint8_t actual[32];
  SHA256::Generate(reinterpret_cast<const uint8_t*>(input), strlen(input),
                   actual);
  for (size_t i = 0; i < 32; ++i)
    EXPECT_EQ(expected[i], actual[i]) << " at byte " << i;
}

}  // namespace crypto
