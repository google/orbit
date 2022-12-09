// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
#ifdef _MSC_VER
#pragma warning(disable : 4996)
#endif

#include "PdbFileLlvm.h"

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/hash/hash.h>
#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <absl/strings/str_format.h>
#include <llvm/ADT/StringRef.h>
#include <llvm/ADT/iterator.h>
#include <llvm/DebugInfo/CodeView/CVRecord.h>
#include <llvm/DebugInfo/CodeView/CVSymbolVisitor.h>
#include <llvm/DebugInfo/CodeView/CodeView.h>
#include <llvm/DebugInfo/CodeView/GUID.h>
#include <llvm/DebugInfo/CodeView/LazyRandomTypeCollection.h>
#include <llvm/DebugInfo/CodeView/SymbolDeserializer.h>
#include <llvm/DebugInfo/CodeView/SymbolRecord.h>
#include <llvm/DebugInfo/CodeView/SymbolVisitorCallbackPipeline.h>
#include <llvm/DebugInfo/CodeView/SymbolVisitorCallbacks.h>
#include <llvm/DebugInfo/CodeView/TypeDeserializer.h>
#include <llvm/DebugInfo/CodeView/TypeIndex.h>
#include <llvm/DebugInfo/CodeView/TypeRecord.h>
#include <llvm/DebugInfo/MSF/MappedBlockStream.h>
#include <llvm/DebugInfo/PDB/Native/DbiModuleDescriptor.h>
#include <llvm/DebugInfo/PDB/Native/DbiModuleList.h>
#include <llvm/DebugInfo/PDB/Native/DbiStream.h>
#include <llvm/DebugInfo/PDB/Native/GlobalsStream.h>
#include <llvm/DebugInfo/PDB/Native/ISectionContribVisitor.h>
#include <llvm/DebugInfo/PDB/Native/ModuleDebugStream.h>
#include <llvm/DebugInfo/PDB/Native/NativeSession.h>
#include <llvm/DebugInfo/PDB/Native/PDBFile.h>
#include <llvm/DebugInfo/PDB/Native/PublicsStream.h>
#include <llvm/DebugInfo/PDB/Native/RawConstants.h>
#include <llvm/DebugInfo/PDB/Native/RawTypes.h>
#include <llvm/DebugInfo/PDB/Native/SymbolStream.h>
#include <llvm/DebugInfo/PDB/Native/TpiStream.h>
#include <llvm/DebugInfo/PDB/PDB.h>
#include <llvm/DebugInfo/PDB/PDBSymbolExe.h>
#include <llvm/DebugInfo/PDB/PDBTypes.h>
#include <llvm/Demangle/Demangle.h>
#include <llvm/Object/COFF.h>
#include <llvm/Support/BinaryStreamArray.h>
#include <llvm/Support/BinaryStreamRef.h>
#include <llvm/Support/Endian.h>
#include <llvm/Support/Error.h>

#include <algorithm>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "GrpcProtos/symbol.pb.h"
#include "ObjectUtils/ObjectFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_object_utils {

namespace {

[[nodiscard]] uint64_t ComputeAddress(
    uint64_t offset_in_section, uint16_t section, uint64_t image_base,
    const llvm::FixedStreamArray<llvm::object::coff_section>& section_headers) {
  // Unlike DIA, LLVM won't give us the RVA directly, but the symbol's offset in the respective
  // section. We can compute the RVA as the section's RVA + the symbol's offset.
  // Note: The segments are numbered starting at 1 and match what you observe using
  // `dumpbin /HEADERS`.
  ORBIT_CHECK(section_headers.size() >= section && section > 0);
  uint64_t section_rva = section_headers[section - 1].VirtualAddress;
  uint64_t rva = offset_in_section + section_rva;

  // To get the address we use in Orbit, we add the object's "image base" to the RVA.
  // The loader might choose a different image base when actually loading the object file at runtime
  // and thus, the virtual address might differ from the address we compute here.
  // See https://docs.microsoft.com/en-us/windows/win32/debug/pe-format#general-concepts.
  return rva + image_base;
}

// Codeview debug records from a PDB file can be accessed through llvm using a visitor
// interface (using llvm::codeview::CVSymbolVisitor::visitSymbolStream). This can be
// customized by implementing one's own visitor class, which we do here to fill out
// all symbol info required for functions.
class SymbolInfoVisitor : public llvm::codeview::SymbolVisitorCallbacks {
 public:
  SymbolInfoVisitor(std::vector<SymbolInfo>* symbol_infos,
                    absl::flat_hash_set<uint64_t>* addresses_from_module_debug_stream,
                    const ObjectFileInfo& object_file_info,
                    llvm::FixedStreamArray<llvm::object::coff_section>* section_headers,
                    llvm::pdb::TpiStream* type_info_stream)
      : symbol_infos_(symbol_infos),
        addresses_from_module_debug_stream_(addresses_from_module_debug_stream),
        object_file_info_(object_file_info),
        section_headers_(section_headers),
        type_info_stream_(type_info_stream) {
    ORBIT_CHECK(symbol_infos != nullptr);
    ORBIT_CHECK(type_info_stream != nullptr);
  }

