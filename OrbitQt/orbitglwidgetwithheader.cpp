// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitglwidgetwithheader.h"

// This needs to be first because if it is not GL/glew.h
// complains about being included after gl.h
// clang-format off
#include "OpenGl.h"
// clang-format on
#include "ui_orbitglwidgetwithheader.h"

//-----------------------------------------------------------------------------
OrbitGlWidgetWithHeader::OrbitGlWidgetWithHeader(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitGlWidgetWithHeader) {
  ui->setupUi(this);
  ui->gridLayout->setSpacing(0);
  ui->gridLayout_2->setSpacing(0);
  ui->gridLayout->setMargin(0);
  ui->gridLayout_2->setMargin(0);
}

//-----------------------------------------------------------------------------
OrbitGlWidgetWithHeader::~OrbitGlWidgetWithHeader() { delete ui; }

//-----------------------------------------------------------------------------
OrbitTreeView* OrbitGlWidgetWithHeader::GetTreeView() { return ui->treeView; }

//-----------------------------------------------------------------------------
OrbitGLWidget* OrbitGlWidgetWithHeader::GetGLWidget() {
  return ui->openGLWidget;
}
