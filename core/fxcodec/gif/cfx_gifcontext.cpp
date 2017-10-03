// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/gif/cfx_gifcontext.h"

#include <algorithm>
#include <utility>

#include "core/fxcodec/codec/ccodec_gifmodule.h"
#include "core/fxcodec/gif/cfx_gif.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

namespace {

const int32_t s_gif_interlace_step[4] = {8, 8, 4, 2};

}  // namespace

CFX_GifContext::CFX_GifContext(CCodec_GifModule* gif_module,
                               CCodec_GifModule::Delegate* delegate)
    : gif_module_(gif_module),
      delegate_(delegate),
      global_pal_exp_(0),
      img_row_offset_(0),
      img_row_avail_size_(0),
      avail_in_(0),
      decode_status_(GIF_D_STATUS_SIG),
      skip_size_(0),
      next_in_(nullptr),
      width_(0),
      height_(0),
      bc_index_(0),
      pixel_aspect_(0),
      global_sort_flag_(0),
      global_color_resolution_(0),
      img_pass_num_(0) {}

CFX_GifContext::~CFX_GifContext() {}

void CFX_GifContext::RecordCurrentPosition(uint32_t* cur_pos) {
  delegate_->GifRecordCurrentPosition(*cur_pos);
}

void CFX_GifContext::ReadScanline(int32_t row_num, uint8_t* row_buf) {
  delegate_->GifReadScanline(row_num, row_buf);
}

bool CFX_GifContext::GetRecordPosition(uint32_t cur_pos,
                                       int32_t left,
                                       int32_t top,
                                       int32_t width,
                                       int32_t height,
                                       int32_t pal_num,
                                       CFX_GifPalette* pal,
                                       int32_t delay_time,
                                       bool user_input,
                                       int32_t trans_index,
                                       int32_t disposal_method,
                                       bool interlace) {
  return delegate_->GifInputRecordPositionBuf(
      cur_pos, FX_RECT(left, top, left + width, top + height), pal_num, pal,
      delay_time, user_input, trans_index, disposal_method, interlace);
}

CFX_GifDecodeStatus CFX_GifContext::ReadHeader() {
  CFX_GifDecodeStatus status = ReadGifSignature();
  if (status != CFX_GifDecodeStatus::Success)
    return status;
  return ReadLogicalScreenDescriptor();
}

CFX_GifDecodeStatus CFX_GifContext::GetFrame() {
  CFX_GifDecodeStatus ret = CFX_GifDecodeStatus::Success;
  while (true) {
    switch (decode_status_) {
      case GIF_D_STATUS_TAIL:
        return CFX_GifDecodeStatus::Success;
      case GIF_D_STATUS_SIG: {
        uint8_t* signature = nullptr;
        if (!ReadData(&signature, 1))
          return CFX_GifDecodeStatus::Unfinished;

        switch (*signature) {
          case GIF_SIG_EXTENSION:
            SaveDecodingStatus(GIF_D_STATUS_EXT);
            continue;
          case GIF_SIG_IMAGE:
            SaveDecodingStatus(GIF_D_STATUS_IMG_INFO);
            continue;
          case GIF_SIG_TRAILER:
            SaveDecodingStatus(GIF_D_STATUS_TAIL);
            return CFX_GifDecodeStatus::Success;
          default:
            if (avail_in_) {
              // The Gif File has non_standard Tag!
              SaveDecodingStatus(GIF_D_STATUS_SIG);
              continue;
            }
            // The Gif File Doesn't have Trailer Tag!
            return CFX_GifDecodeStatus::Success;
        }
      }
      case GIF_D_STATUS_EXT: {
        uint8_t* extension = nullptr;
        if (!ReadData(&extension, 1))
          return CFX_GifDecodeStatus::Unfinished;

        switch (*extension) {
          case GIF_BLOCK_CE:
            SaveDecodingStatus(GIF_D_STATUS_EXT_CE);
            continue;
          case GIF_BLOCK_GCE:
            SaveDecodingStatus(GIF_D_STATUS_EXT_GCE);
            continue;
          case GIF_BLOCK_PTE:
            SaveDecodingStatus(GIF_D_STATUS_EXT_PTE);
            continue;
          default: {
            int32_t status = GIF_D_STATUS_EXT_UNE;
            if (*extension == GIF_BLOCK_PTE) {
              status = GIF_D_STATUS_EXT_PTE;
            }
            SaveDecodingStatus(status);
            continue;
          }
        }
      }
      case GIF_D_STATUS_IMG_INFO: {
        ret = DecodeImageInfo();
        if (ret != CFX_GifDecodeStatus::Success)
          return ret;

        continue;
      }
      case GIF_D_STATUS_IMG_DATA: {
        uint8_t* img_data_size = nullptr;
        uint8_t* img_data = nullptr;
        uint32_t skip_size_org = skip_size_;
        if (!ReadData(&img_data_size, 1))
          return CFX_GifDecodeStatus::Unfinished;

        while (*img_data_size != GIF_BLOCK_TERMINAL) {
          if (!ReadData(&img_data, *img_data_size)) {
            skip_size_ = skip_size_org;
            return CFX_GifDecodeStatus::Unfinished;
          }

          SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
          skip_size_org = skip_size_;
          if (!ReadData(&img_data_size, 1))
            return CFX_GifDecodeStatus::Unfinished;
        }
        SaveDecodingStatus(GIF_D_STATUS_SIG);
        continue;
      }
      default: {
        ret = DecodeExtension();
        if (ret != CFX_GifDecodeStatus::Success)
          return ret;
        break;
      }
    }
  }
  return CFX_GifDecodeStatus::Success;
}

