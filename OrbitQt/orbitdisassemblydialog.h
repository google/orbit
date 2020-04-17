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
  explicit OrbitDisassemblyDialog(QWidget* parent = nullptr);
  ~OrbitDisassemblyDialog() override;

  void SetText(const std::string& a_Text);

 private:
  Ui::OrbitDisassemblyDialog* ui;
};

#endif  // ORBITDISASSEMBLYDIALOG_H
