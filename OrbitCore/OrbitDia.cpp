//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "OrbitDia.h"

#include <dia2.h>

#include "Core.h"
#include "Log.h"
#include "dia2dump.h"

#define StringRef char*

//-----------------------------------------------------------------------------
template <typename ArgType>
void DumpDIAValue(std::ostream& OS, int Indent, StringRef Name,
                  IDiaSymbol* Symbol,
                  HRESULT (__stdcall IDiaSymbol::*Method)(ArgType*)) {
  ArgType Value;
  if (S_OK == (Symbol->*Method)(&Value)) {
    OS << "\n";
    // OS.indent( Indent );
    OS << Name << ": " << Value;
  }
}

//-----------------------------------------------------------------------------
void DumpDIAValue(std::ostream& OS, int Indent, StringRef Name,
                  IDiaSymbol* Symbol,
                  HRESULT (__stdcall IDiaSymbol::*Method)(GUID*)) {
  GUID Value;
  if (S_OK == (Symbol->*Method)(&Value)) {
    OS << "\n";
    // OS.indent( Indent );
    OS << Name << ": " << GuidToString(Value);
  }
}

//-----------------------------------------------------------------------------
void DumpDIAValue(std::ostream& OS, int Indent, StringRef Name,
                  IDiaSymbol* Symbol,
                  HRESULT (__stdcall IDiaSymbol::*Method)(BSTR*)) {
  BSTR Value = nullptr;
  if (S_OK != (Symbol->*Method)(&Value)) return;

  std::wstring wresult = Value;
  std::string result = ws2s(wresult);
  {
    OS << "\n";
    // OS.indent( Indent );
    OS << Name << ": " << result;
  }

  ::SysFreeString(Value);
}

//-----------------------------------------------------------------------------
void DumpDIAValue(std::ostream& OS, int Indent, StringRef Name,
                  IDiaSymbol* Symbol,
                  HRESULT (__stdcall IDiaSymbol::*Method)(VARIANT*)) {
  PRINT_FUNC;
  /*VARIANT Value;
  Value.vt = VT_EMPTY;
  if( S_OK != ( Symbol->*Method )( &Value ) )
      return;
  OS << "\n";
  OS.indent( Indent );
  Variant V = VariantFromVARIANT( Value );
  OS << V;*/
}

