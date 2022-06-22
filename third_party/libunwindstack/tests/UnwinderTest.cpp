/*
 * Copyright (C) 2017 The Android Open Source Project
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

#include <elf.h>
#include <stdint.h>
#include <sys/mman.h>

#include <memory>
#include <set>
#include <string>

#include <gtest/gtest.h>

#include <android-base/silent_death_test.h>
#include <unwindstack/Maps.h>
#include <unwindstack/Memory.h>
#include <unwindstack/Object.h>
#include <unwindstack/Regs.h>
#include <unwindstack/RegsArm.h>
#include <unwindstack/RegsArm64.h>
#include <unwindstack/RegsMips.h>
#include <unwindstack/RegsMips64.h>
#include <unwindstack/RegsX86.h>
#include <unwindstack/RegsX86_64.h>
#include <unwindstack/Unwinder.h>

#include "ElfFake.h"
#include "ElfTestUtils.h"
#include "utils/MemoryFake.h"
#include "utils/RegsFake.h"

namespace unwindstack {

class UnwinderTest : public ::testing::Test {
 protected:
  // We set up `memory_` and `regs_` per test fixture, because some tests modify these, which
  // can lead to tests influencing other tests and causing them to fail. This is only
  // a problem when all tests are run in the same process, which is for example the
  // case when running these tests locally during development.
  UnwinderTest() : memory_(new MemoryFake), process_memory_(memory_), regs_(5) {
    // dex debug data
    memory_->SetData32(0xf180c, 0xf3000);
    memory_->SetData32(0xf3000, 0xf4000);
    memory_->SetData32(0xf3004, 0xf4000);
    memory_->SetData32(0xf3008, 0xf5000);
    // jit debug data
    memory_->SetData32(0xf1900, 1);
    memory_->SetData32(0xf1904, 0);
    memory_->SetData32(0xf1908, 0xf6000);
    memory_->SetData32(0xf190c, 0xf6000);
    memory_->SetData32(0xf6000, 0);
    memory_->SetData32(0xf6004, 0);
    memory_->SetData32(0xf6008, 0xf7000);
    memory_->SetData32(0xf600c, 0);
    memory_->SetData64(0xf6010, 0x1000);

    ElfInterfaceFake::FakeClear();
    regs_.FakeSetArch(ARCH_ARM);
    regs_.FakeSetReturnAddressValid(false);
  }

  static MapInfo* AddMapInfo(uint64_t start, uint64_t end, uint64_t offset, uint64_t flags,
                             const char* name, Object* object = nullptr) {
    std::string str_name(name);
    maps_->Add(start, end, offset, flags, name);
    MapInfo* map_info = maps_->Find(start).get();
    if (object != nullptr) {
      map_info->set_object(object);
    }
    return map_info;
  }

  static void SetUpTestSuite() {
    maps_.reset(new Maps);

    ElfFake* elf;
    ElfInterfaceFake* interface;
    MapInfo* map_info;

    elf = new ElfFake(new MemoryFake);
    interface = new ElfInterfaceFake(nullptr);
    interface->FakeSetBuildID("FAKE");
    elf->FakeSetInterface(interface);
    AddMapInfo(0x1000, 0x8000, 0, PROT_READ | PROT_WRITE, "/system/fake/libc.so", elf);

    AddMapInfo(0x10000, 0x12000, 0, PROT_READ | PROT_WRITE, "[stack]");

    AddMapInfo(0x13000, 0x15000, 0, PROT_READ | PROT_WRITE | MAPS_FLAGS_DEVICE_MAP,
               "/dev/fake_device");

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    AddMapInfo(0x20000, 0x22000, 0, PROT_READ | PROT_WRITE, "/system/fake/libunwind.so", elf);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    AddMapInfo(0x23000, 0x24000, 0, PROT_READ | PROT_WRITE, "/fake/libanother.so", elf);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    AddMapInfo(0x33000, 0x34000, 0, PROT_READ | PROT_WRITE, "/fake/compressed.so", elf);

    elf = new ElfFake(new MemoryFake);
    interface = new ElfInterfaceFake(nullptr);
    interface->FakeSetSoname("lib_fake.so");
    elf->FakeSetInterface(interface);
    map_info = AddMapInfo(0x43000, 0x44000, 0x1d000, PROT_READ | PROT_WRITE, "/fake/fake.apk", elf);
    map_info->set_object_start_offset(0x1d000);

    AddMapInfo(0x53000, 0x54000, 0, PROT_READ | PROT_WRITE, "/fake/fake.oat");

    AddMapInfo(0xa3000, 0xa4000, 0, PROT_READ | PROT_WRITE | PROT_EXEC, "/fake/fake.vdex");

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    elf->FakeSetLoadBias(0x5000);
    AddMapInfo(0xa5000, 0xa6000, 0, PROT_READ | PROT_WRITE | PROT_EXEC, "/fake/fake_load_bias.so",
               elf);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    map_info = AddMapInfo(0xa7000, 0xa8000, 0, PROT_READ | PROT_WRITE | PROT_EXEC,
                          "/fake/fake_offset.oat", elf);
    map_info->set_object_offset(0x8000);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    map_info = AddMapInfo(0xc0000, 0xc1000, 0, PROT_READ | PROT_WRITE | PROT_EXEC,
                          "/fake/unreadable.so", elf);
    map_info->set_memory_backed_object(true);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    map_info = AddMapInfo(0xc1000, 0xc2000, 0, PROT_READ | PROT_WRITE | PROT_EXEC, "[vdso]", elf);
    map_info->set_memory_backed_object(true);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    map_info = AddMapInfo(0xc2000, 0xc3000, 0, PROT_READ | PROT_WRITE | PROT_EXEC, "", elf);
    map_info->set_memory_backed_object(true);

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetInterface(new ElfInterfaceFake(nullptr));
    map_info = AddMapInfo(0xc3000, 0xc4000, 0, PROT_READ | PROT_WRITE | PROT_EXEC,
                          "/memfd:/jit-cache", elf);
    map_info->set_memory_backed_object(true);

    map_info =
        AddMapInfo(0xd0000, 0xd1000, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, "/fake/fake.apk");
    map_info->set_object_start_offset(0x1000);

    elf = new ElfFake(new MemoryFake);
    interface = new ElfInterfaceFake(nullptr);
    elf->FakeSetInterface(interface);
    interface->FakeSetGlobalVariable("__dex_debug_descriptor", 0x1800);
    interface->FakeSetGlobalVariable("__jit_debug_descriptor", 0x1900);
    interface->FakeSetDataOffset(0x1000);
    interface->FakeSetDataVaddrStart(0x1000);
    interface->FakeSetDataVaddrEnd(0x8000);
    AddMapInfo(0xf0000, 0xf1000, 0, PROT_READ | PROT_WRITE | PROT_EXEC, "/fake/global.so", elf);
    AddMapInfo(0xf1000, 0xf9000, 0x1000, PROT_READ | PROT_WRITE, "/fake/global.so");

    elf = new ElfFake(new MemoryFake);
    elf->FakeSetValid(false);
    elf->FakeSetLoadBias(0x300);
    map_info = AddMapInfo(0x100000, 0x101000, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC,
                          "/fake/jit.so", elf);
    map_info->set_object_start_offset(0x100);
    map_info->set_offset(0x200);
  }

  static std::unique_ptr<Maps> maps_;

  MemoryFake* memory_;
  std::shared_ptr<Memory> process_memory_;
  RegsFake regs_;
};

std::unique_ptr<Maps> UnwinderTest::maps_;

TEST_F(UnwinderTest, multiple_frames) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x1104, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x1204, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(3U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x100U, frame->rel_pc);
  EXPECT_EQ(0x1100U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[2];
  EXPECT_EQ(2U, frame->num);
  EXPECT_EQ(0x200U, frame->rel_pc);
  EXPECT_EQ(0x1200U, frame->pc);
  EXPECT_EQ(0x10020U, frame->sp);
  EXPECT_EQ("Frame2", frame->function_name);
  EXPECT_EQ(2U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, multiple_frames_dont_resolve_names) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x1104, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x1204, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.SetResolveNames(false);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(3U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x100U, frame->rel_pc);
  EXPECT_EQ(0x1100U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[2];
  EXPECT_EQ(2U, frame->num);
  EXPECT_EQ(0x200U, frame->rel_pc);
  EXPECT_EQ(0x1200U, frame->pc);
  EXPECT_EQ(0x10020U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, non_zero_load_bias) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0xa5500);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x5500U, frame->rel_pc);
  EXPECT_EQ(0xa5500U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake_load_bias.so", frame->map_info->name());
  EXPECT_EQ("/fake/fake_load_bias.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xa5000U, frame->map_info->start());
  EXPECT_EQ(0xa6000U, frame->map_info->end());
  EXPECT_EQ(0x5000U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

TEST_F(UnwinderTest, non_zero_object_offset) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0xa7500);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x8500U, frame->rel_pc);
  EXPECT_EQ(0xa7500U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake_offset.oat", frame->map_info->name());
  EXPECT_EQ("/fake/fake_offset.oat", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xa7000U, frame->map_info->start());
  EXPECT_EQ(0xa8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

TEST_F(UnwinderTest, non_zero_map_offset) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0x43000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x43000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake.apk", frame->map_info->name());
  EXPECT_EQ("/fake/fake.apk!lib_fake.so", frame->map_info->GetFullName());
  EXPECT_EQ(0x1d000U, frame->map_info->object_start_offset());
  EXPECT_EQ(0x1d000U, frame->map_info->offset());
  EXPECT_EQ(0x43000U, frame->map_info->start());
  EXPECT_EQ(0x44000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify that no attempt to continue after the step indicates it is done.
TEST_F(UnwinderTest, no_frames_after_finished) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame3", 3));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame4", 4));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x1000, 0x10000, true));
  ElfInterfaceFake::FakePushStepData(StepData(0x1102, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x1202, 0x10020, false));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify the maximum frames to save.
TEST_F(UnwinderTest, max_frames) {
  for (size_t i = 0; i < 30; i++) {
    ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame" + std::to_string(i), i));
    ElfInterfaceFake::FakePushStepData(StepData(0x1104 + i * 0x100, 0x10010 + i * 0x10, false));
  }

  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);

  Unwinder unwinder(20, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_MAX_FRAMES_EXCEEDED, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(20U, unwinder.NumFrames());

  for (size_t i = 0; i < 20; i++) {
    auto* frame = &unwinder.frames()[i];
    EXPECT_EQ(i, frame->num);
    SCOPED_TRACE(testing::Message() << "Failed at frame " << i);
    EXPECT_EQ(i * 0x100, frame->rel_pc);
    EXPECT_EQ(0x1000 + i * 0x100, frame->pc);
    EXPECT_EQ(0x10000 + 0x10 * i, frame->sp);
    EXPECT_EQ("Frame" + std::to_string(i), frame->function_name);
    EXPECT_EQ(i, frame->function_offset);
    ASSERT_TRUE(frame->map_info != nullptr);
    EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
    EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
    EXPECT_EQ(0U, frame->map_info->object_start_offset());
    EXPECT_EQ(0U, frame->map_info->offset());
    EXPECT_EQ(0x1000U, frame->map_info->start());
    EXPECT_EQ(0x8000U, frame->map_info->end());
    EXPECT_EQ(0U, frame->map_info->GetLoadBias());
    EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
  }
}

// Verify that initial map names frames are removed.
TEST_F(UnwinderTest, verify_frames_skipped) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));

  regs_.set_pc(0x20000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x23004, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x23104, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x20004, 0x10030, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x21004, 0x10040, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x1002, 0x10050, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x21004, 0x10060, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x23002, 0x10070, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  std::vector<std::string> skip_libs{"libunwind.so", "libanother.so"};
  unwinder.Unwind(&skip_libs);
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(3U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10050U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x1000U, frame->rel_pc);
  EXPECT_EQ(0x21000U, frame->pc);
  EXPECT_EQ(0x10060U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libunwind.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libunwind.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x20000U, frame->map_info->start());
  EXPECT_EQ(0x22000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[2];
  EXPECT_EQ(2U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x23000U, frame->pc);
  EXPECT_EQ(0x10070U, frame->sp);
  EXPECT_EQ("Frame2", frame->function_name);
  EXPECT_EQ(2U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/libanother.so", frame->map_info->name());
  EXPECT_EQ("/fake/libanother.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x23000U, frame->map_info->start());
  EXPECT_EQ(0x24000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify SP in a non-existant map is okay.
TEST_F(UnwinderTest, sp_not_in_map) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x63000);
  ElfInterfaceFake::FakePushStepData(StepData(0x21004, 0x50020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x63000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x1000U, frame->rel_pc);
  EXPECT_EQ(0x21000U, frame->pc);
  EXPECT_EQ(0x50020U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libunwind.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libunwind.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x20000U, frame->map_info->start());
  EXPECT_EQ(0x22000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify that unwinding stops at the requested function.
TEST_F(UnwinderTest, unwind_stops_at_requested_function) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x1104, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x1204, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  std::map<uint64_t /*function_start*/, uint64_t /*size*/> functions_to_stop_at{{0x1100, 100}};
  unwinder.Unwind(nullptr, nullptr, &functions_to_stop_at);
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x100U, frame->rel_pc);
  EXPECT_EQ(0x1100U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify PC in a device stops the unwind.
TEST_F(UnwinderTest, pc_in_device_stops_unwind) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));

  regs_.set_pc(0x13000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x23002, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x23102, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());
}

