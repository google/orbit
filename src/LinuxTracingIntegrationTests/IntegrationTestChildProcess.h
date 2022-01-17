// Copyright (c) 2021 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_CHILD_PROCESS_H_
#define LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_CHILD_PROCESS_H_

#include <sys/types.h>
#include <unistd.h>

#include <array>
#include <functional>
#include <string>

#include "OrbitBase/Logging.h"

namespace orbit_linux_tracing_integration_tests {

// This class handles a child process forked from the current one. It allows to write to the child's
// stdin and to read from its stdout through standard use of pipes. The constructor runs the child
// and the destructor waits for its completion.
class ChildProcess {
 public:
  explicit ChildProcess(const std::function<int()>& child_main) {
    std::array<int, 2> parent_to_child_pipe{};
    ORBIT_CHECK(pipe(parent_to_child_pipe.data()) == 0);

    std::array<int, 2> child_to_parent_pipe{};
    ORBIT_CHECK(pipe(child_to_parent_pipe.data()) == 0);

    pid_t child_pid = fork();
    ORBIT_CHECK(child_pid >= 0);
    if (child_pid > 0) {
      // Parent.
      child_pid_ = child_pid;

      // Close unused ends of the pipes.
      ORBIT_CHECK(close(parent_to_child_pipe[0]) == 0);
      ORBIT_CHECK(close(child_to_parent_pipe[1]) == 0);

      reading_fd_ = child_to_parent_pipe[0];
      writing_fd_ = parent_to_child_pipe[1];

    } else {
      // Child.

      // Close unused ends of the pipes.
      ORBIT_CHECK(close(parent_to_child_pipe[1]) == 0);
      ORBIT_CHECK(close(child_to_parent_pipe[0]) == 0);

      // Redirect reading end of parent_to_child_pipe to stdin and close the pipe's original fd.
      ORBIT_CHECK(close(STDIN_FILENO) == 0);
      ORBIT_CHECK(dup2(parent_to_child_pipe[0], STDIN_FILENO) == STDIN_FILENO);
      ORBIT_CHECK(close(parent_to_child_pipe[0]) == 0);

      // Redirect writing end of child_to_parent_pipe to stdout and close the pipe's original fd.
      ORBIT_CHECK(close(STDOUT_FILENO) == 0);
      ORBIT_CHECK(dup2(child_to_parent_pipe[1], STDOUT_FILENO) == STDOUT_FILENO);
      ORBIT_CHECK(close(child_to_parent_pipe[1]) == 0);

      // Run the child and exit.
      exit(child_main());
    }
  }

  ~ChildProcess() {
    ORBIT_CHECK(close(reading_fd_) == 0);
    ORBIT_CHECK(close(writing_fd_) == 0);

    ORBIT_CHECK(waitpid(child_pid_, nullptr, 0) == child_pid_);
  }

  [[nodiscard]] pid_t GetChildPidNative() const { return child_pid_; }

  void WriteLine(std::string_view str) {
    std::string string_with_newline = std::string{str}.append("\n");
    ORBIT_CHECK(write(writing_fd_, string_with_newline.c_str(), string_with_newline.length()) ==
                static_cast<ssize_t>(string_with_newline.length()));
  }

  [[nodiscard]] std::string ReadLine() {
    std::string str;
    while (true) {
      char c;
      ORBIT_CHECK(read(reading_fd_, &c, sizeof(c)) == sizeof(c));
      if (c == '\n' || c == '\0') break;
      str.push_back(c);
    }
    return str;
  }

 private:
  pid_t child_pid_ = -1;
  int reading_fd_ = -1;
  int writing_fd_ = -1;
};

}  // namespace orbit_linux_tracing_integration_tests

#endif  // LINUX_TRACING_INTEGRATION_TESTS_INTEGRATION_TEST_CHILD_PROCESS_H_
