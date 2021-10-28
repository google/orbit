// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "SessionSetup/RetrieveInstances.h"

#include <utility>

#include "MainThreadExecutor.h"
#include "OrbitBase/Future.h"

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Client;
using orbit_ggp::Instance;
using orbit_ggp::Project;

RetrieveInstances::RetrieveInstances(Client* ggp_client, MainThreadExecutor* main_thread_executor,
                                     QObject* parent)
    : QObject(parent), ggp_client_(ggp_client), main_thread_executor_(main_thread_executor) {
  CHECK(ggp_client != nullptr);
}

Future<ErrorMessageOr<QVector<Instance>>> RetrieveInstances::LoadInstances(
    const std::optional<Project>& project, bool all) {
  auto key = std::make_pair(project, all);
  if (instance_cache_.contains(key)) {
    return Future<ErrorMessageOr<QVector<Instance>>>{instance_cache_.at(key)};
  }
  return LoadInstancesWithoutCache(project, all);
}

Future<ErrorMessageOr<QVector<Instance>>> RetrieveInstances::LoadInstancesWithoutCache(
    const std::optional<Project>& project, bool all) {
  auto future = ggp_client_->GetInstancesAsync(all, project);
  (void)future.ThenIfSuccess(main_thread_executor_, [this, key = std::make_pair(project, all)](
                                                        QVector<Instance> instances) {
    instance_cache_.emplace(key, instances);
  });
  return future;
}

}  // namespace orbit_session_setup