//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Path.h"

#include <filesystem>
#include <fstream>

#include "PrintVar.h"
#include "Utils.h"

#ifdef _WIN32
#include <direct.h>

#include "Shlobj.h"
#else
#include <dirent.h>
#include <sys/stat.h>
#endif

std::string Path::base_path_;
bool Path::is_packaged_;

void Path::Init() { GetBasePath(); }

std::string Path::GetExecutableName() {
#ifdef _WIN32
  WCHAR cwBuffer[2048] = {0};
  LPWSTR pszBuffer = cwBuffer;
  DWORD dwMaxChars = _countof(cwBuffer);
  DWORD dwLength = 0;

  dwLength = ::GetModuleFileNameW(NULL, pszBuffer, dwMaxChars);
  std::wstring exeFullName = std::wstring(pszBuffer);

  // Clean up "../" inside full path
  wchar_t buffer[MAX_PATH];
  GetFullPathName(exeFullName.c_str(), MAX_PATH, buffer, nullptr);
  exeFullName = buffer;

  std::replace(exeFullName.begin(), exeFullName.end(), '\\', '/');
  return ws2s(exeFullName);
#else
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  std::string fullPath = std::string(result, (count > 0) ? count : 0);
  return fullPath;
#endif
}

std::string Path::GetExecutablePath() {
  std::string fullPath = GetExecutableName();
  std::string path = fullPath.substr(0, fullPath.find_last_of("/")) + "/";
  return path;
}

bool Path::FileExists(const std::string& file) {
  struct stat statbuf;
  int ret = stat(file.c_str(), &statbuf);
  return ret == 0;
}

uint64_t Path::FileSize(const std::string& file) {
  struct stat stat_buf;
  int ret = stat(file.c_str(), &stat_buf);
  return ret == 0 ? stat_buf.st_size : 0;
}

bool Path::DirExists(const std::string& dir) {
#if _WIN32
  DWORD ftyp = GetFileAttributesA(dir.c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES) return false;

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;

  return false;
#else
  // TODO: also use stat here?
  DIR* pDir;
  bool exists = false;
  pDir = opendir(dir.c_str());
  if (pDir != nullptr) {
    exists = true;
    (void)closedir(pDir);
  }

  return exists;
#endif
}

void Path::MakeDir(const std::string& a_Directory) {
  // TODO: Use std::filesystem::create_directory once when we have c++ 17.
#if _WIN32
  _mkdir(a_Directory.c_str());
#else
  mkdir(a_Directory.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH);
#endif
}

std::string Path::GetBasePath() {
  if (!base_path_.empty()) {
    return base_path_;
  }

  std::string exePath = GetExecutablePath();
  base_path_ = exePath.substr(0, exePath.find("bin/"));
  is_packaged_ = DirExists(GetBasePath() + "text");

  return base_path_;
}

std::string Path::GetOrbitAppPdb() {
  return GetBasePath() + std::string("bin/Win32/Debug/OrbitApp.pdb");
}

std::string Path::GetDllPath(bool a_Is64Bit) {
  std::string basePath = GetBasePath();

  return basePath + GetDllName(a_Is64Bit);
}

std::string Path::GetDllName(bool a_Is64Bit) {
  return a_Is64Bit ? "Orbit64.dll" : "Orbit32.dll";
}

std::string Path::GetParamsFileName() {
  std::string paramsDir = Path::GetAppDataPath() + "config/";
  Path::MakeDir(paramsDir);
  return paramsDir + "config.xml";
}

std::string Path::GetFileMappingFileName() {
  std::string paramsDir = Path::GetAppDataPath() + "config/";
  Path::MakeDir(paramsDir);
  return paramsDir + std::string("FileMapping.txt");
}

std::string Path::GetSymbolsFileName() {
  std::string paramsDir = Path::GetAppDataPath() + "config/";
  Path::MakeDir(paramsDir);
  return paramsDir + std::string("SymbolPaths.txt");
}

std::string Path::GetLicenseName() {
  std::string appDataDir = Path::GetAppDataPath();
  Path::MakeDir(appDataDir);
  return appDataDir + std::string("user.txt");
}

std::string Path::GetCachePath() {
  std::string cacheDir = Path::GetAppDataPath() + "cache/";
  Path::MakeDir(cacheDir);
  return cacheDir;
}

std::string Path::GetPresetPath() {
  std::string presetDir = Path::GetAppDataPath() + "presets/";
  Path::MakeDir(presetDir);
  return presetDir;
}

std::string Path::GetPluginPath() {
  std::string presetDir = Path::GetAppDataPath() + "plugins/";
  Path::MakeDir(presetDir);
  return presetDir;
}

std::string Path::GetCapturePath() {
  std::string captureDir = Path::GetAppDataPath() + "output/";
  Path::MakeDir(captureDir);
  return captureDir;
}

