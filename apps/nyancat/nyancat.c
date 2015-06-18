/**
 * @file sanitycheck.c
 *
 * This file describes a standard DOS handler for use with a disk (Filesystemtype).
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "execbase_private.h"
#include "ports.h"
#include "devices.h"
#include "dos.h"
#include "dos_packets.h"
#include "dos_errors.h"
#include "dos_io.h"

#include "exec_interface.h"
#include "dos_interface.h"
#include "utility_interface.h"

#include "animation.h"

#define TEMPLATE 		"CON/S,TIME/N,FRAME/N"
#define OPT_CONSOLE		0
#define OPT_TIME		1
#define OPT_FRAME		2
#define OPT_COUNT		3

#define PASSED "\033[60G[\033[1;32mPASS\033[0;39m]\n\r"
#define FAILED "\033[60G[\033[1;31mFAIL\033[0;39m]\n\r"

struct GlobalData
{
	APTR SysBase;
	APTR DOSBase;
	APTR UtilBase;
};

static void _prbuf(INT32 ch, INT8 **str);
static int sprintf(struct GlobalData *gd, char* str, char *fmt,...);
static void PrintCon(struct GlobalData *gd, pIOStdReq io, STRPTR string);
void printf2(struct GlobalData *gd, pIOStdReq io, char *fmt,...);

// "\033[1;34m%s-%s \033[1;31m%d\033[1;34m %s#\033[0m ", __kernel_name, version_number, retval, current_process->wd_name

//#define printf(a...) printf2(&gd, io, ##a)


DOSIO main(pSysBase SysBase)
{
	APTR		DOSBase;
	DOSIO		rc = RETURN_ERROR;
	UINT32		opts[OPT_COUNT];
	struct RDargs*	rdargs;
	struct GlobalData gd;

	if ( (DOSBase = OpenLibrary("dos.library", 0)) )
	{
		BOOL con = TRUE;
		APTR UtilBase = OpenLibrary("utility.library", 0);
		gd.UtilBase = UtilBase;
		gd.SysBase	= SysBase;
		gd.DOSBase	= DOSBase;
		MemSet((char*)opts, 0, sizeof(opts));
		rdargs = ReadArgs(TEMPLATE, opts);
		if (rdargs == NULL)
		{
			PrintFault(IoErr(), NULL);
		} else
		{
			UINT32 time = 6;
			unsigned int frame_count = 150;
			if (opts[OPT_CONSOLE] != TRUE) con = FALSE;
			if (opts[OPT_FRAME]) frame_count = *((UINT32*)opts[OPT_FRAME]);
			if (opts[OPT_TIME])  time = *((UINT32*)opts[OPT_TIME]);
			Printf("frame: %d, time: %d\n", frame_count, time);
//			return 0;
//			Printf("%x",opts[OPT_CONSOLE]);
//			return 0;
			pMsgPort p = CreateMsgPort(NULL);
			if (p)
			{
				pIOStdReq io = CreateIORequest(p,0);
				if (io)
				{
					if (!OpenDevice("consolex86.device", 0, io, 0))
					{
						rc = RETURN_OK;
						char buffer[256];
						const char * colors[256] = {NULL};
						const char * output = "  ";
						int clear_screen = 1;
						int min_row = -1;
						int max_row = -1;
						int min_col = -1;
						int max_col = -1;
						char using_automatic_width = 0;
						char using_automatic_height = 0;
						int ttype = 1;
						int always_escape = 0; /* Used for text mode */
#if 1
						colors[',']  = "\033[48;5;17m";  /* Blue background */
						colors['.']  = "\033[48;5;231m"; /* White stars */
						colors['\''] = "\033[48;5;16m";  /* Black border */
						colors['@']  = "\033[48;5;230m"; /* Tan poptart */
						colors['$']  = "\033[48;5;175m"; /* Pink poptart */
						colors['-']  = "\033[48;5;162m"; /* Red poptart */
						colors['>']  = "\033[48;5;196m"; /* Red rainbow */
						colors['&']  = "\033[48;5;214m"; /* Orange rainbow */
						colors['+']  = "\033[48;5;226m"; /* Yellow Rainbow */
						colors['#']  = "\033[48;5;118m"; /* Green rainbow */
						colors['=']  = "\033[48;5;33m";  /* Light blue rainbow */
						colors[';']  = "\033[48;5;19m";  /* Dark blue rainbow */
						colors['*']  = "\033[48;5;240m"; /* Gray cat face */
						colors['%']  = "\033[48;5;175m"; /* Pink cheeks */
