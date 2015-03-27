/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PATHTRC.C
*
*    This module reads in the status and path files and builds a meaningful
*    path context as a prototype for a C/Motif implementation of PathTrc.
*
*  Creation Date:
*
*       28-Sep-2000
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_TIMER_
#include "timer.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_SYNHELP_
#include "synhelp.h"
#endif

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

static Array_t compptr_array;
static Display *disp;
static Widget draww, prevpb, nextpb, exitpb, helppb, explainpb;
static Pixmap drawp, scratchpad;
static GC gc;
static int page = 0, npgs = 0, ncptrs[100], nncp, final_ncptrs;
static Compound_t *cptrs[100][1000], *final_cptrs[1000];
static PstCB_t *glob_pstcb_p;

#define PathComp_Ptr_Get(node) (Compound_t *) Array_1d32_Get (&compptr_array, (node))
#define PathComp_Ptr_Put(node,ptr) Array_1d32_Put (&compptr_array, (node), (ptr))

typedef struct CMPSTK
  {
  SymTab_t *inx;
  Dsp_Molecule_t *cmolp;
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

static CHARVAR paths_str;
static COMPOUND *cpparm;
static STRINGLIST *slhparm, *sltparm;

/* functions from ptrc_func.c */
void putptrc (CHARVAR, COMPOUND *, STRINGLIST *, STRINGLIST *, Boolean_t, PstCB_t *);
void input_ptrc (Compound_t **, int, COMPOUND **, STRINGLIST **, STRINGLIST **);
int dump_stacks (int, Boolean_t, Display *, Window, Pixmap, Pixmap, GC);

/* local functions and callbacks */
void doit (int, char *[]);
Boolean_t must_switch (int, int);
void Path_Pst_Serialize (PstNode_t);
void redraw_path (Widget, XtPointer, XtPointer);
void update_page (Widget, XtPointer, XtPointer);
void exit_ptrc (Widget, XtPointer, XtPointer);
void label_next_pb (int, int);
Boolean_t overlapped (int, int);
void dump_paths (char *, int *map, Boolean_t, Boolean_t);


/****************************************************************************
*
*  Function Name:                 main
*
*    Read in the status file and execute scans.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
main
  (
  int   argc,
  char  *argv[]
  )
{
  Widget         frame, topform;
  XtAppContext   schlcontext;
  ScreenAttr_t  *sca_p;
  XmFontList flhv14,flhv18,flco18,fl6x10;
  XmFontListEntry helv14,helv18,cour18,fs6x10;
  XmString label;
  int i, j;

  doit (argc, argv);
  input_ptrc (final_cptrs, final_ncptrs, &cpparm, &slhparm, &sltparm);
  putptrc (paths_str, cpparm, slhparm, sltparm, TRUE, glob_pstcb_p);
  toplevel = XtVaAppInitialize (&schlcontext, "PathTrc", NULL, 0, &argc, argv, NULL, NULL);

  SynAppR_PreInit ((Widget) toplevel, SAR_APPSIZE_DFLT_WD);
  SynAppR_PostInit ((Widget) toplevel);

  helv18 = XmFontListEntryLoad (XtDisplay ((Widget) toplevel), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  cour18 = XmFontListEntryLoad (XtDisplay ((Widget) toplevel), "-*-courier-medium-r-normal-*-18-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  fs6x10 = XmFontListEntryLoad (XtDisplay ((Widget) toplevel), "-*-courier-medium-r-normal-*-24-*-75-75-*-*-iso8859-1",
    XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
  flhv18 = XmFontListAppendEntry (NULL, helv18);
  flco18 = XmFontListAppendEntry (NULL, cour18);
  fl6x10 = XmFontListAppendEntry (NULL, fs6x10);

  XtVaSetValues ((Widget) toplevel,
    XmNbackground,     SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,     SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNlabelFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,   /* SynAppR_FontList_Get (&GSynAppR), */ fl6x10,
    XmNheight,         900 /* AppDim_AppHeight_Get (&GAppDim) */,
    XmNwidth,          1105 /* AppDim_AppWidth_Get (&GAppDim) */,
    XmNresizePolicy,   XmRESIZE_NONE,
    XmNresizable,      False,
    XmNautoUnmanage,   False,
    XmNtitle,          "PATHTRC",
    XmNx,              0,
    XmNy,              0,
    NULL);

  XtRealizeWidget ((Widget) toplevel);

  InfoWarn_Create ((Widget) toplevel);
  Help_Form_Create ((Widget) toplevel);

  frame = XtVaCreateManagedWidget ("PathTrcFr",
    xmMainWindowWidgetClass,  (Widget) toplevel,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    NULL);

  topform = XtVaCreateWidget ("form", xmFormWidgetClass, frame,
    XmNfractionBase, 800,
    NULL);

  draww = XtVaCreateManagedWidget ("SyntheticPath",
    xmDrawingAreaWidgetClass, topform,
    XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK),
    XmNheight,            850, /* PDRW_DA_PXMP_HT, */
    XmNwidth,             1100, /* PDRW_DA_PXMP_WD, */
    XmNfontList, fl6x10,
    XmNresize, False,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  XtAddCallback (draww, XmNexposeCallback, redraw_path, (XtPointer) NULL);

  label = XmStringCreateLocalized ("Explain Values");

  explainpb = XtVaCreateManagedWidget ("SynPathPrev",
    xmPushButtonWidgetClass, topform,
    XmNlabelString, label,
    XmNtopOffset, 10,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, draww,
    XmNleftPosition, 150,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNrightPosition, 250,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XmStringFree (label);

  XtAddCallback (explainpb, XmNactivateCallback, Help_CB, (XtPointer) "ptrc_exp:Explanation of Synthesis Parametric Values --");

  label = XmStringCreateLocalized ("Help");

  helppb = XtVaCreateManagedWidget ("SynPathPrev",
    xmPushButtonWidgetClass, topform,
    XmNlabelString, label,
    XmNtopOffset, 10,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, draww,
    XmNleftPosition, 580,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNrightPosition, 620,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XmStringFree (label);

  XtAddCallback (helppb, XmNactivateCallback, Help_CB, (XtPointer) "pathtrc:PathTrc Synthetic Path Rendition");

  label = XmStringCreateLocalized ("<- Previous Page");

  prevpb = XtVaCreateManagedWidget ("SynPathPrev",
    xmPushButtonWidgetClass, topform,
    XmNlabelString, label,
    XmNtopOffset, 10,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, draww,
    XmNleftOffset, 20,
    XmNleftAttachment, XmATTACH_FORM,
    NULL);

  XmStringFree (label);

  XtAddCallback (prevpb, XmNactivateCallback, update_page, (XtPointer) -1);

  label = XmStringCreateLocalized ("Available Compounds"); /* Initialize to larger string to set width */

  nextpb = XtVaCreateManagedWidget ("SynPathNext",
    xmPushButtonWidgetClass, topform,
    XmNlabelString, label,
    XmNtopOffset, 10,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, draww,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 680,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightOffset, 20,
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XmStringFree (label);

  XtAddCallback (nextpb, XmNactivateCallback, update_page, (XtPointer) 1);

  label = XmStringCreateLocalized ("Dismiss");

  exitpb = XtVaCreateManagedWidget ("SynPathExit",
    xmPushButtonWidgetClass, topform,
    XmNlabelString, label,
    XmNtopOffset, 10,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, draww,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 350,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 450,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  XmStringFree (label);

  XtAddCallback (exitpb, XmNactivateCallback, exit_ptrc, (XtPointer) NULL);

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  disp = Screen_TDisplay_Get (sca_p);
  gc = SynAppR_MolGC_Get (&GSynAppR);
  XSetBackground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
  XSetFillStyle (disp, gc, FillSolid);
  XSetLineAttributes (disp, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));

  drawp = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
    1100 /* PDRW_DA_PXMP_WD */, 850 /* PDRW_DA_PXMP_HT */, Screen_Depth_Get (sca_p));

  scratchpad = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
    500 /* PDRW_DA_PXMP_WD */, 500 /* PDRW_DA_PXMP_HT */, Screen_Depth_Get (sca_p));

  XFillRectangle (disp, drawp, gc, 0, 0, 1100, 850 /*PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT */);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));

  XtUnmanageChild (prevpb);

