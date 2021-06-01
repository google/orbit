// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CallTreeViewItemModel.h"

#include <absl/strings/str_format.h>
#include <stddef.h>

#include <QtCore>
#include <algorithm>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "OrbitBase/ThreadConstants.h"

CallTreeViewItemModel::CallTreeViewItemModel(std::unique_ptr<CallTreeView> call_tree_view,
                                             QObject* parent)
    : QAbstractItemModel{parent}, call_tree_view_{std::move(call_tree_view)} {}

QVariant CallTreeViewItemModel::GetDisplayRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<CallTreeNode*>(index.internalPointer());
  auto* thread_item = dynamic_cast<CallTreeThread*>(item);
  auto* function_item = dynamic_cast<CallTreeFunction*>(item);
  if (thread_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        if (thread_item->thread_id() == orbit_base::kAllProcessThreadsTid) {
          return QString::fromStdString(
              thread_item->thread_name().empty()
                  ? "(all threads)"
                  : absl::StrFormat("%s (all threads)", thread_item->thread_name()));
        } else {
          return QString::fromStdString(thread_item->thread_name().empty()
                                            ? std::to_string(thread_item->thread_id())
                                            : absl::StrFormat("%s [%d]", thread_item->thread_name(),
                                                              thread_item->thread_id()));
        }
      case kInclusive:
        return QString::fromStdString(absl::StrFormat(
            "%.2f%% (%llu)", thread_item->GetInclusivePercent(call_tree_view_->sample_count()),
            thread_item->sample_count()));
      case kExclusive:
        return QString::fromStdString(absl::StrFormat(
            "%.2f%% (%llu)", thread_item->GetExclusivePercent(call_tree_view_->sample_count()),
            thread_item->GetExclusiveSampleCount()));
      case kOfParent:
        return QString::fromStdString(absl::StrFormat("%.2f%%", thread_item->GetPercentOfParent()));
    }
  } else if (function_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        return QString::fromStdString(function_item->function_name());
      case kInclusive:
        return QString::fromStdString(absl::StrFormat(
            "%.2f%% (%llu)", function_item->GetInclusivePercent(call_tree_view_->sample_count()),
            function_item->sample_count()));
      case kExclusive:
        return QString::fromStdString(absl::StrFormat(
            "%.2f%% (%llu)", function_item->GetExclusivePercent(call_tree_view_->sample_count()),
            function_item->GetExclusiveSampleCount()));
      case kOfParent:
        return QString::fromStdString(
            absl::StrFormat("%.2f%%", function_item->GetPercentOfParent()));
      case kModule:
        return QString::fromStdString(function_item->GetModuleName());
      case kFunctionAddress:
        return QString::fromStdString(
            absl::StrFormat("%#llx", function_item->function_absolute_address()));
    }
  }
  return QVariant();
}

QVariant CallTreeViewItemModel::GetEditRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<CallTreeNode*>(index.internalPointer());
  auto* thread_item = dynamic_cast<CallTreeThread*>(item);
  auto* function_item = dynamic_cast<CallTreeFunction*>(item);
  if (thread_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        // Threads are sorted by tid, not by name.
        return thread_item->thread_id();
      case kInclusive:
        return thread_item->GetInclusivePercent(call_tree_view_->sample_count());
      case kExclusive:
        return thread_item->GetExclusivePercent(call_tree_view_->sample_count());
      case kOfParent:
        return thread_item->GetPercentOfParent();
    }
  } else if (function_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        return QString::fromStdString(function_item->function_name());
      case kInclusive:
        return function_item->GetInclusivePercent(call_tree_view_->sample_count());
      case kExclusive:
        return function_item->GetExclusivePercent(call_tree_view_->sample_count());
      case kOfParent:
        return function_item->GetPercentOfParent();
      case kModule:
        return QString::fromStdString(function_item->GetModuleName());
      case kFunctionAddress:
        return static_cast<qulonglong>(function_item->function_absolute_address());
    }
  }
  return QVariant();
}

QVariant CallTreeViewItemModel::GetToolTipRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<CallTreeNode*>(index.internalPointer());
  auto* function_item = dynamic_cast<CallTreeFunction*>(item);
  if (function_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        return QString::fromStdString(function_item->function_name());
      case kModule:
        return QString::fromStdString(function_item->module_path());
    }
  }
  return QVariant();
}

