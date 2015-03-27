/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     FGLIST.C
*
*    Creates a selection list of all attributes in the functional group table
*    for use within various posttransform condition editing modules.
*
*  Creation Date:
*
*    16-Feb-2000
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

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/List.h>
#include <Xm/PushBG.h>
#include <Xm/ScrolledW.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

static Widget topform = (Widget) NULL, fglist;
static void (*fg_selected)(int);
static int fg_vpos[1000];
static int sel_vpos=0;

void FGDraw_Show_Window (int, int);

void Select_FG_CB (Widget, XtPointer, XtPointer);
void Draw_FG_CB (Widget, XtPointer, XtPointer);
void init_fglist (Widget);
void select_fg (Widget, int);
int pos2fgn (int);

void FG_Form_Close ()
{
if (topform==(Widget)NULL) return;
XtUnmanageChild (topform);
}

void FG_Form_Create (Widget top_level, int initial_selection, void (*handler)(int))
{
	XmFontListEntry
		courb18, helv18;
	XmFontList
		flcob18, flhv18;
	int
		i;
	int slider_val,
	slider_size,
	slider_inc,
	page_inc,
	slider_min,
	slider_max;

	XmString
		label, title;
	Widget
		listwin, drawpb;
	Display
		*disp;

if (topform == (Widget) NULL)
{
	topform = XmCreateFormDialog (top_level, "Functional Group List", NULL, 0);
  label = XmStringCreateLocalized ("Functional Group List");

  XtVaSetValues (topform,
    XmNresizePolicy, XmRESIZE_NONE,
    XmNdialogTitle, label,
    XmNwidth,800,
    XmNheight,700,
    XmNfractionBase,    800,
    NULL);

  XmStringFree (label);

	listwin = XtVaCreateWidget("Functional Groups",
		xmScrolledWindowWidgetClass, topform,
		XmNtopOffset,0,XmNtopAttachment,XmATTACH_FORM,
		XmNbottomPosition,775,XmNbottomAttachment,XmATTACH_POSITION,
		XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
		XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
		NULL);

        label = XmStringCreateLocalized ("Show FG Drawings");

        drawpb = XtVaCreateManagedWidget("Draw FGs",
                xmPushButtonGadgetClass, topform,
                XmNlabelString, label,
                XmNtopOffset, 0, XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, listwin,
                XmNbottomOffset, 0, XmNbottomAttachment, XmATTACH_FORM,
                XmNleftPosition, 300, XmNleftAttachment, XmATTACH_POSITION,
                XmNrightPosition, 500, XmNrightAttachment, XmATTACH_POSITION,
                NULL);

        XmStringFree (label);
        XtAddCallback (drawpb, XmNactivateCallback, Draw_FG_CB, (XtPointer) NULL);

	fglist = XmCreateList(listwin, "Functional Group List", NULL, 0);

	disp = XtDisplay (topform);

	courb18 = XmFontListEntryLoad (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
		XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
	helv18 = XmFontListEntryLoad (disp, "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
		XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);

	flcob18=XmFontListAppendEntry (NULL, courb18);
	flhv18 = XmFontListAppendEntry (NULL, helv18);

	XtVaSetValues(fglist,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        	SAR_CLRI_WHITE),
        	XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        	SAR_CLRI_BLACK),
        	XmNfontList,flcob18,
        	XmNtitle,"Functional Groups",XmNx,0,XmNy,0,
        	XmNselectionPolicy,XmSINGLE_SELECT,
		NULL);

        XtAddCallback(fglist,XmNsingleSelectionCallback,Select_FG_CB,(XtPointer) NULL);

	init_fglist (fglist);
	XtManageChild (fglist);
	XtManageChild (listwin);
}
	fg_selected = handler;
	select_fg (fglist, initial_selection);

	XtManageChild (topform);
}

void Draw_FG_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild (topform);
  FGDraw_Show_Window (0, sel_vpos);
}

void Select_FG_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
	int *pos, count, selected_fg;

        XmListGetSelectedPos(w,&pos,&count);
        if (!count)
        {
          XtFree ((char *) pos);
          return;
        }

	selected_fg = pos2fgn (pos[0]);

	XtUnmanageChild (topform);

	fg_selected (selected_fg);

        XtFree ((char *) pos);
}

void init_fglist(Widget w)
{
	XmString item;
	char itemstr[200];
	int i, pos, *fgvp;
        FILE *f;

f=fopen(FCB_SEQDIR_FNGP ("/fgdata.vpos"),"r");
fgvp=fg_vpos;
	XmListSetAddMode(w,TRUE);
	for (i=pos=1; i<fgend; i++) if (fgname[i]!=NULL)
	{
fscanf(f,"%d",fgvp);
fgvp++;
		sprintf (itemstr, "%3d: %s", i, fgname[i]);
		item=XmStringCreate(itemstr /*fgname[i]*/,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	XmListSetAddMode(w,FALSE);
	XmListSetBottomPos(w,0);
	XmListSetPos(w,1);

fclose(f);
}

void select_fg (Widget w, int num)
{
	int i,j,pos,which;
	char itemstr[200],*typepos;
	XmString item;

	for (i=pos=1; i<num; i++) if (fgname[i]!=NULL) pos++;
	XmListSelectPos(w,pos,FALSE);
	XmListSetPos(w,pos);
sel_vpos=fg_vpos[pos-1];
}

int pos2fgn (int pos)
{
	int i,tpos;

	for (i=tpos=1; tpos<=pos; i++) if (fgname[i]!=NULL) tpos++;
	i--;
	return(i);
}
