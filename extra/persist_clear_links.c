#include <stdio.h>

#define _H_PERSIST_GLOBAL_
#include "persist.h"
#undef _H_PERSIST_GLOBAL_

main (int argc, char *argv[])
{
	int recnum;
	FILE *f;

	if (argc != 2)
	{
		printf("Usage: %s <recnum>\n",argv[0]);
		exit(1);
	}
	recnum=atoi(argv[1]);
	if (recnum==0 && argv[1][0]!='0')
	{
		printf("Usage: %s <recnum>\n",argv[0]);
		exit(1);
	}
	f=fopen("//D/SYNCHEM/testdata/rxnlib/rkbstd.inx","r+b");
	fseek(f,recnum*PER_RXNINFO_SIZE,SEEK_SET);
	fread(rxn_info[0],PER_RXNINFO_SIZE,1,f);
	rxn_info[0][0].replacement_rec = rxn_info[0][0].temp_replacement_rec = rxn_info[0][0].replaces_which_rec =
		rxn_info[0][0].replaces_which_temp_rec = PER_NONE;
	fseek(f,recnum*PER_RXNINFO_SIZE,SEEK_SET);
	fwrite(rxn_info[0],PER_RXNINFO_SIZE,1,f);
	fclose(f);
}