#define RAW_METHOD_DUMP(Stream, Method) \
  DumpDIAValue(Stream, Indent, #Method, Symbol, &IDiaSymbol::Method);

//-----------------------------------------------------------------------------
void OrbitDia::DiaDump(IDiaSymbol* Symbol, std::ostream& OS, int Indent) {
  if (Symbol == nullptr) return;

  OS << Format("\n\nDia details:\nthis: 0x%p", Symbol);
  RAW_METHOD_DUMP(OS, get_access)
  RAW_METHOD_DUMP(OS, get_addressOffset)
  RAW_METHOD_DUMP(OS, get_addressSection)
  RAW_METHOD_DUMP(OS, get_addressTaken)
  RAW_METHOD_DUMP(OS, get_age)
  RAW_METHOD_DUMP(OS, get_arrayIndexType)
  RAW_METHOD_DUMP(OS, get_arrayIndexTypeId)
  RAW_METHOD_DUMP(OS, get_backEndBuild)
  RAW_METHOD_DUMP(OS, get_backEndMajor)
  RAW_METHOD_DUMP(OS, get_backEndMinor)
  RAW_METHOD_DUMP(OS, get_backEndQFE)
  RAW_METHOD_DUMP(OS, get_baseDataOffset)
  RAW_METHOD_DUMP(OS, get_baseDataSlot)
  RAW_METHOD_DUMP(OS, get_baseSymbol)
  RAW_METHOD_DUMP(OS, get_baseSymbolId)
  RAW_METHOD_DUMP(OS, get_baseType)
  RAW_METHOD_DUMP(OS, get_bindID)
  RAW_METHOD_DUMP(OS, get_bindSlot)
  RAW_METHOD_DUMP(OS, get_bindSpace)
  RAW_METHOD_DUMP(OS, get_bitPosition)
  RAW_METHOD_DUMP(OS, get_builtInKind)
  RAW_METHOD_DUMP(OS, get_callingConvention)
  RAW_METHOD_DUMP(OS, get_characteristics)
  RAW_METHOD_DUMP(OS, get_classParent)
  RAW_METHOD_DUMP(OS, get_classParentId)
  RAW_METHOD_DUMP(OS, get_code)
  RAW_METHOD_DUMP(OS, get_coffGroup)
  RAW_METHOD_DUMP(OS, get_compilerGenerated)
  RAW_METHOD_DUMP(OS, get_compilerName)
  RAW_METHOD_DUMP(OS, get_constantExport)
  RAW_METHOD_DUMP(OS, get_constructor)
  RAW_METHOD_DUMP(OS, get_constType)
  RAW_METHOD_DUMP(OS, get_container)
  RAW_METHOD_DUMP(OS, get_count)
  RAW_METHOD_DUMP(OS, get_countLiveRanges)
  RAW_METHOD_DUMP(OS, get_customCallingConvention)
  RAW_METHOD_DUMP(OS, get_dataExport)
  RAW_METHOD_DUMP(OS, get_dataKind)
  RAW_METHOD_DUMP(OS, get_editAndContinueEnabled)
  RAW_METHOD_DUMP(OS, get_exceptionHandlerAddressOffset)
  RAW_METHOD_DUMP(OS, get_exceptionHandlerAddressSection)
  RAW_METHOD_DUMP(OS, get_exceptionHandlerRelativeVirtualAddress)
  RAW_METHOD_DUMP(OS, get_exceptionHandlerVirtualAddress)
  RAW_METHOD_DUMP(OS, get_exportHasExplicitlyAssignedOrdinal)
  RAW_METHOD_DUMP(OS, get_exportIsForwarder)
  RAW_METHOD_DUMP(OS, get_farReturn)
  RAW_METHOD_DUMP(OS, get_finalLiveStaticSize)
  RAW_METHOD_DUMP(OS, get_framePointerPresent)
  RAW_METHOD_DUMP(OS, get_frameSize)
  RAW_METHOD_DUMP(OS, get_frontEndBuild)
  RAW_METHOD_DUMP(OS, get_frontEndMajor)
  RAW_METHOD_DUMP(OS, get_frontEndMinor)
  RAW_METHOD_DUMP(OS, get_frontEndQFE)
  RAW_METHOD_DUMP(OS, get_function)
  RAW_METHOD_DUMP(OS, get_guid)
  RAW_METHOD_DUMP(OS, get_hasAlloca)
  RAW_METHOD_DUMP(OS, get_hasAssignmentOperator)
  RAW_METHOD_DUMP(OS, get_hasCastOperator)
  RAW_METHOD_DUMP(OS, get_hasControlFlowCheck)
  RAW_METHOD_DUMP(OS, get_hasDebugInfo)
  RAW_METHOD_DUMP(OS, get_hasEH)
  RAW_METHOD_DUMP(OS, get_hasEHa)
  RAW_METHOD_DUMP(OS, get_hasInlAsm)
  RAW_METHOD_DUMP(OS, get_hasLongJump)
  RAW_METHOD_DUMP(OS, get_hasManagedCode)
  RAW_METHOD_DUMP(OS, get_hasNestedTypes)
  RAW_METHOD_DUMP(OS, get_hasSecurityChecks)
  RAW_METHOD_DUMP(OS, get_hasSEH)
  RAW_METHOD_DUMP(OS, get_hasSetJump)
  RAW_METHOD_DUMP(OS, get_hasValidPGOCounts)
  RAW_METHOD_DUMP(OS, get_hfaDouble)
  RAW_METHOD_DUMP(OS, get_hfaFloat)
  RAW_METHOD_DUMP(OS, get_indirectVirtualBaseClass)
  RAW_METHOD_DUMP(OS, get_inlSpec)
  RAW_METHOD_DUMP(OS, get_interruptReturn)
  RAW_METHOD_DUMP(OS, get_intrinsic)
  RAW_METHOD_DUMP(OS, get_intro)
  RAW_METHOD_DUMP(OS, get_isAcceleratorGroupSharedLocal)
  RAW_METHOD_DUMP(OS, get_isAcceleratorPointerTagLiveRange)
  RAW_METHOD_DUMP(OS, get_isAcceleratorStubFunction)
  RAW_METHOD_DUMP(OS, get_isAggregated)
  RAW_METHOD_DUMP(OS, get_isConstructorVirtualBase)
  RAW_METHOD_DUMP(OS, get_isCTypes)
  RAW_METHOD_DUMP(OS, get_isCVTCIL)
  RAW_METHOD_DUMP(OS, get_isCxxReturnUdt)
  RAW_METHOD_DUMP(OS, get_isDataAligned)
  RAW_METHOD_DUMP(OS, get_isHLSLData)
  RAW_METHOD_DUMP(OS, get_isHotpatchable)
  RAW_METHOD_DUMP(OS, get_isInterfaceUdt)
  RAW_METHOD_DUMP(OS, get_isLocationControlFlowDependent)
  RAW_METHOD_DUMP(OS, get_isLTCG)
  RAW_METHOD_DUMP(OS, get_isMatrixRowMajor)
  RAW_METHOD_DUMP(OS, get_isMSILNetmodule)
  RAW_METHOD_DUMP(OS, get_isMultipleInheritance)
  RAW_METHOD_DUMP(OS, get_isNaked)
  RAW_METHOD_DUMP(OS, get_isOptimizedAway)
  RAW_METHOD_DUMP(OS, get_isOptimizedForSpeed)
  RAW_METHOD_DUMP(OS, get_isPGO)
  RAW_METHOD_DUMP(OS, get_isPointerBasedOnSymbolValue)
  RAW_METHOD_DUMP(OS, get_isPointerToDataMember)
  RAW_METHOD_DUMP(OS, get_isPointerToMemberFunction)
  RAW_METHOD_DUMP(OS, get_isRefUdt)
  RAW_METHOD_DUMP(OS, get_isReturnValue)
  RAW_METHOD_DUMP(OS, get_isSafeBuffers)
  RAW_METHOD_DUMP(OS, get_isSdl)
  RAW_METHOD_DUMP(OS, get_isSingleInheritance)
  RAW_METHOD_DUMP(OS, get_isSplitted)
  RAW_METHOD_DUMP(OS, get_isStatic)
  RAW_METHOD_DUMP(OS, get_isStripped)
  RAW_METHOD_DUMP(OS, get_isValueUdt)
  RAW_METHOD_DUMP(OS, get_isVirtualInheritance)
  RAW_METHOD_DUMP(OS, get_isWinRTPointer)
  RAW_METHOD_DUMP(OS, get_language)
  RAW_METHOD_DUMP(OS, get_length)
  RAW_METHOD_DUMP(OS, get_lexicalParent)
  RAW_METHOD_DUMP(OS, get_lexicalParentId)
  RAW_METHOD_DUMP(OS, get_libraryName)
  RAW_METHOD_DUMP(OS, get_liveRangeLength)
  RAW_METHOD_DUMP(OS, get_liveRangeStartAddressOffset)
  RAW_METHOD_DUMP(OS, get_liveRangeStartAddressSection)
  RAW_METHOD_DUMP(OS, get_liveRangeStartRelativeVirtualAddress)
  RAW_METHOD_DUMP(OS, get_localBasePointerRegisterId)
  RAW_METHOD_DUMP(OS, get_locationType)
  RAW_METHOD_DUMP(OS, get_lowerBound)
  RAW_METHOD_DUMP(OS, get_lowerBoundId)
  RAW_METHOD_DUMP(OS, get_machineType)
  RAW_METHOD_DUMP(OS, get_managed)
  RAW_METHOD_DUMP(OS, get_memorySpaceKind)
  RAW_METHOD_DUMP(OS, get_msil)
  RAW_METHOD_DUMP(OS, get_name)
  RAW_METHOD_DUMP(OS, get_nested)
  RAW_METHOD_DUMP(OS, get_noInline)
  RAW_METHOD_DUMP(OS, get_noNameExport)
  RAW_METHOD_DUMP(OS, get_noReturn)
  RAW_METHOD_DUMP(OS, get_noStackOrdering)
  RAW_METHOD_DUMP(OS, get_notReached)
  RAW_METHOD_DUMP(OS, get_numberOfAcceleratorPointerTags)
  RAW_METHOD_DUMP(OS, get_numberOfColumns)
  RAW_METHOD_DUMP(OS, get_numberOfModifiers)
  RAW_METHOD_DUMP(OS, get_numberOfRegisterIndices)
  RAW_METHOD_DUMP(OS, get_numberOfRows)
  RAW_METHOD_DUMP(OS, get_objectFileName)
  RAW_METHOD_DUMP(OS, get_objectPointerType)
  RAW_METHOD_DUMP(OS, get_oemId)
  RAW_METHOD_DUMP(OS, get_oemSymbolId)
  RAW_METHOD_DUMP(OS, get_offset)
  RAW_METHOD_DUMP(OS, get_offsetInUdt)
  RAW_METHOD_DUMP(OS, get_optimizedCodeDebugInfo)
  RAW_METHOD_DUMP(OS, get_ordinal)
  RAW_METHOD_DUMP(OS, get_overloadedOperator)
  RAW_METHOD_DUMP(OS, get_packed)
  RAW_METHOD_DUMP(OS, get_paramBasePointerRegisterId)
  RAW_METHOD_DUMP(OS, get_PGODynamicInstructionCount)
  RAW_METHOD_DUMP(OS, get_PGOEdgeCount)
  RAW_METHOD_DUMP(OS, get_PGOEntryCount)
  RAW_METHOD_DUMP(OS, get_phaseName)
  RAW_METHOD_DUMP(OS, get_platform)
  RAW_METHOD_DUMP(OS, get_privateExport)
  RAW_METHOD_DUMP(OS, get_pure)
  RAW_METHOD_DUMP(OS, get_rank)
  RAW_METHOD_DUMP(OS, get_reference)
  RAW_METHOD_DUMP(OS, get_registerId)
  RAW_METHOD_DUMP(OS, get_registerType)
  RAW_METHOD_DUMP(OS, get_relativeVirtualAddress)
  RAW_METHOD_DUMP(OS, get_restrictedType)
  RAW_METHOD_DUMP(OS, get_RValueReference)
  RAW_METHOD_DUMP(OS, get_samplerSlot)
  RAW_METHOD_DUMP(OS, get_scoped)
  RAW_METHOD_DUMP(OS, get_sealed)
  RAW_METHOD_DUMP(OS, get_signature)
  RAW_METHOD_DUMP(OS, get_sizeInUdt)
  RAW_METHOD_DUMP(OS, get_slot)
  RAW_METHOD_DUMP(OS, get_sourceFileName)
  RAW_METHOD_DUMP(OS, get_staticSize)
  RAW_METHOD_DUMP(OS, get_strictGSCheck)
  RAW_METHOD_DUMP(OS, get_stride)
  RAW_METHOD_DUMP(OS, get_subType)
  RAW_METHOD_DUMP(OS, get_subTypeId)
  RAW_METHOD_DUMP(OS, get_symbolsFileName)
  RAW_METHOD_DUMP(OS, get_symIndexId)
  RAW_METHOD_DUMP(OS, get_symTag)
  RAW_METHOD_DUMP(OS, get_targetOffset)
  RAW_METHOD_DUMP(OS, get_targetRelativeVirtualAddress)
  RAW_METHOD_DUMP(OS, get_targetSection)
  RAW_METHOD_DUMP(OS, get_targetVirtualAddress)
  RAW_METHOD_DUMP(OS, get_textureSlot)
  RAW_METHOD_DUMP(OS, get_thisAdjust)
  RAW_METHOD_DUMP(OS, get_thunkOrdinal)
  RAW_METHOD_DUMP(OS, get_timeStamp)
  RAW_METHOD_DUMP(OS, get_token)
  RAW_METHOD_DUMP(OS, get_type)
  RAW_METHOD_DUMP(OS, get_typeId)
  RAW_METHOD_DUMP(OS, get_uavSlot)
  RAW_METHOD_DUMP(OS, get_udtKind)
  RAW_METHOD_DUMP(OS, get_unalignedType)
  RAW_METHOD_DUMP(OS, get_undecoratedName)
  RAW_METHOD_DUMP(OS, get_unmodifiedType)
  RAW_METHOD_DUMP(OS, get_unmodifiedTypeId)
  RAW_METHOD_DUMP(OS, get_unused)
  RAW_METHOD_DUMP(OS, get_upperBound)
  RAW_METHOD_DUMP(OS, get_upperBoundId)
  RAW_METHOD_DUMP(OS, get_value)
  RAW_METHOD_DUMP(OS, get_virtual)
  RAW_METHOD_DUMP(OS, get_virtualAddress)
  RAW_METHOD_DUMP(OS, get_virtualBaseClass)
  RAW_METHOD_DUMP(OS, get_virtualBaseDispIndex)
  RAW_METHOD_DUMP(OS, get_virtualBaseOffset)
  RAW_METHOD_DUMP(OS, get_virtualBasePointerOffset)
  RAW_METHOD_DUMP(OS, get_virtualBaseTableType)
  RAW_METHOD_DUMP(OS, get_virtualTableShape)
  RAW_METHOD_DUMP(OS, get_virtualTableShapeId)
  RAW_METHOD_DUMP(OS, get_volatileType)
  RAW_METHOD_DUMP(OS, get_wasInlined)

  OS << std::endl << std::endl;
}

//-----------------------------------------------------------------------------
void OrbitDia::DiaDump(IDiaSymbol* a_Symbol) {
  std::stringstream l_StringStream;

  DiaDump(a_Symbol, l_StringStream, 0);

  OutputDebugStringA(l_StringStream.str().c_str());
  ORBIT_VIZ(l_StringStream.str().c_str());
}

//-----------------------------------------------------------------------------
void OrbitDia::DiaDump(unsigned long a_SymbolID) {
  OrbitDiaSymbol symbol;
  if (g_pDiaSession->symbolById(a_SymbolID, &symbol.m_Symbol) == S_OK) {
    OrbitDia::DiaDump(symbol.m_Symbol);
  }
}