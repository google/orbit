from conans import ConanFile, tools
from conans.errors import ConanInvalidConfiguration
import os
import json
import re

class CrashpadConan(ConanFile):
    name = "crashpad"
    version = "20191105"
    description = "Crashpad is a crash-reporting system."
    license = "Apache-2.0"
    homepage = "https://github.com/chromium/crashpad.git"
    url = "https://github.com/bincrafters/conan-crashpad"
    topics = ("conan", "crash-reporting", "logging", "minidump", "crash")
    settings = "os", "compiler", "build_type", "arch"
    options = {'linktime_optimization': [True, False], 'fPIC': [True, False]}
    default_options = {"linktime_optimization": False, 'fPIC': True}
    exports = [ "patches/*", "LICENSE.md" ]
    short_paths = True

    _commit_id = "1b60c8172c783040b86c3c6960aba4df73990bc5"
    _source_dir = "crashpad"
    _build_name = "out/Conan"
    _build_dir = os.path.join(_source_dir, _build_name)

    def _depot_tools(self):
        return os.path.join(self.source_folder, "depot_tools")

    def _crashpad_source_base(self):
        return os.path.join(self.source_folder, "crashpad_source")

    def _crashpad_source(self):
        return os.path.join(self._crashpad_source_base(), "crashpad")

    def build_requirements(self):
        if self.settings.os == "Windows":
            self.build_requires("depot_tools_installer/20190909@bincrafters/stable")
        else:
            self.build_requires("depot_tools_installer/20200207@bincrafters/stable")
            self.build_requires("openssl/1.1.1d@orbitdeps/stable")
        self.build_requires("ninja/1.9.0")

    def _mangle_spec_for_gclient(self, solutions):
        return json.dumps(solutions)          \
                   .replace("\"", "\\\"")     \
                   .replace("false", "False") \
                   .replace("true", "True")

    def _make_spec(self):
        solutions = [{
            "url": "%s@%s" % (self.homepage, self._commit_id),
            "managed": False,
            "name": "%s" % (self.name),
        }]
        return "solutions=%s" % self._mangle_spec_for_gclient(solutions)

    def config_options(self):
        if self.settings.os == "Windows":
            del self.options.fPIC

    def configure(self):
        # It's not a C project, but libcxx is hardcoded in the project
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

        if self.settings.os != "Windows" and not self.options.fPIC:
            raise ConanInvalidConfiguration("We only support compiling with fPIC enabled!")

    def source(self):
        self.run("gclient config --spec=\"%s\"" % self._make_spec(), run_environment=True)
        self.run("gclient sync --no-history", run_environment=True)

        if self.settings.os == "Windows":
            tools.patch(base_path=os.path.join(self._source_dir, "third_party/mini_chromium/mini_chromium"),
                        patch_file="patches/windows_adaptions.patch")

    def _get_target_cpu(self):
        arch = str(self.settings.arch)

        if arch == "x86":
            return "x86"
        elif arch == "x86_64":
            return "x64"

        # best effort... please contribute, if you actually tested those platforms
        elif arch.startswith("arm"):
            match = re.match('^armv([0-9]+)', arch)
            if int(match.group(1)) >= 8 and not "32" in arch:
                return "arm64"
            else:
                return "arm"
        elif arch.startswith("mips"):
            return "mipsel"

        raise ConanInvalidConfiguration("your architecture (%s) is not supported" % arch)

    def _setup_args_gn(self):
        os_map = { 'Windows': 'win', 'Linux': 'linux' } # Maps conan os identifiers to GN os identifiers
        target_os = os_map[str(self.settings.os)]
        args = ["is_debug=%s" % ("true" if self.settings.build_type == "Debug" else "false"),
                "target_cpu=\\\"%s\\\"" % self._get_target_cpu(),
                "target_os=\\\"{}\\\"".format(target_os),
                "target_sysroot=\\\"{}\\\"".format(os.environ.get('sysroot', '')) ]

        if self.settings.os == "Macos" and self.settings.get_safe("os.version"):
            args += [ "mac_deployment_target=\\\"%s\\\"" % self.settings.os.version ]
        if self.settings.os == "Windows":
            args += [ "linktime_optimization=%s" % str(self.options.linktime_optimization).lower()]
        if self.settings.os == "Windows" and self.settings.get_safe("compiler.runtime"):
            crt = str(self.settings.compiler.runtime)
            args += [ "dynamic_crt=%s" % ("true" if crt.startswith("MD") else "false") ]
        if self.settings.os == "Linux":
            args += [ "crashpad_use_boringssl_for_http_transport_socket=true"]
        return " ".join(args)

    def build(self):
        tools.patch(patch_file="patches/openssl_lib_order.patch")

        BUILD_gn = 'crashpad/third_party/mini_chromium/mini_chromium/build/BUILD.gn'
        tools.replace_in_file(BUILD_gn, 'cc = "clang"', 'cc = "{}"'.format(os.environ.get('CC', 'cc')))
        tools.replace_in_file(BUILD_gn, 'cxx = "clang++"', 'cxx = "{}"'.format(os.environ.get('CXX', 'c++')))

        if self.settings.os != "Windows" and self.settings.compiler == "gcc":
            tools.replace_in_file(BUILD_gn, '"-Werror",', '"-Wno-multichar",')
            tools.replace_in_file(BUILD_gn, '"-Wheader-hygiene",', '')
            tools.replace_in_file(BUILD_gn, '"-Wnewline-eof",', '')
            tools.replace_in_file(BUILD_gn, '"-Wstring-conversion",', '')
            tools.replace_in_file(BUILD_gn, '"-Wexit-time-destructors"', '')
            tools.replace_in_file(BUILD_gn, '"-fobjc-call-cxx-cdtors",', '')

        def quote(x):
            return '"{}"'.format(x)

        ldflags = list(map(quote, os.environ.get('LDFLAGS', '').split(' ')))

        cflags = list(map(quote, os.environ.get('CFLAGS', '').split(' ')))
        cflags.append('"-std=c11"')

        cxxflags = list(map(quote, os.environ.get('CXXFLAGS', '').split(' ')))
        cxxflags.append('"-std=c++14"')

        if "openssl" in self.deps_cpp_info.deps:
            openssl_info = self.deps_cpp_info["openssl"]
            ldflags.append('"-L{}"'.format(openssl_info.lib_paths[0]))
            cxxflags.append('"-I{}"'.format(openssl_info.include_paths[0]))

        tools.replace_in_file(BUILD_gn, 'ldflags = []', 'ldflags = [ {} ]'.format(", ".join(ldflags)))
        tools.replace_in_file(BUILD_gn, 'cflags_c = [ "-std=c11" ]', 'cflags_c = [ {} ]'.format(", ".join(cflags)))
        tools.replace_in_file(BUILD_gn, 'cflags_cc = [ "-std=c++14" ]', 'cflags_cc = [ {} ]'.format(", ".join(cxxflags)))

        with tools.chdir(self._source_dir):
            self.run('gn gen %s --args="%s"' % (self._build_name, self._setup_args_gn()), run_environment=True)
            self.run("ninja -j%d -C %s" % (tools.cpu_count(), self._build_name), run_environment=True)

    def _copy_lib(self, src_dir):
        self.copy("*.a", dst="lib",
                  src=os.path.join(self._build_dir, src_dir), keep_path=False)
        self.copy("*.lib", dst="lib",
                  src=os.path.join(self._build_dir, src_dir), keep_path=False)

    def _copy_headers(self, dst_dir, src_dir):
        self.copy("*.h", dst=os.path.join("include", dst_dir),
                         src=os.path.join(self._source_dir, src_dir))

    def _copy_bin(self, src_bin):
        self.copy(src_bin, src=self._build_dir, dst="bin")
        self.copy("%s.exe" % src_bin, src=self._build_dir, dst="bin")

    def package(self):
        self.copy("LICENSE", dst="licenses", src=self._source_dir,
                             ignore_case=True, keep_path=False)

        self._copy_headers("crashpad/client", "client")
        self._copy_headers("crashpad/util",   "util")
        self._copy_headers("mini_chromium",   "third_party/mini_chromium/mini_chromium")
        self._copy_lib("obj/client")
        self._copy_lib("obj/util")
        self._copy_lib("obj/third_party/mini_chromium")
        self._copy_bin("crashpad_handler")

    def package_info(self):
        self.cpp_info.includedirs = [ "include/crashpad", "include/mini_chromium" ]
        self.cpp_info.libdirs = [ "lib" ]
        self.cpp_info.libs = [ "client", "util", "base" ]

        if self.settings.os == "Macos":
            self.cpp_info.exelinkflags.append("-framework CoreFoundation")
            self.cpp_info.exelinkflags.append("-framework CoreGraphics")
            self.cpp_info.exelinkflags.append("-framework CoreText")
            self.cpp_info.exelinkflags.append("-framework Foundation")
            self.cpp_info.exelinkflags.append("-framework IOKit")
            self.cpp_info.exelinkflags.append("-framework Security")
            self.cpp_info.exelinkflags.append("-lbsm")
            self.cpp_info.sharedlinkflags = self.cpp_info.exelinkflags
