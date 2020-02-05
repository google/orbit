//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Diff.h"

#include <fstream>

#include "Core.h"
#include "Params.h"
#include "Path.h"

#ifdef _WIN32
#include <Shellapi.h>
#endif

//-----------------------------------------------------------------------------
void Diff::Exec(const std::string& a_A, const std::string& a_B) {
  std::string fileNameA = Path::GetTmpPath() + "A.txt";
  std::string fileNameB = Path::GetTmpPath() + "B.txt";

  std::ofstream fileA;
  std::ofstream fileB;

  fileA.open(fileNameA);
  fileB.open(fileNameB);

  if (fileA.fail() || fileB.fail()) {
    fileA.close();
    fileB.close();
    return;
  }

  fileA << a_A;
  fileB << a_B;

  fileA.close();
  fileB.close();

  std::string args = GParams.m_DiffArgs;
  ReplaceStringInPlace(args, "%1", fileNameA);
  ReplaceStringInPlace(args, "%2", fileNameB);

#ifdef _WIN32
  ShellExecute(0, nullptr, s2ws(GParams.m_DiffExe).c_str(), s2ws(args).c_str(),
               0, SW_HIDE);
#else
  std::string cmd = GParams.m_DiffExe + " " + GParams.m_DiffArgs;
  system(cmd.c_str());
#endif
}
