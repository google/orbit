// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TOP_DOWN_WIDGET_H_
#define ORBIT_QT_TOP_DOWN_WIDGET_H_

#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QWidget>
#include <memory>

#include "TopDownView.h"
#include "TopDownViewItemModel.h"
#include "ui_topdownwidget.h"

class OrbitApp;

class TopDownWidget : public QWidget {
  Q_OBJECT

 public:
  explicit TopDownWidget(QWidget* parent = nullptr)
      : QWidget{parent}, ui_{std::make_unique<Ui::TopDownWidget>()} {
    ui_->setupUi(this);
    connect(ui_->topDownTreeView, &QTreeView::customContextMenuRequested, this,
            &TopDownWidget::onCustomContextMenuRequested);
  }

  void Initialize(OrbitApp* app) { app_ = app; }

  void SetTopDownView(std::unique_ptr<TopDownView> top_down_view);

 private slots:
  void onCustomContextMenuRequested(const QPoint& point);
  void on_searchLineEdit_textEdited(const QString& text);

 private:
  static const QString kActionExpandRecursively;
  static const QString kActionCollapseRecursively;
  static const QString kActionCollapseChildrenRecursively;
  static const QString kActionExpandAll;
  static const QString kActionCollapseAll;
  static const QString kActionLoadSymbols;
  static const QString kActionSelect;
  static const QString kActionDeselect;
  static const QString kActionDisassembly;
  static const QString kActionCopySelection;

  class HighlightCustomFilterSortFilterProxyModel : public QSortFilterProxyModel {
   public:
    explicit HighlightCustomFilterSortFilterProxyModel(QObject* parent)
        : QSortFilterProxyModel{parent} {}

    void SetFilter(std::string_view filter) {
      lowercase_filter_tokens_ =
          absl::StrSplit(absl::AsciiStrToLower(filter), ' ', absl::SkipWhitespace());
    }

    static const int kMatchesCustomFilterRole = Qt::UserRole;

    QVariant data(const QModelIndex& index, int role) const override;

   private:
    bool ItemMatchesFilter(const QModelIndex& index) const;

    std::vector<std::string> lowercase_filter_tokens_;
  };

  class HookedIdentityProxyModel : public QIdentityProxyModel {
   public:
    HookedIdentityProxyModel(OrbitApp* app, QObject* parent)
        : QIdentityProxyModel{parent}, app_{app} {}

    QVariant data(const QModelIndex& index, int role) const override;

   private:
    OrbitApp* app_;
  };

  std::unique_ptr<Ui::TopDownWidget> ui_;
  OrbitApp* app_ = nullptr;
  std::unique_ptr<TopDownViewItemModel> model_;
  std::unique_ptr<HighlightCustomFilterSortFilterProxyModel> search_proxy_model_;
  std::unique_ptr<HookedIdentityProxyModel> hooked_proxy_model_;
};

#endif  // ORBIT_QT_TOP_DOWN_WIDGET_H_
