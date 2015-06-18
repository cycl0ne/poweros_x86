/**
 * @file console_cmds.c
 *
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "console_device.h"

#define uint32_t UINT32
#define uint16_t UINT16
#define uint8_t UINT8

#define UTF8_ACCEPT 0
#define UTF8_REJECT 1

static const uint8_t utf8d[] = {
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 00..1f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 20..3f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 40..5f
	0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0, // 60..7f
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9,9, // 80..9f
	7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7,7, // a0..bf
	8,8,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2,2, // c0..df
	0xa,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x3,0x4,0x3,0x3, // e0..ef
	0xb,0x6,0x6,0x6,0x5,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8,0x8, // f0..ff
	0x0,0x1,0x2,0x3,0x5,0x8,0x7,0x1,0x1,0x1,0x4,0x6,0x1,0x1,0x1,0x1, // s0..s0
	1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,1,0,1,1,1,1,1,0,1,0,1,1,1,1,1,1, // s1..s2
	1,2,1,1,1,1,1,2,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1, // s3..s4
	1,2,1,1,1,1,1,1,1,2,1,1,1,1,1,1,1,1,1,1,1,1,1,3,1,3,1,1,1,1,1,1, // s5..s6
	1,3,1,1,1,1,1,3,1,3,1,1,1,1,1,1,1,3,1,1,1,1,1,1,1,1,1,1,1,1,1,1, // s7..s8
};

static uint32_t inline
decode(uint32_t* state, uint32_t* codep, uint32_t byte) {
	uint32_t type = utf8d[byte];

	*codep = (*state != UTF8_ACCEPT) ?
		(byte & 0x3fu) | (*codep << 6) :
		(0xff >> type) & (byte);

	*state = utf8d[256 + *state*16 + type];
	return *state;
}

extern term_state_t * ansi_init(pSysBase SysBase, APTR unit, int w, int h, term_callbacks_t * callbacks_in) ;
term_callbacks_t term_callbacks;
#define wchar_t UINT32
int wcwidth(wchar_t wc);

#define INT32_MAX 0x7FFFFFFF


#define PALETTE_COLORS 256
extern UINT32 vga_colors[PALETTE_COLORS];

static char vga_to_ansi[] = {
	0, 4, 2, 6, 1, 5, 3, 7,
	8,12,10,14, 9,13,11,15
};

static void *MemMove( void * s1, const void * s2, UINT32 n )
{
	char * dest = (char *) s1;
	const char * src = (const char *) s2;
	if ( dest <= src ) while ( n-- ) *dest++ = *src++;
    else
    {
        src += n;
        dest += n;
        while ( n-- ) *--dest = *--src;
    }
    return s1;
}

#define SysBase ((pConBase)io->io_Device)->dev_SysBase
#define DEVBase ((pConBase)io->io_Device)

static void Invalid(struct IOStdReq *io);
static void Read(pIOStdReq io);
static void Write(pIOStdReq io);

/*******************

Function Table 

********************/

void (*con_commandVector[])(struct IOStdReq *) = {
	Invalid, Invalid, Read, Write, Invalid,
	Invalid, Invalid, Invalid, Invalid
//	Invalid, Reset, Read, Write, Invalid, 
//	/*Clear*/ Invalid, Stop, Start, Flush
};

/*******************

Console Functions

********************/

void con_EndCommand(struct IOStdReq *io, UINT32 error)
{
	io->io_Error = error;
	if (TEST_BITS(io->io_Flags, IOF_QUICK)) return; //{ KPrintF("Returning Quick [Error: %d]\n", error);return;}
	ReplyMsg(&io->io_Message);
	//KPrintF("Replied\n");
	return;	
}

static void Invalid(struct IOStdReq *io)
{
	con_EndCommand(io, IOERR_NOCMD);
}

/*
int r = read(fd_master, buf, 1024);
for (uint32_t i = 0; i < r; ++i) {
	ansi_put(ansi_state, buf[i]);
}
*/

static void Read(struct IOStdReq *io)
{
	con_EndCommand(io, IOERR_NOCMD);
}

void ansi_put(term_state_t * s, char c);

static void Write(struct IOStdReq *io)
{
	pConsoleUnit unit = (pConsoleUnit)io->io_Unit;
	UINT32 io_Actual = 0;
	UINT32 io_Length = io->io_Length;
	UINT8*	io_Data = io->io_Data;
	
	while(io_Actual < io_Length)
	{
		//KPrintF("[%c]", io_Data[io_Actual]);
		ansi_put(unit->con_AnsiState, io_Data[io_Actual]);
		io_Actual++;
	}
	io->io_Actual = io_Actual;
	con_EndCommand(io, 0);
}

/*******************

Internal Console Functions 

********************/
#undef SysBase
#define SysBase unit->con_SysBase

#define term_width	unit->con_Width
#define term_height	unit->con_Height
#define csr_x		unit->con_CSRX
#define csr_y		unit->con_CSRY
#define term_buffer unit->con_Buffer
#define current_fg	unit->con_CurFG
#define current_bg	unit->con_CurBG
#define ansi_state	unit->con_AnsiState
#define _hold_out	unit->con_HoldOut
#define cursor_on	unit->con_Cursor
#define timer_tick	unit->con_TimerTick

static void term_set_cell(pConsoleUnit unit, int x, int y, uint32_t c);

static uint32_t ununicode(uint32_t c) 
{
/*
	wchar_t * w = box_chars_in;
	while (*w) {
		if (*w == c) {
			return box_chars_out[w-box_chars_in];
		}
		w++;
	}
*/
	return 4;
}

static int color_distance(uint32_t a, uint32_t b) 
{
	int a_r = (a & 0xFF0000) >> 16;
	int a_g = (a & 0xFF00) >> 8;
	int a_b = (a & 0xFF);

	int b_r = (b & 0xFF0000) >> 16;
	int b_g = (b & 0xFF00) >> 8;
	int b_b = (b & 0xFF);

	int distance = 0;
	distance += ABS(a_r - b_r) * 3;
	distance += ABS(a_g - b_g) * 6;
	distance += ABS(a_b - b_b) * 10;

	return distance;
}

static uint32_t vga_base_colors[] = {
	0x000000,
	0xAA0000,
	0x00AA00,
	0xAA5500,
	0x0000AA,
	0xAA00AA,
	0x00AAAA,
	0xAAAAAA,
	0x555555,
	0xFF5555,
	0x55AA55,
	0xFFFF55,
	0x5555FF,
	0xFF55FF,
	0x55FFFF,
	0xFFFFFF,
};

static int is_gray(uint32_t a) {
	int a_r = (a & 0xFF0000) >> 16;
	int a_g = (a & 0xFF00) >> 8;
	int a_b = (a & 0xFF);

	return (a_r == a_g && a_g == a_b);
}

static int best_match(uint32_t a) {
	int best_distance = INT32_MAX;
	int best_index = 0;
	for (int j = 0; j < 16; ++j) {
		if (is_gray(a) && !is_gray(vga_base_colors[j]));
		int distance = color_distance(a, vga_base_colors[j]);
		if (distance < best_distance) {
			best_index = j;
			best_distance = distance;
		}
	}
	return best_index;
}

unsigned short * textmemptr = (unsigned short *)0xB8000;
static inline void placech(unsigned char c, int x, int y, int attr) 
{
	unsigned short *where;
	unsigned att = attr << 8;
	where = textmemptr + (y * 80 + x);
	*where = c | att;
}

static void term_write_char(pConsoleUnit unit, UINT32 val, UINT16 x, UINT16 y, UINT32 fg, UINT32 bg, UINT8 flags) 
{
	if (val > 128) val = ununicode(val);
	if (fg > 256) {
		fg = best_match(fg);
	}
	if (bg > 256) {
		bg = best_match(bg);
	}
	if (fg > 16) {
		fg = vga_colors[fg];
	}
	if (bg > 16) {
		bg = vga_colors[bg];
	}
	if (fg == 16) fg = 0;
	if (bg == 16) bg = 0;
//	KPrintF("fg%d",fg);
//	fg = 2;
	placech(val, x, y, (vga_to_ansi[fg] & 0xF) | (vga_to_ansi[bg] << 4));
}

static void cell_redraw(pConsoleUnit unit, UINT16 x, UINT16 y) 
{
	if (x >= term_width || y >= term_height) return;
	term_cell_t *cell = (term_cell_t *)((UINT32)term_buffer + (y * term_width + x) * sizeof(term_cell_t));
	if (((UINT32 *)cell)[0] == 0x00000000) 
	{
		//KPrintF("#");
		term_write_char(unit, ' ', x * char_width, y * char_height, TERM_DEFAULT_FG, TERM_DEFAULT_BG, TERM_DEFAULT_FLAGS);
	} else 
	{
		//KPrintF("*");
		term_write_char(unit, cell->c, x * char_width, y * char_height, cell->fg, cell->bg, cell->flags);
	}
}

static void cell_set(pConsoleUnit unit, uint16_t x, uint16_t y, uint32_t c, uint32_t fg, uint32_t bg, uint8_t flags) 
{
	if (x >= term_width || y >= term_height) return;
	term_cell_t * cell = (term_cell_t *)((UINT32)term_buffer + (y * term_width + x) * sizeof(term_cell_t));
//KPrintF("fg %d, bg %d", fg, bg);
	cell->c     = c;
	cell->fg    = fg;
	cell->bg    = bg;
	cell->flags = flags;
}

static void cell_redraw_inverted(pConsoleUnit unit, uint16_t x, uint16_t y) 
{
	if (x >= term_width || y >= term_height) return;
	term_cell_t * cell = (term_cell_t *)((UINT32)term_buffer + (y * term_width + x) * sizeof(term_cell_t));
	if (((UINT32 *)cell)[0] == 0x00000000) {
		term_write_char(unit,' ', x * char_width, y * char_height, TERM_DEFAULT_BG, TERM_DEFAULT_FG, TERM_DEFAULT_FLAGS | ANSI_SPECBG);
	} else {
		term_write_char(unit,cell->c, x * char_width, y * char_height, cell->bg, cell->fg, cell->flags | ANSI_SPECBG);
	}
}

