// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUFFER_OUTPUT_STREAM_H_
#define BUFFER_OUTPUT_STREAM_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <vector>

namespace orbit_capture_file_internal {

// A ZeroCopyOutputStream implementation backed by a vector of raw bytes.
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream
class BufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream {
 public:
  ~BufferOutputStream() override = default;

  // Obtains a buffer into which data can be written.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyOutputStream.Next.details
  bool Next(void** data, int* size) override;
  // Backs up a number of bytes, so that the end of the last buffer returned by Next() is not
  // actually written.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyOutputStream.BackUp.details
  void BackUp(int count) override;
  // Returns the total number of bytes written since this object was created.
  google::protobuf::int64 ByteCount() const override;

  // Support consuming outputted data by swapping the buffer content.
  void Swap(std::vector<unsigned char>& target);
  [[nodiscard]] const std::vector<unsigned char>& Read() const;

 private:
  static constexpr size_t kMinimumSize = 16;
  mutable absl::Mutex mutex_;
  std::vector<unsigned char> buffer_ ABSL_GUARDED_BY(mutex_);  // The byte vector.
};

}  // namespace orbit_capture_file_internal

#endif  // BUFFER_OUTPUT_STREAM_H_