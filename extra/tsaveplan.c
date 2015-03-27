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

#ifndef _H_SYNHELP_
#include "synhelp.h"
#endif

#ifndef _H_PERSIST_
#include "submit_saveplan.h"
#endif

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

/* Static Variables */

static SavePlanCB_t SPCB;

static char glob_path[256];

static XtAppContext ac;

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

  SynAppR_Printer_Put (&GSynAppR, NULL);
  SynAppR_RemoteDisp_Put (&GSynAppR, NULL);
  my_args = argv;
  app_size = SAR_APPSIZE_DFLT_WD;

  /*  SYNCHEM initializations.  */
  Debug_Init ();
  IO_Init ();

#ifndef _CYGWIN_
/* Causes uninformative warning messages under cygwin, despite definition of XAPPLRESDIR, yet serves no known useful purpose. */
  XtSetLanguageProc (NULL, NULL, NULL);
#endif

  toplevel = XtVaAppInitialize (&ac, "SynView", 
    NULL, 0, &argc, argv, NULL, NULL);

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
  topform = SavePlan_Create (&SPCB, (Widget) toplevel);

  XtManageChild (topform);

  XtRealizeWidget ((Widget) toplevel);

  SynAppR_PostInit ((Widget) toplevel);

  XtAppMainLoop (ac);
}
