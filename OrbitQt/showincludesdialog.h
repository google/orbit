#ifndef SHOWINCLUDESDIALOG_H
#define SHOWINCLUDESDIALOG_H

#include <QDialog>
#include <QModelIndex>

class OrbitTreeModel;

namespace Ui {
class ShowIncludesDialog;
}

class ShowIncludesDialog : public QDialog {
  Q_OBJECT

 public:
  explicit ShowIncludesDialog(QWidget* parent = nullptr);
  ~ShowIncludesDialog() override;

 public slots:
  void onCustomContextMenu(const QPoint& point);
  void OnMenuClicked(int);

 private slots:
  void on_plainTextEdit_textChanged();

  void on_lineEdit_textChanged(const QString& arg1);

  void on_pushButton_clicked();

  void on_pushButton_2_clicked();

  void on_lineEdit_2_textChanged(const QString& arg1);

 private:
  Ui::ShowIncludesDialog* ui;
  QModelIndex m_ModelIndex;

  OrbitTreeModel* m_TreeModel;
  OrbitTreeModel* m_FilteredTreeModel;
};

#endif  // SHOWINCLUDESDIALOG_H
