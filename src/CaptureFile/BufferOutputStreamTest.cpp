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
  google::protobuf::io::CopyingOutputStreamAdaptor adaptor(&output_stream);
  EXPECT_EQ(adaptor.ByteCount(), 0);

  // CopyingOutputStreamAdaptor::Next is called when constructing the coded_output_stream. To get
  // the actural written bytes of the adaptor, we need to trim the unused bytes.
  google::protobuf::io::CodedOutputStream coded_output_stream(&adaptor);
  EXPECT_GT(adaptor.ByteCount(), 0);
  coded_output_stream.Trim();
  EXPECT_EQ(adaptor.ByteCount(), 0);

  // Test write data to adaptor and then flush data to output stream. Note that we need to trim
  // unused bytes in the adaptor before flushing data to the output stream.
  constexpr int kBytesToWrite = 10000;
  std::string data_to_write = GenerateRandomString(kBytesToWrite);
  coded_output_stream.WriteString(data_to_write);
  EXPECT_GT(adaptor.ByteCount(), kBytesToWrite);
  coded_output_stream.Trim();
  EXPECT_TRUE(adaptor.Flush());
  EXPECT_EQ(adaptor.ByteCount(), kBytesToWrite);

  // Test take buffered data away from the output stream.
  std::vector<unsigned char> buffered_data = output_stream.TakeBuffer();
  std::string buffered_content(buffered_data.begin(), buffered_data.end());
  EXPECT_EQ(buffered_content, data_to_write);
}

}  // namespace orbit_capture_file
