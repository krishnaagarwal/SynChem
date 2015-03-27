/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     XPATHS.C
*
*    Traces and stores the various paths through a molecule that connect a pair
*    of nodes - currently used only as a part of the CEDOT process.
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
#include "xtr.h"

/*
XPATHS: PROC;
DCL TSDP PTR;
DCL NODES(2) BIN FIXED(15);
DCL (PATHSZ,PATHNUM) BIN FIXED(15);
DCL ALL BIT(1);
*/
/* GAM (12/13/85): Written originally to provide CEDOT with a means of
   dotting the nodes connecting a root and one of its CEBs, XPATHS was
   seen as having the potential for more general utility and was designed
   accordingly.  (MINCPTH is the entry written specifically for CEDOT.)
   Among the parameters, those included in every entry are TSDP (a pointer
   to a TSD structure) and NODES (a 2-member array containing the pair of
   nodes of interest).  As with XTR primitives, the XPATHS entries build
   the PATH_LIST structures as needed and retain them until specifically
   freed.  In contrast to XTR, however, freeing of path data is not cur-
   rently automatic upon call to FREETSD - the call(s) to ZPATHS must be
   explicitly made prior to a call to FREETSD, either upon individual
   node-pairs (ALL='0'B) or upon the entire TSD (ALL='1'B). */
/* GAM (REVISION) 2/25/86: Added check for NULL pointer in ZPATHS.
   Also added ZPATHS call to FREETSD. */
/*
DCL PATHP PTR INIT(NULL);
DCL (PREV_P,NEXT_P,DUMMYP) PTR;
DCL PATH(PATHSIZE) BIN FIXED(15) BASED(PATHP);
DCL PATHSIZE BIN FIXED(15);
DCL (I,TSIZE,DUMMY1,DUMMY2) BIN FIXED(15);
DCL 1 TSD BASED(TSDP),
   2 NA BIN FIXED(15),
   2 ROW(TSD.NA,11) BIN FIXED(15);
DCL PLHEAD PTR STATIC INIT(NULL);
DCL PLPTR PTR;
DCL 1 PATH_LIST BASED(PLPTR),
   2 NPATHS BIN FIXED(15),
   2 PNODES(2) BIN FIXED(15),
   2 PATHTSD PTR,
   2 PATHPTR(PATH_LIST.NPATHS) PTR,
   2 PSIZE(PATH_LIST.NPATHS) BIN FIXED(15),
   2 LINKPTR PTR;
DCL PLTEMP CHAR(6*NUM_PATHS+14) BASED(PLPTR);
DCL NUM_PATHS BIN FIXED(15);
DCL 1 TRACE EXT,
   2 OPTIONS(100) BIN FIXED(15),
   2 PARMS(100) BIN FIXED(15);
DCL DEBUG BIN FIXED(15) INIT(OPTIONS(88));
DCL XLIFO ENTRY;
DCL ALLOCT ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR,BIN FIXED(15),CHAR(*) VAR,
   BIN FIXED(15));
DCL ALLOCN ENTRY(CHAR(*) VAR,CHAR(*) VAR) RETURNS(BIN FIXED(15));
DCL MEMPTR ENTRY(CHAR(*) VAR,CHAR(*) VAR,BIN FIXED(15),BIN FIXED(15),
   BIN FIXED(15)) RETURNS(PTR);
DCL FREE ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR);
DCL NULL BUILTIN;
*/

typedef struct pathlist
{
  int npaths;
  int pnodes[2];
  Xtr_t *pathxtr;
  int **pathptr;
  int *psize;
  struct pathlist *linkptr;
} *PLPTR;

static PLPTR plhead = NULL;
static int num_paths, pathsize, *pathp, path_allocn = 0, path_node_allocn = 0;
static int **path_stack = NULL, *path_node_stack = NULL, *path_size_stack = NULL;
static Boolean_t *visit, **pair_found;

void ZPaths (Xtr_t *, int *, Boolean_t);
int *Path_Pointer (Xtr_t *, int *, int, int *);
void Store_Paths (Xtr_t *, int *);
void Trace_Paths (Xtr_t *, int, int);
void Remainder (Xtr_t *, int, int);
Boolean_t C_Path (Xtr_t *, int *, int);
Boolean_t Neighbor_Node (Xtr_t *, int, int);
int *minpath (Xtr_t *, int *, int *, int *);
int *mincpath (Xtr_t *, int *, int *, int *);
int *maxpath (Xtr_t *, int *, int *, int *);
int *maxcpath (Xtr_t *, int *, int *, int *);
void path_alloct (int);
void path_node_alloct ();
void path_free ();
void path_node_free ();

