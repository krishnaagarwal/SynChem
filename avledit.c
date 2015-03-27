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

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

void SNSlings
  (
  )
{
  /*  Read in zero record of Isam sling info file.  Verify version
      number and get number of slings.  Read in the slings.
  */

  Isam_Read_Nobuffer (&Avd_DictCtrl_InfoFC_Get (SDictControl), 0,
    &Avd_DictCtrl_InfoZR_Get (SDictControl), AVC_RECZERO_SZ);

  if (Avc_RecZero_VerNum_Get (Avd_DictCtrl_InfoZR_Get (SDictControl))
      != AVI_VERSION_ISAMINFO)
    {
    fprintf (stderr, "\nSNSlings:  incorrect Isam info file version.\n");
    exit (-1);
    }

  slg_count = Avc_RecZero_CntSz_Get (Avd_DictCtrl_InfoZR_Get (SDictControl));
}

int main(int argc, char *argv[])
{
	XtAppContext schlContext;
	int i,j,dir_len;
	char avl_comp[512],*fn_p;
	ScreenAttr_t *scra_p;
	U8_t buff[AVI_INFOREC_LENMAX];

	scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

	toplevel=XtVaAppInitialize(&schlContext,"AvlEdit",NULL,0,&argc,argv,
		NULL,NULL);

	SynAppR_PreInit((Widget) toplevel,SAR_APPSIZE_DFLT_WD);

	SynAppR_PostInit((Widget) toplevel);

if (argc==4)
{
strcpy(avl_comp,argv[1]);
Login_From_Main (argv[2]);
if (strcmp (argv[3], "no") == 0) strcat (avl_comp, " ");
/*
	XtVaSetValues((Widget) toplevel,XmNheight, 25, XmNwidth, 25, XmNx,750,XmNy,500,NULL);
*/
	XtVaSetValues((Widget) toplevel,XmNheight,/*1000*/ Screen_Height_Get (scra_p) - SWS_APPSHELL_OFFY,
		XmNwidth,/*1300*/ Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
		XmNtitle,"Available Compound Editor",XmNx,0,XmNy,0,NULL);
}
else
{
avl_comp[0]='\0';
	XtVaSetValues((Widget) toplevel,XmNheight,/*1000*/ Screen_Height_Get (scra_p) - SWS_APPSHELL_OFFY,
		XmNwidth,/*1300*/ Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
		XmNtitle,"Available Compound Editor",XmNx,0,XmNy,0,NULL);
}
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

	fn_p = IO_FileName_Get (Isam_File_Get (&Avd_DictCtrl_InfoFC_Get (SDictControl)));
        strcpy (fn_p, FCB_SEQDIR_AVLC (""));
	dir_len = strlen(fn_p);
	strncpy (fn_p + dir_len, AVI_DATAFILE_ISAMINFO, MX_FILENAME - (dir_len + 1));
	Isam_Open (&Avd_DictCtrl_InfoFC_Get (SDictControl), ISAM_TYPE_AVLCOMP, ISAM_OPEN_READ);
	SNSlings ();
//printf("kka:sling count=%d",slg_count);
	for (i=1; i<=slg_count; i++)
	{
		Isam_Read_Nobuffer (&Avd_DictCtrl_InfoFC_Get (SDictControl), i, buff, AVI_INFOREC_LENMAX);
		avinfo_p[i-1] = AviCmpInfo_Extract(buff);
	}
	Isam_Close (&Avd_DictCtrl_InfoFC_Get (SDictControl));

	Avl_Init_Free_VarLen ();

	Isam_Open (&Avd_DictCtrl_InfoFC_Get (SDictControl), ISAM_TYPE_AVLCOMP, ISAM_OPEN_WRITE);

	Help_Form_Create ((Widget) toplevel);
	InfoMess_Create ((Widget) toplevel);
	InfoWarn_Create ((Widget) toplevel);
	AvlEdit_Create_Form ((Widget) toplevel, schlContext, avl_comp);

	XtAppMainLoop(schlContext);
}
