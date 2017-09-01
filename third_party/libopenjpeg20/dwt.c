/*
 * The copyright in this software is being made available under the 2-clauses
 * BSD License, included below. This software may be subject to other third
 * party and contributor rights, including patent rights, and no such rights
 * are granted under this license.
 *
 * Copyright (c) 2002-2014, Universite catholique de Louvain (UCL), Belgium
 * Copyright (c) 2002-2014, Professor Benoit Macq
 * Copyright (c) 2001-2003, David Janssens
 * Copyright (c) 2002-2003, Yannick Verschueren
 * Copyright (c) 2003-2007, Francois-Olivier Devaux
 * Copyright (c) 2003-2014, Antonin Descampe
 * Copyright (c) 2005, Herve Drolon, FreeImage Team
 * Copyright (c) 2007, Jonathan Ballard <dzonatas@dzonux.net>
 * Copyright (c) 2007, Callum Lerwick <seg@haxxed.com>
 * Copyright (c) 2017, IntoPIX SA <support@intopix.com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS `AS IS'
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <assert.h>

#define OPJ_SKIP_POISON
#include "opj_includes.h"

#ifdef __SSE__
#include <xmmintrin.h>
#endif
#ifdef __SSE2__
#include <emmintrin.h>
#endif
#ifdef __SSSE3__
#include <tmmintrin.h>
#endif
#ifdef __AVX2__
#include <immintrin.h>
#endif

#if defined(__GNUC__)
#pragma GCC poison malloc calloc realloc free
#endif

/** @defgroup DWT DWT - Implementation of a discrete wavelet transform */
/*@{*/

#ifdef __AVX2__
/** Number of int32 values in a AVX2 register */
#define VREG_INT_COUNT       8
#else
/** Number of int32 values in a SSE2 register */
#define VREG_INT_COUNT       4
#endif

/** Number of columns that we can process in parallel in the vertical pass */
#define PARALLEL_COLS_53     (2*VREG_INT_COUNT)

/** @name Local data structures */
/*@{*/

typedef struct dwt_local {
    OPJ_INT32* mem;
    OPJ_SIZE_T mem_count;
    OPJ_INT32 dn;
    OPJ_INT32 sn;
    OPJ_INT32 cas;
} opj_dwt_t;

typedef union {
    OPJ_FLOAT32 f[4];
} opj_v4_t;

typedef struct v4dwt_local {
    opj_v4_t*   wavelet ;
    OPJ_INT32       dn ;
    OPJ_INT32       sn ;
    OPJ_INT32       cas ;
} opj_v4dwt_t ;

static const OPJ_FLOAT32 opj_dwt_alpha =  1.586134342f; /*  12994 */
static const OPJ_FLOAT32 opj_dwt_beta  =  0.052980118f; /*    434 */
static const OPJ_FLOAT32 opj_dwt_gamma = -0.882911075f; /*  -7233 */
static const OPJ_FLOAT32 opj_dwt_delta = -0.443506852f; /*  -3633 */

static const OPJ_FLOAT32 opj_K      = 1.230174105f; /*  10078 */
static const OPJ_FLOAT32 opj_c13318 = 1.625732422f;

/*@}*/

/**
Virtual function type for wavelet transform in 1-D
*/
typedef void (*DWT1DFN)(const opj_dwt_t* v);

/** @name Local static functions */
/*@{*/

/**
Forward lazy transform (horizontal)
*/
static void opj_dwt_deinterleave_h(OPJ_INT32 *a, OPJ_INT32 *b, OPJ_INT32 dn,
                                   OPJ_INT32 sn, OPJ_INT32 cas);
/**
Forward lazy transform (vertical)
*/
static void opj_dwt_deinterleave_v(OPJ_INT32 *a, OPJ_INT32 *b, OPJ_INT32 dn,
                                   OPJ_INT32 sn, OPJ_INT32 x, OPJ_INT32 cas);
/**
Forward 5-3 wavelet transform in 1-D
*/
static void opj_dwt_encode_1(OPJ_INT32 *a, OPJ_SIZE_T a_count, OPJ_INT32 dn,
    OPJ_INT32 sn, OPJ_INT32 cas);

/**
Forward 9-7 wavelet transform in 1-D
*/
static void opj_dwt_encode_1_real(OPJ_INT32 *a, OPJ_SIZE_T a_count,
    OPJ_INT32 dn, OPJ_INT32 sn, OPJ_INT32 cas);

/**
Explicit calculation of the Quantization Stepsizes
*/
static void opj_dwt_encode_stepsize(OPJ_INT32 stepsize, OPJ_INT32 numbps,
                                    opj_stepsize_t *bandno_stepsize);
/**
Inverse wavelet transform in 2-D.
*/
static OPJ_BOOL opj_dwt_decode_tile(opj_thread_pool_t* tp,
                                    const opj_tcd_tilecomp_t* tilec, OPJ_UINT32 i);

static OPJ_BOOL opj_dwt_encode_procedure(const opj_tcd_tilecomp_t * tilec,
        void(*p_function)(OPJ_INT32 *, OPJ_SIZE_T, OPJ_INT32, OPJ_INT32, OPJ_INT32));

static OPJ_UINT32 opj_dwt_max_resolution(opj_tcd_resolution_t* OPJ_RESTRICT r,
        OPJ_UINT32 i);

/* <summary>                             */
/* Inverse 9-7 wavelet transform in 1-D. */
/* </summary>                            */
static void opj_v4dwt_decode(opj_v4dwt_t* OPJ_RESTRICT dwt);

static void opj_v4dwt_interleave_h(opj_v4dwt_t* OPJ_RESTRICT w,
                                   OPJ_FLOAT32* OPJ_RESTRICT a, OPJ_INT32 x, OPJ_INT32 size);

static void opj_v4dwt_interleave_v(opj_v4dwt_t* OPJ_RESTRICT v,
                                   OPJ_FLOAT32* OPJ_RESTRICT a, OPJ_INT32 x, OPJ_INT32 nb_elts_read);

#ifdef __SSE__
static void opj_v4dwt_decode_step1_sse(opj_v4_t* w, OPJ_INT32 count,
                                       const __m128 c);

static void opj_v4dwt_decode_step2_sse(opj_v4_t* l, opj_v4_t* w, OPJ_INT32 k,
                                       OPJ_INT32 m, __m128 c);

#else
static void opj_v4dwt_decode_step1(opj_v4_t* w, OPJ_INT32 count,
                                   const OPJ_FLOAT32 c);

static void opj_v4dwt_decode_step2(opj_v4_t* l, opj_v4_t* w, OPJ_INT32 k,
                                   OPJ_INT32 m, OPJ_FLOAT32 c);

#endif

/*@}*/

/*@}*/

#define IDX_S(i) (i)*2
#define IDX_D(i) 1 + (i)* 2
#define UNDERFLOW_SN(i) ((i) >= sn&&sn>0)
#define UNDERFLOW_DN(i) ((i) >= dn&&dn>0)
#define OVERFLOW_S(i) (IDX_S(i) >= a_count)
#define OVERFLOW_D(i) (IDX_D(i) >= a_count)

#define OPJ_S(i) a[IDX_S(i)]
#define OPJ_D(i) a[IDX_D(i)]
#define OPJ_S_(i) ((i)<0 ? OPJ_S(0) : (UNDERFLOW_SN(i) ? OPJ_S(sn - 1) : OVERFLOW_S(i) ? OPJ_S(i - 1) : OPJ_S(i)))
#define OPJ_D_(i) ((i)<0 ? OPJ_D(0) : (UNDERFLOW_DN(i) ? OPJ_D(dn - 1) : OVERFLOW_D(i) ? OPJ_D(i - 1) : OPJ_D(i)))
/* new */
#define OPJ_SS_(i) ((i)<0 ? OPJ_S(0) : (UNDERFLOW_DN(i) ? OPJ_S(dn - 1) : OVERFLOW_S(i) ? OPJ_S(i - 1) : OPJ_S(i)))
#define OPJ_DD_(i) ((i)<0 ? OPJ_D(0) : (UNDERFLOW_SN(i) ? OPJ_D(sn - 1) : OVERFLOW_D(i) ? OPJ_D(i - 1) : OPJ_D(i)))

/* <summary>                                                              */
/* This table contains the norms of the 5-3 wavelets for different bands. */
/* </summary>                                                             */
static const OPJ_FLOAT64 opj_dwt_norms[4][10] = {
    {1.000, 1.500, 2.750, 5.375, 10.68, 21.34, 42.67, 85.33, 170.7, 341.3},
    {1.038, 1.592, 2.919, 5.703, 11.33, 22.64, 45.25, 90.48, 180.9},
    {1.038, 1.592, 2.919, 5.703, 11.33, 22.64, 45.25, 90.48, 180.9},
    {.7186, .9218, 1.586, 3.043, 6.019, 12.01, 24.00, 47.97, 95.93}
};

