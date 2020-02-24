#include "ElfFile.h"

#include <vector>

#include "OrbitFunction.h"
#include "Path.h"
#include "PrintVar.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ObjectFile.h"

namespace {

template <typename ElfT>
class ElfFileImpl : public ElfFile {
 public:
  ElfFileImpl(
      absl::string_view file_path,
      llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary);

  bool GetFunctions(Pdb* pdb, std::vector<Function>* functions) const override;
  absl::optional<uint64_t> GetLoadBias() const override;
  bool IsAddressInTextSection(uint64_t address) const override;

 private:
  void FindTextSection();

  const std::string file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::ELFObjectFile<ElfT>* object_file_;
  std::unique_ptr<llvm::object::SectionRef> text_section_;
};

template <typename ElfT>
ElfFileImpl<ElfT>::ElfFileImpl(
    absl::string_view file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(file_path), owning_binary_(std::move(owning_binary)) {
  object_file_ = llvm::dyn_cast<llvm::object::ELFObjectFile<ElfT>>(
      owning_binary_.getBinary());
  FindTextSection();
}

template <typename ElfT>
void ElfFileImpl<ElfT>::FindTextSection() {
  // Find the .text section
  for (llvm::object::SectionRef section : object_file_->sections()) {
    llvm::StringRef name;
    section.getName(name);
    if (name.str() == ".text") {
      text_section_ = std::make_unique<llvm::object::SectionRef>(section);
      break;
    }
  }
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::IsAddressInTextSection(uint64_t address) const {
  if (!text_section_) {
    PRINT(".text section was not found\n");
    return false;
  }

  uint64_t section_begin_address = text_section_->getAddress();
  uint64_t section_end_address =
      section_begin_address + text_section_->getSize();

  return (address >= section_begin_address) && (address < section_end_address);
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::GetFunctions(Pdb* pdb,
                                     std::vector<Function>* functions) const {
  bool function_added = false;

  // TODO: we should probably check if .symtab is available before doing this.
  for (const llvm::object::ELFSymbolRef& symbol_ref : object_file_->symbols()) {
    if ((symbol_ref.getFlags() & llvm::object::BasicSymbolRef::SF_Undefined) !=
        0) {
      continue;
    }

    std::string name = symbol_ref.getName() ? symbol_ref.getName().get() : "";

    // Unknown type - skip and generate a warning
    if (!symbol_ref.getType()) {
      PRINT(
          absl::StrFormat("WARNING: Type is not set for symbol \"%s\" in "
                          "\"%s\", skipping.",
                          name, file_path_));
      continue;
    }

    // Limit list of symbols to functions. Ignore sections and variables.
    if (symbol_ref.getType().get() != llvm::object::SymbolRef::ST_Function) {
      continue;
    }

    functions->emplace_back(name, Path::GetFileName(file_path_),
                            symbol_ref.getValue(), symbol_ref.getSize(), pdb);
    function_added = true;
  }

  return function_added;
}

template <typename ElfT>
absl::optional<uint64_t> ElfFileImpl<ElfT>::GetLoadBias() const {
  const llvm::object::ELFFile<ElfT>* elf_file = object_file_->getELFFile();

  uint64_t min_vaddr = UINT64_MAX;
  bool pt_load_found = false;
  llvm::Expected<typename ElfT::PhdrRange> range = elf_file->program_headers();

  if (!range) {
    PRINT(absl::StrFormat("No program headers found in %s\n", file_path_));
    return {};
  }

  for (const typename ElfT::Phdr& phdr : range.get()) {
    if (phdr.p_type != llvm::ELF::PT_LOAD) {
      continue;
    }
    pt_load_found = true;

    if (min_vaddr > phdr.p_vaddr) {
      min_vaddr = phdr.p_vaddr;
    }
  }

  if (!pt_load_found) {
    PRINT(absl::StrFormat("No PT_LOAD program headers found in %s\n",
                          file_path_));
    return {};
  }
  return min_vaddr;
}

}  // namespace

std::unique_ptr<ElfFile> ElfFile::Create(const std::string& file_path) {
  llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>>
      object_file_or_error =
          llvm::object::ObjectFile::createObjectFile(file_path);

  if (!object_file_or_error) {
    return nullptr;
  }

  llvm::object::OwningBinary<llvm::object::ObjectFile>& file =
      object_file_or_error.get();

  llvm::object::ObjectFile* object_file = file.getBinary();

  std::unique_ptr<ElfFile> result;

  // Create appropriate ElfFile implementation
  if (llvm::dyn_cast<llvm::object::ELF32LEObjectFile>(object_file) != nullptr) {
    result = std::unique_ptr<ElfFile>(
        new ElfFileImpl<llvm::object::ELF32LE>(file_path, std::move(file)));
  } else if (llvm::dyn_cast<llvm::object::ELF64LEObjectFile>(object_file) !=
             nullptr) {
    result = std::unique_ptr<ElfFile>(
        new ElfFileImpl<llvm::object::ELF64LE>(file_path, std::move(file)));
  } else {
    // Big-endians are not supported
    return nullptr;
  }

  return result;
}
