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
    : QWidget(parent), ui_(new Ui::OrbitSamplingReport) {
  ui_->setupUi(this);
  if (!sampling_report_ || !sampling_report_->HasCallstacks()) {
    ui_->NextCallstackButton->setEnabled(false);
    ui_->PreviousCallstackButton->setEnabled(false);
  }

  QList<int> sizes;
  sizes.append(5000);
  sizes.append(5000);
  ui_->splitter->setSizes(sizes);
}

OrbitSamplingReport::~OrbitSamplingReport() { delete ui_; }

void OrbitSamplingReport::Initialize(DataView* callstack_data_view,
                                     const std::shared_ptr<SamplingReport>& report) {
  ui_->CallstackTreeView->Initialize(callstack_data_view, SelectionType::kExtended,
                                     FontType::kDefault, false);
  sampling_report_ = report;

  if (!report) return;

  sampling_report_->SetUiRefreshFunc([&]() { this->RefreshCallstackView(); });

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

    treeView->Link(ui_->CallstackTreeView);

    // This is hack - it is needed to update ui when data changes
    // TODO: Remove this once model is implemented properly and there
    //  is no need for manual updates.
    orbit_data_views_.push_back(treeView);

    QString thread_name = QString::fromStdString(report_data_view.GetName());
    ui_->tabWidget->addTab(tab, thread_name);
  }

  connect(ui_->tabWidget, &QTabWidget::currentChanged, this,
          &OrbitSamplingReport::OnCurrentThreadTabChanged);
}

void OrbitSamplingReport::Deinitialize() {
  for (OrbitDataViewPanel* panel : orbit_data_views_) {
    panel->Deinitialize();
  }
  ui_->CallstackTreeView->Deinitialize();
}

void OrbitSamplingReport::on_NextCallstackButton_clicked() {
  CHECK(sampling_report_ != nullptr);
  sampling_report_->IncrementCallstackIndex();
  RefreshCallstackView();
}

void OrbitSamplingReport::on_PreviousCallstackButton_clicked() {
  CHECK(sampling_report_ != nullptr);
  sampling_report_->DecrementCallstackIndex();
  RefreshCallstackView();
}

void OrbitSamplingReport::OnCurrentThreadTabChanged(int current_tab_index) {
  OrbitDataViewPanel* data_view = orbit_data_views_[current_tab_index];
  QModelIndexList index_list = data_view->GetTreeView()->selectionModel()->selectedIndexes();
  std::vector<int> row_list;
  for (QModelIndex& index : index_list) {
    row_list.push_back(index.row());
  }
  data_view->GetTreeView()->GetModel()->OnRowsSelected(row_list);
  RefreshCallstackView();
}

void OrbitSamplingReport::RefreshCallstackView() {
  if (sampling_report_ == nullptr) {
    return;
  }

  ui_->NextCallstackButton->setEnabled(sampling_report_->HasCallstacks());
  ui_->PreviousCallstackButton->setEnabled(sampling_report_->HasCallstacks());

  std::string label = sampling_report_->GetSelectedCallstackString();
  ui_->CallstackLabel->setText(QString::fromStdString(label));
  ui_->CallstackTreeView->Refresh();
}

void OrbitSamplingReport::RefreshTabs() {
  if (sampling_report_ == nullptr) {
    return;
  }

  for (OrbitDataViewPanel* panel : orbit_data_views_) {
    panel->Refresh();
  }
}
