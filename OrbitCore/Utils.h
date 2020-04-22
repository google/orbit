//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <xxhash.h>

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <codecvt>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <fstream>
#include <functional>
#include <iomanip>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

#include "BaseTypes.h"
#include "Platform.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"

//-----------------------------------------------------------------------------
inline std::string ws2s(const std::wstring& wstr) {
  std::string str;
  str.resize(wstr.size());
  for (std::size_t i = 0; i < str.size(); ++i) {
    str[i] = (char)wstr[i];
  }

  return str;
}

//-----------------------------------------------------------------------------
inline std::wstring s2ws(const std::string& str) {
  std::wstring wstr;
  wstr.resize(str.size());
  for (std::size_t i = 0; i < str.size(); ++i) {
    wstr[i] = str[i];
  }

  return wstr;
}

//-----------------------------------------------------------------------------
inline std::string GetEnvVar(const char* a_Var) {
  std::string var;

#ifdef _WIN32
  char* buf = nullptr;
  size_t sz = 0;
  if (_dupenv_s(&buf, &sz, a_Var) == 0 && buf != nullptr) {
    var = buf;
    free(buf);
  }
#else
  char* env = getenv(a_Var);
  if (env) var = env;
#endif

  return var;
}

//-----------------------------------------------------------------------------
inline unsigned long long StringHash(const std::string& a_String) {
  return XXH64(a_String.data(), a_String.size(), 0xBADDCAFEDEAD10CC);
}

//-----------------------------------------------------------------------------
inline unsigned long long StringHash(const std::wstring& a_String) {
  return XXH64(a_String.data(), a_String.size() * sizeof(wchar_t),
               0xBADDCAFEDEAD10CC);
}

#ifdef _WIN32
#define MemPrintf(Dest, DestSize, Source, ...) \
  _stprintf_s(Dest, DestSize, Source, __VA_ARGS__)
#define Log(Msg, ...) OrbitPrintf(Msg, __VA_ARGS__)
#endif

//-----------------------------------------------------------------------------
template <typename T, size_t N>
inline size_t SizeOfArray(const T (&)[N]) {
  return N;
}

template <typename T, typename U>
inline void Fill(T& a_Array, U& a_Value) {
  std::fill(std::begin(a_Array), std::end(a_Array), a_Value);
}

//-----------------------------------------------------------------------------
template <class T>
inline T ToLower(const T& a_Str) {
  T str = a_Str;
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

//-----------------------------------------------------------------------------
inline std::vector<std::string> Tokenize(std::string a_String,
                                         const char* a_Delimiters = " ") {
#ifdef _WIN32
#define strtok_r strtok_s
#endif

  std::vector<std::string> tokens;
  char* next_token;
  char* token = strtok_r(&a_String[0], a_Delimiters, &next_token);
  while (token != NULL) {
    tokens.push_back(token);
    token = strtok_r(NULL, a_Delimiters, &next_token);
  }

#ifdef _WIN32
#undef strtok_r
#endif

  return tokens;
}

//-----------------------------------------------------------------------------
inline std::vector<std::wstring> Tokenize(std::wstring a_String,
                                          const wchar_t* a_Delimiters = L" ") {
  std::vector<std::wstring> tokens;
  wchar_t* next_token;
  wchar_t* token = wcstok_s(&a_String[0], a_Delimiters, &next_token);
  while (token != NULL) {
    tokens.push_back(token);
    token = wcstok_s(NULL, a_Delimiters, &next_token);
  }

  return tokens;
}

//-----------------------------------------------------------------------------
inline bool IsDigit(const char value) { return std::isdigit(value); }

//-----------------------------------------------------------------------------
inline bool IsAllDigits(const std::string_view value) {
  return std::all_of(value.begin(), value.end(), IsDigit);
}

//-----------------------------------------------------------------------------
inline std::string FileToString(const std::string_view a_FileName) {
  std::string file_name(a_FileName);
  std::stringstream buffer;
  std::ifstream inFile(file_name);
  if (!inFile.fail()) {
    buffer << inFile.rdbuf();
    inFile.close();
  }
  return buffer.str();
}

//-----------------------------------------------------------------------------
template <class T>
inline void Append(std::vector<T>& a_Dest, const std::vector<T>& a_Source) {
  a_Dest.insert(std::end(a_Dest), std::begin(a_Source), std::end(a_Source));
}

//-----------------------------------------------------------------------------
inline void RemoveTrailingNewLine(std::string& a_String) {
  if (absl::EndsWith(a_String, "\n")) {
    a_String.pop_back();
  } else if (absl::EndsWith(a_String, "\r\n")) {
    a_String.pop_back();
    a_String.pop_back();
  }
}

//-----------------------------------------------------------------------------
inline void ReplaceStringInPlace(std::string& subject,
                                 const std::string& search,
                                 const std::string& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += std::max(replace.length(), (size_t)1);
  }
}

