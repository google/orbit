//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

namespace Orbit{ class Plugin; }

class PluginManager
{
public:
    void Initialize();

    void OnReceiveUserData( const Message & a_Msg );
    void OnReceiveOrbitData( const Message & a_Msg );

    std::vector<Orbit::Plugin*> m_Plugins;
};

extern PluginManager GPluginManager;