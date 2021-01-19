// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// This is test executable used by LinuxTracing tests to test profiling functionality
// The control is done by executing commands

#include <dlfcn.h>
#include <stddef.h>

#include <filesystem>
#include <iostream>
#include <string>

#include "OrbitBase/ExecutablePath.h"
#include "OrbitBase/Logging.h"

// dlopen and call function_that_sleeps_for_one_second 10 times.
static void run_dlopen() {
  static const char* kLibraryFile = "libLinuxTracingTestPuppetLibrary.so";
  static const char* kFunctionName = "function_that_works_for_considerable_amount_of_time";
  // Setting rpath in CMAKE is a nightmare.. I was not able to do that, so we are going
  // to emulate "$ORIGIN/../lib" rpath here.
  std::string library_path =
      (orbit_base::GetExecutableDir() / ".." / "lib" / kLibraryFile).string();
  void* handle = dlopen(library_path.c_str(), RTLD_NOW);
  if (handle == nullptr) {
    FATAL("Unable to open \"%s\": %s", kLibraryFile, dlerror());
  }

  typedef double (*function_type)();
  function_type function = reinterpret_cast<function_type>(dlsym(handle, kFunctionName));
  if (function == nullptr) {
    FATAL("Unable to find function \"%s\" in \"%s\": %s", kFunctionName, kLibraryFile, dlerror());
  }

  for (size_t i = 0; i < 10; ++i) {
    std::cout << "Some useless number: " << function() << std::endl;
  }
}

int main() {
  std::cout << "Press ENTER to continue... ";
  std::string line;
  std::getline(std::cin, line);
  run_dlopen();
  std::cout << "Press ENTER to exit... ";
  std::getline(std::cin, line);
  return 0;
}