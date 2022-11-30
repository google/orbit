// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>
#include <absl/strings/string_view.h>
#include <stdlib.h>

#include <QApplication>
#include <QString>
#include <atomic>
#include <filesystem>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "CaptureClient/LoadCapture.h"
#include "CaptureFile/CaptureFile.h"
#include "ClientData/ScopeId.h"
#include "MizarBase/BaselineOrComparison.h"
#include "MizarBase/ThreadId.h"
#include "MizarData/BaselineAndComparison.h"
#include "MizarData/MizarData.h"
#include "MizarData/MizarDataProvider.h"
#include "MizarWidgets/MizarMainWindow.h"
#include "OrbitBase/Logging.h"
#include "OrbitBase/Result.h"

using ::orbit_client_data::ScopeId;
using ::orbit_mizar_base::Baseline;
using ::orbit_mizar_base::Comparison;
using ::orbit_mizar_base::TID;

[[nodiscard]] static ErrorMessageOr<void> LoadCapture(orbit_mizar_data::MizarData* data,
                                                      std::filesystem::path path) {
  OUTCOME_TRY(auto capture_file, orbit_capture_file::CaptureFile::OpenForReadWrite(path));
  std::atomic<bool> capture_loading_cancellation_requested = false;

  // The treatment is the same for CaptureOutcome::kComplete, CaptureOutcome::kCancelled
  std::ignore = orbit_capture_client::LoadCapture(data, capture_file.get(),
                                                  &capture_loading_cancellation_requested);
  return outcome::success();
}

ABSL_FLAG(std::string, baseline_path, "", "The path to the baseline capture file");
ABSL_FLAG(std::string, comparison_path, "", "The path to the comparison capture file");

static std::string ExpandPathHomeFolder(std::string_view path) {
  constexpr const char* kHomeForderEnvVariable = "HOME";
  if (path[0] == '~') return std::string{getenv(kHomeForderEnvVariable)}.append(path.substr(1));
  return std::string{path};
}

[[nodiscard]] static QString MakeFileName(const std::filesystem::path& path) {
  return QString::fromStdString(path.filename().string());
}

int main(int argc, char** argv) {
  // The main in its current state is used to testing/experimenting and serves no other purpose
  absl::ParseCommandLine(argc, argv);

  const std::filesystem::path baseline_path =
      ExpandPathHomeFolder(absl::GetFlag(FLAGS_baseline_path));
  const std::filesystem::path comparison_path =
      ExpandPathHomeFolder(absl::GetFlag(FLAGS_comparison_path));

  auto baseline = std::make_unique<orbit_mizar_data::MizarData>();
  auto comparison = std::make_unique<orbit_mizar_data::MizarData>();

  auto baseline_error_message = LoadCapture(baseline.get(), baseline_path);
  if (baseline_error_message.has_error()) {
    ORBIT_ERROR("%s", baseline_error_message.error().message());
    return 1;
  }

  auto comparison_error_message = LoadCapture(comparison.get(), comparison_path);
  if (comparison_error_message.has_error()) {
    ORBIT_ERROR("%s", comparison_error_message.error().message());
    return 1;
  }

  orbit_mizar_data::BaselineAndComparison bac =
      CreateBaselineAndComparison(std::move(baseline), std::move(comparison));

  QApplication app(argc, argv);
  QApplication::setOrganizationName("The Orbit Authors");
  QApplication::setApplicationName("Mizar comparison tool");

  orbit_mizar_widgets::MizarMainWindow main_window(
      &bac, Baseline<QString>(MakeFileName(baseline_path)),
      Comparison<QString>(MakeFileName(comparison_path)));
  main_window.show();
  return QApplication::exec();
}