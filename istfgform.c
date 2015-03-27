/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     ISTFGFORM.C
*
*    Selection list for selecting functional groups as a complement to pattern
*    drawings for the ISTEHER search.
*
*  Creation Date:
*
*    18-Oct-2000
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
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ScrolledW.h>
#include <Xm/Text.h>

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

#include "submit_draw.h"

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

#ifndef _H_EXTERN_
#include "extern.h"
#endif

#define NUMBER_OF_ILLEGAL_STRUCTURES 29
#define SYNTHEME_FLAG 0x7fff
#define ILLEGAL_FLAG 0x7ffe

static Boolean_t fgsel[2][7][2*MX_FUNCGROUPS],fgpossel[2][7][2*MX_FUNCGROUPS],illegal[MX_FUNCGROUPS],toggle_on=TRUE;
static Widget tl, topform = (Widget) NULL, prel, conflict_msg[2], msgtext[2];
static int illegal_fg[NUMBER_OF_ILLEGAL_STRUCTURES] /* FROM COMPOK */ =
	{48, 246, 247, 248, 249, 250,      252, 253, 254, 255,
	 257, 258, 259, 260, 262, 263, 264, 265, 266, 267,
	 268, 269, 271, 272, 273, 274, 275, 270, 549};
static char conflict_text[2][10000];
static int *glob_goalfg, *glob_subgfg, *glob_gminin, *glob_sminin;

void IsTList_Update (Boolean_t);

void Refresh_IstList (Boolean_t);
void itoggle_fg (Widget, XtPointer, XtPointer);
void IFGHelp_CB (Widget, XtPointer, XtPointer);
void IFGButt_CB (Widget, XtPointer, XtPointer);
void ifill_fglist(Widget);
void ifg_select(Widget,int,char *, Boolean_t);
void ifg_deselect(Widget,int);
void ifg_pos_select(Widget,int,int, Boolean_t);
void ifg_pos_deselect(Widget,int,Boolean_t);
void ifg_deselect_all(Widget,Boolean_t);
void ifg_restore (Widget);

