// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_GL_SYMBOL_LOADER_H_
#define ORBIT_GL_SYMBOL_LOADER_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <stdint.h>

#include <filesystem>
#include <memory>
#include <optional>
#include <string>
#include <string_view>
#include <thread>

#include "ClientData/ModuleData.h"
#include "ClientServices/ProcessManager.h"
#include "DataViews/SymbolLoadingState.h"
#include "GrpcProtos/symbol.pb.h"
#include "Http/HttpDownloadManager.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/MainThreadExecutor.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "OrbitBase/StopToken.h"
#include "OrbitBase/ThreadPool.h"
#include "OrbitPaths/Paths.h"
#include "RemoteSymbolProvider/MicrosoftSymbolServerSymbolProvider.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "Symbols/SymbolHelper.h"

namespace orbit_gl {

class SymbolLoader {
 public:
  class AppInterface {
   public:
    virtual ~AppInterface() = default;

    [[nodiscard]] virtual const orbit_client_data::ModuleData* GetModuleByModuleIdentifier(
        const orbit_symbol_provider::ModuleIdentifier& module_id) const = 0;
    [[nodiscard]] virtual bool IsConnected() const = 0;
    [[nodiscard]] virtual bool IsLocalTarget() const = 0;
    virtual orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>>
    DownloadFileFromInstance(std::filesystem::path path_on_instance,
                             std::filesystem::path local_path,
                             orbit_base::StopToken stop_token) = 0;
    virtual void OnModuleListUpdated() = 0;
    virtual void AddSymbols(const orbit_symbol_provider::ModuleIdentifier& module_id,
                            const orbit_grpc_protos::ModuleSymbols& module_symbols) = 0;
    virtual void AddFallbackSymbols(const orbit_symbol_provider::ModuleIdentifier& module_id,
                                    const orbit_grpc_protos::ModuleSymbols& fallback_symbols) = 0;
  };

  SymbolLoader(AppInterface* app_interface, std::thread::id main_thread_id,
               orbit_base::ThreadPool* thread_pool,
               orbit_base::MainThreadExecutor* main_thread_executor,
               orbit_client_services::ProcessManager* process_manager);

  // RetrieveModuleAndLoadSymbols tries to retrieve and load the module symbols by calling
  // `RetrieveModuleSymbolsAndLoadSymbols`. If this fails, it falls back on
  // `RetrieveModuleItselfAndLoadFallbackSymbols`.
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>> RetrieveModuleAndLoadSymbols(
      const orbit_client_data::ModuleData* module_data);

  // This method is pretty similar to `RetrieveModuleSymbols`, but it also requires debug
  // information to be present.
  orbit_base::Future<ErrorMessageOr<std::filesystem::path>> RetrieveModuleWithDebugInfo(
      const orbit_symbol_provider::ModuleIdentifier& module_id);

  void DisableDownloadForModule(std::string_view module_path);
  void EnableDownloadForModules(const absl::flat_hash_set<std::string>& module_paths);

  void RequestSymbolDownloadStop(std::string_view module_path);

  [[nodiscard]] bool IsModuleDownloading(std::string_view module_path) const;
  [[nodiscard]] orbit_data_views::SymbolLoadingState GetSymbolLoadingStateForModule(
      const orbit_client_data::ModuleData* module) const;
  [[nodiscard]] bool IsSymbolLoadingInProgressForModule(
      const orbit_symbol_provider::ModuleIdentifier& module_id) const;

 private:
  struct ModuleDownloadOperation {
    orbit_base::StopSource stop_source;
    orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>> future;
  };

  void InitRemoteSymbolProviders();

