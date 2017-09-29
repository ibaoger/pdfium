// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PARSER_CPDF_CRYPTO_HANDLER_H_
#define CORE_FPDFAPI_PARSER_CPDF_CRYPTO_HANDLER_H_

#include <memory>

#include "core/fdrm/crypto/fx_crypt.h"
#include "core/fxcrt/cfx_binarybuf.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"

class CPDF_Dictionary;
class CPDF_Object;
class CPDF_SecurityHandler;

class CPDF_CryptoHandler : public Retainable {
 public:
  template <typename T, typename... Args>
  friend RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  static bool IsSignatureDictionary(const CPDF_Dictionary* dictionary);

  bool Init(CPDF_Dictionary* pEncryptDict,
            CPDF_SecurityHandler* pSecurityHandler);

  std::unique_ptr<CPDF_Object> DecryptObjectTree(
      std::unique_ptr<CPDF_Object> object);

  uint32_t EncryptGetSize(uint32_t objnum,
                          uint32_t version,
                          const uint8_t* src_buf,
                          uint32_t src_size);
  bool EncryptContent(uint32_t objnum,
                      uint32_t version,
                      const uint8_t* src_buf,
                      uint32_t src_size,
                      uint8_t* dest_buf,
                      uint32_t& dest_size);

  bool Init(int cipher, const uint8_t* key, int keylen);
  bool IsCipherAES() const;

 private:
  CPDF_CryptoHandler();
  ~CPDF_CryptoHandler() override;

  uint32_t DecryptGetSize(uint32_t src_size);
  void* DecryptStart(uint32_t objnum, uint32_t gennum);
  ByteString Decrypt(uint32_t objnum, uint32_t gennum, const ByteString& str);
  bool DecryptStream(void* context,
                     const uint8_t* src_buf,
                     uint32_t src_size,
                     CFX_BinaryBuf& dest_buf);
  bool DecryptFinish(void* context, CFX_BinaryBuf& dest_buf);

  void PopulateKey(uint32_t objnum, uint32_t gennum, uint8_t* key);
  void CryptBlock(bool bEncrypt,
                  uint32_t objnum,
                  uint32_t gennum,
                  const uint8_t* src_buf,
                  uint32_t src_size,
                  uint8_t* dest_buf,
                  uint32_t& dest_size);
  void* CryptStart(uint32_t objnum, uint32_t gennum, bool bEncrypt);
  bool CryptStream(void* context,
                   const uint8_t* src_buf,
                   uint32_t src_size,
                   CFX_BinaryBuf& dest_buf,
                   bool bEncrypt);
  bool CryptFinish(void* context, CFX_BinaryBuf& dest_buf, bool bEncrypt);

  uint8_t m_EncryptKey[32];
  int m_KeyLen;
  int m_Cipher;
  std::unique_ptr<CRYPT_aes_context, FxFreeDeleter> m_pAESContext;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_CRYPTO_HANDLER_H_
