//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ModulesDataView.h"

#include "App.h"
#include "Core.h"
#include "OrbitModule.h"

//-----------------------------------------------------------------------------
ModulesDataView::ModulesDataView() : DataView(DataViewType::MODULES) {
  GOrbitApp->RegisterModulesDataView(this);
}

//-----------------------------------------------------------------------------
const std::vector<DataView::Column>& ModulesDataView::GetColumns() {
  static const std::vector<Column> columns = [] {
    std::vector<Column> columns;
    columns.resize(COLUMN_NUM);
    columns[COLUMN_INDEX] = {"Index", .0f, SortingOrder::Ascending};
    columns[COLUMN_NAME] = {"Name", .2f, SortingOrder::Ascending};
    columns[COLUMN_PATH] = {"Path", .3f, SortingOrder::Ascending};
    columns[COLUMN_ADDRESS_RANGE] = {"Address Range", .15f,
                                     SortingOrder::Ascending};
    columns[COLUMN_HAS_PDB] = {"Debug info", .0f, SortingOrder::Descending};
    columns[COLUMN_PDB_SIZE] = {"Pdb Size", .0f, SortingOrder::Descending};
    columns[COLUMN_LOADED] = {"Loaded", .0f, SortingOrder::Descending};
    return columns;
  }();
  return columns;
}

//-----------------------------------------------------------------------------
std::string ModulesDataView::GetValue(int row, int col) {
  const std::shared_ptr<Module>& module = GetModule(row);

  switch (col) {
    case COLUMN_INDEX:
      return std::to_string(row);
    case COLUMN_NAME:
      return module->m_Name;
    case COLUMN_PATH:
      return module->m_FullName;
    case COLUMN_ADDRESS_RANGE:
      return module->m_AddressRange;
    case COLUMN_HAS_PDB:
      return module->m_FoundPdb ? "*" : "";
    case COLUMN_PDB_SIZE:
      return module->m_FoundPdb ? GetPrettySize(module->m_PdbSize) : "";
    case COLUMN_LOADED:
      return module->GetLoaded() ? "*" : "";
    default:
      return "";
  }
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(m_Modules[a]->Member, m_Modules[b]->Member, \
                               ascending);                                 \
  }

//-----------------------------------------------------------------------------
void ModulesDataView::DoSort() {
  bool ascending = m_SortingOrders[m_SortingColumn] == SortingOrder::Ascending;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (m_SortingColumn) {
    case COLUMN_NAME:
      sorter = ORBIT_PROC_SORT(m_Name);
      break;
    case COLUMN_PATH:
      sorter = ORBIT_PROC_SORT(m_FullName);
      break;
    case COLUMN_ADDRESS_RANGE:
      sorter = ORBIT_PROC_SORT(m_AddressStart);
      break;
    case COLUMN_HAS_PDB:
      sorter = ORBIT_PROC_SORT(m_FoundPdb);
      break;
    case COLUMN_PDB_SIZE:
      sorter = ORBIT_PROC_SORT(m_PdbSize);
      break;
    case COLUMN_LOADED:
      sorter = ORBIT_PROC_SORT(GetLoaded());
      break;
    default:
      break;
  }

  if (sorter) {
    std::stable_sort(m_Indices.begin(), m_Indices.end(), sorter);
  }
}

//-----------------------------------------------------------------------------
const std::string ModulesDataView::MENU_ACTION_MODULES_LOAD = "Load Symbols";
const std::string ModulesDataView::MENU_ACTION_DLL_FIND_PDB = "Find Pdb";
const std::string ModulesDataView::MENU_ACTION_DLL_EXPORTS = "Load Symbols";

