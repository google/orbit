import os
import fnmatch
import platform
from functools import total_ordering
from conans.errors import ConanInvalidConfiguration, ConanException
from conans import ConanFile, AutoToolsBuildEnvironment, tools
from conans.tools import os_info


@total_ordering
class OpenSSLVersion(object):
    def __init__(self, version_str):
        self._pre = ""

        tokens = version_str.split("-")
        if len(tokens) > 1:
            self._pre = tokens[1]
        version_str = tokens[0]

        tokens = version_str.split(".")
        self._major = int(tokens[0])
        self._minor = 0
        self._patch = 0
        self._build = ""
        if len(tokens) > 1:
            self._minor = int(tokens[1])
            if len(tokens) > 2:
                self._patch = tokens[2]
                if self._patch[-1].isalpha():
                    self._build = self._patch[-1]
                    self._patch = self._patch[:1]
                self._patch = int(self._patch)

    @property
    def base(self):
        return "%s.%s.%s" % (self._major, self._minor, self._patch)

    @property
    def as_list(self):
        return [self._major, self._minor, self._patch, self._build, self._pre]

    def __eq__(self, other):
        return self.compare(other) == 0

    def __lt__(self, other):
        return self.compare(other) == -1

    def __hash__(self):
        return hash(self.as_list)

    def compare(self, other):
        if not isinstance(other, OpenSSLVersion):
            other = OpenSSLVersion(other)
        if self.as_list == other.as_list:
            return 0
        elif self.as_list < other.as_list:
            return -1
        else:
            return 1


