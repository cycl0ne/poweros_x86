#ifndef console_h
#define console_h
/**
 * @file console.c
 *
 * ANSI Handling for a console.device
 * This file is not usable ALONE. Its only a skeleton for handling with ESC chars
 */
/* Original Idea by  Kevin Lange of ToaruOS, rewritten/designed by Claus Herrmann*/
/* PowerOS, Copyright (C) 2014.  All rights reserved. */
#include "types.h"
#include "exec_interface.h"
#include "utility_interface.h"
#define UtilBase s->utilBase

#define TERM_BUF_LEN 128

typedef struct {
	UINT32	c;     /* codepoint */
	UINT32	fg;    /* background indexed color */
	UINT32	bg;    /* foreground indexed color */
	UINT32	flags; /* other flags */
} term_cell_t;

struct ConsoleUnit;

typedef struct term_callbacks {
	void (*writer)(struct ConsoleUnit *,char);
	void (*set_color)(struct ConsoleUnit *,UINT32, UINT32);
	void (*set_csr)(struct ConsoleUnit *,int,int);
	int  (*get_csr_x)(struct ConsoleUnit *);
	int  (*get_csr_y)(struct ConsoleUnit *);
	void (*set_cell)(struct ConsoleUnit *,int,int,UINT32);
	void (*cls)(struct ConsoleUnit *,int);
	void (*scroll)(struct ConsoleUnit *,int);
	void (*redraw_cursor)(struct ConsoleUnit *);
	void (*input_buffer_stuff)(struct ConsoleUnit *,char *);
	void (*set_font_size)(struct ConsoleUnit *,float);
	void (*set_title)(struct ConsoleUnit *,char *);
} term_callbacks_t;

typedef struct {
	UINT16	x;       /* Current cursor location */
	UINT16	y;       /*    "      "       "     */
	UINT16	save_x;  /* Last cursor save */
	UINT16	save_y;
	UINT32	width;   /* Terminal width */
	UINT32	height;  /*     "    height */
	UINT32	fg;      /* Current foreground color */
	UINT32	bg;      /* Current background color */
	UINT8	flags;   /* Bright, etc. */
	UINT8	escape;  /* Escape status */
	UINT8	box;
	UINT8	buflen;  /* Buffer Length */
	char     buffer[TERM_BUF_LEN];  /* Previous buffer */
	term_callbacks_t * callbacks;
	pUtilBase	utilBase;
	struct ConsoleUnit *unit;
	int volatile lock;
} term_state_t;

/* Triggers escape mode. */
#define ANSI_ESCAPE  27
/* Escape verify */
#define ANSI_BRACKET '['
#define ANSI_BRACKET_RIGHT ']'
#define ANSI_OPEN_PAREN '('
/* Anything in this range (should) exit escape mode. */
#define ANSI_LOW    'A'
#define ANSI_HIGH   'z'
/* Escape commands */
#define ANSI_CUU    'A' /* CUrsor Up                  */
#define ANSI_CUD    'B' /* CUrsor Down                */
#define ANSI_CUF    'C' /* CUrsor Forward             */
#define ANSI_CUB    'D' /* CUrsor Back                */
#define ANSI_CNL    'E' /* Cursor Next Line           */
#define ANSI_CPL    'F' /* Cursor Previous Line       */
#define ANSI_CHA    'G' /* Cursor Horizontal Absolute */
#define ANSI_CUP    'H' /* CUrsor Position            */
#define ANSI_ED     'J' /* Erase Data                 */
#define ANSI_EL     'K' /* Erase in Line              */
#define ANSI_SU     'S' /* Scroll Up                  */
#define ANSI_SD     'T' /* Scroll Down                */
#define ANSI_HVP    'f' /* Horizontal & Vertical Pos. */
#define ANSI_SGR    'm' /* Select Graphic Rendition   */
#define ANSI_DSR    'n' /* Device Status Report       */
#define ANSI_SCP    's' /* Save Cursor Position       */
#define ANSI_RCP    'u' /* Restore Cursor Position    */
#define ANSI_HIDE   'l' /* DECTCEM - Hide Cursor      */
#define ANSI_SHOW   'h' /* DECTCEM - Show Cursor      */
/* Display flags */
#define ANSI_BOLD      0x01
#define ANSI_UNDERLINE 0x02
#define ANSI_ITALIC    0x04
#define ANSI_ALTFONT   0x08 /* Character should use alternate font */
#define ANSI_SPECBG    0x10
#define ANSI_BORDER    0x20
#define ANSI_WIDE      0x40 /* Character is double width */
#define ANSI_CROSS     0x80 /* And that's all I'm going to support (for now) */

#define ANSI_EXT_IOCTL 'z'  /* These are special escapes only we support */

/* Default color settings */
#define TERM_DEFAULT_FG     0x07 /* Index of default foreground */
#define TERM_DEFAULT_BG     0x10 /* Index of default background */
#define TERM_DEFAULT_FLAGS  0x00 /* Default flags for a cell */
#define TERM_DEFAULT_OPAC   0xF2 /* For background, default transparency */



#endif
