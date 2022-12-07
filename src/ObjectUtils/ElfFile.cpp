// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/ElfFile.h"

#include <absl/base/casts.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <llvm/ADT/ArrayRef.h>
#include <llvm/ADT/Optional.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/iterator.h>
#include <llvm/BinaryFormat/Dwarf.h>
#include <llvm/BinaryFormat/ELF.h>
#include <llvm/DebugInfo/DIContext.h>
#include <llvm/DebugInfo/DWARF/DWARFCompileUnit.h>
#include <llvm/DebugInfo/DWARF/DWARFContext.h>
#include <llvm/DebugInfo/DWARF/DWARFDebugAranges.h>
#include <llvm/DebugInfo/DWARF/DWARFDebugFrame.h>
#include <llvm/DebugInfo/DWARF/DWARFDebugLine.h>
#include <llvm/DebugInfo/DWARF/DWARFDie.h>
#include <llvm/DebugInfo/DWARF/DWARFFormValue.h>
#include <llvm/DebugInfo/Symbolize/Symbolize.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Object/Binary.h>
#include <llvm/Object/ELF.h>
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ELFTypes.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Object/SymbolicFile.h>
#include <llvm/Support/CRC.h>
#include <llvm/Support/Casting.h>
#include <llvm/Support/Endian.h>
#include <llvm/Support/Error.h>
#include <llvm/Support/MemoryBuffer.h>

#include <algorithm>
#include <cstring>
#include <type_traits>
#include <utility>
#include <vector>

#include "GrpcProtos/module.pb.h"
#include "GrpcProtos/symbol.pb.h"
#include "Introspection/Introspection.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

namespace {

using orbit_grpc_protos::LineInfo;
using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

template <typename ElfT>
class ElfFileImpl : public ElfFile {
 public:
  ElfFileImpl(std::filesystem::path file_path,
              llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary);

  ErrorMessageOr<void> Initialize();

  // Loads symbols from the .symtab section.
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadDebugSymbols() override;
  [[nodiscard]] bool HasDebugSymbols() const override;
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadSymbolsFromDynsym() override;
  [[nodiscard]] bool HasDynsym() const override;
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadEhOrDebugFrameEntriesAsSymbols() override;
  [[nodiscard]] ErrorMessageOr<ModuleSymbols> LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols()
      override;

  [[nodiscard]] uint64_t GetLoadBias() const override;
  [[nodiscard]] uint64_t GetExecutableSegmentOffset() const override;
  [[nodiscard]] uint64_t GetImageSize() const override;
  [[nodiscard]] const std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment>& GetObjectSegments()
      const override;
  [[nodiscard]] bool HasDebugInfo() const override;
  [[nodiscard]] bool HasGnuDebuglink() const override;
  [[nodiscard]] bool Is64Bit() const override;
  [[nodiscard]] bool IsElf() const override;
  [[nodiscard]] bool IsCoff() const override;
  [[nodiscard]] std::string GetBuildId() const override;
  [[nodiscard]] std::string GetName() const override;
  [[nodiscard]] std::string GetSoname() const override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override;
  [[nodiscard]] ErrorMessageOr<LineInfo> GetLineInfo(uint64_t address) override;
  [[nodiscard]] ErrorMessageOr<LineInfo> GetDeclarationLocationOfFunction(
      uint64_t address) override;
  [[nodiscard]] std::optional<GnuDebugLinkInfo> GetGnuDebugLinkInfo() const override;
  [[nodiscard]] ErrorMessageOr<LineInfo> GetLocationOfFunction(uint64_t address) override;

 private:
  ErrorMessageOr<void> InitSections();
  ErrorMessageOr<void> InitProgramHeaders();
  ErrorMessageOr<void> InitDynamicEntries();
  ErrorMessageOr<SymbolInfo> CreateSymbolInfo(
      const llvm::object::ELFSymbolRef& symbol_ref,
      const absl::flat_hash_set<uint64_t>& hotpachable_addresses);
  [[nodiscard]] absl::flat_hash_set<uint64_t> LoadHotpatchableAddresses();

  const std::filesystem::path file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> owning_binary_;
  llvm::object::ELFObjectFile<ElfT>* object_file_;
  llvm::symbolize::LLVMSymbolizer symbolizer_;
  std::string build_id_;
  std::string soname_;
  bool has_symtab_section_;
  bool has_dynsym_section_;
  bool has_patchable_function_entries_section_;
  bool has_debug_info_section_;
  std::optional<GnuDebugLinkInfo> gnu_debuglink_info_;

