// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-spec-builders.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QComboBox>
#include <QCoreApplication>
#include <QLineEdit>
#include <QPushButton>
#include <QSettings>
#include <QSignalSpy>
#include <QTest>
#include <QTimer>
#include <memory>
#include <optional>

#include "OrbitGgp/Client.h"
#include "OrbitGgp/Project.h"
#include "QtUtils/MainThreadExecutorImpl.h"
#include "SessionSetup/RetrieveInstances.h"
#include "SessionSetup/RetrieveInstancesWidget.h"

namespace {
constexpr const char* kOrganizationName = "The Orbit Authors";
constexpr const char* kApplicationName{"RetrieveInstancesWidgetTest"};
constexpr const char* kAllInstancesKey{"kAllInstancesKey"};
constexpr const char* kSelectedProjectIdKey{"kSelectedProjectIdKey"};
constexpr const char* kSelectedProjectDisplayNameKey{"kSelectedProjectDisplayNameKey"};
}  // namespace

namespace orbit_session_setup {

using orbit_base::Future;
using orbit_ggp::Instance;
using orbit_ggp::Project;
using testing::Return;
using InstanceListScope = orbit_ggp::Client::InstanceListScope;

class MockRetrieveInstances : public RetrieveInstances {
 public:
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>>, LoadInstances,
              (const std::optional<Project>& project, InstanceListScope scope), (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<QVector<orbit_ggp::Instance>>>,
              LoadInstancesWithoutCache,
              (const std::optional<Project>& project, InstanceListScope scope), (override));
  MOCK_METHOD(orbit_base::Future<ErrorMessageOr<LoadProjectsAndInstancesResult>>,
              LoadProjectsAndInstances,
              (const std::optional<Project>& project, InstanceListScope scope), (override));
};

namespace {

const Project kTestProject1{
    "Test Project 1",   /* display_name */
    "test_project_1_id" /* id */
};

const Project kTestProject2{
    "Test Project 2",   /* display_name */
    "test_project_2_id" /* id */
};

const Instance kTestInstance1{
    "Test Instance 1",                                          /* display_name */
    "test_instance_1_id",                                       /* id */
    "1.1.1.10",                                                 /* ip_address */
    QDateTime::fromString("2020-01-01T00:42:42Z", Qt::ISODate), /* last_updated */
    "test_owner_1@",                                            /* owner */
    "foo-bar-pool-1",                                           /* pool */
    "RESERVED",                                                 /* state */
};

const Instance kTestInstance2{
    "Test Instance 2",                                          /* display_name */
    "test_instance_2_id",                                       /* id */
    "2.2.2.20",                                                 /* ip_address */
    QDateTime::fromString("2020-02-02T00:42:42Z", Qt::ISODate), /* last_updated */
    "test_owner_2@",                                            /* owner */
    "foo-bar-pool-2",                                           /* pool */
    "IN_USE",                                                   /* state */
};

const QVector<Instance> kTestInstancesProject1{kTestInstance1, kTestInstance2};

const RetrieveInstances::LoadProjectsAndInstancesResult kInitialTestDataDefault{
    QVector<Project>{kTestProject1, kTestProject2}, /* projects */
    Project{kTestProject1},                         /* default_project */
    QVector<Instance>{kTestInstancesProject1},      /* instances */
    std::optional<Project>{std::nullopt},           /* project_of_instances */
};

const RetrieveInstances::LoadProjectsAndInstancesResult kInitialTestDataWithProjectOfInstances{
    QVector<Project>{kTestProject1, kTestProject2}, /* projects */
    Project{kTestProject1},                         /* default_project */
    QVector<Instance>{kTestInstancesProject1},      /* instances */
    std::optional<Project>{kTestProject1},          /* project_of_instances */
};

class RetrieveInstancesWidgetTest : public testing::Test {
 public:
  RetrieveInstancesWidgetTest() : widget_(&mock_retrieve_instances_) {
    filter_line_edit_ = widget_.findChild<QLineEdit*>("filterLineEdit");
    all_check_box_ = widget_.findChild<QCheckBox*>("allCheckBox");
    project_combo_box_ = widget_.findChild<QComboBox*>("projectComboBox");
    reload_button_ = widget_.findChild<QPushButton*>("reloadButton");
  }

