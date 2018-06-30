//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <filesystem>
#include <functional>

class Path
{
public:
    static void Init();

    static std::wstring GetExecutableName();
    static std::wstring GetExecutablePath();
    static std::wstring GetBasePath();
    static std::wstring GetOrbitAppPdb();
    static std::wstring GetDllPath( bool a_Is64Bit );
    static std::wstring GetDllName( bool a_Is64Bit );
    static std::wstring GetParamsFileName();
    static std::wstring GetFileMappingFileName();
    static std::wstring GetSymbolsFileName();
    static std::wstring GetLicenseName();
    static std::wstring GetCachePath();
    static std::wstring GetPresetPath();
    static std::wstring GetPluginPath();
    static std::wstring GetCapturePath();
    static std::wstring GetDumpPath();
    static std::wstring GetTmpPath();
    static std::wstring GetFileName( const std::wstring & a_FullName );
    static std::wstring GetFileNameNoExt( const std::wstring & a_FullName );
    static std::wstring StripExtension( const std::wstring & a_FullName );
    static std::wstring GetExtension( const std::wstring & a_FullName );
    static std::wstring GetDirectory( const std::wstring & a_FullName );
    static std::wstring GetProgramFilesPath();
    static std::wstring GetAppDataPath();
    static std::wstring GetMainDrive();
    
    static bool FileExists( const std::wstring & a_File );
    static bool DirExists( const std::wstring & a_Dir );
    static bool IsSourceFile( const std::wstring & a_File );
    static bool IsPackaged() { return m_IsPackaged; }

    static std::vector< std::wstring > ListFiles( const std::wstring & a_Dir, std::function< bool(const std::wstring &)> a_Filter = [](const std::wstring &){ return true; });
    static std::vector< std::wstring > ListFiles( const std::wstring & a_Dir, const std::wstring & a_Filter );

private:
    static std::wstring m_BasePath;
    static bool m_IsPackaged;
};

//#include "Path.cpp"

