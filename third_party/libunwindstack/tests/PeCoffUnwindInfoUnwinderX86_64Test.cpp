/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "PeCoffUnwindInfoUnwinderX86_64.h"

#include <algorithm>
#include <limits>

#include "unwindstack/Error.h"
#include "unwindstack/MachineX86_64.h"
#include "unwindstack/RegsX86_64.h"
#include "utils/MemoryFake.h"

#include "PeCoffEpilog.h"
#include "PeCoffRuntimeFunctions.h"
#include "PeCoffUnwindInfoEvaluator.h"
#include "PeCoffUnwindInfos.h"

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace unwindstack {

class MockPeCoffRuntimeFunctions : public PeCoffRuntimeFunctions {
 public:
  MOCK_METHOD(bool, Init, (uint64_t, uint64_t), (override));
  MOCK_METHOD(bool, FindRuntimeFunction, (uint64_t, RuntimeFunction*), (const, override));
};

class MockPeCoffUnwindInfos : public PeCoffUnwindInfos {
 public:
  MOCK_METHOD(bool, GetUnwindInfo, (uint64_t, UnwindInfo**), (override));
};

class MockPeCoffEpilog : public PeCoffEpilog {
 public:
  MOCK_METHOD(bool, Init, (), (override));
  MOCK_METHOD(bool, DetectAndHandleEpilog, (uint64_t, uint64_t, uint64_t, Memory*, Regs*, bool*),
              (override));

  void FailWithError(ErrorCode error_code) { last_error_.code = error_code; }
};

class MockPeCoffUnwindInfoEvaluator : public PeCoffUnwindInfoEvaluator {
 public:
  MOCK_METHOD(bool, Eval, (Memory*, Regs*, const UnwindInfo&, PeCoffUnwindInfos*, uint64_t),
              (override));

  void FailWithError(ErrorCode error_code) { last_error_.code = error_code; }
};

class TestPeCoffUnwindInfoUnwinderX86_64 : public PeCoffUnwindInfoUnwinderX86_64 {
 public:
  TestPeCoffUnwindInfoUnwinderX86_64() : PeCoffUnwindInfoUnwinderX86_64(nullptr, 0, 0, 0, {}) {}

  void SetFakeRuntimeFuntions(std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions) {
    this->runtime_functions_ = std::move(runtime_functions);
  }

  void SetFakeUnwindInfos(std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos) {
    this->unwind_infos_ = std::move(unwind_infos);
  }

  void SetFakeEpilog(std::unique_ptr<MockPeCoffEpilog> epilog) {
    this->epilog_ = std::move(epilog);
  }

  void SetFakeUnwindInfoEvaluator(
      std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator) {
    this->unwind_info_evaluator_ = std::move(unwind_info_evaluator);
  }
};

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_succeeds_on_leaf_functions) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  // Leaf functions are exactly the functions that don't have RUNTIME_FUNCTION entries.
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction(0, testing::_))
      .WillOnce(::testing::Return(false));
  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));

  RegsX86_64 regs;
  MemoryFake process_memory;
  bool finished = false;
  bool is_signal_frame = false;

  regs.set_sp(0x0);
  process_memory.SetData64(0x0, 0x1000);

  EXPECT_TRUE(test_unwinder.Step(0, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_EQ(regs.sp(), 0x8);
  EXPECT_EQ(regs.pc(), 0x1000);
  EXPECT_FALSE(finished);
  EXPECT_FALSE(is_signal_frame);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_fails_on_leaf_functions_if_memory_invalid) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();

  // Leaf functions are exactly the functions that don't have RUNTIME_FUNCTION entries.
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction(0, testing::_))
      .WillOnce(::testing::Return(false));

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));

  RegsX86_64 regs;
  MemoryFake process_memory;
  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_FALSE(test_unwinder.Step(0, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_EQ(test_unwinder.GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test,
     step_succeeds_when_epilog_detection_is_triggered_and_succeeds) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x20;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x16;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  EXPECT_CALL(*epilog, DetectAndHandleEpilog)
      .WillOnce(testing::DoAll(testing::SetArgPointee<5>(true), testing::Return(true)));

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  EXPECT_CALL(*unwind_info_evaluator, Eval).Times(0);

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;
  MemoryFake process_memory;
  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_TRUE(test_unwinder.Step(kPc, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_TRUE(finished);
  EXPECT_FALSE(is_signal_frame);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_fails_if_error_occurs_in_epilog_detection) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x20;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x16;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  epilog->FailWithError(ERROR_MEMORY_INVALID);
  EXPECT_CALL(*epilog, DetectAndHandleEpilog).WillOnce(testing::Return(false));

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  EXPECT_CALL(*unwind_info_evaluator, Eval).Times(0);

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;
  MemoryFake process_memory;
  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_FALSE(test_unwinder.Step(kPc, 0, &regs, &process_memory, &finished, &is_signal_frame));

  // Make sure the unwinder reports the same error that epilog detection reported.
  EXPECT_EQ(test_unwinder.GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_succeeds_if_eval_succeeds_inside_of_prolog) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x10;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x20;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  EXPECT_CALL(*epilog, DetectAndHandleEpilog).Times(0);

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  EXPECT_CALL(*unwind_info_evaluator, Eval).WillOnce(testing::Return(true));

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;

  // We need to make sure memory can be read when reading the return address, otherwise the step
  // will fail. Since we are mocking everything, the registers are not updated correctly and it
  // doesn't really make sense to test for a specific location of the stack pointer.
  MemoryFakeAlwaysReadZero process_memory;

  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_TRUE(test_unwinder.Step(kPc, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_TRUE(finished);
  EXPECT_FALSE(is_signal_frame);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_succeeds_if_eval_succeeds_outside_of_prolog) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x10;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x8;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  EXPECT_CALL(*epilog, DetectAndHandleEpilog)
      .WillOnce(testing::DoAll(testing::SetArgPointee<5>(false), testing::Return(true)));

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  EXPECT_CALL(*unwind_info_evaluator, Eval).WillOnce(testing::Return(true));

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;

  // We need to make sure memory can be read when reading the return address, otherwise the step
  // will fail. Since we are mocking everything, the registers are not updated correctly and it
  // doesn't really make sense to test for a specific location of the stack pointer.
  MemoryFakeAlwaysReadZero process_memory;

  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_TRUE(test_unwinder.Step(kPc, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_TRUE(finished);
  EXPECT_FALSE(is_signal_frame);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_fails_if_eval_fails_inside_of_prolog) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x10;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x20;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  EXPECT_CALL(*epilog, DetectAndHandleEpilog).Times(0);

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  unwind_info_evaluator->FailWithError(ERROR_MEMORY_INVALID);
  EXPECT_CALL(*unwind_info_evaluator, Eval).WillOnce(testing::Return(false));

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;
  MemoryFake process_memory;
  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_FALSE(test_unwinder.Step(kPc, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_EQ(test_unwinder.GetLastError().code, ERROR_MEMORY_INVALID);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test,
     step_skips_epilog_detection_even_outside_of_prolog_for_non_zero_pc_adjustment) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x10;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x8;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  EXPECT_CALL(*epilog, DetectAndHandleEpilog).Times(0);

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  EXPECT_CALL(*unwind_info_evaluator, Eval).WillOnce(testing::Return(true));

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;

  // We need to make sure memory can be read when reading the return address, otherwise the step
  // will fail. Since we are mocking everything, the registers are not updated correctly and it
  // doesn't really make sense to test for a specific location of the stack pointer.
  MemoryFakeAlwaysReadZero process_memory;

  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_TRUE(test_unwinder.Step(kPc, 1, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_TRUE(finished);
  EXPECT_FALSE(is_signal_frame);
}

TEST(PeCoffUnwindInfoUnwinderX86_64Test, step_fails_after_eval_if_return_address_location_invalid) {
  TestPeCoffUnwindInfoUnwinderX86_64 test_unwinder;

  constexpr uint64_t kFunctionStartAddress = 0x100;
  constexpr uint64_t kFunctionEndAddress = 0x200;
  constexpr uint64_t kPc = kFunctionStartAddress + 0x10;

  std::unique_ptr<MockPeCoffRuntimeFunctions> runtime_functions =
      std::make_unique<MockPeCoffRuntimeFunctions>();
  EXPECT_CALL(*runtime_functions, FindRuntimeFunction)
      .WillOnce([](uint64_t, RuntimeFunction* runtime_function) {
        RuntimeFunction function{kFunctionStartAddress, kFunctionEndAddress, 0};
        *runtime_function = function;
        return true;
      });

  std::unique_ptr<MockPeCoffUnwindInfos> unwind_infos = std::make_unique<MockPeCoffUnwindInfos>();
  UnwindInfo unwind_info_to_return;
  unwind_info_to_return.prolog_size = 0x20;
  EXPECT_CALL(*unwind_infos, GetUnwindInfo)
      .WillOnce([&unwind_info_to_return](uint64_t, UnwindInfo** unwind_info) {
        *unwind_info = &unwind_info_to_return;
        return true;
      });

  std::unique_ptr<MockPeCoffEpilog> epilog = std::make_unique<MockPeCoffEpilog>();
  EXPECT_CALL(*epilog, DetectAndHandleEpilog).Times(0);

  std::unique_ptr<MockPeCoffUnwindInfoEvaluator> unwind_info_evaluator =
      std::make_unique<MockPeCoffUnwindInfoEvaluator>();
  EXPECT_CALL(*unwind_info_evaluator, Eval).WillOnce(testing::Return(true));

  test_unwinder.SetFakeRuntimeFuntions(std::move(runtime_functions));
  test_unwinder.SetFakeUnwindInfos(std::move(unwind_infos));
  test_unwinder.SetFakeEpilog(std::move(epilog));
  test_unwinder.SetFakeUnwindInfoEvaluator(std::move(unwind_info_evaluator));

  RegsX86_64 regs;
  MemoryFake process_memory;

  bool finished = false;
  bool is_signal_frame = false;

  EXPECT_FALSE(test_unwinder.Step(kPc, 0, &regs, &process_memory, &finished, &is_signal_frame));
  EXPECT_EQ(test_unwinder.GetLastError().code, ERROR_MEMORY_INVALID);
}

}  // namespace unwindstack
