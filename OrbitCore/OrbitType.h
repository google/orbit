//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>
#include <vector>
#include <map>
#include <set>

#include "cvconst.h"

#include "OrbitDbgHelp.h"
#include "Variable.h"
#include "FunctionStats.h"
#include "SerializationMacros.h"
#include "OrbitFunction.h"

using namespace std;

//-----------------------------------------------------------------------------
struct Parent
{
    Parent() : m_TypeId(0), m_BaseOffset(0) {}
    Parent( ULONG Id, LONG Offset ) : m_TypeId(Id), m_BaseOffset(Offset) {}
    ULONG m_TypeId;
    LONG  m_BaseOffset;
    std::wstring m_Name;
};

//-----------------------------------------------------------------------------
class Type
{
public:
    Type() : m_Id(0)
           , m_UnmodifiedId(0)
           , m_PointedTypeId(0)
           , m_Selected(false)
           , m_BaseOffset(0)
           , m_Length(0)
           , m_NumVariables(0)
           , m_NumFunctions(0)
           , m_NumBaseClasses(0)
           , m_Pdb(nullptr)
           , m_Hash(0)
           , m_DiaInfoLoaded(false)
           , m_HierarchyGenerated(false)
    {
    }

    enum MemberID
    {
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
    void GenerateDiaHierarchy( struct IDiaSymbol* a_DiaSymbol );
    void AddParent( IDiaSymbol* a_Parent );
    const map<ULONG, Variable> & GetFullVariableMap() const;
    Pdb* GetPdb() const { return m_Pdb; }
    const std::wstring & GetName() const { return m_Name; }
    const std::wstring & GetNameLower(){ if( m_NameLower.size() == 0 ){ m_NameLower = ToLower( m_Name ); } return m_NameLower;  }
    IDiaSymbol* GetDiaSymbol();
    bool IsA( const std::wstring & a_TypeName );
    int GetOffset( const std::wstring & a_Member );
    bool HasMembers() const { return m_DataMembers.size() > 0; }
    Variable* FindImmediateChild( const std::wstring & a_Name );

    std::shared_ptr<Variable> GetTemplateVariable();
    std::shared_ptr<Variable> GenerateVariable( DWORD64 a_Address, const std::wstring* a_Name = nullptr );

protected:
    void GenerateDataLayout() const;
    void ListDataMembers( ULONG a_BaseOffset, map<ULONG, Variable> & o_DataMembers ) const;
    void OutputPadding() const;

public:
    void GenerateHierarchy( map<ULONG, Parent> & a_ParentTypes, int a_Offset = 0 ) const;
    unsigned long long Hash();

    ULONG              m_Id;
    ULONG              m_UnmodifiedId;
    ULONG              m_PointedTypeId;
    wstring            m_Name;
    wstring            m_NameLower;
    ULONG64            m_Length;
    int                m_NumVariables;
    int                m_NumFunctions;
    int                m_NumBaseClasses;
    bool               m_Nested;
    LONG               m_BaseOffset;

    vector<Function>   m_Functions;
    map<ULONG, Parent> m_ParentTypes;
    UdtKind            m_Type;
    bool               m_Selected;
    TypeInfo           m_TypeInfo;
    Pdb*               m_Pdb;
    unsigned long long m_Hash;
    bool               m_DiaInfoLoaded;
    bool               m_HierarchyGenerated;

    // TODO: should not be mutable, but they are so we can lazy populate them
    mutable map<ULONG, Variable> m_DataMembers;     //offset, Member
    mutable map<ULONG, Variable> m_DataMembersFull; //offset, Variable
    mutable map<ULONG, Parent>   m_Hierarchy;
    std::shared_ptr<Variable>    m_TemplateVariable;
};
