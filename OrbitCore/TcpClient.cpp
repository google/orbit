//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

// clang-format off
#include "OrbitAsio.h"
// clang-format on

#include "TcpClient.h"

#include <thread>

#include "BaseTypes.h"
#include "Core.h"
#include "Hijacking.h"
#include "Log.h"
#include "OrbitLib.h"
#include "OrbitType.h"
#include "Tcp.h"
#include "absl/strings/str_format.h"
#include "OrbitBase/Logging.h"

std::unique_ptr<TcpClient> GTcpClient;

#ifdef __linux__
inline bool IsBadWritePtr(void*, int) { return false; }
inline bool IsBadReadPtr(void*, int) { return false; }
#endif

//-----------------------------------------------------------------------------
TcpClient::TcpClient() {}

//-----------------------------------------------------------------------------
TcpClient::TcpClient(const std::string& a_Host) { Connect(a_Host); }

//-----------------------------------------------------------------------------
TcpClient::~TcpClient() { Stop(); }

//-----------------------------------------------------------------------------
void TcpClient::Connect(const std::string& a_Host) {
  PRINT_FUNC;
  PRINT_VAR(a_Host);
  std::vector<std::string> vec = Tokenize(a_Host, ":");
  std::string& host = vec[0];
  std::string& port = vec[1];

  m_TcpService->m_IoService = new asio::io_service();
  m_TcpSocket->m_Socket = new tcp::socket(*m_TcpService->m_IoService);

  asio::ip::tcp::socket& socket = *m_TcpSocket->m_Socket;
  asio::ip::tcp::resolver resolver(*m_TcpService->m_IoService);
  asio::ip::tcp::resolver::query query(host, port,
                                       tcp::resolver::query::canonical_name);
  asio::ip::tcp::resolver::iterator endpoint_iterator = resolver.resolve(query);
  asio::ip::tcp::resolver::iterator end;

  // Connect to host
  asio::error_code error = asio::error::host_not_found;
  while (error && endpoint_iterator != end) {
    socket.close();
    auto err = socket.connect(*endpoint_iterator++, error);
    PRINT_VAR(err.message().c_str());
  }

  if (error) {
    PRINT_VAR(error.message().c_str());
    m_IsValid = false;
    return;
  }

  m_IsValid = true;
}

void TcpClient::Stop() {
  const bool inWorkerThread =
      std::this_thread::get_id() == workerThread_.get_id();

  if (!inWorkerThread && workerThread_.joinable()) {
    CHECK(m_TcpService);
    CHECK(m_TcpService->m_IoService);
    m_TcpService->m_IoService->stop();
    workerThread_.join();
  }

  TcpEntity::Stop();
}

//-----------------------------------------------------------------------------
void TcpClient::Start() {
  TcpEntity::Start();

  PRINT_FUNC;
  CHECK(!workerThread_.joinable());
  workerThread_ = std::thread{[this]() { ClientThread(); }};

  std::string msg("Hello from TcpClient");
  this->Send(msg);

  ReadMessage();
}

//-----------------------------------------------------------------------------
void TcpClient::ClientThread() {
  SetCurrentThreadName(L"OrbitTcpClient");
  LOG("io_service started...");
  asio::io_service::work work(*m_TcpService->m_IoService);
  m_TcpService->m_IoService->run();
  LOG("io_service ended...");
}

//-----------------------------------------------------------------------------
void TcpClient::ReadMessage() {
  asio::async_read(*m_TcpSocket->m_Socket,
                   asio::buffer(&m_Message, sizeof(Message)),
                   [this](const asio::error_code& ec, size_t) {
                     if (!ec) {
                       ReadPayload();
                     } else {
                       OnError(ec);
                     }
                   });
}

//-----------------------------------------------------------------------------
void TcpClient::ReadPayload() {
  if (m_Message.m_Size == 0) {
    m_Message.m_Data = nullptr;
    ReadFooter();
  } else {
    m_Payload.resize(m_Message.m_Size);
    asio::async_read(*m_TcpSocket->m_Socket,
                     asio::buffer(m_Payload.data(), m_Message.m_Size),

                     [this](asio::error_code ec, std::size_t /*length*/) {
                       if (!ec) {
                         m_Message.m_Data = m_Payload.data();
                         ReadFooter();
                       } else {
                         OnError(ec);
                       }
                     });
  }
}

//-----------------------------------------------------------------------------
void TcpClient::ReadFooter() {
  unsigned int footer = 0;
  asio::read(*m_TcpSocket->m_Socket, asio::buffer(&footer, 4));
  assert(footer == MAGIC_FOOT_MSG);
  DecodeMessage(m_Message);
  ReadMessage();
}

//-----------------------------------------------------------------------------
void TcpClient::OnError(const std::error_code& ec) {
  if ((ec == asio::error::eof) || (ec == asio::error::connection_reset)) {
    Message msg(Msg_Unload);
    DecodeMessage(msg);
  }

  PRINT_VAR(ec.message().c_str());
  LOG("Closing socket");
  m_IsValid = false;
  Stop();
}

