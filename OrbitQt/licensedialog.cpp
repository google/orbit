#include "licensedialog.h"
#include <QPlainTextEdit>
#include "ui_licensedialog.h"

LicenseDialog::LicenseDialog(QWidget *parent)
    : QDialog(parent), ui(new Ui::LicenseDialog) {
  ui->setupUi(this);
}

LicenseDialog::~LicenseDialog() { delete ui; }

std::wstring LicenseDialog::GetLicense() {
  return ui->LicenseTextEdit->document()->toPlainText().toStdWString();
}