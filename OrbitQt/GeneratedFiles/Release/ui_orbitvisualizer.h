/********************************************************************************
** Form generated from reading UI file 'orbitvisualizer.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITVISUALIZER_H
#define UI_ORBITVISUALIZER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QWidget>
#include "orbitglwidget.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitVisualizer
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QSplitter *splitter;
    OrbitGLWidget *RuleEditor;
    OrbitGLWidget *Visualizer;
    QMenuBar *menubar;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *OrbitVisualizer)
    {
        if (OrbitVisualizer->objectName().isEmpty())
            OrbitVisualizer->setObjectName(QStringLiteral("OrbitVisualizer"));
        OrbitVisualizer->resize(800, 600);
        centralwidget = new QWidget(OrbitVisualizer);
        centralwidget->setObjectName(QStringLiteral("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        RuleEditor = new OrbitGLWidget(splitter);
        RuleEditor->setObjectName(QStringLiteral("RuleEditor"));
        splitter->addWidget(RuleEditor);
        Visualizer = new OrbitGLWidget(splitter);
        Visualizer->setObjectName(QStringLiteral("Visualizer"));
        splitter->addWidget(Visualizer);

        gridLayout->addWidget(splitter, 0, 0, 1, 1);

        OrbitVisualizer->setCentralWidget(centralwidget);
        menubar = new QMenuBar(OrbitVisualizer);
        menubar->setObjectName(QStringLiteral("menubar"));
        menubar->setGeometry(QRect(0, 0, 800, 21));
        OrbitVisualizer->setMenuBar(menubar);
        statusbar = new QStatusBar(OrbitVisualizer);
        statusbar->setObjectName(QStringLiteral("statusbar"));
        OrbitVisualizer->setStatusBar(statusbar);

        retranslateUi(OrbitVisualizer);

        QMetaObject::connectSlotsByName(OrbitVisualizer);
    } // setupUi

    void retranslateUi(QMainWindow *OrbitVisualizer)
    {
        OrbitVisualizer->setWindowTitle(QApplication::translate("OrbitVisualizer", "Rule Editor", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitVisualizer: public Ui_OrbitVisualizer {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITVISUALIZER_H
