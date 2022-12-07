// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitGl/SymbolLoader.h"

#include <absl/flags/flag.h>
#include <absl/hash/hash.h>
#include <absl/meta/type_traits.h>
#include <absl/strings/str_format.h>

#include <algorithm>
#include <chrono>
#include <memory>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>

#include "ClientFlags/ClientFlags.h"
#include "ClientSymbols/QSettingsBasedStorageManager.h"
#include "Introspection/Introspection.h"
#include "ObjectUtils/ElfFile.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/Executor.h"
#include "OrbitBase/File.h"
#include "OrbitBase/FutureHelpers.h"
#include "OrbitBase/ImmediateExecutor.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/StopToken.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "Symbols/SymbolUtils.h"

using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_base::NotFoundOr;
using orbit_base::StopToken;

using orbit_client_data::ModuleData;

using orbit_data_views::SymbolLoadingState;

using orbit_symbol_provider::ModuleIdentifier;
using orbit_symbol_provider::SymbolLoadingOutcome;

namespace orbit_gl {

SymbolLoader::SymbolLoader(AppInterface* app_interface, std::thread::id main_thread_id,
                           orbit_base::ThreadPool* thread_pool,
                           orbit_base::MainThreadExecutor* main_thread_executor,
                           orbit_client_services::ProcessManager* process_manager)
    : app_interface_{app_interface},
      main_thread_id_{main_thread_id},
      thread_pool_{thread_pool},
      main_thread_executor_{main_thread_executor},
      process_manager_{process_manager} {
  ORBIT_CHECK(app_interface_ != nullptr);
  ORBIT_CHECK(thread_pool_ != nullptr);
  ORBIT_CHECK(main_thread_executor_ != nullptr);

  orbit_client_symbols::QSettingsBasedStorageManager storage_manager;
  download_disabled_modules_ = storage_manager.LoadDisabledModulePaths();

  if (absl::GetFlag(FLAGS_symbol_store_support)) {
    InitRemoteSymbolProviders();
  }
}

void SymbolLoader::DisableDownloadForModule(std::string_view module_path) {
  download_disabled_modules_.emplace(module_path);
  orbit_client_symbols::QSettingsBasedStorageManager storage_manager;
  storage_manager.SaveDisabledModulePaths(download_disabled_modules_);
}

void SymbolLoader::EnableDownloadForModules(const absl::flat_hash_set<std::string>& module_paths) {
  for (const std::string& module_path : module_paths) {
    download_disabled_modules_.erase(module_path);
  }
  orbit_client_symbols::QSettingsBasedStorageManager storage_manager;
  storage_manager.SaveDisabledModulePaths(download_disabled_modules_);
}

void SymbolLoader::InitRemoteSymbolProviders() {
  download_manager_.emplace();

  microsoft_symbol_provider_.emplace(&symbol_helper_, &*download_manager_);
}

Future<ErrorMessageOr<CanceledOr<void>>> SymbolLoader::RetrieveModuleAndLoadSymbols(
    const orbit_client_data::ModuleData* module_data) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  ORBIT_CHECK(module_data != nullptr);

  const ModuleIdentifier module_id = module_data->module_id();

  modules_with_symbol_loading_error_.erase(module_id);

  if (module_data->AreDebugSymbolsLoaded()) return {outcome::success()};

  const auto it = symbols_currently_loading_.find(module_id);
  if (it != symbols_currently_loading_.end()) {
    return it->second;
  }

  Future<ErrorMessageOr<CanceledOr<void>>> retrieve_module_symbols_and_load_symbols_future =
      RetrieveModuleSymbolsAndLoadSymbols(module_id);

