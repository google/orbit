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

#include <string.h>
#include <list>
#include <algorithm>
#include <stdexcept>
#include "parse.h"
#include "nt-headers.h"
#include "to_string.h"
#include <string>

#include "../../OrbitCore/Log.h"
#include "../../OrbitCore/Pdb.h"

using namespace std;

struct section {
  string                sectionName;
  ::uint64_t            sectionBase;
  bounded_buffer        *sectionData;
  image_section_header  sec;
};

struct importent {
  VA      addr;
  string  symbolName;
  string  moduleName;
};

struct exportent {
  VA      addr;
  ::uint32_t  symRVA;
  string  symbolName;
  string  moduleName;
};

struct reloc {
  VA          shiftedAddr;
  reloc_type  type;
};

struct parsed_pe_internal {
  list<section>   secs;
  list<resource>  rsrcs;
  list<importent> imports;
  list<reloc>     relocs;
  list<exportent> exports;
};

::uint32_t err = 0;
std::string err_loc;

static const char *pe_err_str[] = {
  "None",
  "Out of memory",
  "Invalid header",
  "Invalid section",
  "Invalid resource",
  "Unable to get section for VA",
  "Unable to read data",
  "Unable to open",
  "Unable to stat",
  "Bad magic"
};

int GetPEErr() {
  return err;
}

string GetPEErrString() {
  return pe_err_str[err];
}

string GetPEErrLoc() {
  return err_loc;
}

static bool readCString(const bounded_buffer &buffer, ::uint32_t off,
    string &result)
{
  if (off < buffer.bufLen) {
    ::uint8_t *p = buffer.buf;
    ::uint32_t n = buffer.bufLen;
    ::uint8_t *b = p + off;
    ::uint8_t *x = std::find(b, p + n, 0);
    if (x == p + n)
      return false;
    result.insert(result.end(), b, x);
    return true;
  } else {
    return false;
  }
}

bool getSecForVA(list<section> &secs, VA v, section &sec) {
  for(list<section>::iterator it = secs.begin(), e = secs.end();
      it != e;
      ++it)
  {
    section s = *it;
  
    ::uint64_t  low = s.sectionBase;
    ::uint64_t  high = low + s.sec.Misc.VirtualSize;

    if(v >= low && v < high) {
      sec = s;
      return true;
    }
  }

  return false;
}

void IterRsrc(parsed_pe *pe, iterRsrc cb, void *cbd) {
  parsed_pe_internal *pint = pe->internal;

  for(list<resource>::iterator rit = pint->rsrcs.begin(), e = pint->rsrcs.end();
      rit != e;
      ++rit)
  {
    resource r = *rit;
    if(cb(cbd, r) != 0) {
      break;
    }
  }

  return;
}

bool parse_resource_id(bounded_buffer *data, ::uint32_t id, string &result) {
  ::uint8_t c;
  ::uint16_t len;

  if (readWord(data, id, len) == false)
    return false;
  id += 2;
  for (::uint32_t i = 0; i < uint32_t(len * 2); i++) {
    if(readByte(data, id + i, c) == false)
      return false;
    result.push_back((char) c);
  }
  return true;
}

bool parse_resource_table(bounded_buffer *sectionData, ::uint32_t o, ::uint32_t virtaddr, ::uint32_t depth, resource_dir_entry *dirent, list<resource> &rsrcs) {
  ::uint16_t i = 0;
  resource_dir_table rdt;

  if (!sectionData)
    return false;

  READ_DWORD(sectionData, o, rdt, Characteristics);
  READ_DWORD(sectionData, o, rdt, TimeDateStamp);
  READ_WORD(sectionData, o, rdt, MajorVersion);
  READ_WORD(sectionData, o, rdt, MinorVersion);
  READ_WORD(sectionData, o, rdt, NameEntries);
  READ_WORD(sectionData, o, rdt, IDEntries);

  o += sizeof(resource_dir_table);

  if (!rdt.NameEntries && !rdt.IDEntries)
    return true; // This is not a hard error. It does happen.

  for (i = 0; i < rdt.NameEntries + rdt.IDEntries; i++) {
    resource_dir_entry *rde = dirent;
    if (!dirent) {
      rde = new resource_dir_entry;
    }

    READ_DWORD_PTR(sectionData, o, rde, ID);
    READ_DWORD_PTR(sectionData, o, rde, RVA);

    o += sizeof(resource_dir_entry_sz);

    if (depth == 0) {
      rde->type = rde->ID;
      if (i < rdt.NameEntries) {
        if (parse_resource_id(sectionData, rde->ID & 0x0FFFFFFF, rde->type_str) == false)
          return false;
      }
    } else if (depth == 1) {
      rde->name = rde->ID;
      if (i < rdt.NameEntries) {
        if (parse_resource_id(sectionData, rde->ID & 0x0FFFFFFF, rde->name_str) == false)
          return false;
      }
    } else if (depth == 2) {
      rde->lang = rde->ID;
      if (i < rdt.NameEntries) {
        if (parse_resource_id(sectionData, rde->ID & 0x0FFFFFFF, rde->lang_str) == false)
          return false;
      }
    }

    // High bit 0 = RVA to RDT.
    // High bit 1 = RVA to RDE.
    if (rde->RVA & 0x80000000) {
      if (parse_resource_table(sectionData, rde->RVA & 0x0FFFFFFF, virtaddr, depth + 1, rde, rsrcs) == false)
        return false;
    } else {
      resource_dat_entry rdat;

     /*
      * This one is using rde->RVA as an offset.
      *
      * This is because we don't want to set o because we have to keep the
      * original value when we are done parsing this resource data entry.
      * We could store the original o value and reset it when we are done,
      * but meh.
      */
      READ_DWORD(sectionData, rde->RVA, rdat, RVA);
      READ_DWORD(sectionData, rde->RVA, rdat, size);
      READ_DWORD(sectionData, rde->RVA, rdat, codepage);
      READ_DWORD(sectionData, rde->RVA, rdat, reserved);

      resource rsrc = {};

      rsrc.type_str = rde->type_str;
      rsrc.name_str = rde->name_str;
      rsrc.lang_str = rde->lang_str;
      rsrc.type = rde->type;
      rsrc.name = rde->name;
      rsrc.lang = rde->lang;
      rsrc.codepage = rdat.codepage;
      rsrc.RVA = rdat.RVA;
      rsrc.size = rdat.size;

      // The start address is (RVA - section virtual address).
      uint32_t start = rdat.RVA - virtaddr;
      /*
       * Some binaries (particularly packed) will have invalid addresses here.
       * If those happen, return a zero length buffer.
       * If the start is valid, try to get the data and if that fails return
       * a zero length buffer.
       */
      if (start > rdat.RVA)
        rsrc.buf = splitBuffer(sectionData, 0, 0);
      else {
        rsrc.buf = splitBuffer(sectionData, start, start + rdat.size);
        if (!rsrc.buf)
          rsrc.buf = splitBuffer(sectionData, 0, 0);
      }

      /* If we can't get even a zero length buffer, something is very wrong. */
      if (!rsrc.buf)
        return false;

      rsrcs.push_back(rsrc);
    }

    if (depth == 0)
      rde->type_str.clear();
    else if (depth == 1)
      rde->name_str.clear();
    else if (depth == 2)
      rde->lang_str.clear();
  }

  return true;
}