//-----------------------------------------------------------------------------
void TcpClient::DecodeMessage(Message& a_Message) {
  Callback(a_Message);

#ifdef _WIN32
  Message::Header MessageHeader = a_Message.GetHeader();

  switch (a_Message.GetType()) {
    case Msg_String: {
      char* msg = a_Message.GetData();
      std::cout << msg << std::endl;
      PRINT_VAR(msg);
      break;
    }
    case Msg_SetData: {
      const DataTransferHeader& header = MessageHeader.m_DataTransferHeader;
      if (!IsBadWritePtr((void*)header.m_Address, a_Message.m_Size)) {
        memcpy((void*)header.m_Address, a_Message.GetData(), a_Message.m_Size);
      }
      break;
    }
    case Msg_GetData: {
      const DataTransferHeader& header = MessageHeader.m_DataTransferHeader;
      Message msg = a_Message;
      msg.m_Type = Msg_SetData;
      std::vector<char> buffer;

      if (!IsBadReadPtr((void*)header.m_Address, a_Message.m_Size)) {
        buffer.resize(msg.m_Size);
        memcpy(buffer.data(), (void*)header.m_Address, a_Message.m_Size);
      }

      Send(msg, buffer);
      break;
    }
    case Msg_NewSession:
      Message::GSessionID = a_Message.m_SessionID;
      break;
    case Msg_StartCapture:
      Orbit::Start();
      break;
    case Msg_StopCapture:
      Orbit::Stop();
      break;
    case Msg_FunctionHook: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking function at address: %p", address);
        Hijacking::CreateHook(address);
        GTcpClient->Send(Msg_NumInstalledHooks, i + 1);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookZoneStart: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking zone function at address: %p", address);
        Hijacking::CreateZoneStartHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookZoneStop: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking zone function at address: %p", address);
        Hijacking::CreateZoneStopHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookOutputDebugString: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking OutputDebugString function at address: %p", address);
        Hijacking::CreateOutputDebugStringHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookUnrealActor: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking Unreal Actor function at address: %p", address);
        Hijacking::CreateUnrealActorHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookOrbitData: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking OrbitSendData function at address: %p", address);
        Hijacking::CreateSendDataHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookAlloc: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking Alloc function at address: %p", address);
        Hijacking::CreateAllocHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookFree: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Hooking Free function at address: %p", address);
        Hijacking::CreateFreeHook(address);
      }

      Hijacking::EnableHooks(addresses, numAddresses);

      break;
    }
    case Msg_FunctionHookRealloc: {
      // ULONG64* addresses = (ULONG64*)a_Message.GetData();
      // int numAddresses = a_Message.m_Size / sizeof( ULONG64 );
      // for( int i = 0; i < numAddresses; ++i )
      //{
      //    void* address = (void*)addresses[i];
      //    std::string dbgMsg = absl::StrFormat( "Hooking Realloc function at
      //    address: %p\n", address ); OutputDebugStringA( dbgMsg.c_str() );
      //    //Hijacking::CreateReallocHook( address ); TODO
      //}

      // Hijacking::EnableHooks( addresses, numAddresses );

      break;
    }
    case Msg_WaitLoop:
      Hijacking::SuspendBusyLoopThread((OrbitWaitLoop*)a_Message.GetData());
      break;
    case Msg_ThawMainThread:
      Hijacking::ThawMainThread((OrbitWaitLoop*)a_Message.GetData());
      break;
    case Msg_ClearArgTracking: {
      Hijacking::ClearFunctionArguments();
      break;
    }
    case Msg_CallstackTracking: {
      ULONG64* addresses = (ULONG64*)a_Message.GetData();
      uint32_t numAddresses = (uint32_t)a_Message.m_Size / sizeof(ULONG64);
      for (uint32_t i = 0; i < numAddresses; ++i) {
        void* address = (void*)addresses[i];
        LOG("Callstack tracking for address: %p", address);
        Hijacking::TrackCallstack(addresses[i]);
      }
      break;
    }
    case Msg_ArgTracking: {
      const ArgTrackingHeader& header = MessageHeader.m_ArgTrackingHeader;

      Argument* argPtr = (Argument*)a_Message.GetData();
      FunctionArgInfo argInfo;
      argInfo.m_NumStackBytes =
          1;  // used so that epilog knows we need to send argInfo
      for (uint32_t i = 0; i < header.m_NumArgs; ++i) {
        // TODO: x86: Check if arg is actually on stack or in register
        argInfo.m_ArgDataSize += argPtr[i].m_NumBytes;
        argInfo.m_Args.push_back(argPtr[i]);
      }

      Hijacking::SetFunctionArguments(header.m_Function, argInfo);
      break;
    }
    case Msg_Unload: {
      Orbit::Stop();
      break;
    }
    case Msg_OrbitUnrealInfo: {
      OrbitUnrealInfo* info = (OrbitUnrealInfo*)a_Message.GetData();
      Hijacking::SetUnrealInfo(*info);
      break;
    }
    default:
      break;
  }
#endif
}
