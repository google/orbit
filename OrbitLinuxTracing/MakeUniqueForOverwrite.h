#ifndef ORBIT_LINUX_TRACING_MAKE_UNIQUE_FOR_OVERWRITE_H_
#define ORBIT_LINUX_TRACING_MAKE_UNIQUE_FOR_OVERWRITE_H_

#include <memory>

// Unlike std::make_unique, calls new without parentheses, causing the object
// to be default-initialized instead of value-initialized.
// This can prevent useless memory initialization.
// Standard version coming in C++20.
template <class T>
inline std::unique_ptr<T> make_unique_for_overwrite() {
  return std::unique_ptr<T>(new T);
}

template <class T>
inline std::unique_ptr<T> make_unique_for_overwrite(size_t size) {
  return std::unique_ptr<T>(new typename std::remove_extent<T>::type[size]);
}

#endif  // ORBIT_LINUX_TRACING_MAKE_UNIQUE_FOR_OVERWRITE_H_
