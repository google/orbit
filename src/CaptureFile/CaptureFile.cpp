// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/CaptureFile.h"

#include <absl/strings/str_format.h>
#include <absl/types/span.h>
#include <google/protobuf/io/coded_stream.h>
#include <google/protobuf/io/zero_copy_stream_impl.h>

#include <algorithm>
#include <array>
#include <cstring>
#include <limits>
#include <optional>
#include <string>
#include <string_view>
#include <utility>

#include "CaptureFile/CaptureFileSection.h"
#include "CaptureFile/ProtoSectionInputStream.h"
#include "CaptureFileConstants.h"
#include "OrbitBase/Align.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/SafeStrerror.h"
#include "ProtoSectionInputStreamImpl.h"

#ifdef __linux
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#endif

namespace orbit_capture_file {

namespace {

using orbit_base::unique_fd;

constexpr uint64_t kMaxNumberOfSections = std::numeric_limits<uint16_t>::max();

struct CaptureFileHeader {
  static constexpr uint64_t kSignatureSize = kFileSignature.size();
  static constexpr uint64_t kFileFormatVersionSize = sizeof(uint32_t);
  uint64_t capture_section_offset;
  uint64_t section_list_offset;
  static constexpr uint64_t kSectionListOffsetFieldOffset =
      kSignatureSize + kFileFormatVersionSize + sizeof(capture_section_offset);
};

class CaptureFileImpl : public CaptureFile {
 public:
  explicit CaptureFileImpl(std::filesystem::path file_path) : file_path_{std::move(file_path)} {}
  ~CaptureFileImpl() override = default;

  ErrorMessageOr<void> Initialize();

  ErrorMessageOr<uint64_t> AddUserDataSection(uint64_t section_size) override;

  ErrorMessageOr<void> ExtendSection(uint64_t section_number, size_t new_size) override;

  ErrorMessageOr<void> WriteToSection(uint64_t section_number, uint64_t offset_in_section,
                                      const void* data, size_t size) override;

  [[nodiscard]] const std::vector<CaptureFileSection>& GetSectionList() const override {
    return section_list_;
  }

  [[nodiscard]] std::optional<uint64_t> FindSectionByType(uint64_t section_type) const override;

  [[nodiscard]] std::vector<uint64_t> FindAllSectionsByType(uint64_t section_type) const override;

  ErrorMessageOr<void> ReadFromSection(uint64_t section_number, uint64_t offset_in_section,
                                       void* data, size_t size) override;

  std::unique_ptr<ProtoSectionInputStream> CreateCaptureSectionInputStream() override;

  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;

  std::unique_ptr<ProtoSectionInputStream> CreateProtoSectionInputStream(
      uint64_t section_number) override;

  ErrorMessageOr<uint64_t> AddAdditionalSectionOfType(uint64_t new_section_type,
                                                      size_t new_section_size) override;

 private:
  // Parse header and validate version/file-format
  ErrorMessageOr<void> ReadHeader();
  ErrorMessageOr<void> ReadSectionList();
  ErrorMessageOr<void> CalculateCaptureSectionSize();
  ErrorMessageOr<void> WriteSectionList(absl::Span<const CaptureFileSection> section_list,
                                        uint64_t offset);
  [[nodiscard]] bool IsThereSectionWithOffsetAfterSectionList() const;
  // Calculates where the current content of the file ends. This is the position where new data can
  // be written without overriding existing content.
  ErrorMessageOr<uint64_t> CalculateContentEnd() const;
  // Returns success when capture file is valid, otherwise an error message. A capture file is valid
  // if it contains at most one user data section that is the last section. And if there are no
  // other sections behind the section list.
  ErrorMessageOr<void> VerifyCaptureFileValid() const;
  // Returns true if file contains exactly 1 user data section that is the last section. Returns
  // false it file does not contain a user data section. Returns an error if file contains more than
  // one user data section, or if the one user data section is not the last section.
  ErrorMessageOr<bool> ContainsValidUserDataSection() const;

