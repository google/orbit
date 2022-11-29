// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/types/span.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QLabel>
#include <QListWidget>
#include <QMessageBox>
#include <QMetaObject>
#include <QPushButton>
#include <QString>
#include <QTest>
#include <Qt>
#include <filesystem>
#include <optional>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ClientSymbols/PersistentStorageManager.h"
#include "ConfigWidgets/SymbolLocationsDialog.h"
#include "GrpcProtos/module.pb.h"
#include "OrbitBase/Result.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_config_widgets {

using orbit_client_symbols::ModuleSymbolFileMappings;
using orbit_test_utils::HasError;
using orbit_test_utils::HasValue;

class MockPersistentStorageManager : public orbit_client_symbols::PersistentStorageManager {
 public:
  MOCK_METHOD(void, SavePaths, (absl::Span<const std::filesystem::path>), (override));
  MOCK_METHOD(std::vector<std::filesystem::path>, LoadPaths, (), (override));
  MOCK_METHOD(void, SaveModuleSymbolFileMappings, (const ModuleSymbolFileMappings&), (override));
  MOCK_METHOD((ModuleSymbolFileMappings), LoadModuleSymbolFileMappings, (), (override));
  MOCK_METHOD(void, SaveDisabledModulePaths, (const absl::flat_hash_set<std::string>&), (override));
  MOCK_METHOD(absl::flat_hash_set<std::string>, LoadDisabledModulePaths, (), (override));
  MOCK_METHOD(void, SaveEnableStadiaSymbolStore, (bool), (override));
  MOCK_METHOD(bool, LoadEnableStadiaSymbolStore, (), (override));
  MOCK_METHOD(void, SaveEnableMicrosoftSymbolServer, (bool), (override));
  MOCK_METHOD(bool, LoadEnableMicrosoftSymbolServer, (), (override));
};

class SymbolLocationsDialogTest : public ::testing::Test {
 protected:
  void SetLoadPaths(std::vector<std::filesystem::path> load_paths) {
    EXPECT_CALL(mock_storage_manager_, LoadPaths).WillOnce(testing::Return(std::move(load_paths)));
  }
  void SetExpectSavePaths(std::vector<std::filesystem::path> save_paths) {
    EXPECT_CALL(mock_storage_manager_, SavePaths)
        .WillOnce(
            [save_paths = std::move(save_paths)](absl::Span<const std::filesystem::path> paths) {
              EXPECT_EQ(paths, save_paths);
            });
  }
  void SetLoadMappings(ModuleSymbolFileMappings load_mappings) {
    EXPECT_CALL(mock_storage_manager_, LoadModuleSymbolFileMappings)
        .WillOnce(testing::Return(std::move(load_mappings)));
  }
  void SetExpectedSaveMappings(ModuleSymbolFileMappings expected_save_mappings) {
    EXPECT_CALL(mock_storage_manager_, SaveModuleSymbolFileMappings)
        .WillOnce([expected_save_mappings = std::move(expected_save_mappings)](
                      const ModuleSymbolFileMappings& mappings) {
          ASSERT_EQ(expected_save_mappings.size(), mappings.size());
          for (const auto& [module_path, symbol_file_path] : mappings) {
            EXPECT_TRUE(expected_save_mappings.contains(module_path));
            EXPECT_EQ(expected_save_mappings.at(module_path), symbol_file_path);
          }
        });
  }
  void SetLoadAndExpectedSaveEmpty() {
    SetLoadPaths({});
    SetExpectSavePaths({});
    SetLoadMappings({});
    SetExpectedSaveMappings({});
  }
  static void ScheduleMessageBoxCancellation(SymbolLocationsDialog* dialog, bool* called) {
    QMetaObject::invokeMethod(
        dialog,
        [=]() {
          auto* message_box = dialog->findChild<QMessageBox*>();
          ASSERT_NE(message_box, nullptr);
          message_box->reject();
          *called = true;
        },
        Qt::QueuedConnection);
  }
  static void ScheduleMessageBoxAcceptOverride(SymbolLocationsDialog* dialog, bool* called) {
    QMetaObject::invokeMethod(
        dialog,
        [=]() {
          auto* message_box = dialog->findChild<QMessageBox*>();
          ASSERT_NE(message_box, nullptr);
          // Since the override button is a "custom button" (added via addButton), it is hard to get
          // ahold of a point to it. Instead the message box is accepted here by hitting the enter
          // key. As a side effect, this also tests that the override button has focus.
          QTest::keyClick(message_box, Qt::Key_Enter);
          *called = true;
        },
        Qt::QueuedConnection);
  }

