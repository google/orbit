// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_WINDOWS_API_SHIM_FILE_WRITER_H_
#define ORBIT_WINDOWS_API_SHIM_FILE_WRITER_H_

#include <cppwin32/winmd/winmd_reader.h>

#include <memory>

#include "FunctionIdGenerator.h"
#include "WinMdCache.h"
#include "WinMdHelper.h"

namespace orbit_windows_api_shim {

// FileWriter takes a WinMd (Windows Meta Data) file as input and outputs a set of .h/.cpp files to
// be used by compiled into the WindowsApiShim library. It relies on "cppwin32" project for the
// WinMd parsing and some of the code generation.
class FileWriter {
 public:
  FileWriter(std::vector<std::filesystem::path> input, std::filesystem::path output_dir);
  ~FileWriter() = default;

  void WriteCodeFiles();

 private:
  [[nodiscard]] FunctionIdGenerator WriteManifestHeader();
  void WriteCmakeFile();
  void WriteComplexInterfaceHeader();
  void WriteComplexStructsHeader();
  void WriteNamespaceDispatchCpp();
  void WriteNamespaceHeader(const WinMdCache::Entry& cache_entry);
  void WriteNamespaceCpp(const WinMdCache::Entry& cache_entry);

  const winmd::reader::database* win32_database_ = nullptr;
  std::unique_ptr<winmd::reader::cache> cache_ = nullptr;
  std::unique_ptr<WinMdCache> win_md_cache_ = nullptr;
  std::unique_ptr<WinMdHelper> win32_metadata_helper_;
  FunctionIdGenerator function_id_generator_;
};

}  // namespace orbit_windows_api_shim

#endif  // ORBIT_WINDOWS_API_SHIM_FILE_WRITER_H_