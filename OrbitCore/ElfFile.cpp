#include "ElfFile.h"

#include <vector>

#include "OrbitFunction.h"
#include "Path.h"
#include "PrintVar.h"
#include "absl/strings/str_format.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ObjectFile.h"

std::unique_ptr<ElfFile> ElfFile::Create(const std::string& file_path) {
  std::unique_ptr<ElfFile> result(new ElfFile(file_path));
  if (!result->Load()) {
    return nullptr;
  }

  return result;
}

bool ElfFile::Load() {
  llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>>
      object_file_or_error =
          llvm::object::ObjectFile::createObjectFile(file_path_);

  if (!object_file_or_error) {
    return false;
  }

  file_ = std::move(object_file_or_error.get());

  const llvm::object::ObjectFile* object_file = file_.getBinary();
  // Find the .text section
  for (llvm::object::SectionRef section : object_file->sections()) {
    llvm::StringRef name;
    section.getName(name);
    if (name.str() == ".text") {
      text_section_ = std::make_unique<llvm::object::SectionRef>(section);
      break;
    }
  }

  return true;
}

bool ElfFile::IsAddressInTextSection(uint64_t address) const {
  if (!text_section_) {
    PRINT(".text section was not found\n");
    return false;
  }

  uint64_t section_begin_address = text_section_->getAddress();
  uint64_t section_end_address =
      section_begin_address + text_section_->getSize();

  return (address >= section_begin_address) && (address < section_end_address);
}

bool ElfFile::GetFunctions(Pdb* pdb, std::vector<Function>* functions) const {
  const llvm::object::ObjectFile* object_file = file_.getBinary();
  bool function_added = false;

  // TODO: we should probably check if .symtab is available before doing this.
  for (auto it = object_file->symbol_begin(); it != object_file->symbol_end();
       ++it) {
    auto symbol_ref = static_cast<const llvm::object::ELFSymbolRef*>(&*it);
    if ((symbol_ref->getFlags() & llvm::object::BasicSymbolRef::SF_Undefined) !=
        0) {
      continue;
    }

    std::string name = symbol_ref->getName() ? symbol_ref->getName().get() : "";

    // Unknown type - skip and generate a warning
    if (!symbol_ref->getType()) {
      PRINT(
          absl::StrFormat("WARNING: Type is not set for symbol \"%s\" in "
                          "\"%s\", skipping.",
                          name, file_path_));
      continue;
    }

    // Limit list of symbols to functions. Ignore sections and variables.
    if (symbol_ref->getType().get() != llvm::object::SymbolRef::ST_Function) {
      continue;
    }

    functions->emplace_back(name, Path::GetFileName(file_path_),
                            symbol_ref->getValue(), symbol_ref->getSize(), pdb);
    function_added = true;
  }

  return function_added;
}
