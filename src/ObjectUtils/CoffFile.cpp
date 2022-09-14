// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/CoffFile.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/COFF.h>
#include <llvm/Object/CVDebugRecord.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/Win64EH.h>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "Introspection/Introspection.h"
#include "ObjectUtils/WindowsBuildIdUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

namespace {

using orbit_grpc_protos::ModuleInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

class CoffFileImpl : public CoffFile {
 public:
  CoffFileImpl(std::filesystem::path file_path,
               llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary);

  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadDebugSymbols() override;
  [[nodiscard]] bool HasDebugSymbols() const override;
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadSymbolsFromExportTable() override;
  [[nodiscard]] bool HasExportTable() const override;
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadExceptionTableEntriesAsSymbols() override;
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols()
      override;

  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;
  [[nodiscard]] uint64_t GetLoadBias() const override;
  [[nodiscard]] std::string GetBuildId() const override;
  [[nodiscard]] uint64_t GetExecutableSegmentOffset() const override;
  [[nodiscard]] uint64_t GetImageSize() const override;
  [[nodiscard]] const std::vector<ModuleInfo::ObjectSegment>& GetObjectSegments() const override;
  [[nodiscard]] bool IsElf() const override;
  [[nodiscard]] bool IsCoff() const override;
  [[nodiscard]] ErrorMessageOr<PdbDebugInfo> GetDebugPdbInfo() const override;

 private:
  [[nodiscard]] std::optional<uint64_t> GetVirtualAddressOfSymbolSection(
      const llvm::object::SymbolRef& symbol_ref);
  [[nodiscard]] std::optional<SymbolInfo> CreateSymbolInfo(
      const llvm::object::SymbolRef& symbol_ref);
  [[nodiscard]] std::vector<SymbolInfo> LoadDebugSymbolsFromCoffSymbolTable(
      const llvm::object::ObjectFile::symbol_iterator_range& symbol_range);
  void AddNewDebugSymbolsFromDwarf(llvm::DWARFContext* dwarf_context,
                                   std::vector<SymbolInfo>* symbol_infos);

  [[nodiscard]] ErrorMessageOr<llvm::ArrayRef<llvm::Win64EH::RuntimeFunction>>
  GetRuntimeFunctions();
  [[nodiscard]] ErrorMessageOr<const llvm::Win64EH::RuntimeFunction*> GetPrimaryRuntimeFunction(
      const llvm::Win64EH::RuntimeFunction* runtime_function);
  struct UnwindRange {
    uint64_t start;
    uint64_t end;
  };
  [[nodiscard]] ErrorMessageOr<std::vector<UnwindRange>> GetUnwindRanges();

  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadSymbolsFromExportTableInternal(
      const ErrorMessageOr<std::vector<UnwindRange>>& unwind_ranges_or_error);
  [[nodiscard]] static ErrorMessageOr<ModuleSymbols> LoadExceptionTableEntriesAsSymbolsInternal(
      const ErrorMessageOr<std::vector<UnwindRange>>& unwind_ranges_or_error);

