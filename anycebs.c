/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ANYCEBS.C
*
*    This module constructs the canonical sling for the transform piece and
*    analyzes its CE string to construct equivalence-class codes for the nodes,
*    for use with the CEDOT module.
*
*  Creation Date:
*
*    16-Feb-2000
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

#ifndef FALSE
#undef FALSE
#endif

#include "synchem.h"

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_NAME_
#include "name.h"
#endif

Boolean_t anycebs (Xtr_t *xtr, char **class_ptr, char *csln)
{
  Boolean_t any, anyse;
  Name_t *nptr;
  Array_t *eqclass; /* this is worthless!!! */
  Array_t  tsdmap;
  Sling_t  canname;
  char *sling, *cestr, *sestr; /* why weren't these preserved for outside use, along with non-empty EquivClasses??? */
  char ce_str[256], se_str[256];
  int numat, *ce_class, *se_class, nbr1, nbr2, deg, i, j, k;
#ifdef _MIND_MEM_
char varname[100];
#endif

/* 12/1/82 - GAM:
   WRITTEN FOR THE PURPOSE OF OBTAINING ALL CE CLASSES, INCLUDING
   THOSE FOR TERMINAL NODES, BOTH TO ENABLE AN EXPERIMENTAL SUPPLE-
   MENT TO THE DOTTING ALGORITHM AND TO SERVE AS A GENERAL-PURPOSE
   MODULE FOR SITUATIONS REQUIRING THE ADDITIONAL INFORMATION. */
/* 12/9/82 - GAM: TOTALLY REWRITTEN TO TAKE ADVANTAGE OF 'BLDNAME' */

  Xtr_Name_Set (xtr, &tsdmap);
  nptr = Xtr_Name_Get (xtr);
  canname = Name_Canonical_Get (nptr);
  sling = Sling_Name_Get (canname);
  cestr = strstr (sling, CE_SEP) + 1;
  sestr = strstr (cestr, SE_SEP) + 1;
  strcpy (ce_str, cestr);
  strcpy (se_str, sestr);
  cestr = strstr (ce_str, PARITY_SEP);
  *cestr = '\0';
  strcpy (csln, sling);
  cestr = strstr (csln, CE_SEP) + 1;
  cestr = strstr (cestr, PARITY_SEP);
  *cestr = '\0';

  numat = Xtr_NumAtoms_Get (xtr);

#ifdef _MIND_MEM_
  mind_malloc ("ce_class", "anycebs", &ce_class, numat * sizeof (int));
  mind_malloc ("se_class", "anycebs", &se_class, numat * sizeof (int));
#else
  ce_class = (int *) malloc (numat * sizeof (int));
  se_class = (int *) malloc (numat * sizeof (int));
#endif

  for (i = 0; i < numat; i++)
    {
    ce_class[i] = se_class[i] = -i-1;
#ifdef _MIND_MEM_
    sprintf (varname, "class_ptr[%d]", i);
    mind_malloc ("anycebs", varname, class_ptr + i, 16 * sizeof (char));
#else
    class_ptr[i] = (char *) malloc (16 * sizeof (char));
#endif
    sprintf(class_ptr[i],"%03d",i+500);
    }

  any = Name_Flags_AnyCE_Get (nptr);
  anyse = Name_Flags_AnySE_Get (nptr);

  if (numat == 2)
    {
    for (i = 0; i < 2; i ++) sprintf (class_ptr[i], "999.%03d", Xtr_Attr_Atomid_Get (xtr, i) + 500);
    any = strcmp (class_ptr[0], class_ptr[1]) == 0;
    }

  else
    {
/*
    eqclass = Name_EquivClasses_Get (nptr);
    EMPTY and WORTHLESS
*/

    for (i = 0; i < numat; i++)
      {
      if (Array_1d16_Get (&tsdmap, i) < numat)
        {
        if (any) ce_class[i] = Name_CEMember_Get (nptr, i);
        else ce_class[i] = i;
        }
      if (Array_1d16_Get (&tsdmap, i) < numat)
        {
        if (anyse) se_class[i] = Name_SEMember_Get (nptr, i);
        else se_class[i] = i;
        }
      }

    for (i = 0; i < numat; i++) if (ce_class[i] >= 0)
      {
      sprintf (class_ptr[i], "%03d", ce_class[i]);
      deg = Xtr_Attr_NumNeighbors_Get (xtr, i);

      for (j = 0; j < deg; j++)
        {
        nbr1 = Xtr_Attr_NeighborId_Get (xtr, i, j);

        if (ce_class[nbr1] < 0) sprintf (class_ptr[nbr1], "%s.%03d", class_ptr[i], Xtr_Attr_Atomid_Get (xtr, nbr1) + 500);
        }

      for (j = 0; j < deg - 1 && !any; j++)
        {
        nbr1 = Xtr_Attr_NeighborId_Get (xtr, i, j);

        for (k = j + 1; k < deg && !any; k++)
          {
          nbr2 = Xtr_Attr_NeighborId_Get (xtr, i, k);
          any = strcmp (class_ptr[nbr1], class_ptr[nbr2]) == 0;
          }
        }
      }
    }
#ifdef _MIND_MEM_
  mind_free ("ce_class", "anycebs", ce_class);
  mind_free ("se_class", "anycebs", se_class);
  mind_Array_Destroy ("tsdmap", "anycebs", &tsdmap);
#else
  free (ce_class);
  free (se_class);
  Array_Destroy (&tsdmap);
#endif
  return (any);
}
