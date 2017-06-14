// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/lbmp/fx_bmp.h"

#include <algorithm>
#include <limits>

namespace {

const size_t kBmpCoreHeaderSize = 12;
const size_t kBmpInfoHeaderSize = 40;

// TODO(thestig): Replace with FXDWORD_GET_LSBFIRST?
uint32_t GetDWord_LSBFirst(uint8_t* p) {
  return p[0] | (p[1] << 8) | (p[2] << 16) | (p[3] << 24);
}

void SetDWord_LSBFirst(uint8_t* p, uint32_t v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
  p[2] = (uint8_t)(v >> 16);
  p[3] = (uint8_t)(v >> 24);
}

uint8_t HalfRoundUp(uint8_t value) {
  uint16_t value16 = value;
  return static_cast<uint8_t>((value16 + 1) / 2);
}

}  // namespace

uint16_t GetWord_LSBFirst(uint8_t* p) {
  return p[0] | (p[1] << 8);
}

void SetWord_LSBFirst(uint8_t* p, uint16_t v) {
  p[0] = (uint8_t)v;
  p[1] = (uint8_t)(v >> 8);
}

BMPDecompressor* bmp_create_decompress() {
  BMPDecompressor* bmp_ptr = FX_Alloc(BMPDecompressor, 1);
  memset(bmp_ptr, 0, sizeof(BMPDecompressor));
  bmp_ptr->decode_status = BMP_D_STATUS_HEADER;
  bmp_ptr->bmp_header_ptr = FX_Alloc(BmpFileHeader, 1);
  return bmp_ptr;
}

void bmp_destroy_decompress(BMPDecompressor** bmp_ptr_ptr) {
  if (!bmp_ptr_ptr || !*bmp_ptr_ptr)
    return;

  BMPDecompressor* bmp_ptr = *bmp_ptr_ptr;
  *bmp_ptr_ptr = nullptr;
  FX_Free(bmp_ptr->out_row_buffer);
  FX_Free(bmp_ptr->pal_ptr);
  FX_Free(bmp_ptr->bmp_header_ptr);
  FX_Free(bmp_ptr);
}

void BMPDecompressor::Error(const char* err_msg) {
  strncpy(err_ptr, err_msg, BMP_MAX_ERROR_SIZE - 1);
  longjmp(jmpbuf, 1);
}

void BMPDecompressor::ReadScanline(int32_t row_num, uint8_t* row_buf) {
  auto* p = reinterpret_cast<CBmpContext*>(context_ptr);
  p->m_pDelegate->BmpReadScanline(row_num, row_buf);
}

bool BMPDecompressor::GetDataPosition(uint32_t rcd_pos) {
  auto* p = reinterpret_cast<CBmpContext*>(context_ptr);
  return p->m_pDelegate->BmpInputImagePositionBuf(rcd_pos);
}

