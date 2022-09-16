// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <vector>

#include "GrpcProtos/capture.pb.h"
#include "MockTracerListener.h"
#include "PerfEvent.h"
#include "UprobesFunctionCallManager.h"
#include "UprobesUnwindingVisitor.h"
#include "UprobesUnwindingVisitorTestCommon.h"

using ::testing::ElementsAre;
using ::testing::Mock;
using ::testing::SaveArg;

namespace orbit_linux_tracing {

namespace {

class UprobesUnwindingVisitorDynamicInstrumentationTest : public ::testing::Test {
 protected:
  MockTracerListener listener_;
  MockUprobesReturnAddressManager return_address_manager_{nullptr};

  UprobesUnwindingVisitor visitor_{&listener_,
                                   &function_call_manager_,
                                   &return_address_manager_,
                                   &maps_,
                                   &unwinder_,
                                   &leaf_function_call_manager_,
                                   /*user_space_instrumentation_addresses=*/nullptr,
                                   /*absolute_address_to_size_of_functions_to_stop_at_=*/nullptr};

 private:
  UprobesFunctionCallManager function_call_manager_;
  MockLibunwindstackMaps maps_;
  MockLibunwindstackUnwinder unwinder_;
  MockLeafFunctionCallManager leaf_function_call_manager_{128};
};

}  // namespace

TEST_F(UprobesUnwindingVisitorDynamicInstrumentationTest,
       VisitDynamicInstrumentationPerfEventsInVariousCombinationsSendsFunctionCalls) {
  constexpr pid_t kPid = 42;
  constexpr pid_t kTid = 43;
  visitor_.SetInitialTidToRootNamespaceTidMapping({{kPid, kPid}, {kTid, kTid}});
  constexpr uint32_t kCpu = 1;

  {
    UprobesPerfEvent uprobe1{
        .timestamp = 100,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 1,
                .sp = 0x50,
                .ip = 0x01,
                .return_address = 0x00,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x50, 0x00)).Times(1);
    PerfEvent{uprobe1}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UprobesWithArgumentsPerfEvent uprobe2{
        .timestamp = 200,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 2,
                .return_address = 0x01,
                .regs =
                    {
                        .cx = 4,
                        .dx = 3,
                        .si = 2,
                        .di = 1,
                        .sp = 0x40,
                        .ip = 0x02,
                        .r8 = 5,
                        .r9 = 6,
                    },
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x40, 0x01)).Times(1);
    PerfEvent{uprobe2}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UserSpaceFunctionEntryPerfEvent function_entry3{
        .timestamp = 300,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .function_id = 3,
                .sp = 0x30,
                .return_address = 0x02,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x30, 0x02)).Times(1);
    PerfEvent{function_entry3}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UprobesPerfEvent uprobe4{
        .timestamp = 400,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 4,
                .sp = 0x20,
                .ip = 0x04,
                .return_address = 0x03,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x20, 0x03)).Times(1);
    PerfEvent{uprobe4}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UprobesWithArgumentsPerfEvent uprobe5{
        .timestamp = 500,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .cpu = kCpu,
                .function_id = 5,
                .return_address = 0x04,
                .regs =
                    {
                        .cx = 4,
                        .dx = 3,
                        .si = 2,
                        .di = 1,
                        .sp = 0x10,
                        .ip = 0x05,
                        .r8 = 5,
                        .r9 = 6,
                    },
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTid, 0x10, 0x04)).Times(1);
    PerfEvent{uprobe5}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UretprobesWithReturnValuePerfEvent uretprobe5{
        .timestamp = 600,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .rax = 456,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe5}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 5);
    EXPECT_EQ(actual_function_call.duration_ns(), 100);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 600);
    EXPECT_EQ(actual_function_call.depth(), 4);
    EXPECT_EQ(actual_function_call.return_value(), 456);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre(1, 2, 3, 4, 5, 6));
  }

  {
    UretprobesWithReturnValuePerfEvent uretprobe4{
        .timestamp = 700,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
                .rax = 123,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe4}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 4);
    EXPECT_EQ(actual_function_call.duration_ns(), 300);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 700);
    EXPECT_EQ(actual_function_call.depth(), 3);
    EXPECT_EQ(actual_function_call.return_value(), 123);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }

  {
    UserSpaceFunctionExitPerfEvent function_exit3{
        .timestamp = 800,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{function_exit3}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 3);
    EXPECT_EQ(actual_function_call.duration_ns(), 500);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 800);
    EXPECT_EQ(actual_function_call.depth(), 2);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }

  {
    UretprobesPerfEvent uretprobe2{
        .timestamp = 900,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe2}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 2);
    EXPECT_EQ(actual_function_call.duration_ns(), 700);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 900);
    EXPECT_EQ(actual_function_call.depth(), 1);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre(1, 2, 3, 4, 5, 6));
  }

  {
    UretprobesPerfEvent uretprobe1{
        .timestamp = 1000,
        .data =
            {
                .pid = kPid,
                .tid = kTid,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTid)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{uretprobe1}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPid);
    EXPECT_EQ(actual_function_call.tid(), kTid);
    EXPECT_EQ(actual_function_call.function_id(), 1);
    EXPECT_EQ(actual_function_call.duration_ns(), 900);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 1000);
    EXPECT_EQ(actual_function_call.depth(), 0);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }
}

