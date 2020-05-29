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

#ifndef _NT_HEADERS
#define _NT_HEADERS

//#define _offset(t, f) ((uint32_t)(ptrdiff_t)&(((t*)0)->f))
#define _offset(t, f) offsetof(std::remove_reference<t>::type, f)

//need to pack these structure definitions

//some constant definitions
const uint16_t MZ_MAGIC = 0x5A4D;
const uint32_t NT_MAGIC = 0x00004550;
const uint16_t NUM_DIR_ENTRIES = 16;
const uint16_t NT_OPTIONAL_32_MAGIC = 0x10B;
const uint16_t NT_OPTIONAL_64_MAGIC = 0x20B;
const uint16_t NT_SHORT_NAME_LEN = 8;
const uint16_t DIR_EXPORT = 0;
const uint16_t DIR_IMPORT = 1;
const uint16_t DIR_RESOURCE = 2;
const uint16_t DIR_EXCEPTION = 3;
const uint16_t DIR_SECURITY = 4;
const uint16_t DIR_BASERELOC = 5;
const uint16_t DIR_DEBUG = 6;
const uint16_t DIR_ARCHITECTURE = 7;
const uint16_t DIR_GLOBALPTR = 8;
const uint16_t DIR_TLS = 9;
const uint16_t DIR_LOAD_CONFIG = 10;
const uint16_t DIR_BOUND_IMPORT = 11;
const uint16_t DIR_IAT = 12;
const uint16_t DIR_DELAY_IMPORT = 13;
const uint16_t DIR_COM_DESCRIPTOR = 14;

const uint32_t IMAGE_SCN_TYPE_NO_PAD = 0x00000008;
const uint32_t IMAGE_SCN_CNT_CODE = 0x00000020;
const uint32_t IMAGE_SCN_CNT_INITIALIZED_DATA = 0x00000040; 
const uint32_t IMAGE_SCN_CNT_UNINITIALIZED_DATA = 0x00000080;
const uint32_t IMAGE_SCN_LNK_OTHER = 0x00000100;
const uint32_t IMAGE_SCN_LNK_INFO = 0x00000200;
const uint32_t IMAGE_SCN_LNK_REMOVE = 0x00000800;
const uint32_t IMAGE_SCN_LNK_COMDAT = 0x00001000;
const uint32_t IMAGE_SCN_NO_DEFER_SPEC_EXC = 0x00004000;
const uint32_t IMAGE_SCN_GPREL = 0x00008000;
const uint32_t IMAGE_SCN_MEM_FARDATA = 0x00008000;
const uint32_t IMAGE_SCN_MEM_PURGEABLE = 0x00020000;
const uint32_t IMAGE_SCN_MEM_16BIT = 0x00020000;
const uint32_t IMAGE_SCN_MEM_LOCKED = 0x00040000;
const uint32_t IMAGE_SCN_MEM_PRELOAD = 0x00080000;
const uint32_t IMAGE_SCN_ALIGN_1BYTES = 0x00100000;
const uint32_t IMAGE_SCN_ALIGN_2BYTES = 0x00200000;
const uint32_t IMAGE_SCN_ALIGN_4BYTES = 0x00300000;
const uint32_t IMAGE_SCN_ALIGN_8BYTES = 0x00400000;
const uint32_t IMAGE_SCN_ALIGN_16BYTES = 0x00500000;
const uint32_t IMAGE_SCN_ALIGN_32BYTES = 0x00600000;
const uint32_t IMAGE_SCN_ALIGN_64BYTES = 0x00700000;
const uint32_t IMAGE_SCN_ALIGN_128BYTES = 0x00800000;
const uint32_t IMAGE_SCN_ALIGN_256BYTES = 0x00900000;
const uint32_t IMAGE_SCN_ALIGN_512BYTES = 0x00A00000;
const uint32_t IMAGE_SCN_ALIGN_1024BYTES = 0x00B00000;
const uint32_t IMAGE_SCN_ALIGN_2048BYTES = 0x00C00000;
const uint32_t IMAGE_SCN_ALIGN_4096BYTES = 0x00D00000;
const uint32_t IMAGE_SCN_ALIGN_8192BYTES = 0x00E00000;
const uint32_t IMAGE_SCN_ALIGN_MASK = 0x00F00000;
const uint32_t IMAGE_SCN_LNK_NRELOC_OVFL = 0x01000000;
const uint32_t IMAGE_SCN_MEM_DISCARDABLE = 0x02000000;
const uint32_t IMAGE_SCN_MEM_NOT_CACHED = 0x04000000;
const uint32_t IMAGE_SCN_MEM_NOT_PAGED = 0x08000000;
const uint32_t IMAGE_SCN_MEM_SHARED = 0x10000000;
const uint32_t IMAGE_SCN_MEM_EXECUTE = 0x20000000;
const uint32_t IMAGE_SCN_MEM_READ = 0x40000000;
const uint32_t IMAGE_SCN_MEM_WRITE = 0x80000000;

