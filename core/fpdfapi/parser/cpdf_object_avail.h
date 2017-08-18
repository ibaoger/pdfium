// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PARSER_CPDF_OBJECT_AVAIL_H_
#define CORE_FPDFAPI_PARSER_CPDF_OBJECT_AVAIL_H_

#include <memory>
#include <set>
#include <vector>

#include "core/fpdfapi/parser/cpdf_data_avail.h"
#include "core/fxcrt/cfx_unowned_ptr.h"

class CPDF_Object;
class CPDF_Reference;
class CPDF_IndirectObjectHolder;
class CPDF_ReadValidator;

// Helper for check availability of object tree.
class CPDF_ObjectAvail {
 public:
  using AvailStatus = CPDF_DataAvail::DocAvailStatus;

  CPDF_ObjectAvail(CPDF_ReadValidator* validator,
                   CPDF_IndirectObjectHolder* holder,
                   const CPDF_Object* root);
  CPDF_ObjectAvail(CPDF_ReadValidator* validator,
                   CPDF_IndirectObjectHolder* holder,
                   uint32_t obj_num);
  virtual ~CPDF_ObjectAvail();

  AvailStatus CheckAvail();

 protected:
  virtual bool ExcludeObject(const CPDF_Object* object) const;

 private:
  bool Init();
  void CheckObjects();
  bool AppendObjectSubRefs(const CPDF_Object* object,
                           std::vector<uint32_t>* refs) const;
  void CleanMemory();

  CFX_UnownedPtr<CPDF_ReadValidator> validator_;
  CFX_UnownedPtr<CPDF_IndirectObjectHolder> holder_;
  std::unique_ptr<CPDF_Reference> generated_root_;
  const CPDF_Object* root_;
  std::set<uint32_t> parsed_objnums_;
  std::vector<uint32_t> non_parsed_objects_;
};

// Helper for check availability of page's object tree.
// Exclude references to pages.
class CPDF_PageObjectAvail : public CPDF_ObjectAvail {
 public:
  using CPDF_ObjectAvail::CPDF_ObjectAvail;
  ~CPDF_PageObjectAvail() override;

 protected:
  bool ExcludeObject(const CPDF_Object* object) const override;
};

#endif  // CORE_FPDFAPI_PARSER_CPDF_OBJECT_AVAIL_H_
