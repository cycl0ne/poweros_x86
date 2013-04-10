#include "vgagfx.h"
#include "coregfx.h"
#include "pixmap.h"
#include "rastport.h"
#include "exec_funcs.h"

#define R		0		/* RGBA parms*/
#define G		1
#define B		2
#define A		3

#define NONE	PORTRAIT_NONE
#define LEFT	PORTRAIT_LEFT
#define RIGHT	PORTRAIT_RIGHT
#define DOWN	PORTRAIT_DOWN

#define COPY	0		/* mode parm*/
#define SRCOVER	1

//__attribute__ ((always_inline))