  Future<ErrorMessageOr<CanceledOr<void>>> retrieve_module_itself_and_load_fallback_symbols_future =
      orbit_base::UnwrapFuture(retrieve_module_symbols_and_load_symbols_future.Then(
          main_thread_executor_,
          [this, module_id](const ErrorMessageOr<CanceledOr<void>>&
                                retrieve_module_symbols_and_load_symbols_result)
              -> Future<ErrorMessageOr<CanceledOr<void>>> {
            if (retrieve_module_symbols_and_load_symbols_result.has_value()) {
              return {retrieve_module_symbols_and_load_symbols_result};
            }

            const ModuleData* module_data = app_interface_->GetModuleByModuleIdentifier(module_id);
            if (module_data == nullptr) {
              return {ErrorMessage{
                  absl::StrFormat("Module \"%s\" was not found.", module_id.file_path)}};
            }
            // Report the error if loading debug symbols fails when the fallback symbols are already
            // loaded. This happens when choosing "Load Symbols" on a module that has already
            // "Symbols: Partial".
            if (module_data->AreAtLeastFallbackSymbolsLoaded()) {
              return {retrieve_module_symbols_and_load_symbols_result};
            }

            return RetrieveModuleItselfAndLoadFallbackSymbols(module_id, module_data->file_size())
                .Then(main_thread_executor_,
                      [module_id,
                       previous_message =
                           retrieve_module_symbols_and_load_symbols_result.error().message()](
                          const ErrorMessageOr<CanceledOr<void>>&
                              retrieve_module_itself_and_load_fallback_symbols_result) mutable
                      -> ErrorMessageOr<CanceledOr<void>> {
                        if (retrieve_module_itself_and_load_fallback_symbols_result.has_value()) {
                          return retrieve_module_itself_and_load_fallback_symbols_result;
                        }

                        // Merge the two ErrorMessages if everything fails.
                        return ErrorMessage{absl::StrFormat(
                            "%s\n\nAlso: %s", previous_message,
                            retrieve_module_itself_and_load_fallback_symbols_result.error()
                                .message())};
                      });
          }));

  retrieve_module_itself_and_load_fallback_symbols_future.Then(
      main_thread_executor_,
      [this, module_id](const ErrorMessageOr<CanceledOr<void>>& result) mutable {
        if (result.has_error()) {
          modules_with_symbol_loading_error_.emplace(module_id);
        }
        symbols_currently_loading_.erase(module_id);
        app_interface_->OnModuleListUpdated();
      });

  symbols_currently_loading_.emplace(module_id,
                                     retrieve_module_itself_and_load_fallback_symbols_future);
  app_interface_->OnModuleListUpdated();

  return retrieve_module_itself_and_load_fallback_symbols_future;
}

Future<ErrorMessageOr<CanceledOr<void>>> SymbolLoader::RetrieveModuleSymbolsAndLoadSymbols(
    const ModuleIdentifier& module_id) {
  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> retrieve_module_symbols_future =
      RetrieveModuleSymbols(module_id);

  return orbit_base::UnwrapFuture(retrieve_module_symbols_future.Then(
      main_thread_executor_,
      [this, module_id](const ErrorMessageOr<CanceledOr<std::filesystem::path>>& retrieve_result)
          -> Future<ErrorMessageOr<CanceledOr<void>>> {
        if (retrieve_result.has_error()) {
          return ErrorMessage{absl::StrFormat("Could not load debug symbols for \"%s\": %s",
                                              module_id.file_path,
                                              retrieve_result.error().message())};
        }
        if (orbit_base::IsCanceled(retrieve_result.value())) {
          return {orbit_base::Canceled{}};
        }
        const std::filesystem::path& local_file_path =
            orbit_base::GetNotCanceled(retrieve_result.value());

        orbit_base::ImmediateExecutor executor;
        return LoadSymbols(local_file_path, module_id)
            .ThenIfSuccess(&executor, []() -> CanceledOr<void> { return CanceledOr<void>{}; });
      }));
}

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> SymbolLoader::RetrieveModuleSymbols(
    const ModuleIdentifier& module_id) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);

  const ModuleData* module_data = app_interface_->GetModuleByModuleIdentifier(module_id);
  if (module_data == nullptr) {
    return {ErrorMessage{absl::StrFormat("Module \"%s\" was not found.", module_id.file_path)}};
  }

  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> retrieve_from_local_future =
      FindModuleLocally(module_data)
          .Then(main_thread_executor_,
                [module_id](const ErrorMessageOr<std::filesystem::path>& retrieve_result)
                    -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
                  if (retrieve_result.has_value()) return {retrieve_result.value()};

                  return {ErrorMessage{absl::StrFormat(
                      "Failed to find symbols for module \"%s\" with build_id=\"%s\":\n"
                      "- %s",
                      module_id.file_path, module_id.build_id, retrieve_result.error().message())}};
                });

  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> retrieve_from_remote_future =
      orbit_base::UnwrapFuture(retrieve_from_local_future.Then(
          main_thread_executor_,
          [this, module_id](
              const ErrorMessageOr<CanceledOr<std::filesystem::path>>& previous_result) mutable
          -> Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> {
            if (download_disabled_modules_.contains(module_id.file_path)) return previous_result;

            if (previous_result.has_value()) return {previous_result.value()};

            return RetrieveModuleFromRemote(module_id).Then(
                main_thread_executor_,
                [module_id, current_message = previous_result.error().message()](
                    const ErrorMessageOr<CanceledOr<std::filesystem::path>>&
                        retrieve_result) mutable
                -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
                  if (retrieve_result.has_value()) return retrieve_result;

                  current_message.append(retrieve_result.error().message());
                  return ErrorMessage{current_message};
                });
          }));

  return retrieve_from_remote_future;
}

