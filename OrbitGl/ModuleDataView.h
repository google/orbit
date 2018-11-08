//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include "OrbitType.h"
#include "ProcessUtils.h"
#include "DataView.h"

class ModulesDataView : public DataView
{
public:
    ModulesDataView();

    virtual const std::vector<std::wstring>& GetColumnHeaders() override;
    virtual const std::vector<float>& GetColumnHeadersRatios() override;
    virtual std::vector<std::wstring> GetContextMenu(int a_Index) override;
    virtual std::wstring GetValue(int a_Row, int a_Column) override;;

    virtual void OnFilter(const std::wstring & a_Filter) override;
    virtual void OnSort(int a_Column, bool a_Toggle = true) override;
    virtual void OnContextMenu( const std::wstring & a_Action, int a_MenuIndex, std::vector<int> & a_ItemIndices ) override;
    virtual void OnTimer() override;
    virtual bool WantsDisplayColor() override { return true; }
    virtual bool GetDisplayColor(int /*a_Row*/, int /*a_Column*/, unsigned char& /*r*/, unsigned char& /*g*/, unsigned char& /*b*/);
    virtual std::wstring GetLabel() override { return L"Modules"; }

    void SetProcess(std::shared_ptr<Process> a_Process);

    enum MdvColumn
    {
        MDV_Index,
        MDV_ModuleName,
        MDV_Path,
        MDV_AddressRange,
        MDV_HasPdb,
        MDV_PdbSize,
        MDV_Loaded,
        MDV_NumColumns
    };

protected:
    const std::shared_ptr<Module> & GetModule( unsigned int a_Row ) const;

protected:
    std::shared_ptr<Process>                m_Process;
    std::vector< std::shared_ptr<Module> >  m_Modules;
    static std::vector<float>               s_HeaderRatios;
};

