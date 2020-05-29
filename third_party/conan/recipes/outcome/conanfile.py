# from https://github.com/ned14/outcome/blob/develop/conan/conanfile.py
from conans import ConanFile, tools

class OutcomeConan(ConanFile):
    name = "Outcome"
    version = "3dae433e"
    license = "Apache-2.0"
    description = "Provides very lightweight outcome<T> and result<T>"
    repo_url = "https://github.com/ned14/outcome"

    def source(self):
        file_url = "https://raw.githubusercontent.com/ned14/outcome/{}/".format(self.version)
        tools.download(file_url + "single-header/outcome.hpp", filename="outcome.hpp")
        tools.download(file_url + "Licence.txt", filename="LICENCE")
        header_file = tools.load("outcome.hpp")
        header_file += "\n"
        header_file += "#ifndef OUTCOME_NAMESPACE_INCGUARD_\n"
        header_file += "#define OUTCOME_NAMESPACE_INCGUARD_\n"
        header_file += "namespace outcome = OUTCOME_V2_NAMESPACE;\n"
        header_file += "#endif /* OUTCOME_NAMESPACE_INCGUARD_ */\n"
        tools.save("outcome.hpp", header_file)

    def package(self):
        self.copy("outcome.hpp", dst="include")
        self.copy("LICENCE", dst="licenses")

    def package_id(self):
        self.info.header_only()