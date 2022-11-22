// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <memory>

#include "PdbFileLlvm.h"
#include "PdbFileTest.h"

using orbit_object_utils::PdbFileLlvm;

INSTANTIATE_TYPED_TEST_SUITE_P(PdbFileLlvmTest, PdbFileTest, ::testing::Types<PdbFileLlvm>);