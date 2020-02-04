#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "LinuxUprobesUnwindingVisitor.h"

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
  std::vector<unwindstack::FrameData> unwound_cs, real_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha", "beta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha", "gamma"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));
}

TEST(UprobesCallstackManager, OneUprobe) {
  std::vector<unwindstack::FrameData> unwound_cs, real_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // Begin FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION"});
  real_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  real_cs = MakeTestCallstack({"main", "alpha", "FUNCTION", "beta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));
}

TEST(UprobesCallstackManager, DifferentThread) {
  std::vector<unwindstack::FrameData> unwound_cs, real_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // Begin FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // Sample from another thread.
  unwound_cs = real_cs = MakeTestCallstack({"thread", "omega"});
  processed_cs = callstack_manager.ProcessSampledCallstack(111, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));
}

TEST(UprobesCallstackManager, TwoNestedUprobesAndAnotherUprobe) {
  std::vector<unwindstack::FrameData> unwound_cs, real_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // Begin FOO().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FOO"});
  real_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // Begin BAR().
  unwound_cs = MakeTestUprobesCallstack({"FOO", "beta", "BAR"});
  real_cs = MakeTestCallstack({"main", "alpha", "FOO", "beta", "BAR"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"BAR", "gamma"});
  real_cs = MakeTestCallstack({"main", "alpha", "FOO", "beta", "BAR", "gamma"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End BAR().
  unwound_cs = MakeTestUprobesCallstack({"FOO"});
  real_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({
      "FOO",
      "delta",
  });
  real_cs = MakeTestCallstack({"main", "alpha", "FOO", "delta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End FOO().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = real_cs = MakeTestCallstack({"main"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // Begin FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION"});
  real_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "zeta"});
  real_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION", "zeta"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "epsilon"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = real_cs = MakeTestCallstack({"main"});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));
}

TEST(UprobesCallstackManager, UnwindingError) {
  std::vector<unwindstack::FrameData> unwound_cs, real_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // Begin FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // Unwind error.
  unwound_cs = real_cs = MakeTestCallstack({});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));
}

TEST(UprobesCallstackManager, UnwindingErrorOnStack) {
  std::vector<unwindstack::FrameData> unwound_cs, real_cs, processed_cs;
  UprobesCallstackManager callstack_manager{};

  // Begin FUNCTION() with unwind error.
  unwound_cs = real_cs = MakeTestCallstack({});
  processed_cs = callstack_manager.ProcessUprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  real_cs = MakeTestCallstack({});
  processed_cs = callstack_manager.ProcessSampledCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));

  // End FUNCTION().
  unwound_cs = real_cs = MakeTestCallstack({"main", "alpha"});
  processed_cs = callstack_manager.ProcessUretprobesCallstack(42, unwound_cs);
  EXPECT_THAT(
      TestCallstackToStringPairVector(processed_cs),
      ::testing::ElementsAreArray(TestCallstackToStringPairVector(real_cs)));
}
