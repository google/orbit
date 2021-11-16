// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/RetrieveInstances.h"

#include <absl/container/flat_hash_map.h>
#include <absl/strings/match.h>
#include <absl/strings/str_join.h>
#include <absl/types/span.h>

#include <QVector>
#include <algorithm>
#include <cstddef>
#include <iterator>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "MainThreadExecutor.h"
#include "MetricsUploader/ScopedMetric.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Result.h"

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Client;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using orbit_metrics_uploader::OrbitLogEvent;
using orbit_metrics_uploader::ScopedMetric;

class RetrieveInstancesImpl : public RetrieveInstances {
 public:
  RetrieveInstancesImpl(orbit_ggp::Client* ggp_client, MainThreadExecutor* main_thread_executor);

  orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>> LoadInstances(
      const std::optional<orbit_ggp::Project>& project,
      orbit_ggp::Client::InstanceListScope scope) override;
  orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>> LoadInstancesWithoutCache(
      const std::optional<orbit_ggp::Project>& project,
      orbit_ggp::Client::InstanceListScope scope) override;
  orbit_base::Future<ErrorMessageOr<LoadProjectsAndInstancesResult>> LoadProjectsAndInstances(
      const std::optional<orbit_ggp::Project>& project,
      orbit_ggp::Client::InstanceListScope scope) override;
  void SetMetricsUploader(orbit_metrics_uploader::MetricsUploader* metrics_uploader) override {
    metrics_uploader_ = metrics_uploader;
  }

 private:
  orbit_ggp::Client* ggp_client_ = nullptr;
  // To avoid race conditions to the instance_cache_, the main thread is used.
  MainThreadExecutor* main_thread_executor_ = nullptr;
  absl::flat_hash_map<
      std::pair<std::optional<orbit_ggp::Project>, orbit_ggp::Client::InstanceListScope>,
      QVector<orbit_ggp::Instance>>
      instance_cache_;
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_ = nullptr;
};

std::unique_ptr<RetrieveInstances> RetrieveInstances::Create(
    orbit_ggp::Client* ggp_client, MainThreadExecutor* main_thread_executor) {
  return std::make_unique<RetrieveInstancesImpl>(ggp_client, main_thread_executor);
}

RetrieveInstancesImpl::RetrieveInstancesImpl(Client* ggp_client,
                                             MainThreadExecutor* main_thread_executor)
    : ggp_client_(ggp_client), main_thread_executor_(main_thread_executor) {
  CHECK(ggp_client != nullptr);
  CHECK(main_thread_executor != nullptr);
}

Future<ErrorMessageOr<QVector<Instance>>> RetrieveInstancesImpl::LoadInstances(
    const std::optional<Project>& project, orbit_ggp::Client::InstanceListScope scope) {
  auto key = std::make_pair(project, scope);
  if (instance_cache_.contains(key)) {
    if (metrics_uploader_ != nullptr) {
      metrics_uploader_->SendLogEvent(OrbitLogEvent::ORBIT_INSTANCES_CACHE_HIT);
    }
    return Future<ErrorMessageOr<QVector<Instance>>>{instance_cache_.at(key)};
  }
  return LoadInstancesWithoutCache(project, scope);
}

Future<ErrorMessageOr<QVector<Instance>>> RetrieveInstancesImpl::LoadInstancesWithoutCache(
    const std::optional<Project>& project, orbit_ggp::Client::InstanceListScope scope) {
  ScopedMetric metric{metrics_uploader_, OrbitLogEvent::ORBIT_INSTANCES_LOAD};
  return ggp_client_->GetInstancesAsync(scope, project)
      .Then(main_thread_executor_,
            [metric = std::move(metric)](auto result) mutable {
              if (result.has_error()) {
                metric.SetStatusCode(OrbitLogEvent::INTERNAL_ERROR);
              }
              return result;
            })
      .ThenIfSuccess(main_thread_executor_,
                     [this, key = std::make_pair(project, scope)](QVector<Instance> instances) {
                       instance_cache_.emplace(key, instances);
                       return instances;
                     });
}

Future<ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult>>
RetrieveInstancesImpl::LoadProjectsAndInstances(const std::optional<orbit_ggp::Project>& project,
                                                orbit_ggp::Client::InstanceListScope scope) {
  const Future<ErrorMessageOr<QVector<Project>>> projects_future = ggp_client_->GetProjectsAsync();

  const Future<ErrorMessageOr<Project>> default_project_future =
      ggp_client_->GetDefaultProjectAsync();

  const Future<ErrorMessageOr<QVector<Instance>>> instances_from_project_future =
      LoadInstancesWithoutCache(project, scope);

  // It can be the case that the project (from the argument) does not exist anymore, or the user
  // lost access to it. In this case, a second call (the following) is made to
  // LoadInstancesWithoutCache with the default project (std::nullopt). This result of this call is
  // used when the first call returns that the project "may not exist" (the "it may not exist" error
  // comes from ggp itself). In case the project is already the default project, the second call
  // does not need to be made.
  const Future<ErrorMessageOr<QVector<Instance>>> instances_from_default_project_future =
      project != std::nullopt ? LoadInstancesWithoutCache(std::nullopt, scope)
                              : instances_from_project_future;

  Future<std::tuple<ErrorMessageOr<QVector<Project>>, ErrorMessageOr<Project>,
                    ErrorMessageOr<QVector<Instance>>, ErrorMessageOr<QVector<Instance>>>>
      combined_future = orbit_base::JoinFutures(projects_future, default_project_future,
                                                instances_from_project_future,
                                                instances_from_default_project_future);

  return combined_future.Then(
      main_thread_executor_,
      [project](
          const std::tuple<ErrorMessageOr<QVector<Project>>, ErrorMessageOr<Project>,
                           ErrorMessageOr<QVector<Instance>>, ErrorMessageOr<QVector<Instance>>>&
              result_tuple) -> ErrorMessageOr<LoadProjectsAndInstancesResult> {
        const auto& [projects, default_project, instances_from_project,
                     instances_from_default_project] = result_tuple;

        LoadProjectsAndInstancesResult result;
        std::vector<ErrorMessage> errors;

        if (projects.has_value()) {
          result.projects = projects.value();
        } else {
          errors.push_back(projects.error());
        }

        if (default_project.has_value()) {
          result.default_project = default_project.value();
        } else {
          errors.push_back(default_project.error());
        }

        if (instances_from_project.has_value()) {
          result.instances = instances_from_project.value();
          result.project_of_instances = project;
        } else {
          if (absl::StrContains(instances_from_project.error().message(), "it may not exist")) {
            // If the result from instances_from_project "may not exist", then
            // instances_from_default_project is used.
            if (instances_from_default_project.has_value()) {
              result.instances = instances_from_default_project.value();
              result.project_of_instances = std::nullopt;
            } else {
              errors.push_back(instances_from_default_project.error());
            }
          } else {
            // This is the case that instances_from_project has an error that is different from "it
            // may not exist".
            errors.push_back(instances_from_project.error());
          }
        }

        if (errors.empty()) return result;

        std::string combined_error_messages = absl::StrJoin(
            errors, "\n",
            [](std::string* out, const ErrorMessage& err) { out->append(err.message()); });

        return ErrorMessage{
            absl::StrFormat("The following error occured:\n%s", combined_error_messages)};
      });
}

}  // namespace orbit_session_setup