  MockPersistentStorageManager mock_storage_manager_;
};

TEST_F(SymbolLocationsDialogTest, ConstructEmpty) {
  SetLoadAndExpectedSaveEmpty();

  SymbolLocationsDialog dialog{&mock_storage_manager_};

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), 0);

  auto* module_headline_label = dialog.findChild<QLabel*>("moduleHeadlineLabel");
  ASSERT_NE(module_headline_label, nullptr);
  EXPECT_FALSE(module_headline_label->isVisible());
}

TEST_F(SymbolLocationsDialogTest, ConstructNonEmptyNoUnsafeSymbols) {
  const std::vector<std::filesystem::path> test_paths{"/path/to/somewhere",
                                                      "path/to/somewhere/else"};
  ModuleSymbolFileMappings mappings;
  mappings["path/to/module/on/instance"] = std::filesystem::path{"c:"} / "local" / "path";
  mappings["/different/module/on/instance"] = std::filesystem::path{"test"} / "unix" / "path";

  SetLoadPaths(test_paths);
  SetExpectSavePaths(test_paths);
  SetLoadMappings(mappings);
  SetExpectedSaveMappings(mappings);

  SymbolLocationsDialog dialog{&mock_storage_manager_};

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  // Since unsafe symbols are off, the list shows only the paths, not the mappings
  EXPECT_EQ(list_widget->count(), test_paths.size());
}

TEST_F(SymbolLocationsDialogTest, ConstructNonEmptyWithUnsafeSymbols) {
  const std::vector<std::filesystem::path> test_paths{"/path/to/somewhere",
                                                      "path/to/somewhere/else"};
  ModuleSymbolFileMappings mappings;
  mappings["path/to/module/on/instance"] = std::filesystem::path{"c:"} / "local" / "path";
  mappings["/different/module/on/instance"] = std::filesystem::path{"test"} / "unix" / "path";

  SetLoadPaths(test_paths);
  SetExpectSavePaths(test_paths);
  SetLoadMappings(mappings);
  SetExpectedSaveMappings(mappings);

  SymbolLocationsDialog dialog{&mock_storage_manager_, true};

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), test_paths.size() + mappings.size());
}

TEST_F(SymbolLocationsDialogTest, ConstructWithElfModuleNoBuildId) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path("/path/to/lib.so");
  module_info.set_name("lib.so");
  orbit_client_data::ModuleData module{module_info};

  SetLoadAndExpectedSaveEmpty();

  SymbolLocationsDialog dialog{&mock_storage_manager_, true, &module};

  auto* add_folder_button = dialog.findChild<QPushButton*>("addFolderButton");
  ASSERT_NE(add_folder_button, nullptr);
  EXPECT_FALSE(add_folder_button->isEnabled());
  EXPECT_THAT(add_folder_button->toolTip().toStdString(),
              testing::HasSubstr("does not have a build ID"));
  EXPECT_THAT(add_folder_button->toolTip().toStdString(), testing::HasSubstr(module.name()));
}

TEST_F(SymbolLocationsDialogTest, ConstructWithElfModuleWithBuildId) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path("/path/to/lib.so");
  module_info.set_name("lib.so");
  module_info.set_build_id("some build id");
  orbit_client_data::ModuleData module{module_info};

  SetLoadAndExpectedSaveEmpty();

  SymbolLocationsDialog dialog{&mock_storage_manager_, false, &module};

  auto* add_folder_button = dialog.findChild<QPushButton*>("addFolderButton");
  ASSERT_NE(add_folder_button, nullptr);
  EXPECT_TRUE(add_folder_button->isEnabled());

  auto* module_headline_label = dialog.findChild<QLabel*>("moduleHeadlineLabel");
  ASSERT_NE(module_headline_label, nullptr);
  EXPECT_THAT(module_headline_label->text().toStdString(), testing::HasSubstr(module.name()));
}

