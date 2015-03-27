/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              RXLEDIT.C
*
*    This module provides for the display and editing of the entire reaction
*    library.
*
*  Creation Date:
*
*       21-Mar-2000
*
*  Authors:
*
*    Jerry Miller
*
*  Modification History, reverse chronological
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

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_TIMER_
#include "timer.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_RXLFORM_
#include "rxlform.h"
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

#ifndef _H_LOGIN_
#include "login.h"
#endif

#ifndef _H_EXTERN_
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

int main(int argc, char *argv[])
{
	XtAppContext schlContext;
	int i,j,sel_sch;
        char tempfgname[1000];
	FuncGrp_Record_t *fgrec;
	Isam_Control_t *fgfile;
	ScreenAttr_t *scra_p;

	scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

	toplevel=XtVaAppInitialize(&schlContext,"SchList",NULL,0,&argc,argv,
		NULL,NULL);

	SynAppR_PreInit((Widget) toplevel,SAR_APPSIZE_DFLT_WD);

	SynAppR_PostInit((Widget) toplevel);

if (argc>=4)
{
sel_sch=atoi(argv[1]);
strcpy(sshot_comp,argv[2]);
if (argc==6)
{
sscanf(argv[4],"%x",&glob_run_date);
sscanf(argv[5],"%x",&glob_run_time);
/* if (glob_run_date != PER_TIME_ANY || glob_run_time != PER_TIME_ANY) */ logged_in = TRUE; /* disable changes */
}
else Login_From_Main (argv[3]);
	XtVaSetValues((Widget) toplevel,XmNheight, 25, XmNwidth, 25, XmNx,750,XmNy,500,NULL);
}
else
{
sel_sch=-1;
sshot_comp[0]='\0';
	XtVaSetValues((Widget) toplevel,XmNheight,/*1000*/ Screen_Height_Get (scra_p) - SWS_APPSHELL_OFFY,
		XmNwidth,/*1300*/ Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
		XmNtitle,"Reaction Library Editor",XmNx,0,XmNy,0,NULL);
}
	XtRealizeWidget((Widget) toplevel);

	AppDim_AppHeight_Put (&GAppDim, 1000);
//printf("kka0:in rxledit\n");
#ifdef _DEBUG_
printf("before Debug_Init\n");
#endif
        Debug_Init ();
#ifdef _DEBUG_
printf("before Trace_Init\n");
#endif
        Trace_Init ();
#ifdef _DEBUG_
printf("before IO_Init\n");
#endif
        IO_Init ();
#ifdef _CYGWIN_
        IO_Init_Files ("NUL:", FALSE);
#else
        IO_Init_Files ("/dev/null", FALSE);
#endif

#ifdef _DEBUG_
printf("before FGDraw_Init\n");
#endif
        FGDraw_Init (scra_p,FALSE);
#ifdef _DEBUG_
printf("before FGDraw_Window_Create\n");
#endif
        FGDraw_Window_Create ((Widget) toplevel, scra_p);

	bypass_incomplete_flag = TRUE;
#ifdef _DEBUG_
printf("before React_Initialize\n");
#endif
	React_Initialize ((U8_t *) (concat ("R+W", FCB_SEQDIR_RXNS (""))),
          Persist_Inx_OK (FCB_SEQDIR_RXNS ("/rkbstd.inx"), concat ("R+W", FCB_SEQDIR_RXNS (""))));
#ifdef _DEBUG_
printf("before Persist_Open\n");
#endif
	Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);
#ifdef _DEBUG_
printf("before FuncGroups_Init\n");
#endif
	FuncGroups_Init (FCB_SEQDIR_FNGP (""));
//printf("kka1:in rxledit\n");
	for (i=0; i<1000; i++) fgprsv[i] = FALSE;
        fgfile = (Isam_Control_t *) malloc (ISAMCONTROLSIZE);
        if (fgfile == NULL) IO_Exit_Error (R_AVL, X_SYSCALL,
                "Unable to allocate memory for Isam Control Block.");
        strcpy(IO_FileName_Get (Isam_File_Get (fgfile)),
                FCB_SEQDIR_FNGP ("/fgdata.isam"));
        Isam_Open(fgfile,ISAM_TYPE_FGINFO,ISAM_OPEN_READ);
        fgend=0;
        i=1;
//printf("kka2:in rxledit\n");
        while((fgrec=FuncGrp_Rec_Read(i,fgfile))!=NULL)
        {
                j=FuncGrp_Head_FGNum_Get(FuncGrp_Rec_Head_Get(fgrec));
                while(fgend!=j) fgname[fgend++]=NULL;
                strcpy(tempfgname,String_Value_Get(
                        FuncGrp_Rec_Name_Get(fgrec,0)));
		for (j=1; j<FuncGrp_Head_NumNames_Get (FuncGrp_Rec_Head_Get (fgrec)); j++)
			sprintf(tempfgname+strlen(tempfgname),", %s",
			String_Value_Get(FuncGrp_Rec_Name_Get(fgrec,j)));
                fgname[fgend]=(char *)malloc(strlen(tempfgname)+1);
                strcpy(fgname[fgend],tempfgname);
		fgprsv[fgend] = FuncGrp_Head_FlagsPreserveable_Get (FuncGrp_Rec_Head_Get (fgrec));
                fgend++;
                i++;
        }
        Isam_Close(fgfile);
#ifdef _DEBUG_
printf("after Isam_Close\n");
#endif
//printf("kka3:in rxledit\n");

	Help_Form_Create ((Widget) toplevel);
#ifdef _DEBUG_
printf("after Help_Form_Create\n");
#endif
//printf("kka:after Help_Form_Create\n");
        PT_Form_Create ((Widget) toplevel);
        EYC_Form_Create ((Widget) toplevel);
        Flags_Form_Create ((Widget) toplevel);
        TM_Form_Create ((Widget) toplevel);
        RC_Form_Create ((Widget) toplevel);
//printf("kka:after RC_Form_Create\n");
	InfoMess_Create ((Widget) toplevel);
	InfoWarn_Create ((Widget) toplevel);
	InfoWin_Create ((Widget) toplevel, SVI_INFO_STAT);
	InfoWin_Create ((Widget) toplevel, SVI_INFO_COMPOUND);
	InfoWin_Create ((Widget) toplevel, SVI_INFO_SUBGOAL);
	InfoWin_Create ((Widget) toplevel, SVI_INFO_RXN);
//printf("kka:after InfoWin_Create\n");
	SVF_FileDg_Create ((Widget) toplevel);
//printf("kka:after SVF_FileDg_Create\n");
	SVF_QueryDg_Create ((Widget) toplevel);
//printf("kka:after SVF_QueryDg_Create\n");
	RxlEdit_Create_Form ((Widget) toplevel, schlContext, sel_sch);
//printf("kka4:in rxledit\n");
	XtAppMainLoop(schlContext);
}
