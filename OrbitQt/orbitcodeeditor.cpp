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

// This needs to be first because if it is not GL/glew.h
// complains about being included after gl.h
// clang-format off
#include "OpenGl.h"
// clang-format on

#include "orbitcodeeditor.h"

#include <QPushButton>
#include <QTextCursor>
#include <QtWidgets>
#include <fstream>

#include "../OrbitCore/LogInterface.h"
#include "../OrbitCore/Path.h"
#include "../OrbitCore/PrintVar.h"
#include "../OrbitCore/Utils.h"
#include "App.h"
#include "absl/strings/str_format.h"

OrbitCodeEditor* OrbitCodeEditor::GFileMapEditor;
QWidget* OrbitCodeEditor::GFileMapWidget;

//![constructor]

OrbitCodeEditor::OrbitCodeEditor(QWidget* parent) : QPlainTextEdit(parent) {
  lineNumberArea = new LineNumberArea(this);

  const QFont fixedFont = QFontDatabase::systemFont(QFontDatabase::FixedFont);
  setFont(fixedFont);

  const int tabStop = 4;  // 4 characters
  QFontMetrics metrics(fixedFont);
  setTabStopWidth(tabStop * metrics.width(' '));

  //(QTextOption::NoWrap);
  this->setWordWrapMode(QTextOption::NoWrap);

  connect(this, SIGNAL(blockCountChanged(int)), this,
          SLOT(updateLineNumberAreaWidth(int)));
  connect(this, SIGNAL(updateRequest(QRect, int)), this,
          SLOT(updateLineNumberArea(QRect, int)));
  connect(this, SIGNAL(cursorPositionChanged()), this,
          SLOT(highlightCurrentLine()));

  updateLineNumberAreaWidth(0);
  highlightCurrentLine();

  highlighter = new Highlighter(document());

  QPalette p = this->palette();
  p.setColor(QPalette::Base, QColor(30, 30, 30));
  p.setColor(QPalette::Text, QColor(189, 183, 107));
  this->setPalette(p);

  m_SelectedColors[0] = QColor(231, 68, 53);   // red
  m_SelectedColors[1] = QColor(43, 145, 175);  // blue

  m_FindLineEdit = nullptr;
  m_SaveButton = nullptr;
  m_IsOutput = false;
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::SetEditorType(EditorType a_Type) {
  m_Type = a_Type;

  if (a_Type == EditorType::FILE_MAPPING) {
    GFileMapEditor = this;
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::SetFindLineEdit(QLineEdit* a_Find) {
  if (a_Find) {
    m_FindLineEdit = a_Find;
    m_FindLineEdit->hide();
    connect(a_Find, SIGNAL(textChanged(const QString&)), this,
            SLOT(OnFindTextEntered(const QString&)));
    m_FindLineEdit->installEventFilter(this);
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::SetSaveButton(QPushButton* a_Button) {
  if (m_Type == OrbitCodeEditor::FILE_MAPPING) {
    m_SaveButton = a_Button;
    connect(a_Button, SIGNAL(pressed()), this, SLOT(OnSaveMapFile()));
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::OnSaveMapFile() {
  this->saveFileMap();
  GFileMapWidget->hide();
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::OnFindTextEntered(const QString&) {
  // setFocus();
  PRINT_VAR(m_FindLineEdit->text().toStdString());
  QTextCursor cursor(textCursor());
  cursor.movePosition(QTextCursor::EndOfWord);
  setTextCursor(cursor);
  find(m_FindLineEdit->text());
}

//![constructor]

//![extraAreaWidth]

int OrbitCodeEditor::lineNumberAreaWidth() {
  int digits = 1;
  int max = qMax(1, blockCount());
  while (max >= 10) {
    max /= 10;
    ++digits;
  }

  int space = 3 + fontMetrics().width(QLatin1Char('9')) * digits;

  return space;
}

//-----------------------------------------------------------------------------
bool OrbitCodeEditor::loadCode(std::string a_Msg) {
  std::vector<std::string> tokens = Tokenize(a_Msg, "^");

  if (tokens.size() == 3) {
    QFile file(tokens[1].c_str());
    bool success = file.open(QFile::ReadOnly | QFile::Text);
    if (success) {
      QTextStream ReadFile(&file);
      this->document()->setPlainText(ReadFile.readAll());
      int lineNumber = atoi(tokens[2].c_str());
      gotoLine(lineNumber);
    } else {
      std::string msg = absl::StrFormat("Could not find %s (%s)\n",
                                        tokens[1].c_str(), tokens[2].c_str());
      msg +=
          "Please modify FileMapping.txt shown below if the source code is "
          "available at another location.";
      document()->setPlainText(msg.c_str());
      return false;
    }
  }

  return true;
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::loadFileMap() {
  QFile file(QString::fromStdString(Path::GetFileMappingFileName()));
  bool success = file.open(QFile::ReadWrite | QFile::Text);
  if (success) {
    QTextStream ReadFile(&file);
    this->document()->setPlainText(ReadFile.readAll());
  }
  file.close();
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::gotoLine(int a_Line) {
  moveCursor(QTextCursor::End);
  QTextBlock block = this->document()->findBlockByLineNumber(a_Line - 1);
  QTextCursor text_cursor(block);

  int numMoves = a_Line - block.fragmentIndex();
  for (int i = 0; i < numMoves; ++i) {
    text_cursor.movePosition(QTextCursor::Down);
  }

  text_cursor.select(QTextCursor::LineUnderCursor);

  if (text_cursor.selectedText().trimmed() == "{") {
    text_cursor.movePosition(QTextCursor::Up);
  }

  text_cursor.movePosition(QTextCursor::StartOfLine);

  text_cursor.clearSelection();
  setTextCursor(text_cursor);
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::OnTimer() {
  if (m_IsOutput && isVisible()) {
    std::vector<std::string> outputEntries = LogInterface::GetOutput();
    for (std::string& line : outputEntries) {
      QTextCursor prev_cursor = textCursor();
      moveCursor(QTextCursor::End);
      insertPlainText(line.c_str());
      setTextCursor(prev_cursor);
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::SetText(const std::string& a_Text) {
  this->document()->setPlainText(QString::fromStdString(a_Text));
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::updateLineNumberAreaWidth(int /* newBlockCount */) {
  setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::updateLineNumberArea(const QRect& rect, int dy) {
  if (dy)
    lineNumberArea->scroll(0, dy);
  else
    lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());

  if (rect.contains(viewport()->rect())) updateLineNumberAreaWidth(0);
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::resizeEvent(QResizeEvent* e) {
  QPlainTextEdit::resizeEvent(e);

  QRect cr = contentsRect();
  lineNumberArea->setGeometry(
      QRect(cr.left(), cr.top(), lineNumberAreaWidth(), cr.height()));
}

//-----------------------------------------------------------------------------
bool OrbitCodeEditor::eventFilter(QObject* object, QEvent* event) {
  if (object == m_FindLineEdit && event->type() == QEvent::KeyPress) {
    QKeyEvent* keyEvent = static_cast<QKeyEvent*>(event);

    switch (keyEvent->key()) {
      case Qt::Key_Escape:
        m_FindLineEdit->hide();
        return true;
      case Qt::Key_F3:
      case Qt::Key_Enter:
      case Qt::Key_Return:
        bool backwards = (keyEvent->modifiers() == Qt::ShiftModifier ||
                          keyEvent->modifiers() == Qt::ControlModifier);
        Find(m_FindLineEdit->text(), backwards);
        return true;
    }
  }

  return false;
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::Find(const QString& a_String, bool a_BackWards) {
  UNUSED(a_String);
  find(m_FindLineEdit->text(),
       a_BackWards ? QTextDocument::FindBackward : QTextDocument::FindFlag());
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::keyPressEvent(QKeyEvent* e) {
  QPlainTextEdit::keyPressEvent(e);

  if ((e->key() == Qt::Key_F)) {
    if (m_FindLineEdit && (e->modifiers() == Qt::ControlModifier)) {
      m_FindLineEdit->show();
      m_FindLineEdit->setFocus();
      m_FindLineEdit->selectAll();
    }
  } else if (m_FindLineEdit && e->key() == Qt::Key_F3) {
    if (e->modifiers() == Qt::ControlModifier) {
      m_FindLineEdit->setText(textCursor().selectedText());
    }

    bool backwards = (e->modifiers() == Qt::ShiftModifier);
    Find(m_FindLineEdit->text(), backwards);
  } else if (e->key() == Qt::Key_Escape) {
    if (m_FindLineEdit) {
      m_FindLineEdit->hide();
    }
    textCursor().clearSelection();
  } else if (m_Type == OrbitCodeEditor::FILE_MAPPING && e->key() == Qt::Key_S &&
             e->modifiers() == Qt::ControlModifier) {
    saveFileMap();
    GOrbitApp->LoadFileMapping();
  } else if (e->key() == Qt::Key_M && e->modifiers() == Qt::ControlModifier) {
    if (GFileMapWidget->isVisible()) {
      GFileMapWidget->hide();
    } else {
      GOrbitApp->LoadFileMapping();
      GFileMapEditor->loadFileMap();
      GFileMapWidget->show();
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::saveFileMap() {
  std::string fileName = Path::GetFileMappingFileName();
  std::wofstream outFile(fileName);
  if (!outFile.fail()) {
    outFile << document()->toPlainText().toStdWString();
    outFile.close();
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::HighlightWord(
    const std::wstring& a_Text, const QColor& a_Color,
    QList<QTextEdit::ExtraSelection>& extraSelections) {
  QString searchString(QString::fromStdWString(a_Text));
  QTextDocument* document = this->document();
  QTextCursor highlightCursor(document);
  QTextCursor cursor(document);

  QTextCharFormat plainFormat(highlightCursor.charFormat());

  while (!highlightCursor.isNull() && !highlightCursor.atEnd()) {
    highlightCursor = document->find(searchString, highlightCursor,
                                     QTextDocument::FindWholeWords);

    if (!highlightCursor.isNull()) {
      QTextEdit::ExtraSelection wordSelection;
      QColor wordColor = a_Color;
      wordSelection.format.setBackground(wordColor);
      wordSelection.format.setForeground(QColor(255, 255, 255));
      wordSelection.cursor = highlightCursor;
      extraSelections.append(wordSelection);
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::highlightCurrentLine() {
  QList<QTextEdit::ExtraSelection> extraSelections;

  if (!isReadOnly()) {
    QTextEdit::ExtraSelection selection;
    QColor lineColor = QColor(15, 15, 15);
    selection.format.setBackground(lineColor);
    selection.format.setProperty(QTextFormat::FullWidthSelection, true);
    selection.cursor = textCursor();
    selection.cursor.clearSelection();
    extraSelections.append(selection);

    // Word Selection
    QTextEdit::ExtraSelection wordSelection;
    wordSelection.cursor = textCursor();
    wordSelection.cursor.select(QTextCursor::WordUnderCursor);
    std::wstring word = wordSelection.cursor.selectedText().toStdWString();
    if (word == L"" || !m_SelectedText.Contains(word)) {
      m_SelectedText.Add(word);
    }

    for (size_t i = 0; i < m_SelectedText.Size(); ++i) {
      std::wstring selectedWord = m_SelectedText.Data()[i];
      HighlightWord(selectedWord, m_SelectedColors[i], extraSelections);
    }
  }

  setExtraSelections(extraSelections);
}

//-----------------------------------------------------------------------------
void OrbitCodeEditor::lineNumberAreaPaintEvent(QPaintEvent* event) {
  QPainter painter(lineNumberArea);
  painter.fillRect(event->rect(), QColor(30, 30, 30));

  //![extraAreaPaintEvent_0]

  //![extraAreaPaintEvent_1]
  QTextBlock block = firstVisibleBlock();
  int blockNumber = block.blockNumber();
  int top = static_cast<int>(
      blockBoundingGeometry(block).translated(contentOffset()).top());
  int bottom = top + static_cast<int>(blockBoundingRect(block).height());
  //![extraAreaPaintEvent_1]

  //![extraAreaPaintEvent_2]
  while (block.isValid() && top <= event->rect().bottom()) {
    if (block.isVisible() && bottom >= event->rect().top()) {
      QString number = QString::number(blockNumber + 1);
      painter.setPen(QColor(43, 145, 175));
      painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                       Qt::AlignRight, number);
    }

    block = block.next();
    top = bottom;
    bottom = top + static_cast<int>(blockBoundingRect(block).height());
    ++blockNumber;
  }
}

//-----------------------------------------------------------------------------
Highlighter::Highlighter(QTextDocument* parent) : QSyntaxHighlighter(parent) {
  HighlightingRule rule;

  keywordFormat.setForeground(QColor(86, 156, 205));
  keywordFormat.setFontWeight(QFont::Bold);
  QStringList keywordPatterns;
  keywordPatterns << "\\bchar\\b"
                  << "\\bclass\\b"
                  << "\\bconst\\b"
                  << "\\bdouble\\b"
                  << "\\benum\\b"
                  << "\\bexplicit\\b"
                  << "\\bfriend\\b"
                  << "\\binline\\b"
                  << "\\bint\\b"
                  << "\\blong\\b"
                  << "\\bnamespace\\b"
                  << "\\boperator\\b"
                  << "\\bprivate\\b"
                  << "\\bprotected\\b"
                  << "\\bpublic\\b"
                  << "\\bshort\\b"
                  << "\\bsignals\\b"
                  << "\\bsigned\\b"
                  << "\\bslots\\b"
                  << "\\bstatic\\b"
                  << "\\bstruct\\b"
                  << "\\btemplate\\b"
                  << "\\btypedef\\b"
                  << "\\btypename\\b"
                  << "\\bunion\\b"
                  << "\\bunsigned\\b"
                  << "\\bvirtual\\b"
                  << "\\bvoid\\b"
                  << "\\bvolatile\\b"
                  << "\\b__declspec"
                  << "\\bnoinline"
                  << "\\bnaked"
                  << "\\b__asm"
                  << "\\bbool"
                  << "\\bfloat";
  foreach (const QString& pattern, keywordPatterns) {
    rule.pattern = QRegExp(pattern);
    rule.format = keywordFormat;
    highlightingRules.append(rule);
    //! [0] //! [1]
  }
  //! [1]

  //! [2]
  classFormat.setFontWeight(QFont::Bold);
  classFormat.setForeground(QColor(255, 198, 17));
  rule.pattern = QRegExp("\\bQ[A-Za-z]+\\b");
  rule.format = classFormat;
  highlightingRules.append(rule);
  //! [2]

  //! [3]
  singleLineCommentFormat.setForeground(QColor(87, 166, 74));
  rule.pattern = QRegExp("//[^\n]*");
  rule.format = singleLineCommentFormat;
  highlightingRules.append(rule);

  multiLineCommentFormat.setForeground(QColor(87, 166, 74));
  //! [3]

  //! [4]
  quotationFormat.setForeground(QColor(214, 157, 133));
  rule.pattern = QRegExp("\".*\"");
  rule.format = quotationFormat;
  highlightingRules.append(rule);
  //! [4]

  //! [5]
  functionFormat.setFontItalic(true);
  functionFormat.setForeground(QColor(255, 114, 17));
  rule.pattern = QRegExp("\\b[A-Za-z0-9_]+(?=\\()");
  rule.format = functionFormat;
  highlightingRules.append(rule);
  //! [5]

  //! [6]
  commentStartExpression = QRegExp("/\\*");
  commentEndExpression = QRegExp("\\*/");

  hexFormat.setForeground(QColor(181, 206, 168));
  rule.pattern = QRegExp("0[xX][0-9a-fA-F]+");
  rule.format = hexFormat;
  highlightingRules.append(rule);
}

//-----------------------------------------------------------------------------
void Highlighter::highlightBlock(const QString& text) {
  foreach (const HighlightingRule& rule, highlightingRules) {
    QRegExp expression(rule.pattern);
    int index = expression.indexIn(text);
    while (index >= 0) {
      int length = expression.matchedLength();
      setFormat(index, length, rule.format);
      index = expression.indexIn(text, index + length);
    }
  }
  //! [7] //! [8]
  setCurrentBlockState(0);
  //! [8]

  //! [9]
  int startIndex = 0;
  if (previousBlockState() != 1)
    startIndex = commentStartExpression.indexIn(text);

  //! [9] //! [10]
  while (startIndex >= 0) {
    //! [10] //! [11]
    int endIndex = commentEndExpression.indexIn(text, startIndex);
    int commentLength;
    if (endIndex == -1) {
      setCurrentBlockState(1);
      commentLength = text.length() - startIndex;
    } else {
      commentLength =
          endIndex - startIndex + commentEndExpression.matchedLength();
    }
    setFormat(startIndex, commentLength, multiLineCommentFormat);
    startIndex =
        commentStartExpression.indexIn(text, startIndex + commentLength);
  }
}