// Verify SP in a device stops the unwind.
TEST_F(UnwinderTest, sp_in_device_stops_unwind) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x13000);
  ElfInterfaceFake::FakePushStepData(StepData(0x23002, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x23102, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());
}

// Verify a no map info frame gets a frame.
TEST_F(UnwinderTest, pc_without_map) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0x41000);
  regs_.set_sp(0x13000);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_INVALID_MAP, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x41000U, frame->rel_pc);
  EXPECT_EQ(0x41000U, frame->pc);
  EXPECT_EQ(0x13000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info == nullptr);
}

// Verify that a speculative frame is added.
TEST_F(UnwinderTest, speculative_frame) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));

  // Fake as if code called a nullptr function.
  regs_.set_pc(0);
  regs_.set_sp(0x10000);
  regs_.FakeSetReturnAddress(0x1204);
  regs_.FakeSetReturnAddressValid(true);

  ElfInterfaceFake::FakePushStepData(StepData(0x23104, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(3U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info == nullptr);

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x200U, frame->rel_pc);
  EXPECT_EQ(0x1200U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[2];
  EXPECT_EQ(2U, frame->num);
  EXPECT_EQ(0x100U, frame->rel_pc);
  EXPECT_EQ(0x23100U, frame->pc);
  EXPECT_EQ(0x10020U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/libanother.so", frame->map_info->name());
  EXPECT_EQ("/fake/libanother.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x23000U, frame->map_info->start());
  EXPECT_EQ(0x24000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify that a speculative frame is added then removed because no other
// frames are added.
TEST_F(UnwinderTest, speculative_frame_removed) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));

  // Fake as if code called a nullptr function.
  regs_.set_pc(0x20000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0x10010, false));
  regs_.FakeSetReturnAddress(0x12);
  regs_.FakeSetReturnAddressValid(true);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_INVALID_MAP, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x20000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libunwind.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libunwind.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x20000U, frame->map_info->start());
  EXPECT_EQ(0x22000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info == nullptr);
}

