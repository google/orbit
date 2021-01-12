// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * Bridging accessibility from OrbitQt and OrbitGl.
 *
 * For a detailed documentation on this, check AccessibilityAdapter.cpp - unless changes to the
 * accessibility interaction between OrbitQt and OrbitGl are required, there should be no need to
 * deal with these definitions.
 */

#ifndef ORBIT_QT_ACCESSIBILITY_ADAPTER_H_
#define ORBIT_QT_ACCESSIBILITY_ADAPTER_H_

#include <absl/base/casts.h>
#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>
#include <absl/meta/type_traits.h>

#include <QAccessible>
#include <QAccessibleInterface>
#include <QAccessibleWidget>
#include <QObject>
#include <QRect>
#include <QString>
#include <QWidget>
#include <memory>

#include "AccessibleTimeGraph.h"
#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Logging.h"
#include "orbitglwidget.h"

namespace orbit_qt {

/*
 * Instances of this act as adapters from QAccessibleInterface to orbit_gl::AccessibleInterface
 * and vice versa. Static methods of AccessibilityAdapter provide factory methods and tracking of
 * adapters.
 *
 * See file documentation above for more details.
 */
class AccessibilityAdapter : public QAccessibleInterface {
 public:
  AccessibilityAdapter() = delete;
  AccessibilityAdapter(const AccessibilityAdapter& rhs) = delete;
  AccessibilityAdapter(AccessibilityAdapter&& rhs) = delete;
  AccessibilityAdapter& operator=(const AccessibilityAdapter& rhs) = delete;
  AccessibilityAdapter& operator=(AccessibilityAdapter&& rhs) = delete;

  bool isValid() const override {
    bool result = info_ != nullptr;
    CHECK(!result || all_interfaces_map_.find(info_)->second == this);
    return result;
  }
  QObject* object() const override { return nullptr; }
  QAccessibleInterface* focusChild() const override { return nullptr; }

  QAccessibleInterface* parent() const override {
    return GetOrCreateAdapter(info_->AccessibleParent());
  }
  QAccessibleInterface* child(int index) const override {
    return GetOrCreateAdapter(info_->AccessibleChild(index));
  }
  int childCount() const override { return info_->AccessibleChildCount(); }
  int indexOfChild(const QAccessibleInterface* child) const override;
  QAccessibleInterface* childAt(int x, int y) const override;

  QString text(QAccessible::Text /*t*/) const override {
    return QString::fromStdString(info_->AccessibleName());
  }
  void setText(QAccessible::Text /*t*/, const QString& /*text*/) override{};
  QRect rect() const override;
  QAccessible::Role role() const override;

  virtual QAccessible::State state() const override {
    static_assert(sizeof(QAccessible::State) == sizeof(orbit_accessibility::AccessibilityState));
    return absl::bit_cast<QAccessible::State>(info_->AccessibleState());
  }

  static QAccessibleInterface* GetOrCreateAdapter(
      const orbit_accessibility::AccessibleInterface* iface);
  static void RegisterAdapter(const orbit_accessibility::AccessibleInterface* gl_control,
                              QAccessibleInterface* qt_control) {
    all_interfaces_map_.emplace(gl_control, qt_control);
  }

  static int RegisteredAdapterCount() { return all_interfaces_map_.size(); }

 private:
  explicit AccessibilityAdapter(const orbit_accessibility::AccessibleInterface* info)
      : info_(info){};

  const orbit_accessibility::AccessibleInterface* info_;

  static absl::flat_hash_map<const orbit_accessibility::AccessibleInterface*, QAccessibleInterface*>
      all_interfaces_map_;
  // Subset of all_interfaces_map_: Contains the adapters created by this class and their managed
  // pointers
  static absl::flat_hash_map<const orbit_accessibility::AccessibleInterface*,
                             std::unique_ptr<AccessibilityAdapter>>
      managed_adapters_;

  static void Init();
  static void OnInterfaceDeleted(orbit_accessibility::AccessibleInterface* iface);
};

void InstallAccessibilityFactories();

}  // namespace orbit_qt

#endif