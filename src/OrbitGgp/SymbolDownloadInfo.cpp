// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/SymbolDownloadInfo.h"

#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>

#include "OrbitBase/Result.h"
#include "OrbitGgp/Error.h"

namespace orbit_ggp {
namespace {
ErrorMessageOr<SymbolDownloadInfo> CreateFromJson(const QJsonObject& obj) {
  const auto process = [](const QJsonValue& val) -> ErrorMessageOr<QString> {
    if (!val.isString()) return ErrorMessage{"Unable to parse JSON: String expected."};
    return val.toString();
  };
  OUTCOME_TRY(auto&& file_id, process(obj.value("fileId")));
  OUTCOME_TRY(auto&& url, process(obj.value("downloadUrl")));

  return SymbolDownloadInfo{file_id, url};
}
}  // namespace

ErrorMessageOr<QVector<SymbolDownloadInfo>> SymbolDownloadInfo::GetListFromJson(
    const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);
  if (!doc.isObject()) return ErrorMessage{"Unable to parse JSON: Object expected."};

  const QJsonObject symbols_obj = doc.object();
  if (!symbols_obj.contains("symbols")) {
    return ErrorMessage{"Unable to parse JSON: \"symbols\" key missing."};
  }

  const QJsonValue symbols_value = symbols_obj.value("symbols");
  if (!symbols_value.isArray()) return ErrorMessage{"Unable to parse JSON: Array expected."};

  const QJsonArray symbols_arr = symbols_value.toArray();
  QVector<SymbolDownloadInfo> list;
  for (const QJsonValue& symbol_value : symbols_arr) {
    if (!symbol_value.isObject()) return ErrorMessage{"Unable to parse JSON: Object expected."};

    OUTCOME_TRY(auto&& symbol_download_info, CreateFromJson(symbol_value.toObject()));
    list.push_back(std::move(symbol_download_info));
  }

  return list;
}

}  // namespace orbit_ggp
