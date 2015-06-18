/**
* File: /fonts．h
* User: cycl0ne
* Date: 2014-11-26
* Time: 03:58 PM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#ifndef FONTS_H
#define FONTS_H
#include "types.h"
#include "gfx.h"

typedef unsigned short	IMAGEBITS;
typedef struct _font*	pFont;
typedef struct _fontinfo *	pFontInfo;
typedef struct GfxBase *	pGfxBase;

/* builtin font std names*/
#define FONT_SYSTEM_VAR	"System"	/* winFreeSansSerif 11x13 (ansi)*/
#define FONT_SYSTEM_FIXED	"SystemFixed"	/* X6x13 (should be ansi)*/
#define FONT_GUI_VAR		"System"	/* deprecated (was "Helvetica")*/
#define FONT_OEM_FIXED	"SystemFixed"	/* deprecated (was "Terminal")*/

/* Text/GetTextSize encoding flags*/
#define TF_ASCII	0x00000000L	/* 8 bit packing, ascii*/
#define TF_UTF8	0x00000001L	/* 8 bit packing, utf8*/
#define TF_UC16	0x00000002L	/* 16 bit packing, unicode 16*/
#define TF_UC32	0x00000004L	/* 32 bit packing, unicode 32*/
#define TF_XCHAR2B	0x00000008L	/* 16 bit packing, X11 big endian PCF*/
#define TF_PACKMASK	0x0000000FL	/* packing bits mask*/

/* asian double-byte encodings*/
#define TF_DBCS_BIG5	0x00000100L	/* chinese big5*/
#define TF_DBCS_EUCCN	0x00000200L	/* chinese EUCCN (gb2312+0x80)*/
#define TF_DBCS_EUCKR	0x00000300L	/* korean EUCKR (ksc5601+0x80)*/
#define TF_DBCS_EUCJP	0x00000400L	/* japanese EUCJP*/
#define TF_DBCS_JIS	0x00000500L	/* japanese JISX0213*/
#define TF_DBCSMASK	0x00000700L	/* DBCS encodings bitmask*/

/* Text alignment flags*/
#define TF_TOP	0x01000000L	/* align on top*/
#define TF_BASELINE	0x02000000L	/* align on baseline*/
#define TF_BOTTOM	0x04000000L	/* align on bottom*/

/* SetFontAttr and capabilities flags (not used with TF_ above)*/
#define TF_KERNING	0x0001		/* font kerning*/
#define TF_ANTIALIAS	0x0002		/* antialiased output*/
#define TF_UNDERLINE	0x0004		/* draw underline*/
#define TF_BOLD		0x0008		/* draw bold glyph (not present on all renderers)*/

#define TF_CMAP_DEFAULT 0x0010	/* use default (unicode) charset in truetype font (not required)*/
#define TF_CMAP_0		  0x0020	/* use charmap 0 in truetype font*/
#define TF_CMAP_1       0x0040	/* use charmap 1 in truetype font*/

#define TF_FREETYPE	0x1000		/* FIXME: remove*/
#define TF_SCALEHEIGHT 0x2000		/* font can scale height seperately*/
#define TF_SCALEWIDTH	0x4000		/* font can scale width seperately*/

/* font classes - used to specify a particular renderer*/
#define LF_CLASS_ANY		0	/* any font*/
#define LF_CLASS_BUILTIN	1	/* builtin fonts*/
#define LF_CLASS_FNT		2	/* FNT native fonts*/
#define LF_CLASS_PCF		3	/* X11 PCF/PCF.GZ fonts*/
#define LF_CLASS_FREETYPE	4	/* FreeType 1 or 2 fonts in TT format*/
#define LF_CLASS_T1LIB	5	/* T1LIB outlined Adobe Type 1 fonts*/
#define LF_CLASS_MGL		6	/* MGL (EUCJP) fonts*/
#define LF_CLASS_HZK		7	/* chinese HZK fonts*/

#define LF_FACESIZE		64	/* max facename size*/

/* font type selection - lfOutPrecision*/
#define LF_TYPE_DEFAULT	0	/* any font*/
#define LF_TYPE_SCALED	4	/* outlined font (tt or adobe)*/
#define LF_TYPE_RASTER	5	/* raster only*/
#define LF_TYPE_TRUETYPE	7	/* truetype only*/
#define LF_TYPE_ADOBE		10	/* adobe type 1 only*/

