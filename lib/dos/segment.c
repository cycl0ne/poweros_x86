/**
 * @file segment.c
 *
 * Segment Functions
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"
#include "elf.h"
#define memcpy(a,b,c) CopyMem(b,a,c)

static INT32 int_ValidateElf(pDOSBase DOSBase, UINT8* image, UINT32 size);
static INT32 int_DoElfRelocation(pDOSBase DOSBase, UINT8 *image, Elf32_Rel *re, Elf32_Shdr *rel_sh);
static INT32 int_GetSymAddr(pDOSBase DOSBase, UINT8 *image, UINT32 se_index, UINT32 *sym_addr, UINT32 symtab_sh_index);
static INT32 int_RelocateElf(pDOSBase DOSBase, UINT8 *image, UINT32 size, UINT8 **pas, UINT32 *entry);

//int load_elf_relocatable(pDOSBase DOSBase, unsigned char *image, UINT32* entry);

static pSegment int_CreateSegment(pDOSBase DOSBase, STRPTR name, APTR entry, APTR memory, INT32 flag)
{
	pSegment segment = NULL;
	STRPTR	segname;

	segment = AllocVec(sizeof(struct Segment), MEMF_CLEAR|MEMF_FAST);
	if (segment)
	{
		Strcpy(&segment->seg_Name, name);
		segment->seg_Node.ln_Name	= segment->seg_Name;
		segment->seg_Node.ln_Pri	= flag; //(flag<0)? -10:10; //Internal Commands are allways higher prio as other
		segment->seg_Node.ln_Type	= NT_SEGMENT;
		segment->seg_Flags	= flag;
		segment->seg_Entry	= entry;
		segment->seg_Memory	= memory;
		segment->seg_Count	= 0;
		InitSemaphore(&segment->seg_Lock);
	}
	return segment;
}

DOSCALL	dos_LockSegList(pDOSBase DOSBase)
{
	ObtainSemaphore(&DOSBase->dos_SegLock);
	return DOSCMD_SUCCESS;
}

DOSCALL	dos_UnLockSegList(pDOSBase DOSBase)
{
	ReleaseSemaphore(&DOSBase->dos_SegLock);
	return DOSCMD_SUCCESS;
}

DOSCALL dos_LockSegment(pDOSBase DOSBase, pSegment seg)
{
	if (seg)
	{
		ObtainSemaphore(&seg->seg_Lock);
		return DOSCMD_SUCCESS;
	}
	return DOSCMD_FAIL;
}

DOSCALL dos_UnLockSegment(pDOSBase DOSBase, pSegment seg)
{
	if (seg)
	{
		ReleaseSemaphore(&seg->seg_Lock);
		return DOSCMD_SUCCESS;
	}
	return DOSCMD_FAIL;
}

pSegment _FindName(pDOSBase DOSBase, pList list, STRPTR name)
{
	pSegment tmp = NULL;
	ForeachNode(list, tmp)
	{
		//KPrintF("[%s], Prio: %d\n",tmp->seg_Node.ln_Name, tmp->seg_Node.ln_Pri);
		if (!Stricmp(tmp->seg_Node.ln_Name, name))
		{
			return tmp;
		}
	}
	return NULL;
}

//TODO: fix parameters! No NEXT, no SYSTEM
pSegment dos_FindSegment(pDOSBase DOSBase, STRPTR name, pSegment next, BOOL system)
{
	pSegment rc;
	ObtainSemaphoreShared(&DOSBase->dos_SegLock);
	rc = _FindName(DOSBase, &DOSBase->dos_SegList, name);
//	KPrintF("seg found: %x N:%x Sys:%d [%s]\n", rc, next, system, name);
	ReleaseSemaphore(&DOSBase->dos_SegLock);
	if (rc == NULL) SetIoErr(ERROR_OBJECT_NOT_FOUND);
	return rc;
}

DOSCALL dos_RemSegment(pDOSBase DOSBase, pSegment segment)
{
	SetIoErr(ERROR_OBJECT_IN_USE);

	// Dont allow Systemprgs to be unloaded (-1, -2 and -999)
	LockSegment(segment);
	if ((segment->seg_Flags < 0) || (segment->seg_Count > 0))
	{
		SetIoErr(ERROR_OBJECT_IN_USE);
		UnLockSegment(segment);
		return DOSCMD_FAIL;
	}

	LockSegList();
	Remove((struct Node *) segment);
	UnLockSegList();
	UnLockSegment(segment);
	SetIoErr(0);
	return DOSCMD_SUCCESS;
}

DOSCALL dos_AddSegment(pDOSBase DOSBase, STRPTR name, pSegment segment, INT32 flag)
{
	if (segment)
	{
		Strcpy(&segment->seg_Name, name);
//		KPrintF("segname: %s, name: %s %d, %d\n", segname, name, segname, segment);
		segment->seg_Node.ln_Pri	= flag; //(flag<0)? -10:10; //Internal Commands are allways higher prio as other
		segment->seg_Flags			= flag;
		segment->seg_Node.ln_Name	= &segment->seg_Name;
		ObtainSemaphore(&DOSBase->dos_SegLock);
		Enqueue((struct List *)&DOSBase->dos_SegList, (struct Node *) segment);
		ReleaseSemaphore(&DOSBase->dos_SegLock);
		return DOSCMD_SUCCESS;
	}	
#if 0
	if (!_FindName(DOSBase, (struct List *)&DOSBase->dos_SegList, name))
	{
		segment = AllocVec(sizeof(struct Segment), MEMF_CLEAR|MEMF_FAST);
		if (segment)
		{
			if (Strlen(name) >0)
			{
				segname = AllocVec(Strlen(name), MEMF_FAST);
				if (segname)
				{
					Strcpy(segname, name);

					segment->seg_Node.ln_Name	= segname;
					segment->seg_Node.ln_Pri	= flag; //(flag<0)? -10:10; //Internal Commands are allways higher prio as other
					segment->seg_Node.ln_Type	= NT_SEGMENT;
					segment->seg_Flags	= flag;
					segment->seg_Entry	= entry;
					segment->seg_Memory	= memory;
					segment->seg_Count	= 0;
					InitSemaphore(&segment->seg_Lock);

					ObtainSemaphore(&DOSBase->dos_SegLock);
					Enqueue((struct List *)&DOSBase->dos_SegList, (struct Node *) segment);
					ReleaseSemaphore(&DOSBase->dos_SegLock);
					return DOSCMD_SUCCESS;
				}
			} else{
				KPrintF("Name illegal: [%s]", name);
			}
			FreeVec(segment);
		}
		KPrintF("Findname found something: %s\n", name);
	}
#endif
	return DOSCMD_FAIL;
}

// Add some Errorcodes.
pSegment dos_LoadSegment(pDOSBase DOSBase, STRPTR name)
{
	pFileLock	lock;
	pFileHandle fh;
	pSegment	ret = NULL;
	UINT32		size= 0;

	lock = Lock(name, SHARED_LOCK);
	if (lock)
	{
		FileInfoBlock	fib;
		if (Examine(lock, &fib))
		{
			UnLock(lock);
			size = fib.fib_Size;
			if (size>0 && fib.fib_DirEntryType <0)
			{
				UINT8* buffer = AllocVec(size, MEMF_FAST|MEMF_CLEAR);
				if (buffer)
				{
//					KPrintF("Allocated %d memory for LoadSegment\n", size);
					//We do a Open here until somone finishes: OpenFromLock();
					fh = Open(name, MODE_OLDFILE);
					if (fh)
					{
						UINT32 cnt = FRead(fh, buffer, size, 1); // 1, size);
//						KPrintF("Read CNT: %d\n", cnt);
						Close(fh);
						if (cnt == 1)
						{
//							KPrintF("Read %d chars\n", size);
							UINT8* 	pas = NULL;
							UINT32 entry;
							if (!int_RelocateElf(DOSBase, buffer, size, &pas, &entry))
							{
								FreeVec(buffer);
//								KPrintF("loadsegname: %s %s\n", name, fib.fib_FileName);
								return int_CreateSegment(DOSBase, fib.fib_FileName, entry, pas, CMD_USER);
							}
							if (pas) FreeVec(pas);
						}
//						KPrintF("CNT != 1\n");
					}
					FreeVec(buffer);
				}
			}
		} else
			UnLock(lock);
		SetIoErr(ERROR_BAD_HUNK);
	} else
		SetIoErr(ERROR_OBJECT_NOT_FOUND);
//	KPrintF("File not found\n");
	return ret;
}

DOSCALL dos_UnloadSegment(pDOSBase DOSBase, pSegment segment)
{
	if (!segment) return DOSCMD_FAIL;
	
	LockSegment(segment);
	if (segment->seg_Count > 0)
	{
//		KPrintF("UnloadSeg: Count != 0\n");
		UnLockSegment(segment);
		return DOSCMD_FAIL;	
	}
	if (segment->seg_Flags < CMD_USER) 
	{
//		KPrintF("UnloadSeg: <CMD_USER\n");
		UnLockSegment(segment);
		return DOSCMD_FAIL;	
	}
//	KPrintF("UnloadSeg: Freeing: %x\n", segment->seg_Memory);
	FreeVec(segment->seg_Memory);
	FreeVec(segment);
	return DOSCMD_SUCCESS;
}


/*
** Internal stuff from srinivas 
**
*/

