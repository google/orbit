// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP_HTTP_DOWNLOAD_MANAGER_H
#define HTTP_HTTP_DOWNLOAD_MANAGER_H

#include <QNetworkAccessManager>
#include <QObject>
#include <QPointer>
#include <memory>

#include "Http/HttpDownloadOperation.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopToken.h"
#include "RemoteSymbolProvider/DownloadManagerInterface.h"

namespace orbit_http {

class HttpDownloadManager : public QObject,
                            public orbit_remote_symbol_provider::DownloadManagerInterface {
  Q_OBJECT
 public:
  explicit HttpDownloadManager(QObject* parent = nullptr) : QObject(parent) {}

  ~HttpDownloadManager() override {
    for (const auto& download_operation :
         findChildren<orbit_http_internal::HttpDownloadOperation*>()) {
      download_operation->Abort();
    }
  }

  [[nodiscard]] orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> Download(
      std::string url, std::filesystem::path save_file_path,
      orbit_base::StopToken stop_token) override;

 private:
  QNetworkAccessManager manager_;
};

}  // namespace orbit_http

#endif  // HTTP_HTTP_DOWNLOAD_MANAGER_H
