// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_CHUNK_H_
#define ORBIT_BASE_CHUNK_H_

#include <absl/types/span.h>
#include <stddef.h>

namespace orbit_base {

// Chunk an input vector into spans of specified "chunk_size". Note that the last span can be
// smaller than "chunk_size" if the input vector size is not a multiple of "chunk_size".
//
// Example: Splitting data in chunks to be processed in parallel tasks:
//
// void ProcessObjectsInParallel(std::vector<Object>& objects) {
//   TaskGroup task_group(executor_);
//   for (absl::Span<Object> chunk : CreateChunksOfSize(objects, 1024)) {
//     task_group.AddTask([chunk]() {
//       for (Object& object : chunk) {
//         Process(object);
//       }
//     });
//   }
// }
//
template <typename T>
[[nodiscard]] std::vector<absl::Span<T>> CreateChunksOfSize(std::vector<T>& input_vector,
                                                            const size_t chunk_size) {
  if (input_vector.empty() || chunk_size == 0) return {};
  const size_t num_chunks = (input_vector.size() + (chunk_size - 1)) / chunk_size;
  std::vector<absl::Span<T>> chunks;
  chunks.reserve(num_chunks);
  absl::Span<T> input_span(input_vector);

  for (size_t i = 0; i < num_chunks; ++i) {
    chunks.emplace_back(input_span.subspan(i * chunk_size, chunk_size));
  }

  return chunks;
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_CHUNK_H_