int NPaths (Xtr_t *xtr, int *nodes)
{
  int *dummyp;

  dummyp = Path_Pointer (xtr, nodes, 0, NULL);
  return (num_paths);
}

/*
NPATHS: ENTRY(TSDP,NODES) RETURNS(BIN FIXED(15));
DUMMYP=PATH_POINTER(TSDP,NODES,0,0);
RETURN(NUM_PATHS);
*/

int *path (Xtr_t *xtr, int *nodes, int pathnum, int *pathsz)
{
  return (Path_Pointer (xtr, nodes, pathnum, pathsz));
}

/*
PATH: ENTRY(TSDP,NODES,PATHNUM,PATHSZ) RETURNS(PTR);
RETURN(PATH_POINTER(TSDP,NODES,PATHNUM,PATHSZ));
*/

int *minpath (Xtr_t *xtr, int *nodes, int *pathnum, int *pathsz)
{
  int *dummyp;
  int i, tsize;

  dummyp = Path_Pointer (xtr, nodes, 1, pathsz);
  *pathnum = 1;
  for (i = 2; i <= num_paths; i++)
  {
    dummyp = Path_Pointer (xtr, nodes, i, &tsize);
    if (tsize < *pathsz)
    {
      *pathsz = tsize;
      *pathnum = i;
    }
  }
  return (Path_Pointer (xtr, nodes, *pathnum, pathsz));
}

/*
MINPATH: ENTRY(TSDP,NODES,PATHNUM,PATHSZ) RETURNS(PTR);
DUMMYP=PATH_POINTER(TSDP,NODES,1,PATHSZ);
PATHNUM=1;
DO I=2 TO NUM_PATHS;
   DUMMYP=PATH_POINTER(TSDP,NODES,I,TSIZE);
   IF TSIZE<PATHSZ THEN DO;
      PATHSZ=TSIZE;
      PATHNUM=I;
   END;
END;
RETURN(PATH_POINTER(TSDP,NODES,PATHNUM,PATHSZ));
*/

int *mincpath (Xtr_t *xtr, int *nodes, int *pathnum, int *pathsz)
{
  int *dummyp;
  int i, tsize;

  *pathnum = 0;
  *pathsz = 32767;
  dummyp = Path_Pointer (xtr, nodes, 1, &tsize);
  if (C_Path (xtr, dummyp, tsize))
  {
    *pathnum = 1;
    *pathsz = tsize;
  }
  for (i = 2; i <= num_paths; i++)
  {
    dummyp = Path_Pointer (xtr, nodes, i, &tsize);
    if (tsize < *pathsz && C_Path (xtr, dummyp, tsize))
    {
      *pathsz = tsize;
      *pathnum = i;
    }
  }
  if (*pathnum == 0)
  {
    *pathsz = 0;
    return (NULL);
  }
  return (Path_Pointer (xtr, nodes, *pathnum, pathsz));
}

/*
MINCPTH: ENTRY(TSDP,NODES,PATHNUM,PATHSZ) RETURNS(PTR);
PATHNUM=0;
PATHSZ=32767;
DUMMYP=PATH_POINTER(TSDP,NODES,1,TSIZE);
IF C_PATH(DUMMYP,TSIZE) THEN DO;
   PATHNUM=1;
   PATHSZ=TSIZE;
END;
DO I=2 TO NUM_PATHS;
   DUMMYP=PATH_POINTER(TSDP,NODES,I,TSIZE);
   IF TSIZE<PATHSZ THEN IF C_PATH(DUMMYP,TSIZE) THEN DO;
      PATHSZ=TSIZE;
      PATHNUM=I;
   END;
END;
IF PATHNUM=0 THEN DO;
   PATHSZ=0;
   RETURN(NULL);
END;
RETURN(PATH_POINTER(TSDP,NODES,PATHNUM,PATHSZ));
*/

int *maxpath (Xtr_t *xtr, int *nodes, int *pathnum, int *pathsz)
{
  int *dummyp;
  int i, tsize;

  dummyp = Path_Pointer (xtr, nodes, 1, &tsize);
  *pathnum = 1;
  for (i = 2; i <= num_paths; i++)
  {
    dummyp = Path_Pointer (xtr, nodes, i, &tsize);
    if (tsize > *pathsz)
    {
      *pathsz = tsize;
      *pathnum = i;
    }
  }
  return (Path_Pointer (xtr, nodes, *pathnum, pathsz));
}