static INT32 int_RelocateElf(pDOSBase DOSBase, UINT8 *image, UINT32 size, UINT8 **pas, UINT32 *entry)
{
    //eh = elf header
    //sh = section header
    //re = relocation entry
    //pas = process address space

    UINT32 sh_index, re_index, re_size;
    UINT8 *section_start_address;
    Elf32_Rel  *re;
    Elf32_Shdr *sh;
    Elf32_EHdr *eh;
    UINT32 err;

    // validate
	err = int_ValidateElf(DOSBase, image, size);
	if(err != 0) return 1;

	eh = (Elf32_EHdr *)image;

    // Before doing any relocation, find section size and allocate memory for sections
    UINT32 pas_size = 0;
    for(sh_index = 0; sh_index < eh->eh_shnum; sh_index++)
    {
		sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * sh_index);
		if((sh->sh_flags & SHF_ALLOC) == 0) continue;
		pas_size += sh->sh_size + sh->sh_addralign;
		//Printf("sh_size= %d sh_addralign= %d\n", sh->sh_size, sh->sh_addralign);
		//Printf("so pas_size= %d\n", pas_size);
    }

	//allocate memory for process address space
    *pas = AllocVec(pas_size, MEMF_FAST|MEMF_CLEAR);
//	Printf("pas_size: %d bytes\n", pas_size);
//	Printf("pas start: %x pas end: %x\n", *pas, *pas+pas_size);
    if(*pas == NULL)
    {
		KPrintF("No Memory: LoadSegment\n");
        return 1;
    }

	//copy loadable sections from file image to process address space
	UINT8* address = *pas;
    for(sh_index = 0; sh_index < eh->eh_shnum; sh_index++)
    {
        sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * sh_index);
		if((sh->sh_flags & SHF_ALLOC) == 0) continue;

		//Printf("addr= %x\n", address);

		if(sh->sh_addralign > 1)
		{
			//get an aligned address
			section_start_address = address + (sh->sh_addralign - ((UINT32)address % (UINT32)sh->sh_addralign));
		}
		else
		{
			//align value 0 or 1 means, no need of any allignment
			section_start_address = address;
		}

		//Printf("adjustment= %d\n", (sh->sh_addralign - ((UINT32)address % (UINT32)sh->sh_addralign)));
		//Printf("section_start_address= %x\n", section_start_address);

		//take memory chunk for current section and leave rest
		address = section_start_address + sh->sh_size;

		//Printf("next addr= %x\n", address);

        // if .bss
        if(sh->sh_type == SHT_NOBITS)
		{
			//make all bytes zero
			MemSet(section_start_address, 0, sh->sh_size);
		}
		else
		{
			//copy the whole section from file image
			memcpy(section_start_address, (image + sh->sh_offset), (sh->sh_size));
			//CopyMem()
		}

        // save new offset of section into section header back
        // we will use this to find these sections at the time of relocations
        sh->sh_offset = section_start_address - image;
    }

	//Printf("pas before relocation:\n");
	//hexdump(DOSBase, *pas, pas_size);

    //lets do relocation
    // for each section...
    for(sh_index = 0; sh_index < eh->eh_shnum; sh_index++)
    {
        sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * sh_index);

        // if it is a relocation section, find relocation entry size
        if(sh->sh_type != SHT_REL) continue;

        re_size = sizeof(Elf32_Rel); //we handle only this type of relocation, since 32bit elf supports only this

        // for each relocation...
        //total number of relocation entries present in a .rel section is
        //section size / relocation entry size
        for(re_index = 0; re_index < sh->sh_size / re_size; re_index++)
        {
            re = (Elf32_Rel *)(image + sh->sh_offset + re_size * re_index);
            err = int_DoElfRelocation(DOSBase, image, re, sh);
            if(err != 0) return err;
        }
    }

	//Printf("pas after relocation:\n");
	//hexdump(DOSBase, *pas, pas_size);

    // find start of .text and make it the entry point
	(*entry) = 0;
	for(sh_index = 0; sh_index < eh->eh_shnum; sh_index++)
	{
		sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * sh_index);

		if((sh->sh_flags & SHF_EXECINSTR) == 0) continue;

		INT8* section_name;
		Elf32_Off entry_offset;
		Elf32_Shdr *shstr_sh;
		shstr_sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * eh->eh_shstrndx);
		section_name = (INT8*)(image + shstr_sh->sh_offset + sh->sh_name);
		//KPrintF("section name: %s\n", section_name);

		if(Strcmp(section_name, ".text") != 0)
			continue;

		//if we find .text, lets find entry point
		if(eh->eh_entry >= sh->sh_addr)
		{
			//Printf("entry point as per elf file: %x\n", eh->eh_entry);
			entry_offset = eh->eh_entry - sh->sh_addr;
		}
		else
		{
			//KPrintF("wrong entry point\n");
			return 1;
		}

		//finnaly calculate real entry point in pas
        (*entry) = (UINT32)image + sh->sh_offset + entry_offset;
        //Printf("real .text addr: %x\n", (UINT32)image + sh->sh_offset);
		//Printf("real entry point addr: %x\n", (*entry));
		//Printf("few opcodes:\n");
