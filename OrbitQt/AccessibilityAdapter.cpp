#include "AccessibilityAdapter.h"

#include <QWidget>

namespace orbit_qt {

absl::flat_hash_map<const orbit_gl::GlAccessibleInterface*, QAccessibleInterface*>
    A11yAdapter::interface_map_;
bool A11yAdapter::initialized_ = false;

//================ A11yAdapter ==================

void A11yAdapter::Init() {
  orbit_gl::GlAccessibleInterfaceRegistry::Get().OnUnregistered(
      [](orbit_gl::GlAccessibleInterface* iface) {
        if (interface_map_.contains(iface)) {
          auto adapter = dynamic_cast<A11yAdapter*>(interface_map_[iface]);
          if (adapter != nullptr) {
            delete adapter;
          }
          interface_map_.erase(iface);
        }
      });
  initialized_ = true;
}

QAccessibleInterface* A11yAdapter::GetOrCreateAdapter(
    const orbit_gl::GlAccessibleInterface* iface) {
  if (!initialized_) {
    Init();
  }

  if (iface == nullptr) {
    return nullptr;
  }

  auto it = interface_map_.find(iface);
  if (it != interface_map_.end()) {
    return it->second;
  } else {
    A11yAdapter* adapter = new A11yAdapter(iface);
    RegisterAdapter(iface, adapter);
    return adapter;
  }
}

void A11yAdapter::QAccessibleDeleted(QAccessibleInterface* iface) {
  for (auto& it : interface_map_) {
    if (it.second == iface) {
      interface_map_.erase(it.first);
      return;
    }
  }

  UNREACHABLE();
}

int A11yAdapter::indexOfChild(const QAccessibleInterface* child) const {
  // This could be quite a bottleneck, I am not sure in which context
  // and how excessive this method is actually called.
  for (int i = 0; i < info_->AccessibleChildCount(); ++i) {
    if (GetOrCreateAdapter(info_->AccessibleChild(i)) == child) {
      return i;
    }
  }

  return -1;
}

QAccessibleInterface* A11yAdapter::childAt(int x, int y) const {
  for (int i = 0; i < childCount(); ++i) {
    auto c = child(i);
    auto rect = c->rect();
    if (x >= rect.x() && x < rect.x() + rect.width() && y >= rect.y() &&
        y < rect.y() + rect.height()) {
      return c;
    }
  }

  return nullptr;
}

QRect A11yAdapter::rect() const {
  using orbit_gl::A11yRect;
  using orbit_gl::GlAccessibleInterface;

  A11yRect rect = info_->AccessibleLocalRect();
  if (parent() == nullptr) {
    return QRect(rect.left, rect.top, rect.width, rect.height);
  }

  QRect parent_rect = parent()->rect();
  return QRect(rect.left + parent_rect.left(), rect.top + parent_rect.top(), rect.width,
               rect.height);
}

QAccessible::Role A11yAdapter::role() const {
  auto role = info_->AccessibleRole();
  return *reinterpret_cast<QAccessible::Role*>(&role);
}

//================ OrbitGlWidgetAccessible ==================

OrbitGlWidgetAccessible::OrbitGlWidgetAccessible(OrbitGLWidget* widget)
    : QAccessibleWidget(widget, QAccessible::Role::Graphic, "CaptureWindow") {
  CHECK(widget != nullptr);
  // TODO (freichl@) For some reason setting an accessible name for the Canvas results in a memory
  // access exception during runtime when accessibility is queried
  // This also happens when the accessibleName is explicitely set to "" in Qt Designer, which this
  // check can't catch...
  CHECK(widget->accessibleName() == "");
  A11yAdapter::RegisterAdapter(static_cast<OrbitGLWidget*>(widget)->GetCanvas(), this);
}

int OrbitGlWidgetAccessible::childCount() const {
  return static_cast<OrbitGLWidget*>(widget())->GetCanvas()->AccessibleChildCount();
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
  return A11yAdapter::GetOrCreateAdapter(
      static_cast<OrbitGLWidget*>(widget())->GetCanvas()->AccessibleChild(index));
}

//================ Free functions ==================

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