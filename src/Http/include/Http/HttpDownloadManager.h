// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP_HTTP_DOWNLOAD_MANAGER_H
#define HTTP_HTTP_DOWNLOAD_MANAGER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QString>
#include <filesystem>
#include <string>

#include "Http/DownloadManager.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"

namespace orbit_http {

class HttpDownloadManager : public QObject, public DownloadManager {
 public:
  explicit HttpDownloadManager(QObject* parent = nullptr) : QObject(parent) {}
  ~HttpDownloadManager() override;

  [[nodiscard]] orbit_base::Future<
      ErrorMessageOr<orbit_base::CanceledOr<orbit_base::NotFoundOr<void>>>>
  Download(std::string url, std::filesystem::path save_file_path,
           orbit_base::StopToken stop_token) override;

 private:
  QNetworkAccessManager manager_;
};

}  // namespace orbit_http

#endif  // HTTP_HTTP_DOWNLOAD_MANAGER_H
