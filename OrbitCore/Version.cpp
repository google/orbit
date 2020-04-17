#include "Version.h"

#include "Core.h"
#include "Utils.h"

#define OrbitVersionStr "dev"

//-----------------------------------------------------------------------------
std::string OrbitVersion::GetVersion() { return OrbitVersionStr; }

//-----------------------------------------------------------------------------
bool OrbitVersion::IsDev() { return GetVersion() == std::string("dev"); }
