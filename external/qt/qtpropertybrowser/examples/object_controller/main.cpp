/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the Qt Solutions component.
**
** $QT_BEGIN_LICENSE:BSD$
** You may use this file under the terms of the BSD license as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of Digia Plc and its Subsidiary(-ies) nor the names
**     of its contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QApplication>
#include <QSpinBox>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <QDialog>
#include <QComboBox>
#include <QToolButton>
#include <QPushButton>
#include <QBoxLayout>
#include <QTreeWidget>
#include <QAction>
#include <QDesktopWidget>
#include <QTextDocument>
#include <QCalendarWidget>
#include <QTimeLine>
#include "objectcontroller.h"

class MyController : public QDialog
{
    Q_OBJECT
public:
    MyController(QWidget *parent = 0);
    ~MyController();
private slots:
    void createAndControl();
private:
    QComboBox *theClassCombo;
    ObjectController *theController;
    QStringList theClassNames;
    QObject *theControlledObject;
};

MyController::MyController(QWidget *parent)
    : QDialog(parent), theControlledObject(0)
{
    theClassCombo = new QComboBox(this);
    QToolButton *button = new QToolButton(this);
    theController = new ObjectController(this);
    QDialogButtonBox *buttonBox = new QDialogButtonBox(this);

    connect(button, SIGNAL(clicked()), this, SLOT(createAndControl()));
    connect(buttonBox, SIGNAL(rejected()), this, SLOT(reject()));

    button->setText(tr("Create And Control"));
    buttonBox->setStandardButtons(QDialogButtonBox::Close);

    QVBoxLayout *layout = new QVBoxLayout(this);
    QHBoxLayout *internalLayout = new QHBoxLayout();
    internalLayout->addWidget(theClassCombo);
    internalLayout->addWidget(button);
    layout->addLayout(internalLayout);
    layout->addWidget(theController);
    layout->addWidget(buttonBox);

    theClassNames.append(QLatin1String("QWidget"));
    theClassNames.append(QLatin1String("QPushButton"));
    theClassNames.append(QLatin1String("QDialogButtonBox"));
    theClassNames.append(QLatin1String("QTreeWidget"));
    theClassNames.append(QLatin1String("QCalendarWidget"));
    theClassNames.append(QLatin1String("QAction"));
    theClassNames.append(QLatin1String("QTimeLine"));
    theClassNames.append(QLatin1String("QTextDocument"));

    theClassCombo->addItems(theClassNames);
}

MyController::~MyController()
{
    if (theControlledObject)
        delete theControlledObject;
}

void MyController::createAndControl()
{
    QObject *newObject = 0;
    QString className = theClassNames.at(theClassCombo->currentIndex());
    if (className == QLatin1String("QWidget"))
        newObject = new QWidget();
    else if (className == QLatin1String("QPushButton"))
        newObject = new QPushButton();
    else if (className == QLatin1String("QDialogButtonBox"))
        newObject = new QDialogButtonBox();
    else if (className == QLatin1String("QTreeWidget"))
        newObject = new QTreeWidget();
    else if (className == QLatin1String("QCalendarWidget"))
        newObject = new QCalendarWidget();
    else if (className == QLatin1String("QAction"))
        newObject = new QAction(0);
    else if (className == QLatin1String("QTimeLine"))
        newObject = new QTimeLine();
    else if (className == QLatin1String("QTextDocument"))
        newObject = new QTextDocument();

    if (!newObject)
        return;

    QWidget *newWidget = qobject_cast<QWidget *>(newObject);
    if (newWidget) {
        QRect r = newWidget->geometry();
        r.setSize(newWidget->sizeHint());
        r.setWidth(qMax(r.width(), 150));
        r.setHeight(qMax(r.height(), 50));
        r.moveCenter(QApplication::desktop()->geometry().center());
        newWidget->setGeometry(r);
        newWidget->setWindowTitle(tr("Controlled Object: %1").arg(className));
        newWidget->show();
    }

    if (theControlledObject)
        delete theControlledObject;

    theControlledObject = newObject;
    theController->setObject(theControlledObject);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    MyController *controller = new MyController();
    controller->show();

    int ret = app.exec();

    return ret;
}

#include "main.moc"
