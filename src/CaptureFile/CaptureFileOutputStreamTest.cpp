// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include "CaptureFile/CaptureFileOutputStream.h"
#include "CaptureFileConstants.h"
#include "OrbitBase/ReadFileToString.h"
#include "OrbitBase/TemporaryFile.h"

namespace orbit_capture_file {

static constexpr const char* kAnswerString =
    "Answer to the Ultimate Question of Life, The Universe, and Everything";
static constexpr uint64_t kAnswerKey = 42;

static orbit_grpc_protos::ClientCaptureEvent CreateInternedStringCaptureEvent() {
  orbit_grpc_protos::ClientCaptureEvent event;
  orbit_grpc_protos::InternedString* interned_string = event.mutable_interned_string();
  interned_string->set_key(42);
  interned_string->set_intern(kAnswerString);
  return event;
}

TEST(CaptureFileOutputStream, Smoke) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  std::string temp_file_name = temporary_file.file_path().string();

  auto output_stream_or_error = CaptureFileOutputStream::Create(temp_file_name);
  ASSERT_TRUE(output_stream_or_error.has_value()) << output_stream_or_error.error().message();

  std::unique_ptr<CaptureFileOutputStream> output_stream =
      std::move(output_stream_or_error.value());

  orbit_grpc_protos::ClientCaptureEvent event = CreateInternedStringCaptureEvent();

  auto write_result = output_stream->WriteCaptureEvent(event);
  EXPECT_FALSE(write_result.has_error()) << write_result.error().message();
  output_stream->Close();

  ErrorMessageOr<std::string> file_content_or_error = orbit_base::ReadFileToString(temp_file_name);
  ASSERT_TRUE(file_content_or_error.has_value()) << file_content_or_error.error().message();

  const std::string& file_content = file_content_or_error.value();

  ASSERT_GT(file_content.size(), 24);
  ASSERT_EQ(file_content.substr(0, 4), kFileSignature);
  uint64_t capture_section_offset = 0;
  memcpy(&capture_section_offset, file_content.data() + 8, sizeof(capture_section_offset));
  ASSERT_EQ(capture_section_offset, 24);

  google::protobuf::io::ArrayInputStream input_stream(
      file_content.data() + capture_section_offset,
      static_cast<int>(file_content.size() - capture_section_offset));
  google::protobuf::io::CodedInputStream coded_input_stream(&input_stream);

  orbit_grpc_protos::ClientCaptureEvent event_from_file;
  event_from_file.ParseFromCodedStream(&coded_input_stream);

  ASSERT_EQ(event_from_file.event_case(), orbit_grpc_protos::ClientCaptureEvent::kInternedString);
  EXPECT_EQ(event_from_file.interned_string().key(), kAnswerKey);
  EXPECT_EQ(event_from_file.interned_string().intern(), kAnswerString);
}

TEST(CaptureFileOutputStream, WriteAfterClose) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  std::string temp_file_name = temporary_file.file_path().string();

  auto output_stream_or_error = CaptureFileOutputStream::Create(temp_file_name);
  ASSERT_TRUE(output_stream_or_error.has_value()) << output_stream_or_error.error().message();

  std::unique_ptr<CaptureFileOutputStream> output_stream =
      std::move(output_stream_or_error.value());

  output_stream->Close();

  orbit_grpc_protos::ClientCaptureEvent event = CreateInternedStringCaptureEvent();

  EXPECT_DEATH((void)output_stream->WriteCaptureEvent(event), "");
}

}  // namespace orbit_capture_file