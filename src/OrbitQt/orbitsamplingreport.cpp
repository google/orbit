// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "orbitsamplingreport.h"

#include <QGridLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QList>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPushButton>
#include <QSplitter>
#include <QStaticStringData>
#include <QStringLiteral>
#include <QTabWidget>
#include <Qt>
#include <algorithm>
#include <optional>
#include <string>

#include "OrbitBase/Logging.h"
#include "SamplingReport.h"
#include "SamplingReportDataView.h"
#include "orbitdataviewpanel.h"
#include "orbittablemodel.h"
#include "orbittreeview.h"
#include "types.h"
#include "ui_orbitsamplingreport.h"

OrbitSamplingReport::OrbitSamplingReport(QWidget* parent)
    : QWidget(parent), ui(new Ui::OrbitSamplingReport) {
  ui->setupUi(this);
  if (!m_SamplingReport || !m_SamplingReport->HasCallstacks()) {
    ui->NextCallstackButton->setEnabled(false);
    ui->PreviousCallstackButton->setEnabled(false);
  }

  QList<int> sizes;
  sizes.append(5000);
  sizes.append(5000);
  ui->splitter->setSizes(sizes);
}

OrbitSamplingReport::~OrbitSamplingReport() { delete ui; }

void OrbitSamplingReport::Initialize(DataView* callstack_data_view,
                                     const std::shared_ptr<SamplingReport>& report) {
  ui->CallstackTreeView->Initialize(callstack_data_view, SelectionType::kExtended,
                                    FontType::kDefault, false);
  m_SamplingReport = report;

  if (!report) return;

  m_SamplingReport->SetUiRefreshFunc([&]() { this->RefreshCallstackView(); });

  for (SamplingReportDataView& report_data_view : report->GetThreadReports()) {
    auto* tab = new QWidget();
    tab->setObjectName(QStringLiteral("tab"));

    auto* gridLayout_2 = new QGridLayout(tab);
    gridLayout_2->setObjectName(QStringLiteral("gridLayout_2"));
    auto* treeView = new OrbitDataViewPanel(tab);
    treeView->SetDataModel(&report_data_view);

    if (!report_data_view.IsSortingAllowed()) {
      treeView->GetTreeView()->setSortingEnabled(false);
    } else {
      int column = report_data_view.GetDefaultSortingColumn();
      Qt::SortOrder order =
          report_data_view.GetColumns()[column].initial_order == DataView::SortingOrder::kAscending
              ? Qt::AscendingOrder
              : Qt::DescendingOrder;
      treeView->GetTreeView()->sortByColumn(column, order);
    }

    treeView->setObjectName(QStringLiteral("treeView"));
    gridLayout_2->addWidget(treeView, 0, 0, 1, 1);
    treeView->Initialize(&report_data_view, SelectionType::kExtended, FontType::kDefault);
    treeView->GetTreeView()->header()->resizeSections(QHeaderView::ResizeToContents);
    treeView->GetTreeView()->SetIsMultiSelection(true);

    treeView->Link(ui->CallstackTreeView);

    // This is hack - it is needed to update ui when data changes
    // TODO: Remove this once model is implemented properly and there
    //  is no need for manual updates.
    m_OrbitDataViews.push_back(treeView);

    QString threadName = QString::fromStdString(report_data_view.GetName());
    ui->tabWidget->addTab(tab, threadName);
  }

  connect(ui->tabWidget, &QTabWidget::currentChanged, this,
          &OrbitSamplingReport::OnCurrentThreadTabChanged);
}

void OrbitSamplingReport::Deinitialize() {
  for (OrbitDataViewPanel* panel : m_OrbitDataViews) {
    panel->Deinitialize();
  }
  ui->CallstackTreeView->Deinitialize();
}

void OrbitSamplingReport::on_NextCallstackButton_clicked() {
  CHECK(m_SamplingReport != nullptr);
  m_SamplingReport->IncrementCallstackIndex();
  RefreshCallstackView();
}

void OrbitSamplingReport::on_PreviousCallstackButton_clicked() {
  CHECK(m_SamplingReport != nullptr);
  m_SamplingReport->DecrementCallstackIndex();
  RefreshCallstackView();
}

void OrbitSamplingReport::OnCurrentThreadTabChanged(int current_tab_index) {
  auto treeView = m_OrbitDataViews[current_tab_index];
  QModelIndexList index_list = treeView->GetTreeView()->selectionModel()->selectedIndexes();
  std::vector<int> row_list;
  for (QModelIndex& index : index_list) {
    row_list.push_back(index.row());
  }
  treeView->GetTreeView()->GetModel()->OnMultiRowsSelected(row_list);
  RefreshCallstackView();
}

void OrbitSamplingReport::RefreshCallstackView() {
  if (m_SamplingReport == nullptr) {
    return;
  }

  ui->NextCallstackButton->setEnabled(m_SamplingReport->HasCallstacks());
  ui->PreviousCallstackButton->setEnabled(m_SamplingReport->HasCallstacks());

  std::string label = m_SamplingReport->GetSelectedCallstackString();
  ui->CallStackLabel->setText(QString::fromStdString(label));
  ui->CallstackTreeView->Refresh();
}

void OrbitSamplingReport::RefreshTabs() {
  if (m_SamplingReport == nullptr) {
    return;
  }

  for (OrbitDataViewPanel* panel : m_OrbitDataViews) {
    panel->Refresh();
  }
}
