// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "deploymentconfigurations.h"

#include <QCoreApplication>
#include <QDir>

static const char* const kSignatureExtension = ".asc";
static const char* const kPackageNameTemplate = "OrbitProfiler-%1.deb";
static const char* const kCollectorSubdirectory = "collector";

namespace OrbitQt {

SignedDebianPackageDeployment::SignedDebianPackageDeployment() {
  const auto version = []() {
    auto ver = QCoreApplication::applicationVersion();
    if (!ver.isEmpty() && ver[0] == 'v') {
      ver = ver.mid(1);
    }
    return ver;
  }();

  const auto collector_directory =
      QDir{QDir{QCoreApplication::applicationDirPath()}.absoluteFilePath(kCollectorSubdirectory)};

  const auto deb_path =
      collector_directory.absoluteFilePath(QString(kPackageNameTemplate).arg(version));

  path_to_package = deb_path.toStdString();
  path_to_signature = (deb_path + kSignatureExtension).toStdString();
}

}  // namespace OrbitQt
