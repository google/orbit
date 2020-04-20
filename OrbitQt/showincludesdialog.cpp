#include "showincludesdialog.h"

#include <QMenu>
#include <QSignalMapper>
#include <functional>

#include "orbittreeitem.h"
#include "orbittreemodel.h"
#include "ui_showincludesdialog.h"

ShowIncludesDialog::ShowIncludesDialog(QWidget* parent)
    : QDialog(parent),
      ui(new Ui::ShowIncludesDialog),
      m_TreeModel(nullptr),
      m_FilteredTreeModel(nullptr) {
  ui->setupUi(this);
  m_TreeModel =
      new OrbitTreeModel(QString("one\n two\n  three\n  four\n five\n"));
  ui->treeView->setModel(m_TreeModel);

  ui->treeView->setContextMenuPolicy(Qt::CustomContextMenu);
  connect(ui->treeView, SIGNAL(customContextMenuRequested(const QPoint&)), this,
          SLOT(onCustomContextMenu(const QPoint&)));

  m_ModelIndex = QModelIndex();

  ui->plainTextEdit->setPlainText(
      "// Copy Visual Studio's output generated when\n// \"Show Includes\" "
      "(/showincludes) is active\n// to see a proper tree view on the right "
      "pane.");

  ui->pushButton->setDefault(false);
  ui->pushButton->setAutoDefault(false);
  ui->pushButton_2->setDefault(false);
  ui->pushButton_2->setAutoDefault(false);
}

enum ShowIncludesMenuItems { EXPAND, COLLAPSE };

const std::vector<std::wstring>& GContextMenuShowIncludes = {L"Expand",
                                                             L"Collapse"};

void ShowIncludesDialog::onCustomContextMenu(const QPoint& point) {
  m_ModelIndex = ui->treeView->indexAt(point);
  if (m_ModelIndex.isValid()) {
    if (GContextMenuShowIncludes.size() > 0) {
      QMenu contextMenu(tr("ContextMenu"), this);
      QSignalMapper signalMapper(this);
      std::vector<QAction*> actions;

      for (int i = 0; i < (int)GContextMenuShowIncludes.size(); ++i) {
        actions.push_back(
            new QAction(QString::fromStdWString(GContextMenuShowIncludes[i])));
        connect(actions[i], SIGNAL(triggered()), &signalMapper, SLOT(map()));
        signalMapper.setMapping(actions[i], i);
        contextMenu.addAction(actions[i]);
      }

      connect(&signalMapper, SIGNAL(mapped(int)), this,
              SLOT(OnMenuClicked(int)));
      contextMenu.exec(ui->treeView->mapToGlobal(point));

      for (QAction* action : actions) delete action;
    }
  }
}

void expandChildren(const QModelIndex& index, QTreeView* view) {
  if (!index.isValid()) {
    return;
  }

  int childCount = index.model()->rowCount(index);
  for (int i = 0; i < childCount; i++) {
    const QModelIndex& child = index.child(i, 0);
    expandChildren(child, view);
  }

  if (!view->isExpanded(index)) {
    view->expand(index);
  }
}

void collapseChildren(const QModelIndex& index, QTreeView* view) {
  if (!index.isValid()) {
    return;
  }

  int childCount = index.model()->rowCount(index);
  for (int i = 0; i < childCount; i++) {
    const QModelIndex& child = index.child(i, 0);
    collapseChildren(child, view);
  }

  view->collapse(index);
}

void ShowIncludesDialog::OnMenuClicked(int a_Index) {
  if (a_Index == COLLAPSE) {
    collapseChildren(m_ModelIndex, ui->treeView);
  } else if (a_Index == EXPAND) {
    expandChildren(m_ModelIndex, ui->treeView);
  }
}

ShowIncludesDialog::~ShowIncludesDialog() { delete ui; }

void ShowIncludesDialog::on_plainTextEdit_textChanged() {
  QString text = ui->plainTextEdit->toPlainText();
  QStringList lines = text.split("\n");
  QStringList filtered;
  QString filter = ui->lineEdit->text();

  if (filter != "") {
    for (QString& line : lines) {
      if (line.contains(filter)) {
        QStringList tokens = line.split(filter);
        if (tokens.size() == 2) {
          filtered += tokens[1];
        }
      } else {
        // find .cpp file being compiled
        QStringList tokens = line.split(">  ");
        if (tokens.size() == 2) {
          filtered += tokens[1];
        }
      }
    }
  } else {
    filtered = lines;
  }

  text = "";
  for (QString& line : filtered) {
    text += line;
    text += "\n";
  }

  delete m_TreeModel;
  m_TreeModel = new OrbitTreeModel(text);
  ui->treeView->setModel(m_TreeModel);
}

void ShowIncludesDialog::on_lineEdit_textChanged(const QString& /*arg1*/) {
  on_plainTextEdit_textChanged();
}

void ShowIncludesDialog::on_pushButton_clicked() { ui->treeView->expandAll(); }

void ShowIncludesDialog::on_pushButton_2_clicked() {
  ui->treeView->collapseAll();
}

void iterate(const QModelIndex& index, const QAbstractItemModel* model,
             const std::function<void(const QModelIndex&, int)>& fun,
             int depth = 0) {
  if (index.isValid()) fun(index, depth);
  if (!model->hasChildren(index)) return;
  auto rows = model->rowCount(index);
  auto cols = model->columnCount(index);
  for (int i = 0; i < rows; ++i)
    for (int j = 0; j < cols; ++j)
      iterate(model->index(i, j, index), model, fun, depth + 1);
}

void ShowIncludesDialog::on_lineEdit_2_textChanged(const QString& arg1) {
  std::string filter = arg1.toStdString();
  m_TreeModel->Filter(filter);

  ui->treeView->collapseAll();

  iterate(ui->treeView->rootIndex(), ui->treeView->model(),
          [=](const QModelIndex& idx, int /*depth*/) {
            OrbitTreeItem* item =
                static_cast<OrbitTreeItem*>(idx.internalPointer());
            if (item->IsVisible()) {
              ui->treeView->expand(idx);
            }
          });
}
