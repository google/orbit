// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitQt/orbitglwidget.h"

#include <QCharRef>
#include <QFlags>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QOpenGLContext>
#include <QOpenGLFunctions>
#include <QPainter>
#include <QRect>
#include <QSurfaceFormat>
#include <Qt>
#include <chrono>
#include <string_view>
#include <utility>

#include "Introspection/Introspection.h"
#include "OrbitBase/Logging.h"
#include "OrbitGl/GlCanvas.h"

OrbitGLWidget::OrbitGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
  QSurfaceFormat requested_format = QSurfaceFormat::defaultFormat();
  ORBIT_LOG("OpenGL version requested: %i.%i", requested_format.majorVersion(),
            requested_format.minorVersion());
  setFocusPolicy(Qt::WheelFocus);
  setMouseTracking(true);
  setUpdateBehavior(QOpenGLWidget::PartialUpdate);
  installEventFilter(this);

  QObject::connect(&update_timer_, &QTimer::timeout, &update_timer_, [this]() {
    ORBIT_CHECK(gl_canvas_ != nullptr);  // This timer is only running when gl_canvas_ exists.
    if (!gl_canvas_->IsRedrawNeeded()) return;
    update();
  });
}

bool OrbitGLWidget::eventFilter(QObject* /*object*/, QEvent* event) {
  if (event->type() == QEvent::Paint) {
    if (gl_canvas_) {
      gl_canvas_->PreRender();
      if (!gl_canvas_->IsRedrawNeeded()) {
        return true;
      }
    }
  }

  return false;
}

void OrbitGLWidget::Initialize(std::unique_ptr<GlCanvas> gl_canvas) {
  gl_canvas_ = std::move(gl_canvas);
  constexpr std::chrono::milliseconds kUpdatePeriod{16};  // ...ms - that's roughly 60 FPS.
  update_timer_.start(kUpdatePeriod);
}

void OrbitGLWidget::Deinitialize() {
  update_timer_.stop();
  gl_canvas_.reset();
}

void OrbitGLWidget::initializeGL() {
  initializeOpenGLFunctions();
  PrintContextInformation();
}

void OrbitGLWidget::PrintContextInformation() {
  std::string_view gl_type = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
  std::string_view gl_version = reinterpret_cast<const char*>(glGetString(GL_VERSION));
  std::string_view gl_profile =
      QMetaEnum::fromType<QSurfaceFormat::OpenGLContextProfile>().valueToKey(format().profile());

  ORBIT_LOG(R"(glType="%s", glVersion="%s", glProfile="%s")", gl_type, gl_version, gl_profile);
}

void OrbitGLWidget::resizeGL(int w, int h) {
  if (gl_canvas_) {
    gl_canvas_->Resize(w, h);
    ORBIT_CHECK(this->geometry().width() == w);
    ORBIT_CHECK(this->geometry().height() == h);
  }
}

void OrbitGLWidget::paintGL() {
  ORBIT_SCOPE_FUNCTION;

  if (gl_canvas_) {
    QPainter painter(this);
    gl_canvas_->Render(&painter, width(), height());
  }
}

void OrbitGLWidget::mousePressEvent(QMouseEvent* event) {
  if (gl_canvas_) {
    if (event->buttons() == Qt::LeftButton) {
      gl_canvas_->LeftDown(event->x(), event->y());
    }

    if (event->buttons() == Qt::RightButton) {
      gl_canvas_->RightDown(event->x(), event->y());
    }

    if (event->buttons() == Qt::MiddleButton) {
      gl_canvas_->MiddleDown(event->x(), event->y());
    }
  }

  update();
}

void OrbitGLWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (gl_canvas_) {
    if (event->button() == Qt::LeftButton) {
      gl_canvas_->LeftUp();
    }

    if (event->button() == Qt::RightButton) {
      gl_canvas_->RightUp();
    }

    if (event->button() == Qt::MiddleButton) {
      gl_canvas_->MiddleUp();
    }
  }

  update();
}

void OrbitGLWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    gl_canvas_->LeftDoubleClick();
  }

  update();
}

void OrbitGLWidget::mouseMoveEvent(QMouseEvent* event) {
  if (gl_canvas_) {
    gl_canvas_->MouseMoved(event->x(), event->y(), (event->buttons() & Qt::LeftButton) != 0u,
                           (event->buttons() & Qt::RightButton) != 0u,
                           (event->buttons() & Qt::MiddleButton) != 0u);
  }

  update();
}

void OrbitGLWidget::enterEvent(QEvent*) { gl_canvas_->SetIsMouseOver(true); }

void OrbitGLWidget::leaveEvent(QEvent*) { gl_canvas_->SetIsMouseOver(false); }

void OrbitGLWidget::keyPressEvent(QKeyEvent* event) {
  if (gl_canvas_) {
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0u;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0u;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0u;
    gl_canvas_->KeyPressed(event->key() & 0x00FFFFFF, ctrl, shift, alt);

    QString text = event->text();
    if (text.size() > 0) {
      gl_canvas_->CharEvent(text[0].unicode());
    }
  }

  update();
}

void OrbitGLWidget::keyReleaseEvent(QKeyEvent* event) {
  if (gl_canvas_) {
    bool ctrl = (event->modifiers() & Qt::ControlModifier) != 0u;
    bool shift = (event->modifiers() & Qt::ShiftModifier) != 0u;
    bool alt = (event->modifiers() & Qt::AltModifier) != 0u;
    gl_canvas_->KeyReleased(event->key() & 0x00FFFFFF, ctrl, shift, alt);
  }

  update();
}

void OrbitGLWidget::wheelEvent(QWheelEvent* event) {
  if (gl_canvas_) {
    if (event->orientation() == Qt::Vertical) {
      gl_canvas_->MouseWheelMoved(event->x(), event->y(), event->delta() / 8,
                                  (event->modifiers() & Qt::ControlModifier) != 0u);
    } else {
      gl_canvas_->MouseWheelMovedHorizontally(event->x(), event->y(), event->delta() / 8,
                                              (event->modifiers() & Qt::ControlModifier) != 0u);
    }
  }

  update();
}