//-----------------------------------------------------------------------------
inline void ReplaceStringInPlace(std::wstring& subject,
                                 const std::wstring& search,
                                 const std::wstring& replace) {
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::wstring::npos) {
    subject.replace(pos, search.length(), replace);
    pos += std::max(replace.length(), (size_t)1);
  }
}

//-----------------------------------------------------------------------------
inline std::string Replace(const std::string& a_Subject,
                           const std::string& search,
                           const std::string& replace) {
  std::string subject = a_Subject;
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }

  return subject;
}

//-----------------------------------------------------------------------------
inline std::wstring Replace(const std::wstring& a_Subject,
                            const std::wstring& search,
                            const std::wstring& replace) {
  std::wstring subject = a_Subject;
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::wstring::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }

  return subject;
}

//-----------------------------------------------------------------------------
inline bool IsBlank(const std::string& a_Str) {
  return a_Str.find_first_not_of("\t\n ") == std::string::npos;
}

//-----------------------------------------------------------------------------
inline bool IsBlank(const std::wstring& a_Str) {
  return a_Str.find_first_not_of(L"\t\n ") == std::string::npos;
}

//-----------------------------------------------------------------------------
inline std::string LTrim(std::string str,
                         const std::string& chars = "\t\n\v\f\r ") {
  str.erase(0, str.find_first_not_of(chars));
  return str;
}

//-----------------------------------------------------------------------------
inline std::string RTrim(std::string str,
                         const std::string& chars = "\t\n\v\f\r ") {
  str.erase(str.find_last_not_of(chars) + 1);
  return str;
}

//-----------------------------------------------------------------------------
inline std::string trim(std::string str,
                        const std::string& chars = "\t\n\v\f\r ") {
  return LTrim(RTrim(str, chars), chars);
}

//-----------------------------------------------------------------------------
inline std::string XorString(std::string a_String) {
  const char* keys = "carkeys835fdda1";
  const size_t numKeys = strlen(keys);

  for (uint32_t i = 0; i < a_String.size(); i++) {
    a_String[i] = a_String[i] ^ keys[i % numKeys];
  }

  return a_String;
}

//-----------------------------------------------------------------------------
inline std::wstring XorString(std::wstring a_String) {
  const wchar_t* keys = L"carkeys835fdda1";
  const size_t numKeys = wcslen(keys);

  for (uint32_t i = 0; i < a_String.size(); i++) {
    a_String[i] = a_String[i] ^ keys[i % numKeys];
  }

  return a_String;
}

//-----------------------------------------------------------------------------
std::string GetLastErrorAsString();

//-----------------------------------------------------------------------------
std::string GuidToString(GUID a_Guid);

//-----------------------------------------------------------------------------
inline uint64_t GetMicros(std::string a_TimeStamp) {
  Replace(a_TimeStamp, ":", "");
  std::vector<std::string> tokens = Tokenize(a_TimeStamp, ".");
  if (tokens.size() != 2) {
    return 0;
  }

  uint64_t seconds = atoi(tokens[0].c_str());
  uint64_t micros = atoi(tokens[1].c_str());
  return seconds * 1000000 + micros;
}

//-----------------------------------------------------------------------------
inline void OrbitSleepMs(uint64_t a_Ms) {
#if __linux__
  usleep(a_Ms * 1000);
#else
  Sleep(a_Ms);
#endif
}

