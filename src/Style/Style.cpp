// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Style/Style.h"

#include <QApplication>
#include <QColor>
#include <QPalette>
#include <QStyleFactory>
#include <Qt>

namespace orbit_style {
void ApplyStyle(QApplication* app) {
  QApplication::setStyle(QStyleFactory::create("Fusion"));

  QPalette dark_palette;
  dark_palette.setColor(QPalette::Window, QColor(53, 53, 53));
  dark_palette.setColor(QPalette::WindowText, Qt::white);
  dark_palette.setColor(QPalette::Base, QColor(25, 25, 25));
  dark_palette.setColor(QPalette::AlternateBase, QColor(53, 53, 53));
  dark_palette.setColor(QPalette::ToolTipBase, Qt::white);
  dark_palette.setColor(QPalette::ToolTipText, Qt::white);
  dark_palette.setColor(QPalette::Text, Qt::white);
  dark_palette.setColor(QPalette::Button, QColor(53, 53, 53));
  dark_palette.setColor(QPalette::ButtonText, Qt::white);
  dark_palette.setColor(QPalette::BrightText, Qt::red);
  dark_palette.setColor(QPalette::Link, QColor(42, 130, 218));
  dark_palette.setColor(QPalette::Highlight, QColor(42, 130, 218));
  dark_palette.setColor(QPalette::HighlightedText, Qt::black);

  QColor light_gray{160, 160, 160};
  QColor dark_gray{90, 90, 90};
  QColor darker_gray{80, 80, 80};
  dark_palette.setColor(QPalette::Disabled, QPalette::Window, dark_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::WindowText, light_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::Base, darker_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::AlternateBase, dark_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::ToolTipBase, dark_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::ToolTipText, light_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::Text, light_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::Button, darker_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::ButtonText, light_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::BrightText, light_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::Link, light_gray);
  dark_palette.setColor(QPalette::Disabled, QPalette::Highlight, dark_gray);

  QApplication::setPalette(dark_palette);
  app->setStyleSheet(
      "QToolTip { color: #ffffff; background-color: #2a82da; border: 1px "
      "solid white; }"
      "QMenu::separator { height: 1px; background-color: rgb(90, 90, 90); margin-left: 2px; "
      "margin-right: 2px; }");
}

}  // namespace orbit_style