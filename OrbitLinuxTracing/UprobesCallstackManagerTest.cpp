#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "UprobesCallstackManager.h"

namespace LinuxTracing {

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

std::vector<unwindstack::FrameData> MakeTestUnwindingErrorCallstack() {
  return MakeTestCallstack({});
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

class TestUnwinder {
 public:
  static std::unique_ptr<unwindstack::BufferMaps> ParseMaps(
      const std::string& maps_buffer) {
    auto maps = std::make_unique<unwindstack::BufferMaps>("");
    maps->Parse();
    return maps;
  }

  std::vector<unwindstack::FrameData> Unwind(
      unwindstack::Maps* maps,
      const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
      const char* stack_dump, uint64_t stack_dump_size) {
    return stack_dump_sizes_to_callstack.at(stack_dump_size);
  }

  std::vector<unwindstack::FrameData> Unwind(
      const std::string& maps_buffer,
      const std::array<uint64_t, PERF_REG_X86_64_MAX>& perf_regs,
      const char* stack_dump, uint64_t stack_dump_size) {
    return stack_dump_sizes_to_callstack.at(stack_dump_size);
  }

  uint64_t GetNextStackDumpSize() {
    ++next_stack_dump_size_;
    return next_stack_dump_size_;
  }

  void RegisterStackDumpSizeToCallstack(
      uint64_t stack_dump_size, std::vector<unwindstack::FrameData> callstack) {
    stack_dump_sizes_to_callstack.emplace(
        std::make_pair(stack_dump_size, std::move(callstack)));
  }

 private:
  // Hack for testing: use stack_dump_size as an id to get the desired callstack
  // as the result of unwinding.
  absl::flat_hash_map<uint64_t, std::vector<unwindstack::FrameData>>
      stack_dump_sizes_to_callstack{};
  uint64_t next_stack_dump_size_ = 0;
};

UprobesWithStackPerfEvent MakeTestUprobesWithStackAndRegisterOnTestUnwinder(
    const std::vector<unwindstack::FrameData>& unwound_cs,
    TestUnwinder* unwinder) {
  UprobesWithStackPerfEvent event =
      UprobesWithStackPerfEvent{unwinder->GetNextStackDumpSize()};
  unwinder->RegisterStackDumpSizeToCallstack(event.GetStackSize(), unwound_cs);
  return event;
}

StackSamplePerfEvent MakeTestStackSampleAndRegisterOnTestUnwinder(
    const std::vector<unwindstack::FrameData>& unwound_cs,
    TestUnwinder* unwinder) {
  StackSamplePerfEvent event =
      StackSamplePerfEvent{unwinder->GetNextStackDumpSize()};
  unwinder->RegisterStackDumpSizeToCallstack(event.GetStackSize(), unwound_cs);
  return event;
}
}  // namespace

TEST(UprobesCallstackManager, NoUprobes) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "beta"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "gamma"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, OneUprobe) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};
  UprobesWithStackPerfEvent uprobes_event{0};

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Uprobes corresponding to the function FUNCTION being called.
  unwound_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION", "beta"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // Uretprobes corresponding to FUNCTION returning.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha", "gamma"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, DifferentThread) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};
  UprobesWithStackPerfEvent uprobes_event{0};

  // FUNCTION is called.
  unwound_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  // Sample from another thread.
  unwound_cs = expected_cs = MakeTestCallstack({"thread", "omega"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(111, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

TEST(UprobesCallstackManager, TwoNestedUprobesAndAnotherUprobe) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};
  UprobesWithStackPerfEvent uprobes_event{0};

  unwound_cs = expected_cs = MakeTestCallstack({"main", "alpha"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FOO is called.
  unwound_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"FOO"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // BAR is called.
  unwound_cs = MakeTestUprobesCallstack({"FOO", "beta", "BAR"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"BAR", "gamma"});
  expected_cs =
      MakeTestCallstack({"main", "alpha", "FOO", "beta", "BAR", "gamma"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // BAR returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = MakeTestUprobesCallstack({"FOO", "delta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FOO", "delta"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FOO returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = expected_cs = MakeTestCallstack({"main"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION is called.
  unwound_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION"});
  expected_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "zeta"});
  expected_cs = MakeTestCallstack({"main", "epsilon", "FUNCTION", "zeta"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = expected_cs = MakeTestCallstack({"main"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));
}

TEST(UprobesCallstackManager, UnwindingError) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};
  UprobesWithStackPerfEvent uprobes_event{0};

  // FUNCTION is called.
  unwound_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  // Unwind error.
  unwound_cs = expected_cs = MakeTestUnwindingErrorCallstack();
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

TEST(UprobesCallstackManager, UnwindingErrorOnStack) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};
  UprobesWithStackPerfEvent uprobes_event{0};

  // FUNCTION is called and this uprobes has an unwind error.
  unwound_cs = MakeTestUnwindingErrorCallstack();
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  expected_cs = MakeTestUnwindingErrorCallstack();
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

TEST(UprobesCallstackManager, UnwindingErrorOnStackThenValid) {
  constexpr pid_t tid = 42;
  TestUnwinder unwinder{};
  UprobesCallstackManager<TestUnwinder> callstack_manager{&unwinder, ""};
  std::vector<unwindstack::FrameData> unwound_cs, expected_cs, processed_cs;
  StackSamplePerfEvent sample_event{0};
  UprobesWithStackPerfEvent uprobes_event{0};

  // FUNCTION is called.
  unwound_cs = MakeTestCallstack({"main", "alpha", "FUNCTION"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  // FOO is called and this uprobes has an unwind error.
  unwound_cs = MakeTestUnwindingErrorCallstack();
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"FOO", "gamma"});
  expected_cs = MakeTestUnwindingErrorCallstack();
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // FOO returns.
  callstack_manager.ProcessUretprobes(tid);

  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta"});
  expected_cs = MakeTestCallstack({"main", "alpha", "FUNCTION", "beta"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // BAR is called.
  unwound_cs = MakeTestUprobesCallstack({"FUNCTION", "beta", "BAR"});
  uprobes_event =
      MakeTestUprobesWithStackAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  callstack_manager.ProcessUprobesCallstack(tid, std::move(uprobes_event));

  unwound_cs = MakeTestUprobesCallstack({"BAR", "delta"});
  expected_cs =
      MakeTestCallstack({"main", "alpha", "FUNCTION", "beta", "BAR", "delta"});
  sample_event =
      MakeTestStackSampleAndRegisterOnTestUnwinder(unwound_cs, &unwinder);
  processed_cs = callstack_manager.ProcessSampledCallstack(tid, sample_event);
  EXPECT_THAT(TestCallstackToStringPairVector(processed_cs),
              ::testing::ElementsAreArray(
                  TestCallstackToStringPairVector(expected_cs)));

  // BAR returns.
  callstack_manager.ProcessUretprobes(tid);

  // FUNCTION returns.
  callstack_manager.ProcessUretprobes(tid);
}

}  // namespace LinuxTracing
