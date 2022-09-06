// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <absl/strings/str_replace.h>
#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QPointer>
#include <string_view>

#include "Http/MockDownloadManager.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "OrbitGgp/MockClient.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "RemoteSymbolProvider/StadiaSymbolStoreSymbolProvider.h"
#include "Symbols/MockSymbolCache.h"
#include "TestUtils/TestUtils.h"

namespace orbit_remote_symbol_provider {

namespace {
using orbit_base::Canceled;
using orbit_base::CanceledOr;
using orbit_base::Future;
using orbit_ggp::SymbolDownloadInfo;
using SymbolDownloadQuery = orbit_ggp::Client::SymbolDownloadQuery;
using orbit_symbol_provider::ModuleIdentifier;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;
using ::testing::_;
using ::testing::Return;

const std::filesystem::path kSymbolCacheDir{"symbol/cache/path"};
const std::string kValidModuleName{"only_available_module_name"};
const std::string kValidModuleBuildId{"ABCD12345678"};
const ModuleIdentifier kValidModuleId{absl::StrFormat("module/path/to/%s", kValidModuleName),
                                      kValidModuleBuildId};

class StadiaSymbolStoreSymbolProviderTest : public testing::Test {
 public:
  StadiaSymbolStoreSymbolProviderTest()
      : symbol_provider_{&symbol_cache_, &download_manager_, &ggp_client_},
        executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()) {
    EXPECT_CALL(symbol_cache_, GenerateCachedFileName)
        .WillRepeatedly([](const std::filesystem::path& module_file_path) {
          auto file_name = absl::StrReplaceAll(module_file_path.string(), {{"/", "_"}});
          return kSymbolCacheDir / file_name;
        });
  }

  enum class GgpClientState { kWorking, kTimeout };
  void SetUpGgpClient(GgpClientState ggp_client_state) {
    EXPECT_CALL(ggp_client_, GetSymbolDownloadInfoAsync(_))
        .Times(1)
        .WillOnce([ggp_client_state = std::move(ggp_client_state)](
                      const std::vector<SymbolDownloadQuery>& download_queries)
                      -> Future<ErrorMessageOr<std::vector<SymbolDownloadInfo>>> {
          // Stadia symbol provider queries only one module each time when making the ggp call.
          ORBIT_CHECK(download_queries.size() == 1);

          if (ggp_client_state == GgpClientState::kTimeout) return {ErrorMessage{"Timeout"}};

          if (download_queries.front().module_name != kValidModuleName ||
              download_queries.front().build_id != kValidModuleBuildId) {
            return {ErrorMessage{"Symbols not found"}};
          }

          SymbolDownloadInfo download_info;
          download_info.file_id = QString::fromStdString(
              absl::StrFormat("symbolFiles/%s/%s", kValidModuleBuildId, kValidModuleName));
          download_info.url = "valid_url_for_symbol";
          return {std::vector<SymbolDownloadInfo>{download_info}};
        });
  }

  enum class DownloadResultState { kSuccess, kCanceled, kError };
  void SetUpDownloadManager(DownloadResultState expected_state, std::string error_msg = "") {
    EXPECT_CALL(download_manager_, Download)
        .Times(1)
        .WillOnce([expected_state = std::move(expected_state), error_msg = std::move(error_msg)](
                      std::string /*url*/, std::filesystem::path /*save_file_path*/,
                      orbit_base::StopToken /*token*/) -> Future<ErrorMessageOr<CanceledOr<void>>> {
          switch (expected_state) {
            case DownloadResultState::kSuccess:
              return {outcome::success()};
            case DownloadResultState::kCanceled:
              return {Canceled{}};
            case DownloadResultState::kError:
              return {ErrorMessage{error_msg}};
          }

          ORBIT_UNREACHABLE();
        });
  }

 protected:
  orbit_symbols::MockSymbolCache symbol_cache_;
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
      .Then(executor_.get(), [this](ErrorMessageOr<CanceledOr<std::filesystem::path>> result) {
        EXPECT_THAT(result, HasNoError());
        EXPECT_FALSE(IsCanceled(result.value()));
        const auto& local_file_path = std::get<std::filesystem::path>(result.value());
        EXPECT_EQ(local_file_path, symbol_cache_.GenerateCachedFileName(kValidModuleId.file_path));

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
      .Then(executor_.get(), [](ErrorMessageOr<CanceledOr<std::filesystem::path>> result) {
        EXPECT_THAT(result, HasNoError());
        EXPECT_TRUE(IsCanceled(result.value()));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleDownloadError) {
  SetUpGgpClient(GgpClientState::kWorking);
  SetUpDownloadManager(DownloadResultState::kError, "Failed to download");

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [](ErrorMessageOr<CanceledOr<std::filesystem::path>> result) {
        EXPECT_THAT(result, HasError("Failed to download"));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleNotFound) {
  SetUpGgpClient(GgpClientState::kWorking);

  orbit_symbol_provider::ModuleIdentifier module_id{"module/path/to/some_module_name",
                                                    "some_build_id"};
  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(module_id, stop_source.GetStopToken())
      .Then(executor_.get(), [](ErrorMessageOr<CanceledOr<std::filesystem::path>> result) {
        EXPECT_THAT(result, HasError("Symbols not found"));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

TEST_F(StadiaSymbolStoreSymbolProviderTest, RetrieveModuleTimeout) {
  SetUpGgpClient(GgpClientState::kTimeout);

  orbit_base::StopSource stop_source{};
  symbol_provider_.RetrieveSymbols(kValidModuleId, stop_source.GetStopToken())
      .Then(executor_.get(), [](ErrorMessageOr<CanceledOr<std::filesystem::path>> result) {
        EXPECT_THAT(result, HasError("Timeout"));

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

}  // namespace orbit_remote_symbol_provider