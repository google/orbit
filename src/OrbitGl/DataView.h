// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_DATA_VIEW_H_
#define ORBIT_GL_DATA_VIEW_H_

#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "DataViewTypes.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "absl/container/flat_hash_set.h"

class OrbitApp;

enum class RefreshMode { kOnFilter, kOnSort, kOther };

class DataView {
 public:
  enum class SortingOrder {
    kAscending = 0,
    kDescending = 1,
  };

  struct Column {
    Column() : Column{"", .0f, SortingOrder::kAscending} {}
    Column(std::string header, float ratio, SortingOrder initial_order)
        : header{std::move(header)}, ratio{ratio}, initial_order{initial_order} {}
    std::string header;
    float ratio;
    SortingOrder initial_order;
  };

  explicit DataView(DataViewType type, OrbitApp* app)
      : update_period_ms_(-1), type_(type), app_{app} {}

  virtual ~DataView() = default;

  virtual void SetAsMainInstance() {}
  virtual const std::vector<Column>& GetColumns() = 0;
  virtual bool IsSortingAllowed() { return true; }
  virtual int GetDefaultSortingColumn() { return 0; }
  virtual std::vector<std::string> GetContextMenu(int clicked_index,
                                                  const std::vector<int>& selected_indices);
  virtual size_t GetNumElements() { return indices_.size(); }
  virtual std::string GetValue(int /*a_Row*/, int /*a_Column*/) { return ""; }
  virtual std::string GetToolTip(int /*a_Row*/, int /*a_Column*/) { return ""; }

  // Called from UI layer.
  void OnFilter(const std::string& filter);
  // Called internally to set the filter string programmatically in the UI.
  void SetUiFilterString(const std::string& filter);
  // Filter callback set from UI layer.
  using FilterCallback = std::function<void(const std::string&)>;
  void SetUiFilterCallback(FilterCallback callback) { filter_callback_ = std::move(callback); }
  virtual void OnRefresh(const std::vector<int>& /*visible_selected_indices*/,
                         const RefreshMode& /*mode*/) {}

  void OnSort(int column, std::optional<SortingOrder> new_order);
  virtual void OnContextMenu(const std::string& action, int menu_index,
                             const std::vector<int>& item_indices);
  virtual void OnSelect(const std::vector<int>& /*indices*/) {}
  // This method returns the intersection of selected indices and visible indices. The returned
  // value contains 0 or 1 index for a DataView with single selection, and contains 0 or
  // multiple indices for a DataView with multi-selection.
  [[nodiscard]] virtual std::vector<int> GetVisibleSelectedIndices();
  virtual void OnDoubleClicked(int /*index*/) {}
  virtual void OnDataChanged();
  virtual void OnTimer() {}
  virtual bool WantsDisplayColor() { return false; }
  // TODO(irinashkviro): return a Color instead of using out-parameters
  virtual bool GetDisplayColor(int /*row*/, int /*column*/, unsigned char& /*red*/,
                               unsigned char& /*green*/, unsigned char& /*blue*/) {
    return false;
  }
  virtual std::string GetLabel() { return ""; }
  virtual bool HasRefreshButton() const { return false; }
  virtual void OnRefreshButtonClicked() {}
  virtual void SetGlCanvas(class GlCanvas* /*gl_canvas*/) {}
  virtual void LinkDataView(DataView* /*data_view*/) {}
  virtual bool ScrollToBottom() { return false; }
  virtual bool SkipTimer() { return false; }
  virtual ErrorMessageOr<void> ExportCSV(const std::filesystem::path& file_path);
  virtual void CopySelection(const std::vector<int>& selection);

  int GetUpdatePeriodMs() const { return update_period_ms_; }
  DataViewType GetType() const { return type_; }
  [[nodiscard]] virtual bool ResetOnRefresh() const { return true; }

 protected:
  void InitSortingOrders();
  virtual void DoSort() {}
  virtual void DoFilter() {}
  FilterCallback filter_callback_;

  std::vector<uint64_t> indices_;
  std::vector<SortingOrder> sorting_orders_;
  int sorting_column_ = 0;
  std::string filter_;
  int update_period_ms_;
  absl::flat_hash_set<int> selected_indices_;
  DataViewType type_;

  static const std::string kMenuActionCopySelection;
  static const std::string kMenuActionExportToCsv;

  OrbitApp* app_ = nullptr;
};

#endif  // ORBIT_GL_DATA_VIEW_H_
