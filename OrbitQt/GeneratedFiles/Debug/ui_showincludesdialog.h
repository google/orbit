/********************************************************************************
** Form generated from reading UI file 'showincludesdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_SHOWINCLUDESDIALOG_H
#define UI_SHOWINCLUDESDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTreeView>
#include <QtWidgets/QWidget>
#include "orbitcodeeditor.h"

QT_BEGIN_NAMESPACE

class Ui_ShowIncludesDialog
{
public:
    QGridLayout *gridLayout_3;
    QSplitter *splitter;
    QWidget *widget;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *lineEdit;
    OrbitCodeEditor *plainTextEdit;
    QWidget *widget_2;
    QGridLayout *gridLayout_2;
    QLabel *label_2;
    QPushButton *pushButton_2;
    QPushButton *pushButton;
    QTreeView *treeView;
    QLineEdit *lineEdit_2;

    void setupUi(QDialog *ShowIncludesDialog)
    {
        if (ShowIncludesDialog->objectName().isEmpty())
            ShowIncludesDialog->setObjectName(QStringLiteral("ShowIncludesDialog"));
        ShowIncludesDialog->resize(1030, 500);
        gridLayout_3 = new QGridLayout(ShowIncludesDialog);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        splitter = new QSplitter(ShowIncludesDialog);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        widget = new QWidget(splitter);
        widget->setObjectName(QStringLiteral("widget"));
        gridLayout = new QGridLayout(widget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(widget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        lineEdit = new QLineEdit(widget);
        lineEdit->setObjectName(QStringLiteral("lineEdit"));

        gridLayout->addWidget(lineEdit, 0, 1, 1, 1);

        plainTextEdit = new OrbitCodeEditor(widget);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));

        gridLayout->addWidget(plainTextEdit, 1, 0, 1, 2);

        splitter->addWidget(widget);
        widget_2 = new QWidget(splitter);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        gridLayout_2 = new QGridLayout(widget_2);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        label_2 = new QLabel(widget_2);
        label_2->setObjectName(QStringLiteral("label_2"));
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTrailing|Qt::AlignVCenter);

        gridLayout_2->addWidget(label_2, 0, 2, 1, 1);

        pushButton_2 = new QPushButton(widget_2);
        pushButton_2->setObjectName(QStringLiteral("pushButton_2"));

        gridLayout_2->addWidget(pushButton_2, 0, 1, 1, 1);

        pushButton = new QPushButton(widget_2);
        pushButton->setObjectName(QStringLiteral("pushButton"));

        gridLayout_2->addWidget(pushButton, 0, 0, 1, 1);

        treeView = new QTreeView(widget_2);
        treeView->setObjectName(QStringLiteral("treeView"));

        gridLayout_2->addWidget(treeView, 1, 0, 1, 5);

        lineEdit_2 = new QLineEdit(widget_2);
        lineEdit_2->setObjectName(QStringLiteral("lineEdit_2"));

        gridLayout_2->addWidget(lineEdit_2, 0, 3, 1, 2);

        splitter->addWidget(widget_2);

        gridLayout_3->addWidget(splitter, 0, 0, 1, 1);


        retranslateUi(ShowIncludesDialog);

        QMetaObject::connectSlotsByName(ShowIncludesDialog);
    } // setupUi

    void retranslateUi(QDialog *ShowIncludesDialog)
    {
        ShowIncludesDialog->setWindowTitle(QApplication::translate("ShowIncludesDialog", "Dialog", Q_NULLPTR));
        label->setText(QApplication::translate("ShowIncludesDialog", "Include Filter", Q_NULLPTR));
        lineEdit->setText(QApplication::translate("ShowIncludesDialog", "Note: including file:", Q_NULLPTR));
        label_2->setText(QApplication::translate("ShowIncludesDialog", "Find", Q_NULLPTR));
        pushButton_2->setText(QApplication::translate("ShowIncludesDialog", "Collapse All", Q_NULLPTR));
        pushButton->setText(QApplication::translate("ShowIncludesDialog", "Expand All", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ShowIncludesDialog: public Ui_ShowIncludesDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_SHOWINCLUDESDIALOG_H