CFX_GifDecodeStatus CFX_GifContext::LoadFrame(int32_t frame_num) {
  if (!pdfium::IndexInBounds(images_, frame_num))
    return CFX_GifDecodeStatus::Error;

  uint8_t* img_data_size = nullptr;
  uint8_t* img_data = nullptr;
  uint32_t skip_size_org = skip_size_;
  CFX_GifImage* gif_image = images_[static_cast<size_t>(frame_num)].get();
  uint32_t gif_img_row_bytes = gif_image->image_info.width;
  if (gif_img_row_bytes == 0)
    return CFX_GifDecodeStatus::Error;

  if (decode_status_ == GIF_D_STATUS_TAIL) {
    gif_image->row_buffer.resize(gif_img_row_bytes);
    CFX_GifGraphicControlExtension* gif_img_gce = gif_image->image_GCE.get();
    int32_t loc_pal_num =
        gif_image->image_info.local_flags.local_pal
            ? (2 << gif_image->image_info.local_flags.pal_bits)
            : 0;
    avail_in_ = 0;
    CFX_GifPalette* pLocalPalette = gif_image->local_palettes.empty()
                                        ? nullptr
                                        : gif_image->local_palettes.data();
    if (!gif_img_gce) {
      bool bRes = GetRecordPosition(
          gif_image->data_pos, gif_image->image_info.left,
          gif_image->image_info.top, gif_image->image_info.width,
          gif_image->image_info.height, loc_pal_num, pLocalPalette, 0, 0, -1, 0,
          gif_image->image_info.local_flags.interlace);
      if (!bRes) {
        gif_image->row_buffer.clear();
        return CFX_GifDecodeStatus::Error;
      }
    } else {
      bool bRes = GetRecordPosition(
          gif_image->data_pos, gif_image->image_info.left,
          gif_image->image_info.top, gif_image->image_info.width,
          gif_image->image_info.height, loc_pal_num, pLocalPalette,
          static_cast<int32_t>(gif_image->image_GCE->delay_time),
          gif_image->image_GCE->gce_flags.user_input,
          gif_image->image_GCE->gce_flags.transparency
              ? static_cast<int32_t>(gif_image->image_GCE->trans_index)
              : -1,
          static_cast<int32_t>(gif_image->image_GCE->gce_flags.disposal_method),
          gif_image->image_info.local_flags.interlace);
      if (!bRes) {
        gif_image->row_buffer.clear();
        return CFX_GifDecodeStatus::Error;
      }
    }

    if (gif_image->code_exp > GIF_MAX_LZW_EXP) {
      gif_image->row_buffer.clear();
      return CFX_GifDecodeStatus::Error;
    }

    img_row_offset_ = 0;
    img_row_avail_size_ = 0;
    img_pass_num_ = 0;
    gif_image->row_num = 0;
    SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
  }

  if (decode_status_ == GIF_D_STATUS_IMG_DATA) {
    if (!ReadData(&img_data_size, 1))
      return CFX_GifDecodeStatus::Unfinished;

    if (*img_data_size != GIF_BLOCK_TERMINAL) {
      if (!ReadData(&img_data, *img_data_size)) {
        skip_size_ = skip_size_org;
        return CFX_GifDecodeStatus::Unfinished;
      }

      if (!lzw_decompressor_.get())
        lzw_decompressor_ = CFX_LZWDecompressor::Create(
            !gif_image->local_palettes.empty() ? gif_image->local_pallette_exp
                                               : global_pal_exp_,
            gif_image->code_exp);
      SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
      img_row_offset_ += img_row_avail_size_;
      img_row_avail_size_ = gif_img_row_bytes - img_row_offset_;
      CFX_GifDecodeStatus ret =
          lzw_decompressor_.get()
              ? lzw_decompressor_->Decode(
                    img_data, *img_data_size,
                    gif_image->row_buffer.data() + img_row_offset_,
                    &img_row_avail_size_)
              : CFX_GifDecodeStatus::Error;
      if (ret == CFX_GifDecodeStatus::Error) {
        DecodingFailureAtTailCleanup(gif_image);
        return CFX_GifDecodeStatus::Error;
      }
      while (ret != CFX_GifDecodeStatus::Error) {
        if (ret == CFX_GifDecodeStatus::Success) {
          ReadScanline(gif_image->row_num, gif_image->row_buffer.data());
          gif_image->row_buffer.clear();
          SaveDecodingStatus(GIF_D_STATUS_TAIL);
          return CFX_GifDecodeStatus::Success;
        }
        if (ret == CFX_GifDecodeStatus::Unfinished) {
          skip_size_org = skip_size_;
          if (!ReadData(&img_data_size, 1))
            return CFX_GifDecodeStatus::Unfinished;

          if (*img_data_size != GIF_BLOCK_TERMINAL) {
            if (!ReadData(&img_data, *img_data_size)) {
              skip_size_ = skip_size_org;
              return CFX_GifDecodeStatus::Unfinished;
            }
            if (!lzw_decompressor_.get())
              lzw_decompressor_ = CFX_LZWDecompressor::Create(
                  !gif_image->local_palettes.empty()
                      ? gif_image->local_pallette_exp
                      : global_pal_exp_,
                  gif_image->code_exp);
            SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
            img_row_offset_ += img_row_avail_size_;
            img_row_avail_size_ = gif_img_row_bytes - img_row_offset_;
            ret = lzw_decompressor_.get()
                      ? lzw_decompressor_->Decode(
                            img_data, *img_data_size,
                            gif_image->row_buffer.data() + img_row_offset_,
                            &img_row_avail_size_)
                      : CFX_GifDecodeStatus::Error;
          }
        }
        if (ret == CFX_GifDecodeStatus::InsufficientDestSize) {
          if (gif_image->image_info.local_flags.interlace) {
            ReadScanline(gif_image->row_num, gif_image->row_buffer.data());
            gif_image->row_num += s_gif_interlace_step[img_pass_num_];
            if (gif_image->row_num >=
                static_cast<int32_t>(gif_image->image_info.height)) {
              img_pass_num_++;
              if (img_pass_num_ == FX_ArraySize(s_gif_interlace_step)) {
                DecodingFailureAtTailCleanup(gif_image);
                return CFX_GifDecodeStatus::Error;
              }
              gif_image->row_num = s_gif_interlace_step[img_pass_num_] / 2;
            }
          } else {
            ReadScanline(gif_image->row_num++, gif_image->row_buffer.data());
          }
          img_row_offset_ = 0;
          img_row_avail_size_ = gif_img_row_bytes;
          ret = lzw_decompressor_.get()
                    ? lzw_decompressor_->Decode(
                          img_data, *img_data_size,
                          gif_image->row_buffer.data() + img_row_offset_,
                          &img_row_avail_size_)
                    : CFX_GifDecodeStatus::Error;
        }
        if (ret == CFX_GifDecodeStatus::Error) {
          DecodingFailureAtTailCleanup(gif_image);
          return CFX_GifDecodeStatus::Error;
        }
      }
    }
    SaveDecodingStatus(GIF_D_STATUS_TAIL);
  }
  return CFX_GifDecodeStatus::Error;
}

