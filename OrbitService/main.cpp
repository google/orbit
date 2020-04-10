#include <iostream>

#include "OrbitService.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage("Orbit CPU Profiler Service");
  absl::ParseCommandLine(argc, argv);
  std::cout << "Starting OrbitService" << std::endl;
  OrbitService service;
  service.Run();
}
