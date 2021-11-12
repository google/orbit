// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef SESSION_SETUP_RETRIEVE_INSTANCES_H_
#define SESSION_SETUP_RETRIEVE_INSTANCES_H_

#include <QVector>
#include <memory>
#include <optional>

#include "MainThreadExecutor.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitGgp/Client.h"
#include "OrbitGgp/Instance.h"
#include "OrbitGgp/Project.h"

namespace orbit_session_setup {

// RetrieveInstances manages calls to orbit_ggp::Client for retrieving the list of projects and
// instances. It acts as an abstraction layer that can add functionality like caching and combines
// calls to be executed in parallel. Its intended use is retrieving the data for
// RetrieveInstancesWidget.
class RetrieveInstances {
 public:
  // This struct holds the result of call to LoadProjectsAndInstances.
  // * projects: List of projects.
  // * default_project: Default project.
  // * instances: list of instances.
  // * project_of_instances: project of the list of instances.
  struct LoadProjectsAndInstancesResult {
    QVector<orbit_ggp::Project> projects;
    orbit_ggp::Project default_project;
    QVector<orbit_ggp::Instance> instances;
    std::optional<orbit_ggp::Project> project_of_instances;
  };

  virtual ~RetrieveInstances() = default;

  virtual orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>> LoadInstances(
      const std::optional<orbit_ggp::Project>& project,
      orbit_ggp::Client::InstanceListScope scope) = 0;
  virtual orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>>
  LoadInstancesWithoutCache(const std::optional<orbit_ggp::Project>& project,
                            orbit_ggp::Client::InstanceListScope scope) = 0;
  // LoadProjectsAndInstances always loads the project list and the default project. Additionally,it
  // is attempted to load the list of instances for the input project. The later can fail, when the
  // project does not exist anymore. Then the instance list of the default project is loaded and
  // returned. To indicate to which project the list of instances belongs,
  // LoadProjectsAndInstancesResult has the field project_of_instances.
  virtual orbit_base::Future<ErrorMessageOr<LoadProjectsAndInstancesResult>>
  LoadProjectsAndInstances(const std::optional<orbit_ggp::Project>& project,
                           orbit_ggp::Client::InstanceListScope scope) = 0;
  virtual void SetMetricsUploader(orbit_metrics_uploader::MetricsUploader* metrics_uploader) = 0;

  [[nodiscard]] static std::unique_ptr<RetrieveInstances> Create(
      orbit_ggp::Client* ggp_client, MainThreadExecutor* main_thread_executor);
};

}  // namespace orbit_session_setup

#endif  // SESSION_SETUP_RETRIEVE_INSTANCES_H_