#include "GlAccessibility.h"

#include <QWidget>

namespace orbit_qt {

absl::flat_hash_map<orbit_gl::GlA11yInterface*, A11yAdapter*> A11yAdapter::s_iface_to_adapter_;
absl::flat_hash_set<A11yAdapter*> A11yAdapter::s_valid_adapters_;

// OrbitGLA11y::OrbitGLA11y(QWidget* o) : widget_(o) {}

// int OrbitGLAccessibility::childCount() const { return 2; }

// QAccessibleInterface* OrbitGLAccessibility::child(int index) const { return dummy_tracks_[index];
// }

/*
QString OrbitGLAccessibility::text(QAccessible::Text t) const {
  switch (t) {
    case QAccessible::Name:
      return "Capture Window";
    case QAccessible::Description:
      return "This is my description";
    default:
      return "";
  }
}*/

QAccessibleInterface* GlAccessibilityFactory(const QString& classname, QObject* object) {
  QAccessibleInterface* iface = nullptr;
  /*if (classname == QLatin1String("OrbitGLWidget") && object && object->isWidgetType()) {
    iface =
        static_cast<QAccessibleInterface*>(new OrbitGLAccessibility(static_cast<QWidget*>(object)));
  }*/

  return iface;
}
/*
int TrackAccessibility::childCount() const { return 0; }

QAccessibleInterface* TrackAccessibility::child(int index) const { return nullptr; }

QRect TrackAccessibility::rect() const { QRect parent_rect = parent_->rect();
  return QRect(parent_rect.left(), parent_rect.top() + index_ * 110, parent_rect.width(), 100);
}

QString TrackAccessibility::text(QAccessible::Text t) const {
  switch (t) {
    case QAccessible::Name:
      return "Orbit Track";
    case QAccessible::Description:
      return "Thread track inside Orbits GL Capture Window";
    default:
      return "";
  }
}
*/

A11yAdapter* A11yAdapter::GetOrCreateAdapter(orbit_gl::GlA11yInterface* iface) {
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
  using orbit_gl::GlA11yInterface;

  auto rect_to_global = [](A11yRect& rect, GlA11yInterface* parent) {
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

}  // namespace orbit_qt