/* <summary>                                                              */
/* This table contains the norms of the 9-7 wavelets for different bands. */
/* </summary>                                                             */
static const OPJ_FLOAT64 opj_dwt_norms_real[4][10] = {
    {1.000, 1.965, 4.177, 8.403, 16.90, 33.84, 67.69, 135.3, 270.6, 540.9},
    {2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
    {2.022, 3.989, 8.355, 17.04, 34.27, 68.63, 137.3, 274.6, 549.0},
    {2.080, 3.865, 8.307, 17.18, 34.71, 69.59, 139.3, 278.6, 557.2}
};

/*
==========================================================
   local functions
==========================================================
*/

/* <summary>                             */
/* Forward lazy transform (horizontal).  */
/* </summary>                            */
static void opj_dwt_deinterleave_h(OPJ_INT32 *a, OPJ_INT32 *b, OPJ_INT32 dn,
                                   OPJ_INT32 sn, OPJ_INT32 cas)
{
    OPJ_INT32 i;
    OPJ_INT32 * l_dest = b;
    OPJ_INT32 * l_src = a + cas;

    for (i = 0; i < sn; ++i) {
        *l_dest++ = *l_src;
        l_src += 2;
    }

    l_dest = b + sn;
    l_src = a + 1 - cas;

    for (i = 0; i < dn; ++i)  {
        *l_dest++ = *l_src;
        l_src += 2;
    }
}

/* <summary>                             */
/* Forward lazy transform (vertical).    */
/* </summary>                            */
static void opj_dwt_deinterleave_v(OPJ_INT32 *a, OPJ_INT32 *b, OPJ_INT32 dn,
                                   OPJ_INT32 sn, OPJ_INT32 x, OPJ_INT32 cas)
{
    OPJ_INT32 i = sn;
    OPJ_INT32 * l_dest = b;
    OPJ_INT32 * l_src = a + cas;

    while (i--) {
        *l_dest = *l_src;
        l_dest += x;
        l_src += 2;
    } /* b[i*x]=a[2*i+cas]; */

    l_dest = b + sn * x;
    l_src = a + 1 - cas;

    i = dn;
    while (i--) {
        *l_dest = *l_src;
        l_dest += x;
        l_src += 2;
    } /*b[(sn+i)*x]=a[(2*i+1-cas)];*/
}

#ifdef STANDARD_SLOW_VERSION
/* <summary>                             */
/* Inverse lazy transform (horizontal).  */
/* </summary>                            */
static void opj_dwt_interleave_h(const opj_dwt_t* h, OPJ_INT32 *a)
{
    OPJ_INT32 *ai = a;
    OPJ_INT32 *bi = h->mem + h->cas;
    OPJ_INT32  i    = h->sn;
    while (i--) {
        *bi = *(ai++);
        bi += 2;
    }
    ai  = a + h->sn;
    bi  = h->mem + 1 - h->cas;
    i   = h->dn ;
    while (i--) {
        *bi = *(ai++);
        bi += 2;
    }
}

/* <summary>                             */
/* Inverse lazy transform (vertical).    */
/* </summary>                            */
static void opj_dwt_interleave_v(const opj_dwt_t* v, OPJ_INT32 *a, OPJ_INT32 x)
{
    OPJ_INT32 *ai = a;
    OPJ_INT32 *bi = v->mem + v->cas;
    OPJ_INT32  i = v->sn;
    while (i--) {
        *bi = *ai;
        bi += 2;
        ai += x;
    }
    ai = a + (v->sn * x);
    bi = v->mem + 1 - v->cas;
    i = v->dn ;
    while (i--) {
        *bi = *ai;
        bi += 2;
        ai += x;
    }
}

#endif /* STANDARD_SLOW_VERSION */

/* <summary>                            */
/* Forward 5-3 wavelet transform in 1-D. */
/* </summary>                           */
static void opj_dwt_encode_1(OPJ_INT32 *a, OPJ_SIZE_T a_count, OPJ_INT32 dn,
                             OPJ_INT32 sn, OPJ_INT32 cas)
{
    OPJ_INT32 i;

    if (!cas) {
        if ((dn > 0) || (sn > 1)) { /* NEW :  CASE ONE ELEMENT */
            for (i = 0; i < dn; i++) {
                OPJ_D(i) -= (OPJ_S_(i) + OPJ_S_(i + 1)) >> 1;
            }
            for (i = 0; i < sn; i++) {
                OPJ_S(i) += (OPJ_D_(i - 1) + OPJ_D_(i) + 2) >> 2;
            }
        }
    } else {
        if (!sn && dn == 1) {       /* NEW :  CASE ONE ELEMENT */
            OPJ_S(0) *= 2;
        } else {
            for (i = 0; i < dn; i++) {
                OPJ_S(i) -= (OPJ_DD_(i) + OPJ_DD_(i - 1)) >> 1;
            }
            for (i = 0; i < sn; i++) {
                OPJ_D(i) += (OPJ_SS_(i) + OPJ_SS_(i + 1) + 2) >> 2;
            }
        }
    }
}

#ifdef STANDARD_SLOW_VERSION
/* <summary>                            */
/* Inverse 5-3 wavelet transform in 1-D. */
/* </summary>                           */
static void opj_dwt_decode_1_(OPJ_INT32 *a, OPJ_SIZE_T a_count, OPJ_INT32 dn,
                              OPJ_INT32 sn, OPJ_INT32 cas)
{
    OPJ_INT32 i;

    if (!cas) {
        if ((dn > 0) || (sn > 1)) { /* NEW :  CASE ONE ELEMENT */
            for (i = 0; i < sn; i++) {
                OPJ_S(i) -= (OPJ_D_(i - 1) + OPJ_D_(i) + 2) >> 2;
            }
            for (i = 0; i < dn; i++) {
                OPJ_D(i) += (OPJ_S_(i) + OPJ_S_(i + 1)) >> 1;
            }
        }
    } else {
        if (!sn  && dn == 1) {        /* NEW :  CASE ONE ELEMENT */
            OPJ_S(0) /= 2;
        } else {
            for (i = 0; i < sn; i++) {
                OPJ_D(i) -= (OPJ_SS_(i) + OPJ_SS_(i + 1) + 2) >> 2;
            }
            for (i = 0; i < dn; i++) {
                OPJ_S(i) += (OPJ_DD_(i) + OPJ_DD_(i - 1)) >> 1;
            }
        }
    }
}

static void opj_dwt_decode_1(const opj_dwt_t *v)
{
    opj_dwt_decode_1_(v->mem, v->mem_count, v->dn, v->sn, v->cas);
}

#endif /* STANDARD_SLOW_VERSION */

#if !defined(STANDARD_SLOW_VERSION)
static void  opj_idwt53_h_cas0(OPJ_INT32* tmp,
                               const OPJ_INT32 sn,
                               const OPJ_INT32 len,
                               OPJ_INT32* tiledp)
{
    OPJ_INT32 i, j;
    const OPJ_INT32* in_even = &tiledp[0];
    const OPJ_INT32* in_odd = &tiledp[sn];

#ifdef TWO_PASS_VERSION
    /* For documentation purpose: performs lifting in two iterations, */
    /* but without explicit interleaving */

    assert(len > 1);

    /* Even */
    tmp[0] = in_even[0] - ((in_odd[0] + 1) >> 1);
    for (i = 2, j = 0; i <= len - 2; i += 2, j++) {
        tmp[i] = in_even[j + 1] - ((in_odd[j] + in_odd[j + 1] + 2) >> 2);
    }
    if (len & 1) { /* if len is odd */
        tmp[len - 1] = in_even[(len - 1) / 2] - ((in_odd[(len - 2) / 2] + 1) >> 1);
    }

    /* Odd */
    for (i = 1, j = 0; i < len - 1; i += 2, j++) {
        tmp[i] = in_odd[j] + ((tmp[i - 1] + tmp[i + 1]) >> 1);
    }
    if (!(len & 1)) { /* if len is even */
        tmp[len - 1] = in_odd[(len - 1) / 2] + tmp[len - 2];
    }
#else
    OPJ_INT32 d1c, d1n, s1n, s0c, s0n;

    assert(len > 1);

    /* Improved version of the TWO_PASS_VERSION: */
    /* Performs lifting in one single iteration. Saves memory */
    /* accesses and explicit interleaving. */
    s1n = in_even[0];
    d1n = in_odd[0];
    s0n = s1n - ((d1n + 1) >> 1);

    for (i = 0, j = 1; i < (len - 3); i += 2, j++) {
        d1c = d1n;
        s0c = s0n;

        s1n = in_even[j];
        d1n = in_odd[j];

        s0n = s1n - ((d1c + d1n + 2) >> 2);

        tmp[i  ] = s0c;
        tmp[i + 1] = d1c + ((s0c + s0n) >> 1);
    }

    tmp[i] = s0n;

    if (len & 1) {
        tmp[len - 1] = in_even[(len - 1) / 2] - ((d1n + 1) >> 1);
        tmp[len - 2] = d1n + ((s0n + tmp[len - 1]) >> 1);
    } else {
        tmp[len - 1] = d1n + s0n;
    }
#endif
    memcpy(tiledp, tmp, (OPJ_UINT32)len * sizeof(OPJ_INT32));
}

static void  opj_idwt53_h_cas1(OPJ_INT32* tmp,
                               const OPJ_INT32 sn,
                               const OPJ_INT32 len,
                               OPJ_INT32* tiledp)
{
    OPJ_INT32 i, j;
    const OPJ_INT32* in_even = &tiledp[sn];
    const OPJ_INT32* in_odd = &tiledp[0];

#ifdef TWO_PASS_VERSION
    /* For documentation purpose: performs lifting in two iterations, */
    /* but without explicit interleaving */

    assert(len > 2);

    /* Odd */
    for (i = 1, j = 0; i < len - 1; i += 2, j++) {
        tmp[i] = in_odd[j] - ((in_even[j] + in_even[j + 1] + 2) >> 2);
    }
    if (!(len & 1)) {
        tmp[len - 1] = in_odd[len / 2 - 1] - ((in_even[len / 2 - 1] + 1) >> 1);
    }

    /* Even */
    tmp[0] = in_even[0] + tmp[1];
    for (i = 2, j = 1; i < len - 1; i += 2, j++) {
        tmp[i] = in_even[j] + ((tmp[i + 1] + tmp[i - 1]) >> 1);
    }
    if (len & 1) {
        tmp[len - 1] = in_even[len / 2] + tmp[len - 2];
    }
#else
    OPJ_INT32 s1, s2, dc, dn;

    assert(len > 2);

    /* Improved version of the TWO_PASS_VERSION: */
    /* Performs lifting in one single iteration. Saves memory */
    /* accesses and explicit interleaving. */

    s1 = in_even[1];
    dc = in_odd[0] - ((in_even[0] + s1 + 2) >> 2);
    tmp[0] = in_even[0] + dc;

    for (i = 1, j = 1; i < (len - 2 - !(len & 1)); i += 2, j++) {

        s2 = in_even[j + 1];

        dn = in_odd[j] - ((s1 + s2 + 2) >> 2);
        tmp[i  ] = dc;
        tmp[i + 1] = s1 + ((dn + dc) >> 1);

        dc = dn;
        s1 = s2;
    }

    tmp[i] = dc;

    if (!(len & 1)) {
        dn = in_odd[len / 2 - 1] - ((s1 + 1) >> 1);
        tmp[len - 2] = s1 + ((dn + dc) >> 1);
        tmp[len - 1] = dn;
    } else {
        tmp[len - 1] = s1 + dc;
    }
#endif
    memcpy(tiledp, tmp, (OPJ_UINT32)len * sizeof(OPJ_INT32));
}


#endif /* !defined(STANDARD_SLOW_VERSION) */

/* <summary>                            */
/* Inverse 5-3 wavelet transform in 1-D for one row. */
/* </summary>                           */
/* Performs interleave, inverse wavelet transform and copy back to buffer */
static void opj_idwt53_h(const opj_dwt_t *dwt,
                         OPJ_INT32* tiledp)
{
#ifdef STANDARD_SLOW_VERSION
    /* For documentation purpose */
    opj_dwt_interleave_h(dwt, tiledp);
    opj_dwt_decode_1(dwt);
    memcpy(tiledp, dwt->mem, (OPJ_UINT32)(dwt->sn + dwt->dn) * sizeof(OPJ_INT32));
#else
    const OPJ_INT32 sn = dwt->sn;
    const OPJ_INT32 len = sn + dwt->dn;
    if (dwt->cas == 0) { /* Left-most sample is on even coordinate */
        if (len > 1) {
            opj_idwt53_h_cas0(dwt->mem, sn, len, tiledp);
        } else {
            /* Unmodified value */
        }
    } else { /* Left-most sample is on odd coordinate */
        if (len == 1) {
            tiledp[0] /= 2;
        } else if (len == 2) {
            OPJ_INT32* out = dwt->mem;
            const OPJ_INT32* in_even = &tiledp[sn];
            const OPJ_INT32* in_odd = &tiledp[0];
            out[1] = in_odd[0] - ((in_even[0] + 1) >> 1);
            out[0] = in_even[0] + out[1];
            memcpy(tiledp, dwt->mem, (OPJ_UINT32)len * sizeof(OPJ_INT32));
        } else if (len > 2) {
            opj_idwt53_h_cas1(dwt->mem, sn, len, tiledp);
        }
    }
#endif
}

#if (defined(__SSE2__) || defined(__AVX2__)) && !defined(STANDARD_SLOW_VERSION)

/* Conveniency macros to improve the readabilty of the formulas */
#if __AVX2__
#define VREG        __m256i
#define LOAD_CST(x) _mm256_set1_epi32(x)
#define LOAD(x)     _mm256_load_si256((const VREG*)(x))
#define LOADU(x)    _mm256_loadu_si256((const VREG*)(x))
#define STORE(x,y)  _mm256_store_si256((VREG*)(x),(y))
#define STOREU(x,y) _mm256_storeu_si256((VREG*)(x),(y))
#define ADD(x,y)    _mm256_add_epi32((x),(y))
#define SUB(x,y)    _mm256_sub_epi32((x),(y))
#define SAR(x,y)    _mm256_srai_epi32((x),(y))
#else
#define VREG        __m128i
#define LOAD_CST(x) _mm_set1_epi32(x)
#define LOAD(x)     _mm_load_si128((const VREG*)(x))
#define LOADU(x)    _mm_loadu_si128((const VREG*)(x))
#define STORE(x,y)  _mm_store_si128((VREG*)(x),(y))
#define STOREU(x,y) _mm_storeu_si128((VREG*)(x),(y))
#define ADD(x,y)    _mm_add_epi32((x),(y))
#define SUB(x,y)    _mm_sub_epi32((x),(y))
#define SAR(x,y)    _mm_srai_epi32((x),(y))
#endif
#define ADD3(x,y,z) ADD(ADD(x,y),z)

static
void opj_idwt53_v_final_memcpy(OPJ_INT32* tiledp_col,
                               const OPJ_INT32* tmp,
                               OPJ_INT32 len,
                               OPJ_INT32 stride)
{
    OPJ_INT32 i;
    for (i = 0; i < len; ++i) {
        /* A memcpy(&tiledp_col[i * stride + 0],
                    &tmp[PARALLEL_COLS_53 * i + 0],
                    PARALLEL_COLS_53 * sizeof(OPJ_INT32))
           would do but would be a tiny bit slower.
           We can take here advantage of our knowledge of alignment */
        STOREU(&tiledp_col[i * stride + 0],
               LOAD(&tmp[PARALLEL_COLS_53 * i + 0]));
        STOREU(&tiledp_col[i * stride + VREG_INT_COUNT],
               LOAD(&tmp[PARALLEL_COLS_53 * i + VREG_INT_COUNT]));
    }
}

/** Vertical inverse 5x3 wavelet transform for 8 columns in SSE2, or
 * 16 in AVX2, when top-most pixel is on even coordinate */
static void opj_idwt53_v_cas0_mcols_SSE2_OR_AVX2(
    OPJ_INT32* tmp,
    const OPJ_INT32 sn,
    const OPJ_INT32 len,
    OPJ_INT32* tiledp_col,
    const OPJ_INT32 stride)
{
    const OPJ_INT32* in_even = &tiledp_col[0];
    const OPJ_INT32* in_odd = &tiledp_col[sn * stride];

    OPJ_INT32 i, j;
    VREG d1c_0, d1n_0, s1n_0, s0c_0, s0n_0;
    VREG d1c_1, d1n_1, s1n_1, s0c_1, s0n_1;
    const VREG two = LOAD_CST(2);

    assert(len > 1);
#if __AVX2__
    assert(PARALLEL_COLS_53 == 16);
    assert(VREG_INT_COUNT == 8);
#else
    assert(PARALLEL_COLS_53 == 8);
    assert(VREG_INT_COUNT == 4);
#endif

    /* Note: loads of input even/odd values must be done in a unaligned */
    /* fashion. But stores in tmp can be done with aligned store, since */
    /* the temporary buffer is properly aligned */
    assert((size_t)tmp % (sizeof(OPJ_INT32) * VREG_INT_COUNT) == 0);

    s1n_0 = LOADU(in_even + 0);
    s1n_1 = LOADU(in_even + VREG_INT_COUNT);
    d1n_0 = LOADU(in_odd);
    d1n_1 = LOADU(in_odd + VREG_INT_COUNT);

    /* s0n = s1n - ((d1n + 1) >> 1); <==> */
    /* s0n = s1n - ((d1n + d1n + 2) >> 2); */
    s0n_0 = SUB(s1n_0, SAR(ADD3(d1n_0, d1n_0, two), 2));
    s0n_1 = SUB(s1n_1, SAR(ADD3(d1n_1, d1n_1, two), 2));

    for (i = 0, j = 1; i < (len - 3); i += 2, j++) {
        d1c_0 = d1n_0;
        s0c_0 = s0n_0;
        d1c_1 = d1n_1;
        s0c_1 = s0n_1;

        s1n_0 = LOADU(in_even + j * stride);
        s1n_1 = LOADU(in_even + j * stride + VREG_INT_COUNT);
        d1n_0 = LOADU(in_odd + j * stride);
        d1n_1 = LOADU(in_odd + j * stride + VREG_INT_COUNT);

        /*s0n = s1n - ((d1c + d1n + 2) >> 2);*/
        s0n_0 = SUB(s1n_0, SAR(ADD3(d1c_0, d1n_0, two), 2));
        s0n_1 = SUB(s1n_1, SAR(ADD3(d1c_1, d1n_1, two), 2));

        STORE(tmp + PARALLEL_COLS_53 * (i + 0), s0c_0);
        STORE(tmp + PARALLEL_COLS_53 * (i + 0) + VREG_INT_COUNT, s0c_1);

        /* d1c + ((s0c + s0n) >> 1) */
        STORE(tmp + PARALLEL_COLS_53 * (i + 1) + 0,
              ADD(d1c_0, SAR(ADD(s0c_0, s0n_0), 1)));
        STORE(tmp + PARALLEL_COLS_53 * (i + 1) + VREG_INT_COUNT,
              ADD(d1c_1, SAR(ADD(s0c_1, s0n_1), 1)));
    }

    STORE(tmp + PARALLEL_COLS_53 * (i + 0) + 0, s0n_0);
    STORE(tmp + PARALLEL_COLS_53 * (i + 0) + VREG_INT_COUNT, s0n_1);

    if (len & 1) {
        VREG tmp_len_minus_1;
        s1n_0 = LOADU(in_even + ((len - 1) / 2) * stride);
        /* tmp_len_minus_1 = s1n - ((d1n + 1) >> 1); */
        tmp_len_minus_1 = SUB(s1n_0, SAR(ADD3(d1n_0, d1n_0, two), 2));
        STORE(tmp + PARALLEL_COLS_53 * (len - 1), tmp_len_minus_1);
        /* d1n + ((s0n + tmp_len_minus_1) >> 1) */
        STORE(tmp + PARALLEL_COLS_53 * (len - 2),
              ADD(d1n_0, SAR(ADD(s0n_0, tmp_len_minus_1), 1)));

        s1n_1 = LOADU(in_even + ((len - 1) / 2) * stride + VREG_INT_COUNT);
        /* tmp_len_minus_1 = s1n - ((d1n + 1) >> 1); */
        tmp_len_minus_1 = SUB(s1n_1, SAR(ADD3(d1n_1, d1n_1, two), 2));
        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + VREG_INT_COUNT,
              tmp_len_minus_1);
        /* d1n + ((s0n + tmp_len_minus_1) >> 1) */
        STORE(tmp + PARALLEL_COLS_53 * (len - 2) + VREG_INT_COUNT,
              ADD(d1n_1, SAR(ADD(s0n_1, tmp_len_minus_1), 1)));


    } else {
        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + 0,
              ADD(d1n_0, s0n_0));
        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + VREG_INT_COUNT,
              ADD(d1n_1, s0n_1));
    }

    opj_idwt53_v_final_memcpy(tiledp_col, tmp, len, stride);
}


