/********************************************************************************
** Form generated from reading UI file 'licensedialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_LICENSEDIALOG_H
#define UI_LICENSEDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QDialogButtonBox>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPlainTextEdit>

QT_BEGIN_NAMESPACE

class Ui_LicenseDialog
{
public:
    QGridLayout *gridLayout;
    QDialogButtonBox *buttonBox;
    QPlainTextEdit *LicenseTextEdit;
    QLabel *label;

    void setupUi(QDialog *LicenseDialog)
    {
        if (LicenseDialog->objectName().isEmpty())
            LicenseDialog->setObjectName(QStringLiteral("LicenseDialog"));
        LicenseDialog->resize(400, 300);
        gridLayout = new QGridLayout(LicenseDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        buttonBox = new QDialogButtonBox(LicenseDialog);
        buttonBox->setObjectName(QStringLiteral("buttonBox"));
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);

        gridLayout->addWidget(buttonBox, 2, 0, 1, 1);

        LicenseTextEdit = new QPlainTextEdit(LicenseDialog);
        LicenseTextEdit->setObjectName(QStringLiteral("LicenseTextEdit"));

        gridLayout->addWidget(LicenseTextEdit, 1, 0, 1, 1);

        label = new QLabel(LicenseDialog);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 0, 1, 1);


        retranslateUi(LicenseDialog);
        QObject::connect(buttonBox, SIGNAL(accepted()), LicenseDialog, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), LicenseDialog, SLOT(reject()));

        QMetaObject::connectSlotsByName(LicenseDialog);
    } // setupUi

    void retranslateUi(QDialog *LicenseDialog)
    {
        LicenseDialog->setWindowTitle(QApplication::translate("LicenseDialog", "Dialog", Q_NULLPTR));
        label->setText(QApplication::translate("LicenseDialog", "Welcome to Orbit.  Please enter a valid license.", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class LicenseDialog: public Ui_LicenseDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_LICENSEDIALOG_H
