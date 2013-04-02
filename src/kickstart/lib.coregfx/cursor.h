#ifndef CURSOR_H
#define CURSOR_H

typedef struct Cursor {
	INT32		width;			/* cursor width in pixels*/
	INT32		height;			/* cursor height in pixels*/
	INT32		hotx;			/* relative x pos of hot spot*/
	INT32		hoty;			/* relative y pos of hot spot*/
	UINT32		fgcolor;		/* foreground color*/
	UINT32		bgcolor;		/* background color*/
	UINT16		image[MAX_CURSOR_SIZE*2];/* cursor image bits*/
	UINT16		mask[MAX_CURSOR_SIZE*2];/* cursor mask bits*/
} Cursor_T;

#endif
