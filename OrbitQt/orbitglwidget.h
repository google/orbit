//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <QOpenGLWidget>
#include <QOpenGLFunctions>
#include "../OrbitGl/GlPanel.h"

class QOpenGLDebugMessage;
class QOpenGLDebugLogger;

class OrbitGLWidget : public QOpenGLWidget, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    OrbitGLWidget(QWidget *parent = 0);
    void Initialize( GlPanel::Type a_Type, class OrbitMainWindow* a_MainWindow, void* a_UserData = nullptr );
    void initializeGL();
    void resizeGL(int w, int h);
    void paintGL();
    bool eventFilter( QObject* object, QEvent* event ) override;
    void TakeScreenShot();
    GlPanel* GetPanel() { return m_OrbitPanel; }
    void PrintContextInformation();

    void mousePressEvent        ( QMouseEvent* event );
    void mouseReleaseEvent      ( QMouseEvent* event );
    void mouseDoubleClickEvent  ( QMouseEvent* event );
    void mouseMoveEvent         ( QMouseEvent* event );
    void keyPressEvent          ( QKeyEvent* event );
    void keyReleaseEvent        ( QKeyEvent* event );
    void wheelEvent             ( QWheelEvent* event );

protected slots:
    void messageLogged(const QOpenGLDebugMessage &msg);
    
private:
    GlPanel*            m_OrbitPanel;
    QOpenGLDebugLogger* m_DebugLogger;
};

