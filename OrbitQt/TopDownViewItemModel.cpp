// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "TopDownViewItemModel.h"

TopDownViewItemModel::TopDownViewItemModel(std::unique_ptr<TopDownView> top_down_view,
                                           QObject* parent)
    : QAbstractItemModel{parent}, top_down_view_{std::move(top_down_view)} {}

QVariant TopDownViewItemModel::GetDisplayRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<TopDownNode*>(index.internalPointer());
  auto thread_item = dynamic_cast<TopDownThread*>(item);
  auto function_item = dynamic_cast<TopDownFunction*>(item);
  if (thread_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        if (thread_item->thread_id() == SamplingProfiler::kAllThreadsFakeTid) {
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
            "%.2f%% (%llu)", thread_item->GetInclusivePercent(top_down_view_->sample_count()),
            thread_item->sample_count()));
      case kOfParent:
        return QString::fromStdString(absl::StrFormat("%.2f%%", thread_item->GetPercentOfParent()));
    }
  } else if (function_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        return QString::fromStdString(function_item->function_name());
      case kInclusive:
        return QString::fromStdString(absl::StrFormat(
            "%.2f%% (%llu)", function_item->GetInclusivePercent(top_down_view_->sample_count()),
            function_item->sample_count()));
      case kExclusive:
        return QString::fromStdString(absl::StrFormat(
            "%.2f%% (%llu)", function_item->GetExclusivePercent(top_down_view_->sample_count()),
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

QVariant TopDownViewItemModel::GetEditRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<TopDownNode*>(index.internalPointer());
  auto thread_item = dynamic_cast<TopDownThread*>(item);
  auto function_item = dynamic_cast<TopDownFunction*>(item);
  if (thread_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        // Threads are sorted by tid, not by name.
        return thread_item->thread_id();
      case kInclusive:
        return static_cast<qulonglong>(thread_item->sample_count());
      case kOfParent:
        return static_cast<qulonglong>(thread_item->GetPercentOfParent());
    }
  } else if (function_item != nullptr) {
    switch (index.column()) {
      case kThreadOrFunction:
        return QString::fromStdString(function_item->function_name());
      case kInclusive:
        return static_cast<qulonglong>(function_item->sample_count());
      case kExclusive:
        return static_cast<qulonglong>(function_item->GetExclusiveSampleCount());
      case kOfParent:
        return static_cast<qulonglong>(function_item->GetPercentOfParent());
      case kModule:
        return QString::fromStdString(function_item->GetModuleName());
      case kFunctionAddress:
        return static_cast<qulonglong>(function_item->function_absolute_address());
    }
  }
  return QVariant();
}

QVariant TopDownViewItemModel::GetToolTipRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<TopDownNode*>(index.internalPointer());
  auto function_item = dynamic_cast<TopDownFunction*>(item);
  if (function_item != nullptr) {
    switch (index.column()) {
      case kModule:
        return QString::fromStdString(function_item->module_path());
    }
  }
  return QVariant();
}

QVariant TopDownViewItemModel::GetModulePathRoleData(const QModelIndex& index) const {
  CHECK(index.isValid());
  auto* item = static_cast<TopDownNode*>(index.internalPointer());
  auto function_item = dynamic_cast<TopDownFunction*>(item);
  if (function_item != nullptr) {
    return QString::fromStdString(function_item->module_path());
  }
  return QVariant();
}

QVariant TopDownViewItemModel::data(const QModelIndex& index, int role) const {
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
  }
  return QVariant();
}

Qt::ItemFlags TopDownViewItemModel::flags(const QModelIndex& index) const {
  if (!index.isValid()) {
    return Qt::NoItemFlags;
  }
  return QAbstractItemModel::flags(index);
}

QVariant TopDownViewItemModel::headerData(int section, Qt::Orientation orientation,
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

QModelIndex TopDownViewItemModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  TopDownNode* parent_item;
  if (!parent.isValid()) {
    parent_item = top_down_view_.get();
  } else {
    parent_item = static_cast<TopDownInternalNode*>(parent.internalPointer());
  }

  const std::vector<const TopDownNode*>& siblings = parent_item->children();
  if (row < 0 || static_cast<size_t>(row) >= siblings.size()) {
    return QModelIndex();
  }
  const TopDownNode* item = siblings[row];
  return createIndex(row, column, const_cast<TopDownNode*>(item));
}

QModelIndex TopDownViewItemModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) {
    return QModelIndex();
  }

  auto* child_item = static_cast<TopDownNode*>(index.internalPointer());
  const TopDownNode* item = child_item->parent();
  if (item == top_down_view_.get()) {
    return QModelIndex();
  }

  const TopDownNode* parent_item = item->parent();
  if (parent_item == nullptr) {
    return createIndex(0, 0, const_cast<TopDownNode*>(item));
  }

  const std::vector<const TopDownNode*>& siblings = parent_item->children();
  int row = static_cast<int>(
      std::distance(siblings.begin(), std::find(siblings.begin(), siblings.end(), item)));
  return createIndex(row, 0, const_cast<TopDownNode*>(item));
}

int TopDownViewItemModel::rowCount(const QModelIndex& parent) const {
  if (parent.column() > 0) {
    return 0;
  }
  if (!parent.isValid()) {
    return top_down_view_->child_count();
  }
  auto* item = static_cast<TopDownNode*>(parent.internalPointer());
  return item->child_count();
}

int TopDownViewItemModel::columnCount(const QModelIndex& /*parent*/) const { return kColumnCount; }
