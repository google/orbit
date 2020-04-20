#include "outputdialog.h"

#include "ui_outputdialog.h"

//-----------------------------------------------------------------------------
OutputDialog::OutputDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::OutputDialog) {
  ui->setupUi(this);
}

//-----------------------------------------------------------------------------
OutputDialog::~OutputDialog() { delete ui; }

//-----------------------------------------------------------------------------
void OutputDialog::Reset() {
  ui->OutputTextEdit->setPlainText("");
  ui->staticTextEdit->setPlainText("");
}

//-----------------------------------------------------------------------------
void OutputDialog::SetStatus(const std::string& a_Status) {
  ui->staticTextEdit->setPlainText(a_Status.c_str());
}

//-----------------------------------------------------------------------------
void OutputDialog::AddLog(const std::string& a_Log) {
  QString log =
      ui->OutputTextEdit->toPlainText() + QString::fromStdString(a_Log);
  ui->OutputTextEdit->setPlainText(log);
}
