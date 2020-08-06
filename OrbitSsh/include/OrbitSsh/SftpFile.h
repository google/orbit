// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_SFTP_FILE_H_
#define ORBIT_SSH_SFTP_FILE_H_

#include <OrbitSsh/Error.h>
#include <OrbitSsh/Sftp.h>

#include <memory>

namespace OrbitSsh {

enum class FxfFlags {
  kRead = LIBSSH2_FXF_READ,
  kWrite = LIBSSH2_FXF_WRITE,
  kAppend = LIBSSH2_FXF_APPEND,
  kCreate = LIBSSH2_FXF_CREAT,
  kTruncate = LIBSSH2_FXF_TRUNC,
  kExclusive = LIBSSH2_FXF_EXCL
};

constexpr inline FxfFlags operator|(FxfFlags lhs, FxfFlags rhs) {
  using T = std::underlying_type_t<FxfFlags>;
  return static_cast<FxfFlags>(static_cast<T>(lhs) | static_cast<T>(rhs));
}

class SftpFile {
 public:
  static outcome::result<SftpFile> Open(Session* session, Sftp* sftp,
                                        std::string_view filepath,
                                        FxfFlags flags, long mode);

  outcome::result<std::string> Read(size_t max_length_in_bytes);
  outcome::result<void> Close();
  outcome::result<size_t> Write(std::string_view data);

  LIBSSH2_SFTP_HANDLE* GetRawFilePtr() const noexcept {
    return file_ptr_.get();
  }

 private:
  SftpFile(LIBSSH2_SFTP_HANDLE* file_ptr, Session* session,
           std::string_view filepath)
      : file_ptr_(file_ptr, &libssh2_sftp_close_handle),
        session_(session),
        filepath_(filepath) {}

  std::unique_ptr<LIBSSH2_SFTP_HANDLE, decltype(&libssh2_sftp_close_handle)>
      file_ptr_;
  Session* session_;
  std::string filepath_;
};
}  // namespace OrbitSsh
#endif  // ORBIT_SSH_SFTP_FILE_H_
