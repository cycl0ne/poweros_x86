#include "utility.h"
#include "utility_interface.h"

static inline int isupper(int c) { return (c >= 'A' && c <= 'Z'); }
static inline int islower(int c) { return (c >= 'a' && c <= 'z'); }
static inline int toupper(int c) {	if (islower(c)) return 'A' - 'a' + c; return c; }
static inline int tolower(int c) { if (isupper(c)) return 'a' - 'A' + c; return c; }

UINT8 util_ToUpper(pUtility UtilBase, UINT8 c)
{
	if (islower(c)) return 'A' - 'a' + c; return c;
}

UINT8 util_ToLower(pUtility UtilBase, UINT8 c)
{
	if (isupper(c)) return 'a' - 'A' + c; return c; 
}

INT32 util_Strlen(pUtility UtilBase, const char *str)
{
	const char *s; for (s = str; *s; ++s); return(s - str); 
}

UINT8 *util_Strncpy(pUtility UtilBase, char *dst, const char *src, INT32 n)
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
	return ((UINT8*)dst);
}

INT32 util_Stricmp(pUtility UtilBase, const char *s1, const char *s2)
{
    while (tolower(*s1) == tolower(*s2) && *s1 != '\0')
    {
        s1++;
        s2++;
    }

    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

INT32 util_Strnicmp(pUtility UtilBase, const char *s1, const char *s2, INT32 n)
{
	if (s1 == NULL || s2 == NULL) return (0);

	if (n != 0) 
	{
		const unsigned char *us1 = (const unsigned char *)s1, *us2 = (const unsigned char *)s2;
		do 
		{
			if (tolower(*us1) != tolower(*us2++)) return (tolower(*us1) - tolower(*--us2));
			if (*us1++ == '\0') break;
		} while (--n != 0);
	}
	return (0);
}

INT32 util_Strcmp(pUtility UtilBase, const char *s1, const char *s2)
{
    while (*s1 == *s2 && *s1 != '\0')
    {
        s1++;
        s2++;
    }

    return (int)(unsigned char)*s1 - (int)(unsigned char)*s2;
}

char *util_Strcpy(pUtility UtilBase, char *to, const char *from)
{
	char *save = to;
	for (; (*to = *from) != '\0'; ++from, ++to);
	return(save);
}

STRPTR util_Strchr(pUtility UtilBase, STRPTR s, int c)
{
    char ch = c;
    do
    {
        if (*s == ch) return s;
    } while (*s++);
    return NULL;
}

STRPTR util_Strrchr( pUtility UtilBase, CONST_STRPTR s, int c )
{
	UINT32 i = 0;
	while ( s[i++] );
	do
	{
		if ( s[--i] == (char) c ) return (char *) s + i;
	} while ( i );
	return NULL;
}

UINT32 util_Strcspn( pUtility UtilBase, CONST_STRPTR s1, CONST_STRPTR s2 )
{
	UINT32 len = 0;
	const char * p;
	while ( s1[len] )
	{
		p = s2;
		while ( *p )
		{
			if ( s1[len] == *p++ ) return len;
		}
		++len;
	}
	return len;
}

STRPTR util_Strtok(pUtility UtilBase, STRPTR s1, CONST_STRPTR s2)
{
	static char * tmp = NULL;
	const char * p = s2;

	if ( s1 != NULL ) tmp = s1;
	else 
	{
		if ( tmp == NULL ) return NULL;
		s1 = tmp;
	}

	/* skipping leading s2 characters */
	while ( *p && *s1 )
	{
		if ( *s1 == *p )
		{
			++s1;
			p = s2;
			continue;
		}
		++p;
	}

	/* no more to parse */
	if ( ! *s1 ) return ( tmp = NULL );

	/* skipping non-s2 characters */
	tmp = s1;
	while ( *tmp )
	{
		p = s2;
		while ( *p )
		{
			if ( *tmp == *p++ )
			{
				/* found seperator; overwrite with '\0', position tmp, return */
				*tmp++ = '\0';
				return s1;
			}
		}
		++tmp;
	}
    tmp = NULL;
    return s1;
}

INT32 util_Strspn(pUtility UtilBase, const char * str, const char * accept) 
{
	const char * ptr = str;
	const char * acc;

	while (*str) 
	{
		for (acc = accept; *acc; ++acc) if (*str == *acc) break;
		if (*acc == '\0') break;
		str++;
	}
	return str - ptr;
}

char *util_Strpbrk(pUtility UtilBase, const char * str, const char * accept) 
{
	const char *acc = accept;

	if (!*str) return NULL;
	while (*str) {
		for (acc = accept; *acc; ++acc) if (*str == *acc) break;
		if (*acc) break;
		++str;
	}
	if (*acc == '\0') return NULL;
	return (char *)str;
}

static INT32 lfind(const char * str, const char accept) 
{
	INT32 i = 0;
	while ( str[i] != accept) i++;
	return (INT32)(str) + i;
}

char * util_Strtok_r(pUtility UtilBase, char * str, const char * delim, char ** saveptr) 
{
	char * token;
	if (str == NULL) {
		str = *saveptr;
	}
	str += Strspn((CONST_STRPTR)str, delim);
	if (*str == '\0') {
		*saveptr = str;
		return NULL;
	}
	token = str;
	str = Strpbrk((CONST_STRPTR)token, delim);
	if (str == NULL) {
		*saveptr = (char *)lfind(token, '\0');
	} else {
		*str = '\0';
		*saveptr = str + 1;
	}
	return token;
}

STRPTR util_Strcat(pUtility UtilBase, STRPTR s1, CONST_STRPTR s2 )
{
	STRPTR rc = s1;
	if ( *s1 ) while ( *++s1 );
	while ( (*s1++ = *s2++) );
	return rc;
}

#define SysBase UtilBase->SysBase

STRPTR util_StrDup(pUtility UtilBase, CONST_STRPTR s )
{
	STRPTR ns = NULL;
	if(s)
	{
		UINT32 len = Strlen((STRPTR)s) + 1;
		ns = AllocVec(len, MEMF_FAST);
		if (ns) Strncpy(ns, (STRPTR)s, len);
	}
	return ns;
}
