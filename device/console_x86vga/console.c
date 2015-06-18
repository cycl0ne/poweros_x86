/**
 * @file console.c
 *
 * ANSI Handling for a console.device
 * This file is not usable ALONE. Its only a skeleton for handling with ESC chars
 */
/* Original Idea by  Kevin Lange of ToaruOS, rewritten/designed by Claus Herrmann*/
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "console_device.h"
#include "console.h"

#define MAX_ARGS 1024

static UINT32 strlen(const char *str) {
	int i = 0;
	while (str[i] != (char)0) ++i;
	return i;
}

static void _prbuf(INT32 ch, INT32 *str)
{
	UINT8* buffer = (UINT8*)str[0];
	*buffer++ = (char)ch;
	str[0] = (INT32)buffer;
}

static int sprintf(term_state_t * s, char* str, char *fmt,...)
{
	APTR SysBase = s->unit->con_SysBase;
	INT32	args[1];
	args[0] = (INT32) str;

	va_list pvar;
	va_start(pvar, fmt);
	RawDoFmt((const char *)fmt, pvar,(void(*)()) _prbuf, (APTR) args);
	return strlen((STRPTR)str);
}

static int atoi(const char * str) 
{
	UINT32 len = strlen(str);
	UINT32 out = 0;
	UINT32 i;
	UINT32 pow = 1;
	for (i = len; i > 0; --i) {
		out += (str[i-1] - 48) * pow;
		pow *= 10;
	}
	return out;
}

UINT32 rgb(UINT8 r, UINT8 g, UINT8 b) {
	return 0xFF000000 + (r * 0x10000) + (g * 0x100) + (b * 0x1);
}

UINT32 rgba(UINT8 r, UINT8 g, UINT8 b, UINT8 a) {
	return (a * 0x1000000) + (r * 0x10000) + (g * 0x100) + (b * 0x1);
}

/* Returns the lower of two shorts */
static UINT16 min(UINT16 a, UINT16 b) {
	return (a < b) ? a : b;
}

/* Returns the higher of two shorts */
static UINT16 max(UINT16 a, UINT16 b) {
	return (a > b) ? a : b;
}

/* Write the contents of the buffer, as they were all non-escaped data. */
static void ansi_dump_buffer(term_state_t * s) {
	for (int i = 0; i < s->buflen; ++i) {
		s->callbacks->writer(s->unit, s->buffer[i]);
	}
}

static void ansi_buf_add(term_state_t * s, char c) 
{
	if (s->buflen >= TERM_BUF_LEN-1) return;
	s->buffer[s->buflen] = c;
	s->buflen++;
	s->buffer[s->buflen] = '\0';
}