void CFX_GifContext::SetInputBuffer(uint8_t* src_buf, uint32_t src_size) {
  next_in_ = src_buf;
  avail_in_ = src_size;
  skip_size_ = 0;
}

uint32_t CFX_GifContext::GetAvailInput(uint8_t** avail_buf) const {
  if (avail_buf) {
    *avail_buf = nullptr;
    if (avail_in_ > 0)
      *avail_buf = next_in_;
  }
  return avail_in_;
}

int32_t CFX_GifContext::GetFrameNum() const {
  return pdfium::CollectionSize<int32_t>(images_);
}

uint8_t* CFX_GifContext::ReadData(uint8_t** des_buf_pp, uint32_t data_size) {
  if (!next_in_)
    return nullptr;
  if (avail_in_ <= skip_size_)
    return nullptr;
  if (!des_buf_pp)
    return nullptr;
  if (data_size == 0)
    return nullptr;
  if (avail_in_ - skip_size_ < data_size)
    return nullptr;

  *des_buf_pp = next_in_ + skip_size_;
  skip_size_ += data_size;
  return *des_buf_pp;
}

CFX_GifDecodeStatus CFX_GifContext::ReadGifSignature() {
  CFX_GifHeader* header = nullptr;
  uint32_t skip_size_org = skip_size_;
  if (!ReadData(reinterpret_cast<uint8_t**>(&header), 6)) {
    skip_size_ = skip_size_org;
    return CFX_GifDecodeStatus::Unfinished;
  }

  if (strncmp(header->signature, kGifSignature87, 6) != 0 &&
      strncmp(header->signature, kGifSignature89, 6) != 0)
    return CFX_GifDecodeStatus::Error;

  return CFX_GifDecodeStatus::Success;
}

