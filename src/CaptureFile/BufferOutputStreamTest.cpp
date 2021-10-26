// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <google/protobuf/io/coded_stream.h>
#include <gtest/gtest.h>

#include "BufferOutputStream.h"

namespace orbit_capture_file_internal {

namespace {
std::string GenerateRandomString(int size) {
  std::string random;
  random.resize(size);

  auto randchar = []() -> char {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[std::rand() % max_index];
  };
  std::generate_n(random.data(), size, randchar);

  return random;
}
}  // namespace

TEST(BufferOutputStream, WriteToOutputStream) {
  BufferOutputStream output_stream;
  EXPECT_EQ(output_stream.ByteCount(), 0);

  constexpr int kBytesToWrite = 1024;
  std::string data_to_write = GenerateRandomString(kBytesToWrite);
  google::protobuf::io::CodedOutputStream coded_output_stream(&output_stream);
  coded_output_stream.WriteString(data_to_write);
  EXPECT_EQ(output_stream.ByteCount(), kBytesToWrite);

  const std::vector<unsigned char>& output_buffer = output_stream.Read();
  std::string output_stream_content(output_buffer.begin(), output_buffer.end());
  EXPECT_EQ(output_stream_content, data_to_write);

  std::vector<unsigned char> new_buffer;
  output_stream.Swap(new_buffer);
  EXPECT_EQ(output_stream.ByteCount(), 0);
}

}  // namespace orbit_capture_file_internal
