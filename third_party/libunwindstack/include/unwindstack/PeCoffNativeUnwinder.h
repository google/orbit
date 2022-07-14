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

#pragma once

#include <unwindstack/Error.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Regs.h>

namespace unwindstack {

class PeCoffNativeUnwinder {
 public:
  virtual ~PeCoffNativeUnwinder() = default;

  virtual bool Init() = 0;

  virtual bool Step(uint64_t pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory,
                    bool* finished, bool* is_signal_frame) = 0;
  ErrorData GetLastError() const { return last_error_; }

 protected:
  ErrorData last_error_{ERROR_NONE, 0};
};

}  // namespace unwindstack