  std::filesystem::path file_path_;
  unique_fd fd_;
  CaptureFileHeader header_{};

  // This is used for boundary checks so that we do not end up
  // reading from sections following capture section, this is not
  // the exact size of the section but it is always >= the actual
  // size of the section. The user must rely on CaptureFinished
  // message to detect the last message for the capture section.
  uint64_t capture_section_size_ = 0;

  // This is the list of sections, which does not contain the section_list itself, nor the capture
  // section. The section_list is ordered by section offset. Meaning a section with lower offset
  // will come before a section with higher offset.
  std::vector<CaptureFileSection> section_list_;
};

ErrorMessageOr<uint64_t> GetEndOfFileOffset(const unique_fd& fd) {
#if defined(_WIN32)
  int64_t end_of_file = _lseeki64(fd.get(), 0, SEEK_END);
#else
  int64_t end_of_file = lseek64(fd.get(), 0, SEEK_END);
#endif
  if (end_of_file == -1) {
    return ErrorMessage{SafeStrerror(errno)};
  }

  return end_of_file;
}

ErrorMessageOr<void> CaptureFileImpl::Initialize() {
  OUTCOME_TRY(fd_, orbit_base::OpenExistingFileForReadWrite(file_path_));
  OUTCOME_TRY(ReadHeader());
  OUTCOME_TRY(ReadSectionList());
  OUTCOME_TRY(CalculateCaptureSectionSize());

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::CalculateCaptureSectionSize() {
  // If there are no additional sections the capture section ends at the EOF
  if (header_.section_list_offset == 0) {
    OUTCOME_TRY(auto&& end_of_file_offset, GetEndOfFileOffset(fd_));
    capture_section_size_ = end_of_file_offset - header_.capture_section_offset;
    return outcome::success();
  }

  // Otherwise it ends at the start of the next section or at the section list
  uint64_t min_section_offset = std::numeric_limits<uint64_t>::max();

  ORBIT_CHECK(!section_list_.empty());

  for (const auto& section : section_list_) {
    if (section.offset < min_section_offset) {
      min_section_offset = section.offset;
    }
  }

  if (header_.section_list_offset > header_.capture_section_offset) {
    min_section_offset = std::min(min_section_offset, header_.section_list_offset);
  }

  ORBIT_CHECK(min_section_offset < std::numeric_limits<uint64_t>::max());

  capture_section_size_ = min_section_offset - header_.capture_section_offset;

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::ReadSectionList() {
  if (header_.section_list_offset == 0) {
    return outcome::success();
  }

  OUTCOME_TRY(auto&& number_of_sections,
              orbit_base::ReadFullyAtOffset<uint64_t>(fd_, header_.section_list_offset));
  if (number_of_sections > kMaxNumberOfSections) {
    return ErrorMessage{absl::StrFormat("The section list is too large: %d (must be <= %d)",
                                        number_of_sections, kMaxNumberOfSections)};
  }

  std::vector<CaptureFileSection> section_list{number_of_sections};
  OUTCOME_TRY(auto&& bytes_read,
              orbit_base::ReadFullyAtOffset(
                  fd_, section_list.data(), number_of_sections * sizeof(CaptureFileSection),
                  header_.section_list_offset + sizeof(number_of_sections)));

  if (bytes_read < number_of_sections * sizeof(CaptureFileSection)) {
    return ErrorMessage{absl::StrFormat(
        "Unexpected EOF while reading section list: section list size=%d, bytes available=%d",
        number_of_sections * sizeof(CaptureFileSection), bytes_read)};
  }

  section_list_ = std::move(section_list);

  return outcome::success();
}

ErrorMessageOr<void> ValidateSignature(google::protobuf::io::CodedInputStream* coded_input,
                                       google::protobuf::io::FileInputStream* raw_input) {
  std::array<char, kFileSignature.size()> signature{};
  if (!coded_input->ReadRaw(static_cast<void*>(&signature), sizeof(signature))) {
    return ErrorMessage{
        absl::StrFormat("Failed to read the file signature. (IO subsystem error: %s)",
                        SafeStrerror(raw_input->GetErrno()))};
  }

  if (std::memcmp(signature.data(), kFileSignature.data(), kFileSignature.size()) != 0) {
    return ErrorMessage{"Invalid file signature"};
  }

  return outcome::success();
}

ErrorMessageOr<void> ValidateFileVersion(google::protobuf::io::CodedInputStream* coded_input,
                                         google::protobuf::io::FileInputStream* raw_input) {
  uint32_t version{};
  if (!coded_input->ReadLittleEndian32(&version)) {
    return ErrorMessage{
        absl::StrFormat("Could not read the file's version. (IO subsystem error: %s)",
                        SafeStrerror(raw_input->GetErrno()))};
  }

  if (version != kFileVersion) {
    return ErrorMessage{
        absl::StrFormat("Incompatible version %d, expected %d", version, kFileVersion)};
  }

  return outcome::success();
}

// Calculates how large (bytes) a section list (with `number_of_sections` sections) is when written
// to file.
[[nodiscard]] uint64_t CalculateSectionListSizeInFile(const uint64_t number_of_sections) {
  return sizeof(number_of_sections) + number_of_sections * sizeof(CaptureFileSection);
}

ErrorMessageOr<void> CaptureFileImpl::ReadHeader() {
  google::protobuf::io::FileInputStream raw_input{fd_.get()};
  google::protobuf::io::CodedInputStream coded_input{&raw_input};

  OUTCOME_TRY(ValidateSignature(&coded_input, &raw_input));
  OUTCOME_TRY(ValidateFileVersion(&coded_input, &raw_input));

  CaptureFileHeader header{};

  if (!coded_input.ReadLittleEndian64(&header.capture_section_offset)) {
    return ErrorMessage{"Could not read the capture section's offset value"};
  }

  if (!coded_input.ReadLittleEndian64(&header.section_list_offset)) {
    return ErrorMessage{"Could not read the section list's offset value"};
  }

  header_ = header;
  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::WriteToSection(uint64_t section_number,
                                                     uint64_t offset_in_section, const void* data,
                                                     size_t size) {
  ORBIT_CHECK(section_number < section_list_.size());

  const CaptureFileSection& section = section_list_[section_number];
  ORBIT_CHECK(offset_in_section + size <= section.size);

  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, data, size, section.offset + offset_in_section));

  return outcome::success();
}

bool CaptureFileImpl::IsThereSectionWithOffsetAfterSectionList() const {
  return std::any_of(section_list_.begin(), section_list_.end(),
                     [this](const CaptureFileSection& section) {
                       return section.offset > header_.section_list_offset;
                     });
}

ErrorMessageOr<uint64_t> CaptureFileImpl::AddUserDataSection(uint64_t section_size) {
  if (section_list_.size() == kMaxNumberOfSections) {
    return ErrorMessage{
        absl::StrFormat("Section list has reached its maximum size: %d", section_list_.size())};
  }

  // If there is already a user-data section return an error
  OUTCOME_TRY(const bool contains_user_data_section, ContainsValidUserDataSection());
  if (contains_user_data_section) {
    return ErrorMessage{"Cannot add USER_DATA section, file already contains a user data section"};
  }

  // Take a copy of section list
  auto section_list = section_list_;

  // If there are additional sections, the section list needs to be the last section, so it can be
  // amended.
  if (!section_list.empty() && IsThereSectionWithOffsetAfterSectionList()) {
    return ErrorMessage{
        "Cannot add USER_DATA section - there are sections behind the section list"};
  }

  uint64_t section_list_offset = header_.section_list_offset;

  // If no section list existed before, it is written at the end of the file
  if (header_.section_list_offset == 0) {
    OUTCOME_TRY(auto&& end_of_file, GetEndOfFileOffset(fd_));
    section_list_offset = orbit_base::AlignUp<8>(end_of_file);
  }

  uint64_t number_of_sections = section_list.size() + 1;

  uint64_t section_list_size = CalculateSectionListSizeInFile(number_of_sections);

  uint64_t user_data_section_offset =
      orbit_base::AlignUp<8>(section_list_offset + section_list_size);

  // Add USER_DATA section to the end of file - after section list
  section_list.push_back(CaptureFileSection{/*.type = */ kSectionTypeUserData,
                                            /*.offset = */ user_data_section_offset,
                                            /*.size = */ section_size});

  // Resize the file
  OUTCOME_TRY(orbit_base::ResizeFile(file_path_, user_data_section_offset + section_size));

  OUTCOME_TRY(WriteSectionList(section_list, section_list_offset));

  // Now update section list offset in the header if necessary
  if (header_.section_list_offset != section_list_offset) {
    OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, &section_list_offset,
                                               sizeof(section_list_offset),
                                               CaptureFileHeader::kSectionListOffsetFieldOffset));

    header_.section_list_offset = section_list_offset;
  }
  section_list_ = std::move(section_list);

