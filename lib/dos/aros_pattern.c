/**
 * @file aros_pattern.c
 *
 * Taken from AROS, until we finish our own pattern
 */
/* PowerOS, Copyright (C) 2014.  All rights reserved. */

#include "dosbase_private.h"

/*
    Copyright © 1995-2013, The AROS Development Team. All rights reserved.
    $Id$

    Desc: Support functions for MatchFirst/MatchNext/MatchEnd
    Lang: english
*/

#define BYTE	INT8
#define LONG	INT32
#define ULONG	UINT32
#define WORD	INT16
#define UBYTE	UINT8
#include "aros.h"

struct marker
{
    UBYTE type; /* 0: Split 1: MP_NOT */
    CONST_STRPTR pat; /* Pointer into pattern */
    CONST_STRPTR str; /* Pointer into string */
};

struct markerarray
{
    struct markerarray *next;
    struct markerarray *prev;
    struct marker marker[128];
};

#define PUSH(t,p,s)                                                     \
{                                                                       \
    if(macnt==128)                                                      \
    {                                                                   \
        if(macur->next==NULL)                                           \
        {                                                               \
            macur->next=AllocVec(sizeof(struct markerarray),MEMF_ANY);  \
            if(macur->next==NULL)                                       \
                ERROR(ERROR_NO_FREE_STORE);                             \
            macur->next->prev=macur;                                    \
        }                                                               \
        macur=macur->next;                                              \
        macnt=0;                                                        \
    }                                                                   \
    macur->marker[macnt].type=(t);                                      \
    macur->marker[macnt].pat=(p);                                       \
    macur->marker[macnt].str=(s);                                       \
    macnt++;                                                            \
}

#define POP(t,p,s)                      \
{                                       \
    macnt--;                            \
    if(macnt<0)                         \
    {                                   \
        macnt=127;                      \
        macur=macur->prev;              \
        if(macur==NULL)                 \
            ERROR(0);                   \
    }                                   \
    (t)=macur->marker[macnt].type;      \
    (p)=macur->marker[macnt].pat;       \
    (s)=macur->marker[macnt].str;       \
}

#define MP_ESCAPE               0x81 /* Before characters in [0x81;0x8a] */
#define MP_MULT                 0x82 /* _#(_a) */
#define MP_MULT_END             0x83 /* #(a_)_ */
#define MP_NOT                  0x84 /* _~(_a) */
#define MP_NOT_END              0x85 /* ~(a_)_ */
#define MP_OR                   0x86 /* _(_a|b) */
#define MP_OR_NEXT              0x87 /* (a_|_b) */
#define MP_OR_END               0x88 /* (a|b_)_ */
#define MP_SINGLE               0x89 /* ? */
#define MP_ALL                  0x8a /* #? or * */
#define MP_SET                  0x8b /* _[_ad-g] */
#define MP_NOT_SET              0x8c /* _[~_ad-g] */
#define MP_DASH                 0x8d /* [ad_-g_] */
#define MP_SET_END              0x8e /* [ad-g_]_ */

