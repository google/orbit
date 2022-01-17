// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitSsh/SftpFile.h"

#include <libssh2.h>

#include <utility>

#include "LibSsh2Utils.h"
#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"
#include "OrbitSsh/Sftp.h"

namespace orbit_ssh {
outcome::result<SftpFile> SftpFile::Open(Session* session, Sftp* sftp, std::string_view filepath,
                                         FxfFlags flags, int64_t mode) {
  auto* const result =
      libssh2_sftp_open_ex(sftp->GetRawSftpPtr(), filepath.data(), filepath.size(),
                           static_cast<uint64_t>(flags), mode, LIBSSH2_SFTP_OPENFILE);

  if (result == nullptr) {
    const auto [last_errno, error_message] = LibSsh2SessionLastError(session->GetRawSessionPtr());

    if (last_errno != LIBSSH2_ERROR_EAGAIN) {
      ORBIT_ERROR("Unable to open sftp file \"%s\": %s", filepath, error_message);
    }

    return static_cast<Error>(last_errno);
  }

  return outcome::success(SftpFile{result, session, filepath});
}

outcome::result<std::string> SftpFile::Read(size_t max_length_in_bytes) {
  std::string buffer(max_length_in_bytes, '\0');
  const auto result = libssh2_sftp_read(file_ptr_.get(), buffer.data(), buffer.size());

  if (result < 0) {
    if (result != LIBSSH2_ERROR_EAGAIN) {
      ORBIT_ERROR("Unable to read from sftp file \"%s\": %s", filepath_,
                  LibSsh2SessionLastError(session_->GetRawSessionPtr()).second);
    }
    return static_cast<Error>(result);
  }

  buffer.resize(result);
  return buffer;
}

outcome::result<void> SftpFile::Close() {
  const auto result = libssh2_sftp_close_handle(file_ptr_.get());

  if (result < 0) {
    if (result != LIBSSH2_ERROR_EAGAIN) {
      ORBIT_ERROR("Unable to close sftp file \"%s\": %s", filepath_,
                  LibSsh2SessionLastError(session_->GetRawSessionPtr()).second);
    }
    return static_cast<Error>(result);
  }

  // file_ptr_'s deleter is libssh2_sftp_close_handle, since it was already
  // called user release to avoid calling it again.
  (void)file_ptr_.release();
  return outcome::success();
}

outcome::result<size_t> SftpFile::Write(std::string_view data) {
  const auto result = libssh2_sftp_write(file_ptr_.get(), data.data(), data.size());

  if (result < 0) {
    if (result != LIBSSH2_ERROR_EAGAIN) {
      ORBIT_ERROR("Unable to write to sftp file \"%s\": %s", filepath_,
                  LibSsh2SessionLastError(session_->GetRawSessionPtr()).second);
    }
    return static_cast<Error>(result);
  }

  return outcome::success(result);
}

}  // namespace orbit_ssh
