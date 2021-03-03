// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_CORE_CORE_UTILS_H_
#define ORBIT_CORE_CORE_UTILS_H_

#include <absl/time/time.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/types.h>
#include <xxhash.h>

#include <algorithm>
#include <cctype>
#include <cinttypes>
#include <codecvt>
#include <cstdarg>
#include <cstdio>
#include <cstring>
#include <functional>
#include <iomanip>
#include <iterator>
#include <map>
#include <memory>
#include <outcome.hpp>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "OrbitBase/Logging.h"
#include "absl/strings/match.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"

#define CONCAT_(x, y) x##y
#define CONCAT(x, y) CONCAT_(x, y)
#define UNIQUE_ID CONCAT(Id_, __LINE__)
#define UNUSED(x) (void)(x)

inline std::string ws2s(const std::wstring& wstr) {
  std::string str;
  str.resize(wstr.size());
  for (std::size_t i = 0; i < str.size(); ++i) {
    str[i] = static_cast<char>(wstr[i]);
  }

  return str;
}

inline std::wstring s2ws(const std::string& str) {
  std::wstring wstr;
  wstr.resize(str.size());
  for (std::size_t i = 0; i < str.size(); ++i) {
    wstr[i] = str[i];
  }

  return wstr;
}

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

inline uint64_t StringHash(std::string_view str) {
  return XXH64(str.data(), str.size(), 0xBADDCAFEDEAD10CC);
}

template <typename T, typename U>
inline void Fill(T& a_Array, U& a_Value) {
  std::fill(std::begin(a_Array), std::end(a_Array), a_Value);
}

template <class T>
inline T ToLower(const T& a_Str) {
  T str = a_Str;
  std::transform(str.begin(), str.end(), str.begin(), ::tolower);
  return str;
}

template <class T>
inline void Append(std::vector<T>& a_Dest, const std::vector<T>& a_Source) {
  a_Dest.insert(std::end(a_Dest), std::begin(a_Source), std::end(a_Source));
}

inline std::string Replace(const std::string& a_Subject, const std::string& search,
                           const std::string& replace) {
  std::string subject = a_Subject;
  size_t pos = 0;
  while ((pos = subject.find(search, pos)) != std::string::npos) {
    subject.replace(pos, search.length(), replace);
    pos += replace.length();
  }

  return subject;
}

inline bool IsBlank(const std::string& a_Str) {
  return a_Str.find_first_not_of("\t\n ") == std::string::npos;
}

std::string GetLastErrorAsString();

inline void PrintBuffer(const void* a_Buffer, uint32_t a_Size, uint32_t a_Width = 16) {
  const auto* buffer = static_cast<const uint8_t*>(a_Buffer);
  std::stringstream buffer_string;
  for (size_t i = 0; i < a_Size; ++i) {
    buffer_string << std::hex << std::setfill('0') << std::setw(2) << buffer[i] << " ";

    if ((i + 1) % a_Width == 0) {
      buffer_string << std::endl;
    }
  }

  buffer_string << std::endl;

  for (size_t i = 0; i < a_Size; ++i) {
    buffer_string << buffer[i];

    if ((i + 1) % a_Width == 0) {
      buffer_string << std::endl;
    }
  }

  LOG("%s", buffer_string.str());
}

#ifdef _WIN32
template <typename T>
inline std::string ToHexString(T a_Value) {
  std::stringstream l_StringStream;
  l_StringStream << std::hex << a_Value;
  return l_StringStream.str();
}

inline LONGLONG FileTimeDiffInMillis(const FILETIME& a_T0, const FILETIME& a_T1) {
  __int64 i0 = (__int64(a_T0.dwHighDateTime) << 32) + a_T0.dwLowDateTime;
  __int64 i1 = (__int64(a_T1.dwHighDateTime) << 32) + a_T1.dwLowDateTime;
  return (i1 - i0) / 10000;
}

class CWindowsMessageToString {
 public:
  static std::string GetStringFromMsg(DWORD dwMessage, bool = true);
};
#endif

enum class EllipsisPosition { kMiddle };

