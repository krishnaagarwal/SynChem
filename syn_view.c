/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SYN_VIEW.C
*
*    This is the main executive of the SYNCHEM system. 
*
*  Creation Date:
*
*    10-Jun-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    main
*    PstView_Handle_Grab
*    SShotView_Handle_Grab
*    SynView_Handle_Grab
*    SynView_Exit
*
*    SView_Create
*
*    SSearch_Dismiss_CB
*    SSearch_Halt_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/FileSB.h>
#include <Xm/MessageB.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

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

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_STRATEGY_
#include "strategy.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_SEARCH_GUI_
#include "search_gui.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_RXN_PREVIEW_
#include "rxn_preview.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_CMP_INST_
#include "cmp_inst.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SV_FILES_
#include "sv_files.h"
#endif

#ifndef _H_SSHOT_VIEW_
#include "sshot_view.h"
#endif

#ifndef _H_SUBMIT_
#include "submit.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_SYN_MENU_
#include "syn_menu.h"
#endif

#ifndef _H_SYNHELP_
#include "synhelp.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

/* Static Variables */

static SynViewCB_t SViewCB;

static char  *synview_usage = {"\n"
  "Usage:  synchem [-help] [-half -full -mini] [-distrib]\n"
  "                [-printer <p>] [-display <d>]\n"
  "        -help        help with options\n"
  "        -half        scale with width set to half screen size\n"
  "        -full        set width to full screen size\n"
  "        -mini        scale to 500x600 (web page snapshot)\n"
  "        -distrib     use distributed output data directories\n"
  "        -printer <p> use the printer <p>\n"
  "        -display <d> use the remote display <d>\n"
  "The default is to set the width to three quarters screen size.\n\n"};

/* Static Routine Prototypes */

static Widget SView_Create           (SynViewCB_t *, Widget);

static void   SSearch_Dismiss_CB     (Widget, XtPointer, XtPointer);
static void   SSearch_Halt_CB        (Widget, XtPointer, XtPointer);

static char glob_path[256];


/****************************************************************************
*
*  Function Name:                 main
*
*    This routine.
*
*
*  Implicit Inputs:
*
*    SViewCB, the SynView control block.
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
void main
  (
  int   argc,
  char *argv[]
  )
{
  Widget         topform;
  Pixmap         icon_pmap;
  char         **my_args;                    /* find my arguments */
  U16_t          arg_i;                      /* i_th argument */
  U8_t           app_size;
  char          *icon_file;
#ifdef _CYGWIN_
char *synexe;
#endif

  icon_file = SAR_DIR_BITMAPS (SAR_ICON_FNAME_SYNVIEW);

#ifdef _CYGWIN_
synexe = getenv ("SYNEXE");
sprintf (glob_path, "%s\\\\", synexe);
#else
strcpy (glob_path, argv[0]);
while (glob_path[0] != '\0' && glob_path[strlen(glob_path)-1]!='/') glob_path[strlen(glob_path)-1]='\0';
#endif
sprintf (session_code, "%08x", (int) time (NULL));

  /*  Parse command line */

  SynVCB_Flags_Put (&SViewCB, 0);
  SynAppR_Printer_Put (&GSynAppR, NULL);
  SynAppR_RemoteDisp_Put (&GSynAppR, NULL);
  my_args = argv;
  app_size = SAR_APPSIZE_DFLT_WD;
  ++my_args;
  arg_i = 1;
  while (arg_i < argc)
    { 
    if (**my_args == '-')
      {
      if (strcmp((*my_args + 1) , "help") == 0) 
        {
        fprintf (stderr, "%s", synview_usage);
        exit (0);
        }
      else if (strcmp((*my_args + 1) , "full") == 0) 
        app_size = SAR_APPSIZE_FULL_WD;
      else if (strcmp((*my_args + 1) , "half") == 0) 
        app_size = SAR_APPSIZE_HALF_WD;
      else if (strcmp((*my_args + 1) , "mini") == 0)
        { 
        app_size = SAR_APPSIZE_MINI_WD | SAR_APPSIZE_MINI_HT;
        }
      else if (strcmp((*my_args + 1) , "distrib") == 0)
        {
        SynVCB_FlagParallel_Put (&SViewCB, TRUE);
        }
      else if (strcmp((*my_args + 1) , "display") == 0)
        { 
        ++my_args;
        ++arg_i;
        SynAppR_RemoteDisp_Put (&GSynAppR, *my_args);
        }
      else if (strcmp((*my_args + 1) , "printer") == 0) 
        { 
        ++my_args;
        ++arg_i;
        SynAppR_Printer_Put (&GSynAppR, *my_args);
        }
      }

    ++my_args;
    ++arg_i;
    }

  /*  SYNCHEM initializations.  */
  Debug_Init ();
  IO_Init ();

  Rcb_Init (&GGoalRcb, SynVCB_FlagParallel_Get (&SViewCB));

  if (!Persist_Inx_OK (FCB_SEQDIR_RXNS ("/rkbstd.inx"),
    String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS)))))
    exit (1);

  RunStats_Init (&GRunStats);

