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
  FILE *f;

  f = fopen ("persist.dmp", "w");
  Persist_Dump_RxnInx (f);
  fclose (f);
}