// Verify that a speculative frame is added and left if there are only
// two frames and the pc is in the middle nowhere.
TEST_F(UnwinderTest, speculative_frame_not_removed_pc_bad) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));

  // Fake as if code called a nullptr function.
  regs_.set_pc(0);
  regs_.set_sp(0x10000);
  regs_.FakeSetReturnAddress(0x1204);
  regs_.FakeSetReturnAddressValid(true);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info == nullptr);

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x200U, frame->rel_pc);
  EXPECT_EQ(0x1200U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify that a speculative frame does not cause a crash when it wasn't
// really added due to a filter.
TEST_F(UnwinderTest, speculative_frame_check_with_no_frames) {
  regs_.set_pc(0x23000);
  regs_.set_sp(0x10000);
  regs_.FakeSetReturnAddress(0x23100);
  regs_.FakeSetReturnAddressValid(true);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);

  std::vector<std::string> skip_names{"libanother.so"};
  unwinder.Unwind(&skip_names);
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(0U, unwinder.NumFrames());
}

// Verify that a speculative frame mapping to invalid map doesn't hide error
// for the previous frame.
TEST_F(UnwinderTest, speculative_frame_to_invalid_map_not_hide_prev_error) {
  regs_.set_pc(0x100000);
  regs_.set_sp(0x10000);
  regs_.FakeSetReturnAddress(0x4);
  regs_.FakeSetReturnAddressValid(true);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_INVALID_ELF, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x300U, frame->rel_pc);
  EXPECT_EQ(0x100000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
}

