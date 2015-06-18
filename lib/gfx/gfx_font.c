/**
* File: /gfx_font．c
* User: cycl0ne
* Date: 2014-11-28
* Time: 12:00 AM
* PowerOS, Copyright (C) 2014.  All rights reserved.
**/

#include "gfxbase.h"

#include "dos.h"
#include "dos_io.h"
#include "dos_interface.h"

#define SysBase GfxBase->SysBase
#define DOSBase GfxBase->DOSBase

static int utf8_to_utf16(const unsigned char *utf8, int cc, unsigned short *unicode16);
static int uc16_to_utf8(const unsigned short *us, int cc, unsigned char *s);

static inline int isupper(int c) { return (c >= 'A' && c <= 'Z'); }
static inline int islower(int c) { return (c >= 'a' && c <= 'z'); }
static inline int toupper(int c) {	if (islower(c)) return 'A' - 'a' + c; return c; }
static inline int tolower(int c) { if (isupper(c)) return 'a' - 'A' + c; return c; }
static inline int strlen(const char *str) { const char *s; for (s = str; *s; ++s); return(s - str); }

static inline char *strncpy(STRPTR dst, const STRPTR src, int n)
{
	if (n != 0) {
		char *d = dst;
		const char *s = src;

		do {
			if ((*d++ = *s++) == 0) {
				/* NUL pad the remaining n-1 bytes */
				while (--n != 0) *d++ = 0;
				break;
			}
		} while (--n != 0);
	}
	return (dst);
}

static inline int strcasecmp(const char *s1, const char *s2)
{
	const unsigned char *us1 = (const unsigned char *)s1, *us2 = (const unsigned char *)s2;
	while (tolower(*us1) == tolower(*us2++)) if (*us1++ == '\0') return (0);
	return (tolower(*us1) - tolower(*--us2));
}

