//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <string>

struct IDiaDataSource;
struct IDiaSession;
struct IDiaSymbol;

class DiaManager
{
public:
    DiaManager();
    ~DiaManager();

    void Init();
    void DeInit();
    bool InitDataSource();
    bool LoadDataFromPdb( const wchar_t* a_FileName, IDiaSession** a_Session, IDiaSymbol** a_GlobalSymbol );

    static void InitMsDiaDll();

private:
    IDiaDataSource* m_DiaDataSource;
};