#pragma once

#include <memory>
#include <vector>

#include "OrbitFunction.h"
#include "Pdb.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

class ElfFile {
 public:
  ElfFile() = default;

  virtual bool GetFunctions(Pdb* pdb,
                            std::vector<Function>* functions) const = 0;
  virtual bool IsAddressInTextSection(uint64_t address) const = 0;

  static std::unique_ptr<ElfFile> Create(const std::string& file_path);
};