class OpenSSLConan(ConanFile):
    name = "openssl"
    version = "1.1.1d"
    settings = "os", "compiler", "arch", "build_type"
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://github.com/openssl/openssl"
    license = "OpenSSL"
    topics = ("conan", "openssl", "ssl", "tls", "encryption", "security")
    description = "A toolkit for the Transport Layer Security (TLS) and Secure Sockets Layer (SSL) protocols"
    options = {"no_threads": [True, False],
               "no_zlib": [True, False],
               "shared": [True, False],
               "fPIC": [True, False],
               "no_asm": [True, False],
               "386": [True, False],
               "no_sse2": [True, False],
               "no_bf": [True, False],
               "no_cast": [True, False],
               "no_des": [True, False],
               "no_dh": [True, False],
               "no_dsa": [True, False],
               "no_hmac": [True, False],
               "no_md2": [True, False],
               "no_md5": [True, False],
               "no_mdc2": [True, False],
               "no_rc2": [True, False],
               "no_rc4": [True, False],
               "no_rc5": [True, False],
               "no_rsa": [True, False],
               "no_sha": [True, False],
               "no_async": [True, False],
               "no_dso": [True, False],
               "capieng_dialog": [True, False],
               "openssldir": "ANY"}
    default_options = {key: False for key in options.keys()}
    default_options["fPIC"] = True
    default_options["openssldir"] = None
    _env_build = None
    _source_subfolder = "sources"
    exports_sources = [ "support_for_glibc224.diff" ]

    def build_requirements(self):
        # useful for example for conditional build_requires
        if tools.os_info.is_windows:
            if not self._win_bash:
                self.build_requires("strawberryperl/5.30.0.1")
            if not self.options.no_asm and not tools.which("nasm"):
                self.build_requires("nasm/2.14")
        if self._win_bash:
            if "CONAN_BASH_PATH" not in os.environ and os_info.detect_windows_subsystem() != 'msys2':
                self.build_requires("msys2/20190524")

    @property
    def _is_msvc(self):
        return self.settings.compiler == "Visual Studio"

    @property
    def _is_clangcl(self):
        return self.settings.compiler == "clang" and self.settings.os == "Windows"

    @property
    def _is_mingw(self):
        return self.settings.os == "Windows" and self.settings.compiler == "gcc"

    @property
    def _use_nmake(self):
        return self._is_clangcl or self._is_msvc

    @property
    def _full_version(self):
        return OpenSSLVersion(self.version)

    def source(self):
        try:
            tools.get(**self.conan_data["sources"][self.version])
        except ConanException:
            url = self.conan_data["sources"][self.version]["url"]
            url = url.replace("https://www.openssl.org/source/",
                              "https://www.openssl.org/source/old/%s" % self._full_version.base)
            tools.get(url, sha256=self.conan_data["sources"][self.version]["sha256"])
        extracted_folder = "openssl-" + self.version
        tools.patch(base_path=extracted_folder,
                    patch_file="support_for_glibc224.diff")
        os.rename(extracted_folder, self._source_subfolder)

    def configure(self):
        del self.settings.compiler.libcxx
        del self.settings.compiler.cppstd

    def config_options(self):
        if self.settings.os != "Windows":
            del self.options.capieng_dialog
        else:
            del self.options.fPIC

    def requirements(self):
        if not self.options.no_zlib:
            self.requires("zlib/1.2.11")

    @property
    def _target_prefix(self):
        if self._full_version < "1.1.0" and self.settings.build_type == "Debug":
            return "debug-"
        return ""

    @property
    def _target(self):
        target = "conan-%s-%s-%s-%s-%s" % (self.settings.build_type,
                                           self.settings.os,
                                           self.settings.arch,
                                           self.settings.compiler,
                                           self.settings.compiler.version)
        if self._use_nmake:
            target = "VC-" + target  # VC- prefix is important as it's checked by Configure
        if self._is_mingw:
            target = "mingw-" + target
        return target

    @property
    def _perlasm_scheme(self):
        # right now, we need to tweak this for iOS & Android only, as they inherit from generic targets
        the_arch = str(self.settings.arch)
        the_os = str(self.settings.os)
        if the_os in ["iOS", "watchOS", "tvOS"]:
            return "ios64" if the_arch in ["armv8", "armv8_32", "armv8.3", "x86_64"] else "ios32"
        if str(self.settings.os) == "Android":
            return {"armv7": "void",
                    "armv8": "linux64",
                    "mips": "o32",
                    "mips64": "64",
                    "x86": "android",
                    "x86_64": "elf"}.get(the_arch, None)
        return None

    @property
    def _asm_target(self):
        if str(self.settings.os) in ["Android", "iOS", "watchOS", "tvOS"]:
            return {
                "x86": "x86_asm",
                "x86_64": "x86_64_asm",
                "armv5el": "armv4_asm",
                "armv5hf": "armv4_asm",
                "armv6": "armv4_asm",
                "armv7": "armv4_asm",
                "armv7hf": "armv4_asm",
                "armv7s": "armv4_asm",
                "armv7k": "armv4_asm",
                "armv8": "aarch64_asm",
                "armv8_32": "aarch64_asm",
                "armv8.3": "aarch64_asm",
                "mips": "mips32_asm",
                "mips64": "mips64_asm",
                "sparc": "sparcv8_asm",
                "sparcv9": "sparcv9_asm",
                "ia64": "ia64_asm",
                "ppc32be": "ppc32_asm",
                "ppc32": "ppc32_asm",
                "ppc64le": "ppc64_asm",
                "ppc64": "ppc64_asm",
                "s390": "s390x_asm",
                "s390x": "s390x_asm"
            }.get(str(self.settings.arch), None)

    @property
    def _targets(self):
        is_cygwin = self.settings.get_safe("os.subsystem") == "cygwin"
        is_1_0 = self._full_version < "1.1.0"
        return {
            "Linux-x86-clang": ("%slinux-generic32" % self._target_prefix) if is_1_0 else "linux-x86-clang",
            "Linux-x86_64-clang": ("%slinux-x86_64" % self._target_prefix) if is_1_0 else "linux-x86_64-clang",
            "Linux-x86-*": ("%slinux-generic32" % self._target_prefix) if is_1_0 else "linux-x86",
            "Linux-x86_64-*": "%slinux-x86_64" % self._target_prefix,
            "Linux-armv4-*": "linux-armv4",
            "Linux-armv4i-*": "linux-armv4",
            "Linux-armv5el-*": "linux-armv4",
            "Linux-armv5hf-*": "linux-armv4",
            "Linux-armv6-*": "linux-armv4",
            "Linux-armv7-*": "linux-armv4",
            "Linux-armv7hf-*": "linux-armv4",
            "Linux-armv7s-*": "linux-armv4",
            "Linux-armv7k-*": "linux-armv4",
            "Linux-armv8-*": "linux-aarch64",
            "Linux-armv8.3-*": "linux-aarch64",
            "Linux-armv8-32-*": "linux-arm64ilp32",
            "Linux-mips-*": "linux-mips32",
            "Linux-mips64-*": "linux-mips64",
            "Linux-ppc32-*": "linux-ppc32",
            "Linux-ppc32le-*": "linux-pcc32",
            "Linux-ppc32be-*": "linux-ppc32",
            "Linux-ppc64-*": "linux-ppc64",
            "Linux-ppc64le-*": "linux-ppc64le",
            "Linux-pcc64be-*": "linux-pcc64",
            "Linux-s390x-*": "linux64-s390x",
            "Linux-e2k-*": "linux-generic64",
            "Linux-sparc-*": "linux-sparcv8",
            "Linux-sparcv9-*": "linux64-sparcv9",
            "Linux-*-*": "linux-generic32",
            "Macos-x86-*": "%sdarwin-i386-cc" % self._target_prefix,
            "Macos-x86_64-*": "%sdarwin64-x86_64-cc" % self._target_prefix,
            "Macos-ppc32-*": "%sdarwin-ppc-cc" % self._target_prefix,
            "Macos-ppc32be-*": "%sdarwin-ppc-cc" % self._target_prefix,
            "Macos-ppc64-*": "darwin64-ppc-cc",
            "Macos-ppc64be-*": "darwin64-ppc-cc",
            "Macos-*-*": "darwin-common",
            "iOS-*-*": "iphoneos-cross",
            "watchOS-*-*": "iphoneos-cross",
            "tvOS-*-*": "iphoneos-cross",
            # Android targets are very broken, see https://github.com/openssl/openssl/issues/7398
            "Android-armv7-*": "linux-generic32",
            "Android-armv7hf-*": "linux-generic32",
            "Android-armv8-*": "linux-generic64",
            "Android-x86-*": "linux-generic32",
            "Android-x86_64-*": "linux-generic64",
            "Android-mips-*": "linux-generic32",
            "Android-mips64-*": "linux-generic64",
            "Android-*-*": "linux-generic32",
            "Windows-x86-gcc": "Cygwin-x86" if is_cygwin else "mingw",
            "Windows-x86_64-gcc": "Cygwin-x86_64" if is_cygwin else "mingw64",
            "Windows-*-gcc": "Cygwin-common" if is_cygwin else "mingw-common",
            "Windows-ia64-Visual Studio": "%sVC-WIN64I" % self._target_prefix,  # Itanium
            "Windows-x86-Visual Studio": "%sVC-WIN32" % self._target_prefix,
            "Windows-x86_64-Visual Studio": "%sVC-WIN64A" % self._target_prefix,
            "Windows-armv7-Visual Studio": "VC-WIN32-ARM",
            "Windows-armv8-Visual Studio": "VC-WIN64-ARM",
            "Windows-*-Visual Studio": "VC-noCE-common",
            "Windows-ia64-clang": "%sVC-WIN64I" % self._target_prefix,  # Itanium
            "Windows-x86-clang": "%sVC-WIN32" % self._target_prefix,
            "Windows-x86_64-clang": "%sVC-WIN64A" % self._target_prefix,
            "Windows-armv7-clang": "VC-WIN32-ARM",
            "Windows-armv8-clang": "VC-WIN64-ARM",
            "Windows-*-clang": "VC-noCE-common",
            "WindowsStore-x86-*": "VC-WIN32-UWP",
            "WindowsStore-x86_64-*": "VC-WIN64A-UWP",
            "WindowsStore-armv7-*": "VC-WIN32-ARM-UWP",
            "WindowsStore-armv8-*": "VC-WIN64-ARM-UWP",
            "WindowsStore-*-*": "VC-WIN32-ONECORE",
            "WindowsCE-*-*": "VC-CE",
            "SunOS-x86-gcc": "%ssolaris-x86-gcc" % self._target_prefix,
            "SunOS-x86_64-gcc": "%ssolaris64-x86_64-gcc" % self._target_prefix,
            "SunOS-sparc-gcc": "%ssolaris-sparcv8-gcc" % self._target_prefix,
            "SunOS-sparcv9-gcc": "solaris64-sparcv9-gcc",
            "SunOS-x86-suncc": "%ssolaris-x86-cc" % self._target_prefix,
            "SunOS-x86_64-suncc": "%ssolaris64-x86_64-cc" % self._target_prefix,
            "SunOS-sparc-suncc": "%ssolaris-sparcv8-cc" % self._target_prefix,
            "SunOS-sparcv9-suncc": "solaris64-sparcv9-cc",
            "SunOS-*-*": "solaris-common",
            "*BSD-x86-*": "BSD-x86",
            "*BSD-x86_64-*": "BSD-x86_64",
            "*BSD-ia64-*": "BSD-ia64",
            "*BSD-sparc-*": "BSD-sparcv8",
            "*BSD-sparcv9-*": "BSD-sparcv9",
            "*BSD-armv8-*": "BSD-generic64",
            "*BSD-mips64-*": "BSD-generic64",
            "*BSD-ppc64-*": "BSD-generic64",
            "*BSD-ppc64le-*": "BSD-generic64",
            "*BSD-ppc64be-*": "BSD-generic64",
            "AIX-ppc32-gcc": "aix-gcc",
            "AIX-ppc64-gcc": "aix64-gcc",
            "AIX-pcc32-*": "aix-cc",
            "AIX-ppc64-*": "aix64-cc",
            "AIX-*-*": "aix-common",
            "*BSD-*-*": "BSD-generic32",
            "Emscripten-*-*": "cc",
            "Neutrino-*-*": "BASE_unix",
        }

    @property
    def _ancestor_target(self):
        if "CONAN_OPENSSL_CONFIGURATION" in os.environ:
            return os.environ["CONAN_OPENSSL_CONFIGURATION"]
        query = "%s-%s-%s" % (self.settings.os, self.settings.arch, self.settings.compiler)
        ancestor = next((self._targets[i] for i in self._targets if fnmatch.fnmatch(query, i)), None)
        if not ancestor:
            raise ConanInvalidConfiguration("unsupported configuration: %s %s %s, "
                                            "please open an issue: "
                                            "https://github.com/conan-community/community/issues. "
                                            "alternatively, set the CONAN_OPENSSL_CONFIGURATION environment variable "
                                            "into your conan profile "
                                            "(list of configurations can be found by running './Configure --help')." %
                                            (self.settings.os,
                                            self.settings.arch,
                                            self.settings.compiler))
        return ancestor

    def _tool(self, env_name, apple_name):
        if env_name in os.environ:
            return os.environ[env_name]
        if self.settings.compiler == "apple-clang":
            return getattr(tools.XCRun(self.settings), apple_name)
        return None

    def _patch_makefile_org(self):
        # https://wiki.openssl.org/index.php/Compilation_and_Installation#Modifying_Build_Settings
        # its often easier to modify Configure and Makefile.org rather than trying to add targets to the configure scripts
        def adjust_path(path):
            return path.replace("\\", "/") if tools.os_info.is_windows else path

        makefile_org = os.path.join(self._source_subfolder, "Makefile.org")
        env_build = self._get_env_build()
        with tools.environment_append(env_build.vars):
            if not "CROSS_COMPILE" in os.environ:
                cc = os.environ.get("CC", "cc")
                tools.replace_in_file(makefile_org, "CC= cc", "CC= %s %s" % (adjust_path(cc), os.environ["CFLAGS"]))
                if "AR" in os.environ:
                    tools.replace_in_file(makefile_org, "AR=ar", "AR=%s" % adjust_path(os.environ["AR"]))
                if "RANLIB" in os.environ:
                    tools.replace_in_file(makefile_org, "RANLIB= ranlib", "RANLIB= %s" % adjust_path(os.environ["RANLIB"]))
                rc = os.environ.get("WINDRES", os.environ.get("RC"))
                if rc:
                    tools.replace_in_file(makefile_org, "RC= windres", "RC= %s" % adjust_path(rc))
                if "NM" in os.environ:
                    tools.replace_in_file(makefile_org, "NM= nm", "NM= %s" % adjust_path(os.environ["NM"]))
                if "AS" in os.environ:
                    tools.replace_in_file(makefile_org, "AS=$(CC) -c", "AS=%s" % adjust_path(os.environ["AS"]))

    def _get_env_build(self):
        if not self._env_build:
            self._env_build = AutoToolsBuildEnvironment(self)
            if self.settings.compiler == "apple-clang":
                # add flags only if not already specified, avoid breaking Catalyst which needs very special flags
                flags = " ".join(self._env_build.flags)
                if "-arch" not in flags:
                    self._env_build.flags.append("-arch %s" % tools.to_apple_arch(self.settings.arch))
                if "-isysroot" not in flags:
                    self._env_build.flags.append("-isysroot %s" % tools.XCRun(self.settings).sdk_path)
                if self.settings.get_safe("os.version") and "-version-min=" not in flags and "-target" not in flags:
                    self._env_build.flags.append(tools.apple_deployment_target_flag(self.settings.os,
                                                                              self.settings.os.version))
        return self._env_build

    @property
    def _configure_args(self):
        openssldir = self.options.openssldir if self.options.openssldir else os.path.join(self.package_folder, "res")
        prefix = tools.unix_path(self.package_folder) if self._win_bash else self.package_folder
        openssldir = tools.unix_path(openssldir) if self._win_bash else openssldir
        args = ['"%s"' % (self._target if self._full_version >= "1.1.0" else self._ancestor_target),
                "shared" if self.options.shared else "no-shared",
                "--prefix=%s" % prefix,
                "--openssldir=%s" % openssldir,
                "no-unit-test"]
        if self._full_version >= "1.1.1":
            args.append("PERL=%s" % self._perl)
        if self._full_version < "1.1.0" or self._full_version >= "1.1.1":
            args.append("no-tests")
        if self._full_version >= "1.1.0":
            args.append("--debug" if self.settings.build_type == "Debug" else "--release")

        if str(self.settings.os) == "Android":
            args.append(" -D__ANDROID_API__=%s" % str(self.settings.os.api_level))  # see NOTES.ANDROID
        if str(self.settings.os) == "Emscripten":
            args.append("-D__STDC_NO_ATOMICS__=1")
        if self.settings.os == "Windows":
            if self.options.capieng_dialog:
                args.append("-DOPENSSL_CAPIENG_DIALOG=1")
        else:
            args.append("-fPIC" if self.options.fPIC else "")
        if self.settings.os == "Neutrino":
            args.append("-lsocket no-asm")

        if "zlib" in self.deps_cpp_info.deps:
            zlib_info = self.deps_cpp_info["zlib"]
            include_path = zlib_info.include_paths[0]
            if self.settings.os == "Windows":
                lib_path = "%s/%s.lib" % (zlib_info.lib_paths[0], zlib_info.libs[0])
            else:
                lib_path = zlib_info.lib_paths[0]  # Just path, linux will find the right file
            if tools.os_info.is_windows:
                # clang-cl doesn't like backslashes in #define CFLAGS (builldinf.h -> cversion.c)
                include_path = include_path.replace('\\', '/')
                lib_path = lib_path.replace('\\', '/')
            args.extend(['--with-zlib-include="%s"' % include_path,
                         '--with-zlib-lib="%s"' % lib_path])

        for option_name in self.options.values.fields:
            activated = getattr(self.options, option_name)
            if activated and option_name not in ["fPIC", "openssldir", "capieng_dialog"]:
                self.output.info("activated option: %s" % option_name)
                args.append(option_name.replace("_", "-"))
        return args

    def _create_targets(self):
        config_template = """{targets} = (
    "{target}" => {{
        inherit_from => {ancestor},
        cflags => add("{cflags}"),
        cxxflags => add("{cxxflags}"),
        {defines}
        includes => add({includes}),
        lflags => add("{lflags}"),
        {cc}
        {cxx}
        {ar}
        {ranlib}
        {perlasm_scheme}
    }},
);
"""
        cflags = []

        env_build = self._get_env_build()
        cflags.extend(env_build.flags)
        cxxflags = cflags[:]
        cxxflags.extend(env_build.cxx_flags)

        cc = self._tool("CC", "cc")
        cxx = self._tool("CXX", "cxx")
        ar = self._tool("AR", "ar")
        ranlib = self._tool("RANLIB", "ranlib")

        perlasm_scheme = ""
        if self._perlasm_scheme:
            perlasm_scheme = 'perlasm_scheme => "%s",' % self._perlasm_scheme

        cc = 'cc => "%s",' % cc if cc else ""
        cxx = 'cxx => "%s",' % cxx if cxx else ""
        ar = 'ar => "%s",' % ar if ar else ""
        defines = " ".join(env_build.defines)
        defines = 'defines => add("%s"),' % defines if defines else ""
        ranlib = 'ranlib => "%s",' % ranlib if ranlib else ""
        targets = "my %targets" if self._full_version >= "1.1.1" else "%targets"
        includes = ", ".join(['"%s"' % include for include in env_build.include_paths])
        if self.settings.os == "Windows":
            includes = includes.replace('\\', '/') # OpenSSL doesn't like backslashes

        if self._asm_target:
            ancestor = '[ "%s", asm("%s") ]' % (self._ancestor_target, self._asm_target)
        else:
            ancestor = '[ "%s" ]' % self._ancestor_target

        config = config_template.format(targets=targets,
                                        target=self._target,
                                        ancestor=ancestor,
                                        cc=cc,
                                        cxx=cxx,
                                        ar=ar,
                                        ranlib=ranlib,
                                        cflags=" ".join(cflags),
                                        cxxflags=" ".join(cxxflags),
                                        defines=defines,
                                        includes=includes,
                                        perlasm_scheme=perlasm_scheme,
                                        lflags=" ".join(env_build.link_flags))
        self.output.info("using target: %s -> %s" % (self._target, self._ancestor_target))
        self.output.info(config)

        tools.save(os.path.join(self._source_subfolder, "Configurations", "20-conan.conf"), config)

    def _run_make(self, targets=None, makefile=None, parallel=True):
        command = [self._make_program]
        if makefile:
            command.extend(["-f", makefile])
        if targets:
            command.extend(targets)
        if not self._use_nmake:
            # workaround for random error: size too large (archive member extends past the end of the file)
            # /Library/Developer/CommandLineTools/usr/bin/ar: internal ranlib command failed
            if self.settings.os == "Macos" and self._full_version < "1.1.0":
                parallel = False
            command.append(("-j%s" % tools.cpu_count()) if parallel else "-j1")
        self.run(" ".join(command), win_bash=self._win_bash)

    @property
    def _perl(self):
        if tools.os_info.is_windows and not self._win_bash:
            # enforce strawberry perl, otherwise wrong perl could be used (from Git bash, MSYS, etc.)
            return os.path.join(self.deps_cpp_info["strawberryperl"].rootpath, "bin", "perl.exe")
        return "perl"

    def _make(self):
        with tools.chdir(self._source_subfolder):
            # workaround for MinGW (https://github.com/openssl/openssl/issues/7653)
            if not os.path.isdir(os.path.join(self.package_folder, "bin")):
                os.makedirs(os.path.join(self.package_folder, "bin"))
            # workaround for clang-cl not producing .pdb files
            if self._is_clangcl:
                tools.save("ossl_static.pdb", "")
            args = " ".join(self._configure_args)
            self.output.info(self._configure_args)

            self.run('{perl} ./Configure {args}'.format(perl=self._perl, args=args), win_bash=self._win_bash)

            self._patch_install_name()

            if self._use_nmake and self._full_version < "1.1.0":
                if not self.options.no_asm and self.settings.arch == "x86":
                    self.run(r"ms\do_nasm")
                else:
                    self.run(r"ms\do_ms" if self.settings.arch == "x86" else r"ms\do_win64a")
                makefile = r"ms\ntdll.mak" if self.options.shared else r"ms\nt.mak"

                self._replace_runtime_in_file(os.path.join("ms", "nt.mak"))
                self._replace_runtime_in_file(os.path.join("ms", "ntdll.mak"))
                if self.settings.arch == "x86":
                    tools.replace_in_file(os.path.join("ms", "nt.mak"), "-WX", "")
                    tools.replace_in_file(os.path.join("ms", "ntdll.mak"), "-WX", "")

                self._run_make(makefile=makefile)
                self._run_make(makefile=makefile, targets=["install"], parallel=False)
            else:
                self._run_make()
                self._run_make(targets=["install_sw"], parallel=False)

    @property
    def _cc(self):
        if "CROSS_COMPILE" in os.environ:
            return "gcc"
        if "CC" in os.environ:
            return os.environ["CC"]
        if self.settings.compiler == "apple-clang":
            return tools.XCRun(self.settings).find("clang")
        elif self.settings.compiler == "clang":
            return "clang"
        elif self.settings.compiler == "gcc":
            return "gcc"
        return "cc"

    def build(self):
        with tools.vcvars(self.settings) if self._use_nmake else tools.no_op():
            env_vars = {"PERL": self._perl}
            if self._full_version < "1.1.0":
                cflags = " ".join(self._get_env_build().flags)
                env_vars["CC"] = "%s %s" % (self._cc, cflags)
            if self.settings.compiler == "apple-clang":
                xcrun = tools.XCRun(self.settings)
                env_vars["CROSS_SDK"] = os.path.basename(xcrun.sdk_path)
                env_vars["CROSS_TOP"] = os.path.dirname(os.path.dirname(xcrun.sdk_path))
            with tools.environment_append(env_vars):
                if self._full_version >= "1.1.0":
                    self._create_targets()
                else:
                    self._patch_makefile_org()
                self._make()

    @staticmethod
    def detected_os():
        if tools.OSInfo().is_macos:
            return "Macos"
        if tools.OSInfo().is_windows:
            return "Windows"
        return platform.system()

    @property
    def _cross_building(self):
        if tools.cross_building(self.settings):
            if self.settings.os == self.detected_os():
                if self.settings.arch == "x86" and tools.detected_architecture() == "x86_64":
                    return False
            return True
        return False

    @property
    def _win_bash(self):
        return tools.os_info.is_windows and \
               not self._use_nmake and \
               (self._is_mingw or self._cross_building)

    @property
    def _make_program(self):
        if self._use_nmake:
            return "nmake"
        make_program = tools.get_env("CONAN_MAKE_PROGRAM", tools.which("make") or tools.which('mingw32-make'))
        make_program = tools.unix_path(make_program) if tools.os_info.is_windows else make_program
        if not make_program:
            raise Exception('could not find "make" executable. please set "CONAN_MAKE_PROGRAM" environment variable')
        return make_program

    def _patch_install_name(self):
        if self.settings.os == "Macos" and self.options.shared:
            old_str = '-install_name $(INSTALLTOP)/$(LIBDIR)/'
            new_str = '-install_name '

            makefile = "Makefile" if self._full_version >= "1.1.1" else "Makefile.shared"
            tools.replace_in_file(makefile, old_str, new_str, strict=self.in_local_cache)

    def _replace_runtime_in_file(self, filename):
        for e in ["MDd", "MTd", "MD", "MT"]:
            tools.replace_in_file(filename, "/%s " % e, "/%s " % self.settings.compiler.runtime, strict=False)

    def package(self):
        self.copy(src=self._source_subfolder, pattern="*LICENSE", dst="licenses")
        for root, _, files in os.walk(self.package_folder):
            for filename in files:
                if fnmatch.fnmatch(filename, "*.pdb"):
                    os.unlink(os.path.join(self.package_folder, root, filename))
        if self._use_nmake:
            if self.settings.build_type == 'Debug' and self._full_version >= "1.1.0":
                with tools.chdir(os.path.join(self.package_folder, 'lib')):
                    os.rename('libssl.lib', 'libssld.lib')
                    os.rename('libcrypto.lib', 'libcryptod.lib')
        # Old OpenSSL version family has issues with permissions.
        # See https://github.com/conan-io/conan/issues/5831
        if self._full_version < "1.1.0" and self.options.shared and self.settings.os in ("Android", "FreeBSD", "Linux"):
            with tools.chdir(os.path.join(self.package_folder, "lib")):
                os.chmod("libssl.so.1.0.0", 0o755)
                os.chmod("libcrypto.so.1.0.0", 0o755)
        tools.rmdir(os.path.join(self.package_folder, "lib", "pkgconfig"))

    def package_info(self):
        self.cpp_info.names["cmake_find_package"] = "OpenSSL"
        self.cpp_info.names["cmake_find_package_multi"] = "OpenSSL"
        if self._use_nmake:
            if self._full_version < "1.1.0":
                self.cpp_info.libs = ["ssleay32", "libeay32"]
            else:
                if self.settings.build_type == "Debug":
                    self.cpp_info.libs = ['libssld', 'libcryptod']
                else:
                    self.cpp_info.libs = ['libssl', 'libcrypto']
        else:
            self.cpp_info.libs = ["ssl", "crypto"]
        if self.settings.os == "Windows":
            self.cpp_info.system_libs.extend(["crypt32", "msi", "ws2_32", "advapi32", "user32", "gdi32"])
        elif self.settings.os == "Linux":
            self.cpp_info.system_libs.extend(["dl", "pthread"])

