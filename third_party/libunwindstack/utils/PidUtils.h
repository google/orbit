/*
 * Copyright (C) 2022 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef _LIBUNWINDSTACK_UTILS_PID_UTILS_H
#define _LIBUNWINDSTACK_UTILS_PID_UTILS_H

#include <stdint.h>
#include <sys/types.h>

#include <functional>

namespace unwindstack {

enum PidRunEnum : uint8_t {
  PID_RUN_KEEP_GOING,
  PID_RUN_PASS,
  PID_RUN_FAIL,
};

bool Quiesce(pid_t pid);

bool Attach(pid_t pid);

bool Detach(pid_t pid);

bool RunWhenQuiesced(pid_t pid, bool leave_attached, std::function<PidRunEnum()> fn);

}  // namespace unwindstack

#endif  // _LIBUNWINDSTACK_UTILS_PID_UTILS_H
