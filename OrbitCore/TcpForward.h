// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <array>

class tcp_server;

namespace asio {
class io_context;
}

namespace OrbitCore {
using MagicFooterBuffer = std::array<unsigned char, 4>;
constexpr inline MagicFooterBuffer GetMagicFooter() {
  return MagicFooterBuffer{0xf0, 0x07, 0xba, 0x11};
}
}  // namespace OrbitCore

//#define MAGIC_FOOT_MSG 0xf007ba11
#define ORBIT_HOST "localhost"
#define ORBIT_EXE_NAME "Orbit.exe"