std::string Path::GetDumpPath() {
  std::string captureDir = Path::GetAppDataPath() + "dumps/";
  Path::MakeDir(captureDir);
  return captureDir;
}

std::string Path::GetTmpPath() {
  std::string captureDir = Path::GetAppDataPath() + "temp/";
  Path::MakeDir(captureDir);
  return captureDir;
}

std::string Path::GetFileName(const std::string& a_FullName) {
  std::string FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of("/");
  if (index != std::string::npos) {
    std::string FileName = FullName.substr(FullName.find_last_of("/") + 1);
    return FileName;
  }

  return a_FullName;
}

std::string Path::GetFileNameNoExt(const std::string& a_FullName) {
  return StripExtension(GetFileName(a_FullName));
}

std::string Path::StripExtension(const std::string& a_FullName) {
  size_t index = a_FullName.find_last_of(".");
  if (index != std::string::npos) return a_FullName.substr(0, index);
  return a_FullName;
}

std::string Path::GetExtension(const std::string& a_FullName) {
  // returns ".ext" (includes point)
  size_t index = a_FullName.find_last_of(".");
  if (index != std::string::npos)
    return a_FullName.substr(index, a_FullName.length());
  return "";
}

std::string Path::GetDirectory(const std::string& a_FullName) {
  std::string FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of("/");
  if (index != std::string::npos) {
    std::string FileName = FullName.substr(0, FullName.find_last_of("/") + 1);
    return FileName;
  }

  return "";
}

std::string Path::GetParentDirectory(std::string a_Directory) {
  if (a_Directory.size() < 1) return "";
  std::replace(a_Directory.begin(), a_Directory.end(), '\\', '/');
  wchar_t lastChar = a_Directory.c_str()[a_Directory.size() - 1];
  if (lastChar == '/') a_Directory.erase(a_Directory.size() - 1);

  return GetDirectory(a_Directory);
}

std::string Path::GetProgramFilesPath() {
#ifdef WIN32
  char pf[MAX_PATH] = {0};
  SHGetSpecialFolderPathA(0, pf, CSIDL_PROGRAM_FILES, FALSE);
  return std::string(pf) + "\\OrbitProfiler\\";
#else
  return "TodoLinux";
#endif
}

std::string Path::GetAppDataPath() {
#ifdef WIN32
  std::string appData = GetEnvVar("APPDATA");
  std::string path = appData + "\\OrbitProfiler\\";
#else
  std::string path = Path::GetHome() + ".orbitprofiler/";
#endif
  Path::MakeDir(path);
  return path;
}

std::string Path::GetMainDrive() { return GetEnvVar("SystemDrive"); }

std::string Path::GetSourceRoot() {
  // Assuming current file in <src_path>/OrbitCore
  std::string current_dir = GetDirectory(__FILE__);
  return GetParentDirectory(current_dir);
}

std::string Path::GetHome() {
  std::string home = GetEnvVar("HOME") + "/";
  return home;
}

void Path::Dump() {
  PRINT_VAR(GetExecutableName());
  PRINT_VAR(GetExecutablePath());
  PRINT_VAR(GetBasePath());
  PRINT_VAR(GetOrbitAppPdb());
  PRINT_VAR(GetDllPath(true));
  PRINT_VAR(GetDllName(true));
  PRINT_VAR(GetDllPath(false));
  PRINT_VAR(GetDllName(false));
  PRINT_VAR(GetParamsFileName());
  PRINT_VAR(GetFileMappingFileName());
  PRINT_VAR(GetSymbolsFileName());
  PRINT_VAR(GetLicenseName());
  PRINT_VAR(GetCachePath());
  PRINT_VAR(GetPresetPath());
  PRINT_VAR(GetPluginPath());
  PRINT_VAR(GetCapturePath());
  PRINT_VAR(GetDumpPath());
  PRINT_VAR(GetTmpPath());
  PRINT_VAR(GetProgramFilesPath());
  PRINT_VAR(GetAppDataPath());
  PRINT_VAR(GetMainDrive());
  PRINT_VAR(GetSourceRoot());
}

bool Path::IsSourceFile(const std::string& a_File) {
  std::string ext = Path::GetExtension(a_File);
  return ext == ".c" || ext == ".cpp" || ext == ".h" || ext == ".hpp" ||
         ext == ".inl" || ext == ".cxx" || ext == ".cc";
}

std::vector<std::string> Path::ListFiles(
    const std::string& directory,
    std::function<bool(const std::string&)> filter) {
  std::vector<std::string> files;

  for (const auto& file : std::filesystem::directory_iterator(directory)) {
    if (std::filesystem::is_regular_file(file)) {
      std::string path(file.path().string());
      if (filter(path)) files.push_back(path);
    }
  }

  return files;
}

std::vector<std::string> Path::ListFiles(const std::string& directory,
                                         const std::string& filter) {
  return ListFiles(directory, [&](const std::string& file_name) {
    return absl::StrContains(file_name, filter);
  });
}
