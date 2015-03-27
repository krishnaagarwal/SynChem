/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PRETEST.C
*
*    This module provides for the display and editing of pretransform tests.
*
*  Creation Date:
*
*       16-Feb-2000
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
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
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

#ifndef _CYGHAT_
#ifdef _CYGWIN_
#define _CYGHAT_
#else
#ifdef _REDHAT_
#define _CYGHAT_
#endif
#endif
#endif

#define MAX_TEMPLATES 50
#define NUMBER_OF_ILLEGAL_STRUCTURES 29
#define SYNTHEME_FLAG 0x7fff
#define ILLEGAL_FLAG 0x7ffe

extern char *tempname[], *tempabbr[];
extern int NUM_TEMPLATES;
extern Boolean_t template[MAX_TEMPLATES][MX_FUNCGROUPS], template_complete[4][MAX_TEMPLATES], glob_rxlform;

static Boolean_t fgsel[2][4][MX_FUNCGROUPS],fgpossel[2][4][MAX_TEMPLATES+MX_FUNCGROUPS],illegal[MX_FUNCGROUPS],toggle_on=TRUE,
	IsSyntheme[MX_FUNCGROUPS];
static Widget tl, topform = (Widget) NULL, prel, conflict_msg[2], msgtext[2];
static int illegal_fg[NUMBER_OF_ILLEGAL_STRUCTURES] /* FROM COMPOK */ =
	{48, 246, 247, 248, 249, 250,      252, 253, 254, 255,
	 257, 258, 259, 260, 262, 263, 264, 265, 266, 267,
	 268, 269, 271, 272, 273, 274, 275, 270, 549};
/*
static char *tempabbr[NUM_TEMPLATES] = {"APT", "AST", "BNT", "BST", "EPT", "HST", "WST"},
*/
static char conflict_text[2][10000];
static React_Record_t *schema;
static React_Head_t *sch_head;
static U32_t glob_schn, *glob_roots, *glob_synthemes;

void Pretran_Update (Boolean_t);

void Refresh_Pretests (Boolean_t);
void toggle_fg (Widget, XtPointer, XtPointer);
void PreHelp_CB (Widget, XtPointer, XtPointer);
void PreButt_CB (Widget, XtPointer, XtPointer);
void Conflict_Msg_Dismiss_CB (Widget, XtPointer, XtPointer);
void Conflict_Msg_Explain_CB (Widget, XtPointer, XtPointer);
void fill_fglist(Widget);
void fg_select(Widget,int,char *, Boolean_t);
void fg_deselect(Widget,int);
void fg_pos_select(Widget,int,int, Boolean_t);
void fg_pos_deselect(Widget,int,Boolean_t);
void fg_deselect_all(Widget,Boolean_t);
void fg_restore (Widget);
void append_abbr (char *, int);
void conflict_message (int, int, int, int, Boolean_t);

