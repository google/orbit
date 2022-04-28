#
# Copyright (C) 2021 The Android Open Source Project
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# This is a (hopefully temporary) fork of libunwindstack
# (https://android.googlesource.com/platform/system/unwinding/+/refs/heads/master/libunwindstack/)
# with additional support for Windows and mixed callstack unwinding.
# It is maintained by the Orbit profiler team in
# https://github.com/google/orbit/tree/main/third_party/libunwindstack

package_group(
    name = "users",
    packages = [
        "//...",
    ],
)

package(default_visibility = ["//third_party/libunwindstack:users"])

licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "libunwindstack",
    srcs = [
        "ArmExidx.cpp",
        "ArmExidx.h",
        "AsmGetRegsX86_64.S",
        "Check.h",
        "DexFiles.cpp",
        "DwarfCfa.cpp",
        "DwarfCfa.h",
        "DwarfDebugFrame.h",
        "DwarfEhFrame.h",
        "DwarfEhFrameWithHdr.cpp",
        "DwarfEhFrameWithHdr.h",
        "DwarfEncoding.h",
        "DwarfMemory.cpp",
        "DwarfOp.cpp",
        "DwarfOp.h",
        "DwarfSection.cpp",
        "Elf.cpp",
        "ElfInterface.cpp",
        "ElfInterfaceArm.cpp",
        "ElfInterfaceArm.h",
        "Global.cpp",
        "GlobalDebugImpl.h",
        "JitDebug.cpp",
        "LogStdout.cpp",
        "MapInfo.cpp",
        "Maps.cpp",
        "Memory.cpp",
        "MemoryBuffer.h",
        "MemoryCache.h",
        "MemoryFileAtOffset.h",
        "MemoryLocal.h",
        "MemoryMte.cpp",
        "MemoryOffline.h",
        "MemoryOfflineBuffer.h",
        "MemoryRange.h",
        "MemoryRemote.h",
        "MemoryXz.cpp",
        "MemoryXz.h",
        "Object.cpp",
        "PeCoff.cpp",
        "PeCoffEpilog.cpp",
        "PeCoffEpilog.h",
        "PeCoffInterface.cpp",
        "PeCoffRuntimeFunctions.cpp",
        "PeCoffRuntimeFunctions.h",
        "PeCoffUnwindInfoEvaluator.cpp",
        "PeCoffUnwindInfoEvaluator.h",
        "PeCoffUnwindInfoUnwinderX86_64.cpp",
        "PeCoffUnwindInfoUnwinderX86_64.h",
        "PeCoffUnwindInfos.cpp",
        "PeCoffUnwindInfos.h",
        "Regs.cpp",
        "RegsArm.cpp",
        "RegsArm64.cpp",
        "RegsInfo.h",
        "RegsMips.cpp",
        "RegsMips64.cpp",
        "RegsX86.cpp",
        "RegsX86_64.cpp",
        "Symbols.cpp",
        "ThreadEntry.cpp",
        "ThreadEntry.h",
        "Unwinder.cpp",
    ],
    hdrs = glob(
        [
            "include/unwindstack/*.h",
            "include/*.h",
        ],
        exclude = [
            # Dex-support is Android-specific
            "include/unwindstack/DexFile.h",
            # These headers aren't self-contained
            "include/unwindstack/Arch.h",
            "include/unwindstack/DwarfMemory.h",
            "include/unwindstack/RegsGetLocal.h",
            "include/unwindstack/User*.h",
        ],
    ),
    copts = [
        "-Wno-c99-designator",
        "-Wno-ctad-maybe-unsupported",
        "-Wno-logical-op-parentheses",
    ],
    includes = [
        "include/",
    ],
    textual_hdrs = glob([
        # These headers aren't self-contained
        "include/unwindstack/User*.h",
    ]) + [
        "Symbols.h",
        "include/unwindstack/Arch.h",
        "include/unwindstack/DwarfMemory.h",
        "include/unwindstack/RegsGetLocal.h",
    ],
    deps = [
        "@android_platform_system_libbase//:libbase",
        "@android_platform_system_libprocinfo//:libprocinfo",
        "@com_7-zip_org_sdk//lzma",
        "@com_github_capstone-engine_capstone//capstone",
    ],
)
