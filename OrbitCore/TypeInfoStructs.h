/*
 * Copyright (c) 2020 The Orbit Authors. All rights reserved.
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */

// Author: Oleg Starodumov

#ifndef TypeInfoStructs_h
#define TypeInfoStructs_h

//#define _NO_CVCONST_H
#include "OrbitDbgHelp.h"
#include "Serialization.h"

// Maximal length of name buffers (in characters)
#define TIS_MAXNAMELEN 256
// Maximal number of dimensions of an array
#define TIS_MAXARRAYDIMS 64  // TODO warn when limit is hit

// SymTagBaseType symbol
struct BaseTypeInfo {
  // Basic type (DIA: baseType)
  enum BasicType BaseType;
  // Length (in bytes) (DIA: length)
  ULONG64 Length;
};

// SymTagTypedef symbol
struct TypedefInfo {
  // Name (DIA: name)
  WCHAR* NamePtr;
  // Index of the underlying type (DIA: typeId)
  ULONG TypeIndex;
};

// SymTagPointerType symbol
struct PointerTypeInfo {
  // Length (in bytes) (DIA: length)
  ULONG64 Length;
  // Index of the type the pointer points to (DIA: typeId)
  ULONG TypeIndex;
};

// SymTagUDT symbol (Class or structure)
struct UdtClassInfo {
  // Name (DIA: name)
  WCHAR* NamePtr;
  // Length (DIA: length)
  ULONG64 Length;
  // UDT kind (class, structure or union) (DIA: udtKind)
  enum UdtKind UDTKind;
  // Nested ("true" if the declaration is nested in another UDT) (DIA: nested)
  bool Nested;
  // Member variables
  std::vector<ULONG> Variables;
  // Member functions
  std::vector<ULONG> Functions;
  // Base classes
  std::vector<ULONG> BaseClasses;

  template <class Archive>
  void serialize(Archive& ar, std::uint32_t const version) {
    ar(  // CEREAL_NVP(Name)
        CEREAL_NVP(Length), CEREAL_NVP(UDTKind), CEREAL_NVP(Nested),
        CEREAL_NVP(Variables), CEREAL_NVP(Functions), CEREAL_NVP(BaseClasses));
  }
};

// SymTagUDT symbol (Union)
struct UdtUnionInfo {
  // Name (DIA: name)
  WCHAR* NamePtr;
  // Length (in bytes) (DIA: length)
  ULONG64 Length;
  // UDT kind (class, structure or union) (DIA: udtKind)
  enum UdtKind UDTKind;
  // Nested ("true" if the declaration is nested in another UDT) (DIA: nested)
  bool Nested;
  // Members
  std::vector<ULONG> Members;

  template <class Archive>
  void serialize(Archive& ar, std::uint32_t const version) {
    ar(  // CEREAL_NVP(Name)
        CEREAL_NVP(Length), CEREAL_NVP(UDTKind), CEREAL_NVP(Nested),
        CEREAL_NVP(Members));
  }
};

// SymTagBaseClass symbol
struct BaseClassInfo {
  // Index of the UDT symbol that represents the base class (DIA: type)
  ULONG TypeIndex;
  // Virtual ("true" if the base class is a virtual base class) (DIA:
  // virtualBaseClass)
  bool Virtual;
  // Offset of the base class within the class/structure (DIA: offset)
  // (defined only if Virtual is "false")
  LONG Offset;
  // Virtual base pointer offset (DIA: virtualBasePointerOffset)
  // (defined only if Virtual is "true")
  LONG VirtualBasePointerOffset;
};

// SymTagEnum symbol
struct EnumInfo {
  // Name (DIA: name)
  WCHAR* NamePtr;
  // Index of the symbol that represent the type of the enumerators (DIA:
  // typeId)
  ULONG TypeIndex;
  // Nested ("true" if the declaration is nested in a UDT) (DIA: nested)
  bool Nested;
  // Enumerators (their type indices)
  std::vector<ULONG> Enums;

  template <class Archive>
  void serialize(Archive& ar, std::uint32_t const version) {
    ar(  // CEREAL_NVP(Name)
        CEREAL_NVP(TypeIndex), CEREAL_NVP(Nested), CEREAL_NVP(Enums));
  }
};

// SymTagArrayType symbol
struct ArrayTypeInfo {
  // Index of the symbol that represents the type of the array element
  ULONG ElementTypeIndex;
  // Index of the symbol that represents the type of the array index (DIA:
  // arrayIndexTypeId)
  ULONG IndexTypeIndex;
  // Size of the array (in bytes) (DIA: length)
  ULONG64 Length;
  // Number of dimensions
  int NumDimensions;
  // Dimensions
  ULONG64 Dimensions[TIS_MAXARRAYDIMS];
};