pFont gfx_CreateFont(pGfxBase GfxBase, pPB rp, const char *name, UINT16 height, UINT16 width, const pLogFont plogfont)
{
	int 		i;
	int		fontht;
	int		fontno;
 	int		fontclass;
	int		fontattr = 0;
	pFont		pfont;
	pCoreFont	pf = GfxBase->builtin_fonts;
	pCoreFont	upf;
	FontInfo_t	fontinfo;
	ScreenInfo	scrinfo;
	const char *	fontname;
	char 			fontmapper_fontname[LF_FACESIZE + 1];
	//PixMap			*psd = rp->crp_PixMap;

	//FIXME: GetScreenInfo(rp, &scrinfo);

	/* if plogfont not specified, use passed name, height and any class*/
	if (!plogfont) 
	{
		/* if name not specified, use first builtin*/
		if (!name || !name[0]) name = pf->name;
		fontname = name;
		fontclass = LF_CLASS_ANY;
	} else {
		/* otherwise, use MWLOGFONT name, height and class*/
		/* Copy the name from plogfont->lfFaceName to fontmapper_fontname
		 * Note that it may not be NUL terminated in the source string,
		 * so we're careful to NUL terminate it here.
		 */
		strncpy(fontmapper_fontname, plogfont->lfFaceName, LF_FACESIZE);
		fontmapper_fontname[LF_FACESIZE] = '\0';
		fontname = fontmapper_fontname;
		if (!fontname[0])	/* if name not specified, use first builtin*/
			fontname = pf->name;
		fontclass = plogfont->lfClass;
		height = plogfont->lfHeight;
		width = plogfont->lfWidth;
		if (plogfont->lfUnderline) fontattr = TF_UNDERLINE;
	}
	height = ABS(height);

	/* check builtin fonts first for speed*/
 	if (!height && (fontclass == LF_CLASS_ANY || fontclass == LF_CLASS_BUILTIN)) {
  		for(i = 0; i < /*scrinfo.fonts*/4; ++i) {
			//KPrintF("[%s] looking..\n", pf[i].name);
 			if(!strcasecmp(pf[i].name, fontname)) {
  				pf[i].fontsize = pf[i].cfont->height;
				pf[i].fontattr = fontattr;
				//KPrintF("createfont: (height == 0) found builtin font %s (%d)\n", fontname, i);
  				return (pFont)&pf[i];
  			}
  		}
		/* 
		 * Specified height=0 and no builtin font matched name.
		 * if not font found with other renderer, no font
		 * will be loaded, and 0 returned.
		 *
		 * We used to return the first builtin font.  If a font
		 * return needs to be guaranteed, specify a non-zero
		 * height, and the closest builtin font to the specified
		 * height will always be returned.
		 */
		if (fontclass != LF_CLASS_ANY) KPrintF("builtin-createfont: %s,%d not found\n", fontname, height);
  	}

	/* check user builtin fonts next*/
	upf = GfxBase->user_builtin_fonts;
	while ( (upf != NULL) && (upf->name != NULL) ) {
		if(!strcasecmp(upf->name, fontname) && (upf->cfont->height == height) ) {
			if( upf->fontprocs == NULL ) gen_setfontproc(upf);
			upf->fontsize = upf->cfont->height;
			upf->fontattr = fontattr;
			KPrintF("createfont: (height != 0) found user builtin font %s (%d)\n", fontname, height);
			return (pFont)upf;
		}
		upf++;
	}

	if (fontclass == LF_CLASS_ANY) 
	{
		KPrintF("createfont: %s,%d not found\n", fontname, height);
		KPrintF("  (tried "
			"builtin_createfont"
			")\n");
	}

	/*
	 * No font yet found.  If the height was specified, we'll return the
	 * most close builtin font as a fallback.  Otherwise 0 will be returned.
	 */
 	if (height != 0 && (fontclass == LF_CLASS_ANY || fontclass == LF_CLASS_BUILTIN)) {
		/* find builtin font closest in height*/
		fontno = 0;
		height = ABS(height);
		fontht = MAX_COORD;
		for(i = 0; i < /*scrinfo.fonts*/4; ++i) {
			pfont = (pFont)&pf[i];
			GetFontInfo(pfont, &fontinfo);
			if(fontht > ABS(height-fontinfo.height)) { 
				fontno = i;
				fontht = ABS(height-fontinfo.height);
			}
		}
		pf[fontno].fontsize = pf[fontno].cfont->height;
		pf[fontno].fontattr = fontattr;
		KPrintF("createfont: height given, using builtin font %s (%d) as fallback\n", pf[fontno].name, fontno);
		return (pFont)&pf[fontno];
	}

	/* no font found: don't load any font and return 0*/
	KPrintF("createfont: no height given, fallback search impossible, returning NULL\n");
	return 0;
}

/**
 * Set the size of a font.
 *
 * @param pfont    The font to modify.
 * @param fontsize The new size.
 * @return         The old size.
 */
INT16 gfx_SetFontSize(pGfxBase GfxBase, pFont pfont, INT16 height, INT16 width)
{
	if (pfont->fontprocs->FSetFontSize) return pfont->fontprocs->FSetFontSize(GfxBase, pfont, height, width);
	return 0;
}

/**
 * Set the rotation angle of a font.
 *
 * @param pfont        The font to modify.
 * @param tenthdegrees The new rotation angle, in tenths of degrees.
 * @return             The old rotation angle, in tenths of degrees.
 */
int gfx_SetFontRotation(pGfxBase GfxBase, pFont pfont, int tenthdegrees)
{
	INT16 oldrotation = pfont->fontrotation;
	pfont->fontrotation = tenthdegrees;

	if (pfont->fontprocs->FSetFontRotation) pfont->fontprocs->FSetFontRotation(GfxBase, pfont, tenthdegrees);	
	return oldrotation;
}

/**
 * Set/reset font attributes (TF_KERNING, TF_ANTIALIAS)
 * for a font.
 *
 * @param pfont    The font to modify.
 * @param setflags The flags to set.  Overrides clrflags.
 * @param clrflags The flags to clear.
 * @return         The old font attributes.
 */
int gfx_SetFontAttr(pGfxBase GfxBase, pFont pfont, int setflags, int clrflags)
{
	if (pfont->fontprocs->FSetFontAttr) return pfont->fontprocs->FSetFontAttr(GfxBase, pfont, setflags, clrflags);	
	return 0;
}

