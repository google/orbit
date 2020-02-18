#include "orbitdiffdialog.h"

#include <QFileDialog>

#include "OrbitCore/Diff.h"
#include "OrbitCore/Params.h"
#include "OrbitCore/Path.h"
#include "ui_orbitdiffdialog.h"

OrbitDiffDialog::OrbitDiffDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::OrbitDiffDialog) {
  ui->setupUi(this);
  ui->DiffExeLineEdit->setText(GParams.m_DiffExe.c_str());
  ui->ArgsLineEdit->setText(GParams.m_DiffArgs.c_str());
}

OrbitDiffDialog::~OrbitDiffDialog() { delete ui; }

void OrbitDiffDialog::on_pushButton_clicked() {}

void OrbitDiffDialog::on_BrowseButton_clicked() {
  QString file = QFileDialog::getOpenFileName(
      this, "Specify an external Diff executable...", "", "*.exe");
  GParams.m_DiffExe = file.toStdString();
  ui->DiffExeLineEdit->setText(file);
}

void OrbitDiffDialog::on_DiffExeLineEdit_textChanged(const QString& arg1) {
  GParams.m_DiffExe = arg1.toStdString();
}

void OrbitDiffDialog::on_ArgsLineEdit_textChanged(const QString& arg1) {
  GParams.m_DiffArgs = arg1.toStdString();
}

void OrbitDiffDialog::on_buttonBox_accepted() {
  Diff::Exec(ui->plainTextEdit->toPlainText().toStdString(),
             ui->plainTextEdit_2->toPlainText().toStdString());
}