#ifndef _CYGWIN_
/* Causes uninformative warning messages under cygwin, despite definition of XAPPLRESDIR, yet serves no known useful purpose. */
  XtSetLanguageProc (NULL, NULL, NULL);
#endif

  toplevel = XtVaAppInitialize (SynVCB_AppCon_Get (&SViewCB), "SynView", 
    NULL, 0, &argc, argv, NULL, NULL);
  SynVCB_TopApp_Put (&SViewCB, (Widget) toplevel);

  SynAppR_PreInit ((Widget) toplevel, app_size);

  icon_pmap = XmGetPixmapByDepth (XtScreen ((Widget) toplevel), icon_file, 1, 0, 1);

  XtVaSetValues ((Widget) toplevel,
    XmNbackground,     SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,     SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    XmNlabelFontList,  SynAppR_FontList_Get (&GSynAppR),
    XmNbuttonFontList, SynAppR_FontList_Get (&GSynAppR),
    XmNtextFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNheight,         AppDim_AppHeight_Get (&GAppDim),
    XmNwidth,          AppDim_AppWidth_Get (&GAppDim), 
    XmNresizePolicy,   XmRESIZE_NONE,
    XmNresizable,      False,
    XmNautoUnmanage,   False, 
    XmNiconPixmap,     icon_pmap, 
    XmNtitle,          "SYNCHEM",
    XmNx,              0,
    XmNy,              0,
    NULL);

  /*  Calculate and store the initial size of main form, then create the 
      parent application form widget.  
  */ 
  topform = SView_Create (&SViewCB, (Widget) toplevel);
  SynVCB_TopForm_Put (&SViewCB, topform);

  XtRealizeWidget ((Widget) toplevel);

  SynAppR_PostInit ((Widget) toplevel);
  PstView_PostInit ((Widget) toplevel, SynVCB_PstView_Get (&SViewCB));
  Help_Form_Create ((Widget) toplevel);
  SShotV_IsCreated_Put (SynVCB_SShotView_Get (&SViewCB), FALSE);

  SelTrace_Init (PstView_SelTrace_Get (SynVCB_PstView_Get (&SViewCB)));
  SelTrace_IsCreated_Put (PstView_SelTrace_Get (
    SynVCB_PstView_Get (&SViewCB)), FALSE);

  CmpInst_IsCreated_Put (PstView_CmpInsts_Get (
    SynVCB_PstView_Get (&SViewCB)), FALSE);

  Submit_Created_Put (SynVCB_JobSubmit_Get (&SViewCB), FALSE);

  XtAppMainLoop (*SynVCB_AppCon_Get (&SViewCB));

  Persist_Close ();

  exit (0);
}
/*  End of main  */

/****************************************************************************
*
*  Function Name:                 PstView_Handle_Grab
*
*    This routine returns a handle to the PST view data structure.
*
*
*  Implicit Inputs:
*
*    SViewCB, the SYNCHEM View control block.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Handle to the PST view data structure.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
PstView_t *PstView_Handle_Grab
  (
  void
  )
{
  return (SynVCB_PstView_Get (&SViewCB));
}
/*  End of PstView_Handle_Grab  */

