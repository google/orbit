# A short introduction on how to use the `grpc` conan package

1. Add `grpc/1.27.3@orbitdeps/stable` to the requirements of your project by
   editing your project's `conanfile.py` and add the dependency to the 
   `requires` attribute. If your conanfile defines a `def requirements(self):`
   function add a `self.requires('grpc/1.27.3@orbitdeps/stable')` call to this
   function. The latter is the case for Orbit.

2. Add `grpc_codegen/1.27.3@orbitdeps/stable` to the build requirements of your
   project by editing your project's `conanfile.py` and add the build dependency
   to the `build_requires` attribute. Example:
   `build_requires = 'grpc_codegen/1.27.3@orbitdeps/stable'`.

3. Add some `find_package` calls to your top-level `CMakeLists.txt`-file:
   ```cmake
   find_package(protobuf CONFIG REQUIRED)
   find_package(gRPC CONFIG REQUIRED)
   ```

4. Add your `.proto`-files as `PRVIATE` source files to your cmake-targets and
   call `grpc_helper` on this target. Example:
   ```cmake
   add_executable(test)
   target_sources(test PRIVATE main.cpp test.proto)
   grpc_helper(test)
   ```
   The `grpc_helper`-call will properly call the protobuf-compiler with the
   grpc-cpp-plugin enabled and will feed the resulting C++-files back into the
   cmake-target.


## Limitations

Currently the protobuf-compiler include path is hard-coded to the directory
containing the main `.proto` file.  It would be no problem to inherit the
include directories from the `cmake`-target. Ping me, if necessary.

## Example

Check out `examples/helloworld/` for a small self-sustaining conan- and cmake-
based `grpc`-example.
