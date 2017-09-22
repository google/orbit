/********************************************************************************
** Form generated from reading UI file 'orbitglwidgetwithheader.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITGLWIDGETWITHHEADER_H
#define UI_ORBITGLWIDGETWITHHEADER_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QScrollArea>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "orbitglwidget.h"
#include "orbittreeview.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitGlWidgetWithHeader
{
public:
    QGridLayout *gridLayout_2;
    QVBoxLayout *verticalLayout;
    OrbitTreeView *treeView;
    QScrollArea *scrollArea;
    QWidget *scrollAreaWidgetContents;
    QGridLayout *gridLayout;
    OrbitGLWidget *openGLWidget;

    void setupUi(QWidget *OrbitGlWidgetWithHeader)
    {
        if (OrbitGlWidgetWithHeader->objectName().isEmpty())
            OrbitGlWidgetWithHeader->setObjectName(QStringLiteral("OrbitGlWidgetWithHeader"));
        OrbitGlWidgetWithHeader->resize(663, 594);
        gridLayout_2 = new QGridLayout(OrbitGlWidgetWithHeader);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setSpacing(0);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        treeView = new OrbitTreeView(OrbitGlWidgetWithHeader);
        treeView->setObjectName(QStringLiteral("treeView"));
        treeView->setMaximumSize(QSize(16777215, 20));

        verticalLayout->addWidget(treeView);

        scrollArea = new QScrollArea(OrbitGlWidgetWithHeader);
        scrollArea->setObjectName(QStringLiteral("scrollArea"));
        scrollArea->setWidgetResizable(true);
        scrollAreaWidgetContents = new QWidget();
        scrollAreaWidgetContents->setObjectName(QStringLiteral("scrollAreaWidgetContents"));
        scrollAreaWidgetContents->setGeometry(QRect(0, 0, 641, 552));
        gridLayout = new QGridLayout(scrollAreaWidgetContents);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        openGLWidget = new OrbitGLWidget(scrollAreaWidgetContents);
        openGLWidget->setObjectName(QStringLiteral("openGLWidget"));
        openGLWidget->setMinimumSize(QSize(0, 300));

        gridLayout->addWidget(openGLWidget, 0, 0, 1, 1);

        scrollArea->setWidget(scrollAreaWidgetContents);

        verticalLayout->addWidget(scrollArea);


        gridLayout_2->addLayout(verticalLayout, 0, 0, 1, 1);

        treeView->raise();
        scrollArea->raise();
        openGLWidget->raise();
        openGLWidget->raise();

        retranslateUi(OrbitGlWidgetWithHeader);

        QMetaObject::connectSlotsByName(OrbitGlWidgetWithHeader);
    } // setupUi

    void retranslateUi(QWidget *OrbitGlWidgetWithHeader)
    {
        OrbitGlWidgetWithHeader->setWindowTitle(QApplication::translate("OrbitGlWidgetWithHeader", "Form", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitGlWidgetWithHeader: public Ui_OrbitGlWidgetWithHeader {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITGLWIDGETWITHHEADER_H
