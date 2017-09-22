/********************************************************************************
** Form generated from reading UI file 'outputdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_OUTPUTDIALOG_H
#define UI_OUTPUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QSplitter>

QT_BEGIN_NAMESPACE

class Ui_OutputDialog
{
public:
    QGridLayout *gridLayout;
    QSplitter *splitter;
    QPlainTextEdit *staticTextEdit;
    QPlainTextEdit *OutputTextEdit;

    void setupUi(QDialog *OutputDialog)
    {
        if (OutputDialog->objectName().isEmpty())
            OutputDialog->setObjectName(QStringLiteral("OutputDialog"));
        OutputDialog->resize(400, 300);
        gridLayout = new QGridLayout(OutputDialog);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        splitter = new QSplitter(OutputDialog);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Vertical);
        staticTextEdit = new QPlainTextEdit(splitter);
        staticTextEdit->setObjectName(QStringLiteral("staticTextEdit"));
        splitter->addWidget(staticTextEdit);
        OutputTextEdit = new QPlainTextEdit(splitter);
        OutputTextEdit->setObjectName(QStringLiteral("OutputTextEdit"));
        splitter->addWidget(OutputTextEdit);

        gridLayout->addWidget(splitter, 0, 0, 1, 1);


        retranslateUi(OutputDialog);

        QMetaObject::connectSlotsByName(OutputDialog);
    } // setupUi

    void retranslateUi(QDialog *OutputDialog)
    {
        OutputDialog->setWindowTitle(QApplication::translate("OutputDialog", "Output", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OutputDialog: public Ui_OutputDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_OUTPUTDIALOG_H
