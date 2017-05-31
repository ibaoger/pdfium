// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcodec/codec/ccodec_bmpmodule.h"

#include "core/fxcodec/codec/codec_int.h"
#include "core/fxcodec/fx_codec.h"
#include "core/fxcodec/lbmp/fx_bmp.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxge/fx_dib.h"

struct FXBMP_Context {
  bmp_decompress_struct_p bmp_ptr;
  CCodec_BmpModule* parent_ptr;
  CFX_UnownedPtr<CCodec_BmpModule::Delegate> delegate_ptr;
  void* (*m_AllocFunc)(unsigned int);
  void (*m_FreeFunc)(void*);
  char m_szLastError[256];
};

extern "C" {
static void* bmp_alloc_func(unsigned int size) {
  return FX_Alloc(char, size);
}
static void bmp_free_func(void* p) {
  FX_Free(p);
}
}  // extern "C"

static void bmp_error_data(bmp_decompress_struct_p bmp_ptr,
                           const char* err_msg) {
  strncpy((char*)bmp_ptr->err_ptr, err_msg, BMP_MAX_ERROR_SIZE - 1);
  longjmp(bmp_ptr->jmpbuf, 1);
}

static void bmp_read_scanline(bmp_decompress_struct_p bmp_ptr,
                              int32_t row_num,
                              uint8_t* row_buf) {
  auto* p = reinterpret_cast<FXBMP_Context*>(bmp_ptr->context_ptr);
  p->delegate_ptr->BmpReadScanline(row_num, row_buf);
}

static bool bmp_get_data_position(bmp_decompress_struct_p bmp_ptr,
                                  uint32_t rcd_pos) {
  auto* p = reinterpret_cast<FXBMP_Context*>(bmp_ptr->context_ptr);
  return p->delegate_ptr->BmpInputImagePositionBuf(rcd_pos);
}

CCodec_BmpModule::CCodec_BmpModule() {}

CCodec_BmpModule::~CCodec_BmpModule() {}

FXBMP_Context* CCodec_BmpModule::Start(Delegate* pDelegate) {
  bmp_decompress_struct_p pDecomp = bmp_create_decompress();
  if (pDecomp)
    return nullptr;

  auto* p = new FXBMP_Context;
  p->bmp_ptr = pDecomp;
  p->parent_ptr = this;
  p->delegate_ptr = pDelegate;
  p->m_AllocFunc = bmp_alloc_func;
  p->m_FreeFunc = bmp_free_func;
  memset(p->m_szLastError, 0, sizeof(p->m_szLastError));

  p->bmp_ptr->context_ptr = p;
  p->bmp_ptr->err_ptr = p->m_szLastError;
  p->bmp_ptr->bmp_error_fn = bmp_error_data;
  p->bmp_ptr->bmp_get_row_fn = bmp_read_scanline;
  p->bmp_ptr->bmp_get_data_position_fn = bmp_get_data_position;
  return p;
}

void CCodec_BmpModule::Finish(FXBMP_Context* ctx) {
  if (ctx) {
    bmp_destroy_decompress(&ctx->bmp_ptr);
    delete ctx;
  }
}

int32_t CCodec_BmpModule::ReadHeader(FXBMP_Context* ctx,
                                     int32_t* width,
                                     int32_t* height,
                                     bool* tb_flag,
                                     int32_t* components,
                                     int32_t* pal_num,
                                     uint32_t** pal_pp,
                                     CFX_DIBAttribute* pAttribute) {
  if (setjmp(ctx->bmp_ptr->jmpbuf)) {
    return 0;
  }
  int32_t ret = bmp_read_header(ctx->bmp_ptr);
  if (ret != 1) {
    return ret;
  }
  *width = ctx->bmp_ptr->width;
  *height = ctx->bmp_ptr->height;
  *tb_flag = ctx->bmp_ptr->imgTB_flag;
  *components = ctx->bmp_ptr->components;
  *pal_num = ctx->bmp_ptr->pal_num;
  *pal_pp = ctx->bmp_ptr->pal_ptr;
  if (pAttribute) {
    pAttribute->m_wDPIUnit = FXCODEC_RESUNIT_METER;
    pAttribute->m_nXDPI = ctx->bmp_ptr->dpi_x;
    pAttribute->m_nYDPI = ctx->bmp_ptr->dpi_y;
    pAttribute->m_nBmpCompressType = ctx->bmp_ptr->compress_flag;
  }
  return 1;
}

int32_t CCodec_BmpModule::LoadImage(FXBMP_Context* ctx) {
  if (setjmp(ctx->bmp_ptr->jmpbuf))
    return 0;
  return bmp_decode_image(ctx->bmp_ptr);
}

uint32_t CCodec_BmpModule::GetAvailInput(FXBMP_Context* ctx,
                                         uint8_t** avail_buf_ptr) {
  return bmp_get_avail_input(ctx->bmp_ptr, avail_buf_ptr);
}

void CCodec_BmpModule::Input(FXBMP_Context* ctx,
                             const uint8_t* src_buf,
                             uint32_t src_size) {
  bmp_input_buffer(ctx->bmp_ptr, (uint8_t*)src_buf, src_size);
}
