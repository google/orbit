// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * Bridging accessibility from OrbitQt and OrbitGl.
 *
 * The accessibility adapter exposes accessibility information of the OpenGl capture window to the
 * E2E tests. E2E tests work on top of Microsoft UI Automation
 *(https://docs.microsoft.com/en-us/windows/win32/winauto/uiauto-msaa).
 *
 * QT provides support for this out of the box: Any QT Object has a corresponding
 * QAccessibleInterface that is constructed automatically, and tracked in a central registry.
 * QAccessibleInterfaces implement methods to access their children and parents, and this
 * information is used to walk all objects in the accessibility tree starting from the main window.
 * To simulate widgets underneath the capture window (such as tracks, event bars etc), the capture
 * window widget needs to add QAccessibleInterface objects into this tree that provide information
 * about the children, and their recursive children. The QAccessibleInterface hides all the nitty
 * gritty details that the Windows API requires to query the accessibility tree starting from the
 * main window handle.
 *
 * It is important to note that QObjects do not directly implement the QAccessibleInterface. The
 * tree of QAccessibleInterfaces is managed in a seperate data structure, and each node points to a
 * QObject to query all of the required information.
 *
 * As OrbitGl is compiled without QT dependencies, this file provides classes to bridge between
 * QAccessibleInterface and the elements implemented in OrbitGl.
 *
 * OrbitGl defines a GlAccessibleInterface (see "OrbitGlAccessible.h") that exposes a relevant
 * subset of accessibility information (most importantly child and parent information). Elements in
 * OrbitGl can implement this interface. OrbitQt then defines an adapter class
 * "AccessibilityAdapter" which implements the QAccessibleInterface, translating all calls from a
 * GlAccessibleInterface to the Window API:
 *
 *                OrbitGl                     |                OrbitQt
 *                                            |
 *                 +----------------------+   |   +-----------------------------+
 *                 |GlAccessibleInterface +<------+AccessibilityAdapter         |
 *                 +--+-------------+-----+   |   +-----------------------------+
 *                    |             ^ Parent  |
 *                    v Child(i)    |         |
 *                 +--+-------------+-----+   |   +-----------------------------+
 *                 |GlAccessibleInterface +<------+AccessibilityAdapter         |
 *                 +----------------------+   |   +-----------------------------+
 *                                            |
 *                                            +
 *
 * The static methods of AccessibilityAdapter keep track of all adapters created so far and, given a
 * GlAccessibleInterface, can find the corresponsing QAccessibleInterface (which will be implemented
 * by a AccessibilityAdapter in most cases, but we'll get there...).
 * AccessibilityAdapter::GetOrCreateAdapter will return an existing interface, or create a new
 * adapter if needed. This usually happens as the tree is traversed
 * - each AccessibilityAdapter will query the children exposed through its GlAccessibleInterface
 * pointer, and will create new adapters for each of them as we go down the tree.
 *
 * Everything above OrbitGlWidget is handled by the default implementation of Qt, and everything
 * below is handled by these adapters. To bridge the gap between OrbitGlWidget and GlCanvas, there
 * exists OrbitGlWidgetAccessible. This class inherits from QAccessibleWidget and uses its default
 * functionality to walk *up* the tree, but replaces all methods to walk *down* the tree and
 * forwards those calls to the GlAccessibleInterface of GlCanvas. It is important to note that the
 * *parent* methods of the GlAccessibleInterface associated with GlCanvas are thus never invoked.
 *
 *
 *                OrbitGl                    +                                 OrbitQt
 *                                           |
 *                                           |                          ^
 *                                           |                          | parent(): QT Default
 *                                           |                          |
 *                                           |  +-------------+   +-----+----------------------+
 *                                           |  |OrbitGlWidget+<--+OrbitGlWidgetAccessible     |
 *                                           |  +-------------+   +-----+----------------------+
 *                                           |                          |
 *                                           |                          |
 * +-----------+   +----------------------+  |                          |  Adapter for child()
 * | GlCanvas  +<--+GlAccessibleInterface +<----------------------------+
 * +-----------+   +--+-------------+-----+  |
 *                    |             ^ Parent |
 *                    v Child(i)    |        |
 * +-----------+   +--+-------------+-----+  |                    +-----------------------------+
 * | TimeGraph +<--+GlAccessibleInterface +<----------------------+AccessibilityAdapter         |
 * +-----------+   +----------------------+  |                    +-----------------------------+
 *                                           |
 *                                           +
 *
 * The OrbitGlWidgetAccessible is automatically created when an OrbitGlWidget is constructed by
 * installing a QT Accessibility Factory (InstallAccessibilityFactories()).
 *
 * To make sure adapters created by AccessibilityAdapter::GetOrCreateAdapter are deleted when the
 * corresponding GlAccessibleInterface is deleted, these interfaces register themselves in the
 * GlAccessibleInterfaceRegistry (see OrbitGlAccessibility.h), which in turn allows to register a
 * callback on interface deletion. AccessibilityAdapter registers itself for this callback and
 * cleans up interfaces accordingly.
 */

#include "AccessibilityAdapter.h"

#include <QWidget>
#include <mutex>

namespace orbit_qt {

absl::flat_hash_map<const orbit_gl::GlAccessibleInterface*, QAccessibleInterface*>
    AccessibilityAdapter::interface_map_;
absl::flat_hash_set<const QAccessibleInterface*> AccessibilityAdapter::managed_adapters_;

void AccessibilityAdapter::Init() {
  orbit_gl::GlAccessibleInterfaceRegistry::Get().SetOnUnregisterCallback(
      AccessibilityAdapter::OnInterfaceDeleted);
}

/*
 * Callback fired when GlAccessibleInterfaces are deleted. This takes care of deleting only those
 * interfaces created by AccessibilityAdapter.
 */
void AccessibilityAdapter::OnInterfaceDeleted(orbit_gl::GlAccessibleInterface* iface) {
  if (interface_map_.contains(iface)) {
    QAccessibleInterface* adapter = interface_map_.at(iface);
    interface_map_.erase(iface);
    if (managed_adapters_.contains(adapter)) {
      managed_adapters_.erase(adapter);
      // Cast is needed because ~QAccessibleWidget is protected, but we know that each element in
      // managed_adapters_ is of type AccessibilityAdapter*
      delete static_cast<AccessibilityAdapter*>(adapter);
    }
  }
}

QAccessibleInterface* AccessibilityAdapter::GetOrCreateAdapter(
    const orbit_gl::GlAccessibleInterface* iface) {
  static std::once_flag flag;
  std::call_once(flag, Init);

  if (iface == nullptr) {
    return nullptr;
  }

  auto it = interface_map_.find(iface);
  if (it != interface_map_.end()) {
    return it->second;
  }

  AccessibilityAdapter* adapter = new AccessibilityAdapter(iface);
  RegisterAdapter(iface, adapter);
  managed_adapters_.insert(adapter);
  return adapter;
}

void AccessibilityAdapter::QAccessibleDeleted(QAccessibleInterface* iface) {
  for (auto& it : interface_map_) {
    if (it.second == iface) {
      interface_map_.erase(it.first);
      return;
    }
  }

  UNREACHABLE();
}

int AccessibilityAdapter::indexOfChild(const QAccessibleInterface* child) const {
  // This could be quite a bottleneck, I am not sure in which context
  // and how excessive this method is actually called.
  for (int i = 0; i < info_->AccessibleChildCount(); ++i) {
    if (GetOrCreateAdapter(info_->AccessibleChild(i)) == child) {
      return i;
    }
  }

  return -1;
}

QAccessibleInterface* AccessibilityAdapter::childAt(int x, int y) const {
  for (int i = 0; i < childCount(); ++i) {
    QAccessibleInterface* child_iface = child(i);
    QRect rect = child_iface->rect();
    if (x >= rect.x() && x < rect.x() + rect.width() && y >= rect.y() &&
        y < rect.y() + rect.height()) {
      return child_iface;
    }
  }

  return nullptr;
}

QRect AccessibilityAdapter::rect() const {
  orbit_gl::AccessibilityRect rect = info_->AccessibleLocalRect();
  if (parent() == nullptr) {
    return QRect(rect.left, rect.top, rect.width, rect.height);
  }

  QRect parent_rect = parent()->rect();
  return QRect(rect.left + parent_rect.left(), rect.top + parent_rect.top(), rect.width,
               rect.height);
}

QAccessible::Role AccessibilityAdapter::role() const {
  return absl::bit_cast<QAccessible::Role>(info_->AccessibleRole());
}

// Accessibility interface for OrbitGlWidget. See header file documentation above for details.
class OrbitGlWidgetAccessible : public QAccessibleWidget {
 public:
  explicit OrbitGlWidgetAccessible(OrbitGLWidget* widget);

  QAccessibleInterface* child(int index) const override;
  int childCount() const override;
  int indexOfChild(const QAccessibleInterface* child) const override;

 protected:
  ~OrbitGlWidgetAccessible();
};

OrbitGlWidgetAccessible::OrbitGlWidgetAccessible(OrbitGLWidget* widget)
    : QAccessibleWidget(widget, QAccessible::Role::Graphic, "CaptureWindow") {
  CHECK(widget != nullptr);
  /* TODO(175676123): For some reason setting an accessible name for the Canvas results in a memory
   * access exception during runtime when accessibility is queried.
   * This also happens when the accessibleName is explicitely set to "" in Qt Designer, which this
   * check can't catch...
   */
  CHECK(widget->accessibleName() == "");
  AccessibilityAdapter::RegisterAdapter(
      static_cast<OrbitGLWidget*>(widget)->GetCanvas()->Accessibility(), this);
}

OrbitGlWidgetAccessible::~OrbitGlWidgetAccessible() {
  AccessibilityAdapter::QAccessibleDeleted(this);
}

int OrbitGlWidgetAccessible::childCount() const {
  return static_cast<OrbitGLWidget*>(widget())
      ->GetCanvas()
      ->Accessibility()
      ->AccessibleChildCount();
}

int OrbitGlWidgetAccessible::indexOfChild(const QAccessibleInterface* child) const {
  for (int i = 0; i < childCount(); ++i) {
    if (this->child(i) == child) {
      return i;
    }
  }

  return -1;
}

QAccessibleInterface* OrbitGlWidgetAccessible::child(int index) const {
  return AccessibilityAdapter::GetOrCreateAdapter(
      static_cast<OrbitGLWidget*>(widget())->GetCanvas()->Accessibility()->AccessibleChild(index));
}

QAccessibleInterface* GlAccessibilityFactory(const QString& classname, QObject* object) {
  QAccessibleInterface* iface = nullptr;
  if (classname == QLatin1String("OrbitGLWidget") && object && object->isWidgetType()) {
    iface = static_cast<QAccessibleInterface*>(
        new OrbitGlWidgetAccessible(static_cast<OrbitGLWidget*>(object)));
  }

  return iface;
}

void InstallAccessibilityFactories() {
  QAccessible::installFactory(orbit_qt::GlAccessibilityFactory);
}

}  // namespace orbit_qt