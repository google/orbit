/*
 * Copyright (C) 2021 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <cstdint>
#include <ios>
#include <sstream>

#include <benchmark/benchmark.h>

#include <unwindstack/DwarfLocation.h>
#include <unwindstack/DwarfSection.h>

#include "Utils.h"
#include "utils/DwarfSectionImplFake.h"
#include "utils/MemoryFake.h"
#include "utils/RegsFake.h"

namespace unwindstack {
namespace {

// This collection of benchmarks exercises the DwarfSectionImpl::Eval function with a set of
// artificial unwind data. The number of registers and register evaluation method are varied
// for each individual benchmark.

constexpr int kReturnAddressReg = 5;

template <typename AddresssType>
class EvalBenchmark : public benchmark::Fixture {
 public:
  EvalBenchmark() {
    memory_.Clear();
    section_ = std::make_unique<DwarfSectionImplFake<AddresssType>>(&memory_);
  }

  void TearDown(benchmark::State& state) override { mem_tracker_.SetBenchmarkCounters(state); }

  // Benchmarks DwarfSectionImpl::Eval given the DwarfLocation object, loc_regs, initialized in each
  // individual benchmark macro/function.
  //
  // This method initializes the fake register object and the DwarfCie object the same regardless
  // of the benchmark. So the initialization of loc_regs is carefully crafted in each benchmark
  // macro so that the evaluated PC and SP match the expected values after each call to Eval in the
  // benchmarking loop.
  //
  // In addition to the Eval call, register value assertion is included in the benchmarking loop
  // to ensure that we always capture the actual register evaluation
  // (DwarfSectionImpl::EvalRegister). For example, if Eval is modified to lazily evaluate register
  // values, we will still capture the register evaluation for the PC and SP (common case) in the
  // register value assertion.
  void RunBenchmark(benchmark::State& state, DwarfLocations& loc_regs) {
    DwarfCie cie{.return_address_register = kReturnAddressReg};
    bool finished;
    RegsImplFake<AddresssType> regs(64);
    regs.set_pc(0x1000);
    regs.set_sp(0x3500);
    regs[0] = 0x10000000;
    mem_tracker_.StartTrackingAllocations();
    for (auto _ : state) {
      std::stringstream err_stream;
      if (!section_->Eval(&cie, &memory_, loc_regs, &regs, &finished)) {
        err_stream << "Eval() failed at address " << section_->LastErrorAddress();
        state.SkipWithError(err_stream.str().c_str());
        return;
      }
      if (finished || regs.pc() != 0x60000000U || regs.sp() != 0x10000000U) {
        err_stream
            << "Eval() finished successfully but registers were not evaluated correctly."
            << "\nExpected: finished == false, regs.pc() == 0x60000000, regs.sp() == 0x10000000."
            << "\nActual: finished == " << std::boolalpha << finished << std::hex
            << ", regs.pc() == 0x" << regs.pc() << ", regs.sp() == 0x" << regs.sp();
        state.SkipWithError(err_stream.str().c_str());
        return;
      }
    }
    mem_tracker_.StopTrackingAllocations();
  }

 protected:
  MemoryFake memory_;
  std::unique_ptr<DwarfSectionImplFake<AddresssType>> section_;
  MemoryTracker mem_tracker_;
};

// Benchmarks exercising Eval with the DWARF_LOCATION_REGISTER evaluation method.
BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_register_few_regs, uint64_t)(benchmark::State& state) {
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  loc_regs[kReturnAddressReg] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0x50000000}};
  RunBenchmark(state, loc_regs);
}

BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_register_many_regs, uint64_t)(benchmark::State& state) {
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  for (uint64_t i = 0; i < 64; i++) {
    loc_regs[i] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, i * 0x10000000}};
  }
  RunBenchmark(state, loc_regs);
}

// Benchmarks exercising Eval with the DWARF_LOCATION_VAL_OFFSET evaluation method.
BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_val_offset_few_regs, uint64_t)
(benchmark::State& state) {
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  loc_regs[kReturnAddressReg] = DwarfLocation{DWARF_LOCATION_VAL_OFFSET, {0x50000000, 0}};
  RunBenchmark(state, loc_regs);
}

BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_val_offset_many_regs, uint64_t)
(benchmark::State& state) {
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  for (uint64_t i = 0; i < 64; i++) {
    loc_regs[i] = DwarfLocation{DWARF_LOCATION_VAL_OFFSET, {i * 0x10000000, 0}};
  }
  RunBenchmark(state, loc_regs);
}

// Benchmarks exercising Eval with the DWARF_LOCATION_OFFSET evaluation method.
BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_offset_few_regs, uint64_t)
(benchmark::State& state) {
  memory_.SetData64(0x20000000, 0x60000000);
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  loc_regs[kReturnAddressReg] = DwarfLocation{DWARF_LOCATION_OFFSET, {0x10000000, 0}};
  RunBenchmark(state, loc_regs);
}

BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_offset_many_regs, uint64_t)
(benchmark::State& state) {
  memory_.SetData64(0x20000000, 0x60000000);
  memory_.SetData64(0x30000000, 0x10000000);
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  for (uint64_t i = 1; i < 64; i++) {
    loc_regs[i] = DwarfLocation{DWARF_LOCATION_OFFSET, {0x10000000, 0}};
  }
  // Read from different place in memory for reg 0 so reg 0 maintains value of 0x10000000
  // across multiple calls to Eval.
  loc_regs[0] = DwarfLocation{DWARF_LOCATION_OFFSET, {0x20000000, 0}};
  RunBenchmark(state, loc_regs);
}

// Benchmarks exercising Eval with the DWARF_LOCATION_EXPRESSION evaluation method.
// The dwarf op-code used for the expression benchmarks are OP_const4u (see DwarfOp::Eval).
BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_expression_few_regs, uint64_t)
(benchmark::State& state) {
  memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x80});
  uint64_t pc_value = 0x60000000;
  memory_.SetMemory(0x80000000, &pc_value, sizeof(pc_value));
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  loc_regs[kReturnAddressReg] = DwarfLocation{DWARF_LOCATION_EXPRESSION, {0x4, 0x5004}};
  RunBenchmark(state, loc_regs);
}

BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_expression_many_regs, uint64_t)
(benchmark::State& state) {
  memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x80});
  uint64_t pc_value = 0x60000000;
  memory_.SetMemory(0x80000000, &pc_value, sizeof(pc_value));

  memory_.SetMemory(0x6000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x90});
  uint64_t sp_value = 0x10000000;
  memory_.SetMemory(0x90000000, &sp_value, sizeof(sp_value));

  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  for (uint64_t i = 1; i < 64; i++) {
    loc_regs[i] = DwarfLocation{DWARF_LOCATION_EXPRESSION, {0x4, 0x5004}};
  }
  // Read from different place in memory for reg 0 so reg 0 maintains value of 0x10000000
  // across multiple calls to Eval.
  loc_regs[0] = DwarfLocation{DWARF_LOCATION_EXPRESSION, {0x4, 0x6004}};
  RunBenchmark(state, loc_regs);
}

// Benchmarks exercising Eval with the DWARF_LOCATION_VAL_EXPRESSION evaluation method.
// The dwarf op-code used for the value expression benchmarks are OP_const4u (see DwarfOp::Eval).
BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_val_expression_few_regs, uint64_t)
(benchmark::State& state) {
  memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x60});
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  loc_regs[kReturnAddressReg] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x4, 0x5004}};
  RunBenchmark(state, loc_regs);
}

BENCHMARK_TEMPLATE_F(EvalBenchmark, BM_eval_val_expression_many_regs, uint64_t)
(benchmark::State& state) {
  memory_.SetMemory(0x5000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x60});
  memory_.SetMemory(0x6000, std::vector<uint8_t>{0x0c, 0x00, 0x00, 0x00, 0x10});
  DwarfLocations loc_regs;
  loc_regs[CFA_REG] = DwarfLocation{DWARF_LOCATION_REGISTER, {0, 0}};
  for (uint64_t i = 1; i < 64; i++) {
    loc_regs[i] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x4, 0x5004}};
  }
  // Read from different place in memory for reg 0 so reg 0 maintains value of 0x10000000
  // across multiple calls to Eval.
  loc_regs[0] = DwarfLocation{DWARF_LOCATION_VAL_EXPRESSION, {0x4, 0x6004}};
  RunBenchmark(state, loc_regs);
}

}  // namespace
}  // namespace unwindstack