  return section_list_.size() - 1;
}

ErrorMessageOr<void> CaptureFileImpl::ReadFromSection(uint64_t section_number,
                                                      uint64_t offset_in_section, void* data,
                                                      size_t size) {
  ORBIT_CHECK(section_number < section_list_.size());

  const CaptureFileSection& section = section_list_[section_number];
  ORBIT_CHECK(offset_in_section + size <= section.size);

  OUTCOME_TRY(auto&& bytes_read,
              orbit_base::ReadFullyAtOffset(fd_, data, size, section.offset + offset_in_section));

  // This shouldn't happen, it probably means someone has truncated the file while we were working
  // with it.
  if (bytes_read < size) {
    return ErrorMessage{
        absl::StrFormat("Unexpected EOF while reading from section number %d: This means that the "
                        "file is corrupted.",
                        section_number)};
  }

  return outcome::success();
}

std::unique_ptr<ProtoSectionInputStream> CaptureFileImpl::CreateCaptureSectionInputStream() {
  return std::make_unique<orbit_capture_file_internal::ProtoSectionInputStreamImpl>(
      fd_, header_.capture_section_offset, capture_section_size_);
}

std::unique_ptr<ProtoSectionInputStream> CaptureFileImpl::CreateProtoSectionInputStream(
    uint64_t section_number) {
  ORBIT_CHECK(section_number < section_list_.size());
  const auto& section_info = section_list_[section_number];

  return std::make_unique<orbit_capture_file_internal::ProtoSectionInputStreamImpl>(
      fd_, section_info.offset, section_info.size);
}

std::optional<uint64_t> CaptureFileImpl::FindSectionByType(uint64_t section_type) const {
  for (size_t i = 0; i < section_list_.size(); ++i) {
    if (section_list_[i].type == section_type) {
      return i;
    }
  }

  return std::nullopt;
}

std::vector<uint64_t> CaptureFileImpl::FindAllSectionsByType(uint64_t section_type) const {
  std::vector<uint64_t> result_indices;

  for (size_t i = 0; i < section_list_.size(); ++i) {
    if (section_list_[i].type == section_type) {
      result_indices.push_back(i);
    }
  }

  return result_indices;
}

const std::filesystem::path& CaptureFileImpl::GetFilePath() const { return file_path_; }

ErrorMessageOr<void> orbit_capture_file::CaptureFileImpl::ExtendSection(uint64_t section_number,
                                                                        size_t new_size) {
  // Currently we do it only for last section of the file.
  ORBIT_CHECK(section_number < section_list_.size());

  const CaptureFileSection& section = section_list_[section_number];
  if (section.size >= new_size) {
    return outcome::success();
  }

  // Check format: The section should be the last section in the file and we expect it to go
  // after section list. We currently handle only one read/write section, in case we get more
  // in the future this will need to be revisited.
  if (header_.section_list_offset > section.offset) {
    return ErrorMessage{absl::StrFormat(
        "Cannot resize section %d: The section is located before section list.", section_number)};
  }

  if (std::any_of(section_list_.begin(), section_list_.end(),
                  [&section](const CaptureFileSection& section_to_check) {
                    return section_to_check.offset > section.offset;
                  })) {
    return ErrorMessage{absl::StrFormat(
        "Cannot resize section %d: The section is not the last section in the file.",
        section_number)};
  }

  // Update section size and resize the file
  auto section_list = section_list_;  // create a copy
  section_list[section_number].size = new_size;

  // We checked that this is the last section so the new file size is the section offset + size
  OUTCOME_TRY(orbit_base::ResizeFile(
      file_path_, section_list[section_number].offset + section_list[section_number].size));
  OUTCOME_TRY(WriteSectionList(section_list, header_.section_list_offset));

  section_list_ = section_list;
  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::WriteSectionList(
    absl::Span<const CaptureFileSection> section_list, uint64_t offset) {
  uint64_t number_of_sections = section_list.size();
  // first write new section list at new offset
  OUTCOME_TRY(
      orbit_base::WriteFullyAtOffset(fd_, &number_of_sections, sizeof(number_of_sections), offset));
  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, section_list.data(),
                                             number_of_sections * sizeof(CaptureFileSection),
                                             offset + sizeof(number_of_sections)));
  return outcome::success();
}

