// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <gtest/gtest.h>

#include <QAbstractItemModelTester>
#include <QModelIndex>
#include <QString>
#include <QStringLiteral>
#include <QVariant>
#include <Qt>
#include <algorithm>
#include <cstdint>
#include <memory>
#include <optional>
#include <string>
#include <utility>
#include <vector>

#include "ClientData/CallstackEvent.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/CallstackType.h"
#include "ClientData/CaptureData.h"
#include "ClientData/LinuxAddressInfo.h"
#include "ClientData/ModuleIdentifierProvider.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/PostProcessedSamplingData.h"
#include "ClientModel/SamplingDataPostProcessor.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitGl/CallTreeView.h"
#include "OrbitQt/CallTreeViewItemModel.h"
#include "QtUtils/AssertNoQtLogWarnings.h"

constexpr uint64_t kCallstackId = 1;
constexpr uint64_t kCallstackId2 = 1;
constexpr uint64_t kUnwindErrorCallstackId = 2;
constexpr uint64_t kFunctionAbsoluteAddress = 0x30;
constexpr uint64_t kInstructionAbsoluteAddress = 0x31;
constexpr uint64_t kTimestamp1 = 1234;
constexpr uint64_t kTimestamp2 = 2345;
constexpr uint64_t kTimestamp3 = 3456;
constexpr int32_t kThreadId = 42;
constexpr int32_t kThreadId2 = 43;
constexpr const char* kFunctionName = "example function";
constexpr const char* kModuleName = "example module";
constexpr const char* kThreadName = "example thread";

namespace {

using orbit_client_data::CallstackInfo;
using orbit_client_data::CallstackType;
using orbit_client_data::CaptureData;

std::unique_ptr<CaptureData> GenerateTestCaptureData(
    const orbit_client_data::ModuleIdentifierProvider* module_identifier_provider) {
  auto capture_data = std::make_unique<CaptureData>(
      orbit_grpc_protos::CaptureStarted{}, std::nullopt, absl::flat_hash_set<uint64_t>{},
      CaptureData::DataSource::kLiveCapture, module_identifier_provider);

  // AddressInfo
  orbit_client_data::LinuxAddressInfo address_info{
      kInstructionAbsoluteAddress, kInstructionAbsoluteAddress - kFunctionAbsoluteAddress,
      kModuleName, kFunctionName};
  capture_data->InsertAddressInfo(std::move(address_info));

  // CallstackInfo
  const std::vector<uint64_t> callstack_frames{kInstructionAbsoluteAddress};
  CallstackInfo callstack_info{callstack_frames, CallstackType::kComplete};
  capture_data->AddUniqueCallstack(kCallstackId, std::move(callstack_info));

  // CallstackEvent 1
  orbit_client_data::CallstackEvent callstack_event_1{kTimestamp1, kCallstackId, kThreadId};
  capture_data->AddCallstackEvent(callstack_event_1);

  // CallstackEvent 2
  orbit_client_data::CallstackEvent callstack_event_2{kTimestamp2, kCallstackId, kThreadId};
  capture_data->AddCallstackEvent(callstack_event_2);

  // CallstackInfo
  const std::vector<uint64_t> callstack_error_frames{kInstructionAbsoluteAddress};
  CallstackInfo callstack_error_info{callstack_error_frames,
                                     CallstackType::kFramePointerUnwindingError};

  capture_data->AddUniqueCallstack(kUnwindErrorCallstackId, std::move(callstack_error_info));

  // CallstackEvent
  orbit_client_data::CallstackEvent callstack_error_event{4098, kUnwindErrorCallstackId, kThreadId};
  capture_data->AddCallstackEvent(callstack_error_event);

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
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider;
  orbit_qt_utils::AssertNoQtLogWarnings message_handler{};

  std::unique_ptr<CaptureData> capture_data = GenerateTestCaptureData(&module_identifier_provider);
  orbit_client_data::ModuleManager module_manager{&module_identifier_provider};
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(capture_data->GetCallstackData(),
                                                          *capture_data, module_manager);

  auto call_tree_view = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      sampling_data, &module_manager, capture_data.get());

