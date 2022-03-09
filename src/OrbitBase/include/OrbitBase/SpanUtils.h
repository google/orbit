// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_SPAN_UTILS_H_
#define ORBIT_BASE_SPAN_UTILS_H_

#include <absl/types/span.h>

namespace orbit_base {

// Create spans of specified size from an input std::vector. Note that the last span can be smaller
// than the specified size if the input vector size is not a multiple of the span size.
//
// Example: Splitting data in spans that can be processed in parallel tasks:
//
// void ProcessObjectsInParallel(std::vector<Object>& objects) {
//   TaskGroup task_group;
//   for (absl::Span<Object> span : CreateSpansOfSize(objects, 1024)) {
//     task_group.AddTask([span]() {
//       for (Object& object : span) {
//         Process(object);
//       }
//     });
//   }
// }
//
template <typename T>
[[nodiscard]] inline std::vector<absl::Span<T>> CreateSpansOfSize(std::vector<T>& input_vector,
                                                                  const size_t span_size) {
  if (input_vector.empty() || span_size == 0) return {};
  const size_t num_spans = (input_vector.size() + (span_size - 1)) / span_size;
  std::vector<absl::Span<T>> spans;
  spans.reserve(num_spans);
  absl::Span<T> input_span(input_vector);

  for (size_t i = 0; i < num_spans; ++i) {
    spans.emplace_back(input_span.subspan(i * span_size, span_size));
  }

  return spans;
}

}  // namespace orbit_base

#endif  // ORBIT_BASE_SPAN_UTILS_H_
