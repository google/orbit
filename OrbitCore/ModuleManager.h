//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Core.h"
#include "Pdb.h"

struct Module;

class ModuleManager {
 public:
  ModuleManager();
  ~ModuleManager();

  void Init();
  void OnReceiveMessage(const Message& a_Msg);
  void LoadPdbAsync(const std::shared_ptr<Module>& a_Module,
                    std::function<void()> a_CompletionCallback);
  void LoadPdbAsync(const std::vector<std::wstring> a_Modules,
                    std::function<void()> a_CompletionCallback);

 protected:
  void DequeueAndLoad();
  void OnPdbLoaded();
  void ApplyPresets(std::shared_ptr<Pdb>& a_Pdb);
  void AddPdb(const std::shared_ptr<Pdb>& a_Pdb);

 protected:
  std::function<void()> m_UserCompletionCallback;
  std::vector<std::wstring> m_ModulesQueue;
};

extern ModuleManager GModuleManager;