  // RetrieveModuleSymbolsAndLoadSymbols retrieves the module symbols by calling
  // `RetrieveModuleSymbols` and afterwards loads the symbols by calling `LoadSymbols`.
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>>
  RetrieveModuleSymbolsAndLoadSymbols(const orbit_symbol_provider::ModuleIdentifier& module_id);
  // RetrieveModuleSymbols retrieves a module file and returns the local file path (potentially from
  // the local cache). Only modules with a .symtab section will be considered.
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>>
  RetrieveModuleSymbols(const orbit_symbol_provider::ModuleIdentifier& module_id);
  orbit_base::Future<ErrorMessageOr<std::filesystem::path>> FindModuleLocally(
      const orbit_client_data::ModuleData* module_data);
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>>
  RetrieveModuleFromRemote(const orbit_symbol_provider::ModuleIdentifier& module_id);
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>>
  RetrieveModuleFromInstance(std::string_view module_file_path, orbit_base::StopToken stop_token);

  // RetrieveModuleItselfAndLoadFallbackSymbols retrieves the module's binary by calling
  // `RetrieveModuleItself` and afterwards loads the fallback symbols by calling
  // `LoadFallbackSymbols`.
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>>
  RetrieveModuleItselfAndLoadFallbackSymbols(
      const orbit_symbol_provider::ModuleIdentifier& module_id, uint64_t module_file_size);
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>>
  RetrieveModuleItself(const orbit_symbol_provider::ModuleIdentifier& module_id,
                       uint64_t module_file_size);
  orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<std::filesystem::path>>>
  RetrieveModuleItselfFromInstance(const orbit_symbol_provider::ModuleIdentifier& module_id);

  orbit_base::Future<ErrorMessageOr<void>> LoadSymbols(
      const std::filesystem::path& symbols_path,
      const orbit_symbol_provider::ModuleIdentifier& module_id);
  orbit_base::Future<ErrorMessageOr<void>> LoadFallbackSymbols(
      const std::filesystem::path& object_path,
      const orbit_symbol_provider::ModuleIdentifier& module_id);

  AppInterface* app_interface_;
  std::thread::id main_thread_id_;
  orbit_base::ThreadPool* thread_pool_;
  orbit_base::MainThreadExecutor* main_thread_executor_;
  orbit_client_services::ProcessManager* process_manager_;

  orbit_symbols::SymbolHelper symbol_helper_{orbit_paths::CreateOrGetCacheDirUnsafe()};

  // TODO(b/243520787) The SymbolProvider related logic should be moved to the ProxySymbolProvider
  //  as planned in our symbol refactoring discussion.
  std::optional<orbit_http::HttpDownloadManager> download_manager_;
  std::optional<orbit_remote_symbol_provider::MicrosoftSymbolServerSymbolProvider>
      microsoft_symbol_provider_ = std::nullopt;

  // Map of module file path to download operation future, that holds all symbol downloads that
  // are currently in progress.
  // ONLY access this from the main thread.
  absl::flat_hash_map<std::string, ModuleDownloadOperation> symbol_files_currently_downloading_;

  // Map of "module ID" (file path and build ID) to symbol loading future, that holds all symbol
  // loading operations that are currently in progress. Since downloading a file can be part of the
  // overall symbol loading process, if a module ID is contained in
  // symbol_files_currently_downloading_, it is also contained in symbols_currently_loading_.
  // ONLY access this from the main thread.
  absl::flat_hash_map<orbit_symbol_provider::ModuleIdentifier,
                      orbit_base::Future<ErrorMessageOr<orbit_base::CanceledOr<void>>>>
      symbols_currently_loading_;

  // Set of modules where a symbol loading error has occurred. The module identifier consists of
  // file path and build ID.
  // ONLY access this from the main thread.
  absl::flat_hash_set<orbit_symbol_provider::ModuleIdentifier> modules_with_symbol_loading_error_;

  // Set of modules for which the download is disabled.
  // ONLY access this from the main thread.
  absl::flat_hash_set<std::string> download_disabled_modules_;
};

}  // namespace orbit_gl

#endif  // ORBIT_GL_SYMBOL_LOADER_H_