inline std::string ShortenStringWithEllipsis(std::string_view text, size_t max_len,
                                             EllipsisPosition pos = EllipsisPosition::kMiddle) {
  // Parameter is mainly here to indicate how the util works,
  // and to be potentially extended later
  UNUSED(pos);
  constexpr const size_t kNumCharsEllipsis = 3;

  if (max_len <= kNumCharsEllipsis) {
    return text.length() <= kNumCharsEllipsis ? std::string(text) : "...";
  }
  if (text.length() <= max_len) {
    return std::string(text);
  }

  const size_t chars_to_cut = text.length() - max_len + kNumCharsEllipsis;
  size_t l = text.length() - chars_to_cut;
  // Integer division by two, rounded up
  if (l & 0x1) {
    l = (l + 1) >> 1;
  } else {
    l = l >> 1;
  }

  const size_t r = l + chars_to_cut;
  return std::string(text.substr(0, l)) + "..." + std::string(text.substr(r));
}

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

inline std::string GetPrettyTime(absl::Duration duration) {
  constexpr double Day = 24;

  std::string res;

  if (duration < absl::Microseconds(1)) {
    res = absl::StrFormat("%.3f ns", absl::ToDoubleNanoseconds(duration));
  } else if (duration < absl::Milliseconds(1)) {
    res = absl::StrFormat("%.3f us", absl::ToDoubleMicroseconds(duration));
  } else if (duration < absl::Seconds(1)) {
    res = absl::StrFormat("%.3f ms", absl::ToDoubleMilliseconds(duration));
  } else if (duration < absl::Minutes(1)) {
    res = absl::StrFormat("%.3f s", absl::ToDoubleSeconds(duration));
  } else if (duration < absl::Hours(1)) {
    res = absl::StrFormat("%.3f min", absl::ToDoubleMinutes(duration));
  } else if (duration < absl::Hours(Day)) {
    res = absl::StrFormat("%.3f h", absl::ToDoubleHours(duration));
  } else {
    res = absl::StrFormat("%.3f days", absl::ToDoubleHours(duration) / Day);
  }

  return res;
}

#ifndef WIN32
inline void fopen_s(FILE** fp, const char* fileName, const char* mode) {
  *(fp) = fopen(fileName, mode);
}
#endif

namespace orbit_core {

template <class T>
inline bool Compare(const T& a, const T& b, bool asc) {
  return asc ? a < b : a > b;
}

template <class T>
inline bool CompareAsc(const T& a, const T& b) {
  return a < b;
}

template <class T>
inline bool CompareDesc(const T& a, const T& b) {
  return a > b;
}

template <>
inline bool Compare<std::string>(const std::string& a, const std::string& b, bool asc) {
  return asc ? a < b : a > b;
}

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
    std::sort(vec.begin(), vec.end(), [&a_SortFunc](const PairType& a, const PairType& b) {
      return a_SortFunc(a.second, b.second);
    });
  else
    std::sort(vec.begin(), vec.end(),
              [](const PairType& a, const PairType& b) { return a.second < b.second; });

  return vec;
}

template <class Key, class Val>
std::vector<std::pair<Key, Val> > ValueSort(
    std::map<Key, Val>& a_Map, std::function<bool(const Val&, const Val&)> a_SortFunc = nullptr) {
  typedef std::pair<Key, Val> PairType;
  std::vector<PairType> vec;
  vec.reserve(a_Map.size());

  for (auto& it : a_Map) {
    vec.push_back(it);
  }

  if (a_SortFunc)
    std::sort(vec.begin(), vec.end(), [&a_SortFunc](const PairType& a, const PairType& b) {
      return a_SortFunc(a.second, b.second);
    });
  else
    std::sort(vec.begin(), vec.end(),
              [](const PairType& a, const PairType& b) { return a.second < b.second; });

  return vec;
}

template <class Key, class Val>
std::vector<std::pair<Key, Val> > ReverseValueSort(std::unordered_map<Key, Val>& a_Map) {
  std::function<bool(const Val&, const Val&)> sortFunc = [](const Val& a, const Val& b) {
    return a > b;
  };
  return ValueSort(a_Map, sortFunc);
}

template <class Key, class Val>
std::vector<std::pair<Key, Val> > ReverseValueSort(std::map<Key, Val>& a_Map) {
  std::function<bool(const Val&, const Val&)> sortFunc = [](const Val& a, const Val& b) {
    return a > b;
  };
  return ValueSort(a_Map, sortFunc);
}

std::string FormatTime(const time_t& rawtime);
}  // namespace orbit_core

#endif  // ORBIT_CORE_CORE_UTILS_H_
