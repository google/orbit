#pragma once
#include <vector>
class OrbitService {
 public:
  OrbitService();
  ~OrbitService();

  void Run();

 private:
  bool m_ExitRequested = false;
};