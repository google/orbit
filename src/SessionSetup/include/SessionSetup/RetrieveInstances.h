// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_RETRIEVE_INSTANCES_H_
#define SESSION_SETUP_RETRIEVE_INSTANCES_H_

#include <absl/container/flat_hash_map.h>

#include <QObject>
#include <QVector>
#include <optional>

#include "MainThreadExecutor.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/Project.h"

namespace orbit_session_setup {

class RetrieveInstances : public QObject {
 public:
  struct LoadProjectsAndInstancesResult {
    QVector<orbit_ggp::Project> projects;
    orbit_ggp::Project default_project;
    QVector<orbit_ggp::Instance> instances;
    std::optional<orbit_ggp::Project> instances_project;
  };

  RetrieveInstances(orbit_ggp::Client* ggp_client, MainThreadExecutor* main_thread_executor,
                    QObject* parent = nullptr);

  orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>> LoadInstances(
      const std::optional<orbit_ggp::Project>& project, orbit_ggp::Client::InstanceListScope scope);
  orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>> LoadInstancesWithoutCache(
      const std::optional<orbit_ggp::Project>& project, orbit_ggp::Client::InstanceListScope scope);
  orbit_base::Future<ErrorMessageOr<LoadProjectsAndInstancesResult>> LoadProjectsAndInstances(
      const std::optional<orbit_ggp::Project>& project, orbit_ggp::Client::InstanceListScope scope);

 private:
  orbit_ggp::Client* ggp_client_;
  // To avoid race conditions to the instance_cache_, the main thread is used.
  MainThreadExecutor* main_thread_executor_;
  absl::flat_hash_map<
      std::pair<std::optional<orbit_ggp::Project>, orbit_ggp::Client::InstanceListScope>,
      QVector<orbit_ggp::Instance>>
      instance_cache_;
  LoadProjectsAndInstancesResult projects_and_instances_load_result_;
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_RETRIEVE_INSTANCES_H_