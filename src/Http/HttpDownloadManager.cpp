// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Http/HttpDownloadManager.h"

#include <QList>
#include <optional>
#include <type_traits>
#include <utility>

#include "HttpDownloadOperation.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Promise.h"

namespace orbit_http {
using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_base::NotFoundOr;
using orbit_base::Promise;
using orbit_base::StopToken;

HttpDownloadManager::~HttpDownloadManager() {
  for (const auto& download_operation : findChildren<HttpDownloadOperation*>()) {
    download_operation->Abort();
  }
}

Future<ErrorMessageOr<CanceledOr<NotFoundOr<void>>>> HttpDownloadManager::Download(
    std::string url, std::filesystem::path save_file_path, orbit_base::StopToken stop_token) {
  Promise<ErrorMessageOr<CanceledOr<NotFoundOr<void>>>> promise;
  auto future = promise.GetFuture();

  auto current_download_operation = new HttpDownloadOperation{
      std::move(url), std::move(save_file_path), std::move(stop_token), &manager_, this};

  auto finish_handler = [current_download_operation, promise = std::move(promise)](
                            HttpDownloadOperation::State state,
                            std::optional<std::string> maybe_error_msg) mutable {
    if (promise.HasResult()) return;

    current_download_operation->deleteLater();
    switch (state) {
      case HttpDownloadOperation::State::kError:
        promise.SetResult(ErrorMessage{std::move(maybe_error_msg.value())});
        break;
      case HttpDownloadOperation::State::kCancelled:
        promise.SetResult(orbit_base::Canceled{});
        break;
      case HttpDownloadOperation::State::kNotFound:
        promise.SetResult(orbit_base::NotFound{""});
        break;
      case HttpDownloadOperation::State::kDone:
        promise.SetResult(outcome::success());
        break;
      case HttpDownloadOperation::State::kStarted:
      case HttpDownloadOperation::State::kInitial:
        ORBIT_UNREACHABLE();
    }
  };

  QObject::connect(current_download_operation, &HttpDownloadOperation::finished,
                   std::move(finish_handler));

  current_download_operation->Start();

  return future;
}

}  // namespace orbit_http