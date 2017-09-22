//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <ostream>
struct IDiaSymbol;

namespace OrbitDia
{
    void DiaDump( IDiaSymbol* a_Symbol );
    void DiaDump( unsigned long a_SymbolId );
    void DiaDump( IDiaSymbol* Symbol, std::ostream &OS, int Indent );
}