// SymTagFunctionType
struct FunctionTypeInfo {
  // Index of the return value type symbol (DIA: objectPointerType)
  ULONG RetTypeIndex;
  // Function arguments
  std::vector<ULONG> Args;
  // Calling convention (DIA: callingConvention)
  enum CV_call_e CallConv;
  // "Is member function" flag (member function, if "true")
  bool MemberFunction;
  // Class symbol index (DIA: classParent)
  // (defined only if MemberFunction is "true")
  ULONG ClassIndex;
  // "this" adjustment (DIA: thisAdjust)
  // (defined only if MemberFunction is "true")
  LONG ThisAdjust;
  // "Is static function" flag (static, if "true")
  // (defined only if MemberFunction is "true")
  bool StaticFunction;

  template <class Archive>
  void serialize(Archive& ar, std::uint32_t const version) {
    ar(CEREAL_NVP(RetTypeIndex), CEREAL_NVP(Args), CEREAL_NVP(CallConv),
       CEREAL_NVP(MemberFunction), CEREAL_NVP(ClassIndex),
       CEREAL_NVP(ThisAdjust), CEREAL_NVP(StaticFunction));
  }
};

// SymTagFunctionArgType
struct FunctionArgTypeInfo {
  // Index of the symbol that represents the type of the argument (DIA: typeId)
  ULONG TypeIndex;
};

// SymTagData
struct DataInfo {
  // Name (DIA: name)
  WCHAR* NamePtr;
  // Index of the symbol that represents the type of the variable (DIA: type)
  ULONG TypeIndex;
  // Data kind (local, global, member, etc.) (DIA: dataKind)
  enum DataKind dataKind;
  // Address (defined if dataKind is: DataIsGlobal, DataIsStaticLocal,
  // DataIsFileStatic, DataIsStaticMember) (DIA: address)
  ULONG64 Address;
  // Offset (defined if dataKind is: DataIsLocal, DataIsParam,
  // DataIsObjectPtr, DataIsMember) (DIA: offset)
  ULONG Offset;  // <OS-TODO> Verify it for all listed data kinds
  // Note: Length is not available - use the type symbol to obtain it
};

struct TypeInfo {
  TypeInfo() {
    memset(this, 0, sizeof(TypeInfo));
    Info.sTypedefInfo.NamePtr = Name;
    Info.sDataInfo.NamePtr = Name;
    sUdtClassInfo.NamePtr = Name;
    sUdtUnionInfo.NamePtr = Name;
    sEnumInfo.NamePtr = Name;
  }

  // Name (DIA: name)
  WCHAR Name[TIS_MAXNAMELEN];
  // Symbol tag
  enum SymTagEnum Tag;
  // UDT kind (defined only if "Tag" is SymTagUDT: "true" if the symbol is
  // a class or a structure, "false" if the symbol is a union)
  bool UdtKind;
  // Union of all type information structures
  union TypeInfoStructures {
    BaseTypeInfo sBaseTypeInfo;        // If Tag == SymTagBaseType
    TypedefInfo sTypedefInfo;          // If Tag == SymTagTypedef
    PointerTypeInfo sPointerTypeInfo;  // If Tag == SymTagPointerType
    BaseClassInfo sBaseClassInfo;      // If Tag == SymTagBaseClass
    ArrayTypeInfo sArrayTypeInfo;      // If Tag == SymTagArrayType
    FunctionArgTypeInfo
        sFunctionArgTypeInfo;  // If Tag == SymTagFunctionArgType
    DataInfo sDataInfo;        // If Tag == SymTagData
  } Info;

  UdtClassInfo sUdtClassInfo;  // If Tag == SymTagUDT and UdtKind is "true"
  UdtUnionInfo sUdtUnionInfo;  // If Tag == SymTagUDT and UdtKind is "false"
  EnumInfo sEnumInfo;          // If Tag == SymTagEnum
  FunctionTypeInfo sFunctionTypeInfo;  // If Tag == SymTagFunctionType

  //-----------------------------------------------------------------------------
  template <class Archive>
  void serialize(Archive& ar, std::uint32_t const version) {
    ar(CEREAL_NVP(Tag), CEREAL_NVP(UdtKind), CEREAL_NVP(Info),
       CEREAL_NVP(sUdtClassInfo), CEREAL_NVP(sUdtUnionInfo),
       CEREAL_NVP(sEnumInfo), CEREAL_NVP(sFunctionTypeInfo));
  }
};

//-----------------------------------------------------------------------------
template <class Archive>
void serialize(Archive& archive,
               TypeInfo::TypeInfoStructures& a_TypeInfoStructures,
               std::uint32_t const version) {
  archive(
      cereal::binary_data(&a_TypeInfoStructures, sizeof(a_TypeInfoStructures)));
}

#endif  // TypeInfoStructs_h
