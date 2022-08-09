// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QCoreApplication>
#include <QProcess>
#include <QStringList>
#include <memory>

#include "OrbitBase/CanceledOr.h"
#include "OrbitBase/File.h"
#include "OrbitBase/Future.h"
#include "OrbitBase/Result.h"
#include "OrbitBase/StopSource.h"
#include "OrbitBase/WhenAll.h"
#include "OrbitHttp/HttpDownloadManager.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_http {
using orbit_base::CanceledOr;
using orbit_base::FileExists;
using orbit_base::Future;
using orbit_base::IsCanceled;
using orbit_base::StopSource;
using orbit_test_utils::HasError;
using orbit_test_utils::HasNoError;

namespace {
class HttpDownloadManagerTest : public ::testing::Test {
 protected:
  HttpDownloadManagerTest()
      : executor_(orbit_qt_utils::MainThreadExecutorImpl::Create()), manager_() {
    local_http_server_process_ = new QProcess();
    local_http_server_process_->setProgram("python3");
    local_http_server_process_->setArguments(
        QStringList{"-m", R"(http.server)", "--bind", "localhost", "--directory",
                    QString::fromStdString(orbit_test::GetTestdataDir().string()), "8000"});
    local_http_server_process_->start();
  }

  ~HttpDownloadManagerTest() override { local_http_server_process_->kill(); }

  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor_;
  HttpDownloadManager manager_;

 private:
  QProcess* local_http_server_process_;
};
}  // namespace

TEST_F(HttpDownloadManagerTest, DownloadSingleSucceeded) {
  std::string valid_url = "http://localhost:8000/dllmain.dll";
  std::filesystem::path local_path{"C:/tmp/download.dll"};
  StopSource stop_source{};

  auto future = manager_.Download(valid_url, local_path, stop_source.GetStopToken());
  future.Then(executor_.get(), [&local_path](ErrorMessageOr<CanceledOr<void>> result) {
    EXPECT_THAT(result, HasNoError());
    EXPECT_FALSE(IsCanceled(result.value()));

    auto exists_or_error = FileExists(local_path);
    ASSERT_THAT(exists_or_error, HasNoError());
    EXPECT_TRUE(exists_or_error.value());
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(HttpDownloadManagerTest, DownloadSingleCanceled) {
  std::string valid_url = "http://localhost:8000/dllmain.dll";
  std::filesystem::path local_path{"C:/tmp/download.dll"};
  StopSource stop_source{};
  stop_source.RequestStop();

  auto future = manager_.Download(valid_url, local_path, stop_source.GetStopToken());
  future.Then(executor_.get(), [](ErrorMessageOr<CanceledOr<void>> result) {
    EXPECT_THAT(result, HasNoError());
    EXPECT_TRUE(IsCanceled(result.value()));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(HttpDownloadManagerTest, DownloadSingleFailed) {
  std::string invalid_url = "http://localhost:8000/non_exist.dll";
  std::filesystem::path local_path{"C:/tmp/download.dll"};
  StopSource stop_source{};

  auto future = manager_.Download(invalid_url, local_path, stop_source.GetStopToken());
  future.Then(executor_.get(), [](ErrorMessageOr<CanceledOr<void>> result) {
    EXPECT_THAT(result, HasError("File not found"));
    QCoreApplication::exit();
  });

  QCoreApplication::exec();
}

TEST_F(HttpDownloadManagerTest, DownloadMultipleSucceeded) {
  constexpr size_t kDownloadCounts = 3;
  const std::array<std::string, kDownloadCounts> kURLs = {"http://localhost:8000/dllmain.dll",
                                                          "http://localhost:8000/non_exist.dll",
                                                          "http://localhost:8000/hello_world_elf"};
  const std::array<std::filesystem::path, kDownloadCounts> kLocalPaths{
      "C:/tmp/download01.dll", "C:/tmp/non_exist.dll", "C:/tmp/download02_elf"};
  std::array<StopSource, kDownloadCounts> stop_sources{};

  std::vector<Future<ErrorMessageOr<CanceledOr<void>>>> futures;
  futures.reserve(kDownloadCounts);
  for (size_t i = 0; i < kDownloadCounts; ++i) {
    auto future = manager_.Download(kURLs[i], kLocalPaths[i], stop_sources[i].GetStopToken());
    futures.emplace_back(std::move(future));
  }

  orbit_base::WhenAll(absl::MakeConstSpan(futures))
      .Then(executor_.get(), [&kLocalPaths](std::vector<ErrorMessageOr<CanceledOr<void>>> results) {
        EXPECT_THAT(results[0], HasNoError());
        EXPECT_THAT(results[1], HasError("File not found"));
        EXPECT_THAT(results[2], HasNoError());

        auto exists_or_error = FileExists(kLocalPaths[0]);
        ASSERT_THAT(exists_or_error, HasNoError());
        EXPECT_TRUE(exists_or_error.value());

        exists_or_error = FileExists(kLocalPaths[2]);
        ASSERT_THAT(exists_or_error, HasNoError());
        EXPECT_TRUE(exists_or_error.value());

        QCoreApplication::exit();
      });

  QCoreApplication::exec();
}

}  // namespace orbit_http