// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/CaptureFileHelpers.h"

#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl_lite.h>
#include <stddef.h>

#include <cstdint>
#include <memory>
#include <optional>

#include "CaptureFile/CaptureFile.h"
#include "CaptureFile/CaptureFileSection.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/MakeUniqueForOverwrite.h"

namespace orbit_capture_file {

ErrorMessageOr<void> WriteUserData(
    const std::filesystem::path& capture_file_path,
    const orbit_client_protos::UserDefinedCaptureInfo& user_defined_capture_info) {
  OUTCOME_TRY(auto&& capture_file, CaptureFile::OpenForReadWrite(capture_file_path));

  uint32_t message_size = user_defined_capture_info.ByteSizeLong();
  const size_t buf_size =
      message_size + google::protobuf::io::CodedOutputStream::VarintSize32(message_size);
  auto buf = make_unique_for_overwrite<uint8_t[]>(buf_size);
  google::protobuf::io::ArrayOutputStream array_output_stream{buf.get(),
                                                              static_cast<int>(buf_size)};
  google::protobuf::io::CodedOutputStream coded_output_stream{&array_output_stream};
  coded_output_stream.WriteVarint32(message_size);
  // We do not expect any errors from CodedOutputStream backed by ArrayOutputStream of correct size
  ORBIT_CHECK(user_defined_capture_info.SerializeToCodedStream(&coded_output_stream));

  auto section_index = capture_file->FindSectionByType(kSectionTypeUserData);
  if (section_index.has_value()) {
    OUTCOME_TRY(capture_file->ExtendSection(section_index.value(), buf_size));
  } else {
    OUTCOME_TRY(auto&& section_number, capture_file->AddUserDataSection(buf_size));
    section_index = section_number;
  }

  OUTCOME_TRY(capture_file->WriteToSection(section_index.value(), 0, buf.get(), buf_size));

  return outcome::success();
}

}  // namespace orbit_capture_file