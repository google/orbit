// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Symbols/SymbolLoadingOutcome.h"

#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"

namespace orbit_symbols {

using orbit_base::NotFoundOr;

bool IsCanceled(const SymbolLoadingOutcome& outcome) {
  return outcome.has_value() && orbit_base::IsCanceled(outcome.value());
}

bool IsNotFound(const SymbolLoadingOutcome& outcome) {
  return outcome.has_value() && !IsCanceled(outcome) &&
         orbit_base::IsNotFound(std::get<NotFoundOr<SuccessOutcome>>(outcome.value()));
}

std::string GetNotFoundMessage(const SymbolLoadingOutcome& outcome) {
  ORBIT_CHECK(IsNotFound(outcome));
  return orbit_base::GetNotFoundMessage(std::get<NotFoundOr<SuccessOutcome>>(outcome.value()));
}

bool IsSuccessOutcome(const SymbolLoadingOutcome& outcome) {
  return outcome.has_value() && !IsCanceled(outcome) && !IsNotFound(outcome) &&
         std::holds_alternative<SuccessOutcome>(
             std::get<NotFoundOr<SuccessOutcome>>(outcome.value()));
}

SuccessOutcome GetSuccessOutcome(const SymbolLoadingOutcome& outcome) {
  ORBIT_CHECK(IsSuccessOutcome(outcome));
  return std::get<SuccessOutcome>(std::get<NotFoundOr<SuccessOutcome>>(outcome.value()));
}

}  // namespace orbit_symbols