/*
MAXPATH: ENTRY(TSDP,NODES,PATHNUM,PATHSZ) RETURNS(PTR);
DUMMYP=PATH_POINTER(TSDP,NODES,1,PATHSZ);
PATHNUM=1;
DO I=2 TO NUM_PATHS;
   DUMMYP=PATH_POINTER(TSDP,NODES,I,TSIZE);
   IF TSIZE>PATHSZ THEN DO;
      PATHSZ=TSIZE;
      PATHNUM=I;
   END;
END;
RETURN(PATH_POINTER(TSDP,NODES,PATHNUM,PATHSZ));
*/

int *maxcpath (Xtr_t *xtr, int *nodes, int *pathnum, int *pathsz)
{
  int *dummyp;
  int i, tsize;

  *pathnum = 0;
  *pathsz = 32767;
  dummyp = Path_Pointer (xtr, nodes, 1, &tsize);
  if (C_Path (xtr, dummyp, tsize))
  {
    *pathnum = 1;
    *pathsz = tsize;
  }
  for (i = 2; i <= num_paths; i++)
  {
    dummyp = Path_Pointer (xtr, nodes, i, &tsize);
    if (tsize > *pathsz)
    {
      *pathsz = tsize;
      *pathnum = i;
    }
  }
  if (*pathnum == 0)
  {
    *pathsz = 0;
    return (NULL);
  }
  return (Path_Pointer (xtr, nodes, *pathnum, pathsz));
}

/*
MAXCPTH: ENTRY(TSDP,NODES,PATHNUM,PATHSZ) RETURNS(PTR);
PATHNUM=0;
PATHSZ=0;
DUMMYP=PATH_POINTER(TSDP,NODES,1,TSIZE);
IF C_PATH(DUMMYP,TSIZE) THEN DO;
   PATHNUM=1;
   PATHSZ=TSIZE;
END;
DO I=2 TO NUM_PATHS;
   DUMMYP=PATH_POINTER(TSDP,NODES,I,TSIZE);
   IF TSIZE>PATHSZ THEN IF C_PATH(DUMMYP,TSIZE) THEN DO;
      PATHSZ=TSIZE;
      PATHNUM=I;
   END;
END;
IF PATHNUM=0 THEN DO;
   PATHSZ=0;
   RETURN(NULL);
END;
RETURN(PATH_POINTER(TSDP,NODES,PATHNUM,PATHSZ));
*/

void ZPaths (Xtr_t *xtr, int *nodes, Boolean_t all)
{
  PLPTR prev_p, next_p, plptr;
  int i, pathsize;
int j;
#ifdef _MIND_MEM_
  char varname[100];
#endif

  prev_p = NULL;
  plptr = plhead;
  while (plptr != NULL)
  {
    next_p = plptr->linkptr;
    if (xtr == plptr->pathxtr &&
      (all || (nodes[0] == plptr->pnodes[0] && nodes[1] == plptr->pnodes[1]) ||
      (nodes[0] == plptr->pnodes[1] && nodes[1] == plptr->pnodes[0])))
    {
      num_paths = plptr->npaths;
      for (i = 0; i < num_paths; i++)
      {
        pathsize = plptr->psize[i];
#ifdef _MIND_MEM_
        if (plptr->pathptr[i] != NULL)
{
 sprintf (varname, "plptr->pathptr[%d]", i);
 mind_free (varname, "xpaths", plptr->pathptr[i]);
 plptr->pathptr[i]=NULL;
}
      }
      if (prev_p == NULL) plhead = next_p;
      else prev_p->linkptr = next_p;
      mind_free ("plptr->pathptr", "xpaths", plptr->pathptr);
plptr->pathptr=NULL;
      mind_free ("plptr->psize", "xpaths", plptr->psize);
plptr->psize=NULL;
      mind_free ("plptr", "xpaths", plptr);
#else
        if (plptr->pathptr[i] != NULL)
{
 free (plptr->pathptr[i]);
 plptr->pathptr[i]=NULL;
}
      }
      if (prev_p == NULL) plhead = next_p;
      else prev_p->linkptr = next_p;
      free (plptr->pathptr);
plptr->pathptr=NULL;
      free (plptr->psize);
plptr->psize=NULL;
      free (plptr);
#endif
plptr=NULL;
    }
    else prev_p = plptr;
    plptr = next_p;
  }
}