/**
 * Unload and deallocate a font.  Do not use the font once it has been
 * destroyed.
 *
 * @param pfont The font to deallocate.
 */
void gfx_DestroyFont(pGfxBase GfxBase, pFont pfont)
{
	if (pfont->fontprocs->FDestroyFont) pfont->fontprocs->FDestroyFont(GfxBase, pfont);
}

/**
 * Return information about a specified font.
 *
 * @param pfont The font to query.
 * @param pfontinfo Recieves the result of the query
 * @return TRUE on success, FALSE on error.
 */
BOOL gfx_GetFontInfo(pGfxBase GfxBase, pFont pfont, pFontInfo pfontinfo)
{
	if(!pfont || !pfont->fontprocs->FGetFontInfo) return FALSE;
	return pfont->fontprocs->FGetFontInfo(GfxBase, pfont, pfontinfo);
}

/**
 * Draws text onto a drawing surface (e.g. the screen or a double-buffer).
 * Uses the current font, current foreground color, and possibly the
 * current background color.  Applies clipping if necessary.
 * The background color is only drawn if the gr_usebg flag is set.
 *
 * @param psd   The destination drawing surface.  Non-NULL.
 * @param x     The X co-ordinate to draw the text.
 * @param y     The Y co-ordinate to draw the text.  The flags specify
 *              whether this is the top (TF_TOP), bottom (TF_BOTTOM),
 *              or baseline (TF_BASELINE) of the text.
 * @param str   The string to display.  Non-NULL.
 * @param cc    The length of str.  For Asian DBCS encodings, this is
 *              specified in bytes.  For all other encodings such as ASCII,
 *              UTF8 and UC16, it is specified in characters.  For ASCII
 *              and DBCS encodings, this may be set to -1, and the length
 *              will be calculated automatically.
 * @param flags Flags specifying the encoding of str and the position of the
 *              text.  Specifying the vertical position is mandatory.
 *              The encoding of str defaults to ASCII if not specified.
 */
extern FontProcs_t fontprocs;
 
void gfx_Text(pGfxBase GfxBase, pPB rp, pFont pfont, INT32 x, INT32 y, const void *str, int cc,UINT32 flags)
{
	const void *text;
	int		defencoding = pfont->fontprocs->encoding;
	int		force_uc16 = 0;
	UINT32 *buf = NULL;

	/*
	 * DBCS encoding is handled a little special: if the selected
	 * font is a builtin, then we'll force a conversion to UC16
	 * rather than converting to the renderer specification.  This is
	 * because we allow DBCS-encoded strings to draw using the
	 * specially-compiled-in font if the character is not ASCII.
	 * This is specially handled in gen_drawtext below.
	 *
	 * If the font is not builtin, then the drawtext routine must handle
	 * all glyph output, including ASCII.
	 */
	if (flags & TF_DBCSMASK) {
		/* force double-byte sequences to UC16 if builtin font only*/
		if (pfont->fontprocs == &fontprocs) 
		{
			defencoding = TF_UC16;
			force_uc16 = 1;
		}
	}

	/* use strlen for char count when ascii or dbcs*/
	if(cc == -1 && (flags & TF_PACKMASK) == TF_ASCII) cc = strlen((char *)str);

	/* convert encoding if required*/
	if((flags & (TF_PACKMASK|TF_DBCSMASK)) != defencoding) {
		/* allocate enough for output string utf8/uc32 is max 4 bytes, uc16 max 2*/
		buf = AllocVec(cc * 4, MEMF_FAST);
		cc = ConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~TF_PACKMASK;	/* keep DBCS bits for drawtext*/
		flags |= defencoding;
		text = buf;
	} else 
		text = str;

	if(cc <= 0 || !pfont->fontprocs->FDrawText) 
	{
		if (buf) FreeVec(buf);
		return;
	}

	if (!force_uc16)	/* remove DBCS flags if not needed*/
		flags &= ~TF_DBCSMASK;
	pfont->fontprocs->FDrawText(GfxBase, pfont, rp, x, y, text, cc, flags);

	if (buf) FreeVec(buf);
}

