#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "LinuxUprobesUnwindingVisitor.h"

TEST(UprobesTimerManager, OneUprobe) {
  constexpr pid_t tid = 42;
  bool process_uretprobes_res;
  Timer processed_timer;
  UprobesTimerManager timer_manager;

  timer_manager.ProcessUprobes(tid, 1, 100);

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid, 2, &processed_timer);
  ASSERT_TRUE(process_uretprobes_res);
  EXPECT_EQ(processed_timer.m_TID, tid);
  EXPECT_EQ(processed_timer.m_Start, 1);
  EXPECT_EQ(processed_timer.m_End, 2);
  EXPECT_EQ(processed_timer.m_Depth, 0);
  EXPECT_EQ(processed_timer.m_FunctionAddress, 100);
}

TEST(UprobesTimerManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t tid = 42;
  bool process_uretprobes_res;
  Timer processed_timer;
  UprobesTimerManager timer_manager;

  timer_manager.ProcessUprobes(tid, 1, 100);

  timer_manager.ProcessUprobes(tid, 2, 200);

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid, 3, &processed_timer);
  ASSERT_TRUE(process_uretprobes_res);
  EXPECT_EQ(processed_timer.m_TID, tid);
  EXPECT_EQ(processed_timer.m_Start, 2);
  EXPECT_EQ(processed_timer.m_End, 3);
  EXPECT_EQ(processed_timer.m_Depth, 1);
  EXPECT_EQ(processed_timer.m_FunctionAddress, 200);

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid, 4, &processed_timer);
  ASSERT_TRUE(process_uretprobes_res);
  EXPECT_EQ(processed_timer.m_TID, tid);
  EXPECT_EQ(processed_timer.m_Start, 1);
  EXPECT_EQ(processed_timer.m_End, 4);
  EXPECT_EQ(processed_timer.m_Depth, 0);
  EXPECT_EQ(processed_timer.m_FunctionAddress, 100);

  timer_manager.ProcessUprobes(tid, 5, 300);

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid, 6, &processed_timer);
  ASSERT_TRUE(process_uretprobes_res);
  EXPECT_EQ(processed_timer.m_TID, tid);
  EXPECT_EQ(processed_timer.m_Start, 5);
  EXPECT_EQ(processed_timer.m_End, 6);
  EXPECT_EQ(processed_timer.m_Depth, 0);
  EXPECT_EQ(processed_timer.m_FunctionAddress, 300);
}

TEST(UprobesTimerManager, TwoUprobesDifferentThreads) {
  constexpr pid_t tid = 42;
  constexpr pid_t tid2 = 111;
  bool process_uretprobes_res;
  Timer processed_timer;
  UprobesTimerManager timer_manager;

  timer_manager.ProcessUprobes(tid, 1, 100);

  timer_manager.ProcessUprobes(tid2, 2, 200);

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid, 3, &processed_timer);
  ASSERT_TRUE(process_uretprobes_res);
  EXPECT_EQ(processed_timer.m_TID, tid);
  EXPECT_EQ(processed_timer.m_Start, 1);
  EXPECT_EQ(processed_timer.m_End, 3);
  EXPECT_EQ(processed_timer.m_Depth, 0);
  EXPECT_EQ(processed_timer.m_FunctionAddress, 100);

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid2, 4, &processed_timer);
  ASSERT_TRUE(process_uretprobes_res);
  EXPECT_EQ(processed_timer.m_TID, tid2);
  EXPECT_EQ(processed_timer.m_Start, 2);
  EXPECT_EQ(processed_timer.m_End, 4);
  EXPECT_EQ(processed_timer.m_Depth, 0);
  EXPECT_EQ(processed_timer.m_FunctionAddress, 200);
}

TEST(UprobesTimerManager, OnlyRetprobe) {
  constexpr pid_t tid = 42;
  bool process_uretprobes_res;
  Timer processed_timer;
  UprobesTimerManager timer_manager;

  process_uretprobes_res =
      timer_manager.ProcessUretprobes(tid, 2, &processed_timer);
  ASSERT_FALSE(process_uretprobes_res);
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

  // Begin FUNCTION().
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

  // End FUNCTION().
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, DifferentThread) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // Begin FUNCTION().
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

  // End FUNCTION().
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
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

  // Begin FOO().
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

  // Begin BAR().
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

  // End BAR().
  unwound_cs = MakeTestUprobesCallstack({"FOO"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FOO", "delta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO", "delta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // End FOO().
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = expected_cs = MakeTestCallstack({"main"});
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Begin FUNCTION().
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

  // End FUNCTION().
  unwound_cs = expected_cs = MakeTestCallstack({"main", "epsilon"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

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

  // Begin FUNCTION().
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

  // End FUNCTION().
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, UnwindingErrorOnStack) {
  constexpr pid_t tid = 42;
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // Begin FUNCTION() with unwind error.
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

  // End FUNCTION().
  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(tid, unwound_cs);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}
