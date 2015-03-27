/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     CEDOT.C
*
*    Provides a means of dotting nodes whose CE relationship does not change as
*    a function of the reaction transform, thus eliminating potentially
*    time-consuming duplicate mappings not otherwise screened.
*
*  Creation Date:
*
*    16-Nov-2000
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

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_DOTS_
#include "dots.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_CEDOT_
#include "cedot.h"
#endif

void get_cebs_and_nbrs (Xtr_t *, char **, char **, Boolean_t *, int, int, Boolean_t, U32_t *);
void get_minp (int, char *, int *, Xtr_t *, int **, int *);
int *minpath (Xtr_t *, int *, int *, int *);
int *mincpath (Xtr_t *, int *, int *, int *);
int *maxpath (Xtr_t *, int *, int *, int *);
int *maxcpath (Xtr_t *, int *, int *, int *);

static int *path=NULL, psz;

void cedot (Xtr_t *gxtr, Xtr_t *sxtr, U32_t *roots_ptr, char *buffer)
{
/* 12/2/82 - GAM:
   WRITTEN TO SORT THROUGH THE PIECES OF A GOAL AND A SUBGOAL
   PATTERN AND FIND THOSE NODES THAT HAVE AT LEAST ONE CEB
   IN BOTH THE GOAL AND SUBGOAL PATTERNS, IGNORING THOSE
   WHICH ARE ISOLATED IN THE GOAL PATTERN, AND USING THE
   INFORMATION OBTAINED TO SUPPLEMENT THE DOTTING PROCEDURE. */
/* 12/9/82 - GAM:
   REVISED TO REQUIRE THAT A NODE WHOSE LIST OF NEIGHBORS CHANGES
   FROM GOAL TO SUBGOAL NOT BE CE-DOTTED UNLESS ITS LIST OF CEBS
   DOES NOT CHANGE - THIS WILL PREVENT THE LOSS OF SUBGOALS IN
   SCHEMATA LIKE THE PINACOL REARRANGEMENT, YET WILL RETAIN THE
   BENEFITS OF CE-DOTTING IN THOSE WHERE SYMMETRY IS PRESERVED. */
/* 12/12/85 - GAM:
   REVISED TO REQUIRE THAT A NODE WHICH IS CE TO THE ROOT OF ITS
   PIECE IN THE GOAL PATTERN BE UNDOTTED - THIS WILL PREVENT THE
   LOSS OF SUBGOALS IN SCHEMATA LIKE THE HYDROGENATION OF CYCLO-
   HEXENES, YET WILL RETAIN THE BENEFITS OF CE-DOTTING IN THOSE
   HAVING UNIQUE ROOT NODES. */
/* 12/17/85 - GAM:
   THE REVISION OF 12/12/85, WHILE SOLVING THE PROBLEM FOR THE SPECIFIC
   CASES THAT LED TO ITS DEVELOPMENT, WAS FOUND TO BE EXCESSIVE IN SOME
   OTHER CASES, AND IN STILL OTHERS DID NOT GO FAR ENOUGH TO RETRIEVE
   LOST SUBGOALS.  THIS MODIFICATION, WHICH INVOLVES DOTTING ONE OF THE
   SHORTEST CARBON-CONTAINING PATHS, IF ANY, BETWEEN THE ROOT NODE AND
   ITS CEBS, IS MERELY A HEURISTIC; HOWEVER, A NUMBER OF EXAMPLES, IN-
   CLUDING BUT NOT LIMITED TO THOSE FOR WHICH THE PREVIOUS REVISION
   WAS FOUND TO BE INADEQUATE, HAVE SHOWN IT TO BE SUPERIOR TO THAT
   REVISION.  FURTHER EVALUATION WILL HAVE TO AWAIT THE ANALYSIS OF ITS
   OVERALL PERFORMANCE IN THE ENVIRONMENTS OF SGLSHOT AND SYNCHM3. */
/* 12/18/85 - GAM: Reduced undotting to include up to the center of sym-
   metry along MINCPTH, or one atom beyond if the center is a bond. */

  int sz, gsz, i, *istack, istacksz;
  char **gcstr, **gnstr, **scstr, **snstr;
  Boolean_t *undot;

/*
DCL XPATHS ENTRY;
DCL MINCPTH ENTRY(PTR,(2) BIN FIXED(15),BIN FIXED(15),BIN FIXED(15))
   RETURNS(PTR);
DCL ZPATHS ENTRY(PTR,(2) BIN FIXED(15),BIT(1));
DCL SEPMOL ENTRY(PTR,PTR,BIN FIXED(15));
DCL MSEPMOL ENTRY(PTR,PTR,BIN FIXED(15),PTR);
DCL ANYCEBS ENTRY(PTR,PTR,CHAR(500) VAR) RETURNS(BIT(1));
DCL XXTR ENTRY;
DCL DEGREE ENTRY(PTR,BIN FIXED(15)) RETURNS(BIN FIXED(15));
DCL NEIGHBR ENTRY(PTR,BIN FIXED(15),BIN FIXED(15))
   RETURNS(BIN FIXED(15));
DCL ATOM_ID ENTRY(PTR,BIN FIXED(15)) RETURNS(BIN FIXED(15));
DCL (XTRTSD,TSDXTR) ENTRY(PTR) RETURNS(PTR);
DCL (FREEXTR,FREETSD) ENTRY(PTR);
DCL ATMSYMN ENTRY(CHAR(*) VAR) RETURNS(BIN FIXED(15));
DCL ATOMSYM ENTRY(BIN FIXED(15)) RETURNS(CHAR(4) VAR);
DCL XNAME ENTRY;
DCL GETNAME ENTRY(PTR,BIN FIXED(15)) RETURNS(CHAR(500) VAR);
DCL NMXTR ENTRY(CHAR(*) VAR) RETURNS(PTR);
DCL (GXTR,SXTR) PTR;
DCL DOTS(SZ) BIN FIXED(15) BASED(DOTS_PTR);
DCL ROOTS BIN FIXED(15) BASED(ROOTS_PTR);
DCL (SZ,I) BIN FIXED(15);
DCL (GCSTR(SZ) BASED(GCSTRPTR),GNSTR(SZ) BASED(GNSTRPTR),SCSTR(SZ) BASED
	(SCSTRPTR),SNSTR(SZ) BASED(SNSTRPTR)) CHAR(30) VAR;
DCL UNDOT(SZ) BIT(1) BASED(UNDOTP);
DCL UNDOTP PTR;
DCL ISTACK BIN FIXED(15) BASED(ISTACKPTR);
DCL (DOTS_PTR,ROOTS_PTR,GCSTRPTR,GNSTRPTR,SCSTRPTR,SNSTRPTR,ISTACKPTR) PTR;
DCL LENGTH BUILTIN;
DCL MOD BUILTIN;
DCL HBOUND BUILTIN;
DCL NULL BUILTIN;
DCL XLIFO ENTRY;
DCL ALLOCT ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR,BIN FIXED(15),CHAR(*) VAR,
	BIN FIXED(15));
DCL (ARRSIZ,ARRSIZX,ALLOCN) ENTRY(CHAR(*) VAR,CHAR(*) VAR)
	RETURNS(BIN FIXED(15));
DCL MEMPTR ENTRY(CHAR(*) VAR,CHAR(*) VAR,BIN FIXED(15),BIN FIXED(15),
	BIN FIXED(15)) RETURNS(PTR);
DCL (FREE,FREEX) ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR);
*/

  sz = gsz = Xtr_NumAtoms_Get (gxtr);
  while (Xtr_Attr_NumNeighbors_Get (gxtr, gsz - 1) == 0 && gsz != 0) gsz--; /* don't need to dot disconnected gp atoms! */

  istacksz = 0;
  istack = NULL;
  gcstr = (char **) malloc (gsz * sizeof (char *));
  gnstr = (char **) malloc (gsz * sizeof (char *));
  scstr = (char **) malloc (gsz * sizeof (char *));
  snstr = (char **) malloc (gsz * sizeof (char *));
  undot = (Boolean_t *) malloc (gsz * sizeof (Boolean_t));

  for (i = 0; i < gsz; i++)
    {
    undot[i] = FALSE;
    gcstr[i] = (char *) malloc (256 * sizeof (char)); /* should convert to String_t eventually - generous alloc for now */
    gnstr[i] = (char *) malloc (256 * sizeof (char));
    scstr[i] = (char *) malloc (256 * sizeof (char));
    snstr[i] = (char *) malloc (256 * sizeof (char));
    gcstr[i][0] = gnstr[i][0] = scstr[i][0] = snstr[i][0] = '\0';
    }

  get_cebs_and_nbrs (gxtr, gcstr, gnstr, undot, sz, gsz, FALSE, roots_ptr);
  get_cebs_and_nbrs (sxtr, scstr, snstr, NULL, sz, gsz, TRUE, NULL);

  for (i = gsz - 1; i >= 0; i--) if (undot[i])
    {
    Xtr_AttrFlags_Dot_Put (gxtr, i, FALSE);
    buffer += strlen (buffer);
    sprintf (buffer, "CEDOT: NO DOTTING FOR NODE %d (ON MINCPTH FROM ROOT TO A CEB)\n", i);
    }
  else if (!Xtr_AttrFlags_Dot_Get (gxtr, i) && gcstr[i][0] != '\0' && scstr[i][0] != '\0' &&
    (strcmp (gnstr[i], snstr[i]) == 0 || strcmp (gcstr[i], scstr[i]) == 0))
    {
    Xtr_AttrFlags_Dot_Put (gxtr, i, TRUE);
    istack = (int *) realloc (istack, (istacksz + 1) * sizeof (int));
    istack[istacksz++] = i;
    }

  if (istack == NULL) strcat (buffer, "CEDOT: NO ADDITIONAL DOTTING REQUIRED.\n");
  else
    {
    strcat (buffer, "CEDOT: THE FOLLOWING NODES NOT PREVIOUSLY DOTTED WERE FOUND TO REQUIRE DOTTING "
      "ON THE BASIS OF SYMMETRY COMMON\n\tTO BOTH GOAL AND SUBGOAL PATTERN -");
    for (i = istacksz - 1; i >= 0; i--)
      {
      buffer += strlen (buffer);
      sprintf (buffer, " %d", istack[i]);
      }
    free (istack);
istack=NULL;
    }

  for (i = 0; i < gsz; i++)
    {
    free (gcstr[i]);
gcstr[i]=NULL;
    free (gnstr[i]);
gnstr[i]=NULL;
    free (scstr[i]);
scstr[i]=NULL;
    free (snstr[i]);
snstr[i]=NULL;
    }
  free (gcstr);
gcstr=NULL;
  free (gnstr);
gnstr=NULL;
  free (scstr);
scstr=NULL;
  free (snstr);
snstr=NULL;
  free (undot);
undot=NULL;
}

