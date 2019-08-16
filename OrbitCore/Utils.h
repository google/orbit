//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "Platform.h"
#include <xxhash.h>
#include <stdio.h>
#include <vector>
#include <algorithm>
#include <string>
#include <unordered_map>
#include <map>
#include <functional>
#include <algorithm>
#include <codecvt>
#include <string.h>
#include <BaseTypes.h>
#include <stdarg.h>
#include <memory>
#include <iomanip>

//-----------------------------------------------------------------------------
inline std::string ws2s( const std::wstring& wstr )
{
    std::string str;
    str.resize( wstr.size() );
    for( std::size_t i = 0; i < str.size(); ++i )
    {
        str[i] = (char)wstr[i];
    }
    
    return str;
}

//-----------------------------------------------------------------------------
inline std::wstring s2ws( const std::string& str )
{
    std::wstring wstr;
    wstr.resize( str.size() );
    for( std::size_t i = 0; i < str.size(); ++i )
    {
        wstr[i] = str[i];
    }

    return wstr;
}

//-----------------------------------------------------------------------------
inline std::string GetEnvVar( const char* a_Var )
{
     std::string var;

#ifdef _WIN32
    char* buf = nullptr;
    size_t sz = 0;
    if( _dupenv_s( &buf, &sz, a_Var ) == 0 && buf != nullptr )
    {
        var = buf;
        free( buf );
    }
#else
    char* env = getenv(a_Var);
   if( env )
       var = env;
#endif

    return var;
}

//-----------------------------------------------------------------------------
inline unsigned long long StringHash( const std::string & a_String )
{
    return XXH64( a_String.data(), a_String.size(), 0xBADDCAFEDEAD10CC );
}

//-----------------------------------------------------------------------------
inline unsigned long long StringHash( const std::wstring & a_String )
{
    return XXH64(a_String.data(), a_String.size()*sizeof(wchar_t), 0xBADDCAFEDEAD10CC);
}

#ifdef _WIN32
#define MemPrintf( Dest, DestSize, Source, ... ) _stprintf_s( Dest, DestSize, Source, __VA_ARGS__ )
#define Log( Msg, ... ) OrbitPrintf( Msg, __VA_ARGS__ )
#endif

//-----------------------------------------------------------------------------
template <typename T, size_t N>
inline size_t SizeOfArray(const T(&)[N])
{
    return N;
}

template <typename T, typename U>
inline void Fill( T& a_Array, U& a_Value )
{
    std::fill( std::begin(a_Array), std::end(a_Array), a_Value );
}

//-----------------------------------------------------------------------------
template<typename ... Args>
std::string Format( const char* format, Args ... args )
{
    const size_t size = 4096;
    char buf[size];
    snprintf( buf, size, format, args ... );
    return std::string( buf );
}

//-----------------------------------------------------------------------------
template<typename ... Args>
std::wstring Format( const wchar_t* format, Args ... args )
{
    const size_t size = 4096;
    wchar_t buf[size];
    swprintf( buf, size, format, args ... );
    return std::wstring( buf );
}

//-----------------------------------------------------------------------------
template<class T>
inline T ToLower( const T & a_Str )
{
    T str = a_Str;
    std::transform( str.begin(), str.end(), str.begin(), ::tolower );
    return str;
}

//-----------------------------------------------------------------------------
inline std::vector< std::string > Tokenize( std::string a_String, const char* a_Delimiters = " " )
{
    std::vector< std::string > tokens;
    char* next_token;
    char* token = strtok_r( &a_String[0], a_Delimiters, &next_token );
    while (token != NULL)
    {
        tokens.push_back( token );
        token = strtok_r( NULL, a_Delimiters, &next_token );
    }

    return tokens;
}

