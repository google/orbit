//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>

#include "BaseTypes.h"
#include "SerializationMacros.h"
#include "Utils.h"
#include "absl/strings/str_format.h"

class Pdb;
class Type;
class Message;

class Variable {
 public:
  enum BasicType {
    Invalid,
    Int,
    UInt,
    Int8,
    UInt8,
    Int16,
    UInt16,
    Int32,
    UInt32,
    Int64,
    UInt64,
    Bool,
    Char,
    SChar,
    UChar,
    Short,
    UShort,
    Long,
    ULong,
    LongLong,
    ULongLong,
    Enum,
    Float,
    Double,
    LDouble,
    WChar,
    String,
    WString
  };

  Variable();
  std::string ToString() const {
    return absl::StrFormat("%s %s [%" PRIu64 "] Size: %lu", m_Type.c_str(),
                           m_Name.c_str(), m_Address, m_Size);
  }

  inline const std::string& FilterString();
  void SetType(const std::string& a_Type);
  void SendValue();
  void SyncValue();
  void ReceiveValue(const Message& a_Msg);
  void UpdateFromRaw(const std::vector<char>& a_RawData, DWORD64 a_BaseAddress);
  void Print();
  void Print(int a_Indent, DWORD64& a_ByteCounter, DWORD64 a_TotalSize);
  void PrintHierarchy(int a_Indent = 0);
  void PrintDetails();
  void Populate();
  const Type* GetType() const;
  Type* GetType();
  std::string GetTypeName() const;
  Variable::BasicType GetBasicType();
  void UpdateTypeFromString() { m_BasicType = TypeFromString(m_Type); }
  bool IsBasicType() const { return m_BasicType != Invalid; }
  void SetDouble(double a_Value);
  void AddChild(std::shared_ptr<Variable> a_Variable) {
    m_Children.push_back(a_Variable);
  }
  bool HasChildren() const { return m_Children.size() > 0; }

  static std::shared_ptr<Variable> FindVariable(
      std::shared_ptr<Variable> a_Variable, const std::string& a_Name);
  std::shared_ptr<Variable> FindImmediateChild(const std::string& a_Name);
  static Variable::BasicType TypeFromString(const std::string& a_String);

  //-------------------------------------------------------------------------
  template <typename T>
  void SetValue(T a_Value) {
    T& value = *((T*)&m_Data);
    if (value != a_Value) {
      value = a_Value;
      SendValue();
    }
  }

  union {
    void* m_Data;
    int m_Int;
    unsigned int m_UInt;
#ifdef _WIN32
    __int8 m_Int8;
    unsigned __int8 m_UInt8;
    __int16 m_Int16;
    unsigned __int16 m_UInt16;
    __int32 m_Int32;
    unsigned __int32 m_UInt32;
    __int64 m_Int64;
    unsigned __int64 m_UInt64;
#endif
    bool m_Bool;
    char m_Char;
    signed char m_SChar;
    unsigned char m_UChar;
    short m_Short;
    unsigned short m_UShort;
    long m_Long;
    unsigned long m_ULong;
    long long m_LongLong;
    unsigned long long m_ULongLong;
    float m_Float;
    double m_Double;
    long double m_LDouble;
    wchar_t m_WChar;
  };

  std::string m_Name;
  std::string m_PrettyTypeName;
  std::string m_Type;
  std::string m_Function;
  std::string m_File;
  std::string m_FilterString;

  uint64_t m_Address = 0;
  uint64_t m_BaseOffset = 0;
  uint32_t m_Size = 0;
  uint32_t m_TypeIndex = 0;
  uint32_t m_UnmodifiedTypeId = 0;
  int m_Line = 0;
  bool m_Selected;
  BasicType m_BasicType;
  class Timer* m_SyncTimer = nullptr;
  Pdb* m_Pdb = nullptr;
  bool m_Populated = false;
  bool m_IsParent = false;

  // Hierarchy
  std::vector<std::shared_ptr<Variable> > m_Parents;
  std::vector<std::shared_ptr<Variable> > m_Children;

  std::vector<char> m_RawData;
  std::string m_String;

  ORBIT_SERIALIZABLE;
};

//-----------------------------------------------------------------------------
inline const std::string& Variable::FilterString() {
  if (m_FilterString.size() == 0) {
    m_FilterString =
        ToLower(m_Name) + " " + ToLower(m_File) + " " + ToLower(m_Type);
  }

  return m_FilterString;
}