/**
 * Gets the size of some text in a specified font.
 *
 * @param pfont   The font to measure.  Non-NULL.
 * @param str     The string to measure.  Non-NULL.
 * @param cc      The length of str.  For Asian DBCS encodings, this is
 *                specified in bytes.  For all other encodings such as ASCII,
 *                UTF8 and UC16, it is specified in characters.  For ASCII
 *                and DBCS encodings, this may be set to -1, and the length
 *                will be calculated automatically.
 * @param pwidth  On return, holds the width of the text.
 * @param pheight On return, holds the height of the text.
 * @param pbase   On return, holds the baseline of the text.
 * @param flags   Flags specifying the encoding of str and the position of the
 *                text.  Specifying the vertical position is mandatory.
 *                The encoding of str defaults to ASCII if not specified.
 */
void gfx_GetTextSize(pGfxBase GfxBase, pFont pfont, const void *str, int cc, INT32 *pwidth, INT32 *pheight, INT32 *pbase, UINT32 flags)
{
	const void *	text;
	UINT32	defencoding = pfont->fontprocs->encoding;
	int		force_uc16 = 0;
	UINT32 *buf = NULL;

	/* DBCS handled specially: see comment in GdText*/
	if (flags & TF_DBCSMASK) 
	{
		/* force double-byte sequences to UC16 if builtin font only*/
		if (pfont->fontprocs == &fontprocs) {
			defencoding = TF_UC16;
			force_uc16 = 1;
		}
	}

	/* use strlen for char count when ascii or dbcs*/
	if(cc == -1 && (flags & TF_PACKMASK) == TF_ASCII) cc = strlen((char *)str);

	/* convert encoding if required*/
	if((flags & (TF_PACKMASK|TF_DBCSMASK)) != defencoding) {
		/* allocate enough for output string utf8/uc32 is max 4 bytes, uc16 max 2*/
		buf = AllocVec(cc * 4, MEMF_FAST);
		cc = ConvertEncoding(str, flags, cc, buf, defencoding);
		flags &= ~TF_PACKMASK; /* keep DBCS bits for gettextsize*/
		flags |= defencoding;
		text = buf;
	} else text = str;

	if(cc <= 0 || !pfont->fontprocs->FGetTextSize) {
		*pwidth = *pheight = *pbase = 0;
		if (buf) FreeVec(buf);
		return;
	}

	/* calc height and width of string*/
	if (force_uc16)		/* if UC16 conversion forced, string is DBCS*/
		dbcs_gettextsize(GfxBase, pfont, text, cc, flags, pwidth, pheight, pbase);
	else pfont->fontprocs->FGetTextSize(GfxBase, pfont, text, cc, flags, pwidth, pheight, pbase);

	if (buf) FreeVec(buf);
}

/**
 * Convert text from one encoding to another.
 * Input cc and returned cc is character count, not bytes.
 * Return < 0 on error or can't translate.
 *
 * @param istr   Input string.
 * @param iflags Encoding of istr, as MWTF_xxx constants.
 * @param cc     The length of istr.  For Asian DBCS encodings, this is
 *               specified in bytes.  For all other encodings such as ASCII,
 *               UTF8 and UC16, it is specified in characters.  For ASCII
 *               and DBCS encodings, this may be set to -1, and the length
 *               will be calculated automatically.
 * @param ostr   Output string.
 * @param oflags Encoding of ostr, as MWTF_xxx constants.
 * @return       Number of characters (not bytes) converted.
 */
