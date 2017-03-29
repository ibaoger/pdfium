// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDFIUM_ZLIB_REDEFINE_NAMES_H_
#define PDFIUM_ZLIB_REDEFINE_NAMES_H_

/* PDFIUM change. TODO: disable this using BUILD.gn. */
#define NO_GZIP

#define deflate_copyright FPDFAPI_deflate_copyright
#define adler32 FPDFAPI_adler32
#define adler32_combine FPDFAPI_adler32_combine
#define compress2 FPDFAPI_compress2
#define compress FPDFAPI_compress
#define compressBound FPDFAPI_compressBound
#define get_crc_table FPDFAPI_get_crc_table
#define crc32 FPDFAPI_crc32
#define deflateInit_ FPDFAPI_deflateInit_
#define deflateInit2_ FPDFAPI_deflateInit2_
#define deflateSetDictionary FPDFAPI_deflateSetDictionary
#define deflateReset FPDFAPI_deflateReset
#define deflatePending FPDFAPI_deflatePending
#define deflatePrime FPDFAPI_deflatePrime
#define deflateParams FPDFAPI_deflateParams
#define deflateBound FPDFAPI_deflateBound
#define deflateSetHeader FPDFAPI_deflateSetHeader
#define deflateTune FPDFAPI_deflateTune
#define deflate FPDFAPI_deflate
#define deflateEnd FPDFAPI_deflateEnd
#define deflateCopy FPDFAPI_deflateCopy
#define inflateBackInit_ FPDFAPI_inflateBackInit_
#define inflateBack FPDFAPI_inflateBack
#define inflateBackEnd FPDFAPI_inflateBackEnd
#define inflateReset FPDFAPI_inflateReset
#define inflateInit2_ FPDFAPI_inflateInit2_
#define inflateInit_ FPDFAPI_inflateInit_
#define inflate FPDFAPI_inflate
#define inflateEnd FPDFAPI_inflateEnd
#define inflateSetDictionary FPDFAPI_inflateSetDictionary
#define inflateSync FPDFAPI_inflateSync
#define inflateSyncPoint FPDFAPI_inflateSyncPoint
#define inflateCopy FPDFAPI_inflateCopy
#define uncompress FPDFAPI_uncompress
#define zlibVersion FPDFAPI_zlibVersion
#define zlibCompileFlags FPDFAPI_zlibCompileFlags
#define zError FPDFAPI_zError
#define z_errmsg FPDFAPI_z_errmsg
#define zcfree FPDFAPI_zcfree
#define zcalloc FPDFAPI_zcalloc
#define inflate_fast FPDFAPI_inflate_fast
#define inflate_table FPDFAPI_inflate_table
#define inflate_copyright FPDFAPI_inflate_copyright
#define _length_code FPDFAPI_length_code
#define _tr_flush_block FPDFAPI_tr_flush_block
#define _dist_code FPDFAPI_dist_code
#define _tr_stored_block FPDFAPI_tr_stored_block
#define _tr_init FPDFAPI_tr_init
#define _tr_align FPDFAPI_tr_align
#define _tr_tally FPDFAPI_tr_tally
#define _tr_flush_bits FPDFAPI_tr_flush_bits
#define inflatePrime FPDFAPI_inflatePrime
#define inflateGetDictionary FPDFAPI_inflateGetDictionary
#define inflateGetHeader FPDFAPI_inflateGetHeader
#define crc32_combine FPDFAPI_crc32_combine
#define inflateReset2 FPDFAPI_inflateReset2
#define inflateUndermine FPDFAPI_inflateUndermine
#define inflateMark FPDFAPI_inflateMark
#define adler32_combine64 FPDFAPI_adler32_combine64
#define inflateResetKeep FPDFAPI_inflateResetKeep
#define deflateResetKeep FPDFAPI_deflateResetKeep

#endif
