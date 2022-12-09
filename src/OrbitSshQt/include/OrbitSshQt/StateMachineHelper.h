// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef ORBIT_SSH_QT_BASE_H_
#define ORBIT_SSH_QT_BASE_H_

#include <QObject>
#include <type_traits>

#include "OrbitSsh/Error.h"
#include "OrbitSshQt/Error.h"

namespace orbit_ssh_qt {

/*
  StateMachineHelper is a helper class to implement a monotonic state machine
  with the CRTP pattern.

  State_ refers to an enum class with at least the following states:
  - kInitialized
  - kStarting
  - kStarted
  - kStopping
  - kStopped
  - kError

  kInitialized needs to be the first entry (the smallest value in terms of the
  underlying scalar), kError needs to be the last entry (largest value in
  scalar world.). The state machine is always automatically initialized with
  kInitialized. Use SetState in the construction to immediately move into a
  different state.

  The state machine is supposed to be traversed in a monotonic way. Skipping
  states is allowed, but not going back! kStarting, kStarted, kStopping, and kStopped are considered
  markers. They can be used as real states or just considered markers that are always skipped in
  state transitions. Everything between (including) kStarting and (excluding) kStarted is considered
  the startup phase. Between (including) kStarted and (excluding) kStopping is considered the
  running phase. After (including) kStopping and before (excluding) kStopped it's the stopping
  phase. Everything after (including) kStopped and before (excluding) kError is the stopped state.

  State changes are applied by calling SetState.

  The derived class has to implement these three private member functions:
  - outcome::result<void> startup();
  - outcome::result<void> run();
  - outcome::result<void> shutdown();

  They are called on events according to the current phase. When the
  result-type returns an error the state machine automatically transitions into
  the kError state. Override the SetError(std::error_code) function to do some
  cleanup in this case.

  The helper is supposed to be used with Qt's signal slot system. The user has
  to define the following signals in the derived class. These signals are
  emitted whenever the corresponding phases are completed successfully:
  - void started();     // When reaching kStarted - startup phase completed
  - void aboutToStop(); // When reaching kStopping - running phase completed
  - void stopped();     // When reaching kStopped - stopping phase completed
  - void errorOccurred(std::error_code); // When reaching kError

  Example:
  enum class MyState {
    kInitialzed,                 // mandatory
    kAnyCustomInitState,
    kStarting,                   // mandatory
    kAnyTransitoryStartingState,
    kStarted,                    // mandatory
    kAnyCustomRunningState,
    kStopping,                   // mandatory
    kAnyTransitoryStoppingState,
    kStopped,                    // mandatory
    kAnyStoppedState,
    kError                       // mandatory
  };

  class MyMachine : public StateMachineHelper<MyMachine, MyState> {
  public:
    void Start() {
      if (CurrentState() == State::kInitialized) {
        SetState(State::Starting);
        OnEvent();
      }
    }

    void Stop() {
      if (CurrentState() == State::kStarted) {
        SetState(State::kStopping);
        OnEvent();
      }
    }

    outcome::result<void> startup() {
      switch(CurrentState()) {
      case State::kInitialized:
        ORBIT_FATAL("Should not happen!");
      case State::kStarting:
        ORBIT_LOG("About to start!");
        SetState(State::kStarted);
      case State::kStarted:
        ORBIT_LOG("started!")
        break;
      case State::kStopping:
      case State::kStopped:
      case State::kError:
        ORBIT_FATAL("Should not happen!");
      }

      return outcome::success();
    }

    outcome::result<void> run() {
      switch(CurrentState()) {
      case State::kInitialized:
      case State::kStarting:
        ORBIT_FATAL("Should not happen!");
      case State::kStarted:
      case State::kStopping:
      case State::kStopped:
      case State::kError:
        ORBIT_FATAL("Should not happen!");
      }

      return outcome::success();
    }

    outcome::result<void> shutdown() {
      switch(CurrentState()) {
      case State::kInitialized:
      case State::kStarting:
      case State::kStarted:
        ORBIT_FATAL("Should not happen!");
      case State::kStopping:
        ORBIT_LOG("about to stop!");
        SetState(State::kStopped);
      case State::kStopped:
        ORBIT_LOG("Stopped!");
        break;
      case State::kError:
        ORBIT_FATAL("Should not happen!");
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
    if (self()->CurrentState() > State::kInitialized && self()->CurrentState() < State::kStarted) {
      const auto result = self()->startup();

      if (result) {
        self()->started();
      } else {
        if (orbit_ssh::ShouldITryAgain(result)) {
          self()->HandleEagain();
        } else {
          self()->SetError(result.error());
          return;
        }
      }
    }

    if (self()->CurrentState() >= State::kStarted && self()->CurrentState() < State::kStopping) {
      const auto result = self()->run();

      if (!result) {
        if (orbit_ssh::ShouldITryAgain(result)) {
          self()->HandleEagain();
        } else {
          self()->SetError(result.error());
          return;
        }
      }
    }

    if (self()->CurrentState() >= State::kStopping && self()->CurrentState() < State::kError) {
      const auto result = self()->shutdown();

      if (result) {
        self()->stopped();
      } else {
        if (orbit_ssh::ShouldITryAgain(result)) {
          self()->HandleEagain();
        } else {
          self()->SetError(result.error());
          return;
        }
      }
    }
  }

  [[nodiscard]] bool IsStarting() const {
    return state_ >= State::kStarting && state_ < State::kStarted;
  }

  [[nodiscard]] bool IsStarted() const {
    return state_ >= State::kStarted && state_ < State::kStopping;
  }

  [[nodiscard]] bool IsStopping() const {
    return state_ >= State::kStopping && state_ < State::kStopped;
  }

  [[nodiscard]] bool IsStopped() const {
    return state_ >= State::kStopped && state_ < State::kError;
  }

  [[nodiscard]] bool IsInErrorState() const { return state_ == State::kError; }

 private:
  friend Derived;

  [[nodiscard]] Derived* self() { return static_cast<Derived*>(this); }
  [[nodiscard]] const Derived* self() const { return static_cast<const Derived*>(this); }

  State state_ = State::kInitialized;

  [[nodiscard]] State CurrentState() const { return state_; }
  void SetState(State state) {
    if (state_ != state) {
      if (state != State::kError && state >= State::kStopping && state_ < State::kError) {
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

}  // namespace orbit_ssh_qt

#endif  // ORBIT_SSH_QT_BASE_H_
