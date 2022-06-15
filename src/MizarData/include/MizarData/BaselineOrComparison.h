// Copyright (c) 2022 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef MIZAR_DATA_BASELINE_OR_COMPARISON_H_
#define MIZAR_DATA_BASELINE_OR_COMPARISON_H_

#include <type_traits>
#include <utility>

namespace orbit_mizar_data {

template <typename T, typename U>
using EnableIfUConvertibleToT = std::enable_if_t<std::is_convertible_v<U, T>>;

template <typename T>
class BaselineOrComparison {
 public:
  ~BaselineOrComparison() = default;

  const T* operator->() const { return &value_; }
  T* operator->() { return &value_; }

  const T& operator*() const { return value_; }
  T& operator*() { return value_; }

 protected:
  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  explicit BaselineOrComparison(U&& value) : value_(std::forward<T>(value)) {}
  BaselineOrComparison(BaselineOrComparison&& other) = default;

  template <typename... Args>
  explicit BaselineOrComparison(std::in_place_t /*_*/, Args&&... args)
      : value_(T(std::forward<T>(args)...)) {}

 private:
  T value_;
};

template <typename T>
class Baseline : public BaselineOrComparison<T> {
 public:
  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  explicit Baseline(U&& value) : BaselineOrComparison<T>(std::forward<U>(value)) {}

  template <typename... Args>
  explicit Baseline(std::in_place_t in_place, Args&&... args)
      : BaselineOrComparison<T>(in_place, std::forward<T>(args)...) {}
};

template <typename T>
class Comparison : public BaselineOrComparison<T> {
 public:
  template <typename U, typename = EnableIfUConvertibleToT<T, U>>
  explicit Comparison(U&& value) : BaselineOrComparison<T>(std::forward<U>(value)) {}

  template <typename... Args>
  explicit Comparison(std::in_place_t in_place, Args&&... args)
      : BaselineOrComparison<T>(in_place, std::forward<T>(args)...) {}
};

template <typename T, typename... Args>
Baseline<T> MakeBaseline(Args&&... args) {
  return Baseline<T>(T(std::forward<Args>(args)...));
}

template <typename T, typename... Args>
Comparison<T> MakeComparison(Args&&... args) {
  return Comparison<T>(T(std::forward<Args>(args)...));
}

}  // namespace orbit_mizar_data

#endif  // MIZAR_DATA_BASELINE_OR_COMPARISON_H_