  const std::filesystem::path file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::COFFObjectFile* object_file_;
  std::vector<ModuleInfo::ObjectSegment> sections_;
};

CoffFileImpl::CoffFileImpl(std::filesystem::path file_path,
                           llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(std::move(file_path)), owning_binary_(std::move(owning_binary)) {
  object_file_ = llvm::dyn_cast<llvm::object::COFFObjectFile>(owning_binary_.getBinary());

  for (const llvm::object::SectionRef& section_ref : object_file_->sections()) {
    const llvm::object::coff_section* coff_section = object_file_->getCOFFSection(section_ref);
    ModuleInfo::ObjectSegment& object_segment = sections_.emplace_back();
    object_segment.set_offset_in_file(coff_section->PointerToRawData);
    object_segment.set_size_in_file(coff_section->SizeOfRawData);
    object_segment.set_address(object_file_->getImageBase() + coff_section->VirtualAddress);
    object_segment.set_size_in_memory(coff_section->VirtualSize);
  }
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

  // The COFF symbol table doesn't contain the size of symbols. Set a placeholder which indicates
  // that the size is unknown for now and try to deduce it later. We will later use that placeholder
  // in DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol.
  symbol_info.set_size(kUnknownSymbolSize);

  return symbol_info;
}

std::vector<SymbolInfo> CoffFileImpl::LoadDebugSymbolsFromCoffSymbolTable(
    const llvm::object::ObjectFile::symbol_iterator_range& symbol_range) {
  std::vector<SymbolInfo> symbol_infos;
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
    symbol_infos.emplace_back(std::move(symbol_or_error.value()));
  }
  return symbol_infos;
}

void DeduceDebugSymbolMissingSizesFromUnwindInfo(
    std::vector<SymbolInfo>* symbol_infos,
    const absl::flat_hash_map<uint64_t, uint64_t>& unwind_range_start_to_size) {
  for (SymbolInfo& symbol_info : *symbol_infos) {
    ORBIT_CHECK(symbol_info.size() == SymbolsFile::kUnknownSymbolSize);
    auto unwind_range_start_to_size_it = unwind_range_start_to_size.find(symbol_info.address());
    if (unwind_range_start_to_size_it != unwind_range_start_to_size.end()) {
      symbol_info.set_size(unwind_range_start_to_size_it->second);
    }
  }
}

void CoffFileImpl::AddNewDebugSymbolsFromDwarf(llvm::DWARFContext* dwarf_context,
                                               std::vector<SymbolInfo>* symbol_infos) {
  // Sort so that we can use std::lower_bound below.
  std::sort(symbol_infos->begin(), symbol_infos->end(), &SymbolsFile::SymbolInfoLessByAddress);

  std::vector<SymbolInfo> new_symbol_infos;
  for (const auto& compile_unit : dwarf_context->compile_units()) {
    for (uint32_t die_index = 0; die_index < compile_unit->getNumDIEs(); ++die_index) {
      llvm::DWARFDie full_die = compile_unit->getDIEAtIndex(die_index);
      // We only want symbols of functions, which are DIEs with isSubprogramDIR(),
      // and not of inlined functions, which are DIEs with isSubroutineDIE().
      if (!full_die.isSubprogramDIE()) {
        continue;
      }

      uint64_t low_pc{};
      uint64_t high_pc{};
      uint64_t unused_section_index{};
      // For some functions in some Wine DLLs (such as d3d9.dll, d3d11.dll, dxgi.dll) we get a zero
      // address even if getLowAndHighPC succeeds: skip such functions.
      // We don't know why this happens, but we observed that:
      // - They are from the `std`, `__gnu_cxx`, or `dxvk` namespaces;
      // - Size is zero in the majority of cases, but not always;
      // - Some of these appear with zero address multiple times.
      // Additionally, all these functions appear again once with a non-zero address and size, so in
      // the end they end up listed as expected.
      if (!full_die.getLowAndHighPC(low_pc, high_pc, unused_section_index) || low_pc == 0) {
        continue;
      }

      auto symbol_from_symbol_table_it =
          std::lower_bound(symbol_infos->begin(), symbol_infos->end(), low_pc,
                           [](const orbit_grpc_protos::SymbolInfo& lhs, uint64_t rhs) {
                             return lhs.address() < rhs;
                           });

      if (symbol_from_symbol_table_it != symbol_infos->end() &&
          low_pc == symbol_from_symbol_table_it->address()) {
        // A symbol with this exact address is already in the COFF symbol table.
        if (symbol_from_symbol_table_it->size() == kUnknownSymbolSize) {
          // Note that we never observed a single case where setting the size of the function from
          // the DWARF information is relevant: if a function appears in the DWARF information,
          // either it also appears in the COFF symbol table *and* has a RUNTIME_FUNCTION, or
          // (rarely) it doesn't appear in the COFF symbol table at all. Nonetheless, it makes sense
          // to consider this case.
          symbol_from_symbol_table_it->set_size(high_pc - low_pc);
        }
        continue;
      }

      if (symbol_from_symbol_table_it != symbol_infos->end() &&
          symbol_from_symbol_table_it->address() < high_pc) {
        // A symbol from the COFF symbol table already has its address in this address range. Give
        // complete precedence to the COFF symbol table and skip this address range.
        // Note that we have not observed this case.
        continue;
      }

      if (symbol_from_symbol_table_it != symbol_infos->begin()) {
        symbol_from_symbol_table_it--;
        if (symbol_from_symbol_table_it->size() != kUnknownSymbolSize &&
            low_pc < symbol_from_symbol_table_it->address() + symbol_from_symbol_table_it->size()) {
          // This address is inside the address range of a symbol already extracted from the COFF
          // symbol table (note that this can only be checked for functions from the COFF symbol
          // table for which we obtained the size from the corresponding RUNTIME_FUNCTION, i.e., not
          // for leaf functions).
          // We observed such cases in some Wine DLLs (e.g., combase.dll, dinput8.dll, gdi32.dll,
          // kernel32.dll, kernelbase.dll, msvcr120.dll, msvcrt.dll, ntdll.dll, ole32.dll,
          // oleaut32.dll, sechost.dll, shell32,dll, ucrtbase.dll, user32.dll, winmm.dll,
          // xinput9_1_0.dll), where the address range from the DWARF unwind information starts
          // exactly eight bytes after the address in COFF symbol table, and where those eight bytes
          // always correspond to the single instruction `lea rsp,[rsp+0x0]`, which as far as I can
          // tell is a no-op.
          continue;
        }
      }

      SymbolInfo& symbol_info = new_symbol_infos.emplace_back();
      // The method getName will fall back to ShortName if LinkageName is not present,
      // so this should never return an empty name.
      std::string name(full_die.getName(llvm::DINameKind::LinkageName));
      ORBIT_CHECK(!name.empty());
      symbol_info.set_demangled_name(llvm::demangle(name));
      symbol_info.set_address(low_pc);
      symbol_info.set_size(high_pc - low_pc);
    }
  }

  if (!new_symbol_infos.empty()) {
    ORBIT_LOG(
        "Added %u function symbols from DWARF debug info on top of %u from COFF symbol table for "
        "\"%s\"",
        new_symbol_infos.size(), symbol_infos->size(), file_path_.string());
  }
  symbol_infos->insert(symbol_infos->end(), std::make_move_iterator(new_symbol_infos.begin()),
                       std::make_move_iterator(new_symbol_infos.end()));
}

// The COFF symbol table doesn't contain the size of functions. We obtain it using the information
// from the corresponding RUNTIME_FUNCTIONs (also considering chained unwind info). Note that this
// is not possible for all functions, because leaf functions (in the Microsoft definitions,
// functions that don't allocate stack space) don't have a RUNTIME_FUNCTION.
//
// For Wine DLLs, which are built with MinGW, we also have DWARF unwind information from which we
// can extract function symbols, complete with function names and sizes. However, in general, only
// functions that are already in the COFF symbol table and that already have a RUNTIME_FUNCTION
// appear among these symbols. In addition, in some rare cases the DWARF information reports that a
// function starts eight bytes after the address in the COFF symbol table, where those eight bytes
// correspond to the single instruction `lea rsp,[rsp+0x0]`. Therefore, it appears that using DWARF
// information doesn't add anything to only using the COFF symbol table and RUNTIME_FUNCTIONs, and
// instead can provide conflicting information.
//
// Of course things are not that simple. For a couple of these DLLs (comctl32.dll, msctf.dll,
// msi.dll, ntdll.dll, user32.dll, windowscodecs.dll), a handful of functions only appear in the
// DWARF information, and not in the COFF symbol table. Therefore, we still have to integrate DWARF
// information into the list of symbols. However, we give full priority to the COFF symbol table, by
// skipping all address ranges from DWARF information that intersect an existing symbol from the
// COFF symbol table: in particular, when we have the symbols mentioned above whose address differs
// between the COFF symbol table and the DWARF information by eight bytes, this avoids listing them
// twice.
//
// Remember that at this point we still don't have the sizes of leaf functions. Now, we compute
// those as the distance from the address of the next function. In general this can overestimate the
// size, but we prefer this to not listing those functions at all.
ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadDebugSymbols() {
  std::vector<SymbolInfo> symbol_infos;
  if (object_file_->getSymbolTable() != 0) {
    symbol_infos = LoadDebugSymbolsFromCoffSymbolTable(object_file_->symbols());
  }

  ErrorMessageOr<std::vector<UnwindRange>> unwind_ranges_or_error = GetUnwindRanges();
  if (unwind_ranges_or_error.has_value()) {
    absl::flat_hash_map<uint64_t, uint64_t> unwind_range_start_to_size;
    for (const UnwindRange& unwind_range : unwind_ranges_or_error.value()) {
      unwind_range_start_to_size.emplace(unwind_range.start, unwind_range.end - unwind_range.start);
    }
    DeduceDebugSymbolMissingSizesFromUnwindInfo(&symbol_infos, unwind_range_start_to_size);
  } else {
    ORBIT_ERROR("Could not deduce sizes of symbols from COFF symbol table of \"%s\": %s",
                file_path_.string(), unwind_ranges_or_error.error().message());
  }

  if (const std::unique_ptr<llvm::DWARFContext> dwarf_context =
          llvm::DWARFContext::create(*object_file_);
      dwarf_context != nullptr) {
    AddNewDebugSymbolsFromDwarf(dwarf_context.get(), &symbol_infos);
  }

  DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol(&symbol_infos);

  if (symbol_infos.empty()) {
    return ErrorMessage(
        "Unable to load symbols from PE/COFF file: not even a single function symbol was found.");
  }

  ModuleSymbols module_symbols;
  for (SymbolInfo& symbol_info : symbol_infos) {
    *module_symbols.add_symbol_infos() = std::move(symbol_info);
  }
  return module_symbols;
}

bool CoffFileImpl::HasDebugSymbols() const {
  if (object_file_->getSymbolTable() != 0) {
    for (const auto& symbol_ref : object_file_->symbols()) {
      llvm::Expected<llvm::object::SymbolRef::Type> type = symbol_ref.getType();
      if (type && type.get() != llvm::object::SymbolRef::ST_Function) {
        return true;
      }
    }
  }

  const std::unique_ptr<llvm::DWARFContext> dwarf_context =
      llvm::DWARFContext::create(*object_file_);
  if (dwarf_context == nullptr) {
    return false;
  }
  for (const auto& compile_unit : dwarf_context->compile_units()) {
    for (uint32_t die_index = 0; die_index < compile_unit->getNumDIEs(); ++die_index) {
      llvm::DWARFDie full_die = compile_unit->getDIEAtIndex(die_index);
      if (!full_die.isSubprogramDIE()) {
        continue;
      }

      uint64_t low_pc{};
      uint64_t high_pc{};
      uint64_t unused_section_index{};
      if (full_die.getLowAndHighPC(low_pc, high_pc, unused_section_index) && low_pc != 0) {
        return true;
      }
    }
  }
  return false;
}

ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadSymbolsFromExportTable() {
  return LoadSymbolsFromExportTableInternal(GetUnwindRanges());
}

ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadSymbolsFromExportTableInternal(
    const ErrorMessageOr<std::vector<UnwindRange>>& unwind_ranges_or_error) {
  static constexpr std::string_view kErrorMessagePrefix =
      "Unable to load symbols from the Export Table: ";
  if (!HasExportTable()) {
    return ErrorMessage(
        absl::StrCat(kErrorMessagePrefix, "PE/COFF file does not have an Export Table."));
  }

  if (!unwind_ranges_or_error.has_value()) {
    return ErrorMessage{
        absl::StrFormat("%sUnable to assign sizes to symbols from the Export Table: %s",
                        kErrorMessagePrefix, unwind_ranges_or_error.error().message())};
  }
  absl::flat_hash_map<uint64_t, uint64_t> unwind_range_start_to_size;
  for (const UnwindRange& unwind_range : unwind_ranges_or_error.value()) {
    unwind_range_start_to_size.emplace(unwind_range.start, unwind_range.end - unwind_range.start);
  }

  ModuleSymbols module_symbols;
  for (const llvm::object::ExportDirectoryEntryRef& ref : object_file_->export_directories()) {
    // From https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#export-address-table:
    // "A forwarder RVA exports a definition from some other image, making it appear as if it were
    // being exported by the current image." So these symbols don't really belong to this file, and
    // we skip them.
    bool is_forwarder{};
    // Note that isForwarder takes the output parameter by reference.
    if (llvm::Error error = ref.isForwarder(is_forwarder)) {
      llvm::consumeError(std::move(error));
      continue;
    }
    if (is_forwarder) continue;

    uint32_t rva{};
    if (llvm::Error error = ref.getExportRVA(rva)) {
      llvm::consumeError(std::move(error));
      continue;
    }
    const uint64_t virtual_address = object_file_->getImageBase() + rva;

    std::string name;
    llvm::StringRef name_ref;
    // From the documentation of ExportDirectoryEntryRef::getSymbolName: "If the symbol is exported
    // only by ordinal, the empty string is set as a result." However, if the name pointer table is
    // empty, the error "Invalid data was encountered while parsing the file" is produced instead.
    // So consider both cases.
    if (llvm::Error get_symbol_name_error = ref.getSymbolName(name_ref);
        !get_symbol_name_error && !name_ref.empty()) {
      name = name_ref.str();
    } else {
      if (get_symbol_name_error) llvm::consumeError(std::move(get_symbol_name_error));
      uint32_t ordinal{};
      if (llvm::Error get_ordinal_error = ref.getOrdinal(ordinal)) {
        llvm::consumeError(std::move(get_ordinal_error));
        continue;
      }
      // We arbitrarily choose NONAME# because functions exported only by ordinal are specified in
      // the .def file with the NONAME attribute. See
      // https://docs.microsoft.com/en-us/cpp/build/exporting-functions-from-a-dll-by-ordinal-rather-than-by-name
      name = "NONAME" + std::to_string(ordinal);
    }

    SymbolInfo symbol_info;
    symbol_info.set_demangled_name(std::move(name));
    symbol_info.set_address(virtual_address);

    // The Export Table doesn't contain the size of symbols: obtain this size from the Exception
    // Table, but note that that's not always possible, as leaf functions have no RUNTIME_FUNCTION.
    auto unwind_range_start_to_size_it = unwind_range_start_to_size.find(virtual_address);
    if (unwind_range_start_to_size_it == unwind_range_start_to_size.end()) {
      symbol_info.set_size(0);
    } else {
      symbol_info.set_size(unwind_range_start_to_size_it->second);
    }

    *module_symbols.add_symbol_infos() = std::move(symbol_info);
  }

  if (module_symbols.symbol_infos_size() == 0) {
    return ErrorMessage(absl::StrCat(kErrorMessagePrefix, "not even a single symbol was found."));
  }

  return module_symbols;
}

bool CoffFileImpl::HasExportTable() const {
  return object_file_->getDataDirectory(llvm::COFF::DataDirectoryIndex::EXPORT_TABLE)
             ->RelativeVirtualAddress != 0;
}

// LLVM doesn't provide a simple way to access the RUNTIME_FUNCTIONs, so we have to do it ourselves.
// Even llvm-objdump has to do that:
// https://github.com/llvm-mirror/llvm/blob/master/tools/llvm-objdump/COFFDump.cpp
ErrorMessageOr<llvm::ArrayRef<llvm::Win64EH::RuntimeFunction>> CoffFileImpl::GetRuntimeFunctions() {
  const llvm::object::data_directory* exception_table_data_dir =
      object_file_->getDataDirectory(llvm::COFF::EXCEPTION_TABLE);
  if (exception_table_data_dir == nullptr) {
    return ErrorMessage{"Unable to read Exception Table: No corresponding Data Directory."};
  }

  llvm::ArrayRef<uint8_t> exception_table_bytes;
  if (llvm::Error error = object_file_->getRvaAndSizeAsBytes(
          exception_table_data_dir->RelativeVirtualAddress, exception_table_data_dir->Size,
          exception_table_bytes)) {
    return ErrorMessage{
        absl::StrFormat("Unable to read Exception Table: %s", llvm::toString(std::move(error)))};
  }

  if (exception_table_bytes.size() % sizeof(llvm::Win64EH::RuntimeFunction) != 0) {
    return ErrorMessage{"Unable to read Exception Table: Unexpected size."};
  }

  return llvm::ArrayRef<llvm::Win64EH::RuntimeFunction>{
      reinterpret_cast<const llvm::Win64EH::RuntimeFunction*>(exception_table_bytes.data()),
      exception_table_bytes.size() / sizeof(llvm::Win64EH::RuntimeFunction)};
}

ErrorMessageOr<const llvm::Win64EH::RuntimeFunction*> CoffFileImpl::GetPrimaryRuntimeFunction(
    const llvm::Win64EH::RuntimeFunction* runtime_function) {
  const llvm::Win64EH::UnwindInfo* unwind_info{};
  while (true) {
    uintptr_t unwind_info_bytes{};
    if (llvm::Error error =
            object_file_->getRvaPtr(runtime_function->UnwindInfoOffset, unwind_info_bytes)) {
      return ErrorMessage{absl::StrFormat("Unable to read RUNTIME_FUNCTION at RVA %#x: %s",
                                          runtime_function->UnwindInfoOffset,
                                          llvm::toString(std::move(error)))};
    }

    unwind_info = reinterpret_cast<const llvm::Win64EH::UnwindInfo*>(unwind_info_bytes);
    // From https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64#struct-unwind_info:
    // "UNW_FLAG_CHAININFO: This unwind info structure is not the primary one for the procedure.
    // Instead, the chained unwind info entry is the contents of a previous RUNTIME_FUNCTION entry."
    if ((unwind_info->getFlags() & llvm::Win64EH::UNW_ChainInfo) == 0) {
      break;
    }

    runtime_function = unwind_info->getChainedFunctionEntry();
  }

  return runtime_function;
}

ErrorMessageOr<std::vector<CoffFileImpl::UnwindRange>> CoffFileImpl::GetUnwindRanges() {
  static constexpr std::string_view kErrorMessagePrefix = "Unable to load unwind info ranges: ";
  auto runtime_functions_or_error = GetRuntimeFunctions();
  if (runtime_functions_or_error.has_error()) {
    return ErrorMessage{
        absl::StrCat(kErrorMessagePrefix, runtime_functions_or_error.error().message())};
  }
  const llvm::ArrayRef<llvm::Win64EH::RuntimeFunction>& runtime_functions =
      runtime_functions_or_error.value();

  // We have the RUNTIME_FUNCTIONs, but chained UNWIND_INFO means that a single function can be
  // formed by multiple RUNTIME_FUNCTIONs, so we have to merge some. We use the following rule: we
  // merge two RUNTIME_FUNCTIONs if:
  // - they are adjacent (the end address of the first equals the start address of the second); and
  // - they have the same primary RUNTIME_FUNCTION (where a RUNTIME_FUNCTION that doesn't have
  //   chained UNWIND_INFO is its own primary), as this corresponds to the prologue of the function.
  //
  // Note that this rule doesn't allow for "holes": if two consecutive RUNTIME_FUNCTIONs are not
  // perfectly adjacent, we will consider them two separate functions, even if they have the same
  // primary RUNTIME_FUNCTION. However, we never observed this case.
  //
  // And note that we don't require the primary RUNTIME_FUNCTION to actually be part of the current
  // set of RUNTIME_FUNCTIONs we are merging. We observed this as a very rare but valid case, on
  // which more information can be found below.
  std::vector<UnwindRange> unwind_ranges;
  uint64_t previous_primary_address{};
  uint64_t largest_primary_address{};
  uint64_t number_of_runtime_functions_whose_primary_is_not_the_latest_primary = 0;
  for (const llvm::Win64EH::RuntimeFunction& runtime_function : runtime_functions) {
    auto primary_runtime_function_or_error = GetPrimaryRuntimeFunction(&runtime_function);
    if (primary_runtime_function_or_error.has_error()) {
      return ErrorMessage{
          absl::StrCat(kErrorMessagePrefix, primary_runtime_function_or_error.error().message())};
    }
    const llvm::Win64EH::RuntimeFunction* primary_runtime_function =
        primary_runtime_function_or_error.value();

    const uint64_t start_address = object_file_->getImageBase() + runtime_function.StartAddress;
    const uint64_t end_address = object_file_->getImageBase() + runtime_function.EndAddress;

    if (end_address < start_address) {
      return ErrorMessage{
          absl::StrCat(kErrorMessagePrefix, "RUNTIME_FUNCTION with negative function size.")};
    }

    const uint64_t primary_address =
        object_file_->getImageBase() + primary_runtime_function->StartAddress;

    // https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64#struct-unwind_info states
    // that "the chained unwind info entry is the contents of a *previous* RUNTIME_FUNCTION entry",
    // and we rely on that here.
    if (primary_address > start_address) {
      return ErrorMessage{
          absl::StrCat(kErrorMessagePrefix, "chained RUNTIME_FUNCTION is not a previous one.")};
    }

    if (unwind_ranges.empty()) {
      unwind_ranges.emplace_back(UnwindRange{primary_address, end_address});
      ORBIT_CHECK(previous_primary_address == 0);
      ORBIT_CHECK(largest_primary_address == 0);
      previous_primary_address = primary_address;
      largest_primary_address = primary_address;
      continue;
    }

    ORBIT_CHECK(previous_primary_address != 0);
    ORBIT_CHECK(largest_primary_address != 0);

    UnwindRange& previous_range = unwind_ranges.back();
    const uint64_t previous_end_address = previous_range.end;

    // https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64#struct-runtime_function
    // guarantees that RUNTIME_FUNCTIONs are sorted (by address), and we rely on that here.
    if (start_address < previous_end_address) {
      return ErrorMessage{
          absl::StrCat(kErrorMessagePrefix, "RUNTIME_FUNCTIONs not sorted or overlapping.")};
    }

    if (primary_address == previous_primary_address && start_address == previous_range.end) {
      // This RUNTIME_FUNCTION is adjacent to the previous one, and it has the same primary
      // RUNTIME_FUNCTION as the previous one. Merge them by extending the size of the current
      // UnwindRange (which represents the current function).
      //
      // Normally, the first RUNTIME_FUNCTION of the UnwindRange is a primary RUNTIME_FUNCTION, and
      // it's the primary RUNTIME_FUNCTION of the other ones. However, because of the rare case
      // explained below, this is not a requirement: the primary RUNTIME_FUNCTION shared by the
      // RUNTIME_FUNCTIONs that form the UnwindRange is allowed to not be part of the UnwindRange,
      // as it could be a RUNTIME_FUNCTION that came way earlier and that clearly belongs to a
      // different function, as a means of reusing unwind information from that function.
      previous_range.end = end_address;
    } else {
      ORBIT_CHECK(start_address >= previous_range.end);
      // Start a new UnwindRange (function).
      unwind_ranges.emplace_back(UnwindRange{primary_address, end_address});
    }

    if (primary_address < largest_primary_address) {
      // The primary RUNTIME_FUNCTION of the current RUNTIME_FUNCTION is before the *latest* primary
      // RUNTIME_FUNCTION we have seen.
      //
      // This is rare, but we observed it in a complex game binary. Indeed, this is allowed by
      // https://docs.microsoft.com/en-us/cpp/build/exception-handling-x64chained-unwind-info-structures:
      // "Chained info [...] can be used for noncontiguous code segments. By using chained info, you
      // can reduce the size of the required unwind information, because you do not have to
      // duplicate the unwind codes array from the primary unwind info."
      //
      // In the cases we observed, the address of the primary RUNTIME_FUNCTION is so much lower than
      // the address of the current RUNTIME_FUNCTION that it's clear that they don't actually belong
      // to the same function.
      //
      // Also, in the cases we observed, these RUNTIME_FUNCTIONs come in groups of adjacent
      // RUNTIME_FUNCTIONs with the same primary RUNTIME_FUNCTION, which makes us assume they belong
      // to the same function.
      //
      // Count these RUNTIME_FUNCTIONs and log their number later as this being such a rare case
      // makes it interesting.
      ++number_of_runtime_functions_whose_primary_is_not_the_latest_primary;
    }

    previous_primary_address = primary_address;
    largest_primary_address = std::max(largest_primary_address, primary_address);
  }

  if (number_of_runtime_functions_whose_primary_is_not_the_latest_primary > 0) {
    ORBIT_LOG(
        "\"%s\" has %u RUNTIME_FUNCTIONs whose primary RUNTIME_FUNCTION is not the latest primary "
        "RUNTIME_FUNCTION",
        file_path_.string(), number_of_runtime_functions_whose_primary_is_not_the_latest_primary);
  }
  return unwind_ranges;
}

ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadExceptionTableEntriesAsSymbols() {
  return LoadExceptionTableEntriesAsSymbolsInternal(GetUnwindRanges());
}

ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadExceptionTableEntriesAsSymbolsInternal(
    const ErrorMessageOr<std::vector<UnwindRange>>& unwind_ranges_or_error) {
  static constexpr std::string_view kErrorMessagePrefix =
      "Unable to load unwind info ranges from the Exception Table: ";
  if (!unwind_ranges_or_error.has_value()) {
    return ErrorMessage{
        absl::StrCat(kErrorMessagePrefix, unwind_ranges_or_error.error().message())};
  }

  ModuleSymbols module_symbols;
  for (const UnwindRange& unwind_range : unwind_ranges_or_error.value()) {
    SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
    // Let's assign an arbitrary function name right away, as we want a non-empty and unique name in
    // many places.
    symbol_info->set_demangled_name(absl::StrFormat("[function@%#x]", unwind_range.start));
    symbol_info->set_address(unwind_range.start);
    symbol_info->set_size(unwind_range.end - unwind_range.start);
  }

  if (module_symbols.symbol_infos().empty()) {
    return ErrorMessage{
        absl::StrCat(kErrorMessagePrefix, "not even a single address range found.")};
  }
  return module_symbols;
}

ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols() {
  static constexpr std::string_view kErrorMessagePrefix = "Unable to load fallback symbols: ";

  ErrorMessageOr<std::vector<UnwindRange>> unwind_ranges_or_error = GetUnwindRanges();
  if (!unwind_ranges_or_error.has_value()) {
    return ErrorMessage{
        absl::StrCat(kErrorMessagePrefix, unwind_ranges_or_error.error().message())};
  }

  ErrorMessageOr<ModuleSymbols> dynamic_linking_symbols =
      LoadSymbolsFromExportTableInternal(unwind_ranges_or_error.value());
  ErrorMessageOr<ModuleSymbols> unwind_ranges_as_symbols =
      LoadExceptionTableEntriesAsSymbolsInternal(unwind_ranges_or_error.value());
  if (!dynamic_linking_symbols.has_value() && !unwind_ranges_as_symbols.has_value()) {
    return ErrorMessage{absl::StrFormat("%s1) %s 2) %s", kErrorMessagePrefix,
                                        dynamic_linking_symbols.error().message(),
                                        unwind_ranges_as_symbols.error().message())};
  }
  std::vector<SymbolInfo> dynamic_linking_symbols_and_unwind_ranges_as_symbols;

  absl::flat_hash_set<uint64_t> dynamic_linking_addresses;
  if (dynamic_linking_symbols.has_value()) {
    for (SymbolInfo& symbol_info : *dynamic_linking_symbols.value().mutable_symbol_infos()) {
      dynamic_linking_addresses.insert(symbol_info.address());
      // Remember that the Export Table doesn't contain the size of symbols. For leaf functions,
      // we couldn't extract the size from unwind information, so we had set it to zero. Let's set
      // those zeros back to our placeholder, so that later we can deduce these sizes with
      // DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol.
      if (symbol_info.size() == 0) {
        symbol_info.set_size(SymbolsFile::kUnknownSymbolSize);
      }
      dynamic_linking_symbols_and_unwind_ranges_as_symbols.emplace_back(std::move(symbol_info));
    }
  }

  if (unwind_ranges_as_symbols.has_value()) {
    for (SymbolInfo& symbol_info : *unwind_ranges_as_symbols.value().mutable_symbol_infos()) {
      if (dynamic_linking_addresses.contains(symbol_info.address())) {
        continue;
      }
      dynamic_linking_symbols_and_unwind_ranges_as_symbols.emplace_back(std::move(symbol_info));
    }
  }

  // NOTE: If an exported leaf function (for which we set the size to kUnknownSymbolSize, see above)
  // is followed by a non-exported leaf function, the non-exported leaf function won't appear in our
  // symbols (it doesn't have a RUNTIME_FUNCTION), and the size deduced for the exported leaf
  // function will include the non-exported one. This is not ideal, but we have to accept it.
  DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol(
      &dynamic_linking_symbols_and_unwind_ranges_as_symbols);

  ModuleSymbols resulting_module_symbols;
  for (SymbolInfo& symbol_info : dynamic_linking_symbols_and_unwind_ranges_as_symbols) {
    *resulting_module_symbols.add_symbol_infos() = std::move(symbol_info);
  }
  return resulting_module_symbols;
}

const std::filesystem::path& CoffFileImpl::GetFilePath() const { return file_path_; }

std::string CoffFileImpl::GetName() const { return file_path_.filename().string(); }

uint64_t CoffFileImpl::GetLoadBias() const { return object_file_->getImageBase(); }

uint64_t CoffFileImpl::GetExecutableSegmentOffset() const {
  ORBIT_CHECK(object_file_->is64());
  return object_file_->getPE32PlusHeader()->BaseOfCode;
}

uint64_t CoffFileImpl::GetImageSize() const {
  ORBIT_CHECK(object_file_->is64());
  return object_file_->getPE32PlusHeader()->SizeOfImage;
}

const std::vector<ModuleInfo::ObjectSegment>& CoffFileImpl::GetObjectSegments() const {
  return sections_;
}

bool CoffFileImpl::IsElf() const { return false; }
bool CoffFileImpl::IsCoff() const { return true; }

ErrorMessageOr<PdbDebugInfo> CoffFileImpl::GetDebugPdbInfo() const {
  const llvm::codeview::DebugInfo* debug_info = nullptr;
  llvm::StringRef pdb_file_path;

  // If 'this' was successfully created with CreateCoffFile, 'object_file_' cannot be nullptr.
  ORBIT_CHECK(object_file_ != nullptr);

  if (llvm::Error error = object_file_->getDebugPDBInfo(debug_info, pdb_file_path)) {
    return ErrorMessage(
        absl::StrFormat("Unable to load debug PDB info: %s", llvm::toString(std::move(error))));
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

  return std::make_unique<CoffFileImpl>(file_path, std::move(file));
}

}  // namespace orbit_object_utils
