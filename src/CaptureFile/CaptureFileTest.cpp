// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/base/casts.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stdint.h>

#include "CaptureFile/CaptureFile.h"
#include "CaptureFile/CaptureFileOutputStream.h"
#include "CaptureFileConstants.h"
#include "OrbitBase/TemporaryFile.h"

namespace orbit_capture_file {

static constexpr const char* kAnswerString =
    "Answer to the Ultimate Question of Life, The Universe, and Everything";
static constexpr const char* kNotAnAnswerString = "Some odd number, not the answer.";
static constexpr uint64_t kAnswerKey = 42;
static constexpr uint64_t kNotAnAnswerKey = 43;

static orbit_grpc_protos::ClientCaptureEvent CreateInternedStringCaptureEvent(
    uint64_t key, const std::string& str) {
  orbit_grpc_protos::ClientCaptureEvent event;
  orbit_grpc_protos::InternedString* interned_string = event.mutable_interned_string();
  interned_string->set_key(key);
  interned_string->set_intern(str);
  return event;
}

TEST(CaptureFile, CreateCaptureFileAndAddSection) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  std::string temp_file_name = temporary_file.file_path().string();

  auto output_stream_or_error = CaptureFileOutputStream::Create(temp_file_name);
  ASSERT_TRUE(output_stream_or_error.has_value()) << output_stream_or_error.error().message();

  std::unique_ptr<CaptureFileOutputStream> output_stream =
      std::move(output_stream_or_error.value());

  EXPECT_TRUE(output_stream->IsOpen());

  orbit_grpc_protos::ClientCaptureEvent event1 =
      CreateInternedStringCaptureEvent(kAnswerKey, kAnswerString);
  orbit_grpc_protos::ClientCaptureEvent event2 =
      CreateInternedStringCaptureEvent(kNotAnAnswerKey, kNotAnAnswerString);

  auto write_result = output_stream->WriteCaptureEvent(event1);
  ASSERT_FALSE(write_result.has_error()) << write_result.error().message();
  write_result = output_stream->WriteCaptureEvent(event2);
  ASSERT_FALSE(write_result.has_error()) << write_result.error().message();
  auto close_result = output_stream->Close();
  ASSERT_FALSE(close_result.has_error()) << close_result.error().message();

  auto capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(capture_file_or_error.has_value()) << capture_file_or_error.error().message();
  std::unique_ptr<CaptureFile> capture_file = std::move(capture_file_or_error.value());
  EXPECT_EQ(capture_file->GetSectionList().size(), 0);

  auto section_number_or_error = capture_file->AddSection(1, 333);
  ASSERT_TRUE(section_number_or_error.has_value()) << section_number_or_error.error().message();
  ASSERT_EQ(capture_file->GetSectionList().size(), 1);
  EXPECT_EQ(section_number_or_error.value(), 0);
  {
    const auto& capture_file_section = capture_file->GetSectionList()[0];
    EXPECT_EQ(capture_file_section.size, 333);
    EXPECT_EQ(capture_file_section.type, 1);
  }

  // Write something to the section
  std::string something{"this is something"};
  constexpr uint64_t kOffsetInSection = 25;
  auto write_to_section_result =
      capture_file->WriteToSection(0, kOffsetInSection, something.c_str(), something.size());
  ASSERT_FALSE(write_to_section_result.has_error()) << write_to_section_result.error().message();

  {
    std::string content;
    content.resize(something.size());
    auto read_result =
        capture_file->ReadFromSection(0, kOffsetInSection, content.data(), something.size());
    ASSERT_FALSE(read_result.has_error()) << read_result.error().message();
    EXPECT_EQ(content, something);
  }

  // Reopen the file to make sure this information was saved
  capture_file.reset();

  capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(capture_file_or_error.has_value()) << capture_file_or_error.error().message();
  capture_file = std::move(capture_file_or_error.value());
  EXPECT_EQ(capture_file->GetSectionList().size(), 1);
  {
    const auto& capture_file_section = capture_file->GetSectionList()[0];
    EXPECT_EQ(capture_file_section.type, 1);
    EXPECT_EQ(capture_file_section.size, 333);
    EXPECT_GT(capture_file_section.offset, 0);
  }

  {
    std::string content;
    content.resize(something.size());
    auto read_result =
        capture_file->ReadFromSection(0, kOffsetInSection, content.data(), something.size());
    ASSERT_FALSE(read_result.has_error()) << read_result.error().message();
    EXPECT_EQ(content, something);
  }
}

TEST(CaptureFile, OpenCaptureFileInvalidSignature) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  auto write_result =
      orbit_base::WriteFully(temporary_file.fd(), "This is not an Orbit Capture File");

  auto capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(capture_file_or_error.has_error());
  EXPECT_EQ(capture_file_or_error.error().message(), "Invalid file signature");
}

TEST(CaptureFile, OpenCaptureFileTooSmall) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  auto write_result = orbit_base::WriteFully(temporary_file.fd(), "ups");

  auto capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(capture_file_or_error.has_error());
  EXPECT_EQ(capture_file_or_error.error().message(), "Not enough bytes left in the file: 3 < 24");
}

static std::string CreateHeader(uint32_t version, uint64_t capture_section_offset,
                                uint64_t section_list_offset) {
  std::string header{"ORBT", 4};
  header.append(std::string_view{absl::bit_cast<const char*>(&version), sizeof(version)});
  header.append(std::string_view{absl::bit_cast<const char*>(&capture_section_offset),
                                 sizeof(capture_section_offset)});
  header.append(std::string_view{absl::bit_cast<const char*>(&section_list_offset),
                                 sizeof(section_list_offset)});

  return header;
}

TEST(CaptureFile, OpenCaptureFileInvalidVersion) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  std::string header = CreateHeader(0, 0, 0);

  auto write_result = orbit_base::WriteFully(temporary_file.fd(), header);

  auto capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(capture_file_or_error.has_error());
  EXPECT_EQ(capture_file_or_error.error().message(), "Incompatible version 0, expected 1");
}

TEST(CaptureFile, OpenCaptureFileInvalidSectionListSize) {
  auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
  ASSERT_TRUE(temporary_file_or_error.has_value()) << temporary_file_or_error.error().message();
  orbit_base::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());

  std::string header = CreateHeader(1, 24, 32);
  header.append(std::string_view{"12345678", 8});
  constexpr uint16_t kSectionListSize = 10;
  header.append(
      std::string_view{absl::bit_cast<const char*>(&kSectionListSize), sizeof(kSectionListSize)});

  auto write_result = orbit_base::WriteFully(temporary_file.fd(), header);

  auto capture_file_or_error = CaptureFile::OpenForReadWrite(temporary_file.file_path());
  ASSERT_TRUE(capture_file_or_error.has_error());
  EXPECT_THAT(capture_file_or_error.error().message(),
              testing::HasSubstr("Unexpected EOF while reading section list"));
}

}  // namespace orbit_capture_file