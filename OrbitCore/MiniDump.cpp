//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Core.h"
#include "MiniDump.h"
#include "OrbitProcess.h"
#include "CoreApp.h"
#include <memory>

#ifdef _WIN32

#include <google_breakpad/processor/minidump.h>

using namespace google_breakpad;

//-----------------------------------------------------------------------------
MiniDump::MiniDump( std::wstring a_FileName )
{
    m_MiniDump = new google_breakpad::Minidump( ws2s(a_FileName) );
    m_MiniDump->Read();
}

//-----------------------------------------------------------------------------
MiniDump::~MiniDump()
{
    delete m_MiniDump;
}

//-----------------------------------------------------------------------------
std::shared_ptr<Process> MiniDump::ToOrbitProcess()
{
    MinidumpModuleList* moduleList = m_MiniDump->GetModuleList();
    if( moduleList )
    {
        m_MiniDump->Print();
        std::shared_ptr<Process> process = std::make_shared<Process>();
        process->SetIsRemote(true);
        process->SetID( 0 );

        unsigned int numModules = moduleList->module_count();
        for( unsigned int i = 0; i < numModules; ++i )
        {
            const MinidumpModule* module = moduleList->GetModuleAtIndex(i);
            PRINT_VAR( module->base_address() );
            PRINT_VAR( module->code_file() );
            PRINT_VAR( module->code_identifier() );
            PRINT_VAR( module->debug_file() );
            PRINT_VAR( module->debug_identifier() );

            std::shared_ptr<Module> mod = std::make_shared<Module>();
            
            mod->m_FullName = s2ws( module->code_file() );
            mod->m_Name = Path::GetFileName( mod->m_FullName );
            
            if( EndsWith( mod->m_Name, TEXT( ".exe" ) ) )
            {
                process->m_Name = mod->m_Name;
            }

            mod->m_Directory = Path::GetDirectory( mod->m_FullName );
            mod->m_AddressStart = module->base_address();
            mod->m_AddressEnd =   module->base_address() + module->size();
            mod->m_DebugSignature = s2ws( module->debug_identifier() );

            process->AddModule(mod);
        }

        process->FindPdbs(GCoreApp->m_SymbolLocations);
        return process;
    }

    return nullptr;
}

#else

MiniDump::MiniDump( std::wstring a_FileName ){}
MiniDump::~MiniDump(){}
std::shared_ptr<Process> MiniDump::ToOrbitProcess(){ return nullptr; }

#endif