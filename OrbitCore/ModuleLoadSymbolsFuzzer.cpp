#include <libfuzzer/libfuzzer_macro.h>

#include "OrbitModule.h"
#include "symbol.pb.h"

DEFINE_PROTO_FUZZER(const ModuleSymbols& symbols) {
  Module module{};
  module.LoadSymbols(symbols);
}