/********************************************************************************
** Form generated from reading UI file 'processlauncherwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_PROCESSLAUNCHERWIDGET_H
#define UI_PROCESSLAUNCHERWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QCheckBox>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QWidget>
#include "orbitdataviewpanel.h"

QT_BEGIN_NAMESPACE

class Ui_ProcessLauncherWidget
{
public:
    QGridLayout *gridLayout;
    QSplitter *splitter;
    OrbitDataViewPanel *LiveProcessList;
    QFrame *frame;
    QGridLayout *gridLayout_2;
    QPushButton *LaunchButton;
    QComboBox *ArgumentsComboBox;
    QComboBox *ProcessComboBox;
    QPushButton *BrowseButton;
    QCheckBox *checkBoxPause;

    void setupUi(QWidget *ProcessLauncherWidget)
    {
        if (ProcessLauncherWidget->objectName().isEmpty())
            ProcessLauncherWidget->setObjectName(QStringLiteral("ProcessLauncherWidget"));
        ProcessLauncherWidget->resize(550, 416);
        gridLayout = new QGridLayout(ProcessLauncherWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        splitter = new QSplitter(ProcessLauncherWidget);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Vertical);
        LiveProcessList = new OrbitDataViewPanel(splitter);
        LiveProcessList->setObjectName(QStringLiteral("LiveProcessList"));
        splitter->addWidget(LiveProcessList);
        frame = new QFrame(splitter);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::StyledPanel);
        frame->setFrameShadow(QFrame::Raised);
        gridLayout_2 = new QGridLayout(frame);
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        LaunchButton = new QPushButton(frame);
        LaunchButton->setObjectName(QStringLiteral("LaunchButton"));

        gridLayout_2->addWidget(LaunchButton, 1, 1, 1, 1);

        ArgumentsComboBox = new QComboBox(frame);
        ArgumentsComboBox->setObjectName(QStringLiteral("ArgumentsComboBox"));
        ArgumentsComboBox->setEditable(true);

        gridLayout_2->addWidget(ArgumentsComboBox, 1, 0, 1, 1);

        ProcessComboBox = new QComboBox(frame);
        ProcessComboBox->setObjectName(QStringLiteral("ProcessComboBox"));
        ProcessComboBox->setEditable(true);

        gridLayout_2->addWidget(ProcessComboBox, 0, 0, 1, 1);

        BrowseButton = new QPushButton(frame);
        BrowseButton->setObjectName(QStringLiteral("BrowseButton"));

        gridLayout_2->addWidget(BrowseButton, 0, 1, 1, 1);

        checkBoxPause = new QCheckBox(frame);
        checkBoxPause->setObjectName(QStringLiteral("checkBoxPause"));

        gridLayout_2->addWidget(checkBoxPause, 2, 0, 1, 1);

        splitter->addWidget(frame);

        gridLayout->addWidget(splitter, 0, 0, 1, 1);


        retranslateUi(ProcessLauncherWidget);

        QMetaObject::connectSlotsByName(ProcessLauncherWidget);
    } // setupUi

    void retranslateUi(QWidget *ProcessLauncherWidget)
    {
        ProcessLauncherWidget->setWindowTitle(QApplication::translate("ProcessLauncherWidget", "Form", Q_NULLPTR));
        LaunchButton->setText(QApplication::translate("ProcessLauncherWidget", "Launch", Q_NULLPTR));
        BrowseButton->setText(QApplication::translate("ProcessLauncherWidget", "...", Q_NULLPTR));
#ifndef QT_NO_TOOLTIP
        checkBoxPause->setToolTip(QApplication::translate("ProcessLauncherWidget", "Resume exectution by starting a capture", Q_NULLPTR));
#endif // QT_NO_TOOLTIP
        checkBoxPause->setText(QApplication::translate("ProcessLauncherWidget", "Pause at Entry Point", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class ProcessLauncherWidget: public Ui_ProcessLauncherWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_PROCESSLAUNCHERWIDGET_H
