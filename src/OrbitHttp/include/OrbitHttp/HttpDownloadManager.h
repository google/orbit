// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_HTTP_REMOTE_SYMBOL_STORE_DOWNLOAD_MANAGER_H
#define ORBIT_HTTP_REMOTE_SYMBOL_STORE_DOWNLOAD_MANAGER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <memory>
#include <queue>

#include "OrbitBase/AnyInvocable.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Promise.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "OrbitHttp/HttpDownloadOperation.h"

namespace orbit_http {

class HttpDownloadManager : public QObject {
  Q_OBJECT
 public:
  explicit HttpDownloadManager();

  ~HttpDownloadManager() override;

  [[nodiscard]] orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> Download(
      const std::string& url, const std::filesystem::path& save_file_path,
      orbit_base::StopToken stop_token);

 private:
  struct HttpDownloadOperationMetadata {
    std::string url;
    std::filesystem::path save_file_path;
    orbit_base::StopToken stop_token;
    orbit_base::Promise<ErrorMessageOr<orbit_base::CanceledOr<void>>> promise;
  };

  void DoDownload(HttpDownloadOperationMetadata metadata);

  std::queue<HttpDownloadOperationMetadata> waiting_download_operations_;
  orbit_http_internal::HttpDownloadOperation* current_download_operation_ = nullptr;
  std::unique_ptr<QNetworkAccessManager> manager_;
};

}  // namespace orbit_http

#endif  // ORBIT_HTTP_REMOTE_SYMBOL_STORE_DOWNLOAD_MANAGER_H