BOOL patternMatch(CONST_STRPTR pat, CONST_STRPTR str, BOOL useCase,
                  pDOSBase DOSBase)
{
    CONST_STRPTR  s;
    BOOL    match = FALSE;

    struct markerarray   ma;
    struct markerarray  *macur = &ma;
    struct markerarray  *cur2;

    LONG   macnt = 0;
    LONG   cnt2;

    ULONG  level;
    UBYTE  a, b, c, t;

    LONG error;

#undef  ERROR
#define ERROR(a) { error = (a); goto end; }

    ma.next = ma.prev = NULL;

    while(TRUE)
    {
        switch((UINT8)*pat)
        {
        case P_REPBEG: /* _#(_a), _#a_ or _#[_a] */
            PUSH(0, ++pat, str);
            level = 1;

            while(TRUE)
            {
                c = *pat++;

                if(c == P_REPBEG)
                {
                    level++;
                }
                else if(c == P_REPEND)
                {
                    if(!--level)
                    {
                        break;
                    }
                }
            }
            break;
            
        case P_REPEND: /* #(a_)_ */
            level = 1;

            while(TRUE)
            {
                c = *--pat;

                if(c == P_REPEND)
                {
                    level++;
                }
                else if(c == P_REPBEG)
                {
                    if(!--level)
                    {
                        break;
                    }
                }
            }
            break;
            
        case P_NOT: /* _~(_a) */
            s = ++pat;
            level = 1;

            while(TRUE)
            {
                c = *s++;

                if(c == P_NOT)
                {
                    level++;
                }
                else if(c == P_NOTEND)
                {
                    if(!--level)
                    {
                        break;
                    }
                }
            }

            PUSH(1, s, str);
            break;

        case P_NOTEND: /* ~(a_)_ */
            cnt2 = macnt;
            cur2 = macur;

            do
            {
                cnt2--;

                if(cnt2 < 0)
                {
                    cnt2 = 127;
                    cur2 = cur2->prev;
                }
            }while(!cur2->marker[cnt2].type);

            if(!*str++)
            {
                macnt = cnt2;
                macur=cur2;
            }
            else if(str > cur2->marker[cnt2].str)
            {
                    cur2->marker[cnt2].str = str;
            }
            
            POP(t, pat, str);

            if(t && *str)
            {
                PUSH(1, pat, str + 1);
            }

            break;

        case P_ORSTART: /* ( */
            s = ++pat;
            level = 1;

            while(TRUE)
            {
                c = *s++;

                if(c == P_ORSTART)
                {
                    level++;
                }
                else if(c == P_ORNEXT)
                {
                    if(level == 1)
                    {
                        PUSH(0, s, str);
                    }
                }
                else if(c == P_OREND)
                {
                    if(!--level)
                    {
                        break;
                    }
                }
            }
            
            break;
            
        case P_ORNEXT: /* | */
            pat++;
            level = 1;

            while(TRUE)
            {
                c = *pat++;

                if(c == P_ORSTART)
                {
                    level++;
                }
                else if(c == P_OREND)
                {
                    if(!--level)
                    {
                        break;
                    }
                }
            }
            
            break;
            
        case P_OREND: /* ) */
            pat++;
            break;

        case P_SINGLE: /* ? */
            pat++;

            if(*str)
            {
                str++;
            }
            else
            {
                POP(t, pat, str);
                
                if(t && *str)
                { 
                    PUSH(1, pat, str + 1);
                }
            }

            break;

        case P_CLASS: /* [ */
            pat++;

            while(TRUE)
            {
                a = b = *pat++;

                if(a == P_CLASS)
                {
                    POP(t, pat, str);

                    if(t && *str)
                    { 
                        PUSH(1, pat, str + 1);
                    }

                    break;
                }
                
                if(*pat == '-')
                {
                    b = *++pat;

                    if(b == P_CLASS)
                    {
                        b = 255;
                    }
                }

                if(useCase)
                {
                    c = *str;
                }
                else
                {
                    c = ToUpper(*str);
                }

                if(c >= a && c <= b)
                {
                    str++;

                    while(*pat++ != P_CLASS)
                        ;

                    break;
                }
            }

            break;

        case P_NOTCLASS: /* [~ */
            if(!*str)
            {
                POP(t, pat, str);

                if(t && *str)
                {
                    PUSH(1, pat, str + 1);
                }
                
                break;
            }
            
            pat++;

            while(TRUE)
            {
                a = b = *pat++;

                if(a == P_CLASS)
                {
                    str++;
                    break;
                }

                if(*pat == '-')
                {
                    b = *++pat;

                    if(b == P_CLASS)
                    {
                        b = 255;
                    }
                }

                if(useCase)
                {
                    c = *str;
                }
                else
                {
                    c = ToUpper(*str);
                }

                if(c >= a && c <= b)
                {
                    POP(t, pat, str);

                    if(t && *str)
                    {
                        PUSH(1, pat, str + 1);
                    }

                    break;
                }
            }

            break;

        case P_ANY: /* #? */
            /* This often used pattern has extra treatment to be faster */
            if(*str)
            {
                PUSH(0, pat, str + 1);
            }

            pat++;
            break;

        case 0:
            if(!*str)
            {
                match = TRUE;
                ERROR(0);
            }
            else
            {
                POP(t, pat, str);

                if(t && *str)
                { 
                    PUSH(1, pat, str + 1);
                }
            }
            break;

        default:
            {
                UBYTE ch;

                if(useCase)
                {
                    ch = *str;
                }
                else
                {
                    ch = ToUpper(*str);
                }

                if(*pat++ == ch)
                {
                    str++;
                }
                else
                {
                    POP(t, pat, str);
                    
                    if(t && *str)
                    {
                        PUSH(1, pat, str + 1);
                    }
                }
            }

            break;

        } /* switch(*pat) */
    } /* while(TRUE) */ 

 end:
    
    macur = ma.next;
    
    while(macur != NULL)
    {
        struct markerarray *next = macur->next;
        
        FreeVec(macur); //, sizeof(struct markerarray));
        macur = next;
    }
    
    SetIoErr(error);
    
    return match;
}