void Pre_Form_Create (Widget top_level, U32_t schn, U32_t *roots, U32_t *synthemes)
{
	Boolean_t
		notall[MX_FUNCGROUPS], notany[MX_FUNCGROUPS],
		mustall[MX_FUNCGROUPS], mustany[MX_FUNCGROUPS],
		anycl, anycn, anyml, anymn;
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
		prew, button[6], txtform[2], helppb;
	static char
		*itemstr[6] = {"Reset", "Restore", "Edit Templates", "View Structures", "Exit; Update Buffer", "Cancel"};
#ifdef _CYGHAT_
        static Widget box[2];
Arg al[50];
int ac;
#endif

	disp = XtDisplay (tl=top_level);
	glob_schn=schn;
	glob_roots=roots;
	glob_synthemes=synthemes;

	if (topform == (Widget) NULL)
		{
		topform = XmCreateFormDialog (top_level, "Pretransform Editor", NULL, 0);
		label = XmStringCreateLocalized ("Pretransform Test Editor");

		XtVaSetValues (topform /*[which]*/,
			XmNresizePolicy, XmRESIZE_NONE,
			XmNdialogTitle, label,
			XmNwidth,1100,
			XmNheight,900,
			XmNfractionBase,    800,
XmNy, 25, /* for window managers that are too stupid to put the top border on the screen! */
			NULL);

		prew = XmCreateScrolledWindow(topform, "PreTransform Tests", NULL, 0);
		prel = XmCreateList(prew, "PreTransform Test List", NULL, 0);

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
        		/* XmNtitle,"PreTransform Tests", */ XmNx,0,XmNy,0,
        		XmNselectionPolicy,XmMULTIPLE_SELECT,NULL);

		XtVaSetValues(prew,
			XmNtopOffset,0,XmNtopAttachment,XmATTACH_FORM,
			XmNbottomPosition,775,XmNbottomAttachment,XmATTACH_POSITION,
			XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
			XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
			NULL);
        	XtAddCallback(prel,XmNmultipleSelectionCallback,toggle_fg,(XtPointer) disp);

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

		XtAddCallback (helppb, XmNactivateCallback, PreHelp_CB, (XtPointer) "pretest:Pretransform Test Editor");

		for (i=0; i<6; i++)
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
				XmNleftPosition, 125 * i + 51,
				XmNleftAttachment, XmATTACH_POSITION,
				XmNrightPosition, 125 * (i + 1) + 49,
				XmNrightAttachment, XmATTACH_POSITION,
				NULL);

			XmStringFree (label);

			XtAddCallback (button[i], XmNactivateCallback, PreButt_CB, (XtPointer) i);
			XtManageChild (button[i]);
			}

		for (i=0; i<2; i++)
			{
			conflict_msg[i] = XmCreateMessageDialog (top_level, "Message", NULL, 0);
			title=XmStringCreateLocalized("Dismiss");
			title2=XmStringCreateLocalized("Explain");
			label=XmStringCreateLocalized("");
			XtVaSetValues (conflict_msg[i],
				XmNmessageString, label,
				XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
				XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
				XmNokLabelString, title,
				XmNhelpLabelString, title2,
				NULL);
			XmStringFree(title);
			XmStringFree(title2);
			XmStringFree(label);

			txtform[i] = XmCreateForm (conflict_msg[i], "txtform", NULL, 0);
			XtVaSetValues (txtform[i],
				XmNtopOffset, 0,
				XmNtopAttachment, XmATTACH_WIDGET,
				XmNtopWidget, XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_MESSAGE_LABEL),
				XmNbottomOffset, 0,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget, XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_SEPARATOR),
				XmNleftOffset, 0,
				XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
				XmNleftWidget, XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_MESSAGE_LABEL),
				XmNrightOffset, 0,
				XmNrightAttachment, XmATTACH_OPPOSITE_WIDGET,
				XmNrightWidget, XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_MESSAGE_LABEL),
				NULL);
#ifdef _CYGHAT_
                        box[i] = XtVaCreateManagedWidget ("box",
                                xmRowColumnWidgetClass, txtform[i],
                                XmNleftAttachment,   XmATTACH_FORM,
                                XmNrightAttachment,  XmATTACH_FORM,
                                XmNbottomAttachment, XmATTACH_FORM,
                                XmNorientation,      XmHORIZONTAL,
                                XmNheight,           1,
                                NULL);

ac=0;
XtSetArg(al[ac],
				XmNheight, 250); ac++;
XtSetArg(al[ac],
				XmNwidth, 500); ac++;
XtSetArg(al[ac],
				XmNscrollingPolicy, XmAPPLICATION_DEFINED); ac++;
XtSetArg(al[ac],
				XmNscrollBarDisplayPolicy, XmAS_NEEDED); ac++;
XtSetArg(al[ac],
                                XmNscrollVertical, True); ac++;
XtSetArg(al[ac],
				XmNeditMode, XmMULTI_LINE_EDIT); ac++;
XtSetArg(al[ac],
				XmNeditable, False); ac++;
XtSetArg(al[ac],
				XmNautoShowCursorPosition, False); ac++;
XtSetArg(al[ac],
				XmNcursorPositionVisible, False); ac++;
XtSetArg(al[ac],
				XmNvalue, conflict_text); ac++;
XtSetArg(al[ac],
				XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
				XmNmarginWidth, 0); ac++;
XtSetArg(al[ac],
				XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
				XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK)); ac++;
			msgtext[i] = XmCreateScrolledText (txtform[i], "msgtxt", al, ac);

                        XtVaSetValues (XtParent (msgtext[i]),
				XmNtopOffset, 0,
				XmNtopAttachment, XmATTACH_FORM,
				XmNbottomOffset, 0,
				XmNbottomAttachment, XmATTACH_WIDGET,
				XmNbottomWidget, box[i],
				XmNleftOffset, 0,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightOffset, 0,
				XmNrightAttachment, XmATTACH_FORM,
				NULL);