QVariant CallTreeViewItemModel::GetModulePathRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<CallTreeNode*>(index.internalPointer());
  auto* function_item = dynamic_cast<CallTreeFunction*>(item);
  if (function_item != nullptr) {
    return QString::fromStdString(function_item->module_path());
  }
  return QVariant();
}

QVariant CallTreeViewItemModel::GetModuleBuildIdRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<CallTreeNode*>(index.internalPointer());
  auto* function_item = dynamic_cast<CallTreeFunction*>(item);
  if (function_item != nullptr) {
    return QString::fromStdString(function_item->module_build_id());
  }
  return QVariant();
}

QVariant CallTreeViewItemModel::data(const QModelIndex& index, int role) const {
  if (!index.isValid()) {
    return QVariant();
  }
  switch (role) {
    case Qt::DisplayRole:
      return GetDisplayRoleData(index);
    // The value returned when role == Qt::EditRole is used for sorting.
    case Qt::EditRole:
      return GetEditRoleData(index);
    // When role == Qt::ToolTipRole more detailed information than it's shown is returned.
    case Qt::ToolTipRole:
      return GetToolTipRoleData(index);
    case kModulePathRole:
      return GetModulePathRoleData(index);
    case kModuleBuildIdRole:
      return GetModuleBuildIdRoleData(index);
  }
  return QVariant();
}

Qt::ItemFlags CallTreeViewItemModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }
  return QAbstractItemModel::flags(index);
}

QVariant CallTreeViewItemModel::headerData(int section, Qt::Orientation orientation,
                                           int role) const {
  if (orientation != Qt::Horizontal) {
    return QVariant();
  }
  switch (role) {
    case Qt::DisplayRole: {
      switch (section) {
        case Columns::kThreadOrFunction:
          return "Thread / Function";
        case Columns::kInclusive:
          return "Inclusive";
        case Columns::kExclusive:
          return "Exclusive";
        case Columns::kOfParent:
          return "Of parent";
        case Columns::kModule:
          return "Module";
        case Columns::kFunctionAddress:
          return "Function address";
      }
    } break;
    case Qt::InitialSortOrderRole: {
      switch (section) {
        case Columns::kThreadOrFunction:
          return Qt::AscendingOrder;
        case Columns::kInclusive:
          return Qt::DescendingOrder;
        case Columns::kExclusive:
          return Qt::DescendingOrder;
        case Columns::kOfParent:
          return Qt::DescendingOrder;
        case Columns::kModule:
          return Qt::AscendingOrder;
        case Columns::kFunctionAddress:
          return Qt::AscendingOrder;
      }
    } break;
  }
  return QVariant();
}

QModelIndex CallTreeViewItemModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  CallTreeNode* parent_item;
  if (!parent.isValid()) {
    parent_item = call_tree_view_.get();
  } else {
    parent_item = static_cast<CallTreeNode*>(parent.internalPointer());
  }

  const std::vector<const CallTreeNode*>& siblings = parent_item->children();
  if (row < 0 || static_cast<size_t>(row) >= siblings.size()) {
    return QModelIndex();
  }
  const CallTreeNode* item = siblings[row];
  return createIndex(row, column, const_cast<CallTreeNode*>(item));
}

QModelIndex CallTreeViewItemModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) {
    return QModelIndex();
  }

  auto* child_item = static_cast<CallTreeNode*>(index.internalPointer());
  const CallTreeNode* item = child_item->parent();
  if (item == call_tree_view_.get()) {
    return QModelIndex();
  }

  const CallTreeNode* parent_item = item->parent();
  if (parent_item == nullptr) {
    return createIndex(0, 0, const_cast<CallTreeNode*>(item));
  }

  const std::vector<const CallTreeNode*>& siblings = parent_item->children();
  int row = static_cast<int>(
      std::distance(siblings.begin(), std::find(siblings.begin(), siblings.end(), item)));
  return createIndex(row, 0, const_cast<CallTreeNode*>(item));
}

int CallTreeViewItemModel::rowCount(const QModelIndex& parent) const {
  if (parent.column() > 0) {
    return 0;
  }
  if (!parent.isValid()) {
    return call_tree_view_->child_count();
  }
  auto* item = static_cast<CallTreeNode*>(parent.internalPointer());
  return item->child_count();
}

int CallTreeViewItemModel::columnCount(const QModelIndex& /*parent*/) const { return kColumnCount; }