/****************************************************************************
*
*  Function Name:                 SShotView_Handle_Grab
*
*    This routine returns a handle to the Singleshot view data structure.
*
*
*  Implicit Inputs:
*
*    SViewCB, the SYNCHEM View control block.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Handle to the singleshot view data structure.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
SShotView_t *SShotView_Handle_Grab
  (
  void
  )
{
  return (SynVCB_SShotView_Get (&SViewCB));
}
/*  End of SShotView_Handle_Grab  */

/****************************************************************************
*
*  Function Name:                 SynView_Handle_Grab
*
*    This routine returns a handle to the SYNCHEM view data structure.
*
*
*  Implicit Inputs:
*
*    SViewCB, the SYNCHEM View control block.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Handle to the SYNCHEM view data structure.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
SynViewCB_t *SynView_Handle_Grab
  (
  void
  )
{
  return (&SViewCB);
}
/*  End of SynView_Handle_Grab  */

/****************************************************************************
*
*  Function Name:                 SynView_Exit
*
*    This routine makes a clean exit (well sort of--not fully implemented).
*
*
*  Implicit Inputs:
*
*    SViewCB, the SYNCHEM View control block.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Exits with code 0.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SynView_Exit
  (
  void
  )
{
  PstView_t     *pstv_p;

  pstv_p = SynVCB_PstView_Get (&SViewCB);

 /*  Destroy Reaction Preview Dialogs.  */
  RxnPreV_Destroy (ViewLvl_RxnPV_Get (PstView_PathVLvl_Get (pstv_p)));
  RxnPreV_Destroy (ViewLvl_RxnPV_Get (PstView_IthVLvl_Get (pstv_p, 
    PVW_LEVEL_TOP)));
  RxnPreV_Destroy (ViewLvl_RxnPV_Get (PstView_IthVLvl_Get (pstv_p, 
    PVW_LEVEL_MID)));
  RxnPreV_Destroy (ViewLvl_RxnPV_Get (PstView_IthVLvl_Get (pstv_p, 
    PVW_LEVEL_BTM)));

  /*  Destroy PST View data structures.  */
  PstView_Destroy (pstv_p);

  /*  Lastly, destroy the SYNCHEM application resources.  */
  SynAppR_Destroy ();

  /*  Destroy synchem search related data structures.  */
  Destroy_SAtable_Attr_Xtr ();

#ifdef _MIND_MEM_
  dump_vars ();
#endif
  exit (0);
}
/*  End of SynView_Exit  */