#else
						colors[',']  = "\033[0;34;44m";  /* Blue background */
						colors['.']  = "\033[1;37;47m";  /* White stars */
						colors['\''] = "\033[0;30;40m";  /* Black border */
						colors['@']  = "\033[1;37;47m";  /* Tan poptart */
						colors['$']  = "\033[1;35;45m";  /* Pink poptart */
						colors['-']  = "\033[1;31;41m";  /* Red poptart */
						colors['>']  = "\033[1;31;41m";  /* Red rainbow */
						colors['&']  = "\033[0;33;43m";  /* Orange rainbow */
						colors['+']  = "\033[1;33;43m";  /* Yellow Rainbow */
						colors['#']  = "\033[1;32;42m";  /* Green rainbow */
						colors['=']  = "\033[1;34;44m";  /* Light blue rainbow */
						colors[';']  = "\033[0;34;44m";  /* Dark blue rainbow */
						colors['*']  = "\033[1;30;40m";  /* Gray cat face */
						colors['%']  = "\033[1;35;45m";  /* Pink cheeks */
#endif
						/*
						 * Actual width/height of terminal.
						 */
						int terminal_width = 80;
						int terminal_height = 25;
						if (min_col == max_col) {
							min_col = (FRAME_WIDTH - terminal_width/2) / 2;
							max_col = (FRAME_WIDTH + terminal_width/2) / 2;
							using_automatic_width = 1;
						}
//Printf("max: %d, min: %d\n", min_col, max_col);
						if (min_row == max_row) {
							min_row = (FRAME_HEIGHT - (terminal_height-1)) / 2;
							max_row = (FRAME_HEIGHT + (terminal_height-1)) / 2;
							using_automatic_height = 1;
						}
//Printf("max: %d, min: %d\n", min_row, max_row);
						if (clear_screen) {
							/* Clear the screen */
							if (con)
							{
								sprintf(&gd, buffer, "\033[H\033[2J\033[?25l");
								PrintCon(&gd, io, buffer);
							} else
								Printf("\033[H\033[2J\033[?25l");
						} else {
							Printf("\033[s");
						}
//						Printf("Clearscreen\n");
						int playing = 1;    /* Animation should continue [left here for modifications] */
						UINT32 i = 0;       /* Current frame # */
						unsigned int f = 0; /* Total frames passed */
						char last = 0;      /* Last color index rendered */
						int y, x;        /* x/y coordinates of what we're drawing */

						while(playing)
						{
							if (clear_screen) {
								if (con)
								{
									sprintf(&gd, buffer, "\033[H");
									PrintCon(&gd, io, buffer);
								} else
									Printf("\033[H");
							} else {
								Printf("\033[u");
							}

							for (y = min_row; y < max_row; ++y) {
								for (x = min_col; x < max_col; ++x) {
									char color;
									if (y > 23 && y < 43 && x < 0) {
										/*
										 * Generate the rainbow tail.
										 *
										 * This is done with a pretty simplistic square wave.
										 */
										int mod_x = ((-x+2) % 16) / 8;
										if ((i / 2) % 2) {
											mod_x = 1 - mod_x;
										}
										/*
										 * Our rainbow, with some padding.
										 */
										const char *rainbow = ",,>>&&&+++###==;;;,,";
										color = rainbow[mod_x + y-23];
										if (color == 0) color = ',';
									} else if (x < 0 || y < 0 || y >= FRAME_HEIGHT || x >= FRAME_WIDTH) {
										/* Fill all other areas with background */
										color = ',';
									} else {
										/* Otherwise, get the color from the animation frame. */
										color = frames[i][y][x];
									}
									if (always_escape) {
										/* Text mode (or "Always Send Color Escapes") */
										if (con)
										{
											sprintf(&gd, buffer, "%s", colors[(int)color]);
											PrintCon(&gd, io, buffer);
										} else
											Printf("%s", colors[(int)color]);
									} else {
										if (color != last && colors[(int)color]) {
											/* Normal Mode, send escape (because the color changed) */
											last = color;
											if (con)
											{
												sprintf(&gd, buffer, "%s%s", colors[(int)color], output);
												PrintCon(&gd, io, buffer);
											} else
												Printf("%s%s", colors[(int)color], output);
										} else {
											/* Same color, just send the output characters */
											if (con)
											{
												sprintf(&gd, buffer, "%s", output);
												PrintCon(&gd, io, buffer);
											} else
												Printf("%s", output);
										}
									}
								}
								/* End of row, send newline */
								//newline(1);
//								for(;;);
								if (con)
								{
									sprintf(&gd, buffer, "\r\n");
									PrintCon(&gd, io, buffer);
								} else
									Printf("\n");
							}
							/* Reset the last color so that the escape sequences rewrite */
							last = 0;
							/* Update frame count */
							++f;
							if (frame_count != 0 && f == frame_count) {
								//finish();
								playing = 0;
							}
							++i;
							if (!frames[i]) {
								/* Loop animation */
								i = 0;
							}
							Delay(time);
							//for(int claus = 0; claus < 0xffffff; claus++);
						}
