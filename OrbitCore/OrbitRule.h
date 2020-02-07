//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------
#pragma once

#include <memory>
#include <vector>

class Function;
class Variable;
namespace Orbit {
class Plugin;
}

class Rule {
 public:
  Function* m_Function;
  bool m_TrackArguments;
  bool m_TrackReturnValue;
  std::vector<std::shared_ptr<Variable>> m_TrackedVariables;
};
