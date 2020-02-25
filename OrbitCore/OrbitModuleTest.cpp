#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "Path.h"
#include "Pdb.h"

TEST(OrbitModule, LoadPdb) {
  std::string executable_path = Path::GetExecutablePath();
  // This is a simple executable that prints "Hello Earh!"
  std::string test_elf_file = executable_path + "/testdata/hello_world_elf";

  Pdb pdb;
  ASSERT_TRUE(pdb.LoadFunctions(test_elf_file.c_str()));

  // Check functions
  const std::vector<Function>& functions = pdb.GetFunctions();

  EXPECT_EQ(functions.size(), 10);
  const Function* function = &functions[0];

  EXPECT_EQ(function->Name(), "deregister_tm_clones");
  EXPECT_EQ(function->PrettyName(), "deregister_tm_clones");
  EXPECT_EQ(function->Address(), 0x1080);
  EXPECT_EQ(function->Size(), 0);
  EXPECT_EQ(function->GetPdb(), &pdb);
  EXPECT_EQ(function->Probe(), test_elf_file + ":deregister_tm_clones");

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
  EXPECT_EQ(function->Probe(), test_elf_file + ":main");
}