//-----------------------------------------------------------------------------
inline void PrintBuffer(const void* a_Buffer, uint32_t a_Size,
                        uint32_t a_Width = 16) {
  unsigned char* buffer = (unsigned char*)a_Buffer;
  for (size_t i = 0; i < a_Size; ++i) {
    std::cout << std::hex << std::setfill('0') << std::setw(2) << (int)buffer[i]
              << " ";

    if ((i + 1) % a_Width == 0) {
      std::cout << std::endl;
    }
  }

  std::cout << std::endl;

  for (size_t i = 0; i < a_Size; ++i) {
    std::cout << buffer[i];

    if ((i + 1) % a_Width == 0) {
      std::cout << std::endl;
    }
  }

  std::cout << std::endl;
}

#ifdef _WIN32
//-----------------------------------------------------------------------------
template <typename T>
inline std::string ToHexString(T a_Value) {
  std::stringstream l_StringStream;
  l_StringStream << std::hex << a_Value;
  return l_StringStream.str();
}

//-----------------------------------------------------------------------------
inline LONGLONG FileTimeDiffInMillis(const FILETIME& a_T0,
                                     const FILETIME& a_T1) {
  __int64 i0 = (__int64(a_T0.dwHighDateTime) << 32) + a_T0.dwLowDateTime;
  __int64 i1 = (__int64(a_T1.dwHighDateTime) << 32) + a_T1.dwLowDateTime;
  return (i1 - i0) / 10000;
}

//-----------------------------------------------------------------------------
class CWindowsMessageToString {
 public:
  static std::string GetStringFromMsg(DWORD dwMessage, bool = true);
};
#endif

//-----------------------------------------------------------------------------
inline std::string GetPrettySize(uint64_t size) {
  constexpr double KB = 1024.0;
  constexpr double MB = 1024.0 * KB;
  constexpr double GB = 1024.0 * MB;
  constexpr double TB = 1024.0 * GB;

  if (size < KB) return absl::StrFormat("%" PRIu64 " B", size);
  if (size < MB) return absl::StrFormat("%.2f KB", size / KB);
  if (size < GB) return absl::StrFormat("%.2f MB", size / MB);
  if (size < TB) return absl::StrFormat("%.2f GB", size / GB);

  return absl::StrFormat("%.2f TB", size / TB);
}

//-----------------------------------------------------------------------------
inline std::string GetPrettyTime(double milliseconds) {
  constexpr double Day = 24 * 60 * 60 * 1000;
  constexpr double Hour = 60 * 60 * 1000;
  constexpr double Minute = 60 * 1000;
  constexpr double Second = 1000;
  constexpr double Milli = 1;
  constexpr double Micro = 0.001;
  constexpr double Nano = 0.000001;

  std::string res;

  if (milliseconds < Micro) {
    res = absl::StrFormat("%.3f ns", milliseconds / Nano);
  } else if (milliseconds < Milli) {
    res = absl::StrFormat("%.3f us", milliseconds / Micro);
  } else if (milliseconds < Second) {
    res = absl::StrFormat("%.3f ms", milliseconds);
  } else if (milliseconds < Minute) {
    res = absl::StrFormat("%.3f s", milliseconds / Second);
  } else if (milliseconds < Hour) {
    res = absl::StrFormat("%.3f min", milliseconds / Minute);
  } else if (milliseconds < Day) {
    res = absl::StrFormat("%.3f h", milliseconds / Hour);
  } else {
    res = absl::StrFormat("%.3f days", milliseconds / Day);
  }

  return res;
}

//-----------------------------------------------------------------------------
inline std::string GetPrettyBitRate(uint64_t size_in_bytes) {
  uint64_t size = 8 * size_in_bytes;

  constexpr double KB = 1024.0;
  constexpr double MB = 1024.0 * KB;
  constexpr double GB = 1024.0 * MB;
  constexpr double TB = 1024.0 * GB;

  if (size < KB) return absl::StrFormat("%" PRIu64 " bit/s", size);
  if (size < MB) return absl::StrFormat("%.2f kbit/s", size / KB);
  if (size < GB) return absl::StrFormat("%.2f Mbit/s", size / MB);
  if (size < TB) return absl::StrFormat("%.2f Gbit/s", size / GB);

  return absl::StrFormat("%.2f Tbit/s", size / TB);
}

