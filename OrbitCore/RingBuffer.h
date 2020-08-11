// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#pragma once

#include <cstddef>

template <class T, size_t BUFFER_SIZE>
class RingBuffer {
 public:
  RingBuffer() : m_CurrentSize(0), m_CurrentIndex(0) {}

  ~RingBuffer() {}

  inline void Clear() { m_CurrentSize = m_CurrentIndex = 0; }

  inline void Add(const T& a_Item) {
    m_Data[m_CurrentIndex++] = a_Item;
    m_CurrentIndex %= BUFFER_SIZE;
    ++m_CurrentSize;
  }

  inline void Fill(const T& a_Item) {
    for (size_t i = 0; i < BUFFER_SIZE; ++i) {
      Add(a_Item);
    }
  }

  inline bool Contains(const T& a_Item) const {
    for (size_t i = 0; i < Size(); ++i) {
      if (m_Data[i] == a_Item) return true;
    }

    return false;
  }

  inline size_t Size() const {
    return m_CurrentSize > BUFFER_SIZE ? BUFFER_SIZE : m_CurrentSize;
  }

  inline size_t GetCurrentIndex() const { return m_CurrentIndex; }

  inline T* Data() { return m_Data; }

  inline size_t IndexOfOldest() const {
    return m_CurrentSize > BUFFER_SIZE ? m_CurrentIndex : 0;
  }

  T& operator[](size_t a_Index) {
    size_t index = (IndexOfOldest() + a_Index) % BUFFER_SIZE;
    return m_Data[index];
  }

  inline const T& Latest() { return (*this)[Size() - 1]; }

 protected:
  T m_Data[BUFFER_SIZE];
  size_t m_CurrentSize;
  size_t m_CurrentIndex;
};
