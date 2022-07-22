// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QApplication>
#include <QComboBox>
#include <QLineEdit>
#include <QListWidget>
#include <QStringLiteral>
#include <QTest>
#include <array>
#include <limits>
#include <string>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeInfo.h"
#include "MizarBase/ThreadId.h"
#include "MizarData/FrameTrack.h"
#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"
#include "TestUtils/ContainerHelpers.h"

using ::orbit_client_data::ScopeId;
using ::orbit_client_data::ScopeType;
using ::orbit_grpc_protos::PresentEvent;
using ::orbit_mizar_base::TID;
using ::orbit_mizar_data::FrameTrackId;
using ::orbit_mizar_data::FrameTrackInfo;
using ::orbit_test_utils::MakeMap;
using ::testing::ElementsAreArray;
using ::testing::NotNull;
using ::testing::Return;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAreArray;

namespace {
class MockPairedData {
 public:
  MOCK_METHOD((const absl::flat_hash_map<TID, std::string>&), TidToNames, (), (const));
  MOCK_METHOD((const absl::flat_hash_map<TID, std::uint64_t>&), TidToCallstackSampleCounts, (),
              (const));
  MOCK_METHOD((absl::flat_hash_map<FrameTrackId, FrameTrackInfo>), GetFrameTracks, (), (const));
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

constexpr size_t kScopeFrameTracksCount = 3;
constexpr size_t kFrameTracksCount = kScopeFrameTracksCount + 1;
constexpr std::array<FrameTrackId, kFrameTracksCount> kFrameTrackIds = {
    FrameTrackId(ScopeId(1)), FrameTrackId(ScopeId(2)), FrameTrackId(ScopeId(10)),
    FrameTrackId(PresentEvent::kD3d9)};
constexpr std::array<std::string_view, kScopeFrameTracksCount> kFrameTrackNames = {"Foo", "Boo",
                                                                                   "Manual"};
constexpr std::array<ScopeType, kScopeFrameTracksCount> kScopeInfoTypes = {
    ScopeType::kDynamicallyInstrumentedFunction, ScopeType::kDynamicallyInstrumentedFunction,
    ScopeType::kApiScope};
constexpr std::array<FrameTrackId, kFrameTracksCount> kScopeIdsInExpectedOrder = {
    FrameTrackId(ScopeId(2)), FrameTrackId(ScopeId(1)), FrameTrackId(ScopeId(10)),
    FrameTrackId(PresentEvent::kD3d9)};
const std::vector<FrameTrackInfo> kFrameTrackInfos = [] {
  std::vector<FrameTrackInfo> result;
  for (size_t i = 0; i < kScopeFrameTracksCount; ++i) {
    const std::string_view name_view = kFrameTrackNames[i];
    orbit_client_data::ScopeInfo info(std::string(name_view), kScopeInfoTypes[i]);
    result.emplace_back(info);
  }
  result.emplace_back(PresentEvent::kD3d9);
  return result;
}();

const absl::flat_hash_map<FrameTrackId, FrameTrackInfo> kFrameTracks =
    MakeMap(kFrameTrackIds, kFrameTrackInfos);

class SamplingWithFrameTrackInputWidgetTest : public ::testing::Test {
 public:
  SamplingWithFrameTrackInputWidgetTest()
      : widget_(std::make_unique<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData>>(nullptr)) {
    EXPECT_CALL(data_, TidToNames).WillRepeatedly(ReturnRef(kTidToName));
    EXPECT_CALL(data_, TidToCallstackSampleCounts).WillRepeatedly(ReturnRef(kTidToCount));
    EXPECT_CALL(data_, GetFrameTracks).WillRepeatedly(Return(kFrameTracks));

    widget_->Init(data_, kInputName);
  }

  void SetUp() override {
    title_ = widget_->findChild<QLabel*>("title_");
    ASSERT_THAT(title_, NotNull());

    thread_list_ = widget_->findChild<QListWidget*>("thread_list_");
    ASSERT_THAT(thread_list_, NotNull());

    frame_track_list_ = widget_->findChild<QComboBox*>("frame_track_list_");
    ASSERT_THAT(thread_list_, NotNull());

    start_ms_ = widget_->findChild<QLineEdit*>("start_ms_");
    ASSERT_THAT(start_ms_, NotNull());
  }

  void SelectThreadListRow(int row) const {
    thread_list_->selectionModel()->select(thread_list_->model()->index(row, 0),
                                           QItemSelectionModel::Select);
  }

  void ClearThreadListSelection() const { thread_list_->selectionModel()->clearSelection(); }

  void ExpectSelectedTidsAre(std::initializer_list<TID> tids) const {
    EXPECT_THAT(widget_->MakeConfig().tids, UnorderedElementsAreArray(tids));
  }

  void ExpectSelectedFrameTrackIdIs(FrameTrackId frame_track_id) const {
    EXPECT_EQ(widget_->MakeConfig().frame_track_id, frame_track_id);
  }

  void ExpectRelativeStartNsIs(uint64_t start_relative_ns_) const {
    EXPECT_EQ(widget_->MakeConfig().start_relative_ns, start_relative_ns_);
  }

  MockPairedData data_;
  std::unique_ptr<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData>> widget_;
  QLabel* title_{};
  QListWidget* thread_list_{};
  QComboBox* frame_track_list_{};
  QLineEdit* start_ms_{};
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

  frame_track_list_->setCurrentIndex(3);
  ExpectSelectedFrameTrackIdIs(kScopeIdsInExpectedOrder[3]);
}

TEST_F(SamplingWithFrameTrackInputWidgetTest, OnStartMsChangedIsCorrect) {
  start_ms_->setText("");
  ExpectRelativeStartNsIs(0);

  start_ms_->setText("123");
  ExpectRelativeStartNsIs(123'000'000);

  start_ms_->setText("0123");
  ExpectRelativeStartNsIs(123'000'000);

  start_ms_->setText("99999999999999999999999999");
  ExpectRelativeStartNsIs(static_cast<uint64_t>(std::numeric_limits<uint64_t>::max()));

  start_ms_->setText("-0");
  ExpectRelativeStartNsIs(0);

  start_ms_->setText("-0");
  ExpectRelativeStartNsIs(0);
}

}  // namespace orbit_mizar_widgets