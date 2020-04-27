// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitStartupWindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDialogButtonBox>
#include <QGridLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QVector>

#include "OrbitBase/Logging.h"
#include "OrbitGgp/GgpClient.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpInstanceItemModel.h"

const GgpInstance localhost_placeholder_instance_ = {
    /* display_name */ "localhost",
    /* id */ "",
    /* ip_address */ "127.0.0.1",
    /* last_updated */ QDateTime{},
    /* owner */ "",
    /* pool */ ""};

OrbitStartupWindow::OrbitStartupWindow(QWidget* parent)
    : QDialog{parent, Qt::Dialog}, model_(QPointer(new GgpInstanceItemModel)) {
  // General UI
  const int width = 700;
  const int height = 400;
  setMinimumSize(QSize(width, height));
  setSizeGripEnabled(true);

  // Layout
  const auto layout = QPointer{new QGridLayout{this}};

  // Top label
  const auto label = QPointer{new QLabel{"Choose profiling target:"}};
  layout->addWidget(label, 0, 0);

  // Main content table
  const auto table_view = QPointer{new QTableView{}};
  table_view->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  table_view->setSelectionBehavior(
      QAbstractItemView::SelectionBehavior::SelectRows);
  table_view->viewport()->setFocusPolicy(Qt::NoFocus);
  table_view->horizontalHeader()->setStretchLastSection(true);
  table_view->setModel(model_);
  layout->addWidget(table_view, 1, 0);

  // Refresh Button
  const auto refresh_button = QPointer{new QPushButton{this}};
  refresh_button->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
  layout->addWidget(refresh_button, 1, 1, Qt::AlignTop);
  QObject::connect(refresh_button, &QPushButton::clicked, this,
                   &OrbitStartupWindow::ReloadInstances);

  // Ok / Cancel Buttons
  const auto button_box =
      QPointer{new QDialogButtonBox{QDialogButtonBox::StandardButton::Ok |
                                    QDialogButtonBox::StandardButton::Cancel}};
  button_box->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
  QObject::connect(button_box, &QDialogButtonBox::accepted, this,
                   &QDialog::accept);
  QObject::connect(button_box, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);
  layout->addWidget(button_box, 2, 0);

  // Logic for choosing a table item
  QObject::connect(
      table_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
      [&, button_box](const QModelIndex& current) {
        if (!current.isValid()) {
          chosen_instance_ = std::nullopt;
          return;
        }

        CHECK(current.model() == model_);
        chosen_instance_ = current.data(Qt::UserRole).value<GgpInstance>();
        button_box->button(QDialogButtonBox::StandardButton::Ok)
            ->setEnabled(true);
      });

  QObject::connect(table_view, &QTableView::doubleClicked, this,
                   &QDialog::accept);

  // Fill content table
  model_->SetInstances({localhost_placeholder_instance_});

  GgpClient::ResultOrQString<GgpClient> init_result = GgpClient::Create();
  if (!init_result) {
    QString error_string = QString(
                               "Orbit was unable to communicate with the GGP "
                               "command line tool. %1")
                               .arg(init_result.error());
    ERROR("%s", error_string.toStdString().c_str());
    refresh_button->setDisabled(true);
    refresh_button->setToolTip(error_string);
    return;
  }
  ggp_client_.emplace(std::move(init_result.value()));

  ReloadInstances();
}

void OrbitStartupWindow::ReloadInstances() {
  if (!ggp_client_) {
    ERROR("ggp client is not initialized");
    return;
  }

  if (ggp_client_->GetNumberOfRequestsRunning() > 0) return;

  ggp_client_->GetInstancesAsync(
      [&](GgpClient::ResultOrQString<QVector<GgpInstance>> instances) {
        if (!instances) {
          QMessageBox::critical(
              this, QApplication::applicationDisplayName(),
              QString(
                  "Orbit was unable to retrieve the list of available Stadia "
                  "Instances. The error message was: %1")
                  .arg(instances.error()));
          return;
        }

        if (!model_) return;
        QVector<GgpInstance> instances_with_localhost =
            std::move(instances.value());
        instances_with_localhost.push_back(localhost_placeholder_instance_);
        model_->SetInstances(std::move(instances_with_localhost));
      });
}

int OrbitStartupWindow::Run(std::string* ip_address) {
  // This blocks until the dialog box is dismissed
  int dialog_result = exec();

  if (dialog_result != 0) {
    CHECK(chosen_instance_.has_value());
    *ip_address = chosen_instance_->ip_address.toStdString();
  }
  return dialog_result;
}