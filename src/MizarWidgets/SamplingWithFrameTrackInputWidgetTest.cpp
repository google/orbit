// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QComboBox>
#include <QListWidget>
#include <QStringLiteral>
#include <QTest>
#include <array>
#include <string>

#include "ClientData/ScopeInfo.h"
#include "MizarBase/ThreadId.h"
#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"
#include "TestUtils/ContainerHelpers.h"

using orbit_mizar_base::TID;
using testing::ElementsAreArray;
using testing::NotNull;
using testing::Return;
using testing::ReturnRef;
using testing::UnorderedElementsAreArray;

using orbit_test_utils::MakeMap;

namespace {
class MockPairedData {
 public:
  MOCK_METHOD((const absl::flat_hash_map<TID, std::string>&), TidToNames, (), (const));
  MOCK_METHOD((const absl::flat_hash_map<TID, std::uint64_t>&), TidToCallstackSampleCounts, (),
              (const));
  MOCK_METHOD((const absl::flat_hash_map<uint64_t, orbit_client_data::ScopeInfo>), GetFrameTracks,
              (), (const));
};
}  // namespace

namespace orbit_mizar_widgets {

constexpr TID kTid(0x3EAD1);
constexpr TID kOtherTid(0x3EAD2);
const std::string kThreadName = "Thread";
const std::string kOtherThreadName = "Other Thread";
constexpr uint64_t kThreadSamplesCount = 5;
constexpr uint64_t kOtherThreadSamplesCount = 2;
const absl::flat_hash_map<TID, std::string> kTidToName = {{kTid, kThreadName},
                                                          {kOtherTid, kOtherThreadName}};

const absl::flat_hash_map<TID, uint64_t> kTidToCount = {{kTid, kThreadSamplesCount},
                                                        {kOtherTid, kOtherThreadSamplesCount}};
constexpr size_t kThreadCount = 2;

static std::string MakeThreadListItemString(std::string_view name, TID tid) {
  return absl::StrFormat("[%u] %s", *tid, name);
}

const std::vector<std::string> kThreadNamesSorted = {
    MakeThreadListItemString(kThreadName, kTid),
    MakeThreadListItemString(kOtherThreadName, kOtherTid)};
const QString kInputName = QStringLiteral("InputName");

constexpr size_t kFrameTracksCount = 3;
constexpr std::array<uint64_t, kFrameTracksCount> kScopeIds = {1, 2, 10};
constexpr std::array<std::string_view, kFrameTracksCount> kFrameTrackNames = {"Foo", "Boo",
                                                                              "Manual"};
constexpr std::array<orbit_client_data::ScopeType, kFrameTracksCount> kScopeInfoTypes = {
    orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction,
    orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction,
    orbit_client_data::ScopeType::kApiScope};
const absl::flat_hash_map<orbit_client_data::ScopeType, std::string> kScopeTypeToString = {
    {orbit_client_data::ScopeType::kDynamicallyInstrumentedFunction, "[ D]"},
    {orbit_client_data::ScopeType::kApiScope, "[MS]"}};
const std::vector<std::string> kExpectedFrameTrackListContent = {"[ D] Boo", "[ D] Foo",
                                                                 "[MS] Manual"};
constexpr std::array<uint64_t, kFrameTracksCount> kScopeIdsInExpectedOrder = {2, 1, 10};
const std::vector<orbit_client_data::ScopeInfo> kScopeInfos = [] {
  std::vector<orbit_client_data::ScopeInfo> result;
  for (size_t i = 0; i < kFrameTracksCount; ++i) {
    result.emplace_back(std::string(kFrameTrackNames[i]), kScopeInfoTypes[i]);
  }
  return result;
}();

const absl::flat_hash_map<uint64_t, orbit_client_data::ScopeInfo> kFrameTracks =
    MakeMap(kScopeIds, kScopeInfos);

constexpr const char* kOrgName = "The Orbit Authors";

class SamplingWithFrameTrackInputWidgetTest : public ::testing::Test {
 public:
  SamplingWithFrameTrackInputWidgetTest()
      : widget_(std::make_unique<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData>>(nullptr)) {
    QApplication::setOrganizationName(kOrgName);
    QApplication::setApplicationName("SamplingWithFrameTrackInputWidgetTest.InitIsCorrect");

    EXPECT_CALL(data_, TidToNames).WillRepeatedly(ReturnRef(kTidToName));
    EXPECT_CALL(data_, TidToCallstackSampleCounts).WillRepeatedly(ReturnRef(kTidToCount));
    EXPECT_CALL(data_, GetFrameTracks).WillRepeatedly(Return(kFrameTracks));

    widget_->Init(data_, kInputName);

    title_ = widget_->findChild<QLabel*>("title_");
    EXPECT_THAT(title_, NotNull());

    thread_list_ = widget_->findChild<QListWidget*>("thread_list_");
    EXPECT_THAT(thread_list_, NotNull());

    frame_track_list_ = widget_->findChild<QComboBox*>("frame_track_list_");
    EXPECT_THAT(thread_list_, NotNull());
  }

  void SelectThreadListRow(int row) const {
    thread_list_->selectionModel()->select(thread_list_->model()->index(row, 0),
                                           QItemSelectionModel::Select);
  }

  void ClearThreadListSelection() const { thread_list_->selectionModel()->clearSelection(); }

  void ExpectSelectedTidsAre(std::initializer_list<TID> tids) const {
    EXPECT_THAT(widget_->MakeConfig().tids, UnorderedElementsAreArray(tids));
  }

  void ExpectSelectedFrameTrackIdIs(uint32_t scope_id) const {
    EXPECT_EQ(widget_->MakeConfig().frame_track_scope_id, scope_id);
  }

  MockPairedData data_;
  std::unique_ptr<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData>> widget_;
  QLabel* title_;
  QListWidget* thread_list_;
  QComboBox* frame_track_list_;
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

TEST_F(SamplingWithFrameTrackInputWidgetTest, OnFrameTrackSelectionChangedIsCorrect) {
  std::vector<std::string> frame_track_list_content;
  for (int i = 0; i < frame_track_list_->count(); ++i) {
    frame_track_list_content.push_back(frame_track_list_->itemText(i).toStdString());
  }
  EXPECT_THAT(frame_track_list_content, ElementsAreArray(frame_track_list_content));

  frame_track_list_->setCurrentIndex(0);
  ExpectSelectedFrameTrackIdIs(kScopeIdsInExpectedOrder[0]);

  frame_track_list_->setCurrentIndex(2);
  ExpectSelectedFrameTrackIdIs(kScopeIdsInExpectedOrder[2]);

  frame_track_list_->setCurrentIndex(1);
  ExpectSelectedFrameTrackIdIs(kScopeIdsInExpectedOrder[1]);
}

}  // namespace orbit_mizar_widgets