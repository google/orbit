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
#include "OrbitGgp/GgpClient.h"
#include "OrbitGgp/GgpInstance.h"
#include "OrbitGgp/GgpInstanceItemModel.h"
#include "OrbitGgp/GgpSshInfo.h"
#include "Path.h"

OrbitStartupWindow::OrbitStartupWindow(QWidget* parent)
    : QDialog{parent, Qt::Dialog}, model_{new GgpInstanceItemModel{{}, this}} {
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
  const auto refresh_button = QPointer{new QPushButton{this}};
  refresh_button->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_BrowserReload));
  layout->addWidget(refresh_button, 0, 1, Qt::AlignRight);
  QObject::connect(
      refresh_button, &QPushButton::clicked, this,
      [this, refresh_button]() { ReloadInstances(refresh_button); });

  // Main content table
  const auto table_view = QPointer{new QTableView{}};
  table_view->setSelectionMode(
      QAbstractItemView::SelectionMode::SingleSelection);
  table_view->setSelectionBehavior(
      QAbstractItemView::SelectionBehavior::SelectRows);
  table_view->viewport()->setFocusPolicy(Qt::NoFocus);
  table_view->horizontalHeader()->setStretchLastSection(true);
  table_view->setModel(model_);
  layout->addWidget(table_view, 1, 0, 1, 2);

  // Ok / Cancel Buttons
  const auto button_box =
      QPointer{new QDialogButtonBox{QDialogButtonBox::StandardButton::Reset |
                                    QDialogButtonBox::StandardButton::Ok |
                                    QDialogButtonBox::StandardButton::Cancel}};
  button_box->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(false);

  // Open Capture button
  // We use the Reset button role for the load capture button since it's in all
  // styles located on the left.
  const auto load_capture_button =
      button_box->button(QDialogButtonBox::StandardButton::Reset);
  CHECK(load_capture_button);
  load_capture_button->setIcon(
      QApplication::style()->standardIcon(QStyle::SP_DialogOpenButton));
  load_capture_button->setText("Load Capture");

  QObject::connect(
      load_capture_button, &QPushButton::clicked, this, [this, button_box]() {
        const QString file = QFileDialog::getOpenFileName(
            this, "Open capture...",
            QString::fromStdString(Path::GetCapturePath()), "*.orbit");
        if (!file.isEmpty()) {
          result_ = file;
          accept();
        }
      });

  QObject::connect(
      button_box, &QDialogButtonBox::accepted, this, [this, button_box]() {
        button_box->button(QDialogButtonBox::StandardButton::Ok)
            ->setText("Loading...");
        button_box->button(QDialogButtonBox::StandardButton::Ok)
            ->setEnabled(false);
        button_box->button(QDialogButtonBox::StandardButton::Reset)
            ->setEnabled(false);
        CHECK(chosen_instance_);
        if (chosen_instance_->display_name == "localhost") {
          button_box->button(QDialogButtonBox::StandardButton::Ok)
              ->setText("Ok");
          this->accept();
        }
        CHECK(ggp_client_);
        const auto self = QPointer{this};
        ggp_client_->GetSshInformationAsync(
            *chosen_instance_,
            [self,
             button_box](GgpClient::ResultOrQString<GgpSshInfo> ssh_info) {
              // The dialog might not exist anymore when this callback returns.
              // So we have to check for this.
              if (self && button_box) {
                button_box->button(QDialogButtonBox::StandardButton::Ok)
                    ->setText("Ok");
                button_box->button(QDialogButtonBox::StandardButton::Ok)
                    ->setEnabled(true);
                button_box->button(QDialogButtonBox::StandardButton::Reset)
                    ->setEnabled(true);
                if (!ssh_info) {
                  QMessageBox::critical(
                      self, QApplication::applicationDisplayName(),
                      QString("Orbit was unable to retrieve the information "
                              "necessary to connect via ssh. The error message "
                              "was: %1")
                          .arg(ssh_info.error()));
                } else {
                  self->result_ = ssh_info.value();
                  self->accept();
                }
              }
            });
      });
  QObject::connect(button_box, &QDialogButtonBox::rejected, this,
                   &QDialog::reject);
  layout->addWidget(button_box, 2, 0, 1, 2, Qt::AlignRight);

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

  QObject::connect(table_view, &QTableView::doubleClicked, button_box,
                   &QDialogButtonBox::accepted);

  // Fill content table
  model_->SetInstances({});

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

  ReloadInstances(refresh_button);
}

void OrbitStartupWindow::ReloadInstances(QPointer<QPushButton> refresh_button) {
  if (!ggp_client_) {
    ERROR("ggp client is not initialized");
    return;
  }

  if (ggp_client_->GetNumberOfRequestsRunning() > 0) return;

  refresh_button->setEnabled(false);
  refresh_button->setText("Loading...");

  ggp_client_->GetInstancesAsync(
      [model = model_, refresh_button](
          GgpClient::ResultOrQString<QVector<GgpInstance>> instances) {
        if (refresh_button) {
          refresh_button->setEnabled(true);
          refresh_button->setText("");
        }

        if (!instances) {
          QMessageBox::critical(
              nullptr, QApplication::applicationDisplayName(),
              QString(
                  "Orbit was unable to retrieve the list of available Stadia "
                  "Instances. The error message was: %1")
                  .arg(instances.error()));
          return;
        }

        if (model) {
          model->SetInstances(std::move(instances.value()));
        }
      });
}
