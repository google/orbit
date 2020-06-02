// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "LinuxTracingBuffer.h"

TEST(LinuxTracingBuffer, Empty) {
  LinuxTracingBuffer buffer;

  std::vector<ContextSwitch> context_switches;
  EXPECT_FALSE(buffer.ReadAllContextSwitches(&context_switches));
  EXPECT_TRUE(context_switches.empty());

  std::vector<Timer> timers;
  EXPECT_FALSE(buffer.ReadAllTimers(&timers));
  EXPECT_TRUE(timers.empty());

  std::vector<LinuxCallstackEvent> callstacks;
  EXPECT_FALSE(buffer.ReadAllCallstacks(&callstacks));
  EXPECT_TRUE(callstacks.empty());

  std::vector<CallstackEvent> hashed_callstacks;
  EXPECT_FALSE(buffer.ReadAllHashedCallstacks(&hashed_callstacks));
  EXPECT_TRUE(hashed_callstacks.empty());
}

TEST(LinuxTracingBuffer, ContextSwitches) {
  LinuxTracingBuffer buffer;

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 1;
    context_switch.m_ThreadId = 1;
    context_switch.m_Type = ContextSwitch::Out;
    context_switch.m_Time = 87;
    context_switch.m_ProcessorIndex = 7;
    context_switch.m_ProcessorNumber = 8;

    buffer.RecordContextSwitch(std::move(context_switch));
  }

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 1;
    context_switch.m_ThreadId = 2;
    context_switch.m_Type = ContextSwitch::In;
    context_switch.m_Time = 78;
    context_switch.m_ProcessorIndex = 17;
    context_switch.m_ProcessorNumber = 18;

    buffer.RecordContextSwitch(std::move(context_switch));
  }

  std::vector<ContextSwitch> context_switches;
  EXPECT_TRUE(buffer.ReadAllContextSwitches(&context_switches));
  EXPECT_FALSE(buffer.ReadAllContextSwitches(&context_switches));

  EXPECT_EQ(context_switches.size(), 2);

  EXPECT_EQ(context_switches[0].m_ProcessId, 1);
  EXPECT_EQ(context_switches[0].m_ThreadId, 1);
  EXPECT_EQ(context_switches[0].m_Time, 87);
  EXPECT_EQ(context_switches[0].m_ProcessorIndex, 7);
  EXPECT_EQ(context_switches[0].m_ProcessorNumber, 8);

  EXPECT_EQ(context_switches[1].m_ProcessId, 1);
  EXPECT_EQ(context_switches[1].m_ThreadId, 2);
  EXPECT_EQ(context_switches[1].m_Time, 78);
  EXPECT_EQ(context_switches[1].m_ProcessorIndex, 17);
  EXPECT_EQ(context_switches[1].m_ProcessorNumber, 18);

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 11;
    context_switch.m_ThreadId = 12;
    context_switch.m_Type = ContextSwitch::Out;
    context_switch.m_Time = 187;
    context_switch.m_ProcessorIndex = 27;
    context_switch.m_ProcessorNumber = 28;

    buffer.RecordContextSwitch(std::move(context_switch));
  }

  // Check that the vector is reset, even if it was not empty.
  EXPECT_TRUE(buffer.ReadAllContextSwitches(&context_switches));
  EXPECT_EQ(context_switches.size(), 1);

  EXPECT_FALSE(buffer.ReadAllContextSwitches(&context_switches));
  EXPECT_EQ(context_switches.size(), 1);

  EXPECT_EQ(context_switches[0].m_ProcessId, 11);
  EXPECT_EQ(context_switches[0].m_ThreadId, 12);
  EXPECT_EQ(context_switches[0].m_Time, 187);
  EXPECT_EQ(context_switches[0].m_ProcessorIndex, 27);
  EXPECT_EQ(context_switches[0].m_ProcessorNumber, 28);
}

