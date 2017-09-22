/********************************************************************************
** Form generated from reading UI file 'orbitdataviewpanel.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITDATAVIEWPANEL_H
#define UI_ORBITDATAVIEWPANEL_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QWidget>
#include "orbittreeview.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitDataViewPanel
{
public:
    QGridLayout *gridLayout;
    QHBoxLayout *horizontalLayout;
    QLabel *label;
    QLineEdit *FilterLineEdit;
    OrbitTreeView *treeView;

    void setupUi(QWidget *OrbitDataViewPanel)
    {
        if (OrbitDataViewPanel->objectName().isEmpty())
            OrbitDataViewPanel->setObjectName(QStringLiteral("OrbitDataViewPanel"));
        OrbitDataViewPanel->resize(690, 484);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::MinimumExpanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OrbitDataViewPanel->sizePolicy().hasHeightForWidth());
        OrbitDataViewPanel->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(OrbitDataViewPanel);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        horizontalLayout = new QHBoxLayout();
        horizontalLayout->setSpacing(6);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        label = new QLabel(OrbitDataViewPanel);
        label->setObjectName(QStringLiteral("label"));

        horizontalLayout->addWidget(label);

        FilterLineEdit = new QLineEdit(OrbitDataViewPanel);
        FilterLineEdit->setObjectName(QStringLiteral("FilterLineEdit"));

        horizontalLayout->addWidget(FilterLineEdit);


        gridLayout->addLayout(horizontalLayout, 0, 0, 1, 1);

        treeView = new OrbitTreeView(OrbitDataViewPanel);
        treeView->setObjectName(QStringLiteral("treeView"));
        treeView->header()->setDefaultSectionSize(0);
        treeView->header()->setMinimumSectionSize(0);

        gridLayout->addWidget(treeView, 1, 0, 1, 1);


        retranslateUi(OrbitDataViewPanel);

        QMetaObject::connectSlotsByName(OrbitDataViewPanel);
    } // setupUi

    void retranslateUi(QWidget *OrbitDataViewPanel)
    {
        OrbitDataViewPanel->setWindowTitle(QApplication::translate("OrbitDataViewPanel", "Form", Q_NULLPTR));
        label->setText(QApplication::translate("OrbitDataViewPanel", "TextLabel", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitDataViewPanel: public Ui_OrbitDataViewPanel {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITDATAVIEWPANEL_H