//-----------------------------------------------------------------------------
inline std::vector< std::wstring > Tokenize( std::wstring a_String, const wchar_t* a_Delimiters = L" " )
{
    std::vector< std::wstring > tokens;
    wchar_t* next_token;
    wchar_t* token = wcstok_s(&a_String[0], a_Delimiters, &next_token);
    while (token != NULL)
    {
        tokens.push_back(token);
        token = wcstok_s(NULL, a_Delimiters, &next_token);
    }

    return tokens;
}

//-----------------------------------------------------------------------------
template < class U >
inline bool Contains( const std::string & a_String, const U & a_SubString, bool a_MatchCase = true )
{
    std::string str( a_String );
    std::string substr( a_SubString );

    if( !a_MatchCase )
    {
        std::transform(str.begin(), str.end(), str.begin(), ::tolower);
        std::transform(substr.begin(), substr.end(), substr.begin(), ::tolower);
    }

    return str.find( substr ) != std::string::npos;
}

//-----------------------------------------------------------------------------
template < class U >
inline bool Contains( const std::wstring & a_String, const U & a_SubString, bool a_MatchCase = false )
{
    std::wstring str( a_String );
    std::wstring substr( a_SubString );

    if( !a_MatchCase )
    {
        std::transform( str.begin(), str.end(), str.begin(), ::tolower );
        std::transform( substr.begin(), substr.end(), substr.begin(), ::tolower );
    }

    return str.find( substr ) != std::wstring::npos;
}

//-----------------------------------------------------------------------------
template < class T >
inline void Append( std::vector< T > & a_Dest, const std::vector< T > & a_Source )
{
    a_Dest.insert(std::end(a_Dest), std::begin(a_Source), std::end(a_Source));
}

//-----------------------------------------------------------------------------
inline bool StartsWith( const std::string & a_String, const char* a_Prefix )
{
    return a_String.compare( 0, strlen(a_Prefix), a_Prefix ) == 0;
}

//-----------------------------------------------------------------------------
inline bool StartsWith( const std::wstring & a_String, const wchar_t* a_Prefix )
{
    return a_String.compare( 0, wcslen( a_Prefix ), a_Prefix ) == 0;
}

//-----------------------------------------------------------------------------
inline bool EndsWith( const std::string & a_String, const char* a_Suffix )
{
    size_t strLen = a_String.size();
    size_t len = strlen( a_Suffix );
    if( len <= strLen )
    {
        return a_String.compare( strLen - len, len, a_Suffix ) == 0;
    }
    
    return false;
}

//-----------------------------------------------------------------------------
inline bool EndsWith( const std::wstring & a_String, const wchar_t* a_Suffix )
{
    size_t strLen = a_String.size();
    size_t len = wcslen( a_Suffix );
    if( len <= strLen )
    {
        return a_String.compare( strLen - len, len, a_Suffix ) == 0;
    }

    return false;
}

//-----------------------------------------------------------------------------
inline void RemoveTrailingNewLine( std::string & a_String )
{
    if( EndsWith( a_String, "\n" ) )
    {
        a_String.pop_back();
    }
    else if( EndsWith( a_String, "\r\n" ) )
    {
        a_String.pop_back(); a_String.pop_back();
    }
}

//-----------------------------------------------------------------------------
inline void ReplaceStringInPlace( std::string& subject, const std::string& search,const std::string& replace ) 
{
    size_t pos = 0;
    while( ( pos = subject.find( search, pos ) ) != std::string::npos )
    {
        subject.replace( pos, search.length(), replace );
        pos += std::max( replace.length(), (size_t)1 );
    }
}

//-----------------------------------------------------------------------------
inline void ReplaceStringInPlace( std::wstring& subject, const std::wstring& search, const std::wstring& replace )
{
    size_t pos = 0;
    while( ( pos = subject.find( search, pos ) ) != std::wstring::npos ) {
        subject.replace( pos, search.length(), replace );
        pos += std::max(replace.length(),(size_t)1);
    }
}

