// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_BASE_H_
#define ORBIT_SSH_QT_BASE_H_

#include <QObject>

#include "OrbitBase/Logging.h"
#include "OrbitSsh/Error.h"
#include "OrbitSshQt/Error.h"

namespace OrbitSshQt {

/*
  StateMachineHelper is a helper class to implement a monotonic state machine
  with the CRTP pattern.

  State_ refers to an enum class with at least the following states:
  - kInitial
  - kStarted
  - kShutdown
  - kError

  kInitial needs to be the first entry (the smallest value in terms of the
  underlying scalar), kError needs to be the last entry (largest value in
  scalar world.). The state machine is always automatically initialized with
  kInitial. Use SetState in the construction to immediately move into a
  different state.

  The state machine is supposed to be traversed in a monotonic way. Skipping
  states is allowed, but not going back! kStarted and kShutdown are considered
  markers. They can be used as real states or just considered markers.
  Everything before kStarted is considered the startup phase. Between
  (including) kStarted and (excluding) kShutdown is considered the running
  phase. After (including) kShutdown and before (excluding) kError it's the
  shutdown phase.

  State changes are applied by calling SetState.

  The derived class has to implement these three private member functions:
  - outcome::result<void> startup();
  - outcome::result<void> run();
  - outcome::result<void> shutdown();

  They are called on events according to the current phase. When the
  result-type returns an error the state machine automatically transitions into
  the kError state. Override the SetError(std::error_code) function to do some
  cleanup in this case.

  You can override SetStateHook(State) to perform some generic task on a state
  change like logging.

  The helper is supposed to be used with Qt's signal slot system. The user has
  to define the following signals in the derived class. These signals are
  emitted whenever the corresponding phases are entered to left:
  - void started(); // When leaving kInitial
  - void stopped(); // When reaching a steady state after kShutdown
  - void aboutToShutdown(); // When leaving a state before kShutdown and
    // entering a state after (including) kShutdown
  - void errorOccurred(std::error_code); // When an error occurred.

  Example:
  enum class MyState {
    kInitial,  // mandatory
    kStarting,
    kStarted,  // mandatory
    kShutdown, // mandatory
    kDone,
    kError     // mandatory
  };

  class MyMachine : public StateMachineHelper<MyMachine, MyState> {
  public:
    void Start() {
      if (CurrentState() == State::kInitial) {
        SetState(State::Starting);
        OnEvent();
      }
    }

    void Stop() {
      if (CurrentState() == State::kStarted) {
        SetState(State::kShutdown);
        OnEvent();
      }
    }

    outcome::result<void> startup() {
      switch(CurrentState()) {
      case State::kInitial:
        FATAL("Should not happen!");
      case State::kStarting:
        LOG("About to start!");
        SetState(State::kStarted);
      case State::kStarted:
        LOG("started!")
        break;
      case State::kShutdown:
      case State::kDone:
      case State::kError:
        FATAL("Should not happen!");
      }

      return outcome::success();
    }

    outcome::result<void> run() {
      switch(CurrentState()) {
      case State::kInitial:
      case State::kStarting:
        FATAL("Should not happen!");
      case State::kStarted:
      case State::kShutdown:
      case State::kDone:
      case State::kError:
        FATAL("Should not happen!");
      }

      return outcome::success();
    }

    outcome::result<void> shutdown() {
      switch(CurrentState()) {
      case State::kInitial:
      case State::kStarting:
      case State::kStarted:
        FATAL("Should not happen!");
      case State::kShutdown:
        LOG("about to shut down!");
        SetState(State::kDone);
      case State::kDone:
        LOG("shutted down!");
        break;
      case State::kError:
        FATAL("Should not happen!");
      }

      return outcome::success();
    }
  };
*/
template <typename Derived, typename State_>
class StateMachineHelper : public QObject {
 public:
  using State = State_;
  using QObject::QObject;

  void OnEvent() {
    if (self()->CurrentState() > State::kInitial &&
        self()->CurrentState() < State::kStarted) {
      const auto result = self()->startup();

      if (result) {
        self()->started();
      } else {
        if (OrbitSsh::ShouldITryAgain(result)) {
          self()->HandleEagain();
        } else {
          self()->SetError(result.error());
          return;
        }
      }
    }

    if (self()->CurrentState() >= State::kStarted &&
        self()->CurrentState() < State::kShutdown) {
      const auto result = self()->run();

      if (!result) {
        if (OrbitSsh::ShouldITryAgain(result)) {
          self()->HandleEagain();
        } else {
          self()->SetError(result.error());
          return;
        }
      }
    }

    if (self()->CurrentState() >= State::kShutdown &&
        self()->CurrentState() < State::kError) {
      const auto result = self()->shutdown();

      if (result) {
        self()->stopped();
      } else {
        if (OrbitSsh::ShouldITryAgain(result)) {
          self()->HandleEagain();
        } else {
          self()->SetError(result.error());
          return;
        }
      }
    }
  }

 private:
  friend Derived;

  Derived* self() { return static_cast<Derived*>(this); }
  const Derived* self() const { return static_cast<const Derived*>(this); }

  State state_ = State::kInitial;

  State CurrentState() const { return state_; }
  void SetStateHook(State) {}
  void SetState(State state) {
    if (state_ != state) {
      self()->SetStateHook(state);
      if (state != State::kError && state >= State::kShutdown &&
          state_ < State::kShutdown) {
        self()->aboutToShutdown();
      }
      state_ = state;
    }
  }
  void SetError(std::error_code e) {
    self()->SetState(State::kError);
    self()->errorOccurred(e);
  }
  void SetError(Error e) { self()->SetError(make_error_code(e)); }
};

}  // namespace OrbitSshQt

#endif  // ORBIT_SSH_QT_BASE_H_
