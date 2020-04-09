//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ModulesDataView.h"

#include "App.h"
#include "Core.h"
#include "OrbitModule.h"

//-----------------------------------------------------------------------------
ModulesDataView::ModulesDataView() {
  InitColumnsIfNeeded();
  m_SortingOrders.insert(m_SortingOrders.end(), s_InitialOrders.begin(),
                         s_InitialOrders.end());

  GOrbitApp->RegisterModulesDataView(this);
}

//-----------------------------------------------------------------------------
std::vector<std::string> ModulesDataView::s_Headers;
std::vector<float> ModulesDataView::s_HeaderRatios;
std::vector<DataView::SortingOrder> ModulesDataView::s_InitialOrders;

//-----------------------------------------------------------------------------
void ModulesDataView::InitColumnsIfNeeded() {
  if (s_Headers.empty()) {
    s_Headers.emplace_back("Index");
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Name");
    s_HeaderRatios.push_back(0.2f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Path");
    s_HeaderRatios.push_back(0.3f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Address Range");
    s_HeaderRatios.push_back(0.15f);
    s_InitialOrders.push_back(AscendingOrder);

    s_Headers.emplace_back("Debug info");
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back("Pdb Size");
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);

    s_Headers.emplace_back("Loaded");
    s_HeaderRatios.push_back(0);
    s_InitialOrders.push_back(DescendingOrder);
  }
}

//-----------------------------------------------------------------------------
const std::vector<std::string>& ModulesDataView::GetColumnHeaders() {
  return s_Headers;
}

//-----------------------------------------------------------------------------
const std::vector<float>& ModulesDataView::GetColumnHeadersRatios() {
  return s_HeaderRatios;
}

//-----------------------------------------------------------------------------
const std::vector<DataView::SortingOrder>&
ModulesDataView::GetColumnInitialOrders() {
  return s_InitialOrders;
}

//-----------------------------------------------------------------------------
int ModulesDataView::GetDefaultSortingColumn() { return MDV_PdbSize; }

//-----------------------------------------------------------------------------
std::string ModulesDataView::GetValue(int row, int col) {
  const std::shared_ptr<Module>& module = GetModule(row);
  std::string value;

  switch (col) {
    case MDV_Index:
      value = std::to_string((long)row);
      break;
    case MDV_ModuleName:
      value = module->m_Name;
      break;
    case MDV_Path:
      value = module->m_FullName;
      break;
    case MDV_AddressRange:
      value = module->m_AddressRange;
      break;
    case MDV_HasPdb:
      value = module->m_FoundPdb ? "*" : "";
      break;
    case MDV_PdbSize:
      value = module->m_FoundPdb ? GetPrettySize(module->m_PdbSize) : "";
      break;
    case MDV_Loaded:
      value = module->GetLoaded() ? "*" : "";
      break;
    default:
      break;
  }

  return value;
}

//-----------------------------------------------------------------------------
#define ORBIT_PROC_SORT(Member)                                            \
  [&](int a, int b) {                                                      \
    return OrbitUtils::Compare(m_Modules[a]->Member, m_Modules[b]->Member, \
                               ascending);                                 \
  }

//-----------------------------------------------------------------------------
void ModulesDataView::OnSort(int a_Column,
                             std::optional<SortingOrder> a_NewOrder) {
  auto mdvColumn = static_cast<MdvColumn>(a_Column);

  if (a_NewOrder.has_value()) {
    m_SortingOrders[mdvColumn] = a_NewOrder.value();
  }

  bool ascending = m_SortingOrders[mdvColumn] == AscendingOrder;
  std::function<bool(int a, int b)> sorter = nullptr;

  switch (mdvColumn) {
    case MDV_ModuleName:
      sorter = ORBIT_PROC_SORT(m_Name);
      break;
    case MDV_Path:
      sorter = ORBIT_PROC_SORT(m_FullName);
      break;
    case MDV_AddressRange:
      sorter = ORBIT_PROC_SORT(m_AddressStart);
      break;
    case MDV_HasPdb:
      sorter = ORBIT_PROC_SORT(m_FoundPdb);
      break;
    case MDV_PdbSize:
      sorter = ORBIT_PROC_SORT(m_PdbSize);
      break;
    case MDV_Loaded:
      sorter = ORBIT_PROC_SORT(GetLoaded());
      break;
    default:
      break;
  }

  if (sorter) {
    std::sort(m_Indices.begin(), m_Indices.end(), sorter);
  }

  m_LastSortedColumn = a_Column;
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
void ModulesDataView::OnFilter(const std::string& a_Filter) {
  std::vector<uint32_t> indices;
  std::vector<std::string> tokens = Tokenize(ToLower(a_Filter));

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

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
}

//-----------------------------------------------------------------------------
void ModulesDataView::SetProcess(std::shared_ptr<Process> a_Process) {
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

  if (m_LastSortedColumn != -1) {
    OnSort(m_LastSortedColumn, {});
  }
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