#ifndef WIN32
inline void fopen_s(FILE** fp, const char* fileName, const char* mode) {
  *(fp) = fopen(fileName, mode);
}
#endif

namespace OrbitUtils {
bool VisualStudioOpenFile(char const* a_Filename, unsigned int a_Line);

//-----------------------------------------------------------------------------
template <class T>
inline bool Compare(const T& a, const T& b, bool asc) {
  return asc ? a < b : a > b;
}

//-----------------------------------------------------------------------------
template <class T>
inline bool CompareAsc(const T& a, const T& b) {
  return a < b;
}

//-----------------------------------------------------------------------------
template <class T>
inline bool CompareDesc(const T& a, const T& b) {
  return a > b;
}

//-----------------------------------------------------------------------------
template <>
inline bool Compare<std::string>(const std::string& a, const std::string& b,
                                 bool asc) {
  return asc ? a < b : a > b;
}

//-----------------------------------------------------------------------------
template <class Key, class Val>
std::vector<std::pair<Key, Val> > ValueSort(
    std::unordered_map<Key, Val>& a_Map,
    std::function<bool(const Val&, const Val&)> a_SortFunc = nullptr) {
  typedef std::pair<Key, Val> PairType;
  std::vector<PairType> vec;
  vec.reserve(a_Map.size());

  for (auto& it : a_Map) {
    vec.push_back(it);
  }

  if (a_SortFunc)
    std::sort(vec.begin(), vec.end(),
              [&a_SortFunc](const PairType& a, const PairType& b) {
                return a_SortFunc(a.second, b.second);
              });
  else
    std::sort(vec.begin(), vec.end(),
              [&a_SortFunc](const PairType& a, const PairType& b) {
                return a.second < b.second;
              });

  return vec;
}

//-----------------------------------------------------------------------------
template <class Key, class Val>
std::vector<std::pair<Key, Val> > ValueSort(
    std::map<Key, Val>& a_Map,
    std::function<bool(const Val&, const Val&)> a_SortFunc = nullptr) {
  typedef std::pair<Key, Val> PairType;
  std::vector<PairType> vec;
  vec.reserve(a_Map.size());

  for (auto& it : a_Map) {
    vec.push_back(it);
  }

  if (a_SortFunc)
    std::sort(vec.begin(), vec.end(),
              [&a_SortFunc](const PairType& a, const PairType& b) {
                return a_SortFunc(a.second, b.second);
              });
  else
    std::sort(vec.begin(), vec.end(), [](const PairType& a, const PairType& b) {
      return a.second < b.second;
    });

  return vec;
}

//-----------------------------------------------------------------------------
template <class Key, class Val>
std::vector<std::pair<Key, Val> > ReverseValueSort(
    std::unordered_map<Key, Val>& a_Map) {
  std::function<bool(const Val&, const Val&)> sortFunc =
      [](const Val& a, const Val& b) { return a > b; };
  return ValueSort(a_Map, sortFunc);
}

//-----------------------------------------------------------------------------
template <class Key, class Val>
std::vector<std::pair<Key, Val> > ReverseValueSort(std::map<Key, Val>& a_Map) {
  std::function<bool(const Val&, const Val&)> sortFunc =
      [](const Val& a, const Val& b) { return a > b; };
  return ValueSort(a_Map, sortFunc);
}

std::string GetTimeStamp();
inline std::wstring GetTimeStampW() { return s2ws(GetTimeStamp()); }
std::string FormatTime(const time_t& rawtime);
}  // namespace OrbitUtils

//-----------------------------------------------------------------------------
bool ReadProcessMemory(uint32_t pid, uint64_t address, byte* buffer,
                       uint64_t size, size_t* num_bytes_read);

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)
#define UNIQUE_VAR CONCAT(Unique, __LINE__)
#define UNIQUE_ID CONCAT(Id_, __LINE__)
#define UNUSED(x) (void)(x)

#if __linux__
#define FUNCTION_NAME __PRETTY_FUNCTION__
#else
#define FUNCTION_NAME __FUNCTION__
#endif
