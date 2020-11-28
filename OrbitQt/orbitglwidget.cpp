// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitglwidget.h"

#include <QImageWriter>
#include <QMenuBar>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QSignalMapper>

#include "GlCanvas.h"
#include "OrbitBase/Logging.h"
#include "orbitmainwindow.h"

#define ORBIT_DEBUG_OPEN_GL 0

OrbitGLWidget::OrbitGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
  gl_canvas_ = nullptr;
  debug_logger_ = nullptr;
  setFocusPolicy(Qt::WheelFocus);
  setMouseTracking(true);
  setUpdateBehavior(QOpenGLWidget::PartialUpdate);
  installEventFilter(this);
}

bool OrbitGLWidget::eventFilter(QObject* /*object*/, QEvent* event) {
  if (event->type() == QEvent::Paint) {
    if (gl_canvas_) {
      gl_canvas_->PreRender();
      if (!gl_canvas_->GetNeedsRedraw()) {
        return true;
      }
      if (gl_canvas_->GetNeedsCheckHighlightChange()) {
        checkFunctionHighlightChange();
        gl_canvas_->ResetNeedsCheckHighlightChange();
      }
    }
  }

  return false;
}

void OrbitGLWidget::Initialize(GlCanvas::CanvasType canvas_type, OrbitMainWindow* a_MainWindow,
                               uint32_t font_size) {
  gl_canvas_ = GlCanvas::Create(canvas_type, font_size);

  if (a_MainWindow) {
    a_MainWindow->RegisterGlWidget(this);
  }
}

void OrbitGLWidget::initializeGL() {
#if ORBIT_DEBUG_OPEN_GL
  debug_logger_ = new QOpenGLDebugLogger(this);
  if (debug_logger_->initialize()) {
    LOG("GL_DEBUG Debug Logger %s", debug_logger_->metaObject()->className());
    connect(debug_logger_, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
            SLOT(messageLogged(QOpenGLDebugMessage)));
    debug_logger_->startLogging(QOpenGLDebugLogger::SynchronousLogging);
  }
  format().setOption(QSurfaceFormat::DebugContext);
#endif

  initializeOpenGLFunctions();

  if (gl_canvas_) {
    gl_canvas_->Initialize();
  }

  PrintContextInformation();
}

void OrbitGLWidget::PrintContextInformation() {
  std::string glType;
  std::string glVersion;
  std::string glProfile;

  // Get Version Information
  glType = (context()->isOpenGLES()) ? "OpenGL ES" : "OpenGL";
  glVersion = reinterpret_cast<const char*>(glGetString(GL_VERSION));

  // Get Profile Information
#define CASE(c)           \
  case QSurfaceFormat::c: \
    glProfile = #c;       \
    break
  switch (format().profile()) {
    CASE(NoProfile);
    CASE(CoreProfile);
    CASE(CompatibilityProfile);
  }
#undef CASE
  LOG(R"(glType="%s", glVersion="%s", glProfile="%s")", glType, glVersion, glProfile);
}

void OrbitGLWidget::messageLogged(const QOpenGLDebugMessage& msg) {
  // From: http://www.trentreed.net/topics/cc/

  QString error;

  // Format based on severity
  switch (msg.severity()) {
    case QOpenGLDebugMessage::NotificationSeverity:
      error += "--";
      break;
    case QOpenGLDebugMessage::HighSeverity:
      error += "!!";
      break;
    case QOpenGLDebugMessage::MediumSeverity:
      error += "!~";
      break;
    case QOpenGLDebugMessage::LowSeverity:
      error += "~~";
      break;
    default:
      break;
  }

  error += " (";

  // Format based on source
#define CASE(c)                \
  case QOpenGLDebugMessage::c: \
    error += #c;               \
    break
  switch (msg.source()) {
    CASE(APISource);
    CASE(WindowSystemSource);
    CASE(ShaderCompilerSource);
    CASE(ThirdPartySource);
    CASE(ApplicationSource);
    CASE(OtherSource);
    CASE(InvalidSource);
    default:
      break;
  }
#undef CASE

  error += " : ";

  // Format based on type
#define CASE(c)                \
  case QOpenGLDebugMessage::c: \
    error += #c;               \
    break
  switch (msg.type()) {
    CASE(ErrorType);
    CASE(DeprecatedBehaviorType);
    CASE(UndefinedBehaviorType);
    CASE(PortabilityType);
    CASE(PerformanceType);
    CASE(OtherType);
    CASE(MarkerType);
    CASE(GroupPushType);
    CASE(GroupPopType);
    default:
      break;
  }
#undef CASE

  error += ")";
  LOG("%s\n%s", qPrintable(error), qPrintable(msg.message()));
}

