#include "GlAccessibility.h"

#include <QWidget>

OrbitGLAccessibility::OrbitGLAccessibility(QWidget* o) : widget_(o) {
  dummy_tracks_.push_back(new TrackAccessibility(this, 0));
  dummy_tracks_.push_back(new TrackAccessibility(this, 1));
}

OrbitGLAccessibility::~OrbitGLAccessibility() {
  for (auto track : dummy_tracks_) {
    delete track;
  }
  dummy_tracks_.clear();
}

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
  if (classname == QLatin1String("OrbitGLWidget") && object && object->isWidgetType()) {
    iface =
        static_cast<QAccessibleInterface*>(new OrbitGLAccessibility(static_cast<QWidget*>(object)));
  }

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