struct dos_header {
    uint16_t   e_magic;           
    uint16_t   e_cblp;            
    uint16_t   e_cp;              
    uint16_t   e_crlc;            
    uint16_t   e_cparhdr;         
    uint16_t   e_minalloc;        
    uint16_t   e_maxalloc;        
    uint16_t   e_ss;              
    uint16_t   e_sp;              
    uint16_t   e_csum;            
    uint16_t   e_ip;              
    uint16_t   e_cs;              
    uint16_t   e_lfarlc; 
    uint16_t   e_ovno;            
    uint16_t   e_res[4];          
    uint16_t   e_oemid;           
    uint16_t   e_oeminfo; 
    uint16_t   e_res2[10];        
    uint32_t   e_lfanew;          
};

struct file_header {
    uint16_t   Machine;
    uint16_t   NumberOfSections;
    uint32_t   TimeDateStamp;
    uint32_t   PointerToSymbolTable;
    uint32_t   NumberOfSymbols;
    uint16_t   SizeOfOptionalHeader;
    uint16_t   Characteristics;
};

struct data_directory {
  uint32_t VirtualAddress;
  uint32_t Size;
};

struct optional_header_32 {
  uint16_t   Magic;
  uint8_t    MajorLinkerVersion;
  uint8_t    MinorLinkerVersion;
  uint32_t   SizeOfCode;
  uint32_t   SizeOfInitializedData;
  uint32_t   SizeOfUninitializedData;
  uint32_t   AddressOfEntryPoint;
  uint32_t   BaseOfCode;
  uint32_t   BaseOfData;
  uint32_t   ImageBase;
  uint32_t   SectionAlignment;
  uint32_t   FileAlignment;
  uint16_t   MajorOperatingSystemVersion;
  uint16_t   MinorOperatingSystemVersion;
  uint16_t   MajorImageVersion;
  uint16_t   MinorImageVersion;
  uint16_t   MajorSubsystemVersion;
  uint16_t   MinorSubsystemVersion;
  uint32_t   Win32VersionValue;
  uint32_t   SizeOfImage;
  uint32_t   SizeOfHeaders;
  uint32_t   CheckSum;
  uint16_t   Subsystem;
  uint16_t   DllCharacteristics;
  uint32_t   SizeOfStackReserve;
  uint32_t   SizeOfStackCommit;
  uint32_t   SizeOfHeapReserve;
  uint32_t   SizeOfHeapCommit;
  uint32_t   LoaderFlags;
  uint32_t   NumberOfRvaAndSizes;
  data_directory    DataDirectory[NUM_DIR_ENTRIES];
};

/*
 * This is used for PE32+ binaries. It is similar to optional_header_32
 * except some fields don't exist here (BaseOfData), and others are bigger.
 */