/*
ZPATHS: ENTRY(TSDP,NODES,ALL);
PREV_P=NULL;
PLPTR=PLHEAD;
DO WHILE(PLPTR^=NULL);
   NEXT_P=LINKPTR;
   IF TSDP=PATHTSD &
      (ALL | (NODES(1)=PNODES(1) & NODES(2)=PNODES(2)) |
      (NODES(1)=PNODES(2) & NODES(2)=PNODES(1))) THEN DO;
      NUM_PATHS=NPATHS;
      DO I=1 TO NUM_PATHS;
         PATHSIZE=PSIZE(I);
         IF PATHPTR(I)^=NULL THEN FREE PATHPTR(I)->PATH;
      END;
      IF PREV_P=NULL THEN PLHEAD=NEXT_P;
      ELSE PREV_P->LINKPTR=NEXT_P;
      FREE PATH_LIST;
   END;
   ELSE PREV_P=PLPTR;
   PLPTR=NEXT_P;
END;
*/

int *Path_Pointer (Xtr_t *x, int *n, int i, int *size)
{
  PLPTR plptr;

  while (TRUE) /* i.e., one or two passes, depending upon whether the
                  paths have already been analyzed */
  {
    plptr = plhead;
    *size = 0;
    num_paths = 0;
    while (plptr != NULL) /* walk list to find correspondence with desired
                             combination of TSD and node-pair */
    {
      if (x == plptr->pathxtr &&
        ((n[0] == plptr->pnodes[0] && n[1] == plptr->pnodes[1]) || (n[0] == plptr->pnodes[1] && n[1] == plptr->pnodes[0])))
      {
        num_paths = plptr->npaths;
        if (i < 1 || i > num_paths) return (NULL);
        *size = plptr->psize[i - 1];
        return (plptr->pathptr[i - 1]);
      }
      plptr = plptr->linkptr;
    }
    Store_Paths (x, n); /* if not found, pursue analysis */
  }
}

/*
PATH_POINTER: PROC(T,N,I,SIZE) RETURNS(PTR);
DCL T PTR;
DCL (N(2),I,SIZE) BIN FIXED(15);
DCL BF31 BIN FIXED(31);
DO WHILE('1'B); *//* i.e., one or two passes, depending upon whether the
                   paths have already been analyzed *//*
   PLPTR=PLHEAD;
   SIZE=0;
   NUM_PATHS=0;
   DO WHILE(PLPTR^=NULL); *//* walk list to find correspondence with desired
                             combination of TSD and node-pair *//*
      IF T=PATHTSD &
         ((N(1)=PNODES(1) & N(2)=PNODES(2)) |
         (N(1)=PNODES(2) & N(2)=PNODES(1))) THEN DO;
         NUM_PATHS=NPATHS;
         IF I<1 | I>NUM_PATHS THEN RETURN(NULL);
         SIZE=PSIZE(I);
         IF DEBUG>2 THEN DO;
            UNSPEC(BF31)=UNSPEC(PATHPTR(I));
            PUT SKIP EDIT('PATHPTR(',I,')=',BF31)(A,F(2),A,F(11));
         END;
         RETURN(PATHPTR(I));
      END;
      PLPTR=LINKPTR;
   END;
   CALL STORE_PATHS; *//* if not found, pursue analysis *//*
END;
END;
*/

