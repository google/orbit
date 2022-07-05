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
#include "Introspection/Introspection.h"
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
  [[nodiscard]] ErrorMessageOr<void> Initialize();

  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() override;
  [[nodiscard]] bool HasDebugSymbols() const override;
  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;
  [[nodiscard]] uint64_t GetLoadBias() const override;
  [[nodiscard]] std::string GetBuildId() const override;
  [[nodiscard]] uint64_t GetExecutableSegmentOffset() const override;
  [[nodiscard]] uint64_t GetExecutableSegmentSize() const override;
  [[nodiscard]] uint64_t GetImageSize() const override;
  [[nodiscard]] bool IsElf() const override;
  [[nodiscard]] bool IsCoff() const override;
  [[nodiscard]] ErrorMessageOr<PdbDebugInfo> GetDebugPdbInfo() const override;

 private:
  [[nodiscard]] std::optional<uint64_t> GetVirtualAddressOfSymbolSection(
      const llvm::object::SymbolRef& symbol_ref);
  [[nodiscard]] std::optional<SymbolInfo> CreateSymbolInfo(
      const llvm::object::SymbolRef& symbol_ref);
  void AddNewDebugSymbolsFromCoffSymbolTable(
      const llvm::object::ObjectFile::symbol_iterator_range& symbol_range,
      std::vector<SymbolInfo>* symbol_infos);
  [[nodiscard]] bool AreDebugSymbolsEmpty() const;

  const std::filesystem::path file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::COFFObjectFile* object_file_;
  bool has_debug_info_;
  uint64_t text_section_virtual_size_ = 0;
};