void IsTFG_Form_Create (Widget top_level, int *goalfg, int *subgfg, int *gminin, int *sminin)
{
	Boolean_t
		notg[MX_FUNCGROUPS], notsg[MX_FUNCGROUPS],
		mustg[MX_FUNCGROUPS], mustsg[MX_FUNCGROUPS];
	XmFontListEntry
		courb18, helv18;
	XmFontList
		flcob18, flhv18;
	Display
		*disp;
	int
		i, j;
	XmString
		label, title, title2;
	static Widget
		prew, button[5], txtform[2], helppb;
	static char
		*itemstr[6] = {"Reset", "Restore", "View Structures", "Exit; Update List", "Cancel"};

	disp = XtDisplay (tl=top_level);
	glob_goalfg=goalfg;
	glob_subgfg=subgfg;
	glob_gminin=gminin;
	glob_sminin=sminin;

	if (topform == (Widget) NULL)
		{
		topform = XmCreateFormDialog (top_level, "IsThere FG Editor", NULL, 0);
		label = XmStringCreateLocalized ("IsThere Functional Group List Editor");

		XtVaSetValues (topform,
			XmNresizePolicy, XmRESIZE_NONE,
			XmNdialogTitle, label,
			XmNwidth,1100,
			XmNheight,900,
			XmNfractionBase,    800,
			NULL);

		prew = XmCreateScrolledWindow(topform, "IsThere FGs", NULL, 0);
		prel = XmCreateList(prew, "IsThere FGList", NULL, 0);

		courb18 = XmFontListEntryLoad (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
			XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
		helv18 = XmFontListEntryLoad (disp, "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
			XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);

		flcob18=XmFontListAppendEntry (NULL, courb18);
		flhv18 = XmFontListAppendEntry (NULL, helv18);

		XtVaSetValues(prel,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        		SAR_CLRI_WHITE),
        		XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        		SAR_CLRI_BLACK),
        		XmNfontList,flcob18,
        		XmNx,0,XmNy,0,
        		XmNselectionPolicy,XmMULTIPLE_SELECT,NULL);

		XtVaSetValues(prew,
			XmNtopOffset,0,XmNtopAttachment,XmATTACH_FORM,
			XmNbottomPosition,775,XmNbottomAttachment,XmATTACH_POSITION,
			XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
			XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
			NULL);
        	XtAddCallback(prel,XmNmultipleSelectionCallback,itoggle_fg,(XtPointer) disp);

		label = XmStringCreateLocalized ("Help");

		helppb = XtVaCreateManagedWidget ("HelpPB",
			xmPushButtonGadgetClass, topform,
			XmNfontList, flhv18,
			XmNlabelString, label,
			XmNrecomputeSize, True,
			XmNtopOffset, 0,
			XmNtopAttachment, XmATTACH_WIDGET,
			XmNtopWidget, prew,
			XmNbottomOffset, 0,
			XmNbottomAttachment, XmATTACH_FORM,
			XmNleftPosition, 1,
			XmNleftAttachment, XmATTACH_POSITION,
			XmNrightPosition, 49,
			XmNrightAttachment, XmATTACH_POSITION,
			NULL);

		XmStringFree (label);

		XtAddCallback (helppb, XmNactivateCallback, IFGHelp_CB,
                  (XtPointer) "istfgform:IsThere Functional Group List Editor");

		for (i=0; i<5; i++)
			{
			label = XmStringCreateLocalized (itemstr[i]);

			button[i] = XtVaCreateManagedWidget (itemstr[i],
				xmPushButtonGadgetClass, topform,
				XmNfontList, flhv18,
				XmNlabelString, label,
				XmNrecomputeSize, True,
				XmNtopOffset, 0,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, prew,
				XmNbottomOffset, 0,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNleftPosition, 150 * i + 51,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNrightPosition, 150 * (i + 1) + 49,
				XmNrightAttachment, XmATTACH_POSITION,
				NULL);

			XmStringFree (label);

			XtAddCallback (button[i], XmNactivateCallback, IFGButt_CB, (XtPointer) i);
			XtManageChild (button[i]);
			}

		ifill_fglist (prel);
		XtManageChild (prel);
		XtManageChild (prew);
		}

	XtManageChild (topform);

	ifg_deselect_all (prel,TRUE);

        for (i=0; i<MX_FUNCGROUPS; i++)
                notg[i] = notsg[i] = mustg[i] = mustsg[i] = FALSE;

	for (i=0; goalfg[i]!=0; i++)
		{
		if (goalfg[i]<0) 
			{
			notg[-goalfg[i]] = TRUE;
			ifg_select(prel,-2*goalfg[i]-1,"-G",TRUE);
			}
		else
			{
			mustg[goalfg[i]] = TRUE;
			switch (gminin[i])
				{
			case -3:
				ifg_select(prel,2*goalfg[i]-1,"<3",TRUE);
				break;
			case -2:
				ifg_select(prel,2*goalfg[i]-1,"<2",TRUE);
				break;
			case 1:
				ifg_select(prel,2*goalfg[i]-1,"+G",TRUE);
				break;
			case 2:
				ifg_select(prel,2*goalfg[i]-1,">1",TRUE);
				break;
			case 3:
				ifg_select(prel,2*goalfg[i]-1,">2",TRUE);
				break;
				}
			}
		}

	for (i=0; subgfg[i]!=0; i++)
		{
		if (subgfg[i]<0) 
			{
			notsg[-subgfg[i]] = TRUE;
			ifg_select(prel,-2*subgfg[i],"-S",TRUE);
			}
		else
			{
			mustsg[subgfg[i]] = TRUE;
			switch (gminin[i])
				{
			case -3:
				ifg_select(prel,2*subgfg[i],"<3",TRUE);
				break;
			case -2:
				ifg_select(prel,2*subgfg[i],"<2",TRUE);
				break;
			case 1:
				ifg_select(prel,2*subgfg[i],"+S",TRUE);
				break;
			case 2:
				ifg_select(prel,2*subgfg[i],">1",TRUE);
				break;
			case 3:
				ifg_select(prel,2*subgfg[i],">2",TRUE);
				break;
				}
			}
		}
}

void IFGHelp_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild (topform);
  Help_CB (w, client_data, call_data);
}

