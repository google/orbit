// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "FileWriter.h"
#include "OrbitBase/ExecutablePath.h"

namespace {

[[nodiscard]] const std::filesystem::path GetExeDir() {
  return orbit_base::GetExecutablePath().parent_path();
}

[[nodiscard]] const std::filesystem::path GetCppWin32Dir() {
  return std::filesystem::absolute(GetExeDir() / "../../third_party/cppwin32/");
}

[[nodiscard]] const std::filesystem::path GetMetadataDir() {
  return std::filesystem::absolute(GetExeDir() / "../../third_party/winmd/");
}

[[nodiscard]] const std::filesystem::path GetSourceDir() {
  return GetExeDir() / "../../src/WindowsApiShim/";
}

[[nodiscard]] const std::filesystem::path GetOutputDir() {
  return std::filesystem::absolute(GetExeDir() / "../src/WindowsApiShim/generated/");
}

[[nodiscard]] std::vector<std::filesystem::path> GetInputFiles() {
  return {GetMetadataDir() / "Windows.Win32.winmd",
          GetMetadataDir() / "Windows.Win32.Interop.winmd"};
}

void CopyFile(std::filesystem::path source, std::filesystem::path dest) {
  std::filesystem::create_directories(dest.parent_path());
  std::filesystem::copy_file(source, dest, std::filesystem::copy_options::overwrite_existing);
}

}  // namespace

int main(int const argc, char* argv[]) {
  // Remove existing output directory.
  std::filesystem::path output_dir = GetOutputDir();
  std::filesystem::remove_all(output_dir);

  // Copy files.
  CopyFile(GetCppWin32Dir() / "cppwin32" / "base.h", output_dir / "win32" / "base.h");
  CopyFile(GetSourceDir() / "NamespaceDispatcher.h",
           output_dir / "win32" / "NamespaceDispatcher.h");

  // Generate code.
  orbit_windows_api_shim::FileWriter file_writer(GetInputFiles(), output_dir);
  file_writer.WriteCodeFiles();
}