paths_str.len = 1;
paths_str.string[0] = 'A';

  fprintf (stdout, "done.\n\n");

  Rcb_Destroy (&GGoalRcb);
  RunStats_Destroy (&GRunStats);
  IO_Close_Files ();

  XtManageChild(topform);

  XtAppMainLoop (schlcontext);
}

void doit (int argc, char *argv[])
{
  FILE          *pathf_p;
  char          statusfilename[128];
  char          pathfilename[128];
  SymTab_t      *symtab_p, *curr_sym;
  int            arg_i, ncmps, i, j, k, l, m, n, map[100], arrsize;
  U32_t          cmp, cindxs[1000];
  Compound_t    *cmp_p, *prev_cmp, *curr_cmp;
  Subgoal_t     *root_p, *prev_sg, *curr_sg;
  PstCB_t       *pstcb_p;
  PstNode_t      node;
  Boolean_t      changed;

  if (argc != 5)
    {
    fprintf (stderr, "\n1: pathtrc -f statusfile -p pathfile\n");
    exit (0);
    }

  statusfilename[0] = pathfilename[0] = '\0';
  arg_i = 1;
  while (arg_i < argc)
    { 
    if (*argv[arg_i] == '-')
      {
      if (*(argv[arg_i] + 1) == 'f') 
	{
	++arg_i;
	sprintf (statusfilename, "//D/SYNCHEM/testdata/status/%s.status", argv[arg_i]);
	}
      else if (*(argv[arg_i] + 1) == 'p') 
	{
	++arg_i;
	sprintf (pathfilename, "//D/SYNCHEM/testdata/pfile/%s.path", argv[arg_i]);
	}
      }

    ++arg_i;
    }

  if (statusfilename[0] == '\0' || pathfilename[0] == '\0')
    {
    fprintf (stderr, "\n2: pathtrc -f statusfile -p pathfile\n");
    exit (0);
    }

  /* Initialization:  

     - DEBUG
     - SYNIO
     - Tracing and debugging variables
     - Run control parameters from submission file and/or status file

  */

  Debug_Init ();
  Trace_Init ();
  IO_Init ();
Rcb_Init (&GGoalRcb, FALSE);
  React_Initialize ((U8_t *) (concat ("R+W", FCB_SEQDIR_RXNS (""))),
    Persist_Inx_OK (FCB_SEQDIR_RXNS ("/rkbstd.inx"), concat ("R+W", FCB_SEQDIR_RXNS (""))));
  Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);

  fprintf (stdout, "\nLoading status file %s ... ", statusfilename);
  Status_File_Read (statusfilename, &GGoalRcb, &GRunStats);
  fprintf (stdout, "done.\n\n");

  Rcb_ChemistryTrace_Put (&GGoalRcb, TL_NONE);
  GTrace[DB_CHEMISTRY].options = TL_NONE;
