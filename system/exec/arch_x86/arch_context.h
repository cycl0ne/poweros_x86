#ifndef context_h
#define context_h
#include "types.h"

typedef UINT32 IRQMask;

#define STACK_ITEM_SIZE  4
#define SP_DELTA  (8 + STACK_ITEM_SIZE)

#define context_set(c, _pc, stack, size) \
	do { \
		(c)->pc = (UINT32) (_pc); \
		(c)->sp = ((UINT32) (stack)) + (size) - SP_DELTA; \
		(c)->ebp = 0; \
	} while (0)

#define context_save arch_ctx_save
#define context_restore arch_ctx_restore

typedef struct {
	UINT32 sp;
	UINT32 pc;
	UINT32 ebx;
	UINT32 esi;
	UINT32 edi;
	UINT32 ebp;
	UINT32 ipl;
} __attribute__ ((packed)) Context;

#define OFFSET_SP   0x00
#define OFFSET_PC   0x04
#define OFFSET_EBX  0x08
#define OFFSET_ESI  0x0C
#define OFFSET_EDI  0x10
#define OFFSET_EBP  0x14

#define OFFSET_IPL  0x18
#define OFFSET_TLS  0x18

extern int arch_ctx_save(Context *ctx) __attribute__((returns_twice));
extern void arch_ctx_restore(Context *ctx) __attribute__((noreturn));

#endif