/** Vertical inverse 5x3 wavelet transform for 8 columns in SSE2, or
 * 16 in AVX2, when top-most pixel is on odd coordinate */
static void opj_idwt53_v_cas1_mcols_SSE2_OR_AVX2(
    OPJ_INT32* tmp,
    const OPJ_INT32 sn,
    const OPJ_INT32 len,
    OPJ_INT32* tiledp_col,
    const OPJ_INT32 stride)
{
    OPJ_INT32 i, j;

    VREG s1_0, s2_0, dc_0, dn_0;
    VREG s1_1, s2_1, dc_1, dn_1;
    const VREG two = LOAD_CST(2);

    const OPJ_INT32* in_even = &tiledp_col[sn * stride];
    const OPJ_INT32* in_odd = &tiledp_col[0];

    assert(len > 2);
#if __AVX2__
    assert(PARALLEL_COLS_53 == 16);
    assert(VREG_INT_COUNT == 8);
#else
    assert(PARALLEL_COLS_53 == 8);
    assert(VREG_INT_COUNT == 4);
#endif

    /* Note: loads of input even/odd values must be done in a unaligned */
    /* fashion. But stores in tmp can be done with aligned store, since */
    /* the temporary buffer is properly aligned */
    assert((size_t)tmp % (sizeof(OPJ_INT32) * VREG_INT_COUNT) == 0);

    s1_0 = LOADU(in_even + stride);
    /* in_odd[0] - ((in_even[0] + s1 + 2) >> 2); */
    dc_0 = SUB(LOADU(in_odd + 0),
               SAR(ADD3(LOADU(in_even + 0), s1_0, two), 2));
    STORE(tmp + PARALLEL_COLS_53 * 0, ADD(LOADU(in_even + 0), dc_0));

    s1_1 = LOADU(in_even + stride + VREG_INT_COUNT);
    /* in_odd[0] - ((in_even[0] + s1 + 2) >> 2); */
    dc_1 = SUB(LOADU(in_odd + VREG_INT_COUNT),
               SAR(ADD3(LOADU(in_even + VREG_INT_COUNT), s1_1, two), 2));
    STORE(tmp + PARALLEL_COLS_53 * 0 + VREG_INT_COUNT,
          ADD(LOADU(in_even + VREG_INT_COUNT), dc_1));

    for (i = 1, j = 1; i < (len - 2 - !(len & 1)); i += 2, j++) {

        s2_0 = LOADU(in_even + (j + 1) * stride);
        s2_1 = LOADU(in_even + (j + 1) * stride + VREG_INT_COUNT);

        /* dn = in_odd[j * stride] - ((s1 + s2 + 2) >> 2); */
        dn_0 = SUB(LOADU(in_odd + j * stride),
                   SAR(ADD3(s1_0, s2_0, two), 2));
        dn_1 = SUB(LOADU(in_odd + j * stride + VREG_INT_COUNT),
                   SAR(ADD3(s1_1, s2_1, two), 2));

        STORE(tmp + PARALLEL_COLS_53 * i, dc_0);
        STORE(tmp + PARALLEL_COLS_53 * i + VREG_INT_COUNT, dc_1);

        /* tmp[i + 1] = s1 + ((dn + dc) >> 1); */
        STORE(tmp + PARALLEL_COLS_53 * (i + 1) + 0,
              ADD(s1_0, SAR(ADD(dn_0, dc_0), 1)));
        STORE(tmp + PARALLEL_COLS_53 * (i + 1) + VREG_INT_COUNT,
              ADD(s1_1, SAR(ADD(dn_1, dc_1), 1)));

        dc_0 = dn_0;
        s1_0 = s2_0;
        dc_1 = dn_1;
        s1_1 = s2_1;
    }
    STORE(tmp + PARALLEL_COLS_53 * i, dc_0);
    STORE(tmp + PARALLEL_COLS_53 * i + VREG_INT_COUNT, dc_1);

    if (!(len & 1)) {
        /*dn = in_odd[(len / 2 - 1) * stride] - ((s1 + 1) >> 1); */
        dn_0 = SUB(LOADU(in_odd + (len / 2 - 1) * stride),
                   SAR(ADD3(s1_0, s1_0, two), 2));
        dn_1 = SUB(LOADU(in_odd + (len / 2 - 1) * stride + VREG_INT_COUNT),
                   SAR(ADD3(s1_1, s1_1, two), 2));

        /* tmp[len - 2] = s1 + ((dn + dc) >> 1); */
        STORE(tmp + PARALLEL_COLS_53 * (len - 2) + 0,
              ADD(s1_0, SAR(ADD(dn_0, dc_0), 1)));
        STORE(tmp + PARALLEL_COLS_53 * (len - 2) + VREG_INT_COUNT,
              ADD(s1_1, SAR(ADD(dn_1, dc_1), 1)));

        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + 0, dn_0);
        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + VREG_INT_COUNT, dn_1);
    } else {
        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + 0, ADD(s1_0, dc_0));
        STORE(tmp + PARALLEL_COLS_53 * (len - 1) + VREG_INT_COUNT,
              ADD(s1_1, dc_1));
    }

    opj_idwt53_v_final_memcpy(tiledp_col, tmp, len, stride);
}

