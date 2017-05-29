// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fdrm/crypto/sha256.h"

#include <string.h>

#define F0(x, y, z) ((x & y) | (z & (x | y)))
#define F1(x, y, z) (z ^ (x & (y ^ z)))
#define SHR(x, n) ((x & 0xFFFFFFFF) >> n)
#define ROTR(x, n) (SHR(x, n) | (x << (32 - n)))
#define S0(x) (ROTR(x, 7) ^ ROTR(x, 18) ^ SHR(x, 3))
#define S1(x) (ROTR(x, 17) ^ ROTR(x, 19) ^ SHR(x, 10))
#define S2(x) (ROTR(x, 2) ^ ROTR(x, 13) ^ ROTR(x, 22))
#define S3(x) (ROTR(x, 6) ^ ROTR(x, 11) ^ ROTR(x, 25))
#define R(t) (W[t] = S1(W[t - 2]) + W[t - 7] + S0(W[t - 15]) + W[t - 16])

#define P(a, b, c, d, e, f, g, h, x, K)               \
  {                                                   \
    uint32_t temp1 = h + S3(e) + F1(e, f, g) + K + x; \
    uint32_t temp2 = S2(a) + F0(a, b, c);             \
    d += temp1;                                       \
    h = temp1 + temp2;                                \
  }

#define PUT_UINT32(n, b, i)              \
  {                                      \
    (b)[(i)] = (uint8_t)((n) >> 24);     \
    (b)[(i) + 1] = (uint8_t)((n) >> 16); \
    (b)[(i) + 2] = (uint8_t)((n) >> 8);  \
    (b)[(i) + 3] = (uint8_t)((n));       \
  }

#define GET_UINT32(n, b, i)                                             \
  {                                                                     \
    (n) = ((uint32_t)(b)[(i)] << 24) | ((uint32_t)(b)[(i) + 1] << 16) | \
          ((uint32_t)(b)[(i) + 2] << 8) | ((uint32_t)(b)[(i) + 3]);     \
  }