[[nodiscard]] static std::vector<std::filesystem::path> GetAllSymbolPaths() {
  orbit_client_symbols::QSettingsBasedStorageManager storage_manager;
  std::vector<std::filesystem::path> all_paths = storage_manager.LoadPaths();
  std::vector<std::string> temp_paths = absl::GetFlag(FLAGS_additional_symbol_paths);
  all_paths.insert(all_paths.end(), temp_paths.begin(), temp_paths.end());
  return all_paths;
}

static ErrorMessageOr<std::optional<std::filesystem::path>> GetOverrideSymbolFileForModule(
    const ModuleData& module_data) {
  orbit_client_symbols::QSettingsBasedStorageManager storage_manager;
  absl::flat_hash_map<std::string, std::filesystem::path> mappings =
      storage_manager.LoadModuleSymbolFileMappings();
  if (!mappings.contains(module_data.file_path())) return std::nullopt;

  std::filesystem::path symbol_file_path = mappings[module_data.file_path()];

  OUTCOME_TRY(bool file_exists, orbit_base::FileOrDirectoryExists(symbol_file_path));

  if (!file_exists) {
    return ErrorMessage{absl::StrFormat(
        R"(A symbol file override is in place for module "%s", but the symbols file "%s" does not exist.)",
        module_data.file_path(), symbol_file_path.string())};
  }

  return symbol_file_path;
}

static ErrorMessageOr<std::filesystem::path> FindModuleLocallyImpl(
    const orbit_symbols::SymbolHelper& symbol_helper, const ModuleData& module_data) {
  ORBIT_SCOPE_FUNCTION;
  if (absl::GetFlag(FLAGS_enable_unsafe_symbols)) {
    // First checkout if a symbol file override exists and if it does, use it
    OUTCOME_TRY(std::optional<std::filesystem::path> overriden_symbols_file,
                GetOverrideSymbolFileForModule(module_data));
    if (overriden_symbols_file.has_value()) return overriden_symbols_file.value();
  }

  if (module_data.build_id().empty()) {
    return ErrorMessage(
        absl::StrFormat("Unable to find local symbols for module \"%s\": build id is empty.",
                        module_data.file_path()));
  }

  // Note that the bullet points in the ErrorMessage constructed by this function are indented (by
  // one level). This is because the caller of this function integrates this ErrorMessage into an
  // ErrorMessage that also has bullet points (with no indentation, i.e., at top level).

  std::string error_message;
  {
    std::vector<std::filesystem::path> search_paths = GetAllSymbolPaths();
    std::filesystem::path module_path(module_data.file_path());
    search_paths.emplace_back(module_path.parent_path());

    const auto symbols_path =
        symbol_helper.FindSymbolsFileLocally(module_data.file_path(), module_data.build_id(),
                                             module_data.object_file_type(), search_paths);
    if (symbols_path.has_value()) {
      ORBIT_LOG(
          "Found symbols for module \"%s\" in user provided symbol folder. Symbols filename: "
          "\"%s\"",
          module_data.file_path(), symbols_path.value().string());
      return symbols_path.value();
    }
    error_message += "\n  * " + symbols_path.error().message();
  }
  {
    const auto symbols_path =
        symbol_helper.FindSymbolsInCache(module_data.file_path(), module_data.build_id());
    if (symbols_path.has_value()) {
      ORBIT_LOG("Found symbols for module \"%s\" in cache. Symbols filename: \"%s\"",
                module_data.file_path(), symbols_path.value().string());
      return symbols_path.value();
    }
    error_message += "\n  * " + symbols_path.error().message();
  }
  {
    // Check whether a valid symbol file exists on the local machine at module_data.file_path().
    // This is valuable when a local target is profiled (aka OrbitService runs on the local
    // machine). In case Orbit is connected to a remote machine this will likely fail.
    const auto symbols_included_in_module =
        orbit_symbols::VerifySymbolFile(module_data.file_path(), module_data.build_id());
    if (symbols_included_in_module.has_value()) {
      ORBIT_LOG("Found symbols included in module: \"%s\"", module_data.file_path());
      return module_data.file_path();
    }
    error_message += "\n  * Symbols are not included in module file: " +
                     symbols_included_in_module.error().message();
  }

  error_message = absl::StrFormat("Did not find local symbols for module \"%s\": %s",
                                  module_data.file_path(), error_message);
  ORBIT_LOG("%s", error_message);
  return ErrorMessage(error_message);
}