TEST_F(UprobesUnwindingVisitorDynamicInstrumentationTest,
       VisitDynamicInstrumentationPerfEventsWithTidNamespaceTranslation) {
  constexpr pid_t kPidTargetNamespace = 42;
  constexpr pid_t kTidTargetNamespace = 43;
  constexpr pid_t kPidRootNamespace = 1042;
  constexpr pid_t kTidRootNamespace = 1043;
  visitor_.SetInitialTidToRootNamespaceTidMapping(
      {{kPidTargetNamespace, kPidRootNamespace}, {kTidTargetNamespace, kTidRootNamespace}});

  constexpr pid_t kPidUnkown = 44;
  constexpr pid_t kTidUnkown = 45;

  constexpr pid_t kTidNewTargetNamespace = 54;
  constexpr pid_t kTidNewRootNamespace = 1054;

  {
    UserSpaceFunctionEntryPerfEvent function_entry1{
        .timestamp = 300,
        .data =
            {
                .pid = kPidTargetNamespace,
                .tid = kTidTargetNamespace,
                .function_id = 1,
                .sp = 0x30,
                .return_address = 0x02,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTidRootNamespace, 0x30, 0x02))
        .Times(1);
    PerfEvent{function_entry1}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UserSpaceFunctionExitPerfEvent function_exit1{
        .timestamp = 800,
        .data =
            {
                .pid = kPidTargetNamespace,
                .tid = kTidTargetNamespace,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTidRootNamespace)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{function_exit1}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPidRootNamespace);
    EXPECT_EQ(actual_function_call.tid(), kTidRootNamespace);
    EXPECT_EQ(actual_function_call.function_id(), 1);
    EXPECT_EQ(actual_function_call.duration_ns(), 500);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 800);
    EXPECT_EQ(actual_function_call.depth(), 0);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }

  {
    UserSpaceFunctionEntryPerfEvent function_entry2{
        .timestamp = 300,
        .data =
            {
                .pid = kPidUnkown,
                .tid = kTidUnkown,
                .function_id = 1,
                .sp = 0x30,
                .return_address = 0x02,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTidUnkown, 0x30, 0x02)).Times(0);
    PerfEvent{function_entry2}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UserSpaceFunctionExitPerfEvent function_exit2{
        .timestamp = 800,
        .data =
            {
                .pid = kPidUnkown,
                .tid = kTidUnkown,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTidUnkown)).Times(0);
    EXPECT_CALL(listener_, OnFunctionCall).Times(0);
    PerfEvent{function_exit2}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
  }

  {
    TaskNewtaskPerfEvent task_newtask{
        .timestamp = 1000,
        .data =
            {
                .new_tid = kTidNewRootNamespace,
                .was_created_by_tid = kTidRootNamespace,
            },
    };
    PerfEvent{task_newtask}.Accept(&visitor_);
    CloneExitPerfEvent clone_exit_event{
        .timestamp = 1001,
        .data =
            {
                .tid = kTidRootNamespace,
                .ret_tid = kTidNewTargetNamespace,
            },
    };
    PerfEvent{clone_exit_event}.Accept(&visitor_);
  }

  {
    UserSpaceFunctionEntryPerfEvent function_entry3{
        .timestamp = 1300,
        .data =
            {
                .pid = kPidTargetNamespace,
                .tid = kTidNewTargetNamespace,
                .function_id = 3,
                .sp = 0x30,
                .return_address = 0x02,
            },
    };
    EXPECT_CALL(return_address_manager_, ProcessFunctionEntry(kTidNewRootNamespace, 0x30, 0x02))
        .Times(1);
    PerfEvent{function_entry3}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
  }

  {
    UserSpaceFunctionExitPerfEvent function_exit3{
        .timestamp = 1800,
        .data =
            {
                .pid = kPidTargetNamespace,
                .tid = kTidNewTargetNamespace,
            },
    };

    EXPECT_CALL(return_address_manager_, ProcessFunctionExit(kTidNewRootNamespace)).Times(1);
    orbit_grpc_protos::FunctionCall actual_function_call;
    EXPECT_CALL(listener_, OnFunctionCall).Times(1).WillOnce(SaveArg<0>(&actual_function_call));
    PerfEvent{function_exit3}.Accept(&visitor_);
    Mock::VerifyAndClearExpectations(&return_address_manager_);
    Mock::VerifyAndClearExpectations(&listener_);
    EXPECT_EQ(actual_function_call.pid(), kPidRootNamespace);
    EXPECT_EQ(actual_function_call.tid(), kTidNewRootNamespace);
    EXPECT_EQ(actual_function_call.function_id(), 3);
    EXPECT_EQ(actual_function_call.duration_ns(), 500);
    EXPECT_EQ(actual_function_call.end_timestamp_ns(), 1800);
    EXPECT_EQ(actual_function_call.depth(), 0);
    EXPECT_EQ(actual_function_call.return_value(), 0);
    EXPECT_THAT(actual_function_call.registers(), ElementsAre());
  }
}

}  // namespace orbit_linux_tracing
