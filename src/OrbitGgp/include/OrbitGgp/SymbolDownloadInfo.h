// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_SYMBOL_DOWNLOAD_INFO_H_
#define ORBIT_GGP_SYMBOL_DOWNLOAD_INFO_H_

#include <QByteArray>
#include <QString>
#include <QVector>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

struct SymbolDownloadInfo {
  QString file_id;
  QString url;

  static ErrorMessageOr<QVector<SymbolDownloadInfo>> GetListFromJson(const QByteArray& json);
};

}  // namespace orbit_ggp

#endif  // ORBIT_GGP_SYMBOL_DOWNLOAD_INFO_H_
