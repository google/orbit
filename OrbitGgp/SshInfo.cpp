// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGgp/SshInfo.h"

#include <QJsonDocument>
#include <QJsonObject>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/Error.h"

namespace OrbitGgp {

outcome::result<SshInfo> SshInfo::CreateFromJson(const QByteArray& json) {
  const QJsonDocument doc = QJsonDocument::fromJson(json);

  if (!doc.isObject()) return Error::kUnableToParseJson;
  const QJsonObject obj = doc.object();

  const auto process = [](const QJsonValue& val) -> outcome::result<QString> {
    if (!val.isString()) {
      return Error::kUnableToParseJson;
    } else {
      return val.toString();
    }
  };

  OUTCOME_TRY(host, process(obj.value("host")));
  OUTCOME_TRY(key_path, process(obj.value("keyPath")));
  OUTCOME_TRY(known_hosts_path, process(obj.value("knownHostsPath")));
  OUTCOME_TRY(port, process(obj.value("port")));
  OUTCOME_TRY(user, process(obj.value("user")));

  // The json has the port formatted as a string ("port":"333"), hence this
  // conversion. This is standard the Qt way to check whether the casting worked
  bool ok;
  int port_int = port.toInt(&ok);
  if (!ok) return Error::kUnableToParseJson;

  SshInfo ggp_ssh_info;

  ggp_ssh_info.host = host;
  ggp_ssh_info.key_path = key_path;
  ggp_ssh_info.known_hosts_path = known_hosts_path;
  ggp_ssh_info.port = port_int;
  ggp_ssh_info.user = user;

  return ggp_ssh_info;
}

}  // namespace OrbitGgp