void get_cebs_and_nbrs (Xtr_t *xtr, char **ceb_str, char **nbr_str, Boolean_t *undot, int numat, int gna, Boolean_t singles, U32_t *roots)
{
  Boolean_t found;
  Tsd_t *tsptr, *t;
  Xtr_t *x;
  int i, j, k, id, npc, *map, nat, pci, nodi, pcj, nodj, node_pair[2];
  struct molecs
    {
    struct molecs *next;
    Tsd_t *tptr;
    } *mptr, *temp;
  char **slptr, csln[512];
  struct class_st
    {
    int csz;
    char **class;
    } **clptr;

  tsptr = Xtr2Tsd (xtr);

  for (i = 0; i < gna; i++) /* no variables outside gna */
    {
    for (j = 0; j < MX_NEIGHBORS; j++) /* CHECK FOR AND REPLACE VARIABLE BONDS FOR NAME */
      if (Tsd_Atom_NeighborBond_Get (tsptr, i, j) == BOND_VARIABLE)
      Tsd_Atom_NeighborBond_Put (tsptr, i, j, BOND_SINGLE);

    if ((id = Tsd_Atomid_Get (tsptr, i)) >= VARIABLE_START && id <= VARIABLE_END) /* MAKE ALL $VARIABLES EQUIVALENT */
      Tsd_Atomid_Put (tsptr, i, VARIABLE_START);
    }

  map = (int *) malloc (2 * numat * sizeof (int));
  
  MSepMol (tsptr, (void **) &mptr, &npc, map);

  Tsd_Destroy (tsptr);


  clptr = (struct class_st **) malloc (npc * sizeof (struct class_st *));
  slptr = (char **) malloc (npc * sizeof (char *));
  for (i = 0; i < npc; i++)
  {
    clptr[i] = NULL;
    slptr[i] = NULL;
  }

  temp = mptr;

  for (i = 0; i < npc; i++)
    {
    clptr[i] = (struct class_st *) malloc (sizeof (struct class_st));

    mptr = temp;
    x = Tsd2Xtr (mptr->tptr);
    csln[0] = '\0';

    if ((nat = Tsd_NumAtoms_Get (mptr->tptr)) > 1)
      {
      clptr[i]->csz = nat;
      clptr[i]->class = (char **) malloc (nat * sizeof (char *));
      if (anycebs (x, clptr[i]->class, csln));
      }

    else if (singles)
      {
      clptr[i]->csz = 1;
      clptr[i]->class = (char **) malloc (sizeof (char *));
      strcpy (csln, Atomid2Symbol (Xtr_Attr_Atomid_Get (x, 0)));
      clptr[i]->class[0] = (char *) malloc (16 * sizeof (char));
      strcpy (clptr[i]->class[0], csln);
      if (strlen (csln) > 1) sprintf (csln, "(%s)", clptr[i]->class[0]);
      strcat (csln, CE_SEP NOCEBS);
      }

    else
      {
      clptr[i]->csz = 0;
      clptr[i]->class = NULL;
      }

    if (csln[0] == '\0') slptr[i] = NULL;
    else
      {
      slptr[i] = (char *) malloc (512 * sizeof (char));
      strcpy (slptr[i], csln);
      }

    Xtr_Destroy (x);
    Tsd_Destroy (mptr->tptr);
    temp = temp->next;
    free (mptr);
mptr=NULL;
    }

  for (i = 0; i < gna; i++) /* don't check atoms that are disconnected in the gp */
    {
    pci = map[2 * i];
    nodi = map[2 * i + 1];

if (slptr[pci] != NULL)
{
for (j = 0; j < (singles ? numat : gna); j++)
{
if (j != i)
      {
      pcj = map[2 * j];
      nodj = map[2 * j + 1];
if (slptr[pcj] != NULL)
        {
        if (pci != pcj)
          {
          if (strcmp (slptr[pci], slptr[pcj]) == 0 && strcmp (clptr[pci]->class[nodi], clptr[pcj]->class[nodj]) == 0)
            sprintf (ceb_str[i] + strlen (ceb_str[i]), "%3d", j);
          }
        else
          {
          if (strcmp (clptr[pci]->class[nodi], clptr[pci]->class[nodj]) == 0)
            sprintf (ceb_str[i] + strlen (ceb_str[i]), "%3d", j);

          for (k = 0, found = FALSE; k < Xtr_Attr_NumNeighbors_Get (xtr, j) && !found; k++)
            found = Xtr_Attr_NeighborId_Get (xtr, j, k) == i;

          if (found) sprintf (nbr_str[i] + strlen (nbr_str[i]), "%3d", j);
          }
        }
      }
}
}
    }

  if (!singles) /* i.e., if analyzing GOAL pattern */
    {
    for (j = 0; j < MX_ROOTS; j++) if (roots[j] != REACT_NODE_INVALID)
      {
      i = roots[j];
      get_minp (roots[j], ceb_str[i], map, xtr, &path, &psz);
      if (path != NULL) for (k = 1; k < (psz + 2) / 2; k++)
        undot[path[k]] = TRUE; /* ignore root; undot at least half of distance */
      }
    ZPaths (xtr, node_pair, TRUE);
    }

  free (map);
map=NULL;

  for (i = 0; i < npc; i++)
  {
    if (slptr[i] != NULL)
{
 free (slptr[i]);
 slptr[i]=NULL;
}
    if (clptr[i] != NULL)
    {
      if (clptr[i]->class != NULL)
      {
        for (j = 0; j < clptr[i]->csz; j++)
{
 free (clptr[i]->class[j]);
 clptr[i]->class[j]=NULL;
}
        free (clptr[i]->class);
clptr[i]->class=NULL;
      }
      free (clptr[i]);
clptr[i]=NULL;
    }
  }

  free (slptr);
slptr=NULL;
  free (clptr);
clptr=NULL;
}

void get_minp (int i, char *str, int *map, Xtr_t *x, int **p, int *s)
{
  int n[2], *ts, k, l, m, nn, node, pathnum;
  char nodestr[4];
  int **tp;

  n[0] = i;
*p=NULL;
*s=32767;
  l = strlen (str);
  nn = l / 3;
  ts = (int *) malloc (nn * sizeof (int));
  for (k = 0; k < nn; k++) ts[k] = 0;
  tp = (int **) malloc (nn * sizeof (int *));
  for (k = 0; k < l - 2; k += 3)
    {
/*
    m = (k + 2) / 3; *//* 1-justified version - obsolete *//*
*/
    m = k / 3;
    strncpy (nodestr, str + k, 3);
    nodestr[3]='\0';
    sscanf (nodestr, "%d", &node); /* extract nodes CE to root */
    n[1] = node;
    if (map[2 * node] == map[2 * i]) tp[m] = mincpath (x, n, &pathnum, ts+m);
    }
  for (k = 0; k < l / 3; k++) if (ts[k] > 0 && ts[k] < *s)
    {
    *p = tp[k];
    *s = ts[k];
    }
free (ts);
ts=NULL;
free(tp);
tp=NULL;
}
