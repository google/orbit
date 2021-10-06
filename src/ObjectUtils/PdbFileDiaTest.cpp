// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PdbFileTest.h"

// Must be included after PdbFileTest.h
#include "PdbFileDia.h"

using orbit_object_utils::PdbFileDia;

INSTANTIATE_TYPED_TEST_SUITE_P(PdbFileDiaTest, PdbFileTest, ::testing::Types<PdbFileDia>);