#undef VREG
#undef LOAD_CST
#undef LOADU
#undef LOAD
#undef STORE
#undef STOREU
#undef ADD
#undef ADD3
#undef SUB
#undef SAR

#endif /* (defined(__SSE2__) || defined(__AVX2__)) && !defined(STANDARD_SLOW_VERSION) */

#if !defined(STANDARD_SLOW_VERSION)
/** Vertical inverse 5x3 wavelet transform for one column, when top-most
 * pixel is on even coordinate */
static void opj_idwt3_v_cas0(OPJ_INT32* tmp,
                             const OPJ_INT32 sn,
                             const OPJ_INT32 len,
                             OPJ_INT32* tiledp_col,
                             const OPJ_INT32 stride)
{
    OPJ_INT32 i, j;
    OPJ_INT32 d1c, d1n, s1n, s0c, s0n;

    assert(len > 1);

    /* Performs lifting in one single iteration. Saves memory */
    /* accesses and explicit interleaving. */

    s1n = tiledp_col[0];
    d1n = tiledp_col[sn * stride];
    s0n = s1n - ((d1n + 1) >> 1);

    for (i = 0, j = 0; i < (len - 3); i += 2, j++) {
        d1c = d1n;
        s0c = s0n;

        s1n = tiledp_col[(j + 1) * stride];
        d1n = tiledp_col[(sn + j + 1) * stride];

        s0n = s1n - ((d1c + d1n + 2) >> 2);

        tmp[i  ] = s0c;
        tmp[i + 1] = d1c + ((s0c + s0n) >> 1);
    }

    tmp[i] = s0n;

    if (len & 1) {
        tmp[len - 1] =
            tiledp_col[((len - 1) / 2) * stride] -
            ((d1n + 1) >> 1);
        tmp[len - 2] = d1n + ((s0n + tmp[len - 1]) >> 1);
    } else {
        tmp[len - 1] = d1n + s0n;
    }

    for (i = 0; i < len; ++i) {
        tiledp_col[i * stride] = tmp[i];
    }
}


/** Vertical inverse 5x3 wavelet transform for one column, when top-most
 * pixel is on odd coordinate */
static void opj_idwt3_v_cas1(OPJ_INT32* tmp,
                             const OPJ_INT32 sn,
                             const OPJ_INT32 len,
                             OPJ_INT32* tiledp_col,
                             const OPJ_INT32 stride)
{
    OPJ_INT32 i, j;
    OPJ_INT32 s1, s2, dc, dn;
    const OPJ_INT32* in_even = &tiledp_col[sn * stride];
    const OPJ_INT32* in_odd = &tiledp_col[0];

    assert(len > 2);

    /* Performs lifting in one single iteration. Saves memory */
    /* accesses and explicit interleaving. */

    s1 = in_even[stride];
    dc = in_odd[0] - ((in_even[0] + s1 + 2) >> 2);
    tmp[0] = in_even[0] + dc;
    for (i = 1, j = 1; i < (len - 2 - !(len & 1)); i += 2, j++) {

        s2 = in_even[(j + 1) * stride];

        dn = in_odd[j * stride] - ((s1 + s2 + 2) >> 2);
        tmp[i  ] = dc;
        tmp[i + 1] = s1 + ((dn + dc) >> 1);

        dc = dn;
        s1 = s2;
    }
    tmp[i] = dc;
    if (!(len & 1)) {
        dn = in_odd[(len / 2 - 1) * stride] - ((s1 + 1) >> 1);
        tmp[len - 2] = s1 + ((dn + dc) >> 1);
        tmp[len - 1] = dn;
    } else {
        tmp[len - 1] = s1 + dc;
    }

    for (i = 0; i < len; ++i) {
        tiledp_col[i * stride] = tmp[i];
    }
}
#endif /* !defined(STANDARD_SLOW_VERSION) */

