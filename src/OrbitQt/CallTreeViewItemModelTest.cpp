// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <memory>
#include <optional>

#include "CallTreeView.h"
#include "CallTreeViewItemModel.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientModel/CaptureData.h"
#include "ClientModel/SamplingDataPostProcessor.h"
#include "QtUtils/AssertNoQtLogWarnings.h"
#include "capture.pb.h"
#include "capture_data.pb.h"

constexpr uint64_t kCallstackId = 1;
constexpr uint64_t kFunctionAbsoluteAddress = 0x30;
constexpr uint64_t kInstructionAbsoluteAddress = 0x31;
constexpr int32_t kThreadId = 42;
constexpr const char* kFunctionName = "example function";
constexpr const char* kModuleName = "example module";
constexpr const char* kThreadName = "example thread";

namespace {

std::unique_ptr<orbit_client_model::CaptureData> GenerateTestCaptureData() {
  auto capture_data = std::make_unique<orbit_client_model::CaptureData>(
      nullptr, orbit_grpc_protos::CaptureStarted{}, std::nullopt, absl::flat_hash_set<uint64_t>{});

  // AddressInfo
  orbit_client_protos::LinuxAddressInfo address_info;
  address_info.set_absolute_address(kInstructionAbsoluteAddress);
  address_info.set_offset_in_function(kInstructionAbsoluteAddress - kFunctionAbsoluteAddress);
  address_info.set_function_name(kFunctionName);
  address_info.set_module_path(kModuleName);
  capture_data->InsertAddressInfo(address_info);

  // CallstackInfo
  const std::vector<uint64_t> callstack_frames{kInstructionAbsoluteAddress};
  orbit_client_protos::CallstackInfo callstack_info;
  *callstack_info.mutable_frames() = {callstack_frames.begin(), callstack_frames.end()};
  callstack_info.set_type(orbit_client_protos::CallstackInfo_CallstackType_kComplete);
  capture_data->AddUniqueCallstack(kCallstackId, std::move(callstack_info));

  // CallstackEvent
  orbit_client_protos::CallstackEvent callstack_event;
  callstack_event.set_callstack_id(kCallstackId);
  callstack_event.set_thread_id(kThreadId);
  capture_data->AddCallstackEvent(std::move(callstack_event));

  capture_data->AddOrAssignThreadName(kThreadId, kThreadName);

  return capture_data;
}

}  // namespace

TEST(CallTreeViewItemModel, AbstractItemModelTesterEmptyModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  CallTreeViewItemModel model{std::make_unique<CallTreeView>()};

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST(CallTreeViewItemModel, AbstractItemModelTesterFilledModel) {
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  std::unique_ptr<orbit_client_model::CaptureData> capture_data = GenerateTestCaptureData();
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(*capture_data->GetCallstackData(),
                                                          *capture_data);

  auto call_tree_view =
      CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(sampling_data, *capture_data);

  CallTreeViewItemModel model{std::move(call_tree_view)};

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST(CallTreeViewItemModel, SummaryItem) {
  std::unique_ptr<orbit_client_model::CaptureData> capture_data = GenerateTestCaptureData();

  {
    bool generate_summary = false;

    orbit_client_data::PostProcessedSamplingData sampling_data =
        orbit_client_model::CreatePostProcessedSamplingData(*capture_data->GetCallstackData(),
                                                            *capture_data, generate_summary);
    auto call_tree_view =
        CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(sampling_data, *capture_data);

    CallTreeViewItemModel model{std::move(call_tree_view)};

    EXPECT_EQ(model.rowCount({}), 1);
  }

  {
    bool generate_summary = true;
    orbit_client_data::PostProcessedSamplingData sampling_data =
        orbit_client_model::CreatePostProcessedSamplingData(*capture_data->GetCallstackData(),
                                                            *capture_data, generate_summary);
    auto call_tree_view =
        CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(sampling_data, *capture_data);

    CallTreeViewItemModel model{std::move(call_tree_view)};

    EXPECT_EQ(model.rowCount({}), 2);
  }
}

TEST(CallTreeViewItemModel, GetDisplayRoleData) {
  std::unique_ptr<orbit_client_model::CaptureData> capture_data = GenerateTestCaptureData();
  // do not create summary, because this creates an additional top level row (all threads) and this
  // is not tested here
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(*capture_data->GetCallstackData(),
                                                          *capture_data, false);

  auto call_tree_view =
      CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(sampling_data, *capture_data);

  CallTreeViewItemModel model{std::move(call_tree_view)};

  // One top level entry for thread with id: kThreadId
  ASSERT_EQ(model.rowCount({}), 1);

  // Thread entry
  {  // thread name
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, {});
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(),
              QString{"%1 [%2]"}.arg(kThreadName).arg(kThreadId));
  }

  {  // inclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kInclusive, {});
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00% (1)");
  }

  {  // exclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kExclusive, {});
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "0.00% (0)");
  }

  {  // of parent
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kOfParent, {});
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00%");
  }

  {  // kModule
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kModule, {});
    EXPECT_FALSE(model.data(index, Qt::DisplayRole).isValid());
  }

  {  // kFunctionAddress
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kFunctionAddress, {});
    EXPECT_FALSE(model.data(index, Qt::DisplayRole).isValid());
  }

  // Function entry
  QModelIndex thread_index = model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, {});
  EXPECT_TRUE(thread_index.isValid());

  {  // function name
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString(kFunctionName));
  }

  {  // inclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kInclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00% (1)");
  }

  {  // exclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kExclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00% (1)");
  }

  {  // of parent
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kOfParent, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00%");
  }

  {  // kModule
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kModule, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString{kModuleName});
  }

  {  // kFunctionAddress
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kFunctionAddress, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString{"0x30"});
  }
}

TEST(CallTreeViewItemModel, GetEditRoleData) {
  std::unique_ptr<orbit_client_model::CaptureData> capture_data = GenerateTestCaptureData();
  // do not create summary, because this creates an additional top level row (all threads) and this
  // is not tested here
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(*capture_data->GetCallstackData(),
                                                          *capture_data, false);

  auto call_tree_view =
      CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(sampling_data, *capture_data);

  CallTreeViewItemModel model{std::move(call_tree_view)};

  // One top level entry for thread with id: kThreadId
  ASSERT_EQ(model.rowCount({}), 1);

  // Thread entry
  {  // thread id
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, {});
    EXPECT_EQ(model.data(index, Qt::EditRole).toInt(), kThreadId);
  }

  {  // inclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kInclusive, {});
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  {  // exclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kExclusive, {});
    bool ok = false;
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(&ok), 0);
    EXPECT_TRUE(ok);
  }

  {  // of parent
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kOfParent, {});
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  {  // kModule
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kModule, {});
    EXPECT_FALSE(model.data(index, Qt::EditRole).isValid());
  }

  {  // kFunctionAddress
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kFunctionAddress, {});
    EXPECT_FALSE(model.data(index, Qt::EditRole).isValid());
  }

  // Function entry
  QModelIndex thread_index = model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, {});
  EXPECT_TRUE(thread_index.isValid());

  {  // function name
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, thread_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toString(), QString(kFunctionName));
  }

  {  // inclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kInclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  {  // exclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kExclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  {  // of parent
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kOfParent, thread_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  {  // kModule
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kModule, thread_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toString(), QString{kModuleName});
  }

  {  // kFunctionAddress
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kFunctionAddress, thread_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toLongLong(), kFunctionAbsoluteAddress);
  }
}