TEST_F(SymbolLocationsDialogTest, TryAddSymbolPath) {
  std::filesystem::path path{"/absolute/test/path1"};
  std::filesystem::path path_2{R"(C:\windows\test\path1)"};
  std::filesystem::path file{"/path/to/file.ext"};
  std::vector<std::filesystem::path> save_paths = {path, path_2, file};

  SetLoadPaths({});
  SetExpectSavePaths(save_paths);
  SetLoadMappings({});
  SetExpectedSaveMappings({});

  SymbolLocationsDialog dialog{&mock_storage_manager_};
  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), 0);

  {  // simple add succeeds
    auto result = dialog.TryAddSymbolPath(path);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(list_widget->count(), 1);
  }

  {  // add the same path again -> warning that needs to be dismissed and nothing changes
    auto result = dialog.TryAddSymbolPath(path);
    ASSERT_TRUE(result.has_error());
    EXPECT_THAT(result, HasError("Unable to add selected path, it is already part of the list."));
    EXPECT_EQ(list_widget->count(), 1);
  }

  {  // add different path succeeds
    auto result = dialog.TryAddSymbolPath(path_2);
    EXPECT_TRUE(result.has_value());
    EXPECT_EQ(list_widget->count(), 2);
  }

  {  // add path to file should succeed
    auto result = dialog.TryAddSymbolPath(file);
    EXPECT_EQ(list_widget->count(), 3);
  }
}

TEST_F(SymbolLocationsDialogTest, TryAddSymbolFileWithoutModule) {
  std::filesystem::path hello_world_elf = orbit_test::GetTestdataDir() / "hello_world_elf";
  std::vector<std::filesystem::path> save_paths{hello_world_elf};

  SetLoadPaths({});
  SetExpectSavePaths(save_paths);
  SetLoadMappings({});
  SetExpectedSaveMappings({});

  SymbolLocationsDialog dialog{&mock_storage_manager_};

  // success case
  {
    auto result = dialog.TryAddSymbolFile(hello_world_elf);
    EXPECT_TRUE(result.has_value());
  }

  // fails because not an object_file
  std::filesystem::path text_file = orbit_test::GetTestdataDir() / "textfile.txt";
  {
    auto result = dialog.TryAddSymbolFile(text_file);
    EXPECT_THAT(result, HasError("The selected file is not a viable symbol file"));
  }

  // fails because no build-id
  std::filesystem::path hello_world_elf_no_build_id =
      orbit_test::GetTestdataDir() / "hello_world_elf_no_build_id";
  {
    auto result = dialog.TryAddSymbolFile(hello_world_elf_no_build_id);

    EXPECT_THAT(result, HasError("The selected file does not contain a build id"));
  }
}

TEST_F(SymbolLocationsDialogTest, TryAddSymbolFileWithModuleNoOverride) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path((orbit_test::GetTestdataDir() / "no_symbols_elf").string());
  module_info.set_build_id("b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  orbit_client_data::ModuleData module{module_info};

  std::filesystem::path no_symbols_elf_debug =
      orbit_test::GetTestdataDir() / "no_symbols_elf.debug";
  std::vector<std::filesystem::path> save_paths{no_symbols_elf_debug};

  SetLoadPaths({});
  SetExpectSavePaths(save_paths);
  SetLoadMappings({});
  SetExpectedSaveMappings({});

  SymbolLocationsDialog dialog{&mock_storage_manager_, false, &module};

  // Success (build id matches)
  {
    auto result = dialog.TryAddSymbolFile(no_symbols_elf_debug);
    EXPECT_TRUE(result.has_value());
  }

  // fail (build id different)
  std::filesystem::path libc_debug = orbit_test::GetTestdataDir() / "libc.debug";
  {
    auto result = dialog.TryAddSymbolFile(libc_debug);
    EXPECT_THAT(result, HasError("The build ids of module and symbols file do not match."));
  }
}

TEST_F(SymbolLocationsDialogTest, TryAddSymbolFileOverrideStaleSymbols) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path((orbit_test::GetTestdataDir() / "no_symbols_elf").string());
  module_info.set_build_id("b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  orbit_client_data::ModuleData module{module_info};

  std::filesystem::path no_symbols_elf_debug =
      orbit_test::GetTestdataDir() / "no_symbols_elf.debug";
  std::filesystem::path no_symbols_elf_stale_debug =
      orbit_test::GetTestdataDir() / "no_symbols_elf_stale.debug";

  SetLoadPaths({});
  SetExpectSavePaths({no_symbols_elf_debug});
  SetLoadMappings({});
  ModuleSymbolFileMappings mappings;
  mappings[module.file_path()] = no_symbols_elf_stale_debug;
  SetExpectedSaveMappings(mappings);

  SymbolLocationsDialog dialog(&mock_storage_manager_, true, &module);

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);

  {  // build id matches, symbols file is added without warning
    EXPECT_THAT(dialog.TryAddSymbolFile(no_symbols_elf_debug), HasValue());
    EXPECT_EQ(list_widget->count(), 1);
  }

  {  // build id mismatch. Warning is displayed and dismissed
    bool message_box_cancelled = false;
    ScheduleMessageBoxCancellation(&dialog, &message_box_cancelled);
    EXPECT_THAT(dialog.TryAddSymbolFile(no_symbols_elf_stale_debug), HasValue());
    EXPECT_TRUE(message_box_cancelled);
    EXPECT_EQ(list_widget->count(), 1);
  }

  {  // build id mismatch. Warning is displayed and accepted
    bool message_box_accepted = false;
    ScheduleMessageBoxAcceptOverride(&dialog, &message_box_accepted);
    EXPECT_THAT(dialog.TryAddSymbolFile(no_symbols_elf_stale_debug), HasValue());
    EXPECT_TRUE(message_box_accepted);
    EXPECT_EQ(list_widget->count(), 2);
  }
}