void OrbitGLWidget::resizeGL(int w, int h) {
  if (gl_canvas_) {
    gl_canvas_->Resize(w, h);
    gl_canvas_->SetMainWindowSize(this->geometry().width(), this->geometry().height());
  }
}

void OrbitGLWidget::paintGL() {
  ORBIT_SCOPE_FUNCTION;
  if (gl_canvas_) {
    gl_canvas_->Render(width(), height());
  }

  static volatile bool doScreenShot = false;
  if (doScreenShot) {
    TakeScreenShot();
  }
}

void OrbitGLWidget::TakeScreenShot() {
  QImage img = this->grabFramebuffer();
  QImageWriter writer("screenshot", "jpg");
  writer.write(img);
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
      if (gl_canvas_->RightUp()) {
        showContextMenu();
      }
    }

    if (event->button() == Qt::MiddleButton) {
      gl_canvas_->MiddleUp(event->x(), event->y());
    }
  }

  update();
}

void OrbitGLWidget::showContextMenu() {
  std::vector<std::string> menu = gl_canvas_->GetContextMenu();

  if (!menu.empty()) {
    QMenu contextMenu(tr("GlContextMenu"), this);
    QSignalMapper signalMapper(this);
    std::vector<QAction*> actions;

    for (size_t i = 0; i < menu.size(); ++i) {
      actions.push_back(new QAction(QString::fromStdString(menu[i])));
      connect(actions[i], SIGNAL(triggered()), &signalMapper, SLOT(map()));
      signalMapper.setMapping(actions[i], i);
      contextMenu.addAction(actions[i]);
    }

    connect(&signalMapper, SIGNAL(mapped(int)), this, SLOT(OnMenuClicked(int)));
    contextMenu.exec(QCursor::pos());

    for (QAction* action : actions) delete action;
  }
}

void OrbitGLWidget::OnMenuClicked(int a_Index) {
  const std::vector<std::string>& menu = gl_canvas_->GetContextMenu();
  gl_canvas_->OnContextMenu(menu[a_Index], a_Index);
}

void OrbitGLWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    gl_canvas_->LeftDoubleClick();
  }

  update();
}

void OrbitGLWidget::mouseMoveEvent(QMouseEvent* event) {
  if (gl_canvas_) {
    gl_canvas_->MouseMoved(event->x(), event->y(), event->buttons() & Qt::LeftButton,
                           event->buttons() & Qt::RightButton, event->buttons() & Qt::MiddleButton);
  }

  update();
}

void OrbitGLWidget::enterEvent(QEvent*) { gl_canvas_->SetIsMouseOver(true); }

void OrbitGLWidget::leaveEvent(QEvent*) { gl_canvas_->SetIsMouseOver(false); }

void OrbitGLWidget::keyPressEvent(QKeyEvent* event) {
  if (gl_canvas_) {
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    bool alt = event->modifiers() & Qt::AltModifier;
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
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    bool alt = event->modifiers() & Qt::AltModifier;
    gl_canvas_->KeyReleased(event->key() & 0x00FFFFFF, ctrl, shift, alt);
  }

  update();
}

void OrbitGLWidget::wheelEvent(QWheelEvent* event) {
  if (gl_canvas_) {
    if (event->orientation() == Qt::Vertical) {
      gl_canvas_->MouseWheelMoved(event->x(), event->y(), event->delta() / 8,
                                  event->modifiers() & Qt::ControlModifier);
    } else {
      gl_canvas_->MouseWheelMovedHorizontally(event->x(), event->y(), event->delta() / 8,
                                              event->modifiers() & Qt::ControlModifier);
    }
  }

  update();
}
