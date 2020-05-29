from conan.packager import ConanMultiPackager
import platform
import os

if __name__ == "__main__":
    builder = ConanMultiPackager(visual_runtimes=["MT", "MD"])

    if platform.system() == "Windows":
        builder.add_common_builds(visual_versions=[12, 13])

    if platform.system() == "Linux":
        for ver in (filter(None,
                           os.getenv("CONAN_GCC_VERSIONS", "").split(","))
                    or ["4.8", "4.9", "5.2", "5.3"]):
            for arch in ["x86", "x86_64"]:
                builder.add({"arch": arch,
                             "build_type": "Release",
                             "compiler": "gcc",
                             "compiler.version": ver}, {})

    if platform.system() == "Darwin":
        for ver in (filter(None,
                           os.getenv("CONAN_APPLE_CLANG_VERSIONS_VERSIONS", "").split(","))
                    or ["5.0", "5.1", "6.0", "6.1", "7.0"]):
            for arch in ["x86", "x86_64"]:
                builder.add({"arch": arch,
                             "build_type": "Release",
                             "compiler": "apple-clang",
                             "compiler.version": ver}, {})
    builder.run()