static void cell_redraw_box(pConsoleUnit unit, uint16_t x, uint16_t y) 
{
	if (x >= term_width || y >= term_height) return;
	term_cell_t * cell = (term_cell_t *)((UINT32)term_buffer + (y * term_width + x) * sizeof(term_cell_t));
	if (((UINT32 *)cell)[0] == 0x00000000) 
	{
		term_write_char(unit,' ', x * char_width, y * char_height, TERM_DEFAULT_FG, TERM_DEFAULT_BG, TERM_DEFAULT_FLAGS | ANSI_BORDER);
	} else 
	{
		term_write_char(unit,cell->c, x * char_width, y * char_height, cell->fg, cell->bg, cell->flags | ANSI_BORDER);
	}
}

static void outb(unsigned char _data, unsigned short _port) {
	__asm__ __volatile__ ("outb %1, %0" : : "dN" (_port), "a" (_data));
}

static void render_cursor(pConsoleUnit unit) 
{
	unsigned int tmp = csr_y * 80 + csr_x;
	outb(14, 0x3D4);
	outb(tmp >> 8, 0x3D5);
	outb(15, 0x3D4);
	outb(tmp, 0x3D5);
}

static void draw_cursor(pConsoleUnit unit) 
{
	if (!cursor_on) return;
	timer_tick = 0;
	render_cursor(unit);
}

static void term_redraw_all(pConsoleUnit unit)
{
	for (UINT16 y = 0; y < term_height; ++y)
	{
		for (UINT16 x = 0; x < term_width; ++x) 
		{
			cell_redraw(unit, x, y);
		}
	}
}

void reinit(pIOStdReq io, pConsoleUnit unit)
{
	unit->con_SysBase = ((pConBase)io->io_Device)->dev_SysBase;
	if (term_buffer == NULL)
	{
		term_buffer = AllocVec(sizeof(term_cell_t) * term_width *term_height, MEMF_CLEAR|MEMF_FAST); 
	}
	
	ansi_state = ansi_init(SysBase, unit, term_width, term_height, &term_callbacks);
	term_redraw_all(unit);
}

static void term_clear(pConsoleUnit unit, int i) 
{
	if (i == 2) 
	{
		/* Oh dear */
		csr_x = 0;
		csr_y = 0;
		MemSet((void *)term_buffer, 0x00, term_width * term_height * sizeof(term_cell_t));
		term_redraw_all(unit);
	} else if (i == 0) {
		for (int x = csr_x; x < term_width; ++x) {
			term_set_cell(unit, x, csr_y, ' ');
		}
		for (int y = csr_y + 1; y < term_height; ++y) {
			for (int x = 0; x < term_width; ++x) {
				term_set_cell(unit, x, y, ' ');
			}
		}
	} else if (i == 1) {
		for (int y = 0; y < csr_y; ++y) {
			for (int x = 0; x < term_width; ++x) {
				term_set_cell(unit, x, y, ' ');
			}
		}
		for (int x = 0; x < csr_x; ++x) {
			term_set_cell(unit, x, csr_y, ' ');
		}
	}
}

static void term_scroll(pConsoleUnit unit, int how_much) 
{
	if (how_much >= term_height || -how_much >= term_height) 
	{
		term_clear(unit, 2);
		return;
	}
	if (how_much == 0) 
	{
		return;
	}
	if (how_much > 0) 
	{
		/* Shift terminal cells one row up */
		MemMove(term_buffer, (void *)((UINT32)term_buffer + sizeof(term_cell_t) * term_width), sizeof(term_cell_t) * term_width * (term_height - how_much));
		/* Reset the "new" row to clean cells */
		MemSet((void *)((UINT32)term_buffer + sizeof(term_cell_t) * term_width * (term_height - how_much)), 0x0, sizeof(term_cell_t) * term_width * how_much);
		term_redraw_all(unit);
	} else {
		how_much = -how_much;
		/* Shift terminal cells one row up */
		MemMove((void *)((UINT32)term_buffer + sizeof(term_cell_t) * term_width), term_buffer, sizeof(term_cell_t) * term_width * (term_height - how_much));
		/* Reset the "new" row to clean cells */
		MemSet(term_buffer, 0x0, sizeof(term_cell_t) * term_width * how_much);
		term_redraw_all(unit);
	}
}

int is_wide(uint32_t codepoint) 
{
	if (codepoint < 256) return 0;
	return wcwidth(codepoint) == 2;
}

void term_write(pConsoleUnit unit, char c) 
{
	static uint32_t codepoint = 0;
	static uint32_t unicode_state = 0;

	cell_redraw(unit, csr_x, csr_y);
	if (!decode(&unicode_state, &codepoint, (uint8_t)c)) {
		if (c == '\r') {
			csr_x = 0;
			return;
		}
		if (csr_x == term_width) {
			csr_x = 0;
			++csr_y;
		}
		if (csr_y == term_height) {
			term_scroll(unit, 1);
			csr_y = term_height - 1;
		}
		if (c == '\n') {
			if (csr_x == 0 && _hold_out) {
				_hold_out = 0;
				return;
			}
			++csr_y;
			if (csr_y == term_height) {
				term_scroll(unit, 1);
				csr_y = term_height - 1;
			}
			draw_cursor(unit);
		} else if (c == '\007') {
			/* bell */
#if USE_BELL
			for (int i = 0; i < term_height; ++i) {
				for (int j = 0; j < term_width; ++j) {
					cell_redraw_inverted(j, i);
				}
			}
			syscall_nanosleep(0,10);
			term_redraw_all();
#endif
		} else if (c == '\b') 
		{
			if (csr_x > 0) 
			{
				--csr_x;
			}
			cell_redraw(unit, csr_x, csr_y);
			draw_cursor(unit);
		} else if (c == '\t') 
		{
			csr_x += (8 - csr_x % 8);
			draw_cursor(unit);
		} else {
			int wide = is_wide(codepoint);
			uint8_t flags = ansi_state->flags;
			if (wide && csr_x == term_width - 1) 
			{
				csr_x = 0;
				++csr_y;
			}
			if (wide) 
			{
				flags = flags | ANSI_WIDE;
			}
			cell_set(unit, csr_x,csr_y, codepoint, current_fg, current_bg, flags);
			cell_redraw(unit, csr_x,csr_y);
			csr_x++;
			if (wide && csr_x != term_width) 
			{
				cell_set(unit, csr_x, csr_y, 0xFFFF, current_fg, current_bg, ansi_state->flags);
				cell_redraw(unit, csr_x,csr_y);
				cell_redraw(unit, csr_x-1,csr_y);
				csr_x++;
			}
		}
	} else if (unicode_state == UTF8_REJECT) 
	{
		unicode_state = 0;
	}
	draw_cursor(unit);
}

void term_set_csr(pConsoleUnit unit, int x, int y) 
{
	cell_redraw(unit, csr_x,csr_y);
	csr_x = x;
	csr_y = y;
	draw_cursor(unit);
}

int term_get_csr_x(pConsoleUnit unit) 
{
	return csr_x;
}

int term_get_csr_y(pConsoleUnit unit) 
{
	return csr_y;
}

void term_set_csr_show(pConsoleUnit unit, uint8_t on) 
{
	cursor_on = on;
}

void term_set_colors(pConsoleUnit unit, uint32_t fg, uint32_t bg) 
{
	current_fg = fg;
	current_bg = bg;
}

void term_redraw_cursor(pConsoleUnit unit) 
{
	if (term_buffer) {
		draw_cursor(unit);
	}
}

void flip_cursor(pConsoleUnit unit) 
{
	static uint8_t cursor_flipped = 0;
	if (cursor_flipped) 
	{
		cell_redraw(unit, csr_x, csr_y);
	} else {
		render_cursor(unit);
	}
	cursor_flipped = 1 - cursor_flipped;
}

static void term_set_cell(pConsoleUnit unit, int x, int y, uint32_t c) 
{
	cell_set(unit, x, y, c, current_fg, current_bg, ansi_state->flags);
	cell_redraw(unit, x, y);
}

static void term_redraw_cell(pConsoleUnit unit, int x, int y) 
{
	if (x < 0 || y < 0 || x >= term_width || y >= term_height) return;
	cell_redraw(unit, x,y);
}

static void set_title(pConsoleUnit unit, char * c) 
{
	/* Do nothing */
}

static void input_buffer_stuff(pConsoleUnit unit, char * str) 
{
//	size_t s = strlen(str) + 1;
//	write(fd_master, str, s);
}

static void set_term_font_size(pConsoleUnit unit, float s) 
{
	/* do nothing */
}

/*******************

Prototype Console Functions + FuncTable

********************/

term_callbacks_t term_callbacks = {
	/* writer*/
	&term_write,
	/* set_color*/
	&term_set_colors,
	/* set_csr*/
	&term_set_csr,
	/* get_csr_x*/
	&term_get_csr_x,
	/* get_csr_y*/
	&term_get_csr_y,
	/* set_cell*/
	&term_set_cell,
	/* cls*/
	&term_clear,
	/* scroll*/
	&term_scroll,
	/* redraw_cursor*/
	&term_redraw_cursor,
	/* input_buffer_stuff*/
	&input_buffer_stuff,
	/* set_font_size*/
	&set_term_font_size,
	/* set_title*/
	&set_title,
};

