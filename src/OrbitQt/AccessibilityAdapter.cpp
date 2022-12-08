// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

/*
 * Bridging accessibility from OrbitQt and OrbitGl.
 *
 * Design can be found at go/stadia-orbit-capture-window-e2e.
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
 * QAccessibleInterface and the elements implemented in OrbitGl. All required interfaces for
 * accessibility are defined in the "OrbitAccessibility" module, which is required by OrbitGl.
 *
 * OrbitAccessibility defines an AccessibleInterface (see "AccessibleInterface.h") that exposes a
 * relevant subset of accessibility information (most importantly child and parent information).
 * Elements in OrbitGl can implement this interface. OrbitQt then defines an adapter class
 * "AccessibilityAdapter" which implements the QAccessibleInterface, translating all calls from a
 * AccessibleInterface to the Window API:
 *
 *        OrbitAccessibility                  |                OrbitQt
 *                                            |
 *                 +----------------------+   |   +-----------------------------+
 *                 |AccessibleInterface   +<------+AccessibilityAdapter         |
 *                 +--+-------------+-----+   |   +-----------------------------+
 *                    |             ^ Parent  |
 *                    v Child(i)    |         |
 *                 +--+-------------+-----+   |   +-----------------------------+
 *                 |AccessibleInterface   +<------+AccessibilityAdapter         |
 *                 +----------------------+   |   +-----------------------------+
 *                                            |
 *                                            +
 *
 * AdapterRegistry keeps track of all adapters created so far and, given
 * an AccessibleInterface, can find the corresponsing QAccessibleInterface (which will be
 * implemented by a AccessibilityAdapter in most cases, but we'll get there...).
 * AdapterRegistry::GetOrCreateAdapter will return an existing interface, or create a new
 * adapter if needed. This usually happens as the tree is traversed - each AccessibilityAdapter
 * will query the children exposed through its AccessibleInterface pointer, and will create new
 * adapters for each of them as we go down the tree.
 *
 * To not interfere with the internal resource management of QT, AdapterRegistry does not create
 * AccessibilityAdapter instances directly, but instead creates dummy QObjects of type
 * OrbitGlInterfaceWrapper. By registering a custom accessibility factory, QT then takes care of
 * creating the corresponding AccessibilityAdapters, and AdapterRegistry merely manages the lifetime
 * of the dummy objects instead of the adapters.
 *
 * Everything above OrbitGlWidget is handled by the default implementation of Qt, and everything
 * below is handled by these adapters. To bridge the gap between OrbitGlWidget and GlCanvas, there
 * exists OrbitGlWidgetAccessible. This class inherits from QAccessibleWidget and uses its default
 * functionality to walk *up* the tree, but replaces all methods to walk *down* the tree and
 * forwards those calls to the AccessibleInterface of GlCanvas. It is important to note that the
 * *parent* methods of the AccessibleInterface associated with GlCanvas are thus never invoked.
 *
 *
 *   OrbitGl           OrbitAccessibility    +                                 OrbitQt
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
 * | GlCanvas  +<--+AccessibleInterface   +<----------------------------+
 * +-----------+   +--+-------------+-----+  |
 *                    |             ^ Parent |
 *                    v Child(i)    |        |
 * +-----------+   +--+-------------+-----+  |                    +-----------------------------+
 * | TimeGraph +<--+AccessibleInterface   +<----------------------+AccessibilityAdapter         |
 * +-----------+   +----------------------+  |                    +-----------------------------+
 *                                           |
 *                                           +
 *
 * The OrbitGlWidgetAccessible is automatically created when an OrbitGlWidget is constructed by
 * installing a QT Accessibility Factory (InstallAccessibilityFactories()).
 *
 * To make sure adapters created by AdapterRegistry::GetOrCreateAdapter are deleted when the
 * corresponding AccessibleInterface is deleted, these interfaces register themselves in the
 * AccessibleInterfaceRegistry (see AccessibleInterfaceRegistry.h), which in turn allows to register
 * a callback on interface deletion. AccessibilityAdapter registers itself for this callback and
 * cleans up interfaces accordingly.
 */

#include "OrbitQt/AccessibilityAdapter.h"

#include <absl/meta/type_traits.h>

#include <QAccessibleWidget>
#include <QLatin1String>
#include <mutex>
#include <utility>

#include "OrbitAccessibility/AccessibleInterfaceRegistry.h"
#include "OrbitGl/GlCanvas.h"
#include "OrbitQt/orbitglwidget.h"

using orbit_accessibility::AccessibleInterface;
using orbit_accessibility::AccessibleInterfaceRegistry;

