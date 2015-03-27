/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     AVLEDIT.C
*
*    The available compounds editor for the Synchem GUI suite.
*
*  Creation Date:
*
*    11-Oct-2000
*
*  Authors:
*
*    Gerald A. Miller
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
#include <string.h>
#include <math.h>

#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/PushB.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#include "app_resrc.h"

#include "synhelp.h"

#include "rxnpatt_draw.h"

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_AVLFORM_
#include "avlform.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
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

#ifndef _H_SUBMIT_
#include "submit.h"
#endif

#ifndef _H_EXTERN_
#define _H_DRAWPAD_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#undef _H_DRAWPAD_
#endif

int main(int argc, char *argv[])
{
	SubmitCB_t scbt;
	XtAppContext schlContext;
	int i,j,dir_len;
	char avl_comp[512],*fn_p;
	ScreenAttr_t *scra_p;
	U8_t buff[AVI_INFOREC_LENMAX];

	if (argc != 1) strcpy (sshot_comp, argv[1]);

	scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

	toplevel=XtVaAppInitialize(&schlContext,"AvlEdit",NULL,0,&argc,argv,
		NULL,NULL);

	SynAppR_PreInit((Widget) toplevel,SAR_APPSIZE_DFLT_WD);

	SynAppR_PostInit((Widget) toplevel);

        XtVaSetValues((Widget) toplevel,XmNheight,Screen_Height_Get (scra_p) - SWS_APPSHELL_OFFY,
                XmNwidth,Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
                XmNtitle,"DrawPad",XmNx,0,XmNy,0,NULL);

	XtRealizeWidget((Widget) toplevel);

	AppDim_AppHeight_Put (&GAppDim, 1000);

        Debug_Init ();
        Trace_Init ();
        IO_Init ();
#ifdef _CYGWIN_
        IO_Init_Files ("NUL:", FALSE);
#else
        IO_Init_Files ("/dev/null", FALSE);
#endif

	Help_Form_Create ((Widget) toplevel);
	InfoMess_Create ((Widget) toplevel);
	InfoWarn_Create ((Widget) toplevel);

	Submit_Created_Put (&scbt, FALSE);
	ValCompEntry_Create (&scbt, (Widget) toplevel, &schlContext, -2);

	XtManageChild (Submit_FormDlg_Get (&scbt));

	XtAppMainLoop(schlContext);
}

DrawPad_Drawn_Sling (char *sling)
{
fprintf(stderr,"%s\n",sling);
  printf ("\n%s\n", sling); /* isolate line from any unterminated debug output */
  exit (0);
}