Future<ErrorMessageOr<std::filesystem::path>> SymbolLoader::FindModuleLocally(
    const ModuleData* module_data) {
  ORBIT_SCOPE_FUNCTION;
  return thread_pool_->Schedule([this, module_data]() -> ErrorMessageOr<std::filesystem::path> {
    return FindModuleLocallyImpl(symbol_helper_, *module_data);
  });
}

static Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>>
ConvertSymbolProviderRetrieveFuture(const Future<SymbolLoadingOutcome>& future,
                                    orbit_base::Executor* executor,
                                    std::string symbol_provider_name, std::string error_msg) {
  return future.Then(
      executor,
      [symbol_provider_name = std::move(symbol_provider_name),
       error_msg = std::move(error_msg)](const SymbolLoadingOutcome& retrieve_result) mutable
      -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
        if (orbit_symbol_provider::IsSuccessResult(retrieve_result)) {
          return orbit_symbol_provider::GetSuccessResult(retrieve_result).path;
        }

        if (orbit_symbol_provider::IsCanceled(retrieve_result)) return orbit_base::Canceled{};

        error_msg.append(
            absl::StrFormat("\n- Did not find symbols from %s: %s", symbol_provider_name,
                            orbit_symbol_provider::IsNotFound(retrieve_result)
                                ? orbit_symbol_provider::GetNotFoundMessage(retrieve_result)
                                : retrieve_result.error().message()));
        return ErrorMessage{error_msg};
      });
};

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> SymbolLoader::RetrieveModuleFromRemote(
    const ModuleIdentifier& module_id) {
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);

  if (const auto it = symbol_files_currently_downloading_.find(module_id.file_path);
      it != symbol_files_currently_downloading_.end()) {
    return it->second.future;
  }

  orbit_base::StopSource stop_source;

  using SymbolRetrieveResult = ErrorMessageOr<CanceledOr<std::filesystem::path>>;
  Future<SymbolRetrieveResult> retrieve_from_instance_future = orbit_base::UnwrapFuture(
      main_thread_executor_->Schedule([this, module_id,
                                       stop_token = stop_source.GetStopToken()]() mutable
                                      -> Future<SymbolRetrieveResult> {
        // If Orbit is in local profiling mode, it cannot download files from the instance, because
        // no ssh channel exists. We still return an ErrorMessage to enable continuing searching for
        // symbols from other symbol sources.
        if (app_interface_->IsLocalTarget() || !app_interface_->IsConnected() ||
            absl::GetFlag(FLAGS_disable_instance_symbols)) {
          return {ErrorMessage{"\n- Not able to search for symbols on the instance."}};
        }

        return RetrieveModuleFromInstance(module_id.file_path, std::move(stop_token))
            .Then(main_thread_executor_,
                  [](const SymbolRetrieveResult& retrieve_result) mutable -> SymbolRetrieveResult {
                    if (retrieve_result.has_value()) return retrieve_result;

                    return ErrorMessage{
                        absl::StrFormat("\n- Did not find symbols on the instance: %s",
                                        retrieve_result.error().message())};
                  });
      }));

  Future<SymbolRetrieveResult> retrieve_from_microsoft_future =
      orbit_base::UnwrapFuture(retrieve_from_instance_future.Then(
          main_thread_executor_,
          [this, module_id, stop_token = stop_source.GetStopToken()](
              const SymbolRetrieveResult& previous_result) mutable -> Future<SymbolRetrieveResult> {
            if (orbit_client_symbols::QSettingsBasedStorageManager storage_manager;
                microsoft_symbol_provider_ == std::nullopt ||
                !storage_manager.LoadEnableMicrosoftSymbolServer()) {
              return {previous_result};
            }

            if (previous_result.has_value()) return {previous_result.value()};

            return ConvertSymbolProviderRetrieveFuture(
                microsoft_symbol_provider_->RetrieveSymbols(module_id, std::move(stop_token)),
                main_thread_executor_, "Microsoft symbol server",
                previous_result.error().message());
          }));

  symbol_files_currently_downloading_.emplace(
      module_id.file_path,
      ModuleDownloadOperation{std::move(stop_source), retrieve_from_microsoft_future});
  app_interface_->OnModuleListUpdated();
  retrieve_from_microsoft_future.Then(
      main_thread_executor_,
      [this, module_file_path = module_id.file_path](const SymbolRetrieveResult& /*result*/) {
        symbol_files_currently_downloading_.erase(module_file_path);
        app_interface_->OnModuleListUpdated();
      });

  return retrieve_from_microsoft_future;
}

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> SymbolLoader::RetrieveModuleFromInstance(
    std::string_view module_file_path, StopToken stop_token) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);

  Future<ErrorMessageOr<NotFoundOr<std::filesystem::path>>> check_file_on_remote =
      thread_pool_->Schedule([this, module_file_path = std::string{module_file_path}]()
                                 -> ErrorMessageOr<NotFoundOr<std::filesystem::path>> {
        std::vector<std::string> additional_instance_folder;
        if (!absl::GetFlag(FLAGS_instance_symbols_folder).empty()) {
          additional_instance_folder.emplace_back(absl::GetFlag(FLAGS_instance_symbols_folder));
        }
        ORBIT_CHECK(process_manager_ != nullptr);
        return process_manager_->FindDebugInfoFile(module_file_path, additional_instance_folder);
      });

  auto download_file = [this, module_file_path = std::string{module_file_path},
                        stop_token = std::move(stop_token)](
                           const NotFoundOr<std::filesystem::path>& remote_search_outcome) mutable
      -> Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> {
    // TODO(b/231455031): For now, we treat the ErrorMessage and the NotFound the same way.
    if (orbit_base::IsNotFound(remote_search_outcome)) {
      return {ErrorMessage{orbit_base::GetNotFoundMessage(remote_search_outcome)}};
    }
    const std::filesystem::path& remote_debug_file_path =
        orbit_base::GetFound(remote_search_outcome);
    ORBIT_LOG("Found symbols file on the remote: \"%s\" - loading it using scp...",
              remote_debug_file_path.string());

    const std::filesystem::path local_debug_file_path =
        symbol_helper_.GenerateCachedFilePath(module_file_path);

    const std::chrono::time_point<std::chrono::steady_clock> copy_begin =
        std::chrono::steady_clock::now();
    ORBIT_LOG("Copying \"%s\" started", remote_debug_file_path.string());
    Future<ErrorMessageOr<CanceledOr<void>>> copy_result = app_interface_->DownloadFileFromInstance(
        remote_debug_file_path, local_debug_file_path, std::move(stop_token));

    orbit_base::ImmediateExecutor immediate_executor{};
    return copy_result.Then(&immediate_executor,
                            [remote_debug_file_path, local_debug_file_path,
                             copy_begin](ErrorMessageOr<CanceledOr<void>> sftp_result)
                                -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
                              if (sftp_result.has_error()) {
                                return ErrorMessage{absl::StrFormat(
                                    "Could not copy debug info file from the remote: %s",
                                    sftp_result.error().message())};
                              }
                              if (orbit_base::IsCanceled(sftp_result.value())) {
                                return orbit_base::Canceled{};
                              }
                              const auto duration =
                                  std::chrono::duration_cast<std::chrono::milliseconds>(
                                      std::chrono::steady_clock::now() - copy_begin);
                              ORBIT_LOG("Copying \"%s\" took %.3f ms",
                                        remote_debug_file_path.string(), duration.count());
                              return local_debug_file_path;
                            });
  };

  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> chained_result_future = UnwrapFuture(
      check_file_on_remote.ThenIfSuccess(main_thread_executor_, std::move(download_file)));

  return chained_result_future;
}