#ifdef _WINNT_OS_
  IO_Init_Files ("NUL:", FALSE);
#else
#ifdef _CYGWIN_
  IO_Init_Files ("NUL:", FALSE);
#else
  IO_Init_Files ("/dev/null", FALSE);
#endif
#endif
/*
  AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_EXISTS);
  AvcLib_Init ((char *) String_Value_Get (FileCB_DirStr_Get ( 
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_AVLC))), AVC_INIT_INFO);
*/
  AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_EXISTS);
  AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_INFO);
/**************** May be needed later for PathTrc *****************
  FuncGroups_Init (String_Value_Get (FileCB_DirStr_Get ( 
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_FNGP))));

******************************************************************/

#ifdef _WIN32
  pathf_p = fopen (gccfix (pathfilename), "r");
#else
  pathf_p = fopen (pathfilename, "r");
#endif

  if (pathf_p == NULL)
    {
    fprintf (stderr, "\npathtrc:  unable to open %s\n", pathfilename);
    fprintf (stderr, "\npathtrc -f statusfile -p pathfile\n");
    exit (0);
    }

  if (getc(pathf_p) != '0')
  {
    fprintf (stderr, "\007Warning: Not PathTrc-friendly - results unpredictable.\n");
    fclose (pathf_p);
#ifdef _WIN32
    pathf_p = fopen (gccfix (pathfilename), "r");
#else
    pathf_p = fopen (pathfilename, "r");
#endif
  }

  glob_pstcb_p = pstcb_p = Pst_ControlHandle_Get ();
  arrsize = PstCB_CompoundIndex_Get (pstcb_p);

  fprintf (stdout, "Scanning Path  ... ");

  for (ncmps = 0; fscanf (pathf_p, "%d", &cmp) == 1; ncmps++)
  {
    if (cmp >= arrsize)
    {
      fprintf (stderr, "\007Invalid path (exceeds maximum node number).\n");
      exit (1);
    }
    cindxs[ncmps] = cmp;
  }
  fclose (pathf_p);

  if (ncmps < 2)
  {
    fprintf (stderr, "\007Invalid path (<2 nodes).\n");
    exit (1);
  }