bool getResources(bounded_buffer *b, bounded_buffer *fileBegin, list<section> secs, list<resource> &rsrcs) {

  if (!b)
    return false;

  for (list<section>::iterator sit = secs.begin(), e = secs.end(); sit != e; ++sit) {
    section s = *sit;
    if (s.sectionName != ".rsrc")
      continue;

    if (parse_resource_table(s.sectionData, 0, s.sec.VirtualAddress, 0, NULL, rsrcs) == false)
      return false;

    break; // Because there should only be one .rsrc
  }

  return true;
}

bool getSections( bounded_buffer  *b, 
                  bounded_buffer  *fileBegin,
                  nt_header_32    &nthdr, 
                  list<section>   &secs) {
  if(b == NULL) {
    return false;
  }

  //get each of the sections...
  for(::uint32_t i = 0; i < nthdr.FileHeader.NumberOfSections; i++) {
    image_section_header  curSec;
    
    ::uint32_t  o = i*sizeof(image_section_header);
    for(::uint32_t k = 0; k < NT_SHORT_NAME_LEN; k++) {
      if(readByte(b, o+k, curSec.Name[k]) == false) {
        return false;
      }
    }

    READ_DWORD(b, o, curSec, Misc.VirtualSize);
    READ_DWORD(b, o, curSec, VirtualAddress);
    READ_DWORD(b, o, curSec, SizeOfRawData);
    READ_DWORD(b, o, curSec, PointerToRawData);
    READ_DWORD(b, o, curSec, PointerToRelocations);
    READ_DWORD(b, o, curSec, PointerToLinenumbers);
    READ_WORD(b, o, curSec, NumberOfRelocations);
    READ_WORD(b, o, curSec, NumberOfLinenumbers);
    READ_DWORD(b, o, curSec, Characteristics);

    //now we have the section header information, so fill in a section 
    //object appropriately
    section thisSec;
    for(::uint32_t i = 0; i < NT_SHORT_NAME_LEN; i++) {
      ::uint8_t c = curSec.Name[i];
      if(c == 0) {
        break;
      }

      thisSec.sectionName.push_back((char)c);
    }

    if (nthdr.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
      thisSec.sectionBase = nthdr.OptionalHeader.ImageBase + curSec.VirtualAddress;
    } else if (nthdr.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
      thisSec.sectionBase = nthdr.OptionalHeader64.ImageBase + curSec.VirtualAddress;
    } else {
      PE_ERR(PEERR_MAGIC);
    }

    thisSec.sec = curSec;
    ::uint32_t  lowOff = curSec.PointerToRawData;
    ::uint32_t  highOff = lowOff+curSec.SizeOfRawData;
    thisSec.sectionData = splitBuffer(fileBegin, lowOff, highOff);
    
    secs.push_back(thisSec);
  }

  return true;
}

