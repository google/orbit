#ifndef ORBITWATCHWIDGET_H
#define ORBITWATCHWIDGET_H

#include <QWidget>
#include <mutex>
#include <unordered_map>

namespace Ui {
class OrbitWatchWidget;
}

class Variable;
class Type;
class QtProperty;
class QtAbstractPropertyManager;

class OrbitWatchWidget : public QWidget {
  Q_OBJECT

 public:
  explicit OrbitWatchWidget(QWidget* parent = nullptr);
  ~OrbitWatchWidget() override;

  void SetupPropertyBrowser();
  void AddToWatch(const Variable* a_Variable);
  void UpdateVariable(const Variable* a_Variable);
  void Reset();
  void OnUpdateWatch(const Variable* a_Variable);

 protected:
  QtProperty* AddProp(QtProperty* a_Parent, const Variable* a_Variable);
  QtAbstractPropertyManager* GetManager(const Variable* a_Variable);
  void AddToMap(const Variable* a_Variable, QtProperty* a_QtProperty);
  QtProperty* GetProperty(const Variable* a_Variable);
  void UpdateProperty(const Variable* a_Variable);

 private slots:
  void on_FindLineEdit_textChanged(const QString& arg1);

  void valueChanged(QtProperty* property, int val);
  void valueChanged(QtProperty* property, bool val);
  void valueChanged(QtProperty* property, double val);
  void valueChanged(QtProperty* property, const QString& val);

  void on_RefreshButton_clicked();

  void on_ClearButton_clicked();

 private:
  Ui::OrbitWatchWidget* ui;

  class QtBoolPropertyManager* boolManager;
  class QtIntPropertyManager* intManager;
  class QtDoublePropertyManager* doubleManager;
  class QtStringPropertyManager* stringManager;
  class QtSizePropertyManager* sizeManager;
  class QtRectPropertyManager* rectManager;
  class QtSizePolicyPropertyManager* sizePolicyManager;
  class QtEnumPropertyManager* enumManager;
  class QtGroupPropertyManager* groupManager;

  class QtCheckBoxFactory* checkBoxFactory;
  class QtSpinBoxFactory* spinBoxFactory;
  class QtSliderFactory* sliderFactory;
  class QtScrollBarFactory* scrollBarFactory;
  class QtLineEditFactory* lineEditFactory;
  class QtEnumEditorFactory* comboBoxFactory;
  class QtDoubleSpinBoxFactory* doubleFactory;

  class QtAbstractPropertyBrowser* m_Editor;
  class QGridLayout* m_Layout;

  std::unordered_map<const Variable*, QtProperty*> m_Properties;
  std::recursive_mutex m_Mutex;
};

#endif  // ORBITWATCHWIDGET_H
