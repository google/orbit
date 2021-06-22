// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>

ABSL_FLAG(bool, local, false, "Connects to local instance of OrbitService");
ABSL_FLAG(std::string, process_name, "",
          "Automatically select and connect to the specified process");
ABSL_FLAG(bool, devmode, false, "Enable developer mode in the client's UI");