void Store_Paths (Xtr_t *xtr, int *nodes)
{
  PLPTR plptr;
  int *path_node, i, j, nummem, na;

#ifdef _MIND_MEM_
  char varname[100];

  na = Xtr_NumAtoms_Get (xtr);
  mind_malloc ("visit", "xpaths{1}", &visit, na * sizeof (Boolean_t));
  mind_malloc ("pair_found", "xpaths{1}", &pair_found, na * sizeof (Boolean_t *));
  for (i = 0; i < na; i++)
  {
    visit[i] = FALSE;
    sprintf (varname, "pair_found[%d]", i);
    mind_malloc (varname, "xpaths{1}", pair_found + i, na * sizeof (Boolean_t));
    for (j = 0; j < na; j++) pair_found[i][j] = FALSE;
  }
  visit[nodes[0]] = TRUE; /* starting node always flagged */
  if (nodes[0] != nodes[1]) Trace_Paths (xtr, nodes[0], nodes[1]);
  num_paths = path_allocn;
  if (num_paths == 0) /* trap special case to prevent access violation */
  {
    num_paths = 1;
    mind_malloc ("plptr", "xpaths{1}", &plptr, sizeof (struct pathlist));
    plptr->npaths = num_paths;
    plptr->linkptr = plhead;
    plhead = plptr;
    plptr->pathxtr = xtr;
    plptr->pnodes[0] = nodes[0];
    plptr->pnodes[1] = nodes[1];
mind_malloc("plptr->pathptr", "xpaths{1}", &plptr->pathptr, sizeof(int *));
    plptr->pathptr[0] = NULL;
mind_malloc("plptr->psize", "xpaths{1}", &plptr->psize, sizeof(int));
    plptr->psize[0] = 0;
    mind_free ("visit", "xpaths", visit);
visit=NULL;
    for (i = 0; i < na; i++)
{
 sprintf(varname,"pair_found[%d]",i);
 mind_free (varname, "xpaths", pair_found[i]);
 pair_found[i]=NULL;
}
    mind_free ("pair_found", "xpaths", pair_found);
pair_found=NULL;
    return;
  }
  mind_malloc ("plptr", "xpaths{1a}", &plptr, sizeof (struct pathlist));
  plptr->npaths = num_paths;
  plptr->linkptr = plhead;
  plhead = plptr;
  plptr->pathxtr = xtr;
  plptr->pnodes[0] = nodes[0];
  plptr->pnodes[1] = nodes[1];
  mind_malloc ("plptr->pathptr", "xpaths{1a}", &plptr->pathptr, num_paths * sizeof (int *));
  mind_malloc ("plptr->psize", "xpaths{1a}", &plptr->psize, num_paths * sizeof (int));
  for (i = 0; i < num_paths; i++)
  {
    pathp = path_stack[i];
    nummem = path_size_stack[i];
    plptr->psize[i] = pathsize = nummem;
    sprintf (varname, "plptr->pathptr[%d]", i);
    mind_malloc (varname, "xpaths{1}", plptr->pathptr + i,
                                        pathsize * sizeof (int)); /* copy XLIFO stack into explicit allocations
                                                                     of PATH - this removes the complication
                                                                     of having to FREE in non-LIFO order */
    for (j = 0; j < nummem; j++) plptr->pathptr[i][j] = pathp[j];
  }
  for (i = 0; i < num_paths; i++) path_free ();
  mind_free ("visit", "xpaths", visit);
visit=NULL;
  for (i = 0; i < na; i++)
{
sprintf(varname, "pair_found[%d]",i);
mind_free (varname, "xpaths", pair_found[i]);
pair_found[i]=NULL;
}
  mind_free ("pair_found", "xpaths", pair_found);
#else
  na = Xtr_NumAtoms_Get (xtr);
  visit = (Boolean_t *) malloc (na * sizeof (Boolean_t));
  pair_found = (Boolean_t **) malloc (na * sizeof (Boolean_t *));
  for (i = 0; i < na; i++)
  {
    visit[i] = FALSE;
    pair_found[i] = (Boolean_t *) malloc (na * sizeof (Boolean_t));
    for (j = 0; j < na; j++) pair_found[i][j] = FALSE;
  }
  visit[nodes[0]] = TRUE; /* starting node always flagged */
  if (nodes[0] != nodes[1]) Trace_Paths (xtr, nodes[0], nodes[1]);
  num_paths = path_allocn;
  if (num_paths == 0) /* trap special case to prevent access violation */
  {
    num_paths = 1;
    plptr = (PLPTR) malloc (sizeof (struct pathlist));
    plptr->npaths = num_paths;
    plptr->linkptr = plhead;
    plhead = plptr;
    plptr->pathxtr = xtr;
    plptr->pnodes[0] = nodes[0];
    plptr->pnodes[1] = nodes[1];
plptr->pathptr=(int **)malloc(sizeof(int *));
    plptr->pathptr[0] = NULL;
plptr->psize=(int *)malloc(sizeof(int));
    plptr->psize[0] = 0;
    free (visit);
visit=NULL;
    for (i = 0; i < na; i++)
{
 free (pair_found[i]);
 pair_found[i]=NULL;
}
    free (pair_found);
pair_found=NULL;
    return;
  }
  plptr = (PLPTR) malloc (sizeof (struct pathlist));
  plptr->npaths = num_paths;
  plptr->linkptr = plhead;
  plhead = plptr;
  plptr->pathxtr = xtr;
  plptr->pnodes[0] = nodes[0];
  plptr->pnodes[1] = nodes[1];
  plptr->pathptr = (int **) malloc (num_paths * sizeof (int *));
  plptr->psize = (int *) malloc (num_paths * sizeof (int));
  for (i = 0; i < num_paths; i++)
  {
    pathp = path_stack[i];
    nummem = path_size_stack[i];
    plptr->psize[i] = pathsize = nummem;
    plptr->pathptr[i] = (int *) malloc (pathsize * sizeof (int)); /* copy XLIFO stack into explicit allocations
                                                                     of PATH - this removes the complication
                                                                     of having to FREE in non-LIFO order */
    for (j = 0; j < nummem; j++) plptr->pathptr[i][j] = pathp[j];
  }
  for (i = 0; i < num_paths; i++) path_free ();
  free (visit);
visit=NULL;
  for (i = 0; i < na; i++)
{
free (pair_found[i]);
pair_found[i]=NULL;
}
  free (pair_found);
#endif
pair_found=NULL;
}