Future<ErrorMessageOr<CanceledOr<void>>> SymbolLoader::RetrieveModuleItselfAndLoadFallbackSymbols(
    const ModuleIdentifier& module_id, uint64_t module_file_size) {
  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> retrieve_module_itself_future =
      RetrieveModuleItself(module_id, module_file_size);

  return orbit_base::UnwrapFuture(retrieve_module_itself_future.Then(
      main_thread_executor_,
      [this, module_id](const ErrorMessageOr<CanceledOr<std::filesystem::path>>& retrieve_result)
          -> Future<ErrorMessageOr<CanceledOr<void>>> {
        if (retrieve_result.has_error()) {
          return ErrorMessage{absl::StrFormat("Could not load fallback symbols for \"%s\": %s",
                                              module_id.file_path,
                                              retrieve_result.error().message())};
        }
        if (orbit_base::IsCanceled(retrieve_result.value())) {
          return {orbit_base::Canceled{}};
        }
        const std::filesystem::path& local_file_path =
            orbit_base::GetNotCanceled(retrieve_result.value());

        orbit_base::ImmediateExecutor executor;
        return LoadFallbackSymbols(local_file_path, module_id)
            .ThenIfSuccess(&executor, []() -> CanceledOr<void> { return CanceledOr<void>{}; });
      }));
}

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> SymbolLoader::RetrieveModuleItself(
    const ModuleIdentifier& module_id, uint64_t module_file_size) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);

  // Note that the bullet points in the ErrorMessage constructed by this function are indented (by
  // one level). This is because the caller of this method integrates this ErrorMessage into an
  // ErrorMessage that also has bullet points (with no indentation, i.e., at top level).

  auto find_in_cache_or_locally =
      [this, module_id, module_file_size]() -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
    std::string error_message;
    {
      auto object_in_cache = symbol_helper_.FindObjectInCache(module_id.file_path,
                                                              module_id.build_id, module_file_size);
      if (object_in_cache.has_value()) {
        ORBIT_LOG("Found module file \"%s\" itself in cache", module_id.file_path);
        return {object_in_cache.value()};
      }
      error_message += absl::StrFormat("\n  * Could not find module file itself in cache: %s",
                                       object_in_cache.error().message());
    }
    if (app_interface_->IsLocalTarget()) {
      auto verify_object_file_result = orbit_symbols::VerifyObjectFile(
          module_id.file_path, module_id.build_id, module_file_size);
      if (verify_object_file_result.has_value()) {
        ORBIT_LOG("Found module file \"%s\" itself locally", module_id.file_path);
        return module_id.file_path;
      }
      error_message += "\n  * Could not find module file itself locally.";
    }
    return ErrorMessage{error_message};
  };

  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> find_in_cache_or_locally_future =
      thread_pool_->Schedule(find_in_cache_or_locally);

  auto retrieve_from_instance = [this,
                                 module_id](const ErrorMessageOr<CanceledOr<std::filesystem::path>>&
                                                find_in_cache_or_locally_result) mutable
      -> Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> {
    if (download_disabled_modules_.contains(module_id.file_path)) {
      return find_in_cache_or_locally_result;
    }
    if (find_in_cache_or_locally_result.has_value()) {
      return {find_in_cache_or_locally_result};
    }

    if (app_interface_->IsLocalTarget() || !app_interface_->IsConnected() ||
        absl::GetFlag(FLAGS_disable_instance_symbols)) {
      return {ErrorMessage{
          absl::StrFormat("%s\n  * Could not search for module file itself on the instance.",
                          find_in_cache_or_locally_result.error().message())}};
    }

    return RetrieveModuleItselfFromInstance(module_id).Then(
        main_thread_executor_,
        [current_message = find_in_cache_or_locally_result.error().message()](
            const ErrorMessageOr<CanceledOr<std::filesystem::path>>& retrieve_from_instance_result)
            -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
          if (retrieve_from_instance_result.has_error()) {
            return ErrorMessage{absl::StrFormat("%s\n  * %s", current_message,
                                                retrieve_from_instance_result.error().message())};
          }
          return retrieve_from_instance_result;
        });
  };

  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> retrieve_from_instance_future =
      orbit_base::UnwrapFuture(find_in_cache_or_locally_future.Then(
          main_thread_executor_, std::move(retrieve_from_instance)));

  return retrieve_from_instance_future;
}

Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>>
SymbolLoader::RetrieveModuleItselfFromInstance(const ModuleIdentifier& module_id) {
  ORBIT_SCOPE_FUNCTION;
  ORBIT_CHECK(std::this_thread::get_id() == main_thread_id_);

  if (const auto it = symbol_files_currently_downloading_.find(module_id.file_path);
      it != symbol_files_currently_downloading_.end()) {
    return it->second.future;
  }

  orbit_base::StopSource stop_source;

  auto download = [this, module_id, stop_token = stop_source.GetStopToken()]() mutable
      -> Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> {
    ORBIT_LOG("Copying module file \"%s\" itself using scp...", module_id.file_path);
    const std::filesystem::path cache_path =
        symbol_helper_.GenerateCachedFilePath(module_id.file_path);
    const std::chrono::time_point<std::chrono::steady_clock> copy_begin =
        std::chrono::steady_clock::now();
    Future<ErrorMessageOr<CanceledOr<void>>> copy_result = app_interface_->DownloadFileFromInstance(
        module_id.file_path, cache_path, std::move(stop_token));

    orbit_base::ImmediateExecutor immediate_executor{};
    return copy_result.Then(
        &immediate_executor,
        [module_id, cache_path, copy_begin](ErrorMessageOr<CanceledOr<void>> sftp_result)
            -> ErrorMessageOr<CanceledOr<std::filesystem::path>> {
          if (sftp_result.has_error()) {
            return ErrorMessage{absl::StrFormat("Could not copy module file from the remote: %s",
                                                sftp_result.error().message())};
          }
          if (orbit_base::IsCanceled(sftp_result.value())) {
            return orbit_base::Canceled{};
          }
          const auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
              std::chrono::steady_clock::now() - copy_begin);
          ORBIT_LOG("Copying \"%s\" took %.3f ms", module_id.file_path, duration.count());
          return cache_path;
        });
  };

  Future<ErrorMessageOr<CanceledOr<std::filesystem::path>>> download_future =
      UnwrapFuture(thread_pool_->Schedule(std::move(download)));

  symbol_files_currently_downloading_.emplace(
      module_id.file_path, ModuleDownloadOperation{std::move(stop_source), download_future});
  app_interface_->OnModuleListUpdated();
  download_future.Then(main_thread_executor_,
                       [this, module_file_path = module_id.file_path](
                           const ErrorMessageOr<CanceledOr<std::filesystem::path>>& /*result*/) {
                         symbol_files_currently_downloading_.erase(module_file_path);
                         app_interface_->OnModuleListUpdated();
                       });
  return download_future;
}

