#include <string>

#include "SymbolHelper.h"

extern "C" int LLVMFuzzerTestOneInput(uint8_t* buf, size_t len) {
  const SymbolHelper symbol_helper;
  (void)symbol_helper.LoadSymbolsCollector(
      std::string{reinterpret_cast<const char*>(buf), len});
  return 0;
}