  // This is the only record type (ProcSym) we are interested in, so we only override this
  // method. Other records will simply return llvm::Error::success without any work done.
  llvm::Error visitKnownRecord(llvm::codeview::CVSymbol& /*unused*/,
                               llvm::codeview::ProcSym& proc) override {
    SymbolInfo symbol_info;
    symbol_info.set_demangled_name(llvm::demangle(proc.Name.str()));

    // The ProcSym's name does not contain an argument list. However, this information is required
    // when dealing with overloads and it is available in the type info stream. See:
    // https://llvm.org/docs/PDB/TpiStream.html
    llvm::StringRef argument_list = RetrieveArgumentList(proc);
    if (!argument_list.empty()) {
      symbol_info.set_demangled_name(
          absl::StrCat(symbol_info.demangled_name(), argument_list.data()));
    }

    uint64_t address = ComputeAddress(proc.CodeOffset, proc.Segment, object_file_info_.load_bias,
                                      *section_headers_);
    symbol_info.set_address(address);
    symbol_info.set_size(proc.CodeSize);
    // We currently only support hotpatchable functions in elf files.
    symbol_info.set_is_hotpatchable(false);

    ORBIT_CHECK(addresses_from_module_debug_stream_ != nullptr);
    addresses_from_module_debug_stream_->insert(symbol_info.address());
    ORBIT_CHECK(symbol_infos_ != nullptr);
    symbol_infos_->emplace_back(std::move(symbol_info));

    return llvm::Error::success();
  }

 private:
  [[nodiscard]] llvm::StringRef RetrieveArgumentList(const llvm::codeview::ProcSym& proc) const {
    ORBIT_CHECK(type_info_stream_ != nullptr);
    llvm::codeview::LazyRandomTypeCollection& type_collection = type_info_stream_->typeCollection();

    // We expect function types being either LF_PROCEDURE or LF_MFUNCTION, which are non-simple
    // types. However, there are cases where the function type is "<no type>", which is a simple
    // type. In those cases, we can't retrieve the argument list. Other simple types are not
    // expected here (as they are mostly base types). However, the call to `getType` below will fail
    // on any simple type. So we check for all simple types here, instead of only for "<no type>".
    if (proc.FunctionType.isSimple()) {
      llvm::StringRef function_type = type_collection.getTypeName(proc.FunctionType);
      ORBIT_ERROR(
          "Unable to retrieve parameter list for function \"%s\"; The function type is \"%s\"",
          proc.Name.data(), function_type.data());
      return "";
    }

    llvm::codeview::CVType function_type = type_info_stream_->getType(proc.FunctionType);
    switch (function_type.kind()) {
      case llvm::codeview::LF_PROCEDURE: {
        llvm::codeview::ProcedureRecord procedure_record;
        llvm::Error error =
            llvm::codeview::TypeDeserializer::deserializeAs<llvm::codeview::ProcedureRecord>(
                function_type, procedure_record);
        if (error) {
          ORBIT_ERROR(
              "Unable to retrieve parameter list for function \"%s\"; The function is of type "
              "\"LF_PROCEDURE\", but we can not deserialize it to a \"ProcedureRecord\".",
              proc.Name.data());
          return "";
        }

        llvm::StringRef parameter_list = type_collection.getTypeName(procedure_record.ArgumentList);
        return parameter_list;
      }
      case llvm::codeview::LF_MFUNCTION: {
        llvm::codeview::MemberFunctionRecord member_function_record;
        llvm::Error error =
            llvm::codeview::TypeDeserializer::deserializeAs<llvm::codeview::MemberFunctionRecord>(
                function_type, member_function_record);
        if (error) {
          ORBIT_ERROR(
              "Unable to retrieve parameter list for function \"%s\"; The function is of type "
              "\"LF_MFUNCTION\", but we can not deserialize it to a \"MemberFunctionRecord\".",
              proc.Name.data());
          return "";
        }

        return type_collection.getTypeName(member_function_record.ArgumentList);
      }
      default:
        ORBIT_UNREACHABLE();
    }
  }

