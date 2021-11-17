// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/Account.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

ErrorMessageOr<Account> Account::GetDefaultAccountFromJson(const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isArray()) return ErrorMessage{"Unable to parse JSON: Array expected."};

  for (const auto& array_entry : doc.array()) {
    if (!array_entry.isObject()) return ErrorMessage{"Unable to parse JSON: Object expected."};

    const QJsonObject obj{array_entry.toObject()};

    if (!obj.contains("default")) {
      return ErrorMessage{"Unable to parse JSON: \"default\" key missing."};
    }

    if (obj.value("default").toString() != "yes") continue;

    if (!obj.contains("account")) {
      return ErrorMessage{"Unable to parse JSON: \"account\" key missing."};
    }

    return Account{obj.value("account").toString()};
  }
  return ErrorMessage{"Failed to find default ggp account."};
}

}  // namespace orbit_ggp