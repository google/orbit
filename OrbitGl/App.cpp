//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "TextRenderer.h"
#include "GlCanvas.h"
#include "Capture.h"
#include "ImGuiOrbit.h"
#include "Params.h"
#include "Log.h"
#include "App.h"
#include "ProcessDataView.h"
#include "ModuleDataView.h"
#include "FunctionDataView.h"
#include "LiveFunctionDataView.h"
#include "CallStackDataView.h"
#include "TypeDataView.h"
#include "GlobalDataView.h"
#include "SessionsDataView.h"
#include "SamplingReport.h"
#include "ScopeTimer.h"
#include "Callstack.h"
#include "OrbitSession.h"
#include "Serialization.h"
#include "CaptureWindow.h"
#include "LogDataView.h"
#include "DiaManager.h"
#include "MiniDump.h"
#include "CaptureSerializer.h"
#include "PluginManager.h"
#include "RuleEditor.h"

#include "OrbitAsm/OrbitAsm.h"
#include "OrbitCore/Pdb.h"
#include "OrbitCore/ModuleManager.h"
#include "OrbitCore/TcpServer.h"
#include "OrbitCore/TcpClient.h"
#include "OrbitCore/TimerManager.h"
#include "OrbitCore/Injection.h"
#include "OrbitCore/Utils.h"
#include "OrbitCore/Systrace.h"
#include "Tcp.h"
#include "ConnectionManager.h"
#include "PrintVar.h"
#include "Version.h"
#include "curl/curl.h"
#include "Debugger.h"
#include "PrintVar.h"
#include "TestRemoteMessages.h"
#include "BpfTrace.h"

#include <thread>
#include <cmath>
#include <fstream>
#include <chrono>

#define FREEGLUT_STATIC
#define GLUT_DISABLE_ATEXIT_HACK
#include "GL/freeglut.h"

#ifdef _WIN32
#include "Disassembler.h"
#include "EventTracer.h"
#else
#include "LinuxUtils.h"
#include "LinuxPerf.h"
#endif

class OrbitApp* GOrbitApp;
float GFontSize;
bool DoZoom = false;

#ifdef _WIN32
// TODO: This shouldn't be needed
char ImGuiTextBuffer::EmptyString[1];
#endif

//-----------------------------------------------------------------------------
OrbitApp::OrbitApp()
{
    m_Debugger = nullptr;
#ifdef _WIN32
    m_Debugger = new Debugger();
#endif
}

//-----------------------------------------------------------------------------
OrbitApp::~OrbitApp()
{
#ifdef _WIN32
    oqpi_tk::stop_scheduler();
    delete m_Debugger;
#endif
    GOrbitApp = nullptr;
}