struct optional_header_64 {
  uint16_t   Magic;
  uint8_t    MajorLinkerVersion;
  uint8_t    MinorLinkerVersion;
  uint32_t   SizeOfCode;
  uint32_t   SizeOfInitializedData;
  uint32_t   SizeOfUninitializedData;
  uint32_t   AddressOfEntryPoint;
  uint32_t   BaseOfCode;
  uint64_t   ImageBase;
  uint32_t   SectionAlignment;
  uint32_t   FileAlignment;
  uint16_t   MajorOperatingSystemVersion;
  uint16_t   MinorOperatingSystemVersion;
  uint16_t   MajorImageVersion;
  uint16_t   MinorImageVersion;
  uint16_t   MajorSubsystemVersion;
  uint16_t   MinorSubsystemVersion;
  uint32_t   Win32VersionValue;
  uint32_t   SizeOfImage;
  uint32_t   SizeOfHeaders;
  uint32_t   CheckSum;
  uint16_t   Subsystem;
  uint16_t   DllCharacteristics;
  uint64_t   SizeOfStackReserve;
  uint64_t   SizeOfStackCommit;
  uint64_t   SizeOfHeapReserve;
  uint64_t   SizeOfHeapCommit;
  uint32_t   LoaderFlags;
  uint32_t   NumberOfRvaAndSizes;
  data_directory    DataDirectory[NUM_DIR_ENTRIES];
};

struct nt_header_32 {
  uint32_t     Signature;
  file_header         FileHeader;
  optional_header_32  OptionalHeader;
  optional_header_64  OptionalHeader64;
  uint16_t     OptionalMagic;
};

/*
 * This structure is only used to know how far to move the offset
 * when parsing resources. The data is stored in a resource_dir_entry
 * struct but that also has extra information used in the parsing which
 * causes the size to be inaccurate.
 */
struct resource_dir_entry_sz {
  uint32_t ID;
  uint32_t RVA;
};

struct resource_dir_entry {
  inline resource_dir_entry(void)
      : ID(0),
        RVA(0),
        type(0),
        name(0),
        lang(0) {}

  uint32_t ID;
  uint32_t RVA;
  uint32_t type;
  uint32_t name;
  uint32_t lang;
  std::string     type_str;
  std::string     name_str;
  std::string     lang_str;
};

struct resource_dir_table {
  uint32_t Characteristics;
  uint32_t TimeDateStamp;
  uint16_t MajorVersion;
  uint16_t MinorVersion;
  uint16_t NameEntries;
  uint16_t IDEntries;
};

struct resource_dat_entry {
  uint32_t RVA;
  uint32_t size;
  uint32_t codepage;
  uint32_t reserved;
};

struct image_section_header {
    uint8_t    Name[NT_SHORT_NAME_LEN];
    union {
            uint32_t   PhysicalAddress;
            uint32_t   VirtualSize;
    } Misc;
    uint32_t   VirtualAddress;
    uint32_t   SizeOfRawData;
    uint32_t   PointerToRawData;
    uint32_t   PointerToRelocations;
    uint32_t   PointerToLinenumbers;
    uint16_t   NumberOfRelocations;
    uint16_t   NumberOfLinenumbers;
    uint32_t   Characteristics;
};

struct import_dir_entry {
  uint32_t LookupTableRVA;
  uint32_t TimeStamp;
  uint32_t ForwarderChain;
  uint32_t NameRVA;
  uint32_t AddressRVA;
};

struct export_dir_table {
  uint32_t ExportFlags;
  uint32_t TimeDateStamp;
  uint16_t MajorVersion;
  uint16_t MinorVersion;
  uint32_t NameRVA;
  uint32_t OrdinalBase;
  uint32_t AddressTableEntries;
  uint32_t NumberOfNamePointers;
  uint32_t ExportAddressTableRVA;
  uint32_t NamePointerRVA;
  uint32_t OrdinalTableRVA;
};

enum reloc_type {
  ABSOLUTE = 0,
  HIGH = 1,
  LOW = 2,
  HIGHLOW = 3,
  HIGHADJ = 4,
  MIPS_JMPADDR = 5,
  MIPS_JMPADDR16 = 9,
  IA64_IMM64 = 9,
  DIR64 = 10
};

struct reloc_block {
  uint32_t PageRVA;
  uint32_t BlockSize;
};

#endif
