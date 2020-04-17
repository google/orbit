#ifndef PROCESSLAUNCHERWIDGET_H
#define PROCESSLAUNCHERWIDGET_H

#include <QWidget>

namespace Ui {
class ProcessLauncherWidget;
}

class ProcessLauncherWidget : public QWidget {
  Q_OBJECT

 public:
  explicit ProcessLauncherWidget(QWidget* parent = nullptr);
  ~ProcessLauncherWidget() override;

  void SetProcessParams();
  void UpdateProcessParams();

 private slots:
  void on_BrowseButton_clicked();

  void on_LaunchButton_clicked();

  void on_checkBoxPause_clicked(bool checked);

  void on_BrowseWorkingDirButton_clicked();

 private:
  Ui::ProcessLauncherWidget* ui;
};

#endif  // PROCESSLAUNCHERWIDGET_H