  CallTreeViewItemModel model{std::move(call_tree_view)};

  QAbstractItemModelTester(&model, QAbstractItemModelTester::FailureReportingMode::Warning);
}

TEST(CallTreeViewItemModel, RowsWithoutSummaryItem) {
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider;
  std::unique_ptr<CaptureData> capture_data = GenerateTestCaptureData(&module_identifier_provider);

  orbit_client_data::ModuleManager module_manager{&module_identifier_provider};
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(capture_data->GetCallstackData(),
                                                          *capture_data, module_manager);
  auto call_tree_view = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      sampling_data, &module_manager, capture_data.get());

  CallTreeViewItemModel model{std::move(call_tree_view)};

  EXPECT_EQ(model.rowCount({}), 1);
}

TEST(CallTreeViewItemModel, RowsWithSummaryItem) {
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider;
  std::unique_ptr<CaptureData> capture_data = GenerateTestCaptureData(&module_identifier_provider);
  orbit_client_data::CallstackEvent callstack_event_1{kTimestamp3, kCallstackId2, kThreadId2};
  capture_data->AddCallstackEvent(callstack_event_1);

  orbit_client_data::ModuleManager module_manager{&module_identifier_provider};
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(capture_data->GetCallstackData(),
                                                          *capture_data, module_manager);
  auto call_tree_view = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      sampling_data, &module_manager, capture_data.get());

  CallTreeViewItemModel model{std::move(call_tree_view)};

  EXPECT_EQ(model.rowCount({}), 3);
}

TEST(CallTreeViewItemModel, GetDisplayRoleData) {
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider;
  std::unique_ptr<CaptureData> capture_data = GenerateTestCaptureData(&module_identifier_provider);
  orbit_client_data::ModuleManager module_manager{&module_identifier_provider};
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(capture_data->GetCallstackData(),
                                                          *capture_data, module_manager);

  auto call_tree_view = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      sampling_data, &module_manager, capture_data.get());

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
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00% (3)");
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
  ASSERT_EQ(model.rowCount(thread_index), 2);

  {  // function name
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString(kFunctionName));
  }

  {  // inclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kInclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "66.67% (2)");
  }

  {  // exclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kExclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "66.67% (2)");
  }

  {  // of parent
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kOfParent, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "66.67%");
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

  {  // error string
    QModelIndex index =
        model.index(1, CallTreeViewItemModel::Columns::kThreadOrFunction, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QStringLiteral("[Unwind errors]"));
  }

  {  // inclusive
    QModelIndex index = model.index(1, CallTreeViewItemModel::Columns::kInclusive, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "33.33% (1)");
  }

  {  // of parent
    QModelIndex index = model.index(1, CallTreeViewItemModel::Columns::kOfParent, thread_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "33.33%");
  }

  // Unwind errors entry
  QModelIndex unwind_errors_index =
      model.index(1, CallTreeViewItemModel::Columns::kThreadOrFunction, thread_index);
  EXPECT_TRUE(unwind_errors_index.isValid());
  ASSERT_EQ(model.rowCount(unwind_errors_index), 1);

  {  // error type
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, unwind_errors_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(),
              QString::fromStdString(orbit_client_data::CallstackTypeToString(
                  CallstackType::kFramePointerUnwindingError)));
  }

  {  // inclusive
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kInclusive, unwind_errors_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "33.33% (1)");
  }

  {  // of parent
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kOfParent, unwind_errors_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00%");
  }

  // Unwind error function entry
  QModelIndex unwinding_error_type_index =
      model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, unwind_errors_index);
  EXPECT_TRUE(unwinding_error_type_index.isValid());
  ASSERT_EQ(model.rowCount(unwinding_error_type_index), 1);

  {  // function name
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction,
                                    unwinding_error_type_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString(kFunctionName));
  }

  {  // inclusive
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kInclusive, unwinding_error_type_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "33.33% (1)");
  }

  {  // exclusive
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kExclusive, unwinding_error_type_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "33.33% (1)");
  }

  {  // of parent
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kOfParent, unwinding_error_type_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), "100.00%");
  }

  {  // kModule
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kModule, unwinding_error_type_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString{kModuleName});
  }

  {  // kFunctionAddress
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kFunctionAddress,
                                    unwinding_error_type_index);
    EXPECT_EQ(model.data(index, Qt::DisplayRole).toString(), QString{"0x30"});
  }
}

