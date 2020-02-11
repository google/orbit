//-----------------------------------
// Copyright Pierric Gimmig 2013-2017
//-----------------------------------

#include "Disassembler.h"

#include <capstone/capstone.h>
#include <capstone/platform.h>

#define LOGF(format, ...) { \
  std::string log = absl::StrFormat(format, __VA_ARGS__); \
  m_String += s2ws(log); \
}

#define LOG(str) { m_String += s2ws(str); }

//-----------------------------------------------------------------------------
void Disassembler::LogHex(const unsigned char* str, size_t len) {
  const unsigned char* c;

  LOGF("%s", "Code: ");
  for (c = str; c < str + len; c++) {
    LOGF("0x%02x ", *c & 0xff);
  }
  LOGF("%s", "\n");
}

//-----------------------------------------------------------------------------
void Disassembler::Disassemble(const unsigned char* a_MachineCode,
                               size_t a_Size, DWORD64 a_Address,
                               bool a_Is64Bit) {
  std::wstring disAsm;
  csh handle = 0;
  cs_arch arch = CS_ARCH_X86;
  cs_insn* insn = nullptr;
  size_t count = 0;
  cs_err err;
  cs_mode mode = a_Is64Bit ? CS_MODE_64 : CS_MODE_32;

  LOG("\n");
  LOGF("Platform: %s\n",
       a_Is64Bit ? "X86 64 (Intel syntax)" : "X86 32 (Intel syntax)");
  err = cs_open(arch, mode, &handle);
  if (err) {
    LOGF("Failed on cs_open() with error returned: %u\n", err);
    return;
  }

  count = cs_disasm(handle, a_MachineCode, a_Size, a_Address, 0, &insn);

  if (count) {
    size_t j;

    for (j = 0; j < count; j++) {
      LOGF("0x%" PRIx64 ":\t%-12s %s\n", insn[j].address, insn[j].mnemonic,
           insn[j].op_str);

      /*std::string log = Format("0x%" PRIx64 ":\t%-12s %s\n"
          , insn[j].address
          , insn[j].mnemonic
          , insn[j].op_str);
      OutputDebugStringA(log.c_str());*/
    }

    // print out the next offset, after the last insn
    LOGF("0x%" PRIx64 ":\n", insn[j - 1].address + insn[j - 1].size);

    // free memory allocated by cs_disasm()
    cs_free(insn, count);
  } else {
    LOG("****************\n");
    LOG("ERROR: Failed to disasm given code!\n");
  }

  LOG("\n");

  cs_close(&handle);
}
