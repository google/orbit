
Overview
========

**krabsetw** is a C++ library that simplifies interacting with ETW. It allows for any number of traces and providers to be enabled and for client code to register for event notifications from these traces.

**krabsetw** also provides code to simplify parsing generic event data into strongly typed data types.

**Microsoft.O365.Security.Native.ETW** is a C++ CLI (.NET) wrapper around **krabsetw**. It provides the same functionality as **krabsetw** to .NET applications and is used in production by the Office 365 Security team. It's affectionately referred to as **Lobsters**.

Examples & Documentation
========

* An [ETW Primer](docs/EtwPrimer.md).
* Simple examples can be found in the `examples` folder.
* Please refer to [KrabsExample.md](docs/KrabsExample.md) and [LobstersExample.md](docs/LobstersExample.md) for detailed examples.
* SampleKrabsCSharpExe is a non-trivial example demonstrating how to manage the trace objects.
* [Using Message Analyzer to find new ETW event sources.](docs/UsingMessageAnalyzerToFindETWSources.md)

Important Notes
==============
* `krabsetw` and `Microsoft.O365.Security.Native.ETW` only support x64. No effort has been made to support x86.
* `krabsetw` and `Microsoft.O365.Security.Native.ETW` are only supported on Windows 7 or Windows 2008R2 machines and above.
* Throwing exceptions in the event handler callback or krabsetw or Microsoft.O365.Security.Native.ETW will cause the trace to stop processing events.
* The call to "start" on the trace object is blocking so thread management may be necessary.
* The Visual Studio solution is krabs\krabs.sln.
* When building a native code binary using the `krabsetw` package, please refer to the [compilation readme](krabs/README.md) for notes about the `TYPEASSERT` and `NDEBUG` compilation flags.

NuGet Packages
==============
NuGet packages are available both for the krabsetw C++ headers and the Microsoft.O365.Security.Native.ETW .NET library:
* https://www.nuget.org/packages/Microsoft.O365.Security.Native.ETW/
* https://www.nuget.org/packages/Microsoft.O365.Security.Native.ETW.Debug/ (for development - provides type asserts)
* https://www.nuget.org/packages/Microsoft.O365.Security.Krabsetw/

For verifying the .NET binaries, you can use the following command:
`sn -T Microsoft.O365.Security.Native.ETW.dll`

The expected output is:
```
Microsoft (R) .NET Framework Strong Name Utility  Version 4.0.30319.0
Copyright (c) Microsoft Corporation.  All rights reserved.

Public key token is 31bf3856ad364e35
```

Community & Contact
==============
Please feel free to file issues through GitHub for bugs and feature requests and we'll respond to them as quickly as we're able.
