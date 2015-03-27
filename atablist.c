/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     ATABLIST.C
*
*    This module defines the routines needed to display the DIST ATABLE
*    attributes from within the Synchem GUI's.
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

extern char *atable[][2];

static Widget topform = (Widget) NULL, atlist;
static void (*at_selected)(int);
static int at_vpos[1000];
static int sel_vpos=0;

void FGDraw_Show_Window (int, int);

void Select_AT_CB (Widget, XtPointer, XtPointer);
void Draw_AT_CB (Widget, XtPointer, XtPointer);
void init_atlist (Widget);
void select_at (Widget, int);
int pos2atn (int);

void AT_Form_Close ()
{
if (topform==(Widget)NULL) return;
XtUnmanageChild (topform);
}

void AT_Form_Create (Widget top_level, int initial_selection, void (*handler)(int))
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
	topform = XmCreateFormDialog (top_level, "Directed Attribute List", NULL, 0);
  label = XmStringCreateLocalized ("Directed Attribute List");

  XtVaSetValues (topform,
    XmNresizePolicy, XmRESIZE_NONE,
    XmNdialogTitle, label,
    XmNwidth,800,
    XmNheight,700,
    XmNfractionBase,    800,
    NULL);

  XmStringFree (label);

	listwin = XtVaCreateWidget("Directed Attributes",
		xmScrolledWindowWidgetClass, topform,
		XmNtopOffset,0,XmNtopAttachment,XmATTACH_FORM,
		XmNbottomPosition,775,XmNbottomAttachment,XmATTACH_POSITION,
		XmNleftOffset,0,XmNleftAttachment,XmATTACH_FORM,
		XmNrightOffset,0,XmNrightAttachment,XmATTACH_FORM,
		NULL);

        label = XmStringCreateLocalized ("Show Attribute Drawings");

        drawpb = XtVaCreateManagedWidget("Draw AT",
                xmPushButtonGadgetClass, topform,
                XmNlabelString, label,
                XmNtopOffset, 0, XmNtopAttachment, XmATTACH_WIDGET, XmNtopWidget, listwin,
                XmNbottomOffset, 0, XmNbottomAttachment, XmATTACH_FORM,
                XmNleftPosition, 300, XmNleftAttachment, XmATTACH_POSITION,
                XmNrightPosition, 500, XmNrightAttachment, XmATTACH_POSITION,
                NULL);

        XmStringFree (label);
        XtAddCallback (drawpb, XmNactivateCallback, Draw_AT_CB, (XtPointer) NULL);

	atlist = XmCreateList(listwin, "Directed Attribute List", NULL, 0);

	disp = XtDisplay (topform);

	courb18 = XmFontListEntryLoad (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
		XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
	helv18 = XmFontListEntryLoad (disp, "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1",
		XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);

	flcob18=XmFontListAppendEntry (NULL, courb18);
	flhv18 = XmFontListAppendEntry (NULL, helv18);

	XtVaSetValues(atlist,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        	SAR_CLRI_WHITE),
        	XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        	SAR_CLRI_BLACK),
        	XmNfontList,flcob18,
        	XmNtitle,"Functional Groups",XmNx,0,XmNy,0,
        	XmNselectionPolicy,XmSINGLE_SELECT,
		NULL);

        XtAddCallback(atlist,XmNsingleSelectionCallback,Select_AT_CB,(XtPointer) NULL);

	init_atlist (atlist);
	XtManageChild (atlist);
	XtManageChild (listwin);
}
	at_selected = handler;
	select_at (atlist, initial_selection);

	XtManageChild (topform);
}

void Draw_AT_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtManageChild (topform);
  FGDraw_Show_Window (1, sel_vpos);
}

void Select_AT_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
	int *pos, count, selected_at;

        XmListGetSelectedPos(w,&pos,&count);
        if (!count)
        {
          XtFree ((char *) pos);
          return;
        }

	selected_at = pos2atn (pos[0]);

	XtUnmanageChild (topform);

	at_selected (selected_at);

        XtFree ((char *) pos);
}

void init_atlist(Widget w)
{
	XmString item;
	char itemstr[200];
	int i, pos;
	FILE *f;

	f=fopen(FCB_SEQDIR_FNGP ("/atable.vpos"),"r");
	XmListSetAddMode(w,TRUE);
        item = XmStringCreate ("SUBSTITUENT WITH SLING ... ",XmSTRING_DEFAULT_CHARSET);
	XmListAddItem(w,item,0);
	XmStringFree(item);
	for (i=pos=1; atable[i-1][0] != NULL; i++)
	{
	fscanf(f,"%d",at_vpos+i-1);
		sprintf (itemstr, "%3d: %s", i, atable[i-1][1]);
		item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
		XmListAddItem(w,item,0);
		XmStringFree(item);
	}
	XmListSetAddMode(w,FALSE);
	XmListSetBottomPos(w,0);
	XmListSetPos(w,1);
	fclose(f);
}

void select_at (Widget w, int num)
{
	XmListSelectPos(w,num+1,FALSE);
	XmListSetPos(w,num+1);
	if (num==0) sel_vpos=0;
	else sel_vpos=at_vpos[num-1];
}

int pos2atn (int pos)
{
	return(pos-1);
}
