#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <memory>

#include "OrbitFunction.h"
#include "OrbitModule.h"
#include "Path.h"
#include "SymbolHelper.h"

const std::string executable_directory =
    Path::GetExecutablePath() + "testdata/";

TEST(SymbolHelper, LoadSymbolsIncludedInBinary) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);

  SymbolHelper symbolHelper;
  ASSERT_TRUE(symbolHelper.LoadSymbolsIncludedInBinary(module));

  EXPECT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, file_path);
  EXPECT_TRUE(module->GetLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), file_path);
  EXPECT_EQ(pdb.GetName(), executable_name);
}

TEST(SymbolHelper, LoadSymbolsCollectorSameFile) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);

  SymbolHelper symbolHelper({executable_directory}, {});
  ASSERT_TRUE(symbolHelper.LoadSymbolsCollector(module));

  EXPECT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, file_path);
  EXPECT_TRUE(module->GetLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), file_path);
  EXPECT_EQ(pdb.GetName(), executable_name);
}

TEST(SymbolHelper, LoadSymbolsCollectorSeparateFile) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);
  module->m_DebugSignature = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";

  std::string symbols_file_name = "no_symbols_elf.debug";
  std::string symbols_path = executable_directory + symbols_file_name;

  SymbolHelper symbolHelper({executable_directory}, {});
  ASSERT_TRUE(symbolHelper.LoadSymbolsCollector(module));

  EXPECT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, symbols_path);
  EXPECT_TRUE(module->GetLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), symbols_path);
  EXPECT_EQ(pdb.GetName(), symbols_file_name);
}

TEST(SymbolHelper, LoadSymbolsUsingSymbolsFile) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);
  module->m_DebugSignature = "b5413574bbacec6eacb3b89b1012d0e2cd92ec6b";

  std::string symbols_file_name = "no_symbols_elf.debug";
  std::string symbols_path = executable_directory + symbols_file_name;

  SymbolHelper symbolHelper({}, {executable_directory});
  ASSERT_TRUE(symbolHelper.LoadSymbolsUsingSymbolsFile(module));

  EXPECT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, symbols_path);
  EXPECT_TRUE(module->GetLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), symbols_path);
  EXPECT_EQ(pdb.GetName(), symbols_file_name);
}

TEST(SymbolHelper, LoadSymbolsFromDebugInfo) {
  const std::string executable_name = "no_symbols_elf";
  const std::string file_path = executable_directory + executable_name;

  ModuleDebugInfo module_info;
  module_info.m_Name = executable_name;
  module_info.m_PdbName = "path/symbols_file_name";
  module_info.load_bias = 0x400;
  Function function;
  function.SetName("function name");
  module_info.m_Functions = {function};

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0x40, 0);
  SymbolHelper symbolHelper;
  symbolHelper.LoadSymbolsFromDebugInfo(module, module_info);

  ASSERT_NE(module->m_Pdb, nullptr);
  EXPECT_EQ(module->m_PdbName, "path/symbols_file_name");
  EXPECT_TRUE(module->GetLoaded());

  Pdb& pdb = *module->m_Pdb;
  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), "path/symbols_file_name");
  EXPECT_EQ(pdb.GetName(), "symbols_file_name");
  EXPECT_EQ(pdb.GetHModule(), 0x40);
  EXPECT_EQ(pdb.GetLoadBias(), 0x400);
  EXPECT_EQ(pdb.GetFunctions()[0].Name(), "function name");
}

TEST(SymbolHelper, FillDebugInfoFromModule) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);
  SymbolHelper symbolHelper;
  ASSERT_TRUE(symbolHelper.LoadSymbolsIncludedInBinary(module));

  ModuleDebugInfo module_info;
  symbolHelper.FillDebugInfoFromModule(module, module_info);

  EXPECT_EQ(module_info.m_Name, executable_name);
  EXPECT_EQ(module_info.m_PdbName, file_path);
  EXPECT_EQ(module_info.load_bias, 0x0);
  EXPECT_EQ(module_info.m_Functions.size(), 10);
}
