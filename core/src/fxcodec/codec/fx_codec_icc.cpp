// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../../third_party/lcms2-2.6/include/lcms2.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "codec_int.h"

const FX_DWORD N_COMPONENT_LAB = 3;
const FX_DWORD N_COMPONENT_GRAY = 1;
const FX_DWORD N_COMPONENT_RGB = 3;
const FX_DWORD N_COMPONENT_CMYK = 4;
const FX_DWORD N_COMPONENT_DEFAULT = 3;

FX_BOOL MD5ComputeID( const void* buf, FX_DWORD dwSize, uint8_t ID[16] )
{
    return cmsMD5computeIDExt(buf, dwSize, ID);
}
struct CLcmsCmm  {
    cmsHTRANSFORM m_hTransform;
    int			m_nSrcComponents;
    int			m_nDstComponents;
    FX_BOOL		m_bLab;
};
extern "C" {
    int ourHandler(int ErrorCode, const char *ErrorText)
    {
        return TRUE;
    }
};
FX_BOOL CheckComponents(cmsColorSpaceSignature cs, int nComponents, FX_BOOL bDst)
{
    if (nComponents <= 0 || nComponents > 15) {
        return FALSE;
    }
    switch(cs) {
        case cmsSigLabData:
            if (nComponents < 3) {
                return FALSE;
            }
            break;
        case cmsSigGrayData:
            if (bDst && nComponents != 1) {
                return FALSE;
            }
            if (!bDst && nComponents > 2) {
                return FALSE;
            }
            break;
        case cmsSigRgbData:
            if (bDst && nComponents != 3) {
                return FALSE;
            }
            break;
        case cmsSigCmykData:
            if (bDst && nComponents != 4) {
                return FALSE;
            }
            break;
        default:
            if (nComponents != 3) {
                return FALSE;
            }
            break;
    }
    return TRUE;
}
int32_t GetCSComponents(cmsColorSpaceSignature cs)
{
    FX_DWORD components;
    switch (cs) {
        case cmsSigLabData:
            components =  N_COMPONENT_LAB;
            break;
        case cmsSigGrayData:
            components =  N_COMPONENT_GRAY;
            break;
        case cmsSigRgbData:
            components =  N_COMPONENT_RGB;
            break;
        case cmsSigCmykData:
            components =  N_COMPONENT_CMYK;
            break;
        default:
            components = N_COMPONENT_DEFAULT;
            break;
    }
    return components;
}
void* IccLib_CreateTransform(const unsigned char* pSrcProfileData, FX_DWORD dwSrcProfileSize, int32_t& nSrcComponents,
                             const unsigned char* pDstProfileData, FX_DWORD dwDstProfileSize, int32_t nDstComponents,
                             int intent, FX_DWORD dwSrcFormat = Icc_FORMAT_DEFAULT, FX_DWORD dwDstFormat = Icc_FORMAT_DEFAULT)
{
    cmsHPROFILE srcProfile = NULL;
    cmsHPROFILE dstProfile = NULL;
    cmsHTRANSFORM hTransform = NULL;
    CLcmsCmm* pCmm = NULL;
    nSrcComponents = 0;
    srcProfile = cmsOpenProfileFromMem((void*)pSrcProfileData, dwSrcProfileSize);
    if (srcProfile == NULL) {
        return NULL;
    }
    if(pDstProfileData == NULL && dwDstProfileSize == 0 && nDstComponents == 3) {
        dstProfile = cmsCreate_sRGBProfile();
    } else {
        dstProfile = cmsOpenProfileFromMem((void*)pDstProfileData, dwDstProfileSize);
    }
    if (dstProfile == NULL) {
        cmsCloseProfile(srcProfile);
        return NULL;
    }
    int srcFormat;
    FX_BOOL bLab = FALSE;
    cmsColorSpaceSignature srcCS = cmsGetColorSpace(srcProfile);
    nSrcComponents = GetCSComponents(srcCS);
    if (srcCS == cmsSigLabData) {
        srcFormat = COLORSPACE_SH(PT_Lab) | CHANNELS_SH(nSrcComponents) | BYTES_SH(0);
        bLab = TRUE;
    } else {
        srcFormat = COLORSPACE_SH(PT_ANY) | CHANNELS_SH(nSrcComponents) | BYTES_SH(1);
        if (srcCS == cmsSigRgbData && T_DOSWAP(dwSrcFormat)) {
            srcFormat |= DOSWAP_SH(1);
        }
    }
    cmsColorSpaceSignature dstCS = cmsGetColorSpace(dstProfile);
    if (!CheckComponents(dstCS, nDstComponents, TRUE)) {
        cmsCloseProfile(srcProfile);
        cmsCloseProfile(dstProfile);
        return NULL;
    }
    switch(dstCS) {
        case cmsSigGrayData:
            hTransform = cmsCreateTransform(srcProfile, srcFormat, dstProfile, TYPE_GRAY_8, intent, 0);
            break;
        case cmsSigRgbData:
            hTransform = cmsCreateTransform(srcProfile, srcFormat, dstProfile, TYPE_BGR_8, intent, 0);
            break;
        case cmsSigCmykData:
            hTransform = cmsCreateTransform(srcProfile, srcFormat, dstProfile,
                                            T_DOSWAP(dwDstFormat) ? TYPE_KYMC_8 : TYPE_CMYK_8,
                                            intent, 0);
            break;
        default:
            break;
    }
    if (hTransform == NULL) {
        cmsCloseProfile(srcProfile);
        cmsCloseProfile(dstProfile);
        return NULL;
    }
    pCmm = new CLcmsCmm;
    pCmm->m_nSrcComponents = nSrcComponents;
    pCmm->m_nDstComponents = nDstComponents;
    pCmm->m_hTransform = hTransform;
    pCmm->m_bLab = bLab;
    cmsCloseProfile(srcProfile);
    cmsCloseProfile(dstProfile);
    return pCmm;
}
void* IccLib_CreateTransform_sRGB(const unsigned char* pProfileData, FX_DWORD dwProfileSize, int32_t& nComponents, int32_t intent, FX_DWORD dwSrcFormat)
{
    return IccLib_CreateTransform(pProfileData, dwProfileSize, nComponents, NULL, 0, 3, intent, dwSrcFormat);
}
void IccLib_DestroyTransform(void* pTransform)
{
    if (pTransform == NULL) {
        return;
    }
    cmsDeleteTransform(((CLcmsCmm*)pTransform)->m_hTransform);
    delete (CLcmsCmm*)pTransform;
}
void IccLib_Translate(void* pTransform, FX_DWORD nSrcComponents, FX_FLOAT* pSrcValues, FX_FLOAT* pDestValues)
{
    if (pTransform == NULL) {
        return;
    }
    CLcmsCmm* p = (CLcmsCmm*)pTransform;
    uint8_t output[4];
    if (p->m_bLab) {
        CFX_FixedBufGrow<double, 16> inputs(nSrcComponents);
        double* input = inputs;
        for (FX_DWORD i = 0; i < nSrcComponents; i ++) {
            input[i] = pSrcValues[i];
        }
        cmsDoTransform(p->m_hTransform, input, output, 1);
    } else {
        CFX_FixedBufGrow<uint8_t, 16> inputs(nSrcComponents);
        uint8_t* input = inputs;
        for (FX_DWORD i = 0; i < nSrcComponents; i ++) {
            if (pSrcValues[i] > 1.0f) {
                input[i] = 255;
            } else if (pSrcValues[i] < 0) {
                input[i] = 0;
            } else {
                input[i] = (int)(pSrcValues[i] * 255.0f);
            }
        }
        cmsDoTransform(p->m_hTransform, input, output, 1);
    }
    switch(p->m_nDstComponents) {
        case 1:
            pDestValues[0] = output[0] / 255.0f;
            break;
        case 3:
            pDestValues[0] = output[2] / 255.0f;
            pDestValues[1] = output[1] / 255.0f;
            pDestValues[2] = output[0] / 255.0f;
            break;
        case 4:
            pDestValues[0] = output[0] / 255.0f;
            pDestValues[1] = output[1] / 255.0f;
            pDestValues[2] = output[2] / 255.0f;
            pDestValues[3] = output[3] / 255.0f;
            break;
    }
}
void IccLib_TranslateImage(void* pTransform, unsigned char* pDest, const unsigned char* pSrc, int32_t pixels)
{
    cmsDoTransform(((CLcmsCmm*)pTransform)->m_hTransform, (void*)pSrc, pDest, pixels);
}
void* CreateProfile_Gray(double gamma)
{
    cmsCIExyY* D50 = (cmsCIExyY*)cmsD50_xyY();
    if (!cmsWhitePointFromTemp(D50, 6504)) {
        return NULL;
    }
    cmsToneCurve* curve = cmsBuildGamma(NULL, gamma);
    if (curve == NULL)	{
        return NULL;
    }
    void* profile = cmsCreateGrayProfile(D50, curve);
    cmsFreeToneCurve(curve);
    return profile;
}
ICodec_IccModule::IccCS GetProfileCSFromHandle(void* pProfile)
{
    if (pProfile == NULL)	{
        return ICodec_IccModule::IccCS_Unknown;
    }
    switch (cmsGetColorSpace(pProfile)) {
        case cmsSigXYZData:
            return ICodec_IccModule::IccCS_XYZ;
        case cmsSigLabData:
            return ICodec_IccModule::IccCS_Lab;
        case cmsSigLuvData:
            return ICodec_IccModule::IccCS_Luv;
        case cmsSigYCbCrData:
            return ICodec_IccModule::IccCS_YCbCr;
        case cmsSigYxyData:
            return ICodec_IccModule::IccCS_Yxy;
        case cmsSigRgbData:
            return ICodec_IccModule::IccCS_Rgb;
        case cmsSigGrayData:
            return ICodec_IccModule::IccCS_Gray;
        case cmsSigHsvData:
            return ICodec_IccModule::IccCS_Hsv;
        case cmsSigHlsData:
            return ICodec_IccModule::IccCS_Hls;
        case cmsSigCmykData:
            return ICodec_IccModule::IccCS_Cmyk;
        case cmsSigCmyData:
            return ICodec_IccModule::IccCS_Cmy;
        default:
            return ICodec_IccModule::IccCS_Unknown;
    }
}
ICodec_IccModule::IccCS CCodec_IccModule::GetProfileCS(const uint8_t* pProfileData, FX_DWORD dwProfileSize)
{
    ICodec_IccModule::IccCS cs;
    cmsHPROFILE hProfile = cmsOpenProfileFromMem((void*)pProfileData, dwProfileSize);
    if (hProfile == NULL) {
        return IccCS_Unknown;
    }
    cs = GetProfileCSFromHandle(hProfile);
    if (hProfile)	{
        cmsCloseProfile(hProfile);
    }
    return cs;
}
ICodec_IccModule::IccCS CCodec_IccModule::GetProfileCS(IFX_FileRead* pFile)
{
    if (pFile == NULL) {
        return IccCS_Unknown;
    }
    ICodec_IccModule::IccCS cs;
    FX_DWORD dwSize = (FX_DWORD)pFile->GetSize();
    uint8_t* pBuf = FX_Alloc(uint8_t, dwSize);
    pFile->ReadBlock(pBuf, 0, dwSize);
    cs = GetProfileCS(pBuf, dwSize);
    FX_Free(pBuf);
    return cs;
}
FX_DWORD TransferProfileType(void* pProfile, FX_DWORD dwFormat)
{
    cmsColorSpaceSignature cs = cmsGetColorSpace(pProfile);
    switch (cs) {
        case cmsSigXYZData:
            return TYPE_XYZ_16;
        case cmsSigLabData:
            return TYPE_Lab_DBL;
        case cmsSigLuvData:
            return TYPE_YUV_8;
        case cmsSigYCbCrData:
            return TYPE_YCbCr_8;
        case cmsSigYxyData:
            return TYPE_Yxy_16;
        case cmsSigRgbData:
            return T_DOSWAP(dwFormat) ? TYPE_RGB_8 : TYPE_BGR_8;
        case cmsSigGrayData:
            return TYPE_GRAY_8;
        case cmsSigHsvData:
            return TYPE_HSV_8;
        case cmsSigHlsData:
            return TYPE_HLS_8;
        case cmsSigCmykData:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC_8 : TYPE_CMYK_8;
        case cmsSigCmyData:
            return TYPE_CMY_8;
        case cmsSigMCH5Data:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC5_8 : TYPE_CMYK5_8;
        case cmsSigMCH6Data:
            return TYPE_CMYK6_8;
        case cmsSigMCH7Data:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC7_8 : TYPE_CMYK7_8;
        case cmsSigMCH8Data:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC8_8 : TYPE_CMYK8_8;
        case cmsSigMCH9Data:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC9_8 : TYPE_CMYK9_8;
        case cmsSigMCHAData:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC10_8 : TYPE_CMYK10_8;
        case cmsSigMCHBData:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC11_8 : TYPE_CMYK11_8;
        case cmsSigMCHCData:
            return T_DOSWAP(dwFormat) ? TYPE_KYMC12_8 : TYPE_CMYK12_8;
        default:
            return 0;
    }
}
class CFX_IccProfileCache
{
public:
    CFX_IccProfileCache();
    ~CFX_IccProfileCache();
    void* m_pProfile;
    FX_DWORD	m_dwRate;
protected:
    void	Purge();
};
CFX_IccProfileCache::CFX_IccProfileCache()
{
    m_pProfile = NULL;
    m_dwRate = 1;
}
CFX_IccProfileCache::~CFX_IccProfileCache()
{
    if (m_pProfile) {
        cmsCloseProfile(m_pProfile);
    }
}
void CFX_IccProfileCache::Purge()
{
}
class CFX_IccTransformCache
{
public:
    CFX_IccTransformCache(CLcmsCmm* pCmm = NULL);
    ~CFX_IccTransformCache();
    void*	m_pIccTransform;
    FX_DWORD		m_dwRate;
    CLcmsCmm*		m_pCmm;
protected:
    void			Purge();
};
CFX_IccTransformCache::CFX_IccTransformCache(CLcmsCmm* pCmm)
{
    m_pIccTransform = NULL;
    m_dwRate = 1;
    m_pCmm = pCmm;
}
CFX_IccTransformCache::~CFX_IccTransformCache()
{
    if (m_pIccTransform) {
        cmsDeleteTransform(m_pIccTransform);
    }
    if (m_pCmm) {
        FX_Free(m_pCmm);
    }
}
void CFX_IccTransformCache::Purge()
{
}
class CFX_ByteStringKey : public CFX_BinaryBuf
{
public:
    CFX_ByteStringKey() : CFX_BinaryBuf() {}
    CFX_ByteStringKey&		operator << (FX_DWORD i);
};
CFX_ByteStringKey& CFX_ByteStringKey::operator << (FX_DWORD i)
{
    AppendBlock(&i, sizeof(FX_DWORD));
    return *this;
}
void* CCodec_IccModule::CreateProfile(ICodec_IccModule::IccParam* pIccParam, Icc_CLASS ic, CFX_BinaryBuf* pTransformKey)
{
    CFX_IccProfileCache* pCache = NULL;
    CFX_ByteStringKey key;
    CFX_ByteString text;
    key << pIccParam->ColorSpace << (pIccParam->dwProfileType | ic << 8);
    uint8_t ID[16];
    switch (pIccParam->dwProfileType) {
        case Icc_PARAMTYPE_NONE:
            return NULL;
        case Icc_PARAMTYPE_BUFFER:
            MD5ComputeID(pIccParam->pProfileData, pIccParam->dwProfileSize, ID);
            break;
        case Icc_PARAMTYPE_PARAM:
            FXSYS_memset(ID, 0, 16);
            switch (pIccParam->ColorSpace) {
                case IccCS_Gray:
                    text.Format("%lf", pIccParam->Gamma);
                    break;
                default:
                    ;
            }
            MD5ComputeID(text.GetBuffer(0), text.GetLength(), ID);
            break;
        default:
            ;
    }
    key.AppendBlock(ID, 16);
    CFX_ByteString ProfileKey(key.GetBuffer(), key.GetSize());
    ASSERT(pTransformKey);
    pTransformKey->AppendBlock(ProfileKey.GetBuffer(0), ProfileKey.GetLength());
    if (!m_MapProfile.Lookup(ProfileKey, (void*&)pCache)) {
        pCache = new CFX_IccProfileCache;
        switch (pIccParam->dwProfileType) {
            case Icc_PARAMTYPE_BUFFER:
                pCache->m_pProfile = cmsOpenProfileFromMem(pIccParam->pProfileData, pIccParam->dwProfileSize);
                break;
            case Icc_PARAMTYPE_PARAM:
                switch (pIccParam->ColorSpace) {
                    case IccCS_Rgb:
                        pCache->m_pProfile = cmsCreate_sRGBProfile();
                        break;
                    case IccCS_Gray:
                        pCache->m_pProfile = CreateProfile_Gray(pIccParam->Gamma);
                        break;
                    default:
                        break;
                }
                break;
            default:
                break;
        }
        m_MapProfile.SetAt(ProfileKey, pCache);
    } else {
        pCache->m_dwRate++;
    }
    return pCache->m_pProfile;
}
void* CCodec_IccModule::CreateTransform(ICodec_IccModule::IccParam* pInputParam,
        ICodec_IccModule::IccParam* pOutputParam,
        ICodec_IccModule::IccParam* pProofParam,
        FX_DWORD dwIntent, FX_DWORD dwFlag, FX_DWORD dwPrfIntent, FX_DWORD dwPrfFlag)
{
    CLcmsCmm* pCmm = NULL;
    ASSERT(pInputParam && pOutputParam);
    CFX_ByteStringKey key;
    void* pInputProfile = CreateProfile(pInputParam, Icc_CLASS_INPUT, &key);
    if (pInputProfile == NULL)	{
        return NULL;
    }
    void* pOutputProfile = CreateProfile(pOutputParam, Icc_CLASS_OUTPUT, &key);
    if (pOutputProfile == NULL)	{
        return NULL;
    }
    FX_DWORD dwInputProfileType = TransferProfileType(pInputProfile, pInputParam->dwFormat);
    FX_DWORD dwOutputProfileType = TransferProfileType(pOutputProfile, pOutputParam->dwFormat);
    if (dwInputProfileType == 0 || dwOutputProfileType == 0) {
        return NULL;
    }
    void* pProofProfile = NULL;
    if (pProofParam) {
        pProofProfile = CreateProfile(pProofParam, Icc_CLASS_PROOF, &key);
    }
    key << dwInputProfileType << dwOutputProfileType << dwIntent << dwFlag << (pProofProfile != NULL) << dwPrfIntent << dwPrfFlag;
    CFX_ByteStringC TransformKey(key.GetBuffer(), key.GetSize());
    CFX_IccTransformCache* pTransformCache;
    if (!m_MapTranform.Lookup(TransformKey, (void*&)pTransformCache)) {
        pCmm = FX_Alloc(CLcmsCmm, 1);
        pCmm->m_nSrcComponents = T_CHANNELS(dwInputProfileType);
        pCmm->m_nDstComponents = T_CHANNELS(dwOutputProfileType);
        pCmm->m_bLab = T_COLORSPACE(pInputParam->dwFormat) == PT_Lab;
        pTransformCache = new CFX_IccTransformCache(pCmm);
        if (pProofProfile) {
            pTransformCache->m_pIccTransform = cmsCreateProofingTransform(pInputProfile, dwInputProfileType, pOutputProfile, dwOutputProfileType,
                                               pProofProfile, dwIntent, dwPrfIntent, dwPrfFlag);
        } else {
            pTransformCache->m_pIccTransform = cmsCreateTransform(pInputProfile, dwInputProfileType, pOutputProfile, dwOutputProfileType,
                                               dwIntent, dwFlag);
        }
        pCmm->m_hTransform = pTransformCache->m_pIccTransform;
        m_MapTranform.SetAt(TransformKey, pTransformCache);
    } else {
        pTransformCache->m_dwRate++;
    }
    return pTransformCache->m_pCmm;
}
CCodec_IccModule::~CCodec_IccModule()
{
    FX_POSITION pos = m_MapProfile.GetStartPosition();
    CFX_ByteString key;
    CFX_IccProfileCache* pProfileCache;
    while (pos) {
        m_MapProfile.GetNextAssoc(pos, key, (void*&)pProfileCache);
        delete pProfileCache;
    }
    pos = m_MapTranform.GetStartPosition();
    CFX_IccTransformCache* pTransformCache;
    while (pos) {
        m_MapTranform.GetNextAssoc(pos, key, (void*&)pTransformCache);
        delete pTransformCache;
    }
}
void* CCodec_IccModule::CreateTransform_sRGB(const uint8_t* pProfileData, FX_DWORD dwProfileSize, int32_t& nComponents, int32_t intent, FX_DWORD dwSrcFormat)
{
    return IccLib_CreateTransform_sRGB(pProfileData, dwProfileSize, nComponents, intent, dwSrcFormat);
}
void* CCodec_IccModule::CreateTransform_CMYK(const uint8_t* pSrcProfileData, FX_DWORD dwSrcProfileSize, int32_t& nSrcComponents,
    const uint8_t* pDstProfileData, FX_DWORD dwDstProfileSize, int32_t intent,
        FX_DWORD dwSrcFormat , FX_DWORD dwDstFormat)
{
    return IccLib_CreateTransform(pSrcProfileData, dwSrcProfileSize, nSrcComponents,
                                  pDstProfileData, dwDstProfileSize, 4, intent, dwSrcFormat, dwDstFormat);
}
void CCodec_IccModule::DestroyTransform(void* pTransform)
{
    IccLib_DestroyTransform(pTransform);
}
void CCodec_IccModule::Translate(void* pTransform, FX_FLOAT* pSrcValues, FX_FLOAT* pDestValues)
{
    IccLib_Translate(pTransform, m_nComponents, pSrcValues, pDestValues);
}
void CCodec_IccModule::TranslateScanline(void* pTransform, uint8_t* pDest, const uint8_t* pSrc, int32_t pixels)
{
    IccLib_TranslateImage(pTransform, pDest, pSrc, pixels);
}
const uint8_t g_CMYKSamples[81 * 81 * 3] = {
    255, 255, 255, 225, 226, 228, 199, 200, 202, 173, 174, 178, 147, 149, 152, 123, 125, 128,  99, 99, 102,  69, 70, 71,  34, 30, 31,
    255, 253, 229, 226, 224, 203, 200, 199, 182, 173, 173, 158, 149, 148, 135, 125, 124, 113,  99, 99, 90,  70, 69, 63,  33, 29, 24,
    255, 251, 204, 228, 223, 182, 201, 198, 163, 174, 172, 142, 150, 147, 122, 125, 123, 101,  99, 98, 80,  70, 68, 54,  32, 28, 16,
    255, 249, 179, 230, 222, 160, 203, 197, 144, 174, 170, 124, 150, 145, 105, 125, 122, 88,  99, 97, 69,  70, 68, 46,  31, 28,  6,
    255, 247, 154, 229, 220, 138, 203, 195, 122, 176, 169, 107, 150, 145, 91, 125, 121, 74, 100, 96, 57,  70, 67, 35,  29, 26,  0,
    255, 246, 128, 231, 217, 114, 205, 194, 101, 176, 167, 88, 150, 144, 75, 125, 120, 60, 100, 96, 44,  70, 66, 24,  28, 26,  0,
    255, 244, 96, 231, 217, 87, 203, 192, 78, 175, 167, 66, 150, 143, 56, 125, 119, 43, 100, 95, 29,  69, 66,  7,  26, 26,  0,
    255, 243, 51, 232, 215, 51, 204, 191, 43, 176, 165, 38, 150, 142, 28, 125, 118, 17,  99, 94,  0,  68, 65,  0,  24, 25,  0,
    255, 241,  0, 231, 215,  0, 203, 190,  0, 176, 164,  0, 150, 141,  0, 126, 117,  0,  99, 93,  0,  68, 65,  0,  24, 25,  0,
    252, 228, 238, 222, 201, 211, 197, 180, 190, 171, 156, 166, 147, 133, 143, 123, 111, 119,  99, 88, 94,  71, 61, 66,  34, 22, 26,
    254, 226, 213, 224, 201, 191, 199, 179, 171, 172, 155, 148, 147, 133, 128, 123, 110, 106,  98, 87, 83,  70, 59, 57,  33, 21, 18,
    254, 224, 191, 224, 199, 172, 200, 177, 153, 173, 154, 133, 147, 132, 115, 123, 109, 94,  98, 86, 74,  70, 59, 49,  32, 21,  9,
    255, 222, 168, 227, 198, 150, 200, 175, 135, 173, 153, 118, 148, 130, 99, 123, 109, 82,  98, 86, 64,  69, 58, 40,  31, 19,  0,
    255, 221, 145, 227, 196, 129, 201, 174, 115, 173, 151, 99, 148, 129, 85, 124, 108, 69,  98, 85, 52,  69, 58, 30,  30, 19,  0,
    255, 219, 121, 227, 195, 109, 201, 174, 97, 174, 150, 83, 148, 129, 70, 124, 107, 55,  98, 84, 40,  69, 58, 19,  28, 18,  0,
    255, 218, 92, 229, 194, 82, 202, 173, 75, 174, 150, 63, 149, 128, 51, 124, 106, 39,  98, 84, 24,  68, 57,  3,  26, 18,  0,
    255, 217, 54, 228, 193, 52, 201, 172, 46, 174, 148, 36, 148, 127, 27, 123, 105, 14,  98, 83,  0,  68, 56,  0,  25, 18,  0,
    255, 216,  0, 229, 192,  2, 202, 171,  4, 173, 148,  0, 148, 126,  0, 124, 105,  0,  98, 83,  0,  68, 56,  0,  24, 17,  0,
    249, 204, 223, 219, 181, 199, 195, 160, 178, 170, 140, 156, 146, 119, 134, 123, 99, 112,  98, 77, 88,  70, 52, 61,  34, 11, 20,
    250, 201, 200, 221, 180, 178, 197, 159, 161, 171, 139, 139, 147, 119, 120, 123, 98, 99,  98, 77, 78,  69, 51, 52,  34, 11, 10,
    252, 201, 180, 223, 179, 162, 197, 159, 144, 170, 138, 125, 146, 117, 107, 122, 97, 89,  98, 76, 69,  69, 50, 44,  32, 11,  2,
    252, 199, 158, 222, 177, 143, 199, 158, 127, 171, 137, 110, 147, 117, 93, 122, 96, 76,  97, 75, 58,  69, 50, 36,  32, 10,  0,
    253, 198, 137, 223, 177, 123, 198, 156, 110, 171, 136, 95, 146, 116, 80, 122, 96, 65,  97, 75, 47,  69, 50, 25,  30, 10,  0,
    254, 197, 115, 225, 175, 104, 198, 156, 92, 172, 135, 79, 147, 115, 66, 123, 95, 52,  98, 74, 37,  69, 49, 15,  29, 10,  0,
    254, 196, 89, 224, 175, 80, 199, 154, 70, 172, 134, 59, 146, 114, 48, 122, 95, 36,  97, 74, 21,  68, 49,  0,  27,  9,  0,
    255, 195, 57, 225, 173, 52, 198, 154, 44, 172, 133, 36, 147, 113, 26, 123, 94, 14,  98, 74,  0,  68, 49,  0,  26, 10,  0,
    254, 194, 15, 225, 172, 12, 198, 153,  7, 172, 132,  3, 146, 113,  0, 123, 93,  0,  98, 73,  0,  68, 49,  0,  26,  9,  0,
    246, 178, 209, 218, 159, 186, 194, 140, 166, 168, 122, 145, 144, 104, 125, 121, 85, 103,  97, 65, 81,  69, 41, 55,  34,  0, 12,
    248, 176, 186, 219, 157, 166, 195, 139, 149, 168, 121, 130, 144, 103, 111, 121, 85, 91,  97, 65, 71,  69, 41, 46,  34,  0,  4,
    249, 175, 168, 220, 156, 150, 196, 139, 135, 169, 121, 116, 144, 103, 100, 122, 84, 83,  98, 65, 63,  70, 41, 39,  33,  0,  0,
    249, 175, 148, 220, 155, 133, 196, 138, 119, 169, 120, 103, 145, 101, 87, 121, 83, 71,  97, 65, 54,  69, 41, 31,  32,  0,  0,
    249, 173, 128, 222, 154, 115, 195, 137, 102, 170, 119, 88, 145, 101, 74, 122, 83, 59,  97, 64, 43,  68, 40, 20,  30,  0,  0,
    250, 172, 108, 221, 154, 98, 195, 136, 86, 170, 118, 73, 145, 100, 61, 122, 82, 48,  97, 63, 32,  69, 40, 11,  28,  0,  0,
    250, 171, 85, 221, 153, 76, 196, 136, 67, 170, 117, 56, 145, 99, 44, 121, 82, 33,  97, 63, 17,  68, 40,  0,  28,  0,  0,
    251, 171, 58, 222, 152, 50, 197, 135, 43, 169, 117, 34, 146, 99, 25, 121, 81, 10,  96, 63,  0,  68, 40,  0,  27,  0,  0,
    250, 170, 26, 222, 151, 19, 196, 134, 13, 169, 116,  4, 145, 99,  0, 122, 81,  0,  97, 63,  0,  67, 40,  0,  26,  0,  0,
    244, 153, 194, 215, 136, 173, 192, 121, 155, 167, 104, 135, 143, 89, 115, 121, 72, 96,  97, 54, 75,  70, 31, 49,  34,  0,  6,
    245, 153, 173, 216, 136, 155, 192, 120, 138, 167, 104, 121, 144, 88, 103, 121, 71, 85,  97, 54, 66,  69, 31, 42,  34,  0,  0,
    246, 152, 157, 217, 135, 140, 193, 120, 126, 167, 103, 109, 143, 88, 92, 121, 72, 76,  97, 54, 58,  69, 31, 35,  33,  0,  0,
    245, 150, 139, 218, 134, 125, 193, 119, 111, 167, 103, 96, 144, 87, 80, 121, 71, 66,  96, 53, 49,  68, 31, 26,  32,  0,  0,
    246, 151, 122, 218, 133, 108, 194, 118, 96, 168, 102, 81, 144, 86, 69, 120, 71, 55,  95, 53, 39,  68, 30, 17,  31,  0,  0,
    248, 150, 103, 218, 133, 91, 193, 118, 81, 168, 102, 69, 143, 86, 56, 120, 70, 43,  96, 53, 28,  68, 31,  6,  29,  0,  0,
    247, 149, 81, 218, 132, 72, 194, 117, 62, 168, 101, 52, 144, 86, 42, 121, 70, 29,  96, 52, 13,  68, 30,  0,  28,  0,  0,
    247, 148, 55, 219, 131, 50, 194, 117, 43, 167, 101, 32, 144, 85, 22, 120, 69,  8,  96, 52,  0,  67, 30,  0,  27,  0,  0,
    247, 147, 29, 218, 131, 24, 194, 116, 20, 168, 100, 11, 144, 85,  0, 120, 69,  0,  96, 52,  0,  67, 30,  0,  26,  0,  0,
    242, 130, 179, 214, 114, 160, 190, 101, 143, 166, 87, 125, 143, 72, 107, 120, 58, 88,  96, 42, 68,  69, 17, 44,  35,  0,  0,
    243, 129, 161, 215, 114, 143, 191, 101, 128, 166, 87, 113, 143, 73, 96, 120, 58, 79,  96, 41, 60,  69, 18, 37,  33,  0,  0,
    243, 129, 146, 216, 114, 130, 192, 101, 117, 166, 87, 101, 143, 72, 86, 121, 58, 69,  96, 42, 52,  69, 18, 29,  31,  0,  0,
    243, 128, 130, 216, 114, 115, 191, 101, 102, 165, 86, 88, 142, 72, 75, 120, 58, 60,  95, 42, 43,  68, 19, 21,  30,  0,  0,
    244, 127, 112, 217, 113, 101, 192, 99, 89, 166, 85, 75, 142, 72, 63, 119, 57, 50,  96, 41, 35,  68, 19, 13,  30,  0,  0,
    244, 127, 96, 216, 112, 86, 191, 99, 75, 166, 86, 64, 143, 72, 52, 120, 57, 40,  95, 41, 24,  67, 20,  1,  29,  0,  0,
    245, 126, 77, 216, 113, 68, 191, 100, 59, 166, 85, 49, 142, 71, 38, 119, 57, 26,  95, 41, 10,  67, 20,  0,  28,  0,  0,
    244, 126, 55, 216, 112, 48, 191, 99, 40, 166, 85, 31, 143, 71, 20, 119, 57,  6,  95, 42,  0,  67, 20,  0,  28,  0,  0,
    245, 126, 33, 217, 112, 26, 192, 99, 22, 166, 84, 11, 142, 70,  0, 119, 57,  0,  95, 41,  0,  66, 20,  0,  27,  0,  0,
    241, 102, 167, 213, 90, 149, 189, 79, 133, 165, 66, 115, 141, 54, 98, 119, 41, 81,  96, 25, 63,  69,  0, 38,  30,  0,  0,
    241, 102, 149, 213, 90, 133, 189, 79, 119, 165, 66, 103, 142, 55, 88, 119, 41, 71,  96, 25, 53,  69,  0, 31,  28,  0,  0,
    241, 102, 135, 214, 90, 121, 190, 79, 108, 165, 66, 92, 141, 55, 78, 119, 42, 63,  96, 26, 46,  69,  0, 24,  28,  0,  0,
    241, 101, 120, 214, 90, 107, 189, 79, 95, 165, 67, 83, 141, 54, 68, 118, 41, 54,  95, 27, 39,  68,  0, 16,  27,  0,  0,
    241, 102, 106, 213, 90, 93, 189, 78, 82, 164, 67, 70, 141, 55, 58, 118, 42, 45,  94, 27, 29,  67,  2,  6,  27,  0,  0,
    242, 101, 90, 214, 89, 79, 190, 79, 69, 166, 67, 59, 141, 55, 47, 118, 41, 35,  95, 27, 19,  67,  3,  0,  26,  0,  0,
    242, 102, 72, 213, 89, 63, 191, 79, 56, 164, 67, 45, 141, 55, 34, 118, 42, 22,  94, 28,  6,  67,  3,  0,  26,  0,  0,
    242, 100, 51, 214, 89, 45, 190, 78, 38, 164, 67, 30, 141, 55, 18, 118, 42,  3,  95, 28,  0,  66,  4,  0,  26,  0,  0,
    243, 100, 33, 214, 90, 27, 190, 78, 22, 165, 67, 13, 141, 55,  0, 118, 43,  0,  94, 29,  0,  66,  5,  0,  26,  0,  0,
    237, 69, 153, 211, 58, 135, 187, 51, 121, 163, 41, 105, 141, 28, 90, 118, 15, 73,  96,  0, 56,  68,  0, 33,  25,  0,  0,
    239, 67, 137, 212, 60, 123, 189, 50, 110, 163, 41, 94, 141, 29, 79, 118, 17, 65,  95,  0, 48,  69,  0, 26,  25,  0,  0,
    240, 69, 124, 211, 60, 111, 188, 50, 98, 163, 42, 85, 141, 31, 72, 118, 18, 57,  94,  0, 41,  68,  0, 19,  25,  0,  0,
    240, 70, 112, 212, 61, 99, 188, 52, 87, 163, 41, 74, 140, 31, 62, 118, 20, 48,  94,  2, 32,  68,  0, 11,  24,  0,  0,
    239, 70, 98, 212, 62, 86, 188, 53, 77, 164, 42, 64, 140, 32, 52, 118, 20, 40,  94,  3, 24,  67,  0,  3,  23,  0,  0,
    239, 71, 85, 212, 61, 74, 187, 53, 65, 163, 44, 54, 140, 34, 43, 118, 22, 30,  95,  3, 14,  67,  0,  0,  23,  0,  0,
    239, 70, 67, 212, 62, 59, 188, 53, 51, 163, 45, 42, 141, 34, 31, 117, 22, 17,  94,  5,  2,  66,  0,  0,  23,  0,  0,
    239, 71, 50, 213, 62, 43, 188, 54, 37, 164, 45, 28, 139, 34, 16, 117, 22,  2,  94,  7,  0,  65,  0,  0,  23,  0,  0,
    240, 71, 34, 212, 63, 29, 189, 54, 24, 163, 46, 15, 139, 36,  2, 117, 25,  0,  94,  8,  0,  66,  0,  0,  23,  0,  0,
    237,  0, 140, 209,  0, 124, 186,  0, 112, 162,  0, 97, 141,  0, 82, 118,  0, 67,  95,  0, 49,  68,  0, 27,  20,  0,  0,
    237,  0, 126, 210,  0, 113, 187,  0, 99, 163,  0, 86, 139,  0, 72, 118,  0, 58,  95,  0, 42,  67,  0, 20,  20,  0,  0,
    237,  1, 114, 209,  1, 102, 187,  0, 90, 163,  0, 78, 139,  0, 64, 118,  0, 50,  95,  0, 35,  67,  0, 13,  20,  0,  0,
    236, 16, 102, 209,  7, 91, 186,  0, 80, 162,  0, 68, 139,  0, 56, 117,  0, 43,  94,  0, 27,  67,  0,  6,  20,  0,  0,
    238, 15, 89, 209, 13, 79, 186,  6, 69, 162,  0, 58, 139,  0, 47, 117,  0, 34,  93,  0, 20,  66,  0,  2,  20,  0,  0,
    237, 20, 78, 210, 12, 68, 187,  4, 59, 163,  0, 49, 139,  0, 38, 116,  0, 26,  94,  0, 11,  66,  0,  0,  20,  0,  0,
    237, 25, 64, 210, 18, 56, 186, 11, 48, 162,  4, 39, 138,  0, 27, 117,  0, 14,  93,  0,  0,  66,  0,  0,  20,  0,  0,
    238, 25, 48, 210, 22, 43, 186, 15, 35, 162,  8, 26, 140,  0, 14, 117,  0,  0,  93,  0,  0,  65,  0,  0,  20,  0,  0,
    238, 28, 35, 210, 21, 30, 187, 15, 24, 162,  8, 16, 139,  1,  2, 117,  0,  0,  93,  0,  0,  65,  0,  0,  22,  0,  0,
    219, 242, 252, 195, 214, 225, 172, 191, 201, 148, 165, 175, 127, 142, 150, 106, 119, 126,  84, 95, 101,  58, 66, 72,  24, 27, 32,
    222, 239, 226, 196, 213, 202, 173, 189, 180, 150, 165, 158, 129, 141, 135, 107, 118, 113,  85, 94, 90,  58, 66, 63,  21, 26, 24,
    223, 237, 203, 198, 211, 182, 175, 188, 163, 152, 164, 141, 129, 140, 121, 107, 117, 101,  85, 93, 80,  58, 64, 54,  21, 26, 18,
    226, 236, 179, 201, 210, 160, 177, 187, 143, 153, 162, 125, 130, 139, 106, 108, 116, 89,  85, 92, 69,  58, 64, 45,  20, 25,  8,
    227, 234, 153, 201, 208, 139, 178, 185, 124, 154, 161, 107, 131, 138, 91, 108, 115, 75,  85, 91, 58,  58, 63, 35,  17, 25,  0,
    229, 233, 130, 203, 207, 116, 178, 184, 104, 154, 160, 90, 131, 137, 76, 109, 114, 62,  85, 90, 46,  58, 63, 25,  16, 24,  0,
    230, 231, 100, 202, 205, 90, 179, 183, 80, 154, 159, 69, 131, 136, 57, 109, 113, 46,  86, 90, 32,  58, 63, 10,  14, 24,  0,
    230, 230, 65, 204, 204, 58, 180, 182, 52, 155, 157, 44, 132, 135, 35, 110, 113, 24,  86, 89,  9,  57, 62,  0,  11, 24,  0,
    232, 230, 19, 204, 204, 19, 180, 181, 17, 155, 157, 10, 131, 134,  2, 109, 112,  0,  85, 89,  0,  57, 62,  0,  10, 23,  0,
    218, 216, 236, 194, 192, 211, 172, 171, 188, 149, 149, 164, 128, 127, 141, 106, 106, 119,  84, 84, 94,  59, 57, 66,  25, 18, 26,
    221, 214, 211, 196, 191, 190, 174, 170, 170, 150, 148, 148, 128, 126, 127, 107, 105, 106,  85, 83, 84,  59, 56, 58,  23, 17, 18,
    222, 213, 190, 197, 189, 170, 175, 169, 153, 151, 147, 133, 129, 126, 113, 108, 105, 94,  85, 82, 74,  59, 56, 49,  22, 17, 11,
    224, 211, 168, 199, 188, 151, 175, 168, 135, 152, 146, 117, 129, 124, 99, 107, 103, 82,  84, 82, 64,  59, 55, 41,  21, 17,  1,
    224, 210, 145, 199, 187, 130, 176, 166, 117, 152, 145, 101, 129, 123, 86, 107, 103, 70,  85, 81, 53,  58, 55, 31,  19, 17,  0,
    227, 208, 123, 200, 186, 110, 177, 165, 98, 153, 143, 84, 130, 122, 70, 108, 102, 57,  85, 80, 41,  58, 54, 20,  18, 16,  0,
    227, 208, 97, 202, 185, 86, 177, 164, 77, 153, 142, 65, 130, 122, 54, 108, 101, 42,  85, 80, 27,  58, 54,  7,  16, 16,  0,
    228, 206, 66, 202, 184, 58, 178, 163, 50, 154, 141, 42, 131, 121, 33, 109, 101, 21,  86, 79,  5,  58, 54,  0,  13, 16,  0,
    228, 206, 29, 202, 183, 25, 178, 163, 20, 154, 141, 15, 131, 121,  5, 108, 100,  0,  85, 79,  0,  58, 53,  0,  13, 16,  0,
    217, 193, 221, 193, 172, 198, 172, 153, 178, 149, 133, 154, 128, 114, 132, 107, 94, 111,  85, 74, 89,  59, 49, 61,  25,  8, 22,
    219, 191, 198, 195, 171, 178, 173, 153, 159, 149, 132, 139, 128, 113, 119, 107, 94, 100,  85, 73, 79,  59, 48, 52,  25,  7, 14,
    221, 191, 180, 196, 170, 160, 174, 152, 144, 150, 132, 125, 129, 113, 107, 107, 93, 89,  85, 73, 69,  59, 48, 45,  23,  7,  4,
    222, 189, 159, 197, 169, 142, 174, 151, 127, 151, 131, 110, 129, 112, 94, 108, 93, 78,  85, 72, 60,  58, 47, 37,  22,  7,  0,
    223, 188, 138, 197, 168, 123, 175, 150, 109, 151, 130, 95, 130, 111, 81, 108, 92, 65,  85, 72, 49,  59, 47, 27,  21,  7,  0,
    224, 187, 118, 198, 167, 105, 176, 149, 93, 152, 129, 79, 130, 110, 68, 108, 91, 54,  85, 71, 38,  59, 47, 17,  18,  7,  0,
    224, 187, 93, 199, 166, 83, 176, 148, 73, 152, 128, 62, 129, 109, 51, 108, 90, 39,  85, 71, 25,  58, 46,  3,  16,  8,  0,
    226, 186, 64, 200, 165, 57, 177, 147, 50, 153, 127, 40, 130, 108, 31, 108, 90, 19,  85, 70,  3,  58, 46,  0,  16,  8,  0,
    227, 185, 35, 200, 165, 30, 176, 146, 25, 152, 127, 18, 130, 108,  7, 108, 89,  0,  85, 70,  0,  57, 46,  0,  14,  8,  0,
    216, 169, 205, 192, 150, 184, 171, 134, 164, 149, 116, 144, 128, 99, 124, 107, 81, 103,  85, 63, 81,  60, 39, 55,  26,  0, 15,
    217, 168, 186, 193, 150, 165, 172, 134, 149, 150, 116, 130, 128, 99, 111, 107, 81, 92,  85, 62, 72,  59, 39, 47,  25,  0,  6,
    219, 168, 168, 194, 149, 150, 173, 133, 135, 150, 116, 117, 128, 98, 99, 107, 80, 82,  86, 62, 63,  59, 38, 39,  24,  0,  0,
    219, 166, 148, 195, 149, 133, 173, 133, 119, 150, 115, 103, 128, 98, 88, 107, 80, 72,  85, 61, 54,  59, 38, 32,  23,  0,  0,
    220, 166, 129, 196, 148, 116, 174, 132, 103, 151, 114, 89, 129, 97, 75, 107, 79, 60,  85, 61, 44,  59, 38, 22,  21,  0,  0,
    222, 164, 110, 197, 147, 99, 175, 131, 87, 151, 113, 75, 129, 96, 63, 107, 79, 49,  85, 61, 33,  58, 38, 12,  19,  0,  0,
    222, 164, 88, 197, 146, 79, 174, 130, 69, 151, 113, 58, 129, 95, 47, 107, 78, 35,  85, 60, 20,  58, 38,  0,  18,  0,  0,
    223, 164, 63, 198, 145, 55, 175, 129, 48, 151, 112, 39, 129, 95, 29, 107, 78, 16,  85, 60,  1,  58, 38,  0,  17,  0,  0,
    223, 163, 36, 198, 145, 32, 174, 129, 26, 151, 111, 17, 129, 95,  7, 107, 78,  0,  84, 60,  0,  57, 37,  0,  15,  0,  0,
    215, 147, 192, 191, 130, 172, 170, 116, 153, 148, 100, 133, 127, 85, 115, 107, 69, 96,  85, 51, 75,  60, 28, 50,  25,  0,  8,
    217, 146, 173, 192, 130, 154, 171, 115, 138, 149, 100, 121, 128, 84, 103, 107, 68, 85,  85, 51, 66,  60, 28, 42,  25,  0,  0,
    217, 145, 157, 193, 129, 140, 173, 115, 125, 149, 100, 109, 128, 84, 92, 107, 68, 76,  85, 51, 58,  59, 28, 35,  23,  0,  0,
    218, 145, 140, 193, 129, 125, 172, 114, 110, 149, 99, 96, 128, 83, 81, 107, 67, 65,  84, 51, 49,  59, 29, 27,  22,  0,  0,
    219, 144, 121, 194, 128, 108, 172, 113, 96, 149, 98, 83, 128, 83, 69, 107, 68, 55,  85, 50, 40,  59, 28, 18,  20,  0,  0,
    220, 143, 104, 195, 128, 93, 173, 114, 82, 150, 98, 69, 127, 82, 58, 107, 67, 45,  85, 50, 30,  59, 28,  7,  19,  0,  0,
    220, 143, 84, 195, 127, 74, 173, 113, 65, 149, 97, 55, 128, 82, 44, 106, 67, 32,  84, 50, 16,  58, 28,  0,  18,  0,  0,
    221, 142, 62, 196, 126, 53, 173, 112, 46, 150, 97, 37, 128, 82, 26, 107, 66, 14,  84, 50,  0,  58, 28,  0,  16,  0,  0,
    222, 142, 38, 196, 126, 34, 174, 112, 27, 150, 96, 17, 128, 82,  6, 106, 66,  0,  84, 50,  0,  57, 29,  0,  16,  0,  0,
    214, 123, 179, 191, 110, 159, 169, 98, 143, 147, 84, 124, 126, 70, 106, 107, 55, 88,  85, 39, 69,  60, 15, 45,  23,  0,  2,
    216, 123, 161, 192, 110, 144, 170, 98, 129, 148, 84, 112, 127, 70, 95, 107, 55, 79,  85, 39, 61,  60, 15, 37,  20,  0,  0,
    217, 122, 145, 192, 110, 130, 170, 97, 116, 149, 84, 101, 127, 70, 85, 106, 55, 70,  85, 39, 53,  59, 16, 30,  19,  0,  0,
    217, 123, 131, 192, 109, 116, 171, 96, 103, 149, 83, 89, 127, 70, 75, 106, 55, 60,  85, 40, 45,  59, 16, 23,  17,  0,  0,
    217, 122, 114, 193, 109, 101, 172, 96, 91, 149, 82, 77, 128, 69, 64, 106, 55, 50,  84, 39, 35,  59, 17, 14,  17,  0,  0,
    218, 122, 98, 194, 108, 87, 171, 96, 77, 149, 82, 65, 127, 69, 52, 106, 55, 40,  84, 40, 25,  59, 18,  3,  15,  0,  0,
    219, 122, 80, 193, 108, 70, 172, 95, 61, 149, 82, 51, 127, 69, 40, 106, 55, 28,  84, 39, 12,  58, 17,  0,  13,  0,  0,
    219, 121, 59, 194, 108, 52, 172, 96, 44, 149, 82, 35, 127, 68, 24, 106, 55, 11,  84, 40,  0,  57, 18,  0,  13,  0,  0,
    219, 121, 40, 193, 108, 33, 172, 95, 26, 149, 81, 19, 128, 68,  6, 106, 54,  0,  84, 39,  0,  57, 18,  0,  13,  0,  0,
    213, 99, 165, 189, 87, 148, 169, 76, 132, 147, 64, 115, 126, 52, 98, 106, 39, 81,  85, 23, 63,  60,  0, 39,  16,  0,  0,
    214, 98, 149, 191, 87, 133, 170, 76, 119, 148, 65, 103, 127, 53, 88, 106, 39, 72,  85, 24, 55,  60,  0, 32,  15,  0,  0,
    215, 99, 136, 191, 87, 121, 170, 77, 108, 148, 65, 93, 126, 53, 79, 106, 40, 64,  85, 24, 47,  59,  0, 25,  14,  0,  0,
    215, 99, 121, 192, 87, 108, 170, 77, 96, 148, 65, 82, 126, 53, 69, 106, 40, 55,  85, 25, 39,  59,  0, 18,  13,  0,  0,
    216, 99, 106, 191, 87, 95, 170, 76, 83, 148, 65, 71, 126, 53, 58, 106, 41, 45,  85, 26, 30,  59,  0,  8,  11,  0,  0,
    216, 98, 91, 192, 88, 82, 170, 77, 71, 148, 65, 60, 127, 53, 48, 105, 41, 36,  83, 26, 21,  58,  1,  2,  11,  0,  0,
    217, 99, 75, 192, 87, 66, 170, 76, 57, 148, 65, 47, 126, 53, 36, 105, 41, 24,  83, 26,  8,  57,  2,  0,   9,  0,  0,
    217, 98, 57, 192, 87, 49, 171, 77, 41, 147, 65, 32, 126, 53, 21, 105, 41,  8,  84, 27,  0,  57,  3,  0,   9,  0,  0,
    217, 98, 40, 193, 87, 34, 171, 76, 27, 148, 65, 19, 126, 53,  6, 105, 41,  0,  83, 27,  0,  57,  4,  0,   9,  0,  0,
    211, 67, 152, 189, 58, 136, 168, 50, 122, 147, 39, 105, 127, 28, 89, 106, 14, 74,  85,  0, 56,  59,  0, 33,   9,  0,  0,
    213, 68, 138, 190, 59, 123, 169, 51, 109, 148, 40, 95, 126, 30, 80, 106, 16, 65,  85,  0, 48,  59,  0, 27,   9,  0,  0,
    214, 69, 125, 190, 59, 111, 168, 51, 99, 148, 41, 86, 126, 31, 72, 106, 18, 58,  85,  0, 41,  59,  0, 20,   7,  0,  0,
    215, 70, 112, 190, 61, 100, 169, 52, 88, 147, 42, 76, 126, 32, 63, 106, 19, 49,  84,  1, 34,  58,  0, 13,   7,  0,  0,
    214, 70, 99, 190, 62, 88, 169, 53, 77, 147, 43, 65, 125, 32, 53, 106, 20, 40,  84,  3, 26,  58,  0,  4,   7,  0,  0,
    214, 71, 86, 190, 61, 75, 169, 53, 65, 146, 43, 54, 126, 33, 44, 105, 21, 31,  83,  4, 17,  57,  0,  0,   7,  0,  0,
    215, 71, 71, 191, 62, 62, 169, 53, 53, 147, 44, 44, 126, 34, 33, 105, 22, 20,  83,  5,  4,  57,  0,  0,   7,  0,  0,
    215, 71, 54, 191, 62, 47, 169, 54, 39, 147, 44, 30, 126, 35, 20, 105, 23,  6,  83,  6,  0,  56,  0,  0,   5,  0,  0,
    215, 71, 41, 191, 63, 34, 170, 54, 27, 147, 45, 17, 126, 35,  6, 105, 23,  0,  83,  8,  0,  56,  0,  0,   5,  0,  0,
    210, 13, 140, 189,  1, 125, 167,  0, 110, 146,  0, 96, 126,  0, 81, 106,  0, 67,  85,  0, 51,  59,  0, 28,   4,  0,  0,
    212, 18, 126, 190,  7, 113, 168,  0, 100, 146,  0, 86, 126,  0, 73, 106,  0, 59,  84,  0, 43,  59,  0, 22,   4,  0,  0,
    212, 21, 115, 190, 13, 103, 168,  3, 91, 146,  0, 78, 125,  0, 65, 105,  0, 52,  84,  0, 36,  58,  0, 16,   4,  0,  0,
    213, 24, 103, 189, 19, 91, 168,  9, 82, 146,  0, 69, 125,  0, 57, 105,  0, 44,  84,  0, 29,  58,  0,  7,   4,  0,  0,
    213, 27, 92, 188, 21, 81, 168, 14, 71, 146,  1, 59, 125,  0, 48, 105,  0, 36,  84,  0, 21,  58,  0,  4,   4,  0,  0,
    213, 30, 80, 189, 22, 69, 168, 17, 61, 146,  5, 50, 125,  0, 39, 104,  0, 27,  83,  0, 12,  57,  0,  0,   4,  0,  0,
    214, 30, 67, 189, 25, 57, 168, 20, 50, 146,  9, 40, 125,  0, 29, 104,  0, 17,  83,  0,  2,  56,  0,  0,   4,  0,  0,
    214, 32, 53, 189, 27, 44, 169, 20, 38, 146, 13, 28, 124,  2, 17, 104,  0,  4,  83,  0,  0,  56,  0,  0,   4,  0,  0,
    214, 33, 41, 190, 27, 33, 168, 23, 27, 146, 13, 18, 125,  3,  5, 105,  0,  0,  83,  0,  0,  56,  0,  0,   4,  0,  0,
    185, 229, 250, 164, 204, 223, 146, 182, 199, 127, 158, 174, 108, 136, 149,  89, 113, 125,  70, 90, 100,  46, 62, 71,  10, 25, 33,
    189, 227, 225, 168, 202, 201, 148, 181, 179, 129, 157, 156, 109, 135, 134,  90, 113, 113,  70, 89, 90,  46, 62, 62,   8, 24, 25,
    192, 226, 202, 170, 202, 182, 151, 179, 162, 130, 156, 141, 110, 133, 121,  91, 112, 101,  71, 89, 80,  46, 61, 54,   7, 24, 19,
    194, 224, 179, 173, 200, 160, 153, 178, 144, 132, 155, 125, 112, 133, 107,  92, 111, 89,  71, 88, 69,  46, 61, 45,   6, 23, 10,
    196, 223, 155, 174, 198, 139, 154, 176, 124, 132, 153, 107, 113, 131, 91,  92, 110, 75,  72, 87, 58,  47, 60, 37,   4, 23,  0,
    198, 221, 131, 175, 197, 117, 155, 175, 105, 133, 152, 91, 113, 130, 76,  92, 109, 63,  72, 86, 47,  46, 60, 26,   3, 23,  0,
    200, 220, 104, 176, 196, 94, 156, 175, 84, 134, 151, 72, 113, 129, 59,  93, 108, 47,  72, 85, 33,  46, 59, 13,   0, 23,  0,
    201, 219, 73, 179, 195, 65, 157, 173, 57, 135, 150, 48, 114, 129, 39,  94, 108, 28,  72, 85, 15,  47, 59,  0,   0, 22,  0,
    203, 219, 42, 178, 195, 37, 157, 173, 32, 135, 150, 26, 114, 128, 16,  94, 107,  6,  73, 85,  0,  46, 58,  0,   0, 22,  0,
    186, 205, 233, 165, 183, 209, 148, 163, 187, 128, 142, 163, 109, 121, 140,  91, 101, 118,  71, 80, 94,  48, 54, 66,  12, 15, 27,
    189, 204, 211, 169, 182, 189, 151, 163, 169, 131, 141, 147, 111, 121, 126,  92, 101, 105,  72, 79, 84,  48, 54, 58,  11, 15, 19,
    192, 202, 190, 171, 181, 170, 152, 161, 152, 131, 141, 133, 112, 120, 113,  93, 100, 94,  72, 79, 74,  48, 53, 50,  10, 15, 11,
    195, 201, 169, 172, 179, 151, 153, 160, 135, 132, 139, 117, 113, 119, 100,  93, 99, 82,  72, 78, 64,  48, 53, 41,   9, 14,  3,
    195, 200, 146, 174, 179, 131, 154, 159, 117, 133, 138, 101, 113, 118, 86,  93, 98, 70,  73, 77, 53,  48, 52, 32,   8, 15,  0,
    198, 199, 125, 175, 177, 111, 155, 158, 100, 133, 137, 85, 113, 117, 71,  93, 97, 57,  72, 77, 42,  47, 52, 22,   5, 14,  0,
    199, 198, 101, 176, 177, 89, 155, 157, 79, 134, 136, 68, 113, 116, 56,  94, 97, 44,  73, 76, 30,  47, 52, 10,   2, 15,  0,
    200, 197, 72, 178, 176, 63, 157, 156, 56, 135, 136, 46, 114, 116, 37,  94, 96, 26,  73, 76, 11,  47, 51,  0,   0, 14,  0,
    201, 197, 45, 177, 175, 38, 156, 155, 31, 135, 135, 25, 114, 115, 17,  94, 96,  5,  73, 75,  0,  46, 51,  0,   0, 14,  0,
    187, 183, 218, 167, 165, 197, 149, 147, 176, 129, 127, 153, 111, 109, 132,  92, 90, 111,  73, 70, 89,  49, 46, 62,  15,  4, 22,
    190, 183, 197, 170, 164, 177, 151, 146, 159, 130, 127, 139, 112, 109, 119,  93, 90, 99,  72, 70, 78,  49, 45, 53,  14,  4, 15,
    192, 182, 179, 171, 163, 161, 153, 145, 144, 132, 126, 125, 113, 108, 107,  93, 89, 88,  73, 70, 69,  49, 45, 45,  13,  5,  6,
    195, 181, 159, 172, 162, 142, 152, 145, 127, 132, 125, 111, 113, 107, 94,  93, 88, 77,  73, 69, 59,  48, 45, 37,  11,  5,  0,
    195, 180, 139, 173, 161, 124, 153, 143, 110, 133, 125, 96, 113, 106, 81,  94, 88, 66,  73, 68, 49,  49, 44, 28,   9,  6,  0,
    196, 179, 118, 174, 160, 106, 154, 142, 94, 133, 124, 81, 113, 105, 68,  94, 87, 54,  73, 68, 39,  48, 44, 18,   5,  5,  0,
    197, 178, 96, 176, 159, 86, 155, 141, 75, 134, 123, 64, 114, 105, 53,  94, 87, 40,  73, 68, 26,  48, 44,  5,   2,  6,  0,
    199, 178, 70, 176, 158, 62, 156, 141, 54, 134, 122, 44, 114, 104, 35,  94, 86, 23,  73, 67,  8,  47, 44,  0,   2,  6,  0,
    199, 177, 45, 178, 158, 40, 156, 140, 32, 135, 122, 26, 114, 104, 16,  94, 86,  4,  73, 67,  0,  47, 44,  0,   0,  7,  0,
    188, 161, 204, 168, 144, 183, 149, 129, 164, 130, 112, 144, 112, 95, 123,  93, 78, 103,  74, 60, 81,  50, 36, 56,  16,  0, 16,
    190, 160, 185, 170, 144, 165, 151, 128, 148, 132, 111, 130, 112, 95, 110,  93, 78, 92,  74, 59, 72,  50, 36, 48,  16,  0,  8,
    192, 160, 167, 171, 143, 150, 153, 128, 134, 132, 111, 117, 112, 94, 100,  94, 77, 82,  74, 59, 63,  50, 36, 40,  14,  0,  0,
    193, 159, 149, 172, 143, 134, 153, 127, 119, 133, 110, 103, 113, 94, 87,  93, 77, 72,  73, 59, 54,  50, 36, 32,  12,  0,  0,
    195, 159, 131, 173, 142, 117, 153, 127, 104, 132, 110, 90, 113, 93, 76,  93, 76, 61,  74, 59, 45,  49, 36, 23,   9,  0,  0,
    196, 158, 113, 174, 141, 101, 155, 126, 89, 133, 109, 76, 113, 93, 64,  94, 76, 51,  74, 58, 35,  49, 36, 14,   6,  0,  0,
    197, 157, 92, 174, 141, 80, 154, 125, 71, 134, 108, 60, 114, 92, 50,  94, 75, 37,  73, 58, 22,  48, 36,  1,   5,  0,  0,
    197, 157, 68, 175, 140, 59, 155, 124, 51, 134, 108, 41, 113, 91, 32,  94, 75, 21,  73, 57,  5,  48, 35,  0,   5,  0,  0,
    198, 156, 46, 176, 140, 40, 155, 124, 32, 134, 107, 24, 114, 91, 14,  94, 75,  2,  73, 57,  0,  48, 36,  0,   3,  0,  0,
    189, 140, 191, 168, 126, 172, 150, 112, 154, 131, 97, 134, 112, 82, 115,  94, 66, 96,  74, 49, 75,  51, 25, 50,  12,  0, 10,
    191, 139, 173, 170, 125, 154, 152, 111, 138, 132, 96, 121, 113, 81, 103,  94, 66, 85,  74, 48, 66,  50, 26, 42,  12,  0,  1,
    192, 139, 157, 171, 125, 140, 152, 111, 125, 132, 96, 109, 113, 81, 92,  94, 65, 76,  74, 48, 58,  50, 26, 35,   9,  0,  0,
    193, 139, 140, 172, 124, 125, 153, 110, 112, 133, 95, 96, 113, 80, 82,  94, 65, 66,  74, 49, 50,  50, 26, 28,   7,  0,  0,
    194, 138, 123, 172, 123, 109, 153, 110, 97, 133, 95, 84, 113, 80, 70,  94, 65, 56,  74, 48, 40,  50, 26, 20,   6,  0,  0,
    194, 138, 105, 173, 123, 94, 153, 109, 83, 133, 94, 70, 112, 79, 59,  94, 64, 46,  74, 48, 31,  50, 26,  9,   4,  0,  0,
    196, 138, 87, 174, 122, 77, 153, 109, 67, 133, 93, 56, 113, 79, 46,  94, 64, 34,  73, 48, 18,  49, 27,  0,   4,  0,  0,
    196, 137, 65, 174, 122, 57, 154, 108, 49, 133, 93, 39, 113, 79, 29,  94, 64, 18,  74, 48,  3,  49, 27,  0,   2,  0,  0,
    197, 137, 47, 175, 122, 40, 155, 108, 32, 133, 93, 23, 114, 79, 14,  94, 64,  1,  73, 48,  0,  48, 27,  0,   2,  0,  0,
    189, 119, 177, 168, 106, 159, 150, 94, 142, 131, 81, 124, 113, 67, 107,  94, 53, 89,  74, 37, 69,  51, 11, 45,   6,  0,  3,
    191, 119, 161, 170, 106, 144, 152, 94, 129, 132, 81, 112, 113, 67, 96,  94, 53, 79,  74, 37, 61,  51, 13, 38,   6,  0,  0,
    192, 119, 146, 170, 106, 131, 152, 94, 117, 132, 80, 101, 112, 67, 85,  94, 53, 70,  74, 37, 53,  50, 14, 31,   4,  0,  0,
    192, 119, 131, 171, 106, 117, 153, 94, 105, 132, 80, 89, 113, 67, 75,  94, 54, 61,  74, 38, 45,  51, 14, 23,   3,  0,  0,
    193, 118, 114, 171, 106, 102, 153, 93, 90, 132, 80, 78, 113, 67, 65,  94, 53, 52,  74, 37, 36,  50, 15, 16,   1,  0,  0,
    194, 118, 99, 172, 105, 89, 153, 93, 78, 132, 80, 66, 113, 67, 54,  94, 53, 42,  74, 38, 27,  50, 16,  5,   1,  0,  0,
    194, 118, 82, 173, 105, 72, 153, 93, 63, 132, 79, 53, 113, 67, 42,  94, 53, 30,  74, 38, 15,  49, 16,  0,   0,  0,  0,
    195, 117, 63, 173, 105, 55, 154, 93, 47, 133, 79, 37, 113, 66, 27,  94, 53, 15,  73, 38,  0,  48, 16,  0,   0,  0,  0,
    195, 117, 46, 173, 104, 39, 154, 92, 32, 133, 79, 22, 113, 66, 13,  94, 53,  0,  73, 38,  0,  48, 17,  0,   0,  0,  0,
    189, 96, 166, 168, 85, 147, 150, 74, 132, 131, 62, 115, 113, 51, 99,  94, 38, 82,  74, 21, 63,  51,  0, 40,   1,  0,  0,
    190, 96, 150, 170, 85, 133, 152, 75, 119, 132, 63, 104, 113, 51, 88,  94, 38, 72,  75, 22, 55,  51,  0, 33,   1,  0,  0,
    192, 96, 137, 170, 85, 121, 152, 74, 108, 132, 64, 94, 113, 52, 79,  94, 39, 64,  74, 23, 48,  50,  0, 26,   0,  0,  0,
    192, 96, 122, 171, 86, 109, 152, 75, 96, 132, 63, 83, 113, 52, 69,  94, 39, 56,  74, 24, 41,  50,  0, 19,   0,  0,  0,
    193, 96, 107, 171, 85, 96, 152, 75, 84, 132, 64, 72, 113, 52, 60,  94, 39, 47,  74, 24, 32,  50,  1, 10,   0,  0,  0,
    193, 96, 93, 172, 85, 82, 152, 75, 72, 133, 63, 61, 113, 51, 49,  94, 39, 37,  73, 25, 23,  49,  2,  2,   0,  0,  0,
    194, 96, 78, 172, 85, 68, 152, 75, 59, 132, 63, 49, 113, 52, 39,  94, 40, 26,  73, 25, 11,  48,  3,  0,   0,  0,  0,
    194, 96, 60, 173, 85, 52, 153, 75, 44, 132, 64, 35, 112, 52, 25,  94, 40, 12,  73, 26,  0,  48,  4,  0,   0,  0,  0,
    195, 96, 46, 173, 85, 38, 154, 74, 31, 133, 63, 22, 113, 52, 11,  93, 40,  0,  73, 26,  0,  47,  5,  0,   0,  0,  0,
    188, 67, 153, 168, 58, 137, 151, 49, 122, 131, 39, 106, 113, 28, 90,  94, 13, 75,  75,  0, 57,  51,  0, 35,   0,  0,  0,
    190, 68, 138, 170, 59, 123, 152, 50, 110, 132, 41, 96, 113, 29, 80,  94, 16, 66,  75,  0, 49,  50,  0, 27,   0,  0,  0,
    191, 69, 126, 170, 59, 112, 151, 52, 100, 132, 42, 86, 113, 30, 73,  95, 17, 58,  75,  0, 42,  50,  0, 21,   0,  0,  0,
    192, 70, 113, 170, 61, 100, 151, 52, 89, 132, 42, 77, 113, 31, 64,  94, 19, 50,  74,  1, 35,  50,  0, 14,   0,  0,  0,
    192, 70, 100, 170, 62, 89, 151, 53, 77, 131, 43, 66, 112, 32, 54,  94, 20, 42,  74,  2, 27,  49,  0,  5,   0,  0,  0,
    192, 71, 87, 171, 61, 77, 152, 53, 67, 131, 44, 57, 112, 33, 45,  94, 21, 33,  74,  4, 19,  49,  0,  1,   0,  0,  0,
    193, 71, 74, 171, 62, 64, 152, 53, 55, 132, 44, 45, 113, 34, 34,  94, 22, 23,  73,  5,  7,  48,  0,  0,   0,  0,  0,
    193, 70, 58, 172, 62, 50, 152, 54, 42, 132, 44, 32, 112, 35, 22,  93, 23, 10,  73,  6,  0,  47,  0,  0,   0,  0,  0,
    193, 70, 45, 172, 62, 38, 153, 54, 31, 132, 44, 21, 112, 35,  9,  94, 23,  0,  73,  7,  0,  47,  0,  0,   0,  0,  0,
    189, 26, 141, 169, 15, 126, 150,  2, 112, 131,  0, 97, 113,  0, 82,  94,  0, 67,  75,  0, 51,  50,  0, 29,   0,  0,  0,
    190, 28, 128, 170, 18, 114, 151,  8, 101, 132,  0, 88, 113,  0, 74,  94,  0, 60,  75,  0, 44,  50,  0, 23,   0,  0,  0,
    191, 30, 117, 170, 23, 104, 152, 11, 92, 132,  1, 79, 113,  0, 67,  95,  0, 53,  75,  0, 37,  50,  0, 17,   0,  0,  0,
    191, 33, 105, 170, 26, 93, 151, 18, 83, 132,  6, 70, 112,  0, 58,  94,  0, 45,  75,  0, 30,  49,  0,  8,   0,  0,  0,
    191, 34, 93, 170, 27, 82, 151, 20, 72, 131,  8, 61, 112,  0, 49,  94,  0, 38,  74,  0, 23,  49,  0,  4,   0,  0,  0,
    191, 36, 82, 170, 29, 71, 151, 22, 63, 131, 11, 52, 112,  0, 41,  93,  0, 29,  74,  0, 14,  48,  0,  1,   0,  0,  0,
    191, 38, 69, 170, 31, 60, 151, 24, 51, 131, 14, 41, 112,  1, 31,  93,  0, 19,  73,  0,  3,  48,  0,  0,   0,  0,  0,
    192, 37, 56, 171, 31, 47, 152, 25, 40, 131, 17, 30, 112,  4, 19,  93,  0,  7,  73,  0,  0,  47,  0,  0,   0,  0,  0,
    192, 38, 45, 171, 33, 36, 152, 26, 30, 131, 18, 21, 111,  7,  9,  93,  0,  0,  73,  0,  0,  47,  0,  0,   0,  0,  0,
    149, 218, 248, 133, 194, 222, 119, 173, 198, 102, 151, 173,  86, 130, 148,  70, 108, 125,  53, 85, 100,  32, 59, 71,   0, 22, 33,
    154, 216, 223, 137, 193, 200, 122, 172, 178, 106, 150, 156,  89, 128, 133,  73, 107, 112,  54, 85, 89,  31, 59, 63,   0, 22, 26,
    159, 215, 202, 141, 192, 181, 126, 171, 161, 108, 149, 141,  90, 128, 121,  74, 107, 100,  55, 85, 80,  32, 58, 55,   0, 22, 19,
    161, 213, 179, 144, 190, 160, 126, 170, 143, 109, 148, 125,  92, 127, 107,  74, 106, 89,  56, 84, 69,  32, 58, 46,   0, 21, 11,
    163, 211, 156, 144, 189, 139, 129, 168, 125, 110, 147, 108,  93, 126, 92,  75, 105, 76,  57, 83, 58,  33, 58, 37,   0, 21,  1,
    167, 211, 133, 147, 188, 120, 130, 167, 105, 110, 145, 92,  93, 125, 78,  76, 104, 64,  58, 83, 48,  33, 57, 27,   0, 21,  0,
    169, 210, 108, 149, 187, 96, 131, 166, 86, 112, 144, 74,  94, 124, 62,  77, 103, 49,  58, 82, 35,  33, 57, 15,   0, 21,  0,
    170, 209, 80, 151, 186, 71, 133, 165, 62, 114, 143, 52,  95, 123, 42,  77, 103, 32,  58, 81, 18,  33, 56,  0,   0, 21,  0,
    173, 208, 55, 152, 186, 49, 134, 165, 41, 114, 143, 34,  95, 122, 25,  77, 102, 14,  58, 81,  0,  33, 56,  0,   0, 21,  0,
    154, 195, 232, 137, 174, 207, 122, 156, 185, 105, 136, 163,  89, 116, 140,  73, 97, 117,  56, 76, 94,  35, 51, 66,   0, 13, 28,
    158, 194, 209, 141, 174, 187, 125, 155, 167, 109, 135, 146,  91, 116, 125,  75, 96, 105,  57, 75, 83,  35, 50, 57,   0, 12, 21,
    161, 193, 189, 144, 173, 169, 128, 154, 151, 110, 134, 132,  93, 115, 113,  77, 95, 94,  58, 75, 74,  35, 50, 50,   0, 12, 13,
    164, 192, 168, 145, 171, 151, 129, 153, 134, 111, 133, 117,  94, 114, 100,  76, 95, 82,  58, 75, 64,  36, 50, 42,   0, 12,  5,
    165, 191, 147, 147, 170, 131, 130, 152, 117, 113, 132, 102,  95, 113, 86,  77, 94, 71,  58, 74, 54,  35, 50, 33,   0, 13,  0,
    167, 189, 126, 148, 169, 113, 132, 151, 100, 113, 131, 86,  96, 112, 73,  77, 93, 59,  59, 73, 43,  35, 49, 23,   0, 12,  0,
    170, 189, 104, 150, 168, 91, 133, 150, 81, 114, 130, 69,  96, 111, 57,  78, 92, 46,  59, 73, 31,  35, 49, 11,   0, 13,  0,
    171, 188, 78, 152, 168, 68, 134, 149, 60, 115, 130, 50,  96, 111, 41,  78, 92, 29,  60, 73, 15,  35, 49,  0,   0, 12,  0,
    173, 187, 55, 153, 167, 47, 134, 149, 39, 115, 129, 33,  97, 110, 24,  79, 92, 13,  60, 72,  0,  35, 48,  0,   0, 12,  0,
    157, 175, 217, 139, 157, 196, 125, 141, 175, 109, 122, 153,  92, 104, 132,  76, 86, 110,  59, 67, 88,  37, 43, 61,   1,  1, 23,
    161, 174, 196, 144, 156, 176, 127, 140, 158, 110, 121, 137,  94, 104, 118,  77, 85, 98,  59, 67, 78,  37, 43, 53,   0,  2, 16,
    163, 174, 178, 146, 156, 160, 130, 139, 143, 112, 121, 124,  95, 103, 106,  78, 85, 88,  60, 66, 69,  37, 42, 46,   0,  2,  7,
    166, 173, 159, 147, 154, 142, 130, 138, 127, 113, 120, 111,  96, 103, 95,  78, 84, 77,  60, 66, 59,  37, 43, 37,   0,  2,  0,
    166, 172, 139, 148, 154, 125, 131, 137, 112, 113, 120, 96,  96, 102, 81,  78, 84, 66,  60, 65, 50,  37, 42, 29,   0,  3,  0,
    167, 171, 120, 149, 153, 107, 133, 137, 95, 114, 118, 81,  97, 101, 69,  79, 84, 56,  60, 65, 40,  37, 42, 19,   0,  3,  0,
    170, 170, 99, 151, 152, 87, 134, 136, 77, 115, 118, 66,  97, 101, 55,  79, 83, 42,  61, 65, 28,  37, 42,  7,   0,  3,  0,
    172, 170, 75, 152, 151, 65, 134, 135, 57, 115, 117, 48,  97, 100, 38,  79, 83, 27,  61, 64, 12,  36, 42,  0,   0,  3,  0,
    172, 169, 55, 154, 151, 46, 135, 134, 40, 116, 116, 32,  97, 99, 21,  80, 82, 10,  61, 64,  0,  36, 41,  0,   0,  3,  0,
    160, 154, 203, 143, 139, 182, 127, 124, 164, 111, 107, 143,  95, 91, 122,  78, 75, 103,  60, 57, 81,  39, 33, 56,   1,  0, 18,
    163, 154, 184, 146, 138, 165, 130, 123, 148, 113, 107, 129,  96, 90, 110,  79, 74, 92,  61, 56, 72,  39, 34, 48,   2,  0,  9,
    165, 154, 167, 147, 137, 149, 131, 122, 134, 114, 106, 117,  96, 90, 100,  79, 74, 82,  61, 56, 64,  39, 33, 40,   2,  0,  1,
    166, 153, 150, 149, 137, 133, 132, 122, 119, 114, 106, 104,  97, 90, 88,  79, 74, 72,  61, 56, 55,  39, 34, 33,   0,  0,  0,
    168, 152, 132, 149, 136, 117, 132, 121, 104, 114, 105, 90,  97, 89, 76,  79, 73, 62,  61, 56, 46,  38, 34, 25,   0,  0,  0,
    169, 151, 114, 150, 135, 101, 133, 121, 90, 114, 104, 77,  97, 89, 65,  80, 73, 51,  61, 56, 36,  38, 34, 16,   0,  0,  0,
    170, 150, 94, 151, 135, 83, 134, 120, 73, 115, 104, 62,  98, 88, 51,  80, 72, 39,  61, 56, 24,  38, 34,  3,   0,  0,  0,
    172, 150, 72, 153, 134, 63, 135, 119, 55, 115, 103, 45,  98, 88, 36,  80, 72, 24,  61, 55,  9,  38, 34,  0,   0,  0,  0,
    172, 150, 54, 153, 134, 47, 135, 119, 38, 116, 103, 30,  98, 87, 21,  80, 72,  8,  62, 55,  0,  37, 34,  0,   0,  0,  0,
    162, 134, 190, 145, 120, 171, 129, 108, 153, 113, 93, 134,  97, 78, 115,  80, 63, 96,  62, 46, 75,  41, 23, 51,   0,  0, 11,
    165, 134, 173, 147, 120, 154, 131, 107, 138, 114, 92, 120,  97, 78, 103,  80, 63, 85,  62, 46, 66,  40, 23, 43,   0,  0,  2,
    166, 134, 157, 148, 120, 140, 132, 106, 125, 114, 92, 109,  97, 77, 93,  81, 63, 77,  62, 46, 58,  40, 24, 36,   0,  0,  0,
    168, 133, 140, 149, 119, 125, 132, 106, 112, 115, 92, 97,  98, 77, 82,  81, 62, 67,  62, 46, 50,  40, 24, 29,   0,  0,  0,
    168, 133, 123, 150, 119, 110, 133, 106, 97, 115, 91, 84,  98, 77, 70,  81, 62, 57,  62, 46, 41,  40, 24, 20,   0,  0,  0,
    169, 132, 107, 150, 118, 94, 133, 105, 84, 115, 91, 72,  98, 76, 60,  80, 62, 47,  62, 46, 32,  39, 25, 11,   0,  0,  0,
    171, 132, 89, 152, 118, 79, 135, 105, 69, 115, 90, 58,  98, 76, 47,  80, 62, 36,  62, 46, 21,  39, 25,  0,   0,  0,  0,
    171, 132, 69, 153, 117, 60, 135, 104, 52, 116, 90, 42,  98, 76, 33,  81, 61, 21,  62, 46,  6,  38, 25,  0,   0,  0,  0,
    172, 132, 54, 153, 118, 45, 135, 104, 38, 116, 90, 28,  98, 76, 18,  81, 61,  6,  62, 46,  0,  38, 25,  0,   0,  0,  0,
    164, 115, 177, 146, 103, 159, 130, 91, 143, 114, 78, 125,  97, 65, 107,  81, 51, 89,  63, 34, 69,  41,  9, 46,   0,  0,  4,
    166, 115, 161, 148, 103, 144, 132, 91, 129, 115, 78, 112,  98, 65, 96,  81, 51, 79,  63, 35, 61,  41, 11, 38,   0,  0,  0,
    167, 115, 146, 150, 102, 131, 132, 91, 117, 115, 78, 101,  98, 65, 86,  81, 51, 71,  63, 35, 54,  41, 12, 32,   0,  0,  0,
    168, 114, 132, 150, 103, 118, 133, 91, 105, 116, 78, 91,  98, 64, 76,  82, 51, 61,  63, 36, 46,  41, 13, 24,   0,  0,  0,
    169, 114, 116, 150, 102, 103, 134, 90, 91, 116, 78, 79,  98, 65, 66,  81, 51, 53,  63, 36, 37,  40, 14, 17,   0,  0,  0,
    169, 114, 101, 151, 101, 89, 134, 90, 79, 116, 77, 67,  98, 64, 56,  81, 51, 44,  63, 36, 29,  40, 15,  7,   0,  0,  0,
    170, 114, 85, 152, 101, 75, 135, 90, 65, 116, 77, 54,  98, 64, 44,  81, 51, 32,  63, 36, 17,  39, 15,  0,   0,  0,  0,
    172, 113, 66, 152, 101, 58, 135, 89, 49, 116, 77, 40,  99, 64, 30,  81, 51, 18,  62, 36,  3,  38, 16,  0,   0,  0,  0,
    171, 113, 51, 153, 101, 44, 136, 89, 36, 116, 77, 28,  99, 64, 18,  81, 51,  5,  62, 36,  0,  38, 16,  0,   0,  0,  0,
    165, 94, 166, 147, 82, 147, 132, 72, 132, 115, 61, 115,  98, 49, 99,  82, 36, 82,  64, 19, 64,  42,  0, 41,   0,  0,  0,
    167, 93, 150, 150, 83, 134, 133, 73, 120, 116, 62, 104,  99, 49, 88,  82, 36, 72,  64, 20, 55,  41,  0, 33,   0,  0,  0,
    169, 93, 137, 150, 83, 122, 134, 73, 109, 116, 61, 94,  99, 50, 80,  82, 37, 65,  64, 21, 49,  41,  0, 27,   0,  0,  0,
    169, 94, 123, 150, 83, 110, 133, 73, 97, 116, 61, 83,  99, 50, 70,  82, 38, 57,  63, 23, 42,  41,  0, 20,   0,  0,  0,
    169, 94, 109, 150, 84, 97, 134, 73, 85, 116, 62, 73,  99, 51, 61,  81, 38, 48,  63, 23, 33,  41,  1, 11,   0,  0,  0,
    170, 94, 96, 150, 83, 84, 134, 73, 74, 116, 61, 62,  99, 50, 51,  82, 38, 39,  64, 23, 24,  40,  3,  4,   0,  0,  0,
    171, 93, 79, 152, 82, 70, 135, 73, 61, 116, 62, 51,  98, 51, 40,  81, 38, 28,  63, 24, 14,  39,  4,  0,   0,  0,  0,
    171, 94, 64, 152, 83, 55, 135, 73, 47, 116, 62, 37,  98, 50, 27,  81, 38, 15,  63, 24,  1,  39,  4,  0,   0,  0,  0,
    172, 93, 51, 153, 82, 42, 135, 73, 35, 117, 62, 26,  99, 51, 16,  81, 39,  3,  63, 25,  0,  38,  5,  0,   0,  0,  0,
    166, 68, 153, 148, 59, 137, 133, 49, 121, 115, 39, 106,  99, 28, 91,  82, 13, 75,  65,  0, 58,  42,  0, 36,   0,  0,  0,
    168, 68, 139, 150, 59, 124, 134, 50, 110, 116, 40, 96,  99, 30, 81,  82, 16, 66,  64,  0, 50,  41,  0, 29,   0,  0,  0,
    169, 69, 126, 150, 59, 113, 134, 51, 101, 117, 42, 87, 100, 30, 73,  82, 17, 59,  65,  0, 43,  41,  0, 23,   0,  0,  0,
    169, 70, 115, 150, 61, 102, 134, 52, 89, 116, 42, 77,  99, 32, 65,  82, 19, 52,  64,  0, 36,  41,  0, 15,   0,  0,  0,
    169, 70, 101, 150, 61, 90, 134, 52, 79, 116, 43, 68,  99, 32, 55,  82, 21, 43,  64,  2, 28,  41,  0,  6,   0,  0,  0,
    170, 70, 89, 151, 62, 79, 134, 53, 69, 116, 44, 58,  99, 33, 46,  81, 21, 34,  64,  3, 20,  41,  0,  2,   0,  0,  0,
    170, 71, 76, 152, 62, 66, 134, 53, 57, 116, 43, 46,  99, 33, 36,  82, 22, 24,  64,  5, 10,  40,  0,  0,   0,  0,  0,
    171, 70, 61, 152, 62, 52, 135, 53, 44, 116, 44, 35,  99, 34, 24,  82, 22, 12,  63,  6,  0,  39,  0,  0,   0,  0,  0,
    171, 71, 49, 153, 62, 41, 135, 54, 33, 117, 45, 25,  98, 34, 13,  81, 23,  0,  63,  7,  0,  39,  0,  0,   0,  0,  0,
    167, 33, 142, 149, 24, 127, 134, 10, 113, 116,  0, 97, 100,  0, 83,  83,  0, 68,  65,  0, 52,  40,  0, 30,   0,  0,  0,
    169, 33, 129, 150, 26, 115, 134, 17, 102, 116,  3, 89, 100,  0, 75,  83,  0, 60,  65,  0, 45,  40,  0, 24,   0,  0,  0,
    169, 36, 118, 151, 27, 104, 134, 19, 93, 116,  7, 80, 100,  0, 67,  83,  0, 54,  65,  0, 38,  41,  0, 17,   0,  0,  0,
    169, 39, 107, 150, 30, 94, 134, 22, 84, 116, 11, 71,  99,  0, 59,  83,  0, 46,  64,  0, 31,  40,  0,  9,   0,  0,  0,
    169, 39, 95, 151, 31, 83, 134, 24, 73, 116, 15, 62, 100,  1, 51,  83,  0, 38,  64,  0, 24,  40,  0,  5,   0,  0,  0,
    169, 41, 83, 151, 33, 73, 134, 26, 64, 117, 17, 54,  99,  4, 42,  82,  0, 30,  64,  0, 16,  40,  0,  1,   0,  0,  0,
    170, 42, 71, 152, 34, 62, 134, 28, 53, 117, 19, 44,  99,  6, 33,  82,  0, 21,  63,  0,  4,  39,  0,  0,   0,  0,  0,
    171, 42, 59, 152, 35, 50, 134, 29, 42, 117, 21, 32,  99,  9, 22,  82,  0,  9,  63,  0,  0,  38,  0,  0,   0,  0,  0,
    172, 42, 48, 152, 36, 40, 135, 29, 32, 117, 21, 23,  99, 10, 12,  82,  0,  0,  63,  0,  0,  38,  0,  0,   0,  0,  0,
    107, 207, 246,  96, 185, 220,  86, 165, 196,  73, 144, 171,  60, 123, 147,  46, 103, 125,  32, 82, 100,   9, 56, 71,   0, 20, 33,
    115, 206, 221, 104, 184, 198,  92, 164, 178,  78, 143, 154,  64, 123, 133,  51, 102, 111,  34, 81, 89,  10, 56, 63,   0, 20, 27,
    122, 204, 200, 108, 183, 180,  95, 163, 161,  82, 142, 140,  68, 122, 120,  54, 102, 101,  36, 81, 79,  11, 56, 55,   0, 20, 20,
    125, 203, 179, 111, 181, 160,  97, 162, 143,  85, 141, 124,  70, 121, 107,  55, 101, 89,  38, 80, 69,  14, 55, 46,   0, 19, 10,
    128, 202, 156, 113, 180, 140, 102, 161, 125,  87, 140, 108,  71, 120, 92,  56, 100, 76,  39, 79, 59,  14, 55, 38,   0, 20,  3,
    132, 200, 135, 117, 179, 121, 103, 159, 106,  88, 139, 93,  73, 119, 79,  57, 100, 65,  41, 79, 49,  15, 54, 28,   0, 19,  0,
    134, 200, 111, 119, 178, 98, 105, 158, 87,  89, 138, 76,  74, 118, 64,  58, 99, 51,  41, 78, 37,  16, 54, 17,   0, 19,  0,
    137, 199, 85, 122, 177, 75, 108, 158, 66,  91, 137, 56,  75, 118, 46,  59, 98, 35,  42, 78, 22,  16, 54,  3,   0, 19,  0,
    140, 198, 62, 125, 177, 55, 109, 158, 47,  92, 137, 40,  76, 117, 32,  59, 98, 21,  42, 78,  6,  16, 54,  0,   0, 18,  0,
    118, 186, 231, 106, 167, 206,  93, 149, 184,  81, 130, 161,  67, 111, 139,  54, 92, 117,  39, 72, 93,  17, 48, 66,   0, 10, 29,
    123, 185, 207, 110, 166, 186,  98, 148, 167,  85, 129, 145,  71, 111, 125,  56, 92, 104,  40, 72, 83,  18, 48, 57,   0, 10, 22,
    128, 184, 188, 113, 165, 168, 102, 147, 151,  88, 128, 131,  73, 110, 113,  58, 91, 94,  42, 71, 74,  19, 48, 50,   0,  9, 15,
    131, 183, 168, 116, 164, 151, 104, 146, 134,  89, 127, 117,  73, 109, 100,  58, 90, 83,  42, 71, 65,  20, 48, 42,   0,  9,  5,
    134, 182, 148, 120, 163, 131, 105, 145, 118,  90, 126, 102,  75, 108, 86,  59, 90, 72,  43, 71, 55,  19, 47, 34,   0,  9,  0,
    136, 181, 128, 122, 162, 115, 107, 144, 102,  92, 125, 87,  76, 107, 74,  61, 89, 60,  44, 70, 45,  20, 47, 24,   0,  8,  0,
    139, 180, 106, 124, 161, 95, 109, 144, 83,  93, 124, 71,  77, 107, 60,  61, 89, 47,  44, 70, 33,  20, 47, 13,   0,  8,  0,
    142, 179, 82, 125, 160, 72, 111, 143, 63,  94, 124, 54,  77, 106, 44,  61, 88, 32,  44, 69, 18,  20, 46,  0,   0,  8,  0,
    143, 179, 62, 127, 160, 54, 111, 142, 47,  94, 124, 39,  78, 106, 29,  62, 88, 18,  45, 69,  3,  20, 46,  0,   0,  8,  0,
    124, 167, 216, 112, 150, 194,  99, 134, 174,  87, 117, 153,  73, 100, 131,  58, 82, 110,  43, 64, 88,  23, 40, 61,   0,  0, 24,
    129, 166, 195, 116, 150, 175, 103, 134, 158,  89, 116, 137,  75, 99, 118,  60, 82, 98,  44, 63, 78,  23, 40, 53,   0,  0, 17,
    132, 166, 177, 119, 149, 160, 106, 133, 143,  90, 115, 124,  76, 99, 107,  61, 81, 88,  45, 63, 69,  24, 40, 46,   0,  0,  9,
    136, 166, 159, 121, 148, 143, 107, 132, 126,  92, 115, 111,  77, 98, 94,  62, 81, 78,  46, 63, 60,  23, 40, 38,   0,  0,  0,
    138, 164, 140, 122, 147, 125, 108, 131, 111,  93, 114, 97,  79, 98, 82,  63, 80, 67,  46, 62, 50,  24, 40, 29,   0,  0,  0,
    139, 163, 122, 124, 146, 109, 110, 131, 96,  94, 114, 83,  79, 97, 70,  63, 81, 57,  46, 62, 41,  24, 40, 21,   0,  0,  0,
    141, 163, 101, 126, 145, 90, 111, 130, 79,  95, 113, 68,  79, 96, 56,  63, 80, 44,  47, 62, 30,  23, 40, 10,   0,  0,  0,
    144, 162, 79, 127, 145, 70, 112, 129, 60,  95, 112, 51,  79, 96, 41,  64, 79, 30,  47, 61, 15,  23, 40,  0,   0,  0,  0,
    145, 162, 60, 129, 145, 52, 113, 129, 46,  96, 112, 37,  79, 95, 27,  64, 79, 16,  47, 61,  1,  23, 39,  0,   0,  0,  0,
    131, 147, 202, 117, 133, 181, 105, 119, 162,  91, 103, 142,  77, 87, 122,  62, 71, 102,  47, 54, 81,  26, 31, 56,   0,  0, 18,
    135, 147, 183, 120, 132, 164, 107, 118, 147,  93, 102, 128,  78, 87, 110,  63, 71, 92,  47, 54, 72,  26, 31, 48,   0,  0, 10,
    138, 147, 166, 123, 131, 149, 108, 118, 133,  94, 102, 116,  79, 86, 100,  64, 71, 82,  48, 54, 64,  27, 31, 41,   0,  0,  2,
    139, 146, 149, 124, 131, 134, 111, 117, 119,  94, 101, 103,  79, 86, 88,  64, 70, 72,  48, 53, 55,  27, 31, 33,   0,  0,  0,
    141, 146, 132, 125, 131, 117, 111, 117, 104,  95, 101, 91,  80, 86, 77,  65, 70, 62,  48, 53, 46,  26, 31, 25,   0,  0,  0,
    143, 145, 115, 126, 130, 101, 112, 116, 90,  96, 100, 78,  80, 85, 65,  65, 70, 52,  49, 53, 37,  27, 32, 17,   0,  0,  0,
    144, 144, 96, 128, 129, 85, 112, 115, 75,  97, 100, 64,  81, 85, 52,  65, 69, 40,  49, 53, 26,  26, 31,  5,   0,  0,  0,
    146, 144, 76, 129, 129, 67, 114, 115, 58,  97, 99, 48,  82, 84, 38,  66, 69, 27,  49, 53, 12,  26, 32,  0,   0,  0,  0,
    146, 144, 59, 130, 128, 51, 114, 114, 43,  98, 99, 35,  82, 84, 25,  66, 69, 13,  49, 53,  0,  26, 32,  0,   0,  0,  0,
    135, 129, 189, 122, 115, 170, 107, 103, 152,  94, 89, 133,  79, 74, 114,  64, 60, 95,  49, 43, 75,  29, 20, 51,   0,  0, 12,
    138, 129, 171, 124, 115, 153, 110, 103, 138,  95, 89, 120,  81, 74, 103,  66, 60, 86,  50, 44, 67,  28, 21, 43,   0,  0,  3,
    140, 129, 156, 125, 115, 140, 111, 103, 125,  96, 89, 109,  81, 74, 93,  67, 60, 76,  50, 44, 59,  29, 22, 36,   0,  0,  0,
    142, 128, 140, 127, 115, 125, 112, 102, 112,  97, 88, 97,  82, 74, 83,  67, 60, 67,  50, 44, 51,  29, 22, 29,   0,  0,  0,
    142, 128, 124, 127, 114, 111, 113, 102, 98,  98, 88, 85,  82, 74, 71,  66, 60, 58,  50, 44, 42,  29, 22, 21,   0,  0,  0,
    144, 127, 108, 128, 114, 96, 113, 101, 85,  98, 87, 73,  82, 74, 61,  67, 60, 48,  50, 44, 33,  28, 23, 12,   0,  0,  0,
    145, 127, 91, 129, 114, 81, 115, 101, 71,  98, 87, 60,  82, 73, 48,  67, 59, 37,  50, 44, 22,  29, 23,  1,   0,  0,  0,
    147, 127, 73, 130, 113, 63, 115, 101, 55,  98, 87, 45,  83, 73, 35,  67, 59, 24,  50, 44, 10,  28, 24,  0,   0,  0,  0,
    147, 127, 58, 131, 113, 49, 115, 100, 42,  99, 86, 33,  83, 73, 23,  67, 59, 10,  50, 44,  0,  27, 24,  0,   0,  0,  0,
    138, 110, 177, 124, 99, 159, 110, 88, 142,  96, 75, 125,  82, 62, 107,  66, 48, 89,  51, 33, 70,  30,  8, 46,   0,  0,  5,
    142, 111, 160, 127, 99, 144, 113, 88, 130,  98, 75, 112,  82, 62, 96,  68, 49, 80,  51, 33, 61,  30, 10, 39,   0,  0,  0,
    143, 111, 146, 128, 99, 131, 114, 88, 118,  98, 75, 101,  83, 62, 86,  68, 49, 71,  52, 33, 54,  30, 11, 32,   0,  0,  0,
    144, 111, 132, 128, 99, 118, 113, 88, 106,  99, 75, 91,  83, 62, 77,  68, 49, 62,  52, 34, 46,  30, 12, 25,   0,  0,  0,
    144, 111, 117, 129, 98, 104, 114, 87, 92,  99, 75, 80,  83, 62, 67,  68, 49, 53,  51, 34, 38,  30, 13, 18,   0,  0,  0,
    145, 111, 103, 130, 98, 91, 114, 87, 80,  99, 75, 68,  83, 63, 57,  68, 50, 45,  51, 34, 30,  30, 14,  8,   0,  0,  0,
    146, 110, 87, 131, 98, 76, 115, 87, 67,  99, 75, 56,  83, 62, 45,  68, 49, 33,  52, 35, 19,  30, 15,  2,   0,  0,  0,
    148, 110, 70, 131, 98, 60, 116, 86, 52,  99, 74, 43,  84, 62, 33,  69, 49, 21,  52, 35,  6,  29, 15,  0,   0,  0,  0,
    148, 110, 56, 132, 97, 48, 117, 87, 40, 100, 75, 31,  84, 62, 22,  68, 49,  9,  51, 35,  0,  28, 15,  0,   0,  0,  0,
    142, 91, 166, 126, 80, 148, 113, 71, 132,  98, 59, 115,  83, 47, 99,  69, 34, 82,  53, 17, 64,  32,  0, 41,   0,  0,  0,
    143, 91, 150, 128, 81, 135, 114, 71, 120,  99, 60, 104,  85, 48, 89,  69, 35, 73,  53, 19, 56,  32,  0, 34,   0,  0,  0,
    145, 91, 137, 129, 81, 122, 115, 71, 109, 100, 60, 94,  85, 48, 81,  69, 35, 65,  53, 19, 49,  32,  0, 28,   0,  0,  0,
    146, 92, 124, 130, 81, 110, 115, 71, 98, 100, 60, 84,  85, 49, 71,  69, 36, 57,  53, 21, 42,  32,  0, 21,   0,  0,  0,
    147, 91, 110, 130, 81, 97, 115, 71, 86, 100, 60, 74,  84, 49, 62,  69, 36, 48,  53, 22, 34,  32,  0, 13,   0,  0,  0,
    147, 92, 97, 130, 81, 85, 116, 72, 76, 100, 60, 63,  85, 49, 52,  69, 37, 40,  53, 22, 26,  31,  1,  5,   0,  0,  0,
    148, 92, 82, 131, 81, 71, 116, 71, 62, 100, 60, 53,  84, 49, 42,  69, 37, 30,  52, 23, 16,  31,  2,  0,   0,  0,  0,
    148, 91, 67, 132, 81, 57, 117, 71, 49, 100, 60, 39,  84, 49, 30,  69, 37, 18,  52, 23,  2,  30,  2,  0,   0,  0,  0,
    149, 91, 54, 132, 81, 46, 118, 71, 39, 101, 60, 29,  85, 49, 19,  69, 37,  6,  52, 23,  0,  29,  3,  0,   0,  0,  0,
    143, 68, 153, 128, 59, 137, 115, 49, 122,  99, 39, 107,  85, 28, 91,  70, 13, 75,  54,  0, 58,  32,  0, 36,   0,  0,  0,
    146, 68, 140, 131, 59, 125, 116, 51, 111, 100, 40, 97,  85, 29, 82,  70, 15, 67,  54,  0, 50,  32,  0, 29,   0,  0,  0,
    147, 68, 127, 131, 59, 114, 117, 51, 102, 101, 41, 88,  86, 30, 74,  70, 17, 60,  54,  0, 44,  32,  0, 23,   0,  0,  0,
    147, 70, 115, 131, 60, 103, 116, 52, 91, 100, 42, 78,  85, 32, 65,  70, 19, 53,  54,  1, 38,  32,  0, 17,   0,  0,  0,
    147, 70, 103, 131, 61, 91, 117, 53, 81, 101, 43, 69,  86, 32, 57,  70, 20, 44,  54,  2, 30,  32,  0,  7,   0,  0,  0,
    148, 70, 91, 132, 61, 80, 117, 52, 70, 101, 43, 59,  85, 33, 48,  70, 21, 36,  53,  4, 22,  32,  0,  3,   0,  0,  0,
    148, 70, 78, 132, 62, 68, 117, 53, 58, 101, 43, 48,  85, 34, 38,  70, 22, 26,  53,  6, 12,  31,  0,  0,   0,  0,  0,
    149, 71, 64, 132, 62, 54, 118, 54, 46, 101, 44, 37,  85, 34, 27,  69, 23, 15,  53,  7,  1,  30,  0,  0,   0,  0,  0,
    150, 70, 53, 134, 61, 44, 118, 54, 36, 101, 44, 28,  85, 35, 17,  69, 23,  4,  52,  8,  0,  30,  0,  0,   0,  0,  0,
    145, 38, 143, 130, 29, 128, 117, 18, 114, 101,  3, 98,  87,  0, 84,  72,  0, 69,  54,  0, 53,  30,  0, 31,   0,  0,  0,
    147, 38, 130, 132, 30, 116, 117, 22, 103, 101,  8, 89,  87,  0, 76,  72,  0, 62,  54,  0, 46,  30,  0, 24,   0,  0,  0,
    148, 40, 119, 132, 31, 105, 117, 23, 94, 101, 13, 81,  87,  0, 68,  71,  0, 55,  54,  0, 39,  30,  0, 18,   0,  0,  0,
    148, 42, 108, 132, 34, 96, 117, 25, 85, 102, 15, 73,  86,  2, 60,  71,  0, 47,  54,  0, 33,  30,  0, 11,   0,  0,  0,
    148, 43, 96, 133, 35, 85, 117, 28, 75, 102, 18, 64,  87,  5, 52,  71,  0, 40,  54,  0, 25,  30,  0,  5,   0,  0,  0,
    149, 44, 85, 132, 36, 75, 118, 29, 66, 101, 20, 55,  86,  8, 44,  70,  0, 32,  53,  0, 18,  29,  0,  2,   0,  0,  0,
    149, 45, 74, 133, 37, 64, 118, 31, 55, 102, 21, 45,  85, 10, 34,  70,  0, 22,  53,  0,  6,  28,  0,  0,   0,  0,  0,
    150, 46, 61, 133, 39, 52, 118, 31, 44, 102, 23, 34,  85, 12, 24,  70,  0, 12,  52,  0,  0,  28,  0,  0,   0,  0,  0,
    150, 46, 51, 133, 40, 42, 119, 32, 35, 102, 24, 25,  85, 13, 14,  70,  0,  1,  52,  0,  0,  27,  0,  0,   0,  0,  0,
    53, 198, 244,  49, 177, 218,  41, 158, 195,  32, 138, 171,  22, 118, 147,  11, 98, 124,   0, 78, 100,   0, 54, 71,   0, 18, 34,
    69, 196, 220,  64, 175, 196,  54, 157, 176,  45, 137, 154,  32, 117, 133,  19, 98, 111,   0, 78, 89,   0, 53, 63,   0, 17, 27,
    80, 195, 198,  69, 175, 179,  60, 156, 159,  50, 136, 139,  38, 116, 120,  25, 98, 101,   4, 77, 80,   0, 53, 55,   0, 17, 21,
    84, 193, 177,  75, 173, 159,  64, 155, 142,  55, 135, 124,  41, 116, 107,  27, 97, 89,   9, 76, 70,   0, 53, 47,   0, 17, 11,
    89, 193, 157,  79, 172, 140,  70, 154, 125,  57, 134, 109,  44, 115, 92,  32, 96, 76,  13, 76, 59,   0, 52, 39,   0, 16,  4,
    94, 191, 135,  85, 171, 121,  72, 152, 108,  60, 133, 94,  47, 114, 80,  32, 95, 65,  15, 76, 49,   0, 52, 29,   0, 16,  0,
    98, 190, 113,  87, 170, 100,  76, 152, 89,  62, 132, 77,  49, 113, 65,  35, 95, 52,  18, 75, 37,   0, 52, 18,   0, 15,  0,
    103, 190, 89,  90, 169, 80,  78, 151, 70,  64, 132, 60,  51, 113, 49,  37, 94, 38,  20, 75, 25,   0, 52,  5,   0, 15,  0,
    106, 189, 69,  93, 169, 61,  80, 151, 53,  66, 131, 45,  52, 113, 36,  37, 94, 25,  19, 74, 11,   0, 51,  0,   0, 15,  0,
    76, 178, 229,  68, 159, 205,  61, 142, 183,  50, 124, 160,  40, 106, 138,  28, 88, 116,  12, 69, 93,   0, 45, 66,   0,  5, 29,
    86, 177, 207,  78, 158, 184,  67, 142, 166,  56, 123, 145,  45, 106, 125,  31, 88, 105,  16, 69, 83,   0, 45, 58,   0,  6, 22,
    93, 176, 187,  81, 158, 168,  71, 141, 150,  61, 123, 131,  47, 105, 113,  35, 87, 94,  20, 68, 74,   0, 45, 51,   0,  5, 16,
    98, 175, 168,  84, 157, 150,  75, 140, 134,  63, 122, 117,  50, 104, 100,  37, 87, 83,  21, 68, 65,   0, 45, 42,   0,  4,  7,
    100, 174, 149,  89, 155, 132,  76, 139, 117,  65, 121, 102,  53, 104, 87,  39, 86, 72,  23, 67, 55,   0, 45, 34,   0,  3,  0,
    103, 173, 130,  92, 155, 115,  80, 138, 102,  68, 120, 88,  53, 103, 75,  40, 86, 61,  24, 67, 45,   0, 45, 25,   0,  3,  0,
    107, 172, 108,  95, 154, 96,  82, 137, 85,  70, 119, 73,  55, 102, 61,  42, 85, 49,  25, 67, 34,   0, 45, 14,   0,  3,  0,
    110, 172, 86,  97, 153, 76,  85, 137, 67,  70, 119, 57,  56, 102, 46,  42, 84, 35,  26, 66, 21,   0, 44,  1,   0,  3,  0,
    112, 171, 67,  98, 153, 59,  86, 137, 52,  71, 119, 44,  58, 102, 34,  44, 85, 22,  27, 66,  7,   0, 44,  0,   0,  3,  0,
    90, 160, 215,  81, 144, 193,  70, 129, 173,  61, 112, 151,  49, 95, 131,  37, 79, 109,  22, 61, 87,   0, 38, 61,   0,  0, 25,
    96, 160, 194,  86, 143, 174,  75, 128, 157,  65, 112, 137,  53, 95, 117,  40, 78, 98,  25, 60, 78,   0, 38, 53,   0,  0, 17,
    100, 159, 177,  89, 143, 159,  79, 128, 143,  67, 111, 124,  55, 95, 107,  42, 78, 89,  27, 60, 70,   2, 38, 46,   0,  0,  9,
    104, 158, 159,  92, 142, 143,  81, 127, 127,  69, 110, 110,  56, 94, 94,  43, 78, 78,  28, 60, 60,   2, 38, 38,   0,  0,  1,
    107, 157, 140,  94, 141, 125,  82, 126, 112,  71, 110, 97,  59, 94, 82,  45, 77, 67,  29, 59, 51,   4, 37, 30,   0,  0,  0,
    110, 156, 122,  97, 140, 109,  85, 125, 97,  72, 109, 83,  58, 93, 71,  45, 77, 57,  29, 60, 42,   5, 38, 22,   0,  0,  0,
    111, 156, 103,  99, 139, 91,  87, 125, 81,  73, 108, 69,  60, 92, 58,  46, 77, 45,  30, 59, 31,   5, 38, 12,   0,  0,  0,
    115, 156, 82, 101, 140, 73,  88, 124, 63,  74, 108, 53,  60, 92, 44,  46, 76, 32,  31, 59, 18,   6, 37,  0,   0,  0,  0,
    116, 155, 65, 102, 139, 58,  89, 124, 49,  75, 108, 41,  61, 92, 32,  48, 76, 21,  31, 59,  6,   5, 37,  0,   0,  0,  0,
    100, 141, 201,  88, 127, 181,  79, 114, 162,  69, 99, 142,  57, 83, 122,  44, 68, 102,  30, 51, 81,   7, 28, 56,   0,  0, 19,
    105, 141, 182,  94, 127, 163,  83, 114, 146,  71, 98, 128,  59, 83, 110,  46, 68, 91,  31, 51, 72,  10, 28, 48,   0,  0, 11,
    108, 141, 166,  96, 127, 149,  85, 113, 133,  73, 98, 116,  60, 83, 99,  46, 68, 82,  32, 51, 64,  11, 29, 41,   0,  0,  2,
    111, 141, 149,  98, 126, 134,  88, 112, 119,  74, 97, 103,  61, 83, 88,  48, 67, 72,  33, 51, 56,  11, 29, 34,   0,  0,  0,
    112, 140, 132, 100, 125, 118,  89, 112, 105,  75, 97, 91,  62, 82, 77,  49, 68, 62,  33, 51, 47,  12, 29, 26,   0,  0,  0,
    115, 140, 116, 102, 125, 103,  90, 111, 91,  76, 96, 78,  62, 82, 65,  49, 67, 52,  34, 51, 38,  13, 29, 18,   0,  0,  0,
    117, 139, 97, 103, 124, 87,  91, 111, 77,  78, 96, 65,  63, 81, 54,  49, 67, 41,  34, 51, 27,  12, 29,  7,   0,  0,  0,
    119, 138, 78, 105, 124, 69,  92, 110, 60,  78, 95, 50,  65, 81, 40,  50, 67, 29,  34, 51, 15,  13, 30,  0,   0,  0,  0,
    120, 138, 64, 106, 124, 54,  93, 110, 47,  78, 95, 38,  65, 81, 29,  50, 66, 17,  34, 50,  2,  13, 29,  0,   0,  0,  0,
    107, 124, 189,  96, 111, 169,  85, 99, 152,  73, 85, 132,  61, 71, 114,  48, 57, 95,  34, 41, 75,  14, 18, 51,   0,  0, 13,
    111, 124, 171, 100, 111, 153,  88, 99, 137,  75, 85, 120,  63, 72, 103,  50, 58, 85,  36, 41, 66,  15, 19, 43,   0,  0,  4,
    113, 124, 156, 101, 111, 139,  90, 99, 125,  77, 85, 109,  64, 71, 93,  51, 57, 77,  36, 42, 59,  17, 20, 37,   0,  0,  0,
    115, 124, 140, 103, 111, 125,  90, 99, 112,  78, 85, 97,  64, 71, 82,  52, 57, 67,  36, 42, 50,  16, 20, 30,   0,  0,  0,
    117, 123, 125, 104, 110, 111,  92, 98, 99,  79, 85, 86,  65, 71, 72,  51, 58, 59,  37, 42, 43,  17, 21, 22,   0,  0,  0,
    118, 123, 110, 105, 110, 97,  93, 98, 86,  78, 84, 74,  66, 71, 62,  52, 57, 49,  37, 42, 34,  17, 22, 14,   0,  0,  0,
    120, 123, 93, 106, 109, 82,  94, 97, 72,  80, 84, 61,  66, 71, 50,  52, 57, 38,  37, 42, 24,  17, 22,  2,   0,  0,  0,
    121, 122, 75, 108, 109, 66,  95, 97, 58,  80, 84, 48,  66, 71, 37,  52, 57, 26,  37, 42, 12,  16, 22,  0,   0,  0,  0,
    122, 123, 62, 108, 109, 52,  95, 97, 45,  81, 84, 36,  67, 70, 26,  52, 57, 14,  37, 42,  0,  15, 22,  0,   0,  0,  0,
    113, 107, 177, 102, 96, 159,  89, 85, 141,  78, 72, 124,  65, 60, 107,  52, 46, 89,  37, 30, 70,  18,  5, 46,   0,  0,  6,
    116, 107, 160, 104, 96, 144,  92, 85, 129,  80, 72, 112,  67, 60, 96,  53, 47, 80,  38, 31, 62,  19,  7, 39,   0,  0,  0,
    118, 107, 147, 105, 96, 131,  93, 85, 118,  80, 72, 101,  67, 60, 87,  54, 47, 71,  39, 31, 54,  19,  8, 32,   0,  0,  0,
    119, 107, 132, 106, 96, 118,  94, 85, 106,  81, 73, 91,  67, 60, 77,  54, 47, 63,  39, 32, 47,  20,  9, 25,   0,  0,  0,
    119, 107, 118, 106, 95, 105,  94, 85, 93,  81, 72, 80,  68, 60, 68,  54, 47, 54,  39, 32, 39,  20, 11, 18,   0,  0,  0,
    121, 107, 104, 107, 96, 92,  95, 84, 80,  81, 72, 69,  68, 61, 58,  54, 48, 46,  39, 33, 31,  20, 12,  9,   0,  0,  0,
    123, 107, 88, 108, 95, 77,  96, 84, 68,  82, 72, 57,  68, 60, 46,  54, 47, 35,  39, 33, 20,  19, 13,  2,   0,  0,  0,
    123, 106, 72, 110, 95, 63,  96, 84, 54,  82, 72, 45,  69, 60, 35,  55, 48, 23,  39, 33,  9,  18, 14,  0,   0,  0,  0,
    125, 106, 60, 110, 94, 50,  98, 84, 42,  83, 72, 34,  69, 60, 25,  55, 48, 12,  39, 33,  0,  17, 13,  0,   0,  0,  0,
    118, 89, 165, 105, 79, 148,  93, 69, 132,  81, 57, 115,  68, 45, 99,  55, 32, 82,  41, 15, 64,  21,  0, 41,   0,  0,  0,
    120, 89, 150, 107, 79, 135,  96, 69, 121,  82, 58, 105,  70, 46, 89,  56, 34, 73,  41, 17, 56,  21,  0, 34,   0,  0,  0,
    121, 89, 137, 108, 79, 123,  96, 69, 109,  82, 58, 95,  70, 47, 81,  56, 34, 66,  41, 18, 49,  21,  0, 28,   0,  0,  0,
    122, 90, 124, 109, 79, 110,  96, 69, 99,  83, 58, 85,  70, 47, 72,  56, 35, 58,  41, 19, 42,  21,  0, 22,   0,  0,  0,
    123, 90, 111, 110, 79, 98,  97, 69, 87,  83, 59, 75,  70, 47, 63,  56, 35, 50,  41, 20, 35,  21,  0, 14,   0,  0,  0,
    123, 90, 98, 110, 79, 87,  97, 70, 76,  84, 58, 64,  70, 48, 53,  56, 36, 41,  40, 21, 26,  21,  0,  5,   0,  0,  0,
    125, 89, 84, 111, 79, 73,  97, 69, 64,  84, 59, 54,  70, 48, 43,  56, 36, 31,  40, 22, 17,  20,  1,  1,   0,  0,  0,
    125, 89, 69, 112, 79, 60,  98, 70, 51,  84, 59, 42,  70, 48, 32,  56, 36, 20,  41, 22,  5,  19,  2,  0,   0,  0,  0,
    126, 89, 57, 112, 79, 49,  99, 70, 41,  84, 59, 32,  70, 48, 22,  56, 36, 10,  40, 22,  0,  18,  2,  0,   0,  0,  0,
    121, 67, 154, 108, 58, 138,  97, 50, 124,  84, 39, 107,  71, 28, 92,  58, 12, 76,  43,  0, 59,  20,  0, 37,   0,  0,  0,
    124, 68, 140, 111, 59, 126,  98, 50, 112,  84, 40, 98,  71, 29, 83,  58, 15, 67,  42,  0, 51,  20,  0, 30,   0,  0,  0,
    124, 68, 129, 111, 59, 114,  99, 51, 102,  86, 41, 88,  71, 30, 75,  58, 17, 60,  42,  0, 45,  20,  0, 24,   0,  0,  0,
    125, 70, 116, 111, 60, 103,  99, 51, 92,  85, 41, 79,  71, 31, 66,  58, 19, 53,  42,  3, 38,  20,  0, 17,   0,  0,  0,
    125, 70, 104, 111, 61, 93,  99, 52, 81,  85, 43, 69,  72, 32, 58,  58, 20, 45,  42,  4, 31,  20,  0,  8,   0,  0,  0,
    126, 70, 92, 111, 61, 81,  99, 52, 71,  85, 42, 60,  71, 33, 49,  57, 21, 37,  42,  6, 23,  20,  0,  3,   0,  0,  0,
    126, 70, 79, 112, 61, 70,  99, 53, 60,  85, 43, 50,  71, 33, 39,  57, 22, 28,  41,  7, 13,  19,  0,  0,   0,  0,  0,
    127, 71, 66, 113, 62, 56, 100, 53, 48,  86, 44, 39,  71, 34, 29,  57, 23, 18,  41,  8,  2,  18,  0,  0,   0,  0,  0,
    128, 70, 55, 114, 62, 46, 100, 54, 39,  86, 44, 30,  71, 34, 20,  57, 23,  7,  41,  9,  0,  18,  0,  0,   0,  0,  0,
    124, 41, 145, 111, 32, 128,  99, 23, 114,  86, 10, 100,  73,  0, 85,  60,  0, 71,  43,  0, 54,  17,  0, 32,   0,  0,  0,
    126, 42, 131, 113, 33, 117, 100, 25, 104,  86, 14, 90,  73,  0, 77,  60,  0, 63,  44,  0, 47,  18,  0, 25,   0,  0,  0,
    127, 43, 120, 113, 34, 106, 101, 26, 95,  86, 17, 82,  73,  2, 69,  59,  0, 56,  43,  0, 41,  18,  0, 19,   0,  0,  0,
    127, 45, 109, 113, 37, 97, 101, 28, 85,  86, 19, 74,  73,  5, 61,  59,  0, 48,  43,  0, 34,  19,  0, 11,   0,  0,  0,
    127, 46, 98, 114, 38, 86, 100, 30, 76,  87, 21, 65,  73,  9, 54,  59,  0, 41,  43,  0, 26,  18,  0,  5,   0,  0,  0,
    127, 47, 87, 113, 39, 76, 101, 31, 67,  86, 22, 56,  72, 11, 45,  59,  0, 33,  43,  0, 19,  18,  0,  2,   0,  0,  0,
    128, 48, 75, 114, 39, 65, 101, 33, 56,  86, 23, 46,  72, 12, 36,  58,  0, 24,  42,  0,  9,  17,  0,  0,   0,  0,  0,
    129, 48, 63, 114, 41, 54, 102, 33, 46,  87, 24, 36,  72, 14, 26,  58,  1, 14,  42,  0,  2,  16,  0,  0,   0,  0,  0,
    128, 48, 53, 114, 41, 44, 102, 34, 37,  87, 25, 27,  72, 15, 17,  58,  1,  3,  41,  0,  0,  15,  0,  0,   0,  0,  0,
    0, 189, 242,   0, 169, 217,   0, 151, 194,   0, 132, 170,   0, 113, 147,   0, 94, 123,   0, 74, 99,   0, 51, 71,   0, 15, 34,
    1, 187, 219,   1, 167, 195,   0, 150, 175,   0, 131, 153,   0, 113, 132,   0, 94, 111,   0, 74, 89,   0, 50, 63,   0, 13, 28,
    1, 186, 198,   1, 167, 178,   0, 149, 158,   0, 130, 139,   0, 111, 119,   0, 93, 100,   0, 74, 80,   0, 50, 55,   0, 13, 22,
    1, 185, 176,   1, 165, 159,   1, 148, 142,   0, 129, 123,   0, 111, 106,   0, 93, 89,   0, 73, 70,   0, 50, 47,   0, 13, 13,
    1, 184, 157,   1, 164, 141,   1, 147, 125,   0, 128, 110,   0, 110, 93,   0, 92, 77,   0, 73, 60,   0, 50, 39,   0, 12,  5,
    25, 182, 137,  25, 163, 122,  17, 146, 109,   0, 128, 96,   0, 110, 81,   0, 92, 66,   0, 73, 51,   0, 50, 30,   0, 10,  0,
    42, 181, 114,  35, 163, 102,  30, 145, 91,  14, 127, 80,   0, 109, 67,   0, 91, 53,   0, 72, 39,   0, 50, 19,   0, 10,  0,
    52, 181, 92,  43, 162, 83,  32, 145, 73,  19, 126, 63,   0, 108, 52,   0, 90, 40,   0, 72, 27,   0, 50,  7,   0, 10,  0,
    57, 181, 74,  48, 162, 66,  37, 144, 57,  24, 126, 49,   7, 108, 40,   0, 90, 29,   0, 72, 15,   0, 49,  0,   0, 10,  0,
    1, 170, 227,   1, 152, 203,   0, 136, 182,   0, 119, 159,   0, 101, 137,   0, 84, 115,   0, 65, 92,   0, 43, 66,   0,  1, 29,
    1, 169, 206,   1, 151, 184,   1, 136, 165,   0, 118, 144,   0, 102, 125,   0, 84, 105,   0, 65, 83,   0, 43, 58,   0,  0, 22,
    29, 168, 186,  21, 151, 167,  14, 135, 150,   4, 118, 131,   0, 101, 112,   0, 83, 94,   0, 65, 75,   0, 43, 51,   0,  0, 16,
    41, 167, 167,  33, 150, 150,  31, 134, 134,  19, 117, 117,   4, 100, 100,   0, 83, 83,   0, 65, 65,   0, 42, 43,   0,  0,  8,
    48, 167, 149,  41, 149, 133,  33, 133, 118,  25, 116, 103,  13, 99, 88,   0, 83, 73,   0, 65, 56,   0, 42, 35,   0,  0,  0,
    58, 165, 130,  49, 148, 115,  42, 132, 103,  31, 115, 89,  18, 99, 75,   0, 82, 61,   0, 64, 46,   0, 42, 26,   0,  0,  0,
    62, 164, 110,  55, 147, 97,  45, 132, 87,  35, 115, 75,  22, 98, 63,   5, 82, 50,   0, 64, 36,   0, 42, 16,   0,  0,  0,
    69, 164, 89,  60, 147, 78,  50, 131, 70,  37, 114, 59,  26, 98, 49,  10, 81, 37,   0, 64, 24,   0, 42,  4,   0,  0,  0,
    71, 164, 71,  63, 147, 63,  53, 131, 55,  40, 114, 47,  28, 98, 38,  13, 81, 26,   0, 64, 12,   0, 42,  0,   0,  0,  0,
    28, 153, 214,  24, 138, 193,  23, 123, 171,  16, 107, 150,   0, 91, 130,   0, 75, 109,   0, 58, 87,   0, 35, 61,   0,  0, 25,
    48, 153, 194,  41, 138, 174,  34, 123, 156,  27, 107, 136,  16, 91, 117,   1, 75, 98,   0, 57, 78,   0, 35, 53,   0,  0, 17,
    55, 153, 177,  47, 137, 158,  42, 122, 142,  33, 107, 124,  22, 91, 106,   6, 75, 88,   0, 57, 70,   0, 35, 46,   0,  0,  9,
    61, 152, 158,  53, 136, 143,  45, 122, 127,  36, 106, 111,  24, 90, 94,  10, 74, 78,   0, 57, 61,   0, 35, 39,   0,  0,  2,
    67, 151, 141,  59, 135, 126,  49, 121, 112,  39, 105, 98,  29, 90, 83,  14, 74, 68,   0, 57, 52,   0, 35, 31,   0,  0,  0,
    71, 150, 123,  62, 135, 110,  54, 120, 98,  42, 105, 84,  31, 89, 71,  16, 74, 58,   0, 57, 43,   0, 35, 22,   0,  0,  0,
    74, 150, 105,  64, 134, 92,  55, 120, 83,  45, 104, 71,  34, 89, 59,  20, 73, 47,   0, 57, 32,   0, 35, 13,   0,  0,  0,
    78, 149, 84,  69, 134, 75,  59, 120, 66,  47, 103, 56,  34, 88, 46,  22, 73, 34,   1, 57, 20,   0, 35,  1,   0,  0,  0,
    80, 149, 69,  70, 133, 61,  60, 119, 53,  49, 103, 44,  36, 88, 35,  23, 73, 24,   2, 56, 10,   0, 35,  0,   0,  0,  0,
    58, 136, 200,  50, 122, 180,  45, 109, 162,  38, 94, 141,  27, 80, 121,  15, 65, 102,   0, 48, 81,   0, 26, 56,   0,  0, 19,
    66, 136, 182,  59, 122, 163,  52, 109, 146,  42, 94, 128,  32, 80, 109,  20, 65, 91,   2, 48, 72,   0, 26, 49,   0,  0, 11,
    70, 136, 165,  62, 122, 149,  55, 108, 133,  46, 94, 116,  35, 80, 99,  21, 65, 82,   4, 49, 64,   0, 26, 41,   0,  0,  3,
    76, 135, 149,  66, 121, 133,  58, 108, 119,  48, 94, 103,  36, 79, 88,  23, 65, 73,   7, 49, 56,   0, 27, 34,   0,  0,  0,
    78, 135, 133,  69, 120, 118,  60, 107, 106,  50, 93, 92,  39, 79, 77,  26, 65, 63,   8, 49, 47,   0, 27, 26,   0,  0,  0,
    82, 134, 117,  71, 120, 104,  62, 107, 92,  51, 93, 79,  39, 78, 66,  27, 64, 53,  10, 48, 39,   0, 27, 18,   0,  0,  0,
    84, 134, 99,  73, 119, 87,  64, 106, 77,  53, 92, 66,  42, 78, 55,  28, 64, 42,  11, 48, 29,   0, 28,  9,   0,  0,  0,
    87, 133, 81,  76, 119, 72,  66, 106, 62,  55, 92, 52,  43, 78, 42,  29, 64, 31,  12, 48, 17,   0, 28,  0,   0,  0,  0,
    88, 134, 67,  77, 119, 58,  68, 106, 51,  56, 92, 42,  44, 78, 32,  30, 64, 20,  12, 48,  6,   0, 28,  0,   0,  0,  0,
    73, 120, 189,  64, 107, 168,  57, 96, 151,  47, 82, 133,  38, 69, 114,  26, 55, 95,  11, 39, 75,   0, 16, 51,   0,  0, 14,
    78, 120, 171,  69, 107, 153,  62, 95, 137,  51, 82, 119,  40, 69, 102,  29, 55, 85,  15, 39, 66,   0, 17, 44,   0,  0,  4,
    81, 120, 156,  71, 107, 140,  64, 95, 125,  53, 82, 109,  42, 69, 93,  31, 55, 77,  16, 39, 59,   0, 18, 37,   0,  0,  0,
    85, 120, 141,  74, 107, 126,  65, 95, 112,  54, 82, 97,  43, 69, 82,  32, 55, 67,  17, 39, 51,   0, 19, 30,   0,  0,  0,
    86, 119, 126,  76, 106, 112,  66, 95, 100,  56, 81, 85,  45, 69, 72,  33, 55, 59,  18, 40, 43,   0, 19, 22,   0,  0,  0,
    89, 119, 110,  78, 106, 98,  69, 94, 87,  56, 81, 75,  46, 68, 62,  33, 55, 49,  18, 40, 35,   0, 20, 15,   0,  0,  0,
    89, 119, 95,  80, 106, 83,  70, 94, 73,  58, 81, 63,  46, 68, 51,  34, 55, 39,  19, 40, 25,   0, 20,  4,   0,  0,  0,
    92, 118, 78,  82, 106, 68,  70, 93, 59,  59, 81, 49,  47, 68, 39,  34, 55, 28,  19, 40, 14,   0, 20,  0,   0,  0,  0,
    93, 118, 65,  82, 105, 55,  72, 93, 48,  60, 81, 39,  47, 68, 29,  34, 55, 18,  20, 40,  2,   0, 20,  0,   0,  0,  0,
    83, 104, 177,  74, 93, 159,  65, 82, 142,  56, 70, 124,  45, 57, 106,  33, 44, 89,  20, 28, 70,   1,  4, 46,   0,  0,  8,
    86, 104, 161,  78, 93, 145,  68, 82, 128,  58, 70, 112,  48, 58, 96,  35, 45, 80,  21, 29, 62,   1,  6, 40,   0,  0,  0,
    89, 104, 147,  79, 93, 131,  69, 82, 118,  59, 70, 102,  47, 58, 87,  36, 45, 72,  23, 29, 55,   3,  7, 33,   0,  0,  0,
    90, 104, 132,  80, 93, 119,  71, 82, 106,  60, 70, 91,  48, 58, 77,  37, 45, 62,  23, 30, 47,   4,  7, 26,   0,  0,  0,
    92, 104, 118,  82, 93, 105,  72, 82, 93,  61, 69, 80,  50, 58, 68,  37, 45, 55,  23, 30, 39,   4,  8, 18,   0,  0,  0,
    94, 104, 105,  82, 92, 93,  72, 82, 82,  61, 70, 70,  50, 58, 58,  38, 46, 46,  23, 31, 31,   6, 10, 11,   0,  0,  0,
    95, 104, 90,  84, 92, 79,  74, 82, 70,  62, 70, 58,  50, 58, 48,  37, 46, 36,  23, 31, 22,   4, 11,  3,   0,  0,  0,
    96, 103, 74,  85, 92, 65,  75, 81, 56,  63, 70, 47,  50, 58, 37,  38, 46, 25,  24, 31, 11,   3, 11,  0,   0,  0,  0,
    97, 103, 62,  86, 92, 53,  76, 81, 45,  63, 69, 36,  51, 58, 27,  38, 46, 15,  23, 31,  0,   3, 11,  0,   0,  0,  0,
    90, 87, 165,  81, 77, 148,  72, 67, 132,  62, 55, 116,  50, 44, 99,  39, 31, 82,  25, 14, 64,   1,  0, 42,   0,  0,  1,
    93, 87, 150,  83, 77, 135,  74, 67, 121,  63, 56, 105,  52, 45, 90,  40, 32, 74,  25, 16, 57,   3,  0, 35,   0,  0,  0,
    95, 87, 138,  85, 77, 123,  75, 67, 109,  63, 57, 95,  53, 45, 81,  41, 33, 66,  26, 17, 50,   4,  0, 28,   0,  0,  0,
    95, 88, 124,  85, 77, 111,  75, 67, 99,  63, 57, 86,  53, 45, 72,  41, 33, 58,  26, 18, 43,   5,  1, 22,   0,  0,  0,
    97, 88, 112,  87, 77, 100,  76, 68, 88,  64, 57, 76,  53, 46, 63,  41, 34, 50,  26, 19, 35,   5,  2, 14,   0,  0,  0,
    99, 87, 99,  87, 78, 88,  76, 68, 77,  65, 57, 65,  53, 46, 54,  41, 35, 42,  27, 20, 27,   6,  2,  5,   0,  0,  0,
    100, 87, 85,  88, 77, 75,  77, 68, 65,  65, 57, 54,  53, 46, 44,  41, 35, 32,  27, 21, 19,   5,  3,  2,   0,  0,  0,
    100, 88, 71,  89, 77, 61,  78, 68, 53,  66, 57, 44,  53, 47, 33,  41, 35, 22,  27, 21,  7,   5,  3,  0,   0,  0,  0,
    101, 87, 60,  90, 77, 52,  79, 68, 44,  66, 58, 34,  53, 47, 25,  41, 35, 13,  26, 22,  0,   5,  3,  0,   0,  0,  0,
    97, 67, 155,  86, 58, 138,  77, 50, 125,  66, 39, 108,  55, 28, 92,  43, 12, 76,  29,  0, 59,   2,  0, 37,   0,  0,  0,
    99, 67, 141,  88, 59, 127,  78, 50, 113,  68, 40, 98,  56, 29, 83,  44, 15, 68,  29,  0, 52,   3,  0, 30,   0,  0,  0,
    100, 68, 129,  89, 59, 115,  80, 51, 103,  68, 41, 89,  56, 30, 75,  44, 16, 61,  30,  0, 45,   4,  0, 24,   0,  0,  0,
    100, 69, 118,  90, 60, 104,  80, 51, 92,  67, 41, 79,  56, 31, 66,  44, 18, 53,  29,  2, 38,   4,  0, 18,   0,  0,  0,
    101, 69, 104,  90, 61, 93,  79, 51, 82,  67, 42, 70,  56, 32, 59,  44, 20, 46,  29,  4, 31,   6,  0,  9,   0,  0,  0,
    102, 69, 93,  90, 61, 83,  80, 52, 72,  68, 42, 61,  56, 33, 50,  43, 20, 38,  29,  5, 23,   7,  0,  4,   0,  0,  0,
    102, 70, 80,  91, 61, 71,  80, 52, 61,  68, 43, 51,  56, 32, 40,  44, 21, 29,  30,  6, 14,   7,  0,  0,   0,  0,  0,
    103, 70, 68,  92, 61, 58,  81, 53, 50,  69, 43, 41,  56, 34, 31,  43, 22, 19,  29,  7,  3,   7,  0,  0,   0,  0,  0,
    104, 70, 57,  92, 61, 48,  82, 53, 40,  69, 43, 32,  56, 34, 22,  43, 23, 10,  29,  8,  0,   6,  0,  0,   0,  0,  0,
    101, 45, 145,  91, 35, 129,  80, 26, 116,  69, 15, 101,  59,  0, 86,  46,  0, 71,  31,  0, 55,   0,  0, 33,   0,  0,  0,
    104, 44, 132,  92, 36, 118,  82, 28, 105,  71, 17, 91,  58,  3, 77,  46,  0, 63,  31,  0, 48,   2,  0, 26,   0,  0,  0,
    104, 46, 121,  93, 37, 107,  82, 30, 96,  70, 20, 83,  58,  6, 70,  46,  0, 57,  32,  0, 41,   4,  0, 20,   0,  0,  0,
    104, 48, 110,  93, 40, 98,  82, 31, 87,  70, 22, 74,  59,  9, 62,  45,  0, 49,  31,  0, 35,   6,  0, 13,   0,  0,  0,
    104, 48, 99,  92, 41, 88,  82, 32, 77,  70, 23, 65,  58, 11, 54,  46,  0, 42,  32,  0, 27,   7,  0,  5,   0,  0,  0,
    105, 50, 88,  93, 41, 77,  82, 34, 68,  71, 24, 57,  58, 13, 46,  45,  1, 35,  31,  0, 21,   7,  0,  2,   0,  0,  0,
    105, 50, 76,  94, 41, 66,  83, 34, 57,  71, 25, 47,  58, 15, 37,  45,  2, 25,  32,  0, 11,   7,  0,  0,   0,  0,  0,
    106, 50, 64,  94, 42, 55,  83, 35, 47,  71, 26, 38,  58, 16, 27,  45,  4, 17,  31,  0,  4,   7,  0,  0,   0,  0,  0,
    106, 51, 54,  95, 42, 45,  83, 35, 38,  71, 27, 30,  58, 16, 19,  45,  5,  7,  30,  0,  0,   6,  0,  0,   0,  0,  0,
    0, 181, 240,   0, 162, 216,   0, 144, 193,   0, 126, 168,   0, 109, 146,   0, 91, 123,   0, 71, 98,   0, 48, 71,   0,  9, 34,
    0, 179, 218,   0, 161, 195,   0, 144, 174,   0, 126, 153,   0, 108, 132,   0, 90, 110,   0, 71, 88,   0, 48, 63,   0,  8, 29,
    0, 178, 197,   0, 159, 177,   0, 143, 159,   0, 125, 139,   0, 107, 119,   0, 90, 99,   0, 71, 79,   0, 48, 55,   0,  8, 22,
    0, 177, 177,   0, 158, 158,   0, 142, 141,   0, 124, 123,   0, 107, 106,   0, 89, 88,   0, 71, 70,   0, 48, 47,   0,  8, 14,
    0, 176, 157,   0, 158, 141,   0, 141, 126,   0, 123, 109,   0, 106, 93,   0, 89, 78,   0, 70, 60,   0, 47, 39,   0,  7,  5,
    0, 175, 138,   0, 157, 123,   0, 141, 110,   0, 123, 96,   0, 105, 81,   0, 88, 67,   0, 70, 51,   0, 48, 30,   0,  6,  0,
    0, 173, 115,   0, 155, 104,   0, 140, 92,   0, 122, 80,   0, 105, 67,   0, 88, 55,   0, 69, 40,   0, 47, 20,   0,  6,  0,
    0, 173, 94,   0, 155, 85,   0, 139, 75,   0, 121, 64,   0, 104, 53,   0, 88, 42,   0, 70, 28,   0, 47,  9,   0,  6,  0,
    0, 173, 76,   0, 155, 70,   0, 138, 61,   0, 122, 53,   0, 104, 44,   0, 87, 32,   0, 69, 18,   0, 47,  0,   0,  6,  0,
    0, 164, 226,   0, 147, 203,   0, 131, 181,   0, 114, 158,   0, 97, 136,   0, 80, 115,   0, 63, 92,   0, 40, 65,   0,  0, 30,
    0, 162, 205,   0, 145, 184,   0, 130, 164,   0, 114, 143,   0, 97, 124,   0, 81, 104,   0, 63, 83,   0, 40, 58,   0,  0, 23,
    0, 162, 187,   0, 145, 167,   0, 130, 150,   0, 113, 131,   0, 96, 112,   0, 80, 93,   0, 62, 74,   0, 40, 50,   0,  0, 16,
    0, 160, 167,   0, 144, 150,   0, 129, 134,   0, 112, 116,   0, 96, 100,   0, 80, 82,   0, 62, 65,   0, 40, 43,   0,  0,  7,
    0, 160, 148,   0, 143, 133,   0, 128, 118,   0, 111, 103,   0, 96, 88,   0, 80, 73,   0, 62, 56,   0, 40, 35,   0,  0,  0,
    0, 158, 130,   0, 142, 117,   0, 127, 104,   0, 111, 89,   0, 95, 76,   0, 79, 62,   0, 62, 46,   0, 40, 26,   0,  0,  0,
    0, 158, 111,   0, 141, 99,   0, 127, 88,   0, 111, 76,   0, 95, 63,   0, 79, 51,   0, 62, 37,   0, 40, 18,   0,  0,  0,
    0, 158, 91,   0, 141, 81,   0, 126, 72,   0, 110, 62,   0, 94, 50,   0, 79, 39,   0, 62, 25,   0, 40,  5,   0,  0,  0,
    0, 157, 74,   0, 141, 66,   0, 126, 59,   0, 110, 49,   0, 94, 40,   0, 78, 29,   0, 61, 15,   0, 40,  0,   0,  0,  0,
    0, 148, 214,   0, 133, 192,   0, 119, 171,   0, 103, 150,   0, 87, 129,   0, 72, 108,   0, 55, 86,   0, 32, 61,   0,  0, 25,
    0, 147, 193,   0, 132, 173,   0, 118, 155,   0, 103, 136,   0, 87, 116,   0, 72, 98,   0, 55, 78,   0, 32, 53,   0,  0, 17,
    0, 147, 176,   0, 132, 158,   0, 118, 142,   0, 102, 124,   0, 87, 106,   0, 72, 88,   0, 55, 69,   0, 33, 46,   0,  0,  9,
    0, 146, 159,   0, 131, 142,   0, 117, 127,   0, 102, 111,   0, 87, 95,   0, 71, 79,   0, 55, 61,   0, 33, 39,   0,  0,  2,
    0, 145, 140,   0, 130, 126,   0, 117, 112,   0, 101, 98,   0, 86, 83,   0, 71, 68,   0, 55, 52,   0, 33, 31,   0,  0,  0,
    0, 144, 124,   0, 130, 111,   0, 116, 99,   0, 101, 84,   0, 86, 72,   0, 71, 59,   0, 55, 43,   0, 33, 23,   0,  0,  0,
    0, 144, 106,   0, 129, 94,   0, 115, 83,   0, 101, 72,   0, 85, 60,   0, 71, 48,   0, 55, 34,   0, 33, 14,   0,  0,  0,
    3, 143, 86,   0, 129, 77,   0, 115, 68,   0, 100, 58,   0, 85, 48,   0, 70, 36,   0, 54, 22,   0, 33,  3,   0,  0,  0,
    18, 143, 72,  13, 128, 63,   0, 115, 57,   0, 100, 47,   0, 85, 37,   0, 70, 26,   0, 54, 13,   0, 33,  0,   0,  0,  0,
    0, 132, 200,   0, 118, 179,   0, 105, 161,   0, 91, 140,   0, 76, 121,   0, 62, 101,   0, 46, 81,   0, 24, 56,   0,  0, 19,
    0, 131, 182,   0, 118, 163,   0, 105, 146,   0, 91, 128,   0, 77, 110,   0, 62, 91,   0, 46, 72,   0, 25, 48,   0,  0, 11,
    0, 131, 165,   0, 117, 149,   0, 104, 133,   0, 91, 116,   0, 77, 99,   0, 62, 82,   0, 46, 64,   0, 25, 41,   0,  0,  4,
    0, 131, 149,   0, 116, 134,   0, 104, 119,   0, 91, 104,   0, 77, 89,   0, 62, 73,   0, 46, 56,   0, 25, 34,   0,  0,  0,
    10, 130, 133,   2, 116, 119,   0, 104, 106,   0, 90, 91,   0, 76, 78,   0, 62, 64,   0, 46, 48,   0, 26, 27,   0,  0,  0,
    23, 130, 118,  20, 116, 104,  13, 103, 93,   3, 89, 79,   0, 76, 67,   0, 62, 54,   0, 46, 39,   0, 26, 19,   0,  0,  0,
    33, 129, 101,  27, 115, 89,  19, 103, 79,   9, 89, 67,   0, 75, 56,   0, 61, 43,   0, 46, 29,   0, 26, 10,   0,  0,  0,
    41, 128, 83,  35, 115, 73,  27, 102, 64,  15, 89, 55,   0, 76, 45,   0, 62, 33,   0, 46, 18,   0, 26,  0,   0,  0,  0,
    43, 129, 69,  38, 115, 61,  30, 102, 54,  17, 89, 45,   2, 75, 34,   0, 61, 23,   0, 46,  9,   0, 26,  0,   0,  0,  0,
    1, 116, 188,   1, 104, 168,   0, 92, 151,   0, 79, 132,   0, 66, 113,   0, 52, 94,   0, 36, 75,   0, 14, 52,   0,  0, 14,
    17, 116, 171,  16, 104, 153,  14, 92, 137,   8, 79, 119,   0, 67, 102,   0, 53, 85,   0, 37, 67,   0, 16, 44,   0,  0,  4,
    31, 116, 155,  27, 104, 140,  21, 92, 125,  13, 79, 109,   3, 66, 93,   0, 53, 77,   0, 37, 59,   0, 16, 38,   0,  0,  0,
    37, 115, 141,  30, 103, 126,  26, 92, 112,  16, 79, 98,   5, 66, 83,   0, 53, 67,   0, 38, 51,   0, 17, 31,   0,  0,  0,
    41, 115, 126,  37, 103, 112,  31, 92, 100,  22, 79, 86,  10, 66, 72,   0, 53, 59,   0, 38, 44,   0, 17, 23,   0,  0,  0,
    48, 115, 111,  41, 102, 99,  34, 91, 88,  24, 78, 76,  14, 66, 63,   0, 53, 50,   0, 38, 36,   0, 18, 15,   0,  0,  0,
    51, 115, 95,  46, 102, 85,  37, 91, 74,  26, 78, 63,  16, 66, 52,   0, 53, 40,   0, 38, 26,   0, 18,  5,   0,  0,  0,
    55, 114, 80,  47, 102, 69,  40, 90, 60,  30, 78, 51,  19, 66, 41,   3, 53, 29,   0, 38, 15,   0, 17,  0,   0,  0,  0,
    56, 114, 66,  50, 102, 58,  40, 91, 50,  32, 78, 41,  18, 66, 32,   4, 53, 21,   0, 38,  5,   0, 17,  0,   0,  0,  0,
    39, 102, 178,  37, 90, 159,  30, 79, 142,  21, 68, 124,  14, 55, 106,   0, 42, 89,   0, 26, 70,   0,  4, 46,   0,  0,  8,
    48, 102, 161,  42, 90, 145,  35, 79, 128,  26, 68, 112,  19, 55, 96,   3, 43, 79,   0, 27, 62,   0,  6, 40,   0,  0,  0,
    50, 102, 147,  44, 90, 132,  37, 79, 118,  30, 68, 102,  20, 56, 87,   7, 43, 72,   0, 28, 55,   0,  6, 34,   0,  0,  0,
    53, 101, 133,  47, 90, 118,  41, 79, 106,  32, 68, 91,  21, 56, 78,   9, 43, 63,   0, 28, 47,   0,  6, 26,   0,  0,  0,
    57, 101, 119,  50, 89, 106,  42, 79, 94,  34, 67, 81,  24, 56, 68,   9, 44, 55,   0, 29, 40,   0,  6, 19,   0,  0,  0,
    60, 100, 105,  50, 90, 94,  45, 80, 83,  36, 68, 71,  24, 56, 59,  11, 44, 46,   0, 29, 32,   0,  7, 12,   0,  0,  0,
    63, 101, 91,  55, 90, 80,  46, 79, 70,  37, 68, 59,  26, 56, 49,  12, 44, 37,   1, 29, 23,   0,  7,  3,   0,  0,  0,
    64, 101, 75,  56, 89, 67,  48, 79, 57,  37, 68, 48,  27, 56, 37,  15, 44, 26,   0, 29, 12,   0,  7,  0,   0,  0,  0,
    66, 101, 64,  58, 89, 55,  49, 79, 47,  39, 68, 38,  27, 56, 29,  14, 44, 18,   1, 30,  2,   0,  7,  0,   0,  0,  0,
    57, 86, 165,  51, 75, 148,  45, 65, 133,  38, 54, 116,  28, 43, 100,  16, 29, 83,   0, 13, 64,   0,  0, 42,   0,  0,  3,
    60, 86, 151,  55, 75, 135,  47, 66, 121,  39, 55, 105,  30, 44, 90,  18, 31, 74,   3, 16, 57,   0,  1, 35,   0,  0,  0,
    62, 86, 139,  56, 75, 123,  49, 66, 110,  40, 55, 95,  30, 44, 81,  19, 31, 66,   4, 17, 51,   0,  1, 29,   0,  0,  0,
    65, 86, 125,  56, 76, 112,  49, 66, 99,  39, 55, 86,  31, 44, 72,  19, 32, 59,   5, 18, 44,   0,  1, 23,   0,  0,  0,
    67, 86, 113,  58, 75, 100,  51, 66, 88,  41, 56, 77,  31, 45, 64,  20, 32, 51,   6, 18, 35,   0,  1, 14,   0,  0,  0,
    69, 86, 99,  61, 76, 88,  52, 66, 78,  43, 56, 66,  32, 45, 55,  20, 33, 42,   7, 19, 27,   0,  1,  6,   0,  0,  0,
    69, 86, 86,  61, 76, 75,  53, 67, 66,  43, 56, 55,  33, 45, 45,  21, 34, 34,   8, 20, 20,   0,  2,  2,   0,  0,  0,
    71, 86, 72,  63, 75, 62,  54, 66, 55,  45, 56, 45,  33, 45, 35,  22, 34, 23,   7, 20,  8,   0,  2,  0,   0,  0,  0,
    71, 86, 62,  64, 75, 53,  55, 66, 46,  45, 56, 36,  33, 46, 27,  22, 34, 15,   8, 20,  0,   0,  2,  0,   0,  0,  0,
    69, 67, 156,  61, 58, 140,  53, 50, 125,  45, 39, 108,  35, 28, 93,  25, 12, 77,  12,  0, 59,   0,  0, 37,   0,  0,  0,
    71, 68, 142,  63, 59, 126,  56, 50, 114,  47, 40, 98,  37, 28, 84,  26, 15, 68,  12,  0, 53,   0,  0, 30,   0,  0,  0,
    72, 68, 130,  63, 59, 116,  56, 50, 104,  47, 40, 90,  38, 30, 75,  27, 16, 61,  13,  0, 46,   0,  0, 24,   0,  0,  0,
    73, 69, 118,  65, 59, 105,  57, 51, 92,  47, 41, 80,  37, 30, 67,  26, 18, 53,  14,  1, 39,   0,  0, 18,   0,  0,  0,
    74, 69, 106,  65, 60, 93,  57, 51, 82,  48, 41, 70,  38, 31, 59,  26, 19, 46,  13,  2, 32,   0,  0, 10,   0,  0,  0,
    76, 69, 95,  66, 61, 84,  58, 52, 73,  48, 42, 61,  37, 32, 50,  26, 20, 38,  14,  4, 24,   0,  0,  4,   0,  0,  0,
    76, 69, 81,  68, 60, 72,  58, 52, 62,  48, 42, 51,  38, 32, 41,  27, 21, 30,  14,  4, 16,   0,  0,  1,   0,  0,  0,
    76, 69, 68,  68, 61, 60,  60, 52, 51,  49, 43, 41,  38, 33, 32,  27, 21, 20,  14,  5,  5,   0,  0,  0,   0,  0,  0,
    78, 70, 59,  69, 61, 50,  60, 52, 42,  49, 43, 34,  39, 33, 24,  27, 22, 13,  14,  7,  1,   0,  0,  0,   0,  0,  0,
    75, 46, 146,  68, 38, 131,  60, 30, 117,  50, 19, 102,  41,  4, 87,  29,  0, 72,  13,  0, 55,   0,  0, 33,   0,  0,  0,
    78, 47, 132,  70, 39, 119,  61, 30, 105,  53, 20, 92,  42,  5, 78,  30,  0, 64,  13,  0, 49,   0,  0, 27,   0,  0,  0,
    79, 48, 122,  70, 40, 108,  62, 32, 96,  52, 22, 84,  42,  9, 71,  30,  0, 58,  14,  0, 42,   0,  0, 20,   0,  0,  0,
    79, 50, 111,  70, 42, 99,  62, 33, 88,  52, 23, 74,  41, 11, 63,  29,  0, 50,  14,  0, 36,   0,  0, 14,   0,  0,  0,
    80, 50, 99,  70, 42, 89,  61, 34, 78,  52, 25, 67,  41, 14, 55,  30,  0, 42,  15,  0, 28,   0,  0,  6,   0,  0,  0,
    81, 51, 89,  71, 43, 78,  62, 35, 69,  52, 25, 58,  42, 15, 47,  30,  3, 36,  15,  0, 22,   0,  0,  3,   0,  0,  0,
    81, 51, 77,  71, 44, 68,  63, 36, 59,  53, 26, 49,  41, 16, 38,  31,  4, 27,  16,  0, 12,   0,  0,  0,   0,  0,  0,
    81, 52, 65,  72, 43, 56,  63, 36, 48,  53, 27, 39,  41, 17, 29,  30,  4, 18,  14,  0,  3,   0,  0,  0,   0,  0,  0,
    81, 52, 55,  73, 44, 47,  64, 36, 39,  53, 28, 32,  42, 18, 21,  31,  6,  9,  14,  0,  0,   0,  0,  0,   0,  0,  0,
    0, 174, 239,   0, 156, 214,   0, 139, 192,   0, 121, 168,   0, 105, 145,   0, 87, 123,   0, 68, 98,   0, 46, 70,   0,  3, 35,
    0, 172, 217,   0, 155, 194,   0, 139, 173,   0, 121, 152,   0, 104, 130,   0, 87, 110,   0, 69, 88,   0, 46, 63,   0,  4, 28,
    0, 171, 197,   0, 153, 175,   0, 138, 158,   0, 121, 139,   0, 103, 118,   0, 86, 100,   0, 68, 79,   0, 46, 55,   0,  4, 22,
    0, 170, 177,   0, 152, 158,   0, 136, 141,   0, 119, 124,   0, 103, 106,   0, 86, 88,   0, 68, 70,   0, 45, 47,   0,  3, 14,
    0, 169, 157,   0, 152, 141,   0, 136, 126,   0, 119, 109,   0, 102, 94,   0, 86, 78,   0, 68, 60,   0, 46, 39,   0,  3,  5,
    0, 167, 138,   0, 150, 124,   0, 135, 111,   0, 118, 97,   0, 102, 82,   0, 85, 68,   0, 68, 52,   0, 46, 31,   0,  3,  0,
    0, 167, 118,   0, 150, 104,   0, 135, 94,   0, 118, 81,   0, 101, 69,   0, 84, 56,   0, 67, 41,   0, 45, 21,   0,  3,  0,
    0, 166, 97,   0, 149, 87,   0, 134, 77,   0, 117, 67,   0, 101, 56,   0, 85, 44,   0, 67, 30,   0, 45, 10,   0,  3,  0,
    0, 165, 79,   0, 149, 73,   0, 133, 64,   0, 117, 56,   0, 101, 46,   0, 85, 34,   0, 68, 21,   0, 46,  1,   0,  3,  0,
    0, 158, 225,   0, 141, 201,   0, 126, 180,   0, 109, 158,   0, 94, 136,   0, 78, 114,   0, 60, 91,   0, 38, 66,   0,  0, 30,
    0, 156, 203,   0, 140, 183,   0, 125, 164,   0, 109, 143,   0, 94, 124,   0, 78, 104,   0, 61, 83,   0, 38, 57,   0,  0, 23,
    0, 156, 186,   0, 140, 166,   0, 125, 150,   0, 109, 130,   0, 93, 111,   0, 77, 93,   0, 60, 74,   0, 38, 50,   0,  0, 17,
    0, 155, 167,   0, 138, 149,   0, 124, 134,   0, 109, 117,   0, 93, 100,   0, 76, 83,   0, 60, 65,   0, 38, 43,   0,  0,  9,
    0, 153, 147,   0, 138, 134,   0, 124, 120,   0, 107, 103,   0, 92, 88,   0, 77, 73,   0, 60, 56,   0, 38, 35,   0,  0,  0,
    0, 153, 131,   0, 137, 118,   0, 122, 105,   0, 107, 90,   0, 91, 76,   0, 76, 63,   0, 60, 47,   0, 39, 28,   0,  0,  0,
    0, 153, 111,   0, 136, 100,   0, 123, 90,   0, 107, 77,   0, 92, 65,   0, 76, 52,   0, 60, 37,   0, 38, 18,   0,  0,  0,
    0, 152, 93,   0, 136, 82,   0, 122, 74,   0, 106, 63,   0, 91, 52,   0, 76, 40,   0, 59, 26,   0, 38,  6,   0,  0,  0,
    0, 151, 78,   0, 136, 69,   0, 121, 61,   0, 106, 52,   0, 91, 43,   0, 76, 32,   0, 59, 17,   0, 38,  0,   0,  0,  0,
    0, 143, 213,   0, 128, 191,   0, 115, 171,   0, 100, 149,   0, 84, 128,   0, 69, 108,   0, 52, 86,   0, 30, 61,   0,  0, 25,
    0, 142, 193,   0, 127, 173,   0, 114, 154,   0, 99, 134,   0, 84, 116,   0, 69, 98,   0, 52, 77,   0, 31, 53,   0,  0, 18,
    0, 141, 176,   0, 127, 158,   0, 114, 141,   0, 98, 122,   0, 84, 105,   0, 69, 88,   0, 53, 69,   0, 31, 46,   0,  0,  9,
    0, 141, 159,   0, 126, 142,   0, 113, 127,   0, 98, 110,   0, 83, 95,   0, 69, 78,   0, 53, 60,   0, 32, 39,   0,  0,  2,
    0, 140, 140,   0, 126, 126,   0, 112, 112,   0, 98, 98,   0, 83, 83,   0, 68, 69,   0, 52, 52,   0, 31, 31,   0,  0,  0,
    0, 140, 124,   0, 125, 112,   0, 112, 100,   0, 97, 86,   0, 83, 72,   0, 68, 59,   0, 52, 44,   0, 31, 23,   0,  0,  0,
    0, 139, 106,   0, 125, 96,   0, 111, 85,   0, 97, 72,   0, 83, 62,   0, 68, 49,   0, 52, 35,   0, 31, 15,   0,  0,  0,
    0, 138, 88,   0, 124, 79,   0, 111, 70,   0, 96, 59,   0, 82, 48,   0, 68, 38,   0, 52, 24,   0, 31,  4,   0,  0,  0,
    0, 139, 76,   0, 124, 66,   0, 111, 58,   0, 96, 50,   0, 82, 40,   0, 68, 29,   0, 52, 15,   0, 31,  0,   0,  0,  0,
    0, 129, 200,   0, 114, 179,   0, 102, 160,   0, 87, 139,   0, 74, 120,   0, 60, 101,   0, 44, 81,   0, 22, 56,   0,  0, 19,
    0, 127, 181,   0, 114, 163,   0, 102, 146,   0, 88, 127,   0, 74, 109,   0, 60, 91,   0, 44, 72,   0, 23, 48,   0,  0, 11,
    0, 127, 166,   0, 113, 148,   0, 101, 133,   0, 87, 115,   0, 74, 99,   0, 60, 82,   0, 44, 64,   0, 23, 42,   0,  0,  4,
    0, 127, 150,   0, 113, 134,   0, 101, 119,   0, 87, 104,   0, 74, 89,   0, 60, 73,   0, 44, 56,   0, 23, 35,   0,  0,  0,
    0, 125, 134,   0, 112, 118,   0, 100, 106,   0, 87, 92,   0, 73, 78,   0, 60, 64,   0, 44, 48,   0, 23, 27,   0,  0,  0,
    0, 125, 118,   0, 112, 105,   0, 100, 94,   0, 86, 80,   0, 73, 68,   0, 60, 54,   0, 44, 39,   0, 23, 20,   0,  0,  0,
    0, 125, 101,   0, 111, 90,   0, 99, 80,   0, 86, 69,   0, 73, 58,   0, 59, 45,   0, 44, 30,   0, 23, 11,   0,  0,  0,
    0, 124, 85,   0, 111, 75,   0, 99, 66,   0, 86, 56,   0, 73, 45,   0, 59, 34,   0, 44, 20,   0, 23,  1,   0,  0,  0,
    0, 125, 72,   0, 111, 62,   0, 99, 56,   0, 86, 46,   0, 73, 36,   0, 60, 26,   0, 44, 12,   0, 23,  0,   0,  0,  0,
    0, 114, 188,   0, 101, 167,   0, 89, 150,   0, 77, 131,   0, 64, 113,   0, 50, 95,   0, 34, 75,   0, 12, 52,   0,  0, 14,
    0, 113, 170,   0, 101, 153,   0, 89, 137,   0, 77, 120,   0, 64, 102,   0, 50, 85,   0, 35, 67,   0, 12, 44,   0,  0,  4,
    0, 113, 156,   0, 100, 139,   0, 89, 125,   0, 77, 109,   0, 64, 92,   0, 51, 77,   0, 35, 60,   0, 12, 38,   0,  0,  0,
    0, 112, 141,   0, 100, 126,   0, 89, 113,   0, 77, 98,   0, 64, 83,   0, 51, 68,   0, 35, 51,   0, 12, 30,   0,  0,  0,
    0, 112, 127,   0, 100, 112,   0, 89, 100,   0, 76, 87,   0, 64, 74,   0, 51, 59,   0, 35, 44,   0, 13, 24,   0,  0,  0,
    0, 112, 111,   0, 100, 100,   0, 88, 88,   0, 76, 76,   0, 64, 64,   0, 51, 52,   0, 36, 37,   0, 13, 17,   0,  0,  0,
    0, 111, 96,   0, 99, 85,   0, 88, 76,   0, 76, 64,   0, 64, 53,   0, 51, 41,   0, 36, 27,   0, 13,  6,   0,  0,  0,
    0, 111, 81,   0, 99, 71,   0, 88, 62,   0, 76, 52,   0, 64, 43,   0, 51, 31,   0, 36, 17,   0, 13,  0,   0,  0,  0,
    0, 111, 69,   0, 99, 60,   0, 88, 52,   0, 75, 43,   0, 63, 34,   0, 51, 21,   0, 36,  7,   0, 13,  0,   0,  0,  0,
    0, 99, 177,   0, 88, 158,   0, 77, 141,   0, 66, 123,   0, 53, 106,   0, 40, 89,   0, 25, 71,   0,  5, 47,   0,  0,  8,
    0, 99, 160,   0, 88, 144,   0, 77, 129,   0, 66, 112,   0, 54, 97,   0, 41, 80,   0, 26, 62,   0,  5, 40,   0,  0,  0,
    0, 99, 147,   0, 87, 132,   0, 78, 117,   0, 66, 102,   0, 54, 87,   0, 42, 72,   0, 26, 55,   0,  5, 34,   0,  0,  0,
    0, 99, 134,   0, 88, 119,   0, 77, 107,   0, 66, 92,   0, 54, 78,   0, 42, 64,   0, 27, 48,   0,  5, 27,   0,  0,  0,
    0, 99, 120,   0, 87, 107,   0, 78, 94,   0, 66, 81,   0, 54, 68,   0, 42, 55,   0, 27, 40,   0,  6, 20,   0,  0,  0,
    0, 98, 105,   0, 87, 94,   0, 77, 84,   0, 65, 71,   0, 55, 59,   0, 42, 47,   0, 28, 33,   0,  6, 12,   0,  0,  0,
    0, 98, 93,   0, 87, 81,   0, 77, 72,   0, 66, 61,   0, 54, 49,   0, 42, 37,   0, 28, 24,   0,  6,  4,   0,  0,  0,
    0, 98, 77,   0, 87, 68,   0, 77, 59,   0, 65, 49,   0, 54, 39,   0, 42, 27,   0, 29, 14,   0,  6,  0,   0,  0,  0,
    1, 98, 65,   7, 87, 56,   0, 77, 49,   0, 66, 41,   0, 54, 30,   0, 42, 19,   0, 29,  3,   0,  6,  0,   0,  0,  0,
    0, 84, 166,   0, 74, 149,   0, 64, 134,   0, 53, 117,   0, 41, 100,   0, 28, 83,   0, 11, 64,   0,  0, 42,   0,  0,  3,
    0, 84, 151,   0, 74, 135,   0, 64, 121,   0, 53, 105,   0, 42, 90,   0, 30, 75,   0, 14, 58,   0,  0, 36,   0,  0,  0,
    0, 84, 138,   0, 74, 124,   1, 64, 110,   0, 54, 95,   0, 43, 81,   0, 30, 67,   0, 15, 51,   0,  1, 29,   0,  0,  0,
    14, 84, 126,  12, 74, 112,   2, 65, 99,   0, 54, 85,   0, 44, 73,   0, 31, 59,   0, 16, 44,   0,  1, 23,   0,  0,  0,
    16, 84, 113,  13, 74, 100,   6, 65, 89,   0, 54, 77,   0, 44, 65,   0, 31, 51,   0, 17, 36,   0,  1, 16,   0,  0,  0,
    24, 84, 100,  18, 74, 88,  13, 65, 78,   2, 55, 68,   0, 44, 55,   0, 32, 43,   0, 18, 28,   0,  1,  6,   0,  0,  0,
    26, 84, 87,  24, 74, 76,  17, 65, 67,   7, 54, 57,   0, 44, 46,   0, 32, 35,   0, 19, 21,   0,  2,  3,   0,  0,  0,
    30, 84, 74,  28, 74, 64,  20, 65, 55,  12, 55, 46,   0, 44, 35,   0, 32, 24,   0, 18,  9,   0,  1,  0,   0,  0,  0,
    32, 84, 63,  28, 74, 54,  21, 65, 47,  13, 54, 38,   0, 44, 28,   0, 32, 16,   0, 18,  1,   0,  1,  0,   0,  0,  0,
    30, 67, 155,  20, 58, 139,  20, 49, 126,  12, 39, 110,   0, 27, 94,   0, 13, 77,   0,  0, 60,   0,  0, 37,   0,  0,  0,
    35, 67, 142,  30, 58, 126,  23, 50, 114,  16, 40, 99,   7, 29, 85,   0, 15, 69,   0,  0, 52,   0,  0, 30,   0,  0,  0,
    35, 68, 131,  30, 59, 116,  27, 50, 104,  18, 40, 90,   9, 29, 76,   0, 17, 62,   0,  2, 46,   0,  0, 24,   0,  0,  0,
    37, 69, 119,  33, 59, 106,  27, 51, 94,  21, 41, 80,   9, 30, 67,   0, 18, 54,   0,  3, 39,   0,  0, 18,   0,  0,  0,
    40, 69, 107,  36, 59, 94,  28, 51, 84,  18, 41, 72,  10, 31, 60,   0, 19, 47,   0,  4, 32,   0,  0, 10,   0,  0,  0,
    42, 69, 95,  36, 59, 84,  29, 51, 74,  19, 41, 63,  10, 31, 52,   0, 20, 39,   0,  4, 25,   0,  0,  4,   0,  0,  0,
    43, 69, 83,  38, 60, 73,  32, 51, 62,  23, 42, 53,  11, 31, 42,   0, 20, 31,   0,  5, 17,   0,  0,  1,   0,  0,  0,
    45, 69, 70,  39, 60, 60,  33, 51, 52,  24, 42, 43,  13, 32, 33,   0, 21, 21,   0,  5,  6,   0,  0,  0,   0,  0,  0,
    47, 69, 59,  41, 60, 51,  34, 51, 43,  24, 42, 35,  12, 33, 26,   1, 22, 14,   0,  5,  1,   0,  0,  0,   0,  0,  0,
    46, 48, 146,  42, 40, 131,  36, 32, 118,  27, 22, 103,  17,  6, 88,   5,  0, 73,   0,  0, 55,   0,  0, 33,   0,  0,  0,
    48, 48, 133,  44, 40, 119,  37, 32, 107,  28, 22, 93,  20,  8, 79,   7,  0, 65,   0,  0, 49,   0,  0, 27,   0,  0,  0,
    48, 50, 123,  44, 41, 109,  37, 33, 97,  30, 23, 83,  21, 11, 71,   8,  0, 58,   0,  0, 42,   0,  0, 21,   0,  0,  0,
    49, 51, 111,  45, 42, 99,  38, 34, 87,  29, 25, 75,  20, 13, 63,   8,  0, 51,   0,  0, 36,   0,  0, 14,   0,  0,  0,
    52, 52, 100,  44, 43, 89,  38, 35, 79,  29, 26, 68,  19, 15, 56,  10,  1, 43,   0,  0, 28,   0,  0,  6,   0,  0,  0,
    52, 52, 90,  47, 44, 79,  39, 36, 70,  30, 27, 59,  20, 16, 47,   9,  2, 36,   0,  0, 22,   0,  0,  2,   0,  0,  0,
    52, 53, 78,  46, 44, 68,  39, 37, 60,  32, 27, 49,  22, 17, 39,  10,  3, 28,   0,  0, 12,   0,  0,  0,   0,  0,  0,
    53, 53, 66,  47, 44, 57,  40, 36, 48,  32, 27, 39,  22, 18, 30,   9,  4, 18,   0,  0,  3,   0,  0,  0,   0,  0,  0,
    54, 53, 57,  48, 45, 49,  41, 37, 41,  33, 28, 32,  22, 19, 23,  11,  6, 10,   1,  0,  0,   0,  0,  0,   0,  0,  0,
};
void AdobeCMYK_to_sRGB1(uint8_t c, uint8_t m, uint8_t y, uint8_t k, uint8_t& R, uint8_t& G, uint8_t& B)
{
    int fix_c = c << 8;
    int fix_m = m << 8;
    int fix_y = y << 8;
    int fix_k = k << 8;
    int c_index = (fix_c + 4096) >> 13;
    int m_index = (fix_m + 4096) >> 13;
    int y_index = (fix_y + 4096) >> 13;
    int k_index = (fix_k + 4096) >> 13;
    int pos = (c_index * 9 * 9 * 9 + m_index * 9 * 9 + y_index * 9 + k_index) * 3;
    int fix_r = g_CMYKSamples[pos] << 8;
    int fix_g = g_CMYKSamples[pos + 1] << 8;
    int fix_b = g_CMYKSamples[pos + 2] << 8;
    int c1_index = fix_c >> 13;
    if (c1_index == c_index) {
        c1_index = c1_index == 8 ? c1_index - 1 : c1_index + 1;
    }
    int m1_index = fix_m >> 13;
    if (m1_index == m_index) {
        m1_index = m1_index == 8 ? m1_index - 1 : m1_index + 1;
    }
    int y1_index = fix_y >> 13;
    if (y1_index == y_index) {
        y1_index = y1_index == 8 ? y1_index - 1 : y1_index + 1;
    }
    int k1_index = fix_k >> 13;
    if (k1_index == k_index) {
        k1_index = k1_index == 8 ? k1_index - 1 : k1_index + 1;
    }
    int c1_pos = pos + (c1_index - c_index) * 9 * 9 * 9 * 3;
    int m1_pos = pos + (m1_index - m_index) * 9 * 9 * 3;
    int y1_pos = pos + (y1_index - y_index) * 9 * 3;
    int k1_pos = pos + (k1_index - k_index) * 3;
    int c_r_delta = g_CMYKSamples[pos] - g_CMYKSamples[c1_pos];
    int c_g_delta = g_CMYKSamples[pos + 1] - g_CMYKSamples[c1_pos + 1];
    int c_b_delta = g_CMYKSamples[pos + 2] - g_CMYKSamples[c1_pos + 2];
    int m_r_delta = g_CMYKSamples[pos] - g_CMYKSamples[m1_pos];
    int m_g_delta = g_CMYKSamples[pos + 1] - g_CMYKSamples[m1_pos + 1];
    int m_b_delta = g_CMYKSamples[pos + 2] - g_CMYKSamples[m1_pos + 2];
    int y_r_delta = g_CMYKSamples[pos] - g_CMYKSamples[y1_pos];
    int y_g_delta = g_CMYKSamples[pos + 1] - g_CMYKSamples[y1_pos + 1];
    int y_b_delta = g_CMYKSamples[pos + 2] - g_CMYKSamples[y1_pos + 2];
    int k_r_delta = g_CMYKSamples[pos] - g_CMYKSamples[k1_pos];
    int k_g_delta = g_CMYKSamples[pos + 1] - g_CMYKSamples[k1_pos + 1];
    int k_b_delta = g_CMYKSamples[pos + 2] - g_CMYKSamples[k1_pos + 2];
    int c_rate = (fix_c - (c_index << 13)) * (c_index - c1_index);
    fix_r += c_r_delta * c_rate / 32;
    fix_g += c_g_delta * c_rate / 32;
    fix_b += c_b_delta * c_rate / 32;
    int m_rate = (fix_m - (m_index << 13)) * (m_index - m1_index);
    fix_r += m_r_delta * m_rate / 32;
    fix_g += m_g_delta * m_rate / 32;
    fix_b += m_b_delta * m_rate / 32;
    int y_rate = (fix_y - (y_index << 13)) * (y_index - y1_index);
    fix_r += y_r_delta * y_rate / 32;
    fix_g += y_g_delta * y_rate / 32;
    fix_b += y_b_delta * y_rate / 32;
    int k_rate = (fix_k - (k_index << 13)) * (k_index - k1_index);
    fix_r += k_r_delta * k_rate / 32;
    fix_g += k_g_delta * k_rate / 32;
    fix_b += k_b_delta * k_rate / 32;
    if (fix_r < 0) {
        fix_r = 0;
    }
    if (fix_g < 0) {
        fix_g = 0;
    }
    if (fix_b < 0) {
        fix_b = 0;
    }
    R = fix_r >> 8;
    G = fix_g >> 8;
    B = fix_b >> 8;
}
void AdobeCMYK_to_sRGB(FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B)
{
    uint8_t c1 = FXSYS_round(c * 255);
    uint8_t m1 = FXSYS_round(m * 255);
    uint8_t y1 = FXSYS_round(y * 255);
    uint8_t k1 = FXSYS_round(k * 255);
    uint8_t r, g, b;
    AdobeCMYK_to_sRGB1(c1, m1, y1, k1, r, g, b);
    R = 1.0f * r / 255;
    G = 1.0f * g / 255;
    B = 1.0f * b / 255;
}