/*
  glob_pstcb_p = pstcb_p = Pst_ControlHandle_Get ();
*/

  root_p = PstCB_Root_Get (pstcb_p);

/*
  Array_1d_Create (&compptr_array, PstCB_CompoundIndex_Get (pstcb_p), ADDRSIZE);
*/
  Array_1d_Create (&compptr_array, arrsize, ADDRSIZE);
  Array_Set (&compptr_array, 0);
  PstNode_Subgoal_Put (&node, root_p);
  Path_Pst_Serialize (node);

for (i=0, nncp=0, prev_sg=NULL, prev_cmp = NULL; i<ncmps; i++) if (cindxs[i]!=1)
  {
  cmp_p=PathComp_Ptr_Get(cindxs[i]);
  if (prev_cmp == NULL) curr_cmp = PathComp_Ptr_Get (1); /* Can't use NULL, because not every compound is necessarily developed --
                                                            Therefore, we use this hack, for lack of anything better! */
  else
    {
    curr_sg = PstComp_Father_Get(prev_cmp);
    curr_cmp = PstSubg_Father_Get(curr_sg);
    }
  if (PstComp_Father_Get(cmp_p) != prev_sg)
    {
    if (curr_cmp != SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (cmp_p))) ncptrs[nncp++] = 0;
    prev_sg=PstComp_Father_Get(cmp_p);
    cptrs[nncp - 1][ncptrs[nncp - 1]++]=cmp_p;
    }
  prev_cmp = cmp_p;
  }

dump_paths ("After initial path construction", NULL, FALSE, FALSE);

/* bypass additional processing if only one path */

  if (nncp == 1)
  {
    final_ncptrs = ncptrs[0];
    for (i = 0; i < final_ncptrs; i++) final_cptrs[i] = cptrs[0][i];
dump_paths ("Special case of only main path", NULL, FALSE, TRUE);
    return;
  }

/* complete each path from other paths */

  for (i = 0; i < nncp; i++)
    for (j = 0; j < nncp && cptrs[i][ncptrs[i] - 1] != PathComp_Ptr_Get (1); j++)
    if (j != i)
  {
    curr_sg = PstComp_Father_Get (cptrs[i][ncptrs[i] - 1]);
    curr_cmp = PstSubg_Son_Get (curr_sg);
    while (curr_cmp != NULL && cptrs[i][ncptrs[i] - 1] != PathComp_Ptr_Get (1))
    {
      if (curr_cmp != cptrs[i][ncptrs[i] - 1])
      {
        k = ncptrs[j] - 1;
        if (cptrs[j][k] != curr_cmp)
          for (k--; k >= 0; k--)
          if (cptrs[j][k] == curr_cmp)
          for (l = k + 1; l < ncptrs[j]; l++) cptrs[i][ncptrs[i]++] = cptrs[j][l];
      }
      curr_cmp = PstComp_Brother_Get (curr_cmp);
    }
  }

