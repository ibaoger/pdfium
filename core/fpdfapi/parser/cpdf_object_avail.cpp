// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/parser/cpdf_object_avail.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_indirect_object_holder.h"
#include "core/fpdfapi/parser/cpdf_object_walker.h"
#include "core/fpdfapi/parser/cpdf_read_validator.h"
#include "core/fpdfapi/parser/cpdf_reference.h"

CPDF_ObjectAvail::CPDF_ObjectAvail(CPDF_ReadValidator* validator,
                                   CPDF_IndirectObjectHolder* holder,
                                   const CPDF_Object* root)
    : validator_(validator), holder_(holder), root_(root) {
  ASSERT(validator_);
  ASSERT(holder);
  ASSERT(root_);
  if (!root_->IsInline())
    parsed_objnums_.insert(root_->GetObjNum());
}

CPDF_ObjectAvail::CPDF_ObjectAvail(CPDF_ReadValidator* validator,
                                   CPDF_IndirectObjectHolder* holder,
                                   uint32_t obj_num)
    : validator_(validator),
      holder_(holder),
      generated_root_(pdfium::MakeUnique<CPDF_Reference>(holder, obj_num)),
      root_(generated_root_.get()) {
  ASSERT(validator_);
  ASSERT(holder);
}

CPDF_ObjectAvail::~CPDF_ObjectAvail() {}

CPDF_ObjectAvail::AvailStatus CPDF_ObjectAvail::CheckAvail() {
  if (!Init())
    return AvailStatus::DataNotAvailable;

  CheckObjects();
  if (non_parsed_objects_.empty()) {
    CleanMemory();
    return AvailStatus::DataAvailable;
  }
  return AvailStatus::DataNotAvailable;
}

bool CPDF_ObjectAvail::Init() {
  if (!non_parsed_objects_.empty())
    return true;
  while (root_ && root_->IsReference()) {
    const uint32_t ref_obj_num = root_->AsReference()->GetRefObjNum();
    if (parsed_objnums_.count(ref_obj_num) > 0) {
      root_ = nullptr;
      return true;
    }
    const CPDF_ReadValidator::Session parse_session(validator_.Get());
    const CPDF_Object* direct = holder_->GetOrParseIndirectObject(ref_obj_num);
    if (validator_->has_read_problems())
      return false;

    parsed_objnums_.insert(ref_obj_num);
    root_ = direct;
  }
  if (!AppendObjectSubRefs(root_, &non_parsed_objects_)) {
    non_parsed_objects_.clear();
    return false;
  }
  return true;
}

void CPDF_ObjectAvail::CheckObjects() {
  std::vector<uint32_t> objects_to_check = std::move(non_parsed_objects_);
  std::vector<uint32_t> non_parsed_objects;
  while (!objects_to_check.empty()) {
    const uint32_t obj_num = objects_to_check.back();
    objects_to_check.pop_back();
    if (parsed_objnums_.count(obj_num) > 0)
      continue;

    const CPDF_ReadValidator::Session parse_session(validator_.Get());
    const CPDF_Object* direct = holder_->GetOrParseIndirectObject(obj_num);
    if (direct == root_)
      continue;

    if (validator_->has_read_problems() ||
        !AppendObjectSubRefs(direct, &objects_to_check)) {
      non_parsed_objects.push_back(obj_num);
      continue;
    }
    parsed_objnums_.insert(obj_num);
  }
  non_parsed_objects_ = std::move(non_parsed_objects);
}

bool CPDF_ObjectAvail::AppendObjectSubRefs(const CPDF_Object* object,
                                           std::vector<uint32_t>* refs) const {
  ASSERT(refs);
  if (!object)
    return true;

  CPDF_ObjectWalker walker(object);
  while (const CPDF_Object* obj = walker.GetNext()) {
    const CPDF_ReadValidator::Session parse_session(validator_.Get());

    // Skip root object if it is inlined subobject.
    bool skip = walker.GetParent() && obj == root_;
    // Do not enter into Parent objects.
    skip = skip || walker.dictionary_key() == "Parent";

    skip = skip || ((obj != root_) && ExcludeObject(obj));

    // We can parse other objects while checking values ("Type" value of
    // dictionary for example), and its are may be referenced, as result we
    // should wait it availabilty to correct filtration.
    if (validator_->has_read_problems())
      return false;

    if (skip) {
      walker.SkipWalkIntoCurrentObject();
      continue;
    }

    if (obj->IsReference())
      refs->push_back(obj->AsReference()->GetRefObjNum());
  }
  return true;
}

void CPDF_ObjectAvail::CleanMemory() {
  root_ = nullptr;
  generated_root_.reset();
  parsed_objnums_.clear();
}

bool CPDF_ObjectAvail::ExcludeObject(const CPDF_Object* object) const {
  return false;
}

CPDF_PageObjectAvail::~CPDF_PageObjectAvail() {}

bool CPDF_PageObjectAvail::ExcludeObject(const CPDF_Object* object) const {
  if (CPDF_ObjectAvail::ExcludeObject(object))
    return true;

  return object->IsDictionary() &&
         object->GetDict()->GetStringFor("Type") == "Page";
}