ErrorMessageOr<uint64_t> CaptureFileImpl::CalculateContentEnd() const {
  // If no section list exists, the end of the file is used.
  if (header_.section_list_offset == 0) {
    return GetEndOfFileOffset(fd_);
  }

  OUTCOME_TRY(const bool contains_user_data_section, ContainsValidUserDataSection());

  if (contains_user_data_section) {
    return section_list_.back().offset + section_list_.back().size;
  }

  // Otherwise the section list is the last thing in the capture file.

  if (IsThereSectionWithOffsetAfterSectionList()) {
    return ErrorMessage{
        "Unable to calculate where the content of the capture file ends: The file contains a non "
        "user data section after the section list."};
  }

  return header_.section_list_offset + CalculateSectionListSizeInFile(section_list_.size());
}

ErrorMessageOr<void> CaptureFileImpl::VerifyCaptureFileValid() const {
  OUTCOME_TRY(ContainsValidUserDataSection());

  const bool contains_non_user_data_section_after_section_list = std::any_of(
      section_list_.begin(), section_list_.end(), [this](const CaptureFileSection& section) {
        return section.type != kSectionTypeUserData && section.offset > header_.section_list_offset;
      });
  if (contains_non_user_data_section_after_section_list) {
    return ErrorMessage{
        "Capture file is invalid, because there are additional (non user data) sections after the "
        "section list."};
  }

  return outcome::success();
}

