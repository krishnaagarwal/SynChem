#include <stdio.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "rcb.h"
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

char *fgname[1000];
Boolean_t fgprsv[1000];
int fgend;

void backup_rxnlib ();

/*
void PstView_Handle_Grab ()
{
}

void SynView_Handle_Grab ()
{
}

void SynView_Exit ()
{
}
*/

main ()
{
	int i, j, k, done, ok;
	char buf[32768], line[1024], *b, *l;

	backup_rxnlib ();
	React_Init ("R+W//D/SYNCHEM/testdata/rxnlib");
	Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);
	for (j = done = 0; !done; j++)
		{
		i = React_NumSchemas_Get ();
		b = buf;
printf("i=%d j=%d\n",i,j);
		do
			{
			if (!fgets (line, 1024, stdin)) done = 1;
			else
				{
printf(line);
				if (l=strstr(line,"\r")) strcpy (l, l+1);
				strcpy (b, line);
				b += strlen(b);
				}
			}
		while (line[0] != '\n' && !done);
printf("done=%d\n",done);
		if (!done)
			{
			sscanf (buf, "%d", &k);
printf("k=%d i=%d\n%s\n",k,i,buf);
			ok=React_Schema_Import (i, buf);
printf("ok=%d; preparing to write\n",ok);
			React_Schema_Write (i);
printf("should have written record %d\n",i);
			Persist_Update_Rxn (i, k, FALSE, FALSE, "updrxns");
printf("should have updated %d with %d\n",k,i);
			}
		}
}
