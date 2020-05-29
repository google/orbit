#!/bin/bash
# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find . \( -name '*.cpp' -o -name '*.h' \) ! -path './external/*' ! -path './build/*' ! -path './cmake-*' ! -path './contrib*' ! -name 'resource.h' | xargs clang-format -i
