// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CRASH_HANDLER_CRASH_HANDLER_H_
#define CRASH_HANDLER_CRASH_HANDLER_H_

#include <client/crash_report_database.h>
#include <client/crashpad_client.h>

#include <memory>
#include <string>
#include <vector>

#include "OrbitBase/CrashHandler.h"

namespace orbit_crash_handler {

class CrashHandler : public orbit_base::CrashHandler {
 public:
  explicit CrashHandler(const std::string& dump_path, const std::string& handler_path,
                        const std::string& crash_server_url,
                        const std::vector<std::string>& attachments);
  void DumpWithoutCrash() const;
  void SetUploadsEnabled(bool is_upload_enabled);

 private:
  inline static bool is_init_{false};
  crashpad::CrashpadClient crashpad_client_;
  std::unique_ptr<crashpad::CrashReportDatabase> crash_report_db_;
};

}  // namespace orbit_crash_handler

#endif  // CRASH_HANDLER_CRASH_HANDLER_H_
