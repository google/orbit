// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gtest/gtest.h>

#include <filesystem>

#include "OrbitBase/Logging.h"
#include "Path.h"
#include "absl/strings/match.h"

TEST(Path, FileExistsEmptyFilename) {
  std::string filename{};
  EXPECT_FALSE(std::filesystem::exists(filename));
}

TEST(Path, FileExistsRootDir) {
  std::string filename;
#ifdef _WIN32
  filename = "C:\\";
#else
  filename = "/";
#endif
  EXPECT_TRUE(std::filesystem::exists(filename));
}

#ifndef _WIN32
TEST(Path, FileExistsDevNull) {
  std::string filename = "/dev/null";
  EXPECT_TRUE(std::filesystem::exists(filename));
}
#endif

TEST(Path, JoinPathPartsEmpty) {
  std::string actual = Path::JoinPath({});
  std::string expected{};
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsRelativeSlash) {
  std::string actual = Path::JoinPath({"dir1/dir2", "dir3/dir4/", "dir5/dir6"});
#ifdef _WIN32
  std::string expected = "dir1/dir2\\dir3/dir4/dir5/dir6";
#else
  std::string expected = "dir1/dir2/dir3/dir4/dir5/dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteSlash) {
  std::string actual = Path::JoinPath({"/dir1/dir2", "dir3/dir4/", "dir5/dir6"});
#ifdef _WIN32
  std::string expected = "/dir1/dir2\\dir3/dir4/dir5/dir6";
#else
  std::string expected = "/dir1/dir2/dir3/dir4/dir5/dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteRootSlash) {
  std::string actual = Path::JoinPath({"/", "dir1/dir2", "dir3/dir4"});
#ifdef _WIN32
  std::string expected = "/dir1/dir2\\dir3/dir4";
#else
  std::string expected = "/dir1/dir2/dir3/dir4";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsRelativeBackslash) {
  std::string actual = Path::JoinPath({"dir1\\dir2", "dir3\\dir4\\", "dir5\\dir6"});
#ifdef _WIN32
  std::string expected = "dir1\\dir2\\dir3\\dir4\\dir5\\dir6";
#else
  std::string expected = "dir1\\dir2/dir3\\dir4\\/dir5\\dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteBackslash) {
  std::string actual = Path::JoinPath({"C:\\dir1\\dir2", "dir3\\dir4\\", "dir5\\dir6"});
#ifdef _WIN32
  std::string expected = "C:\\dir1\\dir2\\dir3\\dir4\\dir5\\dir6";
#else
  std::string expected = "C:\\dir1\\dir2/dir3\\dir4\\/dir5\\dir6";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, JoinPathPartsAbsoluteRootBackslash) {
  std::string actual = Path::JoinPath({"C:", "dir1\\dir2", "dir3\\dir4"});
#ifdef _WIN32
  std::string expected = "C:dir1\\dir2\\dir3\\dir4";
#else
  std::string expected = "C:/dir1\\dir2/dir3\\dir4";
#endif
  EXPECT_EQ(actual, expected);
}

TEST(Path, FileNamesAndExtensions) {
#ifdef _WIN32
  std::string abs_input[] = {"C:\\some_abs_path\\file.txt", "C:\\dir.with.dots\\file_without_ext"};
  std::string abs_stripped[] = {"C:/some_abs_path/file", "C:/dir.with.dots/file_without_ext"};
#else
  std::string abs_input[] = {"/some_abs_path/file.txt", "/dir.with.dots/file_without_ext"};
  std::string abs_stripped[] = {"/some_abs_path/file", "/dir.with.dots/file_without_ext"};
#endif
  EXPECT_EQ(Path::GetExtension(abs_input[0]), ".txt");
  EXPECT_EQ(Path::StripExtension(abs_input[0]), abs_stripped[0]);
  EXPECT_EQ(Path::GetFileName(abs_input[0]), "file.txt");

  EXPECT_EQ(Path::GetExtension(abs_input[1]), "");
  EXPECT_EQ(Path::StripExtension(abs_input[1]), abs_stripped[1]);
  EXPECT_EQ(Path::GetFileName(abs_input[1]), "file_without_ext");

  std::string input = "relative\\mixed/path/file.txt";
  EXPECT_EQ(Path::GetExtension(input), ".txt");
  EXPECT_EQ(Path::StripExtension(input), "relative/mixed/path/file");
  EXPECT_EQ(Path::GetFileName(input), "file.txt");

  input = "simple.file";
  EXPECT_EQ(Path::GetExtension(input), ".file");
  EXPECT_EQ(Path::StripExtension(input), "simple");
  EXPECT_EQ(Path::GetFileName(input), "simple.file");

  input = "no/ext";
  EXPECT_EQ(Path::GetExtension(input), "");
  EXPECT_EQ(Path::StripExtension(input), "no/ext");
  EXPECT_EQ(Path::GetFileName(input), "ext");

  input = "double.ext.txt";
  EXPECT_EQ(Path::GetExtension(input), ".txt");
  EXPECT_EQ(Path::StripExtension(input), "double.ext");
  EXPECT_EQ(Path::StripExtension(Path::StripExtension(input)), "double");
  EXPECT_EQ(Path::GetFileName(input), "double.ext.txt");
}

