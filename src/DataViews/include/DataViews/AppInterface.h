// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_APP_INTERFACE_H_
#define DATA_VIEWS_APP_INTERFACE_H_

#include <absl/types/span.h>
#include <stdint.h>

#include <string>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientModel/CaptureData.h"
#include "DataViews/PresetLoadState.h"
#include "OrbitBase/Future.h"
#include "PresetFile/PresetFile.h"
#include "capture_data.pb.h"

namespace orbit_data_views {

class AppInterface {
 public:
  virtual ~AppInterface() noexcept = default;

  // Functions needed by DataView
  virtual void SetClipboard(const std::string& contents) = 0;
  [[nodiscard]] virtual std::string GetSaveFile(const std::string& extension) const = 0;

  virtual void SendErrorToUi(const std::string& title, const std::string& text) = 0;

  // Functions needed by PresetsDataView
  virtual void LoadPreset(const orbit_preset_file::PresetFile& preset) = 0;
  [[nodiscard]] virtual PresetLoadState GetPresetLoadState(
      const orbit_preset_file::PresetFile& preset) const = 0;

  // Functions needed by FunctionsDataView
  [[nodiscard]] virtual bool IsFunctionSelected(
      const orbit_client_protos::FunctionInfo& func) const = 0;

  [[nodiscard]] virtual bool IsFrameTrackEnabled(
      const orbit_client_protos::FunctionInfo& function) const = 0;
  [[nodiscard]] virtual bool HasFrameTrackInCaptureData(
      uint64_t instrumented_function_id) const = 0;

  [[nodiscard]] virtual bool HasCaptureData() const = 0;
  [[nodiscard]] virtual const orbit_client_model::CaptureData& GetCaptureData() const = 0;

  // This needs to be called from the main thread.
  [[nodiscard]] virtual bool IsCaptureConnected(
      const orbit_client_model::CaptureData& capture) const = 0;

  [[nodiscard]] virtual const orbit_client_data::ProcessData* GetTargetProcess() const = 0;

  [[nodiscard]] virtual const orbit_client_data::ModuleData* GetModuleByPathAndBuildId(
      const std::string& path, const std::string& build_id) const = 0;

  virtual void SelectFunction(const orbit_client_protos::FunctionInfo& func) = 0;
  virtual void DeselectFunction(const orbit_client_protos::FunctionInfo& func) = 0;

  virtual void EnableFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void DisableFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void AddFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void RemoveFrameTrack(const orbit_client_protos::FunctionInfo& function) = 0;

  virtual void Disassemble(int32_t pid, const orbit_client_protos::FunctionInfo& function) = 0;
  virtual void ShowSourceCode(const orbit_client_protos::FunctionInfo& function) = 0;
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_APP_INTERFACE_H_
