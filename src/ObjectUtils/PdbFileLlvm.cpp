// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PdbFileLlvm.h"

#include <absl/memory/memory.h>
#include <absl/strings/str_cat.h>
#include <llvm/DebugInfo/CodeView/LazyRandomTypeCollection.h>
#include <llvm/DebugInfo/CodeView/TypeDeserializer.h>
#include <llvm/DebugInfo/CodeView/TypeRecord.h>
#include <llvm/DebugInfo/PDB/Native/TpiStream.h>

#include <memory>

#include "Introspection/Introspection.h"
#include "ObjectUtils/CoffFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

namespace orbit_object_utils {

namespace {

// Codeview debug records from a PDB file can be accessed through llvm using a visitor
// interface (using llvm::codeview::CVSymbolVisitor::visitSymbolStream). This can be
// customized by implementing one's own visitor class, which we do here to fill out
// all symbol info required for functions.
class SymbolInfoVisitor : public llvm::codeview::SymbolVisitorCallbacks {
 public:
  SymbolInfoVisitor(DebugSymbols* debug_symbols, const ObjectFileInfo& object_file_info,
                    llvm::pdb::TpiStream* type_info_stream)
      : debug_symbols_(debug_symbols),
        object_file_info_(object_file_info),
        type_info_stream_(type_info_stream) {
    ORBIT_CHECK(debug_symbols != nullptr);
    ORBIT_CHECK(type_info_stream != nullptr);
  }

  // This is the only record type (ProcSym) we are interested in, so we only override this
  // method. Other records will simply return llvm::Error::success without any work done.
  llvm::Error visitKnownRecord(llvm::codeview::CVSymbol& /*unused*/,
                               llvm::codeview::ProcSym& proc) override {
    FunctionSymbol& function_symbol = debug_symbols_->function_symbols.emplace_back();
    function_symbol.demangled_name = proc.Name.str();

    // The ProcSym's name does not contain an argument list. However, this information is required
    // when dealing with overloads and it is available in the type info stream. See:
    // https://llvm.org/docs/PDB/TpiStream.html
    llvm::StringRef argument_list = RetrieveArgumentList(proc);
    if (!argument_list.empty()) {
      function_symbol.demangled_name.append(argument_list.data());
    }

    // The address in PDB files is a relative virtual address (RVA), to make the address compatible
    // with how we do the computation, we need to add both the load bias (ImageBase for COFF) and
    // the offset of the executable section.
    function_symbol.address =
        proc.CodeOffset + object_file_info_.load_bias + object_file_info_.executable_segment_offset;
    function_symbol.size = proc.CodeSize;

    ORBIT_CHECK(debug_symbols_ != nullptr);

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

  DebugSymbols* debug_symbols_;
  ObjectFileInfo object_file_info_;
  llvm::pdb::TpiStream* type_info_stream_;
};

}  // namespace

PdbFileLlvm::PdbFileLlvm(std::filesystem::path file_path, const ObjectFileInfo& object_file_info,
                         std::unique_ptr<llvm::pdb::IPDBSession> session)
    : file_path_(std::move(file_path)),
      object_file_info_(object_file_info),
      session_(std::move(session)) {}

ErrorMessageOr<DebugSymbols> PdbFileLlvm::LoadRawDebugSymbols() {
  ORBIT_SCOPE_FUNCTION;
  DebugSymbols debug_symbols;
  debug_symbols.symbols_file_path = file_path_.string();
  debug_symbols.load_bias = object_file_info_.load_bias;

  auto* native_session = dynamic_cast<llvm::pdb::NativeSession*>(session_.get());
  ORBIT_CHECK(native_session != nullptr);
  llvm::pdb::PDBFile& pdb_file = native_session->getPDBFile();

  if (!pdb_file.hasPDBDbiStream()) {
    return ErrorMessage("PDB file does not have a DBI stream.");
  }

  llvm::Expected<llvm::pdb::DbiStream&> debug_info_stream = pdb_file.getPDBDbiStream();
  // Given that we check hasPDBDbiStream above, we must get a DbiStream here.
  ORBIT_CHECK(debug_info_stream);

  if (!pdb_file.hasPDBTpiStream()) {
    return ErrorMessage("PDB file does not have a TPI stream.");
  }
  llvm::Expected<llvm::pdb::TpiStream&> type_info_stream = pdb_file.getPDBTpiStream();
  // Given that we check hasPDBTpiStream above, we must get a TpiStream here.
  ORBIT_CHECK(type_info_stream);

  const llvm::pdb::DbiModuleList& modules = debug_info_stream->modules();

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
    SymbolInfoVisitor symbol_visitor(&debug_symbols, object_file_info_, &type_info_stream.get());
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
  return debug_symbols;
}

ErrorMessageOr<std::unique_ptr<PdbFile>> PdbFileLlvm::CreatePdbFile(
    const std::filesystem::path& file_path, const ObjectFileInfo& object_file_info) {
  std::string file_path_string = file_path.string();
  llvm::StringRef pdb_path{file_path_string};
  std::unique_ptr<llvm::pdb::IPDBSession> session;
  llvm::Error error =
      llvm::pdb::loadDataForPDB(llvm::pdb::PDB_ReaderType::Native, pdb_path, session);
  if (error) {
    return ErrorMessage(absl::StrFormat("Unable to load PDB file %s with error: %s",
                                        file_path.string(), llvm::toString(std::move(error))));
  }
  return absl::WrapUnique<PdbFileLlvm>(
      new PdbFileLlvm(file_path, object_file_info, std::move(session)));
}

}  // namespace orbit_object_utils
