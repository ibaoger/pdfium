// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcodec/gif/cfx_gifcontext.h"

#include "core/fxcrt/unowned_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFX_GifContextTestWrapper {
 public:
  explicit CFX_GifContextTestWrapper(CFX_GifContext* context)
      : context_(context) {}
  ~CFX_GifContextTestWrapper() {}

  CFX_GifDecodeStatus ReadGifSignature() {
    return context_->ReadGifSignature();
  }
  CFX_GifDecodeStatus ReadLogicalScreenDescriptor() {
    return context_->ReadLogicalScreenDescriptor();
  }
  uint8_t* ReadData(uint8_t** des_buf_pp, uint32_t data_size) {
    return context_->ReadData(des_buf_pp, data_size);
  }

 private:
  UnownedPtr<CFX_GifContext> context_;
};

TEST(CFX_GifContext, SetInputBuffer) {
  CFX_GifContext context(nullptr, nullptr);

  context.SetInputBuffer(nullptr, 0);
  EXPECT_EQ(nullptr, context.next_in_);
  EXPECT_EQ(0u, context.avail_in_);
  EXPECT_EQ(0u, context.skip_size_);

  context.SetInputBuffer(nullptr, 100);
  EXPECT_EQ(nullptr, context.next_in_);
  EXPECT_EQ(100u, context.avail_in_);
  EXPECT_EQ(0u, context.skip_size_);

  uint8_t buffer[] = {0x00, 0x01, 0x02};
  context.SetInputBuffer(buffer, 0);
  EXPECT_EQ(buffer, context.next_in_);
  EXPECT_EQ(0u, context.avail_in_);
  EXPECT_EQ(0u, context.skip_size_);

  context.SetInputBuffer(buffer, 3);
  EXPECT_EQ(buffer, context.next_in_);
  EXPECT_EQ(3u, context.avail_in_);
  EXPECT_EQ(0u, context.skip_size_);

  context.SetInputBuffer(buffer, 100);
  EXPECT_EQ(buffer, context.next_in_);
  EXPECT_EQ(100u, context.avail_in_);
  EXPECT_EQ(0u, context.skip_size_);
}

TEST(CFX_GifContext, ReadData) {
  CFX_GifContext context(nullptr, nullptr);
  CFX_GifContextTestWrapper wrapper(&context);

  context.SetInputBuffer(nullptr, 0);
  EXPECT_EQ(nullptr, wrapper.ReadData(nullptr, 0));
  EXPECT_EQ(nullptr, wrapper.ReadData(nullptr, 10));

  uint8_t* dest_buffer = nullptr;
  EXPECT_EQ(nullptr,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 0));
  EXPECT_EQ(nullptr,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 10));

  uint8_t src_buffer[] = {0x00, 0x01, 0x02, 0x03, 0x04,
                          0x05, 0x06, 0x07, 0x08, 0x09};
  context.SetInputBuffer(src_buffer, 0);
  EXPECT_EQ(nullptr,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 10));
  EXPECT_EQ(reinterpret_cast<uint8_t*>(src_buffer), context.next_in_);
  EXPECT_EQ(0u, context.avail_in_);
  EXPECT_EQ(0u, context.skip_size_);

  dest_buffer = nullptr;
  context.SetInputBuffer(src_buffer, 10);
  EXPECT_EQ(src_buffer,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 10));
  EXPECT_EQ(reinterpret_cast<uint8_t*>(src_buffer), dest_buffer);
  EXPECT_EQ(src_buffer, context.next_in_);
  EXPECT_EQ(10u, context.avail_in_);
  EXPECT_EQ(10u, context.skip_size_);

  dest_buffer = nullptr;
  context.SetInputBuffer(src_buffer, 10);
  EXPECT_EQ(src_buffer,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 5));
  EXPECT_EQ(reinterpret_cast<uint8_t*>(src_buffer), dest_buffer);
  EXPECT_EQ(src_buffer, context.next_in_);
  EXPECT_EQ(10u, context.avail_in_);
  EXPECT_EQ(5u, context.skip_size_);

  dest_buffer = nullptr;
  EXPECT_EQ(src_buffer + 5,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 5));
  EXPECT_EQ(reinterpret_cast<uint8_t*>(src_buffer + 5), dest_buffer);
  EXPECT_EQ(src_buffer, context.next_in_);
  EXPECT_EQ(10u, context.avail_in_);
  EXPECT_EQ(10u, context.skip_size_);

  dest_buffer = nullptr;
  EXPECT_EQ(nullptr,
            wrapper.ReadData(reinterpret_cast<uint8_t**>(&dest_buffer), 5));
  EXPECT_EQ(nullptr, dest_buffer);
  EXPECT_EQ(src_buffer, context.next_in_);
  EXPECT_EQ(10u, context.avail_in_);
  EXPECT_EQ(10u, context.skip_size_);
}