  std::vector<SymbolInfo>* symbol_infos_;
  absl::flat_hash_set<uint64_t>* addresses_from_module_debug_stream_;
  ObjectFileInfo object_file_info_;
  llvm::FixedStreamArray<llvm::object::coff_section>* section_headers_;
  llvm::pdb::TpiStream* type_info_stream_;
};

// This visitor will try to deduce the missing size information from the given symbol using
// the section contributions information.
// Unfortunately, this is performing a linear search on the contribution records, but LLVM
// does not offer a better way to access the information.
class SectionContributionsVisitor : public llvm::pdb::ISectionContribVisitor {
 public:
  SectionContributionsVisitor(
      const ObjectFileInfo& object_file_info,
      llvm::FixedStreamArray<llvm::object::coff_section>* section_headers,
      absl::flat_hash_map<uint64_t, std::vector<SymbolInfo*>>* address_to_symbols_with_missing_size)
      : object_file_info_(object_file_info),
        section_headers_(section_headers),
        address_to_symbols_with_missing_size_(address_to_symbols_with_missing_size) {}

  void visit(const llvm::pdb::SectionContrib& section_contrib) override {
    ORBIT_CHECK(section_headers_ != nullptr);
    uint64_t address = ComputeAddress(section_contrib.Off, section_contrib.ISect,
                                      object_file_info_.load_bias, *section_headers_);

    auto symbols_it = address_to_symbols_with_missing_size_->find(address);
    if (symbols_it != address_to_symbols_with_missing_size_->end()) {
      for (SymbolInfo* symbol_info : symbols_it->second) {
        ORBIT_CHECK(symbol_info != nullptr);
        ORBIT_CHECK(symbol_info->size() == ObjectFile::kUnknownSymbolSize);
        symbol_info->set_size(section_contrib.Size);
      }
    }
  }

  void visit(const llvm::pdb::SectionContrib2& section_contrib) override {
    visit(section_contrib.Base);
  }

 private:
  ObjectFileInfo object_file_info_;
  llvm::FixedStreamArray<llvm::object::coff_section>* section_headers_{};
  const absl::flat_hash_map<uint64_t, std::vector<SymbolInfo*>>*
      address_to_symbols_with_missing_size_{};
};

ErrorMessageOr<void> LoadDebugSymbolsFromModuleStreams(
    llvm::pdb::PDBFile& pdb_file, llvm::pdb::DbiStream& debug_info_stream,
    llvm::pdb::TpiStream& type_info_stream,
    llvm::FixedStreamArray<llvm::object::coff_section>& section_headers,
    const ObjectFileInfo& object_file_info, std::vector<SymbolInfo>* symbol_infos,
    absl::flat_hash_set<uint64_t>* addresses_from_module_debug_stream) {
  const llvm::pdb::DbiModuleList& modules = debug_info_stream.modules();

  for (uint32_t index = 0; index < modules.getModuleCount(); ++index) {
    auto modi = modules.getModuleDescriptor(index);
    uint16_t modi_stream_index = modi.getModuleStreamIndex();

    if (modi_stream_index == llvm::pdb::kInvalidStreamIndex) {
      continue;
    }

    std::unique_ptr<llvm::msf::MappedBlockStream> mod_stream_data =
        pdb_file.createIndexedStream(modi_stream_index);
    llvm::pdb::ModuleDebugStreamRef mod_debug_stream(modi, std::move(mod_stream_data));

    // This line is critical, otherwise the stream will not have any data.
    llvm::Error reload_error = mod_debug_stream.reload();
    if (reload_error) {
      return ErrorMessage{
          absl::StrFormat("Error trying to reload module debug stream with llvm error: %s",
                          llvm::toString(std::move(reload_error)))};
    }

    llvm::codeview::SymbolVisitorCallbackPipeline pipeline;
    llvm::codeview::SymbolDeserializer deserializer(nullptr,
                                                    llvm::codeview::CodeViewContainer::Pdb);
    pipeline.addCallbackToPipeline(deserializer);
    SymbolInfoVisitor symbol_visitor(symbol_infos, addresses_from_module_debug_stream,
                                     object_file_info, &section_headers, &type_info_stream);
    pipeline.addCallbackToPipeline(symbol_visitor);
    llvm::codeview::CVSymbolVisitor visitor(pipeline);

    llvm::BinarySubstreamRef symbol_substream = mod_debug_stream.getSymbolsSubstream();
    const llvm::codeview::CVSymbolArray& symbol_array = mod_debug_stream.getSymbolArray();

    // Not sure why it's necessary to pass the symbol stream offset here, but this is
    // following the implementation of llvm-pdbutil in llvm/tools.
    llvm::Error error = visitor.visitSymbolStream(symbol_array, symbol_substream.Offset);
    if (error) {
      return ErrorMessage{
          absl::StrFormat("Error while reading symbols from PDB debug info stream: %s",
                          llvm::toString(std::move(error)))};
    }
  }
  return outcome::success();
}

void LoadDebugSymbolsFromPublicSymbolStream(
    llvm::pdb::PublicsStream& public_symbol_stream, llvm::pdb::SymbolStream& symbol_stream,
    llvm::FixedStreamArray<llvm::object::coff_section>& section_headers,
    const ObjectFileInfo& object_file_info,
    const absl::flat_hash_set<uint64_t>& addresses_from_module_debug_stream,
    std::vector<SymbolInfo>* symbol_infos) {
  const llvm::pdb::GSIHashTable& public_symbol_has_records = public_symbol_stream.getPublicsTable();
  for (const auto& hash_record : public_symbol_has_records) {
    llvm::Expected<llvm::codeview::PublicSym32> record =
        llvm::codeview::SymbolDeserializer::deserializeAs<llvm::codeview::PublicSym32>(
            symbol_stream.readRecord(hash_record));
    ORBIT_CHECK(record);

    // Skip this symbol if it is not a function (but rather a global constant).
    if ((record->Flags & llvm::codeview::PublicSymFlags::Function) ==
        llvm::codeview::PublicSymFlags::None) {
      continue;
    }

    uint64_t address = ComputeAddress(record->Offset, record->Segment, object_file_info.load_bias,
                                      section_headers);

    if (addresses_from_module_debug_stream.contains(address)) continue;

    SymbolInfo symbol_info;
    symbol_info.set_address(address);
    symbol_info.set_demangled_name(llvm::demangle(record->Name.str()));
    // The PDB public symbols don't contain the size of symbols. Set a placeholder which indicates
    // that the size is unknown for now and try to deduce it later. We will later use that
    // placeholder to look-up the size in `SectionContributionsVisitor` or in
    // `DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol` (as a fallback).
    symbol_info.set_size(ObjectFile::kUnknownSymbolSize);
    // We currently only support hotpatchable functions in elf files.
    symbol_info.set_is_hotpatchable(false);

    symbol_infos->emplace_back(std::move(symbol_info));
  }
}

}  // namespace

