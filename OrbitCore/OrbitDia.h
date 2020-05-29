// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#pragma once

#include <ostream>
struct IDiaSymbol;

namespace OrbitDia {
void DiaDump(IDiaSymbol* a_Symbol);
void DiaDump(unsigned long a_SymbolId);
void DiaDump(IDiaSymbol* Symbol, std::ostream& OS, int Indent);
}  // namespace OrbitDia