#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "OrbitModule.h"
#include "Path.h"
#include "Pdb.h"
#include "SymbolHelper.h"

const std::string executable_directory =
    Path::GetExecutablePath() + "testdata/";

TEST(OrbitModule, Constructor) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;
  const uint64_t executable_size = 16616;

  uint64_t address_start = 0x700;  // sample test data
  uint64_t address_end = 0x1000;

  Module module(file_path, address_start, address_end);

  EXPECT_EQ(module.m_FullName, file_path);
  EXPECT_EQ(module.m_Name, executable_name);
  EXPECT_EQ(module.m_Directory, executable_directory);
  EXPECT_EQ(module.m_PdbSize, executable_size);

  EXPECT_EQ(module.m_AddressStart, address_start);
  EXPECT_EQ(module.m_AddressEnd, address_end);

  EXPECT_EQ(module.m_PrettyName, file_path);
  EXPECT_EQ(module.m_AddressRange, "[0000000000000700 - 0000000000001000]");

  EXPECT_TRUE(module.m_FoundPdb);

  EXPECT_EQ(module.m_Pdb, nullptr);
  EXPECT_EQ(module.m_PdbName, "");
  EXPECT_FALSE(module.GetLoaded());
}

TEST(OrbitModule, LoadFunctions) {
  const std::string executable_name = "hello_world_elf";
  const std::string file_path = executable_directory + executable_name;

  std::shared_ptr<Module> module = std::make_shared<Module>(file_path, 0, 0);

  SymbolHelper symbolHelper;
  ASSERT_TRUE(symbolHelper.LoadSymbolsIncludedInBinary(module));
  Pdb& pdb = *module->m_Pdb;

  // Check functions
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  EXPECT_EQ(functions.size(), 10);
  const Function* function = functions[0].get();

  EXPECT_EQ(function->Name(), "deregister_tm_clones");
  EXPECT_EQ(function->PrettyName(), "deregister_tm_clones");
  EXPECT_EQ(function->Address(), 0x1080);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), file_path + ":deregister_tm_clones");

  // This function points to .init section and should not have a Probe
  function = functions[4].get();
  EXPECT_EQ(function->Name(), "_init");
  EXPECT_EQ(function->PrettyName(), "_init");
  EXPECT_EQ(function->Address(), 0x1000);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), "");

  function = functions[9].get();
  EXPECT_EQ(function->Name(), "main");
  EXPECT_EQ(function->PrettyName(), "main");
  EXPECT_EQ(function->Address(), 0x1135);
  EXPECT_EQ(function->Size(), 35);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), file_path + ":main");
}

TEST(OrbitModule, GetFunctionFromExactAddress) {
  const std::string file_path = executable_directory + "hello_world_static_elf";

  std::shared_ptr<Module> module =
      std::make_shared<Module>(file_path, 0x400000, 0);

  SymbolHelper symbolHelper;
  ASSERT_TRUE(symbolHelper.LoadSymbolsIncludedInBinary(module));
  Pdb& pdb = *module->m_Pdb;

  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();
  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;
  const Function* function = pdb.GetFunctionFromExactAddress(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");

  EXPECT_EQ(pdb.GetFunctionFromExactAddress(__free_pc_addr), nullptr);
}

TEST(OrbitModule, GetFunctionFromProgramCounter) {
  const std::string file_path = executable_directory + "hello_world_static_elf";

  std::shared_ptr<Module> module =
      std::make_shared<Module>(file_path, 0x400000, 0);

  SymbolHelper symbolHelper;

  ASSERT_TRUE(symbolHelper.LoadSymbolsIncludedInBinary(module));
  Pdb& pdb = *module->m_Pdb;

  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();

  const std::vector<std::shared_ptr<Function>>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;

  const Function* function =
      pdb.GetFunctionFromProgramCounter(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");

  function = pdb.GetFunctionFromProgramCounter(__free_pc_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");
}
