// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "OrbitStartupWindow.h"

#include <QAbstractItemView>
#include <QApplication>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QGridLayout>
#include <QHeaderView>
#include <QItemSelectionModel>
#include <QLabel>
#include <QMessageBox>
#include <QPushButton>
#include <QTableView>
#include <QVector>

#include "OrbitBase/Logging.h"
#include "Path.h"

namespace OrbitQt {

OrbitStartupWindow::OrbitStartupWindow(QWidget* parent)
    : QDialog{parent, Qt::Dialog}, model_{new InstanceItemModel{{}, this}} {
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

  // Refresh Button
  refresh_button_ = QPointer{new QPushButton{this}};
  refresh_button_->setIcon(QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
  layout->addWidget(refresh_button_, 0, 1, Qt::AlignRight);
  QObject::connect(refresh_button_, &QPushButton::clicked, this, [this]() { ReloadInstances(); });

  // Main content table
  const auto table_view = QPointer{new QTableView{}};
  table_view->setSelectionMode(QAbstractItemView::SelectionMode::SingleSelection);
  table_view->setSelectionBehavior(QAbstractItemView::SelectionBehavior::SelectRows);
  table_view->viewport()->setFocusPolicy(Qt::NoFocus);
  table_view->horizontalHeader()->setStretchLastSection(true);
  table_view->setModel(model_);
  layout->addWidget(table_view, 1, 0, 1, 2);

  // Ok / Cancel Buttons
  const auto button_box = QPointer{new QDialogButtonBox{QDialogButtonBox::StandardButton::Reset |
                                                        QDialogButtonBox::StandardButton::Ok |
                                                        QDialogButtonBox::StandardButton::Cancel}};
  // An instance needs to be chosen before the ok button is enabled.
  button_box->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

  // Open Capture button
  // We use the Reset button role for the load capture button since it's in all
  // styles located on the left.
  const auto load_capture_button = button_box->button(QDialogButtonBox::StandardButton::Reset);
  CHECK(load_capture_button);
  load_capture_button->setIcon(QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  load_capture_button->setText("Load Capture");

  QObject::connect(load_capture_button, &QPushButton::clicked, this, [this, button_box]() {
    const QString file = QFileDialog::getOpenFileName(
        this, "Open capture...", QString::fromStdString(Path::CreateOrGetCaptureDir()), "*.orbit");
    if (!file.isEmpty()) {
      result_ = file;
      accept();
    }
  });

  QObject::connect(button_box, &QDialogButtonBox::accepted, this, [this, button_box]() {
    button_box->button(QDialogButtonBox::StandardButton::Ok)->setText("Loading...");
    button_box->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);
    button_box->button(QDialogButtonBox::StandardButton::Reset)->setEnabled(false);
    CHECK(chosen_instance_);
    ggp_client_->GetSshInfoAsync(
        *chosen_instance_, [this, button_box](outcome::result<SshInfo> ssh_info) {
          // this callback is only called when ggp_client still exists.
          button_box->button(QDialogButtonBox::StandardButton::Ok)->setText("Ok");
          button_box->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
          button_box->button(QDialogButtonBox::StandardButton::Reset)->setEnabled(true);
          if (!ssh_info) {
            QMessageBox::critical(this, QApplication::applicationDisplayName(),
                                  QString("Orbit was unable to retrieve the information "
                                          "necessary to connect via ssh. The error message "
                                          "was: %1")
                                      .arg(QString::fromStdString(ssh_info.error().message())));
          } else {
            result_ = std::move(ssh_info.value());
            accept();
          }
        });
  });

  QObject::connect(button_box, &QDialogButtonBox::rejected, this, &QDialog::reject);
  layout->addWidget(button_box, 2, 0, 1, 2, Qt::AlignRight);

  // Logic for choosing a table item
  QObject::connect(table_view->selectionModel(), &QItemSelectionModel::currentChanged, this,
                   [&, button_box](const QModelIndex& current) {
                     if (!current.isValid()) {
                       chosen_instance_ = std::nullopt;
                       return;
                     }

                     CHECK(current.model() == model_);
                     chosen_instance_ = current.data(Qt::UserRole).value<Instance>();
                     button_box->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
                   });

  QObject::connect(table_view, &QTableView::doubleClicked, button_box, &QDialogButtonBox::accepted);

  // Fill content table
  model_->SetInstances({});
}

void OrbitStartupWindow::ReloadInstances() {
  CHECK(ggp_client_);

  refresh_button_->setEnabled(false);
  refresh_button_->setText("Loading...");

  ggp_client_->GetInstancesAsync([this](outcome::result<QVector<Instance>> instances) {
    refresh_button_->setEnabled(true);
    refresh_button_->setText("");

    if (!instances) {
      QMessageBox::critical(this, QApplication::applicationDisplayName(),
                            QString("Orbit was unable to retrieve the list of available Stadia "
                                    "instances. The error message was: %1")
                                .arg(QString::fromStdString(instances.error().message())));
    } else {
      model_->SetInstances(std::move(instances.value()));
    }
  });
}

}  // namespace OrbitQt
