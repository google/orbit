#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "Path.h"
#include "Pdb.h"

TEST(OrbitModule, LoadPdb) {
  const std::string executable_path = Path::GetExecutablePath();
  // This is a simple executable that prints "Hello Earh!"
  const std::string file_path = executable_path + "testdata/hello_world_elf";

  Pdb pdb;
  ASSERT_TRUE(pdb.LoadFunctions(file_path.c_str()));

  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), file_path);
  EXPECT_EQ(pdb.GetName(), "hello_world_elf");

  // Check functions
  const std::vector<Function>& functions = pdb.GetFunctions();

  EXPECT_EQ(functions.size(), 10);
  const Function* function = &functions[0];

  EXPECT_EQ(function->Name(), "deregister_tm_clones");
  EXPECT_EQ(function->PrettyName(), "deregister_tm_clones");
  EXPECT_EQ(function->Address(), 0x1080);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), file_path + ":deregister_tm_clones");

  // This function points to .init section and should not have a Probe
  function = &functions[4];
  EXPECT_EQ(function->Name(), "_init");
  EXPECT_EQ(function->PrettyName(), "_init");
  EXPECT_EQ(function->Address(), 0x1000);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), "");

  function = &functions[9];
  EXPECT_EQ(function->Name(), "main");
  EXPECT_EQ(function->PrettyName(), "main");
  EXPECT_EQ(function->Address(), 0x1135);
  EXPECT_EQ(function->Size(), 35);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), file_path + ":main");
}

TEST(OrbitModule, LoadPdbSeparateSymbols) {
  const std::string executable_path = Path::GetExecutablePath();
  const std::string file_path = executable_path + "testdata/no_symbols_elf";

  Pdb pdb;
  ASSERT_TRUE(pdb.LoadFunctions(file_path.c_str()));

  std::string symbols_file_name = "no_symbols_elf.debug";
  std::string symbols_path = executable_path + "testdata/" + symbols_file_name;

  EXPECT_EQ(pdb.GetLoadedModuleName(), file_path);
  EXPECT_EQ(pdb.GetFileName(), symbols_path);
  EXPECT_EQ(pdb.GetName(), symbols_file_name);
}

TEST(OrbitModule, GetFunctionFromExactAddress) {
  const std::string executable_path = Path::GetExecutablePath();
  const std::string file_path =
      executable_path + "testdata/hello_world_static_elf";

  Pdb pdb;
  ASSERT_TRUE(pdb.LoadFunctions(file_path.c_str()));
  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();
  pdb.SetMainModule(0x400000);
  const std::vector<Function>& functions = pdb.GetFunctions();

  ASSERT_EQ(functions.size(), 1125);

  constexpr const uint64_t __free_start_addr = 0x41b840;
  constexpr const uint64_t __free_pc_addr = 0x41b854;
  const Function* function = pdb.GetFunctionFromExactAddress(__free_start_addr);
  ASSERT_NE(function, nullptr);
  EXPECT_EQ(function->Name(), "__free");

  EXPECT_EQ(pdb.GetFunctionFromExactAddress(__free_pc_addr), nullptr);
}

TEST(OrbitModule, GetFunctionFromProgramCounter) {
  const std::string executable_path = Path::GetExecutablePath();
  const std::string file_path =
      executable_path + "testdata/hello_world_static_elf";

  Pdb pdb;
  ASSERT_TRUE(pdb.LoadFunctions(file_path.c_str()));
  pdb.PopulateFunctionMap();
  pdb.PopulateStringFunctionMap();
  pdb.SetMainModule(0x400000);
  const std::vector<Function>& functions = pdb.GetFunctions();

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

