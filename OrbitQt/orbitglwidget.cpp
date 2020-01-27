//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitglwidget.h"

#include <QImageWriter>
#include <QMenuBar>
#include <QMouseEvent>
#include <QOpenGLDebugLogger>
#include <QOpenGLDebugMessage>
#include <QSignalMapper>

#include "../OrbitCore/PrintVar.h"
#include "../OrbitCore/Utils.h"
#include "orbitmainwindow.h"

#define ORBIT_DEBUG_OPEN_GL 0

//-----------------------------------------------------------------------------
OrbitGLWidget::OrbitGLWidget(QWidget* parent) : QOpenGLWidget(parent) {
  m_OrbitPanel = nullptr;
  m_DebugLogger = nullptr;
  setFocusPolicy(Qt::WheelFocus);
  setMouseTracking(true);
  setUpdateBehavior(QOpenGLWidget::PartialUpdate);
  installEventFilter(this);
}

//-----------------------------------------------------------------------------
bool OrbitGLWidget::eventFilter(QObject* /*object*/, QEvent* event) {
  if (event->type() == QEvent::Paint) {
    if (m_OrbitPanel) {
      m_OrbitPanel->PreRender();
      if (!m_OrbitPanel->GetNeedsRedraw()) {
        return true;
      }
    }
  }
  return false;
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::Initialize(GlPanel::Type a_Type,
                               OrbitMainWindow* a_MainWindow,
                               void* a_UserData) {
  UNUSED(a_Type);
  UNUSED(a_MainWindow);

  m_OrbitPanel = GlPanel::Create(a_Type, a_UserData);

  if (a_MainWindow) {
    a_MainWindow->RegisterGlWidget(this);
  }
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::initializeGL() {
#if ORBIT_DEBUG_OPEN_GL
  m_DebugLogger = new QOpenGLDebugLogger(this);
  if (m_DebugLogger->initialize()) {
    qDebug() << "GL_DEBUG Debug Logger" << m_DebugLogger << "\n";
    connect(m_DebugLogger, SIGNAL(messageLogged(QOpenGLDebugMessage)), this,
            SLOT(messageLogged(QOpenGLDebugMessage)));
    m_DebugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
  }
  format().setOption(QSurfaceFormat::DebugContext);
#endif

  initializeOpenGLFunctions();

  if (m_OrbitPanel) {
    m_OrbitPanel->Initialize();
  }

  PrintContextInformation();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::PrintContextInformation() {
  QString glType;
  QString glVersion;
  QString glProfile;

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

  PRINT_VAR(qPrintable(glType));
  PRINT_VAR(qPrintable(glVersion));
  PRINT_VAR(qPrintable(glProfile));
}

//-----------------------------------------------------------------------------
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
  qDebug() << qPrintable(error) << "\n" << qPrintable(msg.message()) << "\n";
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::resizeGL(int w, int h) {
  if (m_OrbitPanel) {
    m_OrbitPanel->Resize(w, h);

    QPoint localPoint(0, 0);
    QPoint windowPoint = this->mapTo(this->window(), localPoint);
    m_OrbitPanel->SetWindowOffset(windowPoint.x(), windowPoint.y());
    m_OrbitPanel->SetMainWindowSize(this->geometry().width(),
                                    this->geometry().height());
  }
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::paintGL() {
  if (m_OrbitPanel) {
    m_OrbitPanel->Render(width(), height());
  }

  static volatile bool doScreenShot = false;
  if (doScreenShot) {
    TakeScreenShot();
  }
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::TakeScreenShot() {
  QImage img = this->grabFramebuffer();
  QImageWriter writer("screenshot", "jpg");
  writer.write(img);
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::mousePressEvent(QMouseEvent* event) {
  if (m_OrbitPanel) {
    if (event->buttons() == Qt::LeftButton) {
      m_OrbitPanel->LeftDown(event->x(), event->y());
    }

    if (event->buttons() == Qt::RightButton) {
      m_OrbitPanel->RightDown(event->x(), event->y());
    }

    if (event->buttons() == Qt::MiddleButton) {
      m_OrbitPanel->MiddleDown(event->x(), event->y());
    }
  }

  update();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (m_OrbitPanel) {
    if (event->button() == Qt::LeftButton) {
      m_OrbitPanel->LeftUp();
    }

    if (event->button() == Qt::RightButton) {
      if (m_OrbitPanel->RightUp()) {
        showContextMenu();
      }
    }

    if (event->button() == Qt::MiddleButton) {
      m_OrbitPanel->MiddleUp(event->x(), event->y());
    }
  }

  update();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::showContextMenu() {
  std::vector<std::wstring> menu = m_OrbitPanel->GetContextMenu();

  if (menu.size() > 0) {
    QMenu contextMenu(tr("GlContextMenu"), this);
    QSignalMapper signalMapper(this);
    std::vector<QAction*> actions;

    for (int i = 0; i < (int)menu.size(); ++i) {
      actions.push_back(new QAction(QString::fromStdWString(menu[i])));
      connect(actions[i], SIGNAL(triggered()), &signalMapper, SLOT(map()));
      signalMapper.setMapping(actions[i], i);
      contextMenu.addAction(actions[i]);
    }

    connect(&signalMapper, SIGNAL(mapped(int)), this, SLOT(OnMenuClicked(int)));
    contextMenu.exec(QCursor::pos());

    for (QAction* action : actions) delete action;
  }
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::OnMenuClicked(int a_Index) {
  const std::vector<std::wstring>& menu = m_OrbitPanel->GetContextMenu();
  m_OrbitPanel->OnContextMenu(menu[a_Index], a_Index);
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::mouseDoubleClickEvent(QMouseEvent* event) {
  if (event->button() == Qt::LeftButton) {
    m_OrbitPanel->LeftDoubleClick();
  }

  update();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::mouseMoveEvent(QMouseEvent* event) {
  if (m_OrbitPanel) {
    m_OrbitPanel->MouseMoved(event->x(), event->y(),
                             event->buttons() & Qt::LeftButton,
                             event->buttons() & Qt::RightButton,
                             event->buttons() & Qt::MiddleButton);
  }

  update();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::keyPressEvent(QKeyEvent* event) {
  if (m_OrbitPanel) {
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    bool alt = event->modifiers() & Qt::AltModifier;
    m_OrbitPanel->KeyPressed(event->key() & 0x00FFFFFF, ctrl, shift, alt);

    QString text = event->text();
    if (text.size() > 0) {
      m_OrbitPanel->CharEvent(text[0].unicode());
    }
  }

  update();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::keyReleaseEvent(QKeyEvent* event) {
  if (m_OrbitPanel) {
    bool ctrl = event->modifiers() & Qt::ControlModifier;
    bool shift = event->modifiers() & Qt::ShiftModifier;
    bool alt = event->modifiers() & Qt::AltModifier;
    m_OrbitPanel->KeyReleased(event->key() & 0x00FFFFFF, ctrl, shift, alt);
  }

  update();
}

//-----------------------------------------------------------------------------
void OrbitGLWidget::wheelEvent(QWheelEvent* event) {
  if (m_OrbitPanel) {
    m_OrbitPanel->MouseWheelMoved(event->x(), event->y(), event->delta() / 8,
                                  event->modifiers() & Qt::ControlModifier);
  }

  update();
}
