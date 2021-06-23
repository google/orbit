// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/CoffFile.h"

#include <absl/strings/str_format.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/COFF.h>
#include <llvm/Object/ObjectFile.h>

#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "symbol.pb.h"

namespace orbit_object_utils {

namespace {

using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

class CoffFileImpl : public CoffFile {
 public:
  CoffFileImpl(std::filesystem::path file_path,
               llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary);
  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() override;
  [[nodiscard]] bool HasDebugSymbols() const override;
  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;
  [[nodiscard]] bool IsElf() const override;
  [[nodiscard]] bool IsCoff() const override;

 private:
  ErrorMessageOr<uint64_t> GetSectionOffsetForSymbol(const llvm::object::SymbolRef& symbol_ref);
  ErrorMessageOr<SymbolInfo> CreateSymbolInfo(const llvm::object::SymbolRef& symbol_ref);
  const std::filesystem::path file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::COFFObjectFile* object_file_;
  bool has_symbol_table_;
};

CoffFileImpl::CoffFileImpl(std::filesystem::path file_path,
                           llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(std::move(file_path)),
      owning_binary_(std::move(owning_binary)),
      has_symbol_table_(false) {
  object_file_ = llvm::dyn_cast<llvm::object::COFFObjectFile>(owning_binary_.getBinary());
  // Per specification, a COFF file has a symbol table if the pointer to the symbol table is not 0.
  has_symbol_table_ = (object_file_->getSymbolTable() != 0);
}

ErrorMessageOr<uint64_t> CoffFileImpl::GetSectionOffsetForSymbol(
    const llvm::object::SymbolRef& symbol_ref) {
  // Symbol addresses are relative virtual addresses (RVAs) that are relative to the
  // start of the section. To get the address relative to the start of the object
  // file, we need to find the section and add the base address of the section.
  llvm::object::COFFSymbolRef coff_symbol_ref = object_file_->getCOFFSymbol(symbol_ref);
  int32_t section_id = coff_symbol_ref.getSectionNumber();
  const llvm::object::coff_section* section = nullptr;
  std::error_code err = object_file_->getSection(section_id, section);
  if (err) {
    return ErrorMessage(absl::StrFormat("Section of symbol not found: %s", err.message()));
  }
  return section->VirtualAddress;
}

ErrorMessageOr<SymbolInfo> CoffFileImpl::CreateSymbolInfo(
    const llvm::object::SymbolRef& symbol_ref) {
  if ((symbol_ref.getFlags() & llvm::object::BasicSymbolRef::SF_Undefined) != 0) {
    return ErrorMessage("Symbol is defined in another object file (SF_Undefined flag is set).");
  }
  const std::string name = symbol_ref.getName() ? symbol_ref.getName().get() : "";
  // Unknown type - skip and generate a warning.
  if (!symbol_ref.getType()) {
    LOG("WARNING: Type is not set for symbol \"%s\" in \"%s\", skipping.", name,
        file_path_.string());
    return ErrorMessage(absl::StrFormat(R"(Type is not set for symbol "%s" in "%s", skipping.)",
                                        name, file_path_.string()));
  }
  // Limit list of symbols to functions. Ignore sections and variables.
  if (symbol_ref.getType().get() != llvm::object::SymbolRef::ST_Function) {
    return ErrorMessage("Symbol is not a function.");
  }

  auto section_offset = GetSectionOffsetForSymbol(symbol_ref);
  if (section_offset.has_error()) {
    return ErrorMessage(absl::StrFormat("Error retrieving section offset for symbol \"%s\": %s",
                                        name, section_offset.error().message()));
  }
  SymbolInfo symbol_info;
  symbol_info.set_name(name);
  symbol_info.set_demangled_name(llvm::demangle(name));
  symbol_info.set_address(symbol_ref.getValue() + section_offset.value());

  llvm::object::COFFSymbolRef coff_symbol_ref = object_file_->getCOFFSymbol(symbol_ref);
  symbol_info.set_size(coff_symbol_ref.getValue());

  return symbol_info;
}

ErrorMessageOr<ModuleSymbols> CoffFileImpl::LoadDebugSymbols() {
  if (!has_symbol_table_) {
    return ErrorMessage("COFF file does not have a symbol table.");
  }

  ModuleSymbols module_symbols;
  module_symbols.set_symbols_file_path(file_path_.string());

  for (const auto& symbol_ref : object_file_->symbols()) {
    auto symbol_or_error = CreateSymbolInfo(symbol_ref);
    if (symbol_or_error.has_value()) {
      *module_symbols.add_symbol_infos() = std::move(symbol_or_error.value());
    }
  }

  if (module_symbols.symbol_infos_size() == 0) {
    return ErrorMessage(
        "Unable to load symbols from object file, not even a single symbol of "
        "type function found.");
  }
  return module_symbols;
}

bool CoffFileImpl::HasDebugSymbols() const { return has_symbol_table_; }

const std::filesystem::path& CoffFileImpl::GetFilePath() const { return file_path_; }

std::string CoffFileImpl::GetName() const { return file_path_.filename().string(); }

bool CoffFileImpl::IsElf() const { return false; }
bool CoffFileImpl::IsCoff() const { return true; }

}  // namespace

ErrorMessageOr<std::unique_ptr<CoffFile>> CreateCoffFile(const std::filesystem::path& file_path) {
  // TODO(hebecker): Remove this explicit construction of StringRef when we
  // switch to LLVM10.
  const std::string file_path_str = file_path.string();
  const llvm::StringRef file_path_llvm{file_path_str};

  llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>> object_file_or_error =
      llvm::object::ObjectFile::createObjectFile(file_path_llvm);

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
  llvm::object::ObjectFile* object_file = file.getBinary();
  if (llvm::dyn_cast<llvm::object::COFFObjectFile>(object_file) != nullptr) {
    return std::unique_ptr<CoffFile>(new CoffFileImpl(file_path, std::move(file)));
  } else {
    return ErrorMessage(absl::StrFormat("Unable to load object file \"%s\":", file_path.string()));
  }
}

}  // namespace orbit_object_utils
