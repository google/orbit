//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "LogInterface.h"

#include "Log.h"

std::vector<std::string> LogInterface::GetOutput() {
  return GLogger.ConsumeEntries(OrbitLog::Viz);
}