int wcwidth(wchar_t wc) {
	if (wc == 0) return 0;
	else if (wc < 0x20) return -1;
	else if (wc < 0x7f) return 1;
	else if (wc < 0xa0) return -1;
	else if (wc < 0x300) return 1;
	else if (wc < 0x370) return 0;
	else if (wc < 0x378) return 1;
	else if (wc < 0x37a) return -1;
	else if (wc < 0x37f) return 1;
	else if (wc < 0x384) return -1;
	else if (wc < 0x38b) return 1;
	else if (wc < 0x38c) return -1;
	else if (wc < 0x38d) return 1;
	else if (wc < 0x38e) return -1;
	else if (wc < 0x3a2) return 1;
	else if (wc < 0x3a3) return -1;
	else if (wc < 0x483) return 1;
	else if (wc < 0x487) return 0;
	else if (wc < 0x488) return -1;
	else if (wc < 0x48a) return 0;
	else if (wc < 0x524) return 1;
	else if (wc < 0x531) return -1;
	else if (wc < 0x557) return 1;
	else if (wc < 0x559) return -1;
	else if (wc < 0x560) return 1;
	else if (wc < 0x561) return -1;
	else if (wc < 0x588) return 1;
	else if (wc < 0x589) return -1;
	else if (wc < 0x58b) return 1;
	else if (wc < 0x591) return -1;
	else if (wc < 0x5be) return 0;
	else if (wc < 0x5bf) return 1;
	else if (wc < 0x5c0) return 0;
	else if (wc < 0x5c1) return 1;
	else if (wc < 0x5c3) return 0;
	else if (wc < 0x5c4) return 1;
	else if (wc < 0x5c6) return 0;
	else if (wc < 0x5c7) return 1;
	else if (wc < 0x5c8) return 0;
	else if (wc < 0x5d0) return -1;
	else if (wc < 0x5eb) return 1;
	else if (wc < 0x5f0) return -1;
	else if (wc < 0x5f5) return 1;
	else if (wc < 0x600) return -1;
	else if (wc < 0x604) return 0;
	else if (wc < 0x606) return -1;
	else if (wc < 0x610) return 1;
	else if (wc < 0x616) return 0;
	else if (wc < 0x61c) return 1;
	else if (wc < 0x61e) return -1;
	else if (wc < 0x620) return 1;
	else if (wc < 0x621) return -1;
	else if (wc < 0x64b) return 1;
	else if (wc < 0x65f) return 0;
	else if (wc < 0x660) return -1;
	else if (wc < 0x670) return 1;
	else if (wc < 0x671) return 0;
	else if (wc < 0x6d6) return 1;
	else if (wc < 0x6e5) return 0;
	else if (wc < 0x6e7) return 1;
	else if (wc < 0x6e9) return 0;
	else if (wc < 0x6ea) return 1;
	else if (wc < 0x6ee) return 0;
	else if (wc < 0x70e) return 1;
	else if (wc < 0x70f) return -1;
	else if (wc < 0x710) return 0;
	else if (wc < 0x711) return 1;
	else if (wc < 0x712) return 0;
	else if (wc < 0x730) return 1;
	else if (wc < 0x74b) return 0;
	else if (wc < 0x74d) return -1;
	else if (wc < 0x7a6) return 1;
	else if (wc < 0x7b1) return 0;
	else if (wc < 0x7b2) return 1;
	else if (wc < 0x7c0) return -1;
	else if (wc < 0x7eb) return 1;
	else if (wc < 0x7f4) return 0;
	else if (wc < 0x7fb) return 1;
	else if (wc < 0x901) return -1;
	else if (wc < 0x903) return 0;
	else if (wc < 0x93a) return 1;
	else if (wc < 0x93c) return -1;
	else if (wc < 0x93d) return 0;
	else if (wc < 0x941) return 1;
	else if (wc < 0x949) return 0;
	else if (wc < 0x94d) return 1;
	else if (wc < 0x94e) return 0;
	else if (wc < 0x950) return -1;
	else if (wc < 0x951) return 1;
	else if (wc < 0x955) return 0;
	else if (wc < 0x958) return -1;
	else if (wc < 0x962) return 1;
	else if (wc < 0x964) return 0;
	else if (wc < 0x973) return 1;
	else if (wc < 0x97b) return -1;
	else if (wc < 0x980) return 1;
	else if (wc < 0x981) return -1;
	else if (wc < 0x982) return 0;
	else if (wc < 0x984) return 1;
	else if (wc < 0x985) return -1;
	else if (wc < 0x98d) return 1;
	else if (wc < 0x98f) return -1;
	else if (wc < 0x991) return 1;
	else if (wc < 0x993) return -1;
	else if (wc < 0x9a9) return 1;
	else if (wc < 0x9aa) return -1;
	else if (wc < 0x9b1) return 1;
	else if (wc < 0x9b2) return -1;
	else if (wc < 0x9b3) return 1;
	else if (wc < 0x9b6) return -1;
	else if (wc < 0x9ba) return 1;
	else if (wc < 0x9bc) return -1;
	else if (wc < 0x9bd) return 0;
	else if (wc < 0x9c1) return 1;
	else if (wc < 0x9c5) return 0;
	else if (wc < 0x9c7) return -1;
	else if (wc < 0x9c9) return 1;
	else if (wc < 0x9cb) return -1;
	else if (wc < 0x9cd) return 1;
	else if (wc < 0x9ce) return 0;
	else if (wc < 0x9cf) return 1;
	else if (wc < 0x9d7) return -1;
	else if (wc < 0x9d8) return 1;
	else if (wc < 0x9dc) return -1;
	else if (wc < 0x9de) return 1;
	else if (wc < 0x9df) return -1;
	else if (wc < 0x9e2) return 1;
	else if (wc < 0x9e4) return 0;
	else if (wc < 0x9e6) return -1;
	else if (wc < 0x9fb) return 1;
	else if (wc < 0xa01) return -1;
	else if (wc < 0xa03) return 0;
	else if (wc < 0xa04) return 1;
	else if (wc < 0xa05) return -1;
	else if (wc < 0xa0b) return 1;
	else if (wc < 0xa0f) return -1;
	else if (wc < 0xa11) return 1;
	else if (wc < 0xa13) return -1;
	else if (wc < 0xa29) return 1;
	else if (wc < 0xa2a) return -1;
	else if (wc < 0xa31) return 1;
	else if (wc < 0xa32) return -1;
	else if (wc < 0xa34) return 1;
	else if (wc < 0xa35) return -1;
	else if (wc < 0xa37) return 1;
	else if (wc < 0xa38) return -1;
	else if (wc < 0xa3a) return 1;
	else if (wc < 0xa3c) return -1;
	else if (wc < 0xa3d) return 0;
	else if (wc < 0xa3e) return -1;
	else if (wc < 0xa41) return 1;
	else if (wc < 0xa43) return 0;
	else if (wc < 0xa47) return -1;
	else if (wc < 0xa49) return 0;
	else if (wc < 0xa4b) return -1;
	else if (wc < 0xa4e) return 0;
	else if (wc < 0xa51) return -1;
	else if (wc < 0xa52) return 1;
	else if (wc < 0xa59) return -1;
	else if (wc < 0xa5d) return 1;
	else if (wc < 0xa5e) return -1;
	else if (wc < 0xa5f) return 1;
	else if (wc < 0xa66) return -1;
	else if (wc < 0xa70) return 1;
	else if (wc < 0xa72) return 0;
	else if (wc < 0xa76) return 1;
	else if (wc < 0xa81) return -1;
	else if (wc < 0xa83) return 0;
	else if (wc < 0xa84) return 1;
	else if (wc < 0xa85) return -1;
	else if (wc < 0xa8e) return 1;
	else if (wc < 0xa8f) return -1;
	else if (wc < 0xa92) return 1;
	else if (wc < 0xa93) return -1;
	else if (wc < 0xaa9) return 1;
	else if (wc < 0xaaa) return -1;
	else if (wc < 0xab1) return 1;
	else if (wc < 0xab2) return -1;
	else if (wc < 0xab4) return 1;
	else if (wc < 0xab5) return -1;
	else if (wc < 0xaba) return 1;
	else if (wc < 0xabc) return -1;
	else if (wc < 0xabd) return 0;
	else if (wc < 0xac1) return 1;
	else if (wc < 0xac6) return 0;
	else if (wc < 0xac7) return -1;
	else if (wc < 0xac9) return 0;
	else if (wc < 0xaca) return 1;
	else if (wc < 0xacb) return -1;
	else if (wc < 0xacd) return 1;
	else if (wc < 0xace) return 0;
	else if (wc < 0xad0) return -1;
	else if (wc < 0xad1) return 1;
	else if (wc < 0xae0) return -1;
	else if (wc < 0xae2) return 1;
	else if (wc < 0xae4) return 0;
	else if (wc < 0xae6) return -1;
	else if (wc < 0xaf0) return 1;
	else if (wc < 0xaf1) return -1;
	else if (wc < 0xaf2) return 1;
	else if (wc < 0xb01) return -1;
	else if (wc < 0xb02) return 0;
	else if (wc < 0xb04) return 1;
	else if (wc < 0xb05) return -1;
	else if (wc < 0xb0d) return 1;
	else if (wc < 0xb0f) return -1;
	else if (wc < 0xb11) return 1;
	else if (wc < 0xb13) return -1;
	else if (wc < 0xb29) return 1;
	else if (wc < 0xb2a) return -1;
	else if (wc < 0xb31) return 1;
	else if (wc < 0xb32) return -1;
	else if (wc < 0xb34) return 1;
	else if (wc < 0xb35) return -1;
	else if (wc < 0xb3a) return 1;
	else if (wc < 0xb3c) return -1;
	else if (wc < 0xb3d) return 0;
	else if (wc < 0xb3f) return 1;
	else if (wc < 0xb40) return 0;
	else if (wc < 0xb41) return 1;
	else if (wc < 0xb44) return 0;
	else if (wc < 0xb45) return 1;
	else if (wc < 0xb47) return -1;
	else if (wc < 0xb49) return 1;
	else if (wc < 0xb4b) return -1;
	else if (wc < 0xb4d) return 1;
	else if (wc < 0xb4e) return 0;
	else if (wc < 0xb56) return -1;
	else if (wc < 0xb57) return 0;
	else if (wc < 0xb58) return 1;
	else if (wc < 0xb5c) return -1;
	else if (wc < 0xb5e) return 1;
	else if (wc < 0xb5f) return -1;
	else if (wc < 0xb64) return 1;
	else if (wc < 0xb66) return -1;
	else if (wc < 0xb72) return 1;
	else if (wc < 0xb82) return -1;
	else if (wc < 0xb83) return 0;
	else if (wc < 0xb84) return 1;
	else if (wc < 0xb85) return -1;
	else if (wc < 0xb8b) return 1;
	else if (wc < 0xb8e) return -1;
	else if (wc < 0xb91) return 1;
	else if (wc < 0xb92) return -1;
	else if (wc < 0xb96) return 1;
	else if (wc < 0xb99) return -1;
	else if (wc < 0xb9b) return 1;
	else if (wc < 0xb9c) return -1;
	else if (wc < 0xb9d) return 1;
	else if (wc < 0xb9e) return -1;
	else if (wc < 0xba0) return 1;
	else if (wc < 0xba3) return -1;
	else if (wc < 0xba5) return 1;
	else if (wc < 0xba8) return -1;
	else if (wc < 0xbab) return 1;
	else if (wc < 0xbae) return -1;
	else if (wc < 0xbba) return 1;
	else if (wc < 0xbbe) return -1;
	else if (wc < 0xbc0) return 1;
	else if (wc < 0xbc1) return 0;
	else if (wc < 0xbc3) return 1;
	else if (wc < 0xbc6) return -1;
	else if (wc < 0xbc9) return 1;
	else if (wc < 0xbca) return -1;
	else if (wc < 0xbcd) return 1;
	else if (wc < 0xbce) return 0;
	else if (wc < 0xbd0) return -1;
	else if (wc < 0xbd1) return 1;
	else if (wc < 0xbd7) return -1;
	else if (wc < 0xbd8) return 1;
	else if (wc < 0xbe6) return -1;
	else if (wc < 0xbfb) return 1;
	else if (wc < 0xc01) return -1;
	else if (wc < 0xc04) return 1;
	else if (wc < 0xc05) return -1;
	else if (wc < 0xc0d) return 1;
	else if (wc < 0xc0e) return -1;
	else if (wc < 0xc11) return 1;
	else if (wc < 0xc12) return -1;
	else if (wc < 0xc29) return 1;
	else if (wc < 0xc2a) return -1;
	else if (wc < 0xc34) return 1;
	else if (wc < 0xc35) return -1;
	else if (wc < 0xc3a) return 1;
	else if (wc < 0xc3d) return -1;
	else if (wc < 0xc3e) return 1;
	else if (wc < 0xc41) return 0;
	else if (wc < 0xc45) return 1;
	else if (wc < 0xc46) return -1;
	else if (wc < 0xc49) return 0;
	else if (wc < 0xc4a) return -1;
	else if (wc < 0xc4e) return 0;
	else if (wc < 0xc55) return -1;
	else if (wc < 0xc57) return 0;
	else if (wc < 0xc58) return -1;
	else if (wc < 0xc5a) return 1;
	else if (wc < 0xc60) return -1;
	else if (wc < 0xc64) return 1;
	else if (wc < 0xc66) return -1;
	else if (wc < 0xc70) return 1;
	else if (wc < 0xc78) return -1;
	else if (wc < 0xc80) return 1;
	else if (wc < 0xc82) return -1;
	else if (wc < 0xc84) return 1;
	else if (wc < 0xc85) return -1;
	else if (wc < 0xc8d) return 1;
	else if (wc < 0xc8e) return -1;
	else if (wc < 0xc91) return 1;
	else if (wc < 0xc92) return -1;
	else if (wc < 0xca9) return 1;
	else if (wc < 0xcaa) return -1;
	else if (wc < 0xcb4) return 1;
	else if (wc < 0xcb5) return -1;
	else if (wc < 0xcba) return 1;
	else if (wc < 0xcbc) return -1;
	else if (wc < 0xcbd) return 0;
	else if (wc < 0xcc5) return 1;
	else if (wc < 0xcc6) return -1;
	else if (wc < 0xcc9) return 1;
	else if (wc < 0xcca) return -1;
	else if (wc < 0xccc) return 1;
	else if (wc < 0xcce) return 0;
	else if (wc < 0xcd5) return -1;
	else if (wc < 0xcd7) return 1;
	else if (wc < 0xcde) return -1;
	else if (wc < 0xcdf) return 1;
	else if (wc < 0xce0) return -1;
	else if (wc < 0xce2) return 1;
	else if (wc < 0xce4) return 0;
	else if (wc < 0xce6) return -1;
	else if (wc < 0xcf0) return 1;
	else if (wc < 0xcf1) return -1;
	else if (wc < 0xcf3) return 1;
	else if (wc < 0xd02) return -1;
	else if (wc < 0xd04) return 1;
	else if (wc < 0xd05) return -1;
	else if (wc < 0xd0d) return 1;
	else if (wc < 0xd0e) return -1;
	else if (wc < 0xd11) return 1;
	else if (wc < 0xd12) return -1;
	else if (wc < 0xd29) return 1;
	else if (wc < 0xd2a) return -1;
	else if (wc < 0xd3a) return 1;
	else if (wc < 0xd3d) return -1;
	else if (wc < 0xd41) return 1;
	else if (wc < 0xd44) return 0;
	else if (wc < 0xd45) return 1;
	else if (wc < 0xd46) return -1;
	else if (wc < 0xd49) return 1;
	else if (wc < 0xd4a) return -1;
	else if (wc < 0xd4d) return 1;
	else if (wc < 0xd4e) return 0;
	else if (wc < 0xd57) return -1;
	else if (wc < 0xd58) return 1;
	else if (wc < 0xd60) return -1;
	else if (wc < 0xd64) return 1;
	else if (wc < 0xd66) return -1;
	else if (wc < 0xd76) return 1;
	else if (wc < 0xd79) return -1;
	else if (wc < 0xd80) return 1;
	else if (wc < 0xd82) return -1;
	else if (wc < 0xd84) return 1;
	else if (wc < 0xd85) return -1;
	else if (wc < 0xd97) return 1;
	else if (wc < 0xd9a) return -1;
	else if (wc < 0xdb2) return 1;
	else if (wc < 0xdb3) return -1;
	else if (wc < 0xdbc) return 1;
	else if (wc < 0xdbd) return -1;
	else if (wc < 0xdbe) return 1;
	else if (wc < 0xdc0) return -1;
	else if (wc < 0xdc7) return 1;
	else if (wc < 0xdca) return -1;
	else if (wc < 0xdcb) return 0;
	else if (wc < 0xdcf) return -1;
	else if (wc < 0xdd2) return 1;
	else if (wc < 0xdd5) return 0;
	else if (wc < 0xdd6) return -1;
	else if (wc < 0xdd7) return 0;
	else if (wc < 0xdd8) return -1;
	else if (wc < 0xde0) return 1;
	else if (wc < 0xdf2) return -1;
	else if (wc < 0xdf5) return 1;
	else if (wc < 0xe01) return -1;
	else if (wc < 0xe31) return 1;
	else if (wc < 0xe32) return 0;
	else if (wc < 0xe34) return 1;
	else if (wc < 0xe3b) return 0;
	else if (wc < 0xe3f) return -1;
	else if (wc < 0xe47) return 1;
	else if (wc < 0xe4f) return 0;
	else if (wc < 0xe5c) return 1;
	else if (wc < 0xe81) return -1;
	else if (wc < 0xe83) return 1;
	else if (wc < 0xe84) return -1;
	else if (wc < 0xe85) return 1;
	else if (wc < 0xe87) return -1;
	else if (wc < 0xe89) return 1;
	else if (wc < 0xe8a) return -1;
	else if (wc < 0xe8b) return 1;
	else if (wc < 0xe8d) return -1;
	else if (wc < 0xe8e) return 1;
	else if (wc < 0xe94) return -1;
	else if (wc < 0xe98) return 1;
	else if (wc < 0xe99) return -1;
	else if (wc < 0xea0) return 1;
	else if (wc < 0xea1) return -1;
	else if (wc < 0xea4) return 1;
	else if (wc < 0xea5) return -1;
	else if (wc < 0xea6) return 1;
	else if (wc < 0xea7) return -1;
	else if (wc < 0xea8) return 1;
	else if (wc < 0xeaa) return -1;
	else if (wc < 0xeac) return 1;
	else if (wc < 0xead) return -1;
	else if (wc < 0xeb1) return 1;
	else if (wc < 0xeb2) return 0;
	else if (wc < 0xeb4) return 1;
	else if (wc < 0xeba) return 0;
	else if (wc < 0xebb) return -1;
	else if (wc < 0xebd) return 0;
	else if (wc < 0xebe) return 1;
	else if (wc < 0xec0) return -1;
	else if (wc < 0xec5) return 1;
	else if (wc < 0xec6) return -1;
	else if (wc < 0xec7) return 1;
	else if (wc < 0xec8) return -1;
	else if (wc < 0xece) return 0;
	else if (wc < 0xed0) return -1;
	else if (wc < 0xeda) return 1;
	else if (wc < 0xedc) return -1;
	else if (wc < 0xede) return 1;
	else if (wc < 0xf00) return -1;
	else if (wc < 0xf18) return 1;
	else if (wc < 0xf1a) return 0;
	else if (wc < 0xf35) return 1;
	else if (wc < 0xf36) return 0;
	else if (wc < 0xf37) return 1;
	else if (wc < 0xf38) return 0;
	else if (wc < 0xf39) return 1;
	else if (wc < 0xf3a) return 0;
	else if (wc < 0xf48) return 1;
	else if (wc < 0xf49) return -1;
	else if (wc < 0xf6d) return 1;
	else if (wc < 0xf71) return -1;
	else if (wc < 0xf7f) return 0;
	else if (wc < 0xf80) return 1;
	else if (wc < 0xf85) return 0;
	else if (wc < 0xf86) return 1;
	else if (wc < 0xf88) return 0;
	else if (wc < 0xf8c) return 1;
	else if (wc < 0xf90) return -1;
	else if (wc < 0xf98) return 0;
	else if (wc < 0xf99) return -1;
	else if (wc < 0xfbd) return 0;
	else if (wc < 0xfbe) return -1;
	else if (wc < 0xfc6) return 1;
	else if (wc < 0xfc7) return 0;
	else if (wc < 0xfcd) return 1;
	else if (wc < 0xfce) return -1;
	else if (wc < 0xfd5) return 1;
	else if (wc < 0x1000) return -1;
	else if (wc < 0x102d) return 1;
	else if (wc < 0x1031) return 0;
	else if (wc < 0x1032) return 1;
	else if (wc < 0x1033) return 0;
	else if (wc < 0x1036) return 1;
	else if (wc < 0x1038) return 0;
	else if (wc < 0x1039) return 1;
	else if (wc < 0x103a) return 0;
	else if (wc < 0x1058) return 1;
	else if (wc < 0x105a) return 0;
	else if (wc < 0x109a) return 1;
	else if (wc < 0x109e) return -1;
	else if (wc < 0x10c6) return 1;
	else if (wc < 0x10d0) return -1;
	else if (wc < 0x10fd) return 1;
	else if (wc < 0x1100) return -1;
	else if (wc < 0x115a) return 2;
	else if (wc < 0x115f) return -1;
	else if (wc < 0x1160) return 2;
	else if (wc < 0x11a3) return 0;
	else if (wc < 0x11a8) return -1;
	else if (wc < 0x11fa) return 0;
	else if (wc < 0x1200) return -1;
	else if (wc < 0x1249) return 1;
	else if (wc < 0x124a) return -1;
	else if (wc < 0x124e) return 1;
	else if (wc < 0x1250) return -1;
	else if (wc < 0x1257) return 1;
	else if (wc < 0x1258) return -1;
	else if (wc < 0x1259) return 1;
	else if (wc < 0x125a) return -1;
	else if (wc < 0x125e) return 1;
	else if (wc < 0x1260) return -1;
	else if (wc < 0x1289) return 1;
	else if (wc < 0x128a) return -1;
	else if (wc < 0x128e) return 1;
	else if (wc < 0x1290) return -1;
	else if (wc < 0x12b1) return 1;
	else if (wc < 0x12b2) return -1;
	else if (wc < 0x12b6) return 1;
	else if (wc < 0x12b8) return -1;
	else if (wc < 0x12bf) return 1;
	else if (wc < 0x12c0) return -1;
	else if (wc < 0x12c1) return 1;
	else if (wc < 0x12c2) return -1;
	else if (wc < 0x12c6) return 1;
	else if (wc < 0x12c8) return -1;
	else if (wc < 0x12d7) return 1;
	else if (wc < 0x12d8) return -1;
	else if (wc < 0x1311) return 1;
	else if (wc < 0x1312) return -1;
	else if (wc < 0x1316) return 1;
	else if (wc < 0x1318) return -1;
	else if (wc < 0x135b) return 1;
	else if (wc < 0x135f) return -1;
	else if (wc < 0x1360) return 0;
	else if (wc < 0x137d) return 1;
	else if (wc < 0x1380) return -1;
	else if (wc < 0x139a) return 1;
	else if (wc < 0x13a0) return -1;
	else if (wc < 0x13f5) return 1;
	else if (wc < 0x1401) return -1;
	else if (wc < 0x1677) return 1;
	else if (wc < 0x1680) return -1;
	else if (wc < 0x169d) return 1;
	else if (wc < 0x16a0) return -1;
	else if (wc < 0x16f1) return 1;
	else if (wc < 0x1700) return -1;
	else if (wc < 0x170d) return 1;
	else if (wc < 0x170e) return -1;
	else if (wc < 0x1712) return 1;
	else if (wc < 0x1715) return 0;
	else if (wc < 0x1720) return -1;
	else if (wc < 0x1732) return 1;
	else if (wc < 0x1735) return 0;
	else if (wc < 0x1737) return 1;
	else if (wc < 0x1740) return -1;
	else if (wc < 0x1752) return 1;
	else if (wc < 0x1754) return 0;
	else if (wc < 0x1760) return -1;
	else if (wc < 0x176d) return 1;
	else if (wc < 0x176e) return -1;
	else if (wc < 0x1771) return 1;
	else if (wc < 0x1772) return -1;
	else if (wc < 0x1774) return 0;
	else if (wc < 0x1780) return -1;
	else if (wc < 0x17b4) return 1;
	else if (wc < 0x17b6) return 0;
	else if (wc < 0x17b7) return 1;
	else if (wc < 0x17be) return 0;
	else if (wc < 0x17c6) return 1;
	else if (wc < 0x17c7) return 0;
	else if (wc < 0x17c9) return 1;
	else if (wc < 0x17d4) return 0;
	else if (wc < 0x17dd) return 1;
	else if (wc < 0x17de) return 0;
	else if (wc < 0x17e0) return -1;
	else if (wc < 0x17ea) return 1;
	else if (wc < 0x17f0) return -1;
	else if (wc < 0x17fa) return 1;
	else if (wc < 0x1800) return -1;
	else if (wc < 0x180b) return 1;
	else if (wc < 0x180e) return 0;
	else if (wc < 0x180f) return 1;
	else if (wc < 0x1810) return -1;
	else if (wc < 0x181a) return 1;
	else if (wc < 0x1820) return -1;
	else if (wc < 0x1878) return 1;
	else if (wc < 0x1880) return -1;
	else if (wc < 0x18a9) return 1;
	else if (wc < 0x18aa) return 0;
	else if (wc < 0x18ab) return 1;
	else if (wc < 0x1900) return -1;
	else if (wc < 0x191d) return 1;
	else if (wc < 0x1920) return -1;
	else if (wc < 0x1923) return 0;
	else if (wc < 0x1927) return 1;
	else if (wc < 0x192c) return 0;
	else if (wc < 0x1930) return -1;
	else if (wc < 0x1932) return 1;
	else if (wc < 0x1933) return 0;
	else if (wc < 0x1939) return 1;
	else if (wc < 0x193c) return 0;
	else if (wc < 0x1940) return -1;
	else if (wc < 0x1941) return 1;
	else if (wc < 0x1944) return -1;
	else if (wc < 0x196e) return 1;
	else if (wc < 0x1970) return -1;
	else if (wc < 0x1975) return 1;
	else if (wc < 0x1980) return -1;
	else if (wc < 0x19aa) return 1;
	else if (wc < 0x19b0) return -1;
	else if (wc < 0x19ca) return 1;
	else if (wc < 0x19d0) return -1;
	else if (wc < 0x19da) return 1;
	else if (wc < 0x19de) return -1;
	else if (wc < 0x1a17) return 1;
	else if (wc < 0x1a19) return 0;
	else if (wc < 0x1a1c) return 1;
	else if (wc < 0x1a1e) return -1;
	else if (wc < 0x1a20) return 1;
	else if (wc < 0x1b00) return -1;
	else if (wc < 0x1b04) return 0;
	else if (wc < 0x1b34) return 1;
	else if (wc < 0x1b35) return 0;
	else if (wc < 0x1b36) return 1;
	else if (wc < 0x1b3b) return 0;
	else if (wc < 0x1b3c) return 1;
	else if (wc < 0x1b3d) return 0;
	else if (wc < 0x1b42) return 1;
	else if (wc < 0x1b43) return 0;
	else if (wc < 0x1b4c) return 1;
	else if (wc < 0x1b50) return -1;
	else if (wc < 0x1b6b) return 1;
	else if (wc < 0x1b74) return 0;
	else if (wc < 0x1b7d) return 1;
	else if (wc < 0x1b80) return -1;
	else if (wc < 0x1bab) return 1;
	else if (wc < 0x1bae) return -1;
	else if (wc < 0x1bba) return 1;
	else if (wc < 0x1c00) return -1;
	else if (wc < 0x1c38) return 1;
	else if (wc < 0x1c3b) return -1;
	else if (wc < 0x1c4a) return 1;
	else if (wc < 0x1c4d) return -1;
	else if (wc < 0x1c80) return 1;
	else if (wc < 0x1d00) return -1;
	else if (wc < 0x1dc0) return 1;
	else if (wc < 0x1dcb) return 0;
	else if (wc < 0x1de7) return 1;
	else if (wc < 0x1dfe) return -1;
	else if (wc < 0x1e00) return 0;
	else if (wc < 0x1f16) return 1;
	else if (wc < 0x1f18) return -1;
	else if (wc < 0x1f1e) return 1;
	else if (wc < 0x1f20) return -1;
	else if (wc < 0x1f46) return 1;
	else if (wc < 0x1f48) return -1;
	else if (wc < 0x1f4e) return 1;
	else if (wc < 0x1f50) return -1;
	else if (wc < 0x1f58) return 1;
	else if (wc < 0x1f59) return -1;
	else if (wc < 0x1f5a) return 1;
	else if (wc < 0x1f5b) return -1;
	else if (wc < 0x1f5c) return 1;
	else if (wc < 0x1f5d) return -1;
	else if (wc < 0x1f5e) return 1;
	else if (wc < 0x1f5f) return -1;
	else if (wc < 0x1f7e) return 1;
	else if (wc < 0x1f80) return -1;
	else if (wc < 0x1fb5) return 1;
	else if (wc < 0x1fb6) return -1;
	else if (wc < 0x1fc5) return 1;
	else if (wc < 0x1fc6) return -1;
	else if (wc < 0x1fd4) return 1;
	else if (wc < 0x1fd6) return -1;
	else if (wc < 0x1fdc) return 1;
	else if (wc < 0x1fdd) return -1;
	else if (wc < 0x1ff0) return 1;
	else if (wc < 0x1ff2) return -1;
	else if (wc < 0x1ff5) return 1;
	else if (wc < 0x1ff6) return -1;
	else if (wc < 0x1fff) return 1;
	else if (wc < 0x2000) return -1;
	else if (wc < 0x200b) return 1;
	else if (wc < 0x2010) return 0;
	else if (wc < 0x2028) return 1;
	else if (wc < 0x202a) return -1;
	else if (wc < 0x202f) return 0;
	else if (wc < 0x2060) return 1;
	else if (wc < 0x2064) return 0;
	else if (wc < 0x2065) return 1;
	else if (wc < 0x206a) return -1;
	else if (wc < 0x2070) return 0;
	else if (wc < 0x2072) return 1;
	else if (wc < 0x2074) return -1;
	else if (wc < 0x208f) return 1;
	else if (wc < 0x2090) return -1;
	else if (wc < 0x2095) return 1;
	else if (wc < 0x20a0) return -1;
	else if (wc < 0x20b6) return 1;
	else if (wc < 0x20d0) return -1;
	else if (wc < 0x20f0) return 0;
	else if (wc < 0x20f1) return 1;
	else if (wc < 0x2100) return -1;
	else if (wc < 0x2150) return 1;
	else if (wc < 0x2153) return -1;
	else if (wc < 0x2189) return 1;
	else if (wc < 0x2190) return -1;
	else if (wc < 0x2329) return 1;
	else if (wc < 0x232b) return 2;
	else if (wc < 0x23e8) return 1;
	else if (wc < 0x2400) return -1;
	else if (wc < 0x2427) return 1;
	else if (wc < 0x2440) return -1;
	else if (wc < 0x244b) return 1;
	else if (wc < 0x2460) return -1;
	else if (wc < 0x269e) return 1;
	else if (wc < 0x26a0) return -1;
	else if (wc < 0x26c4) return 1;
	else if (wc < 0x2701) return -1;
	else if (wc < 0x2705) return 1;
	else if (wc < 0x2706) return -1;
	else if (wc < 0x270a) return 1;
	else if (wc < 0x270c) return -1;
	else if (wc < 0x2728) return 1;
	else if (wc < 0x2729) return -1;
	else if (wc < 0x274c) return 1;
	else if (wc < 0x274d) return -1;
	else if (wc < 0x274e) return 1;
	else if (wc < 0x274f) return -1;
	else if (wc < 0x2753) return 1;
	else if (wc < 0x2756) return -1;
	else if (wc < 0x2757) return 1;
	else if (wc < 0x2758) return -1;
	else if (wc < 0x275f) return 1;
	else if (wc < 0x2761) return -1;
	else if (wc < 0x2795) return 1;
	else if (wc < 0x2798) return -1;
	else if (wc < 0x27b0) return 1;
	else if (wc < 0x27b1) return -1;
	else if (wc < 0x27bf) return 1;
	else if (wc < 0x27c0) return -1;
	else if (wc < 0x27cb) return 1;
	else if (wc < 0x27cc) return -1;
	else if (wc < 0x27cd) return 1;
	else if (wc < 0x27d0) return -1;
	else if (wc < 0x2b4d) return 1;
	else if (wc < 0x2b50) return -1;
	else if (wc < 0x2b55) return 1;
	else if (wc < 0x2c00) return -1;
	else if (wc < 0x2c2f) return 1;
	else if (wc < 0x2c30) return -1;
	else if (wc < 0x2c5f) return 1;
	else if (wc < 0x2c60) return -1;
	else if (wc < 0x2c70) return 1;
	else if (wc < 0x2c71) return -1;
	else if (wc < 0x2c7e) return 1;
	else if (wc < 0x2c80) return -1;
	else if (wc < 0x2ceb) return 1;
	else if (wc < 0x2cf9) return -1;
	else if (wc < 0x2d26) return 1;
	else if (wc < 0x2d30) return -1;
	else if (wc < 0x2d66) return 1;
	else if (wc < 0x2d6f) return -1;
	else if (wc < 0x2d70) return 1;
	else if (wc < 0x2d80) return -1;
	else if (wc < 0x2d97) return 1;
	else if (wc < 0x2da0) return -1;
	else if (wc < 0x2da7) return 1;
	else if (wc < 0x2da8) return -1;
	else if (wc < 0x2daf) return 1;
	else if (wc < 0x2db0) return -1;
	else if (wc < 0x2db7) return 1;
	else if (wc < 0x2db8) return -1;
	else if (wc < 0x2dbf) return 1;
	else if (wc < 0x2dc0) return -1;
	else if (wc < 0x2dc7) return 1;
	else if (wc < 0x2dc8) return -1;
	else if (wc < 0x2dcf) return 1;
	else if (wc < 0x2dd0) return -1;
	else if (wc < 0x2dd7) return 1;
	else if (wc < 0x2dd8) return -1;
	else if (wc < 0x2ddf) return 1;
	else if (wc < 0x2de0) return -1;
	else if (wc < 0x2e31) return 1;
	else if (wc < 0x2e80) return -1;
	else if (wc < 0x2e9a) return 2;
	else if (wc < 0x2e9b) return -1;
	else if (wc < 0x2ef4) return 2;
	else if (wc < 0x2f00) return -1;
	else if (wc < 0x2fd6) return 2;
	else if (wc < 0x2ff0) return -1;
	else if (wc < 0x2ffc) return 2;
	else if (wc < 0x3000) return -1;
	else if (wc < 0x302a) return 2;
	else if (wc < 0x3030) return 0;
	else if (wc < 0x303f) return 2;
	else if (wc < 0x3040) return 1;
	else if (wc < 0x3041) return -1;
	else if (wc < 0x3097) return 2;
	else if (wc < 0x3099) return -1;
	else if (wc < 0x309b) return 0;
	else if (wc < 0x3100) return 2;
	else if (wc < 0x3105) return -1;
	else if (wc < 0x312e) return 2;
	else if (wc < 0x3131) return -1;
	else if (wc < 0x318f) return 2;
	else if (wc < 0x3190) return -1;
	else if (wc < 0x31b8) return 2;
	else if (wc < 0x31c0) return -1;
	else if (wc < 0x31d0) return 2;
	else if (wc < 0x31f0) return -1;
	else if (wc < 0x321f) return 2;
	else if (wc < 0x3220) return -1;
	else if (wc < 0x3244) return 2;
	else if (wc < 0x3250) return -1;
	else if (wc < 0x32ff) return 2;
	else if (wc < 0x3300) return -1;
	else if (wc < 0x4db6) return 2;
	else if (wc < 0x4dc0) return -1;
	else if (wc < 0x9fbc) return 2;
	else if (wc < 0xa000) return -1;
	else if (wc < 0xa48d) return 2;
	else if (wc < 0xa490) return -1;
	else if (wc < 0xa4c7) return 2;
	else if (wc < 0xa500) return -1;
	else if (wc < 0xa62c) return 1;
	else if (wc < 0xa640) return -1;
	else if (wc < 0xa660) return 1;
	else if (wc < 0xa662) return -1;
	else if (wc < 0xa674) return 1;
	else if (wc < 0xa67c) return -1;
	else if (wc < 0xa698) return 1;
	else if (wc < 0xa700) return -1;
	else if (wc < 0xa78d) return 1;
	else if (wc < 0xa7fb) return -1;
	else if (wc < 0xa802) return 1;
	else if (wc < 0xa803) return 0;
	else if (wc < 0xa806) return 1;
	else if (wc < 0xa807) return 0;
	else if (wc < 0xa80b) return 1;
	else if (wc < 0xa80c) return 0;
	else if (wc < 0xa825) return 1;
	else if (wc < 0xa827) return 0;
	else if (wc < 0xa82c) return 1;
	else if (wc < 0xa840) return -1;
	else if (wc < 0xa878) return 1;
	else if (wc < 0xa880) return -1;
	else if (wc < 0xa8c5) return 1;
	else if (wc < 0xa8ce) return -1;
	else if (wc < 0xa8da) return 1;
	else if (wc < 0xa900) return -1;
	else if (wc < 0xa954) return 1;
	else if (wc < 0xa95f) return -1;
	else if (wc < 0xa960) return 1;
	else if (wc < 0xaa00) return -1;
	else if (wc < 0xaa37) return 1;
	else if (wc < 0xaa40) return -1;
	else if (wc < 0xaa4e) return 1;
	else if (wc < 0xaa50) return -1;
	else if (wc < 0xaa5a) return 1;
	else if (wc < 0xaa5c) return -1;
	else if (wc < 0xaa60) return 1;
	else if (wc < 0xac00) return -1;
	else if (wc < 0xd7a4) return 2;
	else if (wc < 0xe000) return -1;
	else if (wc < 0xf8f0) return 1;
	else if (wc < 0xf900) return 0;
	else if (wc < 0xfa2e) return 2;
	else if (wc < 0xfa30) return -1;
	else if (wc < 0xfa6b) return 2;
	else if (wc < 0xfa70) return -1;
	else if (wc < 0xfada) return 2;
	else if (wc < 0xfb00) return -1;
	else if (wc < 0xfb07) return 1;
	else if (wc < 0xfb13) return -1;
	else if (wc < 0xfb18) return 1;
	else if (wc < 0xfb1d) return -1;
	else if (wc < 0xfb1e) return 1;
	else if (wc < 0xfb1f) return 0;
	else if (wc < 0xfb37) return 1;
	else if (wc < 0xfb38) return -1;
	else if (wc < 0xfb3d) return 1;
	else if (wc < 0xfb3e) return -1;
	else if (wc < 0xfb3f) return 1;
	else if (wc < 0xfb40) return -1;
	else if (wc < 0xfb42) return 1;
	else if (wc < 0xfb43) return -1;
	else if (wc < 0xfb45) return 1;
	else if (wc < 0xfb46) return -1;
	else if (wc < 0xfbb2) return 1;
	else if (wc < 0xfbd3) return -1;
	else if (wc < 0xfd40) return 1;
	else if (wc < 0xfd50) return -1;
	else if (wc < 0xfd90) return 1;
	else if (wc < 0xfd92) return -1;
	else if (wc < 0xfdc8) return 1;
	else if (wc < 0xfdf0) return -1;
	else if (wc < 0xfdfe) return 1;
	else if (wc < 0xfe00) return -1;
	else if (wc < 0xfe10) return 0;
	else if (wc < 0xfe1a) return 2;
	else if (wc < 0xfe20) return -1;
	else if (wc < 0xfe24) return 0;
	else if (wc < 0xfe27) return 1;
	else if (wc < 0xfe30) return -1;
	else if (wc < 0xfe53) return 2;
	else if (wc < 0xfe54) return -1;
	else if (wc < 0xfe67) return 2;
	else if (wc < 0xfe68) return -1;
	else if (wc < 0xfe6c) return 2;
	else if (wc < 0xfe70) return -1;
	else if (wc < 0xfe75) return 1;
	else if (wc < 0xfe76) return -1;
	else if (wc < 0xfefd) return 1;
	else if (wc < 0xfeff) return -1;
	else if (wc < 0xff00) return 0;
	else if (wc < 0xff01) return -1;
	else if (wc < 0xff61) return 2;
	else if (wc < 0xffbf) return 1;
	else if (wc < 0xffc2) return -1;
	else if (wc < 0xffc8) return 1;
	else if (wc < 0xffca) return -1;
	else if (wc < 0xffd0) return 1;
	else if (wc < 0xffd2) return -1;
	else if (wc < 0xffd8) return 1;
	else if (wc < 0xffda) return -1;
	else if (wc < 0xffdd) return 1;
	else if (wc < 0xffe0) return -1;
	else if (wc < 0xffe7) return 2;
	else if (wc < 0xffe8) return -1;
	else if (wc < 0xffef) return 1;
	else if (wc < 0xfff9) return -1;
	else if (wc < 0xfffc) return 0;
	else if (wc < 0xfffe) return 1;
	else if (wc < 0x10000) return -1;
	else if (wc < 0x1000c) return 1;
	else if (wc < 0x1000d) return -1;
	else if (wc < 0x10027) return 1;
	else if (wc < 0x10028) return -1;
	else if (wc < 0x1003b) return 1;
	else if (wc < 0x1003c) return -1;
	else if (wc < 0x1003e) return 1;
	else if (wc < 0x1003f) return -1;
	else if (wc < 0x1004e) return 1;
	else if (wc < 0x10050) return -1;
	else if (wc < 0x1005e) return 1;
	else if (wc < 0x10080) return -1;
	else if (wc < 0x100fb) return 1;
	else if (wc < 0x10100) return -1;
	else if (wc < 0x10103) return 1;
	else if (wc < 0x10107) return -1;
	else if (wc < 0x10134) return 1;
	else if (wc < 0x10137) return -1;
	else if (wc < 0x1018b) return 1;
	else if (wc < 0x10190) return -1;
	else if (wc < 0x1019c) return 1;
	else if (wc < 0x101d0) return -1;
	else if (wc < 0x101fe) return 1;
	else if (wc < 0x10280) return -1;
	else if (wc < 0x1029d) return 1;
	else if (wc < 0x102a0) return -1;
	else if (wc < 0x102d1) return 1;
	else if (wc < 0x10300) return -1;
	else if (wc < 0x1031f) return 1;
	else if (wc < 0x10320) return -1;
	else if (wc < 0x10324) return 1;
	else if (wc < 0x10330) return -1;
	else if (wc < 0x1034b) return 1;
	else if (wc < 0x10380) return -1;
	else if (wc < 0x1039e) return 1;
	else if (wc < 0x1039f) return -1;
	else if (wc < 0x103c4) return 1;
	else if (wc < 0x103c8) return -1;
	else if (wc < 0x103d6) return 1;
	else if (wc < 0x10400) return -1;
	else if (wc < 0x1049e) return 1;
	else if (wc < 0x104a0) return -1;
	else if (wc < 0x104aa) return 1;
	else if (wc < 0x10800) return -1;
	else if (wc < 0x10806) return 1;
	else if (wc < 0x10808) return -1;
	else if (wc < 0x10809) return 1;
	else if (wc < 0x1080a) return -1;
	else if (wc < 0x10836) return 1;
	else if (wc < 0x10837) return -1;
	else if (wc < 0x10839) return 1;
	else if (wc < 0x1083c) return -1;
	else if (wc < 0x1083d) return 1;
	else if (wc < 0x1083f) return -1;
	else if (wc < 0x10840) return 1;
	else if (wc < 0x10900) return -1;
	else if (wc < 0x1091a) return 1;
	else if (wc < 0x1091f) return -1;
	else if (wc < 0x10920) return 1;
	else if (wc < 0x10a00) return -1;
	else if (wc < 0x10a01) return 1;
	else if (wc < 0x10a04) return 0;
	else if (wc < 0x10a05) return -1;
	else if (wc < 0x10a07) return 0;
	else if (wc < 0x10a0c) return -1;
	else if (wc < 0x10a10) return 0;
	else if (wc < 0x10a14) return 1;
	else if (wc < 0x10a15) return -1;
	else if (wc < 0x10a18) return 1;
	else if (wc < 0x10a19) return -1;
	else if (wc < 0x10a34) return 1;
	else if (wc < 0x10a38) return -1;
	else if (wc < 0x10a3b) return 0;
	else if (wc < 0x10a3f) return -1;
	else if (wc < 0x10a40) return 0;
	else if (wc < 0x10a48) return 1;
	else if (wc < 0x10a50) return -1;
	else if (wc < 0x10a59) return 1;
	else if (wc < 0x12000) return -1;
	else if (wc < 0x1236f) return 1;
	else if (wc < 0x12400) return -1;
	else if (wc < 0x12463) return 1;
	else if (wc < 0x12470) return -1;
	else if (wc < 0x12474) return 1;
	else if (wc < 0x1d000) return -1;
	else if (wc < 0x1d0f6) return 1;
	else if (wc < 0x1d100) return -1;
	else if (wc < 0x1d127) return 1;
	else if (wc < 0x1d129) return -1;
	else if (wc < 0x1d167) return 1;
	else if (wc < 0x1d16a) return 0;
	else if (wc < 0x1d173) return 1;
	else if (wc < 0x1d183) return 0;
	else if (wc < 0x1d185) return 1;
	else if (wc < 0x1d18c) return 0;
	else if (wc < 0x1d1aa) return 1;
	else if (wc < 0x1d1ae) return 0;
	else if (wc < 0x1d1de) return 1;
	else if (wc < 0x1d200) return -1;
	else if (wc < 0x1d242) return 1;
	else if (wc < 0x1d245) return 0;
	else if (wc < 0x1d246) return 1;
	else if (wc < 0x1d300) return -1;
	else if (wc < 0x1d357) return 1;
	else if (wc < 0x1d360) return -1;
	else if (wc < 0x1d372) return 1;
	else if (wc < 0x1d400) return -1;
	else if (wc < 0x1d455) return 1;
	else if (wc < 0x1d456) return -1;
	else if (wc < 0x1d49d) return 1;
	else if (wc < 0x1d49e) return -1;
	else if (wc < 0x1d4a0) return 1;
	else if (wc < 0x1d4a2) return -1;
	else if (wc < 0x1d4a3) return 1;
	else if (wc < 0x1d4a5) return -1;
	else if (wc < 0x1d4a7) return 1;
	else if (wc < 0x1d4a9) return -1;
	else if (wc < 0x1d4ad) return 1;
	else if (wc < 0x1d4ae) return -1;
	else if (wc < 0x1d4ba) return 1;
	else if (wc < 0x1d4bb) return -1;
	else if (wc < 0x1d4bc) return 1;
	else if (wc < 0x1d4bd) return -1;
	else if (wc < 0x1d4c4) return 1;
	else if (wc < 0x1d4c5) return -1;
	else if (wc < 0x1d506) return 1;
	else if (wc < 0x1d507) return -1;
	else if (wc < 0x1d50b) return 1;
	else if (wc < 0x1d50d) return -1;
	else if (wc < 0x1d515) return 1;
	else if (wc < 0x1d516) return -1;
	else if (wc < 0x1d51d) return 1;
	else if (wc < 0x1d51e) return -1;
	else if (wc < 0x1d53a) return 1;
	else if (wc < 0x1d53b) return -1;
	else if (wc < 0x1d53f) return 1;
	else if (wc < 0x1d540) return -1;
	else if (wc < 0x1d545) return 1;
	else if (wc < 0x1d546) return -1;
	else if (wc < 0x1d547) return 1;
	else if (wc < 0x1d54a) return -1;
	else if (wc < 0x1d551) return 1;
	else if (wc < 0x1d552) return -1;
	else if (wc < 0x1d6a6) return 1;
	else if (wc < 0x1d6a8) return -1;
	else if (wc < 0x1d7cc) return 1;
	else if (wc < 0x1d7ce) return -1;
	else if (wc < 0x1d800) return 1;
	else if (wc < 0x1f000) return -1;
	else if (wc < 0x1f02c) return 1;
	else if (wc < 0x1f030) return -1;
	else if (wc < 0x1f094) return 1;
	else if (wc < 0x20000) return -1;
	else if (wc < 0x2a6d7) return 2;
	else if (wc < 0x2f800) return -1;
	else if (wc < 0x2fa1e) return 2;
	else if (wc < 0xe0001) return -1;
	else if (wc < 0xe0002) return 0;
	else if (wc < 0xe0020) return -1;
	else if (wc < 0xe0080) return 0;
	else if (wc < 0xe0100) return -1;
	else if (wc < 0xe01f0) return 0;
	else if (wc < 0xf0000) return -1;
	else if (wc < 0xffffe) return 1;
	else if (wc < 0x100000) return -1;
	else if (wc < 0x10fffe) return 1;
	else if (wc < 0x110000) return -1;
	return -1;
}

