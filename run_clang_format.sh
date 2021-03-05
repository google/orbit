#!/bin/bash

# Copyright (c) 2021 The Orbit Authors. All rights reserved.
# Use of this source code is governed by a BSD-style license that can be
# found in the LICENSE file.

find src/ \( -name '*.cpp' -o -name '*.h' \) ! -name 'resource.h' | xargs /usr/bin/clang-format-9 -i
