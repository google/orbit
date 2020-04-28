// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/GgpInstance.h"

#include <QByteArray>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

namespace {
GgpInstance GetInstanceFromJson(const QJsonObject& obj) {
  GgpInstance inst{};

  if (const QJsonValue val = obj.value("displayName"); !val.isUndefined()) {
    inst.display_name = val.toString();
  }

  if (const QJsonValue val = obj.value("id"); !val.isUndefined()) {
    inst.id = val.toString();
  }

  if (const QJsonValue val = obj.value("ipAddress"); !val.isUndefined()) {
    inst.ip_address = val.toString();
  }

  if (const QJsonValue val = obj.value("lastUpdated"); !val.isUndefined()) {
    inst.last_updated = QDateTime::fromString(val.toString(), Qt::ISODate);
  }

  if (const QJsonValue val = obj.value("owner"); !val.isUndefined()) {
    inst.owner = val.toString();
  }

  if (const QJsonValue val = obj.value("pool"); !val.isUndefined()) {
    inst.pool = val.toString();
  }

  return inst;
}
}  // namespace

QVector<GgpInstance> GgpInstance::GetListFromJson(const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isArray()) return {};

  const QJsonArray arr = doc.array();

  QVector<GgpInstance> list;

  std::transform(arr.begin(), arr.end(), std::back_inserter(list),
                 [](const QJsonValue& val) -> GgpInstance {
                   if (!val.isObject()) return {};

                   const QJsonObject obj = val.toObject();

                   return GetInstanceFromJson(obj);
                 });

  return list;
}

bool GgpInstance::CmpById(const GgpInstance& lhs, const GgpInstance& rhs) {
  return lhs.id < rhs.id;
}