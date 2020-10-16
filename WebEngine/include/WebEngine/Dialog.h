// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef WEB_ENGINE_DIALOG_H_
#define WEB_ENGINE_DIALOG_H_

#include <QDialog>
#include <QGridLayout>
#include <QUrl>
#include <QWebChannel>
#include <QWebEnginePage>
#include <QWebEngineProfile>
#include <QWebEngineView>
#include <optional>

#include "WebEngine/View.h"

namespace web_engine {

/* web_engine::Dialog is a dialog window with a full screen web view.

   You can use the `Load` member function to load a webpage.

   The loaded page can use JavaScript's `window.close()` to close this
   dialog.

   Be aware that constructing that object can take around 100 to 200
   milliseconds, when the Chromium engine needs to be fired
   up first. Depending on the use case that delay might be noticable to
   the user. In that case it helps to construct the dialog upfront and
   hide it until it's needed.

   Example:
   QWebEngineProfile profile{};
   web_engine::Dialog dialog{&profile};
   dialog.Load(QUrl{"https://www.google.com/"});
   dialog.exec();
*/
class Dialog : public QDialog {
  Q_OBJECT

 public:
  explicit Dialog(QWebEngineProfile* profile, QWidget* parent = nullptr);

  QWebEngineView* GetWebEngineView() { return &view_; }
  const QWebEngineView* GetWebEngineView() const { return &view_; }

  QWebEnginePage* GetWebEnginePage() { return &page_; }
  const QWebEnginePage* GetWebEnginePage() const { return &page_; }

  void SetWebChannel(QWebChannel* channel) { page_.setWebChannel(channel); }
  void SetView(web_engine::View* view) { SetWebChannel(view->GetWebChannel()); }

  void SetWindowTitle(QString title) { setWindowTitle(title); }

  void Load(QUrl url) { page_.load(url); }

 private:
  QGridLayout layout_;
  QWebEngineView view_;
  QWebEnginePage page_;
};
}  // namespace web_engine

#endif  // WEB_ENGINE_DIALOG_H_