// Verify that an unwind stops when a frame is in given suffix.
TEST_F(UnwinderTest, map_ignore_suffixes) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame3", 3));

  // Fake as if code called a nullptr function.
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x43404, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x53504, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  std::vector<std::string> suffixes{"oat"};
  unwinder.Unwind(nullptr, &suffixes);
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());
  // Make sure the object was not initialized.
  MapInfo* map_info = maps_->Find(0x53000).get();
  ASSERT_TRUE(map_info != nullptr);
  EXPECT_TRUE(map_info->object() == nullptr);

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0x43400U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake.apk", frame->map_info->name());
  EXPECT_EQ("/fake/fake.apk!lib_fake.so", frame->map_info->GetFullName());
  EXPECT_EQ(0x1d000U, frame->map_info->object_start_offset());
  EXPECT_EQ(0x1d000U, frame->map_info->offset());
  EXPECT_EQ(0x43000U, frame->map_info->start());
  EXPECT_EQ(0x44000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

// Verify that an unwind stops when the sp and pc don't change.
TEST_F(UnwinderTest, sp_pc_do_not_change) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame2", 2));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame3", 3));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame4", 4));

  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0x33404, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x33504, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x33504, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x33504, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0x33504, 0x10020, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_REPEATED_FRAME, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(3U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0x33400U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/compressed.so", frame->map_info->name());
  EXPECT_EQ("/fake/compressed.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x33000U, frame->map_info->start());
  EXPECT_EQ(0x34000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[2];
  EXPECT_EQ(2U, frame->num);
  EXPECT_EQ(0x500U, frame->rel_pc);
  EXPECT_EQ(0x33500U, frame->pc);
  EXPECT_EQ(0x10020U, frame->sp);
  EXPECT_EQ("Frame2", frame->function_name);
  EXPECT_EQ(2U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/compressed.so", frame->map_info->name());
  EXPECT_EQ("/fake/compressed.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x33000U, frame->map_info->start());
  EXPECT_EQ(0x34000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, dex_pc_in_map) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  regs_.FakeSetDexPc(0xa3400);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0xa3400U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake.vdex", frame->map_info->name());
  EXPECT_EQ("/fake/fake.vdex", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xa3000U, frame->map_info->start());
  EXPECT_EQ(0xa4000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, dex_pc_in_map_non_zero_offset) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  regs_.FakeSetDexPc(0xd0400);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0xd0400U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake.apk", frame->map_info->name());
  EXPECT_EQ("/fake/fake.apk", frame->map_info->GetFullName());
  EXPECT_EQ(0x1000U, frame->map_info->object_start_offset());
  EXPECT_EQ(0x1000U, frame->map_info->offset());
  EXPECT_EQ(0xd0000U, frame->map_info->start());
  EXPECT_EQ(0xd1000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, dex_pc_not_in_map) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  regs_.FakeSetDexPc(0x50000);

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_DEX_PC_NOT_IN_MAP, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x50000U, frame->rel_pc);
  EXPECT_EQ(0x50000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info == nullptr);

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, dex_pc_not_in_map_valid_dex_files) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  regs_.FakeSetDexPc(0x50000);

  std::unique_ptr<DexFiles> dex_files = CreateDexFiles(regs_.Arch(), process_memory_);
  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.SetDexFiles(dex_files.get());
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_DEX_PC_NOT_IN_MAP, unwinder.warnings());

  ASSERT_EQ(2U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x50000U, frame->rel_pc);
  EXPECT_EQ(0x50000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info == nullptr);

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, dex_pc_multiple_frames) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame1", 1));
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  regs_.FakeSetDexPc(0xa3400);
  ElfInterfaceFake::FakePushStepData(StepData(0x33404, 0x10010, false));
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(3U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0xa3400U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake.vdex", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xa3000U, frame->map_info->start());
  EXPECT_EQ(0xa4000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());

  frame = &unwinder.frames()[1];
  EXPECT_EQ(1U, frame->num);
  EXPECT_EQ(0U, frame->rel_pc);
  EXPECT_EQ(0x1000U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x1000U, frame->map_info->start());
  EXPECT_EQ(0x8000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());

  frame = &unwinder.frames()[2];
  EXPECT_EQ(2U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0x33400U, frame->pc);
  EXPECT_EQ(0x10010U, frame->sp);
  EXPECT_EQ("Frame1", frame->function_name);
  EXPECT_EQ(1U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/compressed.so", frame->map_info->name());
  EXPECT_EQ("/fake/compressed.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0x33000U, frame->map_info->start());
  EXPECT_EQ(0x34000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame->map_info->flags());
}

TEST_F(UnwinderTest, dex_pc_max_frames) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));
  regs_.set_pc(0x1000);
  regs_.set_sp(0x10000);
  regs_.FakeSetDexPc(0xa3400);

  Unwinder unwinder(1, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_MAX_FRAMES_EXCEEDED, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x400U, frame->rel_pc);
  EXPECT_EQ(0xa3400U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/fake/fake.vdex", frame->map_info->name());
  EXPECT_EQ("/fake/fake.vdex", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xa3000U, frame->map_info->start());
  EXPECT_EQ(0xa4000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

TEST_F(UnwinderTest, object_file_not_readable) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0xc0050);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x50U, frame->rel_pc);
  EXPECT_EQ(0xc0050U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_TRUE(frame->map_info->ObjectFileNotReadable());
  EXPECT_EQ("/fake/unreadable.so", frame->map_info->name());
  EXPECT_EQ("/fake/unreadable.so", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xc0000U, frame->map_info->start());
  EXPECT_EQ(0xc1000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

TEST_F(UnwinderTest, elf_from_memory_but_no_valid_file_with_bracket) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0xc1050);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x50U, frame->rel_pc);
  EXPECT_EQ(0xc1050U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("[vdso]", frame->map_info->name());
  EXPECT_EQ("[vdso]", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xc1000U, frame->map_info->start());
  EXPECT_EQ(0xc2000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

TEST_F(UnwinderTest, elf_from_memory_but_empty_filename) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0xc2050);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x50U, frame->rel_pc);
  EXPECT_EQ(0xc2050U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("", frame->map_info->name());
  EXPECT_EQ("", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xc2000U, frame->map_info->start());
  EXPECT_EQ(0xc3000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

TEST_F(UnwinderTest, elf_from_memory_but_from_memfd) {
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 0));

  regs_.set_pc(0xc3050);
  regs_.set_sp(0x10000);
  ElfInterfaceFake::FakePushStepData(StepData(0, 0, true));

  Unwinder unwinder(64, maps_.get(), &regs_, process_memory_);
  unwinder.Unwind();
  EXPECT_EQ(ERROR_NONE, unwinder.LastErrorCode());
  EXPECT_EQ(WARNING_NONE, unwinder.warnings());

  ASSERT_EQ(1U, unwinder.NumFrames());

  auto* frame = &unwinder.frames()[0];
  EXPECT_EQ(0U, frame->num);
  EXPECT_EQ(0x50U, frame->rel_pc);
  EXPECT_EQ(0xc3050U, frame->pc);
  EXPECT_EQ(0x10000U, frame->sp);
  EXPECT_EQ("Frame0", frame->function_name);
  EXPECT_EQ(0U, frame->function_offset);
  ASSERT_TRUE(frame->map_info != nullptr);
  EXPECT_EQ("/memfd:/jit-cache", frame->map_info->name());
  EXPECT_EQ("/memfd:/jit-cache", frame->map_info->GetFullName());
  EXPECT_EQ(0U, frame->map_info->object_start_offset());
  EXPECT_EQ(0U, frame->map_info->offset());
  EXPECT_EQ(0xc3000U, frame->map_info->start());
  EXPECT_EQ(0xc4000U, frame->map_info->end());
  EXPECT_EQ(0U, frame->map_info->GetLoadBias());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame->map_info->flags());
}

