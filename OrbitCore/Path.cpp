//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Path.h"
#include "Utils.h"
#include <direct.h>
#include <fstream>
#include "Shlobj.h"

std::wstring Path::m_BasePath;
bool         Path::m_IsPackaged;

//-----------------------------------------------------------------------------
void Path::Init()
{
}

//-----------------------------------------------------------------------------
std::wstring Path::GetExecutableName()
{
    WCHAR  cwBuffer[2048] = { 0 };
    LPWSTR pszBuffer = cwBuffer;
    DWORD  dwMaxChars = _countof(cwBuffer);
    DWORD  dwLength = 0;

    dwLength = ::GetModuleFileNameW(NULL, pszBuffer, dwMaxChars);
    wstring exeFullName = wstring(pszBuffer);

    // Clean up "../" inside full path
    wchar_t buffer[MAX_PATH];
    GetFullPathName( exeFullName.c_str(), MAX_PATH, buffer, nullptr );
    exeFullName = buffer;

    std::replace(exeFullName.begin(), exeFullName.end(), '\\', '/');
    return exeFullName;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetExecutablePath()
{
    wstring fullPath = GetExecutableName();
    wstring path = fullPath.substr(0, fullPath.find_last_of(L"/")) + L"/";
    return path;
}

//-----------------------------------------------------------------------------
bool Path::FileExists(const std::wstring & a_File)
{
    ifstream f( a_File.c_str() );
    return f.good();
}

//-----------------------------------------------------------------------------
bool Path::DirExists( const std::wstring & a_Dir )
{
    DWORD ftyp = GetFileAttributesA( ws2s(a_Dir).c_str() );
    if( ftyp == INVALID_FILE_ATTRIBUTES )
        return false;

    if( ftyp & FILE_ATTRIBUTE_DIRECTORY )
        return true;

    return false;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetBasePath()
{
    if( m_BasePath.size() > 0 )
        return m_BasePath;

    wstring exePath = GetExecutablePath();
    m_BasePath = exePath.substr(0, exePath.find(L"bin/"));
    m_IsPackaged = DirExists( GetBasePath() + L"text" );

    return m_BasePath;
}
 
//-----------------------------------------------------------------------------
std::wstring Path::GetOrbitAppPdb()
{
    return GetBasePath() + wstring( L"bin/Win32/Debug/OrbitApp.pdb" );
}

//-----------------------------------------------------------------------------
std::wstring Path::GetDllPath( bool a_Is64Bit )
{
    std::wstring basePath = GetBasePath();
    
#ifdef _DEBUG
    basePath += m_IsPackaged ? L"" : ( a_Is64Bit ? L"bin/x64/Debug/" : L"bin/Win32/Debug/" );
#else
    basePath += m_IsPackaged ? L"" : ( a_Is64Bit ? L"bin/x64/Release/" : L"bin/Win32/Release/" );
#endif

    return basePath + GetDllName( a_Is64Bit );
}

//-----------------------------------------------------------------------------
std::wstring Path::GetDllName( bool a_Is64Bit )
{
    return a_Is64Bit ? L"Orbit64.dll" : L"Orbit32.dll";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetParamsFileName()
{
    std::wstring paramsDir = Path::GetAppDataPath() + L"config/";
    _mkdir( ws2s( paramsDir ).c_str() );
    return paramsDir + L"config.xml";
}

//-----------------------------------------------------------------------------
std::wstring Path::GetFileMappingFileName()
{
    std::wstring paramsDir = Path::GetAppDataPath() + L"config/";
    _mkdir( ws2s( paramsDir ).c_str());
    return paramsDir + wstring( L"FileMapping.txt" );
}

//-----------------------------------------------------------------------------
std::wstring Path::GetSymbolsFileName()
{
    std::wstring paramsDir = Path::GetAppDataPath() + L"config/";
    _mkdir( ws2s( paramsDir ).c_str() );
    return paramsDir + wstring( L"Symbols.txt" );
}

//-----------------------------------------------------------------------------
std::wstring Path::GetLicenseName()
{
    std::wstring appDataDir = Path::GetAppDataPath();
    _mkdir( ws2s( appDataDir ).c_str() );
    return  appDataDir + wstring( L"user.txt" );
}

//-----------------------------------------------------------------------------
std::wstring Path::GetCachePath()
{
    std::wstring cacheDir = Path::GetAppDataPath() + L"cache/";
    _mkdir( ws2s( cacheDir ).c_str() );
    return cacheDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetPresetPath()
{
    std::wstring presetDir = Path::GetAppDataPath() + L"presets/";
    _mkdir( ws2s( presetDir ).c_str() );
    return presetDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetPluginPath()
{
    std::wstring presetDir = Path::GetAppDataPath() + L"plugins/";
    _mkdir( ws2s( presetDir ).c_str() );
    return presetDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetCapturePath()
{
    std::wstring captureDir = Path::GetAppDataPath() + L"output/";
    _mkdir( ws2s( captureDir ).c_str() );
    return captureDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetDumpPath()
{
    std::wstring captureDir = Path::GetAppDataPath() + L"dumps/";
    _mkdir( ws2s( captureDir ).c_str() );
    return captureDir;
}

//-----------------------------------------------------------------------------
wstring Path::GetTmpPath()
{
    std::wstring captureDir = Path::GetAppDataPath() + L"temp/";
    _mkdir( ws2s( captureDir ).c_str() );
    return captureDir;
}

//-----------------------------------------------------------------------------
std::wstring Path::GetFileName( const std::wstring & a_FullName )
{
    std::wstring FullName = a_FullName;
    std::replace( FullName.begin(), FullName.end(), '\\', '/' );
    auto index = FullName.find_last_of( L"/" );
    if( index != wstring::npos )
    {
        std::wstring FileName = FullName.substr( FullName.find_last_of( L"/" ) + 1 );
        return FileName;
    }
    
    return a_FullName;
}

//-----------------------------------------------------------------------------
wstring Path::GetFileNameNoExt( const std::wstring & a_FullName )
{
    return StripExtension( GetFileName( a_FullName ) );
}

//-----------------------------------------------------------------------------
wstring Path::StripExtension( const std::wstring & a_FullName )
{
    size_t index = a_FullName.find_last_of( L"." );
    if( index != std::wstring::npos )
        return a_FullName.substr( 0, index );
    return a_FullName;
}

//-----------------------------------------------------------------------------
wstring Path::GetExtension( const std::wstring & a_FullName )
{
    // returns ".ext" (includes point)
    size_t index = a_FullName.find_last_of( L"." );
    if( index != std::wstring::npos )
        return a_FullName.substr( index, a_FullName.length() );
    return L"";
}

//-----------------------------------------------------------------------------
wstring Path::GetDirectory( const std::wstring & a_FullName )
{
    std::wstring FullName = a_FullName;
    std::replace(FullName.begin(), FullName.end(), '\\', '/');
    auto index = FullName.find_last_of( L"/" );
    if (index != string::npos)
    {
        std::wstring FileName = FullName.substr(0, FullName.find_last_of( L"/" ) + 1);
        return FileName;
    }

    return L"";
}

//-----------------------------------------------------------------------------
wstring Path::GetProgramFilesPath()
{
    TCHAR pf[MAX_PATH] = {0};
    SHGetSpecialFolderPath(
        0,
        pf,
        CSIDL_PROGRAM_FILES,
        FALSE );
    return std::wstring(pf) + L"\\OrbitProfiler\\";
}

//-----------------------------------------------------------------------------
wstring Path::GetAppDataPath()
{
    std::string appData = GetEnvVar( "APPDATA" );
    std::string path = appData + "\\OrbitProfiler\\";
    _mkdir( path.c_str() );
    return s2ws( path );
}

//-----------------------------------------------------------------------------
wstring Path::GetMainDrive()
{
    return s2ws( GetEnvVar("SystemDrive") );
}

//-----------------------------------------------------------------------------
bool Path::IsSourceFile( const std::wstring & a_File )
{
    std::wstring ext = Path::GetExtension( a_File );
    return ext == L".c" || ext == L".cpp" || ext == L".h" || ext == L".hpp" || ext == L".inl" || ext == L".cxx";
}

//-----------------------------------------------------------------------------
std::vector< std::wstring > Path::ListFiles( const std::wstring & a_Dir, std::function< bool(const std::wstring &)> a_Filter )
{
    std::vector< std::wstring > files;

    for( auto it = tr2::sys::recursive_directory_iterator( a_Dir );
        it != tr2::sys::recursive_directory_iterator(); ++it )
    {
        const auto& file = it->path();

        if( !is_directory( file ) && a_Filter( file.wstring() ) )
        {
            files.push_back( file.wstring() );
        }
    }

    return files;
}

//-----------------------------------------------------------------------------
std::vector< std::wstring > Path::ListFiles( const std::wstring & a_Dir, const std::wstring & a_Filter )
{
    return ListFiles( a_Dir, [&]( const std::wstring & a_Name ){ return Contains( a_Name, a_Filter ); } );
}