int32_t BMPDecompressor::ReadHeader() {
  uint32_t skip_size_org = skip_size;
  if (decode_status == BMP_D_STATUS_HEADER) {
    ASSERT(sizeof(BmpFileHeader) == 14);
    BmpFileHeader* bmp_header_ptr = nullptr;
    if (!ReadData((uint8_t**)&bmp_header_ptr, 14))
      return 2;

    bmp_header_ptr->bfType =
        GetWord_LSBFirst((uint8_t*)&bmp_header_ptr->bfType);
    bmp_header_ptr->bfOffBits =
        GetDWord_LSBFirst((uint8_t*)&bmp_header_ptr->bfOffBits);
    data_size = GetDWord_LSBFirst((uint8_t*)&bmp_header_ptr->bfSize);
    if (bmp_header_ptr->bfType != BMP_SIGNATURE) {
      Error("Not A Bmp Image");
      NOTREACHED();
    }
    if (avail_in < sizeof(uint32_t)) {
      skip_size = skip_size_org;
      return 2;
    }
    img_ifh_size = GetDWord_LSBFirst(next_in + skip_size);
    pal_type = 0;
    static_assert(sizeof(BmpCoreHeader) == kBmpCoreHeaderSize,
                  "BmpCoreHeader has wrong size");
    static_assert(sizeof(BmpInfoHeader) == kBmpInfoHeaderSize,
                  "BmpInfoHeader has wrong size");
    switch (img_ifh_size) {
      case kBmpCoreHeaderSize: {
        pal_type = 1;
        BmpCoreHeaderPtr bmp_core_header_ptr = nullptr;
        if (!ReadData((uint8_t**)&bmp_core_header_ptr, img_ifh_size)) {
          skip_size = skip_size_org;
          return 2;
        }
        width = GetWord_LSBFirst((uint8_t*)&bmp_core_header_ptr->bcWidth);
        height = GetWord_LSBFirst((uint8_t*)&bmp_core_header_ptr->bcHeight);
        bitCounts =
            GetWord_LSBFirst((uint8_t*)&bmp_core_header_ptr->bcBitCount);
        compress_flag = BMP_RGB;
        imgTB_flag = false;
      } break;
      case kBmpInfoHeaderSize: {
        BmpInfoHeaderPtr bmp_info_header_ptr = nullptr;
        if (!ReadData((uint8_t**)&bmp_info_header_ptr, img_ifh_size)) {
          skip_size = skip_size_org;
          return 2;
        }
        width = GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biWidth);
        height = GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biHeight);
        bitCounts =
            GetWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biBitCount);
        compress_flag =
            GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biCompression);
        color_used =
            GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biClrUsed);
        dpi_x = (int32_t)GetDWord_LSBFirst(
            (uint8_t*)&bmp_info_header_ptr->biXPelsPerMeter);
        dpi_y = (int32_t)GetDWord_LSBFirst(
            (uint8_t*)&bmp_info_header_ptr->biYPelsPerMeter);
        if (height < 0) {
          if (height == std::numeric_limits<int>::min()) {
            Error("Unsupported height");
            NOTREACHED();
          }
          height = -height;
          imgTB_flag = true;
        }
      } break;
      default: {
        if (img_ifh_size >
            std::min(kBmpInfoHeaderSize, sizeof(BmpInfoHeader))) {
          BmpInfoHeaderPtr bmp_info_header_ptr = nullptr;
          if (!ReadData((uint8_t**)&bmp_info_header_ptr, img_ifh_size)) {
            skip_size = skip_size_org;
            return 2;
          }
          uint16_t biPlanes;
          width = GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biWidth);
          height = GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biHeight);
          bitCounts =
              GetWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biBitCount);
          compress_flag =
              GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biCompression);
          color_used =
              GetDWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biClrUsed);
          biPlanes = GetWord_LSBFirst((uint8_t*)&bmp_info_header_ptr->biPlanes);
          dpi_x = GetDWord_LSBFirst(
              (uint8_t*)&bmp_info_header_ptr->biXPelsPerMeter);
          dpi_y = GetDWord_LSBFirst(
              (uint8_t*)&bmp_info_header_ptr->biYPelsPerMeter);
          if (height < 0) {
            if (height == std::numeric_limits<int>::min()) {
              Error("Unsupported height");
              NOTREACHED();
            }
            height = -height;
            imgTB_flag = true;
          }
          if (compress_flag == BMP_RGB && biPlanes == 1 && color_used == 0) {
            break;
          }
        }
        Error("Unsupported Bmp File");
        NOTREACHED();
      }
    }
    if (width <= 0 || width > BMP_MAX_WIDTH || compress_flag > BMP_BITFIELDS) {
      Error("The Bmp File Is Corrupt");
      NOTREACHED();
    }
    switch (bitCounts) {
      case 1:
      case 4:
      case 8:
      case 16:
      case 24: {
        if (color_used > ((uint32_t)1) << bitCounts) {
          Error("The Bmp File Is Corrupt");
          NOTREACHED();
        }
      }
      case 32:
        break;
      default:
        Error("The Bmp File Is Corrupt");
        NOTREACHED();
    }
    src_row_bytes = BMP_WIDTHBYTES(width, bitCounts);
    switch (bitCounts) {
      case 1:
      case 4:
      case 8:
        out_row_bytes = BMP_WIDTHBYTES(width, 8);
        components = 1;
        break;
      case 16:
      case 24:
        out_row_bytes = BMP_WIDTHBYTES(width, 24);
        components = 3;
        break;
      case 32:
        out_row_bytes = src_row_bytes;
        components = 4;
        break;
    }
    FX_Free(out_row_buffer);
    out_row_buffer = nullptr;

    if (out_row_bytes <= 0) {
      Error("The Bmp File Is Corrupt");
      NOTREACHED();
    }

    out_row_buffer = FX_Alloc(uint8_t, out_row_bytes);
    memset(out_row_buffer, 0, out_row_bytes);
    SaveDecodingStatus(BMP_D_STATUS_PAL);
  }
  if (decode_status == BMP_D_STATUS_PAL) {
    skip_size_org = skip_size;
    if (compress_flag == BMP_BITFIELDS) {
      if (bitCounts != 16 && bitCounts != 32) {
        Error("The Bmp File Is Corrupt");
        NOTREACHED();
      }
      uint32_t* mask;
      if (ReadData((uint8_t**)&mask, 3 * sizeof(uint32_t)) == nullptr) {
        skip_size = skip_size_org;
        return 2;
      }
      mask_red = GetDWord_LSBFirst((uint8_t*)&mask[0]);
      mask_green = GetDWord_LSBFirst((uint8_t*)&mask[1]);
      mask_blue = GetDWord_LSBFirst((uint8_t*)&mask[2]);
      if (mask_red & mask_green || mask_red & mask_blue ||
          mask_green & mask_blue) {
        Error("The Bitfield Bmp File Is Corrupt");
        NOTREACHED();
      }
      if (bmp_header_ptr->bfOffBits < 26 + img_ifh_size) {
        bmp_header_ptr->bfOffBits = 26 + img_ifh_size;
      }
      SaveDecodingStatus(BMP_D_STATUS_DATA_PRE);
      return 1;
    } else if (bitCounts == 16) {
      mask_red = 0x7C00;
      mask_green = 0x03E0;
      mask_blue = 0x001F;
    }
    pal_num = 0;
    if (bitCounts < 16) {
      pal_num = 1 << bitCounts;
      if (color_used != 0) {
        pal_num = color_used;
      }
      uint8_t* src_pal_ptr = nullptr;
      uint32_t src_pal_size = pal_num * (pal_type ? 3 : 4);
      if (ReadData((uint8_t**)&src_pal_ptr, src_pal_size) == nullptr) {
        skip_size = skip_size_org;
        return 2;
      }
      FX_Free(pal_ptr);
      pal_ptr = FX_Alloc(uint32_t, pal_num);
      int32_t src_pal_index = 0;
      if (pal_type == BMP_PAL_OLD) {
        while (src_pal_index < pal_num) {
          pal_ptr[src_pal_index++] = BMP_PAL_ENCODE(
              0x00, src_pal_ptr[2], src_pal_ptr[1], src_pal_ptr[0]);
          src_pal_ptr += 3;
        }
      } else {
        while (src_pal_index < pal_num) {
          pal_ptr[src_pal_index++] = BMP_PAL_ENCODE(
              src_pal_ptr[3], src_pal_ptr[2], src_pal_ptr[1], src_pal_ptr[0]);
          src_pal_ptr += 4;
        }
      }
    }
    if (bmp_header_ptr->bfOffBits <
        14 + img_ifh_size + pal_num * (pal_type ? 3 : 4)) {
      bmp_header_ptr->bfOffBits =
          14 + img_ifh_size + pal_num * (pal_type ? 3 : 4);
    }
    SaveDecodingStatus(BMP_D_STATUS_DATA_PRE);
  }
  return 1;
}

