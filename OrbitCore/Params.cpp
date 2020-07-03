// Copyright (c) 2020 The Orbit Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "Params.h"

#include <fstream>

#include "google/protobuf/io/zero_copy_stream_impl.h"
#include "google/protobuf/text_format.h"

#include "Core.h"
#include "CoreApp.h"
#include "OrbitBase/Logging.h"

Params GParams;

//-----------------------------------------------------------------------------
Params::Params() {}

//-----------------------------------------------------------------------------
bool Params::Save() {
  std::string filename = Path::GetParamsFileName();
  std::ofstream file(filename);
  if (file.fail()) {
    ERROR("Saving Params in \"%s\": %s", filename, "file.fail()");
    return false;
  }

  google::protobuf::io::OstreamOutputStream output_stream(&file);
  if (!google::protobuf::TextFormat::Print(config, &output_stream)) {
    ERROR("Saving Params in \"%s\" failed: %s", filename,
          config.ShortDebugString());
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
bool Params::Load() {
  std::string filename = Path::GetParamsFileName();
  std::ifstream file(filename);
  if (file.fail()) {
    ERROR("Loading Params from \"%s\": %s", filename, "file.fail()");
    // Try creating the file with default values, in case it doesn't exist.
    return Save();
  }

  google::protobuf::io::IstreamInputStream input_stream(&file);
  if (!google::protobuf::TextFormat::Parse(&input_stream, &config)) {
    ERROR("Loading Params from \"%s\" failed", filename);
    // Try overwriting the file with default values, in case it's malformed.
    Save();
    return false;
  }
  return true;
}

//-----------------------------------------------------------------------------
void Params::AddToPdbHistory(const std::string& a_PdbName) {
  if (config.pdb_history(config.pdb_history_size() - 1) != a_PdbName) {
    std::string* new_entry = config.add_pdb_history();
    new_entry->assign(a_PdbName);
    Save();
  }
}
