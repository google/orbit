//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "CrashHandler.h"

#include "Core.h"
#include "OrbitDbgHelp.h"
#include "ScopeTimer.h"
#include "TcpClient.h"

#ifdef _WIN32
#include <client/windows/handler/exception_handler.h>

#include <fstream>

//-----------------------------------------------------------------------------
void SendDumpInternal(const std::wstring& a_Dir, const std::wstring& a_Id);
google_breakpad::ExceptionHandler* GHandler;

//-----------------------------------------------------------------------------
bool DmpFilter(void* context, EXCEPTION_POINTERS* exinfo,
               MDRawAssertionInfo* assertion) {
  return true;
}

//-----------------------------------------------------------------------------
bool DmpCallback(const wchar_t* dump_path, const wchar_t* minidump_id,
                 void* context, EXCEPTION_POINTERS* exinfo,
                 MDRawAssertionInfo* assertion, bool succeeded) {
  PRINT_FUNC;

  std::string dir = Path::GetDumpPath();
  std::string msg = "A crash dump was generated in " + dir;
  MessageBox(NULL, succeeded ? s2ws(msg).c_str() :
             L"Failed to generate crash dump", L"Orbit Crash Handler: ",
             MB_ICONEXCLAMATION | MB_OK);

  return false;
}

//-----------------------------------------------------------------------------
bool OnDemandDmpCallback(const wchar_t* dump_path, const wchar_t* minidump_id,
                         void* context, EXCEPTION_POINTERS* exinfo,
                         MDRawAssertionInfo* assertion, bool succeeded) {
  SendDumpInternal(dump_path, minidump_id);
  return false;
}

//-----------------------------------------------------------------------------
CrashHandler::CrashHandler() {
  // Creates a new ExceptionHandler instance to handle writing minidumps.
  // Before writing a minidump, the optional filter callback will be called.
  // Its return value determines whether or not Breakpad should write a
  // minidump.  Minidump files will be written to dump_path, and the optional
  // callback is called after writing the dump file, as described above.
  // handler_types specifies the types of handlers that should be installed.

  assert(GHandler == nullptr);

  /*GHandler = new google_breakpad::ExceptionHandler(
      Path::GetDumpPath().c_str(),
      DmpFilter,
      DmpCallback,
      0,
      google_breakpad::ExceptionHandler::HANDLER_ALL,
      MiniDumpNormal,
      L"",
      0 );*/
}

//-----------------------------------------------------------------------------
void CrashHandler::SendMiniDump() {
  /*SCOPE_TIMER_LOG(L"CrashHandler::WriteDump");
  google_breakpad::ExceptionHandler::WriteMinidump( Path::GetDumpPath(),
  OnDemandDmpCallback, nullptr );*/
}

//-----------------------------------------------------------------------------
void SendDumpInternal(const std::wstring& a_Dir, const std::wstring& a_Id) {
  std::wstring fileName = a_Dir + a_Id + L".dmp";
  std::ifstream file(fileName.c_str(), std::ios::binary | std::ios::ate);
  std::streamsize size = file.tellg();
  file.seekg(0, std::ios::beg);

  std::vector<char> buffer((unsigned int)size);
  if (file.read(buffer.data(), size)) {
    if (GTcpClient) {
      Message msg(Msg_MiniDump);
      msg.m_Header.m_GenericHeader.m_Address = GetCurrentProcessId();
      GTcpClient->Send(msg, buffer);
    }
  }

  file.close();
}

#endif
