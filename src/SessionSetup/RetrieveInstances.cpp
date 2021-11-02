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
#include "OrbitBase/Future.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/JoinFutures.h"
#include "OrbitBase/Result.h"

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Client;
using orbit_ggp::Instance;
using orbit_ggp::Project;

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

 private:
  orbit_ggp::Client* ggp_client_;
  // To avoid race conditions to the instance_cache_, the main thread is used.
  MainThreadExecutor* main_thread_executor_;
  absl::flat_hash_map<
      std::pair<std::optional<orbit_ggp::Project>, orbit_ggp::Client::InstanceListScope>,
      QVector<orbit_ggp::Instance>>
      instance_cache_;
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
    return Future<ErrorMessageOr<QVector<Instance>>>{instance_cache_.at(key)};
  }
  return LoadInstancesWithoutCache(project, scope);
}

Future<ErrorMessageOr<QVector<Instance>>> RetrieveInstancesImpl::LoadInstancesWithoutCache(
    const std::optional<Project>& project, orbit_ggp::Client::InstanceListScope scope) {
  auto future = ggp_client_->GetInstancesAsync(scope, project);
  (void)future.ThenIfSuccess(main_thread_executor_, [this, key = std::make_pair(project, scope)](
                                                        QVector<Instance> instances) {
    instance_cache_.emplace(key, instances);
  });
  return future;
}

Future<ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult>>
RetrieveInstancesImpl::LoadProjectsAndInstances(const std::optional<orbit_ggp::Project>& project,
                                                orbit_ggp::Client::InstanceListScope scope) {
  const Future<ErrorMessageOr<QVector<Project>>> project_list_future =
      ggp_client_->GetProjectsAsync();

  const Future<ErrorMessageOr<Project>> default_project_future =
      ggp_client_->GetDefaultProjectAsync();

  const Future<ErrorMessageOr<std::pair<QVector<Instance>, std::optional<Project>>>>
      instance_list_future = orbit_base::UnwrapFuture(
          ggp_client_->GetInstancesAsync(scope, project)
              .Then(main_thread_executor_,
                    [this, scope, project](ErrorMessageOr<QVector<Instance>> instances)
                        -> Future<
                            ErrorMessageOr<std::pair<QVector<Instance>, std::optional<Project>>>> {
                      if (instances.has_value()) {
                        return {std::make_pair(instances.value(), project)};
                      }
                      if (project == std::nullopt ||
                          !absl::StrContains(instances.error().message(), "it may not exist")) {
                        return {instances.error()};
                      }
                      // If the error message contains "it may not exist", then the project may not
                      // exist anymore and the call to ggp is retried without a project (default
                      // project)
                      return ggp_client_->GetInstancesAsync(scope, std::nullopt)
                          .ThenIfSuccess(
                              main_thread_executor_,
                              [](const QVector<Instance>& instances)
                                  -> ErrorMessageOr<
                                      std::pair<QVector<Instance>, std::optional<Project>>> {
                                return std::make_pair(instances, std::nullopt);
                              });
                    }));

  Future<std::tuple<ErrorMessageOr<QVector<Project>>, ErrorMessageOr<Project>,
                    ErrorMessageOr<std::pair<QVector<Instance>, std::optional<Project>>>>>
      combined_future = orbit_base::JoinFutures(project_list_future, default_project_future,
                                                instance_list_future);

  return combined_future.Then(
      main_thread_executor_,
      [](std::tuple<ErrorMessageOr<QVector<Project>>, ErrorMessageOr<Project>,
                    ErrorMessageOr<std::pair<QVector<Instance>, std::optional<Project>>>>
             result_tuple) -> ErrorMessageOr<LoadProjectsAndInstancesResult> {
        std::vector<ErrorMessage> errors;
        if (std::get<0>(result_tuple).has_error()) {
          errors.push_back(std::get<0>(result_tuple).error());
        }
        if (std::get<1>(result_tuple).has_error()) {
          errors.push_back(std::get<1>(result_tuple).error());
        }
        if (std::get<2>(result_tuple).has_error()) {
          errors.push_back(std::get<2>(result_tuple).error());
        }

        if (errors.empty()) {
          return LoadProjectsAndInstancesResult{
              std::get<0>(result_tuple).value(),       /* projects*/
              std::get<1>(result_tuple).value(),       /* default_project*/
              std::get<2>(result_tuple).value().first, /* instances*/
              std::get<2>(result_tuple).value().second /* project_of_instances*/
          };
        }

        std::string combined_error_messages = absl::StrJoin(
            errors, "\n",
            [](std::string* out, const ErrorMessage& err) { out->append(err.message()); });

        return ErrorMessage{
            absl::StrFormat("The following error occured:\n%s", combined_error_messages)};
      });
}

}  // namespace orbit_session_setup