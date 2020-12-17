// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_CALL_TREE_WIDGET_H_
#define ORBIT_QT_CALL_TREE_WIDGET_H_

#include <QIdentityProxyModel>
#include <QSortFilterProxyModel>
#include <QString>
#include <QStyledItemDelegate>
#include <QWidget>
#include <memory>

#include "CallTreeView.h"
#include "CallTreeViewItemModel.h"
#include "absl/strings/str_split.h"
#include "ui_CallTreeWidget.h"

class OrbitApp;

class CallTreeWidget : public QWidget {
  Q_OBJECT

 public:
  explicit CallTreeWidget(QWidget* parent = nullptr);

  void Initialize(OrbitApp* app) { app_ = app; }
  void Deinitialize() {
    ClearCallTreeView();
    app_ = nullptr;
  }

  void SetTopDownView(std::unique_ptr<CallTreeView> top_down_view);
  void SetBottomUpView(std::unique_ptr<CallTreeView> bottom_up_view);
  void ClearCallTreeView();

 protected:
  void resizeEvent(QResizeEvent* event) override;

 private slots:
  void onCopyKeySequencePressed();
  void onCustomContextMenuRequested(const QPoint& point);
  void onSearchLineEditTextEdited(const QString& text);

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
    static const QColor kHighlightColor;

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

  // Displays progress bars in the "Inclusive" column as a means to better visualize the percentage
  // in each cell and the distribution of samples in the tree.
  class ProgressBarItemDelegate : public QStyledItemDelegate {
   public:
    explicit ProgressBarItemDelegate(QObject* parent) : QStyledItemDelegate(parent) {}

    void paint(QPainter* painter, const QStyleOptionViewItem& option,
               const QModelIndex& index) const override;
  };

  void SetCallTreeView(std::unique_ptr<CallTreeView> call_tree_view,
                       std::unique_ptr<QIdentityProxyModel> hide_values_proxy_model);

  void ResizeColumnsIfNecessary();

  std::unique_ptr<Ui::CallTreeWidget> ui_;
  OrbitApp* app_ = nullptr;
  std::unique_ptr<CallTreeViewItemModel> model_;
  std::unique_ptr<QIdentityProxyModel> hide_values_proxy_model_;
  std::unique_ptr<HighlightCustomFilterSortFilterProxyModel> search_proxy_model_;
  std::unique_ptr<HookedIdentityProxyModel> hooked_proxy_model_;

  enum class ColumnResizingState {
    kInitial = 0,        // CallTreeWidget hasn't got its first size set yet
    kWidgetSizeSet = 1,  // CallTreeWidget has got its first size, resize the columns when possible
    kDone = 2            // The columns have been resized (once)
  };
  ColumnResizingState column_resizing_state_ = ColumnResizingState::kInitial;
};

#endif  // ORBIT_QT_CALL_TREE_WIDGET_H_
