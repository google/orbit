//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "ModuleManager.h"

#include "Capture.h"
#include "CoreApp.h"
#include "OrbitModule.h"
#include "OrbitProcess.h"
#include "Tcp.h"
#include "TcpServer.h"
#include "absl/strings/str_format.h"

ModuleManager GModuleManager;

//-----------------------------------------------------------------------------
ModuleManager::ModuleManager() { GPdbDbg = std::make_shared<Pdb>(L""); }

//-----------------------------------------------------------------------------
ModuleManager::~ModuleManager() {}

//-----------------------------------------------------------------------------
void ModuleManager::Init() {
  if (GTcpServer)
    GTcpServer->AddCallback(Msg_SetData, [=](const Message& a_Msg) {
      this->OnReceiveMessage(a_Msg);
    });
}

//-----------------------------------------------------------------------------
void ModuleManager::OnReceiveMessage(const Message& a_Msg) {
  if (a_Msg.GetType() == Msg_SetData) {
    const DataTransferHeader& header = a_Msg.GetHeader().m_DataTransferHeader;
    ULONG64 address =
        (ULONG64)header.m_Address - (ULONG64)GPdbDbg->GetHModule();
    DataTransferHeader::DataType dataType = header.m_Type;

    if (dataType == DataTransferHeader::Data) {
      // TODO: make access to watched vars thread safe
      for (std::shared_ptr<Variable> var :
           Capture::GTargetProcess->GetWatchedVariables()) {
        if (var->m_Address == address) {
          var->ReceiveValue(a_Msg);
          break;
        }
      }
    } else if (dataType == DataTransferHeader::Code) {
      Function* func = Capture::GTargetProcess->GetFunctionFromAddress(
          header.m_Address, true);
      std::string name = func ? func->PrettyName()
                              : absl::StrFormat("0x%" PRIx64, header.m_Address);

      GCoreApp->Disassemble(name, header.m_Address, a_Msg.GetData(),
                            a_Msg.m_Size);
    }
  }
}

//-----------------------------------------------------------------------------
void ModuleManager::LoadPdbAsync(const std::shared_ptr<Module>& a_Module,
                                 std::function<void()> a_CompletionCallback) {
  if (!a_Module->GetLoaded()) {
    bool loadExports = a_Module->IsDll() && !a_Module->m_FoundPdb;
    if (a_Module->m_FoundPdb || loadExports) {
      std::wstring pdbName =
          loadExports ? s2ws(a_Module->m_FullName) : s2ws(a_Module->m_PdbName);
      m_UserCompletionCallback = a_CompletionCallback;

      GPdbDbg = a_Module->m_Pdb;
      if (GPdbDbg) {
        GPdbDbg->SetMainModule((HMODULE)a_Module->m_AddressStart);
        GPdbDbg->LoadPdbAsync(pdbName.c_str(), [&]() { this->OnPdbLoaded(); });
      }
    }
  }
}

//-----------------------------------------------------------------------------
void ModuleManager::LoadPdbAsync(const std::vector<std::wstring> a_Modules,
                                 std::function<void()> a_CompletionCallback) {
  m_UserCompletionCallback = a_CompletionCallback;
  m_ModulesQueue = a_Modules;
  DequeueAndLoad();
}

//-----------------------------------------------------------------------------
void ModuleManager::DequeueAndLoad() {
  std::shared_ptr<Module> module = nullptr;

  while (module == nullptr && !m_ModulesQueue.empty()) {
    std::wstring pdbName = m_ModulesQueue.back();
    m_ModulesQueue.pop_back();

    module = Capture::GTargetProcess->FindModule(
        Path::GetFileName(ws2s(pdbName)));
    if (module) {
      GPdbDbg = module->m_Pdb;
      if (module->m_PdbName == "") {
        module->m_PdbName = module->m_FullName;
      }

      pdbName = s2ws(module->m_PdbName);
      GPdbDbg->LoadPdbAsync(pdbName.c_str(), [&]() { this->OnPdbLoaded(); });
      return;
    }
  }

  // No module was found, call user callback
  m_UserCompletionCallback();
}

//-----------------------------------------------------------------------------
void ModuleManager::OnPdbLoaded() {
  std::shared_ptr<Pdb> lastPdb = GPdbDbg;
  AddPdb(lastPdb);

  if (!m_ModulesQueue.empty()) {
    // Start loading next pdb on other thread
    DequeueAndLoad();
  }

#ifdef _WIN32
  // Apply presets on last pdb
  lastPdb->ApplyPresets();
#endif

  if (m_ModulesQueue.empty()) {
    m_UserCompletionCallback();
  }
}

//-----------------------------------------------------------------------------
void ModuleManager::AddPdb(const std::shared_ptr<Pdb>& a_Pdb) {
  std::map<uint64_t, std::shared_ptr<Module> >& modules =
      Capture::GTargetProcess->GetModules();

  auto it = modules.find((DWORD64)a_Pdb->GetHModule());
  if (it != modules.end()) {
    it->second->SetLoaded(true);
  }
}
