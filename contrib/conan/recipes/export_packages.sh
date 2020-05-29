#!/bin/bash
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.


cd "$( dirname "${BASH_SOURCE[0]}" )"

for i in llvm-common *; do
  (cd $i && conan export . orbitdeps/stable)
done
