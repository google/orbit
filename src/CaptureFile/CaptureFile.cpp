// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/CaptureFile.h"

#include "CaptureFileConstants.h"
#include "OrbitBase/File.h"
#include "ProtoSectionInputStreamImpl.h"

namespace orbit_capture_file {

namespace {

using orbit_base::unique_fd;

constexpr uint64_t kMaxNumberOfSections = std::numeric_limits<uint16_t>::max();

struct CaptureFileHeader {
  std::array<char, kFileSignature.size()> signature;
  uint32_t version;
  uint64_t capture_section_offset;
  uint64_t section_list_offset;
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

  ErrorMessageOr<void> ReadFromSection(uint64_t section_number, uint64_t offset_in_section,
                                       void* data, size_t size) override;

  std::unique_ptr<ProtoSectionInputStream> CreateCaptureSectionInputStream() override;

  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;

  std::unique_ptr<ProtoSectionInputStream> CreateProtoSectionInputStream(
      uint64_t section_number) override;

 private:
  // Parse header and validate version/file-format
  ErrorMessageOr<void> ReadHeader();
  ErrorMessageOr<void> ReadSectionList();
  ErrorMessageOr<void> CalculateCaptureSectionSize();
  ErrorMessageOr<void> WriteSectionList(const std::vector<CaptureFileSection>& section_list,
                                        uint64_t offset);
  [[nodiscard]] bool IsThereSectionWithOffsetAfterSectionList() const;

  std::filesystem::path file_path_;
  unique_fd fd_;
  CaptureFileHeader header_{};

  // This is used for boundary checks so that we do not end up
  // reading from sections following capture section, this is not
  // the exact size of the section but it is always >= the actual
  // size of the section. The user must rely on CaptureFinished
  // message to detect the last message for the capture section.
  uint64_t capture_section_size_ = 0;

