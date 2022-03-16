// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <gmock/gmock-actions.h>
#include <gmock/gmock-function-mocker.h>
#include <gmock/gmock-matchers.h>
#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QDialogButtonBox>
#include <QListWidget>
#include <QMessageBox>
#include <QPushButton>
#include <QTest>
#include <filesystem>
#include <vector>

#include "ClientData/ModuleData.h"
#include "ConfigWidgets/SymbolsDialog.h"
#include "GrpcProtos/module.pb.h"
#include "SymbolPaths/PersistentStorageManager.h"
#include "Test/Path.h"
#include "TestUtils/TestUtils.h"

namespace orbit_config_widgets {

class MockPersistentStorageManager : public orbit_symbol_paths::PersistentStorageManager {
 public:
  MOCK_METHOD(void, SavePaths, (absl::Span<const std::filesystem::path>), (override));
  MOCK_METHOD(std::vector<std::filesystem::path>, LoadPaths, (), (override));
  MOCK_METHOD(void, SaveModuleSymbolFileMappings,
              ((const absl::flat_hash_map<std::string, std::filesystem::path>&)), (override));
  MOCK_METHOD((absl::flat_hash_map<std::string, std::filesystem::path>),
              LoadModuleSymbolFileMappings, (), (override));
};

class SymbolsDialogTest : public ::testing::Test {
 protected:
  explicit SymbolsDialogTest() {
    EXPECT_CALL(mock_storage_manager_, LoadPaths).WillOnce(testing::Return(load_paths_));
    EXPECT_CALL(mock_storage_manager_, SavePaths)
        .WillOnce([this](absl::Span<const std::filesystem::path> paths) {
          EXPECT_EQ(paths, expected_save_paths_);
        });
  }

  void SetLoadPaths(std::vector<std::filesystem::path> load_paths) {
    load_paths_ = std::move(load_paths);
  }
  void SetExpectSavePaths(std::vector<std::filesystem::path> expected_save_paths) {
    expected_save_paths_ = std::move(expected_save_paths);
  }

  MockPersistentStorageManager mock_storage_manager_;
  std::vector<std::filesystem::path> load_paths_;
  std::vector<std::filesystem::path> expected_save_paths_;
};

TEST_F(SymbolsDialogTest, ConstructEmpty) {
  SymbolsDialog dialog{&mock_storage_manager_};

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), 0);
}

TEST_F(SymbolsDialogTest, ConstructNonEmpty) {
  const std::vector<std::filesystem::path> test_paths{"/path/to/somewhere",
                                                      "path/to/somewhere/else"};

  SetLoadPaths(test_paths);
  SetExpectSavePaths(test_paths);

  SymbolsDialog dialog{&mock_storage_manager_};

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), test_paths.size());
}

TEST_F(SymbolsDialogTest, ConstructWithElfModule) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path("/path/to/lib.so");
  orbit_client_data::ModuleData module{module_info};

  SymbolsDialog dialog{&mock_storage_manager_, &module};
}

TEST_F(SymbolsDialogTest, TryAddSymbolPath) {
  std::filesystem::path path{"/absolute/test/path1"};
  std::filesystem::path path_2{R"(C:\windows\test\path1)"};
  std::filesystem::path file{"/path/to/file.ext"};
  std::vector<std::filesystem::path> save_paths = {path, path_2, file};

  SetExpectSavePaths(save_paths);

  SymbolsDialog dialog{&mock_storage_manager_};
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
    EXPECT_THAT(result, orbit_test_utils::HasError(
                            "Unable to add selected path, it is already part of the list."));
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

TEST_F(SymbolsDialogTest, TryAddSymbolFileWithoutModule) {
  std::filesystem::path hello_world_elf = orbit_test::GetTestdataDir() / "hello_world_elf";
  std::vector<std::filesystem::path> save_paths{hello_world_elf};

  SetExpectSavePaths(save_paths);

  SymbolsDialog dialog{&mock_storage_manager_};

  // success case
  {
    auto result = dialog.TryAddSymbolFile(hello_world_elf);
    EXPECT_TRUE(result.has_value());
  }

  // fails because not an object_file
  std::filesystem::path text_file = orbit_test::GetTestdataDir() / "textfile.txt";
  {
    auto result = dialog.TryAddSymbolFile(text_file);
    EXPECT_THAT(result,
                orbit_test_utils::HasError("The selected file is not a viable symbol file"));
  }

  // fails because no build-id
  std::filesystem::path hello_world_elf_no_build_id =
      orbit_test::GetTestdataDir() / "hello_world_elf_no_build_id";
  {
    auto result = dialog.TryAddSymbolFile(hello_world_elf_no_build_id);

    EXPECT_THAT(result,
                orbit_test_utils::HasError("The selected file does not contain a build id"));
  }
}

TEST_F(SymbolsDialogTest, TryAddSymbolFileWithModule) {
  orbit_grpc_protos::ModuleInfo module_info;
  module_info.set_object_file_type(orbit_grpc_protos::ModuleInfo::kElfFile);
  module_info.set_file_path((orbit_test::GetTestdataDir() / "no_symbols_elf").string());
  module_info.set_build_id("b5413574bbacec6eacb3b89b1012d0e2cd92ec6b");
  orbit_client_data::ModuleData module{module_info};

  std::filesystem::path no_symbols_elf_debug =
      orbit_test::GetTestdataDir() / "no_symbols_elf.debug";
  std::vector<std::filesystem::path> save_paths{no_symbols_elf_debug};

  SetExpectSavePaths(save_paths);

  SymbolsDialog dialog{&mock_storage_manager_, &module};

  // Success (build id matches)
  {
    auto result = dialog.TryAddSymbolFile(no_symbols_elf_debug);
    EXPECT_TRUE(result.has_value());
  }

  // fail (build id different)
  std::filesystem::path libc_debug = orbit_test::GetTestdataDir() / "libc.debug";
  {
    auto result = dialog.TryAddSymbolFile(libc_debug);
    EXPECT_THAT(result, orbit_test_utils::HasError(
                            "The build ids of module and symbols file do not match."));
  }
}

TEST_F(SymbolsDialogTest, RemoveButton) {
  SetLoadPaths({"random/path/entry"});
  SetExpectSavePaths({});

  SymbolsDialog dialog{&mock_storage_manager_};

  auto* remove_button = dialog.findChild<QPushButton*>("removeButton");
  ASSERT_NE(remove_button, nullptr);
  EXPECT_FALSE(remove_button->isEnabled());

  auto* list_widget = dialog.findChild<QListWidget*>("listWidget");
  ASSERT_NE(list_widget, nullptr);
  EXPECT_EQ(list_widget->count(), 1);
  list_widget->setCurrentRow(0);
  QApplication::processEvents();
  EXPECT_TRUE(remove_button->isEnabled());

  QTest::mouseClick(remove_button, Qt::MouseButton::LeftButton);

  EXPECT_EQ(list_widget->count(), 0);
  EXPECT_FALSE(remove_button->isEnabled());
}

}  // namespace orbit_config_widgets