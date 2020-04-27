#include "orbitwatchwidget.h"

#include <QGridLayout>
#include <QLabel>
#include <QScrollArea>

#include "../OrbitCore/OrbitType.h"
#include "../OrbitCore/Pdb.h"
#include "../OrbitCore/PrintVar.h"
#include "../OrbitCore/Variable.h"
#include "../OrbitGl/App.h"
#include "qtpropertybrowser/qtbuttonpropertybrowser.h"
#include "qtpropertybrowser/qteditorfactory.h"
#include "qtpropertybrowser/qtgroupboxpropertybrowser.h"
#include "qtpropertybrowser/qtpropertymanager.h"
#include "qtpropertybrowser/qttreepropertybrowser.h"
#include "ui_orbitwatchwidget.h"

//-----------------------------------------------------------------------------
void OrbitWatchWidget::valueChanged(QtProperty* property, int val) {
  if (Variable* var = static_cast<Variable*>(property->GetUserData())) {
    var->m_Int = val;
    var->SendValue();
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::valueChanged(QtProperty* property, bool val) {
  if (Variable* var = static_cast<Variable*>(property->GetUserData())) {
    var->m_Bool = val;
    var->SendValue();
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::valueChanged(QtProperty* property, double val) {
  if (Variable* var = static_cast<Variable*>(property->GetUserData())) {
    var->SetDouble(val);
    var->SendValue();
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::valueChanged(QtProperty* property,
                                    const QString& /*val*/) {
  if (Variable* var = static_cast<Variable*>(property->GetUserData())) {
    var->SetDouble(0);
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::SetupPropertyBrowser() {
  QWidget* w = ui->PropertyGridWidget;

  boolManager = new QtBoolPropertyManager(w);
  intManager = new QtIntPropertyManager(w);
  doubleManager = new QtDoublePropertyManager(w);
  stringManager = new QtStringPropertyManager(w);
  sizeManager = new QtSizePropertyManager(w);
  rectManager = new QtRectPropertyManager(w);
  sizePolicyManager = new QtSizePolicyPropertyManager(w);
  enumManager = new QtEnumPropertyManager(w);
  groupManager = new QtGroupPropertyManager(w);

  checkBoxFactory = new QtCheckBoxFactory(w);
  spinBoxFactory = new QtSpinBoxFactory(w);
  sliderFactory = new QtSliderFactory(w);
  scrollBarFactory = new QtScrollBarFactory(w);
  lineEditFactory = new QtLineEditFactory(w);
  comboBoxFactory = new QtEnumEditorFactory(w);
  doubleFactory = new QtDoubleSpinBoxFactory(w);

  m_Editor = new QtTreePropertyBrowser();
  m_Editor->setFactoryForManager(boolManager, checkBoxFactory);
  m_Editor->setFactoryForManager(intManager, spinBoxFactory);
  m_Editor->setFactoryForManager(doubleManager, doubleFactory);
  m_Editor->setFactoryForManager(stringManager, lineEditFactory);
  m_Editor->setFactoryForManager(sizeManager->subIntPropertyManager(),
                                 spinBoxFactory);
  m_Editor->setFactoryForManager(rectManager->subIntPropertyManager(),
                                 spinBoxFactory);
  m_Editor->setFactoryForManager(sizePolicyManager->subIntPropertyManager(),
                                 sliderFactory);
  m_Editor->setFactoryForManager(sizePolicyManager->subEnumPropertyManager(),
                                 comboBoxFactory);
  m_Editor->setFactoryForManager(enumManager, comboBoxFactory);

  m_Layout = new QGridLayout(w);
  m_Layout->setMargin(0);
  m_Layout->addWidget(m_Editor, 1, 0);

  connect(boolManager, SIGNAL(valueChanged(QtProperty*, bool)), this,
          SLOT(valueChanged(QtProperty*, bool)));
  connect(intManager, SIGNAL(valueChanged(QtProperty*, int)), this,
          SLOT(valueChanged(QtProperty*, int)));
  connect(doubleManager, SIGNAL(valueChanged(QtProperty*, double)), this,
          SLOT(valueChanged(QtProperty*, double)));

  GOrbitApp->AddUpdateWatchCallback(
      [this](const Variable* a_Variable) { this->OnUpdateWatch(a_Variable); });
}

//-----------------------------------------------------------------------------
OrbitWatchWidget::OrbitWatchWidget(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitWatchWidget) {
  ui->setupUi(this);

  SetupPropertyBrowser();
}

//-----------------------------------------------------------------------------
OrbitWatchWidget::~OrbitWatchWidget() { delete ui; }

/*
    Variable::Int8;
    Variable::UInt8;
    Variable::Int16;
    Variable::UInt16;

    Variable::Int64;
    Variable::UInt64;
    Variable::LongLong;
    Variable::ULongLong;

    Variable::Float;
    Variable::Double;
    Variable::LDouble;

    Variable::Bool;

    Variable::Char;
    Variable::SChar;
    Variable::UChar;

    Variable::Enum;

    Variable::WChar;
    */

//-----------------------------------------------------------------------------
Variable::BasicType GetBasicType(const Variable* a_Variable) {
  Type* type = const_cast<Type*>(a_Variable->GetType());
  Variable* var = const_cast<Variable*>(a_Variable);

  if (type) {
    // ensure that hierarchy is generated
    type->LoadDiaInfo();
  }

  return var->GetBasicType();
}

//-----------------------------------------------------------------------------
QtAbstractPropertyManager* OrbitWatchWidget::GetManager(
    const Variable* a_Variable) {
  QtAbstractPropertyManager* manager = nullptr;

  auto basicType = GetBasicType(a_Variable);

  switch (basicType) {
    case Variable::Int:
    case Variable::UInt:
    case Variable::Int32:
    case Variable::UInt32:
    case Variable::Short:
    case Variable::UShort:
    case Variable::Long:
    case Variable::ULong:
      manager = intManager;
      break;
    case Variable::Bool:
      manager = boolManager;
      break;
    case Variable::Float:
    case Variable::Double:
    case Variable::LDouble:
      manager = doubleManager;
      break;
    default:
      manager = stringManager;
      break;
  }

  return manager;
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::AddToMap(const Variable* a_Variable,
                                QtProperty* a_QtProperty) {
  std::lock_guard<std::recursive_mutex> lock(m_Mutex);
  m_Properties[a_Variable] = a_QtProperty;
}

//-----------------------------------------------------------------------------
QtProperty* OrbitWatchWidget::GetProperty(const Variable* a_Variable) {
  std::lock_guard<std::recursive_mutex> lock(m_Mutex);
  return m_Properties[a_Variable];
}

//-----------------------------------------------------------------------------
QtProperty* OrbitWatchWidget::AddProp(QtProperty* a_Parent,
                                      const Variable* a_Variable) {
  QtAbstractPropertyManager* manager = GetManager(a_Variable);
  QtProperty* newProperty = nullptr;
  std::string typeName = a_Variable->m_Type.empty() ? a_Variable->GetTypeName()
                                                    : a_Variable->m_Type;

  if (a_Variable->m_Children.size()) {
    newProperty =
        groupManager->addProperty(QString::fromStdString(a_Variable->m_Name));

    for (std::shared_ptr<Variable> member : a_Variable->m_Children) {
      QtProperty* prop = AddProp(newProperty, member.get());
      newProperty->addSubProperty(prop);
    }

    newProperty->setPropertyType(QString::fromStdString(typeName));
  } else {
    newProperty =
        manager->addProperty(QString::fromStdString(a_Variable->m_Name));
    newProperty->setPropertyType(QString::fromStdString(typeName));
    // TODO: Remove const_cast
    newProperty->SetUserData(const_cast<Variable*>(a_Variable));

    AddToMap(a_Variable, newProperty);

    if (a_Parent) {
      a_Parent->addSubProperty(newProperty);
    }
  }

  return newProperty;
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::AddToWatch(const Variable* a_Variable) {
  if (QtProperty* prop = AddProp(nullptr, a_Variable)) {
    m_Editor->addProperty(prop);
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::UpdateVariable(const Variable* a_Variable) {
  if (a_Variable->IsBasicType()) {
    UpdateProperty(a_Variable);
  } else {
    for (const std::shared_ptr<Variable> var : a_Variable->m_Children) {
      UpdateProperty(var.get());
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::UpdateProperty(const Variable* a_Variable) {
  std::lock_guard<std::recursive_mutex> lock(m_Mutex);
  QtProperty* prop = GetProperty(a_Variable);

  if (prop) {
    if (prop->GetUserData() == a_Variable) {
      auto basicType = GetBasicType(a_Variable);

      switch (basicType) {
        case Variable::Int:
        case Variable::UInt:
        case Variable::Int32:
        case Variable::UInt32:
        case Variable::Short:
        case Variable::UShort:
        case Variable::Long:
        case Variable::ULong:
          intManager->setValue(prop, a_Variable->m_Int);
          break;
        case Variable::Bool:
          boolManager->setValue(prop, a_Variable->m_Bool);
          break;
        case Variable::Float:
          doubleManager->setValue(prop, a_Variable->m_Float);
          break;
        case Variable::Double:
          doubleManager->setValue(prop, a_Variable->m_Double);
          break;
        case Variable::LDouble:
          break;
        default:
          break;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::Reset() {
  std::lock_guard<std::recursive_mutex> lock(m_Mutex);
  m_Properties.clear();
  m_Editor->clear();
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::OnUpdateWatch(const Variable* a_Variable) {
  UpdateVariable(a_Variable);
}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::on_FindLineEdit_textChanged(const QString& /*arg1*/) {}

//-----------------------------------------------------------------------------
void OrbitWatchWidget::on_RefreshButton_clicked() { GOrbitApp->RefreshWatch(); }

//-----------------------------------------------------------------------------
void OrbitWatchWidget::on_ClearButton_clicked() {
  Reset();
  GOrbitApp->ClearWatchedVariables();
}
