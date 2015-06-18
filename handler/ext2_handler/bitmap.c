#include "bitmap.h"

int FindFirstUnsetBit(uint32_t *bitmap, unsigned count) {
  for (unsigned i = 0; i < count; ++i) {
    uint32_t dword = bitmap[i];
    if (dword != 0xFFFFFFFF)
      return i * 32 + __builtin_ffs(~dword) - 1;
  }
  return -1;
}
