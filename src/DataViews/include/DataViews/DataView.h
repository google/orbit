// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_DATA_VIEW_H_
#define DATA_VIEWS_DATA_VIEW_H_

#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_join.h>
#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <functional>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "DataViews/AppInterface.h"
#include "DataViews/DataViewType.h"
#include "MetricsUploader/MetricsUploader.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

enum class RefreshMode { kOnFilter, kOnSort, kOther };

namespace orbit_data_views {

// Hooking related actions
constexpr std::string_view kMenuActionLoadSymbols = "Load Symbols";
constexpr std::string_view kMenuActionStopDownload = "Stop downloading...";
constexpr std::string_view kMenuActionSelect = "Hook";
constexpr std::string_view kMenuActionUnselect = "Unhook";
constexpr std::string_view kMenuActionEnableFrameTrack = "Enable frame track(s)";
constexpr std::string_view kMenuActionDisableFrameTrack = "Disable frame track(s)";
constexpr std::string_view kMenuActionAddIterator = "Add iterator(s)";

constexpr std::string_view kMenuActionVerifyFramePointers = "Verify Frame Pointers";

constexpr std::string_view kMenuActionDisassembly = "Go to Disassembly";
constexpr std::string_view kMenuActionSourceCode = "Go to Source code";

// Navigating related actions
constexpr std::string_view kMenuActionJumpToFirst = "Jump to first";
constexpr std::string_view kMenuActionJumpToLast = "Jump to last";
constexpr std::string_view kMenuActionJumpToMin = "Jump to min";
constexpr std::string_view kMenuActionJumpToMax = "Jump to max";

// Preset related actions
constexpr std::string_view kMenuActionLoadPreset = "Load Preset";
constexpr std::string_view kMenuActionDeletePreset = "Delete Preset";
constexpr std::string_view kMenuActionShowInExplorer = "Show in Explorer";

// Exporting relate actions
constexpr std::string_view kMenuActionCopySelection = "Copy Selection";
constexpr std::string_view kMenuActionExportToCsv = "Export to CSV";
constexpr std::string_view kMenuActionExportEventsToCsv = "Export events to CSV";

static constexpr std::string_view kFieldSeparator = ",";
// CSV RFC requires lines to end with CRLF
static constexpr std::string_view kLineSeparator = "\r\n";

// Values in the DataView may contain commas, for example, functions with arguments. We quote all
// values in the output and also escape quotes (with a second quote) in values to ensure the CSV
// files can be imported correctly in spreadsheet applications. The formatting follows the
// specification in https://tools.ietf.org/html/rfc4180.
std::string FormatValueForCsv(std::string_view value);

template <typename Range>
ErrorMessageOr<void> WriteLineToCsv(const orbit_base::unique_fd& fd, const Range& cells) {
  std::string header_line = absl::StrJoin(
      cells, kFieldSeparator,
      [](std::string* out, const std::string& name) { out->append(FormatValueForCsv(name)); });
  header_line.append(kLineSeparator);
  return orbit_base::WriteFully(fd, header_line);
}

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

  struct Action {
    Action(std::string_view name, bool enabled) : name{std::move(name)}, enabled{enabled} {}
    std::string name;
    bool enabled;
  };
  using ActionGroup = std::vector<Action>;

  explicit DataView(DataViewType type, AppInterface* app,
                    orbit_metrics_uploader::MetricsUploader* metrics_uploader)
      : update_period_ms_(-1), type_(type), app_{app}, metrics_uploader_{metrics_uploader} {}

  // Calls virtual initialization methods, therefore cannot be called from the constructor.
  void Init();

  // Creates a DataView and calls the Init() method after construction.
  template <typename DataViewT, typename... Args>
  [[nodiscard]] static std::unique_ptr<DataViewT> CreateAndInit(Args&&... args) {
    auto data_view = std::make_unique<DataViewT>(std::forward<Args>(args)...);
    data_view->Init();
    return data_view;
  }

  virtual ~DataView() = default;

