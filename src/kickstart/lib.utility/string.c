#include "utility.h"

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
	return (dst);
}

INT32 util_Stricmp(pUtility UtilBase, const char *s1, const char *s2)
{
	const unsigned char *us1 = (const unsigned char *)s1, *us2 = (const unsigned char *)s2;
	while (tolower(*us1) == tolower(*us2++)) if (*us1++ == '\0') return (0);
	return (tolower(*us1) - tolower(*--us2));
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
	while (*s1 == *s2++) if (*s1++ == 0) return (0);
	return (*(const unsigned char *)s1 - *(const unsigned char *)--s2);
}

char *util_Strcpy(pUtility UtilBase, char *to, const char *from)
{
	char *save = to;
	for (; (*to = *from) != '\0'; ++from, ++to);
	return(save);
}
