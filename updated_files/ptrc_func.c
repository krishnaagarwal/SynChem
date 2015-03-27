#include <stdio.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "app_resrc.h"
#include "utl.h"
#include "pst.h"
#include "xtr.h"
#include "sling.h"
#include "sling2xtr.h"
#include "reaction.h"
#include "dsp.h"
#include "avlcomp.h"

#define MAXX 1099
#define MAXY 849

typedef struct CMPSTK
  {
  SymTab_t *inx;
  Dsp_Molecule_t *cmolp;
  int level[2];
  Boolean_t semicolon;
  Boolean_t backref;
  Boolean_t start_of_princ_path;
  } CMPSTKST;

typedef struct DAD
  {
  Subgoal_t *subgoal;
  Compound_t *dnum;
  void *ptr;
  } DADST;

typedef struct NODES
  {
  int map;
  Compound_t *nnum;
  int merit;
  DADST dad;
  } NODEST;

typedef struct compound
  {
  SymTab_t *indexn;
  Dsp_Molecule_t *drawing;
  int comp_merit;
  int number_of_instances;
  NODEST *nodes; /* number_of_instances */
  } COMPOUND;

typedef struct charvar
  {
  char string[10000];
  int len;
  } CHARVAR;

typedef struct stringlist
  {
  int first_sgmerit;
  struct stringlist *back;
  struct stringlist *forward;
  int str_len;
  char *string;
  } STRINGLIST;

typedef union
  {
  struct BINFIX
    {
    short constant;
    short var;
    } binfix;
  char str4[4];
  } CHAR4;

typedef union
  {
  short bfix;
  char str2[2];
  } CHAR2;

Boolean_t pathtrc_standalone = TRUE;

static COMPOUND *rootcomp = NULL, *mainroot = NULL;
static CHARVAR paths_str, numstring;
static CHAR4 char4;
static CHAR2 char2;
static STRINGLIST *head = NULL, *tail = NULL, *last = NULL, *slptr = NULL;
static Boolean_t all, write_win = FALSE, first_dump_stacks = TRUE;
static SymTab_t *indx = NULL;
static int /* indx, */ range, num_insts, level = 0, minlevel = 0, nconj[1000], nstr[1000], ncmol = 0, ntp = 0, dc_lvl = 0;
static CMPSTKST *cmolpstack[1000];
static char **textpstack[1000];
static Display *disp = NULL;
static Window win;
static Pixmap pmap, scratchpad;
static GC gc;
static PstCB_t *pstcb = NULL;
static Dsp_Molecule_t *target_drawing = NULL;
static unsigned rgb[3];

void get_levels (int);
void initialize_static_vars ();
void transpCopyArea (Display *, Drawable, Drawable, Drawable, Boolean_t, Boolean_t, GC, int, int, unsigned, unsigned, int, int);
void sum_avl ();
void walk_list (Boolean_t, CHARVAR);
void free_all (COMPOUND *);
int subgoal_merit (Subgoal_t *, COMPOUND *);
int getnum (CHARVAR);
void display_path (int);
int dump_stacks (int, Boolean_t,
  Display *, Window, Pixmap, Pixmap, GC);
void *memptr (char *, int, int);
void alloct (char *, int, int, int);
Boolean_t find_in_stack (int, SymTab_t *);
int allocn (char *, int);
void afree (char *, int);
float fpmin (float, float);
float fpmax (float, float);
int fixmin (int, int);
int fixmax (int, int);
void clear_window ();
void gkstext (int, Boolean_t, Boolean_t, char *, float *);
void draw_line (CMPSTKST *, float, float, int, int, int, int, int, Boolean_t, char *, Boolean_t);
float int2flt (int, Boolean_t);
int flt2int (float, Boolean_t);
void draw_compounds (Dsp_Molecule_t **, /* int, int, */ Compound_t *, Subgoal_t *, int, char *, float *, float *, float *, int *,
  int *, Boolean_t);
void print_overall (float, float, float, int, int, int, char *, char *, char *);
int chpn (Subgoal_t *);
int lib (Subgoal_t *);
int schn (Subgoal_t *);
int s_ease (int);
int s_yield (int);
int s_conf (int);
char *schname (int);
Dsp_Molecule_t *getdsp (SymTab_t *, Boolean_t);
int conj_ea (Subgoal_t *, Compound_t *, Boolean_t);
int conj_ty (Subgoal_t *, Compound_t *, Boolean_t);
int conj_tc (Subgoal_t *, Compound_t *, Boolean_t);
int conj_eyc (Subgoal_t *, Compound_t *, char *, Boolean_t);
void drwcacm (float *, Dsp_Molecule_t *, char *, unsigned, Boolean_t, Boolean_t, Boolean_t);

#define ninst(a) SymTab_InstancesCnt_Get ((a))
#define avail(a) SymTab_Flags_Available_Get ((a))
#define slvd(a) SymTab_Flags_Solved_Get ((a))
#define csmert(a) SymTab_Merit_Solved_Get ((a))
#define cmerit(a) SymTab_Merit_Main_Get ((a))
#define ssmert(a) PstSubg_Merit_Solved_Get ((a))
#define smerit(a) PstSubg_Merit_Main_Get ((a))
#define cptr(a) SymTab_FirstComp_Get ((a))
#define inst(a) PstComp_Prev_Get ((a)) /* ??? BEFORE the "FIRST" instance ??? */
#define cfather(a) PstComp_Father_Get((a))
#define sfather(a) PstSubg_Father_Get((a))
#define solved(a) (PstSubg_Merit_Solved_Get((a)) != SUBG_MERIT_INIT)
#define cson(a) PstComp_Son_Get ((a))
#define sson(a) PstSubg_Son_Get ((a))
#define ncsons(a) PstComp_NumSons_Get ((a))
#define nssons(a) PstSubg_NumSons_Get ((a))
#define cbrthr(a) PstComp_Brother_Get ((a))
#define sbrthr(a) PstSubg_Brother_Get ((a))
#define note(a) 0
#define frcnode(a) FALSE
#define ease(a) PstSubg_Reaction_Ease_Get ((a))
#define yield(a) PstSubg_Reaction_Yield_Get ((a))
#define conf(a) PstSubg_Reaction_Confidence_Get ((a))
#define dev(a) SymTab_DevelopedComp_Get ((a))
#define persist_schn(a) PstSubg_Reaction_Schema_Get ((a))
#define TARGET PstSubg_Son_Get (PstCB_Root_Get (pstcb))
#define A 2 /* constants for subgoal_merit () */
#define B 2
#define C 1
#define D 7

Dsp_Molecule_t *getdsp (SymTab_t *symtab, Boolean_t ok2free)
{
  Sling_t sling, stsling;
  Xtr_t *xtr;
  Dsp_Molecule_t *dsp;

  stsling = SymTab_Sling_Get (symtab);
  sling = Sling_CopyTrunc (stsling);
  xtr = Sling2Xtr (sling);
  if (ok2free) Sling_Destroy (sling);
  dsp = Xtr2Dsp (xtr);
  if (ok2free) Xtr_Destroy (xtr);
  if (!dsp_Shelley (dsp))
  {
    fprintf (stderr, "Error in getdsp\n");
    //exit (1);//kka
  }
  dsp->map_em=FALSE;
  return (dsp);
}

int s_ease (int sch)
{
  React_Record_t *schrec;
  React_Head_t *sch_head;

  schrec = React_Schema_Handle_Get (sch);
  sch_head = React_Head_Get (schrec);
  return (React_Head_Ease_Get (sch_head));
}

int s_yield (int sch)
{
  React_Record_t *schrec;
  React_Head_t *sch_head;

  schrec = React_Schema_Handle_Get (sch);
  sch_head = React_Head_Get (schrec);
  return (React_Head_Yield_Get (sch_head));
}

int s_conf (int sch)
{
  React_Record_t *schrec;
  React_Head_t *sch_head;

  schrec = React_Schema_Handle_Get (sch);
  sch_head = React_Head_Get (schrec);
  return (React_Head_Confidence_Get (sch_head));
}

char *schname (int sch)
{
  React_Record_t *schrec;
  React_TextRec_t *txt;

  schrec = React_Schema_Handle_Get (sch);
  txt = React_Text_Get (schrec);
  return (String_Value_Get (React_TxtRec_Name_Get (txt)));
}

int chpn (Subgoal_t *subg)
{
  React_Record_t *schrec;
  React_Head_t *sch_head;

  schrec = React_Schema_Handle_Get (persist_schn (subg));
  sch_head = React_Head_Get (schrec);
  return (React_Head_SynthemeFG_Get (sch_head));
}

int lib (Subgoal_t *subg)
{
  React_Record_t *schrec;
  React_Head_t *sch_head;

  schrec = React_Schema_Handle_Get (persist_schn (subg));
  sch_head = React_Head_Get (schrec);
  return (React_Head_Library_Get (sch_head));
}

int schn (Subgoal_t *subg)
{
  React_Record_t *schrec;
  React_TextRec_t *txt;
  React_TextHead_t *txt_head;

  schrec = React_Schema_Handle_Get (persist_schn (subg));
  txt = React_Text_Get (schrec);
  txt_head = React_TxtRec_Head_Get (txt);
  return (React_TxtHd_OrigSchema_Get (txt_head));
}

float int2flt (int coor, Boolean_t vert)
{
  if (vert) return (1. - (float) coor / (float) MAXY);
  return ((float) coor / (float) MAXX);
}

int flt2int (float coor, Boolean_t vert)
{
  if (vert) return (MAXY - (int) (coor * (float) MAXY));
  return ((int) (coor * (float) MAXX));
}

float fpmin (float f1, float f2)
{
  if (f1 < f2) return (f1);
  return (f2);
}

float fpmax (float f1, float f2)
{
  if (f1 > f2) return (f1);
  return (f2);
}

int fixmin (int f1, int f2)
{
  if (f1 < f2) return (f1);
  return (f2);
}

int fixmax (int f1, int f2)
{
  if (f1 > f2) return (f1);
  return (f2);
}

void afree (char *name, int inx)
{
  if (strcmp (name, "CMOLPSTACK") == 0)
  {
    free (cmolpstack[--ncmol]);
    cmolpstack[ncmol]=NULL;
  }
  else if (strcmp (name, "TEXTPSTACK") == 0)
  {
    free (textpstack[--ntp]);
    textpstack[ntp]=NULL;
  }
  else if (strcmp (name, "TEXT") == 0)
  {
    if (inx >= ntp)
    {
      fprintf (stderr, "AFREE Error accessing TEXTPSTACK: index = %d; allocation = %d\n", inx, ntp);
      exit (1);
    }
    free (textpstack[inx][--nstr[inx]]);
    textpstack[inx][nstr[inx]]=NULL;
  }
  else
  {
    printf("Error in afree: %s\n", name);
    exit (1);
  }
}

int allocn (char *name, int inx)
{
  if (strcmp (name, "CMOLPSTACK") == 0) return (ncmol);
  else if (strcmp (name, "TEXTPSTACK") == 0) return (ntp);
  else if (strcmp (name, "TEXT") == 0)
  {
    if (inx >= ntp)
    {
      fprintf (stderr, "ALLOCN Error accessing TEXTPSTACK: index = %d; allocation = %d\n", inx, ntp);
      exit (1);
    }
    return (nstr[inx]);
  }
  else
  {
    printf("Error in allocn: %s\n", name);
    exit (1);
  }
}

void alloct (char *name, int inx, int nm, int sz)
{
  void *retval;
  char *string;

  if (strcmp (name, "CMOLPSTACK") == 0)
  {
    cmolpstack[ncmol] = (CMPSTKST *) malloc (nm * sizeof (CMPSTKST));
    nconj[ncmol++] = nm;
  }
  else if (strcmp (name, "TEXTPSTACK") == 0)
  {
    textpstack[ntp] = (char **) malloc (sizeof (char *));
    nstr[ntp++] = 0;
  }
  else if (strcmp (name, "TEXT") == 0)
  {
    if (inx >= ntp)
    {
      fprintf (stderr, "ALLOCT Error accessing TEXTPSTACK: index = %d; allocation = %d\n", inx, ntp);
      exit (1);
    }
    if (nstr[inx] > 0) textpstack[inx] = (char **) realloc (textpstack[inx], (nstr[inx] + 1) * sizeof (char *));
    string = (char *) malloc (sz);
    textpstack[inx][nstr[inx]++] = string;
  }
  else
  {
    printf("Error in alloct: %s\n", name);
    exit (1);
  }
}

Boolean_t find_in_stack (int meminx, SymTab_t *stinx)
{
  int i, j;

  for (i = 0; i < ncmol - 1; i++) if (cmolpstack[i]->inx == stinx)
  {
    cmolpstack[ncmol - 1][meminx].inx = cmolpstack[i]->inx;
    cmolpstack[ncmol - 1][meminx].semicolon = FALSE;
    cmolpstack[ncmol - 1][meminx].backref = cmolpstack[i]->semicolon;
    return (TRUE);
  }

  return (FALSE);
}

void *memptr (char *name, int inx2, int inx1)
{
  if (strcmp (name, "CMOLPSTACK") == 0)
  {
    if (inx1 >= ncmol)
    {
      fprintf (stderr, "MEMPTR Error accessing CMOLPSTACK: index = %d; allocation = %d\n", inx1, ncmol);
      exit (1);
    }
    return (cmolpstack[inx1]);
  }
  else if (strcmp (name, "TEXT") == 0)
  {
    if (inx2 >= ntp)
    {
      fprintf (stderr, "MEMPTR Error accessing TEXTPSTACK: index = %d; allocation = %d\n", inx2, ntp);
      exit (1);
    }
    if (inx1 >= nstr[inx2])
    {
      fprintf (stderr, "MEMPTR Error accessing TEXTPSTACK (%d): index = %d; allocation = %d\n", inx2, inx1, nstr[inx2]);
      exit (1);
    }
    return (textpstack[inx2][inx1]);
  }
  else
  {
    printf("Error in memptr: %s\n", name);
    exit (1);
  }
}

void input_ptrc (Compound_t **comps, int ncomps, COMPOUND **cpparm, STRINGLIST **slhparm, STRINGLIST **sltparm)
{
  int i, adj_ncomps;
  COMPOUND *comp, *prev_comp;
  STRINGLIST *path;
  Subgoal_t *prev_sg;

  for (i = adj_ncomps = 0, prev_comp = NULL, prev_sg = NULL; i < ncomps; i++)
  {
    if (comps[i] == NULL)
    {
      prev_comp->nodes->dad.subgoal = NULL;
      prev_comp->nodes->dad.dnum = NULL;
/*
      adj_ncomps++;
      comp = (COMPOUND *) malloc (sizeof (COMPOUND));
      prev_comp->nodes->dad.ptr = comp;
      comp->indexn = NULL;
      comp->drawing = NULL;
      comp->comp_merit = 0;
      comp->number_of_instances = 1;
      comp->nodes = (NODEST *) malloc (sizeof (NODEST));
      comp->nodes->map = 0;
      comp->nodes->nnum = NULL;
      comp->nodes->dad.subgoal = NULL;
      comp->nodes->dad.dnum = NULL;
      comp->nodes->dad.ptr = NULL;
      comp->nodes->merit = 0;
      prev_comp = comp;
*/
    }
    else
    {
      if (PstComp_Father_Get (comps[i]) != prev_sg && PstComp_Index_Get (comps[i]) != 1)
      {
        adj_ncomps++;
        comp = (COMPOUND *) malloc (sizeof (COMPOUND));
        if (prev_comp == NULL) *cpparm = comp;
        else prev_comp->nodes->dad.ptr = comp;
        comp->indexn = PstComp_SymbolTable_Get (comps[i]);
        comp->drawing = getdsp (comp->indexn, TRUE);
        if (SymTab_Flags_Available_Get (comp->indexn)) comp->comp_merit = 1100;
        else if (SymTab_Flags_Solved_Get (comp->indexn)) comp->comp_merit = 1000 + csmert (comp->indexn);
        else comp->comp_merit = cmerit (comp->indexn);
        comp->number_of_instances = 1;
        comp->nodes = (NODEST *) malloc (sizeof (NODEST));
        comp->nodes->map = 0;
        comp->nodes->nnum = comps[i];
        comp->nodes->dad.subgoal = cfather (comp->nodes->nnum);
        comp->nodes->dad.dnum = sfather (comp->nodes->dad.subgoal);
        comp->nodes->dad.ptr = NULL;
        comp->nodes->merit = subgoal_merit (comp->nodes->dad.subgoal, comp);
        prev_comp = comp;
      }
      prev_sg = PstComp_Father_Get (comps[i]);
    }
  }
/*
  *slhparm = (STRINGLIST *) malloc (sizeof (STRINGLIST));
  *sltparm = (STRINGLIST *) malloc (sizeof (STRINGLIST));
  (*slhparm)->back = (*sltparm)->forward = NULL;
  path = (STRINGLIST *) malloc (sizeof (STRINGLIST));
  (*slhparm)->forward = (*sltparm)->back = path;
  path->back = *slhparm;
  path->forward = *sltparm;
  path->str_len = 2 * adj_ncomps;
  path->string = (char *) malloc (path->str_len);
  for (i = 0; i < path->str_len; i++) path->string[i] = '\0';
*/
*slhparm=*sltparm=NULL;
}

void putptrc (CHARVAR paths_str_parm, COMPOUND *cpparm, STRINGLIST *slhparm, STRINGLIST *sltparm, Boolean_t detail,
  PstCB_t *pstcb_parm)
{
  int i;
  COMPOUND *tempcmp;

  pstcb = pstcb_parm;
  numstring.len = 0;
  char4.binfix.constant = -1;
  paths_str.len = paths_str_parm.len;
  for (i = 0; i < paths_str.len; i++) paths_str.string[i] = paths_str_parm.string[i];
  rootcomp = cpparm;
  for (tempcmp = mainroot = rootcomp; tempcmp != NULL; tempcmp = tempcmp->nodes->dad.ptr)
    if (tempcmp->nodes->dad.subgoal == NULL && tempcmp->nodes->dad.dnum == NULL) mainroot = tempcmp->nodes->dad.ptr;
/*
  head = slhparm;
  tail = sltparm;
  walk_list (detail, paths_str);
  return;
*/
target_drawing=getdsp(PstComp_SymbolTable_Get(TARGET), TRUE);
display_path(1);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
PATHTRC: PROC(NUMAVL);
%include 'sys$library:gksdefs.pli';
DCL NUMAVL CHAR(*) VAR;
/* GAM: 2/25/85 - Program to supplement the SYNCHEM output interpreter in
   the analysis of a status file for a given run.  Whereas the output
   interpreter traces proposed synthetic schemes retrosynthetically from
   the target molecule, PATHTRC solicits a particular precursor (which
   will, in most cases, be an available compound) and displays the most
   direct syntheses from that compound.  Because of the potential for
   combinatorial explosion, it is necessary to specify a range to limit
   the number of allowable steps; otherwise, many circuitous pathways
   would also be found.  Although the user may opt for all pathways
   satisfying the range criterion, the usual procedure will be to specify
   that only solved pathways be displayed. */
/* Revised: 4/24/85 (GAM) - Made merit calculations dynamic, rather than
   simple lookups in the status file, so that the values for a given path
   reflect the merit considerations relevant to that path (i.e., not in-
   fluenced by solutions or developments outside that particular path). */
/* Revised: 4/26/85 (GAM) - Added cumulative values of ease, yield, and
   confidence as part of the full display of paths. */
/* Revised: 7/15/85 (DJB) - Put in check and printing for protection
   schema mechanism. */
/* Revised: 2/13/86 (GAM) - Added METABOL external variable for generality;
   added forced-path recognition */
DCL NAVL BIN FIXED(15); /* parameter to PTRCENT */
DCL (INDX_PARM,MINPLUS_PARM) BIN FIXED(15), SOLV_PARM BIT(1);
   /* parameters to GETPTRC */
DCL PATHS_STR_PARM CHAR(*) VAR, (DETAIL,sysprint_open) BIT(1); /* parameters to
								  PUTPTRC */
DCL (CPPARM,SLHPARM,SLTPARM) PTR; /* parameters to GETPTRC, PUTPTRC */
DCL METABOL BIN FIXED(15) EXT;
dcl XDCL entry(char(*) var);
DCL (XSCREEN,PSCREEN) ENTRY RETURNS(PTR);
dcl (mouse,buttonn) entry returns(bin fixed(31));
dcl vs2menu entry((*) char(*) var,bin fixed(15)) returns(bin fixed(15));
dcl vs2menw entry(bin fixed(31),(*) char(*) var,bin fixed(15))
	returns(bin fixed(15));
dcl MenuBar entry((4) float,(*) char(*) var,bit(1));
DCL GETDVI ENTRY RETURNS(CHAR(80) VAR);
DCL DROPEN ENTRY;
DCL (XRLINFO,CREATE,RDSTRO,XRLIB,XREF,XLIFO,XNAME) ENTRY;
DCL FRCNODE ENTRY(BIN FIXED(15)) RETURNS(BIT(1));
DCL NOTE ENTRY(BIN FIXED(15)) RETURNS(BIN FIXED(15));
DCL LISTAVL ENTRY(BIN FIXED(15));
DCL FREEXTR ENTRY(PTR);
DCL UPRCASE ENTRY(CHAR(*) VAR) RETURNS(CHAR(500) VAR);
DCL HSLNXTR ENTRY(CHAR(*) VAR) RETURNS(PTR);
DCL SLINGOK ENTRY(CHAR(*) VAR) RETURNS(BIT(1));
DCL GETNAME ENTRY(PTR,BIN FIXED(15)) RETURNS(CHAR(500) VAR);
DCL HMATCH ENTRY(BIN FIXED(31)) RETURNS(BIN FIXED(15));
DCL HTSIZE ENTRY RETURNS(BIN FIXED(15));
DCL HASH ENTRY(CHAR(*) VAR,BIN FIXED(31)) RETURNS(BIN FIXED(31));
DCL (OXRLIB,ALLOREF) ENTRY(CHAR(1));
DCL (CXRLIB,FREEREF) ENTRY;
DCL FREFKEY ENTRY(BIN FIXED(15),BIN FIXED(15)) RETURNS(CHAR(8));
DCL SCHNAME ENTRY(BIN FIXED(15),BIN FIXED(15)) RETURNS(CHAR(200) VAR);
DCL (S_EASE,S_YIELD,S_CONF) ENTRY(BIN FIXED(15),BIN FIXED(15))
   RETURNS(BIN FIXED(15));
DCL GREFREC ENTRY(PIC'99999999',CHAR(3),CHAR(6),CHAR(*) VAR,PIC'99999999');
dcl alloct entry(char(*) var,char(*) var,ptr,bin fixed(15),char(*) var,
   bin fixed(15));
dcl (allocn,arrsiz) entry(char(*) var,char(*) var) returns(bin fixed(15));
dcl memptr entry(char(*) var,char(*) var,bin fixed(15),bin fixed(15),
   bin fixed(15)) returns(ptr);
DCL (free,FREEX) ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR);
DCL GETNUM ENTRY(CHAR(*) VAR) RETURNS(BIN FIXED(15));
DCL AVLDATA ENTRY(CHAR(7) VAR, CHAR(2) VAR) RETURNS(CHAR(200) VAR);
DCL AVLSRCH ENTRY(PTR,CHAR(*) VAR,BIT(1),CHAR(*) VAR,BIN FIXED(15));
DCL INITAVL ENTRY(PTR,BIN FIXED(15));
DCL (OPENLIB,CLOSLIB) ENTRY(BIN FIXED(15));
DCL (DEV,SON,BRTHR,FATHER,NINST,INST,CPTR,CINDEX,LIB,CHPN,SCHN,EASE,YIELD,CONF,
   AVLFN,CMERIT,SMERIT,CSMERT,SSMERT,NSONS,TIE,RXMERIT) ENTRY(BIN FIXED(15))
   RETURNS(BIN FIXED(15));
