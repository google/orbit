/********************************************************************************
** Form generated from reading UI file 'orbitsamplingreport.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITSAMPLINGREPORT_H
#define UI_ORBITSAMPLINGREPORT_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QFrame>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHBoxLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>
#include "orbitdataviewpanel.h"

QT_BEGIN_NAMESPACE

class Ui_OrbitSamplingReport
{
public:
    QGridLayout *gridLayout;
    QSplitter *splitter;
    QTabWidget *tabWidget;
    QFrame *frame;
    QVBoxLayout *verticalLayout;
    QGridLayout *gridLayout_2;
    QFrame *CallstackLabelFrame;
    QHBoxLayout *horizontalLayout;
    QPushButton *PreviousCallstackButton;
    QPushButton *NextCallstackButton;
    QLabel *CallStackLabel;
    OrbitDataViewPanel *CallstackTreeView;

    void setupUi(QWidget *OrbitSamplingReport)
    {
        if (OrbitSamplingReport->objectName().isEmpty())
            OrbitSamplingReport->setObjectName(QStringLiteral("OrbitSamplingReport"));
        OrbitSamplingReport->resize(689, 579);
        QSizePolicy sizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
        sizePolicy.setHorizontalStretch(6);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(OrbitSamplingReport->sizePolicy().hasHeightForWidth());
        OrbitSamplingReport->setSizePolicy(sizePolicy);
        gridLayout = new QGridLayout(OrbitSamplingReport);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        splitter = new QSplitter(OrbitSamplingReport);
        splitter->setObjectName(QStringLiteral("splitter"));
        splitter->setOrientation(Qt::Vertical);
        tabWidget = new QTabWidget(splitter);
        tabWidget->setObjectName(QStringLiteral("tabWidget"));
        QSizePolicy sizePolicy1(QSizePolicy::Expanding, QSizePolicy::Maximum);
        sizePolicy1.setHorizontalStretch(0);
        sizePolicy1.setVerticalStretch(0);
        sizePolicy1.setHeightForWidth(tabWidget->sizePolicy().hasHeightForWidth());
        tabWidget->setSizePolicy(sizePolicy1);
        splitter->addWidget(tabWidget);
        frame = new QFrame(splitter);
        frame->setObjectName(QStringLiteral("frame"));
        frame->setFrameShape(QFrame::NoFrame);
        frame->setFrameShadow(QFrame::Raised);
        verticalLayout = new QVBoxLayout(frame);
        verticalLayout->setObjectName(QStringLiteral("verticalLayout"));
        verticalLayout->setContentsMargins(0, 0, 0, -1);
        gridLayout_2 = new QGridLayout();
        gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
        CallstackLabelFrame = new QFrame(frame);
        CallstackLabelFrame->setObjectName(QStringLiteral("CallstackLabelFrame"));
        QSizePolicy sizePolicy2(QSizePolicy::Expanding, QSizePolicy::Fixed);
        sizePolicy2.setHorizontalStretch(0);
        sizePolicy2.setVerticalStretch(0);
        sizePolicy2.setHeightForWidth(CallstackLabelFrame->sizePolicy().hasHeightForWidth());
        CallstackLabelFrame->setSizePolicy(sizePolicy2);
        CallstackLabelFrame->setMaximumSize(QSize(16777215, 100));
        CallstackLabelFrame->setFrameShape(QFrame::StyledPanel);
        CallstackLabelFrame->setFrameShadow(QFrame::Raised);
        horizontalLayout = new QHBoxLayout(CallstackLabelFrame);
        horizontalLayout->setObjectName(QStringLiteral("horizontalLayout"));
        PreviousCallstackButton = new QPushButton(CallstackLabelFrame);
        PreviousCallstackButton->setObjectName(QStringLiteral("PreviousCallstackButton"));
        QSizePolicy sizePolicy3(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy3.setHorizontalStretch(0);
        sizePolicy3.setVerticalStretch(0);
        sizePolicy3.setHeightForWidth(PreviousCallstackButton->sizePolicy().hasHeightForWidth());
        PreviousCallstackButton->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(PreviousCallstackButton);

        NextCallstackButton = new QPushButton(CallstackLabelFrame);
        NextCallstackButton->setObjectName(QStringLiteral("NextCallstackButton"));
        sizePolicy3.setHeightForWidth(NextCallstackButton->sizePolicy().hasHeightForWidth());
        NextCallstackButton->setSizePolicy(sizePolicy3);

        horizontalLayout->addWidget(NextCallstackButton);

        CallStackLabel = new QLabel(CallstackLabelFrame);
        CallStackLabel->setObjectName(QStringLiteral("CallStackLabel"));

        horizontalLayout->addWidget(CallStackLabel);


        gridLayout_2->addWidget(CallstackLabelFrame, 0, 0, 1, 1);

        CallstackTreeView = new OrbitDataViewPanel(frame);
        CallstackTreeView->setObjectName(QStringLiteral("CallstackTreeView"));
        QSizePolicy sizePolicy4(QSizePolicy::Expanding, QSizePolicy::Expanding);
        sizePolicy4.setHorizontalStretch(0);
        sizePolicy4.setVerticalStretch(0);
        sizePolicy4.setHeightForWidth(CallstackTreeView->sizePolicy().hasHeightForWidth());
        CallstackTreeView->setSizePolicy(sizePolicy4);

        gridLayout_2->addWidget(CallstackTreeView, 1, 0, 1, 1);


        verticalLayout->addLayout(gridLayout_2);

        splitter->addWidget(frame);

        gridLayout->addWidget(splitter, 0, 1, 1, 1);


        retranslateUi(OrbitSamplingReport);

        tabWidget->setCurrentIndex(-1);


        QMetaObject::connectSlotsByName(OrbitSamplingReport);
    } // setupUi

    void retranslateUi(QWidget *OrbitSamplingReport)
    {
        OrbitSamplingReport->setWindowTitle(QApplication::translate("OrbitSamplingReport", "Form", Q_NULLPTR));
        PreviousCallstackButton->setText(QApplication::translate("OrbitSamplingReport", "<", Q_NULLPTR));
        NextCallstackButton->setText(QApplication::translate("OrbitSamplingReport", ">", Q_NULLPTR));
        CallStackLabel->setText(QApplication::translate("OrbitSamplingReport", "CallStack", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitSamplingReport: public Ui_OrbitSamplingReport {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITSAMPLINGREPORT_H
