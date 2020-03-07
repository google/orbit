#include <memory>

#include "OrbitTest.h"

int main(int argc, char* argv[]) {
  std::unique_ptr<OrbitTest> test;
  if (argc == 4) {
    uint32_t num_threads = std::stoul(argv[1]);
    uint32_t recurse_depth = std::stoul(argv[2]);
    uint32_t sleep_us = std::stoul(argv[3]);
    test = std::make_unique<OrbitTest>(num_threads, recurse_depth, sleep_us);
  } else {
    test = std::make_unique<OrbitTest>();
  }

  test->Start();
  getchar();
}
