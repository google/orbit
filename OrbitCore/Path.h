//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <filesystem>
#include <functional>

using namespace std;

class Path
{
public:
    static void Init();

    static wstring GetExecutableName();
    static wstring GetExecutablePath();
    static wstring GetBasePath();
    static wstring GetOrbitAppPdb();
    static wstring GetDllPath( bool a_Is64Bit );
    static wstring GetDllName( bool a_Is64Bit );
    static wstring GetParamsFileName();
    static wstring GetFileMappingFileName();
    static wstring GetSymbolsFileName();
    static wstring GetLicenseName();
    static wstring GetCachePath();
    static wstring GetPresetPath();
    static wstring GetPluginPath();
    static wstring GetCapturePath();
    static wstring GetDumpPath();
    static wstring GetTmpPath();
    static wstring GetFileName( const std::wstring & a_FullName );
    static wstring GetFileNameNoExt( const std::wstring & a_FullName );
    static wstring StripExtension( const std::wstring & a_FullName );
    static wstring GetExtension( const std::wstring & a_FullName );
    static wstring GetDirectory( const std::wstring & a_FullName );
    static wstring GetProgramFilesPath();
    static wstring GetAppDataPath();
    static wstring GetMainDrive();
    
    static bool FileExists( const std::wstring & a_File );
    static bool DirExists( const std::wstring & a_Dir );
    static bool IsSourceFile( const std::wstring & a_File );
    static bool IsPackaged() { return m_IsPackaged; }

    static std::vector< std::wstring > ListFiles( const std::wstring & a_Dir, std::function< bool(const std::wstring &)> a_Filter = [](const std::wstring &){ return true; });
    static std::vector< std::wstring > ListFiles( const std::wstring & a_Dir, const std::wstring & a_Filter );

private:
    static wstring m_BasePath;
    static bool m_IsPackaged;
};

//#include "Path.cpp"

