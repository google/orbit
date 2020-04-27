// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBITGGP_GGP_SSH_INFO_H_
#define ORBITGGP_GGP_SSH_INFO_H_

#include <QByteArray>
#include <QString>
#include <optional>

struct GgpSshInfo {
  QString host;
  QString key_path;
  QString known_hosts_path;
  int port;
  QString user;

  static std::optional<GgpSshInfo> CreateFromJson(const QByteArray& json);
};

#endif  // ORBITGGP_GGP_SSH_INFO_H_