/* <summary>                            */
/* Inverse vertical 5-3 wavelet transform in 1-D for several columns. */
/* </summary>                           */
/* Performs interleave, inverse wavelet transform and copy back to buffer */
static void opj_idwt53_v(const opj_dwt_t *dwt,
                         OPJ_INT32* tiledp_col,
                         OPJ_INT32 stride,
                         OPJ_INT32 nb_cols)
{
#ifdef STANDARD_SLOW_VERSION
    /* For documentation purpose */
    OPJ_INT32 k, c;
    for (c = 0; c < nb_cols; c ++) {
        opj_dwt_interleave_v(dwt, tiledp_col + c, stride);
        opj_dwt_decode_1(dwt);
        for (k = 0; k < dwt->sn + dwt->dn; ++k) {
            tiledp_col[c + k * stride] = dwt->mem[k];
        }
    }
#else
    const OPJ_INT32 sn = dwt->sn;
    const OPJ_INT32 len = sn + dwt->dn;
    if (dwt->cas == 0) {
        /* If len == 1, unmodified value */

#if (defined(__SSE2__) || defined(__AVX2__))
        if (len > 1 && nb_cols == PARALLEL_COLS_53) {
            /* Same as below general case, except that thanks to SSE2/AVX2 */
            /* we can efficently process 8/16 columns in parallel */
            opj_idwt53_v_cas0_mcols_SSE2_OR_AVX2(dwt->mem, sn, len, tiledp_col, stride);
            return;
        }
#endif
        if (len > 1) {
            OPJ_INT32 c;
            for (c = 0; c < nb_cols; c++, tiledp_col++) {
                opj_idwt3_v_cas0(dwt->mem, sn, len, tiledp_col, stride);
            }
            return;
        }
    } else {
        if (len == 1) {
            OPJ_INT32 c;
            for (c = 0; c < nb_cols; c++, tiledp_col++) {
                tiledp_col[0] /= 2;
            }
            return;
        }

        if (len == 2) {
            OPJ_INT32 c;
            OPJ_INT32* out = dwt->mem;
            for (c = 0; c < nb_cols; c++, tiledp_col++) {
                OPJ_INT32 i;
                const OPJ_INT32* in_even = &tiledp_col[sn * stride];
                const OPJ_INT32* in_odd = &tiledp_col[0];

                out[1] = in_odd[0] - ((in_even[0] + 1) >> 1);
                out[0] = in_even[0] + out[1];

                for (i = 0; i < len; ++i) {
                    tiledp_col[i * stride] = out[i];
                }
            }

            return;
        }

#if (defined(__SSE2__) || defined(__AVX2__))
        if (len > 2 && nb_cols == PARALLEL_COLS_53) {
            /* Same as below general case, except that thanks to SSE2/AVX2 */
            /* we can efficently process 8/16 columns in parallel */
            opj_idwt53_v_cas1_mcols_SSE2_OR_AVX2(dwt->mem, sn, len, tiledp_col, stride);
            return;
        }
#endif
        if (len > 2) {
            OPJ_INT32 c;
            for (c = 0; c < nb_cols; c++, tiledp_col++) {
                opj_idwt3_v_cas1(dwt->mem, sn, len, tiledp_col, stride);
            }
            return;
        }
    }
#endif
}


/* <summary>                             */
/* Forward 9-7 wavelet transform in 1-D. */
/* </summary>                            */
static void opj_dwt_encode_1_real(OPJ_INT32 *a, OPJ_SIZE_T a_count,
                                  OPJ_INT32 dn, OPJ_INT32 sn, OPJ_INT32 cas)
{
    OPJ_INT32 i;
    if (!cas) {
        if ((dn > 0) || (sn > 1)) { /* NEW :  CASE ONE ELEMENT */
            for (i = 0; i < dn; i++) {
                OPJ_D(i) -= opj_int_fix_mul(OPJ_S_(i) + OPJ_S_(i + 1), 12993);
            }
            for (i = 0; i < sn; i++) {
                OPJ_S(i) -= opj_int_fix_mul(OPJ_D_(i - 1) + OPJ_D_(i), 434);
            }
            for (i = 0; i < dn; i++) {
                OPJ_D(i) += opj_int_fix_mul(OPJ_S_(i) + OPJ_S_(i + 1), 7233);
            }
            for (i = 0; i < sn; i++) {
                OPJ_S(i) += opj_int_fix_mul(OPJ_D_(i - 1) + OPJ_D_(i), 3633);
            }
            for (i = 0; i < dn; i++) {
                OPJ_D(i) = opj_int_fix_mul(OPJ_D(i), 5038);    /*5038 */
            }
            for (i = 0; i < sn; i++) {
                OPJ_S(i) = opj_int_fix_mul(OPJ_S(i), 6659);    /*6660 */
            }
        }
    } else {
        if ((sn > 0) || (dn > 1)) { /* NEW :  CASE ONE ELEMENT */
            for (i = 0; i < dn; i++) {
                OPJ_S(i) -= opj_int_fix_mul(OPJ_DD_(i) + OPJ_DD_(i - 1), 12993);
            }
            for (i = 0; i < sn; i++) {
                OPJ_D(i) -= opj_int_fix_mul(OPJ_SS_(i) + OPJ_SS_(i + 1), 434);
            }
            for (i = 0; i < dn; i++) {
                OPJ_S(i) += opj_int_fix_mul(OPJ_DD_(i) + OPJ_DD_(i - 1), 7233);
            }
            for (i = 0; i < sn; i++) {
                OPJ_D(i) += opj_int_fix_mul(OPJ_SS_(i) + OPJ_SS_(i + 1), 3633);
            }
            for (i = 0; i < dn; i++) {
                OPJ_S(i) = opj_int_fix_mul(OPJ_S(i), 5038);    /*5038 */
            }
            for (i = 0; i < sn; i++) {
                OPJ_D(i) = opj_int_fix_mul(OPJ_D(i), 6659);    /*6660 */
            }
        }
    }
}

static void opj_dwt_encode_stepsize(OPJ_INT32 stepsize, OPJ_INT32 numbps,
                                    opj_stepsize_t *bandno_stepsize)
{
    OPJ_INT32 p, n;
    p = opj_int_floorlog2(stepsize) - 13;
    n = 11 - opj_int_floorlog2(stepsize);
    bandno_stepsize->mant = (n < 0 ? stepsize >> -n : stepsize << n) & 0x7ff;
    bandno_stepsize->expn = numbps - p;
}

/*
==========================================================
   DWT interface
==========================================================
*/


/* <summary>                            */
/* Forward 5-3 wavelet transform in 2-D. */
/* </summary>                           */
static INLINE OPJ_BOOL opj_dwt_encode_procedure(const opj_tcd_tilecomp_t * tilec,
        void(*p_function)(OPJ_INT32 *, OPJ_SIZE_T, OPJ_INT32, OPJ_INT32, OPJ_INT32))
{
    OPJ_INT32 i, j, k;
    OPJ_INT32 *a = 00;
    OPJ_INT32 *aj = 00;
    OPJ_INT32 *bj = 00;
    OPJ_INT32 w, l;

    OPJ_INT32 rw;           /* width of the resolution level computed   */
    OPJ_INT32 rh;           /* height of the resolution level computed  */
    size_t l_data_count;
    size_t l_data_size;

    opj_tcd_resolution_t * l_cur_res = 0;
    opj_tcd_resolution_t * l_last_res = 0;

    w = tilec->x1 - tilec->x0;
    l = (OPJ_INT32)tilec->numresolutions - 1;
    a = tilec->data;

    l_cur_res = tilec->resolutions + l;
    l_last_res = l_cur_res - 1;

    l_data_count = opj_dwt_max_resolution(tilec->resolutions, tilec->numresolutions);
    /* overflow check */
    if (l_data_count > (SIZE_MAX / sizeof(OPJ_INT32))) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    l_data_size = l_data_count * sizeof(OPJ_INT32);
    bj = (OPJ_INT32*)opj_malloc(l_data_size);
    /* l_data_size is equal to 0 when numresolutions == 1 but bj is not used */
    /* in that case, so do not error out */
    if (l_data_size != 0 && ! bj) {
        return OPJ_FALSE;
    }
    i = l;

    while (i--) {
        OPJ_INT32 rw1;      /* width of the resolution level once lower than computed one                                       */
        OPJ_INT32 rh1;      /* height of the resolution level once lower than computed one                                      */
        OPJ_INT32 cas_col;  /* 0 = non inversion on horizontal filtering 1 = inversion between low-pass and high-pass filtering */
        OPJ_INT32 cas_row;  /* 0 = non inversion on vertical filtering 1 = inversion between low-pass and high-pass filtering   */
        OPJ_INT32 dn, sn;

        rw  = l_cur_res->x1 - l_cur_res->x0;
        rh  = l_cur_res->y1 - l_cur_res->y0;
        rw1 = l_last_res->x1 - l_last_res->x0;
        rh1 = l_last_res->y1 - l_last_res->y0;

        cas_row = l_cur_res->x0 & 1;
        cas_col = l_cur_res->y0 & 1;

        sn = rh1;
        dn = rh - rh1;
        for (j = 0; j < rw; ++j) {
            aj = a + j;
            for (k = 0; k < rh; ++k) {
                bj[k] = aj[k * w];
            }

            (*p_function) (bj, l_data_count, dn, sn, cas_col);

            opj_dwt_deinterleave_v(bj, aj, dn, sn, w, cas_col);
        }

        sn = rw1;
        dn = rw - rw1;

        for (j = 0; j < rh; j++) {
            aj = a + j * w;
            for (k = 0; k < rw; k++) {
                bj[k] = aj[k];
            }
            (*p_function) (bj, l_data_count, dn, sn, cas_row);
            opj_dwt_deinterleave_h(bj, aj, dn, sn, cas_row);
        }

        l_cur_res = l_last_res;

        --l_last_res;
    }

    opj_free(bj);
    return OPJ_TRUE;
}

