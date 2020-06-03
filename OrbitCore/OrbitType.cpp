// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.



#include "OrbitType.h"

#include "BaseTypes.h"
#include "Capture.h"
#include "Core.h"
#include "Hashing.h"
#include "Log.h"
#include "Params.h"
#include "Pdb.h"
#include "PrintVar.h"
#include "SamplingProfiler.h"
#include "Serialization.h"
#include "TcpServer.h"

void Type::AddParent(Type* a_Parent) {
  UNUSED(a_Parent);
}

void Type::GenerateDataLayout() const {
  LOG("Dia Loading disabled, TODO: reimplement using LLVM (b/158093728).");
}

void Type::ListDataMembers(ULONG a_BaseOffset,
                           std::map<ULONG, Variable>& o_DataMembersFull) const {
  for (auto& pair : m_DataMembers) {
    ULONG offset = pair.first + a_BaseOffset;
    const Variable& member = pair.second;
    const Type& memberType = m_Pdb->GetTypeFromId(member.m_TypeIndex);
    UNUSED(memberType);

    if (o_DataMembersFull.find(offset) != o_DataMembersFull.end()) {
      ORBIT_LOG("Error in print type");
    }

    o_DataMembersFull[offset] = member;
  }
}

const std::map<ULONG, Variable>& Type::GetFullVariableMap() const {
  GenerateDataLayout();
  return m_DataMembersFull;
}

bool Type::IsA(const std::string& a_TypeName) {
  if (m_Name == a_TypeName) {
    return true;
  }

  for (auto& pair : m_ParentTypes) {
    Parent& parent = pair.second;
    Type& parentType = m_Pdb->GetTypeFromId(parent.m_TypeId);
    if (parentType.IsA(a_TypeName)) {
      return true;
    }
  }

  return false;
}

int Type::GetOffset(const std::string& a_Member) {
  for (auto& pair : m_DataMembersFull) {
    Variable& var = pair.second;
    if (var.m_Name == a_Member) {
      return pair.first;
    }
  }

  return -1;
}

Variable* Type::FindImmediateChild(const std::string& a_Name) {
  for (auto& pair : m_DataMembers) {
    Variable& var = pair.second;
    if (var.m_Name == a_Name) {
      return &var;
    }
  }

  return nullptr;
}

void Type::OutputPadding() const {
  uint64_t nextOffset = 0;
  uint64_t idealNextOffset = 0;

  for (auto& pair : m_DataMembersFull) {
    uint64_t offset = pair.first;
    const Variable& member = pair.second;
    const Type& memberType = m_Pdb->GetTypeFromId(member.m_TypeIndex);

    auto nextIt = m_DataMembersFull.upper_bound(pair.first);
    nextOffset = nextIt != m_DataMembersFull.end() ? nextIt->first : m_Length;

    idealNextOffset = offset + memberType.m_Length;
    if (memberType.m_Length > 0 && nextOffset > idealNextOffset) {
      Variable paddingMember;
      paddingMember.m_Name = "padding";
      paddingMember.m_TypeIndex = 0xFFFFFFFF;
      paddingMember.m_Size = ULONG(nextOffset - idealNextOffset);
      m_DataMembers[idealNextOffset] = paddingMember;
      m_DataMembersFull[idealNextOffset] = m_DataMembers[idealNextOffset];
    }
  }
}

void Type::GenerateHierarchy(std::map<ULONG, Parent>& a_Hierarchy,
                             int a_Offset) const {
  for (auto it : m_ParentTypes) {
    Parent& parent = it.second;
    ULONG parentOffset = parent.m_BaseOffset;

    const Type& parentType = m_Pdb->GetTypeFromId(parent.m_TypeId);

    parentType.GenerateHierarchy(a_Hierarchy, a_Offset + parentOffset);

    if (parentType.m_DataMembers.size() > 0) {
      ULONG firstVarOffset =
          a_Offset + parentOffset + parentType.m_DataMembers.begin()->first;

      if (a_Hierarchy.find(firstVarOffset) != a_Hierarchy.end()) {
        ORBIT_LOG("Error in GenerateHierarchy");
      }

      parent.m_BaseOffset += a_Offset;
      parent.m_Name = parentType.m_Name;
      a_Hierarchy[firstVarOffset] = parent;
    }
  }
}

unsigned long long Type::Hash() {
#ifdef _WIN32
  if (m_Hash == 0) {
    XXH64_state_t xxHashState;
    XXH64_reset(&xxHashState, 0x123456789ABCDEFF);

    XXH64_update(&xxHashState, &m_Pdb, sizeof(m_Pdb));
    XXH64_update(&xxHashState, m_Name.data(), m_Name.size());
    XXH64_update(&xxHashState, &m_Length, sizeof(m_Length));
    XXH64_update(&xxHashState, &m_NumVariables, sizeof(m_NumVariables));
    XXH64_update(&xxHashState, &m_NumFunctions, sizeof(m_NumFunctions));
    XXH64_update(&xxHashState, &m_Id, sizeof(m_Id));
    XXH64_update(&xxHashState, &m_NumBaseClasses, sizeof(m_NumBaseClasses));

    m_Hash = XXH64_digest(&xxHashState);
  }

  return m_Hash;
#else
  printf("Type::Hash() returning 0... fix me\n");
  return 0;
#endif
}

std::shared_ptr<Variable> Type::GetTemplateVariable() {
  if (m_TemplateVariable == nullptr) {
    m_TemplateVariable = GenerateVariable(0);
  }

  return m_TemplateVariable;
}

std::shared_ptr<Variable> Type::GenerateVariable(DWORD64 a_Address,
                                                 const std::string* a_Name) {
  std::shared_ptr<Variable> var = std::make_shared<Variable>();

#ifdef _WIN32
  var->m_Pdb = this->m_Pdb;
  var->m_Address = a_Address;
  var->m_TypeIndex = m_Id;
  var->m_Name = a_Name != nullptr ? *a_Name : this->m_Name;
  var->m_Size = (ULONG)this->m_Length;

  // Parents
  for (auto& pair : m_ParentTypes) {
    Parent& parentType = pair.second;
    ULONG baseOffset = parentType.m_BaseOffset;

    if (Type* type = m_Pdb->GetTypePtrFromId(parentType.m_TypeId)) {
      std::shared_ptr<Variable> parent =
          type->GenerateVariable(a_Address + baseOffset);
      parent->m_IsParent = true;
      parent->m_BaseOffset = baseOffset;
      var->AddChild(parent);
    }
  }

  // Members
  for (auto& pair : m_DataMembers) {
    ULONG memberOffset = pair.first;
    Variable& member = pair.second;
    Type* type = m_Pdb->GetTypePtrFromId(member.m_TypeIndex);

    if (!type) continue;

    if (type && type->HasMembers()) {
      std::string name = member.m_Name;
      var->AddChild(type->GenerateVariable(a_Address + memberOffset, &name));
    } else {
      std::shared_ptr<Variable> newMember = std::make_shared<Variable>(member);
      newMember->m_Address = a_Address + memberOffset;
      newMember->m_Name = member.m_Name;
      var->AddChild(newMember);
    }
  }
#else
  UNUSED(a_Address);
  UNUSED(a_Name);
#endif

  return var;
}