ErrorMessageOr<bool> CaptureFileImpl::ContainsValidUserDataSection() const {
  if (!FindSectionByType(kSectionTypeUserData).has_value()) {
    return false;
  }
  if (FindAllSectionsByType(kSectionTypeUserData).size() > 1) {
    return ErrorMessage{
        "Capture file is invalid, because it contains more than 1 user data section."};
  }

  // File contains exactly one user data section.

  if (section_list_.back().type != kSectionTypeUserData) {
    return ErrorMessage{
        "Capture file is invalid, because the user data section is not the last section."};
  }

  return true;
}

ErrorMessageOr<uint64_t> CaptureFileImpl::AddAdditionalSectionOfType(uint64_t new_section_type,
                                                                     size_t new_section_size) {
  if (section_list_.size() == kMaxNumberOfSections) {
    return ErrorMessage{
        absl::StrFormat("Section list has reached its maximum size: %d", section_list_.size())};
  }
  if (new_section_type == kSectionTypeUserData) {
    return ErrorMessage{"Cannot add a user data section as an additional (read only) section."};
  }
  OUTCOME_TRY(VerifyCaptureFileValid());

  // 1. Copy section list (new_section_list) and append new_section
  std::vector<CaptureFileSection> new_section_list = section_list_;

  // The new section is placed where the section list is currently.
  uint64_t new_section_offset = header_.section_list_offset;
  if (new_section_offset == 0) {
    // If no section list exists, the new section is placed at the end of the file.
    OUTCOME_TRY(const uint64_t end_of_file_offset, GetEndOfFileOffset(fd_));
    new_section_offset = orbit_base::AlignUp<8>(end_of_file_offset);
  }

  const CaptureFileSection new_section{new_section_type, new_section_offset, new_section_size};

  // The new (added) section is put at the end of the section list
  new_section_list.emplace_back(new_section);
  uint64_t new_section_index = new_section_list.size() - 1;

  // 1.1 If a user data section exists, it is swapped with the new_section, so its the last section.
  if (const std::optional<uint64_t> user_data_section_index =
          FindSectionByType(kSectionTypeUserData);
      user_data_section_index.has_value()) {
    std::swap(new_section_list[new_section_index],
              new_section_list[user_data_section_index.value()]);
    new_section_index = user_data_section_index.value();
  }

  // 2. Compute the new_section_list_offset, avoiding override of existing content.
  const uint64_t new_section_end = new_section_offset + new_section_size;
  OUTCOME_TRY(const uint64_t file_content_end, CalculateContentEnd());

  const uint64_t new_section_list_offset =
      orbit_base::AlignUp<8>(std::max(new_section_end, file_content_end));

  // 3. Resize file to make space for the new_section_list.
  const uint64_t new_section_list_end =
      new_section_list_offset + CalculateSectionListSizeInFile(new_section_list.size());
  OUTCOME_TRY(orbit_base::ResizeFile(file_path_, new_section_list_end));

  // 3.3 If a user data section exists, copy behind the new section list.
  if (FindSectionByType(kSectionTypeUserData).has_value()) {
    CaptureFileSection& user_data_section = new_section_list.back();

    const uint64_t new_user_data_section_offset = orbit_base::AlignUp<8>(new_section_list_end);

    const uint64_t new_user_data_section_end =
        new_user_data_section_offset + user_data_section.size;
    OUTCOME_TRY(orbit_base::ResizeFile(file_path_, new_user_data_section_end));

    std::vector<char> bytes(user_data_section.size);
    OUTCOME_TRY(orbit_base::ReadFullyAtOffset(fd_, bytes.data(), user_data_section.size,
                                              user_data_section.offset));
    OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, bytes.data(), user_data_section.size,
                                               new_user_data_section_offset));
    user_data_section.offset = new_user_data_section_offset;
  }

  // 4. Write new section list and update pointer in header.
  OUTCOME_TRY(WriteSectionList(new_section_list, new_section_list_offset));
  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, &new_section_list_offset,
                                             sizeof(new_section_list_offset),
                                             CaptureFileHeader::kSectionListOffsetFieldOffset));
  header_.section_list_offset = new_section_list_offset;
  section_list_ = new_section_list;

  return new_section_index;
}

}  // namespace

ErrorMessageOr<std::unique_ptr<CaptureFile>> CaptureFile::OpenForReadWrite(
    const std::filesystem::path& file_path) {
  auto capture_file = std::make_unique<CaptureFileImpl>(file_path);
  OUTCOME_TRY(capture_file->Initialize());
  return capture_file;
}

}  // namespace orbit_capture_file