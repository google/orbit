// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_GL_ACCESSIBILITY_H_
#define ORBIT_QT_GL_ACCESSIBILITY_H_

#include <absl/container/flat_hash_map.h>
#include <absl/container/flat_hash_set.h>

#include <QAccessible>
#include <QAccessibleWidget>
#include <QWidget>

#include "OrbitBase/Logging.h"
#include "OrbitGlAccessibility.h"
#include "orbitglwidget.h"

namespace orbit_qt {

class A11yAdapter : public QAccessibleInterface {
 public:
  A11yAdapter() = delete;
  A11yAdapter(A11yAdapter& rhs) = delete;
  A11yAdapter(A11yAdapter&& rhs) = delete;

  static QAccessibleInterface* GetOrCreateAdapter(const orbit_gl::GlA11yControlInterface* iface);

  // check for valid pointers
  bool isValid() const override {
    bool result = info_ != nullptr && s_owned_adapters_.contains(this);
    CHECK(!result || s_adapter_map_.find(info_)->second == this);
    return result;
  }
  QObject* object() const override { return &dummy_; }
  QAccessibleInterface* focusChild() const override { return nullptr; }

  // navigation, hierarchy
  QAccessibleInterface* parent() const override {
    return GetOrCreateAdapter(info_->AccessibleParent());
  }
  QAccessibleInterface* child(int index) const override {
    return GetOrCreateAdapter(info_->AccessibleChild(index));
  }
  int childCount() const override { return info_->AccessibleChildCount(); }
  int indexOfChild(const QAccessibleInterface* child) const override;
  [[nodiscard]] QAccessibleInterface* childAt(int x, int y) const override;

  // properties and state
  QString text(QAccessible::Text t) const override { return info_->AccessibleName().c_str(); }
  void setText(QAccessible::Text t, const QString& text) override{};
  QRect rect() const override;
  QAccessible::Role role() const override {
    auto role = info_->AccessibleRole();
    return *reinterpret_cast<QAccessible::Role*>(&role);
  }

  virtual QAccessible::State state() const override {
    static_assert(sizeof(QAccessible::State) == sizeof(orbit_gl::A11yState));
    return *reinterpret_cast<QAccessible::State*>(&info_->AccessibleState());
  }

  static void AddBridge(const orbit_gl::GlA11yControlInterface* gl_control,
                        QAccessibleInterface* qt_control) {
    s_adapter_map_.insert(std::make_pair(gl_control, qt_control));
  }

 private:
  mutable QObject dummy_;
  A11yAdapter(const orbit_gl::GlA11yControlInterface* info) : info_(info){};

  const orbit_gl::GlA11yControlInterface* info_ = nullptr;

  static absl::flat_hash_map<const orbit_gl::GlA11yControlInterface*, QAccessibleInterface*>
      s_adapter_map_;
  static absl::flat_hash_set<A11yAdapter*> s_owned_adapters_;
};

class OrbitGlWidgetAccessible : public QAccessibleWidget {
 public:
  OrbitGlWidgetAccessible(OrbitGLWidget* widget);

  QAccessibleInterface* child(int index) const;
  int childCount() const override;
  int indexOfChild(const QAccessibleInterface* child) const override;
};

void InstallAccessibilityFactories();

}  // namespace orbit_qt

#endif