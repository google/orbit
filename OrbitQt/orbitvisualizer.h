#ifndef ORBITVISUALIZER_H
#define ORBITVISUALIZER_H

#include <QMainWindow>

namespace Ui {
class OrbitVisualizer;
}

class OrbitVisualizer : public QMainWindow {
  Q_OBJECT

 public:
  explicit OrbitVisualizer(QWidget* parent = nullptr);
  ~OrbitVisualizer() override;

  void Initialize(class OrbitMainWindow* a_MainWindow);

 private:
  Ui::OrbitVisualizer* ui;
};

#endif  // ORBITVISUALIZER_H
