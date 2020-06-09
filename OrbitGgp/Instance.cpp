// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Instance.h"

#include <qjsonvalue.h>

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/Error.h"

namespace OrbitGgp {

namespace {

outcome::result<Instance> GetInstanceFromJson(const QJsonObject& obj) {
  const auto process = [](const QJsonValue& val) -> outcome::result<QString> {
    if (!val.isString()) {
      return Error::kUnableToParseJson;
    } else {
      return val.toString();
    }
  };

  OUTCOME_TRY(display_name, process(obj.value("displayName")));
  OUTCOME_TRY(id, process(obj.value("id")));
  OUTCOME_TRY(ip_address, process(obj.value("ipAddress")));
  OUTCOME_TRY(last_updated, process(obj.value("lastUpdated")));
  OUTCOME_TRY(owner, process(obj.value("owner")));
  OUTCOME_TRY(pool, process(obj.value("pool")));

  auto last_updated_date_time =
      QDateTime::fromString(last_updated, Qt::ISODate);
  if (!last_updated_date_time.isValid()) {
    return Error::kUnableToParseJson;
  }

  Instance inst{};

  inst.display_name = display_name;
  inst.id = id;
  inst.ip_address = ip_address;
  inst.last_updated = last_updated_date_time;
  inst.owner = owner;
  inst.pool = pool;

  return inst;
}

}  // namespace

outcome::result<QVector<Instance>> Instance::GetListFromJson(
    const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isArray()) return Error::kUnableToParseJson;

  const QJsonArray arr = doc.array();

  QVector<Instance> list;

  for (const auto& json_value : arr) {
    if (!json_value.isObject()) return Error::kUnableToParseJson;

    OUTCOME_TRY(instance, GetInstanceFromJson(json_value.toObject()));
    list.push_back(std::move(instance));
  }

  return list;
}

bool Instance::CmpById(const Instance& lhs, const Instance& rhs) {
  return lhs.id < rhs.id;
}

}  // namespace OrbitGgp
