#!/bin/bash

# Copyright (c) 2020 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find . \( -name '*.cpp' -o -name '*.h' \) ! -path './third_party/*' ! -path './build*/*' ! -path './cmake-*' ! -name 'resource.h' | xargs clang-format -i
