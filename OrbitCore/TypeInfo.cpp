// Author: Oleg Starodumov

#include "TypeInfo.h"
#include "Core.h"
#include "Log.h"
#include "Pdb.h"
#include <tchar.h>
//#define _NO_CVCONST_H
#include "cvconst.h"
#include "OrbitDbgHelp.h"
#include <stdio.h>
#include <string>
#include <atlconv.h>

#if 0

//-----------------------------------------------------------------------------
CTypeInfoText::CTypeInfoText( HANDLE hProcess, DWORD64 ModBase  ) 
                            : m_pDump( hProcess, ModBase ) 
                            , m_CurrentFunction( NULL )
{
}

//-----------------------------------------------------------------------------
bool CTypeInfoText::GetTypeName( ULONG Index, const TCHAR* pVarName, TString & pTypeName, TypeInfo* o_TypeInfo ) 
{
    //pTypeName = to_wstring(Index);
    m_Types.insert(Index);
    //return true;

    TString VarName = pVarName ? pVarName : TEXT(""); 
    USES_CONVERSION;
    const int cTempBufSize = 128;
    TypeInfo Info; 

    if( m_pDump.DumpType( Index, Info ) )
    {
        int NumPointers = 0; 
        if( Info.Tag == SymTagPointerType )
        {
            ULONG UndTypeIndex = 0; 
            if( !m_pDump.PointerType( Index, UndTypeIndex, NumPointers ) ) 
            {
                return false; 
            }
                
            TypeInfo PointedTypeInfo;
            if( !m_pDump.DumpType( UndTypeIndex, PointedTypeInfo ) ) 
            {
                return false;
            }

            Type ptrType;
            ptrType.m_Id = Index;
            ptrType.m_PointedTypeId = UndTypeIndex;
            ptrType.m_Name = Format("Pointer to %lu", UndTypeIndex);
            ptrType.m_Length = Info.Info.sPointerTypeInfo.Length;
            ptrType.m_TypeInfo = Info;
            GPdbDbg->AddType(ptrType);

            // Save the index of the type the pointer points to 
            Index = UndTypeIndex;
            m_pDump.DumpType( Index, Info );
        } 


        // Display type name 
        switch( Info.Tag )
        {
        case SymTagBaseType:
        {
            TString baseTypeName = BaseTypeStr(Info.Info.sBaseTypeInfo.BaseType, Info.Info.sBaseTypeInfo.Length);
            pTypeName += baseTypeName;
            Type baseType;
            baseType.m_Id = Index;
            baseType.m_Name = ws2s( baseTypeName );
            baseType.m_Length = Info.Info.sBaseTypeInfo.Length;
            baseType.m_TypeInfo = Info;
            GPdbDbg->AddType( baseType );
            break; 
        }
        case SymTagTypedef: 
            pTypeName += W2T(Info.Name); 
            break; 

        case SymTagUDT: 
            {
                // Is it a class/structure or a union ? 

                if( Info.UdtKind ) 
                {
                    // A class/structure 
                    pTypeName += W2T(Info.Name);

                    // Add the UDT and its base classes to the collection of UDT indexes 
                    ReportUdt( Index ); 

                }
                else 
                {
                    // A union 
                    pTypeName += W2T(Info.Name); 
                }
            }
            break; 

        case SymTagEnum: 
            pTypeName += W2T(Info.Name);
            break; 

        case SymTagFunctionType: 
            {
                // Print whether it is static 

                if( Info.sFunctionTypeInfo.MemberFunction ) 
                    if( Info.sFunctionTypeInfo.StaticFunction ) 
                        pTypeName += _T("static "); 

                if( !GetTypeName( Info.sFunctionTypeInfo.RetTypeIndex, pVarName, pTypeName ) )
                    return false; 

                // Print calling convention
                pTypeName += _T(" "); 
                pTypeName += CallConvStr(Info.sFunctionTypeInfo.CallConv);
                pTypeName += _T(" "); 

                // If member function, save the class type index 
                if( Info.sFunctionTypeInfo.MemberFunction ) 
                {
                    ReportUdt( Info.sFunctionTypeInfo.ClassIndex );
                }
                //TypeName += VarName; 

                // Print parameters 

                pTypeName += _T(" (");

                int NumArgs = (int)Info.sFunctionTypeInfo.Args.size();
                for( int i = 0; i < NumArgs; i++ ) 
                {
                    if (!GetTypeName(Info.sFunctionTypeInfo.Args[i], pVarName, pTypeName))
                        return false; 

                    if( i < ( NumArgs - 1 ) ) 
                        pTypeName += _T(", ");
                }

                pTypeName += _T(")");

                // Print "this" adjustment value 
                if( Info.sFunctionTypeInfo.MemberFunction ) 
                    if( Info.sFunctionTypeInfo.ThisAdjust != 0 )
                    {
                        pTypeName += _T("ThisAdjust: ");

                        TCHAR szBuffer[cTempBufSize+1] = _T(""); 
                        MemPrintf( szBuffer, SizeOfArray(szBuffer), _T("%u"), Info.sFunctionTypeInfo.ThisAdjust );

                        pTypeName += szBuffer;
                    }

            }

            break; 

        case SymTagFunctionArgType: 
            if (!GetTypeName(Info.Info.sFunctionArgTypeInfo.TypeIndex, pVarName, pTypeName))
                return false; 
            break; 

        case SymTagArrayType: 
            {
                TString ArrayTypeName = TEXT("");
                if (!GetTypeName(Info.Info.sArrayTypeInfo.ElementTypeIndex, pVarName, ArrayTypeName))
                    return false;

                pTypeName += ArrayTypeName;

                Type arrayType;
                arrayType.m_Id = Index;
                arrayType.m_Name = ws2s(ArrayTypeName);
                arrayType.m_Length = Info.Info.sArrayTypeInfo.Length;
                arrayType.m_TypeInfo = Info;
                GPdbDbg->AddType(arrayType);

                /*TypeName += _T(" ");
                TypeName += VarName;*/

                std::string varStr = ws2s(VarName);
                if( varStr.find("m_Char") != std::string::npos )
                {
                    ORBIT_LOG("BLAH");
                }

                // Print dimensions 

                for( int i = 0; i < Info.Info.sArrayTypeInfo.NumDimensions; i++ ) 
                {
                    TCHAR szBuffer[cTempBufSize+1] = _T(""); 
                    MemPrintf( szBuffer, SizeOfArray(szBuffer), _T("[%llu]"), Info.Info.sArrayTypeInfo.Dimensions[i] );
                    pTypeName += szBuffer;
                }

            }
            break; 

        default: 
            {
                TCHAR szBuffer[cTempBufSize+1] = _T(""); 
                MemPrintf( szBuffer, SizeOfArray(szBuffer), _T("Unknown(%d)"), Info.Tag ); 
                pTypeName += szBuffer;
            }
            break; 
        }


        // If it is a pointer, display * characters 

        if( NumPointers != 0 ) 
        {
            for( int i = 0; i < NumPointers; i++ ) 
                pTypeName += _T("*"); 
        }

    }

    if (o_TypeInfo)
    {
        *o_TypeInfo = Info;
    }

    return !pTypeName.empty();
}