bool BMPDecompressor::ValidateFlag() const {
  switch (compress_flag) {
    case BMP_RGB:
    case BMP_BITFIELDS:
    case BMP_RLE8:
    case BMP_RLE4:
      return true;
    default:
      return false;
  }
}

int32_t BMPDecompressor::DecodeImage() {
  if (decode_status == BMP_D_STATUS_DATA_PRE) {
    avail_in = 0;
    if (!GetDataPosition(bmp_header_ptr->bfOffBits)) {
      decode_status = BMP_D_STATUS_TAIL;
      Error("The Bmp File Is Corrupt, Unexpected Stream Offset");
      NOTREACHED();
    }
    row_num = 0;
    SaveDecodingStatus(BMP_D_STATUS_DATA);
  }
  if (decode_status != BMP_D_STATUS_DATA || !ValidateFlag()) {
    Error("Any Uncontrol Error");
    NOTREACHED();
  }
  switch (compress_flag) {
    case BMP_RGB:
    case BMP_BITFIELDS:
      return DecodeRGB();
    case BMP_RLE8:
      return DecodeRLE8();
    case BMP_RLE4:
      return DecodeRLE4();
    default:
      return 0;
  }
}

bool BMPDecompressor::ValidateColorIndex(uint8_t val) {
  if (val >= pal_num) {
    Error("A color index exceeds range determined by pal_num");
    NOTREACHED();
  }
  return true;
}