//-----------------------------------------------------------------------------
inline std::string Replace( const std::string& a_Subject, const std::string& search, const std::string& replace )
{
    std::string subject = a_Subject;
    size_t pos = 0;
    while( ( pos = subject.find( search, pos ) ) != std::string::npos ) {
        subject.replace( pos, search.length(), replace );
        pos += replace.length();
    }

    return subject;
}

//-----------------------------------------------------------------------------
inline std::wstring Replace( const std::wstring& a_Subject, const std::wstring& search, const std::wstring& replace )
{
    std::wstring subject = a_Subject;
    size_t pos = 0;
    while( ( pos = subject.find( search, pos ) ) != std::wstring::npos ) {
        subject.replace( pos, search.length(), replace );
        pos += replace.length();
    }

    return subject;
}

//-----------------------------------------------------------------------------
inline bool IsBlank(const std::string & a_Str)
{
    return a_Str.find_first_not_of("\t\n ") == std::string::npos;
}

//-----------------------------------------------------------------------------
inline bool IsBlank( const std::wstring & a_Str )
{
    return a_Str.find_first_not_of( L"\t\n " ) == std::string::npos;
}

//-----------------------------------------------------------------------------
inline std::string XorString( std::string a_String )
{
    const char* keys = "carkeys835fdda1";
    const size_t numKeys = strlen( keys );

    for( uint32_t i = 0; i < a_String.size(); i++ )
    {
        a_String[i] = a_String[i] ^ keys[i%numKeys];
    }

    return a_String;
}

//-----------------------------------------------------------------------------
inline std::wstring XorString( std::wstring a_String )
{
    const wchar_t* keys = L"carkeys835fdda1";
    const size_t numKeys = wcslen( keys );

    for( uint32_t i = 0; i < a_String.size(); i++ )
    {
        a_String[i] = a_String[i] ^ keys[i%numKeys];
    }

    return a_String;
}

//-----------------------------------------------------------------------------
std::string GetLastErrorAsString();

//-----------------------------------------------------------------------------
std::string GuidToString( GUID a_Guid );

//-----------------------------------------------------------------------------
inline uint64_t GetMicros(std::string a_TimeStamp)
{
    Replace(a_TimeStamp, ":", "");
    std::vector<std::string> tokens = Tokenize(a_TimeStamp, ".");
    if (tokens.size() != 2)
    {
        return 0;
    }

    uint64_t seconds = atoi(tokens[0].c_str());
    uint64_t micros = atoi(tokens[1].c_str());
    return seconds * 1000000 + micros;
}