TCHAR* CTypeInfoText::DataKindStr(SYMBOL_INFO& rSymbol)
{
    DWORD Dk = 0;

    if (!SymGetTypeInfo(GetCurrentProcess(), rSymbol.ModBase, rSymbol.Index, TI_GET_DATAKIND, &Dk))
    {
        ORBIT_ERROR;
    }

    return DataKindStr((DataKind)Dk);

}

//-----------------------------------------------------------------------------
void CTypeInfoText::SymbolLocationStr(SYMBOL_INFO& rSymbol, TCHAR* pBuffer, int BufferSize)
{
    // Note: The caller must pass a big-enough buffer 
    // 
    // Note: The function must guarantee that a valid string 
    //       is placed into pBuffer in all possible execution paths 
    //       (so that the caller does not have to check the returned 
    //       string for correctness) (if pBuffer is correct, of course) 
    // 

    if (pBuffer == 0)
    {
        ORBIT_ERROR;
        return;
    }

    if (rSymbol.Flags & SYMFLAG_REGISTER)
    {
        LPCTSTR pRegStr = RegisterStr((enum CH_HREG_e) rSymbol.Register);

        if (pRegStr != 0)
            StringCopy(pBuffer, BufferSize, pRegStr);
        else
            MemPrintf(pBuffer, BufferSize, _T("Reg%u"), rSymbol.Register);

        return;
    }
    else if (rSymbol.Flags & SYMFLAG_REGREL)
    {
        const int szRegSize = 32;
        TCHAR szReg[szRegSize];

        LPCTSTR pRegStr = RegisterStr((enum CH_HREG_e) rSymbol.Register);

        if (pRegStr != 0)
            StringCopy(szReg, szRegSize, pRegStr);
        else
            MemPrintf(szReg, szRegSize, _T("Reg%u"), rSymbol.Register);

        MemPrintf(pBuffer, BufferSize, _T("%s%+d"), szReg, (long)rSymbol.Address);

        return;
    }
    else if (rSymbol.Flags & SYMFLAG_FRAMEREL)
    {
        StringCopy(pBuffer, BufferSize, _T("n/a")); // not supported 
        return;
    }
    else
    {
        MemPrintf(pBuffer, BufferSize, _T("%16I64x"), rSymbol.Address);
        return;
    }
}

