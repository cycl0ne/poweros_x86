/**
* File: /assign．c
* User: cycl0ne
* Date: 2014-11-12
* Time: 09:13 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "types.h"

#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#define	VSTRING	"Assign 0.1 (31.10.2014)\n\r"
#define CMDREV  "\0$VER: " VSTRING

#define TEMPLATE    "NAME,TARGET/M,LIST/S,EXISTS/S,DISMOUNT/S,DEFER/S,PATH/S,ADD/S,"\
                    "REMOVE/S,VOLS/S,DIRS/S,DEVICES/S" CMDREV
#define OPT_NAME    0
#define OPT_TARGET  1
#define OPT_LIST    2
#define OPT_EXIST   3
#define OPT_DISMOUNT  4
#define OPT_DEFER   5
#define OPT_PATH    6
#define OPT_ADD     7
#define OPT_REMOVE     8
#define OPT_VOL     9
#define OPT_DIR    10
#define OPT_DEV    11
#define OPT_COUNT  12

#define MAXASNLEN 30  /* Max length for ASSIGN names */

/* Messages - replace for foreign-language versions */
#define MSG_BADDEV      "Invalid device name %s\n"
#define MSG_NODIR       "Can't find %s\n"
#define MSG_INUSE       "Can't cancel %s\n"
#define MSG_IDUNNO      "Can't assign %s\n"
#define MSG_VOLUMES     "Volumes:\n"
#define MSG_ASSIGNS     "\nDirectories:\n"
#define MSG_DEVICES     "\nDevices:\n"
#define MSG_BADLOCK     "Bad lock\n"
#define MSG_NOTASSIGNED "%s: not assigned\n"
#define MSG_MOUNTED     " [Mounted]\n"
#define MSG_CONFLICT    "Only one of ADD, SUB, PATH, or DEFER allowed\n"
#define MSG_NOSUB       "Can't subtract %s from %s\n"
#define MSG_NOADD       "Can't add %s to %s\n"
#define MSG_LOOP        "Assign would refer to itself\n"

