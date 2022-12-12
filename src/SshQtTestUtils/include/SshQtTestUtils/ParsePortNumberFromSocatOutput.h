// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_TEST_UTILS_PARSE_PORT_NUMBER_FROM_SOCAT_OUTPUT_H_
#define ORBIT_SSH_QT_TEST_UTILS_PARSE_PORT_NUMBER_FROM_SOCAT_OUTPUT_H_

#include <optional>
#include <string_view>

#include "OrbitBase/Result.h"

namespace orbit_ssh_qt_test_utils {

// If successful this functions returns a port number that has been parsed from the string
// `socat_output`. If `socat_output` doesn't yet contain the port number this functions returns an
// empty optional.
// If an error occurs the functions returns an error message. The intent is that the user retries to
// call the function until either an error occurs or it yields a port number.
[[nodiscard]] std::optional<ErrorMessageOr<int>> ParsePortNumberFromSocatOutput(
    std::string_view socat_output);

}  // namespace orbit_ssh_qt_test_utils

#endif  // ORBIT_SSH_QT_TEST_UTILS_PARSE_PORT_NUMBER_FROM_SOCAT_OUTPUT_H_