DCL CATN ENTRY(BIN FIXED(15)) RETURNS(CHAR(7) VAR);
DCL GETSTR ENTRY(BIN FIXED(15)) RETURNS(CHAR(500) VAR);
DCL NMSYM ENTRY RETURNS(BIN FIXED(15));
DCL (AVAIL,SLVD,CSTUCK,UNSOLV,SOLVED) ENTRY(BIN FIXED(15)) RETURNS(BIT(1));
DCL (DRAWSLI,DRAWSNP) ENTRY(CHAR(*) VAR,PTR,BIN FIXED(31),BIN FIXED(15),
   BIN FIXED(15),BIN FIXED(15));
dcl xgks entry;
dcl openws entry(char(*) var,bin fixed(31)) returns(bin fixed(31));
dcl (activws,closews) entry(bin fixed(31));
dcl actvall entry returns(bit(1));
dcl deacall entry;
DECLARE goldi ENTRY RETURNS(POINTER);
   DECLARE drawslc ENTRY(CHARACTER(*) VARYING) RETURNS(POINTER);
   DECLARE drwcacm ENTRY((4) FLOAT,POINTER,CHAR(*));
DCL NULL BUILTIN;
dcl textpstack ptr based(tsp);
dcl 1 cmolpstack(nmem) based(cmpsp),
	2 inx bin fixed(15),
	2 cmolp ptr;
dcl (textp,tsp,cmpsp,mole_p) ptr;
dcl text char(500) var based(textp);
dcl (nchar,nmem,memn) bin fixed(15);
DCL 1 mole BASED(mole_p),
   2 sym(100) CHAR(2) VAR,
   ((2 ix, 2 iy)(100), 2 ninx(100,6)) BIN FIXED(15),
   2 bmult(100,6) CHAR(1);
DCL PATHS_STR CHAR(100) VAR;
DCL PTR_VAL BIN FIXED(31);
DCL (LIBRARY,CHAPTER,SCHEMA,NODE,BNODE,INDX,NUM_INSTS,CHOICE,LEVEL,MINLEVEL,
   LEN,RANGE,SIZE1,SIZE2,CRTLIB,CRTAVL) BIN FIXED(15) INIT(0);
DCL TTYPE CHAR(80) VAR init(GETDVI());
DCL MOUSDRAW CHAR(50) VAR INIT('');
DCL (ALL,DONE) BIT(1) INIT('0'B);
dcl first_time bit(1) init('1'b);
DCL ANS CHAR(10) VAR;
DCL (COMPPTR,ROOTCOMP,C4PTR,C2PTR,TEMP,LAST,SLPTR,HEAD,TAIL,DRAWPTR,AVLPTR)
   PTR;
DCL DRAWING(SIZE1,SIZE2) CHAR(1) BASED(DRAWPTR);
DCL 1 STRINGLIST BASED(SLPTR),
   2 FIRST_SGMERIT BIN FIXED(15),
   2 BACK PTR,
   2 FORWARD PTR,
   2 STRLEN BIN FIXED(15),
   2 STRING CHAR(LEN);
DCL CIRCULAR_OR_LONG BIT(1) INIT('0'B);
DCL DUMMY CHAR(10) VAR;
DCL 1 COMPOUND BASED(COMPPTR),
   2 INDEXN BIN FIXED(15),
   2 COMP_MERIT BIN FIXED(15),
   2 NUMBER_OF_INSTANCES BIN FIXED(15),
   2 NODES(NUM_INSTS),
      3 MAP BIN FIXED(15),
      3 NNUM BIN FIXED(15),
      3 MERIT BIN FIXED(15),
      3 DAD,
	 4 SUBGOAL BIN FIXED(15),
	 4 DNUM BIN FIXED(15),
	 4 PTR PTR;
/* NUMSTRING is used in preference to excessive walking of linked lists in
   determining whether a given pathway will become circular upon the addition
   of another compound.  It is also used in a later step for bookkeeping during
   the process of organizing the pathways for display.  The basis for each of
   these operations is elaborated in the part of the program in which the
   operation is performed. */
DCL NUMSTRING CHAR(10000) VAR INIT('');
/* BINFIX and CHAR4 are different variable types addressing the same memory
   location.  BINFIX.CONST is assigned a value of -1 for the purpose of
   aligning CHAR4 into phase with 4-byte intervals along NUMSTRING when the
   INDEX function is used to search for a previous occurrence of CHAR4. */
DCL 1 BINFIX BASED(C4PTR),
   2 CONST BIN FIXED(15),
   2 VAR BIN FIXED(15);
DCL CHAR4 CHAR(4) BASED(C4PTR);
/* BFIX and CHAR2 are different variable types addressing the same memory
   location.  CHAR2 is used for the purpose of concatenation onto NUMSTRING
   for the purpose of defining a particular pathway. */
DCL (BFIX BIN FIXED(15),CHAR2 CHAR(2)) BASED(C2PTR);
DCL NUM_AVL_LIBS BIN FIXED(15);
DCL MAINENT BIT(1) INIT('0'B);
DCL SEND_LIST BIT(1) INIT('1'B);
DCL RECEIVE_LIST BIT(1) INIT('1'B);
RECEIVE_LIST='0'B;
PUTPTRC: ENTRY(PATHS_STR_PARM,CPPARM,SLHPARM,SLTPARM,DETAIL,sysprint_open);
IF RECEIVE_LIST THEN DO;
   if ^sysprint_open then ttype='?';
   ALLOCATE BINFIX;
   CONST=-1;
   ALLOCATE BFIX;
   PATHS_STR=PATHS_STR_PARM;
   ROOTCOMP=CPPARM;
   HEAD=SLHPARM;
   TAIL=SLTPARM;
   CALL WALK_LIST(DETAIL,PATHS_STR);
   FREE BINFIX;
   FREE BFIX;
   IF CRTAVL^=0 THEN CALL CLOSLIB(CRTAVL);
   IF CRTLIB^=0 THEN DO;
      CALL CXRLIB;
      CALL FREEREF;
   END;
   RETURN;
END;
MAINENT='1'B;
NUM_AVL_LIBS = NUMAVL;
CALL DROPEN;
/* prepare for LA-120 hardcopy */
CLOSE FILE(SYSPRINT);
OPEN FILE(SYSPRINT) OUTPUT LINESIZE(132);
CALL INITAVL(AVLPTR,NUM_AVL_LIBS);
/* read status file into memory */
CALL RDSTRO;
PTRCENT: ENTRY(NAVL);
IF ^MAINENT THEN NUM_AVL_LIBS=NAVL;
IF TTYPE='VAXStation II' THEN MOUSDRAW=',its SLING, or the word ''MOUSE''';
ELSE IF INDEX(TTYPE,'VT125')>0 THEN MOUSDRAW=
   ', its SLING, or the word ''DRAWING''';
ELSE MOUSDRAW=' or its SLING';
PUT SKIP EDIT('Do you want a listing of the available compounds for this run ',
   '(Y/default=N)? ')(A,A);
GET EDIT(ANS)(A(10));
ANS=SUBSTR(UPRCASE(ANS),1,1);
IF ANS='Y' THEN CALL LISTAVL(NUM_AVL_LIBS);
SEND_LIST='0'B;
GETPTRC: ENTRY(INDX_PARM,MINPLUS_PARM,CPPARM,SLHPARM,SLTPARM,SOLV_PARM);
ALLOCATE BINFIX;
CONST=-1;
ALLOCATE BFIX;
LEN=1;
/* begin with empty list */
ALLOCATE STRINGLIST SET(HEAD);
ALLOCATE STRINGLIST SET(TAIL);
HEAD->STRLEN=1;
TAIL->STRLEN=1;
DO WHILE(^DONE);
   IF SEND_LIST THEN INDX=INDX_PARM;
   ELSE CALL GET_NUM('Enter index number of precursor of interest '||
      '(0 to end)'||MOUSDRAW||': ',INDX,0,NMSYM(),1);
   DONE=INDX=0;
   IF ^DONE THEN DO;
      DONE=SEND_LIST;
/* initialize double-linking of list */
      HEAD->FORWARD=TAIL;
      HEAD->BACK=NULL;
      TAIL->BACK=HEAD;
      TAIL->FORWARD=NULL;
      LAST=HEAD;
      IF SEND_LIST THEN DO;
         ALL=^SOLV_PARM;
         RANGE=MINPLUS_PARM;
      END;
      ELSE DO;
         IF METABOL=1 THEN
            CALL GET_NUM('Choose (1) metabolized only or (2) all paths: ',
            CHOICE,1,2,-1);
         ELSE CALL GET_NUM('Choose (1) solved only or (2) all paths: ',
            CHOICE,1,2,-1);
         ALL=CHOICE=2;
         CALL GET_NUM('Range of path-length (minimal size to minimal+'||
            '0/1/2...): ',RANGE,0,-1,-1);
      END;
      MINLEVEL=10000;
      LEVEL=0;
      NUM_INSTS=NINST(INDX);
      ALLOCATE COMPOUND SET(ROOTCOMP);
      ROOTCOMP->NUMBER_OF_INSTANCES=NUM_INSTS;
      ROOTCOMP->INDEXN=INDX;
      CALL FILL_NODES(ROOTCOMP,CIRCULAR_OR_LONG);
      IF CIRCULAR_OR_LONG THEN DO;
	 PUT SKIP EDIT('All pathways using compound #',ROOTCOMP->INDEXN,
	    ' are circular')(A,F(4),A);
	 IF ^ALL THEN PUT EDIT(' (or none were solved)')(A);
	 PUT EDIT('.')(A);
      END;
      ELSE DO;
	 CALL SORT_NODES(ROOTCOMP);
/* At this point, the various instances of each compound have been sorted in
   order of decreasing subgoal merit, with all the solved merits preceding
   any unsolved merits. */
	 NUMSTRING='';
	 TEMP=ROOTCOMP;
	 BFIX=1;
	 DO WHILE(TEMP^=NULL);
	    NUMSTRING=NUMSTRING||CHAR2;
	    TEMP=TEMP->PTR(TEMP->MAP(1));
	 END;
	 CALL CHECK_PATH;
	 IF NUMSTRING='' THEN DO;
	    PUT SKIP EDIT('All pathways using compound #',ROOTCOMP->INDEXN,
	       ' are circular')(A,F(4),A);
	    IF ^ALL THEN PUT EDIT(' (or none were solved)')(A);
	    PUT EDIT('.')(A);
            CIRCULAR_OR_LONG='1'B;
	 END;
	 ELSE DO;
	    DO WHILE(NUMSTRING^='');
	       CALL NEXT_PATH;
	    END;
	    CALL SORT_PATHS;
/* At this point, the paths have been sorted into order of decreasing level 1
   subgoal merit (the merit of the reaction which directly produces the target
   compound). */
	    IF SEND_LIST THEN DO;
               IF CIRCULAR_OR_LONG THEN DO;
                  SLPTR=HEAD->FORWARD;
                  DO WHILE(SLPTR^=TAIL);
                     LEN=STRLEN;
                     SLPTR=FORWARD;
                     FREE BACK->STRINGLIST;
                  END;
                  LEN=1;
                  FREE HEAD->STRINGLIST;
                  FREE TAIL->STRINGLIST;
                  CALL FREE_ALL(ROOTCOMP);
                  CPPARM=NULL;
                  SLHPARM=NULL;
                  SLTPARM=NULL;
               END;
               ELSE DO;
                  CPPARM=ROOTCOMP;
                  SLHPARM=HEAD;
                  SLTPARM=TAIL;
               END;
               FREE BINFIX;
               FREE BFIX;
               RETURN;
            END;
            CALL PROMPT_FOR_OUTPUT;
	 END;
      END;
/* free all structures in preparation for another precursor */
      SLPTR=HEAD->FORWARD;
      DO WHILE(SLPTR^=TAIL);
	 LEN=STRLEN;
	 SLPTR=FORWARD;
	 FREE BACK->STRINGLIST;
      END;
      CALL FREE_ALL(ROOTCOMP);
      NUMSTRING='';
      CIRCULAR_OR_LONG='0'B;
   END;
END;
LEN=1;
FREE HEAD->STRINGLIST;
FREE TAIL->STRINGLIST;
FREE BINFIX;
FREE BFIX;
IF CRTAVL^=0 THEN CALL CLOSLIB(CRTAVL);
IF CRTLIB^=0 THEN DO;
   CALL CXRLIB;
   CALL FREEREF;
END;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
FILL_NODES: PROC(P,CIRCULAR) RECURSIVE;
DCL P PTR;
DCL CIRCULAR BIT(1);
DCL (I,INDX,SUBG) BIN FIXED(15);
DCL NEWCOMP PTR;
CIRCULAR='1'B;
/* reject if already too long */
IF LEVEL=MINLEVEL+RANGE THEN RETURN;
LEVEL=LEVEL+1;
INDX=P->INDEXN;
VAR=INDX;
/* Here the pathway is checked for circularity.  NUMSTRING contains a repre-
   sentation of each compound encountered previously along the main path.  If
   the representation of the current compound appears within NUMSTRING, the
   pathway is flagged as circular. */
IF LEVEL=1 THEN DO; /* Obtain merit stored for this precursor, as a starting
			 point for future computations */
   IF AVAIL(INDX) THEN P->COMP_MERIT=1100; /* Available merit always 100 */
   ELSE IF SLVD(INDX) THEN P->COMP_MERIT=CSMERT(INDX)+1000;
   ELSE P->COMP_MERIT=CMERIT(INDX);
END;
IF INDEX(NUMSTRING,CHAR4)>0 THEN DO;
   LEVEL=LEVEL-1;
   RETURN;
END;
NUMSTRING=NUMSTRING||CHAR4;
IF LENGTH(NUMSTRING)=10000 THEN SIGNAL ERROR;
/* for each satisfactory instance of the current compound, fill in that part of
   the data structure, point to a new allocation of that structure, and call
   FILL_NODES if the target compound has not yet been reached */
DO I=1 TO P->NUMBER_OF_INSTANCES;
   IF I=1 THEN P->NNUM(I)=CPTR(INDX);
   ELSE P->NNUM(I)=INST(P->NNUM(I-1));
   SUBG=FATHER(P->NNUM(I));
   P->SUBGOAL(I)=SUBG;
   P->MERIT(I)=-1;
   P->DNUM(I)=0;
   P->PTR(I)=NULL;
   IF ALL | SOLVED(SUBG) THEN DO;
      P->MERIT(I)=SUBGOAL_MERIT(SUBG,P);
      P->DNUM(I)=FATHER(SUBG);
      INDX=CINDEX(P->DNUM(I));
/* if pathway ends here, update MINLEVEL */
      IF INDX=1 THEN DO;
	 MINLEVEL=MIN(MINLEVEL,LEVEL);
	 IF MINLEVEL=10000 THEN SIGNAL ERROR;
      END;
      ELSE DO;
	 NUM_INSTS=NINST(INDX);
	 ALLOCATE COMPOUND SET(NEWCOMP);
	 NEWCOMP->NUMBER_OF_INSTANCES=NUM_INSTS;
	 NEWCOMP->INDEXN=INDX;
	 NEWCOMP->COMP_MERIT=P->MERIT(I);
	 CALL FILL_NODES(NEWCOMP,CIRCULAR);
	 IF CIRCULAR THEN DO;
	    FREE NEWCOMP->COMPOUND;
	    P->DNUM(I)=0;
	    P->MERIT(I)=-1;
	 END;
	 ELSE DO;
	    P->PTR(I)=NEWCOMP;
	    CALL SORT_NODES(NEWCOMP);
	 END;
      END;
   END;
END;
/* determine whether any non-circular pathways exist for this compound */
DO I=1 TO P->NUMBER_OF_INSTANCES WHILE(CIRCULAR);
   IF P->DNUM(I)>0 THEN CIRCULAR='0'B;
END;
/* backtrack and return */
NUMSTRING=SUBSTR(NUMSTRING,1,LENGTH(NUMSTRING)-4);
LEVEL=LEVEL-1;
END FILL_NODES;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
SORT_NODES: PROC(P);
DCL P PTR;
DCL (I,J,K) BIN FIXED(15);
DCL FOUND BIT(1);
/* using a bubble-sort, order the mapping so that increasing subscripts of
   MAP will reference nodes of decreasing subgoal merit */
DO I=1 TO P->NUMBER_OF_INSTANCES;
   FOUND='0'B;
   DO J=1 TO I-1 WHILE(^FOUND);
      FOUND=P->MERIT(I)>P->MERIT(P->MAP(J));
      IF FOUND THEN DO;
	 DO K=I TO J+1 BY -1;
	    P->MAP(K)=P->MAP(K-1);
	 END;
	 P->MAP(J)=I;
      END;
   END;
   IF ^FOUND THEN P->MAP(I)=I;
END;
END SORT_NODES;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
NEXT_PATH: PROC;
/* NUMSTRING contains a representation of a satisfactory pathway.  It is copied
   into the STRING substructure of STRINGLIST and linked into that list.  Then
   the tail of NUMSTRING is incremented and checked for validity. */
LEN=LENGTH(NUMSTRING);
ALLOCATE STRINGLIST;
STRLEN=LEN;
STRING=NUMSTRING;
FIRST_SGMERIT=FIND_MERIT(STRING);
BACK=LAST;
FORWARD=TAIL;
LAST->FORWARD=SLPTR;
LAST=SLPTR;
TAIL->BACK=SLPTR;
CHAR2=SUBSTR(NUMSTRING,LENGTH(NUMSTRING)-1);
BFIX=BFIX+1;
SUBSTR(NUMSTRING,LENGTH(NUMSTRING)-1)=CHAR2;
CALL CHECK_PATH;
END NEXT_PATH;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
CHECK_PATH: PROC RECURSIVE;
DCL TEMP PTR INIT(ROOTCOMP);
DCL NOGOOD BIT(1);
DCL I BIN FIXED(15);
DO I=1 TO LENGTH(NUMSTRING)-3 BY 2;
   CHAR2=SUBSTR(NUMSTRING,I,2);
   TEMP=TEMP->PTR(TEMP->MAP(BFIX));
END;
CHAR2=SUBSTR(NUMSTRING,LENGTH(NUMSTRING)-1);
/* reject if no such pathway */
NOGOOD=BFIX>TEMP->NUMBER_OF_INSTANCES;
/* or if pathway has been previously rejected */
IF ^NOGOOD THEN DO;
   NOGOOD=TEMP->DNUM(TEMP->MAP(BFIX))=0;
   IF NOGOOD THEN NUMSTRING=NUMSTRING||CHAR2;
END;
/* or if its length is not within range */
IF ^NOGOOD THEN DO;
   TEMP=TEMP->PTR(TEMP->MAP(BFIX));
   BFIX=1;
   DO WHILE(TEMP^=NULL);
      NUMSTRING=NUMSTRING||CHAR2;
      IF LENGTH(NUMSTRING)=10000 THEN SIGNAL ERROR;
      TEMP=TEMP->PTR(TEMP->MAP(1));
   END;
   NOGOOD=LENGTH(NUMSTRING)>2*(MINLEVEL+RANGE);
END;
/* or if contains previously-undetected circularity */
IF ^NOGOOD THEN CALL CIRCULARITY_CHECK(NOGOOD);
IF NOGOOD THEN DO;
/* backtrack and increment previous level, then check again */
   NUMSTRING=SUBSTR(NUMSTRING,1,LENGTH(NUMSTRING)-2);
   IF NUMSTRING='' THEN RETURN;
   CHAR2=SUBSTR(NUMSTRING,LENGTH(NUMSTRING)-1);
   BFIX=BFIX+1;
   SUBSTR(NUMSTRING,LENGTH(NUMSTRING)-1)=CHAR2;
   CALL CHECK_PATH;
END;
END CHECK_PATH;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
CIRCULARITY_CHECK: PROC(CIRCULAR);
DCL CIRCULAR BIT(1);
DCL NEWNUMSTRING CHAR(10000) VAR INIT('');
DCL (I,J,LEVEL,S) BIN FIXED(15) INIT(1);
DCL P PTR INIT(ROOTCOMP);
/* concatenate main-path index numbers into NEWNUMSTRING */
DO I=1 TO LENGTH(NUMSTRING)-1 BY 2;
   VAR=P->INDEXN;
   NEWNUMSTRING=NEWNUMSTRING||CHAR4;
   CHAR2=SUBSTR(NUMSTRING,I,2);
   P=P->PTR(P->MAP(BFIX));
END;
VAR=1;
NEWNUMSTRING=NEWNUMSTRING||CHAR4;
P=ROOTCOMP;
/* check all conjuncts for solution later in the main path */
DO I=1 TO LENGTH(NUMSTRING)-1 BY 2;
   CHAR2=SUBSTR(NUMSTRING,I,2);
   J=P->MAP(BFIX);
   S=SON(P->SUBGOAL(J));
   DO WHILE(S^=0);
      VAR=S;
      IF INDEX(NEWNUMSTRING,CHAR4)>LEVEL THEN DO;
	 CIRCULAR='1'B;
	 RETURN;
      END;
      S=BRTHR(S);
   END;
   LEVEL=LEVEL+4;
   P=P->PTR(J);
END;
END CIRCULARITY_CHECK;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
FIND_MERIT: PROC(S) RETURNS(BIN FIXED(15));
DCL S CHAR(*);
DCL TEMP PTR INIT(ROOTCOMP);
DCL I BIN FIXED(15);
DO I=1 TO LENGTH(S)-3 BY 2;
   CHAR2=SUBSTR(S,I,2);
   TEMP=TEMP->PTR(TEMP->MAP(BFIX));
END;
CHAR2=SUBSTR(S,LENGTH(S)-1);
RETURN(TEMP->MERIT(TEMP->MAP(BFIX)));
END FIND_MERIT;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
SORT_PATHS: PROC;
DCL CHANGED BIT(1) INIT('1'B);
DCL TEMP PTR;
/* bubble sort implemented by swapping links in the list */
DO WHILE(CHANGED);
   CHANGED='0'B;
   SLPTR=HEAD->FORWARD;
   DO WHILE(FORWARD^=TAIL);
      IF FIRST_SGMERIT<FORWARD->FIRST_SGMERIT THEN DO;
	 TEMP=FORWARD;
	 CALL SWAP(SLPTR,TEMP);
	 CHANGED='1'B;
      END;
      ELSE SLPTR=FORWARD;
   END;
END;
END SORT_PATHS;

SWAP: PROC(A,B);
DCL (A,B) PTR;
A->BACK->FORWARD=B;
B->FORWARD->BACK=A;
A->FORWARD=B->FORWARD;
B->BACK=A->BACK;
A->BACK=B;
B->FORWARD=A;
END SWAP;
#endif

void walk_list (Boolean_t detail, CHARVAR str)
{
  int i, j;
  Boolean_t all, ok, starred, forced, known, invented;

  i = j = 1;
  all = str.string[0] == 'A';
  ok = starred = forced = known = invented = FALSE;

  if (!all) j = getnum (str);
  slptr = head->forward;
  while (slptr != tail && j > 0)
  {
    if (detail)
    {
      ok = all;
      if (!ok)
      {
        ok = i == j;
        if (ok) j = getnum (str);
      }
      if (ok) display_path (i);
    }
    else /* print_path (i, &starred, &forced, &known, &invented) */;
    slptr = slptr->forward;
    i++;
  }
  putchar ('\n');
  if (starred) printf ("* denotes intervention of a protection or lookahead reaction\n");
  if (forced)
  {
    if (invented) printf ("? denotes \"invention\" of missing chemistry (forced subgoal)\n");
    if (known) printf ("! denotes a forced subgoal generated via chemistry known to SYNCHEM\n");
  }
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
WALK_LIST: PROC(DETAIL,STR);
DCL DETAIL BIT(1);
DCL STR CHAR(*) VAR;
DCL (I,J) BIN FIXED(15) INIT(1);
DCL ALL BIT(1) INIT(SUBSTR(STR,1,1)='A');
DCL (OK,STARRED,FORCED,KNOWN,INVENTED) BIT(1) INIT('0'B);
IF ^ALL THEN J=GETNUM(STR);
SLPTR=HEAD->FORWARD;
DO WHILE(SLPTR^=TAIL & J>0);
   IF DETAIL THEN DO;
      OK=ALL;
      IF ^OK THEN DO;
	 OK=I=J;
	 IF OK THEN J=GETNUM(STR);
      END;
      IF OK THEN CALL DISPLAY_PATH(I);
   END;
   ELSE CALL PRINT_PATH(I,STARRED,FORCED,KNOWN,INVENTED);
   SLPTR=FORWARD;
   I=I+1;
END;
PUT SKIP;
IF STARRED THEN PUT SKIP EDIT
   ('* denotes intervention of a protection or lookahead reaction')(A);
IF FORCED THEN DO;
   IF INVENTED THEN PUT SKIP EDIT
      ('? denotes "invention" of missing chemistry (forced subgoal)')(A);
   IF KNOWN THEN PUT SKIP EDIT
      ('! denotes a forced subgoal generated via chemistry known to SYNCHEM')
      (A);
END;
END WALK_LIST;
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
void print_path (int num, Boolean_t *star, Boolean_t *fpath, Boolean_t *kn, Boolean_t *inv)
{
  int i, j, m, len;
  COMPOUND *temp;
  char c[8], ccpy[8];

  printf ("%d) ", num);
  len = slptr->str_len;
  for (i = 0; i < len; i += 2)
  {
/*
   IF METABOL=1 THEN C=') <-';
   ELSE C=') ->';
*/
    strpcy (c, ") ->");
    char2.str2[0] = slptr->string[i];
    char2.str2[1] = slptr->string[i + 1];
    j = temp->nodes[char2.bfix].map;
    printf ("%4d (", SymTab_Index_Get (temp->indexn));
    if (note (temp->nodes[j].subgoal) != 0)
    {
      printf ("* ");
      *star = TRUE;
    }
    if (frcnode (temp->nodes[j].dad.subgoal)
    {
      if (chpn (temp->nodes[j].dad.subgoal) == 0)
      {
        printf ("? ");
        *inv = TRUE;
      }
      else
      {
        printf ("! ");
        *kn = TRUE;
      }
      *fpath = TRUE;
    }
    printf ("%5d", -temp->nodes[j].dad.subgoal);
    m = temp->nodes[j].merit;
    if (m > 999)
    {
      m -= 1000;
      strcpy (ccpy, c);
      sprintf (c, "(S)%s", ccpy);
    }
    printf (":%3d%s", m, c);
    temp = temp->nodes[j].dad.ptr;
  }
  printf ("   1\n");
}
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
PRINT_PATH: PROC(NUM,STAR,FPATH,KN,INV);
DCL NUM BIN FIXED(15);
DCL (STAR,FPATH,KN,INV) BIT(1);
DCL (I,J) BIN FIXED(15);
DCL TEMP PTR INIT(ROOTCOMP);
DCL M BIN FIXED(15);
DCL C CHAR(7) VAR;
PUT SKIP(3) EDIT(NUM,') ')(A,A);
LEN=STRLEN;
DO I=1 TO LEN-1 BY 2;
   IF METABOL=1 THEN C=') <-';
   ELSE C=') ->';
   CHAR2=SUBSTR(STRING,I,2);
   J=TEMP->MAP(BFIX);
   PUT EDIT(TEMP->INDEXN,' (')(F(4),A);
   IF NOTE(TEMP->SUBGOAL(J))^=0 THEN DO;
      PUT EDIT('* ')(A);
      STAR='1'B;
   END;
   IF FRCNODE(TEMP->SUBGOAL(J)) THEN DO;
      IF CHPN(TEMP->SUBGOAL(J))=0 THEN DO;
         PUT EDIT('? ')(A);
         INV='1'B;
      END;
      ELSE DO;
         PUT EDIT('! ')(A);
         KN='1'B;
      END;
      FPATH='1'B;
   END;
   PUT EDIT(-TEMP->SUBGOAL(J))(F(5));
   M=TEMP->MERIT(J);
   IF M>999 THEN DO;
      M=M-1000;
      C='(S)'||C;
   END;
   PUT EDIT(':',M,C)(A,F(3),A);
   TEMP=TEMP->PTR(J);
END;
PUT EDIT(1)(F(4));
END PRINT_PATH;
#endif

void display_path (int num)
{
  int i, j, tc, m, len, nsteps;
  float ea, oy, ty;
  COMPOUND *temp;
  char c[16];

  ea = 0.;
  nsteps = 0;
  oy = ty = tc = 100.;
  temp = rootcomp;
/*
  len = slptr->str_len;

for (i = 0; i < len; i += 2)
{
    char2.str2[0] = slptr->string[i];
    char2.str2[1] = slptr->string[i + 1];
}
  for (i = j = 0; i < len; i += 2)
*/
  for (i = j = 0; temp != NULL; i += 2)
  {
    c[0] = 0;
/*
    char2.str2[0] = slptr->string[i];
    char2.str2[1] = slptr->string[i + 1];
    j = temp->nodes[char2.bfix].map; 
*/
    m = temp->nodes[j].merit; 
    if (m > 999)
    {
      m -= 1000;
      strcpy (c, " (SOLVED)");
    }
    draw_compounds (&temp->drawing, temp->nodes[j].nnum, temp->nodes[j].dad.subgoal, m, c, &ea, &oy, &ty, &tc, &nsteps, i == 0);
    temp = (COMPOUND *) temp->nodes[j].dad.ptr;
  }
  draw_compounds (&target_drawing, /* 1, 0, */ TARGET, NULL, 0, "", &ea, &oy, &ty, &tc, &nsteps, FALSE);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
DISPLAY_PATH: PROC(NUM);
DCL NUM BIN FIXED(15);
DCL (I,J) BIN FIXED(15);
DCL EA BIN FIXED(15) INIT(0);
DCL (OY,TY,TC) BIN FIXED(15) INIT(100);
DCL TEMP PTR INIT(ROOTCOMP);
DCL M BIN FIXED(15);
DCL C CHAR(9) VAR;
if ttype^='VAXStation II' then PUT PAGE EDIT('Pathway ',NUM,':')(A,F(3),A);
LEN=STRLEN;
DO I=1 TO LEN-1 BY 2;
   C='';
   CHAR2=SUBSTR(STRING,I,2);
   J=TEMP->MAP(BFIX);
   M=TEMP->MERIT(J);
   IF M>999 THEN DO;
      M=M-1000;
      C=' (SOLVED)';
   END;
   CALL DRAW_COMPOUNDS(TEMP->NNUM(J),TEMP->SUBGOAL(J),M,C,EA,OY,TY,TC,I=1);
   TEMP=TEMP->PTR(J);
END;
CALL DRAW_COMPOUNDS(1,0,0,'',EA,OY,TY,TC,'0'B);
if ttype='VAXStation II' then call dump_stacks;
END DISPLAY_PATH;
#endif

int dump_stacks (int page_increment, Boolean_t win_active,
  Display *d, Window w, Pixmap p, Pixmap sp, GC gcp)
{
static  int i, prev_i, j, k, l, m, minx, miny, maxx, maxy, xdiff, ydiff, mindim[100][5], maxdim[100][5], nt,
    ncmps, ncmps2, ncmpsets, ncmpsets2, last_on_line1, last_on_line2, cmps_on_line1, cmps_on_line2, cmps_on_line3,
    npages, numlines, last_on_page[101], semicolon_count;
static  float textheight, rxnbottom, rxnright, rightmax, top, bottom;
  CMPSTKST *stackp;
  Dsp_Molecule_t *mole_p;
  char *textp, sub_syn_str[2][128], msg_str[512], *comma, plural[4];
Colormap cmap;
XColor best, exact;
  static Boolean_t comments = TRUE;
static int which_page = 0;
static int semicolons[100];
static int nsemicolons = 0;
static Boolean_t page_shown[100];

write_win = win_active;
which_page += page_increment;
if (first_dump_stacks)
{
which_page=page_increment;
nsemicolons=0;
first_dump_stacks = FALSE;
  disp = d;
  win = w;
  pmap = p;
  scratchpad = sp;
  gc = gcp;
/*
cmap=DefaultColormap (disp, DefaultScreen (disp));
XAllocNamedColor (disp, cmap, "red2", &best, &exact);
rgb[0]=best.pixel;
XAllocNamedColor (disp, cmap, "green4", &best, &exact);
rgb[1]=best.pixel;
XAllocNamedColor (disp, cmap, "blue3", &best, &exact);
rgb[2]=best.pixel;
*/
rgb[0]=SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_RED);
rgb[1]=SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_SOL_VD);
rgb[2]=SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VD);

  for (i = 0; i < 100; i++) page_shown[i] = FALSE;

  textheight = rxnright = 0.;
  rxnbottom = /* 0.75 */ /* 0.67 */ 0.7;
  ncmps = ncmps2 = ncmpsets = ncmpsets2 = last_on_line1 = last_on_line2 = cmps_on_line1 = cmps_on_line2 = cmps_on_line3 =
    npages = numlines = 1;

  last_on_page[0] = -1;

  for (i = 0; i < ncmol; i++)
  {
    nt = i;
    if (allocn("TEXT", nt) == 0) semicolons[nsemicolons++] = i;
    else for (j=0; j <= allocn("TEXT", nt); j++)
    {
      if (j == 0) textheight += .015;
      else
      {
        textp = (char *) memptr ("TEXT", nt, j - 1);
        if (*textp == '0' || comments) textheight += .015;
      }
    }
    stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i);
    mole_p = stackp[0].cmolp;
    minx = miny = 0;
    maxx = mole_p->molw;
    maxy = mole_p->molh;
    xdiff = maxx - minx;
    ydiff = maxy - miny;
    mindim[i][0] = fixmax (10, fixmin (xdiff, ydiff));
    maxdim[i][0] = fixmax (xdiff, ydiff);
    rxnright += (float) maxdim[i][0] / 3. / (float) mindim[i][0];
    if (nconj[i] == 1) rxnright += .05;
    else
    {
      rightmax = 0.;
      for (j = 1; j < nconj[i]; j++)
      {
        mole_p = stackp[j].cmolp;
        minx = miny = 0;
        maxx = mole_p->molw;
        maxy = mole_p->molh;
        xdiff = maxx - minx;
        ydiff = maxy - miny;
        mindim[i][j] = fixmax (10, fixmin (xdiff, ydiff));
        maxdim[i][j] = fixmax (xdiff, ydiff);
        rightmax = fpmax (rightmax, (float) maxdim[i][j] / 10. / (float) mindim[i][j]);
      }
      rxnright += rightmax;
    }
    if (textheight > rxnbottom && i < ncmol - 2)
    {
      last_on_page[npages] = i;
      npages++;
      textheight = rxnright = 0.;
      rxnbottom = /* 0.75 */ /* 0.67 */ 0.7;
    }
    if (rxnright > 1.)
    {
      rxnright = 0.;
      rxnbottom -= /* 0.25 */ /* 0.33 */ 0.3;
    }
  }
  last_on_page[npages] = ncmol - 1;
}
  i = which_page;
if (i==npages) sum_avl ();
else if (i < npages)
{
  sub_syn_str[0][0] = sub_syn_str[1][0] = '\0';
    clear_window ();
    textheight = 0.;
    for (l = last_on_page[i + 1]; l > last_on_page[i]; l--)
    {
      nt = l;
      k = allocn ("TEXT", nt);
      if (k != 0) for (j = semicolon_count = 0; j < nsemicolons && semicolons[j] < l; j++) semicolon_count++;
      for (j = k - 1; j >= 0; j--)
      {
        textp = memptr ("TEXT", nt, j);
        if (comments || *textp == '0') gkstext (nt - semicolon_count + 1, j == 0, l == ncmol - 1, textp + 1, &textheight);
      }
    }
    textheight += .015;
    ncmps = ncmpsets = 0;
    for (l = last_on_page[i] + 1; l <= last_on_page[i + 1]; l++)
    {
      ncmpsets++;
      stackp = memptr ("CMOLPSTACK", 0, l);
      if (nconj[l] == 1) ncmps += 2;
      else ncmps += 3;
    }
    if (textheight < 0.325 && ncmps > 15) numlines = 3;
    else if (textheight < 0.65 && ncmps > 8) numlines = 2;
    else numlines = 1;
    last_on_line1 = last_on_line2 = last_on_page[i + 1];
    cmps_on_line1 = cmps_on_line2 = cmps_on_line3 = ncmpsets;
    ncmps2 = ncmpsets2 = 0;
    if (numlines == 2) for (l = last_on_page[i] + 1; l <= last_on_page[i + 1]; l++)
    {
      ncmpsets2++;
      stackp = memptr ("CMOLPSTACK", 0, l);
      if (nconj[l] == 1) ncmps2 += 2;
      else ncmps2 += 3;
      if ((ncmps + 1) / (ncmps2 + 1) == 1 && last_on_line1 == last_on_page[i + 1])
      {
        last_on_line1 = l;
        cmps_on_line2 = cmps_on_line1;
        cmps_on_line1 = ncmpsets2;
        cmps_on_line2 -= cmps_on_line1;
        if (cmps_on_line2 == 1)
        {
          cmps_on_line1--;
          last_on_line1 = l - 1;
          cmps_on_line2++;
        }
      }
    }
    else if (numlines == 3) for (l = last_on_page[i] + 1; l <= last_on_page[i + 1]; l++)
    {
      ncmpsets2++;
      stackp = memptr ("CMOLPSTACK", 0, l);
      if (nconj[l] == 1) ncmps2 += 2;
      else ncmps2 += 3;
      if (last_on_line1 == last_on_page[i + 1])
        if ((ncmps + 1) / (ncmps2 + 1) == 2)
      {
        last_on_line1 = l;
        cmps_on_line1 = ncmpsets2;
      }
      else;
      else if (last_on_line2 == last_on_page[i + 1])
        if ((2 * ncmps + 1) / (ncmps2 + 1) == 2)
      {
        last_on_line2 = l;
        cmps_on_line2 = ncmpsets2 - cmps_on_line1;
      }
      if (l = last_on_page[i + 1])
      {
        cmps_on_line3 = ncmpsets2 - cmps_on_line1 - cmps_on_line2;
        if (cmps_on_line3 == 1)
        {
          cmps_on_line2--;
          last_on_line2--;
          cmps_on_line3++;
        }
        if (cmps_on_line2 < 2)
        {
          cmps_on_line1 += cmps_on_line2 - 2;
          last_on_line1 += cmps_on_line2 - 2;
          cmps_on_line2 = 2;
        }
      }
    }
    ncmps = 0;
if (numlines == 0)
{
printf("Error in dump_stacks\n");
exit(1);
}
    for (l = last_on_page[i] + 1; l <= last_on_line1; l++)
    {
      ncmps++;
      top = 0.95;
      bottom = 0.95 - (0.95 - textheight) / (float) numlines;
      stackp = memptr ("CMOLPSTACK", 0, l);
      if (cmps_on_line1 > 0)
      {
        for (j = semicolon_count = 0; j < nsemicolons && semicolons[j] < l; j++) semicolon_count++;
        draw_line (stackp, top, bottom, cmps_on_line1, l, l - semicolon_count, last_on_line1, nconj[l], l != ncmol - 1,
          sub_syn_str[0], TRUE);
      }
    }
    for (; l <= last_on_line2; l++)
    {
      top = 0.95 - (0.95 - textheight) / (float) numlines;
      bottom = 0.95 - 2. * (0.95 - textheight) / (float) numlines;
      stackp = memptr ("CMOLPSTACK", 0, l);
      if (cmps_on_line2 > 0)
      {
        for (j = semicolon_count = 0; j < nsemicolons && semicolons[j] < l; j++) semicolon_count++;
        draw_line (stackp, top, bottom, cmps_on_line2, l, l - semicolon_count, last_on_line2, nconj[l], l != ncmol - 1,
          sub_syn_str[0], FALSE);
      }
    }
    for (; l <= last_on_page[i + 1]; l++)
    {
      top = 0.95 - 2. * (0.95 - textheight) / (float) numlines;
      bottom = textheight;
      stackp = memptr ("CMOLPSTACK", 0, l);
      if (cmps_on_line3 > 0)
      {
        for (j = semicolon_count = 0; j < nsemicolons && semicolons[j] < l; j++) semicolon_count++;
        draw_line (stackp, top, bottom, cmps_on_line3, l, l - semicolon_count, last_on_page[i + 1], nconj[l], l != ncmol - 1,
          sub_syn_str[0], FALSE);
      }
    }
strcpy(sub_syn_str[1],sub_syn_str[0]);
  if (sub_syn_str[0][0] != '\0' && !page_shown[which_page])
    {
    comma = strstr (sub_syn_str[0], ",");
    if (comma == NULL) plural[0] = '\0';
    else
      {
      strcpy (plural, "s");
      for (i = strlen (sub_syn_str[0]) - 1; sub_syn_str[0][i] != ','; i--);
      sub_syn_str[0][i++] = '\0';
      comma = sub_syn_str[0] + i;
      }
    sprintf (msg_str, "PathTrc Explanation~Compound%s %s", plural, sub_syn_str[0]);
    if (comma != NULL) sprintf (msg_str + strlen (msg_str), " and%s appear as co-reactants in", comma);
    else strcat (msg_str, " appears as a co-reactant in a");
    sprintf (msg_str + strlen (msg_str), " subsequent reaction sequence%s.\n\n", plural);

while(sub_syn_str[1][0]!='\0')
{
  comma=strstr(sub_syn_str[1],",");
  if (comma==NULL) comma=sub_syn_str[1]+strlen(sub_syn_str[1]);
  else *comma++='\0';
  sscanf(sub_syn_str[1],"%d",&k);
 for (i=npages-1; i>=which_page; i--) for (l=last_on_page[i+1]; l>last_on_page[i]; l--)
 {
  stackp=memptr("CMOLPSTACK",0,l);
  for (m=1; m<nconj[l]; m++) if (SymTab_Index_Get(stackp[m].inx)==k)
  {
    for (j = semicolon_count = 0; j < nsemicolons && semicolons[j] < l; j++) semicolon_count++;
    sprintf(msg_str+strlen(msg_str),"    Compound %6d appears in step %2d\n",k,l-semicolon_count+1);
  }
 }
  strcpy(sub_syn_str[1],comma);
}

    InfoMess_Show (msg_str);
    }

 if (!page_shown[0]) InfoWarn_Show (NULL); /* try to get Motif to cooperate in window restacking! */
  page_shown[which_page] = TRUE;
}

  if (which_page <= npages) return (npages + 1);

/* else clean up and exit entire app */
  while ((i = allocn ("CMOLPSTACK", 0)) != 0)
  {
    stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i - 1);
    for (j = 0; j < nconj[i - 1]; j++)
      if (stackp[j].cmolp != NULL) free_Molecule(stackp[j].cmolp);
    afree ("CMOLPSTACK", 0);
  }
  while ((i = allocn ("TEXTPSTACK", 0)) != 0)
  {
    while (allocn ("TEXT", i - 1) != 0) afree ("TEXT", i - 1);
    afree ("TEXTPSTACK", 0);
  }

/*
  for (i = ncmol - 1; i >= 0; i--)
  {
    nt = i;
    k = allocn ("TEXT", nt);
    for (j = 0; j < k; j++) afree ("TEXT", nt);
    afree ("TEXTPSTACK", 0);
    k = nconj[i];
    for (j = 0; j < k; j++)
    {
      stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i);
      if (stackp[j].cmolp != NULL) free_Molecule(stackp[j].cmolp);
    }
    afree ("CMOLPSTACK", 0);
  }
*/

if (allocn ("CMOLPSTACK", 0) != 0)
{
  printf("error freeing CMOLPSTACK\n");
  exit (1);
}
if (allocn ("TEXTPSTACK", 0) != 0)
{
  printf("error freeing TEXTPSTACK\n");
  exit (1);
}
free_all (rootcomp);
rootcomp = NULL;
if (pathtrc_standalone) exit(0);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
dump_stacks: proc;
dcl (i,prev_i,j,k,l,minx,miny,maxx,maxy,xdiff,ydiff,(mindim,maxdim)(100,5))
	bin fixed(15);
dcl ncmol bin fixed(15) init(allocn('CMOLPSTACK','PATHTRC'));
dcl ntp bin fixed(15) init(allocn('TEXTPSTACK','PATHTRC'));
dcl nt pic'99';
dcl nconj(100) bin fixed(15);
dcl done bit(1);
dcl textheight float init(0.);
dcl rxnbottom float init(3./4.);
dcl rxnright float init(0.);
dcl rightmax float;
dcl (ncmps,ncmps2,ncmpsets,ncmpsets2,last_on_line1,last_on_line2,cmps_on_line1,
	cmps_on_line2,cmps_on_line3,npages,numlines) bin fixed(15) init(1);
dcl (top,bottom) float;
dcl last_on_page(0:100) bin fixed(15);
dcl stackp ptr;
DCL (xMAX,yMAX) FLOAT;
DCL (id,RX,RY,ERRIND) BIN FIXED(31);
dcl earea(4) float;
dcl str char(80);
dcl xfm_a(1) bin fixed(31);
dcl (xfm,ldr,len) bin fixed(31);
dcl (loc_x,loc_y) float;
dcl cmenu(2) char(50) var init('include comments with references',
	'no comments -- literature references only');
dcl comments bit(1) static;
id=openws('PATHTRC Path Window',GKS$K_WSTYPE_DEFAULT);
CALL GKS$INQ_MAX_DS_SIZE(GKS$K_WSTYPE_DEFAULT,ERRIND,GKS$K_METERS,xMAX,yMAX,
	RX,RY);
CALL GKS$SET_WINDOW(1,0.,1.,0.,1.);
CALL GKS$SET_VIEWPORT(1,0.,1.,0.,.825);
CALL GKS$SET_WS_WINDOW(id,0.,1.,0.,.825);
CALL GKS$SET_WS_VIEWPORT(id,0.,xMAX,0.,yMAX);
call deacall;
CALL ACTIVWS(id);
call gks$inq_xform_list(errind,1,xfm_a,1);
CALL GKS$SELECT_XFORM(1);
if xfm_a(1)^=1 then call gks$set_viewport_priority(1,xfm_a(1),0);
CALL GKS$INQ_LOCATOR_STATE(ID,1,1,errind,0,1,xfm,loc_x,loc_y,3,earea,
	str,ldr,len);
earea = 0.0;
earea(2) = xmax;
earea(4) = ymax;
CALL GKS$INIT_LOCATOR(ID,1,.5,.5,1,1,earea,str,len);
CALL GKS$SET_LOCATOR_MODE(ID,1,0,1);
if first_time then do;
	comments=vs2menw(id,cmenu,1)=1;
	first_time='0'b;
end;
last_on_page(0)=0;
do i=1 to ncmol;
	nt=i;
	do j=0 to allocn('TEXT'||nt,'PATHTRC');
		if j=0 then textheight=textheight+.015;
		else do;
			textp=memptr('TEXT'||nt,'PATHTRC',j,0,0);
			if ^substr(text,1,1) | comments then textheight=
				textheight+.015;
		end;
	end;
	stackp=memptr('CMOLPSTACK','PATHTRC',i,nconj(i),0);
	mole_p=stackp->cmolp(1);
	done='0'b;
	minx=1000;
	maxx=0;
	miny=1000;
	maxy=0;
	do k=1 to 100 while(^done);
		done=sym(k)='--';
		if ^done then do;
			minx=min(minx,ix(k));
			maxx=max(maxx,ix(k));
			miny=min(miny,iy(k));
			maxy=max(maxy,iy(k));
		end;
	end;
	xdiff=maxx-minx;
	ydiff=maxy-miny;
	mindim(i,1)=max(10,min(xdiff,ydiff));
	maxdim(i,1)=max(xdiff,ydiff);
	rxnright=rxnright+float(maxdim(i,1),24)/3./float(mindim(i,1),24);
	if nconj(i)=1 then rxnright=rxnright+.05;
	else do;
		rightmax=0.;
		do j=2 to nconj(i);
			mole_p=stackp->cmolp(j);
			done='0'b;
			minx=1000;
			maxx=0;
			miny=1000;
			maxy=0;
			do k=1 to 100 while(^done);
				done=sym(k)='--';
				if ^done then do;
					minx=min(minx,ix(k));
					maxx=max(maxx,ix(k));
					miny=min(miny,iy(k));
					maxy=max(maxy,iy(k));
				end;
			end;
			xdiff=maxx-minx;
			ydiff=maxy-miny;
			mindim(i,j)=max(10,min(xdiff,ydiff));
			maxdim(i,j)=max(xdiff,ydiff);
			rightmax=max(rightmax,
				float(maxdim(i,j),24)/10./float
				(mindim(i,j),24));
		end;
		rxnright=rxnright+rightmax;
	end;
	if textheight>rxnbottom & i<ncmol-1 then do;
		last_on_page(npages)=i;
		npages=npages+1;
		textheight=0.;
		rxnbottom=3./4.;
		rxnright=0.;
	end;
	if rxnright>1. then do;
		rxnright=0.;
		rxnbottom=rxnbottom-1./4.;
	end;
end;
last_on_page(npages)=ncmol;
i=1;
do while(i<=npages);
	prev_i=i;
	call gks$clear_ws(id,0);
	call putmenu(i,npages);
	textheight=0;
	do l=last_on_page(i) to last_on_page(i-1)+1 by -1;
		nt=l;
		k=allocn('TEXT'||nt,'PATHTRC');
		do j=k to 1 by -1;
			textp=memptr('TEXT'||nt,'PATHTRC',j,0,0);
			if comments | ^substr(text,1,1) then call
				gkstext(nt,j=1,l=ncmol,substr(text,2),
				textheight);
		end;
	end;
	textheight=textheight+.015;
	ncmps=0;
	ncmpsets=0;
	do l=last_on_page(i-1)+1 to last_on_page(i);
		ncmpsets=ncmpsets+1;
		stackp=memptr('CMOLPSTACK','PATHTRC',l,nconj(l),0);
		do j=1 to nconj(l);
			ncmps=ncmps+2-(j>1)-(j>2);
		end;
	end;
	numlines=1;
	if textheight<.65 & ncmps>8 then numlines=2;
	if textheight<.325 & ncmps>15 then numlines=3;
	last_on_line1=last_on_page(i);
	last_on_line2=last_on_page(i);
	cmps_on_line1=ncmpsets;
	cmps_on_line2=ncmpsets;
	cmps_on_line3=ncmpsets;
	ncmps2=0;
	ncmpsets2=0;
	if numlines=2 then do l=last_on_page(i-1)+1 to last_on_page(i);
		ncmpsets2=ncmpsets2+1;
		stackp=memptr('CMOLPSTACK','PATHTRC',l,nconj(l),0);
		do j=1 to nconj(l);
			ncmps2=ncmps2+2-(j>1)-(j>2);
		end;
		if divide(ncmps+1,ncmps2+1,15)=1 & last_on_line1=last_on_page(i)
			then do;
			last_on_line1=l;
			cmps_on_line2=cmps_on_line1;
			cmps_on_line1=ncmpsets2;
			cmps_on_line2=cmps_on_line2-cmps_on_line1;
			if cmps_on_line2=1 then do;
				cmps_on_line1=cmps_on_line1-1;
				last_on_line1=l-1;
				cmps_on_line2=2;
			end;
		end;
	end;
	else if numlines=3 then do l=last_on_page(i-1)+1 to last_on_page(i);
		ncmpsets2=ncmpsets2+1;
		stackp=memptr('CMOLPSTACK','PATHTRC',l,nconj(l),0);
		do j=1 to nconj(l);
			ncmps2=ncmps2+2-(j>1)-(j>2);
		end;
		if last_on_line1=last_on_page(i) then
			if divide(ncmps+1,ncmps2+1,15)=2 then do;
			last_on_line1=l;
			cmps_on_line1=ncmpsets2;
		end;
		else;
		else if last_on_line2=last_on_page(i) then
			if divide(2*ncmps+1,ncmps2+1,15)=2 then do;
			last_on_line2=l;
			cmps_on_line2=ncmpsets2-cmps_on_line1;
		end;
		if l=last_on_page(i) then do;
			cmps_on_line3=ncmpsets2-cmps_on_line1-cmps_on_line2;
			if cmps_on_line3=1 then do;
				cmps_on_line2=cmps_on_line2-1;
				last_on_line2=last_on_line2-1;
				cmps_on_line3=2;
			end;
			if cmps_on_line2<2 then do;
				cmps_on_line1=cmps_on_line1+cmps_on_line2-2;
				last_on_line1=last_on_line1+cmps_on_line2-2;
				cmps_on_line2=2;
			end;
		end;
	end;
	ncmps=0;
	do l=last_on_page(i-1)+1 to last_on_line1;
		ncmps=ncmps+1;
		top=.95;
		bottom=.95-(.95-textheight)/float(numlines,24);
		stackp=memptr('CMOLPSTACK','PATHTRC',l,nconj(l),0);
		if cmps_on_line1>0 then
			call draw_line(stackp,top,bottom,cmps_on_line1,l,
			last_on_line1,nconj(l),l^=ncmol);
	end;
	do l=last_on_line1+1 to last_on_line2;
		top=.95-(.95-textheight)/float(numlines,24);
		bottom=.95-2.*(.95-textheight)/float(numlines,24);
		stackp=memptr('CMOLPSTACK','PATHTRC',l,nconj(l),0);
		if cmps_on_line2>0 then
			call draw_line(stackp,top,bottom,cmps_on_line2,l,
			last_on_line2,nconj(l),l^=ncmol);
	end;
	do l=last_on_line2+1 to last_on_page(i);
		top=.95-2.*(.95-textheight)/float(numlines,24);
		bottom=textheight;
		stackp=memptr('CMOLPSTACK','PATHTRC',l,nconj(l),0);
		if cmps_on_line3>0 then
			call draw_line(stackp,top,bottom,cmps_on_line3,l,
			last_on_page(i),nconj(l),l^=ncmol);
	end;
	do while(i=prev_i);
		call getmenu(i,npages);
	end;
end;
do i=ncmol to 1 by -1;
	nt=i;
	k=allocn('TEXT'||nt,'PATHTRC');
	do j=1 to k;
		call free('TEXT'||nt,'PATHTRC',textpstack);
	end;
	call free('TEXTPSTACK','PATHTRC',tsp);
	k=arrsiz('CMOLPSTACK','PATHTRC');
	do j=1 to k;
		free cmolp(j)->mole;
	end;
	call free('CMOLPSTACK','PATHTRC',cmpsp);
end;
call closews(id);
if ^actvall() then call gks$close_gks;
if xfm_a(1)^=1 then call gks$set_viewport_priority(1,xfm_a(1),1);
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
getmenu: proc(page,npages);
dcl (page,npages) bin fixed(15);
dcl print_bounds(4) float init(0.,.0975,0.,.045);
dcl done_bounds(4) float init(0.,.0975,.045,.09);
dcl next_bounds(4) float init(0.,.0975,.09,.135);
dcl prev_bounds(4) float init(0.,.0975,.135,.18);
dcl menux(4) float init(-.0025,.1,.1,-.0025);
dcl menuy(4) float init(-.0025,-.0025,.1825,.1825);
dcl (x,y) float;
dcl done_text(1) char(5) var init(' DONE');
dcl text2(2) char(9) var;
dcl stat bin fixed(31);
dcl bmenu(4) char(25) var init('portrait with border','portrait -- NO border',
	'landscape with border','landscape -- NO border');
dcl choice bin fixed(15);
CALL GKS$INIT_LOCATOR(ID,1,.5,.5,1,1,earea,str,len);
CALL GKS$REQUEST_LOCATOR(ID,1,stat,1,x,y);
if x>.085 | y>.18 then do;
	put skip(0) edit('')(a);
	return;
end;
if y>.045 then do;
	if y>.135 then
		if page>1 then page=page-1;
		else put skip(0) edit('')(a);
	else if y>.09 then
		if page<npages then page=page+1;
		else put skip(0) edit('')(a);
	else page=npages+1;
	return;
end;
CALL GKS$Set_Fill_Int_Style(GKS$K_IntStyle_Solid);
CALL GKS$Set_Fill_Color_Index(0);
CALL GKS$Fill_Area(4,menuX,menuY);
choice=vs2menw(id,bmenu,4);
call prtscrn(mod(choice,2)=1,choice>2);
#endif

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
putmenu: entry(page,npages);
call menubar(done_bounds,done_text,'1'b);
text2(1)=' PREVIOUS';
text2(2)=' PAGE';
if page^=1 then call menubar(prev_bounds,text2,'1'b);
text2(1)=' NEXT';
if page^=npages then call menubar(next_bounds,text2,'1'b);
text2(1)=' PRINT';
call menubar(print_bounds,text2,'1'b);
call gks$set_pline_linewidth(1);
call gks$set_text_fontprec(1,0);
end;

end;
#endif

void draw_compounds (Dsp_Molecule_t **drawing, /* int */ Compound_t *node, /* int */ Subgoal_t *subg, int merit, char *string,
  float *ea, float *oy, float *ty, int *tc, int *nsteps, Boolean_t start)
{
  React_Record_t *schrec;
  React_TextRec_t *txt;
  React_TextHead_t *txt_head;
  SymTab_t *symtab, *bsymtab;
  Compound_t *brother;
  char x1[512], x2[512], x3[512],libchar[2], reftype[4], refdate[7], reftext[512], estring[101], ystring[101], cstring[101], *textp,
    ttxt[512], *nextrt;
  int i, j, k, index, bindex, /* brother, */ library, chapter, schema, subgnote, lcs_note, ch_sch, tspn, nmem, memn, pschn,
    nref, ncomm;
  Boolean_t prot_bit, ignore, first_ref, comment, cont;
  CMPSTKST *stackp;
int start_princ;

  prot_bit = ignore = first_ref = TRUE;
  comment = FALSE;

/* draws compound; if not target compound, draws arrow to next compound,
   alongside which appear any conjuncts and information about the reaction */

  symtab = PstComp_SymbolTable_Get (node);
  index = SymTab_Index_Get (symtab);
  if (subg == NULL) nmem = 1;
  else nmem = nssons (cfather (node));
  alloct ("CMOLPSTACK", 0, nmem, 0); /* 2 bytes for integer; 4 bytes for pointer */
  memn = 0;
  cmolpstack[ncmol - 1][memn].inx = /* index */ symtab;
/*
  if (*drawing == NULL) cmolpstack[ncmol - 1][memn].cmolp = getdsp (symtab);
  else
*/
    cmolpstack[ncmol - 1][memn].cmolp = *drawing;
  *drawing = NULL; /* prevent freeing twice! */
  cmolpstack[ncmol - 1][memn].semicolon = FALSE;
  cmolpstack[ncmol - 1][memn].backref = FALSE;
  cmolpstack[ncmol - 1][memn].start_of_princ_path = FALSE;
  cmolpstack[ncmol - 1][memn].level[0] = dc_lvl;
  if (index == 1)
  {
for (i = ncmol - 1, start_princ = 0; i > 0; i--)
{
  stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i - 1);
  if (stackp->semicolon)
  {
    if (start_princ == 0) start_princ = i - 1;
    else start_princ--; /* adjust for each additional semicolon */
printf("semicolon found at %d: start_princ=%d\n", i - 1, start_princ);
  }
}
    dc_lvl = 0;
    print_overall (*ea, *oy, *ty, *tc, *nsteps, start_princ, x1, x2, x3);
    alloct ("TEXTPSTACK", 0, 0, 0);
    tspn = allocn ("TEXTPSTACK", 0) - 1;
    strcpy (ttxt, "0____________________________________________________________________________________________________");
    alloct ("TEXT", tspn, 0, strlen (ttxt) + 1);
    textp = (char *) memptr ("TEXT", tspn, 0);
    strcpy (textp, ttxt);
    alloct ("TEXT", tspn, 0, strlen (x1) + 2);
    textp = (char *) memptr ("TEXT", tspn, 1);
    sprintf (textp, "0%s", x1);
    alloct ("TEXT", tspn, 0, strlen (x2) + 2);
    textp = (char *) memptr ("TEXT", tspn, 2);
    sprintf (textp, "0%s", x2);
    alloct ("TEXT", tspn, 0, strlen (x3) + 2);
    textp = (char *) memptr ("TEXT", tspn, 3);
    sprintf (textp, "0%s", x3);
for (i = ncmol; i > 0; i--)
{
  stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i - 1);
  if (stackp->semicolon)
  {
    if (i == ncmol) return;
    stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i);
    stackp->start_of_princ_path = TRUE;
get_levels (i);
    return;
  }
}
stackp=(CMPSTKST *) memptr ("CMOLPSTACK", 0, 0);
stackp->start_of_princ_path = TRUE;
get_levels (0);
    return;
  }
  if (subg == NULL)
  {
    dc_lvl = 0;
    alloct ("TEXTPSTACK", 0, 0, 0);
    cmolpstack[ncmol - 1][0].semicolon = TRUE;
    *ty = *oy = 100.; /* only figure in main path */
    return;
  }
  for (brother = sson (cfather (node)); brother != NULL; brother = cbrthr (brother))
  {
    bsymtab = PstComp_SymbolTable_Get (brother);
    bindex = SymTab_Index_Get (bsymtab);
    if (bindex != index)
    {
      memn++;
      if (!find_in_stack (memn, bsymtab))
      {
        cmolpstack[ncmol - 1][memn].inx = /* bindex */ bsymtab;
        cmolpstack[ncmol - 1][memn].semicolon = cmolpstack[ncmol - 1][memn].backref =
          cmolpstack[ncmol - 1][memn].start_of_princ_path = FALSE;
      }
      cmolpstack[ncmol - 1][memn].level[0] = dc_lvl;
      cmolpstack[ncmol - 1][memn].cmolp = getdsp (bsymtab, TRUE);
    }
  }
  dc_lvl++;
  library = lib (subg);
  chapter = chpn (subg);
  schema = schn (subg);
  pschn = persist_schn (subg);
  level = 0;

  ++*nsteps; /* NOT the same as *nsteps++, which increments addr, NOT value!!! */
/*
  *ea += fixmax (100 - ease (subg), conj_ea (subg, node, start));
  *oy = *oy * yield (subg) / 100;
  *ty = yield (subg) * fixmin (*ty, conj_ty (subg, node, start)) / 100;
  *tc = fixmin (*tc, fixmin (conf (subg), conj_tc (subg, node, start)));
*/
  *ea += (float) (100 - ease (subg));
  *oy *= (float) yield (subg) / 100.;
  *tc = fixmin (*tc, conf (subg));
  alloct ("TEXTPSTACK", 0, 0, 0);
  tspn = allocn ("TEXTPSTACK", 0) - 1;
  if (chapter > 0 && schema > 0) /* chemistry is there */
  {
    if (s_ease (pschn) == ease (subg)) sprintf (estring, " [ease: %3d", s_ease (pschn));
    else sprintf (estring, " [ease: %3d->%3d", s_ease(pschn), ease (subg));
    if (s_yield (pschn) == yield (subg)) sprintf (ystring, "; yield: %3d", s_yield (pschn));
    else sprintf (ystring, "; yield: %3d->%3d", s_yield (pschn), yield (subg));
    if (s_conf (pschn) == conf (subg)) sprintf (cstring, "; confidence: %3d", s_conf(pschn));
    else sprintf (cstring, "; confidence: %3d->%3d", s_conf(pschn), conf(subg));
    sprintf (ttxt, "0   lib %d chap %2d sch %3d (%d): %s%s%s%s; merit%s: %3d]",
      library, chapter, schema, pschn, schname (pschn), estring, ystring, cstring, string, merit);
    alloct ("TEXT", tspn, 0, strlen (ttxt) + 1);
    textp = (char *) memptr ("TEXT", tspn, 0);
    strcpy (textp, ttxt);
    schrec = React_Schema_Handle_Get (pschn);
    txt = React_Text_Get (schrec);
    txt_head = React_TxtRec_Head_Get (txt);
    nref = React_TxtHd_NumReferences_Get (txt_head);
    ncomm = React_TxtHd_NumComments_Get (txt_head);
    for (i = j = 0; i < nref + ncomm; i++)
    {
      cont = FALSE;
      if (i < nref)
      {
/*
        sprintf (reftext, "<REFERENCE>\n%s", String_Value_Get (React_TxtRec_Reference_Get (txt, i)));
        ignore = reftext[12] == '\007';
*/
        sprintf (reftext, "<REFERENCE>  %s", String_Value_Get (React_TxtRec_Reference_Get (txt, i)));
        ignore = reftext[13] == '\007';
        comment = FALSE;
      }
      else
      {
/*
        sprintf (reftext, "<COMMENT>\n%s", String_Value_Get (React_TxtRec_Comment_Get (txt, i - nref)));
        ignore = strncmp (reftext + 10, "Schema ", 6) == 0 &&
          (strncmp (reftext + 17, "moved ", 6) == 0 ||
          strncmp (reftext + 17, "copied ", 7) == 0 ||
          strncmp (reftext + 17, "priority ", 9) == 0);
*/
        sprintf (reftext, "<COMMENT>    %s", String_Value_Get (React_TxtRec_Comment_Get (txt, i - nref)));
        ignore = strncmp (reftext + 13, "Schema ", 6) == 0 &&
          (strncmp (reftext + 20, "moved ", 6) == 0 ||
          strncmp (reftext + 20, "copied ", 7) == 0 ||
          strncmp (reftext + 20, "priority ", 9) == 0);
        comment = TRUE;
      }
      if (!ignore) while (reftext[0] != '\0')
      {
        j++;
        nextrt = strstr (reftext, "\n");
        if (nextrt != NULL) *nextrt++ = '\0';
        else if (strlen (reftext) <= 80) nextrt = reftext + strlen (reftext);
        else
        {
          nextrt = reftext + 80;
          while (*nextrt != ' ' && nextrt != reftext) nextrt--;
          if (nextrt == reftext) nextrt = reftext + strlen (reftext);
          else *nextrt++ = '\0';
        }
/*
        if (cont) sprintf (ttxt, "%d        ", comment ? 1 : 0);
*/
        if (cont) sprintf (ttxt, "%d                ", comment ? 1 : 0);
        else
        {
          sprintf (ttxt, "%d   ", comment ? 1 : 0);
          k = strlen (reftext);
          while (k != 0 && reftext[k - 1] == ' ') reftext[--k] = '\0';
        }
        strcat (ttxt, reftext);
        alloct ("TEXT", tspn, 0, strlen (ttxt) + 1);
        textp = (char *) memptr ("TEXT", tspn, j);
        strcpy (textp, ttxt);
        strcpy (reftext, nextrt);
        cont = TRUE;
      }
    }
  } /* chemistry is there */
/*
ELSE DO;
   CALL FORMAT_SCHNAME('** Unavailable transform - please modify reaction '||
      'library **');
   PUT SKIP EDIT('  |    (EASE: ',100,'; YIELD: ',100,'; CONFIDENCE: ',100,
      '; MERIT',STRING,': ',MERIT,')')(3(A,F(3)),3 A,F(3),A);
END;
IF FRCNODE(SUBG) THEN DO;
   PUT EDIT('  |','  |    ** SYNCHEM was forced to generate this ')(2(SKIP,A));
   IF METABOL=1 THEN PUT EDIT('metabolite **')(A);
   ELSE PUT EDIT('subgoal **')(A);
END;
IF METABOL=0 & ^graphical THEN PUT EDIT('\ | /',' \|/','  V') (3(SKIP,A));
*/
   /* draw arrowhead */
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
DRAW_COMPOUNDS: PROC(NODE,SUBG,MERIT,STRING,EA,OY,TY,TC,START);
dcl (x1,x2) char(500) var;
DCL (NODE,SUBG,MERIT,EA,OY,TY,TC) BIN FIXED(15);
DCL START BIT(1);
DCL STRING CHAR(*) VAR;
DCL (I,J,INDEX,BINDEX,BROTHER,LIBRARY,CHAPTER,SCHEMA,
   SUBGNOTE,LCS_NOTE,CH_SCH) BIN FIXED(15);
DCL LIBPIC PIC'9';
DCL LIBCHAR CHAR(1);
DCL REFTYPE CHAR(3);
DCL REFDATE CHAR(6);
DCL REFTEXT CHAR(80) VAR;
DCL (REFKEY,NEWKEY) PIC'99999999';
DCL (PROT_BIT,IGNORE,FIRST_REF) BIT(1) INIT('1'B);
DCL GRAPHICAL BIT(1) INIT(ttype='VAXStation II');
dcl tspn pic'99';
dcl (estring,ystring,cstring) char(100) var;
dcl comment bit(1) init('0'b);
/* draws compound; if not target compound, draws arrow to next compound,
   alongside which appear any conjuncts and information about the reaction */
INDEX=CINDEX(NODE);
if graphical then do;
	nmem=nsons(father(node));
	call alloct('CMOLPSTACK','PATHTRC',cmpsp,nmem,'CHAR',6); /* 2 bytes for
		integer; 4 bytes for pointer */
	memn=1;
	inx(memn)=index;
	cmolp(memn)=DRAWSlc_fix(GETSTR(INDEX));
end;
else do;
	CALL DRAWSNP(GETSTR(INDEX),DRAWPTR,1,0,SIZE1,SIZE2);
	PUT SKIP;
	DO I=1 TO SIZE1;
	   PUT SKIP;
	   DO J=1 TO SIZE2;
	      PUT EDIT(DRAWING(I,J))(A);
	   END;
	END;
	CALL FREEX('DRAWING','DRAWSLI',DRAWPTR);
	PUT SKIP(2) EDIT('(COMPOUND ',INDEX)(A,F(4));
	IF AVAIL(INDEX) THEN CALL PRINT_AVL_DATA(INDEX);
	ELSE IF SLVD(INDEX) THEN IF METABOL=1 THEN PUT EDIT(' - METABOLIZED')(A);
	ELSE PUT EDIT(' - SOLVED')(A);
	ELSE IF CSTUCK(INDEX) THEN PUT EDIT(' - STUCK')(A);
	ELSE IF UNSOLV(INDEX) THEN PUT EDIT(' - UNSOLVABLE')(A);
	ELSE IF DEV(INDEX)>0 THEN PUT EDIT(' - DEVELOPED')(A);
	PUT EDIT(')')(A);
end;
IF INDEX=1 THEN DO;
   CALL PRINT_OVERALL(EA,OY,TY,TC,x1,x2);
   if graphical then do;
	call alloct('TEXTPSTACK','PATHTRC',tsp,0,'PTR',0);
	tspn=allocn('TEXTPSTACK','PATHTRC');
	call alloct('TEXT'||tspn,'PATHTRC',textpstack,0,'CHAR VAR',500);
	textp=textpstack;
	text='0'||copy('_',100);
	call alloct('TEXT'||tspn,'PATHTRC',textpstack,0,'CHAR VAR',500);
	textp=textpstack;
	text='0'||x1;
	call alloct('TEXT'||tspn,'PATHTRC',textpstack,0,'CHAR VAR',500);
	textp=textpstack;
	text='0'||x2;
   end;
   else put skip(2) edit(x1,x2)(a,skip,a);
   RETURN;
END;
if ^graphical then do;
	IF METABOL=1 THEN DO;
	   "PUT EDIT('  ^',' /|\','/ | \')(3(SKIP,A)); /* draw arrowhead */" /* fool stupid LINTs that ignore #ifdef! */
	   PUT EDIT('  |','  |    METABOLITE ',-SUBG,':','  |')
	      (SKIP,A,SKIP,A,F(4),A,SKIP,A);
	END;
	ELSE PUT EDIT('  |','  |    SUBGOAL ',-SUBG,':','  |')
	   (SKIP,A,SKIP,A,F(4),A,SKIP,A);
end;
BROTHER=SON(FATHER(NODE));
DO WHILE(BROTHER^=0);
   BINDEX=CINDEX(BROTHER);
   IF BINDEX^=INDEX THEN DO;
	if graphical then do;
		memn=memn+1;
		inx(memn)=bindex;
		cmolp(memn)=drawslc_fix(getstr(bindex));
	end;
	else do;
	      CALL DRAWSNP(GETSTR(BINDEX),DRAWPTR,1,0,SIZE1,SIZE2);
	      DO I=1 TO SIZE1;
		 PUT SKIP EDIT('  |    ')(A);
		 DO J=1 TO SIZE2;
		    PUT EDIT(DRAWING(I,J))(A);
		 END;
	      END;
	      CALL FREEX('DRAWING','DRAWSLI',DRAWPTR);
	      PUT EDIT('  |','  |    (COMPOUND ',BINDEX)(2(SKIP,A),F(4));
	      IF AVAIL(BINDEX) THEN CALL PRINT_AVL_DATA(BINDEX);
	      ELSE IF SLVD(BINDEX) THEN IF METABOL=1 THEN PUT EDIT(' - METABOLIZED')
	         (A);
	      ELSE PUT EDIT(' - SOLVED')(A);
	      ELSE IF CSTUCK(BINDEX) THEN PUT EDIT(' - STUCK')(A);
	      ELSE IF UNSOLV(BINDEX) THEN PUT EDIT(' - UNSOLVABLE')(A);
	      ELSE IF DEV(BINDEX)>0 THEN PUT EDIT(' - DEVELOPED')(A);
	      PUT EDIT(')','  |')(A,SKIP,A);
	end;
   END;
   BROTHER=BRTHR(BROTHER);
END;
LIBRARY=LIB(SUBG);
IF LIBRARY^=CRTLIB THEN DO;
   LIBPIC=LIBRARY-1;
   LIBCHAR=LIBPIC;
   IF CRTLIB^=0 THEN DO;
      CALL CXRLIB;
      CALL FREEREF;
   END;
   CRTLIB=LIBRARY;
   CALL OXRLIB(LIBCHAR);
   CALL ALLOREF(LIBCHAR);
END;
CHAPTER=CHPN(SUBG);
SCHEMA=SCHN(SUBG);
LEVEL=0;
EA=MAX(EA+100-EASE(SUBG),CONJ_EA(SUBG,NODE,START));
OY=DIVIDE(OY*YIELD(SUBG),100,15);
TY=DIVIDE(YIELD(SUBG)*MIN(TY,CONJ_TY(SUBG,NODE,START)),100,15);
TC=MIN(TC,MIN(CONF(SUBG),CONJ_TC(SUBG,NODE,START)));
if graphical then call alloct('TEXTPSTACK','PATHTRC',tsp,0,'PTR',0);
tspn=allocn('TEXTPSTACK','PATHTRC');
IF CHAPTER>0 & SCHEMA>0 THEN DO; /* chemistry is there */
   if graphical then do;
	call alloct('TEXT'||tspn,'PATHTRC',textpstack,0,'CHAR VAR',500);
	textp=textpstack;
	if s_ease(chapter,schema)=ease(subg) then put string(estring) edit
		(' [ease: ',s_ease(chapter,schema))(a,f(3));
	else put string(estring) edit(' [ease: ',s_ease(chapter,schema),'->',
		ease(subg))(2(a,f(3)));
	if s_yield(chapter,schema)=yield(subg) then put string(ystring) edit
		('; yield: ',s_yield(chapter,schema))(a,f(3));
	else put string(ystring) edit('; yield: ',s_yield(chapter,schema),'->',
		yield(subg))(2(a,f(3)));
	if s_conf(chapter,schema)=conf(subg) then put string(cstring) edit
		('; confidence: ',s_conf(chapter,schema))(a,f(3));
	else put string(cstring) edit('; confidence: ',s_conf(chapter,schema),
		'->',conf(subg))(2(a,f(3)));
	put string(text) edit('0   lib ',library,' chap ',chapter,' sch ',
		schema,	': ',schname(chapter,schema),estring,ystring,cstring,
		'; merit',string,': ',merit,']')(a,f(1),a,f(2),a,f(3),8 a,f(3),a);
   end;
   else do;
	PUT SKIP EDIT('  |    LIBRARY ',LIBRARY,', CHAPTER ',CHAPTER,
		', SCHEMA ',SCHEMA)(A,F(1),A,F(2),A,F(3));
	   CALL FORMAT_SCHNAME(SCHNAME(CHAPTER,SCHEMA));
	   PUT SKIP EDIT('  |    (EASE: ',S_EASE(CHAPTER,SCHEMA))(A,F(3));
	   IF S_EASE(CHAPTER,SCHEMA)^=EASE(SUBG) THEN PUT EDIT('->',EASE(SUBG))
	      (A,F(3));
	   PUT EDIT('; YIELD: ',S_YIELD(CHAPTER,SCHEMA))(A,F(3));
	   IF S_YIELD(CHAPTER,SCHEMA)^=YIELD(SUBG) THEN PUT EDIT('->',
		YIELD(SUBG))(A,F(3));
	   PUT EDIT('; CONFIDENCE: ',S_CONF(CHAPTER,SCHEMA))(A,F(3));
	   IF S_CONF(CHAPTER,SCHEMA)^=CONF(SUBG) THEN PUT EDIT('->',CONF(SUBG))
	      (A,F(3));
	   PUT EDIT('; MERIT',STRING,': ',MERIT,')')(3 A,F(3),A);
   end;
   REFKEY=FREFKEY(CHAPTER,SCHEMA);
   DO WHILE(REFKEY^=0);
      CALL GREFREC(REFKEY,REFTYPE,REFDATE,REFTEXT,NEWKEY);
      IF REFTYPE='LIB' THEN IGNORE='1'B;
      else if reftype='COM' then do;
	 ignore='0'b;
	 comment='1'b;
      end;
      ELSE IF REFTYPE^='CON' THEN do;
	 IGNORE='0'B;
	 comment='0'b;
      end;
      IF ^IGNORE THEN DO;
	 if graphical then do;
		call alloct('TEXT'||tspn,'PATHTRC',textpstack,0,'CHAR VAR',500);
		textp=textpstack;
		text=comment||'   ';
	 end;
	 else do;
	         IF FIRST_REF THEN PUT SKIP EDIT('  |')(A);
        	 FIRST_REF='0'B;
	         PUT SKIP EDIT('  |    ')(A);
	 end;
         IF REFTYPE='CON' THEN do;
		if graphical then text=comment||'        ';
		else PUT EDIT('     ')(A);
	 end;
         ELSE DO;
            I=LENGTH(REFTEXT);
            IF REFTEXT='' THEN REFTEXT='';
            ELSE DO WHILE(SUBSTR(REFTEXT,I,1)=' ');
               I=I-1;
               REFTEXT=SUBSTR(REFTEXT,1,I);
            END;
            REFTEXT=DATE_FORMAT(REFDATE)||REFTEXT;
         END;
	 if graphical then text=text||reftext;
         else PUT EDIT(REFTEXT)(A);
      END;
      REFKEY=NEWKEY;
   END;
   SUBGNOTE = NOTE(SUBG);
   IF SUBGNOTE ^= 0 THEN DO;
      IF SUBGNOTE < 0 THEN DO;
         IF SUBGNOTE < -16384 THEN DO;
            PROT_BIT = '0'B;
            SUBGNOTE = SUBGNOTE + 16384;
         END;
         LIBRARY = LIB(SUBGNOTE);
         CHAPTER = CHPN(SUBGNOTE);
         SCHEMA = SCHN(SUBGNOTE);
      END;
      ELSE DO;
         LCS_NOTE = SUBGNOTE - 1; /* check for protection schema use */
         LIBRARY = DIVIDE(LCS_NOTE,4000,15) + 1; /* decode schema location */
         CH_SCH = MOD(LCS_NOTE,4000);
         CHAPTER = DIVIDE(CH_SCH,100,15) + 1;
         SCHEMA = MOD(CH_SCH,100) + 1;
      END;
      IF LIBRARY^=CRTLIB THEN DO;
         LIBPIC=LIBRARY-1;
         LIBCHAR=LIBPIC;
         IF CRTLIB^=0 THEN DO;
            CALL CXRLIB;
            CALL FREEREF;
         END;
         CRTLIB=LIBRARY;
         CALL OXRLIB(LIBCHAR);
         CALL ALLOREF(LIBCHAR);
      END;
      IF PROT_BIT THEN PUT SKIP EDIT('  |',
         '  |    Application of the above schema was permitted ',
         'by the presence of a protecting group;',
         '  |    the corresponding deprotection step, which occurs at this ',
         'point, uses...')(2(A,SKIP,A),A);
      ELSE PUT SKIP EDIT('  |','  |    Application of the above schema was ',
         'permitted by the presence of the required functionality;',
         '  |    that functionality is modified at this point through the ',
         'following reaction:')(2(A,SKIP,A),A);
      CALL FORMAT_SCHNAME('  '||SCHNAME(CHAPTER,SCHEMA));
      PUT SKIP EDIT('  |      (LIBRARY ',LIBRARY,', CHAPTER ',CHAPTER,
         ', SCHEMA ',SCHEMA,')')(A,F(1),A,F(2),A,F(3),A);
      REFKEY=FREFKEY(CHAPTER,SCHEMA);
      DO WHILE(REFKEY^=0);
         CALL GREFREC(REFKEY,REFTYPE,REFDATE,REFTEXT,NEWKEY);
         IF REFTYPE='LIB' THEN IGNORE='1'B;
         ELSE IF REFTYPE^='CON' THEN IGNORE='0'B;
         IF ^IGNORE THEN DO;
            IF FIRST_REF THEN PUT SKIP EDIT('  |')(A);
            FIRST_REF='0'B;
            PUT SKIP EDIT('  |      ')(A);
            IF REFTYPE='CON' THEN PUT EDIT('     ')(A);
            ELSE DO;
               I=LENGTH(REFTEXT);
               IF REFTEXT='' THEN REFTEXT='';
               ELSE DO WHILE(SUBSTR(REFTEXT,I,1)=' ');
                  I=I-1;
                  REFTEXT=SUBSTR(REFTEXT,1,I);
               END;
               REFTEXT=DATE_FORMAT(REFDATE)||REFTEXT;
            END;
            PUT EDIT(REFTEXT)(A);
         END;
         REFKEY=NEWKEY;
      END;
   END; /* if SUBGNOTE */
END; /* chemistry is there */
ELSE DO;
   CALL FORMAT_SCHNAME('** Unavailable transform - please modify reaction '||
      'library **');
   PUT SKIP EDIT('  |    (EASE: ',100,'; YIELD: ',100,'; CONFIDENCE: ',100,
      '; MERIT',STRING,': ',MERIT,')')(3(A,F(3)),3 A,F(3),A);
END;
IF FRCNODE(SUBG) THEN DO;
   PUT EDIT('  |','  |    ** SYNCHEM was forced to generate this ')(2(SKIP,A));
   IF METABOL=1 THEN PUT EDIT('metabolite **')(A);
   ELSE PUT EDIT('subgoal **')(A);
END;
IF METABOL=0 & ^graphical THEN PUT EDIT('\ | /',' \|/','  V') (3(SKIP,A));
   /* draw arrowhead */

drawslc_fix: proc(sl) returns(ptr);
dcl sl char(*) var;
dcl sli char(length(sl)) var init(sl);
dcl index builtin;
dcl i bin fixed(15);
i=index(sli,'|');
if i>0 then sli=substr(sli,1,i-1);
return(drawslc(sli));
end;

END DRAW_COMPOUNDS;
#endif

int conj_ea (Subgoal_t *s, Compound_t *n, Boolean_t st)
{
  return (conj_eyc (s, n, "E", st));
}

int conj_ty (Subgoal_t *s, Compound_t *n, Boolean_t st)
{
  return (conj_eyc (s, n, "Y", st));
}

int conj_tc (Subgoal_t *s, Compound_t *n, Boolean_t st)
{
  return (conj_eyc (s, n, "C", st));
}

int conj_eyc (Subgoal_t *s, Compound_t *n, char *x, Boolean_t st)
{
  Compound_t *c, *dev_c;
  Subgoal_t *son, **sub, *best_sub;
  SymTab_t *ci;
  int i, value, new_value, *merit;

  i = 0;
  value = new_value = 100;
  if (*x == 'E') value = 0;
  level++;
  if (level == 100)
  {
    level--;
    fprintf (stderr, "Circularity or excessively long path encountered in CONJ_%c%c\n", *x == 'E' ? *x : 'T', *x == 'E' ? 'A' : *x);
    if (*x == 'E') return (300);
    return (0);
  }
  c = sson (s);
  while (c != NULL)
  {
    ci = PstComp_SymbolTable_Get (c);
    dev_c = dev (ci);
    if (dev_c != NULL && ncsons (dev_c) == 0) dev_c = NULL; /* correct for stuck and
                                                               newly-selected nodes */
    if (*x == 'E') new_value = 0;
    else new_value = 100;
    if ((st || c != n) && !avail (ci)) if (dev_c != NULL)
    {
      sub = (Subgoal_t **) malloc (ncsons (dev_c) * sizeof (Subgoal_t *));
      merit = (int *) malloc (ncsons (dev_c) * sizeof (int));
      son = cson (dev_c);
      i = 0;
      while (son != NULL)
      {
        sub[i] = son;
        if (solved (son)) merit[i] = ssmert (son) + 1000;
	/* prefer best solved pathway */
        else merit[i] = smerit (son);
        i++;
        son = sbrthr (son);
      }
      for (i = 1; i < ncsons (dev_c); i++) if (merit[0] < merit[i])
      {
        merit[0] = merit[i];
        sub[0] = sub[i];
      }
      best_sub = sub[0];
      free (sub);
      sub=NULL;
      free (merit);
      merit=NULL;
      if (*x == 'E') new_value += 100 - ease (best_sub) + conj_ea (best_sub, NULL, FALSE);
      else if (*x == 'Y') new_value = new_value * yield (best_sub) / 100 * conj_ty (best_sub, NULL, FALSE) / 100;
      else new_value = fixmin (fixmin (new_value, conf (best_sub)), conj_tc (best_sub, NULL, FALSE));
    }
    else /* undeveloped compound - use compound merit for estimate */
    {
      new_value = cmerit (ci);
      if (*x == 'E') new_value = 100 - new_value;
    }
    c = cbrthr (c);
    if (*x == 'E') value = fixmax (value, new_value);
    else value = fixmin (value, new_value);
  }
  level--;
  return (value);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
CONJ_EA: PROC(S,N,ST) RECURSIVE RETURNS(BIN FIXED(15));
DCL (S,N) BIN FIXED(15);
DCL ST BIT(1);
DCL (I,C,DEV_C,CI) BIN FIXED(15) INIT(0);
DCL (VALUE,NEW_VALUE) BIN FIXED(15) INIT(100);
DCL X CHAR(1) INIT('C');
X='E';
VALUE=0;
CONJ_TY: ENTRY(S,N,ST) RETURNS(BIN FIXED(15));
IF X='C' THEN X='Y';
CONJ_TC: ENTRY(S,N,ST) RETURNS(BIN FIXED(15));
LEVEL=LEVEL+1;
IF LEVEL=100 THEN DO;
   LEVEL=LEVEL-1;
   PUT SKIP EDIT('CIRCULARITY OR EXCESSIVELY LONG PATH ENCOUNTERED IN CONJ_')
      (A);
   IF X='E' THEN DO;
      PUT EDIT('EA')(A);
      RETURN(300);
   END;
   ELSE PUT EDIT('T',X)(A,A);
   RETURN(0);
END;
C=SON(S);
DO WHILE(C^=0);
   CI=CINDEX(C);
   DEV_C=DEV(CI);
   if dev_c>0 then if nsons(dev_c)=0 then dev_c=0; /* correct for stuck and
                                                      newly-selected nodes */
   IF X='E' THEN NEW_VALUE=0;
   ELSE NEW_VALUE=100;
   IF (ST | C^=N) & ^AVAIL(CI) THEN IF DEV_C^=0 THEN BEGIN;
      DCL ((SUB,MERIT)(NSONS(DEV_C)),CSON) BIN FIXED(15);
      CSON=SON(DEV_C);
      I=0;
      DO WHILE(CSON^=0);
         I=I+1;
         SUB(I)=CSON;
         IF SOLVED(CSON) THEN MERIT(I)=SSMERT(CSON)+1000;
	 /* prefer best solved pathway */
         ELSE MERIT(I)=SMERIT(CSON);
         CSON=BRTHR(CSON);
      END;
      DO I=2 TO HBOUND(SUB,1);
         IF MERIT(1)<MERIT(I) THEN DO;
            MERIT(1)=MERIT(I);
            SUB(1)=SUB(I);
         END;
      END;
      IF X='E' THEN NEW_VALUE=NEW_VALUE+100-EASE(SUB(1))+CONJ_EA(SUB(1),
         0,'0'B);
      ELSE IF X='Y' THEN NEW_VALUE=DIVIDE(DIVIDE(NEW_VALUE*YIELD(SUB(1)),
         100,15)*CONJ_TY(SUB(1),0,'0'B),100,15);
      ELSE NEW_VALUE=MIN(MIN(NEW_VALUE,CONF(SUB(1))),CONJ_TC(SUB(1),0,'0'B));
   END;
   ELSE DO; /* undeveloped compound - use compound merit for estimate */
      NEW_VALUE=CMERIT(CI);
      IF X='E' THEN NEW_VALUE=100-NEW_VALUE;
   END;
   C=BRTHR(C);
   IF X='E' THEN VALUE=MAX(VALUE,NEW_VALUE);
   ELSE VALUE=MIN(VALUE,NEW_VALUE);
END;
LEVEL=LEVEL-1;
RETURN(VALUE);
END CONJ_EA; /* CONJ_TY, CONJ_TC */
#endif

void print_overall (float ea, float oy, float ty, int tc, int nsteps,
  int start_princ, char *x1, char *x2, char *x3)
{
  int em;
  char x[64];

  sprintf (x, "NUMBER OF STEPS IN PRINCIPAL PATHWAY:%3d", nsteps -
    start_princ);
  while (strlen (x) < 59) strcat (x, " ");
  sprintf (x1, "%sASSESSMENT OF THRESHOLD CONFIDENCE:%3d", x, tc);
  sprintf (x2, "                         AVERAGE VALUE OF EASE PER STEP OVER DISPLAYED SYNTHESIS:%3d                         ",
    100 - (int) (ea / (float) nsteps + .5));
  while (strlen (x2) > 100)
  {
    strcpy (x2, x2 + 1);
    x2[strlen (x2) - 1] = '\0';
  }
  while (x2[strlen (x2) - 1] == ' ') x2[strlen (x2) - 1] = '\0';
/*
  sprintf (x3, "PROJ. YIELD ALONG PRINCIPAL DISPLAYED PATHWAY ASSUMING TERMINAL NODE (CMP. %d) IS AVAILABLE:%3d%%",
    SymTab_Index_Get (mainroot->indexn), (int) (oy + .5));
*/
  if (oy + .5 >= 5.)
    sprintf (x3, "PROJ. YIELD ALONG PRINCIPAL PATHWAY ASSUMING STARTING MATERIAL AND CO-REACTANTS ARE AVAILABLE:%3d%%", 
    (int) (oy + .5));
  else
    strcpy (x3, "PROJ. YIELD ALONG PRINCIPAL PATHWAY ASSUMING STARTING MATERIAL AND CO-REACTANTS ARE AVAILABLE: LOW");

#ifdef LEGACY_CODE
  em = 100 - 66 * ea / nsteps / 300;
  if (oy >= 5) sprintf (x, "PROJECTED YIELD BASED ON PRESUMED AVAILABILITY:%3d%%", /* SymTab_Index_Get (mainroot->indexn), */ oy);
  else sprintf (x, "PROJECTED YIELD BASED ON PRESUMED AVAILABILITY: LOW (<5%%)" /*, SymTab_Index_Get (mainroot->indexn) */);
  while (strlen (x) < 59) strcat (x, " ");
  sprintf (x1, "%sPROJECTED OVERALL YIELD BASED ON ALL", x);
  sprintf (x, "NUMBER OF STEPS IN DISPLAYED SYNTHESIS: %3d", nsteps);
  while (strlen (x) < 65) strcat (x, " ");
  if (ty >= 5) sprintf (x2, "%sUNAVAILABLE CONJUNCTS:%3d%%", x, ty);
  else sprintf (x2, "%sUNAVAILABLE CONJUNCTS: LOW (<5%%)", x);
  sprintf (x, "AVERAGE EASE:%3d", fixmax (100 - ea / nsteps * em / 100, 0));
  while (strlen (x) < 59) strcat (x, " ");
  sprintf (x3, "%sASSESSMENT OF THRESHOLD CONFIDENCE:%3d", x, tc);
#endif
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
PRINT_OVERALL: PROC(EA,OY,TY,TC,x1,x2);
DCL (EA,OY,TY,TC) BIN FIXED(15);
dcl (x1,x2) char(*) var;
DCL EM BIN FIXED(15);
dcl x char(59);
EM=100-DIVIDE(66*EA,300,15);
PUT string(x) EDIT('PROJECTED YIELD BASED ON COMPOUND ',ROOTCOMP->INDEXN,':',OY)
	(a,f(5),a,f(3));
put string(x1) edit(x,'PROJECTED OVERALL YIELD BASED ON ALL'||copy(' ',57)||
	'UNAVAILABLE CONJUNCTS:',TY)(2 a,f(3));
put string(x) edit('RELATIVE ESTIMATE OF CUMULATIVE EASE:',
	MAX(100-DIVIDE(EA*EM,100,15),0))(a,f(3));
put string(x2) edit(x,'ASSESSMENT OF THRESHOLD CONFIDENCE:',TC)(2 a,f(3));
END PRINT_OVERALL;

DATE_FORMAT: PROC(D) RETURNS(CHAR(20) VAR);
DCL D CHAR(6);
DCL MONS(12) CHAR(9) VAR INIT('JANUARY','FEBRUARY','MARCH','APRIL','MAY',
   'JUNE','JULY','AUGUST','SEPTEMBER','OCTOBER','NOVEMBER','DECEMBER');
RETURN(MONS(SUBSTR(D,3,2))||' '||SUBSTR(D,5)||', 19'||SUBSTR(D,1,2)||
   ': ');
END DATE_FORMAT;

FORMAT_SCHNAME: PROC(N);
DCL N CHAR(200) VAR;
DCL REMAINDER CHAR(120) VAR;
DCL I BIN FIXED(15) INIT(121);
IF LENGTH(N)<121 THEN REMAINDER=N;
ELSE DO;
   DO WHILE(SUBSTR(N,I,1)^=' ');
      I=I-1;
   END;
   REMAINDER='     '||SUBSTR(N,I+1);
   PUT SKIP EDIT('  |    ',SUBSTR(N,1,I-1))(A,A);
END;
PUT SKIP EDIT('  |    ',REMAINDER)(A,A);
END FORMAT_SCHNAME;

PROMPT_FOR_OUTPUT: PROC;
DCL ANS CHAR(50) VAR INIT(' ');
DCL TRANSLATE BUILTIN;
DO WHILE(SUBSTR(ANS,1,1)^='N');
   CALL WALK_LIST('0'B,'ALL');
   PUT SKIP EDIT('Which pathways do you want to see in detail (A(ll)/N(one)/',
      '<numeric list of desired paths>)? ')(A,A);
   GET EDIT(ANS)(A(10));
   ANS=UPRCASE(ANS)||' ';
   IF SUBSTR(ANS,1,1)^='N' THEN CALL WALK_LIST('1'B,ANS);
END;
END PROMPT_FOR_OUTPUT;
#endif

void free_all (COMPOUND *p)
{
  int i;

  if (p == NULL) return;
  for (i = 0; i < p->number_of_instances; i++) free_all ((COMPOUND *) p->nodes[i].dad.ptr);
  free (p->nodes);
  if (p->drawing != NULL) free_Molecule (p->drawing);
  free (p);
  p=NULL;
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
FREE_ALL: PROC(P) RECURSIVE;
DCL P PTR;
DCL I BIN FIXED(15);
IF P=NULL THEN RETURN;
DO I=1 TO P->NUMBER_OF_INSTANCES;
   CALL FREE_ALL(P->PTR(I));
END;
NUM_INSTS=P->NUMBER_OF_INSTANCES;
FREE P->COMPOUND;
END FREE_ALL;

PRINT_AVL_DATA: PROC(X);
DCL X BIN FIXED(15);
DCL (NOINFO,NOCAT) BIT(1) INIT('1'B);
DCL REGN CHAR(7) VAR;
DCL FILENUM BIN FIXED(15);
DCL (NAME,CATALOG,WLN,ACCNO,TEXTDS,SYNNO) CHAR(200) VAR;
PUT EDIT(' - ')(A);
REGN=CATN(X);
FILENUM=AVLFN(X);
IF FILENUM^=CRTAVL THEN DO;
   IF CRTAVL^=0 THEN CALL CLOSLIB(CRTAVL);
   CALL OPENLIB(FILENUM);
   CRTAVL=FILENUM;
END;
NAME=AVLDATA(REGN,'NM');
IF NAME^='NO FIELD WITH THAT TAG' THEN DO;
   NOINFO='0'B;
   PUT EDIT(NAME,' ')(A,A);
   END;
CATALOG=AVLDATA(REGN,'C#');
IF CATALOG^='NO FIELD WITH THAT TAG' THEN DO;
   NOINFO='0'B;
   NOCAT='0'B;
   PUT EDIT('AVAILABLE from ',CATALOG)(A,A);
   END;
WLN=AVLDATA(REGN,'WL');
IF WLN^='NO FIELD WITH THAT TAG' THEN DO;
   NOINFO='0'B;
   PUT EDIT(' (WLN= ',WLN,' )')(A,A,A);
   END;
ACCNO=AVLDATA(REGN,'AN');
IF ACCNO^='NO FIELD WITH THAT TAG' THEN DO;
   NOINFO='0'B;
   PUT EDIT(' (Accession Number ',ACCNO,' )')(A,A,A);
   END;
SYNNO=AVLDATA(REGN,'SN');
IF SYNNO^='NO FIELD WITH THAT TAG' THEN DO;
   NOINFO='0'B;
   PUT EDIT(' (Synchem Number ',SYNNO,' )')(A,A,A);
   END;
TEXTDS=AVLDATA(REGN,'TD');
IF TEXTDS^='NO FIELD WITH THAT TAG' THEN DO;
   NOINFO='0'B;
   PUT EDIT(' (Text Descriptor ',TEXTDS,' )')(A,A,A);
   END;
IF NOINFO THEN PUT EDIT('AVAILABLE - No additional information on file')(A);
ELSE IF NOCAT THEN PUT EDIT(' - AVAILABLE')(A);
END PRINT_AVL_DATA;

GET_NUM: PROC(TEXT,NUM,LOWER,UPPER,INVALID);
DCL TEXT CHAR(*) VAR;
DCL (NUM,LOWER,UPPER,INVALID) BIN FIXED(15);
DCL ANSWER CHAR(500) VAR;
DCL SLING CHAR(500) VAR;
DCL (OK,VALID,MUST_TOGGLE) BIT(1) INIT('0'B);
DCL XTR PTR;
DCL (I,J) BIN FIXED(15);
DCL MOUSDRAW CHAR(15) VAR INIT('');
IF TTYPE='VAXStation II' THEN MOUSDRAW='or ''MOUSE'' ';
ELSE IF INDEX(TTYPE,'VT125')>0 THEN MOUSDRAW='or ''DRAWING'' ';
DO WHILE(^VALID);
   PUT SKIP(2) EDIT(TEXT)(A);
   GET EDIT(ANSWER)(A(500));
   IF INVALID=1 & VERIFY(ANSWER,' 0123456789')>0 THEN DO;
      SLING=ANSWER;
      OK='0'B;
      DO WHILE(^OK);
	 IF SLING='' THEN DO;
	    PUT SKIP(2) EDIT('Reenter SLING '||MOUSDRAW||'(<CR> to exit): ')
               (A,A);
	    GET EDIT(SLING)(A(500));
	 END;
	 OK=SLING='';
	 IF ^OK THEN DO;
	    SLING=UPRCASE(SLING);
            MUST_TOGGLE=SLING='MOUSE';
	    OK=SLINGOK(SLING);
            IF MUST_TOGGLE THEN PUT EDIT('[?3l','[?3h')(2(SKIP(0),A));
	    IF OK THEN DO;
	       CALL DRAWSNP(SLING,DRAWPTR,1,0,SIZE1,SIZE2);
	       DO I=1 TO SIZE1;
		  PUT SKIP;
		  DO J=1 TO SIZE2;
		     PUT EDIT(DRAWING(I,J))(A);
		  END;
	       END;
	       CALL FREEX('DRAWING','DRAWSLI',DRAWPTR);
	       PUT SKIP EDIT('OK (default=Y/N): ')(A);
	       GET EDIT(ANSWER)(A(50));
	       OK=SUBSTR(UPRCASE(ANSWER),1,1)^='N';
	    END;
	    IF ^OK THEN SLING='';
	 END;
      END;
      IF SLING^='' THEN DO;
	 XTR=HSLNXTR(SLING);
	 SLING=GETNAME(XTR,1);
	 CALL FREEXTR(XTR);
	 NUM=SYMSRCH(SLING);
	 VALID=NUM>1;
	 IF NUM=1 THEN PUT SKIP EDIT('Target cannot be its own precursor!')(A);
	 ELSE IF NUM=0 THEN PUT SKIP EDIT('Compound ''',SLING,
	    ''' was not found in the symbol table.')(3 A);
	 ELSE PUT SKIP EDIT('This is compound #',NUM,' in the symbol table.')
	    (A,F(4),A);
      END;
   END;
   ELSE DO;
      NUM=GETNUM(ANSWER);
      VALID=NUM^=-9999;
   END;
   IF VALID THEN DO;
      IF NUM=INVALID THEN DO;
	 VALID='0'B;
	 PUT SKIP EDIT('THE VALUE ',INVALID,' IS AN INVALID RESPONSE')
	    (A,F(5),A);
      END;
      ELSE IF UPPER=-1 THEN DO;
	 VALID=NUM>=LOWER;
	 IF ^VALID THEN PUT SKIP EDIT('NUMBER MAY NOT BE LESS THAN ',LOWER)
	    (A,F(5));
      END;
      ELSE DO;
	 VALID=NUM>=LOWER & NUM<=UPPER;
	 IF ^VALID THEN PUT SKIP EDIT('NUMBER MUST BE BETWEEN ',LOWER,' AND ',
	    UPPER)(2(A,F(5)));
      END;
   END;
   ELSE IF ^OK THEN PUT SKIP EDIT('A NUMERIC RESPONSE IS NEEDED')(A);
END;
END GET_NUM;

SYMSRCH: PROC(SLING) RETURNS(BIN FIXED(15));
DCL SLING CHAR(*) VAR;
DCL HASH_VALUE BIN FIXED(31);
DCL (K,LAST) BIN FIXED(15);
DCL FOUND BIT(1) INIT('0'B);
HASH_VALUE=HASH(SLING,HTSIZE());
K=HMATCH(HASH_VALUE);
DO WHILE(K^=0 & ^FOUND);
   LAST=K;
   IF GETSTR(K) = SLING THEN FOUND='1'B;
/* sling found in symbol table */
   ELSE K=TIE(LAST);
END;
RETURN(K);
END SYMSRCH;
#endif

int subgoal_merit (Subgoal_t *s, COMPOUND *p)
{
/* Modified from UPDATE.SUBGOAL_MERIT - not recursive in this application,
   because merits are calculated in the synthetic direction. */
  int comp_merit, comp_solved_merit, r, k, merit, solved_merit, minimum_merit, minimum_solved_merit,
    total_merit, total_solved_merit, adjusted_compound_merit, adj_c_s_merit, i, n, divisor;
  Compound_t *c;
  SymTab_t *ci;
  Boolean_t all_solved;

  minimum_merit = minimum_solved_merit = 100;
  total_merit = total_solved_merit = adjusted_compound_merit = adj_c_s_merit = 0;
  divisor = 20;
/* R is the reaction merit of subgoal S */
  r = (A * ease (s) + B * yield (s) + C * conf (s)) / (A + B + C);
  if (!PstSubg_Flags_Active_Get (s)) r = fixmax (0, r - D);
  all_solved = (solved (s) && p->comp_merit > 999);
  n = 0;
  c = sson (s); /* first conjunct for subgoal S */
  while (c != NULL)
  {
    ci = PstComp_SymbolTable_Get (c);
    if (all_solved || !slvd (ci) || (ci == p->indexn && p->comp_merit < 1000))
    {
      n++;
      if (ci == p->indexn) comp_merit = p->comp_merit;
      else comp_merit = cmerit (ci);
      if (comp_merit > 999) comp_merit -= 1000;
      minimum_merit = fixmin (minimum_merit, comp_merit);
      total_merit += comp_merit;
      if (all_solved)
      {
        if (ci == p->indexn) comp_solved_merit = p->comp_merit - 1000;
        else comp_solved_merit = csmert (ci);
	minimum_solved_merit = fixmin (minimum_solved_merit, comp_solved_merit);
	total_solved_merit += comp_solved_merit;
      }
    }
    c = cbrthr (c);
  }
  adjusted_compound_merit = minimum_merit - (100 * (n - 1) + minimum_merit - total_merit) / divisor;
  merit = r * fixmax (adjusted_compound_merit, 0) / 100;
  adj_c_s_merit = minimum_solved_merit - (100 * (n - 1) + minimum_solved_merit - total_solved_merit) / divisor;
  solved_merit = r * fixmax (adj_c_s_merit, 0) / 100;
  if (all_solved) return (solved_merit + 1000);
  return (merit);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
SUBGOAL_MERIT: PROC(S,P) RETURNS(BIN FIXED(15));
/* Modified from UPDATE.SUBGOAL_MERIT - not recursive in this application,
   because merits are calculated in the synthetic direction. */
DCL S BIN FIXED(15);
DCL P PTR;
DCL (COMP_MERIT,COMP_SOLVED_MERIT) BIN FIXED(15);
DCL (R,C,K,CI,MERIT,SOLVED_MERIT) BIN FIXED(15) INIT(0);
DCL ALL_SOLVED BIT(1);
DCL (MINIMUM_MERIT,MINIMUM_SOLVED_MERIT) BIN FIXED(15) INIT(100);
DCL (TOTAL_MERIT,TOTAL_SOLVED_MERIT) BIN FIXED(15) INIT(0);
DCL (ADJUSTED_COMPOUND_MERIT,ADJ_C_S_MERIT) BIN FIXED(15) INIT(0);
DCL (I,N) BIN FIXED(15);
DCL DIVISOR BIN FIXED(15) INIT(20);
/* R is the reaction merit of subgoal S */
R=RXMERIT(S);
ALL_SOLVED=SOLVED(S) & P->COMPOUND.COMP_MERIT>999;
N=0;
C=SON(S); /* first conjunct for subgoal S */
DO WHILE(C>0);
   CI = CINDEX(C);
   IF ALL_SOLVED | ^SLVD(CI) | (CI=P->INDEXN & P->COMPOUND.COMP_MERIT<1000)
      THEN DO;
      N=N+1;
      IF CI=P->INDEXN THEN COMP_MERIT=P->COMPOUND.COMP_MERIT;
      ELSE COMP_MERIT=CMERIT(CI);
      IF COMP_MERIT>999 THEN COMP_MERIT=COMP_MERIT-1000;
      MINIMUM_MERIT=MIN(MINIMUM_MERIT,COMP_MERIT);
      TOTAL_MERIT=TOTAL_MERIT+COMP_MERIT;
      IF ALL_SOLVED THEN DO;
	 IF CI=P->INDEXN THEN COMP_SOLVED_MERIT=P->COMPOUND.COMP_MERIT-1000;
	 ELSE COMP_SOLVED_MERIT=CSMERT(CI);
	 MINIMUM_SOLVED_MERIT=MIN(MINIMUM_SOLVED_MERIT,COMP_SOLVED_MERIT);
	 TOTAL_SOLVED_MERIT=TOTAL_SOLVED_MERIT+COMP_SOLVED_MERIT;
      END;
   END;
   C = BRTHR(C);
END;
ADJUSTED_COMPOUND_MERIT=MINIMUM_MERIT-DIVIDE(100*(N-1)+MINIMUM_MERIT-
   TOTAL_MERIT,DIVISOR,15);
MERIT=MAX(ADJUSTED_COMPOUND_MERIT,0);
MERIT=DIVIDE(MERIT*R,100,15);
ADJ_C_S_MERIT=MINIMUM_SOLVED_MERIT-DIVIDE(100*(N-1)+MINIMUM_SOLVED_MERIT-
   TOTAL_SOLVED_MERIT,DIVISOR,15);
SOLVED_MERIT=DIVIDE(R*MAX(ADJ_C_S_MERIT,0),100,15);
IF ALL_SOLVED THEN RETURN(SOLVED_MERIT+1000);
RETURN(MERIT);
END SUBGOAL_MERIT;
#endif

void gkstext (int numval, Boolean_t prtnum, Boolean_t cancel_prtnum, char *text, float *ht)
{
  char num[8], textcopy[5120], textcopy2[5120];
  int linen, nlines, forw, back, pos, pos2, yadj, tlen;
  Boolean_t done;
XSetFont(disp,gc,XLoadFont(disp,"-*-courier-medium-r-normal-*-14-*-75-75-*-*-iso8859-1"));
/*kka:
XSetFont(disp,gc,XLoadFont(disp,"-*-courier-medium-r-normal-*-15-*-75-75-*-*-iso8859-1"));
*/
  yadj = text[0] == '_' ? -3 : 0;
  sprintf (num, "%2d.", numval);
  tlen = strlen (text);
  nlines = 1 + (tlen + 5 * (tlen / 100)) / 106;
  strcpy (textcopy, text);
  *ht += 0.015 * (float) nlines;
  for (linen = nlines - 1; linen >= 0; linen--)
  {
    pos = back = forw = 95 * linen + 1;
    done = linen == 0;
    while (back > 95 * linen - 25 && !done)
    {
      done = TRUE;
      if (textcopy[forw - 1] == ' ') pos = forw;
      else if (textcopy[back - 1] == ' ') pos = back;
      else
      {
        done = FALSE;
        forw = fixmin (forw + 1, strlen (textcopy));
        back--;
      }
    }
    if (!done) pos = back;
    if (cancel_prtnum) /* call gks$set_text_fontprec(101,0) */;
    if (linen == 0)
    {
      if (write_win) XDrawString (disp, win, gc, flt2int (0.055, FALSE), flt2int (*ht, TRUE) + yadj, textcopy, strlen (textcopy));
      XDrawString (disp, pmap, gc, flt2int (0.055, FALSE), flt2int (*ht, TRUE) + yadj, textcopy, strlen (textcopy));
    }
    else
    {
      pos2 = pos;
      while (!cancel_prtnum && pos2 < strlen (textcopy) && textcopy[pos2] == ' ') pos2++;
      sprintf (textcopy2, "       %s", textcopy + pos2 - 1);
      if (write_win) XDrawString (disp, win, gc, flt2int (0.055, FALSE), flt2int (*ht - 0.015 * (float) linen, TRUE) + yadj,
        textcopy2, strlen (textcopy2));
      XDrawString (disp, pmap, gc, flt2int (0.055, FALSE), flt2int (*ht - 0.015 * (float) linen, TRUE) + yadj,
        textcopy2, strlen (textcopy2));
    }
    textcopy[pos - 1] = '\0';
  }
  if (prtnum && !cancel_prtnum)
  {
    if (write_win) XDrawString (disp, win, gc, flt2int (0.05, FALSE), flt2int (*ht, TRUE), num, 3);
    XDrawString (disp, pmap, gc, flt2int (0.05, FALSE), flt2int (*ht, TRUE), num, 3);
    *ht += 0.015;
  }
XSetFont(disp,gc,XLoadFont(disp,"-*-courier-bold-r-normal-*-12-*-75-75-*-*-iso8859-1"));
/* kka:
XSetFont(disp,gc,XLoadFont(disp,"-*-courier-bold-r-normal-*-13-*-75-75-*-*-iso8859-1"));
*/
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
gkstext: proc(num,prtnum,cancel_prtnum,text,ht);
dcl num char(2);
dcl (prtnum,cancel_prtnum) bit(1);
dcl text char(*) var;
dcl ht float;
dcl linen bin fixed(15);
dcl nlines bin fixed(15) init(1+divide(length(text),106,15));
dcl textcopy char(length(text)) var init(text);
dcl done bit(1);
dcl (forw,back,pos,pos2) bin fixed(15);
ht=ht+.015*nlines;
do linen=nlines-1 to 0 by -1;
	forw=95*linen+1;
	back=forw;
	pos=back;
	done=linen=0;
	do while(back>95*linen-25 & ^done);
		done='1'b;
		if substr(textcopy,forw,1)=' ' then pos=forw;
		else if substr(textcopy,back,1)=' ' then pos=back;
		else do;
			done='0'b;
			forw=min(forw+1,length(textcopy));
			back=back-1;
		end;
	end;
	if ^done then pos=back;
	if cancel_prtnum then call gks$set_text_fontprec(101,0);
	if linen=0 then call gks$text(.105,ht,textcopy);
	else do;
		pos2=pos;
		do while(^cancel_prtnum & pos2<length(textcopy) &
			index(substr(textcopy,pos2+1),' ')=1);
			pos2=pos2+1;
		end;
		call gks$text(.105,ht-.015*linen,
			'       '||substr(textcopy,pos2));
	end;
	call gks$set_text_fontprec(1,0);
	textcopy=substr(textcopy,1,pos-1);
end;
if prtnum & ^cancel_prtnum then do;
	if substr(num,1,1)='0' then substr(num,1,1)=' ';
	call gks$set_text_fontprec(101,0);
	call gks$text(.1,ht,num||'.');
	call gks$set_text_fontprec(1,0);
	ht=ht+.015;
end;
end;
#endif

void draw_line (CMPSTKST *stackp, float top, float bottom, int cmps_on_line, int num1, int num2, int last_on_line, int nconj,
  Boolean_t arrow, char *semicolon_list, Boolean_t top_line)
{
  float coor[4], arrowx[6], arrowy[5], width;
  int i, x, y;
  unsigned color;
  char numch[4], numch11[16], numlvl_str[32];
  Boolean_t avflag;
  XPoint points[5];

if (cmps_on_line == 0)
{
printf("Error in draw_line\n");
exit(1);
}
  width = 0.975 / (float) cmps_on_line;

  arrowx[0] = 0.;
  arrowx[1] = arrowx[3] = 0.045;
  arrowx[2] = arrowx[4] = 0.035;
  arrowx[5] = 0.055;
  arrowy[0] = arrowy[1] = arrowy[3] = 0.01;
  arrowy[2] = 0.;
  arrowy[4] = 0.02;

  if (nconj > 1) for (i = 1; i < 6; i++) arrowx[i] += width / 5.;

  coor[0] = (float) (cmps_on_line - last_on_line + num1 - 1) * width;
  coor[1] = coor[0] + width - (arrow ? arrowx[5] : 0.);
  coor[2] = bottom + 0.02;
  coor[3] = top;

  sprintf (numch, "%2d", num2 + 1);
  avflag = avail (stackp[0].inx);
  sprintf (numch11, "%5d", SymTab_Index_Get (stackp[0].inx));
  if (avflag)
  {
    strcat (numch11, " - av.");
    color = rgb[2];
  }
  else if (stackp[0].semicolon || !arrow) color = rgb[1];
  else color = BlackPixel (disp, DefaultScreen (disp));
while (numch11[0] == ' ') strcpy (numch11, numch11 + 1);

  drwcacm (coor, stackp[0].cmolp, numch11, color, FALSE, stackp[0].start_of_princ_path, top_line);

  if (stackp[0].semicolon)
  {
/* Move this code to FOLLOW drawings, so that semicolon is not erased!  (Leave condition intact for "else".)
    if (semicolon_list[0] == '\0') sprintf (semicolon_list, "%d", SymTab_Index_Get (stackp[0].inx));
    else sprintf (semicolon_list + strlen (semicolon_list), ", %d", SymTab_Index_Get (stackp[0].inx));
    if (write_win)
    {
      XFillArc (disp, win, gc, flt2int (coor[1], FALSE) - 1, flt2int ((top + bottom) / 2. + 0.015, TRUE) - 2, 8, 8, 0, 360 * 64);
      XFillArc (disp, win, gc, flt2int (coor[1], FALSE) - 1, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 2, 8, 8, 0, 360 * 64);
      XDrawArc (disp, win, gc, flt2int (coor[1], FALSE) - 5, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
      XDrawArc (disp, win, gc, flt2int (coor[1], FALSE) - 6, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
      XDrawArc (disp, win, gc, flt2int (coor[1], FALSE) - 7, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
    }
    XFillArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 1, flt2int ((top + bottom) / 2. + 0.015, TRUE) - 2, 8, 8, 0, 360 * 64);
    XFillArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 1, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 2, 8, 8, 0, 360 * 64);
    XDrawArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 5, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
    XDrawArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 6, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
    XDrawArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 7, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
*/
  }
  else if (arrow)
  {
    for (i = 0; i < 5; i++)
    {
      arrowx[i] += coor[1];
      arrowy[i] += (top + bottom) / 2. - 0.005;
      points[i].x = flt2int (arrowx[i], FALSE);
      points[i].y = flt2int (arrowy[i], TRUE);
    }
/* Move this code to FOLLOW drawings, so that step number is not erased!
    if (write_win) XDrawLines (disp, win, gc, points, 5, CoordModeOrigin);
    XDrawLines (disp, pmap, gc, points, 5, CoordModeOrigin);
    x = flt2int ((arrowx[0] + arrowx[1]) / 2. - 0.015, FALSE);
    y = flt2int (arrowy[0] - 0.018, TRUE);
    if (write_win) XDrawString (disp, win, gc, x, y, numch, 2);
    XDrawString (disp, pmap, gc, x, y, numch, 2);
*/
    coor[0] = arrowx[0];
    coor[1] = arrowx[1];
    coor[2] = arrowy[4] + 0.015;
    if (nconj > 1)
    {
      avflag = avail (stackp[1].inx);
      sprintf (numch11, "%5d", SymTab_Index_Get (stackp[1].inx));
      if (avflag)
      {
        strcat (numch11, " - av.");
        color = rgb[2];
      }
      else if (stackp[1].backref) color = rgb[1];
      else color = BlackPixel (disp, DefaultScreen (disp));
while (numch11[0] == ' ') strcpy (numch11, numch11 + 1);

      drwcacm (coor, stackp[1].cmolp, numch11, color, TRUE, FALSE, top_line);
    }
    coor[2] = bottom + 0.02;
    coor[3] = arrowy[2];
    if (nconj > 2)
    {
      avflag = avail (stackp[2].inx);
      sprintf (numch11, "%5d", SymTab_Index_Get (stackp[2].inx));
      if (avflag)
      {
        strcat (numch11, " - av.");
        color = rgb[2];
      }
      else if (stackp[2].backref) color = rgb[1];
      else color = BlackPixel (disp, DefaultScreen (disp));
while (numch11[0] == ' ') strcpy (numch11, numch11 + 1);

      drwcacm (coor, stackp[2].cmolp, numch11, color, TRUE, FALSE, top_line);
    }
    if (write_win) XDrawLines (disp, win, gc, points, 5, CoordModeOrigin);
    XDrawLines (disp, pmap, gc, points, 5, CoordModeOrigin);
    x = flt2int ((arrowx[0] + arrowx[1]) / 2. - 0.015, FALSE);
    y = flt2int (arrowy[0] - 0.018, TRUE);
    if (nconj == 1)
    {
      if (write_win) XDrawString (disp, win, gc, x, y, numch, 2);
      XDrawString (disp, pmap, gc, x, y, numch, 2);
      if (stackp->level[1] < 1000000) sprintf (numlvl_str, "(lvl %d)", stackp->level[1]);
      else strcpy (numlvl_str, "(lvl ???)");
      if (write_win) XDrawString (disp, win, gc, x - 15, y + 20, numlvl_str, strlen (numlvl_str));
      XDrawString (disp, pmap, gc, x - 15, y + 20, numlvl_str, strlen (numlvl_str));
    }
    else
    {
      if (stackp->level[1] < 1000000) sprintf (numlvl_str, "%s (lvl %d)", numch, stackp->level[1]);
      else sprintf (numlvl_str, "%s (lvl ???)", numch);
      if (write_win) XDrawString (disp, win, gc, x - 35, y, numlvl_str, strlen (numlvl_str));
      XDrawString (disp, pmap, gc, x - 35, y, numlvl_str, strlen (numlvl_str));
    }
  }
  if (stackp[0].semicolon)
  {
    if (semicolon_list[0] == '\0') sprintf (semicolon_list, "%d", SymTab_Index_Get (stackp[0].inx));
    else sprintf (semicolon_list + strlen (semicolon_list), ", %d", SymTab_Index_Get (stackp[0].inx));
    if (write_win)
    {
      XFillArc (disp, win, gc, flt2int (coor[1], FALSE) - 11, flt2int ((top + bottom) / 2. + 0.015, TRUE) - 2, 8, 8, 0, 360 * 64);
      XFillArc (disp, win, gc, flt2int (coor[1], FALSE) - 11, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 2, 8, 8, 0, 360 * 64);
      XDrawArc (disp, win, gc, flt2int (coor[1], FALSE) - 15, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
      XDrawArc (disp, win, gc, flt2int (coor[1], FALSE) - 16, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
      XDrawArc (disp, win, gc, flt2int (coor[1], FALSE) - 17, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
    }
    XFillArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 11, flt2int ((top + bottom) / 2. + 0.015, TRUE) - 2, 8, 8, 0, 360 * 64);
    XFillArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 11, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 2, 8, 8, 0, 360 * 64);
    XDrawArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 15, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
    XDrawArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 16, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
    XDrawArc (disp, pmap, gc, flt2int (coor[1], FALSE) - 17, flt2int ((top + bottom) / 2. - 0.025, TRUE) - 3, 12, 12, 0, -90 * 64);
  }
}

void drwcacm (float *coor, Dsp_Molecule_t *dsp, char *txt, unsigned color, Boolean_t on_arrow, Boolean_t start_main, Boolean_t top)
{
  int x, y, x2, y2, width, height, i, temp, maxw, maxh, hshift, vshift, minx, xadj;
float wratio, hratio, factor;

  x = flt2int (coor[0], FALSE);
  y = flt2int (coor[2], TRUE);
  x2 = flt2int (coor[1], FALSE);
  y2 = flt2int (coor[3], TRUE);
  width = fixmin (500, x2 - x);
  height = fixmin (500, y - y2);
  xadj = on_arrow ? -10 : 5;

 if (!dsp->map_em)
 {
  dsp->map_em=TRUE;
  if ((dsp->molw >= dsp->molh) != (width >= height))
  {
    temp = dsp->molw;
    dsp->molw = dsp->molh;
    dsp->molh = temp;
    for (i = 0; i < dsp->natms; i++)
    {
      temp = dsp->atoms[i].x;
      dsp->atoms[i].x = dsp->atoms[i].y;
      dsp->atoms[i].y = temp;
    }
  }

  maxw = 9 * width / 10;
  maxh = 9 * (height - (start_main ? 25: 0)) / 10;

  if (dsp->molw > maxw || dsp->molh > maxh)
  {
    wratio = (float) maxw / (float) dsp->molw;
    hratio = (float) maxh / (float) dsp->molh;
    if (wratio < hratio) factor = wratio;
    else factor = hratio;
    dsp->molw = (int) ((float) dsp->molw * factor);
    dsp->molh = (int) ((float) dsp->molh * factor);
    for (i = 0; i < dsp->natms; i++)
    {
      dsp->atoms[i].x = (int) ((float) dsp->atoms[i].x * factor);
      dsp->atoms[i].y = (int) ((float) dsp->atoms[i].y * factor);
    }
  }

  hshift = (width - dsp->molw) / 2;
  vshift = (height - dsp->molh) / 2;

  dsp->molw += hshift;
  dsp->molh += vshift;
  for (i = 0; i < dsp->natms; i++)
  {
    dsp->atoms[i].x += hshift;
    dsp->atoms[i].y += vshift;
  }
 }

 for (i = 0, minx = dsp->molw; i < dsp->natms; i++) if (dsp->atoms[i].x < minx) minx = dsp->atoms[i].x;

  XSetForeground (disp, gc, WhitePixel (disp, DefaultScreen (disp)));
  XFillRectangle (disp, scratchpad, gc, 0, 0, 500, 500);
  XSetForeground (disp, gc, color);
  Mol_Draw_Either_Color (dsp, scratchpad, FALSE, color);

  if (on_arrow)
    transpCopyArea (disp, scratchpad, win, pmap, write_win, TRUE, gc, 0, 0, dsp->molw + 10, dsp->molh + 10 /*width, height */,
    x + xadj, y2);
  else
  {
    if (write_win) XCopyArea (disp, scratchpad, win, gc, 0, 0, dsp->molw + 10, dsp->molh + 10 /*width, height */, x + xadj,
      y2 /* + (start_main ? 25 : 0) */);
    XCopyArea (disp, scratchpad, pmap, gc, 0, 0, dsp->molw + 10, dsp->molh + 10 /*width, height */, x + xadj,
      y2 /* + (start_main ? 25 : 0) */);
  }
  if (write_win) XDrawString (disp, win, gc, x + minx +xadj + 2, y2 + dsp->molh + 25, txt, strlen (txt));
  XDrawString (disp, pmap, gc, x + minx +xadj + 2, y2 + dsp->molh + 25, txt, strlen (txt));
  if (start_main)
  {
    XSetForeground (disp, gc, rgb[0]);
    if (top)
    {
      if (write_win) XDrawLine (disp, win, gc, x + minx, y2, x + minx, y2 + 25);
      XDrawLine (disp, pmap, gc, x + minx, y2, x + minx, y2 + 25);
      if (write_win) XDrawLine (disp, win, gc, x + minx, y2, x + minx + 175, y2);
      XDrawLine (disp, pmap, gc, x + minx, y2, x + minx + 175, y2);
      if (write_win) XDrawString (disp, win, gc, x + minx + xadj + 2, y2 + 17, "Principal pathway", 17);
      XDrawString (disp, pmap, gc, x + minx + xadj + 2, y2 + 17, "Principal pathway", 17);
    }
    else
    {
      if (write_win) XDrawLine (disp, win, gc, x + minx, y2 + dsp->molh + 25, x + minx, y2 + dsp->molh + 50);
      XDrawLine (disp, pmap, gc, x + minx, y2 + dsp->molh + 25, x + minx, y2 + dsp->molh + 50);
      if (write_win) XDrawLine (disp, win, gc, x + minx, y2 + dsp->molh + 50, x + minx + 175, y2 + dsp->molh + 50);
      XDrawLine (disp, pmap, gc, x + minx, y2 + dsp->molh + 50, x + minx + 175, y2 + dsp->molh + 50);
      if (write_win) XDrawString (disp, win, gc, x + minx + xadj + 2, y2 + dsp->molh + 47, "Principal pathway", 17);
      XDrawString (disp, pmap, gc, x + minx + xadj + 2, y2 + dsp->molh + 47, "Principal pathway", 17);
    }
  }
  XSetForeground (disp, gc, BlackPixel (disp, DefaultScreen (disp)));

  XFlush(disp);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
draw_line: proc(stackp,top,bottom,cmps_on_line,num,last_on_line,nconj,arrow);
dcl stackp ptr;
dcl (top,bottom) float;
dcl (cmps_on_line,num,last_on_line,nconj) bin fixed(15);
dcl arrow bit(1);
dcl coor(4) float init(0.,1.,bottom+.02,top);
dcl arrowx(6) float init(0.,.045,.035,.045,.035,.055);
dcl arrowy(5) float init(.01,.01,0.,.01,.02);
dcl width float init(.975/float(cmps_on_line,24));
dcl i bin fixed(15);
dcl numpic pic'99' init(num);
dcl numch char(2);
dcl numch5 char(5);
dcl avflag bit(1);
numch=numpic;
if numpic<10 then substr(numch,1,1)=' ';
coor(1)=float(cmps_on_line-last_on_line+num-1,24)*width;
if nconj>1 then do i=2 to 6;
	arrowx(i)=arrowx(i)+width/5.;
end;
coor(2)=coor(1)+width-arrowx(6)*float(arrow,24);
avflag=avail(stackp->inx(1));
put string(numch5) edit(stackp->inx(1))(f(5));
if avflag then call drwcacm(coor,stackp->cmolp(1),numch5||' - av.');
else call drwcacm(coor,stackp->cmolp(1),numch5);
if arrow then do;
	do i=1 to 5;
		arrowx(i)=arrowx(i)+coor(2);
		arrowy(i)=arrowy(i)+(top+bottom)/2.-.005;
	end;
	call gks$polyline(5,arrowx,arrowy);
	call gks$set_text_fontprec(101,0);
	call gks$text((arrowx(1)+arrowx(2))/2.-.015,arrowy(1)-.018,numch);
	call gks$set_text_fontprec(1,0);
	coor(1)=arrowx(1);
	coor(2)=arrowx(2);
	coor(3)=arrowy(5)+.015;
	if nconj>1 then do;
		avflag=avail(stackp->inx(2));
		put string(numch5) edit(stackp->inx(2))(f(5));
		if avflag then call drwcacm(coor,stackp->cmolp(2),numch5||
			' - av.');
		else call drwcacm(coor,stackp->cmolp(2),numch5);
	end;
	coor(3)=bottom+.02;
	coor(4)=arrowy(3);
	if nconj>2 then do;
		avflag=avail(stackp->inx(3));
		put string(numch5) edit(stackp->inx(3))(f(5));
		if avflag then call drwcacm(coor,stackp->cmolp(3),numch5||
			' - av.');
		else call drwcacm(coor,stackp->cmolp(3),numch5);
	end;
end;
end;

prtscrn: proc(border,landscape);
dcl (border,landscape) bit(1);
DCL SCREENFILE STREAM;
DCL QUIT BIT(1);
DCL P PTR init(pscreen());
DCL 1 VIDATA BASED(P),
	2 HEIGHT BIN FIXED(15),
	2 WIDTH BIN FIXED(15),
	2 BUFP PTR;
DCL BUFF(HEIGHT) CHAR(WIDTH) BASED(BUFP);
DCL (I,J,K,L) BIN FIXED(15);
DCL LEGEND CHAR(80) VAR;
dcl button bin fixed(15);
call gks$set_text_fontprec(101,0);
call gks$text(.003,.1,'Please');
call gks$text(.003,.085,'wait...');
OPEN FILE(SCREENFILE) OUTPUT title('syn_workfiles:ptrcscreen.lis');
BEGIN;
	DCL BCOPY CHAR(WIDTH+75) VAR;
	PUT FILE(SCREENFILE) EDIT('õ?2',landscape,' Jê0;0;',
		5+landscape,'q"1;1',copy('-',8))(a,b,A,f(1),2 a);
		/* escape sequence for DEC private print format;
		sequence to fit copy on 8.5x11" page */
	DO I=1 TO HEIGHT;
		if (i<6 | i>height-2) & ^border then bcopy='-';
		else do;
			if ^border then bcopy=copy('?',84)||substr(buff(i),10,
				width-20)||'-';
			else BCOPY=copy('?',75)||BUFF(I);
		end;
		if bcopy=copy('?',length(bcopy)-1)||'-' then bcopy='-';
		do while(substr(bcopy,length(bcopy)-1,1)='?');
			bcopy=substr(bcopy,1,length(bcopy)-2)||'-';
		end;
		CALL FIX(BCOPY);
		L=LENGTH(BCOPY);
		DO J=1 TO L BY 128;
			IF J+128>L THEN PUT FILE(SCREENFILE) SKIP EDIT
				(SUBSTR(BCOPY,J))(A);
			ELSE PUT FILE(SCREENFILE) SKIP EDIT(SUBSTR(BCOPY,J,128))
				(A);
		END;
	END;
END;
PUT FILE(SCREENFILE) SKIP EDIT('ú')(A);
call gks$set_text_color_index(0);
call gks$text(.003,.1,'Please');
call gks$text(.003,.085,'wait...');
call gks$set_text_color_index(1);
/*******************************************************************************
   QUIT='0'B;
   DO I=1 TO 3 WHILE(^QUIT);
	PUT SKIP EDIT('Enter line ',I,' of legend (<=80 chars) for this screen',
		'(SPACE for blank line; NULL line to end)')(A,F(1),A,SKIP,A);
	PUT SKIP;
	GET EDIT(LEGEND)(A(80));
	IF LENGTH(LEGEND)=0 THEN QUIT='1'B;
	ELSE PUT FILE(SCREENFILE) SKIP EDIT(LEGEND)(A);
   END;
*******************************************************************************/
PUT FILE(SCREENFILE) EDIT('õ0 J')(A);
	/* restore previous print format */
close file(screenfile);
call gks$text(.003,.125,'Press any');
call gks$text(.003,.11,'mouse');
call gks$text(.003,.095,'button');
call gks$text(.003,.08,'when');
call gks$text(.003,.065,'printer');
call gks$text(.003,.05,'is READY.');
button=buttonn();
call gks$set_text_color_index(0);
call gks$text(.003,.125,'Press any');
call gks$text(.003,.11,'mouse');
call gks$text(.003,.095,'button');
call gks$text(.003,.08,'when');
call gks$text(.003,.065,'printer');
call gks$text(.003,.05,'is READY.');
call gks$set_text_color_index(1);
call gks$set_text_fontprec(1,0);
call xdcl('print syn_workfiles:ptrcscreen');

FIX: PROC(B);
DCL B CHAR(*) VAR;
DCL BB CHAR(LENGTH(B)) VAR;
DCL BBB CHAR(1);
DCL (I,J) BIN FIXED(15) INIT(1);
DO WHILE(I<LENGTH(B));
	BB=SUBSTR(B,I);
	BBB=SUBSTR(BB,1,1);
	J=VERIFY(BB,BBB);
	IF J>4 THEN DO;
		B=SUBSTR(B,1,I-1)||NUM(J-1)||BBB;
		I=LENGTH(B)+1;
		B=B||SUBSTR(BB,J);
	END;
	ELSE I=I+1;
END;

NUM: PROC(N) RETURNS(CHAR(5) VAR);
DCL N BIN FIXED(15);
DCL P PIC'9999' INIT(N);
DCL C CHAR(4) VAR;
C=P;
DO WHILE(SUBSTR(C,1,1)='0');
	C=SUBSTR(C,2);
END;
RETURN('!'||C);
END;
END;

end;

END PATHTRC;
#endif

int getnum (CHARVAR str)
{
  int i, trunc;
  char *sp;

  while (str.string[0] == ' ' && str.len != 0)
  {
    str.len--;
    strncpy (str.string, str.string + 1, str.len);
    str.string[str.len] = '\0';
  }
  sscanf (str.string, "%d", &i);
  sp = strstr (str.string, " ");
  if (sp == NULL) sp = str.string + str.len;
  trunc = sp - str.string;
  strcpy (str.string, sp);
  str.len -= trunc;

  return (i);
}

void clear_window ()
{
  if (write_win) XClearWindow (disp, win);
  XSetForeground (disp, gc, WhitePixel (disp, DefaultScreen (disp)));
  XFillRectangle (disp, pmap, gc, 0, 0, 1100, 850);
  XSetForeground (disp, gc, BlackPixel (disp, DefaultScreen (disp)));
}

void sum_avl ()
{
  int i, j, k, l, y, inx, key, numavl, avlinx[100][2];
  char avlinfo[200], *avi, catnum[64], cmpname[128];
  Sling_t sling;
  Boolean_t found, changed;
  Avi_CmpInfo_t *acip;

  clear_window ();
  for (i = 0; i < 100; i++) avlinx[i][0] = avlinx[i][1] = -1;
  for (i = numavl = 0; i < ncmol; i++) for (j = 0; j < nconj[i]; j++) if (SymTab_Flags_Available_Get (cmolpstack[i][j].inx))
  {
    inx = SymTab_Index_Get (cmolpstack[i][j].inx);
    sling = Sling_CopyTrunc (SymTab_Sling_Get (cmolpstack[i][j].inx));
    key = AvcLib_Key (Sling_Name_Get (sling));
    Sling_Destroy (sling);
    for (k = 0, found = FALSE; k < numavl && !found; k++) if (avlinx[k][0] == inx) found = TRUE;
    if (!found)
    {
      avlinx[numavl][0] = inx;
      avlinx[numavl++][1] = key;
    }
  }
  for (i = 0, changed = TRUE; i < numavl - 1 && changed; i++) for (j = numavl - 1, changed = FALSE; j > i; j--)
    if (avlinx[j][0] < avlinx[j - 1][0])
  {
    changed = TRUE;
    k = avlinx[j][0];
    l = avlinx[j][1];
    avlinx[j][0] = avlinx[j - 1][0];
    avlinx[j][1] = avlinx[j - 1][1];
    avlinx[j - 1][0] = k;
    avlinx[j - 1][1] = l;
  }
  for (i = 0; i < numavl - 1; i++) if (avlinx[i][0] == avlinx[i + 1][0])
  {
    numavl--;
    for (j = i; j < numavl; j++)
    {
      avlinx[j][0] = avlinx[j + 1][0];
      avlinx[j][1] = avlinx[j + 1][1];
    }
  }
  if (numavl == 0)
  {
    sprintf (avlinfo, "No available compounds appear in this synthesis.");
    if (write_win) XDrawString(disp,win,gc,10,25*(i+1),avlinfo,strlen(avlinfo));
    XDrawString(disp,pmap,gc,10,25*(i+1),avlinfo,strlen(avlinfo));
    XFlush (disp);
    return;
  }

  strcpy (avlinfo, "______ __ _________ _________");
  if (write_win) XDrawString(disp,win,gc,10,27,avlinfo,strlen(avlinfo));
  XDrawString(disp,pmap,gc,10,27,avlinfo,strlen(avlinfo));
  strcpy (avlinfo, "LEGEND OF AVAILABLE COMPOUNDS");
  if (write_win) XDrawString(disp,win,gc,10,24,avlinfo,strlen(avlinfo));
  XDrawString(disp,pmap,gc,10,24,avlinfo,strlen(avlinfo));
  for (i=0; i<numavl; i++) if (avlinx[i][1] == 0)
  {
    sprintf (avlinfo, "Compound %6d: <ERROR - Not found in AvlComp library!>", avlinx[i][0]);
    if (write_win) XDrawString(disp,win,gc,10,25*(i+3),avlinfo,strlen(avlinfo));
    XDrawString(disp,pmap,gc,10,25*(i+3),avlinfo,strlen(avlinfo));
  }
  else
  {
acip=AvcLib_Info (avlinx[i][1]);
sprintf(avlinfo,"Compound %6d: ",avlinx[i][0]);
avi = avlinfo + strlen (avlinfo);
if ((l = Avi_CmpInfo_NameLen_Get (acip)) != 0)
{
  strncpy (avi, Avi_CmpInfo_Name_Get (acip), l);
  avi[l]='\0';
}
else strcpy (avi, "<No name stored>");
strcpy (cmpname, avi);
strcat (avlinfo, " (");
l=Avi_CmpInfo_CatLen_Get (acip);
strncpy (catnum, Avi_CmpInfo_Catalog_Get (acip), l);
catnum[l]='\0';
if (l == 0 || (strncmp (catnum, "Aldrich#Y0", 10) == 0 && cmpname[0] != 'Y'))
  strcpy (catnum, "Postulated as available - source unspecified");
avi = avlinfo + strlen (avlinfo);
sprintf (avi, "%s)", catnum);
/*
    sprintf (avlinfo, "Compound %d key %d",avlinx[i][0],avlinx[i][1]);
*/
    if (write_win) XDrawString(disp,win,gc,10,25*(i+3),avlinfo,strlen(avlinfo));
    XDrawString(disp,pmap,gc,10,25*(i+3),avlinfo,strlen(avlinfo));
  }
  XFlush (disp);
}

void transpCopyArea (Display *disp, Drawable src, Drawable dest1, Drawable dest2, Boolean_t d1ok, Boolean_t d2ok, GC gc, int sx, int sy, unsigned w, unsigned h, int dx, int dy)
{
  XImage *img;
  int xinc, yinc;
  U32_t pixel;

  img=XGetImage (disp, src, sx, sy, w, h, AllPlanes, ZPixmap);
  for (yinc = 0; yinc < h; yinc++) for (xinc = 0; xinc < w; xinc++)
  {
    pixel = XGetPixel (img, sx + xinc, sy + yinc);
    if (pixel != WhitePixel (disp, DefaultScreen (disp)))
    {
      if (d1ok) XDrawPoint (disp, dest1, gc, dx + xinc, dy + yinc);
      if (d2ok) XDrawPoint (disp, dest2, gc, dx + xinc, dy + yinc);
    }
  }
  XDestroyImage (img);
}

void initialize_static_vars ()
{
  first_dump_stacks = TRUE;
  write_win = FALSE;
  rootcomp = mainroot = NULL;
  head = tail = last = slptr = NULL;
  indx = NULL;
  level = minlevel = ncmol = ntp = dc_lvl = 0;
  target_drawing = NULL;
}

void get_levels (int start_of_main)
{
  int i, j, k, lvl;
  Boolean_t found;
  CMPSTKST *stackp;

  for (i = ncmol - 1, lvl = 0; i >= start_of_main; i--, lvl++)
  {
    stackp = (CMPSTKST *) memptr ("CMOLPSTACK", 0, i);
    for (j = 0; j < nconj[i]; j++) stackp[j].level[1] = lvl;
  }
  while (i >= 0)
  {
    stackp = memptr ("CMOLPSTACK", 0, i);
    for (j = i + 1, found = FALSE; j < ncmol && !found; j++) for (k = 1; k < nconj[j] && !found; k++)
      if (cmolpstack[j][k].backref && cmolpstack[j][k].inx == stackp->inx)
    {
      found = TRUE;
      stackp->level[1] = cmolpstack[j][k].level[1];
      lvl = stackp->level[0] + stackp->level[1];
    }
    if (!found) lvl = stackp->level[1] = 1111111;
    do
    {
      stackp = memptr ("CMOLPSTACK", 0, i);
      for (j = 0; j < nconj[i]; j++) stackp[j].level[1] = lvl - stackp[j].level[0];
      i--;
    }
    while (i >= 0 && !cmolpstack[i][0].semicolon);
  }
}