Future<ErrorMessageOr<void>> SymbolLoader::LoadSymbols(const std::filesystem::path& symbols_path,
                                                       const ModuleIdentifier& module_id) {
  ORBIT_SCOPE_FUNCTION;

  auto load_symbols_from_file_future = thread_pool_->Schedule(
      [this, symbols_path, module_id]() -> ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> {
        const ModuleData* module_data = app_interface_->GetModuleByModuleIdentifier(module_id);
        orbit_object_utils::ObjectFileInfo object_file_info{module_data->load_bias()};
        ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> symbols_or_error =
            orbit_symbols::SymbolHelper::LoadSymbolsFromFile(symbols_path, object_file_info);
        if (symbols_or_error.has_value()) return symbols_or_error;
        return {ErrorMessage{absl::StrFormat("Could not load debug symbols from \"%s\": %s",
                                             symbols_path.string(),
                                             symbols_or_error.error().message())}};
      });

  auto add_symbols_future = load_symbols_from_file_future.ThenIfSuccess(
      main_thread_executor_,
      [this,
       module_id](const orbit_grpc_protos::ModuleSymbols& symbols) mutable -> ErrorMessageOr<void> {
        app_interface_->AddSymbols(module_id, symbols);
        ORBIT_LOG("Successfully loaded %d symbols for \"%s\"", symbols.symbol_infos_size(),
                  module_id.file_path);
        return outcome::success();
      });

  return add_symbols_future;
}

