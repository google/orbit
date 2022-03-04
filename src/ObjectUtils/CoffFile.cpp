// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/CoffFile.h"

#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/COFF.h>
#include <llvm/Object/CVDebugRecord.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>

#include <system_error>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/WindowsBuildIdUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

namespace {

using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

class CoffFileImpl : public CoffFile {
 public:
  CoffFileImpl(std::filesystem::path file_path,
               llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary);
  [[nodiscard]] ErrorMessageOr<DebugSymbols> LoadRawDebugSymbols() override;
  [[nodiscard]] bool HasDebugSymbols() const override;
  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;
  [[nodiscard]] uint64_t GetLoadBias() const override;
  [[nodiscard]] std::string GetBuildId() const override;
  [[nodiscard]] uint64_t GetExecutableSegmentOffset() const override;
  [[nodiscard]] bool IsElf() const override;
  [[nodiscard]] bool IsCoff() const override;
  [[nodiscard]] ErrorMessageOr<PdbDebugInfo> GetDebugPdbInfo() const override;

 private:
  ErrorMessageOr<uint64_t> GetSectionOffsetForSymbol(const llvm::object::SymbolRef& symbol_ref);
  ErrorMessageOr<SymbolInfo> CreateSymbolInfo(const llvm::object::SymbolRef& symbol_ref);
  [[nodiscard]] bool AreDebugSymbolsEmpty() const;
  const std::filesystem::path file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::COFFObjectFile* object_file_;
  bool has_debug_info_;
};

CoffFileImpl::CoffFileImpl(std::filesystem::path file_path,
                           llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(std::move(file_path)),
      owning_binary_(std::move(owning_binary)),
      has_debug_info_(false) {
  object_file_ = llvm::dyn_cast<llvm::object::COFFObjectFile>(owning_binary_.getBinary());
  const auto dwarf_context = llvm::DWARFContext::create(*owning_binary_.getBinary());
  if (dwarf_context != nullptr) {
    has_debug_info_ = true;
  }
}

static void FillDebugSymbolsFromDWARF(llvm::DWARFContext* dwarf_context,
                                      DebugSymbols* debug_symbols) {
  for (const auto& info_section : dwarf_context->compile_units()) {
    for (uint32_t index = 0; index < info_section->getNumDIEs(); ++index) {
      llvm::DWARFDie full_die = info_section->getDIEAtIndex(index);
      // We only want symbols of functions, which are DIEs with isSubprogramDIR()
      // and not of inlined functions, which are DIEs with isSubroutineDIE().
      if (full_die.isSubprogramDIE()) {
        uint64_t low_pc;
        uint64_t high_pc;
        uint64_t unused_section_index;
        if (!full_die.getLowAndHighPC(low_pc, high_pc, unused_section_index)) {
          continue;
        }
        // The method getName will fallback to ShortName if LinkageName is
        // not present, so this should never return an empty name.
        std::string name(full_die.getName(llvm::DINameKind::LinkageName));
        ORBIT_CHECK(!name.empty());
        FunctionSymbol& function_symbol = debug_symbols->function_symbols.emplace_back();
        function_symbol.mangled_name = name;
        function_symbol.address = low_pc;
        function_symbol.size = high_pc - low_pc;
      }
    }
  }
}

ErrorMessageOr<DebugSymbols> CoffFileImpl::LoadRawDebugSymbols() {
  const auto dwarf_context = llvm::DWARFContext::create(*owning_binary_.getBinary());
  if (dwarf_context == nullptr) {
    return ErrorMessage("Could not create DWARF context.");
  }

  DebugSymbols debug_symbols;
  debug_symbols.load_bias = GetLoadBias();
  debug_symbols.symbols_file_path = file_path_.string();

  FillDebugSymbolsFromDWARF(dwarf_context.get(), &debug_symbols);

  if (debug_symbols.function_symbols.empty()) {
    return ErrorMessage(
        "Unable to load symbols from object file, not even a single symbol of "
        "type function found.");
  }

  DemangleSymbols(debug_symbols.function_symbols);
  return debug_symbols;
}

bool CoffFileImpl::HasDebugSymbols() const { return has_debug_info_ && !AreDebugSymbolsEmpty(); }

bool CoffFileImpl::AreDebugSymbolsEmpty() const {
  const auto dwarf_context = llvm::DWARFContext::create(*owning_binary_.getBinary());
  if (dwarf_context == nullptr) {
    ORBIT_ERROR("Could not create DWARF context.");
    return true;
  }

  for (const auto& info_section : dwarf_context->compile_units()) {
    for (uint32_t index = 0; index < info_section->getNumDIEs(); ++index) {
      llvm::DWARFDie full_die = info_section->getDIEAtIndex(index);
      if (!full_die.isSubprogramDIE()) continue;

      uint64_t low_pc;
      uint64_t high_pc;
      uint64_t unused_section_index;
      if (full_die.getLowAndHighPC(low_pc, high_pc, unused_section_index)) {
        return false;
      }
    }
  }
  return true;
}

const std::filesystem::path& CoffFileImpl::GetFilePath() const { return file_path_; }

std::string CoffFileImpl::GetName() const { return file_path_.filename().string(); }

uint64_t CoffFileImpl::GetLoadBias() const { return object_file_->getImageBase(); }
uint64_t CoffFileImpl::GetExecutableSegmentOffset() const {
  ORBIT_CHECK(object_file_->is64());
  return object_file_->getPE32PlusHeader()->BaseOfCode;
}

bool CoffFileImpl::IsElf() const { return false; }
bool CoffFileImpl::IsCoff() const { return true; }

ErrorMessageOr<PdbDebugInfo> CoffFileImpl::GetDebugPdbInfo() const {
  const llvm::codeview::DebugInfo* debug_info = nullptr;
  llvm::StringRef pdb_file_path;

  // If 'this' was successfully created with CreateCoffFile, 'object_file_' cannot be nullptr.
  ORBIT_CHECK(object_file_ != nullptr);

  llvm::Error error = object_file_->getDebugPDBInfo(debug_info, pdb_file_path);
  if (error) {
    return ErrorMessage(absl::StrFormat("Unable to load debug PDB info with error: %s",
                                        llvm::toString(std::move(error))));
  }
  if (debug_info == nullptr) {
    // Per llvm documentation, this indicates that the file does not have the requested info.
    return ErrorMessage("Object file does not have debug PDB info.");
  }
  // Only support PDB 70 until we learn otherwise.
  constexpr uint32_t kPdb70Signature = 0x53445352;
  ORBIT_CHECK(debug_info->PDB70.CVSignature == kPdb70Signature);

  PdbDebugInfo pdb_debug_info;
  pdb_debug_info.pdb_file_path = pdb_file_path.str();
  std::copy(std::begin(debug_info->PDB70.Signature), std::end(debug_info->PDB70.Signature),
            std::begin(pdb_debug_info.guid));
  pdb_debug_info.age = debug_info->PDB70.Age;
  return pdb_debug_info;
}

std::string CoffFileImpl::GetBuildId() const {
  ErrorMessageOr<PdbDebugInfo> pdb_debug_info = GetDebugPdbInfo();
  if (pdb_debug_info.has_error()) {
    ORBIT_LOG("WARNING: No PDB debug info found, cannot form build id (ignoring).");
    return "";
  }
  return ComputeWindowsBuildId(pdb_debug_info.value().guid, pdb_debug_info.value().age);
}

}  // namespace

ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(const std::filesystem::path& file_path) {
  llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>> object_file_or_error =
      llvm::object::ObjectFile::createObjectFile(file_path.string());

  if (!object_file_or_error) {
    return ErrorMessage(absl::StrFormat("Unable to load COFF file \"%s\": %s", file_path.string(),
                                        llvm::toString(object_file_or_error.takeError())));
  }

  llvm::object::OwningBinary<llvm::object::ObjectFile>& file = object_file_or_error.get();

  return CreateCoffFile(file_path, std::move(file));
}

ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(
    const std::filesystem::path& file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& file) {
  llvm::object::COFFObjectFile* coff_object_file =
      llvm::dyn_cast<llvm::object::COFFObjectFile>(file.getBinary());
  if (coff_object_file != nullptr) {
    if (!coff_object_file->is64()) {
      return ErrorMessage(absl::StrFormat("Only 64-bit object files are supported."));
    }
    return std::unique_ptr<CoffFile>(new CoffFileImpl(file_path, std::move(file)));
  } else {
    return ErrorMessage(absl::StrFormat("Unable to load object file \"%s\":", file_path.string()));
  }
}

}  // namespace orbit_object_utils