TEST(LinuxTracingBuffer, Timers) {
  LinuxTracingBuffer buffer;

  {
    Timer timer;
    timer.m_PID = 1;
    timer.m_TID = 1;
    timer.m_Depth = 0;
    timer.m_SessionID = 42;
    timer.m_Type = Timer::CORE_ACTIVITY;
    timer.m_Processor = 1;
    timer.m_CallstackHash = 2;
    timer.m_FunctionAddress = 3;
    timer.m_UserData[0] = 7;
    timer.m_UserData[1] = 77;
    timer.m_Start = 800;
    timer.m_End = 900;

    buffer.RecordTimer(std::move(timer));
  }

  {
    Timer timer;
    timer.m_PID = 1;
    timer.m_TID = 2;
    timer.m_Depth = 0;
    timer.m_SessionID = 42;
    timer.m_Type = Timer::CORE_ACTIVITY;
    timer.m_Processor = 3;
    timer.m_CallstackHash = 4;
    timer.m_FunctionAddress = 1;
    timer.m_UserData[0] = 17;
    timer.m_UserData[1] = 177;
    timer.m_Start = 1800;
    timer.m_End = 1900;

    buffer.RecordTimer(std::move(timer));
  }

  std::vector<Timer> timers;
  EXPECT_TRUE(buffer.ReadAllTimers(&timers));
  EXPECT_FALSE(buffer.ReadAllTimers(&timers));

  EXPECT_EQ(timers.size(), 2);

  EXPECT_EQ(timers[0].m_PID, 1);
  EXPECT_EQ(timers[0].m_TID, 1);
  EXPECT_EQ(timers[0].m_Depth, 0);
  EXPECT_EQ(timers[0].m_SessionID, 42);
  EXPECT_EQ(timers[0].m_Type, Timer::CORE_ACTIVITY);
  EXPECT_EQ(timers[0].m_Processor, 1);
  EXPECT_EQ(timers[0].m_CallstackHash, 2);
  EXPECT_EQ(timers[0].m_FunctionAddress, 3);
  EXPECT_THAT(timers[0].m_UserData, testing::ElementsAre(7, 77));
  EXPECT_EQ(timers[0].m_Start, 800);
  EXPECT_EQ(timers[0].m_End, 900);

  EXPECT_EQ(timers[1].m_PID, 1);
  EXPECT_EQ(timers[1].m_TID, 2);
  EXPECT_EQ(timers[1].m_Depth, 0);
  EXPECT_EQ(timers[1].m_SessionID, 42);
  EXPECT_EQ(timers[1].m_Type, Timer::CORE_ACTIVITY);
  EXPECT_EQ(timers[1].m_Processor, 3);
  EXPECT_EQ(timers[1].m_CallstackHash, 4);
  EXPECT_EQ(timers[1].m_FunctionAddress, 1);
  EXPECT_THAT(timers[1].m_UserData, testing::ElementsAre(17, 177));
  EXPECT_EQ(timers[1].m_Start, 1800);
  EXPECT_EQ(timers[1].m_End, 1900);

  // Check that the vector is reset, even if it was not empty.
  {
    Timer timer;
    timer.m_PID = 11;
    timer.m_TID = 12;
    timer.m_Depth = 10;
    timer.m_SessionID = 42;
    timer.m_Type = Timer::CORE_ACTIVITY;
    timer.m_Processor = 3;
    timer.m_CallstackHash = 4;
    timer.m_FunctionAddress = 1;
    timer.m_UserData[0] = 7;
    timer.m_UserData[1] = 77;
    timer.m_Start = 1800;
    timer.m_End = 1900;

    buffer.RecordTimer(std::move(timer));
  }

  EXPECT_TRUE(buffer.ReadAllTimers(&timers));
  EXPECT_EQ(timers.size(), 1);

  EXPECT_FALSE(buffer.ReadAllTimers(&timers));
  EXPECT_EQ(timers.size(), 1);

  EXPECT_EQ(timers[0].m_PID, 11);
  EXPECT_EQ(timers[0].m_TID, 12);
  EXPECT_EQ(timers[0].m_Depth, 10);
  EXPECT_EQ(timers[0].m_SessionID, 42);
  EXPECT_EQ(timers[0].m_Type, Timer::CORE_ACTIVITY);
  EXPECT_EQ(timers[0].m_Processor, 3);
  EXPECT_EQ(timers[0].m_CallstackHash, 4);
  EXPECT_EQ(timers[0].m_FunctionAddress, 1);
  EXPECT_THAT(timers[0].m_UserData, testing::ElementsAre(7, 77));
  EXPECT_EQ(timers[0].m_Start, 1800);
  EXPECT_EQ(timers[0].m_End, 1900);
}

