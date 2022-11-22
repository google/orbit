// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <QCheckBox>
#include <QLabel>
#include <QString>
#include <QTest>
#include <Qt>
#include <memory>
#include <string_view>

#include "MizarWidgets/SamplingWithFrameTrackWidget.h"

using ::testing::NotNull;

namespace orbit_mizar_widgets {

constexpr std::string_view kMultiplicityCorrectionEnabledLabel =
    "Probability of false alarm for at least one function:";
constexpr std::string_view kMultiplicityCorrectionDisabledLabel =
    "Probability of false alarm for an individual function:";

class SamplingWithFrameTrackWidgetTest : public ::testing::Test {
 public:
  SamplingWithFrameTrackWidgetTest() : widget_(std::make_unique<SamplingWithFrameTrackWidget>()) {}

  void SetUp() override {
    multiplicity_correction_ = widget_->findChild<QCheckBox*>("multiplicity_correction_");
    ASSERT_THAT(multiplicity_correction_, NotNull());

    significance_level_label_ = widget_->findChild<QLabel*>("significance_level_label_");
    ASSERT_THAT(significance_level_label_, NotNull());
  }

  void ClickMultiplicityCorrectionCheckBox() const {
    QTest::mouseClick(multiplicity_correction_, Qt::MouseButton::LeftButton);
  }

  void ExpectMultiplicityCorrectionEnabledIsCorrectlyShown() const {
    EXPECT_TRUE(multiplicity_correction_->isChecked());
    EXPECT_EQ(significance_level_label_->text().toStdString(), kMultiplicityCorrectionEnabledLabel);
  }

  void ExpectMultiplicityCorrectionDisabledIsCorrectlyShown() const {
    EXPECT_FALSE(multiplicity_correction_->isChecked());
    EXPECT_EQ(significance_level_label_->text().toStdString(),
              kMultiplicityCorrectionDisabledLabel);
  }

  QCheckBox* multiplicity_correction_{};
  QLabel* significance_level_label_{};
  std::unique_ptr<SamplingWithFrameTrackWidget> widget_;
};

TEST_F(SamplingWithFrameTrackWidgetTest, MultiplicityCorrectionCheckBoxIsCorrect) {
  ExpectMultiplicityCorrectionEnabledIsCorrectlyShown();

  ClickMultiplicityCorrectionCheckBox();
  ExpectMultiplicityCorrectionDisabledIsCorrectlyShown();

  ClickMultiplicityCorrectionCheckBox();
  ExpectMultiplicityCorrectionEnabledIsCorrectlyShown();
}

}  // namespace orbit_mizar_widgets
