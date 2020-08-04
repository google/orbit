// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "PresetsDataView.h"

#include <OrbitBase/Logging.h>
#include <OrbitBase/SafeStrerror.h>

#include <cstdio>

#include "App.h"
#include "Callstack.h"
#include "Capture.h"
#include "Core.h"
#include "ModulesDataView.h"
#include "Pdb.h"
#include "absl/strings/str_format.h"

using orbit_client_protos::PresetFile;

//-----------------------------------------------------------------------------
PresetsDataView::PresetsDataView() : DataView(DataViewType::PRESETS) {}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& PresetsDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_SESSION_NAME] = {"Preset", .49f, SortingOrder::Ascending};
    columns[COLUMN_PROCESS_NAME] = {"Process", .49f, SortingOrder::Ascending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string PresetsDataView::GetValue(int row, int col) {
  const std::shared_ptr<PresetFile>& preset = GetPreset(row);

  switch (col) {
    case COLUMN_SESSION_NAME:
      return Path::GetFileName(preset->file_name());
    case COLUMN_PROCESS_NAME:
      return Path::GetFileName(preset->preset_info().process_full_path());
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
std::string PresetsDataView::GetToolTip(int a_Row, int /*a_Column*/) {
  const PresetFile& preset = *GetPreset(a_Row);
  return preset.file_name();
}

//-----------------------------------------------------------------------------
#define ORBIT_PRESET_SORT(Member)                                        \
  [&](int a, int b) {                                                    \
    return OrbitUtils::Compare(presets_[a]->Member, presets_[b]->Member, \
                               ascending);                               \
  }

//-----------------------------------------------------------------------------
void PresetsDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (m_SortingColumn) {
    case COLUMN_SESSION_NAME:
      sorter = ORBIT_PRESET_SORT(file_name());
      break;
    case COLUMN_PROCESS_NAME:
      sorter = ORBIT_PRESET_SORT(preset_info().process_full_path());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
const std::string PresetsDataView::MENU_ACTION_LOAD = "Load Preset";
const std::string PresetsDataView::MENU_ACTION_DELETE = "Delete Preset";

//-----------------------------------------------------------------------------
std::vector<std::string> PresetsDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  std::vector<std::string> menu;
  // Note that the UI already enforces a single selection.
  if (a_SelectedIndices.size() == 1) {
    Append(menu, {MENU_ACTION_LOAD, MENU_ACTION_DELETE});
  }
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void PresetsDataView::OnContextMenu(const std::string& a_Action,
                                    int a_MenuIndex,
                                    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_LOAD) {
    if (a_ItemIndices.size() != 1) {
      return;
    }
    const std::shared_ptr<PresetFile>& preset = GetPreset(a_ItemIndices[0]);

    GOrbitApp->LoadPreset(preset);

  } else if (a_Action == MENU_ACTION_DELETE) {
    if (a_ItemIndices.size() != 1) {
      return;
    }
    int row = a_ItemIndices[0];
    const std::shared_ptr<PresetFile>& preset = GetPreset(row);
    const std::string& filename = preset->file_name();
    int ret = remove(filename.c_str());
    if (ret == 0) {
      presets_.erase(presets_.begin() + m_Indices[row]);
      OnDataChanged();
    } else {
      ERROR("Deleting preset \"%s\": %s", filename, SafeStrerror(errno));
      GOrbitApp->SendErrorToUi(
          "Error deleting preset",
          absl::StrFormat("Could not delete preset \"%s\".", filename));
    }

  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void PresetsDataView::DoFilter() {
  std::vector<uint32_t> indices;

  std::vector<std::string> tokens = absl::StrSplit(ToLower(m_Filter), ' ');

  for (size_t i = 0; i < presets_.size(); ++i) {
    const PresetFile& preset = *presets_[i];
    std::string name = Path::GetFileName(ToLower(preset.file_name()));
    std::string path = ToLower(preset.preset_info().process_full_path());

    bool match = true;

    for (std::string& filterToken : tokens) {
      if (!(name.find(filterToken) != std::string::npos ||
            path.find(filterToken) != std::string::npos)) {
        match = false;
        break;
      }
    }

    if (match) {
      indices.push_back(i);
    }
  }

  m_Indices = indices;

  OnSort(m_SortingColumn, {});
}

//-----------------------------------------------------------------------------
void PresetsDataView::OnDataChanged() {
  m_Indices.resize(presets_.size());
  for (size_t i = 0; i < presets_.size(); ++i) {
    m_Indices[i] = i;
  }

  DataView::OnDataChanged();
}

//-----------------------------------------------------------------------------
void PresetsDataView::SetPresets(
    const std::vector<std::shared_ptr<PresetFile> >& presets) {
  presets_ = presets;
  OnDataChanged();
}

//-----------------------------------------------------------------------------
const std::shared_ptr<PresetFile>& PresetsDataView::GetPreset(
    unsigned int a_Row) const {
  return presets_[m_Indices[a_Row]];
}
