// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/flags/flag.h>
#include <absl/flags/parse.h>

#include <QApplication>
#include <filesystem>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientFlags/ClientFlags.h"
#include "ConfigWidgets/SymbolsDialog.h"
#include "GrpcProtos/module.pb.h"
#include "SymbolPaths/QSettingsBasedStorageManager.h"

int main(int argc, char* argv[]) {
  absl::ParseCommandLine(argc, argv);
  QApplication app{argc, argv};
  QApplication::setApplicationName("SymbolsDialogDemo");
  QApplication::setOrganizationName("The Orbit Authors");

  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_name("test.so");
  module_info.set_file_path("/usr/modules/test.so");
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);

  orbit_client_data::ModuleData module{module_info};

  orbit_symbol_paths::QSettingsBasedStorageManager symbol_paths_storage_manager;
  orbit_config_widgets::SymbolsDialog dialog{&symbol_paths_storage_manager,
                                             absl::GetFlag(FLAGS_enable_unsafe_symbols), &module};
  const int result_code = dialog.exec();

  return result_code;
}