//DOSCALL cmd_assign(APTR SysBase)
DOSCALL main(APTR SysBase)
{
	pDOSBase	DOSBase;
	pUtilBase	UtilBase;
	struct RDargs *rdargs;

	INT32	rc 	= RETURN_FAIL;
	INT32	rc2 = 0;
	INT32 	opts[OPT_COUNT];

	STRPTR	name;
	
	DOSBase = OpenLibrary("dos.library", 0);
	if (DOSBase)
	{
		UtilBase = OpenLibrary("utility.library",0);
		if (UtilBase)
		{
			MemSet((char *)opts, 0, sizeof(opts));
			rdargs = ReadArgs(TEMPLATE, (UINT32*)opts);

			if (rdargs == NULL) PrintFault(IoErr(), NULL);
			else 
			{
				rc = rc2 = 0;
				name = (STRPTR)opts[OPT_NAME];

				if (name)
				{
					Printf("name: [%s]\n", name);
					INT32 len = Strlen(name);
					INT8 *pos = Strchr(name, ':');
					if ((len-1 > MAXASNLEN) || !pos || pos[1] || (name[len] !=':')
					{
						msg = MSG_BADDEV;
						rc	= RETURN_FAIL;
						goto ERRORCASE;
					}
					// strip ":"
					name[len] = '\0';
				}
				
				if (opts[OPT_EXIST])
				{
					opts[OPT_LIST] = 0;
				} else if (opts[OPT_VOL] || opts[OPT_DIR] || opts[OPT_DEV])
				{
					opts[OPT_LIST] = DOSIO_TRUE;
				} else if (opts[OPT_LIST] || !name)
				{
					opts[OPT_LIST] = opts[OPT_VOL] = opts[OPT_DIR]  = opts[OPT_DEV] = DOSIO_TRUE;
				}
			
				if (!target) opts[OPT_DEFER] = opts[OPT_PATH] = opts[OPT_ADD] = opts[OPT_REMOVE] = 0;
				if( (opts[OPT_ADD] != 0)  + (opts[OPT_DEFER] != 0) + (opts[OPT_PATH] != 0) + (opts[OPT_REMOVE] != 0) > 1)
				{
					msg = MSG_CONFLICT;
					rc = RETURN_FAIL;
					goto ERRORCASE;
				}

				if(name && !opts[OPT_EXIST] && !opts[OPT_DISMOUNT])
				{
					do
					{
						if(CheckSignal(SIGBREAKF_CTRL_C))
						{
							PrintFault(ERROR_BREAK, NULL);
							rc = RETURN_FAIL;
							rc2 = ERROR_BREAK;
							goto ERRORCASE;
						}
						
						if(opts[OPT_DEFER])
						{
							/* We know we have a target or DEFER would have been reset above */
							if(msg = CheckPath(DOSBase, name, target, (void **)firstbuf, sizeof(firstbuf)))
							{
								rc = RETURN_FAIL;
								rc2 = (rc2 == -1 ? IoErr() : 0);
							}
							else if (!AssignLate(name, target))
							{
								msg = MSG_IDUNNO;
								rc2 = IoErr();
							}
						}
						else
						{
							lock = NULL;
							if (target && !opts[OPT_PATH] && !(lock=Lock(target, SHARED_LOCK)))
							{
								msg = MSG_NODIR;
								rc2 = IoErr();
								opts[0] = (INT32)target;
								goto ERRORCASE;
							}
							if(opts[OPT_ADD] || opts[OPT_REMOVE])
							{
#if 0
								DosList = LockDosList(LDF_ASSIGNS|LDF_WRITE);

								if( (DosList = FindDosEntry(DosList, name, LDF_ASSIGNS) )
								   &&
								   (
									   DosList->dol_Type != DLT_DIRECTORY 
									   ||
									   (opts[OPT_ADD] && !AssignAdd(name, lock) )
									   ||
									   (opts[OPT_REMOVE] && !RemAssignList(name, lock) )
								   )
								  )
								{
									msg = (opts[OPT_REMOVE] ? MSG_NOSUB : MSG_NOADD);
									opts[0] = (LONG)target;
									opts[1] = (LONG)name;
									rc2 = IoErr();
								}
								UnLockDosList(LDF_ASSIGNS|LDF_WRITE);
								if(opts[OPT_REMOVE]) UnLock(lock);
								if(!DosList && !opts[OPT_REMOVE] && 
								   !AssignLock((UBYTE *)(name), lock))
								{
									msg = MSG_IDUNNO;
									rc2 = IoErr();
									UnLock(lock);
								}
#endif
							} else if(opts[OPT_PATH])
							{
								UnLock(lock);

								if(msg = CheckPath(DOSBase, name, target, 
												   (void **)firstbuf, sizeof(firstbuf)))
								{
									rc = RETURN_FAIL;
									rc2 = (rc2 == -1 ? IoErr() : 0);
								}
								else if(!AssignPath(name, target))
								{
									msg = MSG_IDUNNO;
									rc2 = IoErr();
								}
							}
							else
							{
								if(AssignLock(name, lock) != DOSTRUE)
								{
									msg = MSG_INUSE;
									rc2 = IoErr();
								}

								opts[OPT_ADD] = DOSIO_TRUE;
							}
						}
						if (msg) goto ERRORCASE;
					} while (target && (target = *(++argptr)));
					
					if(opts[OPT_DISMOUNT] && name)
					{
						DosList = LockDosList(LDF_ALL|LDF_WRITE);
						if(DosList = FindDosEntry(DosList, name, LDF_ALL))
						{
							RemDosEntry(DosList);
						}
						UnLockDosList(LDF_ALL|LDF_WRITE);
					}

					if (opts[OPT_LIST] || opts[OPT_EXIST])
					{
						for (didone=cnt=nodetype=0; nodetype<3; nodetype++)
						{
							if(nodetype == 0)
							{
								tmpchar = MSG_VOLUMES;
								flags   = LDF_VOLUMES;
							}
							else if(nodetype == 1)
							{
								tmpchar = MSG_ASSIGNS;
								flags   = LDF_ASSIGNS;
							}
							else
							{
								tmpchar = MSG_DEVICES;
								flags   = LDF_DEVICES;
							}
							if(opts[OPT_LIST]) 
							{
								/* The following depends on OPT_VOL, OPT_DIR and OPT_DEV   */
								/* being consecutive and in that order in the 'opts' array */

								if(!opts[OPT_VOL + nodetype]) continue;

								PutStr(tmpchar);
							}

							buf = &obuf;
							if(!(DosList = LockDosList(flags|LDF_READ)))
							{
								msg = MSG_IDUNNO;
								rc2 = IoErr();
								goto ERRORCASE;
							}

							while(DosList=NextDosEntry(DosList, flags))
							{
								if(opts[OPT_EXIST])
								{
									if(!(DosList=FindDosEntry(DosList, name, flags)))
										break;
									didone = 1;
								}

								tmpchar = (char *)BADDR(DosList->dol_Name);
								tlen = tmpchar[0];
								MyWriteChars(tmpchar+1, tlen, &buf);

								switch(nodetype)
								{
									case 0:
									MyPutStr(DosList->dol_Task ? MSG_MOUNTED : "\n", &buf);
									break;

									case 1:
									tmpchar = (char *)DosList->dol_misc.dol_assign.dol_AssignName;
									MyWriteChars("               ", 
												 (tlen < 15 ? 15-tlen : 1), &buf);
									switch(DosList->dol_Type)
									{
										case DLT_DIRECTORY:
										PutName(DOSBase, DosList->dol_Lock, &buf);

										if(alist=DosList->dol_misc.dol_assign.dol_List)
										{
											for(; alist; alist = alist->al_Next)
											{
												MyPutStr("             + ", &buf);
												PutName(DOSBase, alist->al_Lock, &buf);
											}
										}
										break;


										case DLT_LATE:
										/* Late-binding assign */
										MyWriteChars("<", 1, &buf);
										MyPutStr(tmpchar, &buf);
										MyWriteChars(">\n", 2, &buf);
										break;

										case DLT_NONBINDING:
										MyWriteChars("[", 1, &buf);
										MyPutStr(tmpchar, &buf);
										MyWriteChars("]\n", 2, &buf);
										break;

									}
									break;

									case 2:
									if(opts[OPT_EXIST] || ++cnt == 5)
									{
										cnt = 0;
										MyPutStr("\n", &buf);
									}
									else
									{
										MyPutStr(" ", &buf);
									}
									break;

								}
							}
							UnLockDosList(flags|LDF_READ);
							if(rc2=MyFlush(DOSBase, &obuf)) 
							{
								rc = RETURN_FAIL;
								PrintFault(rc2, NULL);
								goto ERRORCASE;
							}
						}
						if(cnt > 0) PutStr("\n");  /* Might need a last LF for list */
						if(opts[OPT_EXIST] && !didone)
						{
							msg = MSG_NOTASSIGNED;
							rc = RETURN_WARN;
						}
					}
				}
ERRORCASE:
				for(buf=obuf.next; buf; buf=obuf.next)
				{
					obuf.next = buf->next;
					FreeMem((char *)buf, buf->max+sizeof(struct BUFINFO));
				}
				if(msg) VPrintf(msg, opts);				
			}
			FreeArgs(rdargs);
			SetIoErr(rc2);
			CloseLibrary(UtilBase);
		}
		CloseLibrary(DOSBase);
	}
	if(!rc && rc2) rc = RETURN_FAIL;
	return rc;
}