dump_paths ("After completion from other paths", NULL, FALSE, FALSE);

/* sort paths in order of increasing length */

  for (i = 0; i < nncp; i++) map[i] = i;
  for (i = 0, changed = TRUE; i < nncp - 1 && changed; i++)
  {
    for (j = nncp - 1, changed = FALSE; j > i; j--) if (must_switch (map[i], map[j]))
    {
      changed = TRUE;
      m = map[i];
      map[i] = map[j];
      map[j] = m;
    }
  }

dump_paths ("After sort", map, TRUE, FALSE);

/* truncate remaining paths successively */

  for (n = nncp - 1; n > 0; n--)
  {
    for (i = 0; i < n; i++)
    {
      for (j = ncptrs[map[n]] - 1;
        j >= 0 && ncptrs[map[i]] > 1 &&
        /* compare symbol-table pointers, rather than instance pointers, to catch cases where the same intermediate
           (along with its synthesis) appears as more than one instance in the path (e.g., phenylglyoxylyl chloride
           in 5,12-bis(phenylethynyl)naphthacene synthesis)
        */
        PstComp_SymbolTable_Get (cptrs[map[n]][j]) == PstComp_SymbolTable_Get (cptrs[map[i]][ncptrs[map[i]] - 1]);
        j--)
        ncptrs[map[i]]--;
    }
  }

dump_paths ("After successive truncation of shorter paths", map, TRUE, FALSE);

/* empty out any path that overlaps a longer one */

  for (n = nncp - 1; n > 0; n--)
    for (i = 0; i < n; i++) if (overlapped (map[n], map[i])) ncptrs[map[i]] = 0;

dump_paths ("After emptying out redundant paths", map, TRUE, FALSE);

/* eliminate any paths truncated to 1 */

  for (i = 0; i < nncp; i++) if (ncptrs[map[i]] < 2)
  {
    for (j = i + 1; j < nncp; j++) map[j - 1] = map[j];
    nncp--;
  }

dump_paths ("After pruning", map, TRUE, FALSE);

/* sort paths again in order of increasing length */

  for (i = 0, changed = TRUE; i < nncp - 1 && changed; i++)
  {
    for (j = nncp - 1, changed = FALSE; j > i; j--) if (must_switch (map[i], map[j]))
    {
      changed = TRUE;
      m = map[i];
      map[i] = map[j];
      map[j] = m;
    }
  }

dump_paths ("After resort", map, TRUE, FALSE);

/* FINALLY! Create a single path containing NULL separators where the semicolons will go! */

  for (i = final_ncptrs = 0; i < nncp; i++)
  {
    for (j = 0; j < ncptrs[map[i]]; j++) final_cptrs[final_ncptrs++] = cptrs[map[i]][j];
    if (i < nncp - 1) final_cptrs[final_ncptrs++] = NULL;
  }
dump_paths ("Merged", NULL, FALSE, TRUE);
}

/****************** must_switch ******************************
 Tells whether two paths are in descending order by length or,
 if both are the same length, whether the first begins with
 the "more complex" sling (by length).
*************************************************************/
Boolean_t must_switch (int i, int j)
{
  Sling_t isling, jsling;
  int ilen, jlen;

  if (ncptrs[i] < ncptrs[j]) return (FALSE);
  if (ncptrs[i] > ncptrs[j]) return (TRUE);
  isling = Sling_CopyTrunc (SymTab_Sling_Get (PstComp_SymbolTable_Get (cptrs[i][0])));
  jsling = Sling_CopyTrunc (SymTab_Sling_Get (PstComp_SymbolTable_Get (cptrs[j][0])));
  ilen = strlen (Sling_Name_Get (isling));
  jlen = strlen (Sling_Name_Get (jsling));
  Sling_Destroy (isling);
  Sling_Destroy (jsling);
  return (ilen > jlen);
}

