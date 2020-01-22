#include "Version.h"
#include "Utils.h"
#include "Core.h"

static constexpr const char* kOrbitVersionStr = "dev";

//-----------------------------------------------------------------------------
std::string OrbitVersion::s_LatestVersion;

//-----------------------------------------------------------------------------
std::string OrbitVersion::GetVersion()
{
    return kOrbitVersionStr;
}

