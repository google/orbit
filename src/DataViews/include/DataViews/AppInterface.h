// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_APP_INTERFACE_H_
#define DATA_VIEWS_APP_INTERFACE_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <string>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleData.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientData/ProcessData.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/PresetLoadState.h"
#include "GrpcProtos/tracepoint.pb.h"
#include "OrbitBase/Future.h"
#include "PresetFile/PresetFile.h"

namespace orbit_data_views {

class AppInterface {
 public:
  virtual ~AppInterface() = default;

  // Functions needed by DataView
  virtual void SetClipboard(const std::string& contents) = 0;
  [[nodiscard]] virtual std::string GetSaveFile(const std::string& extension) const = 0;

  virtual void SendErrorToUi(const std::string& title, const std::string& text) = 0;

  // Functions needed by PresetsDataView
  virtual void LoadPreset(const orbit_preset_file::PresetFile& preset) = 0;
  [[nodiscard]] virtual PresetLoadState GetPresetLoadState(
      const orbit_preset_file::PresetFile& preset) const = 0;
  virtual void ShowPresetInExplorer(const orbit_preset_file::PresetFile& preset) = 0;

  // Functions needed by FunctionsDataView
  [[nodiscard]] virtual bool IsFunctionSelected(
      const orbit_client_protos::FunctionInfo& func) const = 0;

  // Functions needed by LiveFunctionsDataView
  enum class JumpToTimerMode { kFirst, kLast, kMin, kMax };
  virtual void JumpToTimerAndZoom(uint64_t function_id, JumpToTimerMode selection_mode) = 0;
  [[nodiscard]] virtual uint64_t GetHighlightedFunctionId() const = 0;
  virtual void SetHighlightedFunctionId(uint64_t highlighted_function_id) = 0;
  virtual void SetVisibleFunctionIds(absl::flat_hash_set<uint64_t> visible_functions) = 0;
  virtual void DeselectTimer() = 0;
  [[nodiscard]] virtual bool IsCapturing() const = 0;
  [[nodiscard]] virtual std::vector<const orbit_client_protos::TimerInfo*>
  GetAllTimersForHookedFunction(uint64_t function_id) const = 0;

  // Function needed by CallstackDataView
  [[nodiscard]] virtual orbit_client_data::CaptureData& GetMutableCaptureData() = 0;

  // Functions needed by SamplingReportsDataView
  [[nodiscard]] virtual bool IsFunctionSelected(
      const orbit_client_data::SampledFunction& func) const = 0;

  [[nodiscard]] virtual bool IsFrameTrackEnabled(
      const orbit_client_protos::FunctionInfo& function) const = 0;
  [[nodiscard]] virtual bool HasFrameTrackInCaptureData(
      uint64_t instrumented_function_id) const = 0;

  [[nodiscard]] virtual bool HasCaptureData() const = 0;
  [[nodiscard]] virtual const orbit_client_data::CaptureData& GetCaptureData() const = 0;

  // Functions needed by ModulesDataView
  virtual void OnValidateFramePointers(
      std::vector<const orbit_client_data::ModuleData*> modules_to_validate) = 0;
  virtual void UpdateProcessAndModuleList() = 0;

  // Functions needed by TracepointsDataView
  virtual void SelectTracepoint(const orbit_grpc_protos::TracepointInfo& info) = 0;
  virtual void DeselectTracepoint(const orbit_grpc_protos::TracepointInfo& tracepoint) = 0;
  [[nodiscard]] virtual bool IsTracepointSelected(
      const orbit_grpc_protos::TracepointInfo& info) const = 0;

  // This needs to be called from the main thread.
  [[nodiscard]] virtual bool IsCaptureConnected(
      const orbit_client_data::CaptureData& capture) const = 0;

  [[nodiscard]] virtual const orbit_client_data::ProcessData* GetTargetProcess() const = 0;

  [[nodiscard]] virtual const orbit_client_data::ModuleData* GetModuleByPathAndBuildId(
      const std::string& path, const std::string& build_id) const = 0;
  [[nodiscard]] virtual orbit_client_data::ModuleData* GetMutableModuleByPathAndBuildId(
      const std::string& path, const std::string& build_id) = 0;
  virtual orbit_base::Future<void> RetrieveModulesAndLoadSymbols(
      absl::Span<const orbit_client_data::ModuleData* const> modules) = 0;

  virtual void SelectFunction(const orbit_client_protos::FunctionInfo& func) = 0;
  virtual void DeselectFunction(const orbit_client_protos::FunctionInfo& func) = 0;

  virtual void EnableFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void DisableFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void AddFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void AddFrameTrack(uint64_t instrumented_function_id) = 0;
  virtual void RemoveFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void RemoveFrameTrack(uint64_t instrumented_function_id) = 0;

  virtual void Disassemble(uint32_t pid, const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void ShowSourceCode(const orbit_client_protos::FunctionInfo& function) = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_APP_INTERFACE_H_
