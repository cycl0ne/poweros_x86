/**
* File: /elf．h
* User: cycl0ne
* Date: 2014-10-22
* Time: 07:49 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef ELF_H
#define ELF_H
#include "types.h"

//standard elf types
typedef  INT32   Elf32_Sword;
typedef UINT32   Elf32_Word;
typedef UINT16   Elf32_Half;
typedef UINT32   Elf32_Addr;
typedef UINT32   Elf32_Off;
typedef UINT8    Elf32_unsigned_char;

//EI_ = elf identifier
#define EI_NIDENT 16

//elf header struct
//eh_ = elf header
typedef struct {
    Elf32_unsigned_char eh_ident[EI_NIDENT];
    Elf32_Half eh_type;
    Elf32_Half eh_machine;
    Elf32_Word eh_version;
    Elf32_Addr eh_entry;  // program entry point
    Elf32_Off  eh_phoff;
    Elf32_Off  eh_shoff;  //file offset at which section header table is found
    Elf32_Word eh_flags;
    Elf32_Half eh_ehsize;
    Elf32_Half eh_phentsize;
    Elf32_Half eh_phnum;
    Elf32_Half eh_shentsize; //one entry in section header table takes this much of bytes
    Elf32_Half eh_shnum;  //total number of section headers that exist in section header table
    Elf32_Half eh_shstrndx;
} Elf32_EHdr;

//these are for index to eh_ident[]
//EI_ = elf identifier
#define	EI_MAG0		0
#define	EI_MAG1		1
#define	EI_MAG2		2
#define	EI_MAG3		3
#define	EI_CLASS	4
#define	EI_DATA		5
#define	EI_VERSION	6

//these are values for eh_ident[EI_MAG0] to eh_ident[EI_MAG3]
#define	ELFMAG0		0x7f
#define	ELFMAG1		'E'
#define	ELFMAG2		'L'
#define	ELFMAG3		'F'

// these are values for eh_ident[EI_CLASS]
#define	ELFCLASSNONE	0
#define	ELFCLASS32	1
#define	ELFCLASS64	2

// these are values for eh_ident[EI_DATA]
#define ELFDATANONE	0
#define ELFDATA2LSB	1
#define ELFDATA2MSB	2

// these are values for eh_ident[EI_VERSION] and eh_version
//EV_ = elf version
#define EV_NONE		0
#define EV_CURRENT	1

// These are values for eh_type, elf file types
//ET_ = elf type
#define ET_NONE   0
#define ET_REL    1
#define ET_EXEC   2
#define ET_DYN    3
//...incomplete

//these are values for eh_machine, target machine types
//EM_ = elf machine
#define EM_NONE		0
#define EM_M32		1
#define EM_SPARC	2
#define EM_386		3
#define EM_68K		4
//...incomplete

//section header struct
//sh_ = section header
typedef struct {
    Elf32_Word	sh_name;

    Elf32_Word	sh_type;   //type could be REL or RELA if a section contains relocation info,
							//else type could be NOBITS, PROGBITS etc.

    Elf32_Word	sh_flags;  //write, alloc, execute etc are flags for a section

    Elf32_Addr	sh_addr;

    Elf32_Off	sh_offset; //in this file, this section starts here

    Elf32_Word	sh_size;   //size of this section

    Elf32_Word	sh_link;   //REL/RELA type section holds here the index to section header table,
							//pointing to associated symbol table section.

							//Symbol table (SYMTAB type) section holds here the index to section header table,
							//pointing to associated string table.

    Elf32_Word	sh_info;   //REL/RELA type section holds here the index to section header table,
							//pointing to target section (on which to apply relocations)

    Elf32_Word	sh_addralign;

    Elf32_Word	sh_entsize; //Symbol table (SYMTAB type) section holds here size of each symbol entry
} Elf32_Shdr;

//these are values for sh_type
//SHT_ = section header type
#define SHT_NULL	0
#define SHT_PROGBITS	1
#define SHT_SYMTAB	2
#define SHT_STRTAB	3
#define SHT_RELA	4
#define SHT_HASH	5
#define SHT_DYNAMIC	6
#define SHT_NOTE	7
#define SHT_NOBITS	8
#define SHT_REL		9
//...incomplete

//these are values for sh_flags
//SHF_ = section header flag
#define SHF_WRITE	0x1
#define SHF_ALLOC	0x2
#define SHF_EXECINSTR	0x4
//...incomplete

//relocation entry struct
//re_ = relocation entry
typedef struct {
    Elf32_Addr	re_offset; //offset in target section where to put new value after relocation calculation
    Elf32_Word	re_info; // this contains type of relocation in 1 byte LSB and symbol index in 3 byte MSB
} Elf32_Rel;

// These are used to break re_info
//RE_ = relocation entry
#define ELF32_RE_SYM(x) ((x) >> 8) //tells us which symbol's address to relocate, index to symbol table
#define ELF32_RE_TYPE(x) ((x) & 0xff) //relocation type R_386_32 (calculate S+A) or R_386_PC32 (calculate S+A-P)

// these are values of ELF32_RE_TYPE
// R_ = relocation
#define R_386_NONE      0
#define R_386_32        1
#define R_386_PC32      2
//...incomplete

//symbol entry struct
//se_ = symbol entry
typedef struct {
    Elf32_Word	se_name; //index to symbol string table (section .strtab) for name of symbol
    Elf32_Addr	se_value; //offset from the beginning of the section (which last field "se_shndx" identifies),
							//where the symbol stays. so this indirectly gives the address of symbol.
							//when last field "se_shndx" is COM, it gives allignment constraint for this symbol
    Elf32_Word	se_size;
    Elf32_unsigned_char	se_info; //contains symbol type in 4bit LSB and bind type in 4bit MSB
    Elf32_unsigned_char	se_other; //never used
    Elf32_Half	se_shndx; //index to section header table; this symbol is defined in whih section
							//(mostly in either .data section or .text section). 0 means external sysmbol
							//if section is COM, it means, this symbol is not yet allocated,
							//(could be allocated in bss)
} Elf32_Sym;


// these are used to break se_info
// SE_ = symbol entry
#define ELF32_SE_BIND(x)		((x) >> 4)
#define ELF32_SE_TYPE(x)		(((UINT32) x) & 0xf)

#endif
