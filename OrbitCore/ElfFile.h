#pragma once

#include <memory>
#include <optional>
#include <vector>

#include "OrbitFunction.h"
#include "Pdb.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

class ElfFile {
 public:
  ElfFile() = default;
  virtual ~ElfFile() = default;

  virtual bool GetFunctions(Pdb* pdb,
                            std::vector<Function>* functions) const = 0;
  // Background and some terminology
  // When an elf file is loaded to memory it has its load segments
  // (segments of PT_LOAD type from program headers) mapped to some
  // location in memory. The location of the first segment is called a base
  // address.
  // Symbols addresses in the elf file however are not offsets from the base
  // address, they are calculated as offset from some virtual 0 where that
  // virtual zero can be different from the base address.
  //
  // The way to calculate the virtual zero is to take base address and substract
  // from it the minimum vaddr specified in PT_LOAD program headers.
  //
  // This method returns load bias for the elf-file if program headers are
  // available. This should be the case for all loadable elf-files.
  virtual std::optional<uint64_t> GetLoadBias() const = 0;
  virtual bool IsAddressInTextSection(uint64_t address) const = 0;
  virtual bool HasSymtab() const = 0;
  virtual std::string GetBuildId() const = 0;
  virtual std::string GetFilePath() const = 0;

  static std::unique_ptr<ElfFile> Create(const std::string& file_path);
};
