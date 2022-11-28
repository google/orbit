// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "ConfigWidgets/SourcePathsMappingDialog.h"

#include <QAbstractButton>
#include <QDialogButtonBox>
#include <QFileDialog>
#include <QItemSelectionModel>
#include <QLineEdit>
#include <QListView>
#include <QMetaObject>
#include <QModelIndex>
#include <QModelIndexList>
#include <QPersistentModelIndex>
#include <QPushButton>
#include <QVariant>
#include <Qt>
#include <filesystem>

#include "ui_SourcePathsMappingDialog.h"

namespace orbit_config_widgets {

using orbit_source_paths_mapping::Mapping;

// The destructor needs to be defined here because it needs to see the type
// `Ui::SourcePathsMappingDialog`. The header file only contains a forward declaration.
SourcePathsMappingDialog::~SourcePathsMappingDialog() = default;

SourcePathsMappingDialog::SourcePathsMappingDialog(QWidget* parent) : QDialog{parent} {
  ui_ = std::make_unique<Ui::SourcePathsMappingDialog>();
  ui_->setupUi(this);

  ui_->list_view->setModel(&model_);

  QObject::connect(ui_->add_button, &QAbstractButton::clicked, this, [&]() {
    model_.AppendNewEmptyMapping();
    ui_->list_view->selectionModel()->clearSelection();
    ui_->list_view->selectionModel()->select(model_.index(model_.rowCount() - 1),
                                             QItemSelectionModel::ClearAndSelect);
  });

  QObject::connect(ui_->browse_button, &QAbstractButton::clicked, this, [&]() {
    QString path = QFileDialog::getExistingDirectory(this, "Choose target directory");
    if (path.isEmpty()) return;
    ui_->target_line_edit->setText(path);
    OnTargetPathChanged(ui_->target_line_edit->text());
  });

  QObject::connect(ui_->remove_button, &QAbstractButton::clicked, this,
                   &SourcePathsMappingDialog::OnRemoveSelectedMapping);

  QObject::connect(ui_->list_view->selectionModel(), &QItemSelectionModel::selectionChanged, this,
                   &SourcePathsMappingDialog::OnSelectionChanged);
  OnSelectionChanged(ui_->list_view->selectionModel()->selection(), {});

  QObject::connect(ui_->source_line_edit, &QLineEdit::textEdited, this,
                   &SourcePathsMappingDialog::OnSourcePathChanged);
  QObject::connect(ui_->target_line_edit, &QLineEdit::textEdited, this,
                   &SourcePathsMappingDialog::OnTargetPathChanged);
}

void SourcePathsMappingDialog::OnSelectionChanged(const QItemSelection& selected,
                                                  const QItemSelection& deselected) {
  const bool has_selection = !selected.indexes().isEmpty();

  ui_->source_line_edit->setEnabled(has_selection);
  ui_->target_line_edit->setEnabled(has_selection);
  ui_->browse_button->setEnabled(has_selection);
  ui_->remove_button->setEnabled(has_selection);

  if (has_selection) {
    const auto* mapping = selected.indexes().first().data(Qt::UserRole).value<const Mapping*>();
    ui_->source_line_edit->setText(QString::fromStdString(mapping->source_path.string()));
    ui_->target_line_edit->setText(QString::fromStdString(mapping->target_path.string()));

    const bool valid_mapping_selected = mapping->IsValid();
    ui_->add_button->setEnabled(valid_mapping_selected);
    ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)
        ->setEnabled(valid_mapping_selected);
  } else {
    ui_->source_line_edit->setText({});
    ui_->target_line_edit->setText({});

    ui_->add_button->setEnabled(true);
    ui_->buttonBox->button(QDialogButtonBox::StandardButton::Ok)->setEnabled(true);
  }

  if (!deselected.isEmpty()) {
    const auto* mapping = deselected.indexes().first().data(Qt::UserRole).value<const Mapping*>();
    const bool invalid_mapping_deselected = !mapping->IsValid();
    if (!invalid_mapping_deselected) return;

    // This selection change could be triggered by a model change (i.e. removing rows),
    // so we can't make any changes to the model here (i.e. request another removal of rows),
    // but we can queue a request to be processed later.
    QMetaObject::invokeMethod(
        this,
        [this, idx = QPersistentModelIndex{deselected.indexes().first()}]() {
          if (idx.isValid()) model_.RemoveRows(idx.row(), 1);
        },
        Qt::QueuedConnection);
  }
}

void SourcePathsMappingDialog::OnSourcePathChanged(const QString& new_source) {
  auto indexes = ui_->list_view->selectionModel()->selectedIndexes();
  auto idx = indexes.first();

  Mapping mapping = *idx.data(Qt::UserRole).value<const Mapping*>();
  mapping.source_path = std::filesystem::path{new_source.toStdString()};
  model_.setData(indexes.first(), QVariant::fromValue(mapping));

  OnSelectionChanged(ui_->list_view->selectionModel()->selection(), {});
}

void SourcePathsMappingDialog::OnTargetPathChanged(const QString& new_target) {
  auto indexes = ui_->list_view->selectionModel()->selectedIndexes();
  auto idx = indexes.first();

  Mapping mapping = *idx.data(Qt::UserRole).value<const Mapping*>();
  mapping.target_path = std::filesystem::path{new_target.toStdString()};
  model_.setData(indexes.first(), QVariant::fromValue(mapping));

  OnSelectionChanged(ui_->list_view->selectionModel()->selection(), {});
}

void SourcePathsMappingDialog::OnRemoveSelectedMapping() {
  auto indexes = ui_->list_view->selectionModel()->selectedIndexes();
  auto idx = indexes.first();
  model_.RemoveRows(idx.row(), 1);
}
}  // namespace orbit_config_widgets