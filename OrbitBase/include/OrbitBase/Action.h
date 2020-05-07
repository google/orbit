// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_BASE_ACTION_H_
#define ORBIT_BASE_ACTION_H_

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
  explicit NullaryFunctorAction(F functor) : functor_(functor) {}

  void Execute() override { functor_(); }

 private:
  F functor_;
};

template <typename F>
std::unique_ptr<Action> CreateAction(F&& functor) {
  return std::make_unique<NullaryFunctorAction<F>>(std::forward<F>(functor));
}

#endif  // ORBIT_BASE_ACTION_H_
