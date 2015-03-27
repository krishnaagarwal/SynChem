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

#include "rxnpatt_draw3.h"

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
/*
#ifndef _H_SUBMIT_JOBINFO_
#include "submit_jobinfo.h"
#endif
*/
ScreenAttr_t sat;
Widget toplevel,listw,shell2,textw,shell3,draww,shellpre,prew;
XtAppContext schlContext;
Display *disp;
Window twind,dwind;
GC tgc,dgc;
XGCValues gcv;
FuncGrp_Record_t *fgrec;
Isam_Control_t *fgfile;
char *fgname[1000];
int fgend;
XFontStruct *fs5x8;
XmFontList fl5x8;
XFontStruct *fs6x10;
XmFontList fl6x10;
Boolean_t fgsel[1000],fgpossel[1000];

void toggle_fg(),get_sch(),prev_sch(),next_sch(),dispsch(U32_t,Boolean_t);
U32_t NSch,curr_sch;
U32_t root_syntheme(U32_t, U32_t *,U32_t *);
void mdraw(Dsp_Molecule_t *,Tsd_t *,Display *,Window,GC,int,int,int,int,
	U32_t *,U32_t *);
Tsd_t *composite(Tsd_t *,Tsd_t *);
char * date_calc(long);
int days(int);
int drawString(int,int,char *,int);
void fill_fglist(Widget);
void fg_select(Widget,int,char *);
void fg_deselect(Widget,int);
void fg_pos_select(Widget,int,int);
void fg_pos_deselect(Widget,int);
void fg_deselect_all(Widget);
void InfoWarn_Show(char *);
/*
void RxnInfo_SlingTxt_Update (RxnInfoCB_t **,Dsp_Molecule_t **);
*/

extern Widget XmCreateScrolledList(
			Widget parent,
			String name,
			ArgList args,
			Cardinal arg_count);
extern Widget XmCreateDrawingArea(
			Widget parent,
			String name,
			ArgList args,
			Cardinal arg_count);

int main(int argc,char *argv[])
{
	char tempfgname[100];
	int i,j, sch;
	RxnInfoCB_t rxninfo_p[2];
	Widget drawtool;
	Tsd_t *gtsd,*stsd;
	Xtr_t *gxtr,*sxtr;
	U32_t roots[MX_ROOTS],synthemes[MX_ROOTS];
	React_Record_t *schema;
	React_Head_t *sch_head;

if (argc==2)
  sch=atoi(argv[1]);
else
  sch=0;

	toplevel=XtVaAppInitialize(&schlContext,"SchList",NULL,0,&argc,argv,
		NULL,NULL);

	SynAppR_PreInit(toplevel,SAR_APPSIZE_DFLT_WD);

	SynAppR_PostInit(toplevel);

	XtVaSetValues(toplevel,XmNheight,750,XmNwidth,750,
		XmNtitle,"Reaction Library",XmNx,0,XmNy,0,NULL);
	XtRealizeWidget(toplevel);

	AppDim_AppHeight_Put (&GAppDim, 750);

	if (sch)
	{
		React_Init((U8_t *)"d:/synchem/testdata/rxnlib");
		schema=React_Schema_Handle_Get(sch - 1);
		sch_head=React_Head_Get(schema);
		for (j=0; j<MX_ROOTS; j++)
		{
			roots[j]=React_Head_RootNode_Get(sch_head, j);
			synthemes[j]=React_Head_RootSyntheme_Get(sch_head, j);
		}
		gtsd=React_Goal_Get(schema);
		stsd=React_Subgoal_Get(schema);
		gxtr=Tsd2Xtr(gtsd);
		sxtr=Tsd2Xtr(stsd);
printf("before PDraw_Tool_Create w/ schema\n");
		drawtool=PDraw_Tool_Create(rxninfo_p,toplevel,&schlContext,
			gxtr,sxtr,roots,synthemes);
	}
	else
	{
printf("before PDraw_Tool_Create w/o schema\n");
	drawtool=PDraw_Tool_Create(rxninfo_p,toplevel,&schlContext,
		NULL,NULL,NULL,NULL);
	}
/*
printf("main 1\n");
	XtManageChild (drawtool);
printf("main 2\n");
*/
	XtAppMainLoop(schlContext);
}
/*
void RxnInfo_SlingTxt_Update (RxnInfoCB_t **ji,Dsp_Molecule_t **dm)
{
  Tsd_t    *tsd_p;
  Sling_t   sling; 
  Sling_t   canon_sling; 
  int       pat;

  if (dm == NULL)
    return;
   
  tsd_p = PDsp2Tsd (dm);
  sling = Tsd2Sling (tsd_p);
  canon_sling = PSling2CanonSling (sling);
  printf("Sling: %s\n",Sling_Name_Get(canon_sling));
}
*/
void InfoWarn_Show(char *mess)
{
printf("Warning: %s\n",mess);
}
