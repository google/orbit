/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
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
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
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

#ifndef CODEEDITOR_H
#define CODEEDITOR_H

#include <QObject>
#include <QPlainTextEdit>
#include <QSyntaxHighlighter>
#include <QTextCharFormat>

#include "../OrbitCore/RingBuffer.h"

QT_BEGIN_NAMESPACE
class QPaintEvent;
class QResizeEvent;
class QSize;
class QWidget;
class QTextDocument;
QT_END_NAMESPACE

class LineNumberArea;

//![codeeditordefinition]

class OrbitCodeEditor : public QPlainTextEdit {
  Q_OBJECT

 public:
  explicit OrbitCodeEditor(QWidget* parent = nullptr);

  void lineNumberAreaPaintEvent(QPaintEvent* event);
  int lineNumberAreaWidth();
  bool loadCode(std::string a_Msg);
  void loadFileMap();
  void saveFileMap();
  void gotoLine(int a_Line);
  void OnTimer();
  void SetText(const std::string& a_Text);
  void HighlightWord(const std::wstring& a_Text, const QColor& a_Color,
                     QList<QTextEdit::ExtraSelection>& extraSelections);

  static void setFileMappingWidget(QWidget* a_Widget) {
    GFileMapWidget = a_Widget;
  }

  enum EditorType { CODE_VIEW, FILE_MAPPING };

  void SetEditorType(EditorType a_Type);
  void SetFindLineEdit(class QLineEdit* a_Find);
  void SetSaveButton(class QPushButton* a_Button);
  void SetIsOutputWindow() { m_IsOutput = true; }

 protected:
  void resizeEvent(QResizeEvent* event) Q_DECL_OVERRIDE;
  void keyPressEvent(QKeyEvent* e) override;
  bool eventFilter(QObject* object, QEvent* event) override;
  void Find(const QString& a_String, bool a_BackWards = false);

 private slots:
  void updateLineNumberAreaWidth(int newBlockCount);
  void highlightCurrentLine();
  void updateLineNumberArea(const QRect&, int);
  void OnFindTextEntered(const QString&);
  void OnSaveMapFile();

 private:
  QWidget* lineNumberArea;
  class Highlighter* highlighter;
  class QLineEdit* m_FindLineEdit;
  class QPushButton* m_SaveButton;
  EditorType m_Type;
  bool m_IsOutput;

  static OrbitCodeEditor* GFileMapEditor;
  static QWidget* GFileMapWidget;

  static const int HISTORY_SIZE = 2;
  RingBuffer<std::wstring, HISTORY_SIZE> m_SelectedText;
  QColor m_SelectedColors[HISTORY_SIZE];
};

//![codeeditordefinition]
//![extraarea]

class LineNumberArea : public QWidget {
 public:
  explicit LineNumberArea(OrbitCodeEditor* editor) : QWidget(editor) {
    codeEditor = editor;
  }

  QSize sizeHint() const Q_DECL_OVERRIDE {
    return QSize(codeEditor->lineNumberAreaWidth(), 0);
  }

 protected:
  void paintEvent(QPaintEvent* event) Q_DECL_OVERRIDE {
    codeEditor->lineNumberAreaPaintEvent(event);
  }

 private:
  OrbitCodeEditor* codeEditor;
};

//! [0]
class Highlighter : public QSyntaxHighlighter {
  Q_OBJECT

 public:
  explicit Highlighter(QTextDocument* parent = nullptr);

 protected:
  void highlightBlock(const QString& text) Q_DECL_OVERRIDE;

 private:
  struct HighlightingRule {
    QRegExp pattern;
    QTextCharFormat format;
  };
  QVector<HighlightingRule> highlightingRules;

  QRegExp commentStartExpression;
  QRegExp commentEndExpression;

  QTextCharFormat keywordFormat;
  QTextCharFormat classFormat;
  QTextCharFormat singleLineCommentFormat;
  QTextCharFormat multiLineCommentFormat;
  QTextCharFormat quotationFormat;
  QTextCharFormat functionFormat;
  QTextCharFormat hexFormat;
};

#endif
