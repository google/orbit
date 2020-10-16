// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "WebEngine/Dialog.h"

#include <QMargins>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <optional>

#include "JsonTransport.h"
#include "OrbitBase/Logging.h"

namespace web_engine {

Dialog::Dialog(QWebEngineProfile* profile, QWidget* parent) : QDialog(parent), page_(profile) {
  const QSize default_dialog_size{800, 600};
  resize(default_dialog_size);
  setWindowModality(Qt::ApplicationModal);

  layout_.setContentsMargins(QMargins{});
  setLayout(&layout_);
  layout_.addWidget(&view_);

  view_.setPage(&page_);

  QObject::connect(&page_, &QWebEnginePage::windowCloseRequested, this, &QDialog::close);
}

}  // namespace web_engine