PdbFileLlvm::PdbFileLlvm(std::filesystem::path file_path, const ObjectFileInfo& object_file_info,
                         std::unique_ptr<llvm::pdb::IPDBSession> session)
    : file_path_(std::move(file_path)),
      object_file_info_(object_file_info),
      session_(std::move(session)) {}

std::array<uint8_t, 16> PdbFileLlvm::GetGuid() const {
  constexpr int kGuidSize = 16;
  static_assert(kGuidSize == sizeof(llvm::codeview::GUID));
  std::array<uint8_t, kGuidSize> result{};
  auto global_scope = session_->getGlobalScope();
  const llvm::codeview::GUID& guid = global_scope->getGuid();
  std::copy(std::begin(guid.Guid), std::end(guid.Guid), std::begin(result));
  return result;
}

uint32_t PdbFileLlvm::GetAge() const {
  auto* native_session = dynamic_cast<llvm::pdb::NativeSession*>(session_.get());
  ORBIT_CHECK(native_session != nullptr);
  llvm::pdb::PDBFile& pdb_file = native_session->getPDBFile();
  ORBIT_CHECK(pdb_file.hasPDBDbiStream());
  llvm::Expected<llvm::pdb::DbiStream&> debug_info_stream = pdb_file.getPDBDbiStream();
  ORBIT_CHECK(debug_info_stream);

  return debug_info_stream->getAge();
}

