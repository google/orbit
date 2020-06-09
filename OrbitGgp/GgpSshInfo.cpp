// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/GgpSshInfo.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "OrbitBase/Logging.h"

namespace OrbitGgp {

std::optional<GgpSshInfo> GgpSshInfo::CreateFromJson(const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isObject()) return {};
  const QJsonObject obj = doc.object();

  const QJsonValue host_val = obj.value("host");
  const QJsonValue key_path_val = obj.value("keyPath");
  const QJsonValue known_hosts_path_val = obj.value("knownHostsPath");
  const QJsonValue port_val = obj.value("port");
  const QJsonValue user_val = obj.value("user");

  if (host_val.isUndefined() || !host_val.isString() ||
      key_path_val.isUndefined() || !key_path_val.isString() ||
      known_hosts_path_val.isUndefined() || !known_hosts_path_val.isString() ||
      port_val.isUndefined() || !port_val.isString() ||
      user_val.isUndefined() || !user_val.isString()) {
    return {};
  }

  // The json has the port formatted as a string ("port":"333"), hence this
  // conversion. This is standard the Qt way to check whether the casting worked
  bool ok;
  int port = port_val.toString().toInt(&ok);
  if (!ok) return {};

  GgpSshInfo ggp_ssh_info;

  ggp_ssh_info.host = host_val.toString();
  ggp_ssh_info.key_path = key_path_val.toString();
  ggp_ssh_info.known_hosts_path = known_hosts_path_val.toString();
  ggp_ssh_info.port = port;
  ggp_ssh_info.user = user_val.toString();

  return ggp_ssh_info;
}

}  // namespace OrbitGgp
