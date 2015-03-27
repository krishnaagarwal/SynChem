#include <stdio.h>

#include "synchem.h"
#include "avldict.h"

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

main ()
{
  React_Init ("/home/synchem/data/rxnlib");
  Persist_Open ("/home/synchem/data/rxnlib/rkbstd.inx", NULL, NULL, 1);
printf("before close\n");
  Persist_Close ();
printf("after close\n");
}
