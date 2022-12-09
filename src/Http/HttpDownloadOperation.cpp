// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "HttpDownloadOperation.h"

#include <absl/strings/str_format.h>

#include <QIODevice>
#include <QMetaObject>
#include <QNetworkRequest>
#include <QUrl>
#include <algorithm>

#include "OrbitBase/Future.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"

namespace orbit_http {

void HttpDownloadOperation::UpdateState(State state, std::optional<std::string> maybe_error_msg) {
  ORBIT_CHECK((state == State::kError) == maybe_error_msg.has_value());
  state_ = state;

  const std::string download_details =
      absl::StrFormat("from %s to %s", url_, save_file_path_.string());

  switch (state) {
    case State::kInitial:
      break;
    case State::kStarted:
      ORBIT_LOG("Started downloading %s.\n", download_details);
      break;
    case State::kCancelled:
      ORBIT_LOG("Cancelled downloading %s.\n", download_details);
      emit finished(state, std::nullopt);
      break;
    case State::kDone:
      ORBIT_LOG("Succeeded to download %s.\n", download_details);
      emit finished(state, std::nullopt);
      break;
    case State::kNotFound:
      ORBIT_LOG("Remote file %s not found.", url_);
      emit finished(state, std::nullopt);
      break;
    case State::kError:
      ORBIT_LOG("Failed to download %s:\n%s", download_details, maybe_error_msg.value());
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
  } else if (reply_->error() == QNetworkReply::ContentNotFoundError) {
    output_.remove();
    UpdateState(State::kNotFound, std::nullopt);
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

}  // namespace orbit_http
