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

QRect A11yAdapter::rect() const {
  return QRect();

  using orbit_gl::A11yRect;
  using orbit_gl::GlA11yControlInterface;

  auto rect_to_global = [](A11yRect& rect, const GlA11yControlInterface* parent) {
    if (parent == nullptr) {
      return;
    }
    A11yRect parent_rect = parent->AccessibleLocalRect();
    rect.offset_by(parent_rect.left, parent_rect.top);
  };

  A11yRect rect = info_->AccessibleLocalRect();
  rect_to_global(rect, info_->AccessibleParent());
  return QRect(rect.left, rect.top, rect.width, rect.height);
}

//================ OrbitGlWidgetAccessible ==================

OrbitGlWidgetAccessible::OrbitGlWidgetAccessible(OrbitGLWidget* widget)
    : QAccessibleWidget(widget, QAccessible::Role::Chart) {}

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

QAccessibleInterface* OrbitGlWidgetAccessible::childAt(int x, int y) const { return nullptr; }

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