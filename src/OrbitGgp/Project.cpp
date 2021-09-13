// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "OrbitGgp/Project.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

namespace {

ErrorMessageOr<Project> GetProjectFromJson(const QJsonObject& obj) {
  const auto safe_json_to_string = [](const QJsonValue& val) -> ErrorMessageOr<QString> {
    if (!val.isString()) {
      return ErrorMessage{"Unable to parse JSON: String expected."};
    }
    return val.toString();
  };

  OUTCOME_TRY(auto&& display_name, safe_json_to_string(obj.value("displayName")));
  OUTCOME_TRY(auto&& id, safe_json_to_string(obj.value("id")));

  return Project{display_name, id};
}

}  // namespace

ErrorMessageOr<QVector<Project>> Project::GetListFromJson(const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isArray()) return ErrorMessage{"Unable to parse JSON: Array expected."};

  const QJsonArray arr = doc.array();

  QVector<Project> list;

  for (const auto& json_value : arr) {
    if (!json_value.isObject()) return ErrorMessage{"Unable to parse JSON: Object expected."};

    OUTCOME_TRY(auto&& project, GetProjectFromJson(json_value.toObject()));
    list.push_back(std::move(project));
  }

  return list;
}

}  // namespace orbit_ggp