LPCTSTR CTypeInfoText::TagStr( enum SymTagEnum Tag ) 
{
    switch( Tag ) 
    {
    case SymTagNull:            return _T("Null"); 
    case SymTagExe:             return _T("Exe"); 
    case SymTagCompiland:       return _T("Compiland"); 
    case SymTagCompilandDetails:return _T("CompilandDetails"); 
    case SymTagCompilandEnv:    return _T("CompilandEnv"); 
    case SymTagFunction:        return _T("Function"); 
    case SymTagBlock:           return _T("Block"); 
    case SymTagData:            return _T("Data"); 
    case SymTagAnnotation:      return _T("Annotation"); 
    case SymTagLabel:           return _T("Label"); 
    case SymTagPublicSymbol:    return _T("PublicSymbol"); 
    case SymTagUDT:             return _T("UDT"); 
    case SymTagEnum:            return _T("Enum"); 
    case SymTagFunctionType:    return _T("FunctionType"); 
    case SymTagPointerType:     return _T("PointerType"); 
    case SymTagArrayType:       return _T("ArrayType"); 
    case SymTagBaseType:        return _T("BaseType"); 
    case SymTagTypedef:         return _T("Typedef"); 
    case SymTagBaseClass:       return _T("BaseClass"); 
    case SymTagFriend:          return _T("Friend"); 
    case SymTagFunctionArgType: return _T("FunctionArgType"); 
    case SymTagFuncDebugStart:  return _T("FuncDebugStart"); 
    case SymTagFuncDebugEnd:    return _T("FuncDebugEnd"); 
    case SymTagUsingNamespace:  return _T("UsingNamespace"); 
    case SymTagVTableShape:     return _T("VTableShape"); 
    case SymTagVTable:          return _T("VTable"); 
    case SymTagCustom:          return _T("Custom"); 
    case SymTagThunk:           return _T("Thunk"); 
    case SymTagCustomType:      return _T("CustomType"); 
    case SymTagManagedType:     return _T("ManagedType"); 
    case SymTagDimension:       return _T("Dimension"); 
    case SymTagMax:             return _T("Unknown-SymTagMax"); 
    default:                    return _T("UnknownTag"); 
    }
}