bool readOptionalHeader(bounded_buffer *b, optional_header_32 &header) {
  READ_WORD(b, 0, header, Magic);

  READ_BYTE(b, 0, header, MajorLinkerVersion);
  READ_BYTE(b, 0, header, MinorLinkerVersion);
  READ_DWORD(b, 0, header, SizeOfCode);
  READ_DWORD(b, 0, header, SizeOfInitializedData);
  READ_DWORD(b, 0, header, SizeOfUninitializedData);
  READ_DWORD(b, 0, header, AddressOfEntryPoint);
  READ_DWORD(b, 0, header, BaseOfCode);
  READ_DWORD(b, 0, header, BaseOfData);
  READ_DWORD(b, 0, header, ImageBase);
  READ_DWORD(b, 0, header, SectionAlignment);
  READ_DWORD(b, 0, header, FileAlignment);
  READ_WORD(b, 0, header, MajorOperatingSystemVersion);
  READ_WORD(b, 0, header, MinorOperatingSystemVersion);
  READ_WORD(b, 0, header, MajorImageVersion);
  READ_WORD(b, 0, header, MinorImageVersion);
  READ_WORD(b, 0, header, MajorSubsystemVersion);
  READ_WORD(b, 0, header, MinorSubsystemVersion);
  READ_DWORD(b, 0, header, Win32VersionValue);
  READ_DWORD(b, 0, header, SizeOfImage);
  READ_DWORD(b, 0, header, SizeOfHeaders);
  READ_DWORD(b, 0, header, CheckSum);
  READ_WORD(b, 0, header, Subsystem);
  READ_WORD(b, 0, header, DllCharacteristics);
  READ_DWORD(b, 0, header, SizeOfStackReserve);
  READ_DWORD(b, 0, header, SizeOfStackCommit);
  READ_DWORD(b, 0, header, SizeOfHeapReserve);
  READ_DWORD(b, 0, header, SizeOfHeapCommit);
  READ_DWORD(b, 0, header, LoaderFlags);
  READ_DWORD(b, 0, header, NumberOfRvaAndSizes);

  if (header.NumberOfRvaAndSizes > NUM_DIR_ENTRIES) {
    header.NumberOfRvaAndSizes = NUM_DIR_ENTRIES;
  }

  for(::uint32_t i = 0; i < header.NumberOfRvaAndSizes; i++) {
    ::uint32_t  c = (i*sizeof(data_directory));
    c+= uint32_t(_offset(optional_header_32, DataDirectory[0]));
    ::uint32_t  o;

    o = c + _offset(data_directory, VirtualAddress);
    if(readDword(b, o, header.DataDirectory[i].VirtualAddress) == false) {
      return false;
    }

    o = c + _offset(data_directory, Size);
    if(readDword(b, o, header.DataDirectory[i].Size) == false) {
      return false;
    }
  }

  return true;
}

bool readOptionalHeader64(bounded_buffer *b, optional_header_64 &header) {
  READ_WORD(b, 0, header, Magic);

  READ_BYTE(b, 0, header, MajorLinkerVersion);
  READ_BYTE(b, 0, header, MinorLinkerVersion);
  READ_DWORD(b, 0, header, SizeOfCode);
  READ_DWORD(b, 0, header, SizeOfInitializedData);
  READ_DWORD(b, 0, header, SizeOfUninitializedData);
  READ_DWORD(b, 0, header, AddressOfEntryPoint);
  READ_DWORD(b, 0, header, BaseOfCode);
  READ_QWORD(b, 0, header, ImageBase);
  READ_DWORD(b, 0, header, SectionAlignment);
  READ_DWORD(b, 0, header, FileAlignment);
  READ_WORD(b, 0, header, MajorOperatingSystemVersion);
  READ_WORD(b, 0, header, MinorOperatingSystemVersion);
  READ_WORD(b, 0, header, MajorImageVersion);
  READ_WORD(b, 0, header, MinorImageVersion);
  READ_WORD(b, 0, header, MajorSubsystemVersion);
  READ_WORD(b, 0, header, MinorSubsystemVersion);
  READ_DWORD(b, 0, header, Win32VersionValue);
  READ_DWORD(b, 0, header, SizeOfImage);
  READ_DWORD(b, 0, header, SizeOfHeaders);
  READ_DWORD(b, 0, header, CheckSum);
  READ_WORD(b, 0, header, Subsystem);
  READ_WORD(b, 0, header, DllCharacteristics);
  READ_QWORD(b, 0, header, SizeOfStackReserve);
  READ_QWORD(b, 0, header, SizeOfStackCommit);
  READ_QWORD(b, 0, header, SizeOfHeapReserve);
  READ_QWORD(b, 0, header, SizeOfHeapCommit);
  READ_DWORD(b, 0, header, LoaderFlags);
  READ_DWORD(b, 0, header, NumberOfRvaAndSizes);

  if (header.NumberOfRvaAndSizes > NUM_DIR_ENTRIES) {
    header.NumberOfRvaAndSizes = NUM_DIR_ENTRIES;
  }

  for(::uint32_t i = 0; i < header.NumberOfRvaAndSizes; i++) {
    ::uint32_t  c = (i*sizeof(data_directory));
    c += uint32_t(_offset(optional_header_64, DataDirectory[0]));
    ::uint32_t  o;

    o = c + _offset(data_directory, VirtualAddress);
    if(readDword(b, o, header.DataDirectory[i].VirtualAddress) == false) {
      return false;
    }

    o = c + _offset(data_directory, Size);
    if(readDword(b, o, header.DataDirectory[i].Size) == false) {
      return false;
    }
  }

  return true;
}

