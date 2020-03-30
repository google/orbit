#include <gtest/gtest.h>

#include "UprobesReturnAddressManager.h"

namespace LinuxTracing {

namespace {

class TestStack {
 public:
  explicit TestStack(uint64_t sp) : sp_{sp} {}

  TestStack(const TestStack&) = default;
  TestStack& operator=(const TestStack&) = default;
  TestStack(TestStack&&) = default;
  TestStack& operator=(TestStack&&) = default;

  bool operator==(const TestStack& o) const {
    if (sp_ != o.sp_) {
      return false;
    }
    if (data_.size() != o.data_.size()) {
      return false;
    }
    for (size_t i = 0; i < data_.size(); ++i) {
      if (data_[i] != o.data_[i]) {
        return false;
      }
    }
    return true;
  }

  bool operator!=(const TestStack& o) const { return !(*this == o); }

  void Push(uint64_t value) {
    sp_ -= sizeof(uint64_t);
    data_.insert(data_.begin(), value);
  }

  void Pop() {
    sp_ += sizeof(uint64_t);
    data_.erase(data_.begin());
  }

  void HijackTop(uint64_t new_value) { data_[0] = new_value; }

  uint64_t GetSp() const { return sp_; }

  uint64_t GetTop() const { return data_[0]; }

  char* GetData() { return reinterpret_cast<char*>(data_.data()); }

  const char* GetData() const {
    return reinterpret_cast<const char*>(data_.data());
  }

  uint64_t GetSize() const { return data_.size() * sizeof(uint64_t); }

 private:
  uint64_t sp_;
  std::vector<uint64_t> data_{};
};

class TestHandler {
 public:
  explicit TestHandler(pid_t tid)
      : tid_{tid}, expected_stack_{256}, hijacked_stack_{expected_stack_} {}

  void OnNonUprobesCall() {
    // Fake pushing the return address.
    expected_stack_.Push(next_push_);
    hijacked_stack_.Push(next_push_);
    ++next_push_;

    // Fake pushing other data.
    expected_stack_.Push(next_push_);
    hijacked_stack_.Push(next_push_);
    ++next_push_;
  }

  void OnNonUretprobesReturn() {
    // Fake popping other data.
    expected_stack_.Pop();
    hijacked_stack_.Pop();

    // Fake popping the return address.
    expected_stack_.Pop();
    hijacked_stack_.Pop();
  }

  void OnUprobesCall(UprobesReturnAddressManager* return_address_manager) {
    // Fake pushing the return address.
    expected_stack_.Push(next_push_);
    hijacked_stack_.Push(next_push_);
    ++next_push_;

    return_address_manager->ProcessUprobes(tid_, hijacked_stack_.GetSp(),
                                           hijacked_stack_.GetTop());

    // Fake uretprobes hijacking the return address.
    hijacked_stack_.HijackTop(next_hijack_);
    ++next_hijack_;

    // Fake pushing other data.
    expected_stack_.Push(next_push_);
    hijacked_stack_.Push(next_push_);
    ++next_push_;
  }

  void OnUretprobesReturn(UprobesReturnAddressManager* return_address_manager) {
    // Fake popping other data.
    expected_stack_.Pop();
    hijacked_stack_.Pop();

    // Fake popping the return address.
    expected_stack_.Pop();
    hijacked_stack_.Pop();

    return_address_manager->ProcessUretprobes(tid_);
  }

  void OnUprobesOptimizedTailCall(
      UprobesReturnAddressManager* return_address_manager) {
    // Fake popping other data to clear the frame for the tail call.
    expected_stack_.Pop();
    hijacked_stack_.Pop();

    // Do not fake pushing the return address as this is an optimized tail call.

    return_address_manager->ProcessUprobes(tid_, hijacked_stack_.GetSp(),
                                           hijacked_stack_.GetTop());

    // Fake uretprobes hijacking the return address.
    hijacked_stack_.HijackTop(next_hijack_);
    ++next_hijack_;

    // Fake pushing other data.
    expected_stack_.Push(next_push_);
    hijacked_stack_.Push(next_push_);
    ++next_push_;
  }

  void OnUretprobesAfterTailCallReturn(
      UprobesReturnAddressManager* return_address_manager) {
    // Do not fake popping other data as this function had ended with a tail
    // call, its frame was clear.

    // Do not fake popping the return address as this function had ended with a
    // tail call, only the uretprobe is hit.

    return_address_manager->ProcessUretprobes(tid_);
  }

