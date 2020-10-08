
// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WebEngine/BlockAllUrlRequestInterceptor.h"

namespace web_engine {
void BlockAllUrlRequestInterceptor::interceptRequest(QWebEngineUrlRequestInfo& info) {
  // Block all non-qrc:/// URLs
  info.block(info.requestUrl().scheme() != QStringLiteral("qrc"));
}
}  // namespace web_engine