int32_t BMPDecompressor::DecodeRGB() {
  uint8_t* des_buf = nullptr;
  while (row_num < height) {
    uint8_t* row_buf = out_row_buffer;
    if (!ReadData(&des_buf, src_row_bytes))
      return 2;

    SaveDecodingStatus(BMP_D_STATUS_DATA);
    switch (bitCounts) {
      case 1: {
        for (int32_t col = 0; col < width; ++col)
          *row_buf++ = des_buf[col >> 3] & (0x80 >> (col % 8)) ? 0x01 : 0x00;
      } break;
      case 4: {
        for (int32_t col = 0; col < width; ++col) {
          *row_buf++ = (col & 0x01) ? (des_buf[col >> 1] & 0x0F)
                                    : ((des_buf[col >> 1] & 0xF0) >> 4);
        }
      } break;
      case 16: {
        uint16_t* buf = (uint16_t*)des_buf;
        uint8_t blue_bits = 0;
        uint8_t green_bits = 0;
        uint8_t red_bits = 0;
        for (int32_t i = 0; i < 16; i++) {
          if ((mask_blue >> i) & 0x01)
            blue_bits++;
          if ((mask_green >> i) & 0x01)
            green_bits++;
          if ((mask_red >> i) & 0x01)
            red_bits++;
        }
        green_bits += blue_bits;
        red_bits += green_bits;
        if (blue_bits > 8 || green_bits < 8 || red_bits < 8)
          return 2;
        blue_bits = 8 - blue_bits;
        green_bits -= 8;
        red_bits -= 8;
        for (int32_t col = 0; col < width; ++col) {
          *buf = GetWord_LSBFirst((uint8_t*)buf);
          *row_buf++ = static_cast<uint8_t>((*buf & mask_blue) << blue_bits);
          *row_buf++ = static_cast<uint8_t>((*buf & mask_green) >> green_bits);
          *row_buf++ = static_cast<uint8_t>((*buf++ & mask_red) >> red_bits);
        }
      } break;
      case 8:
      case 24:
      case 32:
        memcpy(out_row_buffer, des_buf, src_row_bytes);
        row_buf += src_row_bytes;
        break;
    }
    for (uint8_t* buf = out_row_buffer; buf < row_buf; ++buf) {
      if (!ValidateColorIndex(*buf))
        return 0;
    }
    ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                 out_row_buffer);
  }
  SaveDecodingStatus(BMP_D_STATUS_TAIL);
  return 1;
}

