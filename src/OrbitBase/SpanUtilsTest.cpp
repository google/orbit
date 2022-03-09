// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include "OrbitBase/SpanUtils.h"
#include "OrbitBase/TaskGroup.h"
#include "OrbitBase/ThreadPool.h"

using orbit_base::CreateSpansOfSize;
using orbit_base::TaskGroup;
using orbit_base::ThreadPool;

namespace {

// Tests that all elements of input array are accounted for by the spans and that the spans don't
// overflow.
template <typename T>
void TestSpansCoverage(const std::vector<T>& test_vector, const std::vector<absl::Span<T>>& spans) {
  const T* element = test_vector.data();
  for (const absl::Span<T>& span : spans) {
    EXPECT_EQ(element, span.data());
    element = span.data() + span.size();
  }

  if (!spans.empty()) {
    EXPECT_EQ(test_vector.data() + test_vector.size(), element);
  }
}

}  // namespace

TEST(SpanUtils, SpansCoverage) {
  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);
  for (size_t i = 0; i < 32; ++i) {
    TestSpansCoverage(counters, CreateSpansOfSize(counters, i));
  }
}

TEST(SpanUtils, EmptyVector) {
  std::vector<uint32_t> empty_vector;
  std::vector<absl::Span<uint32_t>> spans = CreateSpansOfSize(empty_vector, 1);
  EXPECT_EQ(spans.size(), 0);
}

TEST(SpanUtils, ZeroSpanSize) {
  std::vector<uint32_t> test_vector(10);
  std::vector<absl::Span<uint32_t>> spans = CreateSpansOfSize(test_vector, 0);
  EXPECT_EQ(spans.size(), 0);
}

TEST(SpanUtils, ExactMultiple) {
  std::vector<uint32_t> test_vector(1000);
  std::vector<absl::Span<uint32_t>> spans = CreateSpansOfSize(test_vector, 10);
  EXPECT_EQ(spans.size(), 100);
  EXPECT_EQ(spans.back().size(), 10);
}

TEST(SpanUtils, Remainder) {
  {
    std::vector<uint32_t> test_vector(1001);
    std::vector<absl::Span<uint32_t>> spans = CreateSpansOfSize(test_vector, 10);
    EXPECT_EQ(spans.size(), 101);
    EXPECT_EQ(spans.back().size(), 1);
  }

  {
    std::vector<uint32_t> test_vector(1234);
    std::vector<absl::Span<uint32_t>> spans = CreateSpansOfSize(test_vector, 10);
    EXPECT_EQ(spans.size(), 124);
    EXPECT_EQ(spans.back().size(), 4);
  }
}

TEST(SpanUtils, SpanSizeBiggerThanVectorSize) {
  std::vector<uint32_t> test_vector(1);
  std::vector<absl::Span<uint32_t>> spans = CreateSpansOfSize(test_vector, 10);
  EXPECT_EQ(spans.size(), 1);
  EXPECT_EQ(spans.back().size(), 1);
}

TEST(SpanUtils, TaskGroupTestCase) {
  constexpr size_t kThreadPoolMinSize = 2;
  constexpr size_t kThreadPoolMaxSize = 2;
  constexpr absl::Duration kThreadTtl = absl::Milliseconds(5);
  std::shared_ptr<ThreadPool> thread_pool =
      ThreadPool::Create(kThreadPoolMinSize, kThreadPoolMaxSize, kThreadTtl);

  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);

  TaskGroup task_group(thread_pool.get());
  for (absl::Span<uint32_t> span : CreateSpansOfSize(counters, 10)) {
    task_group.AddTask([span]() {
      for (uint32_t& counter : span) ++counter;
    });
  }
  task_group.Wait();

  for (uint32_t counter : counters) {
    EXPECT_EQ(counter, 1);
  }

  thread_pool->ShutdownAndWait();
}
