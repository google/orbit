#ifndef ORBITDISASSEMBLYDIALOG_H
#define ORBITDISASSEMBLYDIALOG_H

#include <QDialog>
#include <string>

namespace Ui {
class OrbitDisassemblyDialog;
}

class OrbitDisassemblyDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OrbitDisassemblyDialog(QWidget* parent = 0);
  ~OrbitDisassemblyDialog();

  void SetText(const std::wstring& a_Text);

 private:
  Ui::OrbitDisassemblyDialog* ui;
};

#endif  // ORBITDISASSEMBLYDIALOG_H
