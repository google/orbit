#!/bin/bash

# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.
clang_format_path="/usr/bin/clang-format-14";

# If we are running in git bash this is the default location:
if  [ -f "/c/Program Files/LLVM/bin/clang-format.exe" ]; then
  clang_format_path="/c/Program Files/LLVM/bin/clang-format.exe";
fi

find src/ \( -name '*.cpp' -o -name '*.h' -o -name '*.proto' \) ! -name 'resource.h' | xargs "$clang_format_path" -i
