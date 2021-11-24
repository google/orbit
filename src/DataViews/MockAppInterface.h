// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_MOCK_APP_INTERFACE_H_
#define DATA_VIEWS_MOCK_APP_INTERFACE_H_

#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <stdint.h>

#include <string>

#include "ClientData/CaptureData.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/AppInterface.h"
#include "DataViews/PresetLoadState.h"
#include "OrbitBase/Future.h"
#include "PresetFile/PresetFile.h"

namespace orbit_data_views {

// This is a mock of AppInterface which can be shared between all data view tests.
class MockAppInterface : public AppInterface {
 public:
  MOCK_METHOD(void, SetClipboard, (const std::string&));
  MOCK_METHOD(std::string, GetSaveFile, (const std::string& extension), (const));

  MOCK_METHOD(void, SendErrorToUi, (const std::string& title, const std::string& text));

  MOCK_METHOD(void, LoadPreset, (const orbit_preset_file::PresetFile&));
  MOCK_METHOD(PresetLoadState, GetPresetLoadState, (const orbit_preset_file::PresetFile&), (const));
  MOCK_METHOD(void, ShowPresetInExplorer, (const orbit_preset_file::PresetFile&));

  MOCK_METHOD(bool, IsFunctionSelected, (const orbit_client_protos::FunctionInfo&), (const));
  MOCK_METHOD(bool, IsFunctionSelected, (const orbit_client_data::SampledFunction&), (const));

  MOCK_METHOD(uint64_t, GetHighlightedFunctionId, (), (const));
  MOCK_METHOD(void, SetHighlightedFunctionId, (uint64_t highlighted_function_id));
  MOCK_METHOD(void, SetVisibleFunctionIds, (absl::flat_hash_set<uint64_t> visible_functions));
  MOCK_METHOD(void, DeselectTimer, ());
  MOCK_METHOD(bool, IsCapturing, (), (const));
  MOCK_METHOD(void, JumpToTimerAndZoom, (uint64_t function_id, JumpToTimerMode selection_mode));
  MOCK_METHOD(std::vector<const orbit_client_protos::TimerInfo*>, GetAllTimersForHookedFunction,
              (uint64_t), (const));

  MOCK_METHOD(bool, IsFrameTrackEnabled, (const orbit_client_protos::FunctionInfo&), (const));
  MOCK_METHOD(bool, HasFrameTrackInCaptureData, (uint64_t), (const));

  MOCK_METHOD(bool, HasCaptureData, (), (const));
  MOCK_METHOD(orbit_client_data::CaptureData&, GetMutableCaptureData, ());
  MOCK_METHOD(const orbit_client_data::CaptureData&, GetCaptureData, (), (const));

  MOCK_METHOD(void, OnValidateFramePointers, (std::vector<const orbit_client_data::ModuleData*>));
  MOCK_METHOD(void, UpdateProcessAndModuleList, ());

  // This needs to be called from the main thread.
  MOCK_METHOD(bool, IsCaptureConnected, (const orbit_client_data::CaptureData&), (const));

  MOCK_METHOD(const orbit_client_data::ProcessData*, GetTargetProcess, (), (const));

  MOCK_METHOD(const orbit_client_data::ModuleData*, GetModuleByPathAndBuildId,
              (const std::string&, const std::string&), (const));
  MOCK_METHOD(orbit_client_data::ModuleData*, GetMutableModuleByPathAndBuildId,
              (const std::string&, const std::string&));
  MOCK_METHOD(orbit_base::Future<void>, RetrieveModulesAndLoadSymbols,
              (absl::Span<const orbit_client_data::ModuleData* const>));

  MOCK_METHOD(void, SelectFunction, (const orbit_client_protos::FunctionInfo&));
  MOCK_METHOD(void, DeselectFunction, (const orbit_client_protos::FunctionInfo&));

  MOCK_METHOD(void, EnableFrameTrack, (const orbit_client_protos::FunctionInfo&));
  MOCK_METHOD(void, DisableFrameTrack, (const orbit_client_protos::FunctionInfo&));
  MOCK_METHOD(void, AddFrameTrack, (const orbit_client_protos::FunctionInfo&));
  MOCK_METHOD(void, AddFrameTrack, (uint64_t instrumented_function_id));
  MOCK_METHOD(void, RemoveFrameTrack, (const orbit_client_protos::FunctionInfo&));
  MOCK_METHOD(void, RemoveFrameTrack, (uint64_t instrumented_function_id));

  MOCK_METHOD(void, Disassemble, (uint32_t pid, const orbit_client_protos::FunctionInfo&));
  MOCK_METHOD(void, ShowSourceCode, (const orbit_client_protos::FunctionInfo&));

  MOCK_METHOD(bool, IsTracepointSelected, (const orbit_grpc_protos::TracepointInfo&), (const));
  MOCK_METHOD(void, SelectTracepoint, (const orbit_grpc_protos::TracepointInfo&));
  MOCK_METHOD(void, DeselectTracepoint, (const orbit_grpc_protos::TracepointInfo&));
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_MOCK_APP_INTERFACE_H_
