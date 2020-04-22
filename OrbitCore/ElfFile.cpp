#include "ElfFile.h"

#include <string_view>
#include <vector>

#include "OrbitFunction.h"
#include "Path.h"
#include "PrintVar.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "llvm/Demangle/Demangle.h"
#include "llvm/Object/ELFObjectFile.h"
#include "llvm/Object/ObjectFile.h"

namespace {

template <typename ElfT>
class ElfFileImpl : public ElfFile {
 public:
  ElfFileImpl(
      std::string_view file_path,
      llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary);

  bool LoadFunctions(Pdb* pdb) const override;
  std::optional<uint64_t> GetLoadBias() const override;
  bool IsAddressInTextSection(uint64_t address) const override;
  bool HasSymtab() const override;
  std::string GetBuildId() const override;
  std::string GetFilePath() const override;

 private:
  void InitSections();

  const std::string file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::ELFObjectFile<ElfT>* object_file_;
  std::unique_ptr<typename ElfT::Shdr> text_section_;
  std::string build_id_;
  bool has_symtab_section_;
};

template <typename ElfT>
ElfFileImpl<ElfT>::ElfFileImpl(
    std::string_view file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(file_path),
      owning_binary_(std::move(owning_binary)),
      has_symtab_section_(false) {
  object_file_ = llvm::dyn_cast<llvm::object::ELFObjectFile<ElfT>>(
      owning_binary_.getBinary());
  InitSections();
}

template <typename ElfT>
void ElfFileImpl<ElfT>::InitSections() {
  const llvm::object::ELFFile<ElfT>* elf_file = object_file_->getELFFile();

  llvm::Expected<typename ElfT::ShdrRange> sections_or_err =
      elf_file->sections();
  if (!sections_or_err) {
    LOG("Unable to load sections");
    return;
  }

  for (const typename ElfT::Shdr& section : sections_or_err.get()) {
    llvm::Expected<llvm::StringRef> name_or_error =
        elf_file->getSectionName(&section);
    if (!name_or_error) {
      LOG("Unable to get section name");
      continue;
    }
    llvm::StringRef name = name_or_error.get();

    if (name.str() == ".text") {
      text_section_ = std::make_unique<typename ElfT::Shdr>(section);
    }

    if (name.str() == ".symtab") {
      has_symtab_section_ = true;
    }

    if (name.str() == ".note.gnu.build-id" &&
        section.sh_type == llvm::ELF::SHT_NOTE) {
      llvm::Error error = llvm::Error::success();
      for (const typename ElfT::Note& note : elf_file->notes(section, error)) {
        if (note.getType() != llvm::ELF::NT_GNU_BUILD_ID) continue;

        llvm::ArrayRef<uint8_t> desc = note.getDesc();
        for (const uint8_t& byte : desc) {
          absl::StrAppend(&build_id_, absl::Hex(byte, absl::kZeroPad2));
        }
      }
      if (error) {
        LOG("Error while reading elf notes");
      }
    }
  }
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::IsAddressInTextSection(uint64_t address) const {
  if (!text_section_) {
    LOG(".text section was not found");
    return false;
  }

  uint64_t section_begin_address = text_section_->sh_addr;
  uint64_t section_end_address = section_begin_address + text_section_->sh_size;

  return (address >= section_begin_address) && (address < section_end_address);
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::LoadFunctions(Pdb* pdb) const {
  // TODO: if we want to use other sections than .symtab in the future for
  //       example .dynsym, than we have to change this.
  if (!has_symtab_section_) {
    return false;
  }
  bool function_added = false;

  std::optional<uint64_t> load_bias_optional = GetLoadBias();
  if (!load_bias_optional) {
    return false;
  }

  uint64_t load_bias = load_bias_optional.value();

  for (const llvm::object::ELFSymbolRef& symbol_ref : object_file_->symbols()) {
    if ((symbol_ref.getFlags() & llvm::object::BasicSymbolRef::SF_Undefined) !=
        0) {
      continue;
    }

    std::string name = symbol_ref.getName() ? symbol_ref.getName().get() : "";
    std::string pretty_name = llvm::demangle(name);

    // Unknown type - skip and generate a warning
    if (!symbol_ref.getType()) {
      LOG("WARNING: Type is not set for symbol \"%s\" in \"%s\", skipping.",
          name.c_str(), file_path_.c_str());
      continue;
    }

    // Limit list of symbols to functions. Ignore sections and variables.
    if (symbol_ref.getType().get() != llvm::object::SymbolRef::ST_Function) {
      continue;
    }

    std::shared_ptr<Function> function = std::make_shared<Function>(
        name, pretty_name, Path::GetFileName(file_path_), symbol_ref.getValue(),
        symbol_ref.getSize(), load_bias, pdb);

    // For uprobes we need a function to be in the .text segment (why?)
    // TODO: Shouldn't m_Functions be limited to the list of functions
    // referencing .text segment?
    if (IsAddressInTextSection(function->Address())) {
      function->SetProbe(file_path_ + ":" + function->Name());
    }

    pdb->AddFunction(function);
    function_added = true;
  }

  return function_added;
}

template <typename ElfT>
std::optional<uint64_t> ElfFileImpl<ElfT>::GetLoadBias() const {
  const llvm::object::ELFFile<ElfT>* elf_file = object_file_->getELFFile();

  uint64_t min_vaddr = UINT64_MAX;
  bool pt_load_found = false;
  llvm::Expected<typename ElfT::PhdrRange> range = elf_file->program_headers();

  if (!range) {
    LOG("No program headers found in %s\n", file_path_);
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
    LOG("No PT_LOAD program headers found in %s", file_path_);
    return {};
  }
  return min_vaddr;
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::HasSymtab() const {
  return has_symtab_section_;
}

template <typename ElfT>
std::string ElfFileImpl<ElfT>::GetBuildId() const {
  return build_id_;
}

template <typename ElfT>
std::string ElfFileImpl<ElfT>::GetFilePath() const {
  return file_path_;
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
