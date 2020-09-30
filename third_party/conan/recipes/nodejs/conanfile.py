import os
from conans import ConanFile, tools
from conans.errors import ConanInvalidConfiguration


class NodejsInstallerConan(ConanFile):
    name = "nodejs"
    description = "nodejs binaries for use in recipes"
    topics = ("conan", "node", "nodejs")
    url = "https://github.com/conan-io/conan-center-index"
    homepage = "https://nodejs.org"
    license = "MIT"
    settings = {"os_build": ["Windows", "Linux", "Macos"], "arch_build": ["x86_64"]}
    no_copy_source = True

    @property
    def _source_subfolder(self):
        return os.path.join(self.source_folder, "source_subfolder")

    def source(self):
        os_name = {"Windows": "-win", "Macos": "-darwin", "Linux": "-linux"}
        for data in self.conan_data["sources"][self.version]:
            url = data["url"]

            if os_name[str(self.settings.os_build)] in url:
                filename = url[url.rfind("/")+1:]
                tools.download(url, filename)
                tools.check_sha256(filename, data["sha256"])
                tools.unzip(filename)
                os.rename(filename[:filename.rfind("x64")+3], self._source_subfolder)

    def package(self):
        self.copy(pattern="LICENSE", dst="licenses", src=self._source_subfolder)
        self.copy(pattern="*", src=os.path.join(self._source_subfolder, "bin"), dst="bin")
        self.copy(pattern="node.exe", src=self._source_subfolder, dst="bin")
        self.copy(pattern="npm", src=os.path.join(self._source_subfolder, "bin"), dst="bin", symlinks=True)
        self.copy(pattern="npx", src=os.path.join(self._source_subfolder, "bin"), dst="bin", symlinks=True)
        self.copy(pattern="npm.cmd", src=self._source_subfolder, dst="bin")
        self.copy(pattern="npx.cmd", src=self._source_subfolder, dst="bin")
        self.copy(pattern="*", src=os.path.join(self._source_subfolder, "include"), dst="include")
        self.copy(pattern="*", src=os.path.join(self._source_subfolder, "lib"), dst="lib")
        self.copy(pattern="*", src=os.path.join(self._source_subfolder, "node_modules"),
                               dst=os.path.join("bin", "node_modules"))

    def package_info(self):
        bin_dir = os.path.join(self.package_folder, "bin")
        self.output.info('Appending PATH environment variable: {}'.format(bin_dir))
        self.env_info.PATH.append(bin_dir)