#else

			msgtext[i] = XmCreateScrolledText (txtform[i], "msgtxt", NULL, 0);
			XtVaSetValues (msgtext[i],
				XmNheight, 250,
				XmNwidth, 500,
				XmNscrollingPolicy, XmAPPLICATION_DEFINED,
				XmNscrollBarDisplayPolicy, XmAS_NEEDED,
                                XmNscrollVertical, True,
				XmNeditMode, XmMULTI_LINE_EDIT,
				XmNeditable, False,
				XmNautoShowCursorPosition, False,
				XmNcursorPositionVisible, False,
				XmNvalue, conflict_text,
				XmNmarginHeight, 0,
				XmNmarginWidth, 0,
				XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
				XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
				XmNtopOffset, 0,
				XmNtopAttachment, XmATTACH_FORM,
				XmNbottomOffset, 0,
				XmNbottomAttachment, XmATTACH_FORM,
				XmNleftOffset, 0,
				XmNleftAttachment, XmATTACH_FORM,
				XmNrightOffset, 0,
				XmNrightAttachment, XmATTACH_FORM,
				NULL);
#endif

			XtManageChild (msgtext[i]);
			XtManageChild (txtform[i]);

			XtUnmanageChild (XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_CANCEL_BUTTON));
			XtAddCallback (XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_HELP_BUTTON), XmNactivateCallback,
				Conflict_Msg_Explain_CB, (XtPointer) i);
			XtAddCallback (XmMessageBoxGetChild (conflict_msg[i], XmDIALOG_OK_BUTTON), XmNactivateCallback,
				Conflict_Msg_Dismiss_CB, (XtPointer) NULL);
			XtUnmanageChild (conflict_msg[i]);
			}

		fill_fglist (prel);
		XtManageChild (prel);
		XtManageChild (prew);
		}

for (i = 0; i < 5; i += (i == 2 ? 2 : 1)) if (glob_rxlform) XtManageChild (button[i]);
else XtUnmanageChild (button[i]);

	XtManageChild (topform);

	fg_deselect_all (prel,TRUE);

	schema=React_Schema_Handle_Get(schn);
	sch_head=React_Head_Get(schema);

/*
        anycl=React_HeadFlags_CantAll_Get(sch_head);
*/
        anycn=React_HeadFlags_CantAny_Get(sch_head);
        anyml=React_HeadFlags_MustAll_Get(sch_head);
        anymn=React_HeadFlags_MustAny_Get(sch_head);

        for (i=0; i<MX_FUNCGROUPS; i++)
        {
/*
                notall[i]=anycl?React_CantAll_Get(schema, i):0;
*/
                notany[i]=anycn?React_CantAny_Get(schema, i):0;
                mustall[i]=anyml?React_MustAll_Get(schema, i):0;
                mustany[i]=anymn?React_MustAny_Get(schema, i):0;
		IsSyntheme[i]=FALSE;
        }
	for (i=0; i<MX_ROOTS && roots[i]!=REACT_NODE_INVALID; i++) IsSyntheme[synthemes[i]]=TRUE;

/*
	if (anycl) for (i=1; i<MX_FUNCGROUPS; i++) if (notall[i])
		fg_select(prel,i,"Cl",TRUE);
*/
	if (anycn) for (i=1; i<MX_FUNCGROUPS; i++) if (notany[i])
		fg_select(prel,i,"Cn",TRUE);
	if (anyml) for (i=1; i<MX_FUNCGROUPS; i++) if (mustall[i])
		fg_select(prel,i,"Ml",TRUE);
	if (anymn) for (i=1; i<MX_FUNCGROUPS; i++) if (mustany[i])
		fg_select(prel,i,"Mn",TRUE);

	for (i=0; i<NUM_TEMPLATES; i++) for (j=0; j<3; j++) if (template_complete[j][i]) fg_pos_select (prel, i+1, 35*j, FALSE);
}

void PreHelp_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild (topform);
  Help_CB (w, client_data, call_data);
}

