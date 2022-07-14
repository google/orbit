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

#include <memory>

#include "PeCoffEpilog.h"
#include "PeCoffRuntimeFunctions.h"
#include "PeCoffUnwindInfoEvaluator.h"
#include "PeCoffUnwindInfos.h"
#include "unwindstack/PeCoffInterface.h"
#include "unwindstack/PeCoffNativeUnwinder.h"

namespace unwindstack {

// Unwinding for native PE/COFF frames on x86_64. The unwinding procedure is specific to the
// x86_64 architecture: The unwind info used here (UNWIND_INFO and RUNTIME_FUNCTION) is only
// defined for 64-bit binaries and parts of the procedure directly look and emulate machine
// code (epilog detection).
class PeCoffUnwindInfoUnwinderX86_64 : public PeCoffNativeUnwinder {
 public:
  explicit PeCoffUnwindInfoUnwinderX86_64(Memory* object_file_memory, int64_t image_base,
                                          uint64_t pdata_begin, uint64_t pdata_end,
                                          const std::vector<Section>& sections)
      : runtime_functions_(CreatePeCoffRuntimeFunctions(object_file_memory)),
        unwind_infos_(CreatePeCoffUnwindInfos(object_file_memory, sections)),
        unwind_info_evaluator_(CreatePeCoffUnwindInfoEvaluator()),
        epilog_(CreatePeCoffEpilog(object_file_memory, sections)),
        image_base_(image_base),
        pdata_begin_(pdata_begin),
        pdata_end_(pdata_end) {}

  bool Init() override {
    if (!epilog_->Init()) {
      return false;
    }
    return runtime_functions_->Init(pdata_begin_, pdata_end_);
  }

  bool Step(uint64_t pc, uint64_t pc_adjustment, Regs* regs, Memory* process_memory, bool* finished,
            bool* is_signal_frame) override;

 protected:
  std::unique_ptr<PeCoffRuntimeFunctions> runtime_functions_;
  std::unique_ptr<PeCoffUnwindInfos> unwind_infos_;
  std::unique_ptr<PeCoffUnwindInfoEvaluator> unwind_info_evaluator_;
  std::unique_ptr<PeCoffEpilog> epilog_;

  int64_t image_base_ = 0;
  uint64_t pdata_begin_ = 0;
  uint64_t pdata_end_ = 0;
};

}  // namespace unwindstack
