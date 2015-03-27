#include <stdio.h>

#include "synchem.h"
#include "synio.h"
#include "rcb.h"
#include "utl.h"
#include "sling.h"
#include "xtr.h"
#include "name.h"
#include "sling2xtr.h"
#include "funcgroups.h"

#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

main ()
{
  char sling[512];
  String_t string_st;
  Sling_t sling_st;
  Xtr_t *xtr;
  FuncGroups_t *fg;

  IO_Init();
  FuncGroups_Init (FCB_SEQDIR_FNGP(""));

  fprintf(stderr,"Target Sling: ");
  fflush(stderr);
  scanf("%s",sling);
  string_st=String_Create(sling,strlen(sling));
  sling_st=String2Sling(string_st);
  xtr=Sling2Xtr_PlusHydrogen(sling_st);
  Xtr_Name_Set (xtr, NULL);
  printf ("merit=%d\n",Pst_Compound_Merit_Set (xtr));

  fprintf(stderr,"\nSling: ");
  fflush(stderr);
  scanf("%s",sling);
  string_st=String_Create(sling,strlen(sling));
  sling_st=String2Sling(string_st);
  xtr=Sling2Xtr_PlusHydrogen(sling_st);
/* must analyze resonance, aromaticity, rings, etc., etc. */
  Xtr_Name_Set (xtr, NULL);
  printf ("merit=%d\n",Pst_Merit_Compute (xtr));
}
