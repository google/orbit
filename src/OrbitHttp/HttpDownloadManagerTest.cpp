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
#include "OrbitBase/TemporaryFile.h"
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
using orbit_base::TemporaryFile;
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

  ~HttpDownloadManagerTest() override {
    local_http_server_process_->kill();
    for (const auto& file_path : files_to_remove_) (void)orbit_base::RemoveFile(file_path);
  }

  std::filesystem::path GetTemporaryFilePath() {
    auto temporary_file_or_error = orbit_base::TemporaryFile::Create();
    EXPECT_THAT(temporary_file_or_error, HasNoError());

    TemporaryFile temporary_file = std::move(temporary_file_or_error.value());
    std::filesystem::path file_path = temporary_file.file_path();
    temporary_file.CloseAndRemove();

    // No matter the file is downloaded or not, we will try to remove it in the end.
    files_to_remove_.push_back(file_path);

    return file_path;
  }

  std::shared_ptr<orbit_qt_utils::MainThreadExecutorImpl> executor_;
  HttpDownloadManager manager_;

 private:
  QProcess* local_http_server_process_;
  std::vector<std::filesystem::path> files_to_remove_;
};
}  // namespace

TEST_F(HttpDownloadManagerTest, DownloadSingleSucceeded) {
  std::string valid_url = "http://localhost:8000/dllmain.dll";
  auto local_path = GetTemporaryFilePath();
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
  auto local_path = GetTemporaryFilePath();
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
  auto local_path = GetTemporaryFilePath();
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
      GetTemporaryFilePath(), GetTemporaryFilePath(), GetTemporaryFilePath()};
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