#include "ElfFile.h"
#include <cstdio>
#include <cstdlib>


extern "C"
int LLVMFuzzerTestOneInput(uint8_t *buf, size_t len) {
 ElfFile::CreateFromBuffer("INMEMORY", buf, len);
 return 0;
}