//-----------------------------------------------------------------------------
std::vector<std::string> ModulesDataView::GetContextMenu(
    int a_ClickedIndex, const std::vector<int>& a_SelectedIndices) {
  bool enable_load = false;
  bool enable_dll = false;
  if (a_SelectedIndices.size() == 1) {
    std::shared_ptr<Module> module = GetModule(a_SelectedIndices[0]);
    if (!module->GetLoaded()) {
      if (module->m_FoundPdb) {
        enable_load = true;
      } else if (module->IsDll()) {
        enable_dll = true;
      }
    }
  } else {
    for (int index : a_SelectedIndices) {
      std::shared_ptr<Module> module = GetModule(index);
      if (!module->GetLoaded() && module->m_FoundPdb) {
        enable_load = true;
      }
    }
  }

  std::vector<std::string> menu;
  if (enable_load) {
    menu.emplace_back(MENU_ACTION_MODULES_LOAD);
  }
  if (enable_dll) {
    Append(menu, {MENU_ACTION_DLL_FIND_PDB, MENU_ACTION_DLL_EXPORTS});
  }
  Append(menu, DataView::GetContextMenu(a_ClickedIndex, a_SelectedIndices));
  return menu;
}

//-----------------------------------------------------------------------------
void ModulesDataView::OnContextMenu(const std::string& a_Action,
                                    int a_MenuIndex,
                                    const std::vector<int>& a_ItemIndices) {
  if (a_Action == MENU_ACTION_MODULES_LOAD) {
    for (int index : a_ItemIndices) {
      const std::shared_ptr<Module>& module = GetModule(index);

      if (module->m_FoundPdb || module->IsDll()) {
        std::map<uint64_t, std::shared_ptr<Module> >& processModules =
            m_Process->GetModules();
        auto it = processModules.find(module->m_AddressStart);
        if (it != processModules.end()) {
          std::shared_ptr<Module>& mod = it->second;

          if (!mod->GetLoaded()) {
            GOrbitApp->EnqueueModuleToLoad(mod);
          }
        }
      }
    }

    GOrbitApp->LoadModules();
  } else if (a_Action == MENU_ACTION_DLL_FIND_PDB) {
    std::string FileName =
        ws2s(GOrbitApp->FindFile(L"Find Pdb File", L"", L"*.pdb"));
    // TODO: the result is unused, should this action be removed?
  } else if (a_Action == MENU_ACTION_DLL_EXPORTS) {
    // TODO: this action is unused, should it be removed?
  } else {
    DataView::OnContextMenu(a_Action, a_MenuIndex, a_ItemIndices);
  }
}

//-----------------------------------------------------------------------------
void ModulesDataView::OnTimer() {}

//-----------------------------------------------------------------------------
void ModulesDataView::DoFilter() {
  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = Tokenize(ToLower(m_Filter));

  for (int i = 0; i < (int)m_Modules.size(); ++i) {
    std::shared_ptr<Module>& module = m_Modules[i];
    std::string name = ToLower(module->GetPrettyName());

    bool match = true;

    for (std::string& filterToken : tokens) {
      if (name.find(filterToken) == std::string::npos) {
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
void ModulesDataView::SetProcess(const std::shared_ptr<Process>& a_Process) {
  m_Modules.clear();
  m_Process = a_Process;

  for (auto& it : a_Process->GetModules()) {
    it.second->GetPrettyName();
    m_Modules.push_back(it.second);
  }

  int numModules = (int)m_Modules.size();
  m_Indices.resize(numModules);
  for (int i = 0; i < numModules; ++i) {
    m_Indices[i] = i;
  }

  OnDataChanged();
}

//-----------------------------------------------------------------------------
const std::shared_ptr<Module>& ModulesDataView::GetModule(
    unsigned int a_Row) const {
  return m_Modules[m_Indices[a_Row]];
}

//-----------------------------------------------------------------------------
bool ModulesDataView::GetDisplayColor(int a_Row, int /*a_Column*/,
                                      unsigned char& r, unsigned char& g,
                                      unsigned char& b) {
  if (GetModule(a_Row)->GetLoaded()) {
    static unsigned char R = 42;
    static unsigned char G = 218;
    static unsigned char B = 130;
    r = R;
    g = G;
    b = B;
    return true;
  } else if (GetModule(a_Row)->m_FoundPdb) {
    static unsigned char R = 42;
    static unsigned char G = 130;
    static unsigned char B = 218;
    r = R;
    g = G;
    b = B;
    return true;
  }

  return false;
}
