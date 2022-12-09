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

#include "OrbitAccessibility/AccessibleInterface.h"
#include "OrbitBase/Logging.h"

namespace orbit_qt {

class AccessibilityAdapter;

class OrbitGlInterfaceWrapper : public QObject {
  Q_OBJECT;

 public:
  explicit OrbitGlInterfaceWrapper(const orbit_accessibility::AccessibleInterface* iface)
      : iface_(iface) {}
  [[nodiscard]] const orbit_accessibility::AccessibleInterface* GetInterface() const {
    return iface_;
  }

 private:
  const orbit_accessibility::AccessibleInterface* iface_;
};

class AdapterRegistry {
 public:
  ~AdapterRegistry() { ORBIT_CHECK(all_interfaces_map_.size() == 0); }
  AdapterRegistry(const AdapterRegistry&) = delete;
  AdapterRegistry(AdapterRegistry&&) = delete;
  AdapterRegistry& operator=(const AdapterRegistry&) = delete;
  AdapterRegistry& operator=(AdapterRegistry&&) = delete;

  [[nodiscard]] static AdapterRegistry& Get();

  [[nodiscard]] QAccessibleInterface* GetOrCreateAdapter(
      const orbit_accessibility::AccessibleInterface* iface);
  void RegisterAdapter(const orbit_accessibility::AccessibleInterface* gl_control,
                       QAccessibleInterface* qt_control) {
    all_interfaces_map_.emplace(gl_control, qt_control);
  }

  void OnInterfaceDeleted(orbit_accessibility::AccessibleInterface* iface);

  [[nodiscard]] QAccessibleInterface* InterfaceWrapperFactory(const QString& classname,
                                                              QObject* object);

 private:
  explicit AdapterRegistry() = default;

  absl::flat_hash_map<const orbit_accessibility::AccessibleInterface*, QAccessibleInterface*>
      all_interfaces_map_;
  absl::flat_hash_map<const orbit_accessibility::AccessibleInterface*,
                      std::unique_ptr<OrbitGlInterfaceWrapper>>
      managed_adapters_;
};

/*
 * Instances of this act as adapters from QAccessibleInterface to orbit_gl::AccessibleInterface
 * and vice versa. Static methods of AccessibilityAdapter provide factory methods and tracking of
 * adapters.
 *
 * See file documentation above for more details.
 */
class AccessibilityAdapter : public QAccessibleInterface {
  friend class AdapterRegistry;

 public:
  AccessibilityAdapter() = delete;
  AccessibilityAdapter(const AccessibilityAdapter& rhs) = delete;
  AccessibilityAdapter(AccessibilityAdapter&& rhs) = delete;
  AccessibilityAdapter& operator=(const AccessibilityAdapter& rhs) = delete;
  AccessibilityAdapter& operator=(AccessibilityAdapter&& rhs) = delete;

  [[nodiscard]] bool isValid() const override {
    bool result = info_ != nullptr && object_ != nullptr;
    return result;
  }

  [[nodiscard]] QObject* object() const override { return object_; }
  [[nodiscard]] QAccessibleInterface* focusChild() const override { return nullptr; }

  [[nodiscard]] QAccessibleInterface* parent() const override {
    return AdapterRegistry::Get().GetOrCreateAdapter(info_->AccessibleParent());
  }
  [[nodiscard]] QAccessibleInterface* child(int index) const override {
    return AdapterRegistry::Get().GetOrCreateAdapter(info_->AccessibleChild(index));
  }
  [[nodiscard]] int childCount() const override { return info_->AccessibleChildCount(); }
  int indexOfChild(const QAccessibleInterface* child) const override;
  [[nodiscard]] QAccessibleInterface* childAt(int x, int y) const override;

  [[nodiscard]] QString text(QAccessible::Text /*t*/) const override {
    return QString::fromStdString(info_->AccessibleName());
  }
  void setText(QAccessible::Text /*t*/, const QString& /*text*/) override{};
  [[nodiscard]] QRect rect() const override;
  [[nodiscard]] QAccessible::Role role() const override;

  [[nodiscard]] virtual QAccessible::State state() const override {
    static_assert(sizeof(QAccessible::State) == sizeof(orbit_accessibility::AccessibilityState));
    return absl::bit_cast<QAccessible::State>(info_->AccessibleState());
  }

 private:
  explicit AccessibilityAdapter(const orbit_accessibility::AccessibleInterface* info,
                                QObject* object)
      : info_(info), object_(object){};

  const orbit_accessibility::AccessibleInterface* info_;
  QObject* object_;
};

void InstallAccessibilityFactories();

}  // namespace orbit_qt

#endif