LPCTSTR CTypeInfoText::BaseTypeStr( enum BasicType Type, ULONG64 Length ) 
{
    switch( Type ) 
    {
    case btInt: switch( Length )
    {
        case 0: return _T("int");
        case 1: return _T("char");
        case 2: return _T("short"); 
        default:return _T("int"); 
    }
    case btUInt: switch (Length)
    {
        case 0: return _T("unsigned int");
        case 1: return _T("unsigned char");
        case 2: return _T("unsigned short");
        default:return _T("unsigned int");
    }
    case btFloat: switch (Length)
    {
        case 0: return _T("float");
        case 4: return _T("float"); 
        default:return _T("double");
    }
    case btNoType:      return _T("NoType");
    case btVoid:        return _T("void");
    case btChar:        return _T("char");
    case btWChar:       return _T("wchar_t");
    case btBCD:         return _T("BCD"); 
    case btBool:        return _T("bool"); 
    case btLong:        return _T("long"); 
    case btULong:       return _T("unsigned long"); 
    case btCurrency:    return _T("Currency"); 
    case btDate:        return _T("Date"); 
    case btVariant:     return _T("Variant"); 
    case btComplex:     return _T("Complex"); 
    case btBit:         return _T("Bit"); 
    case btBSTR:        return _T("BSTR"); 
    case btHresult:     return _T("HRESULT"); 
    default:            
        return _T("UnknownBaseType"); 
    }
}

LPCTSTR CTypeInfoText::CallConvStr( enum CV_call_e CallConv ) 
{
    switch( CallConv ) 
    {
    case CV_CALL_NEAR_C:        return _T("NEAR_C"); 
    case CV_CALL_FAR_C:         return _T("FAR_C"); 
    case CV_CALL_NEAR_PASCAL:   return _T("NEAR_PASCAL"); 
    case CV_CALL_FAR_PASCAL:    return _T("FAR_PASCAL"); 
    case CV_CALL_NEAR_FAST:     return _T("NEAR_FAST"); 
    case CV_CALL_FAR_FAST:      return _T("FAR_FAST"); 
    case CV_CALL_SKIPPED:       return _T("SKIPPED"); 
    case CV_CALL_NEAR_STD:      return _T("NEAR_STD"); 
    case CV_CALL_FAR_STD:       return _T("FAR_STD"); 
    case CV_CALL_NEAR_SYS:      return _T("NEAR_SYS"); 
    case CV_CALL_FAR_SYS:       return _T("FAR_SYS"); 
    case CV_CALL_THISCALL:      return _T("THISCALL"); 
    case CV_CALL_MIPSCALL:      return _T("MIPSCALL"); 
    case CV_CALL_GENERIC:       return _T("GENERIC"); 
    case CV_CALL_ALPHACALL:     return _T("ALPHACALL"); 
    case CV_CALL_PPCCALL:       return _T("PPCCALL"); 
    case CV_CALL_SHCALL:        return _T("SHCALL"); 
    case CV_CALL_ARMCALL:       return _T("ARMCALL"); 
    case CV_CALL_AM33CALL:      return _T("AM33CALL"); 
    case CV_CALL_TRICALL:       return _T("TRICALL"); 
    case CV_CALL_SH5CALL:       return _T("SH5CALL"); 
    case CV_CALL_M32RCALL:      return _T("M32RCALL"); 
    case CV_CALL_RESERVED:      return _T("RESERVED"); 
    default:                    return _T("UNKNOWN"); 
    }
}

TCHAR* CTypeInfoText::DataKindStr( enum DataKind dataKind ) 
{
    switch( dataKind ) 
    {
    case DataIsLocal:           return _T("LOCAL_VAR");
    case DataIsStaticLocal:     return _T("STATIC_LOCAL_VAR");
    case DataIsParam:           return _T("PARAMETER");
    case DataIsObjectPtr:       return _T("OBJECT_PTR");
    case DataIsFileStatic:      return _T("STATIC_VAR");
    case DataIsGlobal:          return _T("GLOBAL_VAR");
    case DataIsMember:          return _T("MEMBER"); 
    case DataIsStaticMember:    return _T("STATIC_MEMBER"); 
    case DataIsConstant:        return _T("CONSTANT");
    default:                    return _T("UNKNOWN"); 
    }
}

