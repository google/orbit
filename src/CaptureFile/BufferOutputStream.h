// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef BUFFER_OUTPUT_STREAM_H_
#define BUFFER_OUTPUT_STREAM_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <vector>

namespace orbit_capture_file {

// A CopyingOutputStream implementation backed by a vector of raw bytes. It supports reading data
// from the buffer of this stream into a provided buffer.
//
// Example Usage:
//   * Create a BufferOutputStream and write data into it. Notice that the unused bytes in the
//   adaptor should be trimmed before flushing data from adaptor's buffer to the output stream
//      BufferOutputStream buffer_output_stream;
//      CopyingOutputStreamAdaptor adaptor(&buffer_output_stream)
//      CodedOutputStream coded_output_stream(&adaptor);
//      coded_output_stream.WriteString(data_to_write);
//      coded_output_stream.Trim();
//      adaptor.Flush();
//
//   * Read buffered data into provided buffer.
//      size_t bytes_read = buffer_output_stream.ReadIntoBuffer(dest, max_size);
//
// Mote details about ZeroCopyOutputStream:
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream
class BufferOutputStream : public google::protobuf::io::CopyingOutputStream {
 public:
  // Write `size` bytes from the provided buffer to the output stream's buffer.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream_impl_lite#CopyingOutputStream.Write.details
  bool Write(const void* buffer, int size) override;

  // Reads at most `max_size` bytes data into the buffer pointed to by `dest` and returns the number
  // of successful read bytes. Note that the read bytes are erased from `buffer_`.
  [[nodiscard]] size_t ReadIntoBuffer(void* dest, size_t max_size);

  [[nodiscard]] google::protobuf::int64 BytesAvailableToRead() const;

 private:
  mutable absl::Mutex mutex_;
  std::vector<unsigned char> buffer_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_capture_file

#endif  // BUFFER_OUTPUT_STREAM_H_