// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CONFIG_WIDGETS_PROPERTY_CONFIG_WIDGET_H_
#define CONFIG_WIDGETS_PROPERTY_CONFIG_WIDGET_H_

#include <QGridLayout>
#include <QObject>
#include <QString>
#include <QWidget>
#include <algorithm>
#include <cmath>
#include <functional>
#include <string_view>

namespace orbit_config_widgets {

template <typename ValueType>
struct PropertyDefinition {
  static_assert(!std::is_same_v<ValueType, ValueType>,
                "You tried to define a property for a type that is currently not (yet) supported."
                "We only support float, int, and bool so far.");
};

template <typename ValueType>
class Property;

template <>
struct PropertyDefinition<float> {
  float initial_value = 0.f;
  float min = 0.f;
  float max = 100.f;
  float step = 0.1f;
  std::string_view label;   // This will be printed in front of the slider. It is recommend to add
                            // a colon at the end of the label.
  std::string_view suffix;  // This can be a unit like px or mm. It will be appended to the value
                            // label. Don't forget to add a space before the unit.

  [[nodiscard]] float SanitizeValue(float value) const { return std::clamp(value, min, max); }
};

template <>
struct PropertyDefinition<int> {
  int initial_value = 0;
  int min = 0;
  int max = 100;
  int step = 1;
  std::string_view label;   // This will be printed in front of the slider. It is recommend to add
                            // a colon at the end of the label.
  std::string_view suffix;  // This can be a unit like px or mm. It will be appended to the value
                            // label. Don't forget to add a space before the unit.

  [[nodiscard]] int SanitizeValue(int value) const { return std::clamp(value, min, max); }
};

template <>
struct PropertyDefinition<bool> {
  bool initial_value = false;
  std::string_view label;  // This will be the checkbox's label. No colon required here because it
                           // goes after the checkbox.

  [[nodiscard]] static bool SanitizeValue(bool value) {
    // Since the data type `bool` exactly represents the allowed set of state, no clamping or
    // whatever is needed here.
    return value;
  }
};

// This widget offers controls for a list of changeable settings (called properties).
// For each registered property one control element will be generated.
// So far we support floating point and integer properties which generate a slider in the widget, as
// well as boolean properties which generate a checkbox.
//
// Check out the Demo in PropertyConfigWidgetTest.cpp to see how it works.
class PropertyConfigWidget : public QWidget {
  Q_OBJECT
 public:
  explicit PropertyConfigWidget(QWidget* parent = nullptr);

  template <typename ValueType>
  class Property {
    friend PropertyConfigWidget;

   public:
    explicit Property(const PropertyDefinition<ValueType>& definition)
        : definition_(definition), value_{definition.SanitizeValue(definition.initial_value)} {}

    // NOLINTNEXTLINE(readability-identifier-naming)
    [[nodiscard]] ValueType value() const { return value_; }

    // NOLINTNEXTLINE(readability-identifier-naming)
    [[nodiscard]] const PropertyDefinition<ValueType>& definition() const { return definition_; }

    void SetValue(ValueType value) {
      value_ = definition().SanitizeValue(value);

      if (!setter_) return;
      setter_(std::move(value));
    }

   private:
    PropertyDefinition<ValueType> definition_;
    ValueType value_;
    std::function<void(ValueType)> setter_;
  };

  using FloatProperty = Property<float>;
  using IntProperty = Property<int>;
  using BoolProperty = Property<bool>;

  // Note, by calling this function you guarantee that `property` stays alive until the end of the
  // widget's lifetime. Making the properties member variables of a subclass of this widget is
  // explicitly allowed even though that violates this rules. (We won't use the properties in the
  // destructor of the widget, so it will be fine.)
  void AddWidgetForProperty(FloatProperty* property);
  void AddWidgetForProperty(IntProperty* property);
  void AddWidgetForProperty(BoolProperty* property);

 private:
  QGridLayout* layout_;

 signals:
  // This signal gets emitted whenever a property gets changed on the UI. It's NOT triggered when
  // someone calls `SetValue` on a property.
  void AnyRegisteredPropertyChangedValue();
};

}  // namespace orbit_config_widgets

#endif  // CONFIG_WIDGETS_PROPERTY_CONFIG_WIDGET_H_