//		for (int i = 0; i < 30; i++)
//		{
//			Printf("%x: %x\n", ((*entry)+i), 0xff & *((char*)((*entry)+i)));
//		}

		break;
	}

	if((*entry) == 0)
	{
		KPrintF("LoadSeg: Can't find section .text, so entry point is unknown\n");
		return 1;
	}
	return 0;
}

static INT32 int_ValidateElf(pDOSBase DOSBase, UINT8* image, UINT32 size)
{
	Elf32_EHdr *eh;

    eh = (Elf32_EHdr *)image;

    if(size < eh->eh_ehsize)
    {
		KPrintF("File corrupted \n");
        return 1;
	}

    if(!(eh->eh_ident[EI_MAG0] == ELFMAG0
                && eh->eh_ident[EI_MAG1] == ELFMAG1
                && eh->eh_ident[EI_MAG2] == ELFMAG2
                && eh->eh_ident[EI_MAG3] == ELFMAG3))
    {
        KPrintF("File is has bad magic value \n");
        return 1;
    }

    if(eh->eh_ident[EI_CLASS] != ELFCLASS32)
    {
        KPrintF("File is not for 32bit machine\n");
        return 1;
    }

    if(eh->eh_ident[EI_DATA] != ELFDATA2LSB)
    {
        KPrintF("File is not little endian\n");
        return 1;
    }

    if(eh->eh_ident[EI_VERSION] != EV_CURRENT)
    {
        KPrintF("File has bad ELF version \n");
        return 1;
    }

    if(eh->eh_type != ET_REL)
    {
        KPrintF("File is not relocatable ELF \n");
        return 1;
    }

    if(eh->eh_machine != EM_386)
    {
        KPrintF("File is not for i386 machine\n");
        return 1;
    }

    if(eh->eh_version != EV_CURRENT)
    {
        KPrintF("File has bad ELF version \n");
        return 1;
    }

	return 0;
}