/* font weights - lfWeight*/
#define LF_WEIGHT_DEFAULT	0	/* any weight*/
#define LF_WEIGHT_THIN	100	/* thin*/
#define LF_WEIGHT_EXTRALIGHT	200
#define LF_WEIGHT_LIGHT	300	/* light */
#define LF_WEIGHT_NORMAL	400	/* regular*/
#define LF_WEIGHT_REGULAR	400
#define LF_WEIGHT_MEDIUM	500	/* medium */
#define LF_WEIGHT_DEMIBOLD	600
#define LF_WEIGHT_BOLD	700	/* bold*/
#define LF_WEIGTH_EXTRABOLD	800
#define LF_WEIGHT_BLACK	900	/* black */

/* font charset - lfCharSet*/
#define LF_CHARSET_ANSI	0	/* win32 ansi*/
#define LF_CHARSET_DEFAULT	1	/* any charset*/
#define LF_CHARSET_UNICODE	254	/* unicode*/
#define LF_CHARSET_OEM	255	/* local hw*/

/* font pitch - lfPitch */
#define LF_PITCH_DEFAULT		0	/* any pitch */
#define LF_PITCH_ULTRACONDENSED	10
#define LF_PITCH_EXTRACONDENSED	20
#define LF_PITCH_CONDENSED		30
#define LF_PITCH_SEMICONDENSED	40
#define LF_PITCH_NORMAL		50
#define LF_PITCH_SEMIEXPANDED		60
#define LF_PITCH_EXPANDED		70
#define LF_PITCH_EXTRAEXPANDED	80
#define LF_PITCH_ULTRAEXPANDED	90

/* flags for the GdAddFont function */
#define LF_FLAGS_ALIAS	1
	
typedef struct CFont {
	char *			name;		/* font name*/
	int				maxwidth;	/* max width in pixels*/
	unsigned int	height;		/* height in pixels*/
	int				ascent;		/* ascent (baseline) height*/
	int				firstchar;	/* first character in bitmap*/
	int				size;		/* font size in characters*/
	const IMAGEBITS *bits;		/* 16-bit right-padded bitmap data*/
	const UINT32 	*offset;	/* offsets into bitmap data*/
	const unsigned char *width;	/* character widths or 0 if fixed*/
	int				defaultchar;/* default char (not glyph index)*/
	INT32			bits_size;	/* # words of IMAGEBITS bits*/
} CFont_t, *pCFont;

typedef struct FontProcs {
	int		capabilities;		/* flags for font subdriver capabilities*/
	UINT32	encoding;	/* routines expect this encoding*/
	BOOL	(*FInit)(pGfxBase GfxBase, pSD psd);
	pFont	(*FCreateFont)(pGfxBase GfxBase,const char *name, INT32 height, INT32 width, int attr);
	BOOL	(*FGetFontInfo)(pGfxBase GfxBase,pFont pfont, pFontInfo pfontinfo);
	void	(*FGetTextSize)(pGfxBase GfxBase,pFont pfont, const void *text, int cc,
			UINT32 flags, INT32 *pwidth, INT32 *pheight,
			INT32 *pbase);
	void	(*FGetTextBits)(pGfxBase GfxBase,pFont pfont, int ch, const IMAGEBITS **retmap,
			INT32 *pwidth, INT32 *pheight, INT32 *pbase);
	void	(*FDestroyFont)(pGfxBase GfxBase,pFont pfont);
	void	(*FDrawText)(pGfxBase GfxBase,pFont pfont, pPB psd, INT32 x, INT32 y,
			const void *str, int count, UINT32 flags);
	int		(*FSetFontSize)(pGfxBase GfxBase,pFont pfont, INT32 height, INT32 width);
	void	(*FSetFontRotation)(pGfxBase GfxBase,pFont pfont, int tenthdegrees);
	int		(*FSetFontAttr)(pGfxBase GfxBase,pFont pfont, int setflags, int clrflags);
	pFont	(*FDuplicate) (pGfxBase GfxBase,pFont psrcfont, INT32 height, INT32 width);
} FontProcs_t, *pFontProcs;

typedef struct _font {		/* common hdr for all font structures*/
	pFontProcs	fontprocs;	/* font-specific rendering routines*/
	INT32		fontsize;	/* font height in pixels*/
	INT32		fontwidth;	/* font width in pixels*/
	INT32		fontrotation; /* font rotation*/
	INT32		fontattr;	/* font attributes: kerning/antialias*/
	/* font-specific rendering data here*/
} Font_t;

