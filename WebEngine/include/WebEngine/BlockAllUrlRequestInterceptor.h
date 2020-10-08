// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_ENGINE_BLOCK_ALL_URL_REQUEST_INTERCEPTOR_H_
#define WEB_ENGINE_BLOCK_ALL_URL_REQUEST_INTERCEPTOR_H_

#include <QWebEngineUrlRequestInterceptor>

namespace web_engine {

/* This interceptor  can be associated with an instance of QWebEngine (Chromium),
   where it will block all non-local requests, i.e. all URLs that
   don't start with qrc:///.

   This can be used as a security precaution to prevent the Chromium
   instance from accidentally loading content from internet.

   Example:
   QWebEngineProfile profile{};
   web_engine::BlockAllUrlRequestInterceptor interceptor{};
   profile.setUrlRequestInterceptor(&interceptor);

*/
class BlockAllUrlRequestInterceptor : public QWebEngineUrlRequestInterceptor {
  Q_OBJECT

 public:
  using QWebEngineUrlRequestInterceptor::QWebEngineUrlRequestInterceptor;

  void interceptRequest(QWebEngineUrlRequestInfo&) override;
};

}  // namespace web_engine

#endif  // WEB_ENGINE_BLOCK_ALL_URL_REQUEST_INTERCEPTOR_H_