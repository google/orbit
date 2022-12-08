// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_BUFFER_OUTPUT_STREAM_H_
#define CAPTURE_FILE_BUFFER_OUTPUT_STREAM_H_

#include <absl/base/thread_annotations.h>
#include <absl/synchronization/mutex.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>

#include <vector>

namespace orbit_capture_file {

// A CopyingOutputStream implementation backed by a vector of raw bytes. It supports reading data
// from the buffer of this stream into a provided buffer.
//
// Note: This output stream will be used together with a CopyingOutputStreamAdaptor. The adaptor
// will flush data to this output stream when:
//   * The adaptor uses up all space in its own buffer. The adaptor will flush all data in its
//   buffer to the output stream;
//   * We force the adaptor to flush data to output stream with CopyingOutputStreamAdaptor::Flush.
//   There might be unused bytes in the adaptor's buffer, remember trim them before calling Flush.
//   * The adaptor is destructed.
//
// Example Usage:
//   * Create a BufferOutputStream and write data into it.
//      BufferOutputStream buffer_output_stream;
//      CopyingOutputStreamAdaptor adaptor(&buffer_output_stream)
//      CodedOutputStream coded_output_stream(&adaptor);
//      coded_output_stream.WriteString(data_to_write);
//      coded_output_stream.Trim(); /* Flush data to the adaptor and trim unused bytes */
//      adaptor.Flush(); /* Flush data to the buffer_output_stream */
//
//   * Take buffered data away from output stream.
//      std::vector<unsigned char> buffered_data = buffer_output_stream.TakeBuffer();
//
// Mote details about ZeroCopyOutputStream:
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream
class BufferOutputStream : public google::protobuf::io::CopyingOutputStream {
 public:
  // Write `size` bytes from the provided buffer to the output stream's buffer.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream_impl_lite#CopyingOutputStream.Write.details
  bool Write(const void* data, int size) override;

  // Take buffered data away from the output stream.
  [[nodiscard]] std::vector<unsigned char> TakeBuffer();

 private:
  mutable absl::Mutex mutex_;
  std::vector<unsigned char> buffer_ ABSL_GUARDED_BY(mutex_);
};

}  // namespace orbit_capture_file

#endif  // CAPTURE_FILE_BUFFER_OUTPUT_STREAM_H_