TEST(CFX_GifContext, ReadGifSignature) {
  CFX_GifContext context(nullptr, nullptr);
  CFX_GifContextTestWrapper wrapper(&context);

  {
    uint8_t data[] = {};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Unfinished, wrapper.ReadGifSignature());
    EXPECT_EQ(0u, context.skip_size_);
  }
  // Make sure testing the entire signature
  {
    uint8_t data[] = {'G', 'I', 'F'};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Unfinished, wrapper.ReadGifSignature());
    EXPECT_EQ(0u, context.skip_size_);
  }
  {
    uint8_t data[] = {'N', 'O', 'T', 'G', 'I', 'F'};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Error, wrapper.ReadGifSignature());
    EXPECT_EQ(6u, context.skip_size_);
  }
  // Make sure not matching GIF8*a
  {
    uint8_t data[] = {'G', 'I', 'F', '8', '0', 'a'};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Error, wrapper.ReadGifSignature());
    EXPECT_EQ(6u, context.skip_size_);
  }
  // Make sure not matching GIF**a
  {
    uint8_t data[] = {'G', 'I', 'F', '9', '2', 'a'};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Error, wrapper.ReadGifSignature());
    EXPECT_EQ(6u, context.skip_size_);
  }
  // One valid signature
  {
    uint8_t data[] = {'G', 'I', 'F', '8', '7', 'a'};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Success, wrapper.ReadGifSignature());
    EXPECT_EQ(6u, context.skip_size_);
  }
  // The other valid signature
  {
    uint8_t data[] = {'G', 'I', 'F', '8', '9', 'a'};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Success, wrapper.ReadGifSignature());
    EXPECT_EQ(6u, context.skip_size_);
  }
}

TEST(CFX_GifContext, ReadLocalScreenDescriptor) {
  CFX_GifContext context(nullptr, nullptr);
  CFX_GifContextTestWrapper wrapper(&context);
  {
    uint8_t data[] = {};
    context.SetInputBuffer(data, sizeof(data));
    EXPECT_EQ(CFX_GifDecodeStatus::Unfinished,
              wrapper.ReadLogicalScreenDescriptor());
  }
  // LSD with all the values zero'd
  {
    CFX_GifLocalScreenDescriptor lsd;
    memset(&lsd, 0, sizeof(CFX_GifLocalScreenDescriptor));
    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&lsd),
                           sizeof(CFX_GifLocalScreenDescriptor));

    EXPECT_EQ(CFX_GifDecodeStatus::Success,
              wrapper.ReadLogicalScreenDescriptor());

    EXPECT_EQ(sizeof(CFX_GifLocalScreenDescriptor), context.skip_size_);
    EXPECT_EQ(0, context.width_);
    EXPECT_EQ(0, context.height_);
    EXPECT_EQ(0u, context.bc_index_);
    EXPECT_EQ(0u, context.pixel_aspect_);
  }
  // LSD with no global palette
  {
    CFX_GifLocalScreenDescriptor lsd{0x0A00, 0x000F, {0, 0, 0, 0}, 1, 2};
    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&lsd),
                           sizeof(CFX_GifLocalScreenDescriptor));

    EXPECT_EQ(CFX_GifDecodeStatus::Success,
              wrapper.ReadLogicalScreenDescriptor());

    EXPECT_EQ(sizeof(CFX_GifLocalScreenDescriptor), context.skip_size_);
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(1u, context.bc_index_);
    EXPECT_EQ(2u, context.pixel_aspect_);
  }
  // LSD with global palette bit set, but no global palette
  {
    CFX_GifLocalScreenDescriptor lsd{0x0A00, 0x000F, {0, 0, 0, 1}, 1, 2};
    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&lsd),
                           sizeof(CFX_GifLocalScreenDescriptor));

    EXPECT_EQ(CFX_GifDecodeStatus::Unfinished,
              wrapper.ReadLogicalScreenDescriptor());

    EXPECT_EQ(0u, context.skip_size_);
  }
  // LSD with global palette
  {
    CFX_GifLocalScreenDescriptor lsd = {0x0A00, 0x000F, {1, 1, 2, 1}, 1, 2};
    CFX_GifPalette palette[4] = {{0, 0, 0}, {1, 1, 1}, {1, 0, 0}, {0, 0, 1}};
    struct {
      CFX_GifLocalScreenDescriptor lsd;
      CFX_GifPalette palette[4];
    } data;
    memcpy(&data.lsd, &lsd, sizeof(data.lsd));
    memcpy(&data.palette, &palette, sizeof(data.lsd));
    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&data), sizeof(data));

    EXPECT_EQ(CFX_GifDecodeStatus::Success,
              wrapper.ReadLogicalScreenDescriptor());

    EXPECT_EQ(sizeof(data), context.skip_size_);
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(1u, context.bc_index_);
    EXPECT_EQ(2u, context.pixel_aspect_);
    EXPECT_EQ(1u, context.global_pal_exp_);
    EXPECT_EQ(1, context.global_sort_flag_);
    EXPECT_EQ(2, context.global_color_resolution_);
    EXPECT_TRUE(0 == memcmp(data.palette, context.global_palette_.data(),
                            sizeof(data.palette)));
  }
}

