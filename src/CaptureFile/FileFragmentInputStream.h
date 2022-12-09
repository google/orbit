// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FILE_FRAGMENT_INPUT_STREAM_H_
#define FILE_FRAGMENT_INPUT_STREAM_H_

#include <google/protobuf/io/zero_copy_stream.h>
#include <stddef.h>
#include <stdint.h>

#include <algorithm>
#include <cstdint>
#include <optional>
#include <vector>

#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_file_internal {

// ZeroCopyInputStream implementation for a file fragment with offset and size.
// This class is used to read protos from capture file sections and makes sure
// we do not overread into other sections of the file.
// https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream
class FileFragmentInputStream : public google::protobuf::io::ZeroCopyInputStream {
 public:
  explicit FileFragmentInputStream(const orbit_base::unique_fd& fd, uint64_t file_offset,
                                   uint64_t size, size_t block_size = 1 << 16)
      : fd_{fd},
        file_fragments_start_{file_offset},
        file_fragments_end_{file_offset + size},
        buffer_(block_size),
        current_position_{file_offset} {
    ORBIT_CHECK(size > 0);
  }

  // Obtains a chunk of data from the stream.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyInputStream.Next.details
  bool Next(const void** data, int* size) override;
  // Backs up a number of bytes, so that the next call to Next() returns data again that was already
  // returned by the last call to Next().
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyInputStream.BackUp.details
  void BackUp(int count) override;
  // Skips a number of bytes.
  // https://developers.google.com/protocol-buffers/docs/reference/cpp/google.protobuf.io.zero_copy_stream#ZeroCopyInputStream.Skip.details
  bool Skip(int count) override;
  [[nodiscard]] int64_t ByteCount() const override;

  [[nodiscard]] std::optional<ErrorMessage> GetLastError() const { return last_error_; }

 private:
  const orbit_base::unique_fd& fd_;
  const uint64_t file_fragments_start_;
  const uint64_t file_fragments_end_;
  std::vector<uint8_t> buffer_;
  uint64_t current_position_;
  std::optional<ErrorMessage> last_error_{};
};

}  // namespace orbit_capture_file_internal

#endif  // FILE_FRAGMENT_INPUT_STREAM_H_
