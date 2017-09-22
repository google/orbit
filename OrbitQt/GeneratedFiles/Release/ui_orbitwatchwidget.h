/********************************************************************************
** Form generated from reading UI file 'orbitwatchwidget.ui'
**
** Created by: Qt User Interface Compiler version 5.8.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ORBITWATCHWIDGET_H
#define UI_ORBITWATCHWIDGET_H

#include <QtCore/QVariant>
#include <QtWidgets/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QButtonGroup>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpacerItem>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_OrbitWatchWidget
{
public:
    QGridLayout *gridLayout;
    QWidget *PropertyGridWidget;
    QLabel *label;
    QLineEdit *FindLineEdit;
    QPushButton *RefreshButton;
    QSpacerItem *horizontalSpacer;
    QPushButton *ClearButton;

    void setupUi(QWidget *OrbitWatchWidget)
    {
        if (OrbitWatchWidget->objectName().isEmpty())
            OrbitWatchWidget->setObjectName(QStringLiteral("OrbitWatchWidget"));
        OrbitWatchWidget->resize(528, 510);
        gridLayout = new QGridLayout(OrbitWatchWidget);
        gridLayout->setObjectName(QStringLiteral("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        PropertyGridWidget = new QWidget(OrbitWatchWidget);
        PropertyGridWidget->setObjectName(QStringLiteral("PropertyGridWidget"));

        gridLayout->addWidget(PropertyGridWidget, 1, 0, 1, 5);

        label = new QLabel(OrbitWatchWidget);
        label->setObjectName(QStringLiteral("label"));

        gridLayout->addWidget(label, 0, 3, 1, 1);

        FindLineEdit = new QLineEdit(OrbitWatchWidget);
        FindLineEdit->setObjectName(QStringLiteral("FindLineEdit"));

        gridLayout->addWidget(FindLineEdit, 0, 4, 1, 1);

        RefreshButton = new QPushButton(OrbitWatchWidget);
        RefreshButton->setObjectName(QStringLiteral("RefreshButton"));

        gridLayout->addWidget(RefreshButton, 0, 0, 1, 1);

        horizontalSpacer = new QSpacerItem(205, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

        gridLayout->addItem(horizontalSpacer, 0, 2, 1, 1);

        ClearButton = new QPushButton(OrbitWatchWidget);
        ClearButton->setObjectName(QStringLiteral("ClearButton"));

        gridLayout->addWidget(ClearButton, 0, 1, 1, 1);


        retranslateUi(OrbitWatchWidget);

        QMetaObject::connectSlotsByName(OrbitWatchWidget);
    } // setupUi

    void retranslateUi(QWidget *OrbitWatchWidget)
    {
        OrbitWatchWidget->setWindowTitle(QApplication::translate("OrbitWatchWidget", "Form", Q_NULLPTR));
        label->setText(QApplication::translate("OrbitWatchWidget", "Find", Q_NULLPTR));
        RefreshButton->setText(QApplication::translate("OrbitWatchWidget", "Refresh", Q_NULLPTR));
        ClearButton->setText(QApplication::translate("OrbitWatchWidget", "Clear", Q_NULLPTR));
    } // retranslateUi

};

namespace Ui {
    class OrbitWatchWidget: public Ui_OrbitWatchWidget {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ORBITWATCHWIDGET_H
