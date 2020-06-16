from conans import ConanFile
import os, re

def replace_in_file(file, line_to_search, replace_with, conanfile_output = None):
    with open(file, 'r') as input_file:
        input = input_file.read()

    output = re.sub(line_to_search, replace_with, input, flags=re.MULTILINE|re.DOTALL)

    if input == output:
        return False

    if conanfile_output is not None:
        conanfile_output.info('Replacing \"{}\" in {} with \"{}\"'.format(line_to_search, file, replace_with))

    with open(file, 'w') as output_file:
        output_file.write(output)

    return True


class LLVMPackage(ConanFile):
    VERSION = '9.0.1-2'
    SOURCE_DIR = '.'
    BUILD_DIR = 'build'
    INSTALL_DIR = 'install'
    PARALLEL_BUILD = os.environ.get('CONAN_LLVM_SINGLE_THREAD_BUILD') is None

    version = os.environ.get("CONAN_VERSION_OVERRIDE", VERSION)
    license = 'BSD'
    url = 'https://gitlab.com/Manu343726/clang-conan-packages'
    generators = 'cmake', 'cmake_paths', 'cmake_find_package'
    settings = 'os', 'compiler', 'build_type', 'arch'
    options = {
        'shared': [True, False],
        'sources_repo': 'ANY',
        'no_rtti': [True, False],
        'allow_undefined_symbols': [True, False],
        'fPIC': [True, False]
    }
    default_options = 'shared=False', 'sources_repo=https://github.com/llvm/llvm-project/releases/download/', 'no_rtti=False', 'allow_undefined_symbols=False', 'fPIC=True'

    def configure(self):
        del self.settings.compiler.libcxx

    def _llvm_dependency_package(self, component):
        return '{}/{}@{}/{}'.format(component, self.version, self.user, self.channel)

    @property
    def _llvm_requires(self):
        return getattr(self, 'llvm_requires', [])

    @property
    def _build_shared(self):
        return self.options.shared if 'shared' in self.options else False

    def requirements(self):
        for component in self._llvm_requires:
            self.requires(self._llvm_dependency_package(component))

    def configure(self):
        for component in self._llvm_requires:
            self.options[component].shared = self._build_shared
            self.options[component].sources_repo = self.options.sources_repo
            self.options[component].no_rtti = self.options.no_rtti
            self.options[component].allow_undefined_symbols = self.options.allow_undefined_symbols
            self.options[component].fPIC = self.options.fPIC
