#ifndef BITMAP_H_
#define BITMAP_H_

#include "types.h"

static inline BOOL GetBit(uint32_t *bitmap, unsigned bit) {
  return bitmap[bit / 32] & (1 << (bit % 32));
}

static inline void SetBit(uint32_t *bitmap, unsigned bit) {
  bitmap[bit / 32] |= (1 << (bit % 32));
}

static inline void UnsetBit(uint32_t *bitmap, unsigned bit) {
  bitmap[bit / 32] &= ~(1 << (bit % 32));
}

int FindFirstUnsetBit(uint32_t *bitmap, unsigned count);

#endif // BITMAP_H_
