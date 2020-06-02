/*
The MIT License (MIT)

Copyright (c) 2013 Andrew Ruef

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#include <iostream>
#include <sstream>
#include <time.h>
#include "parse.h"

using namespace std;

int printExps(void *N, VA funcAddr, std::string &mod, std::string &func) {
  cout << "EXP: ";
  cout << mod;
  cout << "!";
  cout << func;
  cout << ": 0x";
  cout << to_string<uint64_t>(funcAddr, hex);
  cout << endl;
  return 0;
}

int printImports(void *N, VA impAddr, string &modName, string &symName) {
  cout << "0x" << to_string<uint64_t>(impAddr, hex);
  cout << " " << modName << "!" << symName;
  cout << endl;
  return 0;
}

int printRelocs(void *N, VA relocAddr, reloc_type type) {
  cout << "TYPE: ";
  switch(type) {
    case ABSOLUTE:
      cout << "ABSOLUTE";
      break;
    case HIGH:
      cout << "HIGH";
      break;
    case LOW:
      cout << "LOW";
      break;
    case HIGHLOW:
      cout << "HIGHLOW";
      break;
    case HIGHADJ:
      cout << "HIGHADJ";
      break;
    case MIPS_JMPADDR:
      cout << "MIPS_JMPADDR";
      break;
    case MIPS_JMPADDR16:
      cout << "MIPS_JMPADD16";
      break;
    case DIR64:
      cout << "DIR64";
      break;
  }

  cout << " VA: 0x" << to_string<VA>(relocAddr, hex) << endl;

  return 0 ;
}

int printRsrc(void     *N,
              resource r)
{
  if (r.type_str.length())
    cout << "Type (string): " << r.type_str << endl;
  else
    cout << "Type: 0x" << to_string<uint32_t>(r.type, hex) << endl;
  if (r.name_str.length())
    cout << "Name (string): " << r.name_str << endl;
  else
    cout << "Name: 0x" << to_string<uint32_t>(r.name, hex) << endl;
  if (r.lang_str.length())
    cout << "Lang (string): " << r.lang_str << endl;
  else
    cout << "Lang: 0x" << to_string<uint32_t>(r.lang, hex) << endl;
  cout << "Codepage: 0x" << to_string<uint32_t>(r.codepage, hex) << endl;
  cout << "RVA: " << to_string<uint32_t>(r.RVA, dec) << endl;
  cout << "Size: " << to_string<uint32_t>(r.size, dec) << endl;
  return 0;
}

std::string timeStampToHReadble(time_t t)
{
    struct tm buf;
    char str[26];
    localtime_s(&buf, &t);
    asctime_s(str, sizeof str, &buf );
    return std::string(str);
}

int printSecs(void                  *N, 
              VA                    secBase, 
              string                &secName, 
              image_section_header  s,
              bounded_buffer        *data) 
{
  cout << "Sec Name: " << secName << endl;
  cout << "Sec Base: 0x" << to_string<uint64_t>(secBase, hex) << endl;
  if (data)
    cout << "Sec Size: " << to_string<uint64_t>(data->bufLen, dec) << endl;
  else
    cout << "Sec Size: 0" << endl;
  return 0;
}

void Parse(const char* a_FileName)
{
    parsed_pe *p = ParsePEFromFile(a_FileName);

    if (p != NULL)
    {
        // Name
        time_t now = time(NULL);
        cout << "Report generated on : " << timeStampToHReadble(now) << endl;
        std::string bits = Is32Bit(p) ? "32 bit" : "64 bit";
        cout << a_FileName << " [" << bits << "]" << endl;
        cout << "pe timestamp: " << timeStampToHReadble(p->peHeader.nt.FileHeader.TimeDateStamp) << endl;

        //print out some things
#define DUMP_FIELD(x) \
        cout << "" #x << ": 0x"; \
        cout << to_string<uint64_t>(p->peHeader.nt.x, hex) << endl;
#define DUMP_DEC_FIELD(x) \
        cout << "" #x << ": "; \
        cout << to_string<uint64_t>(p->peHeader.nt.x, dec) << endl;

        DUMP_FIELD(Signature);
        DUMP_FIELD(FileHeader.Machine);
        DUMP_FIELD(FileHeader.NumberOfSections);
        DUMP_DEC_FIELD(FileHeader.TimeDateStamp);
        DUMP_FIELD(FileHeader.PointerToSymbolTable);
        DUMP_DEC_FIELD(FileHeader.NumberOfSymbols);
        DUMP_FIELD(FileHeader.SizeOfOptionalHeader);
        DUMP_FIELD(FileHeader.Characteristics);
        if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC)
        {
            DUMP_FIELD(OptionalHeader.Magic);
            DUMP_DEC_FIELD(OptionalHeader.MajorLinkerVersion);
            DUMP_DEC_FIELD(OptionalHeader.MinorLinkerVersion);
            DUMP_FIELD(OptionalHeader.SizeOfCode);
            DUMP_FIELD(OptionalHeader.SizeOfInitializedData);
            DUMP_FIELD(OptionalHeader.SizeOfUninitializedData);
            DUMP_FIELD(OptionalHeader.AddressOfEntryPoint);
            DUMP_FIELD(OptionalHeader.BaseOfCode);
            DUMP_FIELD(OptionalHeader.BaseOfData);
            DUMP_FIELD(OptionalHeader.ImageBase);
            DUMP_FIELD(OptionalHeader.SectionAlignment);
            DUMP_FIELD(OptionalHeader.FileAlignment);
            DUMP_DEC_FIELD(OptionalHeader.MajorOperatingSystemVersion);
            DUMP_DEC_FIELD(OptionalHeader.MinorOperatingSystemVersion);
            DUMP_DEC_FIELD(OptionalHeader.Win32VersionValue);
            DUMP_FIELD(OptionalHeader.SizeOfImage);
            DUMP_FIELD(OptionalHeader.SizeOfHeaders);
            DUMP_FIELD(OptionalHeader.CheckSum);
            DUMP_FIELD(OptionalHeader.Subsystem);
            DUMP_FIELD(OptionalHeader.DllCharacteristics);
            DUMP_FIELD(OptionalHeader.SizeOfStackReserve);
            DUMP_FIELD(OptionalHeader.SizeOfStackCommit);
            DUMP_FIELD(OptionalHeader.SizeOfHeapReserve);
            DUMP_FIELD(OptionalHeader.SizeOfHeapCommit);
            DUMP_FIELD(OptionalHeader.LoaderFlags);
            DUMP_DEC_FIELD(OptionalHeader.NumberOfRvaAndSizes);
        }
        else
        {
            DUMP_FIELD(OptionalHeader64.Magic);
            DUMP_DEC_FIELD(OptionalHeader64.MajorLinkerVersion);
            DUMP_DEC_FIELD(OptionalHeader64.MinorLinkerVersion);
            DUMP_FIELD(OptionalHeader64.SizeOfCode);
            DUMP_FIELD(OptionalHeader64.SizeOfInitializedData);
            DUMP_FIELD(OptionalHeader64.SizeOfUninitializedData);
            DUMP_FIELD(OptionalHeader64.AddressOfEntryPoint);
            DUMP_FIELD(OptionalHeader64.BaseOfCode);
            DUMP_FIELD(OptionalHeader64.ImageBase);
            DUMP_FIELD(OptionalHeader64.SectionAlignment);
            DUMP_FIELD(OptionalHeader64.FileAlignment);
            DUMP_DEC_FIELD(OptionalHeader64.MajorOperatingSystemVersion);
            DUMP_DEC_FIELD(OptionalHeader64.MinorOperatingSystemVersion);
            DUMP_DEC_FIELD(OptionalHeader64.Win32VersionValue);
            DUMP_FIELD(OptionalHeader64.SizeOfImage);
            DUMP_FIELD(OptionalHeader64.SizeOfHeaders);
            DUMP_FIELD(OptionalHeader64.CheckSum);
            DUMP_FIELD(OptionalHeader64.Subsystem);
            DUMP_FIELD(OptionalHeader64.DllCharacteristics);
            DUMP_FIELD(OptionalHeader64.SizeOfStackReserve);
            DUMP_FIELD(OptionalHeader64.SizeOfStackCommit);
            DUMP_FIELD(OptionalHeader64.SizeOfHeapReserve);
            DUMP_FIELD(OptionalHeader64.SizeOfHeapCommit);
            DUMP_FIELD(OptionalHeader64.LoaderFlags);
            DUMP_DEC_FIELD(OptionalHeader64.NumberOfRvaAndSizes);
        }

#undef DUMP_FIELD
#undef DUMP_DEC_FIELD

        cout << "Imports: " << endl;
        IterImpVAString(p, printImports, NULL);
        cout << "Relocations: " << endl;
        IterRelocs(p, printRelocs, NULL);
        cout << "Sections: " << endl;
        IterSec(p, printSecs, NULL);
        cout << "Exports: " << endl;
        IterExpVA(p, printExps, NULL);

        //read the first 8 bytes from the entry point and print them
        VA  entryPoint;
        if (GetEntryPoint(p, entryPoint)) {
            cout << "First 8 bytes from entry point (0x";

            cout << to_string<VA>(entryPoint, hex);
            cout << "):" << endl;
            for (int i = 0; i < 8; i++) {
                ::uint8_t b;
                ReadByteAtVA(p, i + entryPoint, b);
                cout << " 0x" << to_string<uint32_t>(b, hex);
            }

            cout << endl;
        }

        cout << "Resources: " << endl;
        IterRsrc(p, printRsrc, NULL);
        DestructParsedPE(p);
    }
    else
    {
        cout << "Error: " << GetPEErr() << " (" << GetPEErrString() << ")" << endl;
        cout << "Location: " << GetPEErrLoc() << endl;
    }
}