UINT32 vga_colors[PALETTE_COLORS] = {
	0x0,
	0x1,
	0x2,
	0x3,
	0x4,
	0x5,
	0x6,
	0x7,
	0x8,
	0x9,
	0xa,
	0xb,
	0xc,
	0xd,
	0xe,
	0xf,
	0x0, /* #000000 -> #000000 */
	0x4, /* #00005f -> #0000aa */
	0x4, /* #000087 -> #0000aa */
	0x4, /* #0000af -> #0000aa */
	0x4, /* #0000d7 -> #0000aa */
	0xc, /* #0000ff -> #5555ff */
	0x2, /* #005f00 -> #00aa00 */
	0x8, /* #005f5f -> #555555 */
	0x6, /* #005f87 -> #00aaaa */
	0x6, /* #005faf -> #00aaaa */
	0xc, /* #005fd7 -> #5555ff */
	0xc, /* #005fff -> #5555ff */
	0x2, /* #008700 -> #00aa00 */
	0xa, /* #00875f -> #55aa55 */
	0x6, /* #008787 -> #00aaaa */
	0x6, /* #0087af -> #00aaaa */
	0x6, /* #0087d7 -> #00aaaa */
	0xc, /* #0087ff -> #5555ff */
	0x2, /* #00af00 -> #00aa00 */
	0xa, /* #00af5f -> #55aa55 */
	0x6, /* #00af87 -> #00aaaa */
	0x6, /* #00afaf -> #00aaaa */
	0x6, /* #00afd7 -> #00aaaa */
	0xe, /* #00afff -> #55ffff */
	0x2, /* #00d700 -> #00aa00 */
	0xa, /* #00d75f -> #55aa55 */
	0x6, /* #00d787 -> #00aaaa */
	0x6, /* #00d7af -> #00aaaa */
	0x6, /* #00d7d7 -> #00aaaa */
	0xe, /* #00d7ff -> #55ffff */
	0x2, /* #00ff00 -> #00aa00 */
	0xa, /* #00ff5f -> #55aa55 */
	0x6, /* #00ff87 -> #00aaaa */
	0x6, /* #00ffaf -> #00aaaa */
	0xe, /* #00ffd7 -> #55ffff */
	0xe, /* #00ffff -> #55ffff */
	0x1, /* #5f0000 -> #aa0000 */
	0x8, /* #5f005f -> #555555 */
	0x5, /* #5f0087 -> #aa00aa */
	0x5, /* #5f00af -> #aa00aa */
	0x5, /* #5f00d7 -> #aa00aa */
	0xc, /* #5f00ff -> #5555ff */
	0x3, /* #5f5f00 -> #aa5500 */
	0x8, /* #5f5f5f -> #555555 */
	0x8, /* #5f5f87 -> #555555 */
	0x7, /* #5f5faf -> #aaaaaa */
	0xc, /* #5f5fd7 -> #5555ff */
	0xc, /* #5f5fff -> #5555ff */
	0x2, /* #5f8700 -> #00aa00 */
	0xa, /* #5f875f -> #55aa55 */
	0xa, /* #5f8787 -> #55aa55 */
	0x7, /* #5f87af -> #aaaaaa */
	0xc, /* #5f87d7 -> #5555ff */
	0xc, /* #5f87ff -> #5555ff */
	0x2, /* #5faf00 -> #00aa00 */
	0xa, /* #5faf5f -> #55aa55 */
	0xa, /* #5faf87 -> #55aa55 */
	0x7, /* #5fafaf -> #aaaaaa */
	0x7, /* #5fafd7 -> #aaaaaa */
	0xe, /* #5fafff -> #55ffff */
	0x2, /* #5fd700 -> #00aa00 */
	0xa, /* #5fd75f -> #55aa55 */
	0xa, /* #5fd787 -> #55aa55 */
	0x7, /* #5fd7af -> #aaaaaa */
	0xe, /* #5fd7d7 -> #55ffff */
	0xe, /* #5fd7ff -> #55ffff */
	0x2, /* #5fff00 -> #00aa00 */
	0xb, /* #5fff5f -> #ffff55 */
	0xb, /* #5fff87 -> #ffff55 */
	0x7, /* #5fffaf -> #aaaaaa */
	0xe, /* #5fffd7 -> #55ffff */
	0xe, /* #5fffff -> #55ffff */
	0x1, /* #870000 -> #aa0000 */
	0x8, /* #87005f -> #555555 */
	0x5, /* #870087 -> #aa00aa */
	0x5, /* #8700af -> #aa00aa */
	0x5, /* #8700d7 -> #aa00aa */
	0xc, /* #8700ff -> #5555ff */
	0x3, /* #875f00 -> #aa5500 */
	0x8, /* #875f5f -> #555555 */
	0x8, /* #875f87 -> #555555 */
	0x7, /* #875faf -> #aaaaaa */
	0xc, /* #875fd7 -> #5555ff */
	0xc, /* #875fff -> #5555ff */
	0x3, /* #878700 -> #aa5500 */
	0xa, /* #87875f -> #55aa55 */
	0x7, /* #878787 -> #aaaaaa */
	0x7, /* #8787af -> #aaaaaa */
	0x7, /* #8787d7 -> #aaaaaa */
	0xc, /* #8787ff -> #5555ff */
	0x2, /* #87af00 -> #00aa00 */
	0xa, /* #87af5f -> #55aa55 */
	0x7, /* #87af87 -> #aaaaaa */
	0x7, /* #87afaf -> #aaaaaa */
	0x7, /* #87afd7 -> #aaaaaa */
	0xe, /* #87afff -> #55ffff */
	0x2, /* #87d700 -> #00aa00 */
	0xa, /* #87d75f -> #55aa55 */
	0x7, /* #87d787 -> #aaaaaa */
	0x7, /* #87d7af -> #aaaaaa */
	0xe, /* #87d7d7 -> #55ffff */
	0xe, /* #87d7ff -> #55ffff */
	0x2, /* #87ff00 -> #00aa00 */
	0xb, /* #87ff5f -> #ffff55 */
	0xb, /* #87ff87 -> #ffff55 */
	0x7, /* #87ffaf -> #aaaaaa */
	0xe, /* #87ffd7 -> #55ffff */
	0xe, /* #87ffff -> #55ffff */
	0x1, /* #af0000 -> #aa0000 */
	0x5, /* #af005f -> #aa00aa */
	0x5, /* #af0087 -> #aa00aa */
	0x5, /* #af00af -> #aa00aa */
	0x5, /* #af00d7 -> #aa00aa */
	0xd, /* #af00ff -> #ff55ff */
	0x3, /* #af5f00 -> #aa5500 */
	0x9, /* #af5f5f -> #ff5555 */
	0x9, /* #af5f87 -> #ff5555 */
	0x7, /* #af5faf -> #aaaaaa */
	0xd, /* #af5fd7 -> #ff55ff */
	0xd, /* #af5fff -> #ff55ff */
	0x3, /* #af8700 -> #aa5500 */
	0xa, /* #af875f -> #55aa55 */
	0x7, /* #af8787 -> #aaaaaa */
	0x7, /* #af87af -> #aaaaaa */
	0x7, /* #af87d7 -> #aaaaaa */
	0xd, /* #af87ff -> #ff55ff */
	0x2, /* #afaf00 -> #00aa00 */
	0xa, /* #afaf5f -> #55aa55 */
	0x7, /* #afaf87 -> #aaaaaa */
	0x7, /* #afafaf -> #aaaaaa */
	0x7, /* #afafd7 -> #aaaaaa */
	0xf, /* #afafff -> #ffffff */
	0x2, /* #afd700 -> #00aa00 */
	0xb, /* #afd75f -> #ffff55 */
	0x7, /* #afd787 -> #aaaaaa */
	0x7, /* #afd7af -> #aaaaaa */
	0x7, /* #afd7d7 -> #aaaaaa */
	0xf, /* #afd7ff -> #ffffff */
	0x2, /* #afff00 -> #00aa00 */
	0xb, /* #afff5f -> #ffff55 */
	0xb, /* #afff87 -> #ffff55 */
	0x7, /* #afffaf -> #aaaaaa */
	0xf, /* #afffd7 -> #ffffff */
	0xf, /* #afffff -> #ffffff */
	0x1, /* #d70000 -> #aa0000 */
	0x9, /* #d7005f -> #ff5555 */
	0x5, /* #d70087 -> #aa00aa */
	0x5, /* #d700af -> #aa00aa */
	0x5, /* #d700d7 -> #aa00aa */
	0xd, /* #d700ff -> #ff55ff */
	0x3, /* #d75f00 -> #aa5500 */
	0x9, /* #d75f5f -> #ff5555 */
	0x9, /* #d75f87 -> #ff5555 */
	0x7, /* #d75faf -> #aaaaaa */
	0xd, /* #d75fd7 -> #ff55ff */
	0xd, /* #d75fff -> #ff55ff */
	0x3, /* #d78700 -> #aa5500 */
	0x9, /* #d7875f -> #ff5555 */
	0x7, /* #d78787 -> #aaaaaa */
	0x7, /* #d787af -> #aaaaaa */
	0x7, /* #d787d7 -> #aaaaaa */
	0xd, /* #d787ff -> #ff55ff */
	0x2, /* #d7af00 -> #00aa00 */
	0xa, /* #d7af5f -> #55aa55 */
	0x7, /* #d7af87 -> #aaaaaa */
	0x7, /* #d7afaf -> #aaaaaa */
	0x7, /* #d7afd7 -> #aaaaaa */
	0xf, /* #d7afff -> #ffffff */
	0x2, /* #d7d700 -> #00aa00 */
	0xb, /* #d7d75f -> #ffff55 */
	0x7, /* #d7d787 -> #aaaaaa */
	0x7, /* #d7d7af -> #aaaaaa */
	0xf, /* #d7d7d7 -> #ffffff */
	0xf, /* #d7d7ff -> #ffffff */
	0xb, /* #d7ff00 -> #ffff55 */
	0xb, /* #d7ff5f -> #ffff55 */
	0xb, /* #d7ff87 -> #ffff55 */
	0x7, /* #d7ffaf -> #aaaaaa */
	0xf, /* #d7ffd7 -> #ffffff */
	0xf, /* #d7ffff -> #ffffff */
	0x1, /* #ff0000 -> #aa0000 */
	0x9, /* #ff005f -> #ff5555 */
	0x5, /* #ff0087 -> #aa00aa */
	0x5, /* #ff00af -> #aa00aa */
	0x5, /* #ff00d7 -> #aa00aa */
	0xd, /* #ff00ff -> #ff55ff */
	0x3, /* #ff5f00 -> #aa5500 */
	0x9, /* #ff5f5f -> #ff5555 */
	0x9, /* #ff5f87 -> #ff5555 */
	0x7, /* #ff5faf -> #aaaaaa */
	0xd, /* #ff5fd7 -> #ff55ff */
	0xd, /* #ff5fff -> #ff55ff */
	0x3, /* #ff8700 -> #aa5500 */
	0x9, /* #ff875f -> #ff5555 */
	0x9, /* #ff8787 -> #ff5555 */
	0x7, /* #ff87af -> #aaaaaa */
	0xd, /* #ff87d7 -> #ff55ff */
	0xd, /* #ff87ff -> #ff55ff */
	0x2, /* #ffaf00 -> #00aa00 */
	0xb, /* #ffaf5f -> #ffff55 */
	0x7, /* #ffaf87 -> #aaaaaa */
	0x7, /* #ffafaf -> #aaaaaa */
	0x7, /* #ffafd7 -> #aaaaaa */
	0xf, /* #ffafff -> #ffffff */
	0x2, /* #ffd700 -> #00aa00 */
	0xb, /* #ffd75f -> #ffff55 */
	0xb, /* #ffd787 -> #ffff55 */
	0x7, /* #ffd7af -> #aaaaaa */
	0xf, /* #ffd7d7 -> #ffffff */
	0xf, /* #ffd7ff -> #ffffff */
	0xb, /* #ffff00 -> #ffff55 */
	0xb, /* #ffff5f -> #ffff55 */
	0xb, /* #ffff87 -> #ffff55 */
	0xf, /* #ffffaf -> #ffffff */
	0xf, /* #ffffd7 -> #ffffff */
	0xf, /* #ffffff -> #ffffff */
	0x0, /* #080808 -> #000000 */
	0x0, /* #121212 -> #000000 */
	0x0, /* #1c1c1c -> #000000 */
	0x0, /* #262626 -> #000000 */
	0x8, /* #303030 -> #555555 */
	0x8, /* #3a3a3a -> #555555 */
	0x8, /* #444444 -> #555555 */
	0x8, /* #4e4e4e -> #555555 */
	0x8, /* #585858 -> #555555 */
	0x8, /* #626262 -> #555555 */
	0x8, /* #6c6c6c -> #555555 */
	0x8, /* #767676 -> #555555 */
	0x7, /* #808080 -> #aaaaaa */
	0x7, /* #8a8a8a -> #aaaaaa */
	0x7, /* #949494 -> #aaaaaa */
	0x7, /* #9e9e9e -> #aaaaaa */
	0x7, /* #a8a8a8 -> #aaaaaa */
	0x7, /* #b2b2b2 -> #aaaaaa */
	0x7, /* #bcbcbc -> #aaaaaa */
	0x7, /* #c6c6c6 -> #aaaaaa */
	0x7, /* #d0d0d0 -> #aaaaaa */
	0xf, /* #dadada -> #ffffff */
	0xf, /* #e4e4e4 -> #ffffff */
	0xf, /* #eeeeee -> #ffffff */
};