/* Forward 5-3 wavelet transform in 2-D. */
/* </summary>                           */
OPJ_BOOL opj_dwt_encode(opj_tcd_tilecomp_t * tilec)
{
    return opj_dwt_encode_procedure(tilec, opj_dwt_encode_1);
}

/* <summary>                            */
/* Inverse 5-3 wavelet transform in 2-D. */
/* </summary>                           */
OPJ_BOOL opj_dwt_decode(opj_thread_pool_t* tp, opj_tcd_tilecomp_t* tilec,
                        OPJ_UINT32 numres)
{
    return opj_dwt_decode_tile(tp, tilec, numres);
}


/* <summary>                          */
/* Get gain of 5-3 wavelet transform. */
/* </summary>                         */
OPJ_UINT32 opj_dwt_getgain(OPJ_UINT32 orient)
{
    if (orient == 0) {
        return 0;
    }
    if (orient == 1 || orient == 2) {
        return 1;
    }
    return 2;
}

/* <summary>                */
/* Get norm of 5-3 wavelet. */
/* </summary>               */
OPJ_FLOAT64 opj_dwt_getnorm(OPJ_UINT32 level, OPJ_UINT32 orient)
{
    return opj_dwt_norms[orient][level];
}

/* <summary>                             */
/* Forward 9-7 wavelet transform in 2-D. */
/* </summary>                            */
OPJ_BOOL opj_dwt_encode_real(opj_tcd_tilecomp_t * tilec)
{
    return opj_dwt_encode_procedure(tilec, opj_dwt_encode_1_real);
}

/* <summary>                          */
/* Get gain of 9-7 wavelet transform. */
/* </summary>                         */
OPJ_UINT32 opj_dwt_getgain_real(OPJ_UINT32 orient)
{
    (void)orient;
    return 0;
}

/* <summary>                */
/* Get norm of 9-7 wavelet. */
/* </summary>               */
OPJ_FLOAT64 opj_dwt_getnorm_real(OPJ_UINT32 level, OPJ_UINT32 orient)
{
    return opj_dwt_norms_real[orient][level];
}

void opj_dwt_calc_explicit_stepsizes(opj_tccp_t * tccp, OPJ_UINT32 prec)
{
    OPJ_UINT32 numbands, bandno;
    numbands = 3 * tccp->numresolutions - 2;
    for (bandno = 0; bandno < numbands; bandno++) {
        OPJ_FLOAT64 stepsize;
        OPJ_UINT32 resno, level, orient, gain;

        resno = (bandno == 0) ? 0 : ((bandno - 1) / 3 + 1);
        orient = (bandno == 0) ? 0 : ((bandno - 1) % 3 + 1);
        level = tccp->numresolutions - 1 - resno;
        gain = (tccp->qmfbid == 0) ? 0 : ((orient == 0) ? 0 : (((orient == 1) ||
                                          (orient == 2)) ? 1 : 2));
        if (tccp->qntsty == J2K_CCP_QNTSTY_NOQNT) {
            stepsize = 1.0;
        } else {
            OPJ_FLOAT64 norm = opj_dwt_norms_real[orient][level];
            stepsize = (1 << (gain)) / norm;
        }
        opj_dwt_encode_stepsize((OPJ_INT32) floor(stepsize * 8192.0),
                                (OPJ_INT32)(prec + gain), &tccp->stepsizes[bandno]);
    }
}

/* <summary>                             */
/* Determine maximum computed resolution level for inverse wavelet transform */
/* </summary>                            */
static OPJ_UINT32 opj_dwt_max_resolution(opj_tcd_resolution_t* OPJ_RESTRICT r,
        OPJ_UINT32 i)
{
    OPJ_UINT32 mr   = 0;
    OPJ_UINT32 w;
    while (--i) {
        ++r;
        if (mr < (w = (OPJ_UINT32)(r->x1 - r->x0))) {
            mr = w ;
        }
        if (mr < (w = (OPJ_UINT32)(r->y1 - r->y0))) {
            mr = w ;
        }
    }
    return mr ;
}

typedef struct {
    opj_dwt_t h;
    OPJ_UINT32 rw;
    OPJ_UINT32 w;
    OPJ_INT32 * OPJ_RESTRICT tiledp;
    OPJ_UINT32 min_j;
    OPJ_UINT32 max_j;
} opj_dwd_decode_h_job_t;

static void opj_dwt_decode_h_func(void* user_data, opj_tls_t* tls)
{
    OPJ_UINT32 j;
    opj_dwd_decode_h_job_t* job;
    (void)tls;

    job = (opj_dwd_decode_h_job_t*)user_data;
    for (j = job->min_j; j < job->max_j; j++) {
        opj_idwt53_h(&job->h, &job->tiledp[j * job->w]);
    }

    opj_aligned_free(job->h.mem);
    opj_free(job);
}

typedef struct {
    opj_dwt_t v;
    OPJ_UINT32 rh;
    OPJ_UINT32 w;
    OPJ_INT32 * OPJ_RESTRICT tiledp;
    OPJ_UINT32 min_j;
    OPJ_UINT32 max_j;
} opj_dwd_decode_v_job_t;

static void opj_dwt_decode_v_func(void* user_data, opj_tls_t* tls)
{
    OPJ_UINT32 j;
    opj_dwd_decode_v_job_t* job;
    (void)tls;

    job = (opj_dwd_decode_v_job_t*)user_data;
    for (j = job->min_j; j + PARALLEL_COLS_53 <= job->max_j;
            j += PARALLEL_COLS_53) {
        opj_idwt53_v(&job->v, &job->tiledp[j], (OPJ_INT32)job->w,
                     PARALLEL_COLS_53);
    }
    if (j < job->max_j)
        opj_idwt53_v(&job->v, &job->tiledp[j], (OPJ_INT32)job->w,
                     (OPJ_INT32)(job->max_j - j));

    opj_aligned_free(job->v.mem);
    opj_free(job);
}


