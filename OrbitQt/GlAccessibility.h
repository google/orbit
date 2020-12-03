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

#include "CaptureWindowAccessibility.h"
#include "OrbitBase/Logging.h"

namespace orbit_qt {

class A11yAdapter : public QAccessibleInterface {
 public:
  A11yAdapter() = delete;
  A11yAdapter(A11yAdapter& rhs) = delete;
  A11yAdapter(A11yAdapter&& rhs) = delete;

  static A11yAdapter* GetOrCreateAdapter(orbit_gl::GlA11yInterface* iface);
  static void ClearAdapterCache();
  static void ReleaseAdapter(A11yAdapter* adapter);

  // check for valid pointers
  virtual bool isValid() const {
    bool result = info_ != nullptr && s_valid_adapters_.contains(this);
    CHECK(!result || s_iface_to_adapter_.find(info_)->second == this);
    return result;
  }
  virtual QObject* object() const { return nullptr; }

  // relations & interactions - currently not supported
  virtual QVector<QPair<QAccessibleInterface*, QAccessible::Relation>> relations(
      QAccessible::Relation match = QAccessible::AllRelations) const {
    return {};
  }
  virtual QAccessibleInterface* focusChild() const { return nullptr; }

  virtual QAccessibleInterface* childAt(int x, int y) const {
    return GetOrCreateAdapter(info_->AccessibleChildAt(x, y));
  }

  // navigation, hierarchy
  virtual QAccessibleInterface* parent() const {
    return GetOrCreateAdapter(info_->AccessibleParent());
  }
  virtual QAccessibleInterface* child(int index) const {
    return GetOrCreateAdapter(info_->AccessibleChild(index));
  }
  virtual int childCount() const { return info_->AccessibleChildCount(); }
  virtual int indexOfChild(const QAccessibleInterface* child) const;

  // properties and state
  virtual QString text(QAccessible::Text t) const { return info_->AccessibleName().c_str(); }
  virtual void setText(QAccessible::Text t, const QString& text){};
  virtual QRect rect() const;
  virtual QAccessible::Role role() const {
    return static_cast<QAccessible::Role>(info_->AccessibleRole());
  }
  virtual QAccessible::State state() const { return QAccessible::State(); }

 private:
  A11yAdapter(orbit_gl::GlA11yInterface* info) : info_(info){};

  orbit_gl::GlA11yInterface* info_ = nullptr;

  static absl::flat_hash_map<orbit_gl::GlA11yInterface*, A11yAdapter*> s_iface_to_adapter_;
  static absl::flat_hash_set<A11yAdapter*> s_valid_adapters_;
};

/*class OrbitGLA11y : public QAccessibleInterface {
 public:
  OrbitGLA11y(QWidget* o);

  // check for valid pointers
  virtual bool isValid() const { return widget_ != nullptr; }
  virtual QObject* object() const { return widget_; }

  // relations
  virtual QVector<QPair<QAccessibleInterface*, QAccessible::Relation>> relations(
      QAccessible::Relation match = QAccessible::AllRelations) const {
    return {};
  }
  virtual QAccessibleInterface* focusChild() const { return nullptr; }

  virtual QAccessibleInterface* childAt(int x, int y) const { return dummy_tracks_[0]; }

  // navigation, hierarchy
  virtual QAccessibleInterface* parent() const {
    return widget_->parent() == nullptr ? nullptr
                                        : QAccessible::queryAccessibleInterface(widget_->parent());
  }
  virtual QAccessibleInterface* child(int index) const { return dummy_tracks_[index]; }
  virtual int childCount() const { return 2; }
  virtual int indexOfChild(const QAccessibleInterface* child) const {
    return std::find(dummy_tracks_.begin(), dummy_tracks_.end(), child) - dummy_tracks_.begin();
  }

  // properties and state
  virtual QString text(QAccessible::Text t) const { return "GL Capture View"; }
  virtual void setText(QAccessible::Text t, const QString& text){};
  virtual QRect rect() const { return widget_->rect(); }
  virtual QAccessible::Role role() const { return QAccessible::Role::Grouping; }
  virtual QAccessible::State state() const { return QAccessible::State(); }

 private:
  QWidget* widget_ = nullptr;
};*/

QAccessibleInterface* GlAccessibilityFactory(const QString& classname, QObject* object);

}  // namespace orbit_qt

#endif