void PreButt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
	int which, i;
	Boolean_t anycn, anyml, anymn;

	which = (int) client_data;

	switch (which)
	{
	case 0:
XtManageChild (topform);
		fg_deselect_all (prel,FALSE);
return;
		break;
	case 1:
XtManageChild (topform);
		fg_restore (prel);
return;
		break;
	case 2:
XtUnmanageChild (topform);
ETemp_Form_Create (tl);
		break;
	case 3:
		XtManageChild (topform);
		FGDraw_Show_Window (0, 0);
		break;
	case 4:
		anycn=anymn=anyml=FALSE;

        	for (i=0; i<MX_FUNCGROUPS; i++)
        	{
			if (fgsel[1][0][i])
			{
				anycn = TRUE;
                		React_CantAny_Put(schema, i, TRUE);
			}
			else React_CantAny_Put(schema, i, FALSE);

			if (fgsel[1][1][i])
			{
				anymn = TRUE;
                		React_MustAny_Put(schema, i, TRUE);
			}
			else React_MustAny_Put(schema, i, FALSE);

			if (fgsel[1][2][i])
			{
				anyml = TRUE;
                		React_MustAll_Put(schema, i, TRUE);
			}
			else React_MustAll_Put(schema, i, FALSE);
        	}

        	React_HeadFlags_CantAny_Put(sch_head, anycn);
        	React_HeadFlags_MustAny_Put(sch_head, anymn);
        	React_HeadFlags_MustAll_Put(sch_head, anyml);
		Pretran_Update (TRUE);
		fg_deselect_all (prel,FALSE);
		XtUnmanageChild (topform);
		break;
	case 5:
		Pretran_Update (FALSE);
		fg_deselect_all (prel,FALSE);
		XtUnmanageChild (topform);
		break;
	default:
		break;
	}
}

void Conflict_Msg_Explain_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
printf("This is the explanation for this phenomenon: Because ...\n");
}

void Conflict_Msg_Dismiss_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (XtParent(w));
}

void toggle_fg(Widget w, XtPointer client_data, XtPointer call_data)
{
        int *pos,count,rx,ry,x,y,i,j;
        unsigned mask;
	Display *disp;
        Window rw,cw;
        Boolean_t found;

	disp = (Display *) client_data;
        XmListGetSelectedPos(w,&pos,&count);
        if (!count)
        {
          XtFree ((char *) pos);
          fg_deselect_all (w, FALSE);
          return;
        }
        for (i=0; i<count; i++) if (!fgpossel[1][3][pos[i]])
        {
                XQueryPointer(disp,XtWindow(w),&rw,&cw,&rx,&ry,&x,&y,&mask);
                fg_pos_select(w,pos[i],x, TRUE);
        }
        for (i=1; i<=NUM_TEMPLATES+MX_FUNCGROUPS; i++) if (fgpossel[1][3][i] && toggle_on)
        {
                found=FALSE;
                for (j=0; j<count && !found; j++) found=pos[j]==i;
                if (!found) fg_pos_deselect(w,i,TRUE);
        }
	toggle_on=TRUE;
        XtFree ((char *) pos);
}

