// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP_REMOTE_SYMBOL_STORE_DOWNLOAD_MANAGER_H
#define HTTP_REMOTE_SYMBOL_STORE_DOWNLOAD_MANAGER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <memory>

#include "Http/HttpDownloadOperation.h"
#include "OrbitBase/AnyInvocable.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"

namespace orbit_http {

class HttpDownloadManager : public QObject {
  Q_OBJECT
 public:
  explicit HttpDownloadManager(QObject* parent = nullptr) : QObject(parent) {}

  [[nodiscard]] orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> Download(
      std::string url, std::filesystem::path save_file_path, orbit_base::StopToken stop_token);

 private:
  std::vector<orbit_http_internal::HttpDownloadOperation*> download_operations_;
  QNetworkAccessManager manager_;
};

}  // namespace orbit_http

#endif  // HTTP_REMOTE_SYMBOL_STORE_DOWNLOAD_MANAGER_H
