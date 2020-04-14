#include <gmock/gmock-matchers.h>
#include <gtest/gtest.h>

#include <utility>

#include "LinuxTracingSession.h"

TEST(LinuxTracingSession, Empty) {
  LinuxTracingSession session(nullptr);

  std::vector<ContextSwitch> context_switches;
  EXPECT_FALSE(session.ReadAllContextSwitches(&context_switches));
  EXPECT_TRUE(context_switches.empty());

  std::vector<Timer> timers;
  EXPECT_FALSE(session.ReadAllTimers(&timers));
  EXPECT_TRUE(timers.empty());

  std::vector<LinuxCallstackEvent> callstacks;
  EXPECT_FALSE(session.ReadAllCallstacks(&callstacks));
  EXPECT_TRUE(callstacks.empty());

  std::vector<CallstackEvent> hashed_callstacks;
  EXPECT_FALSE(session.ReadAllHashedCallstacks(&hashed_callstacks));
  EXPECT_TRUE(hashed_callstacks.empty());
}

TEST(LinuxTracingSession, ContextSwitches) {
  LinuxTracingSession session(nullptr);

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 1;
    context_switch.m_ThreadId = 1;
    context_switch.m_Type = ContextSwitch::Out;
    context_switch.m_Time = 87;
    context_switch.m_ProcessorIndex = 7;
    context_switch.m_ProcessorNumber = 8;

    session.RecordContextSwitch(std::move(context_switch));
  }

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 1;
    context_switch.m_ThreadId = 2;
    context_switch.m_Type = ContextSwitch::In;
    context_switch.m_Time = 78;
    context_switch.m_ProcessorIndex = 17;
    context_switch.m_ProcessorNumber = 18;

    session.RecordContextSwitch(std::move(context_switch));
  }

  std::vector<ContextSwitch> context_switches;
  EXPECT_TRUE(session.ReadAllContextSwitches(&context_switches));
  EXPECT_FALSE(session.ReadAllContextSwitches(&context_switches));

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

    session.RecordContextSwitch(std::move(context_switch));
  }

  // Check that the vector is reset, even if it was not empty
  EXPECT_TRUE(session.ReadAllContextSwitches(&context_switches));
  EXPECT_FALSE(session.ReadAllContextSwitches(&context_switches));

  EXPECT_EQ(context_switches.size(), 1);

  EXPECT_EQ(context_switches[0].m_ProcessId, 11);
  EXPECT_EQ(context_switches[0].m_ThreadId, 12);
  EXPECT_EQ(context_switches[0].m_Time, 187);
  EXPECT_EQ(context_switches[0].m_ProcessorIndex, 27);
  EXPECT_EQ(context_switches[0].m_ProcessorNumber, 28);
}

TEST(LinuxTracingSession, Timers) {
  LinuxTracingSession session(nullptr);

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

    session.RecordTimer(std::move(timer));
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

    session.RecordTimer(std::move(timer));
  }

  std::vector<Timer> timers;
  EXPECT_TRUE(session.ReadAllTimers(&timers));
  EXPECT_FALSE(session.ReadAllTimers(&timers));

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

  // Check that the vector is reset, even if it was not empty
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

    session.RecordTimer(std::move(timer));
  }

  EXPECT_TRUE(session.ReadAllTimers(&timers));
  EXPECT_EQ(timers.size(), 1);

  EXPECT_FALSE(session.ReadAllTimers(&timers));
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