void fill_fglist(Widget w)
{
	XmString item;
	char itemstr[200];
	int i, pos;

	for (i=0; i<MX_FUNCGROUPS; i++) illegal[i]=FALSE;
	for (i=0; i<NUMBER_OF_ILLEGAL_STRUCTURES; i++) illegal[illegal_fg[i]]=TRUE;
	XmListSetAddMode(w,TRUE);
	for (i=0; i<NUM_TEMPLATES; i++)
	{
		sprintf(itemstr,"[Cn Mn]    %s Template [%s]",tempname[i],tempabbr[i]);
		item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	for (i=pos=1; i<fgend; i++) if (fgname[i]!=NULL)
	{
		if (illegal[i]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[i],i);
		else
		{
			sprintf(itemstr,"[Cn Mn Ml] %s (%d)",fgname[i],i);
			if (fgprsv[i]) strcat (itemstr, "[P]");
		}
		append_abbr (itemstr, i);
		item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	XmListSetAddMode(w,FALSE);
	XmListSetBottomPos(w,0);
	XmListSetPos(w,1);
}

void fg_select(Widget w,int num,char *type,Boolean_t initial)
{
	int i,j,pos,which;
	char itemstr[200],*typepos;
	XmString item;

	if (illegal[num])
	{
		fg_deselect (w, num);
		return;
	}
	if (strcmp (type, "Cn") == 0) which=0;
	else if (strcmp (type, "Mn") == 0) which=1;
	else if (strcmp (type, "Ml") == 0) which=2;
	for (i=pos=1; i<num; i++) if (fgname[i]!=NULL) pos++;
	for (j = initial ? 0: 1; j < 2; j++)
		fgsel[j][which][i]=fgsel[j][3][i]=fgpossel[j][which][pos+NUM_TEMPLATES]=fgpossel[j][3][pos+NUM_TEMPLATES]=TRUE;
	sprintf(itemstr,"[Cn Mn Ml] %s (%d)",fgname[i],i);
	if (fgprsv[i]) strcat (itemstr, "[P]");
	append_abbr (itemstr, i);
	typepos=strstr(itemstr,type);
	strncpy(itemstr,"          ",10);
	strncpy(typepos,type,2);

	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos+NUM_TEMPLATES);
	XmStringFree(item);
	XmListSelectPos(w,pos+NUM_TEMPLATES,FALSE);
}

void fg_deselect(Widget w,int num)
{
	int i,j,pos;
	char itemstr[200];
	XmString item;

	for (i=pos=1; i<num; i++) if (fgname[i]!=NULL) pos++;
	for (j=0; j<4; j++)
		fgsel[1][j][i]=fgpossel[1][j][pos+NUM_TEMPLATES]=FALSE;

	if (illegal[i]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[i],i);
	else
	{
		sprintf(itemstr,"[Cn Mn Ml] %s (%d)",fgname[i],i);
		if (fgprsv[i]) strcat (itemstr, "[P]");
	}

	append_abbr (itemstr, i);

	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos+NUM_TEMPLATES);
	XmStringFree(item);
#ifndef _CYGWIN_
	XmListDeselectPos(w,pos+NUM_TEMPLATES);
#endif
}

void fg_deselect_all(Widget w,Boolean_t initial)
{
	int i,j;

	if (initial) for (j=0; j<2; j++)
		{
		for (i=1; i<=NUM_TEMPLATES; i++)
			fgpossel[j][0][i]=fgpossel[j][1][i]=fgpossel[j][2][i]=fgpossel[j][3][i]=FALSE;
		for (; i<NUM_TEMPLATES+MX_FUNCGROUPS; i++)
			fgpossel[j][0][i]=fgpossel[j][1][i]=fgpossel[j][2][i]=fgpossel[j][3][i]=FALSE;
		for (i=1; i<MX_FUNCGROUPS; i++) fgsel[j][0][i]=fgsel[j][1][i]=fgsel[j][2][i]=fgsel[j][3][i]=FALSE;
		}
	else
		{
		for (i=1; i<=NUM_TEMPLATES; i++) if (fgpossel[1][3][i]) fg_pos_deselect (w, i, FALSE);
		for (i=1; i<MX_FUNCGROUPS; i++) if (fgsel[1][3][i]) fg_deselect(w,i);
		}
}

void fg_restore(Widget w)
{
	int i,j,k;
	static char *type[] = {"Cn", /*"Cl",*/ "Mn","Ml"};

	fg_deselect_all(w,FALSE);
	for (i=1; i<=NUM_TEMPLATES; i++) if (fgpossel[0][3][i])
	{
		for (j=0, k=-1; j<3; j++) if (fgpossel[0][j][i]) k=j;
		fg_pos_select (w,i,k, TRUE);
	}
	else if (fgpossel[1][3][i]) fg_pos_deselect (w,i,FALSE);
	for (i=1; i<MX_FUNCGROUPS; i++) if (fgsel[0][3][i])
	{
		for (j=0, k=-1; j<3; j++) if (fgsel[0][j][i]) k=j;
		fg_select (w,i,type[k],FALSE);
	}
	else if (fgsel[1][3][i]) fg_deselect (w,i);
}