#if 0
						Printf("Sending a CLS to console.device\n");
						UINT8 data[] = "\033[2J";
						io->io_Command = CMD_WRITE;
						io->io_Length = 4;
						io->io_Data = &data;
						DoIO(io);
						Printf("io->io_Error: %d\n", io->io_Error);
						Printf("Screen cleared Response [%d]\n", io->io_Actual);
						Printf("Sending a PASSED to console.device\n");
						PrintCon(&gd, io, "Test it ");
						PrintCon(&gd, io, PASSED);
						PrintCon(&gd, io, "Test it ");
						PrintCon(&gd, io, FAILED);
//						PrintCon(&gd, io, "Test it \n");
						Printf("Done.\n");
						//printf( "\033[2J");
#endif
						if (clear_screen) {
							if (con)
							{
								sprintf(&gd, buffer, "\033[?25h\033[0m\033[H\033[2J");
								PrintCon(&gd, io, buffer);
							} else
								Printf("\033[?25h\033[0m\033[H\033[2J");
						} else {
							Printf("\033[0m\n");
						}

						CloseDevice(io);
					}
					DeleteIORequest(io);
				}
				DeleteMsgPort(p);
			}
			CloseLibrary(DOSBase);
		}
		FreeArgs(rdargs);
		CloseLibrary(UtilBase);
	} else
	{
		FindProcess(NULL)->pr_Result2 = ERROR_INVALID_RESIDENT_LIBRARY;
		rc = RETURN_FAIL;
	}
	return rc;
}

//int make(const char* deviceName, uint64_t primarySuper, uint64_t sectorCount, uint8_t logSectorsPerBand, uint8_t preallocCount, const char* volumeLabel);

static void _prbuf(INT32 ch, INT8 **str)
{
	*(*str)++ = (char)ch;
}

static int sprintf(struct GlobalData *gd, char* str, char *fmt,...)
{
	APTR SysBase = gd->SysBase;
	APTR UtilBase = gd->UtilBase;
	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt((const char *)fmt, pvar,(void(*)()) _prbuf, (APTR) str);
	va_end(pvar);
	return Strlen((STRPTR)str);
}

static void PrintCon(struct GlobalData *gd, pIOStdReq io, STRPTR string)
{
	APTR SysBase = gd->SysBase;
	APTR UtilBase = gd->UtilBase;
	APTR DOSBase = gd->DOSBase;

	io->io_Command = CMD_WRITE;
	io->io_Length = Strlen(string);
	io->io_Data = string;
	INT32 ret = DoIO(io);
	if (ret || (io->io_Length != io->io_Actual)) Printf("Error in DoIO\n");
}

void printf2(struct GlobalData *gd, pIOStdReq io, char *fmt,...)
{
	APTR SysBase = gd->SysBase;
	APTR UtilBase = gd->UtilBase;
	APTR DOSBase = gd->DOSBase;

	char	buffer[256];
	va_list pvar;
	va_start(pvar, fmt);
	sprintf(gd, buffer, pvar);
	va_end(pvar);

	INT32 i = sprintf(gd, buffer, fmt);
	Printf("i=%d\n",i);
//	Printf("Buffer: %s (%s)\n", buffer, fmt);
	PrintCon(gd, io, buffer);
}