 protected:
  void SetUp() override {
    QCoreApplication::setOrganizationName(kOrganizationName);
    QCoreApplication::setApplicationName(kApplicationName);
    QSettings settings;
    settings.clear();

    ASSERT_NE(filter_line_edit_, nullptr);
    ASSERT_NE(all_check_box_, nullptr);
    ASSERT_NE(project_combo_box_, nullptr);
    ASSERT_NE(reload_button_, nullptr);
  }

  void VerifyAllElementsAreEnabled() {
    EXPECT_TRUE(filter_line_edit_->isEnabled());
    EXPECT_TRUE(all_check_box_->isEnabled());
    EXPECT_TRUE(project_combo_box_->isEnabled());
    EXPECT_TRUE(reload_button_->isEnabled());
  }

  void VerifyOnlyReloadIsEnabled() {
    EXPECT_FALSE(filter_line_edit_->isEnabled());
    EXPECT_FALSE(all_check_box_->isEnabled());
    EXPECT_FALSE(project_combo_box_->isEnabled());
    EXPECT_TRUE(reload_button_->isEnabled());
  }

  void VerifyLastLoadingReturnedInstanceList(const QVector<Instance>& instances) {
    ASSERT_GE(loading_successful_spy_.count(), 1);
    QList<QVariant> arguments = loading_successful_spy_.last();
    ASSERT_EQ(arguments.count(), 1);
    ASSERT_TRUE(arguments.at(0).canConvert<QVector<Instance>>());

    EXPECT_EQ(arguments.at(0).value<QVector<Instance>>(), instances);
  }

  void VerifyProjectComboBoxData(const QVector<Project>& projects, const Project& default_project,
                                 const std::optional<Project>& selected_project) {
    // all projects + first default entry
    EXPECT_EQ(project_combo_box_->count(), projects.count() + 1);

    // The first entry is: Default Project (<project name>)
    EXPECT_TRUE(project_combo_box_->itemText(0).contains("Default Project"));
    EXPECT_TRUE(project_combo_box_->itemText(0).contains(default_project.display_name));
    EXPECT_EQ(project_combo_box_->itemData(0), QVariant{});

    // The default projects entry has the form: "<project name> (default)"
    int index_of_default_project_in_full_list =
        project_combo_box_->findData(QVariant::fromValue(default_project));
    ASSERT_NE(index_of_default_project_in_full_list, -1);
    EXPECT_EQ(project_combo_box_->itemText(index_of_default_project_in_full_list),
              QString("%1 (default)").arg(default_project.display_name));

    for (const auto& project : projects) {
      EXPECT_NE(-1, project_combo_box_->findText(project.display_name, Qt::MatchContains));
    }

    if (selected_project.has_value()) {
      EXPECT_EQ(project_combo_box_->currentData().value<Project>(), selected_project);
    } else {
      EXPECT_EQ(project_combo_box_->currentData(), QVariant());
    }
  }

  void VerifyProjectComboBoxHoldsData(
      const RetrieveInstances::LoadProjectsAndInstancesResult& data) {
    VerifyProjectComboBoxData(data.projects, data.default_project, data.project_of_instances);
  }

