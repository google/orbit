// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <QDebug>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QWebChannel>
#include <cstdio>

int main(int argc, char* argv[]) {
  // We need to consume a symbol from the QtWebChannel module. Otherwise
  // the linker won't link against the corresponding library which contains
  // the embedded resource that we're trying to extract.
  QWebChannel _{};

  QFile webchannel_js{":/qtwebchannel/qwebchannel.js"};

  if (!webchannel_js.open(QIODevice::ReadOnly)) {
    qCritical() << "QWebChannelExractor does not come with qwebchannel.js embedded!";
    return 1;
  }

  if (argc != 2) {
    QTextStream stream{stdout};
    stream << webchannel_js.readAll();
    return 0;
  }

  const QByteArray contents = webchannel_js.readAll();

  if (QFile output_file{argv[1]};
      output_file.open(QIODevice::ReadOnly) && contents == output_file.readAll()) {
    return 0;
  }

  // Create the output directory if it does not exist
  QFileInfo{argv[1]}.dir().mkpath(".");

  QFile output_file{argv[1]};
  if (!output_file.open(QIODevice::WriteOnly | QIODevice::Truncate)) {
    qCritical() << "Could not open the output file for writing.";
    return 1;
  }

  output_file.write(contents);
  return 0;
}
