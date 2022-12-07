// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_APP_INTERFACE_H_
#define DATA_VIEWS_APP_INTERFACE_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <cstdint>
#include <string>

#include "ClientData/CaptureData.h"
#include "ClientData/CaptureDataHolder.h"
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/PresetLoadState.h"
#include "DataViews/SymbolLoadingState.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Future.h"
#include "PresetFile/PresetFile.h"
#include "Statistics/BinomialConfidenceInterval.h"
#include "Statistics/Histogram.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_data_views {

class AppInterface : public orbit_client_data::CaptureDataHolder {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  AppInterface() = default;
  virtual ~AppInterface() = default;

  // Functions needed by DataView
  virtual void SetClipboard(std::string_view contents) = 0;
  [[nodiscard]] virtual std::string GetSaveFile(std::string_view extension) const = 0;

  virtual void SendErrorToUi(std::string title, std::string text) = 0;

  // Functions needed by PresetsDataView
  virtual orbit_base::Future<ErrorMessageOr<void>> LoadPreset(
      const orbit_preset_file::PresetFile& preset) = 0;
  [[nodiscard]] virtual PresetLoadState GetPresetLoadState(
      const orbit_preset_file::PresetFile& preset) const = 0;
  virtual void ShowPresetInExplorer(const orbit_preset_file::PresetFile& preset) = 0;

  // Functions needed by FunctionsDataView
  [[nodiscard]] virtual bool IsFunctionSelected(
      const orbit_client_data::FunctionInfo& func) const = 0;

  // Functions needed by LiveFunctionsDataView
  enum class JumpToTimerMode { kFirst, kLast, kMin, kMax };
  virtual void JumpToTimerAndZoom(ScopeId scope_id, JumpToTimerMode selection_mode) = 0;
  [[nodiscard]] virtual std::optional<ScopeId> GetHighlightedScopeId() const = 0;
  virtual void SetHighlightedScopeId(std::optional<ScopeId> highlighted_scope_id) = 0;
  virtual void SetVisibleScopeIds(absl::flat_hash_set<ScopeId> visible_scope_ids) = 0;
  virtual void DeselectTimer() = 0;
  [[nodiscard]] virtual bool IsCapturing() const = 0;
  [[nodiscard]] virtual std::vector<const orbit_client_data::TimerChain*> GetAllThreadTimerChains()
      const = 0;

  // Functions needed by SamplingReportsDataView
  [[nodiscard]] virtual bool IsFunctionSelected(
      const orbit_client_data::SampledFunction& func) const = 0;

  [[nodiscard]] virtual bool IsFrameTrackEnabled(
      const orbit_client_data::FunctionInfo& function) const = 0;
  [[nodiscard]] virtual bool HasFrameTrackInCaptureData(
      uint64_t instrumented_function_id) const = 0;

  [[nodiscard]] virtual const orbit_client_data::ModuleManager* GetModuleManager() const = 0;
  [[nodiscard]] virtual orbit_client_data::ModuleManager* GetMutableModuleManager() = 0;

  // Functions needed by ModulesDataView
  virtual orbit_base::Future<ErrorMessageOr<void>> UpdateProcessAndModuleList() = 0;

  // Functions needed by TracepointsDataView
  virtual void SelectTracepoint(const orbit_grpc_protos::TracepointInfo& info) = 0;
  virtual void DeselectTracepoint(const orbit_grpc_protos::TracepointInfo& tracepoint) = 0;
  [[nodiscard]] virtual bool IsTracepointSelected(
      const orbit_grpc_protos::TracepointInfo& info) const = 0;

  // This needs to be called from the main thread.
  [[nodiscard]] virtual bool IsCaptureConnected(
      const orbit_client_data::CaptureData& capture) const = 0;

  [[nodiscard]] virtual const orbit_client_data::ProcessData* GetTargetProcess() const = 0;

  [[nodiscard]] virtual const orbit_client_data::ModuleData* GetModuleByModuleIdentifier(
      const orbit_symbol_provider::ModuleIdentifier& module_id) const = 0;
  [[nodiscard]] virtual orbit_client_data::ModuleData* GetMutableModuleByModuleIdentifier(
      const orbit_symbol_provider::ModuleIdentifier& module_id) = 0;
  virtual orbit_base::Future<void> LoadSymbolsManually(
      absl::Span<const orbit_client_data::ModuleData* const> modules) = 0;

  virtual void SelectFunction(const orbit_client_data::FunctionInfo& func) = 0;
  virtual void DeselectFunction(const orbit_client_data::FunctionInfo& func) = 0;

  virtual void EnableFrameTrack(const orbit_client_data::FunctionInfo& function) = 0;
  virtual void DisableFrameTrack(const orbit_client_data::FunctionInfo& function) = 0;

  virtual void AddFrameTrack(const orbit_client_data::FunctionInfo& function) = 0;
  virtual void RemoveFrameTrack(const orbit_client_data::FunctionInfo& function) = 0;

  virtual void Disassemble(uint32_t pid, const orbit_client_data::FunctionInfo& function) = 0;
  virtual void ShowSourceCode(const orbit_client_data::FunctionInfo& function) = 0;

  virtual void ShowHistogram(const std::vector<uint64_t>* data, std::string scope_name,
                             std::optional<ScopeId> scope_id) = 0;

  [[nodiscard]] virtual const orbit_statistics::BinomialConfidenceIntervalEstimator&
  GetConfidenceIntervalEstimator() const = 0;

  [[nodiscard]] virtual bool IsModuleDownloading(
      const orbit_client_data::ModuleData* module) const = 0;
  [[nodiscard]] virtual SymbolLoadingState GetSymbolLoadingStateForModule(
      const orbit_client_data::ModuleData* module) const = 0;

  [[nodiscard]] virtual bool IsSymbolLoadingInProgressForModule(
      const orbit_client_data::ModuleData* module) const = 0;
  virtual void RequestSymbolDownloadStop(
      absl::Span<const orbit_client_data::ModuleData* const> modules) = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_APP_INTERFACE_H_
