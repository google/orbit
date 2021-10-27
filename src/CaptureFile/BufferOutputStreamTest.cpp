// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <google/protobuf/io/coded_stream.h>
#include <gtest/gtest.h>

#include <random>

#include "BufferOutputStream.h"

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
    std::mt19937 gen(123);
    std::uniform_int_distribution<int> uniform(0, sizeof(charset) - 1);
    return charset[uniform(gen)];
  };
  std::generate_n(random.data(), size, randchar);

  return random;
}
}  // namespace

TEST(BufferOutputStream, WriteAndRead) {
  BufferOutputStream output_stream;
  EXPECT_EQ(output_stream.ByteCount(), 0);

  // Test write data and trim unused bytes.
  constexpr int kBytesToWrite = 1000;
  std::string data_to_write = GenerateRandomString(kBytesToWrite);
  google::protobuf::io::CodedOutputStream coded_output_stream(&output_stream);
  coded_output_stream.WriteString(data_to_write);
  EXPECT_GE(output_stream.ByteCount(), kBytesToWrite);
  coded_output_stream.Trim();
  EXPECT_EQ(output_stream.ByteCount(), kBytesToWrite);

  // Test read data. The bytes trying to read is less than bytes available to read.
  std::string data_read;
  constexpr int kBytesToRead = 123;
  data_read.resize(kBytesToRead);
  size_t actural_bytes_read = output_stream.ReadIntoBuffer(data_read.data(), kBytesToRead);
  EXPECT_EQ(actural_bytes_read, kBytesToRead);
  EXPECT_EQ(data_read, data_to_write.substr(0, kBytesToRead));
  EXPECT_EQ(output_stream.ByteCount(), kBytesToWrite - kBytesToRead);

  // Test read data. The bytes trying to read is more than bytes available to read.
  constexpr int kBytesMoreThanReadAvailable = 900;
  data_read.resize(kBytesMoreThanReadAvailable);
  actural_bytes_read = output_stream.ReadIntoBuffer(data_read.data(), kBytesMoreThanReadAvailable);
  EXPECT_EQ(actural_bytes_read, kBytesToWrite - kBytesToRead);
  EXPECT_EQ(data_read.substr(0, actural_bytes_read),
            data_to_write.substr(kBytesToRead, kBytesToWrite - kBytesToRead));
  EXPECT_EQ(output_stream.ByteCount(), 0);
}

}  // namespace orbit_capture_file
