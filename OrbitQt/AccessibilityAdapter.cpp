#include "AccessibilityAdapter.h"

#include <QWidget>

namespace orbit_qt {

absl::flat_hash_map<const orbit_gl::GlA11yControlInterface*, A11yAdapter*>
    A11yAdapter::s_iface_to_adapter_;
absl::flat_hash_set<A11yAdapter*> A11yAdapter::s_valid_adapters_;

//================ A11yAdapter ==================

A11yAdapter* A11yAdapter::GetOrCreateAdapter(const orbit_gl::GlA11yControlInterface* iface) {
  auto it = s_iface_to_adapter_.find(iface);
  if (it != s_iface_to_adapter_.end()) {
    CHECK(s_valid_adapters_.contains(it->second));
    return it->second;
  } else {
    s_iface_to_adapter_.insert(std::make_pair(iface, new A11yAdapter(iface)));
    auto result = s_iface_to_adapter_[iface];
    s_valid_adapters_.insert(result);
    return result;
  }
}

void A11yAdapter::ClearAdapterCache() {
  for (auto adapter : s_valid_adapters_) {
    delete adapter;
  }

  s_iface_to_adapter_.clear();
  s_valid_adapters_.clear();
}

void A11yAdapter::ReleaseAdapter(A11yAdapter* adapter) {
  s_valid_adapters_.erase(adapter);
  delete adapter;
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
  return widget_->GetCanvas()->AccessibleChildCount();
}

int OrbitGlWidgetAccessible::indexOfChild(const QAccessibleInterface* child) {
  for (int i = 0; i < childCount(); ++i) {
    if (this->child(i) == child) {
      return i;
    }
  }

  return -1;
}

QAccessibleInterface* OrbitGlWidgetAccessible::childAt(int x, int y) const { return nullptr; }

QAccessibleInterface* OrbitGlWidgetAccessible::child(int index) const {
  return A11yAdapter::GetOrCreateAdapter(widget_->GetCanvas()->AccessibleChild(index));
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

}  // namespace orbit_qt