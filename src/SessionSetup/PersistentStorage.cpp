// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/PersistentStorage.h"

#include <QSettings>

#include "OrbitGgp/Project.h"

namespace orbit_session_setup {

constexpr const char* kSelectedProjectIdKey{"kSelectedProjectIdKey"};
constexpr const char* kSelectedProjectDisplayNameKey{"kSelectedProjectDisplayNameKey"};

using orbit_ggp::Project;

[[nodiscard]] std::optional<orbit_ggp::Project> LoadProjectFromPersistentStorage() {
  QSettings settings;

  std::optional<Project> project;
  if (settings.contains(kSelectedProjectIdKey)) {
    project = Project{
        settings.value(kSelectedProjectDisplayNameKey).toString() /* display_name */,
        settings.value(kSelectedProjectIdKey).toString() /* id */
    };
  }
  return project;
}

void SaveProjectToPersistentStorage(std::optional<Project> project) {
  QSettings settings;

  if (project.has_value()) {
    settings.setValue(kSelectedProjectIdKey, project->id);
    settings.setValue(kSelectedProjectDisplayNameKey, project->display_name);
  } else {
    settings.remove(kSelectedProjectIdKey);
    settings.remove(kSelectedProjectDisplayNameKey);
  }
}

}  // namespace orbit_session_setup