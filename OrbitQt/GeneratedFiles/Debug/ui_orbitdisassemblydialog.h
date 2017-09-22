/********************************************************************************
** Form generated from reading UI file 'orbitdisassemblydialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITDISASSEMBLYDIALOG_H
#define UI_ORBITDISASSEMBLYDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include "orbitcodeeditor.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitDisassemblyDialog
{
public:
    QGridLayout *gridLayout;
    OrbitCodeEditor *plainTextEdit;

    void setupUi(QDialog *OrbitDisassemblyDialog)
    {
        if (OrbitDisassemblyDialog->objectName().isEmpty())
            OrbitDisassemblyDialog->setObjectName(QStringLiteral("OrbitDisassemblyDialog"));
        OrbitDisassemblyDialog->resize(574, 493);
        gridLayout = new QGridLayout(OrbitDisassemblyDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        plainTextEdit = new OrbitCodeEditor(OrbitDisassemblyDialog);
        plainTextEdit->setObjectName(QStringLiteral("plainTextEdit"));

        gridLayout->addWidget(plainTextEdit, 0, 0, 1, 1);


        retranslateUi(OrbitDisassemblyDialog);

        QMetaObject::connectSlotsByName(OrbitDisassemblyDialog);
    } // setupUi

    void retranslateUi(QDialog *OrbitDisassemblyDialog)
    {
        OrbitDisassemblyDialog->setWindowTitle(QApplication::translate("OrbitDisassemblyDialog", "Dialog", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitDisassemblyDialog: public Ui_OrbitDisassemblyDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITDISASSEMBLYDIALOG_H
