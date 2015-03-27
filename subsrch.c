/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*                   SUBSRCH.C
*
*    This module reads the encoded substructure list into memory from
*    the file bldsubt.out and searches for potential occurrences of those
*    substructures within each target molecule.  It supersedes, at least
*    temporarily, the substructure search contained in funcgroups.c,
*    which was found to miss certain attributes in an unpredictable way.
*    Except for the replacement of the binary search by a trie, the method
*    duplicates that used in the original PL/I code written by Rick Boivie.
*
*    The concept of preservable bonds is modified from its original form,
*    in that bonds between hydrogen and a heteroatom are no longer considered
*    preservable.  This opens up the possibility of performing peptide
*    chemistry, as one example of a transform that was needlessly prevented
*    by the original definition of preservability.
*
*  Routines:
*
*    initsub
*    subsrch
*    analyze
*    super_analyze
*    add_char
*    walk_trie
*
*  Creation Date:
*
*    17-Aug-2000
*
*  Authors:
*
*    Jerry Miller (rewritten based on PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
* 13-Sep-00  Miller     Added resetsub and free_trie.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"
#include "rcb.h"
#include "utl.h"
#include "sling.h"
#include "xtr.h"
#include "name.h"
#include "sling2xtr.h"
#include "isam.h"
#include "funcgroups.h"
#include "funcgroup_file.h"
#include "extern.h"

#define INVALID 255
#define GET_FGNUM 127
#define MORE 0
#define STAR 1

typedef struct fgtrie
{
	int fgnum;
	struct fgtrie *chr[13];
} FGT;

char *syms=" */0123456789";
unsigned char charmap[128];

static int num_atoms, num_fgs, num_ref, instance;
static U16_t *ref, **invalid_attr;
static char **fg;
static Boolean_t *preservable;
static U16_t *attr_position, *bfs_q;
static Boolean_t **bond_traced;
static int refcopy[2000];
static char fgcopy[2000][201];
static char *bond_string[] = {"00", "01", "02", "03", "04", "05", "06", "07", "00", "09", "10", "11", "12", "13", "14", "15", "16"};
static Array_t *attribute_table, *preservable_bonds;
static Xtr_t *xtr_p;
static FGT first, *fi;

void analyze (FGT *, int, int, int, int, /* int, int, */ char *, int);
void super_analyze (FGT *, int, int, int *, int *, /* int, int, int *, int *, */ int *, int, int, char *, int);
void add_char(FGT *,char *,int);
int walk_trie(FGT *,FGT **,char *,Boolean_t *);
void free_trie(FGT *);
Boolean_t Stereo_Dup (Xtr_t *, int, int);

/******************* INITSUB ***********************
Reads bldsubt.out into memory; returns number of FG
reference numbers.
***************************************************/

int initsub ()
{
  FILE *fgfile;
  int i, j, num_preserve,refr[6];
  char line[256],*ps;

#ifdef _MIND_MEM_
  char varname[100];

	fi=&first;

  fgfile=fopen(FCB_SEQDIR_FNGP ("/bldsubt.out"),"r");
  fgets (line, 255, fgfile);
  sscanf (line, "%d %d %d", &num_fgs, &num_ref, &num_preserve);

  mind_malloc("fg", "subsrch{1}", &fg, num_fgs * sizeof (char *));
  mind_malloc("ref", "subsrch{1}", &ref, num_fgs * sizeof (int));
  for (i=0; i<num_fgs; i++)
    {
    sprintf (varname, "fg[%d]", i);
    mind_malloc (varname, "subsrch{1}", fg + i, 201);
    }

  mind_malloc ("invalid_attr", "subsrch{1}", &invalid_attr, (num_ref + 1) * sizeof (U16_t *));
  mind_malloc ("preservable", "subsrch{1}", &preservable, (num_ref + 1) * sizeof (Boolean_t));
  for (i=0; i<=num_ref; i++)
    {
    sprintf (varname, "invalid_attr[%d]", i);
    mind_malloc (varname, "subsrch{1}", invalid_attr + i, 6 * sizeof (U16_t));
    }
#else
	fi=&first;

  fgfile=fopen(FCB_SEQDIR_FNGP ("/bldsubt.out"),"r");
  fgets (line, 255, fgfile);
  sscanf (line, "%d %d %d", &num_fgs, &num_ref, &num_preserve);

  fg=(char **) malloc(num_fgs * sizeof (char *));
  ref=(U16_t *) malloc(num_fgs * sizeof (int));
  for (i=0; i<num_fgs; i++) fg[i]=(char *) malloc (201);

  invalid_attr=(U16_t **) malloc ((num_ref + 1) * sizeof (U16_t *));
  preservable=(Boolean_t *) malloc ((num_ref + 1) * sizeof (Boolean_t));
  for (i=0; i<=num_ref; i++) invalid_attr[i]=(U16_t *) malloc (6 * sizeof (U16_t));
#endif

	charmap[0]=GET_FGNUM;
	for (i=1; i<128; i++) charmap[i]=INVALID;
	for (i=0; i<strlen(syms); i++) charmap[syms[i]]=i;
	for (i=0; i<13; i++) fi->chr[i]=NULL;
	fi->fgnum=0;

  for (i=0; i<num_fgs; i++)
  {
    fgets (line,255,fgfile);
    ps=strstr(line,"#");
    *ps=0;
    strcpy(fg[i],line+2); /* chop off "00" common to all */
    fgets (line,255,fgfile);
    sscanf(line,"%d",&j);
    ref[i]=j;
    add_char(fi,fg[i],j);
  }
  for (i=0; i<num_ref; i++) preservable[i]=FALSE;
  for (i=0; i<num_preserve; i++)
  {
    fgets(line,255,fgfile);
    sscanf(line,"%d",refr);
    preservable[refr[0]]=TRUE;
  }
  for (i=1; i<=num_ref; i++)
  {
    fgets(line,255,fgfile);
    sscanf(line,"%d %d %d %d %d %d",refr,refr+1,refr+2,refr+3,refr+4,refr+5);
    for (j=0; j<6; j++) invalid_attr[i][j]=refr[j];
  }

  fclose(fgfile);

return(num_ref);
}

/******************* RESETSUB ***********************
Removes FG trie from memory.
***************************************************/

void resetsub ()
{
  free_trie (&first);
}

/******************** SUBSRCH **************************
Sets up arrays for queue management and data collection;
makes initial call (recursion level 0) to analyze for
each non-H atom; filters out invalid attributes (i.e.,
those that are both more general than a specific
attribute found in the molecule at the same position and
less accurately representative of its chemistry at that
position; frees arrays prior to returning.
*******************************************************/

void subsrch (Xtr_t *xtr, FuncGroups_t *fgp)
{
  int invalid_inst, cand_inv_inst, numnonh;
  Boolean_t ok;
  static int h=1;  
  int refr, i, j, k, table_length, tblwidth, numfgc;
#ifdef _MIND_MEM_
  char varname[100];
#endif

  numfgc=0;
  xtr_p = xtr;
  numnonh = FuncGrp_NumNonHydrogen_Get (fgp);
  attribute_table = FuncGrp_Substructures_Get (fgp);
  preservable_bonds = FuncGrp_Preservable_Get (fgp);

  for (i=0; i<num_fgs; i++)
  {
    strcpy (fgcopy[numfgc], fg[i]);
    refcopy[numfgc++]=ref[i];
  }
  num_atoms=Xtr_NumAtoms_Get(xtr);
#ifdef _MIND_MEM_
  mind_malloc ("attr_position", "subsrch{2}", &attr_position, (num_atoms + 1) * sizeof (U16_t));
  mind_malloc ("bfs_q", "subsrch{2}", &bfs_q, (num_atoms + 1) * sizeof (U16_t));
  mind_malloc ("bond_traced", "subsrch{2}", &bond_traced, (num_atoms + 1) * sizeof (Boolean_t *));
  for (i=0; i<=num_atoms; i++)
    {
    sprintf (varname, "bond_traced[%d]", i);
    mind_malloc (varname, "subsrch{2}", bond_traced + i, (num_atoms + 1) * sizeof (Boolean_t));
    }
#else
  attr_position=(U16_t *) malloc ((num_atoms + 1) * sizeof (U16_t));
  bfs_q=(U16_t *) malloc ((num_atoms + 1) * sizeof (U16_t));
  bond_traced=(Boolean_t **) malloc ((num_atoms + 1) * sizeof (Boolean_t *));
  for (i=0; i<=num_atoms; i++) bond_traced[i]=(Boolean_t *) malloc ((num_atoms + 1) * sizeof (Boolean_t));
#endif

  for (instance=0; instance<num_atoms; instance++) if (Xtr_Attr_Atomid_Get(xtr,instance)!=h)
  {
    for (i=0; i<=num_atoms; i++)
    {
      attr_position[i]=bfs_q[i]=0;
      for (j=0; j<=num_atoms; j++) bond_traced[i][j]=FALSE;
    }
    analyze(fi,instance+1,0,0,0,/*1,numfgc,*/"",2);
  }
  tblwidth=numnonh;
  table_length=num_ref;
  for (i=1; i<=table_length; i++)
  {
    if (invalid_attr[i][0] > 0 && Array_2d16_Get (attribute_table, 0, i) > 0) for (j=1; j<=invalid_attr[i][0]; j++)
      for (instance=1; instance <= Array_2d16_Get (attribute_table, 0, i); instance++)
    {
      ok=TRUE;
      for (cand_inv_inst=1; cand_inv_inst <= Array_2d16_Get (attribute_table, 0, invalid_attr[i][j]) && ok; cand_inv_inst++)
      {
	invalid_inst=cand_inv_inst;
	ok = Array_2d16_Get (attribute_table, instance, i) != Array_2d16_Get (attribute_table, cand_inv_inst, invalid_attr[i][j]);
      }
      if (!ok)
      {
	Array_2d16_Put (attribute_table, 0, invalid_attr[i][j], Array_2d16_Get (attribute_table, 0, invalid_attr[i][j]) - 1);
	for (k=invalid_inst; k<tblwidth; k++)
          Array_2d16_Put (attribute_table, k, invalid_attr[i][j], Array_2d16_Get (attribute_table, k + 1, invalid_attr[i][j]));
      }
    }
  }

#ifdef _MIND_MEM_
  mind_free("attr_position", "subsrch", attr_position);
  mind_free("bfs_q", "subsrch", bfs_q);
  for (i=0; i<=num_atoms; i++)
    {
    sprintf (varname, "bond_traced[%d]", i);
    mind_free(varname, "subsrch", bond_traced[i]);
    }
  mind_free("bond_traced", "subsrch", bond_traced);
#else
  free(attr_position);
  free(bfs_q);
  for (i=0; i<=num_atoms; i++) free(bond_traced[i]);
  free(bond_traced);
#endif
}

/********************** ANALYZE ***********************
Adds a bond and the atom it introduces (or the position
of a previously referenced atom) to the growing string
defining a potential attribute and passes it along to
super_analyze to determine its existence and potential
for further expansion.
******************************************************/

void analyze (FGT *fgt, int node, int bond, int qhead, int qtail, /* int upper, int lower, */ char *attr, int offset)
{
  int father, new_qhead, new_qtail /*, new_lower, new_upper*/;
  char char3var[4], new_attr[251];

  father = bfs_q[qhead];
  bond_traced[node][father]=TRUE;
  bond_traced[father][node]=TRUE;
  strcpy (char3var, bond_string[bond]);
  sprintf (new_attr, "%s%s", attr, char3var);
  if (attr_position[node]==0)
  {
    new_qtail=qtail+1;
    bfs_q[new_qtail]=node;
    sprintf(char3var,"%3d",Xtr_Attr_Atomid_Get(xtr_p,node-1));
    attr_position[node]=new_qtail;
  }
  else
  {
    new_qtail=qtail;
    sprintf(char3var,"/%2d",attr_position[node]);
  }
  strcat (new_attr, char3var);
  super_analyze (fgt, qhead, qtail, &new_qhead, &new_qtail, /* upper, lower, &new_upper, &new_lower, */ &father, node, bond, new_attr, offset);
  if (bond!=0)
  {
    father=bfs_q[qhead];
    sprintf(new_attr,"%s%s%s",attr,bond_string[8],char3var);  /* varbond efficiency */
    super_analyze (fgt, qhead, qtail, &new_qhead, &new_qtail, /* upper, lower, &new_upper, &new_lower, */ &father, node, bond, new_attr, offset);
  }
/* return routine */
  if (qtail!=new_qtail)
  {
    attr_position[node]=0;
    bfs_q[new_qtail]=0;
  }
  father=bfs_q[qhead];
  bond_traced[father][node]=FALSE;
  bond_traced[node][father]=FALSE;
}

/********************* SUPER_ANALYZE ********************
Receives a growing attribute string from analyze; walks
the trie to determine whether it is recognized, and in
either case, whether further expansion is possible,
including expansion through the appending of cursor-
advancing stars (*); calls analyze, with which it is
mutually recursive, for each candidate for expansion.

For each attribute that is identified in full by the walk
of the trie, an instance and its position are added to
the attribute_table of the target, and its preservable
bonds are marked, subject to the new rule that H-hetero
bonds remain unmarked.

In walking the trie, super_analyze keeps track of the
offset that bypasses the substring that has already been
analyzed and the updated trie pointer corresponding to
that offset.  The purpose of this is to improve the
efficiency, hopefully approaching that of the binary
trie of Tito Autrey that it supersedes.
********************************************************/

void super_analyze (FGT *fgt, int qhead, int qtail, int *new_qhead, int *new_qtail, /*int upper, int lower, int *new_upper, int *new_lower,*/
  int *father, int node, int bond, char *new_attr, int offset)
{
  int i, j, l, syn_ref, num_neighbors, neighbor, new_mult/*, real_new_upper, real_new_lower*/, tmpref;
  char temp_attr[251];
  Boolean_t flags[2];
  FGT *next_fgt;

if (strncmp(new_attr,"00",2)!=0)
{
  fprintf(stderr,"Error: new_attr=\"%s\"\n",new_attr);
  return;
}

	syn_ref=walk_trie(fgt,&next_fgt,new_attr+offset,flags); /* chop off "00" common to all */
fgt=next_fgt;

  if (syn_ref != 0)
  {
    if (Array_2d16_Get (attribute_table, 0, syn_ref) == 0 ||
      Array_2d16_Get (attribute_table, Array_2d16_Get (attribute_table, 0, syn_ref), syn_ref) != instance)
    {
      Array_2d16_Put (attribute_table, 0, syn_ref, Array_2d16_Get (attribute_table, 0, syn_ref) + 1);
      Array_2d16_Put (attribute_table, Array_2d16_Get (attribute_table, 0, syn_ref), syn_ref, instance);
    }
    if (preservable[syn_ref])
      for (i=0; i<num_atoms; i++) for (j=0; j<num_atoms; j++)
    {
      if ((Xtr_Attr_Atomid_Get(xtr_p,i)!=6 && Xtr_Attr_Atomid_Get(xtr_p,j)==1) ||
        (Xtr_Attr_Atomid_Get(xtr_p,i)==1 && Xtr_Attr_Atomid_Get(xtr_p,j)!=6)) continue;
      if (bond_traced[i+1][j+1] && i!=instance && j!=instance)
        Array_2d1_Put (preservable_bonds, i, Xtr_Attr_NeighborIndex_Find (xtr_p, i, j), TRUE);
    }
  }

  if (!flags[MORE])
{
 return;
}

  *new_qhead=qhead;

offset=strlen(new_attr);
  while(flags[STAR])
  {
    strcat(new_attr,"*");
	tmpref=walk_trie(fgt,&next_fgt,new_attr+offset,flags); /* chop off "00" common to all */
fgt=next_fgt;
    ++*new_qhead;
offset++;
  }

  *father=bfs_q[*new_qhead];
  num_neighbors=Xtr_Attr_NumNeighbors_Get(xtr_p,*father-1);
  for (i=0; i<num_neighbors; i++)
  {
    neighbor=Xtr_Attr_NeighborId_Get (xtr_p, *father-1, i) + 1;
    if (!bond_traced[*father][neighbor])
    {
      new_mult=Xtr_Attr_NeighborBond_Get (xtr_p, *father-1, i);
      if (*new_qhead!=qhead) analyze(fgt, neighbor, new_mult, *new_qhead, *new_qtail, /* *new_upper, *new_lower, */ new_attr, offset);
      else if (new_mult < bond ||
        (new_mult==bond && (Xtr_Attr_Atomid_Get (xtr_p, neighbor - 1) < Xtr_Attr_Atomid_Get (xtr_p, node - 1) ||
        (Xtr_Attr_Atomid_Get (xtr_p, neighbor - 1) == Xtr_Attr_Atomid_Get (xtr_p, node - 1) &&
        !Stereo_Dup (xtr_p, node - 1, neighbor - 1)))))
        analyze(fgt, neighbor, new_mult, *new_qhead, *new_qtail, /* *new_upper, *new_lower, */ new_attr, offset);
    }
  }

  while(TRUE)
  {
    strcat (new_attr, "*");
	tmpref=walk_trie(fgt,&next_fgt,new_attr+offset,flags); /* chop off "00" common to all */
if (!flags[MORE])
{
 return;
}
fgt=next_fgt;
offset++;

  ++*new_qhead;

  while(flags[STAR])
  {
    strcat(new_attr,"*");
	tmpref=walk_trie(fgt,&next_fgt,new_attr+offset,flags); /* chop off "00" common to all */
fgt=next_fgt;
offset++;
    ++*new_qhead;
  }

    *father = bfs_q[*new_qhead];
    num_neighbors = Xtr_Attr_NumNeighbors_Get (xtr_p, *father - 1);
    for (i=0; i<num_neighbors; i++)
    {
      neighbor = Xtr_Attr_NeighborId_Get (xtr_p, *father - 1, i) + 1;
      if (!bond_traced[*father][neighbor])
        analyze (fgt,neighbor, Xtr_Attr_NeighborBond_Get (xtr_p, *father - 1, i), *new_qhead, *new_qtail,
        /* real_new_upper, real_new_lower, */ new_attr, offset);
    }
  }
}

/************************ STEREO_DUP ***************************
Compares two neighbors of father for stereochemical equivalence.
(They are assumed to have been screened as the same atom type.)

If not equivalent, returns FALSE; if equivalent, returns TRUE,
unless they are in ascending order.  (This is intended to short-
circuit potential combinatorial duplication within super_analyze.)

SE is preferable to CE, because the occasional extra overhead is
a small price to pay for future compatibility and completeness.
***************************************************************/

Boolean_t Stereo_Dup (Xtr_t * xtr, int node1, int node2)
{
  int i;

  if (node1 < node2 || Xtr_Name_Get (xtr) == NULL) return (FALSE);

  if (Xtr_Attr_NumNeighbors_Get (xtr, node1) != Xtr_Attr_NumNeighbors_Get (xtr, node2)) return (FALSE);

  if (Xtr_Attr_NumNeighbors_Get (xtr, node1) == 1) return (TRUE); /* bypass SE string for terminal atoms */

/*
  return (Name_StereochemicalEquivalence_Get (xtr, (U16_t) node1, (U16_t) node2));
*/
  return (Name_StereochemicalEquivilance_Get /* [sic] */ (xtr, (U16_t) node1, (U16_t) node2));
}

/************************ ADD_CHAR *************************
Builds the FG's into the trie on a character-by-character
basis, treating the NUL character (corresponding to the
GET_FGNUM mapping) as the signal to store the appropriate
FG number in the current trie member.

This function is only invoked during the one-time invocation
of initsub, and is therefore left in its original recursive
form.
***********************************************************/

void add_char(FGT *fgt,char *ststr,int fgnum)
{
	int which,i;
#ifdef _MIND_MEM_
        char varname[100];
#endif

	which=charmap[ststr[0]];
	if (which==INVALID)
	{
		fprintf(stderr,"Error: Invalid character seen: ");
		if (ststr[0]>=' ' && ststr[0]<='~') printf("'%c'.\n",ststr[0]);
		else printf("'\\%03o'.\n",ststr[0]);
		exit(1);
	}
	if (which==GET_FGNUM)
	{
		if (fgt->fgnum!=0)
		{
			fprintf(stderr,"Error: Attempting to insert FG#%d where FG#%d already resides.\n",
				fgnum,fgt->fgnum);
			exit(1);
		}
		fgt->fgnum=fgnum;
		return;
	}
	if (fgt->chr[which]==NULL)
	{
#ifdef _MIND_MEM_
                sprintf (varname, "fgt->chr[%d]", which);
		mind_malloc(varname, "subsrch{3}", fgt->chr + which, sizeof(FGT));
#else
		fgt->chr[which]=(FGT *)malloc(sizeof(FGT));
#endif
		for (i=0; i<13; i++) fgt->chr[which]->chr[i]=NULL;
		fgt->chr[which]->fgnum=0;
	}
	add_char(fgt->chr[which],ststr+1,fgnum);
}

/************************** WALK_TRIE *****************************
Originally designed as a recursive prototype intended to be walked
in its entirety relative to the start of the growing attribute
string, this function was both redesigned to be iterative, rather
than recursive, and to update the pointer to allow the existing
context to be retained as a point of continuation in subsequent
walks built onto the current one.
******************************************************************/

int walk_trie(FGT *fgt,FGT **next_fgt,char *ststr,Boolean_t *flags)
{
	int which, i;

	flags[MORE]=flags[STAR]=FALSE;
	while(TRUE)
	{
		*next_fgt=fgt;
		which=charmap[*ststr++];
		if (which==INVALID)
		{
			ststr--;
			fprintf(stderr,"Error: Invalid character seen: ");
			if (ststr[0]>=' ' && ststr[0]<='~') printf("'%c'.\n",ststr[0]);
			else printf("'\\%03o'.\n",ststr[0]);
			exit(1);
		}
		if (fgt==NULL) return(0);
		if (which==GET_FGNUM)
		{
			if (fgt->chr[charmap['*']]) flags[STAR]=TRUE;
			for (i=0; i<13; i++) if (fgt->chr[i])
			{
				flags[MORE]=TRUE;
				if (syms[i]>'*') flags[STAR]=FALSE;
			}
			return(fgt->fgnum);
		}
		fgt=fgt->chr[which];
	}
}

/************************** FREE_TRIE *****************************
Recursively walk to end of each branch; free structure upon return.
******************************************************************/

void free_trie(FGT *fgt)
{
  int i;

  for (i=0; i<13; i++) if (fgt->chr[i]!=NULL) free_trie(fgt->chr[i]);
#ifdef _MIND_MEM_
  if (fgt!=&first) mind_free("fgt", "subsrch", fgt);
#else
  if (fgt!=&first) free(fgt);
#endif
}
