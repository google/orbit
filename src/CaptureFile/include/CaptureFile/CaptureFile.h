// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CAPTURE_FILE_CAPTURE_FILE_H_
#define CAPTURE_FILE_CAPTURE_FILE_H_

#include <stddef.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <vector>

#include "CaptureFile/CaptureFileSection.h"
#include "CaptureFile/ProtoSectionInputStream.h"
#include "OrbitBase/Result.h"

namespace orbit_capture_file {

// The CaptureFile provides functionality to read and write sections to a capture file. The
// CaptureSection is the main section (not contained in the section list) located directly after the
// CaptureFileHeader (use CaptureFileOutputStream to generate this main section of the file). The
// file format description can be found in src/CaptureFile/FORMAT.md file.
class CaptureFile {
 public:
  CaptureFile() = default;
  virtual ~CaptureFile() = default;

  // Get current additional section list. Section number is the index for the section in the vector.
  [[nodiscard]] virtual const std::vector<CaptureFileSection>& GetSectionList() const = 0;

  // Returns the index for the first section with specified type.
  [[nodiscard]] virtual std::optional<uint64_t> FindSectionByType(uint64_t section_type) const = 0;

  // Returns all indices of the the sections with specified type.
  [[nodiscard]] virtual std::vector<uint64_t> FindAllSectionsByType(
      uint64_t section_type) const = 0;

  // Adds user data section, returns added section number. This will return an error if an user data
  // section already exists, or if there are other sections behind the section list. The user data
  // section is added to the end of the section list. The file layout is adjusted accordingly. This
  // function makes the best effort to preserve the format consistency in the case of an I/O error,
  // but the file size could still end up being changed (for example if updated section list was
  // successfully written to file and space for the section was successfully reserved but the
  // function has failed to update file header with the new position of the section list).
  virtual ErrorMessageOr<uint64_t> AddUserDataSection(uint64_t section_size) = 0;

  // Extend the last section in the file. This function is intended as fast-path for USER_DATA
  // read-write section, other sections in the file are supposed to read-only which lets us
  // avoid copying data around for the most of the file in the case when only user data
  // is modified. The function will return a error if the section is not located at the
  // end of file.
  virtual ErrorMessageOr<void> ExtendSection(uint64_t section_number, size_t new_size) = 0;

  // Write data from the buffer to the section with specified offset. The data must be in bound
  // of the section. The function will CHECK fail if it is not.
  virtual ErrorMessageOr<void> WriteToSection(uint64_t section_number, uint64_t section_offset,
                                              const void* data, size_t size) = 0;

  // Read data from the section at specified offset. The data must be in section bounds, otherwise
  // this function will CHECK fail.
  virtual ErrorMessageOr<void> ReadFromSection(uint64_t section_number, uint64_t section_offset,
                                               void* data, size_t size) = 0;

  [[nodiscard]] virtual const std::filesystem::path& GetFilePath() const = 0;

  virtual std::unique_ptr<ProtoSectionInputStream> CreateProtoSectionInputStream(
      uint64_t section_number) = 0;

  virtual std::unique_ptr<ProtoSectionInputStream> CreateCaptureSectionInputStream() = 0;

  static ErrorMessageOr<std::unique_ptr<CaptureFile>> OpenForReadWrite(
      const std::filesystem::path& file_path);

  // Adds an additional section to the capture file and returns the index of the added section. The
  // new section is placed behind existing additional sections. The updated section list is placed
  // after the new section. If a user data section exists, it is copied to after the new section
  // list. This function will return an error in the following cases:
  // * section list is full (kMaxNumberOfSections)
  // * the new section is a user data section (new_section_type == kSectionTypeUserData)
  // * The capture file is invalid. A valid capture file has at most one user data section and there
  // are no additional (non user data) sections located after the section list.
  virtual ErrorMessageOr<uint64_t> AddAdditionalSectionOfType(uint64_t new_section_type,
                                                              size_t new_section_size) = 0;
};

}  // namespace orbit_capture_file
#endif  // CAPTURE_FILE_CAPTURE_FILE_H_