int32_t BMPDecompressor::DecodeRLE8() {
  uint8_t* first_byte_ptr = nullptr;
  uint8_t* second_byte_ptr = nullptr;
  col_num = 0;
  while (true) {
    uint32_t skip_size_org = skip_size;
    if (!ReadData(&first_byte_ptr, 1))
      return 2;

    switch (*first_byte_ptr) {
      case RLE_MARKER: {
        if (!ReadData(&first_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        switch (*first_byte_ptr) {
          case RLE_EOL: {
            if (row_num >= height) {
              SaveDecodingStatus(BMP_D_STATUS_TAIL);
              Error("The Bmp File Is Corrupt");
              NOTREACHED();
            }
            ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                         out_row_buffer);
            col_num = 0;
            memset(out_row_buffer, 0, out_row_bytes);
            SaveDecodingStatus(BMP_D_STATUS_DATA);
            continue;
          }
          case RLE_EOI: {
            if (row_num < height) {
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
            SaveDecodingStatus(BMP_D_STATUS_TAIL);
            return 1;
          }
          case RLE_DELTA: {
            uint8_t* delta_ptr;
            if (!ReadData(&delta_ptr, 2)) {
              skip_size = skip_size_org;
              return 2;
            }
            col_num += (int32_t)delta_ptr[0];
            int32_t bmp_row_num_next = row_num + (int32_t)delta_ptr[1];
            if (col_num >= out_row_bytes || bmp_row_num_next >= height) {
              Error("The Bmp File Is Corrupt Or Not Supported");
              NOTREACHED();
            }
            while (row_num < bmp_row_num_next) {
              memset(out_row_buffer, 0, out_row_bytes);
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
          } break;
          default: {
            int32_t avail_size = out_row_bytes - col_num;
            if (!avail_size ||
                static_cast<int32_t>(*first_byte_ptr) > avail_size) {
              Error("The Bmp File Is Corrupt");
              NOTREACHED();
            }
            if (!ReadData(&second_byte_ptr, *first_byte_ptr & 1
                                                ? *first_byte_ptr + 1
                                                : *first_byte_ptr)) {
              skip_size = skip_size_org;
              return 2;
            }
            uint8_t* first_buf = out_row_buffer + col_num;
            memcpy(out_row_buffer + col_num, second_byte_ptr, *first_byte_ptr);
            for (size_t i = 0; i < *first_byte_ptr; ++i) {
              if (!ValidateColorIndex(first_buf[i]))
                return 0;
            }
            col_num += (int32_t)(*first_byte_ptr);
          }
        }
      } break;
      default: {
        int32_t avail_size = out_row_bytes - col_num;
        if (!avail_size || static_cast<int32_t>(*first_byte_ptr) > avail_size) {
          Error("The Bmp File Is Corrupt");
          NOTREACHED();
        }
        if (!ReadData(&second_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        uint8_t* first_buf = out_row_buffer + col_num;
        memset(out_row_buffer + col_num, *second_byte_ptr, *first_byte_ptr);
        for (size_t i = 0; i < *first_byte_ptr; ++i) {
          if (!ValidateColorIndex(first_buf[i]))
            return 0;
        }
        col_num += (int32_t)(*first_byte_ptr);
      }
    }
  }
  Error("Any Uncontrol Error");
  NOTREACHED();
}

int32_t BMPDecompressor::DecodeRLE4() {
  uint8_t* first_byte_ptr = nullptr;
  uint8_t* second_byte_ptr = nullptr;
  col_num = 0;
  while (true) {
    uint32_t skip_size_org = skip_size;
    if (!ReadData(&first_byte_ptr, 1))
      return 2;

    switch (*first_byte_ptr) {
      case RLE_MARKER: {
        if (!ReadData(&first_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        switch (*first_byte_ptr) {
          case RLE_EOL: {
            if (row_num >= height) {
              SaveDecodingStatus(BMP_D_STATUS_TAIL);
              Error("The Bmp File Is Corrupt");
              NOTREACHED();
            }
            ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                         out_row_buffer);
            col_num = 0;
            memset(out_row_buffer, 0, out_row_bytes);
            SaveDecodingStatus(BMP_D_STATUS_DATA);
            continue;
          }
          case RLE_EOI: {
            if (row_num < height) {
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
            SaveDecodingStatus(BMP_D_STATUS_TAIL);
            return 1;
          }
          case RLE_DELTA: {
            uint8_t* delta_ptr;
            if (!ReadData(&delta_ptr, 2)) {
              skip_size = skip_size_org;
              return 2;
            }
            col_num += (int32_t)delta_ptr[0];
            int32_t bmp_row_num_next = row_num + (int32_t)delta_ptr[1];
            if (col_num >= out_row_bytes || bmp_row_num_next >= height) {
              Error("The Bmp File Is Corrupt Or Not Supported");
              NOTREACHED();
            }
            while (row_num < bmp_row_num_next) {
              memset(out_row_buffer, 0, out_row_bytes);
              ReadScanline(imgTB_flag ? row_num++ : (height - 1 - row_num++),
                           out_row_buffer);
            }
          } break;
          default: {
            int32_t avail_size = out_row_bytes - col_num;
            if (!avail_size) {
              Error("The Bmp File Is Corrupt");
              NOTREACHED();
            }
            uint8_t size = HalfRoundUp(*first_byte_ptr);
            if (static_cast<int32_t>(*first_byte_ptr) > avail_size) {
              if (size + (col_num >> 1) > src_row_bytes) {
                Error("The Bmp File Is Corrupt");
                NOTREACHED();
              }
              *first_byte_ptr = avail_size - 1;
            }
            if (!ReadData(&second_byte_ptr, size & 1 ? size + 1 : size)) {
              skip_size = skip_size_org;
              return 2;
            }
            for (uint8_t i = 0; i < *first_byte_ptr; i++) {
              uint8_t color = (i & 0x01) ? (*second_byte_ptr++ & 0x0F)
                                         : (*second_byte_ptr & 0xF0) >> 4;
              if (!ValidateColorIndex(color))
                return 0;

              *(out_row_buffer + col_num++) = color;
            }
          }
        }
      } break;
      default: {
        int32_t avail_size = out_row_bytes - col_num;
        if (!avail_size) {
          Error("The Bmp File Is Corrupt");
          NOTREACHED();
        }
        if (static_cast<int32_t>(*first_byte_ptr) > avail_size) {
          uint8_t size = HalfRoundUp(*first_byte_ptr);
          if (size + (col_num >> 1) > src_row_bytes) {
            Error("The Bmp File Is Corrupt");
            NOTREACHED();
          }
          *first_byte_ptr = avail_size - 1;
        }
        if (!ReadData(&second_byte_ptr, 1)) {
          skip_size = skip_size_org;
          return 2;
        }
        for (uint8_t i = 0; i < *first_byte_ptr; i++) {
          uint8_t second_byte = *second_byte_ptr;
          second_byte =
              i & 0x01 ? (second_byte & 0x0F) : (second_byte & 0xF0) >> 4;
          if (!ValidateColorIndex(second_byte))
            return 0;
          *(out_row_buffer + col_num++) = second_byte;
        }
      }
    }
  }
  Error("Any Uncontrol Error");
  NOTREACHED();
}

uint8_t* BMPDecompressor::ReadData(uint8_t** des_buf, uint32_t data_size) {
  if (avail_in < skip_size + data_size)
    return nullptr;

  *des_buf = next_in + skip_size;
  skip_size += data_size;
  return *des_buf;
}

void BMPDecompressor::SaveDecodingStatus(int32_t status) {
  decode_status = status;
  next_in += skip_size;
  avail_in -= skip_size;
  skip_size = 0;
}

void BMPDecompressor::SetInputBuffer(uint8_t* src_buf, uint32_t src_size) {
  next_in = src_buf;
  avail_in = src_size;
  skip_size = 0;
}

uint32_t BMPDecompressor::GetAvailInput(uint8_t** avail_buf) {
  if (avail_buf) {
    *avail_buf = nullptr;
    if (avail_in > 0)
      *avail_buf = next_in;
  }
  return avail_in;
}

bmp_compress_struct_p bmp_create_compress() {
  bmp_compress_struct_p bmp_ptr;
  bmp_ptr = FX_Alloc(bmp_compress_struct, 1);
  if (bmp_ptr) {
    memset(bmp_ptr, 0, sizeof(bmp_compress_struct));
  }
  return bmp_ptr;
}
void bmp_destroy_compress(bmp_compress_struct_p bmp_ptr) {
  if (bmp_ptr) {
    if (bmp_ptr->src_free && bmp_ptr->src_buf) {
      FX_Free(bmp_ptr->src_buf);
    }
    FX_Free(bmp_ptr);
  }
}
static void WriteFileHeader(BmpFileHeaderPtr head_ptr, uint8_t* dst_buf) {
  uint32_t offset;
  offset = 0;
  SetWord_LSBFirst(&dst_buf[offset], head_ptr->bfType);
  offset += 2;
  SetDWord_LSBFirst(&dst_buf[offset], head_ptr->bfSize);
  offset += 4;
  SetWord_LSBFirst(&dst_buf[offset], head_ptr->bfReserved1);
  offset += 2;
  SetWord_LSBFirst(&dst_buf[offset], head_ptr->bfReserved2);
  offset += 2;
  SetDWord_LSBFirst(&dst_buf[offset], head_ptr->bfOffBits);
  offset += 4;
}
static void WriteInfoHeader(BmpInfoHeaderPtr info_head_ptr, uint8_t* dst_buf) {
  uint32_t offset;
  offset = sizeof(BmpFileHeader);
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biSize);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biWidth);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biHeight);
  offset += 4;
  SetWord_LSBFirst(&dst_buf[offset], info_head_ptr->biPlanes);
  offset += 2;
  SetWord_LSBFirst(&dst_buf[offset], info_head_ptr->biBitCount);
  offset += 2;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biCompression);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biSizeImage);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biXPelsPerMeter);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biYPelsPerMeter);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biClrUsed);
  offset += 4;
  SetDWord_LSBFirst(&dst_buf[offset], info_head_ptr->biClrImportant);
  offset += 4;
}
static void bmp_encode_bitfields(bmp_compress_struct_p bmp_ptr,
                                 uint8_t*& dst_buf,
                                 uint32_t& dst_size) {
  if (bmp_ptr->info_header.biBitCount != 16 &&
      bmp_ptr->info_header.biBitCount != 32) {
    return;
  }
  uint32_t size, dst_pos, i;
  size = bmp_ptr->src_pitch * bmp_ptr->src_row *
         bmp_ptr->info_header.biBitCount / 16;
  dst_pos = bmp_ptr->file_header.bfOffBits;
  dst_size += size;
  dst_buf = FX_Realloc(uint8_t, dst_buf, dst_size);
  memset(&dst_buf[dst_pos], 0, size);
  uint32_t mask_red;
  uint32_t mask_green;
  uint32_t mask_blue;
  mask_red = 0x7C00;
  mask_green = 0x03E0;
  mask_blue = 0x001F;
  if (bmp_ptr->info_header.biCompression == BMP_BITFIELDS) {
    if (bmp_ptr->bit_type == BMP_BIT_565) {
      mask_red = 0xF800;
      mask_green = 0x07E0;
      mask_blue = 0x001F;
    }
    if (bmp_ptr->info_header.biBitCount == 32) {
      mask_red = 0xFF0000;
      mask_green = 0x00FF00;
      mask_blue = 0x0000FF;
    }
    SetDWord_LSBFirst(&dst_buf[dst_pos], mask_red);
    dst_pos += 4;
    SetDWord_LSBFirst(&dst_buf[dst_pos], mask_green);
    dst_pos += 4;
    SetDWord_LSBFirst(&dst_buf[dst_pos], mask_blue);
    dst_pos += 4;
    bmp_ptr->file_header.bfOffBits = dst_pos;
  }
  uint8_t blue_bits = 0;
  uint8_t green_bits = 0;
  uint8_t red_bits = 0;
  for (i = 0; i < bmp_ptr->info_header.biBitCount; i++) {
    if ((mask_blue >> i) & 0x01) {
      blue_bits++;
    }
    if ((mask_green >> i) & 0x01) {
      green_bits++;
    }
    if ((mask_red >> i) & 0x01) {
      red_bits++;
    }
  }
  green_bits += blue_bits;
  red_bits += green_bits;
  blue_bits = 8 - blue_bits;
  green_bits -= 8;
  red_bits -= 8;
  i = 0;
  for (int32_t row_num = bmp_ptr->src_row - 1; row_num > -1; row_num--, i = 0) {
    while (i < bmp_ptr->src_width * bmp_ptr->src_bpp / 8) {
      uint8_t b = bmp_ptr->src_buf[row_num * bmp_ptr->src_pitch + i++];
      uint8_t g = bmp_ptr->src_buf[row_num * bmp_ptr->src_pitch + i++];
      uint8_t r = bmp_ptr->src_buf[row_num * bmp_ptr->src_pitch + i++];
      if (bmp_ptr->src_bpp == 32) {
        i++;
      }
      uint32_t pix_val = 0;
      pix_val |= (b >> blue_bits) & mask_blue;
      pix_val |= (g << green_bits) & mask_green;
      pix_val |= (r << red_bits) & mask_red;
      if (bmp_ptr->info_header.biBitCount == 16) {
        SetWord_LSBFirst(&dst_buf[dst_pos], pix_val);
        dst_pos += 2;
      } else {
        SetDWord_LSBFirst(&dst_buf[dst_pos], pix_val);
        dst_pos += 4;
      }
    }
  }
  dst_size = dst_pos;
}