TEST(CallTreeViewItemModel, GetEditRoleData) {
  orbit_client_data::ModuleIdentifierProvider module_identifier_provider;
  std::unique_ptr<CaptureData> capture_data = GenerateTestCaptureData(&module_identifier_provider);
  orbit_client_data::ModuleManager module_manager{&module_identifier_provider};
  orbit_client_data::PostProcessedSamplingData sampling_data =
      orbit_client_model::CreatePostProcessedSamplingData(capture_data->GetCallstackData(),
                                                          *capture_data, module_manager);

  auto call_tree_view = CallTreeView::CreateTopDownViewFromPostProcessedSamplingData(
      sampling_data, &module_manager, capture_data.get());

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
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 66.67, 0.01);
  }

  {  // exclusive
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kExclusive, thread_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 66.67, 0.01);
  }

  {  // of parent
    QModelIndex index = model.index(0, CallTreeViewItemModel::Columns::kOfParent, thread_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 66.67, 0.01);
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

  {  // inclusive
    QModelIndex index = model.index(1, CallTreeViewItemModel::Columns::kInclusive, thread_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 33.33, 0.01);
  }

  {  // of parent
    QModelIndex index = model.index(1, CallTreeViewItemModel::Columns::kOfParent, thread_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 33.33, 0.01);
  }

  // Unwind errors entry
  QModelIndex unwind_errors_index =
      model.index(1, CallTreeViewItemModel::Columns::kThreadOrFunction, thread_index);
  EXPECT_TRUE(unwind_errors_index.isValid());
  ASSERT_EQ(model.rowCount(unwind_errors_index), 1);

  {  // error type
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, unwind_errors_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toString(),
              QString::fromStdString(orbit_client_data::CallstackTypeToString(
                  CallstackType::kFramePointerUnwindingError)));
  }

  {  // inclusive
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kInclusive, unwind_errors_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 33.33, 0.01);
  }

  {  // of parent
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kOfParent, unwind_errors_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  // Unwind error type entry
  QModelIndex unwind_error_type_index =
      model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, unwind_errors_index);
  EXPECT_TRUE(unwind_error_type_index.isValid());
  ASSERT_EQ(model.rowCount(unwind_error_type_index), 1);

  {  // function name
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kThreadOrFunction, unwind_error_type_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toString(), QString(kFunctionName));
  }

  {  // inclusive
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kInclusive, unwind_error_type_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 33.33, 0.01);
  }

  {  // exclusive
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kExclusive, unwind_error_type_index);
    EXPECT_NEAR(model.data(index, Qt::EditRole).toFloat(), 33.33, 0.01);
  }

  {  // of parent
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kOfParent, unwind_error_type_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toFloat(), 100);
  }

  {  // kModule
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kModule, unwind_error_type_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toString(), QString{kModuleName});
  }

  {  // kFunctionAddress
    QModelIndex index =
        model.index(0, CallTreeViewItemModel::Columns::kFunctionAddress, unwind_error_type_index);
    EXPECT_EQ(model.data(index, Qt::EditRole).toLongLong(), kFunctionAbsoluteAddress);
  }
}