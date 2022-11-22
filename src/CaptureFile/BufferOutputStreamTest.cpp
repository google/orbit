// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <gtest/gtest.h>

#include <algorithm>
#include <random>
#include <string>
#include <vector>

#include "CaptureFile/BufferOutputStream.h"

namespace orbit_capture_file {

namespace {
std::string GenerateRandomString(int size) {
  std::string random;
  random.resize(size);

  auto randchar = []() -> char {
    const char charset[] =
        "0123456789"
        "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
        "abcdefghijklmnopqrstuvwxyz";
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<int> uniform(0, sizeof(charset) - 1);
    return charset[uniform(gen)];
  };
  std::generate_n(random.data(), size, randchar);

  return random;
}
}  // namespace

TEST(BufferOutputStream, WriteAndRead) {
  BufferOutputStream output_stream;

  auto take_buffer_content = [&]() -> std::string {
    std::vector<unsigned char> buffered_data = output_stream.TakeBuffer();
    std::string buffered_content(buffered_data.begin(), buffered_data.end());
    return buffered_content;
  };

  constexpr int kAdaptorBufferSize = 100;
  constexpr int kBytesToWrite = 234;
  std::string data_to_write = GenerateRandomString(kBytesToWrite);

  {
    google::protobuf::io::CopyingOutputStreamAdaptor adaptor(&output_stream, kAdaptorBufferSize);
    google::protobuf::io::CodedOutputStream coded_output_stream(&adaptor);

    // Write bytes less than kAdaptorBufferSize. Data is not flushed to buffer_output_stream as
    // there is still space in the adaptor's buffer
    constexpr int kBytesLessThanAdaptorBufferSize = 50;
    coded_output_stream.WriteString(data_to_write.substr(0, kBytesLessThanAdaptorBufferSize));
    std::string buffered_content = take_buffer_content();
    EXPECT_TRUE(buffered_content.empty());

    // Write the remaining data. The adaptor flushes data to buffer_output_stream each time when
    // it's buffer has no space to write.
    coded_output_stream.WriteString(data_to_write.substr(kBytesLessThanAdaptorBufferSize));
    int expected_readable_bytes = kBytesToWrite - kBytesToWrite % kAdaptorBufferSize;
    buffered_content = take_buffer_content();
    EXPECT_EQ(buffered_content, data_to_write.substr(0, expected_readable_bytes));
  }

  // When destructing the coded_output_stream and adaptor, all the remaining data is flushed to
  // buffer_output_stream.
  int expected_readable_bytes = kBytesToWrite % kAdaptorBufferSize;
  std::string buffered_content = take_buffer_content();
  EXPECT_EQ(buffered_content, data_to_write.substr(kBytesToWrite - expected_readable_bytes));
}

}  // namespace orbit_capture_file