// TODO: A test for GetExecutableDir is missing - using this here makes this test
// kind of invalid, since GetExecutableDir is using GetExecutablePath internally...
TEST(Path, GetExecutablePath) {
  std::string actual = Path::GetExecutablePath();
#ifdef _WIN32
  std::string expected = Path::JoinPath({Path::GetExecutableDir(), "OrbitCoreTests.exe"});
#else
  std::string expected = Path::JoinPath({Path::GetExecutableDir(), "OrbitCoreTests"});
#endif
  EXPECT_EQ(actual, expected);
}

static std::string GetTestDataAbsPath(const std::string& filename = "") {
  if (filename.empty()) {
    return Path::JoinPath({Path::GetExecutableDir(), "testdata", "OrbitCore"});
  } else {
    return Path::JoinPath({Path::GetExecutableDir(), "testdata", "OrbitCore", filename});
  }
}

TEST(Path, GetDirectoryAndParent) {
  struct Test {
    std::string input;
    std::string dir;
    std::string parent_of_input;
    std::string parent_of_dir;
  };

  // The hoops I'm jumping through to have the test declarations below nicely formatted...
  using V = std::vector<Test>;
#ifdef _WIN32
  V tests = {{"C:\\dir/With\\fwd_slash.txt.ext", "C:/dir/With/", "C:/dir/With/", "C:/dir/"},
             {"C:\\no\\ext\\file", "C:/no/ext/", "C:/no/ext/", "C:/no/"},
             {"C:\\dir\\subdir\\", "C:/dir/subdir/", "C:/dir/", "C:/dir/"},
             {"C:\\file_in_root", "C:/", "C:/", ""},
             {"broken_path", "", "", ""},
             {"C:\\", "C:/", "", ""}};
#else
  V tests = {{"/dir/With/mult_ext.txt.ext", "/dir/With/", "/dir/With/", "/dir/"},
             {"/no/ext/file", "/no/ext/", "/no/ext/", "/no/"},
             {"/dir/subdir/", "/dir/subdir/", "/dir/", "/dir/"},
             {"/file_in_root", "/", "/", ""},
             {"broken_path", "", "", ""},
             {"/", "/", "", ""}};
#endif

  for (auto& test : tests) {
    LOG("Checking directory and parent of \"%s\"", test.input.c_str());
    const std::string result_dir = Path::GetDirectory(test.input);
    EXPECT_EQ(result_dir, test.dir);
    EXPECT_EQ(Path::GetParentDirectory(test.input), test.parent_of_input);
    EXPECT_EQ(Path::GetParentDirectory(result_dir), test.parent_of_dir);
  }
}

TEST(Path, ListFiles) {
  using Set = std::set<std::string>;

  auto empty_file_path = GetTestDataAbsPath("empty_file.txt");
  auto small_file_path = GetTestDataAbsPath("small_file");

  auto files = Path::ListFiles(GetTestDataAbsPath());
  Set unordered_files(files.begin(), files.end());
  EXPECT_EQ(unordered_files, Set({empty_file_path, small_file_path}));

  // Filter is NOT a regex...
  files = Path::ListFiles(GetTestDataAbsPath(), "*.txt");
  EXPECT_TRUE(files.empty());

  // ... but simply a "contains"
  files = Path::ListFiles(GetTestDataAbsPath(), ".txt");
  unordered_files = Set(files.begin(), files.end());
  EXPECT_EQ(unordered_files, Set({empty_file_path}));

  // Filter lambda gets the absolute file path as a parameter
  files = Path::ListFiles(GetTestDataAbsPath(), [](const std::string& file) -> bool {
    return file == GetTestDataAbsPath("small_file");
  });
  unordered_files = Set(files.begin(), files.end());
  EXPECT_EQ(unordered_files, Set({small_file_path}));
}

TEST(Path, AllAutoCreatedDirsExist) {
  auto test_fns = {Path::CreateOrGetOrbitAppDataDir, Path::CreateOrGetDumpDir,
                   Path::CreateOrGetPresetDir, Path::CreateOrGetCacheDir,
                   Path::CreateOrGetOrbitAppDataDir};

  for (auto fn : test_fns) {
    std::string path = fn();
    LOG("Testing existence of \"%s\"", path.c_str());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Path, AllCopiedDirsExist) {
  auto test_fns = {Path::GetIconsPath};

  for (auto fn : test_fns) {
    std::string path = fn();
    LOG("Testing existence of \"%s\"", path.c_str());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}

TEST(Path, AllDirsOfFilesExist) {
  auto test_fns = {Path::GetLogFilePathAndCreateDir};

  for (auto fn : test_fns) {
    std::string path = Path::GetDirectory(fn());
    LOG("Testing existence of \"%s\"", path.c_str());
    EXPECT_TRUE(std::filesystem::is_directory(path));
  }
}
