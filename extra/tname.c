#include <stdio.h>

#include "synchem.h"
#include "synio.h"
#include "rcb.h"
#include "utl.h"
#include "sling.h"
#include "xtr.h"
#include "sling2xtr.h"
#include "name.h"
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

main ()
{
  char sling[512], *cansln;
  String_t string_st;
  Sling_t sling_st, canname;
  Xtr_t *xtr;
  Name_t *nptr;
Array_t tsdmap;
int *ce_class, nat, i, j, nbr, deg;

IO_Init();
  fprintf(stderr,"Sling: ");
  fflush(stderr);
  scanf("%s",sling);
  string_st=String_Create(sling,strlen(sling));
  sling_st=String2Sling(string_st);
  xtr=Sling2Xtr_PlusHydrogen(sling_st);
nat=Xtr_NumAtoms_Get(xtr);
ce_class=(int *)malloc(nat*sizeof(int));
for (i=0; i<nat; i++) ce_class[i]=-i-1;
  Xtr_Name_Set (xtr, &tsdmap);
  nptr = Xtr_Name_Get (xtr);
  canname = Name_Canonical_Get (nptr);
  cansln = Sling_Name_Get (canname);
printf("%s\n",cansln);
return;
  for (i=0; i<nat; i++)
{
/*
    if ((short) Array_1d16_Get (&tsdmap, i) < nat && (short) Array_1d16_Get (&tsdmap, i) >= 0)
      ce_class[i] = Name_CEMember_Get (nptr, Array_1d16_Get (&tsdmap, i));
    if ((short) Array_1d16_Get (&tsdmap, i) < nat && (short) Array_1d16_Get (&tsdmap, i) >= 0)
      ce_class[Array_1d16_Get(&tsdmap,i)] = Name_CEMember_Get (nptr, i);
    if ((short) Array_1d16_Get (&tsdmap, i) < nat && (short) Array_1d16_Get (&tsdmap, i) >= 0)
      ce_class[i] = Name_CEMember_Get (nptr, i);
*/
    if (Array_1d16_Get (&tsdmap, i) < nat) ce_class[i] = Name_CEMember_Get (nptr, i);
printf("ce_class[%d]=%d\n",i,ce_class[i]);
}
    for (i = 0; i < nat; i++) if (ce_class[i] >= 0)
      {
      deg = Xtr_Attr_NumNeighbors_Get (xtr, i);

      for (j = 0; j < deg; j++)
        {
        nbr = Xtr_Attr_NeighborId_Get (xtr, i, j);

        if (ce_class[nbr] < 0)
printf("ce_class[%d]=%d.%d\n",nbr,ce_class[i],Xtr_Attr_Atomid_Get(xtr,nbr)+500);
        }
      }
Xtr_Dump (xtr, &GStdOut);
}
