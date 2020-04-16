inline int myDebugBreak( int ){
    DebugBreak();
    return 0;
}
#define MAXELEMS(x)     (sizeof(x)/sizeof(x[0]))
#define SafeDRef(a, i)  ((i < MAXELEMS(a)) ? a[i] : a[myDebugBreak(i)])

#define MAX_TYPE_IN_DETAIL 5
#define MAX_RVA_LINES_BYTES_RANGE 0x100

extern const wchar_t * const rgBaseType[];
extern const wchar_t * const rgTags[];
extern const wchar_t * const rgFloatPackageStrings[];
extern const wchar_t * const rgProcessorStrings[];
extern const wchar_t * const rgDataKind[];
extern const wchar_t * const rgUdtKind[];
extern const wchar_t * const rgAccess[];
extern const wchar_t * const rgCallingConvention[];
extern const wchar_t * const rgLanguage[];
extern const wchar_t * const rgLocationTypeString[];

std::wstring GetSymTag( DWORD dwSymTag );
std::wstring GetLocation( IDiaSymbol* );
std::wstring GetSymbolType(IDiaSymbol *pSymbol);
std::wstring GetName( IDiaSymbol* );
ULONGLONG GetSize( IDiaSymbol* );
DWORD GetSymbolID( IDiaSymbol* );
DWORD GetTypeID( IDiaSymbol* );
void GetData( IDiaSymbol*, class Type* );
std::wstring GetData( IDiaSymbol* );

void PrintPublicSymbol( IDiaSymbol* );
void PrintGlobalSymbol(IDiaSymbol*);
void OrbitAddGlobalSymbol(IDiaSymbol*);
void PrintSymbol( IDiaSymbol* , DWORD );
void PrintSymTag( DWORD );
void PrintName( IDiaSymbol* );
void PrintUndName( IDiaSymbol* );
void PrintThunk( IDiaSymbol* );
void PrintCompilandDetails( IDiaSymbol* );
void PrintCompilandEnv( IDiaSymbol* );
void PrintLocation( IDiaSymbol* );
void PrintConst( IDiaSymbol* );
void PrintUDT( IDiaSymbol* );
void PrintSymbolType( IDiaSymbol* );
void PrintSymbolTypeNoPrefix( IDiaSymbol* );
void PrintType( IDiaSymbol* );
void PrintBound( IDiaSymbol* );
void PrintData( IDiaSymbol* );
void PrintVariant( VARIANT );
void PrintUdtKind( IDiaSymbol* );
void PrintTypeInDetail( IDiaSymbol* , DWORD );
void PrintFunctionType( IDiaSymbol* );
void PrintSourceFile( IDiaSourceFile* );
void PrintLines( IDiaSession* , IDiaSymbol* );
void PrintLines( IDiaEnumLineNumbers* );
void PrintSource( IDiaSourceFile* );
void PrintSecContribs( IDiaSectionContrib* );
void PrintStreamData( IDiaEnumDebugStreamData* );
void PrintFrameData( IDiaFrameData* );
void PrintPropertyStorage( IDiaPropertyStorage* );

void PrintClassHierarchy( IDiaSymbol* , DWORD, IDiaSymbol* a_Parent = nullptr );

void GetTypeInformation( class Type* a_Type, DWORD a_TagType );
void GetTypeInformation( class Type* a_Type, std::shared_ptr<OrbitDiaSymbol> pSymbol, DWORD a_TagType, DWORD dwIndent );

template<class T> void PrintGeneric( T t ){
  IDiaPropertyStorage* pPropertyStorage;
  
  if(t->QueryInterface( __uuidof(IDiaPropertyStorage), (void **)&pPropertyStorage ) == S_OK){
    PrintPropertyStorage(pPropertyStorage);
    pPropertyStorage->Release();
  }
}

std::string GetCurrentSymTag();

struct OrbitLogger
{
    virtual void Log( const std::wstring & a_Log ) = 0;
};

extern OrbitLogger* GLog;

//-----------------------------------------------------------------------------
struct VizLogger : public OrbitLogger
{
    void Log( const std::wstring & a_Log ) override
    {
        ORBIT_PRINTF(ws2s(a_Log));
    }
};
extern VizLogger GMainLog;

//-----------------------------------------------------------------------------
struct ScopeLog
{
    ScopeLog( OrbitLogger* a_Log ){ GLog = a_Log; }
    ~ScopeLog(){ GLog = &GMainLog; }
};

//-----------------------------------------------------------------------------
struct StringLogger : public OrbitLogger
{
    void Log( const std::wstring & a_Log ) override
    {
        m_String += a_Log;
    }

    std::wstring m_String;
};

template<typename... Args>
inline void LOGF( _In_z_ _Printf_format_string_ const wchar_t* const _Format, Args&&... args )
{
    if( GLog )
    {
        std::wstring log = Format( _Format, std::forward<Args>( args )... );
        GLog->Log( log );
    }
}

#define PRINTF LOGF
#define TYPELOGF LOGF