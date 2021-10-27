// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUFFER_OUTPUT_STREAM_H_
#define BUFFER_OUTPUT_STREAM_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <google/protobuf/io/zero_copy_stream.h>

#include <vector>

namespace orbit_capture_file {

// A ZeroCopyOutputStream implementation backed by a vector of raw bytes. It supports reading data
// from the buffer of this stream into a provided buffer.
//
// Note: The unused bytes should be trimmed with `BackUp` before reading data with `ReadIntoBuffer`.
//
// Example Usage:
//   * Create a BufferOutputStream and write data into it
//   BufferOutputStream buffer_output_stream;
//   CodedOutputStream coded_output_stream(&buffer_output_stream);
//   coded_output_stream.WriteString(data_to_write);
//
//   * Read buffered data into provided buffer
//   coded_output_stream.Trim();
//   size_t bytes_read = buffer_output_stream.ReadIntoBuffer(dest, max_size);
//
// Mote details about ZeroCopyOutputStream:
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream
class BufferOutputStream : public google::protobuf::io::ZeroCopyOutputStream {
 public:
  // Obtains a buffer into which data can be written.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyOutputStream.Next.details
  bool Next(void** data, int* size) override;

  // Backs up a number of bytes, so that the end of the last buffer returned by Next() is not
  // actually written.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyOutputStream.BackUp.details
  void BackUp(int count) override;

  // Returns the total number of bytes written since this object was created.
  google::protobuf::int64 ByteCount() const override;

  // Reads at most `max_size` bytes data into the buffer pointed to by `dest` and returns the number
  // of successful read bytes. Note that the read bytes are erased from `buffer_`.
  [[nodiscard]] size_t ReadIntoBuffer(void* dest, size_t max_size);

 private:
  static constexpr size_t kMinimumSize = 16;
  mutable absl::Mutex mutex_;
  std::vector<unsigned char> buffer_ ABSL_GUARDED_BY(mutex_);  // The byte vector.
  size_t bytes_available_to_read_ ABSL_GUARDED_BY(mutex_) = 0;
};

}  // namespace orbit_capture_file

#endif  // BUFFER_OUTPUT_STREAM_H_