// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <QApplication>
#include <QCheckBox>
#include <QPushButton>
#include <QSignalSpy>
#include <QSlider>
#include <memory>
#include <string_view>

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
      {.initial_value = 42.f, .min = 5.f, .max = 100.f, .step = 0.1f, .label = "My label:"}};
  widget.AddWidgetForProperty(&property);

  const auto to_slider_position = [&property](float value) {
    // QSlider only supports integers, so we do linear scaling between min and max with a resolution
    // of step to support floats.

    return static_cast<int>((value - property.definition().min) / property.definition().step);
  };

  auto* slider = widget.findChild<QSlider*>("slider_my_label_");
  ASSERT_NE(slider, nullptr);
  EXPECT_EQ(slider->value(), to_slider_position(property.definition().initial_value));

  QSignalSpy property_change_signal{
      &widget, &orbit_config_widgets::PropertyConfigWidget::AnyRegisteredPropertyChangedValue};

  // Slider changes adjust the property's value
  property_change_signal.clear();
  slider->setValue(to_slider_position(78.f));
  EXPECT_FLOAT_EQ(property.value(), 78.f);
  // and trigger the change signal.
  EXPECT_EQ(property_change_signal.count(), 1);

  // The slider obeys min and max limits.
  slider->setValue(to_slider_position(142.f));
  EXPECT_FLOAT_EQ(property.value(), property.definition().max);

  // Programmatic value changes adjust the slider
  property_change_signal.clear();
  property.SetValue(43);
  EXPECT_EQ(slider->value(), to_slider_position(43.f));
  // and do NOT trigger the change signal.
  EXPECT_EQ(property_change_signal.count(), 0);

  // Clicking the reset button restores the initial value
  auto* reset_button = widget.findChild<QPushButton*>("reset_button_my_label_");
  ASSERT_NE(reset_button, nullptr);

  property_change_signal.clear();
  reset_button->click();
  EXPECT_FLOAT_EQ(slider->value(), to_slider_position(property.definition().initial_value));
  // and triggers the change signal.
  EXPECT_EQ(property_change_signal.count(), 1);
}

TEST(PropertyConfigWidget, AddWidgetForIntProperty) {
  orbit_config_widgets::PropertyConfigWidget widget{};

  orbit_config_widgets::PropertyConfigWidget::IntProperty property{
      {.initial_value = 42, .min = 0, .max = 100, .step = 1, .label = "My label:"}};
  widget.AddWidgetForProperty(&property);

  auto* slider = widget.findChild<QSlider*>("slider_my_label_");
  ASSERT_NE(slider, nullptr);
  EXPECT_EQ(slider->value(), 42);

  QSignalSpy property_change_signal{
      &widget, &orbit_config_widgets::PropertyConfigWidget::AnyRegisteredPropertyChangedValue};

  // Slider changes adjust the property's value
  property_change_signal.clear();
  slider->setValue(78);
  EXPECT_EQ(property.value(), 78);
  // and trigger the change signal.
  EXPECT_EQ(property_change_signal.count(), 1);

  // The slider obeys min and max limits.
  slider->setValue(142);
  EXPECT_EQ(property.value(), 100);

  // Programmatic value changes adjust the slider
  property_change_signal.clear();
  property.SetValue(43);
  EXPECT_EQ(slider->value(), 43);
  // and do NOT trigger the change signal.
  EXPECT_EQ(property_change_signal.count(), 0);

  // Clicking the reset button restores the initial value
  auto* reset_button = widget.findChild<QPushButton*>("reset_button_my_label_");
  ASSERT_NE(reset_button, nullptr);

  property_change_signal.clear();
  reset_button->click();
  EXPECT_EQ(slider->value(), property.definition().initial_value);
  // and triggers the change signal.
  EXPECT_EQ(property_change_signal.count(), 1);
}

TEST(PropertyConfigWidget, AddWidgetForBoolProperty) {
  orbit_config_widgets::PropertyConfigWidget widget{};

  orbit_config_widgets::PropertyConfigWidget::BoolProperty property{
      {.initial_value = true, .label = "My label:"}};
  widget.AddWidgetForProperty(&property);

  auto* checkbox = widget.findChild<QCheckBox*>("checkbox_my_label_");
  ASSERT_NE(checkbox, nullptr);
  EXPECT_EQ(checkbox->isChecked(), true);

  QSignalSpy property_change_signal{
      &widget, &orbit_config_widgets::PropertyConfigWidget::AnyRegisteredPropertyChangedValue};

  // Changing the checked state of the checkbox adjusts the value of the property
  property_change_signal.clear();
  checkbox->setChecked(false);
  EXPECT_EQ(property.value(), false);
  // and triggers the change signal.
  EXPECT_EQ(property_change_signal.count(), 1);

  // Programmatic value changes adjust the checked state of the checkbox
  property_change_signal.clear();
  property.SetValue(true);
  EXPECT_EQ(checkbox->isChecked(), true);
  // and do NOT trigger the change signal.
  EXPECT_EQ(property_change_signal.count(), 0);

  // Clicking the reset button restores the initial check state
  property.SetValue(false);
  auto* reset_button = widget.findChild<QPushButton*>("reset_button_my_label_");
  ASSERT_NE(reset_button, nullptr);

  property_change_signal.clear();
  reset_button->click();
  EXPECT_EQ(checkbox->isChecked(), property.definition().initial_value);
  // and triggers the change signal.
  EXPECT_EQ(property_change_signal.count(), 1);
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