namespace orbit_qt {

AdapterRegistry& AdapterRegistry::Get() {
  static AdapterRegistry registry;
  static std::once_flag flag;

  std::call_once(flag, []() {
    AccessibleInterfaceRegistry::Get().SetOnUnregisterCallback(
        [](AccessibleInterface* iface) { registry.OnInterfaceDeleted(iface); });
  });

  return registry;
}

QAccessibleInterface* AdapterRegistry::InterfaceWrapperFactory(const QString& classname,
                                                               QObject* object) {
  if (classname == QLatin1String("orbit_qt::OrbitGlInterfaceWrapper")) {
    auto* iface_obj = static_cast<OrbitGlInterfaceWrapper*>(object);
    ORBIT_CHECK(!all_interfaces_map_.contains(iface_obj->GetInterface()));
    auto wrapper = std::make_unique<OrbitGlInterfaceWrapper>(iface_obj->GetInterface());
    QAccessibleInterface* result = new AccessibilityAdapter(iface_obj->GetInterface(), object);
    RegisterAdapter(iface_obj->GetInterface(), result);
    return result;
  }

  return nullptr;
}

/*
 * Callback fired when GlAccessibleInterfaces are deleted. This takes care of deleting only those
 * interfaces created by AccessibilityAdapter.
 */
void AdapterRegistry::OnInterfaceDeleted(AccessibleInterface* iface) {
  if (all_interfaces_map_.contains(iface)) {
    all_interfaces_map_.erase(iface);
  }

  if (managed_adapters_.contains(iface)) {
    managed_adapters_.erase(iface);
  }
}

QAccessibleInterface* AdapterRegistry::GetOrCreateAdapter(const AccessibleInterface* iface) {
  if (iface == nullptr) {
    return nullptr;
  }

  auto it = all_interfaces_map_.find(iface);
  if (it != all_interfaces_map_.end()) {
    return it->second;
  }

  auto wrapper = std::make_unique<OrbitGlInterfaceWrapper>(iface);
  QAccessibleInterface* result = QAccessible::queryAccessibleInterface(wrapper.get());
  ORBIT_CHECK(result != nullptr);
  RegisterAdapter(iface, result);
  managed_adapters_.emplace(iface, std::move(wrapper));
  return result;
}

int AccessibilityAdapter::indexOfChild(const QAccessibleInterface* child) const {
  // This could be quite a bottleneck, I am not sure in which context
  // and how excessive this method is actually called.
  for (int i = 0; i < info_->AccessibleChildCount(); ++i) {
    if (AdapterRegistry::Get().GetOrCreateAdapter(info_->AccessibleChild(i)) == child) {
      return i;
    }
  }

  return -1;
}

QAccessibleInterface* AccessibilityAdapter::childAt(int x, int y) const {
  for (int i = 0; i < childCount(); ++i) {
    QAccessibleInterface* child_iface = child(i);
    if (child_iface == nullptr) {
      continue;
    }

    QRect rect = child_iface->rect();
    if (x >= rect.x() && x < rect.x() + rect.width() && y >= rect.y() &&
        y < rect.y() + rect.height()) {
      return child_iface;
    }
  }

  return nullptr;
}

QRect AccessibilityAdapter::rect() const {
  orbit_accessibility::AccessibilityRect rect = info_->AccessibleRect();

  // Find the outer most AccessibleInterface to convert the coordinates from
  // pixel relative to the CaptureWindow to screen positions.

  const orbit_accessibility::AccessibleInterface* root = info_->AccessibleParent();
  while (root != nullptr && root->AccessibleParent() != nullptr) {
    root = root->AccessibleParent();
  }

  const QAccessibleInterface* bridge = AdapterRegistry::Get().GetOrCreateAdapter(root);

  if (bridge != nullptr) {
    QRect bridge_rect = bridge->rect();
    return {rect.left + bridge_rect.left(), rect.top + bridge_rect.top(), rect.width, rect.height};
  }

  return {rect.left, rect.top, rect.width, rect.height};
}

QAccessible::Role AccessibilityAdapter::role() const {
  return absl::bit_cast<QAccessible::Role>(info_->AccessibleRole());
}

// Accessibility interface for OrbitGlWidget. See header file documentation above for details.
class OrbitGlWidgetAccessible : public QAccessibleWidget {
 public:
  explicit OrbitGlWidgetAccessible(OrbitGLWidget* widget);

  [[nodiscard]] QAccessibleInterface* child(int index) const override;
  [[nodiscard]] int childCount() const override;
  int indexOfChild(const QAccessibleInterface* child) const override;
};

OrbitGlWidgetAccessible::OrbitGlWidgetAccessible(OrbitGLWidget* widget)
    : QAccessibleWidget(widget, QAccessible::Role::Graphic, "CaptureWindow") {
  ORBIT_CHECK(widget != nullptr);
  /* TODO(b/175676123): For some reason setting an accessible name for the Canvas results in
   * a memory access exception during runtime when accessibility is queried. This also happens when
   * the accessibleName is explicitely set to "" in Qt Designer, which this check can't catch...
   */
  ORBIT_CHECK(widget->accessibleName() == "");
  AdapterRegistry::Get().RegisterAdapter(
      static_cast<OrbitGLWidget*>(widget)->GetCanvas()->GetOrCreateAccessibleInterface(), this);
}

int OrbitGlWidgetAccessible::childCount() const {
  return static_cast<OrbitGLWidget*>(widget())
      ->GetCanvas()
      ->GetOrCreateAccessibleInterface()
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
  return AdapterRegistry::Get().GetOrCreateAdapter(static_cast<OrbitGLWidget*>(widget())
                                                       ->GetCanvas()
                                                       ->GetOrCreateAccessibleInterface()
                                                       ->AccessibleChild(index));
}

QAccessibleInterface* GlAccessibilityFactory(const QString& classname, QObject* object) {
  QAccessibleInterface* iface = nullptr;
  if (classname == QLatin1String("OrbitGLWidget") && object->isWidgetType()) {
    iface = static_cast<QAccessibleInterface*>(
        new OrbitGlWidgetAccessible(static_cast<OrbitGLWidget*>(object)));
  }

  return iface;
}

QAccessibleInterface* WrapperAccessibilityFactory(const QString& classname, QObject* object) {
  return AdapterRegistry::Get().InterfaceWrapperFactory(classname, object);
}

void InstallAccessibilityFactories() {
  QAccessible::installFactory(orbit_qt::GlAccessibilityFactory);
  QAccessible::installFactory(WrapperAccessibilityFactory);
}

}  // namespace orbit_qt