TEST(LinuxTracingBuffer, Callstacks) {
  LinuxTracingBuffer buffer;

  {
    LinuxCallstackEvent event;
    event.time_ = 1;
    event.callstack_.m_Hash = 0;
    event.callstack_.m_Depth = 2;
    event.callstack_.m_ThreadId = 5;
    event.callstack_.m_Data.push_back(21);
    event.callstack_.m_Data.push_back(22);

    buffer.RecordCallstack(std::move(event));
  }

  {
    LinuxCallstackEvent event;
    event.time_ = 2;
    event.callstack_.m_Hash = 1;
    event.callstack_.m_Depth = 12;
    event.callstack_.m_ThreadId = 15;
    event.callstack_.m_Data.push_back(121);
    event.callstack_.m_Data.push_back(122);

    buffer.RecordCallstack(std::move(event));
  }

  std::vector<LinuxCallstackEvent> callstacks;
  EXPECT_TRUE(buffer.ReadAllCallstacks(&callstacks));
  EXPECT_FALSE(buffer.ReadAllCallstacks(&callstacks));

  EXPECT_EQ(callstacks.size(), 2);

  EXPECT_EQ(callstacks[0].time_, 1);
  EXPECT_EQ(callstacks[0].callstack_.m_Hash, 0);
  EXPECT_EQ(callstacks[0].callstack_.m_Depth, 2);
  EXPECT_EQ(callstacks[0].callstack_.m_ThreadId, 5);
  EXPECT_THAT(callstacks[0].callstack_.m_Data, testing::ElementsAre(21, 22));

  EXPECT_EQ(callstacks[1].time_, 2);
  EXPECT_EQ(callstacks[1].callstack_.m_Hash, 1);
  EXPECT_EQ(callstacks[1].callstack_.m_Depth, 12);
  EXPECT_EQ(callstacks[1].callstack_.m_ThreadId, 15);
  EXPECT_THAT(callstacks[1].callstack_.m_Data, testing::ElementsAre(121, 122));

  {
    LinuxCallstackEvent event;
    event.time_ = 3;
    event.callstack_.m_Hash = 21;
    event.callstack_.m_Depth = 22;
    event.callstack_.m_ThreadId = 25;
    event.callstack_.m_Data.push_back(221);
    event.callstack_.m_Data.push_back(222);

    buffer.RecordCallstack(std::move(event));
  }

  EXPECT_TRUE(buffer.ReadAllCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_FALSE(buffer.ReadAllCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_EQ(callstacks[0].time_, 3);
  EXPECT_EQ(callstacks[0].callstack_.m_Hash, 21);
  EXPECT_EQ(callstacks[0].callstack_.m_Depth, 22);
  EXPECT_EQ(callstacks[0].callstack_.m_ThreadId, 25);
  EXPECT_THAT(callstacks[0].callstack_.m_Data, testing::ElementsAre(221, 222));
}

TEST(LinuxTracingBuffer, HashedCallstacks) {
  LinuxTracingBuffer buffer;

  buffer.RecordHashedCallstack(CallstackEvent(11, 12, 13));
  buffer.RecordHashedCallstack(CallstackEvent(21, 22, 23));

  std::vector<CallstackEvent> callstacks;
  EXPECT_TRUE(buffer.ReadAllHashedCallstacks(&callstacks));
  EXPECT_FALSE(buffer.ReadAllHashedCallstacks(&callstacks));

  EXPECT_EQ(callstacks.size(), 2);

  EXPECT_EQ(callstacks[0].m_Time, 11);
  EXPECT_EQ(callstacks[0].m_Id, 12);
  EXPECT_EQ(callstacks[0].m_TID, 13);

  EXPECT_EQ(callstacks[1].m_Time, 21);
  EXPECT_EQ(callstacks[1].m_Id, 22);
  EXPECT_EQ(callstacks[1].m_TID, 23);

  buffer.RecordHashedCallstack(CallstackEvent(31, 32, 33));

  EXPECT_TRUE(buffer.ReadAllHashedCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_FALSE(buffer.ReadAllHashedCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_EQ(callstacks[0].m_Time, 31);
  EXPECT_EQ(callstacks[0].m_Id, 32);
  EXPECT_EQ(callstacks[0].m_TID, 33);
}

TEST(LinuxTracingBuffer, AddressInfos) {
  LinuxTracingBuffer buffer;

  {
    LinuxAddressInfo address_info{0x11, "module1", "function1", 0x1};
    buffer.RecordAddressInfo(std::move(address_info));
  }

  {
    LinuxAddressInfo address_info{0x22, "module2", "function2", 0x2};
    buffer.RecordAddressInfo(std::move(address_info));
  }

  std::vector<LinuxAddressInfo> address_infos;
  EXPECT_TRUE(buffer.ReadAllAddressInfos(&address_infos));
  EXPECT_FALSE(buffer.ReadAllAddressInfos(&address_infos));

  EXPECT_EQ(address_infos.size(), 2);

  EXPECT_EQ(address_infos[0].address, 0x11);
  EXPECT_EQ(address_infos[0].module_name, "module1");
  EXPECT_EQ(address_infos[0].function_name, "function1");
  EXPECT_EQ(address_infos[0].offset_in_function, 0x1);

  EXPECT_EQ(address_infos[1].address, 0x22);
  EXPECT_EQ(address_infos[1].module_name, "module2");
  EXPECT_EQ(address_infos[1].function_name, "function2");
  EXPECT_EQ(address_infos[1].offset_in_function, 0x2);

  {
    LinuxAddressInfo address_info{0x33, "module3", "function3", 0x3};
    buffer.RecordAddressInfo(std::move(address_info));
  }

  EXPECT_TRUE(buffer.ReadAllAddressInfos(&address_infos));
  EXPECT_EQ(address_infos.size(), 1);

  EXPECT_FALSE(buffer.ReadAllAddressInfos(&address_infos));
  EXPECT_EQ(address_infos.size(), 1);

  EXPECT_EQ(address_infos[0].address, 0x33);
  EXPECT_EQ(address_infos[0].module_name, "module3");
  EXPECT_EQ(address_infos[0].function_name, "function3");
  EXPECT_EQ(address_infos[0].offset_in_function, 0x3);
}

TEST(LinuxTracingBuffer, KeysAndStrings) {
  LinuxTracingBuffer buffer;

  {
    KeyAndString key_and_string{0, "str0"};
    buffer.RecordKeyAndString(std::move(key_and_string));
  }

  buffer.RecordKeyAndString(1, "str1");

  std::vector<KeyAndString> keys_and_strings;
  EXPECT_TRUE(buffer.ReadAllKeysAndStrings(&keys_and_strings));
  EXPECT_FALSE(buffer.ReadAllKeysAndStrings(&keys_and_strings));

  EXPECT_EQ(keys_and_strings.size(), 2);

  EXPECT_EQ(keys_and_strings[0].key, 0);
  EXPECT_EQ(keys_and_strings[0].str, "str0");

  EXPECT_EQ(keys_and_strings[1].key, 1);
  EXPECT_EQ(keys_and_strings[1].str, "str1");

  buffer.RecordKeyAndString(2, "str2");

  EXPECT_TRUE(buffer.ReadAllKeysAndStrings(&keys_and_strings));
  EXPECT_EQ(keys_and_strings.size(), 1);

  EXPECT_FALSE(buffer.ReadAllKeysAndStrings(&keys_and_strings));
  EXPECT_EQ(keys_and_strings.size(), 1);

  EXPECT_EQ(keys_and_strings[0].key, 2);
  EXPECT_EQ(keys_and_strings[0].str, "str2");
}

TEST(LinuxTracingBuffer, ThreadNames) {
  LinuxTracingBuffer buffer;

  buffer.RecordThreadName(1, "thread1");

  {
    TidAndThreadName tid_and_name{2, "thread2"};
    buffer.RecordThreadName(std::move(tid_and_name));
  }

  std::vector<TidAndThreadName> thread_names;
  EXPECT_TRUE(buffer.ReadAllThreadNames(&thread_names));
  EXPECT_FALSE(buffer.ReadAllThreadNames(&thread_names));

  EXPECT_EQ(thread_names.size(), 2);

  EXPECT_EQ(thread_names[0].tid, 1);
  EXPECT_EQ(thread_names[0].thread_name, "thread1");

  EXPECT_EQ(thread_names[1].tid, 2);
  EXPECT_EQ(thread_names[1].thread_name, "thread2");

  buffer.RecordThreadName(3, "thread3");

  EXPECT_TRUE(buffer.ReadAllThreadNames(&thread_names));
  EXPECT_EQ(thread_names.size(), 1);

  EXPECT_FALSE(buffer.ReadAllThreadNames(&thread_names));
  EXPECT_EQ(thread_names.size(), 1);

  EXPECT_EQ(thread_names[0].tid, 3);
  EXPECT_EQ(thread_names[0].thread_name, "thread3");
}

TEST(LinuxTracingBuffer, Reset) {
  LinuxTracingBuffer buffer;

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 1;
    context_switch.m_ThreadId = 1;
    context_switch.m_Type = ContextSwitch::Out;
    context_switch.m_Time = 87;
    context_switch.m_ProcessorIndex = 7;
    context_switch.m_ProcessorNumber = 8;

    buffer.RecordContextSwitch(std::move(context_switch));
  }

  {
    Timer timer;
    timer.m_PID = 1;
    timer.m_TID = 1;
    timer.m_Depth = 0;
    timer.m_SessionID = 42;
    timer.m_Type = Timer::CORE_ACTIVITY;
    timer.m_Processor = 1;
    timer.m_CallstackHash = 2;
    timer.m_FunctionAddress = 3;
    timer.m_UserData[0] = 7;
    timer.m_UserData[1] = 77;
    timer.m_Start = 800;
    timer.m_End = 900;

    buffer.RecordTimer(std::move(timer));
  }

  {
    LinuxCallstackEvent event;
    event.time_ = 3;
    event.callstack_.m_Hash = 21;
    event.callstack_.m_Depth = 22;
    event.callstack_.m_ThreadId = 25;
    event.callstack_.m_Data.push_back(221);
    event.callstack_.m_Data.push_back(222);

    buffer.RecordCallstack(std::move(event));
  }

  buffer.RecordHashedCallstack(CallstackEvent(11, 12, 13));

  buffer.RecordKeyAndString(42, "str42");

  buffer.RecordThreadName(42, "thread42");

  buffer.Reset();

  std::vector<ContextSwitch> context_switches;
  EXPECT_FALSE(buffer.ReadAllContextSwitches(&context_switches));

  std::vector<Timer> timers;
  EXPECT_FALSE(buffer.ReadAllTimers(&timers));

  std::vector<LinuxCallstackEvent> callstacks;
  EXPECT_FALSE(buffer.ReadAllCallstacks(&callstacks));

  std::vector<CallstackEvent> hashed_callstacks;
  EXPECT_FALSE(buffer.ReadAllHashedCallstacks(&hashed_callstacks));

  std::vector<KeyAndString> keys_and_strings;
  EXPECT_FALSE(buffer.ReadAllKeysAndStrings(&keys_and_strings));

  std::vector<TidAndThreadName> thread_names;
  EXPECT_FALSE(buffer.ReadAllThreadNames(&thread_names));
}