/*
STORE_PATHS: PROC;
DCL VISIT(TSD.NA) BIT(1) INIT((TSD.NA)('0'B));
DCL PAIR_FOUND(TSD.NA,TSD.NA) BIT(1) INIT((TSD.NA*TSD.NA)('0'B));
DCL PATH_NODE BIN FIXED(15) BASED(TPATHP);
DCL TPATHP PTR;
DCL (I,J,NUMMEM) BIN FIXED(15);
VISIT(NODES(1))='1'B; *//* starting node always flagged *//*
IF NODES(1)^=NODES(2) THEN CALL TRACE_PATHS(NODES(1),NODES(2));
NUM_PATHS=ALLOCN('PATH','XPATHS');
IF DEBUG>0 THEN PUT SKIP EDIT('XPATHS.STORE_PATHS: NUM_PATHS=',NUM_PATHS)
   (A,F(3));
IF NUM_PATHS=0 THEN DO; *//* trap special case to prevent access violation *//*
   NUM_PATHS=1;
   ALLOCATE PLTEMP;
   NPATHS=NUM_PATHS;
   LINKPTR=PLHEAD;
   PLHEAD=PLPTR;
   PATHTSD=TSDP;
   PNODES(1)=NODES(1);
   PNODES(2)=NODES(2);
   PATHPTR(1)=NULL;
   PSIZE(1)=0;
   RETURN;
END;
ALLOCATE PLTEMP;
NPATHS=NUM_PATHS;
LINKPTR=PLHEAD;
PLHEAD=PLPTR;
PATHTSD=TSDP;
PNODES(1)=NODES(1);
PNODES(2)=NODES(2);
DO I=1 TO NUM_PATHS;
   IF DEBUG>1 THEN PUT SKIP EDIT('I=',I)(A,F(3));
   PATHP=MEMPTR('PATH','XPATHS',I,NUMMEM,DUMMY2);
   IF DEBUG>1 THEN PUT SKIP EDIT('NUMMEM=',NUMMEM)(A,F(3));
   PSIZE(I)=NUMMEM;
   PATHSIZE=NUMMEM;
   ALLOCATE PATH SET(PATHPTR(I)); *//* copy XLIFO stack into explicit allocations
                                     of PATH - this removes the complication
                                     of having to FREE in non-LIFO order *//*
   DO J=1 TO NUMMEM;
      PATHPTR(I)->PATH(J)=PATH(J);
   END;
END;
DO I=1 TO NUM_PATHS;
   CALL FREE('PATH','XPATHS',PATHP); *//* empty PATH stack after copying *//*
END;
*/

void Trace_Paths (Xtr_t *xtr, int n1, int n2)
{
  int i;

  if (!Neighbor_Node (xtr, n1, n2)) return; /* if no connection */
  path_node_alloct ();
  path_node_stack[path_node_allocn - 1] = n1;
  for (i = 0; i < Xtr_NumAtoms_Get (xtr); i++)
    if (pair_found[n1][i]) Remainder (xtr, i, n2); /* construct all paths from node N1 */
  path_node_free ();
}

/*
TRACE_PATHS: PROC(N1,N2);
DCL (N1,N2) BIN FIXED(15);
DCL I BIN FIXED(15);
IF ^NEIGHBOR_NODE(N1,N2) THEN RETURN; *//* if no connection *//*
CALL ALLOCT('PATH_NODE','XPATHS.STORE_PATHS',TPATHP,0,'BIN FIXED',15);
PATH_NODE=N1;
IF DEBUG>2 THEN CALL DUMP('PATH_NODE','.STORE_PATHS');
DO I=1 TO NA;
   IF PAIR_FOUND(N1,I) THEN CALL REMAINDER(I,N2); *//* construct all paths from
                                                     node N1 *//*
END;
CALL FREE('PATH_NODE','XPATHS.STORE_PATHS',TPATHP);
END;
*/