void fg_pos_select(Widget w,int num,int x, Boolean_t multiple)
{
	int i,j,pos,xinx,conflict_num;
	char itemstr[200],*typepos;
	static char *type[]={"Cn", /*"Cl",*/ "Mn","Ml"};
	XmString item;
	Boolean_t conflict;

	if (x<35) xinx=0;
	else if (x<70) xinx=1;
	else if (x<105 && num>NUM_TEMPLATES) xinx=2;
	else
	{
		fg_pos_deselect (w, num, FALSE);
		return;
	}
	if (num<=NUM_TEMPLATES)
	{
		toggle_on=!multiple;
		conflict=FALSE;
		fgpossel[1][xinx][num]=fgpossel[1][3][num]=TRUE;
		sprintf(itemstr,"[Cn Mn]    %s Template [%s]",tempname[num-1],tempabbr[num-1]);
		if (multiple) for (i=0; i<MX_FUNCGROUPS; i++) if (template[num-1][i])
		{
			for (j=0, conflict_num=-1; j<3 && conflict_num<0; j++) if (fgsel[1][j][i]) conflict_num=j;
			if (illegal[i])
			{
				if (conflict_num<0) conflict_num=ILLEGAL_FLAG;
				else
				{
					conflict=TRUE;
					conflict_message (num - 1, xinx, i, ILLEGAL_FLAG, FALSE);
				}
			}
			if (xinx == 0 && IsSyntheme[i])
			{
				if (conflict_num<0) conflict_num=SYNTHEME_FLAG;
				else
				{
					conflict=TRUE;
					conflict_message (num - 1, xinx, i, SYNTHEME_FLAG, FALSE);
				}
			}
			if (conflict_num<0) fg_select(w,i,type[xinx],FALSE);
			else if (conflict_num!=xinx)
			{
				conflict=TRUE;
				conflict_message (num - 1, xinx, i, conflict_num, FALSE);
			}
		}
		pos=num-NUM_TEMPLATES;
		if (conflict)
		{
			conflict_message (0,0,0,0,TRUE);
			fg_pos_deselect (w, num, FALSE);
			return;
		}
	}
	else
	{
		for (i=pos=1; pos<=num-NUM_TEMPLATES; i++) if (fgname[i]!=NULL) pos++;
		pos--;
		i--;

if (illegal[i])
{
XmListDeselectPos(w,num);
return;
}
		fgsel[1][xinx][i]=fgsel[1][3][i]=fgpossel[1][xinx][pos+NUM_TEMPLATES]=fgpossel[1][3][pos+NUM_TEMPLATES]=TRUE;
/*
		if (illegal[i]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[i],i);
		else
		{
*/
			sprintf(itemstr,"[Cn Mn Ml] %s (%d)",fgname[i],i);
			if (fgprsv[i]) strcat (itemstr, "[P]");
/*
		}
*/
		append_abbr (itemstr, i);
	}
	typepos=strstr(itemstr,type[xinx]);
	strncpy(itemstr,"          ",10);
	strncpy(typepos,type[xinx],2);
	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos+NUM_TEMPLATES);
	XmStringFree(item);
	XmListSelectPos(w,pos+NUM_TEMPLATES,FALSE);
}

void fg_pos_deselect(Widget w,int num,Boolean_t multiple)
{
	int i,j,pos,xinx;
	char itemstr[200];
	XmString item;

	if (num<=NUM_TEMPLATES)
	{
		if (multiple)
		{
			toggle_on=FALSE;
			if (fgpossel[1][0][num]) xinx=0;
			else xinx=1;

			for (i=0; i<MX_FUNCGROUPS; i++) if (fgsel[1][xinx][i] && template[num-1][i]) fg_deselect(w,i);
			else if (template[num-1][i]) printf("fgsel[1][xinx][%d]=%d\n",i,fgsel[1][xinx][i]);
		}
		for (j=0; j<4; j++)
			fgpossel[1][j][num]=FALSE;
		sprintf(itemstr,"[Cn Mn]    %s Template [%s]",tempname[num-1],tempabbr[num-1]);
		pos=num-NUM_TEMPLATES;
	}
	else
	{
		for (i=pos=1; pos<=num-NUM_TEMPLATES; i++) if (fgname[i]!=NULL) pos++;
		pos--;
		i--;
		for (j=0; j<NUM_TEMPLATES; j++) if (template[j][i] && fgpossel[1][3][j+1])
			fg_pos_deselect (w, j+1, FALSE);
		for (j=0; j<4; j++)
			fgsel[1][j][i]=fgpossel[1][j][pos+NUM_TEMPLATES]=FALSE;
		if (illegal[i]) sprintf(itemstr,"*ILLEGAL*  %s (%d)",fgname[i],i);
		else
		{
			sprintf(itemstr,"[Cn Mn Ml] %s (%d)",fgname[i],i);
			if (fgprsv[i]) strcat (itemstr, "[P]");
		}
		append_abbr (itemstr, i);
	}
	item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
	XmListReplaceItemsPos(w,&item,1,pos+NUM_TEMPLATES);
	XmStringFree(item);
	XmListDeselectPos(w,pos+NUM_TEMPLATES);
}