/****************** Path_Pst_Serialize ***********************
 Borrowed from status.c, from which it is unavailable, having
 been declared static to that module.  Needed to make sense of
 the node indices saved in the pfile (.path) file format.
*************************************************************/
void Path_Pst_Serialize (PstNode_t node)
{
  Compound_t *comp_p;
  PstNode_t newnode;

  while (PstNode_Compound_Get (&node) != NULL)
  {
    newnode = Pst_Son_Get (&node);
    Path_Pst_Serialize (newnode);
    newnode = Pst_Brother_Get (&node);

    if (PstNode_Type_Get (&node) == PST_COMPOUND)
    {
      comp_p = PstNode_Compound_Get (&node);
      PathComp_Ptr_Put (PstComp_Index_Get (comp_p), comp_p);
    }
    node = newnode;
  }
}

void redraw_path (Widget w, XtPointer client_data, XtPointer call_data)
{
  XmDrawingAreaCallbackStruct *cbs;
static Boolean_t first = TRUE;

  cbs = (XmDrawingAreaCallbackStruct *) call_data;
if (first)
{
  npgs = dump_stacks (0, TRUE, disp, XtWindow (draww), drawp, scratchpad, gc);
  if (npgs != 2) XtUnmanageChild (explainpb);
/*
  if (npgs == 1) XtUnmanageChild (nextpb);
*/
label_next_pb (0, npgs);
first = FALSE;
}
  XCopyArea (cbs->event->xexpose.display,
    drawp, /* XtWindow (draww) */ cbs->window, gc,
    0, 0, 1100, 850, 0, 0);
  XFlush (cbs->event->xexpose.display);
}

void update_page (Widget w, XtPointer client_data, XtPointer call_data)
{
  int direction;

  direction = (int) client_data;
  page += direction;
  if (page == 0) XtUnmanageChild (prevpb);
  else XtManageChild (prevpb);
label_next_pb (page, npgs);
  if (page == npgs - 1) XtUnmanageChild (nextpb);
  else XtManageChild (nextpb);
  if (page == npgs - 2) XtManageChild (explainpb);
  else XtUnmanageChild (explainpb);
  dump_stacks (direction, TRUE, disp, XtWindow (draww), drawp, scratchpad, gc);
}

void exit_ptrc (Widget w, XtPointer client_data, XtPointer call_data)
{
  dump_stacks (npgs - page, TRUE, disp, XtWindow (draww), drawp, scratchpad, gc);
}

void label_next_pb (int pg, int npp)
{
  XmString label;

  label = XmStringCreateLocalized (pg == npp - 2 ? "Available Compounds" : "Next Page ->");
  XtVaSetValues (nextpb,
    XmNlabelString, label,
    NULL);
  XmStringFree (label);
}

Boolean_t overlapped (int larger, int smaller)
{
  int i, j;
  Boolean_t found;

  for (i = 0, found = FALSE; i <= ncptrs[larger] - ncptrs[smaller] && !found; i++)
  {
    for (j = 0, found = TRUE; j < ncptrs[smaller] && found; j++)
      if (PstComp_SymbolTable_Get (cptrs[larger][i + j]) !=
      PstComp_SymbolTable_Get (cptrs[smaller][j])) found = FALSE;
  }
  return (found);
}

void dump_paths (char *when, int *map, Boolean_t sorted, Boolean_t merged)
{
  int i, j, order_i;

  printf ("\n%s:\n", when);
  if (merged)
  {
    putchar ('\t');
    for (i = 0; i < final_ncptrs; i++) if (final_cptrs[i] == NULL) printf (" NULL");
    else printf (" %d", SymTab_Index_Get (PstComp_SymbolTable_Get (final_cptrs[i])));
    putchar ('\n');
  }
  else for (i = 0; i < nncp; i++)
  {
    order_i = sorted ? map[i] : i;
    printf ("%d)\t", i + 1);
    for (j = 0; j < ncptrs[order_i]; j++) printf (" %d", SymTab_Index_Get (PstComp_SymbolTable_Get (cptrs[order_i][j])));
    putchar ('\n');
  }
}

/* End of PATHTRC.C */
