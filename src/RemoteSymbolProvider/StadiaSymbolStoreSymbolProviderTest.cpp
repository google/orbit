// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QString>
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
#include "OrbitGgp/Client.h"
#include "OrbitGgp/MockClient.h"
#include "OrbitGgp/SymbolDownloadInfo.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "RemoteSymbolProvider/StadiaSymbolStoreSymbolProvider.h"
#include "SymbolProvider/ModuleIdentifier.h"
#include "SymbolProvider/SymbolLoadingOutcome.h"
#include "Symbols/MockSymbolCache.h"
#include "TestUtils/TestUtils.h"

namespace orbit_remote_symbol_provider {

namespace {
using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_base::NotFoundOr;
using orbit_ggp::SymbolDownloadInfo;
using SymbolDownloadQuery = orbit_ggp::Client::SymbolDownloadQuery;
using orbit_symbol_provider::ModuleIdentifier;
using orbit_symbol_provider::SymbolLoadingOutcome;
using orbit_symbol_provider::SymbolLoadingSuccessResult;
using orbit_test_utils::HasError;
using ::testing::_;

const std::filesystem::path kSymbolCacheDir{"symbol/cache/path"};
const std::string kValidModuleName{"valid_module_name"};
const std::string kValidModuleBuildId{"ABCD12345678"};
const ModuleIdentifier kValidModuleId{absl::StrFormat("module/path/to/%s", kValidModuleName),
                                      kValidModuleBuildId};

const std::string kFailedToDownloadMsg{"Failed to download"};
const std::string kGgpTimeoutMsg{"Timeout"};

class StadiaSymbolStoreSymbolProviderTest : public testing::Test {
 public:
  StadiaSymbolStoreSymbolProviderTest()
      : symbol_provider_{&symbol_cache_, &download_manager_, &ggp_client_},
        executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
    EXPECT_CALL(symbol_cache_, GenerateCachedFilePath)
        .WillRepeatedly([](const std::filesystem::path& module_file_path) {
          auto file_name = absl::StrReplaceAll(module_file_path.string(), {{"/", "_"}});
          return kSymbolCacheDir / file_name;
        });
  }

  enum class GgpClientState { kWorking, kTimeout };
  void SetUpGgpClient(GgpClientState ggp_client_state) {
    EXPECT_CALL(ggp_client_, GetSymbolDownloadInfoAsync(_))
        .Times(1)
        .WillOnce([ggp_client_state](const SymbolDownloadQuery& download_query)
                      -> Future<ErrorMessageOr<NotFoundOr<SymbolDownloadInfo>>> {
          if (ggp_client_state == GgpClientState::kTimeout) return {ErrorMessage{kGgpTimeoutMsg}};

          if (download_query.module_name != kValidModuleName ||
              download_query.build_id != kValidModuleBuildId) {
            return {orbit_base::NotFound{""}};
          }

          SymbolDownloadInfo download_info;
          download_info.file_id = QString::fromStdString(
              absl::StrFormat("symbolFiles/%s/%s", kValidModuleBuildId, kValidModuleName));
          download_info.url = "valid_url_for_symbol";
          return {download_info};
        });
  }

  enum class DownloadResultState { kSuccess, kCanceled, kError };
  void SetUpDownloadManager(DownloadResultState expected_state, std::string error_msg = "") {
    EXPECT_CALL(download_manager_, Download)
        .Times(1)
        .WillOnce([expected_state, error_msg = std::move(error_msg)](
                      std::string /*url*/, std::filesystem::path /*save_file_path*/,
                      orbit_base::StopToken /*token*/)
                      -> Future<ErrorMessageOr<CanceledOr<NotFoundOr<void>>>> {
          switch (expected_state) {
            case DownloadResultState::kSuccess:
              return {outcome::success()};
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
  StadiaSymbolStoreSymbolProvider symbol_provider_;
  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor_;

 private:
  orbit_http::MockDownloadManager download_manager_;
  orbit_ggp::MockClient ggp_client_;
};
}  // namespace

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleSuccess) {
  SetUpGgpClient(GgpClientState::kWorking);
  SetUpDownloadManager(DownloadResultState::kSuccess);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [this](const SymbolLoadingOutcome& result) {
        ASSERT_TRUE(orbit_symbol_provider::IsSuccessResult(result));
        SymbolLoadingSuccessResult success_result = orbit_symbol_provider::GetSuccessResult(result);
        EXPECT_EQ(success_result.path,
                  symbol_cache_.GenerateCachedFilePath(kValidModuleId.file_path));
        EXPECT_EQ(success_result.symbol_source,
                  SymbolLoadingSuccessResult::SymbolSource::kStadiaSymbolStore);

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleCanceled) {
  SetUpGgpClient(GgpClientState::kWorking);
  SetUpDownloadManager(DownloadResultState::kCanceled);

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

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleDownloadError) {
  SetUpGgpClient(GgpClientState::kWorking);
  SetUpDownloadManager(DownloadResultState::kError, kFailedToDownloadMsg);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [](const SymbolLoadingOutcome& result) {
        EXPECT_THAT(result, HasError(kFailedToDownloadMsg));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleNotFound) {
  SetUpGgpClient(GgpClientState::kWorking);

  const ModuleIdentifier module_id{"module/path/to/some_module_name", "some_build_id"};
  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(module_id, stop_source.GetStopToken())
      .Then(executor_.get(), [](const SymbolLoadingOutcome& result) {
        ASSERT_TRUE(orbit_symbol_provider::IsNotFound(result));
        EXPECT_EQ(orbit_symbol_provider::GetNotFoundMessage(result),
                  "Symbols not found in Stadia symbol store");

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleTimeout) {
  SetUpGgpClient(GgpClientState::kTimeout);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [](const SymbolLoadingOutcome& result) {
        EXPECT_THAT(result, HasError(kGgpTimeoutMsg));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

}  // namespace orbit_remote_symbol_provider