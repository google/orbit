// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#include "OrbitGgp/Project.h"

#include <absl/strings/str_format.h>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

namespace {

ErrorMessageOr<QString> SafeJsonValueToString(const QJsonValue& val) {
  if (!val.isString()) {
    return ErrorMessage{"Unable to parse JSON: String expected."};
  }
  return val.toString();
}

ErrorMessageOr<QString> SafeGetStringValueForKey(const QJsonObject& obj, QString key) {
  if (!obj.contains(key)) {
    return ErrorMessage{absl::StrFormat("Unable to parse JSON: Object does not contain key \"%s\"",
                                        key.toStdString())};
  }
  return SafeJsonValueToString(obj.value(key));
}

ErrorMessageOr<Project> GetProjectFromJson(const QJsonObject& obj) {
  OUTCOME_TRY(auto&& display_name, SafeGetStringValueForKey(obj, "displayName"));
  OUTCOME_TRY(auto&& id, SafeGetStringValueForKey(obj, "id"));

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

ErrorMessageOr<Project> Project::GetDefaultProjectFromJson(const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isObject()) return ErrorMessage{"Unable to parse JSON: Object expected."};

  const QJsonObject obj = doc.object();

  OUTCOME_TRY(auto&& display_name, SafeGetStringValueForKey(obj, "project"));
  OUTCOME_TRY(auto&& id, SafeGetStringValueForKey(obj, "projectId"));

  return Project{display_name, id};
}

}  // namespace orbit_ggp