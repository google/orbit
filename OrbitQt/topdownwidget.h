// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_TOP_DOWN_WIDGET_H_
#define ORBIT_QT_TOP_DOWN_WIDGET_H_

#include <QSortFilterProxyModel>
#include <QString>
#include <QWidget>
#include <memory>

#include "TopDownView.h"
#include "absl/container/flat_hash_set.h"
#include "ui_topdownwidget.h"

class TopDownWidget : public QWidget {
  Q_OBJECT

 public:
  explicit TopDownWidget(QWidget* parent = nullptr)
      : QWidget{parent}, ui_{std::make_unique<Ui::TopDownWidget>()} {
    ui_->setupUi(this);
    connect(ui_->topDownTreeView, &QTreeView::customContextMenuRequested, this,
            &TopDownWidget::onCustomContextMenuRequested);
  }

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

  class HighlightingSortFilterProxyModel : public QSortFilterProxyModel {
   public:
    explicit HighlightingSortFilterProxyModel(QObject* parent)
        : QSortFilterProxyModel{parent} {}

    // Specify a set where this ProxyModel should put internalPointers of
    // indices that match the current filter.
    void SetFilterAcceptedNodeCollectorSet(
        absl::flat_hash_set<void*>* filter_accepted_nodes_collector);

    // Specify a set of internalPointers whose indices should be highlighted.
    void SetNodesToHighlightSet(
        std::unique_ptr<absl::flat_hash_set<void*>> nodes_to_highlight);

    QVariant data(const QModelIndex& index, int role) const override;

   protected:
    bool filterAcceptsRow(int source_row,
                          const QModelIndex& source_parent) const override;

   private:
    mutable absl::flat_hash_set<void*>* filter_accepted_nodes_collector_ =
        nullptr;
    std::unique_ptr<absl::flat_hash_set<void*>> nodes_to_highlight_ = nullptr;
  };

  std::unique_ptr<Ui::TopDownWidget> ui_;
  HighlightingSortFilterProxyModel* proxy_model_ = nullptr;
};

#endif  // ORBIT_QT_TOP_DOWN_WIDGET_H_
