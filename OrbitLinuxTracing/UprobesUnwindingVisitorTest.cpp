#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "UprobesUnwindingVisitor.h"

namespace LinuxTracing {

TEST(UprobesFunctionCallManager, OneUprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 2);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 100);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 1);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 2);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
}

TEST(UprobesFunctionCallManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  function_call_manager.ProcessUprobes(tid, 200, 2);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 3);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 200);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 2);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 3);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 1);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 100);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 1);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 4);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);

  function_call_manager.ProcessUprobes(tid, 300, 5);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 6);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 300);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 5);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 6);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
}

TEST(UprobesFunctionCallManager, TwoUprobesDifferentThreads) {
  constexpr pid_t tid = 42;
  constexpr pid_t tid2 = 111;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  function_call_manager.ProcessUprobes(tid, 100, 1);

  function_call_manager.ProcessUprobes(tid2, 200, 2);

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 3);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 100);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 1);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 3);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);

  processed_function_call = function_call_manager.ProcessUretprobes(tid2, 4);
  ASSERT_TRUE(processed_function_call.has_value());
  EXPECT_EQ(processed_function_call.value().GetTid(), tid2);
  EXPECT_EQ(processed_function_call.value().GetVirtualAddress(), 200);
  EXPECT_EQ(processed_function_call.value().GetBeginTimestampNs(), 2);
  EXPECT_EQ(processed_function_call.value().GetEndTimestampNs(), 4);
  EXPECT_EQ(processed_function_call.value().GetDepth(), 0);
}

TEST(UprobesFunctionCallManager, OnlyUretprobe) {
  constexpr pid_t tid = 42;
  std::optional<FunctionCall> processed_function_call;
  UprobesFunctionCallManager function_call_manager;

  processed_function_call = function_call_manager.ProcessUretprobes(tid, 2);
  ASSERT_FALSE(processed_function_call.has_value());
}

namespace {
unwindstack::FrameData MakeTestFrame(std::string function_name) {
  unwindstack::FrameData frame_data{};
  frame_data.function_name = std::move(function_name);
  frame_data.map_name = "a.out";
  return frame_data;
}

std::vector<unwindstack::FrameData> MakeTestCallstack(
    const std::vector<std::string>& function_names) {
  std::vector<unwindstack::FrameData> callstack;
  for (auto name_it = function_names.rbegin(); name_it != function_names.rend();
       ++name_it) {
    callstack.push_back(MakeTestFrame(*name_it));
  }
  return callstack;
}

unwindstack::FrameData MakeTestUprobesFrame() {
  unwindstack::FrameData uprobes_frame{};
  uprobes_frame.function_name = "uprobes";
  uprobes_frame.map_name = "[uprobes]";
  return uprobes_frame;
}

std::vector<unwindstack::FrameData> MakeTestUprobesCallstack(
    const std::vector<std::string>& function_names) {
  std::vector<unwindstack::FrameData> callstack;
  for (auto name_it = function_names.rbegin(); name_it != function_names.rend();
       ++name_it) {
    callstack.push_back(MakeTestFrame(*name_it));
  }
  callstack.push_back(MakeTestUprobesFrame());
  return callstack;
}

std::vector<std::pair<std::string, std::string>>
TestCallstackToStringPairVector(
    const std::vector<unwindstack::FrameData>& callstack) {
  std::vector<std::pair<std::string, std::string>> vector{};
  for (const auto& frame : callstack) {
    vector.emplace_back(frame.map_name, frame.function_name);
  }
  return vector;
}
}  // namespace

TEST(UprobesCallstackManager, NoUprobes) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "beta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "gamma"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, OneUprobe) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Uprobes corresponding to the function FUNCTION being called.
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION", "beta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Uretprobes corresponding to FUNCTION returning.
  callstack_manager.ProcessUretprobes(tid);
}

TEST(UprobesCallstackManager, OneUprobeNoCallstackOnUretprobe) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};
  // FUNCTION is called.
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION", "beta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "gamma"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, DifferentThread) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // FUNCTION is called.
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Sample from another thread.
  unwound_cs = expected_cs = MakeTestCallstack({"thread", "omega"});
  processed_cs = callstack_manager.ProcessSampledCallstack(111, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

TEST(UprobesCallstackManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FOO is called.
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FOO"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // BAR is called.
  unwound_cs = MakeTestUprobesCallstack({"FOO", "beta", "BAR"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO", "beta", "BAR"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"BAR", "gamma"});
  expected_cs =
      MakeTestCallstack({"main", "alpha", "FOO", "beta", "BAR", "gamma"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // BAR returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = MakeTestUprobesCallstack({"FOO", "delta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO", "delta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FOO returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = expected_cs = MakeTestCallstack({"main"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION is called.
  unwound_cs = expected_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION"});
  expected_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "zeta"});
  expected_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION", "zeta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = expected_cs = MakeTestCallstack({"main"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, UnwindingError) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // FUNCTION is called.
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Unwind error.
  unwound_cs = expected_cs = MakeTestCallstack({});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

TEST(UprobesCallstackManager, UnwindingErrorOnStack) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // FUNCTION is called and this uprobes has an unwind error.
  unwound_cs = expected_cs = MakeTestCallstack({});
  processed_cs = callstack_manager.ProcessUprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  expected_cs = MakeTestCallstack({});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

}  // namespace LinuxTracing