static void _ansi_put(term_state_t * s, char c) {
	APTR SysBase = s->unit->con_SysBase;
	term_callbacks_t * callbacks = s->callbacks;
	switch (s->escape) {
		case 0:
			/* We are not escaped, check for escape character */
			if (c == ANSI_ESCAPE) {
				/*
				 * Enable escape mode, setup a buffer,
				 * fill the buffer, get out of here.
				 */
				s->escape    = 1;
				s->buflen    = 0;
				ansi_buf_add(s, c);
				return;
			} else if (c == 0) {
				return;
			} else {
				callbacks->writer(s->unit, c);
			}
			break;
		case 1:
			/* We're ready for [ */
			if (c == ANSI_BRACKET) {
				s->escape = 2;
				ansi_buf_add(s, c);
			} else if (c == ANSI_BRACKET_RIGHT) {
				s->escape = 3;
				ansi_buf_add(s, c);
			} else if (c == ANSI_OPEN_PAREN) {
				s->escape = 4;
				ansi_buf_add(s, c);
			} else {
				/* This isn't a bracket, we're not actually escaped!
				 * Get out of here! */
				ansi_dump_buffer(s);
				callbacks->writer(s->unit, c);
				s->escape = 0;
				s->buflen = 0;
				return;
			}
			break;
		case 2:
			if (c >= ANSI_LOW && c <= ANSI_HIGH) {
				/* Woah, woah, let's see here. */
				char * pch;  /* tokenizer pointer */
				char * save; /* Strtok_r pointer */
				char * argv[MAX_ARGS]; /* escape arguments */
				/* Get rid of the front of the buffer */
				Strtok_r(s->buffer,"[",&save);
				pch = Strtok_r(NULL,";",&save);
				/* argc = Number of arguments, obviously */
				int argc = 0;
				while (pch != NULL) {
					argv[argc] = (char *)pch;
					++argc;
					if (argc > MAX_ARGS)
						break;
					pch = Strtok_r(NULL,";",&save);
				}
				
				/* Alright, let's do this */
				switch (c) {
					case ANSI_EXT_IOCTL:
						{
							if (argc > 0) {
								int arg = atoi(argv[0]);
								switch (arg) {
									case 1:
										callbacks->redraw_cursor(s->unit);
										break;
									case 1555:
										if (argc > 1) {
											//callbacks->set_font_size(s->unit, atof(argv[1]));
										}
										break;
									default:
										break;
								}
							}
						}
						break;
					case ANSI_SCP:
						{
							s->save_x = callbacks->get_csr_x(s->unit);
							s->save_y = callbacks->get_csr_y(s->unit);
						}
						break;
					case ANSI_RCP:
						{
							callbacks->set_csr(s->unit, s->save_x, s->save_y);
						}
						break;
					case ANSI_SGR:
						/* Set Graphics Rendition */
						if (argc == 0) {
							/* Default = 0 */
							argv[0] = "0";
							argc    = 1;
						}
						for (int i = 0; i < argc; ++i) {
							int arg = atoi(argv[i]);
							if (arg >= 100 && arg < 110) {
								/* Bright background */
								s->bg = 8 + (arg - 100);
								s->flags |= ANSI_SPECBG;
							} else if (arg >= 90 && arg < 100) {
								/* Bright foreground */
								s->fg = 8 + (arg - 90);
							} else if (arg >= 40 && arg < 49) {
								/* Set background */
								s->bg = arg - 40;
								s->flags |= ANSI_SPECBG;
							} else if (arg == 49) {
								s->bg = TERM_DEFAULT_BG;
								s->flags &= ~ANSI_SPECBG;
							} else if (arg >= 30 && arg < 39) {
								/* Set Foreground */
								s->fg = arg - 30;
							} else if (arg == 39) {
								/* Default Foreground */
								s->fg = 7;
							} else if (arg == 9) {
								/* X-OUT */
								s->flags |= ANSI_CROSS;
							} else if (arg == 7) {
								/* INVERT: Swap foreground / background */
								UINT32 temp = s->fg;
								s->fg = s->bg;
								s->bg = temp;
							} else if (arg == 6) {
								/* proprietary RGBA color support */
								if (i == 0) { break; }
								if (i < argc) {
									int r = atoi(argv[i+1]);
									int g = atoi(argv[i+2]);
									int b = atoi(argv[i+3]);
									int a = atoi(argv[i+4]);
									if (a == 0) a = 1; /* Override a = 0 */
									UINT32 c = rgba(r,g,b,a);
									if (atoi(argv[i-1]) == 48) {
										s->bg = c;
										s->flags |= ANSI_SPECBG;
									} else if (atoi(argv[i-1]) == 38) {
										s->fg = c;
									}
									i += 4;
								}
							} else if (arg == 5) {
								/* Supposed to be blink; instead, support X-term 256 colors */
								if (i == 0) { break; }
								if (i < argc) {
									if (atoi(argv[i-1]) == 48) {
										/* Background to i+1 */
										s->bg = atoi(argv[i+1]);
										s->flags |= ANSI_SPECBG;
									} else if (atoi(argv[i-1]) == 38) {
										/* Foreground to i+1 */
										s->fg = atoi(argv[i+1]);
									}
									++i;
								}
							} else if (arg == 4) {
								/* UNDERLINE */
								s->flags |= ANSI_UNDERLINE;
							} else if (arg == 3) {
								/* ITALIC: Oblique */
								s->flags |= ANSI_ITALIC;
							} else if (arg == 2) {
								/* Konsole RGB color support */
								if (i == 0) { break; }
								if (i < argc - 2) {
									int r = atoi(argv[i+1]);
									int g = atoi(argv[i+2]);
									int b = atoi(argv[i+3]);
									UINT32 c = rgb(r,g,b);
									if (atoi(argv[i-1]) == 48) {
										/* Background to i+1 */
										s->bg = c;
										s->flags |= ANSI_SPECBG;
									} else if (atoi(argv[i-1]) == 38) {
										/* Foreground to i+1 */
										s->fg = c;
									}
									i += 3;
								}
							} else if (arg == 1) {
								/* BOLD/BRIGHT: Brighten the output color */
								s->flags |= ANSI_BOLD;
							} else if (arg == 0) {
								/* Reset everything */
								s->fg = TERM_DEFAULT_FG;
								s->bg = TERM_DEFAULT_BG;
								s->flags = TERM_DEFAULT_FLAGS;
							}
						}
						break;
					case ANSI_SHOW:
						if (argc > 0) {
							if (!Strcmp(argv[0], "?1049")) {
								callbacks->cls(s->unit, 2);
								callbacks->set_csr(s->unit, 0,0);
							}
						}
						break;
					case ANSI_CUF:
						{
							int i = 1;
							if (argc) {
								i = atoi(argv[0]);
							}
							callbacks->set_csr(s->unit, min(callbacks->get_csr_x(s->unit) + i, s->width - 1), callbacks->get_csr_y(s->unit));
						}
						break;
					case ANSI_CUU:
						{
							int i = 1;
							if (argc) {
								i = atoi(argv[0]);
							}
							callbacks->set_csr(s->unit, callbacks->get_csr_x(s->unit), max(callbacks->get_csr_y(s->unit) - i, 0));
						}
						break;
					case ANSI_CUD:
						{
							int i = 1;
							if (argc) {
								i = atoi(argv[0]);
							}
							callbacks->set_csr(s->unit, callbacks->get_csr_x(s->unit), min(callbacks->get_csr_y(s->unit) + i, s->height - 1));
						}
						break;
					case ANSI_CUB:
						{
							int i = 1;
							if (argc) {
								i = atoi(argv[0]);
							}
							callbacks->set_csr(s->unit, max(callbacks->get_csr_x(s->unit) - i,0), callbacks->get_csr_y(s->unit));
						}
						break;
					case ANSI_CHA:
						if (argc < 1) {
							callbacks->set_csr(s->unit, 0,callbacks->get_csr_y(s->unit));
						} else {
							callbacks->set_csr(s->unit, min(max(atoi(argv[0]), 1), s->width) - 1, callbacks->get_csr_y(s->unit));
						}
						break;
					case ANSI_CUP:
						if (argc < 2) {
							callbacks->set_csr(s->unit, 0,0);
						} else {
							callbacks->set_csr(s->unit, min(max(atoi(argv[1]), 1), s->width) - 1, min(max(atoi(argv[0]), 1), s->height) - 1);
						}
						break;
					case ANSI_ED:
						//KPrintF("ANSI_ED");
						if (argc < 1) {
							callbacks->cls(s->unit, 0);
						} else {
							callbacks->cls(s->unit, atoi(argv[0]));
						}
						break;
					case ANSI_EL:
						{
							int what = 0, x = 0, y = 0;
							if (argc >= 1) {
								what = atoi(argv[0]);
							}
							if (what == 0) {
								x = callbacks->get_csr_x(s->unit);
								y = s->width;
							} else if (what == 1) {
								x = 0;
								y = callbacks->get_csr_x(s->unit);
							} else if (what == 2) {
								x = 0;
								y = s->width;
							}
							for (int i = x; i < y; ++i) {
								callbacks->set_cell(s->unit, i, callbacks->get_csr_y(s->unit), ' ');
							}
						}
						break;
					case ANSI_DSR:
						{
							char out[24];
							sprintf(s, out, "\033[%d;%dR", callbacks->get_csr_y(s->unit) + 1, callbacks->get_csr_x(s->unit) + 1);
							callbacks->input_buffer_stuff(s->unit, out);
						}
						break;
					case ANSI_SU:
						{
							int how_many = 1;
							if (argc > 0) {
								how_many = atoi(argv[0]);
							}
							callbacks->scroll(s->unit, how_many);
						}
						break;
					case ANSI_SD:
						{
							int how_many = 1;
							if (argc > 0) {
								how_many = atoi(argv[0]);
							}
							callbacks->scroll(s->unit, -how_many);
						}
						break;
					case 'X':
						{
							int how_many = 1;
							if (argc > 0) {
								how_many = atoi(argv[0]);
							}
							for (int i = 0; i < how_many; ++i) {
								callbacks->writer(s->unit, ' ');
							}
						}
						break;
					case 'd':
						if (argc < 1) {
							callbacks->set_csr(s->unit, callbacks->get_csr_x(s->unit), 0);
						} else {
							callbacks->set_csr(s->unit, callbacks->get_csr_x(s->unit), atoi(argv[0]) - 1);
						}
						break;
					default:
						/* Meh */
						break;
				}
				/* Set the states */
				if (s->flags & ANSI_BOLD && s->fg < 9) {
					callbacks->set_color(s->unit, s->fg % 8 + 8, s->bg);
				} else {
					callbacks->set_color(s->unit, s->fg, s->bg);
				}
				/* Clear out the buffer */
				s->buflen = 0;
				s->escape = 0;
				return;
			} else {
				/* Still escaped */
				ansi_buf_add(s, c);
			}
			break;
		case 3:
			if (c == '\007') {
				/* Tokenize on semicolons, like we always do */
				char * pch;  /* tokenizer pointer */
				char * save; /* Strtok_r pointer */
				char * argv[MAX_ARGS]; /* escape arguments */
				/* Get rid of the front of the buffer */
				Strtok_r(s->buffer,"]",&save);
				pch = Strtok_r(NULL,";",&save);
				/* argc = Number of arguments, obviously */
				int argc = 0;
				while (pch != NULL) {
					argv[argc] = (char *)pch;
					++argc;
					if (argc > MAX_ARGS) break;
					pch = Strtok_r(NULL,";",&save);
				}
				/* Start testing the first argument for what command to use */
				if (argv[0]) {
					if (!Strcmp(argv[0], "1")) {
						if (argc > 1) {
							callbacks->set_title(s->unit, argv[1]);
						}
					} /* Currently, no other options */
				}
				/* Clear out the buffer */
				s->buflen = 0;
				s->escape = 0;
				return;
			} else {
				/* Still escaped */
				if (c == '\n' || s->buflen > 256) {
					ansi_dump_buffer(s);
					callbacks->writer(s->unit, c);
					s->buflen = 0;
					s->escape = 0;
					return;
				}
				ansi_buf_add(s, c);
			}
			break;
		case 4:
			if (c == '0') {
				s->box = 1;
			} else if (c == 'B') {
				s->box = 0;
			} else {
				ansi_dump_buffer(s);
				callbacks->writer(s->unit, c);
			}
			s->escape = 0;
			s->buflen = 0;
			break;
	}
}

void ansi_put(term_state_t * s, char c) 
{
//	spin_lock(&s->lock);
	_ansi_put(s, c);
//	spin_unlock(&s->lock);
}

term_state_t * ansi_init(pSysBase SysBase, APTR unit, int w, int h, term_callbacks_t * callbacks_in) 
{
	term_state_t *s = AllocVec(sizeof(term_state_t), MEMF_PUBLIC|MEMF_CLEAR);
	if (s)
	{
		/* Terminal Defaults */
		s->fg     = TERM_DEFAULT_FG;    /* Light grey */
		s->bg     = TERM_DEFAULT_BG;    /* Black */
		s->flags  = TERM_DEFAULT_FLAGS; /* Nothing fancy*/
		s->width  = w;
		s->height = h;
		s->box    = 0;
		s->unit	  = unit;
		s->callbacks = callbacks_in;
		s->callbacks->set_color(unit, s->fg, s->bg);
		s->utilBase = OpenLibrary("utility.library",0);
		if (!s->utilBase) FreeVec(s);
	}
	return s;
}

