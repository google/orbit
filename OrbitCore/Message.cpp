//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Message.h"
#include "PrintVar.h"

//-----------------------------------------------------------------------------
uint32_t Message::GSessionID;

//-----------------------------------------------------------------------------
void Message::Dump()
{
    PRINT_VAR(offsetof(Message, m_Type));
    PRINT_VAR(offsetof(Message, m_Header));
    PRINT_VAR(offsetof(Message, m_Size));
    PRINT_VAR(offsetof(Message, m_SessionID));
    PRINT_VAR(offsetof(Message, m_ThreadId));
    PRINT_VAR(offsetof(Message, m_Data));

    PRINT_VAR(sizeof(m_Type));
    PRINT_VAR(sizeof(m_Header));
    PRINT_VAR(sizeof(m_Size));
    PRINT_VAR(sizeof(m_SessionID));
    PRINT_VAR(sizeof(m_ThreadId));
    PRINT_VAR(sizeof(m_Data));
    
    PRINT_VAR(sizeof(MessageGeneric));
    PRINT_VAR(sizeof(DataTransferHeader));
    PRINT_VAR(sizeof(ArgTrackingHeader));
    PRINT_VAR(sizeof(UnrealObjectHeader));
}