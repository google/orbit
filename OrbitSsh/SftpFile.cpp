// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/SftpFile.h"

#include "OrbitBase/Logging.h"

namespace OrbitSsh {
outcome::result<SftpFile> SftpFile::Open(Session* session, Sftp* sftp,
                                         std::string_view filepath,
                                         FxfFlags flags, long mode) {
  const auto result = libssh2_sftp_open_ex(
      sftp->GetRawSftpPtr(), filepath.data(), filepath.size(),
      static_cast<unsigned long>(flags), mode, LIBSSH2_SFTP_OPENFILE);

  if (result) {
    return outcome::success(SftpFile{result});
  } else {
    return static_cast<SftpError>(
        libssh2_session_last_errno(session->GetRawSessionPtr()));
  }
}

outcome::result<std::string> SftpFile::Read(int max_length_in_bytes) {
  std::string buffer(max_length_in_bytes, '\0');
  const auto result =
      libssh2_sftp_read(file_ptr_, buffer.data(), buffer.size());

  if (result >= 0) {
    buffer.resize(result);
    return buffer;
  } else {
    return static_cast<SftpError>(result);
  }
}

outcome::result<void> SftpFile::Close() {
  const auto result = libssh2_sftp_close_handle(file_ptr_);

  if (result == 0) {
    file_ptr_ = nullptr;
    return outcome::success();
  } else {
    return static_cast<SftpError>(result);
  }
}

outcome::result<size_t> SftpFile::Write(std::string_view data) {
  const auto result = libssh2_sftp_write(file_ptr_, data.data(), data.size());

  if (result >= 0) {
    return outcome::success(result);
  } else {
    return static_cast<SftpError>(result);
  }
}

SftpFile::SftpFile(SftpFile&& other) noexcept : file_ptr_(other.file_ptr_) {
  other.file_ptr_ = nullptr;
}

SftpFile& SftpFile::operator=(SftpFile&& other) noexcept {
  file_ptr_ = other.file_ptr_;
  other.file_ptr_ = nullptr;
  return *this;
}

SftpFile::~SftpFile() {
  if (file_ptr_) {
    (void)Close();
  }
}
}  // namespace OrbitSsh
