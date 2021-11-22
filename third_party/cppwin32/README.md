# The C++ Windows SDK projection

C++/Win32 (cppwin32) is a standard C++ language projection for Windows SDK APIs. True, the Windows SDK already releases headers that can be consumed from C++, but those headers have some severe historical and compatiblity constraints that hamper C++ developers that wish to enjoy modern language features. Goals include:
- Better compliance with C++ standards, particularly C++17 and newer.
- Eliminating the use of macros for constants, enumerations, and functions, and instead using C++ language constructs that participate in the type system and respect scope.
- Paving the way towards a Windows SDK that can be used with C++ modules.

Another goal under investigation and discussion is to converge this projection with the [C++/WinRT projection](https://github.com/microsoft/cppwinrt/).

## How it works

This projection is similar to [C++/WinRT](https://github.com/microsoft/cppwinrt/) in that it generates projection header files from metadata. Instead of using Windows Runtime metadata, it uses metadata produced by the [Win32 Metadata Project](https://github.com/microsoft/win32metadata/).

This project is still in the early stages and will continue to grow and evolve. Watch for a Nuget package soon.

## Contributing

This project welcomes contributions and suggestions.  Most contributions require you to agree to a
Contributor License Agreement (CLA) declaring that you have the right to, and actually do, grant us
the rights to use your contribution. For details, visit https://cla.opensource.microsoft.com.

When you submit a pull request, a CLA bot will automatically determine whether you need to provide
a CLA and decorate the PR appropriately (e.g., status check, comment). Simply follow the instructions
provided by the bot. You will only need to do this once across all repos using our CLA.

This project has adopted the [Microsoft Open Source Code of Conduct](https://opensource.microsoft.com/codeofconduct/).
For more information see the [Code of Conduct FAQ](https://opensource.microsoft.com/codeofconduct/faq/) or
contact [opencode@microsoft.com](mailto:opencode@microsoft.com) with any additional questions or comments.

## Trademarks

This project may contain trademarks or logos for projects, products, or services. Authorized use of Microsoft 
trademarks or logos is subject to and must follow 
[Microsoft's Trademark & Brand Guidelines](https://www.microsoft.com/en-us/legal/intellectualproperty/trademarks/usage/general).
Use of Microsoft trademarks or logos in modified versions of this project must not cause confusion or imply Microsoft sponsorship.
Any use of third-party trademarks or logos are subject to those third-party's policies.
