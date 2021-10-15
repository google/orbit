/*
 * Copyright (C) 2018 The Android Open Source Project
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

#include <unwindstack/LocalUnwinder.h>

#include <vector>

extern "C" void TestlibLevel4(void* unwinder_data, void* frame_data) {
  unwindstack::LocalUnwinder* unwinder =
      reinterpret_cast<unwindstack::LocalUnwinder*>(unwinder_data);
  std::vector<unwindstack::LocalFrameData>* frame_info =
      reinterpret_cast<std::vector<unwindstack::LocalFrameData>*>(frame_data);
  unwinder->Unwind(frame_info, 256);
}

extern "C" void TestlibLevel3(void* unwinder_data, void* frame_data) {
  TestlibLevel4(unwinder_data, frame_data);
}

extern "C" void TestlibLevel2(void* unwinder_data, void* frame_data) {
  TestlibLevel3(unwinder_data, frame_data);
}

extern "C" void TestlibLevel1(void* unwinder_data, void* frame_data) {
  TestlibLevel2(unwinder_data, frame_data);
}

// The loop in this function is only guaranteed to not be optimized away by the compiler
// if optimizations are turned off. This is partially because the compiler doesn't have
// any idea about the function since it is retrieved using dlsym.
//
// In an effort to defend against the compiler:
//  1. The loop iteration variable is volatile.
//  2. A call to this function should be wrapped in TestUtils::DoNotOptimize().
extern "C" int BusyWait() {
  for (volatile size_t i = 0; i < 1000000; ++i)
    ;
  return 0;
}