Future<ErrorMessageOr<void>> SymbolLoader::LoadFallbackSymbols(
    const std::filesystem::path& object_path, const ModuleIdentifier& module_id) {
  ORBIT_SCOPE_FUNCTION;

  auto load_fallback_symbols_future =
      thread_pool_->Schedule([object_path]() -> ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> {
        ErrorMessageOr<orbit_grpc_protos::ModuleSymbols> fallback_symbols_or_error =
            orbit_symbols::SymbolHelper::LoadFallbackSymbolsFromFile(object_path);
        if (fallback_symbols_or_error.has_value()) return fallback_symbols_or_error;
        return {ErrorMessage{
            absl::StrFormat("Could not load symbols from dynamic linking and/or stack unwinding "
                            "information as symbols from \"%s\": %s",
                            object_path.string(), fallback_symbols_or_error.error().message())}};
      });

  auto add_fallback_symbols_future = load_fallback_symbols_future.ThenIfSuccess(
      main_thread_executor_,
      [this,
       module_id](const orbit_grpc_protos::ModuleSymbols& symbols) mutable -> ErrorMessageOr<void> {
        app_interface_->AddFallbackSymbols(module_id, symbols);
        ORBIT_LOG("Successfully loaded %d fallback symbols for \"%s\"", symbols.symbol_infos_size(),
                  module_id.file_path);
        return outcome::success();
      });

  return add_fallback_symbols_future;
}