[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> PdbFileLlvm::LoadDebugSymbols() {
  auto* native_session = dynamic_cast<llvm::pdb::NativeSession*>(session_.get());
  ORBIT_CHECK(native_session != nullptr);
  llvm::pdb::PDBFile& pdb_file = native_session->getPDBFile();

  if (!pdb_file.hasPDBDbiStream()) {
    return ErrorMessage("PDB file does not have a DBI stream.");
  }
  llvm::Expected<llvm::pdb::DbiStream&> debug_info_stream = pdb_file.getPDBDbiStream();
  ORBIT_CHECK(debug_info_stream);

  if (!pdb_file.hasPDBTpiStream()) {
    return ErrorMessage("PDB file does not have a TPI stream.");
  }
  llvm::Expected<llvm::pdb::TpiStream&> type_info_stream = pdb_file.getPDBTpiStream();
  ORBIT_CHECK(type_info_stream);

  llvm::FixedStreamArray<llvm::object::coff_section> section_headers =
      debug_info_stream->getSectionHeaders();

  std::vector<SymbolInfo> symbol_infos;
  absl::flat_hash_set<uint64_t> addresses_from_module_debug_stream;
  OUTCOME_TRY(LoadDebugSymbolsFromModuleStreams(
      pdb_file, debug_info_stream.get(), type_info_stream.get(), section_headers, object_file_info_,
      &symbol_infos, &addresses_from_module_debug_stream));

  if (!pdb_file.hasPDBPublicsStream()) {
    return ErrorMessage("PDB file does not have a public symbol stream.");
  }
  llvm::Expected<llvm::pdb::PublicsStream&> public_symbol_stream = pdb_file.getPDBPublicsStream();
  ORBIT_CHECK(public_symbol_stream);

  if (!pdb_file.hasPDBSymbolStream()) {
    return ErrorMessage("PDB file does not have a symbol stream.");
  }
  llvm::Expected<llvm::pdb::SymbolStream&> symbol_stream = pdb_file.getPDBSymbolStream();
  ORBIT_CHECK(symbol_stream);

  LoadDebugSymbolsFromPublicSymbolStream(public_symbol_stream.get(), symbol_stream.get(),
                                         section_headers, object_file_info_,
                                         addresses_from_module_debug_stream, &symbol_infos);

  // We try to find the missing size information from public symbols from the section contributions.
  // Note that we sometimes have multiple names for the same address, so we use a vector here as
  // the map's value type.
  absl::flat_hash_map<uint64_t, std::vector<SymbolInfo*>> address_to_symbols_with_missing_size{};
  for (SymbolInfo& symbol_info : symbol_infos) {
    if (symbol_info.size() != ObjectFile::kUnknownSymbolSize) {
      continue;
    }
    address_to_symbols_with_missing_size[symbol_info.address()].push_back(&symbol_info);
  }
  SectionContributionsVisitor visitor{object_file_info_, &section_headers,
                                      &address_to_symbols_with_missing_size};
  debug_info_stream->visitSectionContributions(visitor);

  // It does not seem to be guaranteed that we have section contribution information for all
  // symbols, so let's try to deduce the size of the missing symbols based on the distance from the
  // next symbol
  DeduceDebugSymbolMissingSizesAsDistanceFromNextSymbol(&symbol_infos);

  ModuleSymbols module_symbols;
  for (SymbolInfo& symbol_info : symbol_infos) {
    *(module_symbols.add_symbol_infos()) = std::move(symbol_info);
  }

  return module_symbols;
}

static bool PdbHasDbiStream(llvm::pdb::IPDBSession* session) {
  ORBIT_CHECK(session != nullptr);
  auto* native_session = dynamic_cast<llvm::pdb::NativeSession*>(session);
  ORBIT_CHECK(native_session != nullptr);
  llvm::pdb::PDBFile& pdb_file = native_session->getPDBFile();
  if (!pdb_file.hasPDBDbiStream()) {
    return false;
  }
  llvm::Expected<llvm::pdb::DbiStream&> debug_info_stream = pdb_file.getPDBDbiStream();
  if (!debug_info_stream) {
    return false;
  }
  return true;
}

ErrorMessageOr<std::unique_ptr<PdbFile>> PdbFileLlvm::CreatePdbFile(
    const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info) {
  std::string file_path_string = file_path.string();
  llvm::StringRef pdb_path{file_path_string};
  std::unique_ptr<llvm::pdb::IPDBSession> session;
  llvm::Error error =
      llvm::pdb::loadDataForPDB(llvm::pdb::PDB_ReaderType::Native, pdb_path, session);
  if (error) {
    return ErrorMessage(absl::StrFormat("Unable to load PDB file \"%s\": %s", file_path.string(),
                                        llvm::toString(std::move(error))));
  }

  // We need the debug info stream to retrieve the correct age information (which is used in the
  // build-id). See: https://github.com/llvm/llvm-project/issues/57300
  if (!PdbHasDbiStream(session.get())) {
    return ErrorMessage(absl::StrFormat("Unable to load PDB file \"%s\": PDB has no Dbi Stream.",
                                        file_path.string()));
  }

  return absl::WrapUnique<PdbFileLlvm>(
      new PdbFileLlvm(file_path, object_file_info, std::move(session)));
}

}  // namespace orbit_object_utils