static INT32 int_DoElfRelocation(pDOSBase DOSBase, UINT8 *image, Elf32_Rel *re, Elf32_Shdr *rel_sh)
{
	//t_ = target
    UINT32 t_sect_start_addr, sym_addr;
    Elf32_Shdr *t_sh;
    Elf32_EHdr *eh;
    UINT32 *where;
    UINT32 err;

	eh = (Elf32_EHdr *)image;

    // get start address of target section
    t_sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * rel_sh->sh_info);
    t_sect_start_addr = (UINT32)image + t_sh->sh_offset;

    // this is where a new value will be inserted after relocation calculation
    where = (UINT32 *)(t_sect_start_addr + re->re_offset);

    // get symbol address
    err = int_GetSymAddr(DOSBase, image, ELF32_RE_SYM(re->re_info), &sym_addr, rel_sh->sh_link);
    if(err != 0) return err;

    switch(ELF32_RE_TYPE(re->re_info))
    {
        // absolute reference
        case R_386_32:
			// S + A
            *where = sym_addr + *where;
            break;

        // PC relative reference
        case R_386_PC32:
			// S + A - P
            *where = sym_addr + *where - (UINT32)where;
            break;

        default:
            KPrintF("unsupported relocation type \n");
            return 1;
    }
    return 0;
}