bool readFileHeader(bounded_buffer *b, file_header &header) {
  READ_WORD(b, 0, header, Machine);
  READ_WORD(b, 0, header, NumberOfSections);
  READ_DWORD(b, 0, header, TimeDateStamp);
  READ_DWORD(b, 0, header, PointerToSymbolTable);
  READ_DWORD(b, 0, header, NumberOfSymbols);
  READ_WORD(b, 0, header, SizeOfOptionalHeader);
  READ_WORD(b, 0, header, Characteristics);

  return true;
}

bool readNtHeader(bounded_buffer *b, nt_header_32 &header) {
  if(b == NULL) {
    return false;
  }

  ::uint32_t  pe_magic;
  ::uint32_t  curOffset =0;
  if(readDword(b, curOffset, pe_magic) == false || pe_magic != NT_MAGIC) {
    PE_ERR(PEERR_READ);
    return false;
  }

  header.Signature = pe_magic;
  bounded_buffer  *fhb = 
    splitBuffer(b, _offset(nt_header_32, FileHeader), b->bufLen);
  
  if(fhb == NULL) {
    PE_ERR(PEERR_MEM);
    return false;
  }

  if(readFileHeader(fhb, header.FileHeader) == false) {
    deleteBuffer(fhb);
    return false;
  }

  /*
   * The buffer is split using the OptionalHeader offset, even if it turns
   * out to be a PE32+. The start of the buffer is at the same spot in the
   * buffer regardless.
   */
  bounded_buffer *ohb = 
    splitBuffer(b, _offset(nt_header_32, OptionalHeader), b->bufLen);

  if(ohb == NULL) {
    deleteBuffer(fhb);
    PE_ERR(PEERR_MEM);
    return false;
  }

  /*
   * Read the Magic to determine if it is 32 or 64.
   */
  if (readWord(ohb, 0, header.OptionalMagic) == false) {
    PE_ERR(PEERR_READ);
    if(ohb != NULL)
        deleteBuffer(ohb);
    deleteBuffer(fhb);
    return false;
  }
  if (header.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
    if(readOptionalHeader(ohb, header.OptionalHeader) == false) {
      deleteBuffer(ohb);
      deleteBuffer(fhb);
      return false;
    }
  } else if (header.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
    if(readOptionalHeader64(ohb, header.OptionalHeader64) == false) {
      deleteBuffer(ohb);
      deleteBuffer(fhb);
      return false;
    }
  } else {
    PE_ERR(PEERR_MAGIC);
    deleteBuffer(ohb);
    deleteBuffer(fhb);
    return false;
  }

  deleteBuffer(ohb);
  deleteBuffer(fhb);

  return true;
}

bool getHeader(bounded_buffer *file, pe_header &p, bounded_buffer *&rem) {
  if(file == NULL) {
    return false;
  }

  //start by reading MZ
  ::uint16_t  tmp = 0;
  ::uint32_t  curOffset = 0;
  if(readWord(file, curOffset, tmp) == false) {
    PE_ERR(PEERR_READ);
    return false;
  }
  if(tmp != MZ_MAGIC) {
    PE_ERR(PEERR_MAGIC);
    return false;
  }

  //read the offset to the NT headers
  ::uint32_t  offset;
  if(readDword(file, _offset(dos_header, e_lfanew), offset) == false) {
    PE_ERR(PEERR_READ);
    return false;
  }
  curOffset += offset; 

  //now, we can read out the fields of the NT headers
  bounded_buffer  *ntBuf = splitBuffer(file, curOffset, file->bufLen);

  if(readNtHeader(ntBuf, p.nt) == false) {
    // err is set by readNtHeader
    if(ntBuf != NULL)
      deleteBuffer(ntBuf);
    return false;
  }

  /*
   * Need to determine if this is a PE32 or PE32+ binary and use the
   # correct size.
   */
  ::uint32_t rem_size;
  if (p.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
    // signature + file_header + optional_header_32
    rem_size = sizeof(::uint32_t) + sizeof(file_header) + sizeof(optional_header_32);
  } else if (p.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
    // signature + file_header + optional_header_64
    rem_size = sizeof(::uint32_t) + sizeof(file_header) + sizeof(optional_header_64);
  } else {
    PE_ERR(PEERR_MAGIC);
    deleteBuffer(ntBuf);
    return false;
  }

  //update 'rem' to point to the space after the header
  rem = splitBuffer(ntBuf, rem_size, ntBuf->bufLen);
  deleteBuffer(ntBuf);

  return true;
}

