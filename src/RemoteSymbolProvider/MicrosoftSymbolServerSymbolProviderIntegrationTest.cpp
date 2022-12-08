// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/hash/hash.h>
#include <absl/strings/str_replace.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <utility>
#include <variant>

#include "GrpcProtos/symbol.pb.h"
#include "Http/HttpDownloadManager.h"
#include "ObjectUtils/SymbolsFile.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "RemoteSymbolProvider/MicrosoftSymbolServerSymbolProvider.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "Symbols/MockSymbolCache.h"
#include "TestUtils/TemporaryFile.h"
#include "TestUtils/TestUtils.h"

namespace orbit_remote_symbol_provider {

using orbit_grpc_protos::SymbolInfo;
using orbit_test_utils::HasNoError;
using orbit_test_utils::HasValue;

TEST(MicrosoftSymbolServerSymbolProviderIntegrationTest, RetrieveWindowsPdbAndLoadDebugSymbols) {
  auto temporary_file_or_error = orbit_test_utils::TemporaryFile::Create();
  ASSERT_THAT(temporary_file_or_error, HasNoError());
  orbit_test_utils::TemporaryFile temporary_file = std::move(temporary_file_or_error.value());
  ASSERT_TRUE(temporary_file.file_path().has_parent_path());
  const std::filesystem::path symbol_cache_dir = temporary_file.file_path().parent_path();

  orbit_symbols::MockSymbolCache symbol_cache;
  EXPECT_CALL(symbol_cache, GenerateCachedFilePath)
      .WillRepeatedly([&symbol_cache_dir](const std::filesystem::path& module_file_path) {
        auto file_name = absl::StrReplaceAll(module_file_path.string(), {{"/", "_"}});
        return symbol_cache_dir / file_name;
      });

  orbit_http::HttpDownloadManager download_manager{};
  MicrosoftSymbolServerSymbolProvider symbol_provider{&symbol_cache, &download_manager};

  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor{
      orbit_qt_utils::MainThreadExecutorImpl::Create()};

  const std::string valid_module_name{"d3d11.pdb"};
  const std::string valid_module_build_id{"FF5440275BFED43A86CC2B1F287A72151"};
  orbit_symbol_provider::ModuleIdentifier valid_module_id{valid_module_name, valid_module_build_id};

  orbit_base::StopSource stop_source{};

  symbol_provider.RetrieveSymbols(valid_module_id, stop_source.GetStopToken())
      .Then(executor.get(), [](const orbit_symbol_provider::SymbolLoadingOutcome& result) {
        ASSERT_TRUE(orbit_symbol_provider::IsSuccessResult(result));
        orbit_symbol_provider::SymbolLoadingSuccessResult success_result =
            orbit_symbol_provider::GetSuccessResult(result);

        auto exists_or_error = orbit_base::FileOrDirectoryExists(success_result.path);
        ASSERT_THAT(exists_or_error, HasValue(true));

        constexpr uint64_t kImageBase = 0x10000;
        auto symbols_file_or_error = orbit_object_utils::CreateSymbolsFile(
            success_result.path, orbit_object_utils::ObjectFileInfo{kImageBase});
        ASSERT_THAT(symbols_file_or_error, HasNoError());

        auto symbols_result = symbols_file_or_error.value()->LoadDebugSymbols();
        ASSERT_THAT(symbols_result, HasNoError());

        absl::flat_hash_map<uint64_t, const SymbolInfo*> symbol_infos_by_address;
        for (const SymbolInfo& symbol_info : symbols_result.value().symbol_infos()) {
          symbol_infos_by_address.emplace(symbol_info.address(), &symbol_info);
        }
        ASSERT_EQ(symbol_infos_by_address.size(), 9573);

        {
          const SymbolInfo* symbol = symbol_infos_by_address[0x4aa90 + kImageBase];
          ASSERT_NE(symbol, nullptr);
          EXPECT_EQ(symbol->demangled_name(), "D3D11CreateDevice");
          EXPECT_EQ(symbol->size(), 0x100);
        }

        {
          const SymbolInfo* symbol = symbol_infos_by_address[0x3a800 + kImageBase];
          ASSERT_NE(symbol, nullptr);
          EXPECT_EQ(symbol->demangled_name(), "CContext::ValidateReclaimResources");
          EXPECT_EQ(symbol->size(), 0x100);
        }

        auto removed_or_error = orbit_base::RemoveFile(success_result.path);
        ASSERT_THAT(removed_or_error, HasNoError());
        EXPECT_TRUE(removed_or_error.value());

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

}  // namespace orbit_remote_symbol_provider