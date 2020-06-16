from conans import CMake
from llvmpackage import LLVMPackage, replace_in_file
import conans.tools
import os

class LLVMComponentPackage(LLVMPackage):
    version = LLVMPackage.version

    @property
    def _llvm_component(self):
        return getattr(self, 'llvm_component', self.name)

    @property
    def source_package_name(self):
        version = self.version.split("-")[0]
        return '{}-{}.src'.format(self._llvm_component, version)

    @property
    def _source_dir(self):
        return os.path.join(self.SOURCE_DIR, self.source_package_name)

    @property
    def _root_cmake_file(self):
        return os.path.join(self._source_dir, 'CMakeLists.txt')

    def source(self):
        version = self.version.split("-")[0]
        url = '{}/llvmorg-{}/{}.tar.xz'.format(self.options.sources_repo,
                                               version, self.source_package_name)
        conans.tools.get(url, destination=self.SOURCE_DIR)


        if not replace_in_file(self._root_cmake_file, r'project\((.+?)\)',
r'''message(STATUS "Main {0} CMakeLists.txt (${{CMAKE_CURRENT_LIST_DIR}}) patched by conan")

project(\1)

message(STATUS "Loading conan scripts for {0} dependencies...")
if(EXISTS "${{CMAKE_BINARY_DIR}}/conanbuildinfo.cmake")
  include("${{CMAKE_BINARY_DIR}}/conanbuildinfo.cmake")
else()
  include("${{CMAKE_BINARY_DIR}}/../conanbuildinfo.cmake")
endif()
message(STATUS "Doing conan basic setup")
conan_basic_setup()
list(APPEND CMAKE_PROGRAM_PATH ${{CONAN_BIN_DIRS}})
message(STATUS "Conan setup done. CMAKE_PROGRAM_PATH: ${{CMAKE_PROGRAM_PATH}}")
'''.format(self.name), self.output):
            self.output.warn("Could not patch {} main CMakeLists.txt file to include conan config".format(self._llvm_component))

        conans.tools.replace_in_file(os.path.join(self._source_dir, 'cmake', 'modules', 'HandleLLVMOptions.cmake'),
                              "NOT LLVM_USE_SANITIZER)", "NOT LLVM_USE_SANITIZER AND NOT ALLOW_UNDEFINED_SYMBOLS)")

        conans.tools.replace_in_file(os.path.join(self._source_dir, 'include', 'llvm', 'Support', 'ManagedStatic.h'),
                              "#if !defined(_MSC_VER) || defined(__clang__)",
                              "#if !defined(_MSC_VER) || (_MSC_VER >= 1925) || defined(__clang__)")

    @property
    def _install_dir(self):
        return os.path.join(self.build_folder, self.INSTALL_DIR)

    @property
    def _install_include_dir(self):
        return os.path.join(self._install_dir, 'include')

    @property
    def _install_lib_dir(self):
        return os.path.join(self._install_dir, 'lib')

    @property
    def _install_bin_dir(self):
        return os.path.join(self._install_dir, 'bin')

    def build(self):
        llvm_cmake_options = [
            'INCLUDE_TESTS',
            'BUILD_TESTS',
            'INCLUDE_EXAMPLES',
            'BUILD_EXAMPLES',
            'INCLUDE_DOCS',
            'BUILD_TOOLS'
        ]

        cmake_options = {}

        for option in llvm_cmake_options:
            cmake_options['{}_{}'.format(self.name.upper(), option)] = False
            cmake_options['LLVM_' + option] = False


        cmake_options['CMAKE_INSTALL_PREFIX'] = self._install_dir
        cmake_options['CMAKE_VERBOSE_MAKEFILE'] = False
        cmake_options['BUILD_SHARED_LIBS'] = self._build_shared
        cmake_options['LLVM_TARGETS_TO_BUILD'] = 'X86'
        cmake_options['LLVM_ABI_BREAKING_CHECKS'] = "FORCE_OFF"
        cmake_options['LLVM_ENABLE_TERMINFO'] = False
        cmake_options['LLVM_ENABLE_RTTI'] = not self.options.no_rtti
        cmake_options['ALLOW_UNDEFINED_SYMBOLS'] = self.options.allow_undefined_symbols
        cmake_options['LLVM_ENABLE_PIC'] = self.options.fPIC

        if hasattr(self, 'custom_cmake_options'):
            cmake_options.update(dict(self.custom_cmake_options))

        cmake = CMake(self, parallel=self.PARALLEL_BUILD)
        cmake.configure(defs=cmake_options, source_folder=self._source_dir)
        cmake.build()
        cmake.install()

    def package(self):
        exclude_headers  = getattr(self, 'package_exclude_headers', None)
        exclude_libs     = getattr(self, 'package_exclude_libs', None)
        exclude_binaries = getattr(self, 'package_exclude_binaries', None)

        self.copy(pattern='*',
            excludes=exclude_headers,
            dst='include',
            src=self._install_include_dir,
            keep_path=True)

        for glob in ['*.a', '*.lib', '*.so*', '*.dll', '*.dylib', '*.cmake', '*.h']:
            self.copy(pattern=glob,
                excludes=exclude_libs,
                dst='lib',
                src=self._install_lib_dir,
                keep_path=True,
                symlinks=True)

        self.copy(pattern="*",
            excludes=exclude_binaries,
            dst="bin",
            src=self._install_bin_dir,
            keep_path=True)

        if hasattr(self, 'after_package') and callable(self.after_package):
            self.after_package()

    def package_info(self):
        collected_libs = conans.tools.collect_libs(self)

        if hasattr(self, 'package_info_exclude_libs'):
            self.cpp_info.libs = list(filter(lambda lib: not lib in self.package_info_exclude_libs, collected_libs))
        else:
            self.cpp_info.libs = collected_libs

        if self.settings.os == "Linux":
            self.cpp_info.libs.append('dl')
