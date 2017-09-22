/********************************************************************************
** Form generated from reading UI file 'orbitdiffdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITDIFFDIALOG_H
#define UI_ORBITDIFFDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QWidget>
#include "orbitcodeeditor.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitDiffDialog
{
public:
    QGridLayout *gridLayout_2;
    QWidget *widget;
    QGridLayout *gridLayout_3;
    QSplitter *splitter;
    OrbitCodeEditor *plainTextEdit;
    OrbitCodeEditor *plainTextEdit_2;
    QWidget *widget_2;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *DiffExeLineEdit;
    QPushButton *BrowseButton;
    QLabel *label_2;
    QLineEdit *ArgsLineEdit;
    QDialogButtonBox *buttonBox;

    void setupUi(QDialog *OrbitDiffDialog)
    {
        if (OrbitDiffDialog->objectName().isEmpty())
            OrbitDiffDialog->setObjectName(QStringLiteral("OrbitDiffDialog"));
        OrbitDiffDialog->resize(895, 613);
        gridLayout_2 = new QGridLayout(OrbitDiffDialog);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        widget = new QWidget(OrbitDiffDialog);
        widget->setObjectName(QStringLiteral("widget"));
        gridLayout_3 = new QGridLayout(widget);
        gridLayout_3->setObjectName(QStringLiteral("gridLayout_3"));
        splitter = new QSplitter(widget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        plainTextEdit = new OrbitCodeEditor(splitter);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));
        splitter->addWidget(plainTextEdit);
        plainTextEdit_2 = new OrbitCodeEditor(splitter);
        plainTextEdit_2->setObjectName(QStringLiteral("plainTextEdit_2"));
        splitter->addWidget(plainTextEdit_2);

        gridLayout_3->addWidget(splitter, 0, 0, 1, 2);

        widget_2 = new QWidget(widget);
        widget_2->setObjectName(QStringLiteral("widget_2"));
        gridLayout = new QGridLayout(widget_2);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        label = new QLabel(widget_2);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);

        DiffExeLineEdit = new QLineEdit(widget_2);
        DiffExeLineEdit->setObjectName(QStringLiteral("DiffExeLineEdit"));

        gridLayout->addWidget(DiffExeLineEdit, 0, 1, 1, 1);

        BrowseButton = new QPushButton(widget_2);
        BrowseButton->setObjectName(QStringLiteral("BrowseButton"));

        gridLayout->addWidget(BrowseButton, 0, 2, 1, 1);

        label_2 = new QLabel(widget_2);
        label_2->setObjectName(QStringLiteral("label_2"));

        gridLayout->addWidget(label_2, 1, 0, 1, 1);

        ArgsLineEdit = new QLineEdit(widget_2);
        ArgsLineEdit->setObjectName(QStringLiteral("ArgsLineEdit"));

        gridLayout->addWidget(ArgsLineEdit, 1, 1, 1, 1);

        ArgsLineEdit->raise();
        DiffExeLineEdit->raise();
        label->raise();
        label_2->raise();
        BrowseButton->raise();

        gridLayout_3->addWidget(widget_2, 1, 0, 1, 1);

        buttonBox = new QDialogButtonBox(widget);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        buttonBox->setCenterButtons(false);

        gridLayout_3->addWidget(buttonBox, 1, 1, 1, 1);


        gridLayout_2->addWidget(widget, 0, 0, 1, 1);


        retranslateUi(OrbitDiffDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), OrbitDiffDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), OrbitDiffDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(OrbitDiffDialog);
    } // setupUi

    void retranslateUi(QDialog *OrbitDiffDialog)
    {
        OrbitDiffDialog->setWindowTitle(QApplication::translate("OrbitDiffDialog", "Dialog", Q_NULLPTR));
        label->setText(QApplication::translate("OrbitDiffDialog", "Diff Exe", Q_NULLPTR));
        BrowseButton->setText(QApplication::translate("OrbitDiffDialog", "Browse...", Q_NULLPTR));
        label_2->setText(QApplication::translate("OrbitDiffDialog", "Args", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitDiffDialog: public Ui_OrbitDiffDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITDIFFDIALOG_H