//-----------------------------------------------------------------------------
std::wstring OrbitApp::FindFile( const std::wstring & a_Caption, const std::wstring & a_Dir, const std::wstring & a_Filter )
{
    if( m_FindFileCallback )
    {
         return m_FindFileCallback( a_Caption, a_Dir, a_Filter );
    }
    
    return std::wstring();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCommandLineArguments(const std::vector< std::string > & a_Args)
{ 
    m_Arguments = a_Args;

    for( std::string arg : a_Args )
    {
        if( Contains( arg, "gamelet:" )  )
        { 
            std::string address = Replace(arg, "gamelet:", "");
            Capture::GCaptureHost = s2ws(address);

            GTcpClient = std::make_unique<TcpClient>();
            GTcpClient->AddMainThreadCallback(Msg_RemoteProcess, [=](const Message & a_Msg) { GOrbitApp->OnRemoteProcess(a_Msg); });
            GTcpClient->AddMainThreadCallback(Msg_RemoteProcessList, [=](const Message & a_Msg) { GOrbitApp->OnRemoteProcessList(a_Msg); });
            GTcpClient->AddMainThreadCallback(Msg_RemoteModuleDebugInfo, [=](const Message & a_Msg) { GOrbitApp->OnRemoteModuleDebugInfo(a_Msg); });
            ConnectionManager::Get().ConnectToRemote(address);
            m_ProcessesDataView->SetIsRemote(true);
        }
        else if( Contains( arg, "preset:" ) )
        {
            std::vector< std::string > vec = Tokenize( arg, ":" );
            if( vec.size() > 1 )
            {
                std::string & preset = vec[1];
                Capture::GPresetToLoad = s2ws( preset );
            }
        }
        else if( Contains( arg, "inject:" ) )
        {
            std::vector< std::string > vec = Tokenize( arg, ":" );
            if( vec.size() > 1 )
            {
                std::string & preset = vec[1];
                Capture::GProcessToInject = s2ws( preset );
            }
        }
        else if (Contains(arg, "systrace:"))
        {
            m_PostInitArguments.push_back(arg);               
        }
    }
}

// Get the horizontal and vertical screen sizes in pixel
//-----------------------------------------------------------------------------
void GetDesktopResolution(int& horizontal, int& vertical)
{
#ifdef _WIN32
    RECT desktop;
    // Get a handle to the desktop window
    const HWND hDesktop = GetDesktopWindow();
    // Get the size of screen to the variable desktop
    GetWindowRect(hDesktop, &desktop);
    // The top left corner will have coordinates (0,0)
    // and the bottom right corner will have coordinates
    // (horizontal, vertical)
    horizontal = desktop.right;
    vertical = desktop.bottom;
#endif
}

//-----------------------------------------------------------------------------
void GLoadPdbAsync( const std::vector<std::wstring> & a_Modules )
{
    GModuleManager.LoadPdbAsync( a_Modules, [](){ GOrbitApp->OnPdbLoaded(); } );
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessTimer( Timer* a_Timer, const std::string& a_FunctionName )
{
    if (ConnectionManager::Get().IsRemote())
    {
        // TODO: buffer incoming timers before sending them
        Message Msg(Msg_RemoteTimers);
        Msg.m_Size = sizeof(Timer);
        GTcpServer->Send(Msg, (void*)a_Timer);
    }
    else
    {
        GCurrentTimeGraph->ProcessTimer(*a_Timer);
        ++Capture::GFunctionCountMap[a_Timer->m_FunctionAddress];
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::ProcessCallStack( CallStack* a_CallStack )
{
    if (ConnectionManager::Get().IsRemote())
    {
        // we only need to send the callstack, if it not yet present
        if ( !Capture::GetCallstack(a_CallStack->Hash()) ) {
            std::string messageData = SerializeObjectHumanReadable(*a_CallStack);
            GTcpServer->Send(Msg_RemoteCallStack, (void*) messageData.c_str(), messageData.size());
        }
    }

    Capture::AddCallstack( *a_CallStack );
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSymbol(uint64_t a_Address, const std::string& a_Module, const std::string& a_Name)
{
    if (ConnectionManager::Get().IsRemote())
    {
        LinuxSymbolWithAddress symbol;
        symbol.m_Module = a_Module;
        symbol.m_Name = a_Name;
        symbol.m_Address = a_Address;
        std::string messageData = SerializeObjectHumanReadable(symbol);
        GTcpServer->Send(Msg_RemoteSymbol, (void*)messageData.c_str(), messageData.size());
    }
    auto symbol = std::make_shared<LinuxSymbol>();
    symbol->m_Name = a_Name;
    symbol->m_Module = a_Module;
    Capture::GTargetProcess->AddSymbol( a_Address, symbol );
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadSystrace(const std::string& a_FileName)
{
    SystraceManager::Get().Clear();
    Capture::ClearCaptureData();
    GCurrentTimeGraph->Clear();
    if (Capture::GClearCaptureDataFunc)
    {
        Capture::GClearCaptureDataFunc();
    }

    std::shared_ptr<Systrace> systrace = std::make_shared<Systrace>(a_FileName.c_str());

    for (Function & func : systrace->GetFunctions())
    {
        Capture::GSelectedFunctionsMap[func.GetVirtualAddress()] = &func;
    }
    Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

    for (const auto& timer : systrace->GetTimers())
    {
        GCurrentTimeGraph->ProcessTimer(timer);
        ++Capture::GFunctionCountMap[timer.m_FunctionAddress];
    }

    for (const auto& pair : systrace->GetThreadNames())
    {
        Capture::GTargetProcess->SetThreadName(pair.first, s2ws(pair.second));
    }
    
    SystraceManager::Get().Add(systrace);
    GOrbitApp->FireRefreshCallbacks();
    GOrbitApp->StopCapture();
    DoZoom = true; //TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void OrbitApp::AppendSystrace(const std::string& a_FileName, uint64_t a_TimeOffset)
{
    std::shared_ptr<Systrace> systrace = std::make_shared<Systrace>(a_FileName.c_str(), a_TimeOffset);

    for (Function& func : systrace->GetFunctions())
    {
        Capture::GSelectedFunctionsMap[func.GetVirtualAddress()] = &func;
    }
    Capture::GVisibleFunctionsMap = Capture::GSelectedFunctionsMap;

    for (const auto& timer : systrace->GetTimers())
    {
        GCurrentTimeGraph->ProcessTimer(timer);
        Capture::GFunctionCountMap[timer.m_FunctionAddress];
    }

    for (const auto& pair : systrace->GetThreadNames())
    {
        Capture::GTargetProcess->SetThreadName(pair.first, s2ws(pair.second));
    }

    SystraceManager::Get().Add(systrace);
    GOrbitApp->FireRefreshCallbacks();
    GOrbitApp->StopCapture();
    DoZoom = true; //TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
bool OrbitApp::Init()
{
    GOrbitApp = new OrbitApp();
    GCoreApp = GOrbitApp;
    GTimerManager = std::make_unique<TimerManager>();
    GTcpServer = new TcpServer();

    Path::Init();

    GModuleManager.Init();
    Capture::Init();
    Capture::GSamplingDoneCallback = &OrbitApp::AddSamplingReport;
    Capture::SetLoadPdbAsyncFunc( GLoadPdbAsync );

    #ifdef _WIN32
    DiaManager::InitMsDiaDll();
    oqpi_tk::start_default_scheduler();
    #endif

    GPluginManager.Initialize();

    if( Capture::IsOtherInstanceRunning() )
    {
        ++Capture::GCapturePort;
    }

    GTcpServer->AddCallback( Msg_MiniDump, [=](const Message & a_Msg){ GOrbitApp->OnMiniDump(a_Msg); });
    GTcpServer->Start(Capture::GCapturePort);

    GParams.Load();
    GFontSize = GParams.m_FontSize;
    GOrbitApp->LoadFileMapping();
    GOrbitApp->LoadSymbolsFile();
    OrbitVersion::CheckForUpdate();
    return true;
}

//-----------------------------------------------------------------------------
void OrbitApp::PostInit()
{
    for (std::string& arg : m_PostInitArguments)
    {
        if (Contains(arg, "systrace:"))
        {
            std::string command = Replace(arg, "systrace:", "");
            auto tokens = Tokenize(command, ",");
            if (tokens.size())
            {
                GoToCapture();
                LoadSystrace(tokens[0]);
            }
            for (size_t i = 1; i + 1 < tokens.size(); i += 2)
            {
                AppendSystrace(tokens[i], std::stoull(tokens[i + 1]));
            }
            SystraceManager::Get().Dump();
        }
    }

    if (!GOrbitApp->GetHeadless())
    {
        int my_argc = 0;
        glutInit(&my_argc, NULL);
        glutInitDisplayMode(GLUT_DOUBLE | GLUT_RGB | GLUT_DEPTH);
        GetDesktopResolution(GOrbitApp->m_ScreenRes[0], GOrbitApp->m_ScreenRes[1]);
    }
    else
    {
        // TODO: headless doesn't necessarily means remote...
        ConnectionManager::Get().InitAsRemote();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadFileMapping()
{
    m_FileMapping.clear();
    std::wstring fileName = Path::GetFileMappingFileName();
    if ( !Path::FileExists( fileName ) )
    {
        std::ofstream outfile( ws2s(fileName) );
        outfile << "//-------------------" << std::endl
                << "// Orbit File Mapping" << std::endl
                << "//-------------------" << std::endl
                << "// If the file path in the pdb is \"D:\\NoAccess\\File.cpp\"" << std::endl
                << "// and File.cpp is locally available in \"C:\\Available\\\""  << std::endl
                << "// then enter a file mapping on its own line like so:"        << std::endl
                << "// \"D:\\NoAccess\\File.cpp\" \"C:\\Available\\\"" << std::endl
                << std::endl
                << "\"D:\\NoAccess\" \"C:\\Avalaible\"" << std::endl;

        outfile.close();
    }

    std::wfstream infile(ws2s(fileName));
    if( !infile.fail())
    {
        std::wstring line;
        while( std::getline(infile, line) )
        {
            if( StartsWith( line, L"//") )
                continue;
            
            bool containsQuotes = Contains( line, L"\"" );
            
            std::vector< std::wstring > tokens = Tokenize( line );
            if( tokens.size() == 2 && !containsQuotes )
            {
                m_FileMapping[ ToLower( tokens[0]) ] = ToLower( tokens[1] );
            }
            else
            {
                std::vector< std::wstring > validTokens;
                for( std::wstring token : Tokenize( line, L"\"//") )
                {
                    if( !IsBlank( token ) )
                    {
                        validTokens.push_back( token );
                    }
                }

                if( validTokens.size() > 1 )
                {
                    m_FileMapping[ ToLower( validTokens[0]) ] = ToLower( validTokens[1] );
                }
            }
        }
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadSymbolsFile()
{
    m_SymbolLocations.clear();

    std::wstring fileName = Path::GetSymbolsFileName();
    if( !Path::FileExists( fileName ) )
    {
        std::ofstream outfile( ws2s(fileName) );
        outfile << "//-------------------" << std::endl
            << "// Orbit Symbol Locations" << std::endl
            << "//-------------------" << std::endl
            << "// Orbit will scan the specified directories for pdb files." << std::endl
            << "// Enter one directory per line, like so:" << std::endl
            << "// \"D:\\MyApp\\Release\"" << std::endl
            << "// \"D:\\MySymbolServer\\" << std::endl
            << std::endl;

        outfile.close();
    }

    std::wfstream infile( ws2s(fileName) );
    if( !infile.fail() )
    {
        std::wstring line;
        while( std::getline( infile, line ) )
        {
            if( StartsWith( line, L"//" ) )
                continue;

            std::wstring dir = line;
            if( Path::DirExists( dir ) )
            {
                m_SymbolLocations.push_back( dir );
            }
        }
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::ListSessions()
{
    std::vector< std::wstring > sessionFileNames = Path::ListFiles( Path::GetPresetPath(), L".opr" );
    std::vector< std::shared_ptr< Session > > sessions;
    for( std::wstring & fileName : sessionFileNames )
    {
        std::shared_ptr<Session> session = std::make_shared<Session>();

        std::ifstream file( ws2s(fileName), std::ios::binary );
        if( !file.fail() )
        {
            cereal::BinaryInputArchive archive( file );
            archive( *session );
            file.close();
            session->m_FileName = fileName;
            sessions.push_back(session);
        }
    }

    m_SessionsDataView->SetSessions( sessions );
}

//-----------------------------------------------------------------------------
void OrbitApp::SetRemoteProcess( std::shared_ptr<Process> a_Process )
{
    m_ProcessesDataView->SetRemoteProcess( a_Process );
}

//-----------------------------------------------------------------------------
void OrbitApp::SendRemoteProcess( uint32_t a_PID )
{
    std::shared_ptr<Process> process = m_ProcessesDataView->SelectProcess(a_PID);

    if (process)
    {
        std::string processData = SerializeObjectHumanReadable(*process);
        GTcpServer->Send(Msg_RemoteProcess, (void*)processData.data(), processData.size());
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::RefreshCaptureView()
{
    NeedsRedraw();
    GOrbitApp->FireRefreshCallbacks();
    DoZoom = true; //TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void OrbitApp::AddWatchedVariable( Variable * a_Variable )
{
#ifdef _WIN32
    // Make sure type hierarchy has been generated
    if( Type* type = a_Variable->m_Pdb->GetTypePtrFromId( a_Variable->m_TypeIndex ) )
    {
        type->LoadDiaInfo();
    }

    for( WatchCallback & callback : m_AddToWatchCallbacks )
    {
        callback( a_Variable );
    }
#endif
}

//-----------------------------------------------------------------------------
void OrbitApp::UpdateVariable( Variable * a_Variable )
{
    for( WatchCallback & callback : m_UpdateWatchCallbacks )
    {
        callback( a_Variable );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::ClearWatchedVariables()
{
    if( Capture::GTargetProcess )
    {
        Capture::GTargetProcess->ClearWatchedVariables();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::RefreshWatch()
{
    Capture::GTargetProcess->RefreshWatchedVariables();
}

//-----------------------------------------------------------------------------
void OrbitApp::Disassemble( const std::string & a_FunctionName, DWORD64 a_VirtualAddress, const char * a_MachineCode, size_t a_Size )
{
#ifdef _WIN32
    Disassembler disasm;
    disasm.LOGF( "asm: /* %s */\n", a_FunctionName.c_str() );
    const unsigned char* code = (const unsigned char*)a_MachineCode;
    disasm.Disassemble( code, a_Size, a_VirtualAddress, Capture::GTargetProcess->GetIs64Bit() );
    SendToUiAsync(disasm.GetResult());
#endif
}

//-----------------------------------------------------------------------------
const std::unordered_map<DWORD64, std::shared_ptr<class Rule> > * OrbitApp::GetRules()
{
    return &m_RuleEditor->GetRules();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetLicense( const std::wstring & a_License )
{
    m_License = a_License;
}

//-----------------------------------------------------------------------------
int OrbitApp::OnExit()
{
    if( GTimerManager->m_IsRecording )
        GOrbitApp->StopCapture();

    GParams.Save();
    delete GOrbitApp;
	GTimerManager = nullptr;
    GTcpServer->Stop();
    delete GTcpServer;
    Orbit_ImGui_Shutdown();
    return 0;
}

//-----------------------------------------------------------------------------
Timer GMainTimer;

//-----------------------------------------------------------------------------
void OrbitApp::MainTick()
{
    TRACE_VAR( GMainTimer.QueryMillis() );

    if( GTcpServer )
        GTcpServer->ProcessMainThreadCallbacks();
    if( GTcpClient )
        GTcpClient->ProcessMainThreadCallbacks();

    GMainTimer.Reset();
    Capture::Update();
#ifdef WIN32
    GTcpServer->MainThreadTick();
#endif

    if( Capture::GPresetToLoad != L"" )
    {
        GOrbitApp->OnLoadSession( Capture::GPresetToLoad );
    }
    
    if( Capture::GProcessToInject != L"" )
    {
        std::cout << "Injecting into " << ws2s(Capture::GTargetProcess->GetFullName()) << std::endl;
        std::cout << "Orbit host: " << ws2s(Capture::GCaptureHost) << std::endl;
        GOrbitApp->SelectProcess(Capture::GProcessToInject);
        Capture::InjectRemote();
        exit(0);
    }

#ifdef _WIN32
    GOrbitApp->m_Debugger->MainTick();
#endif
    GOrbitApp->CheckForUpdate();

    ++GOrbitApp->m_NumTicks;

    if( DoZoom )
    {
        GCurrentTimeGraph->UpdateThreadIds();
        GOrbitApp->m_CaptureWindow->ZoomAll();
        GOrbitApp->NeedsRedraw();
        DoZoom = false;
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::CheckForUpdate()
{
    if( !m_HasPromptedForUpdate && OrbitVersion::s_NeedsUpdate )
    {
        SendToUiNow( L"Update" );
        m_HasPromptedForUpdate = true;
    }
}

//-----------------------------------------------------------------------------
std::string OrbitApp::GetVersion()
{
    return OrbitVersion::GetVersion();
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterProcessesDataView( ProcessesDataView* a_Processes )
{
    assert(m_ProcessesDataView == nullptr);
    m_ProcessesDataView = a_Processes;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterModulesDataView( ModulesDataView* a_Modules )
{
    assert(m_ModulesDataView == nullptr);
    assert(m_ProcessesDataView != nullptr);
    m_ModulesDataView = a_Modules;
    m_ProcessesDataView->SetModulesDataView( m_ModulesDataView );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterFunctionsDataView( FunctionsDataView* a_Functions)
{
    m_FunctionsDataView = a_Functions;
    m_Panels.push_back( a_Functions );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterLiveFunctionsDataView( LiveFunctionsDataView* a_Functions )
{
    m_LiveFunctionsDataView = a_Functions;
    m_Panels.push_back( a_Functions );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCallStackDataView( CallStackDataView* a_Callstack )
{
    assert(m_CallStackDataView == nullptr );
    m_CallStackDataView = a_Callstack;
    m_Panels.push_back(a_Callstack);
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterTypesDataView( TypesDataView* a_Types)
{
    m_TypesDataView = a_Types;
    m_Panels.push_back( a_Types );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterGlobalsDataView( GlobalsDataView* a_Globals)
{
    m_GlobalsDataView = a_Globals;
    m_Panels.push_back( a_Globals );
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterSessionsDataView( SessionsDataView* a_Sessions )
{
    m_SessionsDataView = a_Sessions;
    m_Panels.push_back( a_Sessions );
    ListSessions();
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterCaptureWindow( CaptureWindow* a_Capture )
{
    assert(m_CaptureWindow == nullptr);
    m_CaptureWindow = a_Capture;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterOutputLog( LogDataView * a_Log )
{
    assert( m_Log == nullptr );
    m_Log = a_Log;
}

//-----------------------------------------------------------------------------
void OrbitApp::RegisterRuleEditor( RuleEditor* a_RuleEditor )
{
    assert( m_RuleEditor == nullptr );
    m_RuleEditor = a_RuleEditor;
}

//-----------------------------------------------------------------------------
void OrbitApp::NeedsRedraw()
{
    m_CaptureWindow->NeedsUpdate();
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSamplingReport(std::shared_ptr<SamplingProfiler> & a_SamplingProfiler)
{
    auto report = std::make_shared<SamplingReport>(a_SamplingProfiler);

    for( SamplingReportCallback & callback : GOrbitApp->m_SamplingReportsCallbacks )
    {
        callback(report);
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::AddSelectionReport( std::shared_ptr<SamplingProfiler> & a_SamplingProfiler )
{
    auto report = std::make_shared<SamplingReport>( a_SamplingProfiler );

    for( SamplingReportCallback & callback : GOrbitApp->m_SelectionReportCallbacks )
    {
        callback( report );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCode( DWORD64 a_Address )
{
    m_CaptureWindow->FindCode( a_Address );
    SendToUiNow( L"gotocode" );
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCallstack()
{
    SendToUiNow( L"gotocallstack" );
}

//-----------------------------------------------------------------------------
void OrbitApp::GoToCapture()
{
    SendToUiNow( L"gotocapture" );
}

//-----------------------------------------------------------------------------
void OrbitApp::GetDisassembly( DWORD64 a_Address, DWORD a_NumBytesBelow, DWORD a_NumBytes )
{
    std::shared_ptr<Module> module = Capture::GTargetProcess->GetModuleFromAddress( a_Address );
    if( module && module->m_Pdb && Capture::Connect() )
    {
        Message msg( Msg_GetData );
        ULONG64 address = (ULONG64)a_Address - a_NumBytesBelow;
        if( address < module->m_AddressStart )
            address = module->m_AddressStart;
        
        DWORD64 endAddress = address + a_NumBytes;
        if( endAddress > module->m_AddressEnd )
            endAddress = module->m_AddressEnd;

        msg.m_Header.m_DataTransferHeader.m_Address = address;
        msg.m_Header.m_DataTransferHeader.m_Type = DataTransferHeader::Code;
       
        msg.m_Size = (int)a_NumBytes;
        GTcpServer->Send( msg );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::OnOpenPdb( const std::wstring a_FileName )
{
    Capture::GTargetProcess = std::make_shared<Process>();
    std::shared_ptr<Module> mod = std::make_shared<Module>();
    
    mod->m_FullName = ws2s(a_FileName);
    mod->m_Name = ws2s(Path::GetFileName( a_FileName ) );
    mod->m_Directory = ws2s( Path::GetDirectory( a_FileName ) );
    mod->m_PdbName = mod->m_FullName;
    mod->m_FoundPdb = true;
    mod->LoadDebugInfo();

    Capture::GTargetProcess->m_Name = ws2s(Path::StripExtension( s2ws(mod->m_Name) ));
    Capture::GTargetProcess->AddModule( mod );

    m_ModulesDataView->SetProcess( Capture::GTargetProcess );
    Capture::SetTargetProcess( Capture::GTargetProcess );
    GOrbitApp->FireRefreshCallbacks();

    EnqueueModuleToLoad( mod );
    LoadModules();
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLaunchProcess( const std::wstring a_ProcessName, const std::wstring a_WorkingDir, const std::wstring a_Args )
{
#ifdef _WIN32
    m_Debugger->LaunchProcess( a_ProcessName, a_WorkingDir, a_Args );
#endif
}

//-----------------------------------------------------------------------------
std::wstring OrbitApp::GetCaptureFileName()
{
    assert(Capture::GTargetProcess);
    time_t timestamp = std::chrono::system_clock::to_time_t(Capture::GCaptureTimePoint);
    std::wstring timestamp_string = s2ws(OrbitUtils::FormatTime(timestamp));
    return Path::StripExtension( Capture::GTargetProcess->GetName() ) + L"_" + timestamp_string + L".orbit";
}

//-----------------------------------------------------------------------------
std::wstring OrbitApp::GetSessionFileName()
{
    return Capture::GSessionPresets ? Capture::GSessionPresets->m_FileName : L"";
}

//-----------------------------------------------------------------------------
std::wstring OrbitApp::GetSaveFile( const std::wstring & a_Extension )
{
	std::wstring fileName;
	if( m_SaveFileCallback )
		m_SaveFileCallback( a_Extension, fileName );
	return fileName;
}

//-----------------------------------------------------------------------------
void OrbitApp::SetClipboard( const std::wstring & a_Text )
{
    if( m_ClipboardCallback )
        m_ClipboardCallback( a_Text );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnSaveSession( const std::wstring a_FileName )
{
    Capture::SaveSession( a_FileName );
    ListSessions();
    Refresh( DataViewType::SESSIONS );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLoadSession( const std::wstring a_FileName )
{
    std::shared_ptr<Session> session = std::make_shared<Session>();

    std::wstring fileName = Path::GetDirectory( a_FileName ) == L"" ? Path::GetPresetPath() + a_FileName : a_FileName; 

    std::ifstream file( ws2s(fileName) );
    if (!file.fail())
    {
        cereal::BinaryInputArchive archive( file );
        archive(*session);
        if( SelectProcess( Path::GetFileName( session->m_ProcessFullPath ) ) )
        {
            session->m_FileName = fileName;
            Capture::LoadSession( session );
            Capture::GPresetToLoad = L"";
        }

        file.close();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::OnSaveCapture( const std::wstring a_FileName )
{
    CaptureSerializer ar;
    ar.m_TimeGraph = GCurrentTimeGraph;
    ar.Save( a_FileName );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnLoadCapture( const std::wstring a_FileName )
{
    StopCapture();
    Capture::ClearCaptureData();
    GCurrentTimeGraph->Clear();
    if( Capture::GClearCaptureDataFunc )
    {
        Capture::GClearCaptureDataFunc();
    }

    CaptureSerializer ar;
    ar.m_TimeGraph = GCurrentTimeGraph;
    ar.Load( a_FileName );
    StopCapture();
    DoZoom = true; //TODO: remove global, review logic
}

//-----------------------------------------------------------------------------
void GLoadPdbAsync( const std::shared_ptr<Module> & a_Module )
{
    GModuleManager.LoadPdbAsync( a_Module, [](){ GOrbitApp->OnPdbLoaded(); } );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnDisconnect()
{
    GTcpServer->Send( Msg_Unload );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnPdbLoaded()
{
    FireRefreshCallbacks();

    if( m_ModulesToLoad.size() == 0 )
    {
        SendToUiAsync( L"pdbloaded" );
    }
    else
    {
        LoadModules();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::LogMsg( const std::wstring & a_Msg )
{
    ORBIT_LOG( a_Msg );
}

//-----------------------------------------------------------------------------
void OrbitApp::FireRefreshCallbacks( DataViewType a_Type )
{
    for( DataView* panel : m_Panels )
    {
        if( a_Type == DataViewType::ALL || a_Type == panel->GetType() )
        {
            panel->OnDataChanged();
        }
    }

    // UI callbacks
    for( RefreshCallback & callback : m_RefreshCallbacks )
    {
        callback( a_Type );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::AddUiMessageCallback( std::function< void( const std::wstring & ) > a_Callback )
{
    GTcpServer->SetUiCallback( a_Callback );
    m_UiCallback = a_Callback;
}

//-----------------------------------------------------------------------------
void OrbitApp::StartCapture()
{
#ifdef WIN32
    Capture::StartCapture();
#else
    if( Capture::StartCapture() )
    {
        m_BpfTrace = std::make_shared<BpfTrace>();
        m_BpfTrace->Start();
    }
#endif

    if( m_NeedsThawing )
    {
#ifdef _WIN32
        m_Debugger->SendThawMessage();
#endif
        m_NeedsThawing = false;
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::StopCapture()
{
    Capture::StopCapture();

#ifdef __linux__
    if( m_BpfTrace )
    {
        m_BpfTrace->Stop();
    }
#endif

    FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::ToggleCapture()
{
    if( GTimerManager )
    {
        if( GTimerManager->m_IsRecording )
            StopCapture();
        else
            StartCapture();
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::Unregister( DataView * a_Model )
{
    for( size_t i = 0; i < m_Panels.size(); ++i )
    {
        if( m_Panels[i] == a_Model )
        {
            m_Panels.erase( m_Panels.begin()+i );
        }
    }
}

//-----------------------------------------------------------------------------
bool OrbitApp::SelectProcess( const std::wstring & a_Process )
{
    if( m_ProcessesDataView )
    {
        return m_ProcessesDataView->SelectProcess( a_Process );
    }

    return false;
}

//-----------------------------------------------------------------------------
bool OrbitApp::SelectProcess( uint32_t a_ProcessID )
{
    if( m_ProcessesDataView )
    {
        return m_ProcessesDataView->SelectProcess( a_ProcessID ) != nullptr;
    }

    return false;
}

//-----------------------------------------------------------------------------
bool OrbitApp::Inject( unsigned long a_ProcessId )
{
    if( SelectProcess( a_ProcessId ) )
    {
        return Capture::Inject();
    }

    return false;
}

//-----------------------------------------------------------------------------
void OrbitApp::SetCallStack(std::shared_ptr<CallStack> a_CallStack)
{
    m_CallStackDataView->SetCallStack( a_CallStack );
    FireRefreshCallbacks( DataViewType::CALLSTACK );
}

//-----------------------------------------------------------------------------
void OrbitApp::SendToUiAsync( const std::wstring & a_Msg )
{
    GTcpServer->SendToUiAsync( a_Msg );
}

//-----------------------------------------------------------------------------
void OrbitApp::SendToUiNow( const std::wstring & a_Msg )
{
    if( m_UiCallback )
    {
        m_UiCallback( a_Msg );
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::EnqueueModuleToLoad( const std::shared_ptr<Module> & a_Module )
{
    m_ModulesToLoad.push( a_Module );
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadModules()
{
    if( m_ModulesToLoad.size() > 0 )
    {
        if( Capture::IsRemote() )
        {
            LoadRemoteModules();
        }
        else
        {
            std::shared_ptr< Module > module = m_ModulesToLoad.front();
            m_ModulesToLoad.pop();
            GLoadPdbAsync(module);
        }
    }
}

//-----------------------------------------------------------------------------
void OrbitApp::LoadRemoteModules()
{
    std::vector<std::string> modules;
    while (!m_ModulesToLoad.empty())
    {
        auto module = m_ModulesToLoad.front();
        m_ModulesToLoad.pop();
        modules.push_back(module->m_Name);
    }

    std::string moduleData = SerializeObjectHumanReadable(modules);
    GTcpClient->Send(Msg_RemoteModuleDebugInfo, (void*)moduleData.data(), moduleData.size());
}

//-----------------------------------------------------------------------------
bool OrbitApp::IsLoading()
{
    return GPdbDbg->IsLoading();
}

//-----------------------------------------------------------------------------
void OrbitApp::SetTrackContextSwitches( bool a_Value )
{
    GParams.m_TrackContextSwitches = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetTrackContextSwitches()
{
    return GParams.m_TrackContextSwitches;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableUnrealSupport( bool a_Value ) 
{ 
    GParams.m_UnrealSupport = a_Value; 
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetUnrealSupportEnabled()
{ 
    return GParams.m_UnrealSupport; 
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableUnsafeHooking( bool a_Value )
{
    GParams.m_AllowUnsafeHooking = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetUnsafeHookingEnabled()
{
    return GParams.m_AllowUnsafeHooking;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetOutputDebugStringEnabled()
{
    return GParams.m_HookOutputDebugString;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableOutputDebugString( bool a_Value )
{
    GParams.m_HookOutputDebugString = a_Value;
}

//-----------------------------------------------------------------------------
void OrbitApp::EnableSampling( bool a_Value )
{
    GParams.m_TrackSamplingEvents = a_Value;
}

//-----------------------------------------------------------------------------
bool OrbitApp::GetSamplingEnabled()
{
    return GParams.m_TrackSamplingEvents;
}

//-----------------------------------------------------------------------------
void OrbitApp::OnMiniDump( const Message & a_Message )
{
    std::wstring dumpPath = Path::GetDumpPath();
    std::wstring o_File = dumpPath + L"a_received.dmp";
    std::ofstream out( ws2s(o_File), std::ios::binary );
    out.write( a_Message.m_Data, a_Message.m_Size );
    out.close();

    MiniDump miniDump(o_File);
    std::shared_ptr<Process> process = miniDump.ToOrbitProcess();
    process->SetID( (DWORD)a_Message.GetHeader().m_GenericHeader.m_Address );
    GOrbitApp->m_ProcessesDataView->SetRemoteProcess( process );
}

//-----------------------------------------------------------------------------
void OrbitApp::OnRemoteProcess( const Message & a_Message )
{
    std::istringstream buffer(std::string(a_Message.m_Data, a_Message.m_Size));
    cereal::JSONInputArchive inputAr( buffer );
    std::shared_ptr<Process> remoteProcess = std::make_shared<Process>();
    inputAr(*remoteProcess);
    remoteProcess->SetIsRemote(true);
    PRINT_VAR(remoteProcess->GetName());
    GOrbitApp->m_ProcessesDataView->SetRemoteProcess(remoteProcess);
}

//-----------------------------------------------------------------------------
void OrbitApp::OnRemoteProcessList( const Message & a_Message )
{
    std::istringstream buffer(std::string(a_Message.m_Data, a_Message.m_Size));
    cereal::JSONInputArchive inputAr( buffer );
    std::shared_ptr<ProcessList> remoteProcessList = std::make_shared<ProcessList>();
    remoteProcessList->SetRemote(true);
    inputAr(*remoteProcessList);
    PRINT_VAR("remoteProcessList");
    GOrbitApp->m_ProcessesDataView->SetRemoteProcessList(remoteProcessList);
}

//-----------------------------------------------------------------------------
void OrbitApp::OnRemoteModuleDebugInfo(const Message & a_Message)
{
    std::istringstream buffer(std::string(a_Message.m_Data, a_Message.m_Size));
    cereal::JSONInputArchive inputAr(buffer);
    std::vector<ModuleDebugInfo> remoteModuleDebugInfo;
    inputAr(remoteModuleDebugInfo);

    for (auto & moduleInfo : remoteModuleDebugInfo)
    {
        // Get module from name
        std::string name = ToLower(moduleInfo.m_Name);
        std::shared_ptr<Module> module = Capture::GTargetProcess->GetModuleFromName(name);

        if (module)
        {
            module->LoadDebugInfo(); // To allocate m_Pdb - TODO: clean that up

            for (auto& function : moduleInfo.m_Functions)
            {
                // Add function to pdb
                module->m_Pdb->AddFunction(function);
            }

            module->m_Pdb->ProcessData();
            module->SetLoaded(true);
        }
    }

    GOrbitApp->FireRefreshCallbacks();
}

//-----------------------------------------------------------------------------
void OrbitApp::LaunchRuleEditor( Function * a_Function )
{
    m_RuleEditor->m_Window.Launch( a_Function );
    SendToUiNow(TEXT("RuleEditor"));
}
