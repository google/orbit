// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_MAIN_WINDOW_INTERFACE_H_
#define ORBIT_GL_MAIN_WINDOW_INTERFACE_H_

#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string_view>

#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStatsCollection.h"
#include "ClientProtos/capture_data.pb.h"
#include "CodeReport/CodeReport.h"
#include "CodeReport/DisassemblyReport.h"
#include "DataViews/DataViewType.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/StopToken.h"
#include "OrbitGl/CallTreeView.h"
#include "OrbitGl/SelectionData.h"

namespace orbit_gl {

// This abstract base class is an attempt to simplify callbacks
// in OrbitApp and make it easier to refactor things in the future.
//
// OrbitMainWindow and Mocks can derive from this and offer a fixed interface
// to OrbitApp.
class MainWindowInterface {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  virtual void ShowTooltip(std::string_view message) = 0;
  virtual void ShowWarningWithDontShowAgainCheckboxIfNeeded(
      std::string_view title, std::string_view text,
      std::string_view dont_show_again_setting_key) = 0;

  virtual void ShowSourceCode(
      const std::filesystem::path& file_path, size_t line_number,
      std::optional<std::unique_ptr<orbit_code_report::CodeReport>> code_report) = 0;
  virtual void ShowDisassembly(const orbit_client_data::FunctionInfo& function_info,
                               std::string_view assembly,
                               orbit_code_report::DisassemblyReport report) = 0;

  enum class CaptureLogSeverity { kInfo, kWarning, kSevereWarning, kError };
  virtual void AppendToCaptureLog(CaptureLogSeverity severity, absl::Duration capture_time,
                                  std::string_view message) = 0;

  virtual void ShowHistogram(const std::vector<uint64_t>* data, std::string scope_name,
                             std::optional<ScopeId> scope_id) = 0;

  enum class SymbolErrorHandlingResult { kReloadRequired, kSymbolLoadingCancelled };
  virtual SymbolErrorHandlingResult HandleSymbolError(
      const ErrorMessage& error, const orbit_client_data::ModuleData* module) = 0;

  virtual orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> DownloadFileFromInstance(
      std::filesystem::path path_on_instance, std::filesystem::path local_path,
      orbit_base::StopToken stop_token) = 0;
  virtual void OnCaptureCleared() = 0;
  virtual void OnTimerSelectionChanged(const orbit_client_protos::TimerInfo* timer_info) = 0;
  virtual void OnSetClipboard(std::string_view text) = 0;
  virtual void RefreshDataView(orbit_data_views::DataViewType type) = 0;
  virtual void SelectLiveTab() = 0;
  virtual void SelectTopDownTab() = 0;
  virtual std::string OnGetSaveFileName(std::string_view extension) = 0;

  virtual void SetErrorMessage(std::string_view title, std::string_view text) = 0;
  virtual void SetWarningMessage(std::string_view title, std::string_view text) = 0;

  // Returns orbit_base::Canceled if the user chooses cancel in the dialog, void otherwise
  virtual orbit_base::CanceledOr<void> DisplayStopDownloadDialog(
      const orbit_client_data::ModuleData* module) = 0;

  virtual ~MainWindowInterface() = default;

  virtual void SetSelection(const SelectionData& selection_data) = 0;

  virtual bool IsConnected() = 0;
  [[nodiscard]] virtual bool IsLocalTarget() const = 0;

  virtual void SetLiveTabScopeStatsCollection(
      std::shared_ptr<const orbit_client_data::ScopeStatsCollection> scope_collection) = 0;

  virtual void SetTopDownView(std::shared_ptr<const CallTreeView> view) = 0;
  virtual void SetBottomUpView(std::shared_ptr<const CallTreeView> view) = 0;
  virtual void SetSelectionTopDownView(std::shared_ptr<const CallTreeView> view) = 0;
  virtual void SetSelectionBottomUpView(std::shared_ptr<const CallTreeView> view) = 0;
  virtual void SetSamplingReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) = 0;
  virtual void SetSelectionSamplingReport(
      orbit_data_views::DataView* callstack_data_view,
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) = 0;
  virtual void UpdateSamplingReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) = 0;
  virtual void UpdateSelectionReport(
      const orbit_client_data::CallstackData* callstack_data,
      const orbit_client_data::PostProcessedSamplingData* post_processed_sampling_data) = 0;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_MAIN_WINDOW_INTERFACE_H_