/* <summary>                            */
/* Inverse wavelet transform in 2-D.    */
/* </summary>                           */
static OPJ_BOOL opj_dwt_decode_tile(opj_thread_pool_t* tp,
        const opj_tcd_tilecomp_t* tilec, OPJ_UINT32 numres)
{
    opj_dwt_t h;
    opj_dwt_t v;

    opj_tcd_resolution_t* tr = tilec->resolutions;

    OPJ_UINT32 rw = (OPJ_UINT32)(tr->x1 -
                                 tr->x0);  /* width of the resolution level computed */
    OPJ_UINT32 rh = (OPJ_UINT32)(tr->y1 -
                                 tr->y0);  /* height of the resolution level computed */

    OPJ_UINT32 w = (OPJ_UINT32)(tilec->x1 - tilec->x0);
    size_t h_mem_size;
    int num_threads;

    if (numres == 1U) {
        return OPJ_TRUE;
    }
    num_threads = opj_thread_pool_get_thread_count(tp);
    h.mem_count = opj_dwt_max_resolution(tr, numres);
    /* overflow check */
    if (h.mem_count > (SIZE_MAX / PARALLEL_COLS_53 / sizeof(OPJ_INT32))) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    /* We need PARALLEL_COLS_53 times the height of the array, */
    /* since for the vertical pass */
    /* we process PARALLEL_COLS_53 columns at a time */
    h_mem_size = h.mem_count * PARALLEL_COLS_53 * sizeof(OPJ_INT32);
    h.mem = (OPJ_INT32*)opj_aligned_32_malloc(h_mem_size);
    if (! h.mem) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }

    v.mem_count = h.mem_count;
    v.mem = h.mem;

    while (--numres) {
        OPJ_INT32 * OPJ_RESTRICT tiledp = tilec->data;
        OPJ_UINT32 j;

        ++tr;
        h.sn = (OPJ_INT32)rw;
        v.sn = (OPJ_INT32)rh;

        rw = (OPJ_UINT32)(tr->x1 - tr->x0);
        rh = (OPJ_UINT32)(tr->y1 - tr->y0);

        h.dn = (OPJ_INT32)(rw - (OPJ_UINT32)h.sn);
        h.cas = tr->x0 % 2;

        if (num_threads <= 1 || rh <= 1) {
            for (j = 0; j < rh; ++j) {
                opj_idwt53_h(&h, &tiledp[j * w]);
            }
        } else {
            OPJ_UINT32 num_jobs = (OPJ_UINT32)num_threads;
            OPJ_UINT32 step_j;

            if (rh < num_jobs) {
                num_jobs = rh;
            }
            step_j = (rh / num_jobs);

            for (j = 0; j < num_jobs; j++) {
                opj_dwd_decode_h_job_t* job;

                job = (opj_dwd_decode_h_job_t*) opj_malloc(sizeof(opj_dwd_decode_h_job_t));
                if (!job) {
                    /* It would be nice to fallback to single thread case, but */
                    /* unfortunately some jobs may be launched and have modified */
                    /* tiledp, so it is not practical to recover from that error */
                    /* FIXME event manager error callback */
                    opj_thread_pool_wait_completion(tp, 0);
                    opj_aligned_free(h.mem);
                    return OPJ_FALSE;
                }
                job->h = h;
                job->rw = rw;
                job->w = w;
                job->tiledp = tiledp;
                job->min_j = j * step_j;
                job->max_j = (j + 1U) * step_j; /* this can overflow */
                if (j == (num_jobs - 1U)) {  /* this will take care of the overflow */
                    job->max_j = rh;
                }
                job->h.mem = (OPJ_INT32*)opj_aligned_32_malloc(h_mem_size);
                if (!job->h.mem) {
                    /* FIXME event manager error callback */
                    opj_thread_pool_wait_completion(tp, 0);
                    opj_free(job);
                    opj_aligned_free(h.mem);
                    return OPJ_FALSE;
                }
                opj_thread_pool_submit_job(tp, opj_dwt_decode_h_func, job);
            }
            opj_thread_pool_wait_completion(tp, 0);
        }

        v.dn = (OPJ_INT32)(rh - (OPJ_UINT32)v.sn);
        v.cas = tr->y0 % 2;

        if (num_threads <= 1 || rw <= 1) {
            for (j = 0; j + PARALLEL_COLS_53 <= rw;
                    j += PARALLEL_COLS_53) {
                opj_idwt53_v(&v, &tiledp[j], (OPJ_INT32)w, PARALLEL_COLS_53);
            }
            if (j < rw) {
                opj_idwt53_v(&v, &tiledp[j], (OPJ_INT32)w, (OPJ_INT32)(rw - j));
            }
        } else {
            OPJ_UINT32 num_jobs = (OPJ_UINT32)num_threads;
            OPJ_UINT32 step_j;

            if (rw < num_jobs) {
                num_jobs = rw;
            }
            step_j = (rw / num_jobs);

            for (j = 0; j < num_jobs; j++) {
                opj_dwd_decode_v_job_t* job;

                job = (opj_dwd_decode_v_job_t*) opj_malloc(sizeof(opj_dwd_decode_v_job_t));
                if (!job) {
                    /* It would be nice to fallback to single thread case, but */
                    /* unfortunately some jobs may be launched and have modified */
                    /* tiledp, so it is not practical to recover from that error */
                    /* FIXME event manager error callback */
                    opj_thread_pool_wait_completion(tp, 0);
                    opj_aligned_free(v.mem);
                    return OPJ_FALSE;
                }
                job->v = v;
                job->rh = rh;
                job->w = w;
                job->tiledp = tiledp;
                job->min_j = j * step_j;
                job->max_j = (j + 1U) * step_j; /* this can overflow */
                if (j == (num_jobs - 1U)) {  /* this will take care of the overflow */
                    job->max_j = rw;
                }
                job->v.mem = (OPJ_INT32*)opj_aligned_32_malloc(h_mem_size);
                if (!job->v.mem) {
                    /* FIXME event manager error callback */
                    opj_thread_pool_wait_completion(tp, 0);
                    opj_free(job);
                    opj_aligned_free(v.mem);
                    return OPJ_FALSE;
                }
                opj_thread_pool_submit_job(tp, opj_dwt_decode_v_func, job);
            }
            opj_thread_pool_wait_completion(tp, 0);
        }
    }
    opj_aligned_free(h.mem);
    return OPJ_TRUE;
}

static void opj_v4dwt_interleave_h(opj_v4dwt_t* OPJ_RESTRICT w,
                                   OPJ_FLOAT32* OPJ_RESTRICT a, OPJ_INT32 x, OPJ_INT32 size)
{
    OPJ_FLOAT32* OPJ_RESTRICT bi = (OPJ_FLOAT32*)(w->wavelet + w->cas);
    OPJ_INT32 count = w->sn;
    OPJ_INT32 i, k;

    for (k = 0; k < 2; ++k) {
        if (count + 3 * x < size && ((size_t) a & 0x0f) == 0 &&
                ((size_t) bi & 0x0f) == 0 && (x & 0x0f) == 0) {
            /* Fast code path */
            for (i = 0; i < count; ++i) {
                OPJ_INT32 j = i;
                bi[i * 8    ] = a[j];
                j += x;
                bi[i * 8 + 1] = a[j];
                j += x;
                bi[i * 8 + 2] = a[j];
                j += x;
                bi[i * 8 + 3] = a[j];
            }
        } else {
            /* Slow code path */
            for (i = 0; i < count; ++i) {
                OPJ_INT32 j = i;
                bi[i * 8    ] = a[j];
                j += x;
                if (j >= size) {
                    continue;
                }
                bi[i * 8 + 1] = a[j];
                j += x;
                if (j >= size) {
                    continue;
                }
                bi[i * 8 + 2] = a[j];
                j += x;
                if (j >= size) {
                    continue;
                }
                bi[i * 8 + 3] = a[j]; /* This one*/
            }
        }

        bi = (OPJ_FLOAT32*)(w->wavelet + 1 - w->cas);
        a += w->sn;
        size -= w->sn;
        count = w->dn;
    }
}

static void opj_v4dwt_interleave_v(opj_v4dwt_t* OPJ_RESTRICT v,
                                   OPJ_FLOAT32* OPJ_RESTRICT a, OPJ_INT32 x, OPJ_INT32 nb_elts_read)
{
    opj_v4_t* OPJ_RESTRICT bi = v->wavelet + v->cas;
    OPJ_INT32 i;

    for (i = 0; i < v->sn; ++i) {
        memcpy(&bi[i * 2], &a[i * x], (size_t)nb_elts_read * sizeof(OPJ_FLOAT32));
    }

    a += v->sn * x;
    bi = v->wavelet + 1 - v->cas;

    for (i = 0; i < v->dn; ++i) {
        memcpy(&bi[i * 2], &a[i * x], (size_t)nb_elts_read * sizeof(OPJ_FLOAT32));
    }
}

#ifdef __SSE__

static void opj_v4dwt_decode_step1_sse(opj_v4_t* w, OPJ_INT32 count,
                                       const __m128 c)
{
    __m128* OPJ_RESTRICT vw = (__m128*) w;
    OPJ_INT32 i;
    /* 4x unrolled loop */
    for (i = 0; i < count >> 2; ++i) {
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
    }
    count &= 3;
    for (i = 0; i < count; ++i) {
        *vw = _mm_mul_ps(*vw, c);
        vw += 2;
    }
}

void opj_v4dwt_decode_step2_sse(opj_v4_t* l, opj_v4_t* w, OPJ_INT32 k,
                                OPJ_INT32 m, __m128 c)
{
    __m128* OPJ_RESTRICT vl = (__m128*) l;
    __m128* OPJ_RESTRICT vw = (__m128*) w;
    OPJ_INT32 i;
    __m128 tmp1, tmp2, tmp3;
    tmp1 = vl[0];
    for (i = 0; i < m; ++i) {
        tmp2 = vw[-1];
        tmp3 = vw[ 0];
        vw[-1] = _mm_add_ps(tmp2, _mm_mul_ps(_mm_add_ps(tmp1, tmp3), c));
        tmp1 = tmp3;
        vw += 2;
    }
    vl = vw - 2;
    if (m >= k) {
        return;
    }
    c = _mm_add_ps(c, c);
    c = _mm_mul_ps(c, vl[0]);
    for (; m < k; ++m) {
        __m128 tmp = vw[-1];
        vw[-1] = _mm_add_ps(tmp, c);
        vw += 2;
    }
}

#else

static void opj_v4dwt_decode_step1(opj_v4_t* w, OPJ_INT32 count,
                                   const OPJ_FLOAT32 c)
{
    OPJ_FLOAT32* OPJ_RESTRICT fw = (OPJ_FLOAT32*) w;
    OPJ_INT32 i;
    for (i = 0; i < count; ++i) {
        OPJ_FLOAT32 tmp1 = fw[i * 8    ];
        OPJ_FLOAT32 tmp2 = fw[i * 8 + 1];
        OPJ_FLOAT32 tmp3 = fw[i * 8 + 2];
        OPJ_FLOAT32 tmp4 = fw[i * 8 + 3];
        fw[i * 8    ] = tmp1 * c;
        fw[i * 8 + 1] = tmp2 * c;
        fw[i * 8 + 2] = tmp3 * c;
        fw[i * 8 + 3] = tmp4 * c;
    }
}