LPCTSTR CTypeInfoText::RegisterStr( enum CH_HREG_e RegCode ) 
{
    switch( RegCode ) 
    {
        case CV_REG_EAX: return _T("EAX"); 
        case CV_REG_ECX: return _T("ECX"); 
        case CV_REG_EDX: return _T("EDX"); 
        case CV_REG_EBX: return _T("EBX"); 
        case CV_REG_ESP: return _T("ESP"); 
        case CV_REG_EBP: return _T("EBP"); 
        case CV_REG_ESI: return _T("ESI"); 
        case CV_REG_EDI: return _T("EDI");
        case CV_AMD64_RAX: return _T("RAX");
        case CV_AMD64_RBX: return _T("RBX");
        case CV_AMD64_RCX: return _T("RCX");
        case CV_AMD64_RDX: return _T("RDX");
        case CV_AMD64_RSI: return _T("RSI");
        case CV_AMD64_RDI: return _T("RDI");
        case CV_AMD64_RBP: return _T("RBP");
        case CV_AMD64_RSP: return _T("RSP");
        case CV_AMD64_R8 : return _T("R8");
        case CV_AMD64_R9 : return _T("R9");
        case CV_AMD64_R10: return _T("R10");
        case CV_AMD64_R11: return _T("R11");
        case CV_AMD64_R12: return _T("R12");
        case CV_AMD64_R13: return _T("R13");
        case CV_AMD64_R14: return _T("R14");
        case CV_AMD64_R15: return _T("R15");
        case CV_REG_XMM00: return _T("XMM00");
        case CV_REG_XMM01: return _T("XMM01");
        case CV_REG_XMM02: return _T("XMM02");
        case CV_REG_XMM03: return _T("XMM03");
        case CV_REG_XMM10: return _T("XMM10");
        case CV_REG_XMM11: return _T("XMM11");
        case CV_REG_XMM12: return _T("XMM12");
        case CV_REG_XMM13: return _T("XMM13");
        case CV_REG_XMM20: return _T("XMM20");
        case CV_REG_XMM21: return _T("XMM21");
        case CV_REG_XMM22: return _T("XMM22");
        case CV_REG_XMM23: return _T("XMM23");
        case CV_REG_XMM30: return _T("XMM30");
        case CV_REG_XMM31: return _T("XMM31");
        case CV_REG_XMM32: return _T("XMM32");
        case CV_REG_XMM33: return _T("XMM33");
        case CV_REG_XMM40: return _T("XMM40");
        case CV_REG_XMM41: return _T("XMM41");
        case CV_REG_XMM42: return _T("XMM42");
        case CV_REG_XMM43: return _T("XMM43");
        case CV_REG_XMM50: return _T("XMM50");
        case CV_REG_XMM51: return _T("XMM51");
        case CV_REG_XMM52: return _T("XMM52");
        case CV_REG_XMM53: return _T("XMM53");
        case CV_REG_XMM60: return _T("XMM60");
        case CV_REG_XMM61: return _T("XMM61");
        case CV_REG_XMM62: return _T("XMM62");
        case CV_REG_XMM63: return _T("XMM63");
        case CV_REG_XMM70: return _T("XMM70");
        case CV_REG_XMM71: return _T("XMM71");
        case CV_REG_XMM72: return _T("XMM72");
        case CV_REG_XMM73: return _T("XMM73");
        default:         return 0;
    }
}

LPCTSTR CTypeInfoText::UdtKindStr( enum UdtKind Kind ) 
{
    switch( Kind ) 
    {
        case UdtStruct: return _T("STRUCT");
        case UdtClass:  return _T("CLASS");
        case UdtUnion:  return _T("UNION");
        default:        return _T("UNKNOWN_UDT");
    }
}

LPCTSTR CTypeInfoText::LocationTypeStr( enum LocationType Loc ) 
{
    switch( Loc ) 
    {
    case LocIsNull:         return _T("Null");
    case LocIsStatic:       return _T("Static");
    case LocIsTLS:          return _T("TLS");
    case LocIsRegRel:       return _T("RegRel");
    case LocIsThisRel:      return _T("ThisRel");
    case LocIsEnregistered: return _T("Enregistered"); 
    case LocIsBitField:     return _T("BitField"); 
    case LocIsSlot:         return _T("Slot"); 
    case LocIsIlRel:        return _T("IlRel"); 
    case LocInMetaData:     return _T("MetaData"); 
    case LocIsConstant:     return _T("Constant"); 
    default:                return _T("UnknownLoc"); 
    }
}

#endif