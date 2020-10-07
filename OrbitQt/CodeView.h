// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CODE_VIEW_H_
#define ORBIT_QT_CODE_VIEW_H_

#include <QObject>
#include <QVector>

#include "DisassemblyReport.h"
#include "WebEngine/Dialog.h"

namespace orbit_qt {

/* CodeView is a wrapper around web_engine::Dialog for view highlighted code.

   The class exposes three properties (title, source_code, and language) which can
   be read by the corresponding web part of CodeView. Use SetCode
   to change any of these three values. The web part will also be notified and
   will reload accordingly. "language" is an identifier which needs to be recognized
   by PrismJS to identify the syntax highlighting language. Currently we support
   "c", "cpp", and "x86asm". Any other value will lead to no highlighting.

   The first and second constructor argument (profile and web_socket_listen_port)
   are passed through to QWebEngine::Dialog. Check out this class's documentation
   for more details.

   Example:
   QWebEngineProfile profile;
   CodeView view{&profile, std::nullopt};
   view.SetCode("example.cpp", "int main() {}", "cpp");
   view.exec(); // Will open the dialog and block until closed.
*/
class CodeView : public QObject {
  Q_OBJECT

 public:
  explicit CodeView(std::optional<int> web_socket_listen_port, QObject* parent = nullptr);

  Q_PROPERTY(QString title MEMBER title_ NOTIFY sourceCodeChanged)
  Q_PROPERTY(QString source_code MEMBER source_code_ NOTIFY sourceCodeChanged)
  Q_PROPERTY(QString language MEMBER language_ NOTIFY sourceCodeChanged)
  Q_PROPERTY(bool line_numbers_enabled MEMBER line_numbers_enabled_ NOTIFY sourceCodeChanged)
  Q_PROPERTY(bool heatmap_enabled MEMBER heatmap_enabled_ NOTIFY sourceCodeChanged)

  // These vector properties expose sampling data to the javascript context. The vector index
  // refers to the line number in the property source_code.
  Q_PROPERTY(QVector<int> hit_counts READ GetHitCountsPerLine NOTIFY sourceCodeChanged)
  Q_PROPERTY(QVector<double> hit_ratios READ GetHitRatiosPerLine NOTIFY sourceCodeChanged)
  Q_PROPERTY(
      QVector<double> total_hit_ratios READ GetTotalHitRatiosPerLine NOTIFY sourceCodeChanged)

  void SetCode(QString title, QString new_source, QString language);
  void SetCode(QString title, QString new_source, QString language,
               DisassemblyReport disassembly_report);

  QVector<int> GetHitCountsPerLine() const;
  QVector<double> GetHitRatiosPerLine() const;
  QVector<double> GetTotalHitRatiosPerLine() const;

  void SetLineNumbersEnabled(bool enabled) {
    line_numbers_enabled_ = enabled;
    emit sourceCodeChanged();
  }
  void SetHeatmapEnabled(bool enabled) {
    heatmap_enabled_ = enabled;
    emit sourceCodeChanged();
  }

  web_engine::View* GetWebEngineView() { return &web_engine_view_; }
  const web_engine::View* GetWebEngineView() const { return &web_engine_view_; }

 signals:
  void sourceCodeChanged();
  void loadingFinished();

 private:
  web_engine::View web_engine_view_;

  QString title_;
  QString source_code_;
  QString language_;
  bool line_numbers_enabled_ = false;
  bool heatmap_enabled_ = false;
  std::optional<DisassemblyReport> disassembly_report_;
};
}  // namespace orbit_qt

#endif  // ORBIT_QT_CODE_VIEW_H_