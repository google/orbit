//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Variable.h"

#include "Capture.h"
#include "Core.h"
#include "CoreApp.h"
#include "Log.h"
#include "Pdb.h"
#include "ScopeTimer.h"
#include "Serialization.h"
#include "TcpServer.h"

#ifdef _WIN32
#include "DiaParser.h"
#endif

Variable::Variable() { m_SyncTimer = new Timer(); }

void Variable::SetType(const std::string& a_Type) {
  m_Type = a_Type;
  m_BasicType = TypeFromString(a_Type);
}

void Variable::SendValue() {
  if (Capture::GInjected) {
    Message msg(Msg_SetData);
    msg.m_Header.m_DataTransferHeader.m_Address =
        (ULONG64)GPdbDbg->GetHModule() + (ULONG64)m_Address;
    msg.m_Header.m_DataTransferHeader.m_Type = DataTransferHeader::Data;
    GTcpServer->Send(msg, &m_Data, m_Size);
  }
}

void Variable::SyncValue() {
  if (Capture::GInjected) {
    Message msg(Msg_GetData);
    ULONG64 address = (ULONG64)m_Pdb->GetHModule() + (ULONG64)m_Address;
    msg.m_Header.m_DataTransferHeader.m_Address = address;
    msg.m_Header.m_DataTransferHeader.m_Type = DataTransferHeader::Data;
    msg.m_Size = m_Size;
    m_SyncTimer->Start();
    GTcpServer->Send(msg);
  }
}

void Variable::ReceiveValue(const Message& a_Msg) {
  m_SyncTimer->Stop();
  ORBIT_LOGV(m_SyncTimer->ElapsedMillis());
  if (a_Msg.m_Size != m_Size) {
    ORBIT_LOG("Variable::ReceiveValue size mismatch");
  } else {
    if (IsBasicType()) {
      memcpy(&m_Data, a_Msg.GetData(), m_Size);
      GCoreApp->UpdateVariable(this);
    } else {
      m_RawData.resize(m_Size);
      memcpy(m_RawData.data(), a_Msg.GetData(), m_Size);
      UpdateFromRaw(m_RawData, m_Address);
    }
  }
}

void Variable::UpdateFromRaw(const std::vector<char>& a_RawData,
                             DWORD64 a_BaseAddress) {
  for (std::shared_ptr<Variable>& var : m_Children) {
    if (var->IsBasicType()) {
      DWORD64 offset = var->m_Address - a_BaseAddress;
      DWORD64 size = var->m_Size;
      if (offset + size < a_RawData.size()) {
        memcpy(&var->m_Data, a_RawData.data() + offset, size);
      }

      GCoreApp->UpdateVariable(this);
    } else {
      var->UpdateFromRaw(a_RawData, a_BaseAddress);
    }
  }
}

std::string Indent(int a_Indent) {
  std::string str;
  str.reserve(a_Indent);
  for (int i = 0; i < a_Indent; ++i) {
    str += " ";
  }
  return str;
}

int MaxOffsetWidth(DWORD64 a_Size) {
  DWORD64 size = a_Size;
  int count = 1;
  while (size) {
    size = size / 10;
    ++count;
  }
  return count;
}

void Variable::Print() {
  ORBIT_VIZ("\n\nClass hierarchy:\n");
  PrintHierarchy();

  DWORD64 address = 0;
  std::string typeName = GetTypeName();
  ORBIT_VIZ(absl::StrFormat("\n%s size(%i)\n", typeName.c_str(), m_Size));
  Print(1, address, m_Size);
}

void Variable::Print(int a_Indent, DWORD64& a_ByteCounter,
                     DWORD64 a_TotalSize) {
  std::string indent = Indent(a_Indent);
  int width = MaxOffsetWidth(a_TotalSize);

  if (a_ByteCounter != m_Address) {
    int padding = int(m_Address - a_ByteCounter);
    ORBIT_VIZ(absl::StrFormat("[%-*" PRId64 "]%s<alignment member> (size=%i)\n",
                              width, a_ByteCounter, indent.c_str(), padding));
    a_ByteCounter = m_Address;
  }

  ORBIT_VIZ(absl::StrFormat("[%-*" PRId64 "]%s%s (%s)\n", width, m_Address,
                            indent.c_str(), m_Name.c_str(),
                            GetTypeName().c_str()));

  if (!HasChildren()) {
    a_ByteCounter += m_Size;
  }

  for (std::shared_ptr<Variable> child : m_Children) {
    child->Print(a_Indent + 1, a_ByteCounter, a_TotalSize);
  }
}

void Variable::PrintHierarchy(int a_Indent) {
  if (Type* type = GetType()) {
    type->LoadDiaInfo();
    ORBIT_VIZ(absl::StrFormat("%s%s\n", Indent(a_Indent).c_str(),
                              GetTypeName().c_str()));

    for (std::shared_ptr<Variable> var : m_Children) {
      if (var->m_IsParent) {
        var->PrintHierarchy(a_Indent + 1);
      }
    }
  }
}

