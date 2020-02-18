#pragma once

#include <memory>
#include <vector>

#include "OrbitFunction.h"
#include "Pdb.h"
#include "llvm/Object/Binary.h"
#include "llvm/Object/ObjectFile.h"

class ElfFile {
 public:
  static std::unique_ptr<ElfFile> Create(const std::string& file_path);
  bool GetFunctions(Pdb* pdb, std::vector<Function>* functions) const;
  bool IsAddressInTextSection(uint64_t address) const;

 private:
  explicit ElfFile(const std::string& file_path) : file_path_(file_path) {}
  ElfFile() = delete;
  ElfFile(const ElfFile&) = delete;
  ElfFile& operator=(const ElfFile&) = delete;

  bool Load();

  const std::string file_path_;
  llvm::object::OwningBinary<llvm::object::ObjectFile> file_;
  std::unique_ptr<llvm::object::SectionRef> text_section_;
};
