
#define FONT_SYSTEM_VAR		"System"	/* winFreeSansSerif 11x13 (ansi)*/
#define FONT_SYSTEM_FIXED	"SystemFixed"	/* X6x13 (should be ansi)*/
#define FONT_GUI_VAR		"System"	/* deprecated (was "Helvetica")*/
#define FONT_OEM_FIXED		"SystemFixed"	/* deprecated (was "Terminal")*/

void gen_unloadfont(struct CoreGfxBase *CoreGfxBase,pCGfxFont pfont);
void gen_drawtext(struct CoreGfxBase *CoreGfxBase,pCGfxFont pfont, CRastPort *rp, INT32 x, INT32 y, const void *text, int cc, UINT32 flags);
void gen_gettextbits(struct CoreGfxBase *CoreGfxBase,pCGfxFont pfont, int ch, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
void gen_gettextsize(struct CoreGfxBase *CoreGfxBase,pCGfxFont pfont, const void *text, int cc, UINT32 flags, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
BOOL gen_getfontinfo(struct CoreGfxBase *CoreGfxBase,pCGfxFont pfont, pCGfxFontInfo pfontinfo);
void gen_setfontproc(struct CGfxCoreFont *pf);

void dbcs_gettextbits(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, int ch, UINT32 flags, const UINT16 **retmap, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
void dbcs_gettextsize(CoreGfxBase *CoreGfxBase, pCGfxFont pfont, const unsigned short *str, int cc, UINT32 flags, INT32 *pwidth, INT32 *pheight, INT32 *pbase);
