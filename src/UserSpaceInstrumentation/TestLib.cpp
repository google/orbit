// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TestLib.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <iostream>

namespace orbit_user_space_instrumentation {

namespace {

std::filesystem::path GetTmpFilePath() {
  static std::filesystem::path p = std::tmpnam(nullptr);
  return p;
}

}  // namespace

void InitTestLib() {
  auto p = GetTmpFilePath();
  std::cout << "Init Lib. Tmp file is: " << p << std::endl;;
}

void UseTestLib(const std::string& s) {
  auto p = GetTmpFilePath();
  std::ofstream ofs;
  ofs.open(p, std::ofstream::out | std::ofstream::app);
  ofs << s << std::endl;
  ofs.close();
}

void CloseTestLib() {
  auto p = GetTmpFilePath();

  std::cout << "Close Lib. Content of " << p << std::endl;

  std::ifstream tmp_file(p);
  std::string line;
  while (std::getline(tmp_file, line)) {
    std::cout << line << std::endl;
  }
  tmp_file.close();
  std::cout << std::endl << std::endl;

  std::remove(p.c_str());
}

}  // namespace orbit_user_space_instrumentation