static INT32 int_GetSymAddr(pDOSBase DOSBase, UINT8 *image, UINT32 se_index, UINT32 *sym_addr, UINT32 symtab_sh_index)
{
	//se = symbol entry

	Elf32_EHdr *eh;
	Elf32_Shdr *symtab_sh, *sh;
	Elf32_Sym *se;
	UINT32 sect_addr;
	//int err;

	eh = (Elf32_EHdr *)image;

	//symbol table section header index should be well within
	//total number of section headers present in elf file.
	if(symtab_sh_index >= eh->eh_shnum)
	{
		KPrintF("bad symbol table section index \n");
		return 1;
	}

	// point to symbol table
	symtab_sh = (Elf32_Shdr *)(image + eh->eh_shoff + eh->eh_shentsize * symtab_sh_index);

	//number of symbols = symbol table size / each symbol entry size
	if(se_index >= symtab_sh->sh_size / sizeof(Elf32_Sym))
	{
		KPrintF("offset into symbol table exceeds symbol table size\n");
		return 1;
	}

	// get symbol entry
	se = (Elf32_Sym *)(image + symtab_sh->sh_offset) + se_index;

	// external symbol
	if(se->se_shndx == 0)
	{
		KPrintF("elf refers to external symbols\n");
		return 1;
	}

	// internal symbol
	else
	{
		sh = (Elf32_Shdr *)(image + eh->eh_shoff +  eh->eh_shentsize * se->se_shndx);
		sect_addr = (UINT32)image + sh->sh_offset;
		*sym_addr = sect_addr + se->se_value ;
	}
	return 0;
}