void IFGButt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
	int which, i, j, k, l;
	Boolean_t
		notg[MX_FUNCGROUPS], notsg[MX_FUNCGROUPS],
		mustg[MX_FUNCGROUPS], mustsg[MX_FUNCGROUPS];

	which = (int) client_data;
	switch (which)
	{
	case 0:
XtManageChild (topform);
		ifg_deselect_all (prel,FALSE);
return;
		break;
	case 1:
XtManageChild (topform);
		ifg_restore (prel);
return;
		break;
	case 2:
		XtManageChild (topform);
		FGDraw_Show_Window (0, 0);
		break;
	case 3:
        	for (i=1, j=k=0; i<=MX_FUNCGROUPS; i++)
        	{
			notg[i] = fgsel[1][2][2*i-1];
			for (l = 0, mustg[i] = FALSE; l < 6; l += (l == 1 ? 2 : 1)) if (fgsel[1][l][2*i-1])
				{
				mustg[i] = TRUE;
				glob_gminin[j] = l - 3 + (l > 2);
				}
			notsg[i] = fgsel[1][2][2*i];
			for (l = 0, mustsg[i] = FALSE; l < 6; l += (l == 1 ? 2 : 1)) if (fgsel[1][l][2*i])
				{
				mustsg[i] = TRUE;
				glob_sminin[k] = l - 3 + (l > 2);
				}
/*
			if (notg[(i+1)/2])
			{
				glob_gminin[j]=0;
				glob_goalfg[j++]=-(i+1)/2;
				glob_goalfg[j]=0;
			}
			else if (mustg[(i+1)/2])
			{
				glob_goalfg[j++]=(i+1)/2;
				glob_goalfg[j]=0;
			}
			else if (notsg[(i+1)/2])
			{
				glob_sminin[k]=0;
				glob_subgfg[k++]=-(i+1)/2;
				glob_subgfg[k]=0;
			}
			else if (mustsg[(i+1)/2])
			{
				glob_subgfg[k++]=(i+1)/2;
				glob_subgfg[k]=0;
			}
*/
			if (notg[i])
			{
				glob_gminin[j]=0;
				glob_goalfg[j++]=-i;
				glob_goalfg[j]=0;
			}
			else if (mustg[i])
			{
				glob_goalfg[j++]=i;
				glob_goalfg[j]=0;
			}
			if (notsg[i])
			{
				glob_sminin[k]=0;
				glob_subgfg[k++]=-i;
				glob_subgfg[k]=0;
			}
			else if (mustsg[i])
			{
				glob_subgfg[k++]=i;
				glob_subgfg[k]=0;
			}
        	}
printf("goal: ");
for (i=0; glob_goalfg[i]; i++) printf("%d (%d) ",glob_goalfg[i],glob_gminin[i]);
printf("\nsubgoal: ");
for (i=0; glob_subgfg[i]; i++) printf("%d (%d) ",glob_subgfg[i],glob_sminin[i]);
putchar('\n');

		IsTList_Update (TRUE);
		ifg_deselect_all (prel,FALSE);
		XtUnmanageChild (topform);
		break;
	case 4:
		IsTList_Update (FALSE);
		ifg_deselect_all (prel,FALSE);
		XtUnmanageChild (topform);
		break;
	default:
		break;
	}
}

void itoggle_fg(Widget w, XtPointer client_data, XtPointer call_data)
{
        int *pos,count,rx,ry,x,y,i,j;
        unsigned mask;
	Display *disp;
        Window rw,cw;
        Boolean_t found;

printf("itoggle_fg entered\n");
	disp = (Display *) client_data;
        XmListGetSelectedPos(w,&pos,&count);
        if (!count)
          {
          XtFree ((char *) pos);
          ifg_deselect_all (w,FALSE);
	  return;
	  }
printf("itoggle_fg 1\n");
        for (i=0; i<count; i++) if (!fgpossel[1][6][pos[i]])
        {
printf("itoggle_fg 1.%d.0\n",i);
                XQueryPointer(disp,XtWindow(w),&rw,&cw,&rx,&ry,&x,&y,&mask);
printf("itoggle_fg 1.%d.1\n",i);
                ifg_pos_select(w,pos[i],x, TRUE);
printf("itoggle_fg 1.%d.2\n",i);
        }
printf("itoggle_fg 2\n");
        for (i=1; i<=MX_FUNCGROUPS; i++) if (fgpossel[1][6][i] && toggle_on)
        {
                found=FALSE;
                for (j=0; j<count && !found; j++) found=pos[j]==i;
                if (!found) ifg_pos_deselect(w,i,TRUE);
        }
	toggle_on=TRUE;
        XtFree ((char *) pos);
printf("exiting itoggle_fg\n");
}

