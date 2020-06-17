// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



// clang-format off
#include "OrbitAsio.h"
// clang-format on

#include "TcpEntity.h"

#include "Core.h"
#include "Log.h"
#include "Tcp.h"
#include "OrbitBase/Logging.h"

//-----------------------------------------------------------------------------
TcpEntity::TcpEntity()
    : m_NumQueuedEntries(0),
      m_ExitRequested(false),
      m_FlushRequested(false),
      m_NumFlushedItems(0) {
  PRINT_FUNC;
  m_IsValid = false;
  m_TcpSocket = std::make_unique<TcpSocket>();
  m_TcpService = std::make_unique<TcpService>();
}

//-----------------------------------------------------------------------------
TcpEntity::~TcpEntity() { Stop(); }

//-----------------------------------------------------------------------------
void TcpEntity::Start() {
  PRINT_FUNC;
  m_ExitRequested = false;

  CHECK(!senderThread_.joinable());
  senderThread_ = std::thread{[this]() { SendData(); }};
}

//-----------------------------------------------------------------------------
void TcpEntity::Stop() {
  PRINT_FUNC;
  m_ExitRequested = true;
  m_ConditionVariable.signal();

  if (senderThread_.joinable()) {
    senderThread_.join();
  }

  if (m_TcpSocket && m_TcpSocket->m_Socket) {
    if (m_TcpSocket->m_Socket->is_open()) {
      m_TcpSocket->m_Socket->close();
    }
  }
  if (m_TcpService && m_TcpService->m_IoService) {
    m_TcpService->m_IoService->stop();
  }
}

//-----------------------------------------------------------------------------
void TcpEntity::SendMsg(Message& a_Message, const void* a_Payload) {
  TcpPacket buffer(a_Message, a_Payload);
  m_SendQueue.enqueue(buffer);
  ++m_NumQueuedEntries;
  m_ConditionVariable.signal();
}

//-----------------------------------------------------------------------------
void TcpEntity::FlushSendQueue() {
  m_FlushRequested = true;

  const size_t numItems = 4096;
  TcpPacket Timers[numItems];
  m_NumFlushedItems = 0;

  while (!m_ExitRequested) {
    size_t numDequeued = m_SendQueue.try_dequeue_bulk(Timers, numItems);

    if (numDequeued == 0) break;

    m_NumQueuedEntries -= numDequeued;
    m_NumFlushedItems += numDequeued;
  }

  m_FlushRequested = false;
  m_ConditionVariable.signal();
}

//-----------------------------------------------------------------------------
void TcpEntity::SendData() {
  SetCurrentThreadName(L"TcpSender");

  while (!m_ExitRequested) {
    // Wait for non-empty queue
    while ((!m_IsValid || m_NumQueuedEntries <= 0) && !m_ExitRequested) {
      m_ConditionVariable.wait();
    }

    // Send messages
    TcpPacket buffer;
    while (m_IsValid && !m_ExitRequested && !m_FlushRequested &&
           m_SendQueue.try_dequeue(buffer)) {
      --m_NumQueuedEntries;
      TcpSocket* socket = GetSocket();
      if (socket && socket->m_Socket && socket->m_Socket->is_open()) {
        try {
          asio::write(*socket->m_Socket, SharedConstBuffer(buffer));
        } catch (std::exception& e) {
          // We observed std::system_error being thrown by OrbitService when the
          // client is stopped while being debugged. Refer to b/155464095.
          ERROR("asio::write: %s", e.what());
        }
      } else {
        ORBIT_ERROR;
      }
    }
  }
}

//-----------------------------------------------------------------------------
void TcpEntity::Callback(const Message& a_Message) {
  MessageType type = a_Message.GetType();
  // Non main thread
  std::vector<MsgCallback>& callbacks = m_Callbacks[type];
  for (MsgCallback& callback : callbacks) {
    callback(a_Message);
  }

  // Main thread callbacks
  ScopeLock lock(m_Mutex);
  const auto& pair = m_MainThreadCallbacks.find(type);
  if (pair != m_MainThreadCallbacks.end()) {
    std::shared_ptr<MessageOwner> messageOwner =
        std::make_shared<MessageOwner>(a_Message);
    m_MainThreadMessages.push_back(messageOwner);
  }
}

//-----------------------------------------------------------------------------
void TcpEntity::ProcessMainThreadCallbacks() {
  ScopeLock lock(m_Mutex);
  for (auto& message : m_MainThreadMessages) {
    std::vector<MsgCallback>& callbacks =
        m_MainThreadCallbacks[message->GetType()];
    for (MsgCallback& callback : callbacks) {
      callback(*message);
    }
  }

  m_MainThreadMessages.clear();
}