parsed_pe *ParsePEFromFile(const char *filePath) {
  //first, create a new parsed_pe structure
  parsed_pe *p = new parsed_pe();

  if(p == NULL) {
    PE_ERR(PEERR_MEM);
    return NULL;
  }

  //make a new buffer object to hold just our file data 
  p->fileBuffer = readFileToFileBuffer(filePath);

  if(p->fileBuffer == NULL) {
    delete p;
    // err is set by readFileToFileBuffer
    return NULL;
  }

  p->internal = new parsed_pe_internal();

  if(p->internal == NULL) {
    deleteBuffer(p->fileBuffer);
    delete p;
    PE_ERR(PEERR_MEM);
    return NULL;
  }

  //get header information
  bounded_buffer  *remaining = NULL;
  if(getHeader(p->fileBuffer, p->peHeader, remaining) == false) {
    deleteBuffer(p->fileBuffer);
    delete p;
    // err is set by getHeader
    return NULL;
  }

  bounded_buffer  *file = p->fileBuffer;
  if(getSections(remaining, file, p->peHeader.nt, p->internal->secs) == false) {
    deleteBuffer(remaining);
    deleteBuffer(p->fileBuffer);
    delete p;
    PE_ERR(PEERR_SECT);
    return NULL;
  }

  if(getResources(remaining, file, p->internal->secs, p->internal->rsrcs) == false) {
    deleteBuffer(remaining);
    deleteBuffer(p->fileBuffer);
    delete p;
    PE_ERR(PEERR_RESC);
    return NULL;
  }

  //get exports
  data_directory exportDir;
  if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
    exportDir = p->peHeader.nt.OptionalHeader.DataDirectory[DIR_EXPORT];
  } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
    exportDir = p->peHeader.nt.OptionalHeader64.DataDirectory[DIR_EXPORT];
  } else {
    deleteBuffer(remaining);
    deleteBuffer(p->fileBuffer);
    delete p;
    PE_ERR(PEERR_MAGIC);
    return NULL;
  }

  if(exportDir.Size != 0) {
    section s;
    VA addr;
    if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
      addr = exportDir.VirtualAddress + p->peHeader.nt.OptionalHeader.ImageBase;
    } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
      addr = exportDir.VirtualAddress + p->peHeader.nt.OptionalHeader64.ImageBase;
    } else {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_MAGIC);
      return NULL;
    }

    if(getSecForVA(p->internal->secs, addr, s) == false) {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_SECTVA);
      return NULL;
    }

    ::uint32_t  rvaofft = ::uint32_t( addr - s.sectionBase );

    //get the name of this module
    ::uint32_t  nameRva;
    if(readDword( s.sectionData,
                  rvaofft+_offset(export_dir_table, NameRVA),
                  nameRva) == false) 
    {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_READ);
      return NULL;
    }

    VA nameVA;
    if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
      nameVA = nameRva + p->peHeader.nt.OptionalHeader.ImageBase;
    } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
      nameVA = nameRva + p->peHeader.nt.OptionalHeader64.ImageBase;
    } else {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_MAGIC);
      return NULL;
    }

    section nameSec;
    if(getSecForVA(p->internal->secs, nameVA, nameSec) == false) {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_SECTVA);
      return NULL;
    }

    ::uint32_t  nameOff = uint32_t(nameVA - nameSec.sectionBase);
    string      modName;
    if (readCString(*nameSec.sectionData, nameOff, modName) == false) {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_READ);
        return NULL;
    }

    //now, get all the named export symbols
    ::uint32_t  numNames;
    if(readDword( s.sectionData,
                  rvaofft+_offset(export_dir_table, NumberOfNamePointers),
                  numNames) == false)
    {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_READ);
      return NULL;
    }

    if(numNames > 0) {
      //get the names section
      ::uint32_t  namesRVA;
      if(readDword( s.sectionData,
                    rvaofft+_offset(export_dir_table, NamePointerRVA),
                    namesRVA) == false) 
      {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_READ);
        return NULL;
      }

      VA namesVA;
      if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
        namesVA = namesRVA + p->peHeader.nt.OptionalHeader.ImageBase;
      } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
        namesVA = namesRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
      } else {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_MAGIC);
        return NULL;
      }

      section     namesSec;
      if(getSecForVA(p->internal->secs, namesVA, namesSec) == false) {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_SECTVA);
        return NULL;
      }

      ::uint32_t  namesOff = uint32_t(namesVA - namesSec.sectionBase);

      //get the EAT section
      ::uint32_t  eatRVA;
      if(readDword( s.sectionData,
                    rvaofft+_offset(export_dir_table, ExportAddressTableRVA),
                    eatRVA) == false)
      {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_READ);
        return NULL;
      }

      VA eatVA;
      if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
        eatVA = eatRVA + p->peHeader.nt.OptionalHeader.ImageBase;
      } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
        eatVA = eatRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
      } else {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_MAGIC);
        return NULL;
      }

      section     eatSec;
      if(getSecForVA(p->internal->secs, eatVA, eatSec) == false) {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_SECTVA);
        return NULL;
      }

      ::uint32_t  eatOff = uint32_t(eatVA - eatSec.sectionBase);

      //get the ordinal base 
      ::uint32_t  ordinalBase;
      if(readDword( s.sectionData,
                    rvaofft+_offset(export_dir_table, OrdinalBase),
                    ordinalBase) == false)
      {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_READ);
        return NULL;
      }

      //get the ordinal table
      ::uint32_t  ordinalTableRVA;
      if(readDword( s.sectionData,
                    rvaofft+_offset(export_dir_table, OrdinalTableRVA),
                    ordinalTableRVA) == false)

      {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_READ);
        return NULL;
      }

      VA ordinalTableVA;
      if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
        ordinalTableVA = ordinalTableRVA + p->peHeader.nt.OptionalHeader.ImageBase;
      } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
        ordinalTableVA = ordinalTableRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
      } else {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_MAGIC);
        return NULL;
      }

      section     ordinalTableSec;
      if(getSecForVA(p->internal->secs, ordinalTableVA, ordinalTableSec) == false) {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_SECTVA);
        return NULL;
      }

      ::uint32_t  ordinalOff = uint32_t( ordinalTableVA - ordinalTableSec.sectionBase );

      for(::uint32_t  i = 0; i < numNames; i++) {
        ::uint32_t  curNameRVA;
        if(readDword( namesSec.sectionData,
                      namesOff+(i*sizeof(::uint32_t)),
                      curNameRVA) == false)
        {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_READ);
          return NULL;
        }
 
        VA curNameVA;
        if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
          curNameVA = curNameRVA + p->peHeader.nt.OptionalHeader.ImageBase;
        } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
          curNameVA = curNameRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
        } else {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_MAGIC);
          return NULL;
        }

        section     curNameSec;

        if(getSecForVA(p->internal->secs, curNameVA, curNameSec) == false) {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_SECTVA);
          return NULL;
        }

        ::uint32_t  curNameOff = uint32_t( curNameVA - curNameSec.sectionBase );
        string      symName;
        ::uint8_t   d;

        do {
          if(readByte(curNameSec.sectionData, curNameOff, d) == false) {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_READ);
            return NULL;
          }

          if(d == 0) {
            break;
          }

          symName.push_back(d);
          curNameOff++;
        }while(true);

        //now, for this i, look it up in the ExportOrdinalTable
        ::uint16_t  ordinal;
        if(readWord(ordinalTableSec.sectionData, 
                    ordinalOff+(i*sizeof(uint16_t)), 
                    ordinal) == false) 
        {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_READ);
          return NULL;
        }

        //::uint32_t  eatIdx = ordinal - ordinalBase;
        ::uint32_t  eatIdx = (ordinal*sizeof(uint32_t));

        ::uint32_t  symRVA;
        if(readDword(eatSec.sectionData, eatOff+eatIdx, symRVA) == false) {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_READ);
          return NULL;
        }

        bool  isForwarded = 
          ((symRVA >= exportDir.VirtualAddress) && 
          (symRVA < exportDir.VirtualAddress+exportDir.Size));
        
        if(isForwarded == false) {
          ::uint32_t symVA;
          if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
            symVA = symRVA + p->peHeader.nt.OptionalHeader.ImageBase;
          } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
            symVA = symRVA + uint32_t(p->peHeader.nt.OptionalHeader64.ImageBase);
          } else {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_MAGIC);
            return NULL;
          }

          exportent a;

          a.addr = symVA;
          a.symRVA = symRVA;
          a.symbolName = symName;
          a.moduleName = modName;
          p->internal->exports.push_back(a);
        }
      }
    }
  }

  //get relocations, if exist
  data_directory relocDir;
  if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
    relocDir = p->peHeader.nt.OptionalHeader.DataDirectory[DIR_BASERELOC];
  } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
    relocDir = p->peHeader.nt.OptionalHeader64.DataDirectory[DIR_BASERELOC];
  } else {
    deleteBuffer(remaining);
    deleteBuffer(p->fileBuffer);
    delete p;
    PE_ERR(PEERR_MAGIC);
    return NULL;
  }

  if(relocDir.Size != 0) {
    section d;
    VA vaAddr;
    if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
      vaAddr = relocDir.VirtualAddress + p->peHeader.nt.OptionalHeader.ImageBase;
    } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
      vaAddr = relocDir.VirtualAddress + p->peHeader.nt.OptionalHeader64.ImageBase;
    } else {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_MAGIC);
      return NULL;
    }

    if(getSecForVA(p->internal->secs, vaAddr, d) == false) {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_SECTVA);
      return NULL;
    }

    ::uint32_t  rvaofft = uint32_t( vaAddr - d.sectionBase );
    ::uint32_t  pageRva;
    ::uint32_t  blockSize;

    if(readDword( d.sectionData, 
                  rvaofft+_offset(reloc_block, PageRVA), 
                  pageRva) == false)
    {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_READ);
      return NULL;
    }
   
    if(readDword( d.sectionData, 
                  rvaofft+_offset(reloc_block, BlockSize), 
                  blockSize) == false)
    {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_READ);
      return NULL;
    }

    //iter over all of the blocks
    ::uint32_t  blockCount = blockSize/sizeof(::uint16_t);

    rvaofft += sizeof(reloc_block);

    while(blockCount != 0) {
      ::uint16_t  block;
      ::uint8_t   type;
      ::uint16_t  offset;

      if(readWord(d.sectionData, rvaofft, block) == false) {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_READ);
        return NULL;
      }

      //mask out the type and assign
      type = block >> 12;
      //mask out the offset and assign
      offset = block & ~0xf000;

      //produce the VA of the relocation
      ::uint32_t relocVA;
      if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
        relocVA = pageRva + offset + p->peHeader.nt.OptionalHeader.ImageBase;
      } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
        relocVA = pageRva + offset + uint32_t( p->peHeader.nt.OptionalHeader64.ImageBase );
      } else {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_MAGIC);
        return NULL;
      }

      //store in our list
      reloc r;

      r.shiftedAddr = relocVA;
      r.type = (reloc_type)type;
      p->internal->relocs.push_back(r);

      blockCount--;
      rvaofft += sizeof(::uint16_t);
    }
  }
   
  //get imports
  data_directory importDir;
  if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
    importDir = p->peHeader.nt.OptionalHeader.DataDirectory[DIR_IMPORT];
  } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
    importDir = p->peHeader.nt.OptionalHeader64.DataDirectory[DIR_IMPORT];
  } else {
    deleteBuffer(remaining);
    deleteBuffer(p->fileBuffer);
    delete p;
    PE_ERR(PEERR_MAGIC);
    return NULL;
  }

  if(importDir.Size != 0) {
    //get section for the RVA in importDir
    section c;
    VA addr;
    if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
      addr = importDir.VirtualAddress + p->peHeader.nt.OptionalHeader.ImageBase;
    } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
      addr = importDir.VirtualAddress + p->peHeader.nt.OptionalHeader64.ImageBase;
    } else {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_MAGIC);
      return NULL;
    }

    if(getSecForVA(p->internal->secs, addr, c) == false) {
      deleteBuffer(remaining);
      deleteBuffer(p->fileBuffer);
      delete p;
      PE_ERR(PEERR_READ);
      return NULL;
    }

    //get import directory from this section
    ::uint32_t  offt = uint32_t( addr - c.sectionBase );
    do {
      //read each directory entry out
      import_dir_entry  curEnt;

      READ_DWORD_NULL(c.sectionData, offt, curEnt, LookupTableRVA);
      READ_DWORD_NULL(c.sectionData, offt, curEnt, TimeStamp);
      READ_DWORD_NULL(c.sectionData, offt, curEnt, ForwarderChain);
      READ_DWORD_NULL(c.sectionData, offt, curEnt, NameRVA);
      READ_DWORD_NULL(c.sectionData, offt, curEnt, AddressRVA);

      //are all the fields in curEnt null? then we break
      if( curEnt.LookupTableRVA == 0 && 
          curEnt.NameRVA == 0 &&
          curEnt.AddressRVA == 0) {
        break;
      }

      //then, try and get the name of this particular module...
      VA name;
      if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
        name = curEnt.NameRVA + p->peHeader.nt.OptionalHeader.ImageBase;
      } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
        name = curEnt.NameRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
      } else {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_MAGIC);
        return NULL;
      }

      section nameSec;
      if(getSecForVA(p->internal->secs, name, nameSec) == false) {
        PE_ERR(PEERR_SECTVA);
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        return NULL;
      }

      ::uint32_t  nameOff = uint32_t( name - nameSec.sectionBase );
      string      modName;
      if (readCString(*nameSec.sectionData, nameOff, modName) == false ) {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_READ);
          return NULL;
      }
      
      //TODO:
      //boost::to_upper(modName);

      //then, try and get all of the sub-symbols
      VA lookupVA;
      if(curEnt.LookupTableRVA != 0) { 
        if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
          lookupVA = curEnt.LookupTableRVA + p->peHeader.nt.OptionalHeader.ImageBase;
        } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
          lookupVA = curEnt.LookupTableRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
        } else {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_MAGIC);
          return NULL;
        }
      } else if(curEnt.AddressRVA != 0 ) {
        if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
          lookupVA = curEnt.AddressRVA + p->peHeader.nt.OptionalHeader.ImageBase;
        } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
          lookupVA = curEnt.AddressRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
        } else {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_MAGIC);
          return NULL;
        }
      }

      section lookupSec;
      if(getSecForVA(p->internal->secs, lookupVA, lookupSec) == false) {
        deleteBuffer(remaining);
        deleteBuffer(p->fileBuffer);
        delete p;
        PE_ERR(PEERR_SECTVA);
        return NULL;
      }
      
      ::uint32_t  lookupOff = uint32_t( lookupVA - lookupSec.sectionBase );
      ::uint32_t  offInTable = 0;
      do {
        VA          valVA = 0;
        ::uint8_t   ord = 0;
        ::uint16_t  oval = 0;
        ::uint32_t  val32 = 0;
        ::uint64_t  val64 = 0;
        if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
          if(readDword(lookupSec.sectionData, lookupOff, val32) == false) {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_READ);
            return NULL;
          }
          if(val32 == 0) {
            break;
          }
          ord = (val32 >> 31);
          oval = (val32 & ~0xFFFF0000);
          valVA = val32 + p->peHeader.nt.OptionalHeader.ImageBase;
        } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
          if(readQword(lookupSec.sectionData, lookupOff, val64) == false) {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_READ);
            return NULL;
          }
          if(val64 == 0) {
            break;
          }
          ord = (val64 >> 63);
          oval = (val64 & ~0xFFFF0000);
          valVA = val64 + p->peHeader.nt.OptionalHeader64.ImageBase;
        } else {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_MAGIC);
          return NULL;
        }

        if(ord == 0) {
          //import by name
          string  symName;
          section symNameSec;

          if(getSecForVA(p->internal->secs, valVA, symNameSec) == false) {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_SECTVA);
            return NULL;
          }
          
          ::uint32_t  nameOff = uint32_t( valVA - symNameSec.sectionBase );
          nameOff += sizeof(::uint16_t);
          do {
            ::uint8_t d;

            if(readByte(symNameSec.sectionData, nameOff, d) == false) {
              deleteBuffer(remaining);
              deleteBuffer(p->fileBuffer);
              delete p;
              PE_ERR(PEERR_READ);
              return NULL;
            }
            
            if(d == 0) {
              break;
            }

            symName.push_back(d);
            nameOff++;
          } while(true);

          //okay now we know the pair... add it
          importent ent;

          if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
            ent.addr = offInTable + curEnt.AddressRVA + p->peHeader.nt.OptionalHeader.ImageBase;
          } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
            ent.addr = offInTable + curEnt.AddressRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
          } else {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_MAGIC);
            return NULL;
          }

          ent.symbolName = symName;
          ent.moduleName = modName;
          p->internal->imports.push_back(ent);
        } else {
          string      symName = 
            "ORDINAL_" + modName + "_" + to_string<uint32_t>(oval, dec);
          
          importent ent;

          if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
            ent.addr = offInTable + curEnt.AddressRVA + p->peHeader.nt.OptionalHeader.ImageBase;
          } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
            ent.addr = offInTable + curEnt.AddressRVA + p->peHeader.nt.OptionalHeader64.ImageBase;
          } else {
            deleteBuffer(remaining);
            deleteBuffer(p->fileBuffer);
            delete p;
            PE_ERR(PEERR_MAGIC);
            return NULL;
          }
          
          ent.symbolName = symName;
          ent.moduleName = modName;

          p->internal->imports.push_back(ent);
        }
        
        if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC) {
          lookupOff += sizeof(::uint32_t);
          offInTable += sizeof(::uint32_t);
        } else if (p->peHeader.nt.OptionalMagic == NT_OPTIONAL_64_MAGIC) {
          lookupOff += sizeof(::uint64_t);
          offInTable += sizeof(::uint64_t);
        } else {
          deleteBuffer(remaining);
          deleteBuffer(p->fileBuffer);
          delete p;
          PE_ERR(PEERR_MAGIC);
          return NULL;
        }
      } while(true);

      offt += sizeof(import_dir_entry);
    } while(true);
  }

  deleteBuffer(remaining);

  return p;
}