TEST_F(SymbolLocationsDialogTest, TryAddSymbolFileOverrideSymbolsNoBuildId) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path((orbit_test::GetTestdataDir() / "no_symbols_elf").string());
  module_info.set_build_id("b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  orbit_client_data::ModuleData module{module_info};

  std::filesystem::path symbols_file_no_build_id_debug =
      orbit_test::GetTestdataDir() / "symbols_file_no_build_id.debug";

  SetLoadPaths({});
  SetExpectSavePaths({});
  SetLoadMappings({});
  ModuleSymbolFileMappings mappings;
  mappings[module.file_path()] = symbols_file_no_build_id_debug;
  SetExpectedSaveMappings(mappings);

  SymbolLocationsDialog dialog(&mock_storage_manager_, true, &module);

  {  // build id mismatch. Warning is displayed and accepted
    bool message_box_accepted = false;
    ScheduleMessageBoxAcceptOverride(&dialog, &message_box_accepted);
    EXPECT_THAT(dialog.TryAddSymbolFile(symbols_file_no_build_id_debug), HasValue());
    EXPECT_TRUE(message_box_accepted);
  }
}

TEST_F(SymbolLocationsDialogTest, TryAddSymbolFileOverrideModuleNoBuildIdSymbolsNoBuildId) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path((orbit_test::GetTestdataDir() / "no_symbols_elf").string());
  orbit_client_data::ModuleData module{module_info};

  std::filesystem::path symbols_file_no_build_id_debug =
      orbit_test::GetTestdataDir() / "symbols_file_no_build_id.debug";

  SetLoadPaths({});
  SetExpectSavePaths({});
  SetLoadMappings({});
  ModuleSymbolFileMappings mappings;
  mappings[module.file_path()] = symbols_file_no_build_id_debug;
  SetExpectedSaveMappings(mappings);

  SymbolLocationsDialog dialog(&mock_storage_manager_, true, &module);

  {  // build id mismatch. Warning is displayed and accepted
    bool message_box_accepted = false;
    ScheduleMessageBoxAcceptOverride(&dialog, &message_box_accepted);
    EXPECT_THAT(dialog.TryAddSymbolFile(symbols_file_no_build_id_debug), HasValue());
    EXPECT_TRUE(message_box_accepted);
  }
}

TEST_F(SymbolLocationsDialogTest, RemoveButton) {
  SetLoadPaths({"random/path/entry"});
  SetExpectSavePaths({});
  ModuleSymbolFileMappings mappings;
  mappings["/path/to/module"] = std::filesystem::path{"other"} / "path";
  SetLoadMappings(std::move(mappings));
  SetExpectedSaveMappings({});

  SymbolLocationsDialog dialog{&mock_storage_manager_, true};

  auto* remove_button = dialog.findChild<QPushButton*>("removeButton");
  ASSERT_NE(remove_button, nullptr);
  EXPECT_FALSE(remove_button->isEnabled());

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), 2);
  list_widget->setCurrentRow(0);
  QApplication::processEvents();
  EXPECT_TRUE(remove_button->isEnabled());
  QTest::mouseClick(remove_button, Qt::MouseButton::LeftButton);

  EXPECT_EQ(list_widget->count(), 1);
  list_widget->setCurrentRow(0);
  QApplication::processEvents();
  EXPECT_TRUE(remove_button->isEnabled());

  QTest::mouseClick(remove_button, Qt::MouseButton::LeftButton);

  EXPECT_EQ(list_widget->count(), 0);
  EXPECT_FALSE(remove_button->isEnabled());
}

}  // namespace orbit_config_widgets