// Verify format frame code.
TEST_F(UnwinderTest, format_frame) {
  RegsFake regs_arm(10);
  regs_arm.FakeSetArch(ARCH_ARM);
  Unwinder unwinder32(10, maps_.get(), &regs_arm, process_memory_);

  RegsFake regs_arm64(10);
  regs_arm64.FakeSetArch(ARCH_ARM64);
  Unwinder unwinder64(10, maps_.get(), &regs_arm64, process_memory_);

  FrameData frame;
  frame.num = 1;
  frame.rel_pc = 0x1000;
  frame.pc = 0x4000;
  frame.sp = 0x1000;
  frame.function_name = "function";
  frame.function_offset = 100;
  auto map_info = MapInfo::Create(0x3000, 0x6000, 0, PROT_READ, "/fake/libfake.so");
  map_info->set_object_start_offset(0x2000);
  frame.map_info = map_info;

  EXPECT_EQ("  #01 pc 0000000000001000  /fake/libfake.so (offset 0x2000) (function+100)",
            unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so (offset 0x2000) (function+100)",
            unwinder32.FormatFrame(frame));

  map_info->set_object_start_offset(0);
  EXPECT_EQ("  #01 pc 0000000000001000  /fake/libfake.so (function+100)",
            unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so (function+100)", unwinder32.FormatFrame(frame));

  frame.function_offset = 0;
  EXPECT_EQ("  #01 pc 0000000000001000  /fake/libfake.so (function)",
            unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so (function)", unwinder32.FormatFrame(frame));

  // Verify the function name is demangled.
  frame.function_name = "_ZN4funcEv";
  EXPECT_EQ("  #01 pc 0000000000001000  /fake/libfake.so (func())", unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so (func())", unwinder32.FormatFrame(frame));

  frame.function_name = "";
  EXPECT_EQ("  #01 pc 0000000000001000  /fake/libfake.so", unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so", unwinder32.FormatFrame(frame));

  map_info->name() = "";
  EXPECT_EQ("  #01 pc 0000000000001000  <anonymous:3000>", unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  <anonymous:3000>", unwinder32.FormatFrame(frame));

  frame.map_info = nullptr;
  EXPECT_EQ("  #01 pc 0000000000001000  <unknown>", unwinder64.FormatFrame(frame));
  EXPECT_EQ("  #01 pc 00001000  <unknown>", unwinder32.FormatFrame(frame));
}

TEST_F(UnwinderTest, format_frame_build_id) {
  RegsFake regs(10);
  regs.FakeSetArch(ARCH_ARM);
  Unwinder unwinder(10, maps_.get(), &regs, process_memory_);

  FrameData frame;
  frame.num = 1;
  frame.rel_pc = 0x1000;
  frame.pc = 0x4000;
  frame.sp = 0x1000;
  frame.function_name = "function";
  frame.function_offset = 100;
  frame.map_info = MapInfo::Create(0x3000, 0x6000, 0, PROT_READ, "/fake/libfake.so");
  SharedString* build_id = new SharedString(std::string{0x46, 0x41, 0x4b, 0x45});
  frame.map_info->set_build_id(build_id);

  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so (function+100)", unwinder.FormatFrame(frame));
  unwinder.SetDisplayBuildID(true);
  EXPECT_EQ("  #01 pc 00001000  /fake/libfake.so (function+100) (BuildId: 46414b45)",
            unwinder.FormatFrame(frame));
}

static std::string ArchToString(ArchEnum arch) {
  if (arch == ARCH_ARM) {
    return "Arm";
  } else if (arch == ARCH_ARM64) {
    return "Arm64";
  } else if (arch == ARCH_X86) {
    return "X86";
  } else if (arch == ARCH_X86_64) {
    return "X86_64";
  } else {
    return "Unknown";
  }
}

// Verify format frame code.
TEST_F(UnwinderTest, format_frame_by_arch) {
  std::vector<Regs*> reg_list;
  RegsArm* arm = new RegsArm;
  arm->set_pc(0x2300);
  arm->set_sp(0x10000);
  reg_list.push_back(arm);

  RegsArm64* arm64 = new RegsArm64;
  arm64->set_pc(0x2300);
  arm64->set_sp(0x10000);
  reg_list.push_back(arm64);

  RegsX86* x86 = new RegsX86;
  x86->set_pc(0x2300);
  x86->set_sp(0x10000);
  reg_list.push_back(x86);

  RegsX86_64* x86_64 = new RegsX86_64;
  x86_64->set_pc(0x2300);
  x86_64->set_sp(0x10000);
  reg_list.push_back(x86_64);

  RegsMips* mips = new RegsMips;
  mips->set_pc(0x2300);
  mips->set_sp(0x10000);
  reg_list.push_back(mips);

  RegsMips64* mips64 = new RegsMips64;
  mips64->set_pc(0x2300);
  mips64->set_sp(0x10000);
  reg_list.push_back(mips64);

  for (auto regs : reg_list) {
    ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 10));

    Unwinder unwinder(64, maps_.get(), regs, process_memory_);
    unwinder.Unwind();

    ASSERT_EQ(1U, unwinder.NumFrames());
    std::string expected;
    switch (regs->Arch()) {
      case ARCH_ARM:
      case ARCH_X86:
      case ARCH_MIPS:
        expected = "  #00 pc 00001300  /system/fake/libc.so (Frame0+10)";
        break;
      case ARCH_ARM64:
      case ARCH_X86_64:
      case ARCH_MIPS64:
        expected = "  #00 pc 0000000000001300  /system/fake/libc.so (Frame0+10)";
        break;
      default:
        expected = "";
    }
    EXPECT_EQ(expected, unwinder.FormatFrame(0))
        << "Mismatch of frame format for regs arch " << ArchToString(regs->Arch());
    delete regs;
  }
}

TEST_F(UnwinderTest, build_frame_pc_only_errors) {
  RegsFake regs(10);
  regs.FakeSetArch(ARCH_ARM);
  Unwinder unwinder(10, maps_.get(), &regs, process_memory_);

  FrameData frame;

  // Pc not in map
  frame = unwinder.BuildFrameFromPcOnly(0x10);
  EXPECT_EQ(0x10U, frame.pc);
  EXPECT_EQ(0x10U, frame.rel_pc);

  // No regs set
  unwinder.SetRegs(nullptr);
  frame = unwinder.BuildFrameFromPcOnly(0x100310);
  EXPECT_EQ(0x100310U, frame.pc);
  EXPECT_EQ(0x100310U, frame.rel_pc);
  unwinder.SetRegs(&regs);

  // Invalid elf
  frame = unwinder.BuildFrameFromPcOnly(0x100310);
  EXPECT_EQ(0x10030eU, frame.pc);
  EXPECT_EQ(0x60eU, frame.rel_pc);
  ASSERT_TRUE(frame.map_info != nullptr);
  EXPECT_EQ("/fake/jit.so", frame.map_info->name());
  EXPECT_EQ("/fake/jit.so", frame.map_info->GetFullName());
  EXPECT_EQ(0x100U, frame.map_info->object_start_offset());
  EXPECT_EQ(0x200U, frame.map_info->offset());
  EXPECT_EQ(0x100000U, frame.map_info->start());
  EXPECT_EQ(0x101000U, frame.map_info->end());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame.map_info->flags());
  EXPECT_EQ(0U, frame.map_info->GetLoadBias());
  EXPECT_EQ("", frame.function_name);
  EXPECT_EQ(0U, frame.function_offset);
}

TEST_F(UnwinderTest, build_frame_pc_valid_elf) {
  RegsFake regs(10);
  regs.FakeSetArch(ARCH_ARM);
  Unwinder unwinder(10, maps_.get(), &regs, process_memory_);

  FrameData frame;

  // Valid elf, no function data.
  frame = unwinder.BuildFrameFromPcOnly(0x1010);
  EXPECT_EQ(0x100cU, frame.pc);
  EXPECT_EQ(0xcU, frame.rel_pc);
  ASSERT_TRUE(frame.map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame.map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame.map_info->GetFullName());
  EXPECT_EQ(0U, frame.map_info->object_start_offset());
  EXPECT_EQ(0U, frame.map_info->offset());
  EXPECT_EQ(0x1000U, frame.map_info->start());
  EXPECT_EQ(0x8000U, frame.map_info->end());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame.map_info->flags());
  EXPECT_EQ(0U, frame.map_info->GetLoadBias());
  EXPECT_EQ("", frame.function_name);
  EXPECT_EQ(0U, frame.function_offset);

  // Valid elf, function data present, but do not resolve.
  ElfInterfaceFake::FakePushFunctionData(FunctionData("Frame0", 10));
  unwinder.SetResolveNames(false);

  frame = unwinder.BuildFrameFromPcOnly(0x1010);
  EXPECT_EQ(0x100cU, frame.pc);
  EXPECT_EQ(0xcU, frame.rel_pc);
  ASSERT_TRUE(frame.map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame.map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame.map_info->GetFullName());
  EXPECT_EQ(0U, frame.map_info->object_start_offset());
  EXPECT_EQ(0U, frame.map_info->offset());
  EXPECT_EQ(0x1000U, frame.map_info->start());
  EXPECT_EQ(0x8000U, frame.map_info->end());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame.map_info->flags());
  EXPECT_EQ(0U, frame.map_info->GetLoadBias());
  EXPECT_EQ("", frame.function_name);
  EXPECT_EQ(0U, frame.function_offset);

  // Valid elf, function data present.
  unwinder.SetResolveNames(true);

  frame = unwinder.BuildFrameFromPcOnly(0x1010);
  EXPECT_EQ(0x100cU, frame.pc);
  EXPECT_EQ(0xcU, frame.rel_pc);
  ASSERT_TRUE(frame.map_info != nullptr);
  EXPECT_EQ("/system/fake/libc.so", frame.map_info->name());
  EXPECT_EQ("/system/fake/libc.so", frame.map_info->GetFullName());
  EXPECT_EQ(0U, frame.map_info->object_start_offset());
  EXPECT_EQ(0U, frame.map_info->offset());
  EXPECT_EQ(0x1000U, frame.map_info->start());
  EXPECT_EQ(0x8000U, frame.map_info->end());
  EXPECT_EQ(PROT_READ | PROT_WRITE, frame.map_info->flags());
  EXPECT_EQ(0U, frame.map_info->GetLoadBias());
  EXPECT_EQ("Frame0", frame.function_name);
  EXPECT_EQ(10U, frame.function_offset);
}