static void opj_v4dwt_decode_step2(opj_v4_t* l, opj_v4_t* w, OPJ_INT32 k,
                                   OPJ_INT32 m, OPJ_FLOAT32 c)
{
    OPJ_FLOAT32* fl = (OPJ_FLOAT32*) l;
    OPJ_FLOAT32* fw = (OPJ_FLOAT32*) w;
    OPJ_INT32 i;
    for (i = 0; i < m; ++i) {
        OPJ_FLOAT32 tmp1_1 = fl[0];
        OPJ_FLOAT32 tmp1_2 = fl[1];
        OPJ_FLOAT32 tmp1_3 = fl[2];
        OPJ_FLOAT32 tmp1_4 = fl[3];
        OPJ_FLOAT32 tmp2_1 = fw[-4];
        OPJ_FLOAT32 tmp2_2 = fw[-3];
        OPJ_FLOAT32 tmp2_3 = fw[-2];
        OPJ_FLOAT32 tmp2_4 = fw[-1];
        OPJ_FLOAT32 tmp3_1 = fw[0];
        OPJ_FLOAT32 tmp3_2 = fw[1];
        OPJ_FLOAT32 tmp3_3 = fw[2];
        OPJ_FLOAT32 tmp3_4 = fw[3];
        fw[-4] = tmp2_1 + ((tmp1_1 + tmp3_1) * c);
        fw[-3] = tmp2_2 + ((tmp1_2 + tmp3_2) * c);
        fw[-2] = tmp2_3 + ((tmp1_3 + tmp3_3) * c);
        fw[-1] = tmp2_4 + ((tmp1_4 + tmp3_4) * c);
        fl = fw;
        fw += 8;
    }
    if (m < k) {
        OPJ_FLOAT32 c1;
        OPJ_FLOAT32 c2;
        OPJ_FLOAT32 c3;
        OPJ_FLOAT32 c4;
        c += c;
        c1 = fl[0] * c;
        c2 = fl[1] * c;
        c3 = fl[2] * c;
        c4 = fl[3] * c;
        for (; m < k; ++m) {
            OPJ_FLOAT32 tmp1 = fw[-4];
            OPJ_FLOAT32 tmp2 = fw[-3];
            OPJ_FLOAT32 tmp3 = fw[-2];
            OPJ_FLOAT32 tmp4 = fw[-1];
            fw[-4] = tmp1 + c1;
            fw[-3] = tmp2 + c2;
            fw[-2] = tmp3 + c3;
            fw[-1] = tmp4 + c4;
            fw += 8;
        }
    }
}

#endif

/* <summary>                             */
/* Inverse 9-7 wavelet transform in 1-D. */
/* </summary>                            */
static void opj_v4dwt_decode(opj_v4dwt_t* OPJ_RESTRICT dwt)
{
    OPJ_INT32 a, b;
    if (dwt->cas == 0) {
        if (!((dwt->dn > 0) || (dwt->sn > 1))) {
            return;
        }
        a = 0;
        b = 1;
    } else {
        if (!((dwt->sn > 0) || (dwt->dn > 1))) {
            return;
        }
        a = 1;
        b = 0;
    }
#ifdef __SSE__
    opj_v4dwt_decode_step1_sse(dwt->wavelet + a, dwt->sn, _mm_set1_ps(opj_K));
    opj_v4dwt_decode_step1_sse(dwt->wavelet + b, dwt->dn, _mm_set1_ps(opj_c13318));
    opj_v4dwt_decode_step2_sse(dwt->wavelet + b, dwt->wavelet + a + 1, dwt->sn,
                               opj_int_min(dwt->sn, dwt->dn - a), _mm_set1_ps(opj_dwt_delta));
    opj_v4dwt_decode_step2_sse(dwt->wavelet + a, dwt->wavelet + b + 1, dwt->dn,
                               opj_int_min(dwt->dn, dwt->sn - b), _mm_set1_ps(opj_dwt_gamma));
    opj_v4dwt_decode_step2_sse(dwt->wavelet + b, dwt->wavelet + a + 1, dwt->sn,
                               opj_int_min(dwt->sn, dwt->dn - a), _mm_set1_ps(opj_dwt_beta));
    opj_v4dwt_decode_step2_sse(dwt->wavelet + a, dwt->wavelet + b + 1, dwt->dn,
                               opj_int_min(dwt->dn, dwt->sn - b), _mm_set1_ps(opj_dwt_alpha));
#else
    opj_v4dwt_decode_step1(dwt->wavelet + a, dwt->sn, opj_K);
    opj_v4dwt_decode_step1(dwt->wavelet + b, dwt->dn, opj_c13318);
    opj_v4dwt_decode_step2(dwt->wavelet + b, dwt->wavelet + a + 1, dwt->sn,
                           opj_int_min(dwt->sn, dwt->dn - a), opj_dwt_delta);
    opj_v4dwt_decode_step2(dwt->wavelet + a, dwt->wavelet + b + 1, dwt->dn,
                           opj_int_min(dwt->dn, dwt->sn - b), opj_dwt_gamma);
    opj_v4dwt_decode_step2(dwt->wavelet + b, dwt->wavelet + a + 1, dwt->sn,
                           opj_int_min(dwt->sn, dwt->dn - a), opj_dwt_beta);
    opj_v4dwt_decode_step2(dwt->wavelet + a, dwt->wavelet + b + 1, dwt->dn,
                           opj_int_min(dwt->dn, dwt->sn - b), opj_dwt_alpha);
#endif
}


/* <summary>                             */
/* Inverse 9-7 wavelet transform in 2-D. */
/* </summary>                            */
OPJ_BOOL opj_dwt_decode_real(opj_tcd_tilecomp_t* OPJ_RESTRICT tilec,
                             OPJ_UINT32 numres)
{
    opj_v4dwt_t h;
    opj_v4dwt_t v;

    opj_tcd_resolution_t* res = tilec->resolutions;

    OPJ_UINT32 rw = (OPJ_UINT32)(res->x1 -
                                 res->x0);    /* width of the resolution level computed */
    OPJ_UINT32 rh = (OPJ_UINT32)(res->y1 -
                                 res->y0);    /* height of the resolution level computed */

    OPJ_UINT32 w = (OPJ_UINT32)(tilec->x1 - tilec->x0);

    size_t l_data_size;

    l_data_size = opj_dwt_max_resolution(res, numres);
    /* overflow check */
    if (l_data_size > (SIZE_MAX - 5U)) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    l_data_size += 5U;
    /* overflow check */
    if (l_data_size > (SIZE_MAX / sizeof(opj_v4_t))) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    h.wavelet = (opj_v4_t*) opj_aligned_malloc(l_data_size * sizeof(opj_v4_t));
    if (!h.wavelet) {
        /* FIXME event manager error callback */
        return OPJ_FALSE;
    }
    v.wavelet = h.wavelet;

    while (--numres) {
        OPJ_FLOAT32 * OPJ_RESTRICT aj = (OPJ_FLOAT32*) tilec->data;
        OPJ_UINT32 bufsize = (OPJ_UINT32)((tilec->x1 - tilec->x0) *
                                          (tilec->y1 - tilec->y0));
        OPJ_INT32 j;

        h.sn = (OPJ_INT32)rw;
        v.sn = (OPJ_INT32)rh;

        ++res;

        rw = (OPJ_UINT32)(res->x1 -
                          res->x0);   /* width of the resolution level computed */
        rh = (OPJ_UINT32)(res->y1 -
                          res->y0);   /* height of the resolution level computed */

        h.dn = (OPJ_INT32)(rw - (OPJ_UINT32)h.sn);
        h.cas = res->x0 % 2;

        for (j = (OPJ_INT32)rh; j > 3; j -= 4) {
            OPJ_INT32 k;
            opj_v4dwt_interleave_h(&h, aj, (OPJ_INT32)w, (OPJ_INT32)bufsize);
            opj_v4dwt_decode(&h);

            for (k = (OPJ_INT32)rw; --k >= 0;) {
                aj[k               ] = h.wavelet[k].f[0];
                aj[k + (OPJ_INT32)w  ] = h.wavelet[k].f[1];
                aj[k + (OPJ_INT32)w * 2] = h.wavelet[k].f[2];
                aj[k + (OPJ_INT32)w * 3] = h.wavelet[k].f[3];
            }

            aj += w * 4;
            bufsize -= w * 4;
        }

        if (rh & 0x03) {
            OPJ_INT32 k;
            j = rh & 0x03;
            opj_v4dwt_interleave_h(&h, aj, (OPJ_INT32)w, (OPJ_INT32)bufsize);
            opj_v4dwt_decode(&h);
            for (k = (OPJ_INT32)rw; --k >= 0;) {
                switch (j) {
                case 3:
                    aj[k + (OPJ_INT32)w * 2] = h.wavelet[k].f[2];
                /* FALLTHRU */
                case 2:
                    aj[k + (OPJ_INT32)w  ] = h.wavelet[k].f[1];
                /* FALLTHRU */
                case 1:
                    aj[k               ] = h.wavelet[k].f[0];
                }
            }
        }

        v.dn = (OPJ_INT32)(rh - (OPJ_UINT32)v.sn);
        v.cas = res->y0 % 2;

        aj = (OPJ_FLOAT32*) tilec->data;
        for (j = (OPJ_INT32)rw; j > 3; j -= 4) {
            OPJ_UINT32 k;

            opj_v4dwt_interleave_v(&v, aj, (OPJ_INT32)w, 4);
            opj_v4dwt_decode(&v);

            for (k = 0; k < rh; ++k) {
                memcpy(&aj[k * w], &v.wavelet[k], 4 * sizeof(OPJ_FLOAT32));
            }
            aj += 4;
        }

        if (rw & 0x03) {
            OPJ_UINT32 k;

            j = rw & 0x03;

            opj_v4dwt_interleave_v(&v, aj, (OPJ_INT32)w, j);
            opj_v4dwt_decode(&v);

            for (k = 0; k < rh; ++k) {
                memcpy(&aj[k * w], &v.wavelet[k], (size_t)j * sizeof(OPJ_FLOAT32));
            }
        }
    }

    opj_aligned_free(h.wavelet);
    return OPJ_TRUE;
}