CFX_GifDecodeStatus CFX_GifContext::ReadLogicalScreenDescriptor() {
  CFX_GifLocalScreenDescriptor* lsd = nullptr;
  uint32_t skip_size_org = skip_size_;
  if (!ReadData(reinterpret_cast<uint8_t**>(&lsd), 7)) {
    skip_size_ = skip_size_org;
    return CFX_GifDecodeStatus::Unfinished;
  }

  if (lsd->global_flags.global_pal) {
    uint32_t global_pal_size = unsigned(2 << lsd->global_flags.pal_bits) * 3u;
    uint8_t* global_pal = nullptr;
    if (!ReadData(&global_pal, global_pal_size)) {
      skip_size_ = skip_size_org;
      return CFX_GifDecodeStatus::Unfinished;
    }

    global_pal_exp_ = lsd->global_flags.pal_bits;
    global_sort_flag_ = lsd->global_flags.sort_flag;
    global_color_resolution_ = lsd->global_flags.color_resolution;
    global_palette_.resize(global_pal_size / 3);
    memcpy(global_palette_.data(), global_pal, global_pal_size);
  }

  width_ = static_cast<int>(
      FXWORD_GET_MSBFIRST(reinterpret_cast<uint8_t*>(&lsd->width)));
  height_ = static_cast<int>(
      FXWORD_GET_MSBFIRST(reinterpret_cast<uint8_t*>(&lsd->height)));
  bc_index_ = lsd->bc_index;
  pixel_aspect_ = lsd->pixel_aspect;
  return CFX_GifDecodeStatus::Success;
}

void CFX_GifContext::SaveDecodingStatus(int32_t status) {
  decode_status_ = status;
  next_in_ += skip_size_;
  avail_in_ -= skip_size_;
  skip_size_ = 0;
}

