//----------------------------------------
// Orbit Profiler
// Copyright Pierric Gimmig 2013-2017
//----------------------------------------
#pragma once

//-----------------------------------------------------------------------------
// Orbit API
// 
// Simply include this header in your project, there is no lib and no cpp file.
//
// MANUAL INSTRUMENTATION:
// The main feature of Orbit is its ability to dynamically instrument functions
// without having to recompile or even relaunch your application.  However,
// if you still want to manually instrument your code, you can.
//
// Use ORBIT_SCOPE or ORBIT_START/ORBIT_STOP *macros* only.
// DO NOT use OrbitScope(...)/OrbitStart(...)/OrbitStop() directly.
// NOTE: You need to use string literals.  Dynamic strings are not supported yet.
// 
// DLL LOADING:
// If you want to control the loading of Orbit64.dll, i.e. you don't want to have
// to manually inject it into your application, use the OrbitAPI class.  Calling
// OrbitAPI::Connect(...) will load the dll and try to connect to the Orbit instance
// specified in parameters.  Note that Orbit64.dll needs to be next to your
// exectutable.  You can also change the code below (OrbitAPI::Init()).
//-----------------------------------------------------------------------------

#define ORBIT_SCOPE( name )    OrbitScope ORBIT_UNIQUE( ORB )( ORBIT_LITERAL( name ) )
#define ORBIT_START( name )    OrbitStart( ORBIT_LITERAL( name ) )
#define ORBIT_STOP             OrbitStop()
#define ORBIT_SEND( ptr, num ) OrbitSendData( ptr, num )

//-----------------------------------------------------------------------------
struct OrbitAPI
{
    static inline bool Connect( const char* a_Host, int a_Port = 1789 );
    static inline bool IsConnected();

private:
    static inline void Init();

    typedef void( *InitRemote )( char* );
    typedef bool( *GetBool )();

    static inline HINSTANCE  & GetHandle();
    static inline InitRemote & GetInitRemote();
    static inline GetBool    & GetIsConnected();
};

//-----------------------------------------------------------------------------
#define ORBIT_NOOP          static volatile int x; x;
#define ORBIT_LITERAL(x)    ("" x)

//-----------------------------------------------------------------------------
#pragma optimize ("", off) // On purpose to prevent compiler from stripping functions
// NOTE: Do not use these directly, use above macros instead
__declspec( noinline ) inline void OrbitStart( const char* )    { ORBIT_NOOP; }
__declspec( noinline ) inline void OrbitStop()                  { ORBIT_NOOP; }
__declspec( noinline ) inline void OrbitLog( const char* )      { ORBIT_NOOP; }
__declspec( noinline ) inline void OrbitSendData( void*, int )  { ORBIT_NOOP; }
#pragma optimize ("", on)

//-----------------------------------------------------------------------------
struct OrbitScope
{
    OrbitScope( const char * a_Name ) { OrbitStart( a_Name ); }
    ~OrbitScope() { OrbitStop(); }
};

//-----------------------------------------------------------------------------
#define ORBIT_CONCAT_IND(x, y) (x##y)
#define ORBIT_CONCAT(x, y)     ORBIT_CONCAT_IND(x, y)
#define ORBIT_UNIQUE(x)        ORBIT_CONCAT(x, __COUNTER__)

//-----------------------------------------------------------------------------
void OrbitAPI::Init()
{
    HINSTANCE & dllHandle = GetHandle();
    if( !dllHandle )
    {
        const TCHAR * dllName = sizeof(size_t) == 4 ? TEXT("Orbit32.dll") : TEXT("Orbit64.dll");
        if( dllHandle = LoadLibrary( dllName ) )
        {
            GetInitRemote() = (InitRemote)GetProcAddress( dllHandle, "OrbitInitRemote" );
        }
    }
}

//-----------------------------------------------------------------------------
bool OrbitAPI::Connect( const char* a_Host, int a_Port )
{
    Init();
    if( GetHandle() && GetInitRemote() )
    {
        char host[256] = { 0 };
        sprintf_s( host, sizeof( host ), "%s:%i", a_Host, a_Port );
        GetInitRemote()( host );
        return IsConnected();
    }

    return false;
}

//-----------------------------------------------------------------------------
bool OrbitAPI::IsConnected(){ return GetIsConnected() ? GetIsConnected()() : false; }

//-----------------------------------------------------------------------------
HINSTANCE & OrbitAPI::GetHandle(){ static HINSTANCE s_DllHandle; return s_DllHandle; }
OrbitAPI::InitRemote & OrbitAPI::GetInitRemote(){ static InitRemote s_InitRemote; return s_InitRemote; }
OrbitAPI::GetBool    & OrbitAPI::GetIsConnected(){ static GetBool s_IsConnected; return s_IsConnected; }
