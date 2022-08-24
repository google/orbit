// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include "OrbitBase/CanceledOr.h"
#include "Symbols/SymbolLoadingOutcome.h"

namespace orbit_symbols {

namespace {

const SuccessOutcome kSuccessOutcome{std::filesystem::path{"/tmp/test/path"},
                                     SuccessOutcome::SymbolSource::kStadiaInstance,
                                     SuccessOutcome::SymbolFileSeparation::kDifferentFile};

const orbit_base::NotFound kNotFound{"Did not find symbols"};

}  // namespace

TEST(SymbolLoadingOutcome, IsCanceled) {
  SymbolLoadingOutcome outcome{orbit_base::Canceled{}};
  EXPECT_TRUE(IsCanceled(outcome));
}

TEST(SymbolLoadingOutcome, IsSuccessOutcome) {
  SymbolLoadingOutcome outcome{kSuccessOutcome};
  EXPECT_TRUE(IsSuccessOutcome(outcome));
}

TEST(SymbolLoadingOutcome, GetSuccessOutcome) {
  SymbolLoadingOutcome outcome{kSuccessOutcome};
  const SuccessOutcome& success{GetSuccessOutcome(outcome)};
  EXPECT_EQ(success.path, kSuccessOutcome.path);
  EXPECT_EQ(success.symbol_source, kSuccessOutcome.symbol_source);
  EXPECT_EQ(success.symbol_file_separation, kSuccessOutcome.symbol_file_separation);
}

TEST(SymbolLoadingOutcome, IsNotFound) {
  SymbolLoadingOutcome outcome{kNotFound};
  EXPECT_TRUE(IsNotFound(outcome));
}

TEST(SymbolLoadingOutcome, GetNotFoundMessage) {
  SymbolLoadingOutcome outcome{kNotFound};
  EXPECT_EQ(GetNotFoundMessage(outcome), kNotFound.message);
}

}  // namespace orbit_symbols