TEST_F(UnwinderTest, build_frame_pc_in_jit) {
  // The whole ELF will be copied (read), so it must be valid (readable) memory.
  memory_->SetMemoryBlock(0xf7000, 0x1000, 0);

  Elf32_Ehdr ehdr = {};
  TestInitEhdr<Elf32_Ehdr>(&ehdr, ELFCLASS32, EM_ARM);
  ehdr.e_phoff = 0x50;
  ehdr.e_phnum = 1;
  ehdr.e_phentsize = sizeof(Elf32_Phdr);
  ehdr.e_shoff = 0x100;
  ehdr.e_shstrndx = 1;
  ehdr.e_shentsize = sizeof(Elf32_Shdr);
  ehdr.e_shnum = 3;
  memory_->SetMemory(0xf7000, &ehdr, sizeof(ehdr));

  Elf32_Phdr phdr = {};
  phdr.p_flags = PF_X;
  phdr.p_type = PT_LOAD;
  phdr.p_offset = 0x100000;
  phdr.p_vaddr = 0x100000;
  phdr.p_memsz = 0x1000;
  memory_->SetMemory(0xf7050, &phdr, sizeof(phdr));

  Elf32_Shdr shdr = {};
  shdr.sh_type = SHT_NULL;
  memory_->SetMemory(0xf7100, &shdr, sizeof(shdr));

  shdr.sh_type = SHT_SYMTAB;
  shdr.sh_link = 2;
  shdr.sh_addr = 0x300;
  shdr.sh_offset = 0x300;
  shdr.sh_entsize = sizeof(Elf32_Sym);
  shdr.sh_size = shdr.sh_entsize;
  memory_->SetMemory(0xf7100 + sizeof(shdr), &shdr, sizeof(shdr));

  memset(&shdr, 0, sizeof(shdr));
  shdr.sh_type = SHT_STRTAB;
  shdr.sh_name = 0x500;
  shdr.sh_offset = 0x400;
  shdr.sh_size = 0x100;
  memory_->SetMemory(0xf7100 + 2 * sizeof(shdr), &shdr, sizeof(shdr));

  Elf32_Sym sym = {};
  sym.st_shndx = 2;
  sym.st_info = STT_FUNC;
  sym.st_value = 0x100300;
  sym.st_size = 0x100;
  sym.st_name = 1;
  memory_->SetMemory(0xf7300, &sym, sizeof(sym));
  memory_->SetMemory(0xf7401, "FakeJitFunction");

  RegsFake regs(10);
  regs.FakeSetArch(ARCH_ARM);
  std::unique_ptr<JitDebug> jit_debug = CreateJitDebug(regs.Arch(), process_memory_);
  Unwinder unwinder(10, maps_.get(), &regs, process_memory_);
  unwinder.SetJitDebug(jit_debug.get());

  FrameData frame = unwinder.BuildFrameFromPcOnly(0x100310);
  EXPECT_EQ(0x10030eU, frame.pc);
  EXPECT_EQ(0x60eU, frame.rel_pc);
  ASSERT_TRUE(frame.map_info != nullptr);
  EXPECT_EQ("/fake/jit.so", frame.map_info->name());
  EXPECT_EQ("/fake/jit.so", frame.map_info->GetFullName());
  EXPECT_EQ(0x100U, frame.map_info->object_start_offset());
  EXPECT_EQ(0x200U, frame.map_info->offset());
  EXPECT_EQ(0x100000U, frame.map_info->start());
  EXPECT_EQ(0x101000U, frame.map_info->end());
  EXPECT_EQ(PROT_READ | PROT_WRITE | PROT_EXEC, frame.map_info->flags());
  EXPECT_EQ(0U, frame.map_info->GetLoadBias());
  EXPECT_EQ("FakeJitFunction", frame.function_name);
  EXPECT_EQ(0xeU, frame.function_offset);
}

