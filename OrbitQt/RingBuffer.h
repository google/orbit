// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_QT_RING_BUFFER_H_
#define ORBIT_QT_RING_BUFFER_H_

#include <array>
#include <cstddef>

template <class T, size_t BUFFER_SIZE>
class RingBuffer {
 public:
  void Clear() {
    current_size_ = 0;
    current_index_ = 0;
  }

  void Add(const T& item) {
    data_[current_index_++] = item;
    current_index_ %= BUFFER_SIZE;
    ++current_size_;
  }

  void Fill(const T& item) {
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
      Add(item);
    }
  }

  [[nodiscard]] bool Contains(const T& item) const {
    for (size_t i = 0; i < Size(); ++i) {
      if (data_[i] == item) return true;
    }
    return false;
  }

  [[nodiscard]] size_t Size() const {
    return current_size_ > BUFFER_SIZE ? BUFFER_SIZE : current_size_;
  }

  [[nodiscard]] size_t GetCurrentIndex() const { return current_index_; }

  [[nodiscard]] T* Data() { return data_.data(); }

  [[nodiscard]] size_t IndexOfOldest() const {
    return current_size_ > BUFFER_SIZE ? current_index_ : 0;
  }

  [[nodiscard]] T& operator[](size_t index) {
    size_t internal_index = (IndexOfOldest() + index) % BUFFER_SIZE;
    return data_[internal_index];
  }

  [[nodiscard]] const T& operator[](size_t index) const {
    size_t internal_index = (IndexOfOldest() + index) % BUFFER_SIZE;
    return data_[internal_index];
  }

  [[nodiscard]] const T& Latest() const { return (*this)[Size() - 1]; }

 private:
  std::array<T, BUFFER_SIZE> data_;
  size_t current_size_ = 0;
  size_t current_index_ = 0;
};

#endif  // ORBIT_QT_RING_BUFFER_H_
