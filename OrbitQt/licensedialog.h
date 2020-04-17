#ifndef LICENSEDIALOG_H
#define LICENSEDIALOG_H

#include <QDialog>

namespace Ui {
class LicenseDialog;
}

class LicenseDialog : public QDialog {
  Q_OBJECT

 public:
  explicit LicenseDialog(QWidget* parent = nullptr);
  ~LicenseDialog() override;

  std::wstring GetLicense();

 private:
  Ui::LicenseDialog* ui;
};

#endif  // LICENSEDIALOG_H