CoffFileImpl::CoffFileImpl(std::filesystem::path file_path,
                           llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(std::move(file_path)),
      owning_binary_(std::move(owning_binary)),
      has_debug_info_(false) {
  object_file_ = llvm::dyn_cast<llvm::object::COFFObjectFile>(owning_binary_.getBinary());
  const auto dwarf_context = llvm::DWARFContext::create(*owning_binary_.getBinary());
  if (object_file_->getSymbolTable() != 0 || dwarf_context != nullptr) {
    has_debug_info_ = true;
  }
}

ErrorMessageOr<void> CoffFileImpl::Initialize() {
  // We assume that there is only one .text section in a PE/COFF file.
  for (const llvm::object::SectionRef& section_ref : object_file_->sections()) {
    const llvm::object::coff_section* coff_section = object_file_->getCOFFSection(section_ref);
    // section_ref.isText() would also do, but it only checks IMAGE_SCN_CNT_CODE.
    if ((coff_section->Characteristics & llvm::COFF::IMAGE_SCN_CNT_CODE) == 0) continue;
    if ((coff_section->Characteristics & llvm::COFF::IMAGE_SCN_MEM_EXECUTE) == 0) continue;
    text_section_virtual_size_ = coff_section->VirtualSize;
    return outcome::success();
  }

  std::string error =
      absl::StrFormat("No text section found in COFF file \"%s\".", file_path_.string());
  ORBIT_ERROR("%s", error);
  return ErrorMessage(std::move(error));
}

std::optional<uint64_t> CoffFileImpl::GetVirtualAddressOfSymbolSection(
    const llvm::object::SymbolRef& symbol_ref) {
  // Symbols in the COFF symbol table contain the offset relative to the start of the section. To
  // compute the symbol's address relative to the start of the object file (RVA), we need to find
  // the virtual address of the section.
  llvm::object::COFFSymbolRef coff_symbol_ref = object_file_->getCOFFSymbol(symbol_ref);
  int32_t section_index = coff_symbol_ref.getSectionNumber();
  llvm::Expected<const llvm::object::coff_section*> section =
      object_file_->getSection(section_index);
  return section ? std::make_optional(section.get()->VirtualAddress) : std::nullopt;
}

constexpr uint64_t kUnknownSymbolSize = std::numeric_limits<uint64_t>::max();

std::optional<SymbolInfo> CoffFileImpl::CreateSymbolInfo(
    const llvm::object::SymbolRef& symbol_ref) {
  llvm::Expected<llvm::object::SymbolRef::Type> type = symbol_ref.getType();
  ORBIT_CHECK(type);
  ORBIT_CHECK(type.get() == llvm::object::SymbolRef::ST_Function);

  llvm::Expected<uint32_t> flags = symbol_ref.getFlags();
  if (!flags || (flags.get() & llvm::object::BasicSymbolRef::SF_Undefined) != 0) {
    return std::nullopt;
  }

  llvm::Expected<llvm::StringRef> name = symbol_ref.getName();
  if (!name) {
    return std::nullopt;
  }

  // The symbol's "Value" is the offset in the section.
  llvm::Expected<uint64_t> value = symbol_ref.getValue();
  if (!value) {
    return std::nullopt;
  }
  std::optional<uint64_t> section_offset = GetVirtualAddressOfSymbolSection(symbol_ref);
  if (!section_offset.has_value()) {
    return std::nullopt;
  }
  const uint64_t symbol_virtual_address = GetLoadBias() + section_offset.value() + value.get();

  SymbolInfo symbol_info;
  symbol_info.set_demangled_name(llvm::demangle(name.get().str()));
  symbol_info.set_address(symbol_virtual_address);

  // The COFF symbol table doesn't contain the size of symbols. Set a placeholder for now.
  symbol_info.set_size(kUnknownSymbolSize);

  return symbol_info;
}

// Comparator to sort SymbolInfos by address, and perform the corresponding binary searches.
bool SymbolInfoLessByAddress(const SymbolInfo& lhs, const SymbolInfo& rhs) {
  return lhs.address() < rhs.address();
}

void CoffFileImpl::AddNewDebugSymbolsFromCoffSymbolTable(
    const llvm::object::ObjectFile::symbol_iterator_range& symbol_range,
    std::vector<SymbolInfo>* symbol_infos) {
  // Sort so that we can use std::lower_bound below.
  std::sort(symbol_infos->begin(), symbol_infos->end(), &SymbolInfoLessByAddress);

  std::vector<SymbolInfo> new_symbol_infos;

  for (const auto& symbol_ref : symbol_range) {
    llvm::Expected<llvm::object::SymbolRef::Type> type = symbol_ref.getType();
    if (!type || type.get() != llvm::object::SymbolRef::ST_Function) {
      // The symbol is not a function (or has unknown type). We skip it as we only list symbols of
      // functions, ignoring sections and variables.
      continue;
    }

    auto symbol_or_error = CreateSymbolInfo(symbol_ref);
    if (!symbol_or_error.has_value()) {
      continue;
    }
    SymbolInfo& symbol_info = symbol_or_error.value();

    auto symbol_from_dwarf_it = std::lower_bound(symbol_infos->begin(), symbol_infos->end(),
                                                 symbol_info, &SymbolInfoLessByAddress);
    if (symbol_from_dwarf_it != symbol_infos->end() &&
        symbol_info.address() == symbol_from_dwarf_it->address()) {
      // A symbol with this exact address was already extracted from the DWARF debug info.
      continue;
    }

    if (symbol_from_dwarf_it != symbol_infos->begin()) {
      symbol_from_dwarf_it--;
      if (symbol_info.address() < symbol_from_dwarf_it->address() + symbol_from_dwarf_it->size()) {
        // The address of this symbol from the COFF symbol table is inside the address range of a
        // symbol already extracted from the DWARF debug info.
        continue;
      }
    }

    new_symbol_infos.emplace_back(std::move(symbol_info));
  }

  ORBIT_LOG(
      "Added %u function symbols from COFF symbol table on top of %u from DWARF debug info for "
      "\"%s\"",
      new_symbol_infos.size(), symbol_infos->size(), file_path_.string());
  symbol_infos->insert(symbol_infos->end(), std::make_move_iterator(new_symbol_infos.begin()),
                       std::make_move_iterator(new_symbol_infos.end()));
}

void FillDebugSymbolsFromDwarf(llvm::DWARFContext* dwarf_context,
                               std::vector<SymbolInfo>* symbol_infos) {
  for (const auto& info_section : dwarf_context->compile_units()) {
    for (uint32_t index = 0; index < info_section->getNumDIEs(); ++index) {
      llvm::DWARFDie full_die = info_section->getDIEAtIndex(index);
      // We only want symbols of functions, which are DIEs with isSubprogramDIR(),
      // and not of inlined functions, which are DIEs with isSubroutineDIE().
      if (full_die.isSubprogramDIE()) {
        uint64_t low_pc;
        uint64_t high_pc;
        uint64_t unused_section_index;
        if (!full_die.getLowAndHighPC(low_pc, high_pc, unused_section_index)) {
          continue;
        }

        SymbolInfo& symbol_info = symbol_infos->emplace_back();
        // The method getName will fall back to ShortName if LinkageName is not present,
        // so this should never return an empty name.
        std::string name(full_die.getName(llvm::DINameKind::LinkageName));
        ORBIT_CHECK(!name.empty());
        symbol_info.set_demangled_name(llvm::demangle(name));
        symbol_info.set_address(low_pc);
        symbol_info.set_size(high_pc - low_pc);
      }
    }
  }
}

void DeduceDebugSymbolMissingSizes(std::vector<SymbolInfo>* symbol_infos) {
  // We don't have sizes for functions obtained from the COFF symbol table. For these, compute the
  // size as the distance from the address of the next function.
  std::sort(symbol_infos->begin(), symbol_infos->end(), &SymbolInfoLessByAddress);

  for (size_t i = 0; i < symbol_infos->size(); ++i) {
    SymbolInfo& symbol_info = symbol_infos->at(i);
    if (symbol_info.size() != kUnknownSymbolSize) {
      // This function symbol was from DWARF debug info and already has a size.
      continue;
    }

    if (i < symbol_infos->size() - 1) {
      // Deduce the size as the distance from the next function's address.
      symbol_info.set_size(symbol_infos->at(i + 1).address() - symbol_info.address());
    } else {
      // If the last symbol doesn't have a size, we can't deduce it, and we just set it to zero.
      symbol_info.set_size(0);
    }
  }
}

// The COFF symbol table doesn't contain the size of functions. If present, the DWARF debug info
// contains the sizes. However, we observed that the DWARF debug info misses some functions compared
// to the COFF symbol table.
// We integrate the two: we retrieve symbols from the DWARF debug info, and then we add symbols from
// the COFF symbol table, but only the ones that were not in the DWARF debug info (based on their
// address).
// For the symbols that came from the COFF symbol table, we compute the size as the distance from
// the address of the next function. In general this will overestimate the size of function, but we
// prefer this to not listing the function at all.
// Note: In some rare cases we observed that the address reported by the COFF symbol table is
// slightly lower (e.g., by one instruction) that the one reported by the DWARF debug info. As we
// don't have a simple way to deduce that the two addresses belong to the same function, such
// function will appear twice in the symbol list. We accept this.
ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadDebugSymbols() {
  std::vector<SymbolInfo> symbol_infos;

  const std::unique_ptr<llvm::DWARFContext> dwarf_context =
      llvm::DWARFContext::create(*object_file_);
  if (dwarf_context != nullptr) {
    FillDebugSymbolsFromDwarf(dwarf_context.get(), &symbol_infos);
  }

  if (object_file_->getSymbolTable() != 0 && object_file_->getNumberOfSymbols() != 0) {
    AddNewDebugSymbolsFromCoffSymbolTable(object_file_->symbols(), &symbol_infos);
  }

  DeduceDebugSymbolMissingSizes(&symbol_infos);

  if (symbol_infos.empty()) {
    return ErrorMessage(
        "Unable to load symbols from PE/COFF file, not even a single symbol of type function "
        "found.");
  }

  ModuleSymbols module_symbols;
  for (SymbolInfo& symbol_info : symbol_infos) {
    *module_symbols.add_symbol_infos() = std::move(symbol_info);
  }
  return module_symbols;
}

bool CoffFileImpl::HasDebugSymbols() const { return has_debug_info_ && !AreDebugSymbolsEmpty(); }

bool CoffFileImpl::AreDebugSymbolsEmpty() const {
  if (object_file_->getSymbolTable() != 0 && object_file_->getNumberOfSymbols() != 0) {
    return false;
  }

  const std::unique_ptr<llvm::DWARFContext> dwarf_context =
      llvm::DWARFContext::create(*object_file_);
  if (dwarf_context == nullptr) {
    ORBIT_ERROR("Could not create DWARF context for \"%s\"", file_path_.string());
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

uint64_t CoffFileImpl::GetExecutableSegmentSize() const { return text_section_virtual_size_; }

uint64_t CoffFileImpl::GetImageSize() const {
  return object_file_->getPE32PlusHeader()->SizeOfImage;
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
    ORBIT_LOG("WARNING: No PDB debug info found for \"%s\", cannot form build id (ignoring)",
              file_path_.filename().string());
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
  ORBIT_SCOPE_FUNCTION;
  llvm::object::COFFObjectFile* coff_object_file =
      llvm::dyn_cast<llvm::object::COFFObjectFile>(file.getBinary());
  if (coff_object_file == nullptr) {
    return ErrorMessage(absl::StrFormat("Unable to load object file \"%s\":", file_path.string()));
  }
  if (!coff_object_file->is64()) {
    return ErrorMessage(absl::StrFormat("Only 64-bit object files are supported."));
  }

  auto coff_file = std::make_unique<CoffFileImpl>(file_path, std::move(file));
  OUTCOME_TRY(coff_file->Initialize());
  return coff_file;
}

}  // namespace orbit_object_utils
