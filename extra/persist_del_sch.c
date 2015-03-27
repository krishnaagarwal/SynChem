#include <stdio.h>

#include "synchem.h"
#include "avldict.h"

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

main (int argc, char *argv[])
{
  int schn;

  if (argc != 2 || ((schn = atoi (argv[1])) == 0 && argv[1][0] != '0'))
    {
    printf("missing or invalid schema number\n");
    exit(1);
    }
  React_Init ("//D/SYNCHEM/testdata/rxnlib");
  Persist_Open ("//D/SYNCHEM/testdata/rxnlib/rkbstd.inx", NULL, NULL, 0);
  Persist_Delete_Rxn (schn, 0);
  Persist_Close ();
}