CFX_GifDecodeStatus CFX_GifContext::DecodeExtension() {
  uint8_t* data_size = nullptr;
  uint8_t* data_buf = nullptr;
  uint32_t skip_size_org = skip_size_;
  switch (decode_status_) {
    case GIF_D_STATUS_EXT_CE: {
      if (!ReadData(&data_size, 1)) {
        skip_size_ = skip_size_org;
        return CFX_GifDecodeStatus::Unfinished;
      }

      cmt_data_.clear();
      while (*data_size != GIF_BLOCK_TERMINAL) {
        uint8_t block_size = *data_size;
        if (!ReadData(&data_buf, *data_size) || !ReadData(&data_size, 1)) {
          skip_size_ = skip_size_org;
          return CFX_GifDecodeStatus::Unfinished;
        }

        cmt_data_ += ByteString(data_buf, block_size);
      }
      break;
    }
    case GIF_D_STATUS_EXT_PTE: {
      CFX_GifPlainTextExtension* gif_pte = nullptr;
      if (!ReadData(reinterpret_cast<uint8_t**>(&gif_pte), 13))
        return CFX_GifDecodeStatus::Unfinished;

      graphic_control_extension_ = nullptr;
      if (!ReadData(&data_size, 1)) {
        skip_size_ = skip_size_org;
        return CFX_GifDecodeStatus::Unfinished;
      }

      while (*data_size != GIF_BLOCK_TERMINAL) {
        if (!ReadData(&data_buf, *data_size) || !ReadData(&data_size, 1)) {
          skip_size_ = skip_size_org;
          return CFX_GifDecodeStatus::Unfinished;
        }
      }
      break;
    }
    case GIF_D_STATUS_EXT_GCE: {
      CFX_GifGraphicControlExtension* gif_gce = nullptr;
      if (!ReadData(reinterpret_cast<uint8_t**>(&gif_gce), 6))
        return CFX_GifDecodeStatus::Unfinished;

      if (!graphic_control_extension_.get())
        graphic_control_extension_ =
            pdfium::MakeUnique<CFX_GifGraphicControlExtension>();
      graphic_control_extension_->block_size = gif_gce->block_size;
      graphic_control_extension_->gce_flags = gif_gce->gce_flags;
      graphic_control_extension_->delay_time =
          FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&gif_gce->delay_time));
      graphic_control_extension_->trans_index = gif_gce->trans_index;
      break;
    }
    default: {
      if (decode_status_ == GIF_D_STATUS_EXT_PTE)
        graphic_control_extension_ = nullptr;
      if (!ReadData(&data_size, 1))
        return CFX_GifDecodeStatus::Unfinished;

      while (*data_size != GIF_BLOCK_TERMINAL) {
        if (!ReadData(&data_buf, *data_size) || !ReadData(&data_size, 1)) {
          skip_size_ = skip_size_org;
          return CFX_GifDecodeStatus::Unfinished;
        }
      }
    }
  }
  SaveDecodingStatus(GIF_D_STATUS_SIG);
  return CFX_GifDecodeStatus::Success;
}

CFX_GifDecodeStatus CFX_GifContext::DecodeImageInfo() {
  if (width_ <= 0 || height_ <= 0)
    return CFX_GifDecodeStatus::Error;

  uint32_t skip_size_org = skip_size_;
  CFX_CFX_GifImageInfo* img_info = nullptr;
  if (!ReadData(reinterpret_cast<uint8_t**>(&img_info), 9))
    return CFX_GifDecodeStatus::Unfinished;

  auto gif_image = pdfium::MakeUnique<CFX_GifImage>();
  gif_image->image_info.left =
      FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&img_info->left));
  gif_image->image_info.top =
      FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&img_info->top));
  gif_image->image_info.width =
      FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&img_info->width));
  gif_image->image_info.height =
      FXWORD_GET_LSBFIRST(reinterpret_cast<uint8_t*>(&img_info->height));
  gif_image->image_info.local_flags = img_info->local_flags;
  if (gif_image->image_info.left + gif_image->image_info.width > width_ ||
      gif_image->image_info.top + gif_image->image_info.height > height_)
    return CFX_GifDecodeStatus::Error;

  CFX_GifLocalFlags* gif_img_info_lf = &img_info->local_flags;
  if (gif_img_info_lf->local_pal) {
    gif_image->local_pallette_exp = gif_img_info_lf->pal_bits;
    uint32_t loc_pal_size = unsigned(2 << gif_img_info_lf->pal_bits) * 3u;
    uint8_t* loc_pal = nullptr;
    if (!ReadData(&loc_pal, loc_pal_size)) {
      skip_size_ = skip_size_org;
      return CFX_GifDecodeStatus::Unfinished;
    }

    gif_image->local_palettes = std::vector<CFX_GifPalette>(loc_pal_size / 3);
    std::copy(loc_pal, loc_pal + loc_pal_size,
              reinterpret_cast<uint8_t*>(gif_image->local_palettes.data()));
  }

  uint8_t* code_size = nullptr;
  if (!ReadData(&code_size, 1)) {
    skip_size_ = skip_size_org;
    return CFX_GifDecodeStatus::Unfinished;
  }

  gif_image->code_exp = *code_size;
  RecordCurrentPosition(&gif_image->data_pos);
  gif_image->data_pos += skip_size_;
  gif_image->image_GCE = nullptr;
  if (graphic_control_extension_.get()) {
    gif_image->image_GCE = std::move(graphic_control_extension_);
    graphic_control_extension_ = nullptr;
  }
  images_.push_back(std::move(gif_image));
  SaveDecodingStatus(GIF_D_STATUS_IMG_DATA);
  return CFX_GifDecodeStatus::Success;
}

void CFX_GifContext::DecodingFailureAtTailCleanup(CFX_GifImage* gif_image) {
  gif_image->row_buffer.clear();
  SaveDecodingStatus(GIF_D_STATUS_TAIL);
}
