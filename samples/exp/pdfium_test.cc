// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "public/exp/pdfium.h"

#include "third_party/base/ptr_util.h"

static const char kUsageString[] =
    "Usage: pdfium_test [OPTION] [FILE]...\n"
    "";

struct Options {
  Options() {}

  std::string exe_path;
};

bool ParseCommandLine(const std::vector<std::string>& args,
                      Options* options,
                      std::vector<std::string>* files) {
  if (args.empty())
    return false;

  options->exe_path = args[0];
  size_t cur_idx = 1;

  for (size_t i = cur_idx; i < args.size(); i++)
    files->push_back(args[i]);

  return true;
}

class UnsupportedFeature : public pdfium::UnsupportedFeatureDelegate {
 public:
  UnsupportedFeature() = default;
  ~UnsupportedFeature() override = default;

  void HandleUnsupportedFeature(
      pdfium::UnsupportedFeatureDelegate::Type type) const override {
    fprintf(stderr, "Unsupported %d\n", static_cast<int>(type));
  }
};

int main(int argc, char** argv) {
  std::vector<std::string> args(argv, argv + argc);
  Options options;
  std::vector<std::string> files;
  if (!ParseCommandLine(args, &options, &files)) {
    fprintf(stderr, "%s", kUsageString);
    return 1;
  }
  if (files.empty()) {
    fprintf(stderr, "No input files provided.\n");
    return 1;
  }

  pdfium::Initialize(pdfium::MakeUnique<pdfium::PDFiumConfig>());
  pdfium::SetUnsupportedFeatureDelegate(
      pdfium::MakeUnique<UnsupportedFeature>());

  for (auto& file : files) {
    std::unique_ptr<pdfium::Document> doc;
    pdfium::Status status;
    std::tie(doc, status) = pdfium::Document::LoadFile(file.c_str(), "");

    if (status != pdfium::Status::kSuccess) {
      fprintf(stderr, "STATUS: %d\n", static_cast<int>(status));
      continue;
    }

    printf("%zu pages in %s\n", doc->PageCount(), file.c_str());
  }

  pdfium::Shutdown();

  return 0;
}
