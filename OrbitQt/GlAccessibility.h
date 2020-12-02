// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_GL_ACCESSIBILITY_H_
#define ORBIT_QT_GL_ACCESSIBILITY_H_

#include <QAccessible>
#include <QAccessibleWidget>
#include <QWidget>

class TrackAccessibility : public QAccessibleInterface {
 public:
  TrackAccessibility(QAccessibleInterface* parent, int index) : parent_(parent), index_(index){};

  // check for valid pointers
  virtual bool isValid() const { return parent_ != nullptr; }
  virtual QObject* object() const { return nullptr; }

  // relations
  virtual QVector<QPair<QAccessibleInterface*, QAccessible::Relation> > relations(
      QAccessible::Relation match = QAccessible::AllRelations) const {
    return {};
  }
  virtual QAccessibleInterface* focusChild() const { return nullptr; }

  virtual QAccessibleInterface* childAt(int x, int y) const { return nullptr; }

  // navigation, hierarchy
  virtual QAccessibleInterface* parent() const { return parent_; }
  virtual QAccessibleInterface* child(int index) const { return nullptr; }
  virtual int childCount() const { return 0; }
  virtual int indexOfChild(const QAccessibleInterface*) const { return -1; }

  // properties and state
  virtual QString text(QAccessible::Text t) const { return "Track"; }
  virtual void setText(QAccessible::Text t, const QString& text){};
  virtual QRect rect() const { return QRect(0, 0, 100, 100); }
  virtual QAccessible::Role role() const { return QAccessible::Role::Grouping; }
  virtual QAccessible::State state() const { return QAccessible::State(); }

 private:
  QAccessibleInterface* parent_;
  int index_;
};

class OrbitGLAccessibility : public QAccessibleInterface {
 public:
  OrbitGLAccessibility(QWidget* o);
  ~OrbitGLAccessibility();

  // check for valid pointers
  virtual bool isValid() const { return widget_ != nullptr; }
  virtual QObject* object() const { return widget_; }

  // relations
  virtual QVector<QPair<QAccessibleInterface*, QAccessible::Relation> > relations(
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
  std::vector<TrackAccessibility*> dummy_tracks_;
};

QAccessibleInterface* GlAccessibilityFactory(const QString& classname, QObject* object);

#endif