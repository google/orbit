// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <gtest/gtest.h>
#include <string.h>

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>
#include <vector>

#include "CaptureFile/BufferOutputStream.h"
#include "CaptureFile/CaptureFileOutputStream.h"
#include "CaptureFileConstants.h"
#include "GrpcProtos/capture.pb.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/Result.h"
#include "TestUtils/TemporaryFile.h"

namespace orbit_capture_file {

static constexpr const char* kAnswerString =
    "Answer to the Ultimate Question of Life, The Universe, and Everything";
static constexpr const char* kNotAnAnswerString = "Some odd number, not the answer.";
static constexpr uint64_t kAnswerKey = 42;
static constexpr uint64_t kNotAnAnswerKey = 43;

static orbit_grpc_protos::ClientCaptureEvent CreateInternedStringCaptureEvent(uint64_t key,
                                                                              std::string str) {
  orbit_grpc_protos::ClientCaptureEvent event;
  orbit_grpc_protos::InternedString* interned_string = event.mutable_interned_string();
  interned_string->set_key(key);
  interned_string->set_intern(std::move(str));
  return event;
}

TEST(CaptureFileOutputStream, Smoke) {
  orbit_grpc_protos::ClientCaptureEvent event1 =
      CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString);
  orbit_grpc_protos::ClientCaptureEvent event2 =
      CreateInternedStringCaptureEvent(kNotAnAnswerKey, kNotAnAnswerString);

  auto write_events_then_close = [&](CaptureFileOutputStream* output_stream) {
    auto write_result = output_stream->WriteCaptureEvent(event1);
    EXPECT_FALSE(write_result.has_error()) << write_result.error().message();
    write_result = output_stream->WriteCaptureEvent(event2);
    EXPECT_FALSE(write_result.has_error()) << write_result.error().message();
    auto close_result = output_stream->Close();
    ASSERT_FALSE(close_result.has_error()) << close_result.error().message();
  };

  auto check_output_stream_content = [&](std::string_view stream_content) {
    ASSERT_GT(stream_content.size(), 24);
    ASSERT_EQ(stream_content.substr(0, 4), kFileSignature);
    uint64_t capture_section_offset = 0;
    memcpy(&capture_section_offset, stream_content.data() + 8, sizeof(capture_section_offset));
    ASSERT_EQ(capture_section_offset, 24);

    google::protobuf::io::ArrayInputStream input_stream(
        stream_content.data() + capture_section_offset,
        static_cast<int>(stream_content.size() - capture_section_offset));
    google::protobuf::io::CodedInputStream coded_input_stream(&input_stream);

    orbit_grpc_protos::ClientCaptureEvent event_from_file;
    uint32_t event_size;
    ASSERT_TRUE(coded_input_stream.ReadVarint32(&event_size));
    std::vector<uint8_t> buffer(event_size);
    ASSERT_TRUE(coded_input_stream.ReadRaw(buffer.data(), buffer.size()));
    ASSERT_TRUE(event_from_file.ParseFromArray(buffer.data(), buffer.size()));

    ASSERT_EQ(event_from_file.event_case(), orbit_grpc_protos::ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event_from_file.interned_string().key(), kAnswerKey);
    EXPECT_EQ(event_from_file.interned_string().intern(), kAnswerString);

    ASSERT_TRUE(coded_input_stream.ReadVarint32(&event_size));
    buffer = std::vector<uint8_t>(event_size);
    ASSERT_TRUE(coded_input_stream.ReadRaw(buffer.data(), buffer.size()));
    ASSERT_TRUE(event_from_file.ParseFromArray(buffer.data(), buffer.size()));

    ASSERT_EQ(event_from_file.event_case(), orbit_grpc_protos::ClientCaptureEvent::kInternedString);
    EXPECT_EQ(event_from_file.interned_string().key(), kNotAnAnswerKey);
    EXPECT_EQ(event_from_file.interned_string().intern(), kNotAnAnswerString);
  };

  // Test the case of outputting capture file content to a file
  {
    auto temporary_file_or_error = orbit_test_utils::TemporaryFile::Create();
    ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
    orbit_test_utils::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());
    temporary_file.CloseAndRemove();

    std::string temp_file_name = temporary_file.file_path().string();
    auto output_stream_or_error = CaptureFileOutputStream::Create(temp_file_name);
    ASSERT_TRUE(output_stream_or_error.has_value()) << output_stream_or_error.error().message();

    std::unique_ptr<CaptureFileOutputStream> output_stream =
        std::move(output_stream_or_error.value());
    EXPECT_TRUE(output_stream->IsOpen());
    write_events_then_close(output_stream.get());

    ErrorMessageOr<std::string> stream_content_or_error =
        orbit_base::ReadFileToString(temp_file_name);
    ASSERT_TRUE(stream_content_or_error.has_value()) << stream_content_or_error.error().message();
    const std::string& stream_content = stream_content_or_error.value();
    check_output_stream_content(stream_content);
  }

  // Test the case of outputting capture file content to a vector of raw buffers
  {
    BufferOutputStream output_buffer;
    std::unique_ptr<CaptureFileOutputStream> output_stream =
        CaptureFileOutputStream::Create(&output_buffer);
    EXPECT_TRUE(output_stream->IsOpen());
    write_events_then_close(output_stream.get());

    std::vector<unsigned char> buffered_data = output_buffer.TakeBuffer();
    const std::string stream_content(buffered_data.begin(), buffered_data.end());
    check_output_stream_content(stream_content);
  }
}

TEST(CaptureFileOutputStream, WriteAfterClose) {
  auto check_write_after_close = [&](CaptureFileOutputStream* output_stream) {
    EXPECT_TRUE(output_stream->IsOpen());

    auto close_result = output_stream->Close();
    ASSERT_FALSE(close_result.has_error()) << close_result.error().message();

    EXPECT_FALSE(output_stream->IsOpen());

    orbit_grpc_protos::ClientCaptureEvent event =
        CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString);

    EXPECT_DEATH((void)output_stream->WriteCaptureEvent(event), "");
  };

  // Test the case of outputting capture file content to a file
  {
    auto temporary_file_or_error = orbit_test_utils::TemporaryFile::Create();
    ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
    orbit_test_utils::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());
    temporary_file.CloseAndRemove();

    std::string temp_file_name = temporary_file.file_path().string();
    auto output_stream_or_error = CaptureFileOutputStream::Create(temp_file_name);
    ASSERT_TRUE(output_stream_or_error.has_value()) << output_stream_or_error.error().message();
    std::unique_ptr<CaptureFileOutputStream> output_stream =
        std::move(output_stream_or_error.value());
    check_write_after_close(output_stream.get());
  }

  // Test the case of outputting capture file content to a vector of raw buffers
  {
    BufferOutputStream output_buffer;
    std::unique_ptr<CaptureFileOutputStream> output_stream =
        CaptureFileOutputStream::Create(&output_buffer);
    check_write_after_close(output_stream.get());
  }
}

}  // namespace orbit_capture_file