void DestructParsedPE(parsed_pe *p) {
  if(p == NULL) {
    return;
  }

  deleteBuffer(p->fileBuffer);
  delete p->internal;
  delete p;
  return;
}

//iterate over the imports by VA and string
void IterImpVAString(parsed_pe *pe, iterVAStr cb, void *cbd) {
  list<importent> &l = pe->internal->imports;

  for(list<importent>::iterator it = l.begin(), e = l.end(); it != e; ++it) {
    importent i = *it;
    if(cb(cbd, i.addr, i.moduleName, i.symbolName) != 0) {
      break;
    }
  }

  return;
}

//iterate over relocations in the PE file
void IterRelocs(parsed_pe *pe, iterReloc cb, void *cbd) {
  list<reloc> &l = pe->internal->relocs;

  for(list<reloc>::iterator it = l.begin(), e = l.end(); it != e; ++it) {
    reloc r = *it;
    if(cb(cbd, r.shiftedAddr, r.type) != 0) {
      break;
    }
  }

  return;
}

//iterate over the exports by VA
void IterExpVA(parsed_pe *pe, iterExp cb, void *cbd) {
  list<exportent> &l = pe->internal->exports;

  for(list<exportent>::iterator it = l.begin(), e = l.end(); it != e; ++it) {
    exportent i = *it;

    if(cb(cbd, i.addr, i.moduleName, i.symbolName)) {
      break;
    }
  }

  return;
}

