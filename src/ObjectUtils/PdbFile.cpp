// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ObjectUtils/PdbFile.h"

#include <llvm/DebugInfo/CodeView/CVSymbolVisitor.h>
#include <llvm/DebugInfo/CodeView/CodeView.h>
#include <llvm/DebugInfo/CodeView/GUID.h>
#include <llvm/DebugInfo/CodeView/SymbolDeserializer.h>
#include <llvm/DebugInfo/CodeView/SymbolVisitorCallbackPipeline.h>
#include <llvm/DebugInfo/CodeView/SymbolVisitorCallbacks.h>
#include <llvm/DebugInfo/MSF/MappedBlockStream.h>
#include <llvm/DebugInfo/PDB/Native/DbiStream.h>
#include <llvm/DebugInfo/PDB/Native/ModuleDebugStream.h>
#include <llvm/DebugInfo/PDB/Native/NativeSession.h>
#include <llvm/DebugInfo/PDB/Native/PDBFile.h>
#include <llvm/DebugInfo/PDB/PDB.h>
#include <llvm/DebugInfo/PDB/PDBSymbolExe.h>
#include <llvm/DebugInfo/PDB/PDBTypes.h>
#include <llvm/Demangle/Demangle.h>

#include <memory>

#include "ObjectUtils/CoffFile.h"
#include "ObjectUtils/WindowsBuildIdUtils.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "symbol.pb.h"

using orbit_grpc_protos::ModuleSymbols;
using orbit_grpc_protos::SymbolInfo;

namespace orbit_object_utils {

namespace {

// Codeview debug records from a PDB file can be accessed through llvm using a visitor
// interface (using llvm::codeview::CVSymbolVisitor::visitSymbolStream). This can be
// customized by implementing one's own visitor class, which we do here to fill out
// all symbol info required for functions.
class SymbolInfoVisitor : public llvm::codeview::SymbolVisitorCallbacks {
 public:
  SymbolInfoVisitor(ModuleSymbols* module_symbols, const ObjectFileInfo& object_file_info)
      : module_symbols_(module_symbols), object_file_info_(object_file_info) {}

  // This is the only record type (ProcSym) we are interested in, so we only override this
  // method. Other records will simply return llvm::Error::success without any work done.
  llvm::Error visitKnownRecord(llvm::codeview::CVSymbol& /*unused*/,
                               llvm::codeview::ProcSym& proc) override {
    SymbolInfo symbol_info;
    symbol_info.set_name(proc.Name.str());
    symbol_info.set_demangled_name(llvm::demangle(proc.Name.str()));
    // The address in PDB files is a relative virtual address (RVA), to make the address compatible
    // with how we do the computation, we need to add both the load bias (ImageBase for COFF) and
    // the offset of the executable section.
    symbol_info.set_address(proc.CodeOffset + object_file_info_.load_bias +
                            object_file_info_.executable_segment_offset);
    symbol_info.set_size(proc.CodeSize);
    *(module_symbols_->add_symbol_infos()) = std::move(symbol_info);

    return llvm::Error::success();
  }

 private:
  ModuleSymbols* module_symbols_;
  ObjectFileInfo object_file_info_;
};

class PdbFileImpl : public PdbFile {
 public:
  PdbFileImpl(std::filesystem::path file_path, const ObjectFileInfo& object_file_info,
              std::unique_ptr<llvm::pdb::IPDBSession> session);

  [[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> LoadDebugSymbols() override;
  [[nodiscard]] const std::filesystem::path& GetFilePath() const override { return file_path_; }

  [[nodiscard]] std::array<uint8_t, 16> GetGuid() const override {
    constexpr int kGuidSize = 16;
    std::array<uint8_t, kGuidSize> result;
    auto global_scope = session_->getGlobalScope();
    const llvm::codeview::GUID& guid = global_scope->getGuid();
    std::copy(std::begin(guid.Guid), std::end(guid.Guid), std::begin(result));
    return result;
  }

  [[nodiscard]] uint32_t GetAge() const override { return session_->getGlobalScope()->getAge(); }
  [[nodiscard]] std::string GetBuildId() const override {
    return ComputeWindowsBuildId(GetGuid(), GetAge());
  }

 private:
  std::filesystem::path file_path_;
  ObjectFileInfo object_file_info_;
  std::unique_ptr<llvm::pdb::IPDBSession> session_;
};

PdbFileImpl::PdbFileImpl(std::filesystem::path file_path, const ObjectFileInfo& object_file_info,
                         std::unique_ptr<llvm::pdb::IPDBSession> session)
    : file_path_(std::move(file_path)),
      object_file_info_(object_file_info),
      session_(std::move(session)) {}

[[nodiscard]] ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> PdbFileImpl::LoadDebugSymbols() {
  ModuleSymbols module_symbols;
  module_symbols.set_load_bias(object_file_info_.load_bias);
  module_symbols.set_symbols_file_path(file_path_.string());

  llvm::pdb::NativeSession* native_session = static_cast<llvm::pdb::NativeSession*>(session_.get());
  llvm::pdb::PDBFile& pdb_file = native_session->getPDBFile();

  if (!pdb_file.hasPDBDbiStream()) {
    return ErrorMessage("PDB file does not have a DBI stream.");
  }

  llvm::Expected<llvm::pdb::DbiStream&> debug_info_stream = pdb_file.getPDBDbiStream();
  // Given that we check hasPDBDbiStream above, we must get a DbiStream here.
  CHECK(debug_info_stream);

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
    SymbolInfoVisitor symbol_visitor(&module_symbols, object_file_info_);
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
  return module_symbols;
}

}  // namespace

ErrorMessageOr<std::unique_ptr<PdbFile>> CreatePdbFile(const std::filesystem::path& file_path,
                                                       const ObjectFileInfo& object_file_info) {
  std::string file_path_string = file_path.string();
  llvm::StringRef pdb_path{file_path_string};
  std::unique_ptr<llvm::pdb::IPDBSession> session;
  llvm::Error error =
      llvm::pdb::loadDataForPDB(llvm::pdb::PDB_ReaderType::Native, pdb_path, session);
  if (error) {
    return ErrorMessage(absl::StrFormat("Unable to load PDB file %s with error: %s",
                                        file_path.string(), llvm::toString(std::move(error))));
  }
  return std::make_unique<PdbFileImpl>(file_path, object_file_info, std::move(session));
}
}  // namespace orbit_object_utils