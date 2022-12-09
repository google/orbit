// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/strings/str_format.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <stddef.h>

#include <QAbstractItemModel>
#include <QAbstractListModel>
#include <QComboBox>
#include <QItemSelectionModel>
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QListWidgetItem>
#include <QModelIndex>
#include <QObject>
#include <QString>
#include <QStringLiteral>
#include <QVariant>
#include <algorithm>
#include <array>
#include <cstdint>
#include <initializer_list>
#include <limits>
#include <memory>
#include <string>
#include <string_view>
#include <variant>
#include <vector>

#include "ClientData/ScopeId.h"
#include "ClientData/ScopeStats.h"
#include "GrpcProtos/capture.pb.h"
#include "MizarBase/ThreadId.h"
#include "MizarBase/Time.h"
#include "MizarData/FrameTrack.h"
#include "MizarData/SamplingWithFrameTrackComparisonReport.h"
#include "MizarModels/FrameTrackListModel.h"
#include "MizarWidgets/SamplingWithFrameTrackInputWidget.h"
#include "OrbitBase/Typedef.h"

using ::orbit_client_data::ScopeId;
using ::orbit_grpc_protos::PresentEvent;
using ::orbit_mizar_base::RelativeTimeNs;
using ::orbit_mizar_base::TID;
using ::orbit_mizar_data::FrameTrackId;
using ::orbit_mizar_data::FrameTrackInfo;
using ::testing::ElementsAreArray;
using ::testing::NotNull;
using ::testing::ReturnRef;
using ::testing::UnorderedElementsAreArray;

namespace orbit_mizar_widgets {

namespace {

class MockPairedData {
 public:
  MOCK_METHOD((const absl::flat_hash_map<TID, std::string>&), TidToNames, (), (const));
  MOCK_METHOD((const absl::flat_hash_map<TID, std::uint64_t>&), TidToCallstackSampleCounts, (),
              (const));
  MOCK_METHOD(orbit_client_data::ScopeStats, ActiveInvocationTimeStats,
              (const absl::flat_hash_set<TID>&, FrameTrackId, RelativeTimeNs, RelativeTimeNs),
              (const));
};

constexpr size_t kFrameTracksCount = 4;
constexpr std::array<FrameTrackId, kFrameTracksCount> kScopeIds = {
    FrameTrackId(ScopeId(2)), FrameTrackId(ScopeId(1)), FrameTrackId(ScopeId(10)),
    FrameTrackId(PresentEvent::kD3d9)};

class MockFrameTrackListModel : public QAbstractListModel {
 public:
  MockFrameTrackListModel(const MockPairedData*, const absl::flat_hash_set<TID>*,
                          const RelativeTimeNs*, QObject* parent)
      : QAbstractListModel(parent) {}

  [[nodiscard]] int rowCount(const QModelIndex& /*parent*/) const override {
    return kFrameTracksCount;
  }

  [[nodiscard]] QVariant data(const QModelIndex& index, int role) const override {
    if (index.model() != this || role != orbit_mizar_models::kFrameTrackIdRole) return {};
    return QVariant::fromValue(kScopeIds[index.row()]);
  }
};

}  // namespace

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
const QString kFileName = QStringLiteral("FileName");

class SamplingWithFrameTrackInputWidgetTest : public ::testing::Test {
 public:
  SamplingWithFrameTrackInputWidgetTest()
      : widget_(std::make_unique<
                SamplingWithFrameTrackInputWidgetTmpl<MockPairedData, MockFrameTrackListModel>>(
            nullptr)) {
    EXPECT_CALL(data_, TidToNames).WillRepeatedly(ReturnRef(kTidToName));
    EXPECT_CALL(data_, TidToCallstackSampleCounts).WillRepeatedly(ReturnRef(kTidToCount));

    widget_->Init(data_, kInputName, kFileName);
  }

  void SetUp() override {
    title_ = widget_->findChild<QLabel*>("title_");
    ASSERT_THAT(title_, NotNull());

    file_name_ = widget_->findChild<QLabel*>("file_name");
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

  void ExpectRelativeStartNsIs(uint64_t start_relative_ns) const {
    EXPECT_EQ(*widget_->MakeConfig().start_relative, start_relative_ns);
  }

  MockPairedData data_;
  std::unique_ptr<SamplingWithFrameTrackInputWidgetTmpl<MockPairedData, MockFrameTrackListModel>>
      widget_;
  QLabel* title_{};
  QLabel* file_name_{};
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
  ExpectSelectedFrameTrackIdIs(kScopeIds[0]);

  frame_track_list_->setCurrentIndex(2);
  ExpectSelectedFrameTrackIdIs(kScopeIds[2]);

  frame_track_list_->setCurrentIndex(1);
  ExpectSelectedFrameTrackIdIs(kScopeIds[1]);

  frame_track_list_->setCurrentIndex(3);
  ExpectSelectedFrameTrackIdIs(kScopeIds[3]);
}

TEST_F(SamplingWithFrameTrackInputWidgetTest, OnStartMsChangedIsCorrect) {
  start_ms_->setText("");
  ExpectRelativeStartNsIs(0);

  start_ms_->setText("123");
  ExpectRelativeStartNsIs(123'000'000);

  start_ms_->setText("0123");
  ExpectRelativeStartNsIs(123'000'000);

  start_ms_->setText("99999999999999999999999999");
  ExpectRelativeStartNsIs(std::numeric_limits<uint64_t>::max());

  start_ms_->setText("-0");
  ExpectRelativeStartNsIs(0);

  start_ms_->setText("-0");
  ExpectRelativeStartNsIs(0);
}

}  // namespace orbit_mizar_widgets