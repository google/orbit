//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Path.h"
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

std::wstring Path::m_BasePath;
bool Path::m_IsPackaged;

//-----------------------------------------------------------------------------
void Path::Init() { GetBasePath(); }

//-----------------------------------------------------------------------------
std::wstring Path::GetExecutableName() {
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
  return exeFullName;
#else
  char result[PATH_MAX];
  ssize_t count = readlink("/proc/self/exe", result, PATH_MAX);
  std::string fullPath = std::string(result, (count > 0) ? count : 0);
  return s2ws(fullPath);
#endif
}

//-----------------------------------------------------------------------------
std::wstring Path::GetExecutablePath() {
  std::wstring fullPath = GetExecutableName();
  std::wstring path = fullPath.substr(0, fullPath.find_last_of(L"/")) + L"/";
  return path;
}

//-----------------------------------------------------------------------------
bool Path::FileExists(const std::wstring& a_File) {
  std::ifstream f(ws2s(a_File).c_str());
  return f.good();
}

//-----------------------------------------------------------------------------
uint64_t Path::FileSize(const std::string& a_File) {
  struct stat stat_buf;
  std::string filename = a_File;
  int rc = stat(filename.c_str(), &stat_buf);
  return rc == 0 ? stat_buf.st_size : -1;
}

//-----------------------------------------------------------------------------
bool Path::DirExists(const std::wstring& a_Dir) {
#if _WIN32
  DWORD ftyp = GetFileAttributesA(ws2s(a_Dir).c_str());
  if (ftyp == INVALID_FILE_ATTRIBUTES) return false;

  if (ftyp & FILE_ATTRIBUTE_DIRECTORY) return true;

  return false;
#else
  DIR* pDir;
  bool exists = false;
  pDir = opendir(ws2s(a_Dir).c_str());
  if (pDir != NULL) {
    exists = true;
    (void)closedir(pDir);
  }

  return exists;
#endif
}