  uint64_t load_bias_;
  uint64_t executable_segment_offset_;
  uint64_t executable_segment_size_;
  uint64_t image_size_;
  std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment> loadable_segments_;
};

bool IsHotpatchable(const absl::flat_hash_set<uint64_t>& hotpachable_addresses,
                    uint64_t symbol_address) {
  // The hotpatchable addresses stored in the elf file point to the first byte of the padding. We
  // require the binary to be compiled with a five byte padding and a two byte nop at the function
  // entry. So we check for "address - 5" to be listed as hotpatchable.
  constexpr int kPaddingSize = 5;
  return hotpachable_addresses.contains(symbol_address - kPaddingSize);
}

template <typename ElfT>
ErrorMessageOr<GnuDebugLinkInfo> ReadGnuDebuglinkSection(
    const typename ElfT::Shdr& section_header, const llvm::object::ELFFile<ElfT>& elf_file) {
  llvm::Expected<llvm::ArrayRef<uint8_t>> contents_or_error =
      elf_file.getSectionContents(section_header);
  if (!contents_or_error) {
    return ErrorMessage(absl::StrFormat("Could not read .gnu_debuglink section: %s",
                                        llvm::toString(contents_or_error.takeError())));
  }

  const llvm::ArrayRef<uint8_t>& contents = contents_or_error.get();

  using ChecksumType = uint32_t;
  constexpr size_t kMinimumPathLength = 1;
  if (contents.size() < kMinimumPathLength + sizeof(ChecksumType)) {
    return ErrorMessage{"Section is too short."};
  }

  constexpr size_t kOneHundredKiB = 100 * 1024;
  if (contents.size() > kOneHundredKiB) {
    return ErrorMessage{"Section is longer than 100KiB. Something is not right."};
  }

  std::string path{};
  path.reserve(contents.size());

  for (const auto& byte : contents) {
    if (byte == '\0') break;
    path.push_back(static_cast<char>(byte));
  }

  if (path.size() > contents.size() - sizeof(ChecksumType)) {
    return ErrorMessage{"No CRC32 checksum found"};
  }

  static_assert(ElfT::TargetEndianness == llvm::support::little,
                "This code only supports little endian architectures.");
  const auto checksum_storage = contents.slice(contents.size() - sizeof(ChecksumType));
  uint32_t reference_crc{};
  std::memcpy(&reference_crc, checksum_storage.data(), sizeof(reference_crc));

  GnuDebugLinkInfo gnu_debuglink_info{};
  gnu_debuglink_info.path = std::filesystem::path{std::move(path)};
  gnu_debuglink_info.crc32_checksum = reference_crc;
  return gnu_debuglink_info;
}

template <typename ElfT>
ElfFileImpl<ElfT>::ElfFileImpl(std::filesystem::path file_path,
                               llvm::object::OwningBinary<llvm::object::ObjectFile>&& owning_binary)
    : file_path_(std::move(file_path)),
      owning_binary_(std::move(owning_binary)),
      object_file_(llvm::dyn_cast<llvm::object::ELFObjectFile<ElfT>>(owning_binary_.getBinary())),
      has_symtab_section_(false),
      has_dynsym_section_(false),
      has_patchable_function_entries_section_(false),
      has_debug_info_section_(false),
      load_bias_{0},
      executable_segment_offset_{0},
      executable_segment_size_{0},
      image_size_{0} {}

template <typename ElfT>
ErrorMessageOr<void> ElfFileImpl<ElfT>::Initialize() {
  OUTCOME_TRY(InitSections());
  OUTCOME_TRY(InitDynamicEntries());
  OUTCOME_TRY(InitProgramHeaders());
  return outcome::success();
}

template <typename ElfT>
ErrorMessageOr<void> ElfFileImpl<ElfT>::InitDynamicEntries() {
  const llvm::object::ELFFile<ElfT>& elf_file = object_file_->getELFFile();
  auto dynamic_entries_or_error = elf_file.dynamicEntries();
  if (!dynamic_entries_or_error) {
    auto error_message =
        absl::StrFormat("Unable to get dynamic entries from \"%s\": %s", file_path_.string(),
                        llvm::toString(dynamic_entries_or_error.takeError()));
    ORBIT_ERROR("%s (ignored)", error_message);
    // Apparently empty dynamic section results in error - we are going to ignore it.
    return outcome::success();
  }

  std::optional<uint64_t> soname_offset;
  std::optional<uint64_t> dynamic_string_table_addr;
  std::optional<uint64_t> dynamic_string_table_size;
  auto dyn_range = dynamic_entries_or_error.get();
  for (const auto& dyn_entry : dyn_range) {
    switch (dyn_entry.getTag()) {
      case llvm::ELF::DT_SONAME:
        soname_offset.emplace(dyn_entry.getVal());
        break;
      case llvm::ELF::DT_STRTAB:
        dynamic_string_table_addr.emplace(dyn_entry.getPtr());
        break;
      case llvm::ELF::DT_STRSZ:
        dynamic_string_table_size.emplace(dyn_entry.getVal());
        break;
      default:
        break;
    }
  }

  if (!soname_offset.has_value() || !dynamic_string_table_addr.has_value() ||
      !dynamic_string_table_size.has_value()) {
    return outcome::success();
  }

  auto strtab_last_byte_or_error = elf_file.toMappedAddr(dynamic_string_table_addr.value() +
                                                         dynamic_string_table_size.value() - 1);
  if (!strtab_last_byte_or_error) {
    auto error_message =
        absl::StrFormat("Unable to get last byte address of dynamic string table \"%s\": %s",
                        file_path_.string(), llvm::toString(strtab_last_byte_or_error.takeError()));
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  if (soname_offset.value() >= dynamic_string_table_size.value()) {
    auto error_message = absl::StrFormat(
        "Soname offset is out of bounds of the string table (file=\"%s\", offset=%u "
        "strtab size=%u)",
        file_path_.string(), soname_offset.value(), dynamic_string_table_size.value());
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  if (*strtab_last_byte_or_error.get() != 0) {
    auto error_message = absl::StrFormat(
        "Dynamic string table is not null-termintated (file=\"%s\")", file_path_.string());
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  auto strtab_addr_or_error = elf_file.toMappedAddr(dynamic_string_table_addr.value());
  if (!strtab_addr_or_error) {
    auto error_message =
        absl::StrFormat("Unable to get dynamic string table from DT_STRTAB in \"%s\": %s",
                        file_path_.string(), llvm::toString(strtab_addr_or_error.takeError()));
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  soname_ = absl::bit_cast<const char*>(strtab_addr_or_error.get()) + soname_offset.value();

  return outcome::success();
}

template <typename ElfT>
ErrorMessageOr<void> ElfFileImpl<ElfT>::InitSections() {
  const llvm::object::ELFFile<ElfT>& elf_file = object_file_->getELFFile();

  llvm::Expected<typename ElfT::ShdrRange> sections_or_error = elf_file.sections();
  if (!sections_or_error) {
    auto error_message = absl::StrFormat("Unable to load sections: %s",
                                         llvm::toString(sections_or_error.takeError()));
    ORBIT_ERROR("%s", error_message);
    return ErrorMessage{error_message};
  }

  for (const typename ElfT::Shdr& section : sections_or_error.get()) {
    llvm::Expected<llvm::StringRef> name_or_error = elf_file.getSectionName(section);
    if (!name_or_error) {
      return ErrorMessage{absl::StrFormat("Unable to get section name: %s",
                                          llvm::toString(name_or_error.takeError()))};
    }
    llvm::StringRef name = name_or_error.get();

    if (name.str() == ".symtab") {
      has_symtab_section_ = true;
      continue;
    }

    if (section.sh_type == llvm::ELF::SHT_DYNSYM) {
      has_dynsym_section_ = true;
      continue;
    }

    if (name.str() == "__patchable_function_entries") {
      has_patchable_function_entries_section_ = true;
      continue;
    }

    if (name.str() == ".debug_info") {
      has_debug_info_section_ = true;
      continue;
    }

    if (name.str() == ".note.gnu.build-id" && section.sh_type == llvm::ELF::SHT_NOTE) {
      llvm::Error error = llvm::Error::success();
      for (const typename ElfT::Note& note : elf_file.notes(section, error)) {
        if (note.getType() != llvm::ELF::NT_GNU_BUILD_ID) continue;

        llvm::ArrayRef<uint8_t> desc = note.getDesc();
        for (const uint8_t& byte : desc) {
          absl::StrAppend(&build_id_, absl::Hex(byte, absl::kZeroPad2));
        }
      }
      if (error) {
        return ErrorMessage{
            absl::StrFormat("Error while reading elf notes: %s", llvm::toString(std::move(error)))};
      }
      continue;
    }

    if (name.str() == ".gnu_debuglink") {
      ErrorMessageOr<GnuDebugLinkInfo> error_or_info = ReadGnuDebuglinkSection(section, elf_file);
      if (error_or_info.has_value()) {
        gnu_debuglink_info_ = std::move(error_or_info.value());
      } else {
        return ErrorMessage{absl::StrFormat("Invalid .gnu_debuglink section in \"%s\". %s",
                                            file_path_.string(), error_or_info.error().message())};
      }
      continue;
    }
  }

  return outcome::success();
}

template <typename ElfT>
ErrorMessageOr<SymbolInfo> ElfFileImpl<ElfT>::CreateSymbolInfo(
    const llvm::object::ELFSymbolRef& symbol_ref,
    const absl::flat_hash_set<uint64_t>& hotpachable_addresses) {
  std::string name;
  if (auto maybe_name = symbol_ref.getName(); maybe_name) name = maybe_name.get().str();

  llvm::Expected<uint32_t> maybe_flags = symbol_ref.getFlags();
  if (!maybe_flags) {
    ORBIT_LOG("WARNING: Flags are not set for symbol \"%s\" in \"%s\", skipping. Details: %s", name,
              file_path_.string(), llvm::toString(maybe_flags.takeError()));
    return ErrorMessage(absl::StrFormat(R"(Flags are not set for symbol "%s" in "%s", skipping.)",
                                        name, file_path_.string()));
  }

  if ((maybe_flags.get() & llvm::object::BasicSymbolRef::SF_Undefined) != 0) {
    return ErrorMessage("Symbol is defined in another object file (SF_Undefined flag is set).");
  }

  // Unknown type - skip and generate a warning.
  llvm::Expected<llvm::object::SymbolRef::Type> maybe_type = symbol_ref.getType();
  if (!maybe_type) {
    ORBIT_LOG("WARNING: Type is not set for symbol \"%s\" in \"%s\", skipping. Details: %s", name,
              file_path_.string(), llvm::toString(maybe_type.takeError()));
    return ErrorMessage(absl::StrFormat(R"(Type is not set for symbol "%s" in "%s", skipping.)",
                                        name, file_path_.string()));
  }
  // Limit list of symbols to functions. Ignore sections and variables.
  if (maybe_type.get() != llvm::object::SymbolRef::ST_Function) {
    return ErrorMessage("Symbol is not a function.");
  }

  llvm::Expected<uint64_t> maybe_value = symbol_ref.getValue();
  if (!maybe_value) {
    ORBIT_LOG("WARNING: Address is not set for symbol \"%s\" in \"%s\", skipping. Details: %s",
              name, file_path_.string(), llvm::toString(maybe_value.takeError()));
    return ErrorMessage(absl::StrFormat(R"(Address is not set for symbol "%s" in "%s", skipping.)",
                                        name, file_path_.string()));
  }

  SymbolInfo symbol_info;
  symbol_info.set_demangled_name(llvm::demangle(name));
  symbol_info.set_address(maybe_value.get());
  symbol_info.set_size(symbol_ref.getSize());
  symbol_info.set_is_hotpatchable(IsHotpatchable(hotpachable_addresses, maybe_value.get()));
  return symbol_info;
}

template <typename ElfT>
ErrorMessageOr<ModuleSymbols> ElfFileImpl<ElfT>::LoadDebugSymbols() {
  if (!has_symtab_section_) {
    return ErrorMessage("ELF file does not have a .symtab section.");
  }

  const absl::flat_hash_set<uint64_t> hotpachable_addresses = LoadHotpatchableAddresses();
  ModuleSymbols module_symbols;

  for (const llvm::object::ELFSymbolRef& symbol_ref : object_file_->symbols()) {
    auto symbol_or_error = CreateSymbolInfo(symbol_ref, hotpachable_addresses);
    if (symbol_or_error.has_value()) {
      *module_symbols.add_symbol_infos() = std::move(symbol_or_error.value());
    }
  }

  if (module_symbols.symbol_infos_size() == 0) {
    return ErrorMessage(
        "Unable to load symbols from ELF file: not even a single symbol of type function found.");
  }
  return module_symbols;
}

template <typename ElfT>
ErrorMessageOr<ModuleSymbols> ElfFileImpl<ElfT>::LoadSymbolsFromDynsym() {
  if (!has_dynsym_section_) {
    return ErrorMessage("ELF file does not have a .dynsym section.");
  }

  const absl::flat_hash_set<uint64_t> hotpachable_addresses = LoadHotpatchableAddresses();
  ModuleSymbols module_symbols;

  for (const llvm::object::ELFSymbolRef& symbol_ref : object_file_->getDynamicSymbolIterators()) {
    auto symbol_or_error = CreateSymbolInfo(symbol_ref, hotpachable_addresses);
    if (symbol_or_error.has_value()) {
      *module_symbols.add_symbol_infos() = std::move(symbol_or_error.value());
    }
  }

  if (module_symbols.symbol_infos_size() == 0) {
    return ErrorMessage(
        "Unable to load symbols from .dynsym section: not even a single symbol of type function "
        "found.");
  }
  return module_symbols;
}

template <typename ElfT>
absl::flat_hash_set<uint64_t> ElfFileImpl<ElfT>::LoadHotpatchableAddresses() {
  if (!has_patchable_function_entries_section_) {
    return {};
  }
  const llvm::object::ELFFile<ElfT>& elf_file = object_file_->getELFFile();
  llvm::Expected<typename ElfT::ShdrRange> sections_or_error = elf_file.sections();
  if (!sections_or_error) {
    auto error_message = absl::StrFormat("Unable to load sections: %s",
                                         llvm::toString(sections_or_error.takeError()));
    ORBIT_ERROR("%s", error_message);
    return {};
  }

  absl::flat_hash_set<uint64_t> patchable_symbols;
  for (const typename ElfT::Shdr& section : sections_or_error.get()) {
    llvm::Expected<llvm::StringRef> name_or_error = elf_file.getSectionName(section);
    if (!name_or_error) {
      ORBIT_ERROR("%s", absl::StrFormat("Unable to get section name: %s",
                                        llvm::toString(name_or_error.takeError())));
      return {};
    }
    llvm::StringRef name = name_or_error.get();
    if (name.str() != "__patchable_function_entries") {
      continue;
    }

    // We cannot use the type safe version getSectionContentsAsArray since the sh_entsize is not set
    // correctly in the elf binaries (should be eight for 64 bit addresses but is zero). So we read
    // the data as an array of bytes and convert it to 64 bit addresses later.
    llvm::Expected<llvm::ArrayRef<uint8_t>> contents_or_error =
        elf_file.getSectionContents(section);
    if (!contents_or_error) {
      ORBIT_ERROR("Could not read __patchable_function_entries section: %s",
                  llvm::toString(contents_or_error.takeError()));
      continue;
    }
    const llvm::ArrayRef<uint8_t>& contents = contents_or_error.get();
    const size_t length = contents.size() / sizeof(uint64_t);
    std::vector<uint64_t> addresses(length);
    memcpy(addresses.data(), contents.data(), length * sizeof(uint64_t));
    patchable_symbols.insert(addresses.begin(), addresses.end());
  }
  return patchable_symbols;
}

template <typename ElfT>
ErrorMessageOr<orbit_grpc_protos::ModuleSymbols>
ElfFileImpl<ElfT>::LoadEhOrDebugFrameEntriesAsSymbols() {
  const std::unique_ptr<llvm::DWARFContext> dwarf_context =
      llvm::DWARFContext::create(*object_file_);
  constexpr const char* kErrorMessagePrefix =
      "Unable to load unwind info ranges from the .debug_frame or the .eh_frame section: ";
  if (dwarf_context == nullptr) {
    return ErrorMessage{absl::StrCat(kErrorMessagePrefix, "could not create DWARFContext.")};
  }

  const llvm::DWARFDebugFrame* debug_or_eh_frame = nullptr;
  bool is_eh_frame = false;

  // Try .debug_frame first, since it contains the most specific unwind information.
  if (llvm::Expected<const llvm::DWARFDebugFrame*> debug_frame = dwarf_context->getDebugFrame();
      debug_frame && !(*debug_frame)->empty()) {
    debug_or_eh_frame = *debug_frame;
  } else if (llvm::Expected<const llvm::DWARFDebugFrame*> eh_frame = dwarf_context->getEHFrame();
             eh_frame && !(*eh_frame)->empty()) {
    debug_or_eh_frame = *eh_frame;
    is_eh_frame = true;
  } else {
    return ErrorMessage{
        absl::StrCat(kErrorMessagePrefix, "no .debug_frame or .eh_frame section found.")};
  }
  ORBIT_CHECK(debug_or_eh_frame != nullptr);

  // TODO(b/244411070): This is no longer necessary from LLVM 13, which fixed
  //  https://bugs.llvm.org/show_bug.cgi?id=46414 with https://reviews.llvm.org/D100328.
  uint64_t eh_frame_address{};
  if (is_eh_frame && debug_or_eh_frame->getEHFrameAddress() == 0) {
    for (const llvm::object::SectionRef& section : object_file_->sections()) {
      llvm::Expected<llvm::StringRef> section_name = section.getName();
      if (!section_name) continue;
      // LLVM applies this logic to remove prefixes of section names before matching them to known
      // section names, so we do the same.
      std::string section_name_without_prefix = section_name->str();
      if (size_t prefix_start = section_name_without_prefix.find_first_not_of("._z");
          prefix_start < section_name_without_prefix.size()) {
        section_name_without_prefix = section_name_without_prefix.substr(prefix_start);
      }
      if (section_name_without_prefix == "eh_frame") {
        eh_frame_address = section.getAddress();
        break;
      }
    }
  }

  const absl::flat_hash_set<uint64_t> hotpachable_addresses = LoadHotpatchableAddresses();
  ModuleSymbols module_symbols;
  for (const llvm::dwarf::FrameEntry& entry : *debug_or_eh_frame) {
    // We are only interested in Frame Descriptor Entries (skip Common Information Entries).
    if (entry.getKind() != llvm::dwarf::FrameEntry::FK_FDE) continue;
    const auto& fde = static_cast<const llvm::dwarf::FDE&>(entry);

    uint64_t address = fde.getInitialLocation();
    // TODO(b/244411070): This is no longer necessary from LLVM 13, which fixed
    //  https://bugs.llvm.org/show_bug.cgi?id=46414 with https://reviews.llvm.org/D100328.
    if (is_eh_frame && debug_or_eh_frame->getEHFrameAddress() == 0 &&
        (fde.getLinkedCIE()->getFDEPointerEncoding() & 0x70) == llvm::dwarf::DW_EH_PE_pcrel) {
      address += eh_frame_address;
    }

    // Note that the DWARF specification says: "If the range of code addresses for a function is not
    // contiguous, there may be multiple CIEs and FDEs corresponding to the parts of that function."
    // In such a case, we will produce a separate symbol for each range, but there is not much we
    // can do about it.
    SymbolInfo* symbol_info = module_symbols.add_symbol_infos();
    // We assign an arbitrary function name, as we want a non-empty and unique name in many places.
    symbol_info->set_demangled_name(absl::StrFormat("[function@%#x]", address));
    symbol_info->set_address(address);
    symbol_info->set_size(fde.getAddressRange());
    symbol_info->set_is_hotpatchable(IsHotpatchable(hotpachable_addresses, address));
  }

  if (module_symbols.symbol_infos().empty()) {
    return ErrorMessage{
        absl::StrCat(kErrorMessagePrefix, "not even a single address range found.")};
  }
  return module_symbols;
}

template <typename ElfT>
ErrorMessageOr<ModuleSymbols>
ElfFileImpl<ElfT>::LoadDynamicLinkingSymbolsAndUnwindRangesAsSymbols() {
  ErrorMessageOr<ModuleSymbols> dynamic_linking_symbols = LoadSymbolsFromDynsym();
  ErrorMessageOr<ModuleSymbols> unwind_ranges_as_symbols = LoadEhOrDebugFrameEntriesAsSymbols();
  if (!dynamic_linking_symbols.has_value() && !unwind_ranges_as_symbols.has_value()) {
    return ErrorMessage{absl::StrFormat("Unable to load fallback symbols: %s %s",
                                        dynamic_linking_symbols.error().message(),
                                        unwind_ranges_as_symbols.error().message())};
  }
  ModuleSymbols dynamic_linking_symbols_and_unwind_ranges_as_symbols;

  absl::flat_hash_set<uint64_t> dynamic_linking_addresses;
  if (dynamic_linking_symbols.has_value()) {
    for (SymbolInfo& symbol_info : *dynamic_linking_symbols.value().mutable_symbol_infos()) {
      dynamic_linking_addresses.insert(symbol_info.address());
      *dynamic_linking_symbols_and_unwind_ranges_as_symbols.add_symbol_infos() =
          std::move(symbol_info);
    }
  }

  if (unwind_ranges_as_symbols.has_value()) {
    for (SymbolInfo& symbol_info : *unwind_ranges_as_symbols.value().mutable_symbol_infos()) {
      if (dynamic_linking_addresses.contains(symbol_info.address())) {
        continue;
      }
      *dynamic_linking_symbols_and_unwind_ranges_as_symbols.add_symbol_infos() =
          std::move(symbol_info);
    }
  }

  return dynamic_linking_symbols_and_unwind_ranges_as_symbols;
}

template <typename ElfT>
uint64_t ElfFileImpl<ElfT>::GetLoadBias() const {
  return load_bias_;
}

template <typename ElfT>
ErrorMessageOr<void> ElfFileImpl<ElfT>::InitProgramHeaders() {
  const llvm::object::ELFFile<ElfT>& elf_file = object_file_->getELFFile();
  llvm::Expected<typename ElfT::PhdrRange> range = elf_file.program_headers();

  if (!range) {
    std::string error = absl::StrFormat(
        "Unable to get load bias of ELF file: \"%s\". Error loading program headers: %s",
        file_path_.string(), llvm::toString(range.takeError()));
    ORBIT_ERROR("%s", error);
    return ErrorMessage(std::move(error));
  }

  std::optional<uint64_t> first_loadable_segment_vaddr;
  for (const typename ElfT::Phdr& phdr : range.get()) {
    if (phdr.p_type != llvm::ELF::PT_LOAD) {
      continue;
    }

    orbit_grpc_protos::ModuleInfo::ObjectSegment& object_segment =
        loadable_segments_.emplace_back();
    object_segment.set_offset_in_file(phdr.p_offset);
    object_segment.set_size_in_file(phdr.p_filesz);
    object_segment.set_address(phdr.p_vaddr);
    object_segment.set_size_in_memory(phdr.p_memsz);

    // Compute image_size_ as the difference between the end address of the last loadable segment
    // and the start address of the first loadable segment. This is as defined by
    // ObjectFile::GetImageSize and follows SizeOfImage of PEs; however, it can be changed if
    // needed.
    if (!first_loadable_segment_vaddr.has_value()) {
      first_loadable_segment_vaddr = phdr.p_vaddr;
    }
    image_size_ =
        std::max(image_size_, phdr.p_vaddr + phdr.p_memsz - first_loadable_segment_vaddr.value());
  }

  // Find the executable segment and calculate the load bias based on that segment.
  for (const typename ElfT::Phdr& phdr : range.get()) {
    if (phdr.p_type != llvm::ELF::PT_LOAD) {
      continue;
    }

    if ((phdr.p_flags & llvm::ELF::PF_X) == 0) {
      continue;
    }

    load_bias_ = phdr.p_vaddr - phdr.p_offset;
    executable_segment_offset_ = phdr.p_offset;
    executable_segment_size_ = phdr.p_memsz;
    return outcome::success();
  }

  std::string error = absl::StrFormat(
      "Unable to get load bias of ELF file: \"%s\". No executable PT_LOAD segment found.",
      file_path_.string());
  ORBIT_ERROR("%s", error);
  return ErrorMessage(std::move(error));
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::HasDebugSymbols() const {
  return has_symtab_section_;
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::HasDynsym() const {
  return has_dynsym_section_;
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::HasDebugInfo() const {
  return has_debug_info_section_;
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::HasGnuDebuglink() const {
  return gnu_debuglink_info_.has_value();
}

template <typename ElfT>
std::string ElfFileImpl<ElfT>::GetBuildId() const {
  return build_id_;
}

template <typename ElfT>
std::string ElfFileImpl<ElfT>::GetName() const {
  return soname_.empty() ? std::filesystem::path{file_path_}.filename().string() : soname_;
}

template <typename ElfT>
std::string ElfFileImpl<ElfT>::GetSoname() const {
  return soname_;
}

template <typename ElfT>
const std::filesystem::path& ElfFileImpl<ElfT>::GetFilePath() const {
  return file_path_;
}

template <typename ElfT>
ErrorMessageOr<LineInfo> orbit_object_utils::ElfFileImpl<ElfT>::GetLineInfo(uint64_t address) {
  ORBIT_CHECK(has_debug_info_section_);
  auto line_info_or_error =
      symbolizer_.symbolizeInlinedCode(std::string{object_file_->getFileName()},
                                       {address, llvm::object::SectionedAddress::UndefSection});
  if (!line_info_or_error) {
    return ErrorMessage(
        absl::StrFormat("Unable to get line number info for \"%s\", address=0x%x: %s",
                        std::string{object_file_->getFileName()}, address,
                        llvm::toString(line_info_or_error.takeError())));
  }

  auto& symbolizer_line_info = line_info_or_error.get();
  const uint32_t number_of_frames = symbolizer_line_info.getNumberOfFrames();

  // Getting back zero frames means there was some kind of problem. We will return a error.
  if (number_of_frames == 0) {
    return ErrorMessage(absl::StrFormat("Unable to get line info for address=0x%x", address));
  }

  const auto& last_frame = symbolizer_line_info.getFrame(number_of_frames - 1);

  // This is what symbolizer returns in case of an error. We convert it to a ErrorMessage here.
  if (last_frame.FileName == "<invalid>" && last_frame.Line == 0) {
    return ErrorMessage(absl::StrFormat("Unable to get line info for address=0x%x", address));
  }

  LineInfo line_info;
  line_info.set_source_file(last_frame.FileName);
  line_info.set_source_line(last_frame.Line);
  return line_info;
}

template <typename ElfT>
ErrorMessageOr<LineInfo> orbit_object_utils::ElfFileImpl<ElfT>::GetDeclarationLocationOfFunction(
    uint64_t address) {
  const auto dwarf_context = llvm::DWARFContext::create(*owning_binary_.getBinary());
  if (dwarf_context == nullptr) return ErrorMessage{"Could not read DWARF information."};

  const auto offset = dwarf_context->getDebugAranges()->findAddress(address);
  auto* const compile_unit = dwarf_context->getCompileUnitForOffset(offset);
  if (compile_unit == nullptr) return ErrorMessage{"Invalid address"};

  const auto subroutine = compile_unit->getSubroutineForAddress(address);
  if (!subroutine.isValid()) return ErrorMessage{"Address not associated with any subroutine"};

  const auto decl_file_index =
      llvm::dwarf::toUnsigned(subroutine.findRecursively(llvm::dwarf::Attribute::DW_AT_decl_file));
  if (!decl_file_index) return ErrorMessage{"Could not find source file location"};

  const llvm::DWARFDebugLine::LineTable* const line_table =
      dwarf_context->getLineTableForUnit(compile_unit);
  if (line_table == nullptr) return ErrorMessage{"Line Table was missing in debug information"};

  std::string file_path;
  const auto result = line_table->getFileNameByIndex(
      *decl_file_index, compile_unit->getCompilationDir(),
      llvm::DILineInfoSpecifier::FileLineInfoKind::AbsoluteFilePath, file_path);
  if (!result) return ErrorMessage{"Source declaration file path not found in debug information."};

  LineInfo line_info{};
  line_info.set_source_line(subroutine.getDeclLine());
  line_info.set_source_file(file_path);
  return line_info;
}

template <typename ElfT>
ErrorMessageOr<LineInfo> orbit_object_utils::ElfFileImpl<ElfT>::GetLocationOfFunction(
    uint64_t address) {
  ErrorMessageOr<LineInfo> declaration_location = GetDeclarationLocationOfFunction(address);
  if (declaration_location.has_value()) return declaration_location;

  // If the DWARF information doesn't contain a DECL_FILE and DECL_LINE entry we will fall back to
  // determining the beginning of the function through the location of the first line. This is not
  // ideal because it won't point to the function header but better than refusing to show source
  // code.
  return GetLineInfo(address);
}

template <>
bool ElfFileImpl<llvm::object::ELF64LE>::Is64Bit() const {
  return true;
}

template <>
bool ElfFileImpl<llvm::object::ELF32LE>::Is64Bit() const {
  return false;
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::IsCoff() const {
  return false;
}

template <typename ElfT>
bool ElfFileImpl<ElfT>::IsElf() const {
  return true;
}

template <typename ElfT>
std::optional<GnuDebugLinkInfo> ElfFileImpl<ElfT>::GetGnuDebugLinkInfo() const {
  return gnu_debuglink_info_;
}

template <typename ElfT>
uint64_t ElfFileImpl<ElfT>::GetExecutableSegmentOffset() const {
  return executable_segment_offset_;
}

template <typename ElfT>
uint64_t ElfFileImpl<ElfT>::GetImageSize() const {
  return image_size_;
}

template <typename ElfT>
const std::vector<orbit_grpc_protos::ModuleInfo::ObjectSegment>&
ElfFileImpl<ElfT>::GetObjectSegments() const {
  return loadable_segments_;
}
}  // namespace

ErrorMessageOr<std::unique_ptr<ElfFile>> CreateElfFileFromBuffer(
    const std::filesystem::path& file_path, const void* buf, size_t len) {
  std::unique_ptr<llvm::MemoryBuffer> buffer = llvm::MemoryBuffer::getMemBuffer(
      llvm::StringRef(static_cast<const char*>(buf), len), llvm::StringRef("buffer name"), false);
  llvm::Expected<std::unique_ptr<llvm::object::ObjectFile>> object_file_or_error =
      llvm::object::ObjectFile::createObjectFile(buffer->getMemBufferRef());

  if (!object_file_or_error) {
    return ErrorMessage(absl::StrFormat("Unable to load ELF file \"%s\": %s", file_path.string(),
                                        llvm::toString(object_file_or_error.takeError())));
  }

  return CreateElfFile(file_path, llvm::object::OwningBinary<llvm::object::ObjectFile>(
                                      std::move(object_file_or_error.get()), std::move(buffer)));
}

ErrorMessageOr<std::unique_ptr<ElfFile>> CreateElfFile(const std::filesystem::path& file_path) {
  ORBIT_SCOPE_FUNCTION;
  llvm::Expected<llvm::object::OwningBinary<llvm::object::ObjectFile>> object_file_or_error =
      llvm::object::ObjectFile::createObjectFile(file_path.string());

  if (!object_file_or_error) {
    return ErrorMessage(absl::StrFormat("Unable to load ELF file \"%s\": %s", file_path.string(),
                                        llvm::toString(object_file_or_error.takeError())));
  }

  llvm::object::OwningBinary<llvm::object::ObjectFile>& file = object_file_or_error.get();

  return CreateElfFile(file_path, std::move(file));
}

ErrorMessageOr<std::unique_ptr<ElfFile>> CreateElfFile(
    const std::filesystem::path& file_path,
    llvm::object::OwningBinary<llvm::object::ObjectFile>&& file) {
  llvm::object::ObjectFile* object_file = file.getBinary();

  // Create appropriate ElfFile implementation
  if (llvm::dyn_cast<llvm::object::ELF32LEObjectFile>(object_file) != nullptr) {
    auto elf_file =
        std::make_unique<ElfFileImpl<llvm::object::ELF32LE>>(file_path, std::move(file));
    OUTCOME_TRY(elf_file->Initialize());
    return elf_file;
  }

  if (llvm::dyn_cast<llvm::object::ELF64LEObjectFile>(object_file) != nullptr) {
    auto elf_file =
        std::make_unique<ElfFileImpl<llvm::object::ELF64LE>>(file_path, std::move(file));
    OUTCOME_TRY(elf_file->Initialize());
    return elf_file;
  }

  return ErrorMessage(absl::StrFormat(
      "Unable to load \"%s\": Big-endian architectures are not supported.", file_path.string()));
}

ErrorMessageOr<uint32_t> ElfFile::CalculateDebuglinkChecksum(
    const std::filesystem::path& file_path) {
  ErrorMessageOr<orbit_base::unique_fd> fd_or_error = orbit_base::OpenFileForReading(file_path);

  if (fd_or_error.has_error()) {
    return fd_or_error.error();
  }

  constexpr size_t kBufferSize = 4 * 1024 * 1024;  // 4 MiB
  std::vector<unsigned char> buffer(kBufferSize);

  uint32_t rolling_checksum = 0;

  while (true) {
    ErrorMessageOr<size_t> chunksize_or_error =
        orbit_base::ReadFully(fd_or_error.value(), buffer.data(), buffer.size());
    if (chunksize_or_error.has_error()) return chunksize_or_error.error();

    if (chunksize_or_error.value() == 0) break;

    const llvm::ArrayRef<unsigned char> str{buffer.data(), chunksize_or_error.value()};
    rolling_checksum = llvm::crc32(rolling_checksum, str);
  }

  return rolling_checksum;
}

}  // namespace orbit_object_utils
