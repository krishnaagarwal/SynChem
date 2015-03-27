/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     RXL_IMPORT.C
*
*    This utility reads an ASCII dump of the reaction library created by the
*    RXL_EXPORT utility, using it to construct a replica of the library.
*
*  Creation Date:
*
*    11-Oct-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "rcb.h"

#include "avldict.h"

#include "isam.h"

#include "reaction.h"

#include "persist.h"

#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

main ()
{
	FILE *pf;
	int i, j, done, ok, nrecs, rec[6];
	char buf[32768], line[1024], *b, *l, junk[4][256];
	Isam_Control_t SDataCB;
	Isam_Control_t STextCB;

	strcpy (IO_FileName_Get (Isam_File_Get (&SDataCB)), "./data/rxnlib/rkbdata.isam");
	strcpy (IO_FileName_Get (Isam_File_Get (&STextCB)), "./data/rxnlib/rkbtext.isam");
	Isam_Open (&SDataCB, ISAM_TYPE_RKBDATA, ISAM_OPEN_INIT);
	Isam_Close (&SDataCB);
	Isam_Open (&STextCB, ISAM_TYPE_RKBTEXT, ISAM_OPEN_INIT);
	Isam_Close (&STextCB);
	pf=fopen("./data/rxnlib/rkbstd.inx","w");
	fgets (line, 1024, stdin);
	sscanf (line, "%s %s %d", junk, junk+1, &nrecs);
	fgets (line, 1024, stdin);
	fgets (line, 1024, stdin);
	fgets (line, 1024, stdin);
	for (j = 0; j < nrecs; j++)
		{
printf("kka:Reaction#%d\n",j);
		fgets (line, 1024, stdin);
		sscanf (line, "%d %x %x %s %s %s %s", &i, rec, rec+1, junk, junk+1, junk+2, junk+3);
		if (i != j)
			{
			printf ("Error in persist dump\n");
			exit(1);
			}
		for (i = 0; i < 4; i++) if (strcmp (junk[i], "none") == 0) rec[i+2] = PER_NONE;
		else sscanf (junk[i], "%d", rec+i+2);
		fwrite (rec, sizeof(int), 6, pf);
		}
	fclose(pf);
	React_Init ("R+W./data/rxnlib");
	for (j = done = 0; !done; j++)
		{
		b = buf;
		do
			{
			if (!fgets (line, 1024, stdin)) done = 1;
			else
				{
				if (l=strstr(line,"\r")) strcpy (l, l+1);
				strcpy (b, line);
				b += strlen(b);
				}
			}
		while (line[0] != '\n' && !done);
		if (!done)
			{
			sscanf (buf, "%d", &i);
			ok=React_Schema_Import (i, buf);
			React_Schema_Write (i);
			}
		}
}
