#ifndef OUTPUTDIALOG_H
#define OUTPUTDIALOG_H

#include <QDialog>

namespace Ui {
class OutputDialog;
}

class OutputDialog : public QDialog {
  Q_OBJECT

 public:
  explicit OutputDialog(QWidget* parent = 0);
  ~OutputDialog();

  void Reset();
  void SetStatus(const std::string& a_Status);
  void AddLog(const std::wstring& a_Log);

 private:
  Ui::OutputDialog* ui;
};

#endif  // OUTPUTDIALOG_H