  TestStack PatchStackOnSample(
      UprobesReturnAddressManager* return_address_manager) {
    TestStack patched_stack{hijacked_stack_};
    return_address_manager->PatchSample(tid_, patched_stack.GetSp(),
                                        patched_stack.GetData(),
                                        patched_stack.GetSize());
    return patched_stack;
  }

  pid_t GetTid() const { return tid_; }

  const TestStack& GetExpectedStack() const { return expected_stack_; }

  const TestStack& GetHijackedStack() const { return hijacked_stack_; }

 private:
  pid_t tid_;
  TestStack expected_stack_;
  TestStack hijacked_stack_;

  uint64_t next_push_ = 42;
  uint64_t next_hijack_ = 1000;
};

}  // namespace

TEST(UprobesReturnAddressManager, NoUprobes) {
  UprobesReturnAddressManager return_address_manager;
  TestHandler test_handler{42};

  // Fake sample.
  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // Fake call to function A.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // B is called.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // Fake return of function B.
  test_handler.OnNonUretprobesReturn();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // A returns.
  test_handler.OnNonUretprobesReturn();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());
}

TEST(UprobesReturnAddressManager, OneUprobe) {
  UprobesReturnAddressManager return_address_manager;
  TestHandler test_handler{42};

  // A is called.
  test_handler.OnNonUprobesCall();

  // B is called and hits a uprobe.
  test_handler.OnUprobesCall(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // C is called.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // C returns.
  test_handler.OnNonUretprobesReturn();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // B returns and hits a uretprobe.
  test_handler.OnUretprobesReturn(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // A returns.
  test_handler.OnNonUretprobesReturn();
}

TEST(UprobesReturnAddressManager, DifferentThread) {
  UprobesReturnAddressManager return_address_manager;
  TestHandler test_handler{42};
  TestHandler other_test_handler{111};

  // A is called.
  test_handler.OnNonUprobesCall();

  // B is called and hits a uprobe.
  test_handler.OnUprobesCall(&return_address_manager);

  // C is called on the other thread.
  other_test_handler.OnNonUprobesCall();

  // Sample on the other thread.
  EXPECT_EQ(other_test_handler.PatchStackOnSample(&return_address_manager),
            other_test_handler.GetExpectedStack());

  // B returns and hits a uretprobe.
  test_handler.OnUretprobesReturn(&return_address_manager);

  // Sample on the other thread.
  EXPECT_EQ(other_test_handler.PatchStackOnSample(&return_address_manager),
            other_test_handler.GetExpectedStack());

  // C returns (on the other thread).
  other_test_handler.OnNonUprobesCall();

  // A returns.
  test_handler.OnNonUretprobesReturn();
}

TEST(UprobesReturnAddressManager, TwoNestedUprobesAndAnotherUprobe) {
  UprobesReturnAddressManager return_address_manager;
  TestHandler test_handler{42};

  // A is called.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // B is called and hits a uprobe.
  test_handler.OnUprobesCall(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // C is called and hits a uprobe.
  test_handler.OnUprobesCall(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // D is called.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // D returns.
  test_handler.OnNonUretprobesReturn();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // C returns and hits a uretprobe.
  test_handler.OnUretprobesReturn(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // B returns and hits a uretprobe.
  test_handler.OnUretprobesReturn(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // E is called and hits a uprobe.
  test_handler.OnUprobesCall(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // E returns and hits a uretprobe.
  test_handler.OnUretprobesReturn(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // A returns.
  test_handler.OnNonUretprobesReturn();
}

TEST(UprobesReturnAddressManager, TailCallOptimization) {
  UprobesReturnAddressManager return_address_manager;
  TestHandler test_handler{42};

  // A is called.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // B is called and hits a uprobe.
  test_handler.OnUprobesCall(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // C is called with tail-call optimization and hits a uprobe.
  test_handler.OnUprobesOptimizedTailCall(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // D is called.
  test_handler.OnNonUprobesCall();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // D returns.
  test_handler.OnNonUretprobesReturn();

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // C returns and hits a uretprobe.
  test_handler.OnUretprobesReturn(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // B is not on the stack anymore because it had ended with a tail-call, but
  // its uretprobe is still hit.
  test_handler.OnUretprobesAfterTailCallReturn(&return_address_manager);

  EXPECT_EQ(test_handler.PatchStackOnSample(&return_address_manager),
            test_handler.GetExpectedStack());

  // A returns.
  test_handler.OnNonUretprobesReturn();
}

}  // namespace LinuxTracing
