// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "MizarData/MizarData.h"

#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>

#include <QString>
#include <QStringLiteral>
#include <filesystem>
#include <memory>
#include <string>
#include <string_view>
#include <utility>

#include "ClientData/CallstackData.h"
#include "ClientData/CallstackInfo.h"
#include "ClientData/ModuleAndFunctionLookup.h"
#include "ClientData/ModuleData.h"
#include "ClientData/ModuleManager.h"
#include "ClientData/ProcessData.h"
#include "ClientData/ScopeInfo.h"
#include "ClientData/ThreadTrackDataProvider.h"
#include "ClientSymbols/QSettingsBasedStorageManager.h"
#include "GrpcProtos/symbol.pb.h"
#include "MizarBase/AbsoluteAddress.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/Typedef.h"

using ::orbit_mizar_base::AbsoluteAddress;
using ::orbit_mizar_base::ForEachFrame;
using ::orbit_mizar_base::FunctionSymbol;

namespace orbit_mizar_data {

absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> MizarData::AllAddressToFunctionSymbol() const {
  absl::flat_hash_map<AbsoluteAddress, FunctionSymbol> result;

  GetCaptureData().GetCallstackData().ForEachUniqueCallstack(
      [&result, this](const uint64_t /*callstack_id*/,
                      const orbit_client_data::CallstackInfo& info) {
        ForEachFrame(info.frames(), [this, &result](const AbsoluteAddress address) {
          std::optional<std::string> name = this->GetFunctionNameFromAddress(address);
          if (name.has_value()) {
            std::string module_file_name = GetModuleFilenameWithoutExtension(address);
            result.try_emplace(
                address, FunctionSymbol{std::move(name.value()), std::move(module_file_name)});
          }
        });
      });

  return result;
}

[[nodiscard]] static std::string GetFilenameWithoutExtension(std::string_view path) {
  return std::filesystem::path(path)
      .filename()
      .replace_extension()  // remove extension, so `app.exe` on Windows would match `app` on
                            // Linux
      .string();
}

std::string MizarData::GetModuleFilenameWithoutExtension(AbsoluteAddress address) const {
  const std::string& path =
      orbit_client_data::GetModulePathByAddress(*module_manager_, GetCaptureData(), *address);
  // If a function has a name, we know its module. The branch is here for a future debug purpose.
  ORBIT_CHECK(path != orbit_client_data::kUnknownFunctionOrModuleName);

  return GetFilenameWithoutExtension(path);
}

void MizarData::OnCaptureStarted(const orbit_grpc_protos::CaptureStarted& capture_started,
                                 std::optional<std::filesystem::path> file_path,
                                 absl::flat_hash_set<uint64_t> frame_track_function_ids) {
  ConstructCaptureData(capture_started, std::move(file_path), std::move(frame_track_function_ids),
                       orbit_client_data::CaptureData::DataSource::kLoadedCapture);
  module_manager_ = std::make_unique<orbit_client_data::ModuleManager>();
}

void MizarData::OnTimer(const orbit_client_protos::TimerInfo& timer_info) {
  const std::optional<ScopeId> scope_id = GetCaptureData().ProvideScopeId(timer_info);
  if (!scope_id.has_value()) return;

  const orbit_client_data::ScopeType scope_type =
      GetCaptureData().GetScopeInfo(scope_id.value()).GetType();
  if (scope_type == orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction ||
      scope_type == orbit_client_data::ScopeType::kApiScope) {
    GetMutableCaptureData().GetThreadTrackDataProvider()->AddTimer(timer_info);
  }
}

std::optional<std::string> MizarData::GetFunctionNameFromAddress(AbsoluteAddress address) const {
  const std::string name =
      orbit_client_data::GetFunctionNameByAddress(*module_manager_, GetCaptureData(), *address);

  if (name == orbit_client_data::kUnknownFunctionOrModuleName) return std::nullopt;
  return name;
}

void MizarData::UpdateModules(absl::Span<const orbit_grpc_protos::ModuleInfo> module_infos) {
  for (const auto* not_updated_module :
       module_manager_->AddOrUpdateNotLoadedModules(module_infos)) {
    ORBIT_LOG("Module %s is not updated", not_updated_module->file_path());
  }
  GetMutableCaptureData().mutable_process()->UpdateModuleInfos(module_infos);
}

void MizarData::LoadSymbolsForAllModules() {
  for (const orbit_client_data::ModuleData* module_data : module_manager_->GetAllModuleData()) {
    orbit_client_data::ModuleData* mutable_module_data =
        module_manager_->GetMutableModuleByModuleIdentifier(module_data->module_id());
    LoadSymbols(*mutable_module_data);
  }
}

static ErrorMessageOr<std::filesystem::path> SearchSymbolsPathInOrbitSearchPaths(
    const orbit_symbols::SymbolHelper& symbol_helper,
    const orbit_client_data::ModuleData& module_data) {
  // These are the constants used by Orbit Client. This way we read its configs.
  static const QString orbit_organization = QStringLiteral("The Orbit Authors");
  static const QString orbit_app_name = QStringLiteral("orbitprofiler");
  orbit_client_symbols::QSettingsBasedStorageManager storage_manager(orbit_organization,
                                                                     orbit_app_name);
  return symbol_helper.FindSymbolsFileLocally(module_data.file_path(), module_data.build_id(),
                                              module_data.object_file_type(),
                                              storage_manager.LoadPaths());
}

static void LogSymbolsFound(std::string_view module_path, std::string_view symbols_path) {
  ORBIT_LOG("Found symbol path for module \"%s\". Symbols filename: \"%s\"", module_path,
            symbols_path);
}

static ErrorMessageOr<std::filesystem::path> FindSymbolsPath(
    const orbit_symbols::SymbolHelper& symbol_helper,
    const orbit_client_data::ModuleData& module_data) {
  if (auto symbols_paths_or_error = SearchSymbolsPathInOrbitSearchPaths(symbol_helper, module_data);
      symbols_paths_or_error.has_value()) {
    LogSymbolsFound(module_data.file_path(), symbols_paths_or_error.value().string());
    return symbols_paths_or_error;
  }

  if (auto symbols_paths_or_error =
          symbol_helper.FindSymbolsInCache(module_data.file_path(), module_data.build_id());
      symbols_paths_or_error.has_value()) {
    LogSymbolsFound(module_data.file_path(), symbols_paths_or_error.value().string());
    return symbols_paths_or_error;
  }

  // If the symbol file is neither in the search paths, nor a file with the expected build id is not
  // in caches, as the last resort, we try to find in caches the symbol file of the same size as the
  // module. Useful when the module file contains the symbols itself and lacks build id.
  OUTCOME_TRY(auto symbols_paths,
              symbol_helper.FindSymbolsInCache(module_data.file_path(), module_data.file_size()));

  LogSymbolsFound(module_data.file_path(), symbols_paths.string());
  return symbols_paths;
}

static ErrorMessageOr<void> FindAndLoadSymbols(orbit_symbols::SymbolHelper& symbol_helper,
                                               orbit_client_data::ModuleData& module_data) {
  OUTCOME_TRY(const auto symbols_path, FindSymbolsPath(symbol_helper, module_data));

  orbit_object_utils::ObjectFileInfo object_file_info{module_data.load_bias()};
  OUTCOME_TRY(orbit_grpc_protos::ModuleSymbols symbols,
              orbit_symbols::SymbolHelper::LoadSymbolsFromFile(symbols_path, object_file_info));
  module_data.AddSymbols(symbols);

  return outcome::success();
}

void MizarData::LoadSymbols(orbit_client_data::ModuleData& module_data) {
  ORBIT_LOG("Searching for symbols for module: %s", module_data.file_path());

  auto maybe_error = FindAndLoadSymbols(symbol_helper_, module_data);
  if (maybe_error.has_error()) {
    ORBIT_LOG("Symbols could not be loaded for module: %s, because %s", module_data.file_path(),
              maybe_error.error().message());
  }
}

}  // namespace orbit_mizar_data