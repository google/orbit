// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>
#include <stddef.h>

#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "FileFragmentInputStream.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"

namespace orbit_capture_file_internal {

TEST(FileFragmentInputStream, ReadBlocksOfTen) {
  auto temporary_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_test_utils::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  auto write_result =
      orbit_base::WriteFully(temporary_file.fd(),
                             "Vestibulum euismod sapien eget urna molestie euismod. Etiam "
                             "pellentesque porttitor ligula et facilisis.");
  ASSERT_FALSE(write_result.has_error()) << write_result.error().message();

  // The fragment is "urna molestie euismod. Etiam pellentesque"
  FileFragmentInputStream input_stream{temporary_file.fd(), 31, 41, 10};
  EXPECT_EQ(input_stream.ByteCount(), 0);

  const void* bytes = nullptr;
  int size = 0;
  ASSERT_TRUE(input_stream.Next(&bytes, &size));
  EXPECT_EQ((std::string_view{static_cast<const char*>(bytes), static_cast<size_t>(size)}),
            "urna moles");
  EXPECT_EQ(input_stream.ByteCount(), 10);
  ASSERT_TRUE(input_stream.Skip(5));
  EXPECT_EQ(input_stream.ByteCount(), 15);
  ASSERT_TRUE(input_stream.Next(&bytes, &size));

  EXPECT_EQ((std::string_view{static_cast<const char*>(bytes), static_cast<size_t>(size)}),
            "uismod. Et");
  // Make sure we do not go out of fragment boundary
  input_stream.BackUp(100);
  EXPECT_EQ(input_stream.ByteCount(), 0);

  ASSERT_TRUE(input_stream.Next(&bytes, &size));
  EXPECT_EQ((std::string_view{static_cast<const char*>(bytes), static_cast<size_t>(size)}),
            "urna moles");
  EXPECT_EQ(input_stream.ByteCount(), 10);

  // Do not read past end of the fragment
  ASSERT_TRUE(input_stream.Skip(30));
  EXPECT_EQ(input_stream.ByteCount(), 40);

  ASSERT_TRUE(input_stream.Next(&bytes, &size));
  EXPECT_EQ((std::string_view{static_cast<const char*>(bytes), static_cast<size_t>(size)}), "e");
  EXPECT_EQ(input_stream.ByteCount(), 41);

  EXPECT_FALSE(input_stream.Next(&bytes, &size));

  EXPECT_FALSE(input_stream.GetLastError().has_value());

  // Let's read last 5 bytes again
  input_stream.BackUp(5);

  ASSERT_TRUE(input_stream.Next(&bytes, &size));
  EXPECT_EQ((std::string_view{static_cast<const char*>(bytes), static_cast<size_t>(size)}),
            "esque");
  EXPECT_EQ(input_stream.ByteCount(), 41);

  EXPECT_FALSE(input_stream.Next(&bytes, &size));

  EXPECT_FALSE(input_stream.GetLastError().has_value());
}

}  // namespace orbit_capture_file_internal
