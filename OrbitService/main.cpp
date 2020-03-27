#include <iostream>

#include "OrbitService.h"
#include "absl/flags/parse.h"

int main(int argc, char** argv) {
  std::cout << "Starting OrbitService" << std::endl;
  absl::ParseCommandLine(argc, argv);
  OrbitService service;
  service.Run();
}