void ifill_fglist(Widget w)
{
	XmString item;
	char itemstr[200];
	int i;

	for (i=0; i<MX_FUNCGROUPS; i++) illegal[i]=FALSE;
	for (i=0; i<NUMBER_OF_ILLEGAL_STRUCTURES; i++) illegal[illegal_fg[i]]=TRUE;
	XmListSetAddMode(w,TRUE);
	for (i=1; i<2*fgend-1; i++) if (fgname[(i+1)/2]!=NULL)
	{
		if (illegal[(i+1)/2]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[(i+1)/2],(i+1)/2);
		else
		{
			if (i%2) sprintf(itemstr,"[<3 <2 -G +G >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
			else sprintf(itemstr,"[<3 <2 -S +S >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
			if (fgprsv[i/2+1]) strcat (itemstr, "[P]");
		}
		item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	XmListSetAddMode(w,FALSE);
	XmListSetBottomPos(w,0);
	XmListSetPos(w,1);
}

void ifg_select(Widget w,int num,char *type,Boolean_t initial)
{
	int i,j,pos,which;
	char itemstr[200],*typepos;
	XmString item;

	if (illegal[num])
	{
		fg_deselect (w, num);
		return;
	}
/*
	if (strcmp (type, "-G") == 0) which=0;
	else if (strcmp (type, "+G") == 0) which=1;
	else if (strcmp (type, "-S") == 0) which=2;
	else if (strcmp (type, "+S") == 0) which=3;
*/
	if (strcmp (type, "<3") == 0) which=0;
	else if (strcmp (type, "<2") == 0) which=1;
	else if (type[0] == '-') which=2;
	else if (type[0] == '+') which=3;
	else if (strcmp (type, ">1") == 0) which=4;
	else if (strcmp (type, ">2") == 0) which=5;
	for (i=pos=1; i<num; i++) if (fgname[(i+1)/2]!=NULL) pos++;
	for (j = initial ? 0: 1; j < 2; j++)
		fgsel[j][which][i]=fgsel[j][6][i]=fgpossel[j][which][pos]=fgpossel[j][6][pos]=TRUE;
	if (i%2) sprintf(itemstr,"[<3 <2 -G +G >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
	else sprintf(itemstr,"[<3 <2 -S +S >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
	if (fgprsv[(i+1)/2]) strcat (itemstr, "[P]");
	typepos=strstr(itemstr,type);
	strncpy(itemstr,"                   ",19);
	strncpy(typepos,type,2);

	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos);
	XmStringFree(item);
	XmListSelectPos(w,pos,FALSE);
}

void ifg_deselect(Widget w,int num)
{
	int i,j,pos;
	char itemstr[200];
	XmString item;

	for (i=pos=1; i<num; i++) if (fgname[(i+1)/2]!=NULL) pos++;
	for (j=0; j<7; j++)
		fgsel[1][j][i]=fgpossel[1][j][pos]=FALSE;

	if (illegal[(i+1)/2]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[(i+1)/2],(i+1)/2);
	else
	{
		if (i % 2) sprintf(itemstr,"[<3 <2 -G +G >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
		else sprintf(itemstr,"[<3 <2 -S +S >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
		if (fgprsv[i]) strcat (itemstr, "[P]");
	}

	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos);
	XmStringFree(item);
	XmListDeselectPos(w,pos);
}

void ifg_deselect_all(Widget w,Boolean_t initial)
{
	int i,j;

	if (initial) for (j=0; j<2; j++)
		for (i=1; i<2*MX_FUNCGROUPS; i++)
			fgpossel[j][0][i]=fgpossel[j][1][i]=fgpossel[j][2][i]=fgpossel[j][3][i]=fgpossel[j][4][i]=
			fgpossel[j][5][i]=fgpossel[j][6][i]=fgsel[j][0][i]=fgsel[j][1][i]=fgsel[j][2][i]=fgsel[j][3][i]=
			fgsel[j][4][i]=fgsel[j][5][i]=fgsel[j][6][i]=FALSE;
	else
		for (i=1; i<2*MX_FUNCGROUPS; i++) if (fgsel[1][6][i]) ifg_deselect(w,i);
}

void ifg_restore(Widget w)
{
	int i,j,k;
	static char *type[2][6] = {{"<3", "<2", "-S", "+S", ">1", ">2"}, {"<3", "<2", "-G", "+G", ">1", ">2"}};

	ifg_deselect_all(w,FALSE);
	for (i=1; i<2*MX_FUNCGROUPS; i++) if (fgsel[0][6][i])
	{
		for (j=0, k=-1; j<6; j++) if (fgsel[0][j][i]) k=j;
		ifg_select (w,i,type[i%2][k],FALSE);
	}
	else if (fgsel[1][6][i]) ifg_deselect (w,i);
}

void ifg_pos_select(Widget w,int num,int x, Boolean_t multiple)
{
	int i,j,pos,xinx,conflict_num;
	char itemstr[200],*typepos;
	static char *type[2][6] = {{"<3", "<2", "-S", "+S", ">1", ">2"}, {"<3", "<2", "-G", "+G", ">1", ">2"}};
	XmString item;
	Boolean_t conflict;

	if (x<35) xinx=0;
	else if (x<70) xinx=1;
	else if (x<105) xinx=2;
	else if (x<140) xinx=3;
	else if (x<175) xinx=4;
	else if (x<210) xinx=5;
	else
	{
		ifg_pos_deselect (w, num, FALSE);
		return;
	}
printf("ifgps 1\n");
	for (i=pos=1; pos<=num; i++) if (fgname[(i+1)/2]!=NULL) pos++;
	pos--;
	i--;
printf("ifgps 2\n");
	fgsel[1][xinx][i]=fgsel[1][6][i]=fgpossel[1][xinx][pos]=fgpossel[1][6][pos]=TRUE;
printf("ifgps 3\n");
	if (illegal[(i+1)/2]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[(i+1)/2],(i+1)/2);
	else
	{
		if (i%2) sprintf(itemstr,"[<3 <2 -G +G >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
		else sprintf(itemstr,"[<3 <2 -S +S >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
		if (fgprsv[(i+1)/2]) strcat (itemstr, "[P]");
	}
printf("ifgps 4\n");
	typepos=strstr(itemstr,type[i%2][xinx]);
	strncpy(itemstr,"                   ",19);
	strncpy(typepos,type[i%2][xinx],2);
	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos);
	XmStringFree(item);
	XmListSelectPos(w,pos,FALSE);
}

void ifg_pos_deselect(Widget w,int num,Boolean_t multiple)
{
	int i,j,pos,xinx;
	char itemstr[200];
	XmString item;

printf("ifgpd 1\n");
	for (i=pos=1; pos<=num; i++) if (fgname[(i+1)/2]!=NULL) pos++;
	pos--;
	i--;
	for (j=0; j<7; j++)
		fgsel[1][j][i]=fgpossel[1][j][pos]=FALSE;
printf("ifgpd 2\n");
	if (illegal[(i+1)/2]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[(i+1)/2],(i+1)/2);
	else
	{
		if (i%2) sprintf(itemstr,"[<3 <2 -G +G >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
		else sprintf(itemstr,"[<3 <2 -S +S >1 >2] %s (%d)",fgname[(i+1)/2],(i+1)/2);
		if (fgprsv[(i+1)/2]) strcat (itemstr, "[P]");
	}
printf("ifgpd 3\n");
	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos);
	XmStringFree(item);
	XmListDeselectPos(w,pos);
}

void Refresh_IsTList (Boolean_t modified)
{
  if (modified)
	{
	XmListDeleteAllItems(prel);
	ifill_fglist(prel);
	IsTFG_Form_Create (tl,glob_goalfg,glob_subgfg,glob_gminin,glob_sminin);
	}
  else XtManageChild (topform);
}
