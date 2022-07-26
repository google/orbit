// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GGP_SYMBOL_DOWNLOAD_INFO_H_
#define ORBIT_GGP_SYMBOL_DOWNLOAD_INFO_H_

#include <QByteArray>
#include <QString>
#include <vector>

#include "OrbitBase/Result.h"

namespace orbit_ggp {

struct SymbolDownloadInfo {
  QString file_id;
  QString url;

  [[nodiscard]] static ErrorMessageOr<std::vector<SymbolDownloadInfo>> GetListFromJson(
      const QByteArray& json);

  [[nodiscard]] friend bool operator==(const SymbolDownloadInfo& lhs,
                                       const SymbolDownloadInfo& rhs) {
    return (lhs.file_id == rhs.file_id) && (lhs.url == rhs.url);
  }

  [[nodiscard]] friend bool operator!=(const SymbolDownloadInfo& lhs,
                                       const SymbolDownloadInfo& rhs) {
    return !(lhs == rhs);
  }
};

}  // namespace orbit_ggp

#endif  // ORBIT_GGP_SYMBOL_DOWNLOAD_INFO_H_