void Remainder (Xtr_t *xtr, int n1, int n2)
{
  int i;

  if (n1 == n2) /* complete path constructed */
  {
    pathsize = path_node_allocn + 1;
    path_alloct (pathsize);
    path_stack[path_allocn - 1][pathsize - 1] = n2;
    for (i = pathsize - 2; i >= 0; i--) path_stack[path_allocn - 1][i] = path_node_stack[i];
    return;
  }
/* else find all unvisited continuations from node N1 */
  path_node_alloct ();
  path_node_stack[path_node_allocn - 1] = n1;
  visit[n1] = TRUE;
  for (i = 0; i < Xtr_NumAtoms_Get (xtr); i++) if (pair_found[n1][i] && !visit[i]) Remainder (xtr, i, n2);
  visit[n1] = FALSE;
  path_node_free ();
}

/*
REMAINDER: PROC(N1,N2) RECURSIVE;
DCL (N1,N2) BIN FIXED(15);
DCL I BIN FIXED(15);
IF DEBUG>0 THEN PUT SKIP EDIT
   ('XPATHS.STORE_PATHS.REMAINDER ENTERED: N1=',N1,', N2=',N2)(A,F(3),A,F(3));
IF N1=N2 THEN DO; *//* complete path constructed *//*
   PATHSIZE=ALLOCN('PATH_NODE','XPATHS.STORE_PATHS')+1;
   CALL ALLOCT('PATH','XPATHS',PATHP,PATHSIZE,'BIN FIXED',15);
   PATH(PATHSIZE)=N2;
   DO I=PATHSIZE-1 TO 1 BY -1;
      PATH(I)=MEMPTR('PATH_NODE','XPATHS.STORE_PATHS',I,DUMMY1,DUMMY2)->
         PATH_NODE;
   END;
   IF DEBUG>2 THEN CALL DUMP('PATH','');
   RETURN;
END;
*//* else find all unvisited continuations from node N1 *//*
CALL ALLOCT('PATH_NODE','XPATHS.STORE_PATHS',TPATHP,0,'BIN FIXED',15);
PATH_NODE=N1;
IF DEBUG>2 THEN CALL DUMP('PATH_NODE','.STORE_PATHS');
VISIT(N1)='1'B;
DO I=1 TO NA;
   IF PAIR_FOUND(N1,I) & ^VISIT(I) THEN CALL REMAINDER(I,N2);
END;
VISIT(N1)='0'B;
CALL FREE('PATH_NODE','XPATHS.STORE_PATHS',TPATHP);
END;
*/

Boolean_t Neighbor_Node (Xtr_t *xtr, int n1, int n2)
{
  int nbr, nnode;
  Boolean_t path_found;

  path_found = FALSE;
  for (nbr = 0; nbr < Xtr_Attr_NumNeighbors_Get (xtr, n1); nbr++) /* test all unvisited neighbors */
  {
    nnode = Xtr_Attr_NeighborId_Get (xtr, n1, nbr);
    if (!visit[nnode])
    {
      visit[nnode] = TRUE;
      if (nnode == n2) /* completed path found */
      {
        path_found = TRUE;
        pair_found[n1][n2] = TRUE;
      }
      else if (Neighbor_Node (xtr, nnode, n2)) /* test for "completability" */
      {
        path_found = TRUE;
        pair_found[n1][nnode] = TRUE;
      }
      visit[nnode] = FALSE;
    }
  }
  return (path_found);
}

/*
NEIGHBOR_NODE: PROC(N1,N2) RETURNS(BIT(1)) RECURSIVE;
DCL (N1,N2) BIN FIXED(15);
DCL NBR BIN FIXED(15);
DCL NNODE BIN FIXED(15);
DCL PATH_FOUND BIT(1) INIT('0'B);
IF DEBUG>0 THEN PUT SKIP EDIT
   ('XPATHS.STORE_PATHS.NEIGHBOR_NODE ENTERED: N1=',N1,', N2=',N2)
   (A,F(3),A,F(3));
DO NBR=5 TO 10; *//* test all unvisited neighbors *//*
   NNODE=ROW(N1,NBR);
   IF NNODE^=0 THEN IF ^VISIT(NNODE) THEN DO;
      VISIT(NNODE)='1'B;
      IF NNODE=N2 THEN DO; *//* completed path found *//*
         PATH_FOUND='1'B;
         PAIR_FOUND(N1,N2)='1'B;
      END;
      ELSE IF NEIGHBOR_NODE(NNODE,N2) THEN DO; *//* test for "completability" *//*
         PATH_FOUND='1'B;
         PAIR_FOUND(N1,NNODE)='1'B;
      END;
      VISIT(NNODE)='0'B;
   END;
END;
RETURN(PATH_FOUND);
END;
*/