namespace crypto {

namespace {

const uint8_t kPadding[64] = {
    0x80, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    0,    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

}  // namespace

// static
void SHA256::Generate(const uint8_t* data, uint32_t size, uint8_t digest[32]) {
  SHA256 sha;
  sha.Start();
  sha.Update(data, size);
  sha.Finish(digest);
}

SHA256::SHA256() {}

SHA256::~SHA256() {}

void SHA256::Start() {
  total_[0] = 0;
  total_[1] = 0;
  state_[0] = 0x6A09E667;
  state_[1] = 0xBB67AE85;
  state_[2] = 0x3C6EF372;
  state_[3] = 0xA54FF53A;
  state_[4] = 0x510E527F;
  state_[5] = 0x9B05688C;
  state_[6] = 0x1F83D9AB;
  state_[7] = 0x5BE0CD19;
}

void SHA256::Update(const uint8_t* input, uint32_t length) {
  if (!length)
    return;

  uint32_t left = total_[0] & 0x3F;
  uint32_t fill = 64 - left;
  total_[0] += length;
  total_[0] &= 0xFFFFFFFF;
  if (total_[0] < length)
    total_[1]++;

  if (left && length >= fill) {
    memcpy(buffer_ + left, input, fill);
    Process(buffer_);
    length -= fill;
    input += fill;
    left = 0;
  }
  while (length >= 64) {
    Process(input);
    length -= 64;
    input += 64;
  }
  if (length)
    memcpy(buffer_ + left, input, length);
}

void SHA256::Finish(uint8_t digest[32]) {
  uint8_t msglen[8];

  uint32_t high = (total_[0] >> 29) | (total_[1] << 3);
  uint32_t low = (total_[0] << 3);
  PUT_UINT32(high, msglen, 0);
  PUT_UINT32(low, msglen, 4);

  uint32_t last = total_[0] & 0x3F;
  uint32_t padn = (last < 56) ? (56 - last) : (120 - last);

  Update(kPadding, padn);
  Update(msglen, 8);

  PUT_UINT32(state_[0], digest, 0);
  PUT_UINT32(state_[1], digest, 4);
  PUT_UINT32(state_[2], digest, 8);
  PUT_UINT32(state_[3], digest, 12);
  PUT_UINT32(state_[4], digest, 16);
  PUT_UINT32(state_[5], digest, 20);
  PUT_UINT32(state_[6], digest, 24);
  PUT_UINT32(state_[7], digest, 28);
}

void SHA256::Process(const uint8_t data[64]) {
  uint32_t W[64];
  GET_UINT32(W[0], data, 0);
  GET_UINT32(W[1], data, 4);
  GET_UINT32(W[2], data, 8);
  GET_UINT32(W[3], data, 12);
  GET_UINT32(W[4], data, 16);
  GET_UINT32(W[5], data, 20);
  GET_UINT32(W[6], data, 24);
  GET_UINT32(W[7], data, 28);
  GET_UINT32(W[8], data, 32);
  GET_UINT32(W[9], data, 36);
  GET_UINT32(W[10], data, 40);
  GET_UINT32(W[11], data, 44);
  GET_UINT32(W[12], data, 48);
  GET_UINT32(W[13], data, 52);
  GET_UINT32(W[14], data, 56);
  GET_UINT32(W[15], data, 60);

  uint32_t A = state_[0];
  uint32_t B = state_[1];
  uint32_t C = state_[2];
  uint32_t D = state_[3];
  uint32_t E = state_[4];
  uint32_t F = state_[5];
  uint32_t G = state_[6];
  uint32_t H = state_[7];
  P(A, B, C, D, E, F, G, H, W[0], 0x428A2F98);
  P(H, A, B, C, D, E, F, G, W[1], 0x71374491);
  P(G, H, A, B, C, D, E, F, W[2], 0xB5C0FBCF);
  P(F, G, H, A, B, C, D, E, W[3], 0xE9B5DBA5);
  P(E, F, G, H, A, B, C, D, W[4], 0x3956C25B);
  P(D, E, F, G, H, A, B, C, W[5], 0x59F111F1);
  P(C, D, E, F, G, H, A, B, W[6], 0x923F82A4);
  P(B, C, D, E, F, G, H, A, W[7], 0xAB1C5ED5);
  P(A, B, C, D, E, F, G, H, W[8], 0xD807AA98);
  P(H, A, B, C, D, E, F, G, W[9], 0x12835B01);
  P(G, H, A, B, C, D, E, F, W[10], 0x243185BE);
  P(F, G, H, A, B, C, D, E, W[11], 0x550C7DC3);
  P(E, F, G, H, A, B, C, D, W[12], 0x72BE5D74);
  P(D, E, F, G, H, A, B, C, W[13], 0x80DEB1FE);
  P(C, D, E, F, G, H, A, B, W[14], 0x9BDC06A7);
  P(B, C, D, E, F, G, H, A, W[15], 0xC19BF174);
  P(A, B, C, D, E, F, G, H, R(16), 0xE49B69C1);
  P(H, A, B, C, D, E, F, G, R(17), 0xEFBE4786);
  P(G, H, A, B, C, D, E, F, R(18), 0x0FC19DC6);
  P(F, G, H, A, B, C, D, E, R(19), 0x240CA1CC);
  P(E, F, G, H, A, B, C, D, R(20), 0x2DE92C6F);
  P(D, E, F, G, H, A, B, C, R(21), 0x4A7484AA);
  P(C, D, E, F, G, H, A, B, R(22), 0x5CB0A9DC);
  P(B, C, D, E, F, G, H, A, R(23), 0x76F988DA);
  P(A, B, C, D, E, F, G, H, R(24), 0x983E5152);
  P(H, A, B, C, D, E, F, G, R(25), 0xA831C66D);
  P(G, H, A, B, C, D, E, F, R(26), 0xB00327C8);
  P(F, G, H, A, B, C, D, E, R(27), 0xBF597FC7);
  P(E, F, G, H, A, B, C, D, R(28), 0xC6E00BF3);
  P(D, E, F, G, H, A, B, C, R(29), 0xD5A79147);
  P(C, D, E, F, G, H, A, B, R(30), 0x06CA6351);
  P(B, C, D, E, F, G, H, A, R(31), 0x14292967);
  P(A, B, C, D, E, F, G, H, R(32), 0x27B70A85);
  P(H, A, B, C, D, E, F, G, R(33), 0x2E1B2138);
  P(G, H, A, B, C, D, E, F, R(34), 0x4D2C6DFC);
  P(F, G, H, A, B, C, D, E, R(35), 0x53380D13);
  P(E, F, G, H, A, B, C, D, R(36), 0x650A7354);
  P(D, E, F, G, H, A, B, C, R(37), 0x766A0ABB);
  P(C, D, E, F, G, H, A, B, R(38), 0x81C2C92E);
  P(B, C, D, E, F, G, H, A, R(39), 0x92722C85);
  P(A, B, C, D, E, F, G, H, R(40), 0xA2BFE8A1);
  P(H, A, B, C, D, E, F, G, R(41), 0xA81A664B);
  P(G, H, A, B, C, D, E, F, R(42), 0xC24B8B70);
  P(F, G, H, A, B, C, D, E, R(43), 0xC76C51A3);
  P(E, F, G, H, A, B, C, D, R(44), 0xD192E819);
  P(D, E, F, G, H, A, B, C, R(45), 0xD6990624);
  P(C, D, E, F, G, H, A, B, R(46), 0xF40E3585);
  P(B, C, D, E, F, G, H, A, R(47), 0x106AA070);
  P(A, B, C, D, E, F, G, H, R(48), 0x19A4C116);
  P(H, A, B, C, D, E, F, G, R(49), 0x1E376C08);
  P(G, H, A, B, C, D, E, F, R(50), 0x2748774C);
  P(F, G, H, A, B, C, D, E, R(51), 0x34B0BCB5);
  P(E, F, G, H, A, B, C, D, R(52), 0x391C0CB3);
  P(D, E, F, G, H, A, B, C, R(53), 0x4ED8AA4A);
  P(C, D, E, F, G, H, A, B, R(54), 0x5B9CCA4F);
  P(B, C, D, E, F, G, H, A, R(55), 0x682E6FF3);
  P(A, B, C, D, E, F, G, H, R(56), 0x748F82EE);
  P(H, A, B, C, D, E, F, G, R(57), 0x78A5636F);
  P(G, H, A, B, C, D, E, F, R(58), 0x84C87814);
  P(F, G, H, A, B, C, D, E, R(59), 0x8CC70208);
  P(E, F, G, H, A, B, C, D, R(60), 0x90BEFFFA);
  P(D, E, F, G, H, A, B, C, R(61), 0xA4506CEB);
  P(C, D, E, F, G, H, A, B, R(62), 0xBEF9A3F7);
  P(B, C, D, E, F, G, H, A, R(63), 0xC67178F2);
  state_[0] += A;
  state_[1] += B;
  state_[2] += C;
  state_[3] += D;
  state_[4] += E;
  state_[5] += F;
  state_[6] += G;
  state_[7] += H;
}

}  // namespace crypto