TEST(CFX_GifContext, ReadHeader) {
  CFX_GifContext context(nullptr, nullptr);
  CFX_GifContextTestWrapper wrapper(&context);
  // Bad signature
  {
    uint8_t signature[] = {'N', 'O', 'T', 'G', 'I', 'F'};
    CFX_GifLocalScreenDescriptor lsd{0x0A00, 0x000F, {0, 0, 0, 0}, 1, 2};
    struct {
      uint8_t signature[6];
      CFX_GifLocalScreenDescriptor lsd;
    } data;
    memcpy(&data.signature, signature, sizeof(data.signature));
    memcpy(&data.lsd, &lsd, sizeof(data.lsd));
    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&data), sizeof(data));

    EXPECT_EQ(CFX_GifDecodeStatus::Error, context.ReadHeader());

    EXPECT_EQ(sizeof(signature), context.skip_size_);
  }
  // Short after signature
  {
    uint8_t signature[] = {'G', 'I', 'F', '8', '7', 'a'};
    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&signature),
                           sizeof(signature));

    EXPECT_EQ(CFX_GifDecodeStatus::Unfinished, context.ReadHeader());

    EXPECT_EQ(sizeof(signature), context.skip_size_);
  }
  // Success without global palette
  {
    uint8_t signature[] = {'G', 'I', 'F', '8', '7', 'a'};
    CFX_GifLocalScreenDescriptor lsd{0x0A00, 0x000F, {0, 0, 0, 0}, 1, 2};
    struct {
      uint8_t signature[6];
      CFX_GifLocalScreenDescriptor lsd;
    } data;
    memcpy(&data.signature, signature, sizeof(data.signature));
    memcpy(&data.lsd, &lsd, sizeof(data.lsd));

    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&data), sizeof(data));

    EXPECT_EQ(CFX_GifDecodeStatus::Success, context.ReadHeader());

    EXPECT_EQ(sizeof(data), context.skip_size_);
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(1u, context.bc_index_);
    EXPECT_EQ(2u, context.pixel_aspect_);
  }
  // Missing Global Palette
  {
    uint8_t signature[] = {'G', 'I', 'F', '8', '7', 'a'};
    CFX_GifLocalScreenDescriptor lsd{0x0A00, 0x000F, {0, 0, 0, 1}, 1, 2};

    struct {
      uint8_t signature[6];
      CFX_GifLocalScreenDescriptor lsd;
    } data;
    memcpy(&data.signature, signature, sizeof(data.signature));
    memcpy(&data.lsd, &lsd, sizeof(data.lsd));

    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&data), sizeof(data));

    EXPECT_EQ(CFX_GifDecodeStatus::Unfinished, context.ReadHeader());

    EXPECT_EQ(sizeof(signature), context.skip_size_);
  }
  // Success with global palette
  {
    uint8_t signature[] = {'G', 'I', 'F', '8', '7', 'a'};
    CFX_GifLocalScreenDescriptor lsd = {0x0A00, 0x000F, {1, 1, 2, 1}, 1, 2};
    CFX_GifPalette palette[4] = {{0, 0, 0}, {1, 1, 1}, {1, 0, 0}, {0, 0, 1}};

    struct {
      uint8_t signature[6];
      CFX_GifLocalScreenDescriptor lsd;
      CFX_GifPalette palette[4];
    } data;
    memcpy(&data.signature, signature, sizeof(data.signature));
    memcpy(&data.lsd, &lsd, sizeof(data.lsd));
    memcpy(&data.palette, &palette, sizeof(data.lsd));

    context.SetInputBuffer(reinterpret_cast<uint8_t*>(&data), sizeof(data));

    EXPECT_EQ(CFX_GifDecodeStatus::Success, context.ReadHeader());

    EXPECT_EQ(sizeof(data), context.skip_size_);
    EXPECT_EQ(0x000A, context.width_);
    EXPECT_EQ(0x0F00, context.height_);
    EXPECT_EQ(1u, context.bc_index_);
    EXPECT_EQ(2u, context.pixel_aspect_);
    EXPECT_EQ(1u, context.global_pal_exp_);
    EXPECT_EQ(1, context.global_sort_flag_);
    EXPECT_EQ(2, context.global_color_resolution_);
    EXPECT_TRUE(0 == memcmp(data.palette, context.global_palette_.data(),
                            sizeof(data.palette)));
  }
}