  virtual void SetAsMainInstance() {}
  virtual const std::vector<Column>& GetColumns() = 0;
  virtual bool IsSortingAllowed() { return true; }
  virtual int GetDefaultSortingColumn() { return 0; }
  std::vector<ActionGroup> GetContextMenuWithGrouping(int clicked_index,
                                                      const std::vector<int>& selected_indices);
  virtual size_t GetNumElements() { return indices_.size(); }
  virtual std::string GetValue(int /*row*/, int /*column*/) { return ""; }
  virtual std::string GetValueForCopy(int row, int column) { return GetValue(row, column); }
  virtual std::string GetToolTip(int /*row*/, int /*column*/) { return ""; }

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
  void OnContextMenu(const std::string& action, int menu_index,
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
  virtual void LinkDataView(DataView* /*data_view*/) {}
  virtual bool ScrollToBottom() { return false; }
  virtual bool SkipTimer() { return false; }

  int GetUpdatePeriodMs() const { return update_period_ms_; }
  [[nodiscard]] DataViewType GetType() const { return type_; }
  [[nodiscard]] virtual bool ResetOnRefresh() const { return true; }

  void OnLoadSymbolsRequested(const std::vector<int>& selection);
  void OnStopDownloadRequested(const std::vector<int>& selection);
  virtual void OnSelectRequested(const std::vector<int>& selection);
  virtual void OnUnselectRequested(const std::vector<int>& selection);
  void OnEnableFrameTrackRequested(const std::vector<int>& selection);
  void OnDisableFrameTrackRequested(const std::vector<int>& selection);
  virtual void OnIteratorRequested(const std::vector<int>& /*selection*/) {}
  void OnVerifyFramePointersRequested(const std::vector<int>& selection);
  void OnDisassemblyRequested(const std::vector<int>& selection);
  void OnSourceCodeRequested(const std::vector<int>& selection);
  virtual void OnJumpToRequested(const std::string& /*action*/,
                                 const std::vector<int>& /*selection*/) {}
  virtual void OnLoadPresetRequested(const std::vector<int>& /*selection*/) {}
  virtual void OnDeletePresetRequested(const std::vector<int>& /*selection*/) {}
  virtual void OnShowInExplorerRequested(const std::vector<int>& /*selection*/) {}
  void OnCopySelectionRequested(const std::vector<int>& selection);
  void OnExportToCsvRequested();
  virtual void OnExportEventsToCsvRequested(const std::vector<int>& /*selection*/) {}
  [[nodiscard]] virtual uint64_t GetScopeId(uint32_t row) const;

 protected:
  [[nodiscard]] virtual const orbit_client_data::ModuleData* GetModuleDataFromRow(
      int /*row*/) const {
    return nullptr;
  }
  [[nodiscard]] virtual const orbit_client_data::FunctionInfo* GetFunctionInfoFromRow(int /*row*/) {
    return nullptr;
  }

  ErrorMessageOr<void> ExportToCsv(std::string_view file_path);

  template <typename T>
  void ReportErrorIfAny(const ErrorMessageOr<T>& error_message_or,
                        std::string_view error_window_title) const {
    if (error_message_or.has_error()) {
      app_->SendErrorToUi(std::string{error_window_title}, error_message_or.error().message());
    }
  }

  enum class ActionStatus { kInvisible, kVisibleButDisabled, kVisibleAndEnabled };
  [[nodiscard]] virtual ActionStatus GetActionStatus(std::string_view action, int clicked_index,
                                                     const std::vector<int>& selected_indices);

  [[nodiscard]] bool IsScopeDynamicallyInstrumentedFunction(uint64_t scope_id) const;

  void InitSortingOrders();
  virtual void DoSort() {}
  virtual void DoFilter() {}
  FilterCallback filter_callback_;

  // Contains a list of scope_id in the displayed order
  std::vector<uint64_t> indices_;
  std::vector<SortingOrder> sorting_orders_;
  int sorting_column_ = 0;
  std::string filter_;
  int update_period_ms_;
  absl::flat_hash_set<int> selected_indices_;
  DataViewType type_;

  AppInterface* app_ = nullptr;
  orbit_metrics_uploader::MetricsUploader* metrics_uploader_;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_DATA_VIEW_H_