//-----------------------------------------------------------------------------
inline void PrintBuffer( const char* a_Buffer, uint32_t a_Size, uint32_t a_Width = 16 )
{
    unsigned char* buffer = (unsigned char*) a_Buffer;
    for (size_t i = 0; i < a_Size; ++i)
    {
        std::cout << std::hex 
                  << std::setfill('0') << std::setw(2) 
                  << (int)buffer[i] << " ";
        
        if( (i+1) % a_Width == 0 )
        {
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;

    for (size_t i = 0; i < a_Size; ++i)
    {
        std::cout << buffer[i];
        
        if( (i+1) % a_Width == 0 )
        {
            std::cout << std::endl;
        }
    }

    std::cout << std::endl;
}

#ifdef _WIN32
//-----------------------------------------------------------------------------
template <typename T> inline std::string ToHexString( T a_Value )
{
    std::stringstream l_StringStream;
    l_StringStream << std::hex << a_Value;
    return l_StringStream.str();
}

//-----------------------------------------------------------------------------
inline LONGLONG FileTimeDiffInMillis( const FILETIME & a_T0, const FILETIME & a_T1 )
{
    __int64 i0 = (__int64(a_T0.dwHighDateTime) << 32) + a_T0.dwLowDateTime;
    __int64 i1 = (__int64(a_T1.dwHighDateTime) << 32) + a_T1.dwLowDateTime;
    return (i1 - i0) / 10000;
}

//-----------------------------------------------------------------------------
class CWindowsMessageToString
{
public:
    static std::string GetStringFromMsg(DWORD dwMessage, bool = true);
};
#endif

//-----------------------------------------------------------------------------
inline std::wstring GetPrettySize( ULONG64 a_Size )
{
    const double KB = 1024.0;
    const double MB = 1024.0*KB;
    const double GB = 1024.0*MB;
    const double TB = 1024.0*GB;

    double size = (double)a_Size;

    if (size < KB)
        return Format(L"%I64u B", a_Size);
    if (size < MB)
        return Format(L"%.2f KB", size / KB);
    if (size < GB)
        return Format(L"%.2f MB", size / MB);
    if (size < TB)
        return Format(L"%.2f GB", size / GB);

    return Format(L"%.2f TB", size / TB);
}

//-----------------------------------------------------------------------------
inline std::string GetPrettyTime( double a_MilliSeconds )
{
    const double Day    = 24*60*60*1000;
    const double Hour   = 60*60*1000;
    const double Minute = 60*1000;
    const double Second = 1000;
    const double Milli  = 1;
    const double Micro  = 0.001;
    const double Nano   = 0.000001;

    std::string res;

    if( a_MilliSeconds < Micro )
        res = Format( "%.3f ns", a_MilliSeconds/Nano );
    else if( a_MilliSeconds < Milli )
        res = Format( "%.3f us", a_MilliSeconds/Micro );
    else if( a_MilliSeconds < Second )
        res = Format( "%.3f ms", a_MilliSeconds );
    else if( a_MilliSeconds < Minute )
        res = Format( "%.3f s", a_MilliSeconds/Second );
    else if( a_MilliSeconds < Hour )
        res = Format( "%.3f min", a_MilliSeconds/Minute );
    else if( a_MilliSeconds < Day )
        res = Format( "%.3f h", a_MilliSeconds/Hour );
    else
        res = Format( "%.3f days", a_MilliSeconds/Day );

    return res;
}

//-----------------------------------------------------------------------------
inline std::wstring GetPrettyTimeW( double a_MilliSeconds )
{
    const double Day = 24 * 60 * 60 * 1000;
    const double Hour = 60 * 60 * 1000;
    const double Minute = 60 * 1000;
    const double Second = 1000;
    const double Milli = 1;
    const double Micro = 0.001;
    const double Nano = 0.000001;

    std::wstring res;

    if( a_MilliSeconds < Micro )
        res = Format( L"%.3f ns", a_MilliSeconds / Nano );
    else if( a_MilliSeconds < Milli )
        res = Format( L"%.3f us", a_MilliSeconds / Micro );
    else if( a_MilliSeconds < Second )
        res = Format( L"%.3f ms", a_MilliSeconds );
    else if( a_MilliSeconds < Minute )
        res = Format( L"%.3f s", a_MilliSeconds / Second );
    else if( a_MilliSeconds < Hour )
        res = Format( L"%.3f min", a_MilliSeconds / Minute );
    else if( a_MilliSeconds < Day )
        res = Format( L"%.3f h", a_MilliSeconds / Hour );
    else
        res = Format( L"%.3f days", a_MilliSeconds / Day );

    return res;
}

//-----------------------------------------------------------------------------
inline std::string GetPrettyBitRate( ULONG64 a_SizeInBytes )
{
    ULONG64 a_Size = 8*a_SizeInBytes;

    const double KB = 1024.0;
    const double MB = 1024.0*KB;
    const double GB = 1024.0*MB;
    const double TB = 1024.0*GB;

    double size = (double)a_Size;

    if( size < KB )
        return Format( "%I64u bit/s", a_Size );
    if( size < MB )
        return Format( "%.2f kbit/s", size / KB );
    if( size < GB )
        return Format( "%.2f Mbit/s", size / MB );
    if( size < TB )
        return Format( "%.2f Gbit/s", size / GB );

    return Format( "%.2f Tbit/s", size / TB );
}

#ifndef WIN32
inline void fopen_s( FILE** fp, const char* fileName, const char* mode )
{
     *(fp)=fopen( fileName, mode);
}
#endif

namespace OrbitUtils
{
    bool VisualStudioOpenFile( char const * a_Filename, unsigned int a_Line );

    //-----------------------------------------------------------------------------
    template< class T >
    inline bool Compare(const T & a, const T & b, bool asc)
    {
        return asc ? a < b : a > b;
    }

	//-----------------------------------------------------------------------------
	template< class T >
	inline bool CompareAsc(const T & a, const T & b)
	{
		return a < b;
	}

	//-----------------------------------------------------------------------------
	template< class T >
	inline bool CompareDesc(const T & a, const T & b)
	{
		return a > b;
	}

    //-----------------------------------------------------------------------------
    template<> inline bool Compare<std::string>(const std::string & a, const std::string & b, bool asc)
    {
        return asc ? a < b : a > b;
    }

    //-----------------------------------------------------------------------------
    template < class Key, class Val >
    std::vector< std::pair< Key, Val > > ValueSort( std::unordered_map<Key, Val> & a_Map
                                                  , std::function<bool(const Val&, const Val&)> a_SortFunc = nullptr )
    {
        typedef std::pair< Key, Val > PairType;
        std::vector< PairType > vec;
        vec.reserve(a_Map.size());

        for (auto & it : a_Map)
        {
            vec.push_back(it);
        }

        if (a_SortFunc)
            std::sort(vec.begin(), vec.end(), [&a_SortFunc](const PairType & a, const PairType & b) { return a_SortFunc(a.second, b.second); });
        else
            std::sort(vec.begin(), vec.end(), [&a_SortFunc](const PairType & a, const PairType & b) { return a.second < b.second; });

        return vec;
    }

    //-----------------------------------------------------------------------------
    template < class Key, class Val >
    std::vector< std::pair< Key, Val > > ValueSort( std::map<Key, Val> & a_Map
        , std::function<bool( const Val&, const Val& )> a_SortFunc = nullptr )
    {
        typedef std::pair< Key, Val > PairType;
        std::vector< PairType > vec;
        vec.reserve( a_Map.size() );

        for( auto & it : a_Map )
        {
            vec.push_back( it );
        }

        if( a_SortFunc )
            std::sort( vec.begin(), vec.end(), [&a_SortFunc]( const PairType & a, const PairType & b ) { return a_SortFunc( a.second, b.second ); } );
        else
            std::sort( vec.begin(), vec.end(), [&a_SortFunc]( const PairType & a, const PairType & b ) { return a.second < b.second; } );

        return vec;
    }

    //-----------------------------------------------------------------------------
    template < class Key, class Val >
    std::vector< std::pair< Key, Val > > ReverseValueSort( std::unordered_map<Key, Val> & a_Map )
    {
        std::function<bool(const Val&, const Val&)> sortFunc = []( const Val& a, const Val& b ){ return a > b; }; 
        return ValueSort( a_Map, sortFunc );
    }

    //-----------------------------------------------------------------------------
    template < class Key, class Val >
    std::vector< std::pair< Key, Val > > ReverseValueSort( std::map<Key, Val> & a_Map )
    {
        std::function<bool( const Val&, const Val& )> sortFunc = []( const Val& a, const Val& b ){ return a > b; };
        return ValueSort( a_Map, sortFunc );
    }

    std::string GetTimeStamp();
    inline std::wstring GetTimeStampW(){ return s2ws(GetTimeStamp()); }
}

#define CONCAT_(x,y) x##y
#define CONCAT(x,y) CONCAT_(x,y)
#define UNIQUE_VAR CONCAT(Unique, __LINE__)
#define UNIQUE_ID  CONCAT(Id_, __LINE__)
#define UNUSED(x) (void)(x)