  std::vector<CaptureFileSection> section_list_;
};

template <uint64_t alignment>
constexpr uint64_t AlignUp(uint64_t value) {
  // alignment must be a power of 2
  static_assert((alignment & (alignment - 1)) == 0);
  return (value + (alignment - 1)) & ~(alignment - 1);
}

ErrorMessageOr<uint64_t> GetEndOfFileOffset(const unique_fd& fd) {
  off_t end_of_file = lseek(fd.get(), 0, SEEK_END);
  if (end_of_file == -1) {
    return ErrorMessage{SafeStrerror(errno)};
  }

  return end_of_file;
}

ErrorMessageOr<void> CaptureFileImpl::Initialize() {
  auto fd_or_error = orbit_base::OpenExistingFileForReadWrite(file_path_);
  if (fd_or_error.has_error()) {
    return fd_or_error.error();
  }

  fd_ = std::move(fd_or_error.value());

  OUTCOME_TRY(ReadHeader());
  OUTCOME_TRY(ReadSectionList());
  OUTCOME_TRY(CalculateCaptureSectionSize());

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::CalculateCaptureSectionSize() {
  // If there are no additional sections the capture section ends at the EOF
  if (header_.section_list_offset == 0) {
    OUTCOME_TRY(end_of_file_offset, GetEndOfFileOffset(fd_));
    capture_section_size_ = end_of_file_offset - header_.capture_section_offset;
    return outcome::success();
  }

  // Otherwise it ends at the start of the next section
  uint64_t min_section_offset = std::numeric_limits<uint64_t>::max();

  CHECK(!section_list_.empty());

  for (const auto& section : section_list_) {
    if (section.offset < min_section_offset) {
      min_section_offset = section.offset;
    }
  }

  CHECK(min_section_offset < std::numeric_limits<uint64_t>::max());

  capture_section_size_ = min_section_offset - header_.capture_section_offset;

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::ReadSectionList() {
  if (header_.section_list_offset == 0) {
    return outcome::success();
  }

  OUTCOME_TRY(number_of_sections,
              orbit_base::ReadFullyAtOffset<uint64_t>(fd_, header_.section_list_offset));
  if (number_of_sections > kMaxNumberOfSections) {
    return ErrorMessage{absl::StrFormat("The section list is too large: %d (must be <= %d)",
                                        number_of_sections, kMaxNumberOfSections)};
  }

  std::vector<CaptureFileSection> section_list{number_of_sections};
  OUTCOME_TRY(bytes_read,
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

ErrorMessageOr<void> CaptureFileImpl::ReadHeader() {
  OUTCOME_TRY(header, orbit_base::ReadFullyAtOffset<CaptureFileHeader>(fd_, 0));
  header_ = header;

  if (std::memcmp(header_.signature.data(), kFileSignature.data(), kFileSignature.size()) != 0) {
    return ErrorMessage{"Invalid file signature"};
  }

  if (header_.version != kFileVersion) {
    return ErrorMessage{
        absl::StrFormat("Incompatible version %d, expected %d", header_.version, kFileVersion)};
  }

  return outcome::success();
}

ErrorMessageOr<void> CaptureFileImpl::WriteToSection(uint64_t section_number,
                                                     uint64_t offset_in_section, const void* data,
                                                     size_t size) {
  CHECK(section_number < section_list_.size());

  const CaptureFileSection& section = section_list_[section_number];
  CHECK(offset_in_section + size <= section.size);

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

  // Take a copy of section list
  auto section_list = section_list_;

  // If there is already a user-data section return an error
  for (const auto& section : section_list) {
    if (section.type == kSectionTypeUserData) {
      return ErrorMessage{absl::StrFormat(
          "Cannot add USER_DATA section - there is already one at offset %#x", section.offset)};
    }
  }

  uint64_t section_list_offset = header_.section_list_offset;

  // If we don't have any additional sections or if there are any sections starting after
  // section_list move section list to the end of the file.
  if (header_.section_list_offset == 0 || IsThereSectionWithOffsetAfterSectionList()) {
    CHECK(section_list.empty());
    OUTCOME_TRY(end_of_file, GetEndOfFileOffset(fd_));
    section_list_offset = AlignUp<8>(end_of_file);
  }

  uint64_t number_of_sections = section_list.size() + 1;

  uint64_t section_list_size =
      sizeof(number_of_sections) + number_of_sections * sizeof(CaptureFileSection);

  uint64_t new_section_offset = AlignUp<8>(section_list_offset + section_list_size);

  // Add USER_DATA section to the end of file - after section list
  section_list.push_back(CaptureFileSection{/*.type = */ kSectionTypeUserData,
                                            /*.offset = */ new_section_offset,
                                            /*.size = */ section_size});

  // Resize the file
  OUTCOME_TRY(orbit_base::ResizeFile(file_path_, new_section_offset + section_size));

  OUTCOME_TRY(WriteSectionList(section_list, section_list_offset));

  // Now update section list offset in the header if necessary
  if (header_.section_list_offset != section_list_offset) {
    uint64_t section_list_offset_field_offset = offsetof(CaptureFileHeader, section_list_offset);
    OUTCOME_TRY(orbit_base::WriteFullyAtOffset(
        fd_, &section_list_offset, sizeof(section_list_offset), section_list_offset_field_offset));

    header_.section_list_offset = section_list_offset;
  }
  section_list_ = std::move(section_list);

  return section_list_.size() - 1;
}

ErrorMessageOr<void> CaptureFileImpl::ReadFromSection(uint64_t section_number,
                                                      uint64_t offset_in_section, void* data,
                                                      size_t size) {
  CHECK(section_number < section_list_.size());

  const CaptureFileSection& section = section_list_[section_number];
  CHECK(offset_in_section + size <= section.size);

  OUTCOME_TRY(bytes_read,
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
  CHECK(section_number < section_list_.size());
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

const std::filesystem::path& CaptureFileImpl::GetFilePath() const { return file_path_; }

ErrorMessageOr<void> orbit_capture_file::CaptureFileImpl::ExtendSection(uint64_t section_number,
                                                                        size_t new_size) {
  // Currently we do it only for last section of the file.
  CHECK(section_number < section_list_.size());

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
    const std::vector<CaptureFileSection>& section_list, uint64_t offset) {
  uint64_t number_of_sections = section_list.size();
  // first write new section list at new offset
  OUTCOME_TRY(
      orbit_base::WriteFullyAtOffset(fd_, &number_of_sections, sizeof(number_of_sections), offset));
  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, section_list.data(),
                                             number_of_sections * sizeof(CaptureFileSection),
                                             offset + sizeof(number_of_sections)));
  return outcome::success();
}

}  // namespace

ErrorMessageOr<std::unique_ptr<CaptureFile>> CaptureFile::OpenForReadWrite(
    const std::filesystem::path& file_path) {
  auto capture_file = std::make_unique<CaptureFileImpl>(file_path);
  OUTCOME_TRY(capture_file->Initialize());
  return capture_file;
}

}  // namespace orbit_capture_file