int gfx_ConvertEncoding(pGfxBase GfxBase, const void *istr, UINT32 iflags, int cc, void *ostr, UINT32 oflags)
{
	const unsigned char 	*istr8;
	const unsigned short 	*istr16;
	const UINT32			*istr32;
	unsigned char 			*ostr8;
	unsigned short 			*ostr16;
	UINT32					*ostr32;
	unsigned int			ch;
	unsigned short			s;
	int						icc;
	unsigned short 			*buf16 = NULL;

	iflags &= TF_PACKMASK|TF_DBCSMASK;
	oflags &= TF_PACKMASK|TF_DBCSMASK;

	/* allow -1 for len with ascii or dbcs*/
	if(cc == -1 && (iflags == TF_ASCII)) cc = strlen((char *)istr);

	/* first check for utf8 input encoding*/
	if(iflags == TF_UTF8) {
		/* allocate enough for output string, uc16 max 2*/
		if (oflags != TF_UC16) buf16 = AllocVec(cc * 2, MEMF_FAST);

		/* we've only got uc16 now so convert to uc16...*/
		cc = utf8_to_utf16((unsigned char *)istr, cc,
			oflags==TF_UC16?(unsigned short*) ostr: buf16);

		if(oflags == TF_UC16 || cc < 0) {
			if (buf16) FreeVec(buf16);
			return cc;
		}

		/* will decode again to requested format (probably ascii)*/
		iflags = TF_UC16;
		istr = buf16;
	}

	icc = cc;
	cc = 0;
	istr8 = istr;
	istr16 = istr;
	istr32 = istr;
	ostr8 = ostr;
	ostr16 = ostr;
	ostr32 = ostr;

	/* Convert between formats.  Note that there's no error
	 * checking here yet.
	 */
	while(--icc >= 0) {
		switch(iflags) {
		default:
			ch = *istr8++;
			break;
		case TF_UC16:
			ch = *istr16++;
			break;
		case TF_XCHAR2B:
			ch = *istr8++ << 8;
			ch |= *istr8++;
			break;
		case TF_UC32:
			ch = *istr32++;
			break;
		case TF_DBCS_BIG5:	/* Chinese BIG5*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xF9 && icc &&
			    ((*istr8 >= 0x40 && *istr8 <= 0x7E) || (*istr8 >= 0xA1 && *istr8 <= 0xFE))) {
				ch = (ch << 8) | *istr8++;
				--icc;
			}
			break;
		case TF_DBCS_EUCCN:	/* Chinese EUCCN (GB2312+0x80)*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xF7 && icc && *istr8 >= 0xA1 && *istr8 <= 0xFE) {
				ch = (ch << 8) | *istr8++;
				--icc;
			}
			break;
		case TF_DBCS_EUCKR:	/* Korean EUCKR (KSC5601+0x80)*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xFE && icc &&
			    *istr8 >= 0xA1 && *istr8 <= 0xFE) {
#if 0 //MW_CPU_BIG_ENDIAN
				ch = (ch << 8) | *istr8++;
#else
				ch = ch | (*istr8++ << 8);
#endif
				--icc;
			}
			break;
		case TF_DBCS_EUCJP:	/* Japanese EUCJP*/
			ch = *istr8++;
			if (ch >= 0xA1 && ch <= 0xFE && icc && *istr8 >= 0xA1 && *istr8 <= 0xFE) {
				ch = (ch << 8) | *istr8++;
				--icc;
			}
			break;
		case TF_DBCS_JIS:	/* Japanese JISX0213*/
			ch = *istr8++;
			if (icc && (
				(ch >= 0xA1 && ch <= 0xFE && *istr8 >= 0xA1 && *istr8 <= 0xFE) ||
			     (((ch >= 0x81 && ch <= 0x9F) || (ch >= 0xE0 && ch <= 0xEF)) &&
			       (*istr8 >= 0x40 && *istr8 <= 0xFC && *istr8 != 0x7F)))) {
					ch = (ch << 8) | *istr8++;
					--icc;
			}

			break;
		}
		switch(oflags) {
		default:
			*ostr8++ = (unsigned char)ch;
			break;
		case TF_UTF8:
			s = (unsigned short)ch;
			ostr8 += uc16_to_utf8(&s, 1, ostr8);
			break;
		case TF_UC16:
			*ostr16++ = (unsigned short)ch;
			break;
		case TF_XCHAR2B:
			*ostr8++ = (unsigned char)(ch >> 8);
			*ostr8++ = (unsigned char)ch;
			break;
		case TF_UC32:
			*ostr32++ = ch;
			break;
		}
		++cc;
	}

	if (buf16) FreeVec(buf16);
	return cc;
}