void Variable::PrintDetails() {
#ifdef _WIN32
  if (Type* type = GetType()) {
    DiaParser parser;
    type->LoadDiaInfo();
    std::shared_ptr<OrbitDiaSymbol> diaSymbol = type->GetDiaSymbol();
    parser.PrintTypeInDetail(diaSymbol ? diaSymbol->m_Symbol : nullptr, 0);
    ORBIT_VIZ("\n\nDetails:\n");
    ORBIT_VIZ(ws2s(parser.m_Log));
  }
#endif
}

void Variable::Populate() {
  if (!m_Populated) {
    if (Type* type = GetType()) {
      type->LoadDiaInfo();
      const std::map<ULONG, Variable>& TypeMap = type->GetFullVariableMap();
      for (auto& pair : TypeMap) {
        std::shared_ptr<Variable> var = std::make_shared<Variable>(pair.second);
        var->UpdateTypeFromString();
        var->m_Address = this->m_Address + pair.first;
        m_Children.push_back(var);
      }
      m_Populated = true;
    }
  }
}

std::shared_ptr<Variable> Variable::FindVariable(
    std::shared_ptr<Variable> a_Variable, const std::string& a_Name) {
  if (a_Variable->m_Name == a_Name) {
    return a_Variable;
  }

  for (auto& var : a_Variable->m_Children) {
    if (std::shared_ptr<Variable> foundVariable = FindVariable(var, a_Name)) {
      return foundVariable;
    }
  }

  return nullptr;
}

std::shared_ptr<Variable> Variable::FindImmediateChild(
    const std::string& a_Name) {
  for (std::shared_ptr<Variable> var : m_Children) {
    if (var->m_Name == a_Name) {
      return var;
    }
  }

  return nullptr;
}

const Type* Variable::GetType() const {
#ifdef _WIN32
  return m_Pdb ? m_Pdb->GetTypePtrFromId(m_TypeIndex) : nullptr;
#else
  return nullptr;
#endif
}

Type* Variable::GetType() {
#ifdef _WIN32
  return m_Pdb ? m_Pdb->GetTypePtrFromId(m_TypeIndex) : nullptr;
#else
  return nullptr;
#endif
}

std::string Variable::GetTypeName() const {
  const Type* type = GetType();
  std::string typeName = type ? type->GetName() : "";
  return (typeName != "") ? typeName : m_Type;
}

Variable::BasicType Variable::GetBasicType() {
  // TODO: split assignation and get...
  const Type* type = GetType();
  Variable::BasicType basicType = type && type->m_Name != ""
                                      ? Variable::TypeFromString(type->m_Name)
                                      : Variable::TypeFromString(m_Type);
  m_BasicType = basicType;
  return m_BasicType;
}

Variable::BasicType Variable::TypeFromString(const std::string& a_String) {
  static std::map<std::string, BasicType> TypeMap;
  if (TypeMap.size() == 0) {
    TypeMap["int"] = Variable::Int;
    TypeMap["unsigned int"] = Variable::UInt;
    TypeMap["__int8"] = Variable::Int8;
    TypeMap["unsigned __int8"] = Variable::UInt8;
    TypeMap["__int16"] = Variable::Int16;
    TypeMap["unsigned __int16"] = Variable::UInt16;
    TypeMap["__int32"] = Variable::Int32;
    TypeMap["unsigned __int32"] = Variable::UInt32;
    TypeMap["__int64"] = Variable::Int64;
    TypeMap["unsigned __int64"] = Variable::UInt64;
    TypeMap["bool"] = Variable::Bool;
    TypeMap["char"] = Variable::Char;
    TypeMap["signed char"] = Variable::SChar;
    TypeMap["unsigned char"] = Variable::UChar;
    TypeMap["short"] = Variable::Short;
    TypeMap["unsigned short"] = Variable::UShort;
    TypeMap["long"] = Variable::Long;
    TypeMap["unsigned long"] = Variable::ULong;
    TypeMap["long long"] = Variable::LongLong;
    TypeMap["unsigned long long"] = Variable::ULongLong;
    TypeMap["enum"] = Variable::Enum;
    TypeMap["float"] = Variable::Float;
    TypeMap["double"] = Variable::Double;
    TypeMap["long double"] = Variable::LDouble;
    TypeMap["wchar_t"] = Variable::WChar;
  }

  return TypeMap[a_String];
}

void Variable::SetDouble(double a_Value) {
  switch (m_BasicType) {
    case Variable::Double:
      m_Double = a_Value;
      break;
    case Variable::Float:
      m_Float = (float)a_Value;
      break;
    default:
      break;
  }
}

ORBIT_SERIALIZE_WSTRING(Variable, 0) {
  ORBIT_NVP_VAL(0, m_Name);
  ORBIT_NVP_VAL(0, m_Type);
  ORBIT_NVP_VAL(0, m_Function);
  ORBIT_NVP_VAL(0, m_File);
  ORBIT_NVP_VAL(0, m_Line);
  ORBIT_NVP_VAL(0, m_Address);
  ORBIT_NVP_VAL(0, m_Size);
  ORBIT_NVP_VAL(0, m_TypeIndex);
  ORBIT_NVP_VAL(0, m_Children);
}
