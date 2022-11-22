// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CODE_REPORT_ANNOTATE_DISASSEMBLY_H_
#define CODE_REPORT_ANNOTATE_DISASSEMBLY_H_

#include <string>
#include <string_view>
#include <vector>

#include "ClientData/FunctionInfo.h"
#include "CodeReport/AnnotatingLine.h"
#include "CodeReport/DisassemblyReport.h"
#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ElfFile.h"

namespace orbit_code_report {

// Matches source code lines to machine instructions. The mapping is determined from debug
// information (ElfFile). The output reference line numbers in the `DisassemblyReport` and is
// ordered by those.
[[nodiscard]] std::vector<AnnotatingLine> AnnotateDisassemblyWithSourceCode(
    const orbit_client_data::FunctionInfo& function_info,
    const orbit_grpc_protos::LineInfo& location_info, std::string_view source_file_contents,
    orbit_object_utils::ElfFile* elf, const DisassemblyReport& report);

}  // namespace orbit_code_report

#endif  // CODE_REPORT_ANNOTATE_DISASSEMBLY_H_