/****************************************************************************
*
*  Function Name:                 SView_Create
*
*    This routine creates and manages all of the top level application 
*    widgets needed at start up (main menu, pstview, rxn preview and 
*    rxn view.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
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
Widget SView_Create
  (
  SynViewCB_t   *svcb_p,
  Widget         top_level
  )
{
  PstView_t     *pstv_p;               /* PST viewer */ 
  Widget         menubar;
  Widget         pstframe;
  Widget         topform;
  Widget         rxnframe;
  Widget         child;
  XmString       cancel_str;                /* Cancel pb string */
  XmString       mess_str;                  /* Message string */
  XmString       ok_str;                    /* Okay pb string */
  XmString       title_str;                 /* Dialog title string */

  pstv_p = SynVCB_PstView_Get (svcb_p);

  topform = XmCreateForm (top_level, "SynForm", NULL, 0);  
  XtVaSetValues (topform,
    XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
    XmNforeground,     SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    NULL);

  /*  Create the main menu.  */
  menubar = SynMenu_Create (topform, svcb_p, glob_path); 

  /*  Create, the pst viewing area.  */
  pstframe = PstView_Create (top_level, topform, pstv_p);
  
  /*  Create and attach the rxn viewing area.  */
  rxnframe = RxnView_Create (topform, PstView_RxnView_Get (pstv_p));

  /*  Make the top form attachments */
  XtVaSetValues (menubar,
    XmNrightAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (pstframe,
    XmNrightAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, menubar,
    NULL);

  XtVaSetValues (rxnframe,
    XmNrightAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, pstframe,
    NULL);

  /*  Manage parent form widgets.  */
  XtManageChild (menubar);
  XtManageChild (pstframe);
  XtManageChild (rxnframe);
  XtManageChild (topform);

  /*  Create those auxiliary popup windows that the user would expect
      to have immediate access to when needed.  Do not manage the widgets 
      at this point.  Other popup windows will be created as needed.
  */

  /*  Create the symbol legend dialog (in a popup window).  */
  /*  Not yet implemented.  */

  /*  Create the warning message and information dialogs.  */
  InfoMess_Create (top_level);
  InfoWarn_Create (top_level);
  InfoWin_Create (top_level, SVI_INFO_STAT);
  InfoWin_Create (top_level, SVI_INFO_COMPOUND);
  InfoWin_Create (top_level, SVI_INFO_SUBGOAL);
  InfoWin_Create (top_level, SVI_INFO_RXN);
  SVF_FileDg_Create (top_level);
  SVF_QueryDg_Create (top_level);

  /*  Create the cycle count text dialog (in a popup window).  */
  SynVCB_CycleDg_Put (svcb_p, XmCreateWorkingDialog (top_level, 
    "SynWorkMD", NULL, 0));
  child = XmMessageBoxGetChild (SynVCB_CycleDg_Get (svcb_p),   
    XmDIALOG_HELP_BUTTON);
  XtUnmanageChild (child);

  cancel_str = XmStringCreateLtoR ("halt search", XmFONTLIST_DEFAULT_TAG);
  mess_str = XmStringCreateLtoR ("Starting search ...", 
    XmFONTLIST_DEFAULT_TAG);
  ok_str = XmStringCreateLtoR ("dismiss", XmFONTLIST_DEFAULT_TAG);
  title_str = XmStringCreateLtoR ("SYNCHEM Search Engine", 
    XmFONTLIST_DEFAULT_TAG);

  XtVaSetValues (SynVCB_CycleDg_Get (svcb_p),
    XmNdialogTitle, title_str, 
    XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL,
    XmNdefaultPosition, True,
    XmNokLabelString, ok_str,
    XmNcancelLabelString, cancel_str,
    XmNmessageString, mess_str,
    XmNdefaultButtonType, XmDIALOG_NONE,
    NULL);
  XmStringFree (cancel_str);
  XmStringFree (mess_str);
  XmStringFree (ok_str);
  XmStringFree (title_str);

  XtAddCallback (SynVCB_CycleDg_Get (svcb_p), XmNokCallback, 
    SSearch_Dismiss_CB, (XtPointer) NULL);

  XtAddCallback (SynVCB_CycleDg_Get (svcb_p), XmNcancelCallback, 
    SSearch_Halt_CB, (XtPointer) &GRunStats);

  return (topform);
}
/*  End of SView_Create  */

/****************************************************************************
*
*  Function Name:                 SSearch_Dismiss_CB
*
*    This routine unmanages the search message dialog.
*
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
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSearch_Dismiss_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  if (XtIsManaged (SynVCB_CycleDg_Get (&SViewCB)))
    XtUnmanageChild (SynVCB_CycleDg_Get (&SViewCB));

  return ;
}
/*  End of SSearch_Dismiss_CB  */

/****************************************************************************
*
*  Function Name:                 SSearch_Halt_CB
*
*    This routine sets the halt flag in the run statistics. 
*
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
*    None
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSearch_Halt_CB
  (
  Widget          w, 
  XtPointer       client_p, 
  XtPointer       call_p  
  )
{
  RunStats_t     *rstat_p;
 
  rstat_p = (RunStats_t *) client_p;
  RunStats_Flags_SearchHalted_Put (rstat_p, TRUE);

  return ;
}
/*  End of SSearch_Halt_CB  */

/*  End of SYN_VIEW.C */