LONG patternParse(CONST_STRPTR Source, STRPTR Dest, LONG DestLength,
    BOOL useCase, pDOSBase DOSBase)
{
    STRPTR  stack;
    STRPTR  end;
    UBYTE   a;
    LONG    iswild = 0;

#undef ERROR
#define ERROR(a) { SetIoErr(a); return -1; }
    stack = end = Dest + DestLength;
#define PUT(a) { if(Dest >= stack) ERROR(ERROR_BUFFER_OVERFLOW); *Dest++ = (a); }
    
    if(!*Source)
    {
        PUT(0);
        return 0;
    }
    
    while(*Source)
    {
        switch(*Source++)
        {
        case '#':
            iswild = 1;
            
            switch(*Source)
            {
            case '?':
                Source++;
                PUT(P_ANY);
                break;

            case ')':
            case '\0':
                ERROR(ERROR_BAD_TEMPLATE);
                break;

            default:
                PUT(P_REPBEG);
                *--stack = P_REPEND;
                continue;
            }

            break;

        case '~':
            switch(*Source)
            {
            case '\0':
                a = Source[-1];
                PUT(useCase ? a : ToUpper(a));
                break;
                
            case ')':
                ERROR(ERROR_BAD_TEMPLATE);
                break;

            default:
                iswild = 1;
                PUT(P_NOT);
                *--stack = P_NOTEND;
                continue;
            }

            break;

        case '?':
            iswild = 1;
            PUT(P_SINGLE);
            continue;

        case '(':
            PUT(P_ORSTART);
            *--stack = P_OREND;
            continue;

        case '|':
            iswild = 1;

            if(stack == end)
            {
                ERROR(ERROR_BAD_TEMPLATE);
            }
            
            while(!(*stack == P_OREND || stack == end))
            {
                PUT(*stack++);
            }
            
            PUT(P_ORNEXT);
            continue;

        case ')':
            while(!(stack == end || *stack == P_OREND))
            {
                PUT(*stack++);
            }
            
            if(stack == end)
            {
                ERROR(ERROR_BAD_TEMPLATE);
            }
            else
            {
                PUT(*stack++);
            }
            
            break;
            
        case '[':
            iswild = 1;

            if(*Source == '~')
            {
                Source++;
                PUT(P_NOTCLASS);
            }
            else
            {
                PUT(P_CLASS);
            }
            
            a = *Source++;

            if(!a)
            {
                ERROR(ERROR_BAD_TEMPLATE);
            }
            
            do
            {
                if(a == '\'')
                {
                    a=*Source++;
                }
                
                PUT(useCase ? a : ToUpper(a));
                a = *Source++;

                if(!a)
                {
                    ERROR(ERROR_BAD_TEMPLATE);
                }               
            } while(a != ']');

            PUT(P_CLASS);
            break;
#if 0
        case '*':
            if (DOSBase->dl_Root->rn_Flags & RNF_WILDSTAR)
            {
                iswild = 1;
                PUT(P_ANY);
            }
            else
            {
                PUT('*');
            }
            
            break;
#endif
        case '%':
            continue;

        case '\'':
            switch(*Source)
            {
            case '#':
            case '*':
            case '?':
            case '(':
            case '|':
            case ')':
            case '~':
            case '[':
            case ']':
            case '%':
            case '\'':
                Source++;
            default:
                break;
            }

            /* Fall through */
        default:
            a = Source[-1];
            PUT(useCase ? a : ToUpper(a));
            break;
        }

        while(stack != end && *stack != P_OREND)
        {
            PUT(*stack++);
        }
    }

    if(stack != end)
    {
        ERROR(ERROR_BAD_TEMPLATE);
    }
    
    PUT(0);

    return iswild;
}


BOOL dos_MatchPattern(pDOSBase DOSBase, const STRPTR pat, const STRPTR str)
{
	return patternMatch(pat, str, TRUE, DOSBase);
}

BOOL dos_MatchPatternNoCase(pDOSBase DOSBase, const STRPTR pat, const STRPTR str)
{
	return patternMatch(pat, str, FALSE, DOSBase);
}

INT32 dos_ParsePattern(pDOSBase DOSBase, const STRPTR Source, STRPTR Dest, INT32 DestLength)
{
	return patternParse(Source, Dest, DestLength, TRUE, DOSBase);
}

INT32 dos_ParsePatternNoCase(pDOSBase DOSBase, const STRPTR Source, STRPTR Dest, INT32 DestLength)
{
	return patternParse(Source, Dest, DestLength, FALSE, DOSBase);
}

