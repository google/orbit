// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SymbolProvider/SymbolLoadingOutcome.h"

#include <variant>

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"

namespace orbit_symbol_provider {

using orbit_base::NotFoundOr;

bool IsCanceled(const SymbolLoadingOutcome& outcome) {
  return outcome.has_value() && orbit_base::IsCanceled(outcome.value());
}

bool IsNotFound(const SymbolLoadingOutcome& outcome) {
  return outcome.has_value() && !IsCanceled(outcome) &&
         orbit_base::IsNotFound(orbit_base::GetNotCanceled(outcome.value()));
}

std::string GetNotFoundMessage(const SymbolLoadingOutcome& outcome) {
  ORBIT_CHECK(IsNotFound(outcome));
  return orbit_base::GetNotFoundMessage(orbit_base::GetNotCanceled(outcome.value()));
}

bool IsSuccessResult(const SymbolLoadingOutcome& outcome) {
  return outcome.has_value() && !IsCanceled(outcome) && !IsNotFound(outcome) &&
         std::holds_alternative<SymbolLoadingSuccessResult>(
             orbit_base::GetNotCanceled(outcome.value()));
}

SymbolLoadingSuccessResult GetSuccessResult(const SymbolLoadingOutcome& outcome) {
  ORBIT_CHECK(IsSuccessResult(outcome));
  return orbit_base::GetFound(orbit_base::GetNotCanceled(outcome.value()));
}

}  // namespace orbit_symbol_provider