TEST(LinuxTracingSession, Callstacks) {
  LinuxTracingSession session(nullptr);

  {
    LinuxCallstackEvent event;
    event.m_header = "test header 1";
    event.m_time = 1;
    event.m_CS.m_Hash = 0;
    event.m_CS.m_Depth = 2;
    event.m_CS.m_ThreadId = 5;
    event.m_CS.m_Data.push_back(21);
    event.m_CS.m_Data.push_back(22);

    session.RecordCallstack(std::move(event));
  }

  {
    LinuxCallstackEvent event;
    event.m_header = "test header 2";
    event.m_time = 2;
    event.m_CS.m_Hash = 1;
    event.m_CS.m_Depth = 12;
    event.m_CS.m_ThreadId = 15;
    event.m_CS.m_Data.push_back(121);
    event.m_CS.m_Data.push_back(122);

    session.RecordCallstack(std::move(event));
  }

  std::vector<LinuxCallstackEvent> callstacks;
  EXPECT_TRUE(session.ReadAllCallstacks(&callstacks));
  EXPECT_FALSE(session.ReadAllCallstacks(&callstacks));

  EXPECT_EQ(callstacks.size(), 2);

  EXPECT_EQ(callstacks[0].m_header, "test header 1");
  EXPECT_EQ(callstacks[0].m_time, 1);
  EXPECT_EQ(callstacks[0].m_CS.m_Hash, 0);
  EXPECT_EQ(callstacks[0].m_CS.m_Depth, 2);
  EXPECT_EQ(callstacks[0].m_CS.m_ThreadId, 5);
  EXPECT_THAT(callstacks[0].m_CS.m_Data, testing::ElementsAre(21, 22));

  EXPECT_EQ(callstacks[1].m_header, "test header 2");
  EXPECT_EQ(callstacks[1].m_time, 2);
  EXPECT_EQ(callstacks[1].m_CS.m_Hash, 1);
  EXPECT_EQ(callstacks[1].m_CS.m_Depth, 12);
  EXPECT_EQ(callstacks[1].m_CS.m_ThreadId, 15);
  EXPECT_THAT(callstacks[1].m_CS.m_Data, testing::ElementsAre(121, 122));

  {
    LinuxCallstackEvent event;
    event.m_header = "test header 3";
    event.m_time = 3;
    event.m_CS.m_Hash = 21;
    event.m_CS.m_Depth = 22;
    event.m_CS.m_ThreadId = 25;
    event.m_CS.m_Data.push_back(221);
    event.m_CS.m_Data.push_back(222);

    session.RecordCallstack(std::move(event));
  }
  EXPECT_TRUE(session.ReadAllCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_FALSE(session.ReadAllCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_EQ(callstacks[0].m_header, "test header 3");
  EXPECT_EQ(callstacks[0].m_time, 3);
  EXPECT_EQ(callstacks[0].m_CS.m_Hash, 21);
  EXPECT_EQ(callstacks[0].m_CS.m_Depth, 22);
  EXPECT_EQ(callstacks[0].m_CS.m_ThreadId, 25);
  EXPECT_THAT(callstacks[0].m_CS.m_Data, testing::ElementsAre(221, 222));
}

TEST(LinuxTracingSession, HashedCallstacks) {
  LinuxTracingSession session(nullptr);

  session.RecordHashedCallstack(CallstackEvent(11, 12, 13));
  session.RecordHashedCallstack(CallstackEvent(21, 22, 23));

  std::vector<CallstackEvent> callstacks;
  EXPECT_TRUE(session.ReadAllHashedCallstacks(&callstacks));
  EXPECT_FALSE(session.ReadAllHashedCallstacks(&callstacks));

  EXPECT_EQ(callstacks.size(), 2);

  EXPECT_EQ(callstacks[0].m_Time, 11);
  EXPECT_EQ(callstacks[0].m_Id, 12);
  EXPECT_EQ(callstacks[0].m_TID, 13);

  EXPECT_EQ(callstacks[1].m_Time, 21);
  EXPECT_EQ(callstacks[1].m_Id, 22);
  EXPECT_EQ(callstacks[1].m_TID, 23);

  session.RecordHashedCallstack(CallstackEvent(31, 32, 33));

  EXPECT_TRUE(session.ReadAllHashedCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_FALSE(session.ReadAllHashedCallstacks(&callstacks));
  EXPECT_EQ(callstacks.size(), 1);

  EXPECT_EQ(callstacks[0].m_Time, 31);
  EXPECT_EQ(callstacks[0].m_Id, 32);
  EXPECT_EQ(callstacks[0].m_TID, 33);
}

TEST(LinuxTracingSession, Reset) {
  LinuxTracingSession session(nullptr);

  {
    ContextSwitch context_switch;
    context_switch.m_ProcessId = 1;
    context_switch.m_ThreadId = 1;
    context_switch.m_Type = ContextSwitch::Out;
    context_switch.m_Time = 87;
    context_switch.m_ProcessorIndex = 7;
    context_switch.m_ProcessorNumber = 8;

    session.RecordContextSwitch(std::move(context_switch));
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

    session.RecordTimer(std::move(timer));
  }

  {
    LinuxCallstackEvent event;
    event.m_header = "test header 3";
    event.m_time = 3;
    event.m_CS.m_Hash = 21;
    event.m_CS.m_Depth = 22;
    event.m_CS.m_ThreadId = 25;
    event.m_CS.m_Data.push_back(221);
    event.m_CS.m_Data.push_back(222);

    session.RecordCallstack(std::move(event));
  }

  session.RecordHashedCallstack(CallstackEvent(11, 12, 13));

  session.Reset();

  std::vector<ContextSwitch> context_switches;
  EXPECT_FALSE(session.ReadAllContextSwitches(&context_switches));

  std::vector<Timer> timers;
  EXPECT_FALSE(session.ReadAllTimers(&timers));

  std::vector<LinuxCallstackEvent> callstacks;
  EXPECT_FALSE(session.ReadAllCallstacks(&callstacks));

  std::vector<CallstackEvent> hashed_callstacks;
  EXPECT_FALSE(session.ReadAllHashedCallstacks(&hashed_callstacks));
}
