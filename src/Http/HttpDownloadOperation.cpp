// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Http/HttpDownloadOperation.h"

#include <absl/strings/str_format.h>

#include <QNetworkRequest>
#include <QUrl>

#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"

namespace orbit_http_internal {

void HttpDownloadOperation::UpdateState(State state, std::optional<std::string> maybe_error_msg) {
  ORBIT_CHECK((state == State::kError) == maybe_error_msg.has_value());
  state_ = state;

  switch (state) {
    case State::kInitial:
      break;
    case State::kStarted:
      ORBIT_LOG("Started downloading from %s to %s.\n", url_, save_file_path_.string());
      break;
    case State::kCancelled:
      ORBIT_LOG("Cancelled downloading from %s to %s.\n", url_, save_file_path_.string());
      emit finished(state, std::nullopt);
      break;
    case State::kDone:
      ORBIT_LOG("Succeeded to download from %s to %s.\n", url_, save_file_path_.string());
      emit finished(state, std::nullopt);
      break;
    case State::kError:
      ORBIT_LOG("Failed to download from %s to %s:\n%s", url_, save_file_path_.string(),
                maybe_error_msg.value());
      emit finished(state, maybe_error_msg);
      break;
  }
}

void HttpDownloadOperation::OnDownloadFinished() {
  output_.write(reply_->readAll());
  output_.close();

  if (!reply_->error()) {
    UpdateState(State::kDone, std::nullopt);
  } else if (reply_->error() == QNetworkReply::OperationCanceledError) {
    output_.remove();
    UpdateState(State::kCancelled, std::nullopt);
  } else {
    output_.remove();
    UpdateState(State::kError,
                absl::StrFormat("Failed to download: %s\n", reply_->errorString().toStdString()));
  }

  reply_->deleteLater();
  deleteLater();
}

void HttpDownloadOperation::OnDownloadReadyRead() { output_.write(reply_->readAll()); }

void HttpDownloadOperation::Start() {
  ORBIT_CHECK(state_ == State::kInitial);

  output_.setFileName(QString::fromStdString(save_file_path_.string()));
  if (!output_.open(QIODevice::WriteOnly)) {
    UpdateState(State::kError, absl::StrFormat("Failed to open save file: %s\n",
                                               output_.errorString().toStdString()));
    return;
  }

  QNetworkRequest request(QUrl(QString::fromStdString(url_)));
  request.setAttribute(QNetworkRequest::RedirectPolicyAttribute,
                       QNetworkRequest::NoLessSafeRedirectPolicy);
  constexpr const int kMaximumAllowedRedirects = 10;
  request.setMaximumRedirectsAllowed(kMaximumAllowedRedirects);

  reply_ = manager_->get(request);
  connect(reply_, &QNetworkReply::finished, this, &HttpDownloadOperation::OnDownloadFinished);
  connect(reply_, &QNetworkReply::readyRead, this, &HttpDownloadOperation::OnDownloadReadyRead);
  UpdateState(State::kStarted, std::nullopt);

  orbit_base::ImmediateExecutor executor{};
  stop_token_.GetFuture().Then(&executor, [download = QPointer<HttpDownloadOperation>{this}]() {
    if (!download) return;
    QMetaObject::invokeMethod(download, &HttpDownloadOperation::Abort);
  });
}

void HttpDownloadOperation::Abort() {
  if (reply_) reply_->abort();
}

}  // namespace orbit_http_internal
