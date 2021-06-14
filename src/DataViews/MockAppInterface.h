// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef DATA_VIEWS_MOCK_APP_INTERFACE_H_
#define DATA_VIEWS_MOCK_APP_INTERFACE_H_

#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <stdint.h>

#include <string>

#include "ClientData/ModuleData.h"
#include "ClientData/ProcessData.h"
#include "ClientModel/CaptureData.h"
#include "DataViews/AppInterface.h"
#include "DataViews/PresetLoadState.h"
#include "OrbitBase/Future.h"
#include "PresetFile/PresetFile.h"
#include "capture_data.pb.h"

namespace orbit_data_views {

// This is a mock of AppInterface which can be shared between all data view tests.
class MockAppInterface : public AppInterface {
 public:
  MOCK_METHOD(void, SetClipboard, (const std::string&));
  MOCK_METHOD(std::string, GetSaveFile, (const std::string& extension), (const));

  MOCK_METHOD(void, SendErrorToUi, (const std::string& title, const std::string& text));

  MOCK_METHOD(void, LoadPreset, (const orbit_preset_file::PresetFile& preset));
  MOCK_METHOD(PresetLoadState, GetPresetLoadState, (const orbit_preset_file::PresetFile&), (const));
};

}  // namespace orbit_data_views

#endif  // DATA_VIEWS_MOCK_APP_INTERFACE_H_