TEST_F(UnwinderTest, unwinder_from_pid_set_process_memory) {
  auto process_memory = Memory::CreateProcessMemoryCached(getpid());
  UnwinderFromPid unwinder(10, getpid(), process_memory);
  unwinder.SetArch(unwindstack::Regs::CurrentArch());
  ASSERT_TRUE(unwinder.Init());
  ASSERT_EQ(process_memory.get(), unwinder.GetProcessMemory().get());
}

using UnwinderDeathTest = SilentDeathTest;

TEST_F(UnwinderDeathTest, unwinder_from_pid_init_error) {
  UnwinderFromPid unwinder(10, getpid());
  ASSERT_DEATH(unwinder.Init(), "");
}

TEST_F(UnwinderDeathTest, set_jit_debug_error) {
  Maps maps;
  std::shared_ptr<Memory> process_memory(new MemoryFake);
  Unwinder unwinder(10, &maps, process_memory);
  ASSERT_DEATH(CreateJitDebug(ARCH_UNKNOWN, process_memory), "");
}

TEST_F(UnwinderTest, unwinder_from_pid_with_external_maps) {
  LocalMaps map;
  ASSERT_TRUE(map.Parse());

  UnwinderFromPid unwinder1(10, getpid(), &map);
  unwinder1.SetArch(Regs::CurrentArch());
  ASSERT_EQ(&map, unwinder1.GetMaps());
  ASSERT_TRUE(unwinder1.Init());
  ASSERT_EQ(&map, unwinder1.GetMaps());

  UnwinderFromPid unwinder2(10, getpid(), Regs::CurrentArch(), &map);
  ASSERT_EQ(&map, unwinder2.GetMaps());
  ASSERT_TRUE(unwinder2.Init());
  ASSERT_EQ(&map, unwinder2.GetMaps());
}

TEST_F(UnwinderDeathTest, set_dex_files_error) {
#ifndef DEXFILE_SUPPORT
  GTEST_SKIP();
#endif
  Maps maps;
  std::shared_ptr<Memory> process_memory(new MemoryFake);
  Unwinder unwinder(10, &maps, process_memory);
  ASSERT_DEATH(CreateDexFiles(ARCH_UNKNOWN, process_memory), "");
}

}  // namespace unwindstack
