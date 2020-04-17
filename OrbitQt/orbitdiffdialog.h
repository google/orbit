#ifndef ORBITDIFFDIALOG_H
#define ORBITDIFFDIALOG_H

#include <QDialog>

namespace Ui {
class OrbitDiffDialog;
}

class OrbitDiffDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OrbitDiffDialog(QWidget* parent = nullptr);
  ~OrbitDiffDialog() override;

 private slots:
  void on_pushButton_clicked();

  void on_BrowseButton_clicked();

  void on_DiffExeLineEdit_textChanged(const QString& arg1);

  void on_ArgsLineEdit_textChanged(const QString& arg1);

  void on_buttonBox_accepted();

 private:
  Ui::OrbitDiffDialog* ui;
};

#endif  // ORBITDIFFDIALOG_H
