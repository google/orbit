//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "cvconst.h"

#include "DiaManager.h"
#include "FunctionStats.h"
#include "OrbitDbgHelp.h"
#include "OrbitFunction.h"
#include "SerializationMacros.h"
#include "Variable.h"

//-----------------------------------------------------------------------------
struct Parent {
  Parent() : m_TypeId(0), m_BaseOffset(0) {}
  Parent(ULONG Id, LONG Offset) : m_TypeId(Id), m_BaseOffset(Offset) {}
  ULONG m_TypeId;
  LONG m_BaseOffset;
  std::wstring m_Name;
};

//-----------------------------------------------------------------------------
class Type {
 public:
  Type() {}

  enum MemberID {
    NAME,
    LENGTH,
    TYPE_ID,
    TYPE_ID_UNMODIFIED,
    NUM_VARIABLES,
    NUM_FUNCTIONS,
    NUM_BASE_CLASSES,
    BASE_OFFSET,
    MODULE,
    SELECTED,
    INDEX,
    NUM_EXPOSED_MEMBERS
  };

  void LoadDiaInfo();
  void GenerateDiaHierarchy();
  void GenerateDiaHierarchy(struct IDiaSymbol* a_DiaSymbol);
  void AddParent(IDiaSymbol* a_Parent);
  const std::map<ULONG, Variable>& GetFullVariableMap() const;
  Pdb* GetPdb() const { return m_Pdb; }
  const std::wstring& GetName() const { return m_Name; }
  const std::wstring& GetNameLower() {
    if (m_NameLower.size() == 0) {
      m_NameLower = ToLower(m_Name);
    }
    return m_NameLower;
  }

#ifdef _WIN32
  std::shared_ptr<OrbitDiaSymbol> GetDiaSymbol();
#endif

  bool IsA(const std::wstring& a_TypeName);
  int GetOffset(const std::wstring& a_Member);
  bool HasMembers() const { return m_DataMembers.size() > 0; }
  Variable* FindImmediateChild(const std::wstring& a_Name);

  std::shared_ptr<Variable> GetTemplateVariable();
  std::shared_ptr<Variable> GenerateVariable(
      DWORD64 a_Address, const std::wstring* a_Name = nullptr);

 protected:
  void GenerateDataLayout() const;
  void ListDataMembers(ULONG a_BaseOffset,
                       std::map<ULONG, Variable>& o_DataMembers) const;
  void OutputPadding() const;

 public:
  void GenerateHierarchy(std::map<ULONG, Parent>& a_ParentTypes,
                         int a_Offset = 0) const;
  unsigned long long Hash();

  ULONG m_Id = 0;
  ULONG m_UnmodifiedId = 0;
  ULONG m_PointedTypeId = 0;
  std::wstring m_Name;
  std::wstring m_NameLower;
  uint64_t m_Length = 0;
  int m_NumVariables = 0;
  int m_NumFunctions = 0;
  int m_NumBaseClasses = 0;
  bool m_Nested = false;
  LONG m_BaseOffset = 0;

  std::vector<Function> m_Functions;
  std::map<ULONG, Parent> m_ParentTypes;
  UdtKind m_Type;
  bool m_Selected = false;
#ifdef _WIN32
  TypeInfo m_TypeInfo;
#endif
  Pdb* m_Pdb = nullptr;
  unsigned long long m_Hash = 0;
  bool m_DiaInfoLoaded = false;
  bool m_HierarchyGenerated = false;

  // TODO: should not be mutable, but they are so we can lazy populate them
  mutable std::map<ULONG, Variable> m_DataMembers;      // offset, Member
  mutable std::map<ULONG, Variable> m_DataMembersFull;  // offset, Variable
  mutable std::map<ULONG, Parent> m_Hierarchy;
  std::shared_ptr<Variable> m_TemplateVariable;
};