//-----------------------------------------------------------------------------
std::wstring Path::GetBasePath() {
  if (m_BasePath.size() > 0) return m_BasePath;

  std::wstring exePath = GetExecutablePath();
  m_BasePath = exePath.substr(0, exePath.find(L"bin/"));
  m_IsPackaged = DirExists(GetBasePath() + L"text");

  return m_BasePath;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetOrbitAppPdb() {
  return GetBasePath() + std::wstring(L"bin/Win32/Debug/OrbitApp.pdb");
}

//-----------------------------------------------------------------------------
std::wstring Path::GetDllPath(bool a_Is64Bit) {
  std::wstring basePath = GetBasePath();

  return basePath + GetDllName(a_Is64Bit);
}

//-----------------------------------------------------------------------------
std::wstring Path::GetDllName(bool a_Is64Bit) {
  return a_Is64Bit ? L"Orbit64.dll" : L"Orbit32.dll";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetParamsFileName() {
  std::wstring paramsDir = Path::GetAppDataPath() + L"config/";
  _mkdir(ws2s(paramsDir).c_str());
  return paramsDir + L"config.xml";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetFileMappingFileName() {
  std::wstring paramsDir = Path::GetAppDataPath() + L"config/";
  _mkdir(ws2s(paramsDir).c_str());
  return paramsDir + std::wstring(L"FileMapping.txt");
}

//-----------------------------------------------------------------------------
std::wstring Path::GetSymbolsFileName() {
  std::wstring paramsDir = Path::GetAppDataPath() + L"config/";
  _mkdir(ws2s(paramsDir).c_str());
  return paramsDir + std::wstring(L"Symbols.txt");
}

//-----------------------------------------------------------------------------
std::wstring Path::GetLicenseName() {
  std::wstring appDataDir = Path::GetAppDataPath();
  _mkdir(ws2s(appDataDir).c_str());
  return appDataDir + std::wstring(L"user.txt");
}

//-----------------------------------------------------------------------------
std::wstring Path::GetCachePath() {
  std::wstring cacheDir = Path::GetAppDataPath() + L"cache/";
  _mkdir(ws2s(cacheDir).c_str());
  return cacheDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetPresetPath() {
  std::wstring presetDir = Path::GetAppDataPath() + L"presets/";
  _mkdir(ws2s(presetDir).c_str());
  return presetDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetPluginPath() {
  std::wstring presetDir = Path::GetAppDataPath() + L"plugins/";
  _mkdir(ws2s(presetDir).c_str());
  return presetDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetCapturePath() {
  std::wstring captureDir = Path::GetAppDataPath() + L"output/";
  _mkdir(ws2s(captureDir).c_str());
  return captureDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetDumpPath() {
  std::wstring captureDir = Path::GetAppDataPath() + L"dumps/";
  _mkdir(ws2s(captureDir).c_str());
  return captureDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetTmpPath() {
  std::wstring captureDir = Path::GetAppDataPath() + L"temp/";
  _mkdir(ws2s(captureDir).c_str());
  return captureDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetFileName(const std::wstring& a_FullName) {
  std::wstring FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of(L"/");
  if (index != std::wstring::npos) {
    std::wstring FileName = FullName.substr(FullName.find_last_of(L"/") + 1);
    return FileName;
  }

  return a_FullName;
}

//-----------------------------------------------------------------------------
std::string Path::GetFileName(const std::string& a_FullName) {
  std::string FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of("/");
  if (index != std::wstring::npos) {
    std::string FileName = FullName.substr(FullName.find_last_of("/") + 1);
    return FileName;
  }

  return a_FullName;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetFileNameNoExt(const std::wstring& a_FullName) {
  return StripExtension(GetFileName(a_FullName));
}

//-----------------------------------------------------------------------------
std::wstring Path::StripExtension(const std::wstring& a_FullName) {
  size_t index = a_FullName.find_last_of(L".");
  if (index != std::wstring::npos) return a_FullName.substr(0, index);
  return a_FullName;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetExtension(const std::wstring& a_FullName) {
  // returns ".ext" (includes point)
  size_t index = a_FullName.find_last_of(L".");
  if (index != std::wstring::npos)
    return a_FullName.substr(index, a_FullName.length());
  return L"";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetDirectory(const std::wstring& a_FullName) {
  std::wstring FullName = a_FullName;
  std::replace(FullName.begin(), FullName.end(), '\\', '/');
  auto index = FullName.find_last_of(L"/");
  if (index != std::string::npos) {
    std::wstring FileName = FullName.substr(0, FullName.find_last_of(L"/") + 1);
    return FileName;
  }

  return L"";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetParentDirectory(std::wstring a_Directory) {
  if (a_Directory.size() < 1) return L"";
  std::replace(a_Directory.begin(), a_Directory.end(), '\\', '/');
  wchar_t lastChar = a_Directory.c_str()[a_Directory.size() - 1];
  if (lastChar == '/') a_Directory.erase(a_Directory.size() - 1);

  return GetDirectory(a_Directory);
}

//-----------------------------------------------------------------------------
std::wstring Path::GetProgramFilesPath() {
#ifdef WIN32
  TCHAR pf[MAX_PATH] = {0};
  SHGetSpecialFolderPath(0, pf, CSIDL_PROGRAM_FILES, FALSE);
  return std::wstring(pf) + L"\\OrbitProfiler\\";
#else
  return L"TodoLinux";
#endif
}

//-----------------------------------------------------------------------------
std::wstring Path::GetAppDataPath() {
#ifdef WIN32
  std::string appData = GetEnvVar("APPDATA");
#else
  std::string appData = GetEnvVar("XDG_DATA_DIRS");
#endif
  std::string path = appData + "\\OrbitProfiler\\";
  _mkdir(path.c_str());
  return s2ws(path);
}

//-----------------------------------------------------------------------------
std::wstring Path::GetMainDrive() { return s2ws(GetEnvVar("SystemDrive")); }

//-----------------------------------------------------------------------------
std::wstring Path::GetSourceRoot() {
  std::wstring currentDir = GetExecutablePath();
  const int numIterations = 10;
  const std::wstring fileName = L"bootstrap-orbit";

  for (int i = 0; i < numIterations; ++i) {
    if (ContainsFile(currentDir, fileName)) {
      return currentDir;
    }

    currentDir = GetParentDirectory(currentDir);
  }

  return L"";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetExternalPath() {
  if (m_IsPackaged)
    return GetExecutablePath() + L"external/";
  else
    return GetSourceRoot() + L"external/";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetHome() {
  std::string home = GetEnvVar("HOME") + "//";
  return s2ws(home);
}

//-----------------------------------------------------------------------------
bool Path::ContainsFile(const std::wstring a_Dir, const std::wstring a_File) {
  auto fileList = ListFiles(a_Dir, a_File);
  for (const std::wstring& file : fileList) {
    if (Contains(file, a_File)) return true;
  }
  return false;
}

//-----------------------------------------------------------------------------
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

//-----------------------------------------------------------------------------
bool Path::IsSourceFile(const std::wstring& a_File) {
  std::wstring ext = Path::GetExtension(a_File);
  return ext == L".c" || ext == L".cpp" || ext == L".h" || ext == L".hpp" ||
         ext == L".inl" || ext == L".cxx";
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> Path::ListFiles(
    const std::wstring& a_Dir,
    std::function<bool(const std::wstring&)> a_Filter) {
  std::vector<std::wstring> files;

#ifdef _WIN32
  std::wstring pattern(a_Dir);
  pattern.append(L"\\*");
  WIN32_FIND_DATA data;
  HANDLE hFind;
  if ((hFind = FindFirstFile(pattern.c_str(), &data)) != INVALID_HANDLE_VALUE) {
    do {
      files.push_back(data.cFileName);
    } while (FindNextFile(hFind, &data) != 0);
    FindClose(hFind);
  }
#endif

  return files;
}

//-----------------------------------------------------------------------------
std::vector<std::wstring> Path::ListFiles(const std::wstring& a_Dir,
                                          const std::wstring& a_Filter) {
  return ListFiles(a_Dir, [&](const std::wstring& a_Name) {
    return Contains(a_Name, a_Filter);
  });
}
