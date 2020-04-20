#include "orbitdisassemblydialog.h"

#include "ui_orbitdisassemblydialog.h"

//-----------------------------------------------------------------------------
OrbitDisassemblyDialog::OrbitDisassemblyDialog(QWidget* parent)
    : QDialog(parent), ui(new Ui::OrbitDisassemblyDialog) {
  ui->setupUi(this);
}

//-----------------------------------------------------------------------------
OrbitDisassemblyDialog::~OrbitDisassemblyDialog() { delete ui; }

//-----------------------------------------------------------------------------
void OrbitDisassemblyDialog::SetText(const std::string& a_Text) {
  ui->plainTextEdit->SetText(a_Text);
  ui->plainTextEdit->moveCursor(QTextCursor::Start);
  ui->plainTextEdit->ensureCursorVisible();
}
