// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef HTTP_HTTP_DOWNLOAD_OPERATION_H
#define HTTP_HTTP_DOWNLOAD_OPERATION_H

#include <QFile>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QObject>
#include <filesystem>

#include "OrbitBase/StopToken.h"

namespace orbit_http_internal {

class HttpDownloadOperation : public QObject {
  Q_OBJECT
 public:
  explicit HttpDownloadOperation(std::string url, std::filesystem::path save_file_path,
                                 orbit_base::StopToken stop_token, QNetworkAccessManager* manager)
      : QObject(nullptr),
        url_(std::move(url)),
        save_file_path_(std::move(save_file_path)),
        stop_token_(std::move(stop_token)),
        manager_(manager) {}

  enum class State {
    kInitial,
    kStarted,
    kCancelled,
    kDone,
    kError,
  };

  void Start();
  void Abort();

 signals:
  void finished(State state, std::optional<std::string> maybe_error_msg);

 private slots:
  void OnDownloadFinished();
  void OnDownloadReadyRead();

 private:
  void UpdateState(State state, std::optional<std::string> maybe_error_msg);

  State state_ = State::kInitial;

  std::string url_;
  std::filesystem::path save_file_path_;
  orbit_base::StopToken stop_token_;
  QNetworkAccessManager* manager_;
  QNetworkReply* reply_;
  QFile output_;
};

}  // namespace orbit_http_internal

#endif  // HTTP_HTTP_DOWNLOAD_OPERATION_H