void append_abbr (char *str, int fgn)
{
	int i;
	char astr[10];

	for (i=0; i<NUM_TEMPLATES; i++) if (template[i][fgn])
	{
		sprintf(astr," [%s]",tempabbr[i]);
		strcat(str,astr);
	}
}

void conflict_message (int temp, int type, int fg, int type2, Boolean_t flush)
{
	static char *typename[] = {"CAN'T HAVE ANY", "MUST HAVE ANY", "MUST HAVE ALL"};
	static int num_bypassed=0,num_conflicts=0,
		temp_array[100],type_array[100],fg_array[100],type2_array[100],conflict_array[100];
	char msg[10000],msg2[100];
	int i, j;
	XmString label;

	if (flush)
		{
		sprintf(msg, "%s %s Template bypassed the following existing tests:",
			typename[type_array[0]],tempname[temp_array[0]]);
		conflict_text[0][0]=0;
		for (i=0; i<num_bypassed; i++)
			if ((type_array[0] == 0 && (type2_array[i] == 2 || type2_array[i] == SYNTHEME_FLAG)) ||
			(type_array[0] != 0 && type2_array[i] == ILLEGAL_FLAG))
			conflict_array[num_conflicts++] = i;
		else
			{
			if (type2_array[i] == ILLEGAL_FLAG) sprintf(msg2, "ILLEGAL FG %s (%d)\n", fgname[fg_array[i]], fg_array[i]);
			else sprintf(msg2, "%s %s (%d)\n", typename[type2_array[i]], fgname[fg_array[i]], fg_array[i]);
			strcat (conflict_text[0],msg2);
			}
		num_bypassed -= num_conflicts;

		label=XmStringCreateLocalized(msg);
		XtVaSetValues (conflict_msg[0],
			XmNmessageString, label,
			NULL);
		XmStringFree(label);
		XmTextSetString (msgtext[0], conflict_text[0]);
		if (num_bypassed != 0)
			XtManageChild (conflict_msg[0]);
		num_bypassed=0;
		if (num_conflicts != 0)
			{
			sprintf(msg, "%s %s Template conflicts with the following existing tests:",
				typename[type_array[0]],tempname[temp_array[0]]);
			conflict_text[1][0]=0;
			for (j=0; j<num_conflicts; j++)
				{
				i=conflict_array[j];
				if (type2_array[i] == ILLEGAL_FLAG)
					sprintf (msg2, "ILLEGAL FG %s (%d)\n", fgname[fg_array[i]],fg_array[i]);
				else if (type2_array[i] == SYNTHEME_FLAG)
					sprintf (msg2, "SYNTHEME %s (%d)\n", fgname[fg_array[i]],fg_array[i]);
				else sprintf(msg2, "%s %s (%d)\n", typename[type2_array[i]], fgname[fg_array[i]],fg_array[i]);
				strcat (conflict_text[1],msg2);
				}
  			label=XmStringCreateLocalized(msg);
  			XtVaSetValues (conflict_msg[1],
        			XmNmessageString, label,
        			NULL);
  			XmStringFree(label);
			XmTextSetString (msgtext[1], conflict_text[1]);
			num_conflicts=0;
  			XtManageChild (conflict_msg[1]);
			}
		}
	else
		{
		if (num_bypassed==100) return;
		temp_array[num_bypassed]=temp;
		type_array[num_bypassed]=type;
		fg_array[num_bypassed]=fg;
		type2_array[num_bypassed++]=type2;
		}
}

void Refresh_Pretests (Boolean_t modified)
{
  if (modified)
	{
	init_templates ();
	XmListDeleteAllItems(prel);
	fill_fglist(prel);
	Pre_Form_Create (tl,glob_schn,glob_roots,glob_synthemes);
	}
  else XtManageChild (topform);
}