//iterate over sections
void IterSec(parsed_pe *pe, iterSec cb, void *cbd) {
  parsed_pe_internal  *pint = pe->internal;

  for(list<section>::iterator sit = pint->secs.begin(), e = pint->secs.end();
      sit != e;
      ++sit)
  {
    section s = *sit;
    if(cb(cbd, s.sectionBase, s.sectionName, s.sec, s.sectionData) != 0) {
      break;
    }
  }

  return;
}

bool ReadByteAtVA(parsed_pe *pe, VA v, ::uint8_t &b) {
  //find this VA in a section
  section s;

  if(getSecForVA(pe->internal->secs, v, s) == false) {
    PE_ERR(PEERR_SECTVA);
    return false;
  }

  ::uint32_t  off = uint32_t( v - s.sectionBase );

  return readByte(s.sectionData, off, b);
}

bool GetEntryPoint(parsed_pe *pe, VA &v) {

  if(pe != NULL) {
    nt_header_32  *nthdr = &pe->peHeader.nt;

    if (nthdr->OptionalMagic == NT_OPTIONAL_32_MAGIC) {
      v = nthdr->OptionalHeader.AddressOfEntryPoint + nthdr->OptionalHeader.ImageBase;
    } else if (nthdr->OptionalMagic == NT_OPTIONAL_64_MAGIC) {
      v = nthdr->OptionalHeader64.AddressOfEntryPoint + nthdr->OptionalHeader64.ImageBase;
    } else {
      PE_ERR(PEERR_MAGIC);
      return false;
    }

    return true;
  }

  return false;
}

bool Is32Bit(parsed_pe *pe)
{
    return pe->peHeader.nt.OptionalMagic == NT_OPTIONAL_32_MAGIC;
}

void ParseDll(const char* a_FileName)
{
    parsed_pe *pe = ParsePEFromFile(a_FileName);

    if (pe != NULL)
    {
        list<exportent> &l = pe->internal->exports;

        for (list<exportent>::iterator it = l.begin(), e = l.end(); it != e; ++it)
        {
            exportent i = *it;
            Function func;
            func.m_Name = s2ws(i.symbolName);
            func.m_PrettyName = func.m_Name;
            func.m_Address = i.symRVA;
            func.m_Module = s2ws(i.moduleName);
            func.m_Pdb = GPdbDbg.get();
            GPdbDbg->AddFunction( func );
        }

        DestructParsedPE(pe);
    }
    else
    {
        ORBIT_LOG("Could not parse dll");
    }
}
