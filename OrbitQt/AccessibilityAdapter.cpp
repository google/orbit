#include "AccessibilityAdapter.h"

#include <QWidget>

namespace orbit_qt {

absl::flat_hash_map<const orbit_gl::GlA11yControlInterface*, QAccessibleInterface*>
    A11yAdapter::s_adapter_map_;
absl::flat_hash_set<A11yAdapter*> A11yAdapter::s_owned_adapters_;

//================ A11yAdapter ==================

QAccessibleInterface* A11yAdapter::GetOrCreateAdapter(
    const orbit_gl::GlA11yControlInterface* iface) {
  if (iface == nullptr) {
    return nullptr;
  }

  auto it = s_adapter_map_.find(iface);
  if (it != s_adapter_map_.end()) {
    return it->second;
  } else {
    A11yAdapter* adapter = new A11yAdapter(iface);
    s_adapter_map_.insert(std::make_pair(iface, adapter));
    s_owned_adapters_.insert(adapter);
    return adapter;
  }
}

int A11yAdapter::indexOfChild(const QAccessibleInterface* child) const {
  // TODO: This could be quite a bottleneck, I am not sure in which context
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
  using orbit_gl::GlA11yControlInterface;

  A11yRect rect = info_->AccessibleLocalRect();
  if (parent() == nullptr) {
    return QRect(rect.left, rect.top, rect.width, rect.height);
  }

  QRect parent_rect = parent()->rect();
  return QRect(rect.left + parent_rect.left(), rect.top + parent_rect.top(), rect.width,
               rect.height);
}

//================ OrbitGlWidgetAccessible ==================

OrbitGlWidgetAccessible::OrbitGlWidgetAccessible(OrbitGLWidget* widget)
    : QAccessibleWidget(widget, QAccessible::Role::Graphic, "CaptureWindow") {}

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
    A11yAdapter::AddBridge(static_cast<OrbitGLWidget*>(object)->GetCanvas(), iface);
  }

  return iface;
}

void InstallAccessibilityFactories() {
  QAccessible::installFactory(orbit_qt::GlAccessibilityFactory);
}

}  // namespace orbit_qt