// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/**
Bridging accessibility from OrbitQt and OrbitGl.

The accessibility adapter exposes accessibility information of the OpenGl capture window to the E2E
tests. E2E tests work on top of Microsoft UI Automation
(https://docs.microsoft.com/en-us/windows/win32/winauto/uiauto-msaa).

QT provides support for this out of the box: Any QT Object has a corresponding QAccessibleInterface
that is constructed automatically, and tracked in a central registry. QAccessibleInterfaces
implement methods to access their children and parents, and this information is used to walk all
objects in the accessibility tree starting from the main window. To simulate widgets underneath the
capture window (such as tracks, event bars etc), the capture window widget needs to add
QAccessibleInterface objects into this tree that provide information about the children, and their
recursive children. The QAccessibleInterface hides all the nitty gritty details that the Windows API
requires to query the accessibility tree starting from the main window handle.

It is important to note that QObjects do not directly implement the QAccessibleInterface. The tree
of QAccessibleInterfaces is managed in a seperate data structure, and each node points to a QObject
to query all of the required information.

As OrbitGl is compiled without QT dependencies, this file provides classes to bridge between
QAccessibleInterface and the elements implemented in OrbitGl.

OrbitGl defines a GlAccessibleInterface (see "OrbitGlAccessible.h") that exposes a relevant subset
of accessibility information (most importantly child and parent information). Elements in OrbitGl
can implement this interface. OrbitQt then defines an adapter class "A11yAdapter" which implements
the QAccessibleInterface, translating all calls from a GlAccessibleInterface to the Window API:

                OrbitGl                     |                OrbitQt
                                            |
                 +----------------------+   |   +-----------------------------+
                 |GlAccessibleInterface +<------+A11yAdapter                  |
                 +--+-------------+-----+   |   +-----------------------------+
                    |             ^ Parent  |
                    v Child(i)    |         |
                 +--+-------------+-----+   |   +-----------------------------+
                 |GlAccessibleInterface +<------+A11yAdapter                  |
                 +----------------------+   |   +-----------------------------+
                                            |
                                            +

The static methods of A11yAdapter keep track of all adapters created so far and, given a
GlAccessibleInterface, can find the corresponsing QAccessibleInterface (which will be implemented by
a A11yAdapter in most cases, but we'll get there...). A11yAdapter::GetOrCreateAdapter will return an
existing interface, or create a new adapter if needed. This usually happens as the tree is traversed
- each A11yAdapter will query the children exposed through it's GlAccessibleInterface pointer, and
will create new adapters for each of them as we go down the tree.

Everything above OrbitGlWidget is handled by the default implementation of Qt, and everything below
is handled by these adapters. To bridge the gap between OrbitGlWidget and GlCanvas, there exists
OrbitGlWidgetAccessible. This class inherits from QAccessibleWidget and uses it's default
functionality to walk *up* the tree, but replaces all methods to walk *down* the tree and forwards
those calls to the GlAccessibleInterface of GlCanvas. It is important to note that the *parent*
methods of the GlAccessibleInterface associated with GlCanvas as thus never invoked.


               OrbitGl                    +                                 OrbitQt
                                          |
                                          |                          ^
                                          |                          | parent(): QT Default
                                          |                          |
                                          |  +-------------+   +-----+----------------------+
                                          |  |OrbitGlWidget+<--+OrbitGlWidgetAccessible     |
                                          |  +-------------+   +-----+----------------------+
                                          |                          |
                                          |                          |
+-----------+   +----------------------+  |                          |  Adapter for child()
| GlCanvas  +<--+GlAccessibleInterface +<----------------------------+
+-----------+   +--+-------------+-----+  |
                   |             ^ Parent |
                   v Child(i)    |        |
+-----------+   +--+-------------+-----+  |                    +-----------------------------+
| TimeGraph +<--+GlAccessibleInterface +<----------------------+A11yAdapter                  |
+-----------+   +----------------------+  |                    +-----------------------------+
                                          |
                                          +

The OrbitGlWidgetAccessible is automatically created when an OrbitGlWidget is constructed by
installing a QT Accessibility Factory (InstallAccessibilityFactories()).

To make sure adapters created by A11yAdapter::GetOrCreateAdapter are deleted when the corresponding
GlAccessibleInterface is deleted, these interfaces register themselves in the
GlAccessibleInterfaceRegistry (see OrbitGlAccessibility.h), which in turn allows to register a
callback on interface deletion. A11yAdapter registers itself for this callback and cleans up
interfaces accordingly.
**/

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

/*Instances of this act as adapters from QAccessibleInterface to orbit_gl::GlAccessibleInterface and
 vice versa. Static methods of A11yAdapter provide factory methods and tracking of adapters.

 See file documentation above for more details.
 */
class A11yAdapter : public QAccessibleInterface {
 public:
  A11yAdapter() = delete;
  A11yAdapter(A11yAdapter& rhs) = delete;
  A11yAdapter(A11yAdapter&& rhs) = delete;

  // vvv Overrides QAccessibleInterface vvv
  bool isValid() const override {
    bool result = info_ != nullptr;
    CHECK(!result || interface_map_.find(info_)->second == this);
    return result;
  }
  QObject* object() const override { return &dummy_; }
  QAccessibleInterface* focusChild() const override { return nullptr; }

  QAccessibleInterface* parent() const override {
    return GetOrCreateAdapter(info_->AccessibleParent());
  }
  QAccessibleInterface* child(int index) const override {
    return GetOrCreateAdapter(info_->AccessibleChild(index));
  }
  int childCount() const override { return info_->AccessibleChildCount(); }
  int indexOfChild(const QAccessibleInterface* child) const override;
  [[nodiscard]] QAccessibleInterface* childAt(int x, int y) const override;

  QString text(QAccessible::Text /*t*/) const override { return info_->AccessibleName().c_str(); }
  void setText(QAccessible::Text /*t*/, const QString& /*text*/) override{};
  QRect rect() const override;
  QAccessible::Role role() const override;

  virtual QAccessible::State state() const override {
    static_assert(sizeof(QAccessible::State) == sizeof(orbit_gl::A11yState));
    auto state = info_->AccessibleState();
    return *reinterpret_cast<QAccessible::State*>(&state);
  }

  // vvv Statics vvv
  static QAccessibleInterface* GetOrCreateAdapter(const orbit_gl::GlAccessibleInterface* iface);
  static void RegisterAdapter(const orbit_gl::GlAccessibleInterface* gl_control,
                              QAccessibleInterface* qt_control) {
    interface_map_.insert(std::make_pair(gl_control, qt_control));
  }
  // Called when a QAccessibleInterface which has been registered through "RegisterAdapter",
  // but not created by this class, is deleted. Should only be needed for OrbitGlWidgets.
  static void QAccessibleDeleted(QAccessibleInterface* iface);

  static int RegisteredAdapterCount() { return interface_map_.size(); }

 private:
  mutable QObject dummy_;
  A11yAdapter(const orbit_gl::GlAccessibleInterface* info) : info_(info){};

  const orbit_gl::GlAccessibleInterface* info_ = nullptr;

  static absl::flat_hash_map<const orbit_gl::GlAccessibleInterface*, QAccessibleInterface*>
      interface_map_;
  static bool initialized_;

  static void Init();
};

/*Accessibility interface for OrbitGlWidget. See file documentation above for details.*/
class OrbitGlWidgetAccessible : public QAccessibleWidget {
 public:
  OrbitGlWidgetAccessible(OrbitGLWidget* widget);

  QAccessibleInterface* child(int index) const override;
  int childCount() const override;
  int indexOfChild(const QAccessibleInterface* child) const override;
};

void InstallAccessibilityFactories();

}  // namespace orbit_qt

#endif