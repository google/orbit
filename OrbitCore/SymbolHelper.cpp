#include "SymbolHelper.h"

#include <fstream>

#include "ElfFile.h"
#include "OrbitBase/Logging.h"
#include "Path.h"
#include "Pdb.h"

namespace {

std::vector<std::string> ReadSymbolsFile() {
  std::string file_name = Path::GetSymbolsFileName();
  if (!Path::FileExists(file_name)) {
    std::ofstream outfile(file_name);
    outfile << "//-------------------" << std::endl
            << "// Orbit Symbol Locations" << std::endl
            << "//-------------------" << std::endl
            << "// Orbit will scan the specified directories for symbol files."
            << std::endl
            << "// Enter one directory per line, like so:" << std::endl
#ifdef _WIN32
            << "// C:\\MyApp\\Release\\" << std::endl
            << "// D:\\MySymbolServer\\" << std::endl
#else
            << "// /home/git/project/build/" << std::endl
            << "// /home/symbol_server/" << std::endl
#endif
            << std::endl;

    outfile.close();
  }

  std::vector<std::string> directories;
  std::fstream infile(file_name);
  if (!infile.fail()) {
    std::string line;
    while (std::getline(infile, line)) {
      if (absl::StartsWith(line, "//") || line == "") continue;

      std::string dir = line;
      if (Path::DirExists(dir)) {
        directories.push_back(dir);
      }
    }
  }
  return directories;
}

bool LoadFromElfFile(std::shared_ptr<Module> module,
                     const std::unique_ptr<ElfFile>& elf_file) {
  auto load_bias = elf_file->GetLoadBias();
  if (!load_bias) {
    ERROR(
        "Unable to get load_bias %s (does the file have PT_LOAD program "
        "headers?)",
        elf_file->GetFilePath().c_str());
    return false;
  }

  std::shared_ptr<Pdb> pdb =
      std::make_shared<Pdb>(module->m_AddressStart, load_bias.value(),
                            elf_file->GetFilePath(), module->m_FullName);

  if (!elf_file->LoadFunctions(pdb.get())) {
    ERROR("Unable to load functions from %s", elf_file->GetFilePath().c_str());
    return false;
  }

  pdb->ProcessData();

  module->m_Pdb = pdb;
  module->m_PdbName = elf_file->GetFilePath();
  module->SetLoaded(true);
  return true;
}

bool FindAndLoadSymbols(std::shared_ptr<Module> module,
                        const std::vector<std::string>& search_directories) {
  FAIL_IF(module->m_DebugSignature.empty(), "build id is empty of module %s",
          module->m_Name.c_str());
  std::string name_without_extension = Path::StripExtension(module->m_Name);

  std::vector<std::string> search_file_paths;
  for (const auto& directory : search_directories) {
    search_file_paths.emplace_back(directory + name_without_extension +
                                   ".debug");
    search_file_paths.emplace_back(directory + module->m_Name + ".debug");
    search_file_paths.emplace_back(directory + module->m_Name);
  }

  LOG("Trying to find symbols for module %s", module->m_Name.c_str());
  for (const auto& symbols_file_path : search_file_paths) {
    if (!Path::FileExists(symbols_file_path)) continue;

    std::unique_ptr<ElfFile> symbols_file = ElfFile::Create(symbols_file_path);
    if (!symbols_file) continue;
    if (!symbols_file->HasSymtab()) continue;
    if (symbols_file->GetBuildId() != module->m_DebugSignature) continue;

    return LoadFromElfFile(module, symbols_file);
  }

  return false;
}

bool CheckModuleHasBuildId(std::shared_ptr<Module> module) {
  if (module->m_DebugSignature.empty()) {
    ERROR(
        "Symbol loading from a separate file is not supported for module "
        "\"%s\", because it does not contain a build id. This likely means "
        "the build id has been turned off manually.",
        module->m_Name.c_str());
    return false;
  } else {
    return true;
  }
}

}  // namespace

SymbolHelper::SymbolHelper()
    : collector_symbol_directories_{"/home/cloudcast/",
                                    "/home/cloudcast/debug_symbols/",
                                    "/mnt/developer/",
                                    "/mnt/developer/debug_symbols/",
                                    "/srv/game/assets/",
                                    "/srv/game/assets/debug_symbols/"},
      symbols_file_directories_(ReadSymbolsFile()) {}

bool SymbolHelper::LoadSymbolsIncludedInBinary(
    std::shared_ptr<Module> module) const {
  std::unique_ptr<ElfFile> elf_file = ElfFile::Create(module->m_FullName);

  if (!elf_file) {
    ERROR("Unable to load module as elf-file: %s", module->m_FullName.c_str());
    return false;
  }

  if (!elf_file->HasSymtab()) return false;
  return LoadFromElfFile(module, elf_file);
}

bool SymbolHelper::LoadSymbolsCollector(std::shared_ptr<Module> module) const {
  if (LoadSymbolsIncludedInBinary(module)) return true;

  if (!CheckModuleHasBuildId(module)) return false;

  std::vector<std::string> search_directories = collector_symbol_directories_;
  search_directories.emplace_back(Path::GetDirectory(module->m_FullName));

  return FindAndLoadSymbols(module, search_directories);
}

bool SymbolHelper::LoadSymbolsUsingSymbolsFile(
    std::shared_ptr<Module> module) const {
  if (!CheckModuleHasBuildId(module)) return false;

  return FindAndLoadSymbols(module, symbols_file_directories_);
}

void SymbolHelper::LoadSymbolsFromDebugInfo(
    std::shared_ptr<Module> module, const ModuleDebugInfo& module_info) const {
  module->m_Pdb =
      std::make_shared<Pdb>(module->m_AddressStart, module_info.load_bias,
                            module_info.m_PdbName, module->m_FullName);

  for (const auto& function : module_info.m_Functions) {
    module->m_Pdb->AddFunction(function);
  }
  module->m_Pdb->ProcessData();
  module->m_PdbName = module_info.m_PdbName;
  module->SetLoaded(true);
}

void SymbolHelper::FillDebugInfoFromModule(std::shared_ptr<Module> module,
                                           ModuleDebugInfo& module_info) const {
  FAIL_IF(!module->m_Pdb, "Pdb not initiallized for module %s",
          module->m_Name.c_str());

  module_info.m_Name = module->m_Name;
  module_info.m_Functions = module->m_Pdb->GetFunctions();
  module_info.load_bias = module->m_Pdb->GetLoadBias();
  module_info.m_PdbName = module->m_PdbName;
}
