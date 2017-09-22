//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "orbitglwidgetwithheader.h"
#include "ui_orbitglwidgetwithheader.h"

//-----------------------------------------------------------------------------
OrbitGlWidgetWithHeader::OrbitGlWidgetWithHeader(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::OrbitGlWidgetWithHeader)
{
    ui->setupUi(this);
    ui->gridLayout->setSpacing(0);
    ui->gridLayout_2->setSpacing(0);
    ui->gridLayout->setMargin(0);
    ui->gridLayout_2->setMargin(0);
}

//-----------------------------------------------------------------------------
OrbitGlWidgetWithHeader::~OrbitGlWidgetWithHeader()
{
    delete ui;
}

//-----------------------------------------------------------------------------
OrbitTreeView* OrbitGlWidgetWithHeader::GetTreeView()
{
    return ui->treeView;
}

//-----------------------------------------------------------------------------
OrbitGLWidget* OrbitGlWidgetWithHeader::GetGLWidget()
{
    return ui->openGLWidget;
}
