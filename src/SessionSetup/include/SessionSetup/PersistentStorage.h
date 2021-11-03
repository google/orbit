// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_PERSISTENT_STORAGE_H_
#define SESSION_SETUP_PERSISTENT_STORAGE_H_

#include <optional>

#include "OrbitGgp/Project.h"

namespace orbit_session_setup {

[[nodiscard]] std::optional<orbit_ggp::Project> LoadProjectFromPersistentStorage();

void SaveProjectToPersistentStorage(std::optional<orbit_ggp::Project> project);

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_PERSISTENT_STORAGE_H_