static void bmp_encode_rgb(bmp_compress_struct_p bmp_ptr,
                           uint8_t*& dst_buf,
                           uint32_t& dst_size) {
  if (bmp_ptr->info_header.biBitCount == 16) {
    bmp_encode_bitfields(bmp_ptr, dst_buf, dst_size);
    return;
  }
  uint32_t size, dst_pos;
  uint32_t dst_pitch =
      (bmp_ptr->src_width * bmp_ptr->info_header.biBitCount + 31) / 32 * 4;
  size = dst_pitch * bmp_ptr->src_row;
  dst_pos = bmp_ptr->file_header.bfOffBits;
  dst_size += size;
  dst_buf = FX_Realloc(uint8_t, dst_buf, dst_size);
  memset(&dst_buf[dst_pos], 0, size);
  for (int32_t row_num = bmp_ptr->src_row - 1; row_num > -1; row_num--) {
    memcpy(&dst_buf[dst_pos], &bmp_ptr->src_buf[row_num * bmp_ptr->src_pitch],
           bmp_ptr->src_pitch);
    dst_pos += dst_pitch;
  }
  dst_size = dst_pos;
}
static uint8_t bmp_rle8_search(const uint8_t* buf, int32_t len) {
  uint8_t num;
  num = 1;
  while (num < len) {
    if (buf[num - 1] != buf[num] || num == 0xFF) {
      break;
    }
    num++;
  }
  return num;
}
static void bmp_encode_rle8(bmp_compress_struct_p bmp_ptr,
                            uint8_t*& dst_buf,
                            uint32_t& dst_size) {
  uint32_t size, dst_pos, index;
  uint8_t rle[2] = {0};
  size = bmp_ptr->src_pitch * bmp_ptr->src_row * 2;
  dst_pos = bmp_ptr->file_header.bfOffBits;
  dst_size += size;
  dst_buf = FX_Realloc(uint8_t, dst_buf, dst_size);
  memset(&dst_buf[dst_pos], 0, size);
  for (int32_t row_num = bmp_ptr->src_row - 1, i = 0; row_num > -1;) {
    index = row_num * bmp_ptr->src_pitch;
    rle[0] = bmp_rle8_search(&bmp_ptr->src_buf[index + i], size - index - i);
    rle[1] = bmp_ptr->src_buf[index + i];
    if (i + rle[0] >= (int32_t)bmp_ptr->src_pitch) {
      rle[0] = uint8_t(bmp_ptr->src_pitch - i);
      if (rle[0]) {
        dst_buf[dst_pos++] = rle[0];
        dst_buf[dst_pos++] = rle[1];
      }
      dst_buf[dst_pos++] = RLE_MARKER;
      dst_buf[dst_pos++] = RLE_EOL;
      i = 0;
      row_num--;
    } else {
      i += rle[0];
      dst_buf[dst_pos++] = rle[0];
      dst_buf[dst_pos++] = rle[1];
    }
  }
  dst_buf[dst_pos++] = RLE_MARKER;
  dst_buf[dst_pos++] = RLE_EOI;
  dst_size = dst_pos;
}
static uint8_t bmp_rle4_search(const uint8_t* buf, int32_t len) {
  uint8_t num;
  num = 2;
  while (num < len) {
    if (buf[num - 2] != buf[num] || num == 0xFF) {
      break;
    }
    num++;
  }
  return num;
}
static void bmp_encode_rle4(bmp_compress_struct_p bmp_ptr,
                            uint8_t*& dst_buf,
                            uint32_t& dst_size) {
  uint32_t size, dst_pos, index;
  uint8_t rle[2] = {0};
  size = bmp_ptr->src_pitch * bmp_ptr->src_row;
  dst_pos = bmp_ptr->file_header.bfOffBits;
  dst_size += size;
  dst_buf = FX_Realloc(uint8_t, dst_buf, dst_size);
  memset(&dst_buf[dst_pos], 0, size);
  for (int32_t row_num = bmp_ptr->src_row - 1, i = 0; row_num > -1;
       rle[1] = 0) {
    index = row_num * bmp_ptr->src_pitch;
    rle[0] = bmp_rle4_search(&bmp_ptr->src_buf[index + i], size - index - i);
    rle[1] |= (bmp_ptr->src_buf[index + i] & 0x0f) << 4;
    rle[1] |= bmp_ptr->src_buf[index + i + 1] & 0x0f;
    if (i + rle[0] >= (int32_t)bmp_ptr->src_pitch) {
      rle[0] = uint8_t(bmp_ptr->src_pitch - i);
      if (rle[0]) {
        dst_buf[dst_pos++] = rle[0];
        dst_buf[dst_pos++] = rle[1];
      }
      dst_buf[dst_pos++] = RLE_MARKER;
      dst_buf[dst_pos++] = RLE_EOL;
      i = 0;
      row_num--;
    } else {
      i += rle[0];
      dst_buf[dst_pos++] = rle[0];
      dst_buf[dst_pos++] = rle[1];
    }
  }
  dst_buf[dst_pos++] = RLE_MARKER;
  dst_buf[dst_pos++] = RLE_EOI;
  dst_size = dst_pos;
}
bool bmp_encode_image(bmp_compress_struct_p bmp_ptr,
                      uint8_t*& dst_buf,
                      uint32_t& dst_size) {
  uint32_t head_size = sizeof(BmpFileHeader) + sizeof(BmpInfoHeader);
  uint32_t pal_size = sizeof(uint32_t) * bmp_ptr->pal_num;
  if (bmp_ptr->info_header.biClrUsed > 0 &&
      bmp_ptr->info_header.biClrUsed < bmp_ptr->pal_num) {
    pal_size = sizeof(uint32_t) * bmp_ptr->info_header.biClrUsed;
  }
  dst_size = head_size + sizeof(uint32_t) * bmp_ptr->pal_num;
  dst_buf = FX_TryAlloc(uint8_t, dst_size);
  if (!dst_buf)
    return false;

  memset(dst_buf, 0, dst_size);
  bmp_ptr->file_header.bfOffBits = head_size;
  if (bmp_ptr->pal_ptr && pal_size) {
    memcpy(&dst_buf[head_size], bmp_ptr->pal_ptr, pal_size);
    bmp_ptr->file_header.bfOffBits += pal_size;
  }
  WriteInfoHeader(&bmp_ptr->info_header, dst_buf);
  switch (bmp_ptr->info_header.biCompression) {
    case BMP_RGB:
      bmp_encode_rgb(bmp_ptr, dst_buf, dst_size);
      break;
    case BMP_BITFIELDS:
      bmp_encode_bitfields(bmp_ptr, dst_buf, dst_size);
      break;
    case BMP_RLE8:
      bmp_encode_rle8(bmp_ptr, dst_buf, dst_size);
      break;
    case BMP_RLE4:
      bmp_encode_rle4(bmp_ptr, dst_buf, dst_size);
      break;
    default:
      break;
  }
  bmp_ptr->file_header.bfSize = dst_size;
  WriteFileHeader(&bmp_ptr->file_header, dst_buf);
  return true;
}
