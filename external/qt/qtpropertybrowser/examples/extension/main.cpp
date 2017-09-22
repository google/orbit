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
#include "qtvariantproperty.h"
#include "qteditorfactory.h"
#include "qttreepropertybrowser.h"

class VariantManager : public QtVariantPropertyManager
{
    Q_OBJECT
public:
    VariantManager(QObject *parent = 0);
    ~VariantManager();

    virtual QVariant value(const QtProperty *property) const;
    virtual int valueType(int propertyType) const;
    virtual bool isPropertyTypeSupported(int propertyType) const;

    QString valueText(const QtProperty *property) const;

public slots:
    virtual void setValue(QtProperty *property, const QVariant &val);
protected:
    virtual void initializeProperty(QtProperty *property);
    virtual void uninitializeProperty(QtProperty *property);
private slots:
    void slotValueChanged(QtProperty *property, const QVariant &value);
    void slotPropertyDestroyed(QtProperty *property);
private:
    struct Data {
        QVariant value;
        QtVariantProperty *x;
        QtVariantProperty *y;
    };
    QMap<const QtProperty *, Data> propertyToData;
    QMap<const QtProperty *, QtProperty *> xToProperty;
    QMap<const QtProperty *, QtProperty *> yToProperty;
};

VariantManager::VariantManager(QObject *parent)
    : QtVariantPropertyManager(parent)
{
    connect(this, SIGNAL(valueChanged(QtProperty *, const QVariant &)),
                this, SLOT(slotValueChanged(QtProperty *, const QVariant &)));
    connect(this, SIGNAL(propertyDestroyed(QtProperty *)),
                this, SLOT(slotPropertyDestroyed(QtProperty *)));
}

VariantManager::~VariantManager()
{

}

void VariantManager::slotValueChanged(QtProperty *property, const QVariant &value)
{
    if (xToProperty.contains(property)) {
        QtProperty *pointProperty = xToProperty[property];
        QVariant v = this->value(pointProperty);
        QPointF p = v.value<QPointF>();
        p.setX(value.value<double>());
        setValue(pointProperty, p);
    } else if (yToProperty.contains(property)) {
        QtProperty *pointProperty = yToProperty[property];
        QVariant v = this->value(pointProperty);
        QPointF p = v.value<QPointF>();
        p.setY(value.value<double>());
        setValue(pointProperty, p);
    }
}

void VariantManager::slotPropertyDestroyed(QtProperty *property)
{
    if (xToProperty.contains(property)) {
        QtProperty *pointProperty = xToProperty[property];
        propertyToData[pointProperty].x = 0;
        xToProperty.remove(property);
    } else if (yToProperty.contains(property)) {
        QtProperty *pointProperty = yToProperty[property];
        propertyToData[pointProperty].y = 0;
        yToProperty.remove(property);
    }
}

bool VariantManager::isPropertyTypeSupported(int propertyType) const
{
    if (propertyType == QVariant::PointF)
        return true;
    return QtVariantPropertyManager::isPropertyTypeSupported(propertyType);
}

int VariantManager::valueType(int propertyType) const
{
    if (propertyType == QVariant::PointF)
        return QVariant::PointF;
    return QtVariantPropertyManager::valueType(propertyType);
}

QVariant VariantManager::value(const QtProperty *property) const
{
    if (propertyToData.contains(property))
        return propertyToData[property].value;
    return QtVariantPropertyManager::value(property);
}

QString VariantManager::valueText(const QtProperty *property) const
{
    if (propertyToData.contains(property)) {
        QVariant v = propertyToData[property].value;
        QPointF p = v.value<QPointF>();
        return QString(tr("(%1, %2)").arg(QString::number(p.x()))
                                 .arg(QString::number(p.y())));
    }
    return QtVariantPropertyManager::valueText(property);
}

void VariantManager::setValue(QtProperty *property, const QVariant &val)
{
    if (propertyToData.contains(property)) {
        if (val.type() != QVariant::PointF && !val.canConvert(QVariant::PointF))
            return;
        QPointF p = val.value<QPointF>();
        Data d = propertyToData[property];
        d.value = p;
        if (d.x)
            d.x->setValue(p.x());
        if (d.y)
            d.y->setValue(p.y());
        propertyToData[property] = d;
        emit propertyChanged(property);
        emit valueChanged(property, p);
        return;
    }
    QtVariantPropertyManager::setValue(property, val);
}

void VariantManager::initializeProperty(QtProperty *property)
{
    if (propertyType(property) == QVariant::PointF) {
        Data d;

        d.value = QPointF(0, 0);

        VariantManager *that = (VariantManager *)this;

        d.x = that->addProperty(QVariant::Double);
        d.x->setPropertyName(tr("Position X"));
        property->addSubProperty(d.x);
        xToProperty[d.x] = property;

        d.y = that->addProperty(QVariant::Double);
        d.y->setPropertyName(tr("Position Y"));
        property->addSubProperty(d.y);
        yToProperty[d.y] = property;

        propertyToData[property] = d;
    }
    QtVariantPropertyManager::initializeProperty(property);
}

void VariantManager::uninitializeProperty(QtProperty *property)
{
    if (propertyToData.contains(property)) {
        Data d = propertyToData[property];
        if (d.x)
            xToProperty.remove(d.x);
        if (d.y)
            yToProperty.remove(d.y);
        propertyToData.remove(property);
    }
    QtVariantPropertyManager::uninitializeProperty(property);
}

int main(int argc, char **argv)
{
    QApplication app(argc, argv);

    VariantManager *variantManager = new VariantManager();

    QtVariantProperty *item = variantManager->addProperty(QVariant::PointF,
                "PointF Property");
    item->setValue(QPointF(2.5, 13.13));

    QtVariantEditorFactory *variantFactory = new QtVariantEditorFactory();

    QtTreePropertyBrowser ed1;
    QtVariantPropertyManager *varMan = variantManager;
    ed1.setFactoryForManager(varMan, variantFactory);
    ed1.addProperty(item);


    ed1.show();

    int ret = app.exec();

    delete variantFactory;
    delete variantManager;

    return ret;
}

#include "main.moc"