typedef struct CoreFont {
	/* common hdr*/
	pFontProcs		fontprocs;
	INT32			fontsize;	/* font height in pixels*/
	INT32			fontwidth;	/* font width in pixels*/
	INT32			fontrotation;
	INT32			fontattr;
	/* core font specific data*/
	STRPTR			name;			/* Microwindows font name*/
	pCFont			cfont;			/* builtin font data*/
} CoreFont_t, *pCoreFont;

typedef struct _fontinfo {
	/**
	 * Maximum advance width of any character.
	 */
	int maxwidth;

	/**
	 * Height of "most characters" in the font. This does not include any
	 * leading (blank space between lines of text).
	 * Always equal to (baseline+descent).
	 */
	int height;

	/**
	 * The ascent (height above the baseline) of "most characters" in
	 * the font.
	 *
	 * Note: This member variable should be called "ascent", to be
	 * consistent with FreeType 2, and also to be internally consistent
	 * with the "descent" member.  It has not been renamed because that
	 * would break backwards compatibility.  FIXME
	 */
	int baseline;

	/**
	 * The descent (height below the baseline) of "most characters" in
	 * the font.
	 *
	 * Should be a POSITIVE number.
	 */
	int descent;

	/**
	 * Maximum height of any character above the baseline.
	 */
	int maxascent;

	/**
	 * Maximum height of any character below the baseline.
	 *
	 * Should be a POSITIVE number.
	 */
	int maxdescent;

	/**
	 * The distance between the baselines of two consecutive lines of text.
	 * This is usually height plus some font-specific "leading" value.
	 */
	int linespacing;

	/**
	 * First character in the font.
	 */
	int firstchar;

	/**
	 * Last character in the font.
	 */
	int lastchar;

	/**
	 * True (nonzero) if font is fixed width.  In that 
	 * case, maxwidth
	 * gives the width for every character in the font.
	 */
	BOOL fixed;

	/**
	 * Table of character advance widths for characters 0-255.
	 * Note that fonts can contain characters with codes >255 - in that
	 * case this table contains the advance widths for some but not all
	 * characters.  Also note that if the font contains kerning
	 * information, the advance width of the string "AV" may differ from
	 * the sum of the advance widths for the characters 'A' and 'V'.
	 */
	UINT8 widths[256];
} FontInfo_t;

typedef struct {
	INT32	lfHeight;		/* desired height in pixels*/
	INT32	lfWidth;		/* desired width in pixels or 0*/
	INT32	lfEscapement;	/* rotation in tenths of degree*/
	INT32	lfOrientation;	/* not used*/
	INT32	lfWeight;		/* font weight*/
	UINT8	lfItalic;		/* =1 for italic */
	UINT8	lfUnderline;	/* =1 for underline */
	UINT8	lfStrikeOut;	/* not used*/
	UINT8	lfCharSet;		/* font character set*/
	UINT8	lfOutPrecision;	/* font type selection*/
	UINT8	lfClipPrecision;/* not used*/
	UINT8	lfQuality;		/* not used*/
	UINT8 lfPitchAndFamily;/* not used*/
	/* end of windows-compatibility*/

	UINT8 lfClass;		/* font class (renderer) */

	/* Following only used by (the legacy) FONTMAPPER when enabled.
	 * They are only kept around to stay source and binary
	 * compatible to previous microwindows releases.
	 */
	UINT8	lfPitch;		/* font pitch */
	UINT8	lfRoman;		/* =1 for Roman letters (upright) */
	UINT8	lfSerif;		/* =1 for Serifed font */
	UINT8	lfSansSerif;	/* =1 for Sans-serif font */
	UINT8	lfModern;		/* =1 for Modern font */
	UINT8	lfMonospace;	/* =1 for Monospaced font */
	UINT8	lfProportional;	/* =1 for Proportional font */
	UINT8	lfOblique;		/* =1 for Oblique (kind of Italic) */
	UINT8	lfSmallCaps;	/* =1 for small caps */
	/* End of fontmapper-only variables */

	/* render-dependent full path or facename here*/
	char	lfFaceName[LF_FACESIZE];/* font name, may be aliased*/

} LogFont, *pLogFont;

#endif
