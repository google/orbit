// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include <string>
#include <utility>

#include "ClientData/FunctionInfo.h"
#include "ClientData/UserDefinedCaptureData.h"

namespace orbit_client_data {

FunctionInfo CreateFunctionInfo(std::string function_name, uint64_t function_address) {
  FunctionInfo info{"/path/to/module",
                    "build id",
                    function_address,
                    /*size=*/16,
                    std::move(function_name),
                    /*is_hotpatchable=*/false};
  return info;
}

TEST(UserDefinedCaptureData, InsertFrameTrack) {
  UserDefinedCaptureData data;
  FunctionInfo info = CreateFunctionInfo("fun0_name", 0);
  data.InsertFrameTrack(info);
  EXPECT_TRUE(data.ContainsFrameTrack(info));
}

TEST(UserDefinedCaptureData, InsertFrameTrackDuplicateFunctions) {
  UserDefinedCaptureData data;
  FunctionInfo info = CreateFunctionInfo("fun0_name", 0);
  data.InsertFrameTrack(info);
  data.InsertFrameTrack(info);
  EXPECT_TRUE(data.ContainsFrameTrack(info));
}

TEST(UserDefinedCaptureData, InsertFrameTrackDifferentFunctions) {
  UserDefinedCaptureData data;
  FunctionInfo info0 = CreateFunctionInfo("fun0_name", 0);
  FunctionInfo info1 = CreateFunctionInfo("fun1_name", 1);
  data.InsertFrameTrack(info0);
  data.InsertFrameTrack(info1);
  const absl::flat_hash_set<FunctionInfo>& set = data.frame_track_functions();
  EXPECT_EQ(set.size(), 2);
}

TEST(UserDefinedCaptureData, EraseNonExistentFrameTrack) {
  UserDefinedCaptureData data;
  FunctionInfo info = CreateFunctionInfo("fun0_name", 0);
  data.EraseFrameTrack(info);
  EXPECT_FALSE(data.ContainsFrameTrack(info));
}

TEST(UserDefinedCaptureData, EraseFrameTrack) {
  UserDefinedCaptureData data;
  FunctionInfo info = CreateFunctionInfo("fun0_name", 0);
  data.InsertFrameTrack(info);
  data.EraseFrameTrack(info);
  EXPECT_FALSE(data.ContainsFrameTrack(info));
}

TEST(UserDefinedCaptureData, EraseFrameTrackDifferentFunctions) {
  UserDefinedCaptureData data;
  FunctionInfo info0 = CreateFunctionInfo("fun0_name", 0);
  FunctionInfo info1 = CreateFunctionInfo("fun1_name", 1);
  data.InsertFrameTrack(info0);
  data.InsertFrameTrack(info1);
  data.EraseFrameTrack(info0);
  EXPECT_FALSE(data.ContainsFrameTrack(info0));
  EXPECT_TRUE(data.ContainsFrameTrack(info1));
}

TEST(UserDefinedCaptureData, ContainsFrameTrackEmpty) {
  UserDefinedCaptureData data;
  FunctionInfo info = CreateFunctionInfo("fun1_name", 0);
  EXPECT_FALSE(data.ContainsFrameTrack(info));
}

TEST(UserDefinedCaptureData, Clear) {
  UserDefinedCaptureData data;
  FunctionInfo info = CreateFunctionInfo("fun0_name", 0);
  data.InsertFrameTrack(info);
  EXPECT_TRUE(data.ContainsFrameTrack(info));
  data.Clear();
  const absl::flat_hash_set<FunctionInfo>& set = data.frame_track_functions();
  EXPECT_TRUE(set.empty());
}

}  // namespace orbit_client_data