/**
 * UTF-8 to UTF-16 conversion.  Surrogates are handeled properly, e.g.
 * a single 4-byte UTF-8 character is encoded into a surrogate pair.
 * On the other hand, if the UTF-8 string contains surrogate values, this
 * is considered an error and returned as such.
 *
 * The destination array must be able to hold as many Unicode-16 characters
 * as there are ASCII characters in the UTF-8 string.  This in case all UTF-8
 * characters are ASCII characters.  No more will be needed.
 *
 * This function will also accept Java's variant of UTF-8.  This encodes
 * U+0000 as two characters rather than one, so the UTF-8 does not contain
 * any zeroes.
 *
 * @author Copyright (c) 2000 Morten Rolland, Screen Media
 *
 * @param utf8      Input string in UTF8 format.
 * @param cc        Number of bytes to convert.
 * @param unicode16 Destination buffer.
 * @return          Number of characters converted, or -1 if input is not
 *                  valid UTF8.
 */
static int utf8_to_utf16(const unsigned char *utf8, int cc, unsigned short *unicode16)
{
	int count = 0;
	unsigned char c0, c1;
	UINT32 scalar;

	while(--cc >= 0) {
		c0 = *utf8++;
		/*DPRINTF("Trying: %02x\n",c0);*/

		if ( c0 < 0x80 ) {
			/* Plain ASCII character, simple translation :-) */
			*unicode16++ = c0;
			count++;
			continue;
		}

		if ( (c0 & 0xc0) == 0x80 )
			/* Illegal; starts with 10xxxxxx */
			return -1;

		/* c0 must be 11xxxxxx if we get here => at least 2 bytes */
		scalar = c0;
		if(--cc < 0)
			return -1;
		c1 = *utf8++;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x20) ) {
			/* Two bytes UTF-8 */
			if ( (scalar != 0) && (scalar < 0x80) )
				return -1;	/* Overlong encoding */
			*unicode16++ = scalar & 0x7ff;
			count++;
			continue;
		}

		/* c0 must be 111xxxxx if we get here => at least 3 bytes */
		if(--cc < 0)
			return -1;
		c1 = *utf8++;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x10) ) {
			/*DPRINTF("####\n");*/
			/* Three bytes UTF-8 */
			if ( scalar < 0x800 )
				return -1;	/* Overlong encoding */
			if ( scalar >= 0xd800 && scalar < 0xe000 )
				return -1;	/* UTF-16 high/low halfs */
			*unicode16++ = scalar & 0xffff;
			count++;
			continue;
		}

		/* c0 must be 1111xxxx if we get here => at least 4 bytes */
		c1 = *utf8++;
		if(--cc < 0)
			return -1;
		/*DPRINTF("c1=%02x\n",c1);*/
		if ( (c1 & 0xc0) != 0x80 )
			/* Bad byte */
			return -1;
		scalar <<= 6;
		scalar |= (c1 & 0x3f);

		if ( !(c0 & 0x08) ) {
			/* Four bytes UTF-8, needs encoding as surrogates */
			if ( scalar < 0x10000 )
				return -1;	/* Overlong encoding */
			scalar -= 0x10000;
			*unicode16++ = ((scalar >> 10) & 0x3ff) + 0xd800;
			*unicode16++ = (scalar & 0x3ff) + 0xdc00;
			count += 2;
			continue;
		}

		return -1;	/* No support for more than four byte UTF-8 */
	}
	return count;
}

/* 
 * warning: the length of output string may exceed six x the length of the input 
 */ 
static int uc16_to_utf8(const unsigned short *us, int cc, unsigned char *s)
{
	int i;
	unsigned char *t = s;
	unsigned short uc16;
	
	for (i = 0; i < cc; i++) {
		uc16 = *us++;
		if (uc16 <= 0x7F) { 
			*t++ = (char) uc16;
		} else if (uc16 <= 0x7FF) {
			*t++ = 0xC0 | (unsigned char) ((uc16 >> 6) & 0x1F); /* upper 5 bits */
			*t++ = 0x80 | (unsigned char) (uc16 & 0x3F);        /* lower 6 bits */
		} else {
			*t++ = 0xE0 | (unsigned char) ((uc16 >> 12) & 0x0F);/* upper 4 bits */
			*t++ = 0x80 | (unsigned char) ((uc16 >> 6) & 0x3F); /* next 6 bits */
			*t++ = 0x80 | (unsigned char) (uc16 & 0x3F);        /* lowest 6 bits */
		}
	}
	*t = 0;
	return (t - s);
}


