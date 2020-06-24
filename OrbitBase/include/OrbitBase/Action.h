// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ACTION_H_
#define ORBIT_BASE_ACTION_H_

#include "internal/identity.h"

// Actions are executed by MainThreadExecutor
// The Action is an abstract class which can
// be executed.
class Action {
 public:
  Action() = default;
  virtual ~Action() = default;

  // Executes the action.
  virtual void Execute() = 0;
};

// This class implements an action without parameters.
template <typename F>
class NullaryFunctorAction : public Action {
 public:
  template <typename T, typename = std::enable_if_t<!std::is_same_v<
                            std::decay_t<T>, NullaryFunctorAction>>>
  explicit NullaryFunctorAction(T&& functor)
      : functor_(std::forward<T>(functor)) {}

  void Execute() override { functor_(); }

 private:
  F functor_;
};

// This class implements an action that calls a method for some object.
template <typename T>
class MethodAction : public Action {
 public:
  explicit MethodAction(T* object,
                        void (internal::identity<T>::type::*method)())
      : object_(object), method_(method) {}

  void Execute() override { (object_->*method_)(); }

 private:
  T* object_;
  void (T::*method_)();
};

template <typename F>
std::unique_ptr<Action> CreateAction(F&& functor) {
  return std::make_unique<NullaryFunctorAction<std::remove_reference_t<F>>>(
      std::forward<F>(functor));
}

template <typename T>
std::unique_ptr<Action> CreateAction(
    T* object, void (internal::identity<T>::type::*method)()) {
  return std::make_unique<MethodAction<T>>(object, method);
}
#endif  // ORBIT_BASE_ACTION_H_
