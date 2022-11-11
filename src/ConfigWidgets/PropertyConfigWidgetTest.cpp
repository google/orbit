// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <QCheckBox>
#include <QSlider>

#include "ConfigWidgets/PropertyConfigWidget.h"

TEST(FloatProperty, SetValue) {
  orbit_config_widgets::PropertyConfigWidget::FloatProperty property{
      {.initial_value = 42.f, .min = 0.f, .max = 100.f, .step = 1.0f, .label = "My label:"}};
  EXPECT_EQ(property.value(), 42.f);

  property.SetValue(-10.f);
  EXPECT_EQ(property.value(), 0.f);

  property.SetValue(142.f);
  EXPECT_EQ(property.value(), 100.f);
}

TEST(FloatProperty, InitialValueClamping) {
  orbit_config_widgets::PropertyConfigWidget::FloatProperty property{
      {.initial_value = 42.f, .min = 55.5f, .max = 100.f, .step = 1.0f, .label = "My label:"}};
  EXPECT_EQ(property.value(), 55.5);
}

TEST(IntProperty, SetValue) {
  orbit_config_widgets::PropertyConfigWidget::IntProperty property{
      {.initial_value = 42, .min = 0, .max = 100, .step = 1, .label = "My label:"}};
  EXPECT_EQ(property.value(), 42);

  property.SetValue(-10);
  EXPECT_EQ(property.value(), 0);

  property.SetValue(142);
  EXPECT_EQ(property.value(), 100);
}

TEST(IntProperty, InitialValueClamping) {
  orbit_config_widgets::PropertyConfigWidget::IntProperty property{
      {.initial_value = 42, .min = 55, .max = 100, .step = 1, .label = "My label:"}};
  EXPECT_EQ(property.value(), 55);
}

TEST(BoolProperty, SetValue) {
  orbit_config_widgets::PropertyConfigWidget::BoolProperty property{
      {.initial_value = true, .label = "My label:"}};
  EXPECT_EQ(property.value(), true);

  property.SetValue(false);
  EXPECT_EQ(property.value(), false);
}

TEST(PropertyConfigWidget, AddWidgetForFloatProperty) {
  orbit_config_widgets::PropertyConfigWidget widget{};

  orbit_config_widgets::PropertyConfigWidget::FloatProperty property{
      {.initial_value = 42.f, .min = 0.f, .max = 100.f, .step = 1.0f, .label = "My label:"}};
  widget.AddWidgetForProperty(&property);

  auto* slider = widget.findChild<QSlider*>("slider_my_label_");
  ASSERT_NE(slider, nullptr);

  EXPECT_EQ(slider->value(), 42);

  // Slider changes adjust the property's value.
  slider->setValue(78);
  EXPECT_EQ(property.value(), 78);

  // The slider obeys min and max limits.
  slider->setValue(142);
  EXPECT_EQ(property.value(), 100);

  // Programmatic value changes adjust the slider.
  property.SetValue(42);
  EXPECT_EQ(slider->value(), 42);
}

TEST(PropertyConfigWidget, AddWidgetForIntProperty) {
  orbit_config_widgets::PropertyConfigWidget widget{};

  orbit_config_widgets::PropertyConfigWidget::IntProperty property{
      {.initial_value = 42, .min = 0, .max = 100, .step = 1, .label = "My label:"}};
  widget.AddWidgetForProperty(&property);

  auto* slider = widget.findChild<QSlider*>("slider_my_label_");
  ASSERT_NE(slider, nullptr);

  EXPECT_EQ(slider->value(), 42);

  // Slider changes adjust the property's value.
  slider->setValue(78);
  EXPECT_EQ(property.value(), 78);

  // The slider obeys min and max limits.
  slider->setValue(142);
  EXPECT_EQ(property.value(), 100);

  // Programmatic value changes adjust the slider.
  property.SetValue(42);
  EXPECT_EQ(slider->value(), 42);
}

TEST(PropertyConfigWidget, AddWidgetForBoolProperty) {
  orbit_config_widgets::PropertyConfigWidget widget{};

  orbit_config_widgets::PropertyConfigWidget::BoolProperty property{
      {.initial_value = true, .label = "My label:"}};
  widget.AddWidgetForProperty(&property);

  auto* checkbox = widget.findChild<QCheckBox*>("checkbox_my_label_");
  ASSERT_NE(checkbox, nullptr);

  EXPECT_EQ(checkbox->isChecked(), true);

  checkbox->setChecked(false);
  EXPECT_EQ(property.value(), false);

  property.SetValue(true);
  EXPECT_EQ(checkbox->isChecked(), true);
}

// Start the test binary with `--gtest_filter=PropertyConfigWidget.DISABLED_Demo
// --gtest_also_run_disabled_tests` to run this demo.
TEST(PropertyConfigWidget, DISABLED_Demo) {
  orbit_config_widgets::PropertyConfigWidget widget{};

  orbit_config_widgets::PropertyConfigWidget::BoolProperty enable_flux_capacitor{
      {.initial_value = true, .label = "Enable flux capacitor"}};
  widget.AddWidgetForProperty(&enable_flux_capacitor);

  orbit_config_widgets::PropertyConfigWidget::FloatProperty warp_factor{
      {.initial_value = 1.0f, .min = 1.0f, .max = 10.f, .step = 0.1f, .label = "Warp Factor:"}};
  widget.AddWidgetForProperty(&warp_factor);

  orbit_config_widgets::PropertyConfigWidget::IntProperty answer{
      {.initial_value = 41,
       .min = 0,
       .max = 200,
       .step = 1,
       .label = "What you get if you multiply six by nine:"}};
  widget.AddWidgetForProperty(&answer);

  widget.show();
  QApplication::exec();
  SUCCEED();
}