Future<ErrorMessageOr<std::filesystem::path>> SymbolLoader::RetrieveModuleWithDebugInfo(
    const ModuleIdentifier& module_id) {
  auto loaded_module = RetrieveModuleSymbols(module_id);
  return loaded_module.ThenIfSuccess(
      main_thread_executor_,
      [this, module_path = module_id.file_path](
          const CanceledOr<std::filesystem::path>& local_file_path_or_canceled)
          -> ErrorMessageOr<std::filesystem::path> {
        if (orbit_base::IsCanceled(local_file_path_or_canceled)) {
          return ErrorMessage{"User canceled loading."};
        }
        const std::filesystem::path& local_file_path =
            orbit_base::GetNotCanceled(local_file_path_or_canceled);

        auto elf_file = orbit_object_utils::CreateElfFile(local_file_path);

        if (elf_file.has_error()) return elf_file.error();

        if (elf_file.value()->HasDebugInfo()) return local_file_path;

        if (!elf_file.value()->HasGnuDebuglink()) {
          return ErrorMessage{
              absl::StrFormat("Module \"%s\" neither includes debug info, nor does it contain "
                              "a .gnu_debuglink section which could refer to a separate debug "
                              "info file.",
                              module_path)};
        }

        const auto debuglink = elf_file.value()->GetGnuDebugLinkInfo().value();
        ErrorMessageOr<std::filesystem::path> local_debuginfo_path =
            symbol_helper_.FindDebugInfoFileLocally(debuglink.path.filename().string(),
                                                    debuglink.crc32_checksum, GetAllSymbolPaths());
        if (local_debuginfo_path.has_error()) {
          return ErrorMessage{absl::StrFormat(
              "Module \"%s\" doesn't include debug info, and a separate debuginfo file wasn't "
              "found on this machine, when searching the folders from the Symbol Locations. Please "
              "make sure that the debuginfo file can be found in one of the added folders. "
              "According to the .gnu_debuglink section, the debuginfo file must be called \"%s\".",
              module_path, debuglink.path.string())};
        }

        elf_file = orbit_object_utils::CreateElfFile(local_debuginfo_path.value());
        if (elf_file.has_error()) return elf_file.error();
        return local_debuginfo_path;
      });
}

void SymbolLoader::RequestSymbolDownloadStop(std::string_view module_path) {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  if (symbol_files_currently_downloading_.contains(module_path)) {
    symbol_files_currently_downloading_.at(module_path).stop_source.RequestStop();
  }
}

bool SymbolLoader::IsModuleDownloading(std::string_view module_path) const {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  return symbol_files_currently_downloading_.contains(module_path);
}

orbit_data_views::SymbolLoadingState SymbolLoader::GetSymbolLoadingStateForModule(
    const orbit_client_data::ModuleData* module) const {
  ORBIT_CHECK(module != nullptr);
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());

  if (IsModuleDownloading(module->file_path())) return SymbolLoadingState::kDownloading;

  ModuleIdentifier module_id = module->module_id();
  if (symbols_currently_loading_.contains(module_id)) {
    return SymbolLoadingState::kLoading;
  }

  switch (module->GetLoadedSymbolsCompleteness()) {
    case ModuleData::SymbolCompleteness::kNoSymbols:
      break;
    case ModuleData::SymbolCompleteness::kDynamicLinkingAndUnwindInfo:
      return SymbolLoadingState::kFallback;
    case ModuleData::SymbolCompleteness::kDebugSymbols:
      return SymbolLoadingState::kLoaded;
  }

  if (download_disabled_modules_.contains(module->file_path())) {
    return SymbolLoadingState::kDisabled;
  }

  if (modules_with_symbol_loading_error_.contains(module_id)) {
    return SymbolLoadingState::kError;
  }

  return SymbolLoadingState::kUnknown;
}

bool SymbolLoader::IsSymbolLoadingInProgressForModule(
    const orbit_symbol_provider::ModuleIdentifier& module_id) const {
  ORBIT_CHECK(main_thread_id_ == std::this_thread::get_id());
  return symbols_currently_loading_.contains(module_id);
}

}  // namespace orbit_gl
