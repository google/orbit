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
#include "ClientData/FunctionInfo.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientData/ScopeId.h"
#include "ClientProtos/capture_data.pb.h"
#include "DataViews/AppInterface.h"
#include "DataViews/PresetLoadState.h"
#include "DataViews/SymbolLoadingState.h"
#include "OrbitBase/Future.h"
#include "PresetFile/PresetFile.h"
#include "SymbolProvider/ModuleIdentifier.h"

namespace orbit_data_views {

// This is a mock of AppInterface which can be shared between all data view tests.
class MockAppInterface : public AppInterface {
  using ScopeId = orbit_client_data::ScopeId;

 public:
  MOCK_METHOD(void, SetClipboard, (std::string_view), (override));
  MOCK_METHOD(std::string, GetSaveFile, (std::string_view extension), (const, override));

  MOCK_METHOD(void, SendErrorToUi, (std::string title, std::string text), (override));

  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<void>>, LoadPreset,
              (const orbit_preset_file::PresetFile&), (override));
  MOCK_METHOD(PresetLoadState, GetPresetLoadState, (const orbit_preset_file::PresetFile&),
              (const, override));
  MOCK_METHOD(void, ShowPresetInExplorer, (const orbit_preset_file::PresetFile&), (override));

  MOCK_METHOD(bool, IsFunctionSelected, (const orbit_client_data::FunctionInfo&),
              (const, override));
  MOCK_METHOD(bool, IsFunctionSelected, (const orbit_client_data::SampledFunction&),
              (const, override));

  MOCK_METHOD(std::optional<ScopeId>, GetHighlightedScopeId, (), (const, override));
  MOCK_METHOD(void, SetHighlightedScopeId, (std::optional<ScopeId> highlighted_scope_id),
              (override));
  MOCK_METHOD(void, SetVisibleScopeIds, (absl::flat_hash_set<ScopeId> visible_scopes), (override));
  MOCK_METHOD(void, DeselectTimer, (), (override));
  MOCK_METHOD(bool, IsCapturing, (), (const, override));
  MOCK_METHOD(void, JumpToTimerAndZoom, (ScopeId scope_id, JumpToTimerMode selection_mode),
              (override));
  MOCK_METHOD(std::vector<const orbit_client_data::TimerChain*>, GetAllThreadTimerChains, (),
              (const, override));

  MOCK_METHOD(bool, IsFrameTrackEnabled, (const orbit_client_data::FunctionInfo&),
              (const, override));
  MOCK_METHOD(bool, HasFrameTrackInCaptureData, (uint64_t), (const, override));

  MOCK_METHOD(bool, HasCaptureData, (), (const, override));
  MOCK_METHOD(orbit_client_data::CaptureData&, GetMutableCaptureData, (), (override));
  MOCK_METHOD(const orbit_client_data::CaptureData&, GetCaptureData, (), (const, override));
  MOCK_METHOD(const orbit_client_data::ModuleManager*, GetModuleManager, (), (const, override));
  MOCK_METHOD(orbit_client_data::ModuleManager*, GetMutableModuleManager, (), (override));

  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<void>>, UpdateProcessAndModuleList, (), (override));

  // This needs to be called from the main thread.
  MOCK_METHOD(bool, IsCaptureConnected, (const orbit_client_data::CaptureData&), (const, override));

  MOCK_METHOD(const orbit_client_data::ProcessData*, GetTargetProcess, (), (const, override));

  MOCK_METHOD(const orbit_client_data::ModuleData*, GetModuleByModuleIdentifier,
              (const orbit_symbol_provider::ModuleIdentifier&), (const, override));
  MOCK_METHOD(orbit_client_data::ModuleData*, GetMutableModuleByModuleIdentifier,
              (const orbit_symbol_provider::ModuleIdentifier&), (override));
  MOCK_METHOD(orbit_base::Future<void>, LoadSymbolsManually,
              (absl::Span<const orbit_client_data::ModuleData* const>), (override));

  MOCK_METHOD(void, SelectFunction, (const orbit_client_data::FunctionInfo&), (override));
  MOCK_METHOD(void, DeselectFunction, (const orbit_client_data::FunctionInfo&), (override));

  MOCK_METHOD(void, EnableFrameTrack, (const orbit_client_data::FunctionInfo&), (override));
  MOCK_METHOD(void, DisableFrameTrack, (const orbit_client_data::FunctionInfo&), (override));
  MOCK_METHOD(void, AddFrameTrack, (const orbit_client_data::FunctionInfo&), (override));
  MOCK_METHOD(void, RemoveFrameTrack, (const orbit_client_data::FunctionInfo&), (override));

  MOCK_METHOD(void, Disassemble, (uint32_t pid, const orbit_client_data::FunctionInfo&),
              (override));
  MOCK_METHOD(void, ShowSourceCode, (const orbit_client_data::FunctionInfo&), (override));

  MOCK_METHOD(bool, IsTracepointSelected, (const orbit_grpc_protos::TracepointInfo&),
              (const, override));
  MOCK_METHOD(void, SelectTracepoint, (const orbit_grpc_protos::TracepointInfo&), (override));
  MOCK_METHOD(void, DeselectTracepoint, (const orbit_grpc_protos::TracepointInfo&), (override));

  MOCK_METHOD(const orbit_statistics::BinomialConfidenceIntervalEstimator&,
              GetConfidenceIntervalEstimator, (), (const, override));

  MOCK_METHOD(void, ShowHistogram,
              (const std::vector<uint64_t>* data, std::string function_name,
               std::optional<ScopeId> scope_id),
              (override));

  MOCK_METHOD(uint64_t, ProvideScopeId, (const orbit_client_protos::TimerInfo& timer_info),
              (const));

  MOCK_METHOD(bool, IsModuleDownloading, (const orbit_client_data::ModuleData* module),
              (const, override));
  MOCK_METHOD(SymbolLoadingState, GetSymbolLoadingStateForModule,
              (const orbit_client_data::ModuleData* module), (const, override));

  MOCK_METHOD(bool, IsSymbolLoadingInProgressForModule,
              (const orbit_client_data::ModuleData* module), (const, override));
  MOCK_METHOD(void, RequestSymbolDownloadStop,
              (absl::Span<const orbit_client_data::ModuleData* const>), (override));
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_MOCK_APP_INTERFACE_H_