/*
DUMP: PROC(V,E);
DCL (V,E) CHAR(*) VAR;
DCL X BIN FIXED(15) BASED(P);
DCL Y(NM) BIN FIXED(15) BASED(P);
DCL (I,J,NM,SZ) BIN FIXED(15);
DCL P PTR;
DCL BF31 BIN FIXED(31);
DO I=1 TO ALLOCN(V,'XPATHS'||E);
   P=MEMPTR(V,'XPATHS'||E,I,NM,SZ);
   IF NM=0 THEN PUT SKIP EDIT('XPATHS',E,'\',V,'(',I,')=',P->X)(5 A,F(2),A,
      F(6));
   ELSE DO J=1 TO NM;
      PUT SKIP EDIT('XPATHS',E,'\',V,'(',I,',',J,')=',P->Y(J))(5 A,2(F(2),A),
         F(6));
   END;
   UNSPEC(BF31)=UNSPEC(P);
   PUT SKIP EDIT('(ADDR=',BF31,')')(A,F(11),A);
END;
END;

END;
*/

Boolean_t C_Path (Xtr_t *xtr, int *p, int s)
{
  int i;

  pathp = p;
  pathsize = s;
  for (i = 0; i < s; i++) if (Xtr_Attr_Atomid_Get (xtr, i) == 6) return (TRUE);
  return (FALSE);
}

/*
C_PATH: PROC(P,S) RETURNS(BIT(1));
DCL P PTR;
DCL S BIN FIXED(15);
DCL I BIN FIXED(15);
PATHP=P;
PATHSIZE=S;
DO I=1 TO S;
   IF ROW(PATH(I),1)=6 THEN RETURN('1'B);
END;
RETURN('0'B);
END;
*/

void path_alloct (int nm)
{
#ifdef _MIND_MEM_
  char varname[100];

  path_allocn++;
  mind_realloc ("path_stack", "xpaths{2}", &path_stack, path_stack, path_allocn * sizeof (int *));
  sprintf(varname,"path_stack[%d]",path_allocn-1);
  mind_malloc (varname, "xpaths{2}", path_stack + path_allocn - 1, nm * sizeof (int));
  mind_realloc ("path_size_stack{2}", "xpaths", &path_size_stack, path_size_stack, path_allocn * sizeof (int));
#else
  path_allocn++;
  path_stack = (int **) realloc (path_stack, path_allocn * sizeof (int *));
  path_stack[path_allocn - 1] = (int *) malloc (nm * sizeof (int));
  path_size_stack = (int *) realloc (path_size_stack, path_allocn * sizeof (int));
#endif
  path_size_stack[path_allocn - 1] = nm;
}

void path_node_alloct ()
{
  path_node_allocn++;
#ifdef _MIND_MEM_
  mind_realloc ("path_node_stack", "xpaths{3}", &path_node_stack, path_node_stack, path_node_allocn * sizeof (int));
#else
  path_node_stack = (int *) realloc (path_node_stack, path_node_allocn * sizeof (int));
#endif
}

void path_free ()
{
#ifdef _MIND_MEM_
  char varname[100];

  path_allocn--;
if (path_allocn<0)
{
printf("attempt to free nonexistent path\n");
exit(1);
}
  sprintf(varname,"path_stack[%d]",path_allocn);
  mind_free (varname, "xpaths", path_stack[path_allocn]);
path_stack[path_allocn]=NULL;
  mind_realloc ("path_stack", "xpaths{4}", &path_stack, path_stack, path_allocn * sizeof (int *));
  mind_realloc ("path_size_stack", "xpaths{4}", &path_size_stack, path_size_stack, path_allocn * sizeof (int));
#else
  path_allocn--;
if (path_allocn<0)
{
printf("attempt to free nonexistent path\n");
exit(1);
}
  free (path_stack[path_allocn]);
path_stack[path_allocn]=NULL;
  path_stack = (int **) realloc (path_stack, path_allocn * sizeof (int *));
  path_size_stack = (int *) realloc (path_size_stack, path_allocn * sizeof (int));
#endif
}

void path_node_free ()
{
  path_node_allocn--;
if (path_node_allocn<0)
{
printf("attempt to free nonexistent path_node\n");
exit(1);
}
#ifdef _MIND_MEM_
  mind_realloc ("path_node_stack", "xpaths{5}", &path_node_stack, path_node_stack, path_node_allocn * sizeof (int));
#else
  path_node_stack = (int *) realloc (path_node_stack, path_node_allocn * sizeof (int));
#endif
}

/*
END;
*/
