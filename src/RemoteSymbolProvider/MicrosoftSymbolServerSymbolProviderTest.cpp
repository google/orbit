// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <filesystem>
#include <memory>
#include <string>
#include <type_traits>
#include <utility>
#include <variant>

#include "Http/MockDownloadManager.h"
#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/NotFoundOr.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "OrbitBase/StopToken.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "RemoteSymbolProvider/MicrosoftSymbolServerSymbolProvider.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "Symbols/MockSymbolCache.h"
#include "TestUtils/TestUtils.h"

namespace orbit_remote_symbol_provider {

namespace {
using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_base::NotFoundOr;
using orbit_symbol_provider::ModuleIdentifier;
using orbit_symbol_provider::SymbolLoadingOutcome;
using orbit_symbol_provider::SymbolLoadingSuccessResult;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using ::testing::_;
using ::testing::Return;

const std::filesystem::path kSymbolCacheDir{"symbol/cache/path"};
const std::string kValidModuleName{"valid_module_name"};
const std::string kValidModuleBuildId{"ABCD12345678"};
const ModuleIdentifier kValidModuleId{absl::StrFormat("module/path/to/%s", kValidModuleName),
                                      kValidModuleBuildId};
const std::string kValidModuleDownloadUrl{
    absl::StrFormat("https://msdl.microsoft.com/download/symbols/%s.pdb/%s/%s.pdb",
                    kValidModuleName, kValidModuleBuildId, kValidModuleName)};

class MicrosoftSymbolServerSymbolProviderTest : public testing::Test {
 public:
  MicrosoftSymbolServerSymbolProviderTest()
      : symbol_provider_{&symbol_cache_, &download_manager_},
        executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
    EXPECT_CALL(symbol_cache_, GenerateCachedFilePath)
        .WillRepeatedly([](const std::filesystem::path& module_file_path) {
          auto file_name = absl::StrReplaceAll(module_file_path.string(), {{"/", "_"}});
          return kSymbolCacheDir / file_name;
        });
  }

  enum class DownloadResultState { kSuccess, kNotFound, kCanceled, kError };
  void SetUpDownloadManager(DownloadResultState expected_state, std::string expected_url,
                            std::string error_msg = "") {
    EXPECT_CALL(download_manager_, Download)
        .Times(1)
        .WillOnce([expected_state, expected_url = std::move(expected_url),
                   error_msg = std::move(error_msg)](
                      const std::string& url, const std::filesystem::path& /*save_file_path*/,
                      const orbit_base::StopToken& /*token*/)
                      -> Future<ErrorMessageOr<CanceledOr<NotFoundOr<void>>>> {
          EXPECT_EQ(url, expected_url);

          switch (expected_state) {
            case DownloadResultState::kSuccess:
              return {outcome::success()};
            case DownloadResultState::kNotFound:
              return {orbit_base::NotFound{""}};
            case DownloadResultState::kCanceled:
              return {orbit_base::Canceled{}};
            case DownloadResultState::kError:
              return {ErrorMessage{error_msg}};
          }

          ORBIT_UNREACHABLE();
        });
  }

 protected:
  const orbit_symbols::MockSymbolCache symbol_cache_;
  MicrosoftSymbolServerSymbolProvider symbol_provider_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor_;

 private:
  orbit_http::MockDownloadManager download_manager_;
};
}  // namespace

TEST_F(MicrosoftSymbolServerSymbolProviderTest, RetrieveModuleSuccess) {
  SetUpDownloadManager(DownloadResultState::kSuccess, kValidModuleDownloadUrl);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [this](const SymbolLoadingOutcome& result) {
        ASSERT_TRUE(orbit_symbol_provider::IsSuccessResult(result));
        SymbolLoadingSuccessResult success_result = orbit_symbol_provider::GetSuccessResult(result);
        EXPECT_EQ(success_result.path,
                  symbol_cache_.GenerateCachedFilePath(kValidModuleId.file_path));
        EXPECT_EQ(success_result.symbol_source,
                  SymbolLoadingSuccessResult::SymbolSource::kMicrosoftSymbolServer);

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(MicrosoftSymbolServerSymbolProviderTest, RetrieveModuleNotFound) {
  const ModuleIdentifier module_id{"module/path/to/some_module_name", "some_build_id"};
  const std::string expected_url{
      "https://msdl.microsoft.com/download/symbols/some_module_name.pdb/some_build_id/"
      "some_module_name.pdb"};
  SetUpDownloadManager(DownloadResultState::kNotFound, expected_url);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(module_id, stop_source.GetStopToken())
      .Then(executor_.get(), [](const SymbolLoadingOutcome& result) {
        ASSERT_TRUE(orbit_symbol_provider::IsNotFound(result));
        EXPECT_EQ(orbit_symbol_provider::GetNotFoundMessage(result),
                  "Symbols not found in Microsoft symbol server");

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(MicrosoftSymbolServerSymbolProviderTest, RetrieveModuleCanceled) {
  SetUpDownloadManager(DownloadResultState::kCanceled, kValidModuleDownloadUrl);

  // Here we use the mock downloader manager rather than the stop token to simulate the canceled
  // case.
  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [](const SymbolLoadingOutcome& result) {
        EXPECT_TRUE(orbit_symbol_provider::IsCanceled(result));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(MicrosoftSymbolServerSymbolProviderTest, RetrieveModuleError) {
  const std::string error_msg{"error"};
  SetUpDownloadManager(DownloadResultState::kError, kValidModuleDownloadUrl, error_msg);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [error_msg](const SymbolLoadingOutcome& result) {
        EXPECT_THAT(result, HasError(error_msg));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

}  // namespace orbit_remote_symbol_provider