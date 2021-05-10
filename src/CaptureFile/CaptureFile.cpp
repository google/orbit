// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "CaptureFile/CaptureFile.h"

#include "CaptureFileConstants.h"
#include "CaptureSectionInputStreamImpl.h"
#include "OrbitBase/File.h"

namespace orbit_capture_file {

namespace {

using orbit_base::unique_fd;

struct CaptureFileHeader {
  std::array<char, kFileSignatureSize> signature;
  uint32_t version;
  uint64_t capture_section_offset;
  uint64_t section_list_offset;
};

class CaptureFileImpl : public CaptureFile {
 public:
  explicit CaptureFileImpl(std::filesystem::path file_path) : file_path_{std::move(file_path)} {}
  ~CaptureFileImpl() override = default;

  ErrorMessageOr<void> Initialize();

  ErrorMessageOr<uint64_t> AddSection(uint64_t section_type, uint64_t section_size) override;

  ErrorMessageOr<void> WriteToSection(uint64_t section_number, uint64_t offset_in_section,
                                      const void* data, size_t size) override;

  [[nodiscard]] const std::vector<CaptureFileSection>& GetSectionList() const override {
    return section_list_;
  }

  [[nodiscard]] std::optional<uint64_t> FindSectionByType(uint64_t section_type) const override;

  ErrorMessageOr<void> ReadFromSection(uint64_t section_number, uint64_t offset_in_section,
                                       void* data, size_t size) override;

  std::unique_ptr<CaptureSectionInputStream> CreateCaptureSectionInputStream() override;

  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;

 private:
  // Parse header and validate version/file-format
  ErrorMessageOr<void> ReadHeader();
  ErrorMessageOr<void> ReadSectionList();
  ErrorMessageOr<void> CalculateCaptureSectionSize();

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
              orbit_base::ReadFullyAtOffset<uint16_t>(fd_, header_.section_list_offset));
  std::vector<CaptureFileSection> section_list{number_of_sections};
  OUTCOME_TRY(bytes_read,
              orbit_base::ReadFullyAtOffset(fd_, section_list.data(),
                                            number_of_sections * sizeof(CaptureFileSection),
                                            header_.section_list_offset + sizeof(uint16_t)));

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

  if (std::memcmp(header_.signature.data(), kFileSignature, header_.signature.size()) != 0) {
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

ErrorMessageOr<uint64_t> CaptureFileImpl::AddSection(uint64_t section_type, uint64_t section_size) {
  if (section_list_.size() == std::numeric_limits<uint16_t>::max()) {
    return ErrorMessage{
        absl::StrFormat("Section list has reached its maximum size: %d", section_list_.size())};
  }

  // Take a copy of section list
  auto section_list = section_list_;

  // The current file format always places the section list is at the end of the file
  // therefore the current section_offset is going to be the offset for the new section.
  uint64_t new_section_offset = header_.section_list_offset;

  // Except if we don't have any additional sections the first section offset should go
  // to the end of the file.
  if (new_section_offset == 0) {
    CHECK(section_list.empty());
    OUTCOME_TRY(end_of_file, GetEndOfFileOffset(fd_));
    new_section_offset = AlignUp<8>(end_of_file);
  }

  section_list.push_back(CaptureFileSection{/*.type = */ section_type,
                                            /*.offset = */ new_section_offset,
                                            /*.size = */ section_size});

  uint64_t new_section_list_offset = AlignUp<8>(new_section_offset + section_size);

  // first write new section list at new offset
  uint16_t number_of_sections = section_list.size();
  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, &number_of_sections, sizeof(number_of_sections),
                                             new_section_list_offset));
  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, section_list.data(),
                                             number_of_sections * sizeof(CaptureFileSection),
                                             new_section_list_offset + sizeof(number_of_sections)));

  // Now update section list offset in the header
  uint64_t section_list_offset_field_offset = offsetof(CaptureFileHeader, section_list_offset);
  OUTCOME_TRY(orbit_base::WriteFullyAtOffset(fd_, &new_section_list_offset,
                                             sizeof(new_section_list_offset),
                                             section_list_offset_field_offset));

  header_.section_list_offset = new_section_list_offset;
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

std::unique_ptr<CaptureSectionInputStream> CaptureFileImpl::CreateCaptureSectionInputStream() {
  return std::make_unique<orbit_capture_file_internal::CaptureSectionInputStreamImpl>(
      fd_, header_.capture_section_offset, capture_section_size_);
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

}  // namespace

ErrorMessageOr<std::unique_ptr<CaptureFile>> CaptureFile::OpenForReadWrite(
    const std::filesystem::path& file_path) {
  auto capture_file = std::make_unique<CaptureFileImpl>(file_path);
  OUTCOME_TRY(capture_file->Initialize());
  return capture_file;
}

}  // namespace orbit_capture_file