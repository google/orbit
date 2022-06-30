// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <qlistwidget.h>
#include <qstringliteral.h>

#include <QApplication>
#include <QTest>
#include <string>

#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"

using testing::NotNull;
using testing::ReturnRef;
using testing::UnorderedElementsAreArray;

namespace {
class MockPairedData {
 public:
  MOCK_METHOD((const absl::flat_hash_map<uint32_t, std::string>&), TidToNames, (), (const));
  MOCK_METHOD((const absl::flat_hash_map<uint32_t, std::uint64_t>&), TidToCallstackSampleCounts, (),
              (const));
};
}  // namespace

namespace orbit_mizar_widgets {

constexpr uint32_t kTid = 0x3EAD1;
constexpr uint32_t kOtherTid = 0x3EAD2;
const std::string kThreadName = "Thread";
const std::string kOtherThreadName = "Other Thread";
constexpr uint64_t kThreadSamplesCount = 5;
constexpr uint64_t kOtherThreadSamplesCount = 2;
const absl::flat_hash_map<uint32_t, std::string> kTidToName = {{kTid, kThreadName},
                                                               {kOtherTid, kOtherThreadName}};

const absl::flat_hash_map<uint32_t, uint64_t> kTidToCount = {{kTid, kThreadSamplesCount},
                                                             {kOtherTid, kOtherThreadSamplesCount}};
constexpr size_t kThreadCount = 2;

static std::string MakeThreadListItemString(std::string_view name, uint32_t tid) {
  return absl::StrFormat("[%u] %s", tid, name);
}

const std::vector<std::string> kThreadNamesSorted = {
    MakeThreadListItemString(kThreadName, kTid),
    MakeThreadListItemString(kOtherThreadName, kOtherTid)};
const QString kInputName = QStringLiteral("InputName");

constexpr const char* kOrgName = "The Orbit Authors";

class SamplingWithFrameTrackInputWidgetTest : public ::testing::Test {
 public:
  SamplingWithFrameTrackInputWidgetTest()
      : widget_(std::make_unique<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData>>(nullptr)) {
    QApplication::setOrganizationName(kOrgName);
    QApplication::setApplicationName("SamplingWithFrameTrackInputWidgetTest.InitIsCorrect");

    EXPECT_CALL(data_, TidToNames).WillRepeatedly(ReturnRef(kTidToName));
    EXPECT_CALL(data_, TidToCallstackSampleCounts).WillRepeatedly(ReturnRef(kTidToCount));

    widget_->Init(data_, kInputName);

    title_ = widget_->findChild<QLabel*>("title_");
    EXPECT_THAT(title_, NotNull());

    thread_list_ = widget_->findChild<QListWidget*>("thread_list_");
    EXPECT_THAT(thread_list_, NotNull());
  }

  void SelectThreadListRow(int row) const {
    thread_list_->selectionModel()->select(thread_list_->model()->index(row, 0),
                                           QItemSelectionModel::Select);
  }

  void ClearThreadListSelection() const { thread_list_->selectionModel()->clearSelection(); }

  void ExpectSelectedTidsAre(std::initializer_list<uint32_t> tids) const {
    EXPECT_THAT(widget_->MakeConfig().tids, UnorderedElementsAreArray(tids));
  }

  MockPairedData data_;
  std::unique_ptr<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData>> widget_;
  QLabel* title_;
  QListWidget* thread_list_;
};

TEST_F(SamplingWithFrameTrackInputWidgetTest, InitIsCorrect) {
  EXPECT_EQ(title_->text(), kInputName);

  EXPECT_EQ(thread_list_->count(), kThreadCount);
  for (int i = 0; i < thread_list_->count(); ++i) {
    QListWidgetItem* item = thread_list_->item(i);
    EXPECT_THAT(item, NotNull());
    EXPECT_EQ(item->text().toStdString(), kThreadNamesSorted[i]);
  }
}

TEST_F(SamplingWithFrameTrackInputWidgetTest, OnThreadSelectionChangedIsCorrect) {
  SelectThreadListRow(0);
  ExpectSelectedTidsAre({kTid});

  SelectThreadListRow(1);
  ExpectSelectedTidsAre({kTid, kOtherTid});

  ClearThreadListSelection();
  SelectThreadListRow(1);
  ExpectSelectedTidsAre({kOtherTid});
}

}  // namespace orbit_mizar_widgets