  // MockGgpClient mock_ggp_;
  MockRetrieveInstances mock_retrieve_instances_;
  RetrieveInstancesWidget widget_;
  QLineEdit* filter_line_edit_ = nullptr;
  QCheckBox* all_check_box_ = nullptr;
  QComboBox* project_combo_box_ = nullptr;
  QPushButton* reload_button_ = nullptr;
  QSignalSpy loading_started_spy_{&widget_, &RetrieveInstancesWidget::LoadingStarted};
  QSignalSpy loading_successful_spy_{&widget_, &RetrieveInstancesWidget::LoadingSuccessful};
  QSignalSpy loading_failed_spy_{&widget_, &RetrieveInstancesWidget::LoadingFailed};
  QSignalSpy initial_loading_failed_spy_{&widget_, &RetrieveInstancesWidget::InitialLoadingFailed};
};

class RetrieveInstancesWidgetTestStarted : public RetrieveInstancesWidgetTest {
 protected:
  void SetUp() override {
    RetrieveInstancesWidgetTest::SetUp();

    EXPECT_CALL(mock_retrieve_instances_,
                LoadProjectsAndInstances(std::optional<Project>(std::nullopt),
                                         InstanceListScope(InstanceListScope::kOnlyOwnInstances)))
        .WillOnce(Return(Future<ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult>>{
            kInitialTestDataDefault}));

    widget_.Start();
    QCoreApplication::processEvents();

    EXPECT_EQ(loading_started_spy_.count(), 1);
    EXPECT_EQ(loading_successful_spy_.count(), 1);
    EXPECT_EQ(loading_failed_spy_.count(), 0);
    EXPECT_EQ(initial_loading_failed_spy_.count(), 0);

    VerifyAllElementsAreEnabled();
    VerifyProjectComboBoxHoldsData(kInitialTestDataDefault);

    loading_started_spy_.clear();
    loading_successful_spy_.clear();
  }
};

}  // namespace

TEST_F(RetrieveInstancesWidgetTest, FilterTextChanged) {
  QSignalSpy spy{&widget_, &RetrieveInstancesWidget::FilterTextChanged};

  QTest::keyClicks(filter_line_edit_, "test text");

  EXPECT_EQ(spy.count(), 9);  // 9 chars in "test text"

  QList<QVariant> arguments = spy.takeLast();
  ASSERT_EQ(arguments.count(), 1);
  ASSERT_TRUE(arguments.at(0).canConvert<QString>());

  EXPECT_EQ(arguments.at(0).toString(), "test text");
}

TEST_F(RetrieveInstancesWidgetTest, StartSuccessfulDefault) {
  EXPECT_CALL(mock_retrieve_instances_,
              LoadProjectsAndInstances(std::optional<Project>(std::nullopt),
                                       InstanceListScope(InstanceListScope::kOnlyOwnInstances)))
      .WillOnce(Return(Future<ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult>>{
          kInitialTestDataDefault}));

  widget_.Start();
  QCoreApplication::processEvents();

  EXPECT_EQ(loading_started_spy_.count(), 1);
  EXPECT_EQ(loading_successful_spy_.count(), 1);
  EXPECT_EQ(loading_failed_spy_.count(), 0);
  EXPECT_EQ(initial_loading_failed_spy_.count(), 0);

  VerifyLastLoadingReturnedInstanceList(kInitialTestDataDefault.instances);
  VerifyAllElementsAreEnabled();
  VerifyProjectComboBoxHoldsData(kInitialTestDataDefault);
}

TEST_F(RetrieveInstancesWidgetTest, StartSuccessfulWithRememberedSettings) {
  EXPECT_CALL(
      mock_retrieve_instances_,
      LoadProjectsAndInstances(
          std::optional<Project>(kInitialTestDataWithProjectOfInstances.project_of_instances),
          InstanceListScope(InstanceListScope::kAllReservedInstances)))
      .WillOnce(Return(Future<ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult>>{
          kInitialTestDataWithProjectOfInstances}));

  QSettings settings;
  settings.setValue(kSelectedProjectIdKey,
                    kInitialTestDataWithProjectOfInstances.project_of_instances->id);
  settings.setValue(kSelectedProjectDisplayNameKey,
                    kInitialTestDataWithProjectOfInstances.project_of_instances->display_name);
  settings.setValue(kAllInstancesKey, true);

  widget_.Start();
  QCoreApplication::processEvents();

  EXPECT_EQ(loading_started_spy_.count(), 1);
  EXPECT_EQ(loading_successful_spy_.count(), 1);
  EXPECT_EQ(loading_failed_spy_.count(), 0);
  EXPECT_EQ(initial_loading_failed_spy_.count(), 0);

  VerifyLastLoadingReturnedInstanceList(kInitialTestDataWithProjectOfInstances.instances);
  VerifyAllElementsAreEnabled();
  VerifyProjectComboBoxHoldsData(kInitialTestDataWithProjectOfInstances);

  EXPECT_TRUE(all_check_box_->isChecked());
}

TEST_F(RetrieveInstancesWidgetTest, StartFailed) {
  EXPECT_CALL(mock_retrieve_instances_,
              LoadProjectsAndInstances(std::optional<Project>(std::nullopt),
                                       InstanceListScope(InstanceListScope::kOnlyOwnInstances)))
      .WillOnce(Return(Future<ErrorMessageOr<RetrieveInstances::LoadProjectsAndInstancesResult>>{
          ErrorMessage{"error message"}}));

  // close the error message box
  QTimer::singleShot(5, []() {
    QApplication::activeModalWidget()->close();
    QCoreApplication::exit();
  });

  widget_.Start();
  QCoreApplication::exec();

  EXPECT_EQ(loading_started_spy_.count(), 1);
  EXPECT_EQ(loading_successful_spy_.count(), 0);
  EXPECT_EQ(loading_failed_spy_.count(), 1);
  EXPECT_EQ(initial_loading_failed_spy_.count(), 1);

  VerifyOnlyReloadIsEnabled();
}

TEST_F(RetrieveInstancesWidgetTestStarted, ReloadSucceeds) {
  EXPECT_CALL(mock_retrieve_instances_,
              LoadInstancesWithoutCache(std::optional<Project>(std::nullopt),
                                        InstanceListScope(InstanceListScope::kOnlyOwnInstances)))
      .WillOnce(Return(Future<ErrorMessageOr<QVector<Instance>>>(kTestInstancesProject1)))
      .WillOnce(
          Return(Future<ErrorMessageOr<QVector<Instance>>>(QVector<Instance>{kTestInstance1})));

  QTest::mouseClick(reload_button_, Qt::MouseButton::LeftButton);
  QCoreApplication::processEvents();
  EXPECT_EQ(loading_started_spy_.count(), 1);
  EXPECT_EQ(loading_successful_spy_.count(), 1);
  EXPECT_EQ(loading_failed_spy_.count(), 0);
  EXPECT_EQ(initial_loading_failed_spy_.count(), 0);
  VerifyLastLoadingReturnedInstanceList(kTestInstancesProject1);
  VerifyAllElementsAreEnabled();

  QTest::mouseClick(reload_button_, Qt::MouseButton::LeftButton);
  QCoreApplication::processEvents();
  EXPECT_EQ(loading_started_spy_.count(), 2);
  EXPECT_EQ(loading_successful_spy_.count(), 2);
  EXPECT_EQ(loading_failed_spy_.count(), 0);
  EXPECT_EQ(initial_loading_failed_spy_.count(), 0);
  VerifyLastLoadingReturnedInstanceList({kTestInstance1});
  VerifyAllElementsAreEnabled();
}

TEST_F(RetrieveInstancesWidgetTestStarted, ReloadFails) {
  EXPECT_CALL(mock_retrieve_instances_,
              LoadInstancesWithoutCache(std::optional<Project>(std::nullopt),
                                        InstanceListScope(InstanceListScope::kOnlyOwnInstances)))
      .WillOnce(Return(Future<ErrorMessageOr<QVector<Instance>>>(ErrorMessage{"error"})));

  QTest::mouseClick(reload_button_, Qt::MouseButton::LeftButton);

  // close the error message box
  QTimer::singleShot(5, []() {
    QApplication::activeModalWidget()->close();
    QCoreApplication::exit();
  });
  QCoreApplication::exec();

  EXPECT_EQ(loading_started_spy_.count(), 1);
  EXPECT_EQ(loading_successful_spy_.count(), 0);
  EXPECT_EQ(loading_failed_spy_.count(), 1);
  EXPECT_EQ(initial_loading_failed_spy_.count(), 0);

  VerifyAllElementsAreEnabled();
}
}  // namespace orbit_session_setup