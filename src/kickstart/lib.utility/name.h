#ifndef UTIL_NAME_H
#define UTIL_NAME_H

struct NamedObject 
{
	APTR	object;
};

#define	ANO_NameSpace	4000	/* Tag to define namespace	*/
#define	ANO_UserSpace	4001	/* tag to define userspace	*/
#define	ANO_Priority	4002	/* tag to define priority	*/
#define	ANO_Flags	4003	/* tag to define flags		*/

/* Flags for tag ANO_Flags */
#define	NSB_NODUPS	0
#define	NSB_CASE	1

#define	NSF_NODUPS	(1L << NSB_NODUPS)	/* Default allow duplicates */
#define	NSF_CASE	(1L << NSB_CASE)	/* Default to caseless... */

#endif
