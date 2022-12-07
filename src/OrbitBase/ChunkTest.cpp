// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/types/span.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <algorithm>
#include <cstdint>
#include <vector>

#include "OrbitBase/Chunk.h"
#include "OrbitBase/TaskGroup.h"

using orbit_base::CreateChunksOfSize;

namespace {

// Tests that all elements of input array are accounted for by the spans and that the spans don't
// overflow.
template <typename T>
void TestChunksCoverage(absl::Span<const T> test_vector, absl::Span<const absl::Span<T>> chunks) {
  const T* element = test_vector.data();
  for (const absl::Span<T>& chunk : chunks) {
    EXPECT_EQ(element, chunk.data());
    element = chunk.data() + chunk.size();
  }

  if (!chunks.empty()) {
    EXPECT_EQ(test_vector.data() + test_vector.size(), element);
  }
}

}  // namespace

TEST(SpanUtils, SpansCoverage) {
  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);
  for (size_t i = 0; i < 32; ++i) {
    TestChunksCoverage<uint32_t>(counters, CreateChunksOfSize(counters, i));
  }
}

TEST(SpanUtils, EmptyVector) {
  std::vector<uint32_t> empty_vector;
  std::vector<absl::Span<uint32_t>> chunks = CreateChunksOfSize(empty_vector, 1);
  EXPECT_EQ(chunks.size(), 0);
}

TEST(SpanUtils, ZeroSpanSize) {
  std::vector<uint32_t> test_vector(10);
  std::vector<absl::Span<uint32_t>> chunks = CreateChunksOfSize(test_vector, 0);
  EXPECT_EQ(chunks.size(), 0);
}

TEST(SpanUtils, ExactMultiple) {
  std::vector<uint32_t> test_vector(1000);
  std::vector<absl::Span<uint32_t>> chunks = CreateChunksOfSize(test_vector, 10);
  EXPECT_EQ(chunks.size(), 100);
  EXPECT_EQ(chunks.back().size(), 10);
}

TEST(SpanUtils, Remainder) {
  {
    std::vector<uint32_t> test_vector(1001);
    std::vector<absl::Span<uint32_t>> chunks = CreateChunksOfSize(test_vector, 10);
    EXPECT_EQ(chunks.size(), 101);
    EXPECT_EQ(chunks.back().size(), 1);
  }

  {
    std::vector<uint32_t> test_vector(1234);
    std::vector<absl::Span<uint32_t>> chunks = CreateChunksOfSize(test_vector, 10);
    EXPECT_EQ(chunks.size(), 124);
    EXPECT_EQ(chunks.back().size(), 4);
  }
}

TEST(SpanUtils, SpanSizeBiggerThanVectorSize) {
  std::vector<uint32_t> test_vector(1);
  std::vector<absl::Span<uint32_t>> chunks = CreateChunksOfSize(test_vector, 10);
  EXPECT_EQ(chunks.size(), 1);
  EXPECT_EQ(chunks.back().size(), 1);
}

TEST(SpanUtils, TaskGroupTestCase) {
  constexpr size_t kNumElements = 1024;
  std::vector<uint32_t> counters(kNumElements);

  orbit_base::TaskGroup task_group;
  for (absl::Span<uint32_t> chunk : CreateChunksOfSize(counters, 10)) {
    task_group.AddTask([chunk]() {
      for (uint32_t& counter : chunk) ++counter;
    });
  }
  task_group.Wait();

  for (uint32_t counter : counters) {
    EXPECT_EQ(counter, 1);
  }
}
