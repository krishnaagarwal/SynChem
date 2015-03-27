/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              RXLFORM.C
*
*    This module provides for the display and editing of the entire reaction
*    library, providing the functions called by RXLEDIT.
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

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <X11/Xutil.h>
#include <Xm/Xm.h>
#include <Xm/Frame.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/List.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/ScrolledW.h>
#include <Xm/DrawingA.h>
#include <Xm/MainW.h>
#include <Xm/PanedW.h>
#include <Xm/RowColumn.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/ScrollBar.h>
#include <Xm/Separator.h>

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

#include "rxnpatt_draw.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
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

#ifndef _H_LOGIN_
#include "login.h"
#endif

#ifndef _H_SCHFORM_
#include "schform.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#define _GLOBAL_DEF_
#include "rxlform.h"
#undef _GLOBAL_DEF_

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

#define _CYGWIN_    //kka: needed for lesstif
void IsThere (Widget, XtAppContext, int *, int *, Boolean_t, Boolean_t);
void move_schema ();
void sch_move_CB (Widget, XtPointer, XtPointer);
void move_schema_cont (Widget, XtPointer, XtPointer);
void Find_Sch (int);

static Boolean_t glob_sch_changed = FALSE;
static Widget sch_move_msg;
static int glob_sel_sch, taskmap[19] = {0,1,2,18,3,4,5,6,7,8,9,10,11,12,13,14,15,16,17}; /* Saves rewriting entire sections
                                                                                            (thanks to Motif's "support" for
                                                                                            "simple" pulldown menus) */
#ifdef _CYGWIN_
static Display *to_disp = NULL;
static Window to_win;

void to_CB (Widget w, XtPointer p1, XtPointer p2)
{
  if (to_disp != NULL)
    XMapWindow (to_disp, to_win);
}
#endif

static int current_cmenu_choice = 1, menu_y_adj = 0, sc_ht, sc_wd;

void MSB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  static int menu_y_offset = 30;
  static Boolean_t first_popup = TRUE, busy=FALSE;
  int x,y, i;
  char wname[100];
  XtWidgetGeometry geom;

  if (client_data == NULL)
  {
    if (busy) return;
    if (first_popup)
    {
      first_popup=FALSE;
      busy=TRUE;
      XtPopup(w,XtGrabNone);
      busy=FALSE;
      XtQueryGeometry(w,NULL,&geom);
      if (geom.height>sc_ht)
      {
        XtManageChild(XtNameToWidget(w,"ChapMenu.button_0"));
        XtManageChild(XtNameToWidget(w,"ChapMenu.button_42"));
        menu_y_adj=43*geom.height/41-sc_ht+60;
      }
      else menu_y_offset=0;
      XtPopdown(w);
    }
    XtVaSetValues(w,XmNy,menu_y_offset,NULL);
    return;
  }
  menu_y_offset += (int) client_data * menu_y_adj;
  sprintf(wname,"*popup_ChapMenu.ChapMenu.button_%d", current_cmenu_choice);
  XtVaSetValues(XtNameToWidget(tl,"*LibChapFm.ChapMenu"),
    XmNmenuHistory,XtNameToWidget(tl,wname),NULL);
  XtVaSetValues(XtNameToWidget(tl,"*popup_ChapMenu"),
    XmNmenuHistory,XtNameToWidget(tl,wname),NULL);
  XtPopup(XtNameToWidget(tl,"*popup_ChapMenu"),XtGrabNonexclusive);
}

void RxlEdit_Create_Form (Widget top_level, XtAppContext schlContext, int sel_sch)
{
	ScreenAttr_t     *sca_p;
        XmFontList flhv14, flhv16, tflhv20,
          flco16, tflco20, fl6x10;
/*
	XmFontList flhv14,flhv18,flco18,fl6x10;
	XmFontListEntry helv14,helv18,cour18,fs6x10;
*/
        XmFontListEntry helv12, helv14, helv16, helv20, helv22,
          cour12, cour14, cour16, cour20;
	Widget frame, topform, schlist_form, schdisp_form, liblistwin, chaplistwin, schlistwin, textwin, drawwin[2],
		schform,toppaned,ldummy,cdummy,detailpb,helppb,srch_txt,srch_pb,srch_lbl,
pb;
	XtWidgetGeometry geom;
	int i;
	char itemstr[500];
char pbname[64];
Dimension w,h;
/*
	XmString label, labels[41];
*/
XmString label, labels[43];
#ifdef _CYGHAT_
        Widget box;
Arg al[50];
int ac;
#endif

#ifdef _DEBUG_
printf("RxlEdit_Create_From entered\n");
#endif
  tl = top_level;
  schlC = schlContext;
  glob_sel_sch = sel_sch;

  frame = XtVaCreateManagedWidget ("RxlEditFr",
    xmMainWindowWidgetClass,  top_level,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    NULL);
#ifdef _DEBUG_
printf("frame created\n");
#endif

  topform = XtVaCreateWidget ("form", xmFormWidgetClass, frame, NULL);

  toppaned = XtVaCreateWidget ("paned", xmPanedWindowWidgetClass, topform, NULL);

  libchap_form = XtVaCreateWidget ("LibChapFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     False,
    XmNpositionIndex,   0,
/*
    XmNpaneMinimum,     60,
    XmNpaneMaximum,     60,
*/
XmNpaneMinimum, 80,
XmNpaneMaximum, 80,
    XmNfractionBase,    800,
    NULL);
#ifdef _DEBUG_
printf("libchap_form created\n");
#endif

  schform = XtVaCreateWidget ("SchFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     True,
    XmNpaneMaximum,     1600,
    XmNfractionBase,    800,
    NULL);
#ifdef _DEBUG_
printf("schform created\n");
#endif

  schlist_form = XtVaCreateWidget ("RxlListFm",
    xmFormWidgetClass,  schform,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    XmNfractionBase,    800,
    NULL);
#ifdef _DEBUG_
printf("schlist_form created\n");
#endif

  schdisp_form = XtVaCreateWidget ("RxlDispFm",
    xmFormWidgetClass,  schform,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    XmNfractionBase,    800,
    NULL);
#ifdef _DEBUG_
printf("schdisp_form created\n");
#endif

  XtVaSetValues (toppaned,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (schlist_form,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightPosition,    500,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (schdisp_form,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       schlist_form,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
#ifdef _DEBUG_
printf("values set for tp,slf,sdf\n");
#endif
//printf("kka1.1:in rxlform\n");
  helv12 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-12-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  helv14 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  helv16 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-16-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
/*
  helv10 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-12-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  helv18 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
*/
  helv20 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-20-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  helv22 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-22-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
/*
  cour18 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
*/
  cour12 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-12-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  cour14 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  cour16 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-16-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  cour20 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-20-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

/*
  flhv10 = XmFontListAppendEntry (NULL, helv10);
  flhv18 = XmFontListAppendEntry (NULL, helv18);
*/
  tflhv20 = XmFontListAppendEntry (NULL, helv20);
/*
  flco18 = XmFontListAppendEntry (NULL, cour18);
*/
  tflco20 = XmFontListAppendEntry (NULL, cour20);
  fl6x10 = XmFontListAppendEntry (NULL, cour14);
#ifdef _DEBUG_
printf("fontlist entries loaded\n");
#endif

sc_wd = WidthOfScreen(XtScreen(tl));
sc_ht = HeightOfScreen(XtScreen(tl));
if (sc_wd<1200)
{
  flhv14 = XmFontListAppendEntry (NULL, helv12);
  flhv16 = XmFontListAppendEntry (NULL, helv12);
  flco16 = XmFontListAppendEntry (NULL, cour12);
}
else
{
  label=XmStringCreateLocalized("ABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXYABCDEFGHIJKLMNOPQRSTUVWXY");
  if (sc_wd<1400)
  {
    flhv14=XmFontListAppendEntry(NULL, helv14);
    flhv16=XmFontListAppendEntry(NULL, helv16);
  }
  else
  {
    flhv14=XmFontListAppendEntry(NULL, helv16);
    XmStringExtent(tflhv20,label,&w,&h);
    flhv16=XmFontListAppendEntry(NULL, w>3350 || h>22 ? helv20 : helv22);
  }
  XmStringExtent(tflco20,label,&w,&h);
  if (w > 2800 || h > 18) flco16 = XmFontListAppendEntry(NULL, sc_wd<1400 ? cour14 : cour16);
  else flco16 = XmFontListAppendEntry(NULL, sc_wd<1400 ? cour16 : cour20);
  XmStringFree(label);
}
#ifdef _DEBUG_
printf("fontlists appended\n");
#endif

  ldummy = XmVaCreateSimplePulldownMenu (libchap_form, "_lpulldown", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  label = XmStringCreateLocalized("Library");
  labels[0] = XmStringCreateLocalized("All");

  for (i = 0; i < 3; i++)
    {
    sprintf (itemstr, "%d: %s", i + 1, libname[i]);
    labels[i + 1] = XmStringCreateLocalized (itemstr);
    }

  labels[4] = XmStringCreateLocalized("Exit");

  liblistw = XmVaCreateSimpleOptionMenu (libchap_form, "LibMenu", label, '\0', 0, Lib_CB,
	XmNsubMenuId, ldummy,
	XmNbuttonFontList, flhv14,
        XmNorientation, XmVERTICAL,
	XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmVaPUSHBUTTON, labels[0], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[1], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[2], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[3], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[4], '\0', NULL, NULL,
        NULL);
  XmStringFree (label);
  XmStringFree (labels[0]);
  XmStringFree (labels[1]);
  XmStringFree (labels[2]);
  XmStringFree (labels[3]);
  XmStringFree (labels[4]);
#ifdef _DEBUG_
printf("liblistw created\n");
#endif
//printf("kka1.2:in rxlform\n");
  cdummy = XmVaCreateSimplePulldownMenu (libchap_form, "_cpulldown", 0, NULL, XmNbuttonFontList, flhv14, NULL);
//printf("kka1.53:in rxlform\n");
  for (i=0; i<125; i++) itemstr[i]=' ';
  itemstr[i]=0;
//printf("kka1.54:in rxlform\n");
  for (i=0; i<43; i++)
    labels[i]=XmStringCreateLocalized(itemstr);
//printf("kka1.55:in rxlform\n");
  label = XmStringCreateLocalized("Chapter");
/*
  chaplistw = XmVaCreateSimpleOptionMenu (libchap_form, "ChapMenu", label, '\0', 0, Chap_CB,
*/
  chaplistw = XmVaCreateSimpleOptionMenu (libchap_form, "ChapMenu", label, '\0', 1, Chap_CB,
	XmNsubMenuId, cdummy,
	XmNbuttonFontList, flhv14,
        XmNorientation, XmVERTICAL,
	XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmVaPUSHBUTTON, labels[0], '\0', NULL, NULL,
        XmVaDOUBLE_SEPARATOR,
        XmVaPUSHBUTTON, labels[41], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[1], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[2], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[3], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[4], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[5], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[6], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[7], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[8], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[9], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[10], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[11], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[12], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[13], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[14], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[15], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[16], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[17], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[18], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[19], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[20], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[21], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[22], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[23], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[24], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[25], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[26], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[27], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[28], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[29], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[30], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[31], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[32], '\0', NULL, NULL,

        XmVaPUSHBUTTON, labels[33], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[34], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[35], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[36], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[37], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[38], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[39], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[40], '\0', NULL, NULL,
      XmVaDOUBLE_SEPARATOR,
      XmVaPUSHBUTTON, labels[42], '\0', NULL, NULL,
        NULL);
//printf("kka1.6:in rxlform\n");
  XmStringFree (label);
#ifdef _DEBUG_
printf("chaplistw created\n");
#endif

  for (i = 0; i < 43; i++)
    {
#ifdef _DEBUG_
printf("i=%d start\n");
#endif
#ifdef _CYGWIN_
/* lesstif uses a different opaque naming convention from the equally opaque one used by OSF - read their minds! */
    sprintf (pbname,"ChapMenu.popup_ChapMenu.ChapMenu.button_%d",i);
#else
    sprintf (pbname, "popup_ChapMenu.ChapMenu.button_%d", i);
#endif
    pb = XtNameToWidget (libchap_form, pbname);
//printf("kka1.61:in rxlform%d\n",i);
#ifdef _DEBUG_
printf("pb=%p\n",pb);

#endif
//int tmp;
//scanf("%d",&tmp);
    XtRemoveAllCallbacks (pb, XmNactivateCallback);
//printf("kka1.615:in rxlform\n");
    if (i==0 || i==42)
      XtAddCallback (pb, XmNactivateCallback, MSB_CB, (XtPointer) (i==0?-1:1));
    else XtAddCallback (pb, XmNactivateCallback, Chap_CB, (XtPointer) (i-1));
#ifdef _DEBUG_
printf("i=%d end\n");
#endif
    }
#ifdef _DEBUG_
printf("button callbacks changed\n");
#endif
//printf("kka1.62:in rxlform\n");

/*
  for (i=0; i<41; i++)
*/
  for (i=0; i<43; i++)
	XmStringFree(labels[i]);

XtAddCallback (XtNameToWidget(tl,"*popup_ChapMenu"), XmNpopupCallback, MSB_CB,
  (XtPointer) NULL);
//printf("kka1.7:in rxlform\n");
/*
sc_wd = WidthOfScreen(XtScreen(tl));
sc_ht = HeightOfScreen(XtScreen(tl));
*/

  detailpb=XmCreatePushButton(libchap_form,"DetailPB",NULL,0);
  label=XmStringCreateLocalized(" Schema Detail ");
  XtVaSetValues(detailpb,
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNlabelString, label,
/*
	XmNfontList, flhv18,
*/
	XmNfontList, flhv16,
        NULL);
  XmStringFree (label);

  XtAddCallback (detailpb, XmNactivateCallback, Detail_CB, (XtPointer) NULL);
#ifdef _DEBUG_
printf("detailpb created\n");
#endif

  helppb=XmCreatePushButton(libchap_form,"HelpPB",NULL,0);
  label=XmStringCreateLocalized(" Help ");
  XtVaSetValues(helppb,
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNlabelString, label,
/*
	XmNfontList, flhv18,
*/
	XmNfontList, flhv16,
        NULL);
  XmStringFree (label);

  XtAddCallback (helppb, XmNactivateCallback, Help_CB, (XtPointer) "rxlform:Reaction Library Editor");
#ifdef _DEBUG_
printf("helppb created\n");
#endif
//printf("kka1.8:in rxlform\n");
  label=XmStringCreateLocalized(" Additional Schema Operations, etc. ... ");

  othermb=XmVaCreateSimpleMenuBar(libchap_form,"othermb",
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmVaCASCADEBUTTON,label,'\0',
        NULL);

/*
  XmStringFree (label);
*/

  XtVaSetValues(XtNameToWidget(othermb,"button_0"),
/*
    XmNfontList, flhv18,
*/
    XmNfontList, flhv16,
    NULL);
#ifdef _DEBUG_
printf("othermb created\n");
#endif

  labels[0]=XmStringCreateLocalized("Add Schema");
  labels[1]=XmStringCreateLocalized("Disable Schema");
  labels[2]=XmStringCreateLocalized("Delete Schema");
  labels[18]=XmStringCreateLocalized("Move Schema to a Different Chapter");
  labels[3]=XmStringCreateLocalized("Forward Search for Schema (from top)");
  labels[4]=XmStringCreateLocalized("Forward Search for Schema (below current position)");
  labels[5]=XmStringCreateLocalized("Continue Search Forward");
  labels[6]=XmStringCreateLocalized("Backward Search for Schema (from bottom)");
  labels[7]=XmStringCreateLocalized("Backward Search for Schema (above current position)");
  labels[8]=XmStringCreateLocalized("Continue Search Backward");
  labels[9]=XmStringCreateLocalized("IsThere (FORWARD transform pattern embedding search of all libraries)");
  labels[10]=XmStringCreateLocalized("IsThere (BACKWARD transform pattern embedding search of all libraries)");
  labels[11]=XmStringCreateLocalized("Change Password");
  labels[12]=XmStringCreateLocalized("Maintenance Utilities (restricted access)");
  labels[13]=XmStringCreateLocalized("Add New User (restricted access)");
  labels[14]=XmStringCreateLocalized("Delete User (restricted access)");
  labels[15]=XmStringCreateLocalized("Change User Clearance Level (restricted access)");
  labels[16]=XmStringCreateLocalized("List Users (level and/or password only with adequate clearance)");
  labels[17]=XmStringCreateLocalized("Close This Menu");

  otherpdm = XmVaCreateSimplePulldownMenu (othermb, "_opulldown", 0, Other_CB,
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
          SAR_CLRI_WHITE),
        XmNbuttonFontList, flhv16,
/*
        XmNbuttonFontList, flhv18,
        XmNlabelString,    label,
*/
        XmNtearOffModel, XmTEAR_OFF_ENABLED,
        XmVaPUSHBUTTON, labels[0], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[1], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[2], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[18], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[3], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[4], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[5], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[6], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[7], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[8], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[9], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[10], '\0', NULL, NULL,
        XmVaDOUBLE_SEPARATOR,
        XmVaPUSHBUTTON, labels[11], '\0', NULL, NULL,
        XmVaDOUBLE_SEPARATOR,
        XmVaPUSHBUTTON, labels[12], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[13], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[14], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[15], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[16], '\0', NULL, NULL,
        XmVaDOUBLE_SEPARATOR,
        XmVaPUSHBUTTON, labels[17], '\0', NULL, NULL,
        NULL);
//printf("kka2:in rxlform\n");
  XmStringFree (label);

#ifdef _CYGWIN_
  XtAddCallback (otherpdm, XmNtearOffMenuActivateCallback, to_CB, (XtPointer) NULL);
#endif

  for (i=0; i<19; i++)
	XmStringFree(labels[i]);
#ifdef _DEBUG_
printf("otherpdm created\n");
#endif

  schlistwin = XmCreateScrolledWindow (schlist_form, "Schema List", NULL, 0);
  schlistw = XmCreateList (schlistwin, "Schema List", NULL, 0);

  XtVaSetValues(chaplistw,
        XmNorientation, XmVERTICAL,
	XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
/*
        XmNfontList,flhv18,
*/
        XmNfontList,flhv16,
        NULL);

  XtVaSetValues(schlistw,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
/*
        XmNfontList,flco18,
*/
        XmNfontList,flco16,
        XmNselectionPolicy,          XmSINGLE_SELECT,
        XmNtitle,"Schemata",XmNx,0,XmNy,0,NULL);

  XtAddCallback (schlistw, XmNsingleSelectionCallback, Schema_CB, (XtPointer) NULL);

  XtVaSetValues (liblistw,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (chaplistw,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       liblistw,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
/*
XmNrightPosition,500,
XmNrightAttachment,XmATTACH_POSITION,
*/
    NULL);

  XtVaSetValues (detailpb,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomPosition,   400,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       chaplistw,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (helppb,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        detailpb,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       chaplistw,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      detailpb,
    NULL);

  XtVaSetValues (othermb,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       detailpb,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (schlistwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
#ifdef _DEBUG_
printf("values set for clw,slw,llw,dpb,hpb,omb,slwin\n");
#endif

#ifdef _CYGHAT_
  box = XtVaCreateManagedWidget ("box",
        xmRowColumnWidgetClass, schdisp_form,
        XmNleftAttachment,      XmATTACH_FORM,
        XmNrightAttachment,     XmATTACH_FORM,
        XmNbottomAttachment,    XmATTACH_POSITION,
        XmNbottomPosition,      320,
        XmNorientation,         XmHORIZONTAL,
        XmNheight,              1,
        NULL);
#endif

#ifdef _CYGWIN_
  textwin = XtVaCreateWidget ("Schema Data", xmScrolledWindowWidgetClass, schdisp_form, 
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        NULL);
#else
  textwin = XmCreateScrolledWindow (schdisp_form, "Schema Data", NULL, 0);
#endif

#ifdef _CYGHAT_
ac=0;
XtSetArg(al[ac],
        XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
        XmNfontList, flco16); ac++;
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
        XmNvalue, schbuf); ac++;
XtSetArg(al[ac],
        XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
        XmNmarginWidth, 0); ac++;
XtSetArg(al[ac],
        XmNtitle,"Schema Data"); ac++;
XtSetArg(al[ac],
        XmNx,0); ac++;
XtSetArg(al[ac],
        XmNy,0); ac++;
  textw = XmCreateText (textwin, "Schema Data", al, ac);
#else
  textw = XmCreateText (textwin, "Schema Data", NULL, 0);

  XtVaSetValues(textw,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
/*
        XmNfontList, flco18,
*/
        XmNfontList, flco16,
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
        XmNscrollVertical, True,
        XmNeditMode, XmMULTI_LINE_EDIT,
        XmNeditable, False,
        XmNautoShowCursorPosition, False,
        XmNcursorPositionVisible, False,
        XmNvalue, schbuf,
        XmNmarginHeight, 0,
        XmNmarginWidth, 0,
        XmNtitle,"Schema Data",
        XmNx,0,
        XmNy,0,
        NULL);
#endif

  XtVaSetValues (textwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
#ifdef _CYGHAT_
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     box,
#else
    XmNbottomPosition,   320,
    XmNbottomAttachment, XmATTACH_POSITION,
#endif
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
#ifdef _DEBUG_
printf("textwin created\n");
#endif

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  disp = Screen_TDisplay_Get (sca_p);
  gc = XCreateGC (disp, Screen_RootWin_Get (sca_p), 0, NULL);
  XSetBackground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE)); /* WHITE initially for pixmaps */
  XSetFillStyle (disp, gc, FillSolid);
  XSetLineAttributes (disp, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (disp, gc, XLoadFont (disp, "-*-courier-bold-r-normal-*-18-*-75-75-*-*-iso8859-1"));
#ifdef _DEBUG_
printf("drawing setup done\n");
#endif

  for (i = 0; i < 2; i++)
    {
    drawwin[i] = XtVaCreateManagedWidget (i == 0 ? "SchemaTransformG":"SchemaTransformS",
      xmScrolledWindowWidgetClass, schdisp_form,
      XmNscrollingPolicy,          XmAUTOMATIC,
      XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
      NULL);

    draww[i] = XtVaCreateManagedWidget (i == 0 ? "GoalPattern" : "SubgoalPattern",
      xmDrawingAreaWidgetClass, drawwin[i],
      XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
      XmNheight,            PDRW_DA_PXMP_HT,
      XmNwidth,             PDRW_DA_PXMP_WD,
      XmNfontList, fl6x10,
      XmNresize, False,
      NULL);

    XtAddCallback (draww[i],
      XmNexposeCallback,
      redraw_pattern, (XtPointer)(draww+i));

    drawp[i] = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
      PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, Screen_Depth_Get (sca_p));

    XFillRectangle (disp, drawp[i], gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
    }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));
#ifdef _DEBUG_
printf("drawing clear done\n");
#endif

  XtVaSetValues (drawwin[0],
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        textwin,
    XmNbottomPosition,   560,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (drawwin[1],
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        drawwin[0],
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);
#ifdef _DEBUG_
printf("drawing values set\n");
#endif

  srch_popup = XmCreateFormDialog (top_level, "SrchFmDg", NULL, 0);

  label = XmStringCreateLocalized ("Search");
  XtVaSetValues (srch_popup,
    XmNdialogTitle,  label,
    XmNdialogStyle,  XmDIALOG_MODELESS,
    XmNautoUnmanage, False,
    NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("Search schemata for:");
  srch_lbl =  XtVaCreateManagedWidget ("SearchLbl",
  xmLabelWidgetClass, srch_popup,
/*
	XmNfontList, flhv18,
*/
	XmNfontList, flhv16,
        XmNlabelString,  label,
        XmNtopAttachment, XmATTACH_FORM,
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        NULL);
  XmStringFree (label);

  srch_txt =  XtVaCreateManagedWidget ("SearchTxt",
        xmTextWidgetClass, srch_popup,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
/*
	XmNfontList, flhv18,
*/
	XmNfontList, flhv16,
        XmNresizeWidth, True,
        XmNmaxLength, 475,
        XmNvalue, "",
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, srch_lbl,
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        NULL);

  XtVaSetValues (srch_popup,
    XmNinitialFocus, srch_txt,
    NULL);

  label = XmStringCreateLocalized ("Search");
  srch_pb =  XtVaCreateManagedWidget ("SearchPB",
        xmPushButtonGadgetClass, srch_popup,
/*
	XmNfontList, flhv18,
*/
	XmNfontList, flhv16,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   srch_txt,
        XmNtopOffset, 0,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        NULL);
  XmStringFree (label);
  XtAddCallback (srch_pb, XmNactivateCallback, Search_CB,
    (XtPointer) NULL);
#ifdef _DEBUG_
printf("srch_popup done\n");
#endif

  mess_box = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  label=XmStringCreateLocalized("Dismiss");
  XtVaSetValues (mess_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, label,
        NULL);
  XmStringFree(label);
  XtUnmanageChild (XmMessageBoxGetChild (mess_box, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (mess_box, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (mess_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, MBDismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (mess_box);

  sch_move_msg = XmCreateMessageDialog (top_level, "Confirm Schema Move", NULL, 0);
  labels[0]=XmStringCreateLocalized("Confirm_Move");
  labels[1]=XmStringCreateLocalized("Are you sure you want to move this schema?  "
    "(If so, select a chapter from the chapter menu.  You will then have to modify the transform root.)");
  XtVaSetValues (sch_move_msg,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNdialogTitle, labels[0],
        XmNmessageString, labels[1],
        NULL);
  XmStringFree(labels[0]);
  XmStringFree(labels[1]);

  XtUnmanageChild (XmMessageBoxGetChild (sch_move_msg, XmDIALOG_HELP_BUTTON));

  XtAddCallback (XmMessageBoxGetChild (sch_move_msg, XmDIALOG_OK_BUTTON), XmNactivateCallback, sch_move_CB,
    (XtPointer) (int) TRUE);
  XtAddCallback (XmMessageBoxGetChild (sch_move_msg, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, sch_move_CB,
    (XtPointer) (int) FALSE);
  XtUnmanageChild (sch_move_msg);
#ifdef _DEBUG_
printf("mess_box done\n");
#endif

  conf_box = XmCreateMessageDialog (top_level, "Confirm", NULL, 0);
  label=XmStringCreateLocalized("Confirm");
  XtVaSetValues (conf_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNdialogTitle, label,
        NULL);
  XmStringFree(label);

  XtUnmanageChild (XmMessageBoxGetChild (conf_box, XmDIALOG_HELP_BUTTON));

  XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, Confirm_CB,
    (XtPointer) (int) TRUE);
  XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, Confirm_CB,
    (XtPointer) (int) FALSE);
  XtUnmanageChild (conf_box);
#ifdef _DEBUG_
printf("conf_box done\n");
#endif

  XtManageChild (srch_txt);
  XtManageChild (srch_pb);

  XtManageChild (liblistw);

  XtVaSetValues(XmOptionLabelGadget(liblistw),
/*
        XmNfontList,flhv18,
*/
        XmNfontList,flhv16,
        NULL);

  XtManageChild (chaplistw);
#ifdef _CYGWIN_
/* lesstif uses a different opaque naming convention from the equally opaque one used by OSF - read their minds! */
  XtUnmanageChild(XtNameToWidget(libchap_form,"ChapMenu.popup_ChapMenu.ChapMenu.button_0"));
  XtUnmanageChild(XtNameToWidget(libchap_form,"ChapMenu.popup_ChapMenu.ChapMenu.button_42"));
#else
  XtUnmanageChild(XtNameToWidget(libchap_form,"popup_ChapMenu.ChapMenu.button_0"));
  XtUnmanageChild(XtNameToWidget(libchap_form,"popup_ChapMenu.ChapMenu.button_42"));
#endif

  XtVaSetValues(XmOptionLabelGadget(chaplistw),
/*
        XmNfontList,flhv18,
*/
        XmNfontList,flhv16,
        NULL);

  XtManageChild(detailpb);
  XtManageChild(helppb);

  XtManageChild(othermb);

  /* Set libchap_form so that it can't be resized */
  geom.request_mode = CWHeight;
  XtQueryGeometry (liblistw, NULL, &geom);
//printf("kka3:in rxlform\n");
  XtManageChild (libchap_form);
               XtVaSetValues (libchap_form,
/*
               XmNpaneMinimum, geom.height,
               XmNpaneMaximum, geom.height,
*/
XmNpaneMinimum, 80,
XmNpaneMaximum, 80,
               NULL);

  XtManageChild (textw);
  XtManageChild (draww[0]);
  XtManageChild (draww[1]);

  XtManageChild (schlistw);

  XtManageChild (textwin);

  XtManageChild (drawwin[0]);
  XtManageChild (drawwin[1]);

  XtManageChild (schlistwin);

  XtManageChild (schlist_form);

  XtManageChild (schdisp_form);

  XtManageChild (schform);

  XtManageChild (toppaned);
  XtManageChild (topform);

  NSch=React_NumSchemas_Get();

  XtManageChild (frame);
#ifdef _DEBUG_
printf("frame managed\n");
#endif

  update_schemata (TRUE);
#ifdef _DEBUG_
printf("schemata updated\n");
#endif

if (sel_sch>=0)
  {
/*
  XtVaSetValues(tl,
  XmNx, glob_x=750,
  XmNy, glob_y=500,
  XmNwidth, glob_width=25,
  XmNheight, glob_height=25,
  NULL);
*/
  Find_Sch (sel_sch);
  Detail_CB (detailpb,(XtPointer)~(int)NULL,NULL);
  }
#ifdef _DEBUG_
printf("exiting RxlEdit_Create_Form\n");
#endif
}

void Lib_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char msg_str[600];
  XmString msg;
  XmRowColumnCallbackStruct *cbs = (XmRowColumnCallbackStruct *) call_data;

  if (cbs->reason != XmCR_ACTIVATE) return;

  lib_num = (int)client_data;

  if (lib_num == 4)
    {
    exit_set = TRUE;
    sprintf (msg_str, "Are you sure you want to exit?");
    msg = XmStringCreateLocalized (msg_str);
    XtVaSetValues (conf_box,
        XmNmessageString, msg,
        NULL);
    XmStringFree (msg);
    XtManageChild (conf_box);
    }
  else update_schemata (TRUE);
}

void Chap_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XmRowColumnCallbackStruct *cbs = (XmRowColumnCallbackStruct *) call_data;
  if (cbs->reason != XmCR_ACTIVATE) return;

  chap_num = (int)client_data;
current_cmenu_choice=chap_num+1;
  update_schemata (TRUE);
}

void Detail_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  if (schlist_pos == 0)
    {
    no_schemata ("view");
    return;
    }

/*
  save_NSch = NSch;
  XtUnmanageChild (frame);
*/
XtVaGetValues(tl,
XmNx, &glob_x,
XmNy, &glob_y,
XmNwidth, &glob_width,
XmNheight, &glob_height,
NULL);
XtVaSetValues(tl,
XmNx, 750,
XmNy, 500,
XmNwidth, 25,
XmNheight, 25,
NULL);
/*
  SchEdit_Create_Form (tl, schlC, schinx[schlist_pos-1], &NSch);
*/
  if (last_schema_closed || schinx[schlist_pos -1] != NSch - 1)
    {
    if (React_Schema_Allocated (NSch))
      React_Schema_Free (NSch);

    if (!React_Schema_Copy (schinx[schlist_pos-1], NSch))
      {
      printf ("rxlform: Error copying schema %d to %d\n", schinx[schlist_pos - 1], NSch);
      exit(1);
      }
    save_NSch = ++NSch;
    }
/*
  SchEdit_Create_Form (tl, schlC, NSch-1, &NSch, FALSE);
*/
  SchEdit_Create_Form (tl, schlC, NSch-1, schinx + schlist_pos - 1, FALSE, last_schema_closed, TRUE);
if (glob_sch_changed && client_data!=NULL) InfoWarn_Show ("Schema has been modified since this run");
}

void Other_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  int c, pos, lib, chap, sch;
  char string[500],*s, *lcl_srch_str, msg_str[600];
  XmString msg,label;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  Widget pd_parent, pd_grandparent;

  switch(taskmap[(int)client_data])
  {
  case 0:
    if (chap_num == 0)
      {
      label = XmStringCreateLocalized ("Error: No Chapter Selected");
      msg = XmStringCreateLocalized (
        "Please select an appropriate individual chapter/syntheme before attempting to add a schema.");
      XtVaSetValues (mess_box,
        XmNdialogTitle, label,
        XmNmessageString, msg,
        NULL);
      XmStringFree (label);
      XmStringFree (msg);
      XtManageChild (mess_box);
      return;
      }
    if (lib_num != 0 && lib_num != 3)
      {
      label = XmStringCreateLocalized ("New Schema to New Library");
      msg = XmStringCreateLocalized ("The new schema will be placed in Library 3 (Non-Legacy).");
      XtVaSetValues (mess_box,
        XmNdialogTitle, label,
        XmNmessageString, msg,
        NULL);
      XmStringFree (label);
      XmStringFree (msg);
      add_set = TRUE;
      XtManageChild (mess_box);
      }
    else if (logged_in) new_schema ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, new_schema, NULL);
      }
    break;
  case 1:
    if (schlist_pos == 0)
      {
      no_schemata ("disable");
      return;
      }

    disable_set = TRUE;
    schema=React_Schema_Handle_Get(schinx[schlist_pos - 1]);
    sch_head=React_Head_Get(schema);
    text=React_Text_Get(schema);
    txt_head=React_TxtRec_Head_Get(text);
    lib = React_Head_Library_Get (sch_head);
    chap = React_Head_SynthemeFG_Get (sch_head);
    sch = React_TxtHd_OrigSchema_Get(txt_head);
    sprintf (msg_str, "Are you sure you want to disable (temporarily) schema index %d (Lib %d Chap %d Sch %d: %s)?",
      schinx[schlist_pos -1], lib, chap, sch, String_Value_Get (React_TxtRec_Name_Get(text)));
    msg = XmStringCreateLocalized (msg_str);
    XtVaSetValues (conf_box,
      XmNmessageString, msg,
      NULL);
    XmStringFree (msg);
    XtManageChild (conf_box);
    break;
  case 2:
    if (schlist_pos == 0)
      {
      no_schemata ("delete");
      return;
      }

    delete_set = TRUE;
    schema=React_Schema_Handle_Get(schinx[schlist_pos - 1]);
    sch_head=React_Head_Get(schema);
    text=React_Text_Get(schema);
    txt_head=React_TxtRec_Head_Get(text);
    lib = React_Head_Library_Get (sch_head);
    chap = React_Head_SynthemeFG_Get (sch_head);
    sch = React_TxtHd_OrigSchema_Get(txt_head);
    sprintf (msg_str, "Are you ABSOLUTELY SURE you want to DELETE (PERMANENTLY) schema index %d (Lib %d Chap %d Sch %d: %s)?",
      schinx[schlist_pos -1], lib, chap, sch, String_Value_Get (React_TxtRec_Name_Get(text)));
    msg = XmStringCreateLocalized (msg_str);
    XtVaSetValues (conf_box,
      XmNmessageString, msg,
      NULL);
    XmStringFree (msg);
    XtManageChild (conf_box);
    break;
  case 3:
    if (schlist_pos == 0)
      {
      no_schemata ("search");
      return;
      }

    srch_fwd = TRUE;
    srch_pos = 0;
    XtManageChild(srch_popup);
    break;
  case 4:
    if (schlist_pos == 0)
      {
      no_schemata ("search");
      return;
      }

    srch_fwd = TRUE;
    srch_pos = schlist_pos;
    XtManageChild(srch_popup);
    break;
  case 6:
    if (schlist_pos == 0)
      {
      no_schemata ("search");
      return;
      }

    srch_fwd = FALSE;
    srch_pos = schlist_bottom;
    XtManageChild(srch_popup);
    break;
  case 7:
    if (schlist_pos == 0)
      {
      no_schemata ("search");
      return;
      }

    srch_fwd = FALSE;
    srch_pos = schlist_pos;
    XtManageChild(srch_popup);
    break;
  case 5:
  case 8:
    if (schlist_pos == 0)
      {
      no_schemata ("search");
      return;
      }

    if (srch_str[0]=='\0')
    {
      label = XmStringCreateLocalized ("Search Error");
      msg = XmStringCreateLocalized ("No search string defined.");
      XtVaSetValues (mess_box,
        XmNdialogTitle, label,
        XmNmessageString, msg,
        NULL);
      XmStringFree (label);
      XmStringFree (msg);
      XtManageChild (mess_box);
      break;
    }
    pos=Search_Pos(srch_str,schlist_pos, taskmap[(int)client_data] == 5);
    if (pos)
    {
      XmListSetPos(schlistw,pos);
      XmListSelectPos(schlistw,pos,True);
    }
    else
    {
      label = XmStringCreateLocalized ("Search Failed");
      sprintf (msg_str, "Search string \"%s\" not found in %s direction", srch_str,
        taskmap[(int)client_data] == 5 ? "forward" : "reverse");
      msg = XmStringCreateLocalized (msg_str);
      XtVaSetValues (mess_box,
        XmNdialogTitle, label,
        XmNmessageString, msg,
        NULL);
      XmStringFree (label);
      XmStringFree (msg);
      XtManageChild (mess_box);
    }
    break;
  case 9:
  case 10:
    run_isthere (taskmap[(int) client_data] == 9);
    break;
  case 11:
    if (logged_in) change_password ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, change_password, NULL);
      }
    break;
  case 12:
    if (logged_in) maintenance ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, maintenance, NULL);
      }
    break;
  case 13:
    if (logged_in) add_new_user ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, add_new_user, NULL);
      }
    break;
  case 14:
    if (logged_in) delete_user ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, delete_user, NULL);
      }
    break;
  case 15:
    if (logged_in) change_clearance ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, change_clearance, NULL);
      }
    break;
  case 16:
    if (logged_in) list_users ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, list_users, NULL);
      }
    break;
  case 17:
    pd_parent = XtParent (otherpdm);
    pd_grandparent = XtParent (pd_parent);
    if (pd_grandparent != othermb)
#ifdef _CYGWIN_
      {
      XUnmapWindow (XtDisplay(pd_parent),XtWindow(pd_parent));
      to_disp=XtDisplay(pd_parent);
      to_win=XtWindow(pd_parent);
      }
#else
      XUnmapWindow (XtDisplay(pd_parent),XtWindow(pd_parent));
#endif
    break;
  case 18: /* Move Schema */
    if (logged_in) move_schema ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, move_schema, NULL);
      }
    break;
  default:
    break;
  }
}

void Schema_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  int lib, chap, sch, year, hr, min, sec;
  char schname[512], crefstr[512], string[25], *sbuf, credate_str[16], moddate_str[16], cretime_str[16], modtime_str[16];
  Tsd_t *goal, *subgoal;
  Xtr_t *txtr;
  Dsp_Molecule_t *dsp;
  U32_t
        j, k, w, h, twh, x, y, txy,
        rtype,
        credate, moddate, cretime, modtime,
        num_comm, num_ref,
        roots[MX_ROOTS], synthemes[MX_ROOTS];
  Boolean_t rotate, f1, f2, f3, dummy;

  XmListCallbackStruct *cbs = (XmListCallbackStruct *) call_data;

  if (cbs->reason != XmCR_SINGLE_SELECT) return;
  if (widg != schlistw) return;

  schlist_pos = cbs->item_position;
  schema=React_Schema_Handle_Get(schinx[schlist_pos - 1]);

  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

  lib = React_Head_Library_Get (sch_head);
  chap = React_Head_SynthemeFG_Get (sch_head);
  sch = React_TxtHd_OrigSchema_Get(txt_head);

  sprintf (schbuf, "\n Library %2d: %s\n Chapter %2d: %s\n",
    lib, libname[lib - 1], chap, chapname[chap - 1]);

  sprintf(schname, "Schema %3d: %s", sch, String_Value_Get (React_TxtRec_Name_Get(text)));

  sbuf = schbuf;

  wrap_line (&sbuf, schname, MAX_TEXT);

  sbuf=schbuf+strlen(schbuf);

  sprintf(sbuf," %s\n",
		React_HeadFlags_Incomplete_Get(sch_head)?"<SCHEMA IS MARKED AS INCOMPLETE>\n":"");
  sbuf=schbuf+strlen(schbuf);

  credate=React_TxtHd_Created_Get(txt_head);
  moddate=React_TxtHd_LastMod_Get(txt_head);
  num_comm=React_TxtHd_NumComments_Get(txt_head);
  num_ref=React_TxtHd_NumReferences_Get(txt_head);

  sprintf(sbuf," EASE: %d; YLD: %d; CONF: %d\n FLAG:%s%s%s",
		React_Head_Ease_Get(sch_head),
		React_Head_Yield_Get(sch_head),
		React_Head_Confidence_Get(sch_head),
		(f1=React_HeadFlags_Protection_Get(sch_head))?" <PROT>":"",
		(f2=React_HeadFlags_Lookahead_Get(sch_head))?" <LOOK>":"",
		(f3=React_HeadFlags_Disabled_Get(sch_head))?" <DISA>":"");

  sbuf=schbuf+strlen(schbuf);
  sprintf(sbuf,"%s; TYPE: ",
		f1 || f2 || f3?"":" <NONE>");

  rtype=React_Head_ReactionType_Get(sch_head);
  switch(rtype)
	{
	case REACT_MULT_NO_PREF:
		strcat(sbuf,"MNP");
		break;
	case REACT_MULT_PREF_SING:
		strcat(sbuf,"MPS");
		break;
	case REACT_MULT_PREF_MULT:
		strcat(sbuf,"MPM");
		break;
	case REACT_SING_APPL_ONLY:
		strcat(sbuf,"SAO");
		break;
	case REACT_MAX_APPL_ONLY:
		strcat(sbuf,"MAO");
		break;
	}

  sbuf=schbuf+strlen(schbuf);
  sprintf(sbuf,"; MAXUSM: %d\n\n",
		React_Head_MaxNonident_Get(sch_head));

  if (credate)
    {
    sbuf=schbuf+strlen(schbuf);
    date_calc(credate,credate_str,&year,NULL,NULL);
    if (year >= 2000)
      {
      cretime = Persist_ModTime (PER_STD, Persist_Orig_Rec (PER_STD, schinx[schlist_pos - 1], &dummy));
      time_calc(cretime,cretime_str,&hr,&min,&sec);
      sprintf(sbuf," Created: %s %s\n",credate_str,cretime_str);
      }
    else sprintf(sbuf," Created: %s\n",credate_str);
    if (!moddate) strcat(sbuf,"\n");
    }

  if (moddate)
    {
    sbuf=schbuf+strlen(schbuf);
    date_calc(moddate,moddate_str,&year,NULL,NULL);
    if (year >= 2000)
      {
      modtime = Persist_ModTime (PER_STD, schinx[schlist_pos - 1]);
      time_calc(modtime,modtime_str,&hr,&min,&sec);
      sprintf(sbuf," Modified: %s %s\n\n",moddate_str,modtime_str);
      }
    else sprintf(sbuf," Modified: %s\n\n",moddate_str);
    }

  for (j=0; j<num_ref; j++) if (String_Value_Get (React_TxtRec_Reference_Get(text,j))[0] != '\007')
    {
    sprintf(crefstr,"<REF> %s", String_Value_Get (React_TxtRec_Reference_Get(text,j)));
    wrap_line (&sbuf, crefstr, MAX_TEXT);
    }

  for (j=0; j<num_comm; j++)
    {
    sprintf(crefstr,"<COMM> %s", String_Value_Get (React_TxtRec_Comment_Get(text,j)));
    wrap_line (&sbuf, crefstr, MAX_TEXT);
    }

  for (j=0; j<num_ref; j++) if (String_Value_Get (React_TxtRec_Reference_Get(text,j))[0] == '\007')
    {
    sprintf(crefstr,"<LIBRARY UPDATE> %s", String_Value_Get (React_TxtRec_Reference_Get(text,j)) + 1);
    wrap_line (&sbuf, crefstr, MAX_TEXT);
    }

  XmTextSetString (textw, schbuf);

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));

  for (j=0; j<2; j++)
    {
    XClearWindow (disp, XtWindow (draww[j]));
    XFillRectangle (disp, drawp[j], gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
    }
  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  for (j=0; j<MX_ROOTS; j++)
    {
    roots[j]=React_Head_RootNode_Get(sch_head, j);
    synthemes[j]=React_Head_RootSyntheme_Get(sch_head, j);
    }

  sprintf(string,"Goal TSD");
  XDrawString(disp,XtWindow(draww[0]),gc,25,25,string,strlen(string));
  XDrawString(disp,drawp[0],gc,25,25,string,strlen(string));

  sprintf(string,"Subgoal TSD");
  XDrawString(disp,XtWindow(draww[1]),gc,25,25,string,strlen(string));
  XDrawString(disp,drawp[1],gc,25,25,string,strlen(string));

  goal=Tsd_Copy(React_Goal_Get(schema));
/*
Tsd_Dump(goal,&GStdErr);
*/

  Tsd_Strip_H (goal, FALSE);

  txtr=Tsd2Xtr(goal);
  dsp=Xtr2Dsp(txtr);

  if (dsp_Shelley(dsp))
    {
    Dsp_Compress (dsp);
/*
    fixdsp (dsp);
*/
    h=dsp->molh;
    w=dsp->molw;
    if (rotate=w<h)
      {
      twh=w;
      w=h;
      h=twh;
      }
    dsp->molh=h;
    dsp->molw=w;
    for (k=0; k<dsp->natms; k++)
      {
      x=dsp->atoms[k].x;
      y=dsp->atoms[k].y;
      if (rotate)
        {
        txy=x;
        x=y;
        y=txy;
        }
      dsp->atoms[k].x=x;
      dsp->atoms[k].y=y;
      }
    mdraw(dsp,goal,disp,XtWindow(draww[0]),gc,25,50,275,200,
                        roots,synthemes,NULL,TRUE);
    mdraw(dsp,goal,disp,drawp[0],gc,25,50,275,200, roots,synthemes,NULL,FALSE);
    free_Molecule (dsp);
    }
  Xtr_Destroy (txtr);
  Tsd_Destroy (goal);

  subgoal=Tsd_Copy(React_Subgoal_Get(schema));

  Tsd_Strip_H (subgoal, FALSE);

  txtr=Tsd2Xtr(subgoal);
  dsp=Xtr2Dsp(txtr);

  if (dsp_Shelley(dsp))
    {
    Dsp_Compress (dsp);
/*
    fixdsp (dsp);
*/
    h=dsp->molh;
    w=dsp->molw;
    if (rotate=w<h)
      {
      twh=w;
      w=h;
      h=twh;
      }
    dsp->molh=h;
    dsp->molw=w;
    for (k=0; k<dsp->natms; k++)
      {
      x=dsp->atoms[k].x;
      y=dsp->atoms[k].y;
      if (rotate)
        {
        txy=x;
        x=y;
        y=txy;
        }
      dsp->atoms[k].x=x;
      dsp->atoms[k].y=y;
      }
    mdraw(dsp,subgoal,disp,XtWindow(draww[1]),gc,25,50,275,200,
                        NULL,NULL,NULL,TRUE);
    mdraw(dsp,subgoal,disp,drawp[1],gc,25,50,275,200, NULL,NULL,NULL,FALSE);
    free_Molecule (dsp);
    }
  Xtr_Destroy (txtr);
  Tsd_Destroy (subgoal);

  XFlush (disp);
}

void update_schemata (Boolean_t update_list)
{
  static int prev_lib;
  int inx, i, j, l, c, tlib, tchap, tsch;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  char itemstr[500],name[200];
  XmString item;
  Boolean_t upd_count, is_temp;

  upd_count=lib_num==0 && sch_count[0][0]==0 && sch_count[1][0]==0 && sch_count[2][0] == 0;
  if (upd_count) for (i=prev_lib=0; i<3; i++) for (j=0; j<41; j++) sch_count[i][j]=0;

  if (update_list)
    {
    XmListDeleteAllItems(schlistw);
    XmListSetAddMode(schlistw,TRUE);
    for (i = 0; i < 3; i++) for (j = 0; j < 40; j++) last_schnum[i][j] = 0;
    }
  schlist_bottom = 0;

/*
printf("NSch = %d\n",NSch);
*/
  for (inx=j=0; inx<NSch; inx++) if (Persist_Legacy_Rxn (inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
    {
    i = Persist_Current_Rec (PER_STD, inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
    schema=React_Schema_Handle_Get(i);
    sch_head=React_Head_Get(schema);
    text=React_Text_Get(schema);
    txt_head=React_TxtRec_Head_Get(text);

    if (update_list)
      {
      /* get last schema number in each chapter */
      tlib = React_Head_Library_Get (sch_head);
      tchap = React_Head_SynthemeFG_Get (sch_head);
      tsch =  React_TxtHd_OrigSchema_Get(txt_head);
      if (last_schnum[tlib - 1][tchap - 1] < tsch) last_schnum[tlib - 1][tchap - 1] = tsch;
      }

    if ((lib_num == 0 || React_Head_Library_Get (sch_head) == lib_num) &&
        (chap_num == 0 || React_Head_SynthemeFG_Get (sch_head) == chap_num))
      {
      if (lib_num == 0)
        {
        if (chap_num == 0)
          {
          l=React_Head_Library_Get (sch_head);
          c=React_Head_SynthemeFG_Get (sch_head);
          sprintf(itemstr, "%d/%2d/%3d [%06d]: %s",
                l,
		c,
                React_TxtHd_OrigSchema_Get(txt_head),
                i,
                String_Value_Get(
                React_TxtRec_Name_Get(text)));
          if (upd_count)
            {
            sch_count[l-1][c]++;
            sch_count[l-1][0]++;
            }
          }
        else
          sprintf(itemstr, "%d/%3d [%06d]: %s",
                React_Head_Library_Get (sch_head),
                React_TxtHd_OrigSchema_Get(txt_head),
                i,
                String_Value_Get(
                React_TxtRec_Name_Get(text)));
        }
      else
        {
        if (chap_num == 0)
            sprintf(itemstr, "%2d/%3d [%06d]: %s",
		React_Head_SynthemeFG_Get (sch_head),
                React_TxtHd_OrigSchema_Get(txt_head),
                i,
                String_Value_Get(
                React_TxtRec_Name_Get(text)));
        else
            sprintf(itemstr, "%3d [%06d]: %s",
                React_TxtHd_OrigSchema_Get(txt_head),
                i,
                String_Value_Get(
                React_TxtRec_Name_Get(text)));
        }

      if (update_list)
        {
        item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
        XmListAddItem(schlistw,item,j+1);
        schinx[j++] = i;
        XmStringFree(item);
        }
      }
    }

  if (upd_count || lib_num != prev_lib)
    {
    prev_lib=lib_num;

/*
    for (i=0; i<41; i++)
*/
    for (i=0; i<43; i++)
      {
/*
      if (i == 0) sprintf(itemstr, "All Synthemes (%d schemata)",
        lib_num==0 ? sch_count[0][0] + sch_count[1][0] + sch_count[2][0] : sch_count[lib_num - 1][0]);
*/
if (i==0) strcpy(itemstr," Go to Bottom of Menu ");
else if (i==42) strcpy(itemstr," Go to Top of Menu ");
else  if (i == 1) sprintf(itemstr, " All Synthemes (%d schemata)  ",
        lib_num==0 ? sch_count[0][0] + sch_count[1][0] + sch_count[2][0] : sch_count[lib_num - 1][0]);
      else sprintf (itemstr, "Chap %d (%d sch): %s",
/*
        i, lib_num==0 ? sch_count[0][i] + sch_count[1][i] + sch_count[2][i] : sch_count[lib_num - 1][i], chapname[i - 1]);
*/
        i-1, lib_num==0 ? sch_count[0][i-1] + sch_count[1][i-1] + sch_count[2][i-1] : sch_count[lib_num - 1][i-1], chapname[i - 2]);
      item=XmStringCreateLocalized(itemstr);
#ifdef _CYGWIN_
/* lesstif uses a different opaque naming convention from the equally opaque one used by OSF - read their minds! */
      sprintf(name,"ChapMenu.popup_ChapMenu.ChapMenu.button_%d",i);
#else
      sprintf(name,"popup_ChapMenu.ChapMenu.button_%d",i);
#endif
      XtVaSetValues(XtNameToWidget(libchap_form, name),
        XmNlabelString, item,
        NULL);
      XmStringFree(item);
      }
    }

  if (!update_list) return;

  XmListSetAddMode(schlistw,FALSE);
  schlist_bottom = j;

  if (schlist_bottom == 0)
    {
    schlist_pos = 0;

    schbuf[0] = 0;
    XmTextSetString (textw, schbuf);

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_WHITE));

    for (i = 0; i < 2; i++)
      {
      XFillRectangle (disp, drawp[i], gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
      XCopyArea (disp,
        drawp[i], XtWindow (draww[i]), gc,
        0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
      }

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_BLACK));

    return;
    }

  XmListSetBottomPos(schlistw, schlist_bottom);
  XmListSetPos(schlistw,1);
  XmListSelectPos(schlistw,1,True);
}

void mdraw(Dsp_Molecule_t *mol_p,Tsd_t *patt,Display *dsp,/* Drawable */ Window dawin,GC gc,
        int X1,int Y1,int X2,int Y2,U32_t *roots,U32_t *synthemes, Boolean_t *dots, Boolean_t iswin)
{
        Dsp_Atom_t *atom_p;
        Dsp_Bond_t *bond_p;
        char *ftag,atomsym[10];
        float length;
        float norm_x, norm_y;
        int x0, y0, x1, y1, x2, y2;
        int atm_i, bnd_i, var_num;
        int i,j,k,l,bond_order[600];
        Boolean_t atom_conn[100];
        U32_t s;
        Dimension aw, ah, ax, ay;

        XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
        for (i=0; i<600; i++) bond_order[i]=0;
        for (i=0; i<100; i++) atom_conn[i]=FALSE;
        for (i=0; i<mol_p->nbnds; i++)
        {
                j=mol_p->bonds[i].latm_p-mol_p->atoms;
                k=mol_p->bonds[i].ratm_p-mol_p->atoms;
                for (l=0; l<MX_NEIGHBORS; l++)
                        if (patt->atoms[j].neighbors[l].id==k)
                {
                        bond_order[i]=patt->atoms[j].neighbors[l].bond;
                        atom_conn[j]=atom_conn[k]=TRUE;
                }
        }

/*  First draw bonds.  */
        bond_p = mol_p->bonds;

        for (bnd_i = 0; bnd_i < mol_p->nbnds; bnd_i++)
        {
                x1 = bond_p->latm_p->x;
                y1 = bond_p->latm_p->y;
                x2 = bond_p->ratm_p->x;
                y2 = bond_p->ratm_p->y;

                if (bond_order[bnd_i]>3) XSetLineAttributes(dsp,gc,1,
                        LineOnOffDash,CapNotLast,JoinMiter);
                else XSetLineAttributes(dsp,gc,1,
                        LineSolid,CapNotLast,JoinMiter);

                switch(bond_order[bnd_i])
                {
                case 0:
                        break;
                case 1:
                case 5:
                        XDrawLine (dsp, dawin, gc, x1+X1, y1+Y1, x2+X1, y2+Y1);
                        break;
                case 2:
                case 6:
                        length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
                        norm_x = (y2-y1)/length;
                        norm_y = (x2-x1)/length;
                        norm_x *= AppDim_BndDblOff_Get (&GAppDim);
                        norm_y *= AppDim_BndDblOff_Get (&GAppDim);

                        norm_x = (norm_x > 0) ? norm_x + 0.5 : norm_x - 0.5;
                        norm_y = (norm_y > 0) ? norm_y + 0.5 : norm_y - 0.5;
                        x0 = (int) norm_x;
                        y0 = (int) norm_y;
                        XDrawLine (dsp, dawin, gc, x1 + x0 + X1, y1 - y0 + Y1,
                                x2 + x0 + X1, y2 - y0 + Y1);
                        XDrawLine (dsp, dawin, gc, x1 - x0 + X1, y1 + y0 +Y1,
                                x2 - x0 + X1, y2 + y0 + Y1);
                        break;
                case 3:
                        XDrawLine (dsp, dawin, gc, x1 + X1, y1 + Y1,
                                x2 + X1, y2 + Y1);
                        length = sqrt ((x1-x2)*(x1-x2) + (y1-y2)*(y1-y2));
                        norm_x = (y2-y1)/length;
                        norm_y = (x2-x1)/length;
                        norm_x *= AppDim_BndTplOff_Get (&GAppDim);
                        norm_y *= AppDim_BndTplOff_Get (&GAppDim);

                        norm_x = (norm_x > 0) ? norm_x + 0.5 : norm_x - 0.5;
                        norm_y = (norm_y > 0) ? norm_y + 0.5 : norm_y - 0.5;
                        x0 = (int) norm_x;
                        y0 = (int) norm_y;
                        XDrawLine (dsp, dawin, gc, x1 + x0 + X1, y1 - y0 + Y1,
                                x2 + x0 + X1, y2 - y0 + Y1);
                        XDrawLine (dsp, dawin, gc, x1 - x0 + X1, y1 + y0 + Y1,
                                x2 - x0 + X1, y2 + y0 + Y1);
                        break;
                default:
                        printf("unrecognized bond order: %d\n",
                                bond_order[bnd_i]);
                        break;
                }
                ++bond_p;
        }
        XSetLineAttributes(dsp,gc,1,LineSolid,CapNotLast,JoinMiter);

/*  Then draw atoms.  */
        atom_p = mol_p->atoms;

        for (atm_i = 0; atm_i < mol_p->natms; atm_i++)
        {
/*
printf("%d: %s (%d, %d)\n", atm_i, atom_p->sym, atom_p->x, atom_p->y);
*/
                if (roots) s=root_and_syntheme(atm_i,roots,synthemes);
                else s=0;
                if ((!atom_p->isC && atom_conn[atm_i]) || s)
                {
                        strncpy(atomsym,atom_p->sym,4);
                        atomsym[4]=0;
                        if (!strcasecmp (atomsym, "#J")) strcpy (atomsym, "X");
                        else if (atomsym[0] == '$')
                          {
                          sscanf (atomsym + 1, "%d", &var_num);
                          if (var_num % 2) strcpy (atomsym, "R'");
                          else strcpy (atomsym, "R");
                          }

                        aw=14*strlen(atomsym)+11;
                        ah=18;

                        ax = (atom_p->x - (aw >> 1) > 0) ?
                                atom_p->x - (aw >> 1) : 0;
                        ay = (atom_p->y - (ah >> 1) > 0) ?
                                atom_p->y - (ah >> 1) : 0;
                        if (iswin)
                            XClearArea (dsp, dawin, ax + X1, ay + Y1, aw, ah, FALSE);
                        else
                          {
                          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
                            SAR_CLRI_WHITE));
                          XFillRectangle (dsp, dawin, gc, ax + X1, ay + Y1, aw, ah);
                          }
                        if (dots == NULL || !dots[atm_i])
                          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
                            SAR_CLRI_BLACK));
                        else
                          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
                            SAR_CLRI_RED));
                        if (s) XDrawRectangle(dsp,dawin,gc,ax+X1,ay+Y1,aw,ah);

                        XDrawString(dsp,dawin,gc,ax+aw/4+X1,ay+3*ah/4+Y1,
                                atomsym,strlen(atomsym));
                }
                else if (dots != NULL && atom_p->isC && dots[atm_i])
                {
                  XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_RED));
                  XFillArc (dsp, dawin, gc, atom_p->x + X1 - 5, atom_p->y + Y1 - 5, 10, 10, 0, 360 * 64);
                }
                ++atom_p;
        }
}


U32_t root_and_syntheme(U32_t n,U32_t *r,U32_t *s)
{
        while(*r!=REACT_NODE_INVALID)
        {
                if (n==*r) return(*s);
                r++;
                s++;
        }
        return(0);
}

void fixdsp (Dsp_Molecule_t *dsp)
{
	int minx, maxx, minconx, maxconx, i, dx, dw;

	for (i = maxx = maxconx = 0, minx = minconx = dsp->molw; i < dsp->natms; i++)
	{
		if (dsp->atoms[i].x < minx)
			minx = dsp->atoms[i].x;
		if (dsp->atoms[i].adj_info.num_neighbs != 0 &&
			dsp->atoms[i].x < minconx)
			minconx = dsp->atoms[i].x;
		if (dsp->atoms[i].x > maxx)
			maxx = dsp->atoms[i].x;
		if (dsp->atoms[i].adj_info.num_neighbs != 0 &&
			dsp->atoms[i].x > maxconx)
			maxconx = dsp->atoms[i].x;
	}
	dx = minx - minconx;
	dw = maxconx - maxx + dx;
	dsp->molw += dw;
	for (i = 0; i < dsp->natms; i++)
		dsp->atoms[i].x += dx;
}

void redraw_pattern
  (
  Widget      drawing_a,
  XtPointer   client_data,
  XtPointer   call_data
  )

{
  int which;
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) call_data;

  which=(Widget *)client_data - draww;

  XCopyArea (cbs->event->xexpose.display,
    drawp[which], XtWindow (draww[which]), gc,
    0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);
}

void date_calc(long num, char *dc, int *yr, int *mo, int *day)
{
	int m,d,y,count;

	if (!num)
	{
		strcpy(dc,"N/A");
		return;
	}
	y=(int)((float)num/365.25);
	num-=y*365+y/4;
	y+=1900;
	for (m=1, count=31; num>count; num-=days(m++), count=days(m));
	d=num;
	sprintf(dc,"%d/%d/%d",m,d,y);
	if (yr!=NULL) *yr=y;
	if (mo!=NULL) *mo=m;
	if (day!=NULL) *day=d;
}

void time_calc(long num, char *tc, int *hr, int *min, int *sec)
{
	int h,m,s;
	char ap;

	h=num/3600;
	if (hr!=NULL) *hr = h;
	num-=3600*h;
	if (h<12) ap='A';
	else
	{	ap='P';
		h-=12;
	}
	if (h==0) h=12;
	m=num/60;
	if (min!=NULL) *min = m;
	num-=60*m;
	s=num;
	if (sec!=NULL) *sec = s;
	if (h==12 && m==0 && s==0)
	{
		if (ap=='A') sprintf(tc,"midnight");
		else sprintf(tc,"noon");
	}
	else sprintf(tc,"%d:%02d:%02d %c.M.",h,m,s,ap);
}

int days(int m)
{
	switch(m)
	{
	case 1:
	case 3:
	case 5:
	case 7:
	case 8:
	case 10:
	case 12:
		return 31;
	case 2:
		return 28;
	case 4:
	case 6:
	case 9:
	case 11:
		return 30;
	}
}

void wrap_line (char **buf, char *string, int maxlen)
{
  char *tstr, lastc;
  Boolean_t keep_lastc;

  for (tstr = string, keep_lastc = FALSE; *tstr != 0;)
    {
    *buf += strlen (*buf);
    if (tstr != string)
      {
      if (keep_lastc)
        sprintf (string, "  %c%s", lastc, tstr);
      else
        sprintf (string, "  %s", tstr);
      }
    if (strlen(string) > maxlen)
      {
      for (tstr = string + maxlen; *tstr != ' ' && *tstr != '-' &&
          *tstr != '(' && *tstr != ')' &&
          *tstr != '[' && *tstr != ']' &&
          *tstr != '{' && *tstr != '}' &&
           tstr > string + 2; tstr--);
      if (*tstr == '(' || *tstr == '[' || *tstr == '{')
        while(*(tstr - 1) == '(' || *(tstr - 1) == '[' || *(tstr - 1) == '{' || (tstr > string + 2 && *(tstr - 1) == ' ')) tstr--;
      else
        while(*(tstr + 1) == ' ') tstr++;
      if (tstr <= string + 2)
        {
        for (tstr = string + maxlen; *tstr != ' ' && *tstr != '-' &&
            *tstr != '(' && *tstr != ')' &&
            *tstr != '[' && *tstr != ']' &&
            *tstr != '{' && *tstr != '}' &&
            *tstr != 0; tstr++);
        if (*tstr == ')' || *tstr == ']' || *tstr == '}')
          while(*(tstr + 1) == ')' || *(tstr - 1) == ']' || *(tstr - 1) == ')') tstr++;
        while(*(tstr + 1) == ' ') tstr++;
        if (*tstr == 0) lastc = ' ';
        else
          {
          lastc = *tstr;
          *tstr++ = 0;
          }
        }
      else
        {
        lastc = *tstr;
        *tstr++ = 0;
        }
      }
    else
      {
      tstr = string + strlen (string);
      lastc = ' ';
      }
    if (keep_lastc = lastc == '(' || lastc == '[' || lastc == '{')
      sprintf (*buf, " %s-\n", string);
    else if (lastc == ')' || lastc == ']' || lastc == '}')
      sprintf (*buf, " %s%c-\n", string, lastc);
    else
      sprintf (*buf, " %s%c\n", string, lastc);
    }
  *buf += strlen (*buf);
}

int Search_Pos (char *string, int prev_pos, Boolean_t forward)
{
  int inx,i,j,c,pos;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  char str[500],txt[500];
  Boolean_t is_temp;

  for (i=0; i<=strlen(string); i++) str[i]=tolower(string[i]);
  for (inx=pos=0; inx<NSch && (forward || pos<prev_pos); inx++) if (Persist_Legacy_Rxn (inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
    {
    i = Persist_Current_Rec (PER_STD, inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
    schema=React_Schema_Handle_Get(i);
    sch_head=React_Head_Get(schema);
    text=React_Text_Get(schema);
    if ((lib_num == 0 || React_Head_Library_Get (sch_head) == lib_num) &&
        (chap_num == 0 || React_Head_SynthemeFG_Get (sch_head) == chap_num) &&
        ++pos>prev_pos)
      {
      strcpy(txt,String_Value_Get(React_TxtRec_Name_Get(text)));
      for (j=0; j<strlen(txt); j++) txt[j]=tolower(txt[j]);
      if (strstr(txt,str)) return(pos);
      }
    }

  if (forward) return(0);

  for (inx-=2; inx>=0; inx--) if (Persist_Legacy_Rxn (inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
    {
    i = Persist_Current_Rec (PER_STD, inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
    schema=React_Schema_Handle_Get(i);
    sch_head=React_Head_Get(schema);
    text=React_Text_Get(schema);
    if ((lib_num == 0 || React_Head_Library_Get (sch_head) == lib_num) &&
        (chap_num == 0 || React_Head_SynthemeFG_Get (sch_head) == chap_num))
      {
      pos--;
      strcpy(txt,String_Value_Get(React_TxtRec_Name_Get(text)));
      for (j=0; j<strlen(txt); j++) txt[j]=tolower(txt[j]);
      if (strstr(txt,str)) return(pos);
      }
    }

  return(0);
}

void Find_Sch (int sel_sch)
{
  int i,inx,curr_sel,pos;
  Boolean_t is_temp;

  curr_sel = Persist_Current_Rec (PER_STD, sel_sch, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
  if (curr_sel != sel_sch)
{
/* eventually use message box */
printf("\007\007Warning: schema has been modified\007\007\n");
glob_sch_changed=TRUE;
}
else glob_sch_changed=FALSE;
  for (inx=0, pos=1; inx<NSch; inx++, pos++) if (Persist_Legacy_Rxn (inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
    {
    i = Persist_Current_Rec (PER_STD, inx, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
    if (i==curr_sel)
      {
      XmListSetPos(schlistw,pos);
      XmListSelectPos(schlistw,pos,True);
      return;
      }
    }
  else pos--;
printf("Warning: could not find schema index %d (%d)\n",curr_sel,sel_sch);
}

void Search_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  int pos;
  char *lcl_srch_str, msg_str[600];
  XmString msg, label;

  lcl_srch_str = XmTextGetString(XtNameToWidget(srch_popup,"SearchTxt"));
  strcpy(srch_str,lcl_srch_str);
  XtFree(lcl_srch_str);
  XtUnmanageChild(srch_popup);
  pos=Search_Pos(srch_str, srch_pos, srch_fwd);
  if (pos)
    {
    XmListSetPos(schlistw,pos);
    XmListSelectPos(schlistw,pos,True);
    }
  else
    {
    label = XmStringCreateLocalized ("Search Failed");
    sprintf (msg_str, "Search string \"%s\" not found in %s direction.", srch_str, srch_fwd ? "forward" : "reverse");
    msg = XmStringCreateLocalized (msg_str);
    XtVaSetValues (mess_box,
      XmNdialogTitle, label,
      XmNmessageString, msg,
      NULL);
    XmStringFree (label);
    XmStringFree (msg);
    XtManageChild (mess_box);
    }
}

void MBDismiss_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (mess_box);
  if (!add_set)
    {
    if (disable_set || delete_set)
      {
      disable_set = delete_set = FALSE;
      update_schemata (TRUE);
      }
    return;
    }

  add_set = FALSE;
  if (logged_in) new_schema ();
  else
    {
    if (login_failed) LoginFail (tl);
    else Login (tl, new_schema, NULL);
    }
}

void Confirm_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (conf_box);
  if (exit_set)
    {
    exit_set = FALSE;
    if ((Boolean_t) (int) client_data)
      {
      Persist_Close ();
      exit (0);
      }
    }
  else if (disable_set)
    {
/* defer until MBDismiss_CB unless canceled
    disable_set = FALSE;
*/
    if (!(Boolean_t) (int) client_data)
      {
      disable_set = FALSE;
      return;
      }
/* begin disabling of schema */
    if (logged_in) disable_schema ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, disable_schema, NULL);
      }
    }
  else if (delete_set)
    {
/* defer until MBDismiss_CB unless canceled
    delete_set = FALSE;
*/
    if (!(Boolean_t) (int) client_data)
      {
      delete_set = FALSE;
      return;
      }
/* begin disabling of schema */
    if (logged_in) delete_schema ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, delete_schema, NULL);
      }
    }
  else printf ("Unexpected call to Confirm_CB\n");
}

void new_schema()
{
  if (clearance_level < ADD_SCH_LEV)
    {
    LoginFail (tl);
    return;
    }

  save_NSch = NSch;
  if (React_Schema_Allocated (NSch))
    React_Schema_Free (NSch);

  if (!React_Schema_Init (NSch, 3, chap_num, last_schnum[2][chap_num - 1] + 1, UserId ()))
    {
    printf ("rxlform (new_schema): Error initializing schema\n");
    exit (1);
    }

  XtVaGetValues(tl,
    XmNx, &glob_x,
    XmNy, &glob_y,
    XmNwidth, &glob_width,
    XmNheight, &glob_height,
    NULL);

  XtVaSetValues(tl,
    XmNx, 750,
    XmNy, 500,
    XmNwidth, 25,
    XmNheight, 25,
    NULL);

  save_NSch = ++NSch;
  new_schema_flag = TRUE;
/*
  SchEdit_Create_Form (tl, schlC, NSch-1, &NSch, TRUE);
*/
  SchEdit_Create_Form (tl, schlC, NSch-1, NULL, TRUE, last_schema_closed, TRUE);
}

void disable_schema()
{
  disable_delete (FALSE);
}

void delete_schema()
{
  disable_delete (TRUE);
}

void disable_delete (Boolean_t permanent)
{
  int c, pos, lib, chap, sch;
  char string[500],*s, *lcl_srch_str, msg_str[600];
  XmString msg, label;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;

  if (clearance_level < (permanent ? DEL_SCH_LEV : DIS_SCH_LEV))
    {
    LoginFail (tl);
    return;
    }

  schema=React_Schema_Handle_Get(schinx[schlist_pos - 1]);
  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);
  lib = React_Head_Library_Get (sch_head);
  chap = React_Head_SynthemeFG_Get (sch_head);
  sch = React_TxtHd_OrigSchema_Get(txt_head);

  /* create msg_str BEFORE action, because update_schemata resets schpos and updates schinx array */
  sprintf (msg_str, "Schema index %d (Lib %d Chap %d Sch %d: %s) has been %s.",
    schinx[schlist_pos - 1], lib, chap, sch, String_Value_Get (React_TxtRec_Name_Get(text)), permanent ? "deleted" : "disabled");

  backup_rxnlib ();

  if (permanent)
    {
    Persist_Delete_Rxn (schinx[schlist_pos - 1], FALSE);
/* defer until MBDismiss_CB
    update_schemata (TRUE);
*/
    }
  else
    {
/*
    React_HeadFlags_Disabled_Put (sch_head, TRUE);
    React_Schema_Write (schinx[schlist_pos-1]);
    XmListSetPos(schlistw,schlist_pos);
    XmListSelectPos(schlistw,schlist_pos,True);
*/
    if (React_Schema_Allocated (NSch))
      React_Schema_Free (NSch);

    if (!React_Schema_Copy (schinx[schlist_pos-1], NSch))
      {
      printf ("rxlform: Error copying schema %d to %d\n", schinx[schlist_pos - 1], NSch);
      exit(1);
      }
    ++NSch;
    schema=React_Schema_Handle_Get(NSch - 1);
    sch_head=React_Head_Get(schema);
    React_HeadFlags_Disabled_Put (sch_head, TRUE);
    React_Schema_Write (NSch - 1);
    XmListSetPos(schlistw,schlist_pos);
    XmListSelectPos(schlistw,schlist_pos,True);
    Persist_Update_Rxn (NSch - 1, schinx[schlist_pos - 1], FALSE, FALSE, "disable_delete");
    schinx[schlist_pos - 1] = NSch - 1;
    }
  label = XmStringCreateLocalized (permanent ? "Schema Deleted" : "Schema Disabled");
  msg = XmStringCreateLocalized (msg_str);
  XtVaSetValues (mess_box,
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree (label);
  XmStringFree (msg);
  XtManageChild (mess_box);
}

void move_schema ()
{
  if (clearance_level < EDIT_TRANSFORM_LEV)
    {
    LoginFail (tl);
    return;
    }
  XtManageChild (sch_move_msg);
}

void sch_move_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  int i;
  Widget pb;
  char pbname[64];

  XtUnmanageChild (sch_move_msg);
  if (!(Boolean_t)(int)client_data) return;

  for (i = 0; i < 41; i++)
    {
#ifdef _CYGWIN_
/* lesstif uses a different opaque naming convention from the equally opaque one used by OSF - read their minds! */
    sprintf (pbname, "ChapMenu.popup_ChapMenu.ChapMenu.button_%d", i);
#else
    sprintf (pbname, "popup_ChapMenu.ChapMenu.button_%d", i);
#endif
    pb = XtNameToWidget (libchap_form, pbname);
    XtRemoveAllCallbacks (pb, XmNactivateCallback);
    XtAddCallback (pb, XmNactivateCallback, move_schema_cont, (XtPointer) i);
    }
}

void move_schema_cont (Widget w, XtPointer client_data, XtPointer call_data)
{
  int i, c, pos, lib, chap, sch, new_chap_num, nrefs;
  char string[500],*s, *lcl_srch_str, msg_str[600];
  XmString msg, label;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  Widget pb;
  char pbname[64], msgstr[64], ref_string_string[64];
  String_t ref_string;

  XmRowColumnCallbackStruct *cbs = (XmRowColumnCallbackStruct *) call_data;
  if (cbs->reason != XmCR_ACTIVATE) return;

  new_chap_num = (int)client_data;

  if (new_chap_num == 0)
  {
    XtManageChild (sch_move_msg);
    return;
  }

/*
  for (i = 0; i < 41; i++)
*/
  for (i = 0; i < 43; i++)
    {
#ifdef _CYGWIN_
/* lesstif uses a different opaque naming convention from the equally opaque one used by OSF - read their minds! */
    sprintf (pbname, "ChapMenu.popup_ChapMenu.ChapMenu.button_%d", i);
#else
    sprintf (pbname, "popup_ChapMenu.ChapMenu.button_%d", i);
#endif
    pb = XtNameToWidget (libchap_form, pbname);
    XtRemoveAllCallbacks (pb, XmNactivateCallback);
    if (i==0 || i==42)
      XtAddCallback (pb, XmNactivateCallback, MSB_CB, (XtPointer) (i==0?-1:1));
    else XtAddCallback (pb, XmNactivateCallback, Chap_CB, (XtPointer) (i-1));
    }

  schema=React_Schema_Handle_Get(schinx[schlist_pos - 1]);
  sch_head=React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);
  lib = React_Head_Library_Get (sch_head);
  chap = React_Head_SynthemeFG_Get (sch_head);
  sch = React_TxtHd_OrigSchema_Get(txt_head);
  nrefs = React_TxtHd_NumReferences_Get (txt_head);

  /* create msg_str BEFORE action, because update_schemata resets schpos and updates schinx array */
  sprintf (msg_str, "Schema index %d (Lib %d Chap %d Sch %d: %s) has been %s.",
    schinx[schlist_pos - 1], lib, chap, sch, String_Value_Get (React_TxtRec_Name_Get(text)), "moved");

  backup_rxnlib ();

    if (React_Schema_Allocated (NSch))
      React_Schema_Free (NSch);

    if (!React_Schema_Copy (schinx[schlist_pos-1], NSch))
      {
      printf ("rxlform: Error copying schema %d to %d\n", schinx[schlist_pos - 1], NSch);
      exit(1);
      }
    ++NSch;
    ++nrefs;

    schema=React_Schema_Handle_Get(NSch - 1);
    sch_head=React_Head_Get(schema);
    text=React_Text_Get(schema);
    txt_head=React_TxtRec_Head_Get(text);

    React_Head_SynthemeFG_Put (sch_head, new_chap_num);
    React_TxtHd_OrigSchema_Put (txt_head, last_schnum[lib - 1][new_chap_num - 1] + 1);
    text->lit_refs = (String_t *) realloc (text->lit_refs, nrefs * sizeof (String_t));
    React_TxtHd_NumReferences_Put (txt_head, nrefs);
    sprintf (ref_string_string, "\007Moved from Library %d, Chapter %d, Schema %d", lib, chap, sch);
    ref_string = String_Create ((const char *) /* WHY??? */ ref_string_string, 0);
    React_TxtRec_Reference_Put (text, nrefs - 1, ref_string);
    React_Schema_Write (NSch - 1);
/*
    XmListSetPos(schlistw,schlist_pos);
    XmListSelectPos(schlistw,schlist_pos,True);
*/
    Persist_Update_Rxn (NSch - 1, schinx[schlist_pos - 1], FALSE, FALSE, "move_schema");
    schinx[schlist_pos - 1] = NSch - 1;

  sprintf (msgstr, "Schema %d has been moved from Library %d, Chapter %d to Library %d, Chapter %d, Schema %d",
    sch, lib, chap, lib, new_chap_num, last_schnum[lib - 1][new_chap_num - 1] + 1);
  label = XmStringCreateLocalized (msgstr);
  msg = XmStringCreateLocalized (msg_str);
  XtVaSetValues (mess_box,
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree (label);
  XmStringFree (msg);
  XtManageChild (mess_box);

  if (chap_num != 0)
  {
    chap_num = new_chap_num;
lib_num = sch_count[0][0] = sch_count[1][0] = sch_count[2][0] = 0;
    update_schemata (TRUE);
  }
}

void change_password ()
{
  if (clearance_level < CHG_PW_LEV)
    {
    LoginFail (tl);
    return;
    }

  ChangePassword (tl);
}

void change_clearance ()
{
  if (clearance_level < CHG_LEV_LEV)
    {
    LoginFail (tl);
    return;
    }

  ChangeLevel (tl);
}

void maintenance ()
{
  if (clearance_level < MAINT_LEV)
    {
    LoginFail (tl);
    return;
    }
  printf ("Dummy maintenance\n");
}

void add_new_user ()
{
  if (clearance_level < ADD_USR_LEV)
    {
    LoginFail (tl);
    return;
    }

  AddUser (tl);
}

void delete_user ()
{
  if (clearance_level < DEL_USR_LEV)
    {
    LoginFail (tl);
    return;
    }

  DelUser (tl);
}

void list_users ()
{
  if (clearance_level < LIST_USR_LEV)
    {
    LoginFail (tl);
    return;
    }

  ListUsers (tl);
}

void rxl_ist_refresh (int last_viewed_pos)
{
        schlist_pos=last_viewed_pos;
        XmListSetPos(schlistw,schlist_pos);
        XmListSelectPos(schlistw,schlist_pos,True);
}

void refresh_schema (Boolean_t save, Boolean_t close_schema, Boolean_t saved_elsewhere)
{
  int save_lib_num, save_chap_num, saved_schlist_pos, nrefs, hour, minute, now, i;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  Date_t today;
  String_t modify_string;
  char modification[128], today_str[16];
  Boolean_t write_ref;
  static Boolean_t new_schema_still_open = FALSE;
ScreenAttr_t *scra_p;

scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

/*
  XtManageChild (frame);
*/
  if (glob_sel_sch != 0) XtVaSetValues(tl,XmNheight,Screen_Height_Get (scra_p) - SWS_APPSHELL_OFFY,
                XmNwidth,Screen_Width_Get (scra_p) - SWS_APPSHELL_OFFX,
                XmNtitle,"Reaction Library Editor",XmNx,0,XmNy,0,NULL);
  else XtVaSetValues(tl,
    XmNx, /* glob_x,*/ 0,
    XmNy, /* glob_y,*/ 0,
    XmNwidth, glob_width,
    XmNheight, glob_height,
    NULL);

  save_lib_num=lib_num;
  save_chap_num=chap_num;

  if (!save)
  {
    if (saved_elsewhere)
      {
      last_schema_closed = FALSE;

      saved_schlist_pos = schlist_pos;
      lib_num=chap_num=sch_count[0][0]=sch_count[1][0]=sch_count[2][0]=0;
      update_schemata (FALSE);

      if (save_lib_num == 0) lib_num = 0;
      else lib_num=3;
      chap_num=save_chap_num;
      update_schemata (TRUE);
      if (new_schema_flag)
        {
        new_schema_still_open = TRUE;
        XmListSetPos(schlistw,schlist_bottom);
        XmListSelectPos(schlistw,schlist_bottom,True);
        }
      else
        {
        XmListSetPos(schlistw,saved_schlist_pos);
        XmListSelectPos(schlistw,saved_schlist_pos,True);
        }
      }
    else
      {
      save_NSch--;
      NSch--;
      }

    new_schema_flag = FALSE;

    return;
  }

  last_schema_closed = close_schema;

  if (new_schema_flag && !last_schema_closed) new_schema_still_open = TRUE;

/*
only valid after write - not yet implemented
UPDATE: now that write is implemented, it STILL seems to serve no useful purpose

  if (NSch != React_NumSchemas_Get ())
    {
    printf ("Error in refresh_schema()\n");
    exit(1);
    }
  if (NSch == save_NSch) return;
*/

  schema = React_Schema_Handle_Get (NSch - 1);
  sch_head = React_Head_Get(schema);
  text=React_Text_Get(schema);
  txt_head=React_TxtRec_Head_Get(text);

  if (!new_schema_flag)
    {
    today = Persist_Today ();
    now = Persist_Now () / 60;
    if (new_schema_still_open)
    /* update creation date to retain synchronization between date_last_mod and time_last_mod in persist index */
      {
      React_TxtHd_Created_Put(txt_head, today);
      nrefs = React_TxtHd_NumReferences_Get (txt_head);
      for (i = nrefs - 1; i >= 0 && String_Value_Get (React_TxtRec_Reference_Get (text, i))[0] != '\007'; i--);
      if (i < 0)
        {
        printf ("Warning: no <LIBRARY UPDATE> record exists for schema %d\n", NSch - 1);
        write_ref = FALSE;
        }
      else write_ref = TRUE;
      if (write_ref)
        {
        String_Destroy (React_TxtRec_Reference_Get (text, i));
        date_calc (today, today_str, NULL, NULL, NULL);
        hour = now / 60;
        minute = now - 60 * hour;
        sprintf (modification, "\007Schema created by %s on %s at %02d%02d", UserId (), today_str, hour, minute);
        modify_string = String_Create ((const char *) modification, 0);
        React_TxtRec_Reference_Put (text, i, modify_string);
        }
      if (last_schema_closed) new_schema_still_open = FALSE;
      }
    else
      {
      React_TxtHd_LastMod_Put(txt_head, today);
      date_calc (today, today_str, NULL, NULL, NULL);
      hour = now / 60;
      minute = now - 60 * hour;
      sprintf (modification, "\007Schema modified by %s on %s at %02d%02d", UserId (), today_str, hour, minute);
      modify_string = String_Create ((const char *) modification, 0);
      if (last_schema_closed)
        {
        nrefs = React_TxtHd_NumReferences_Get (txt_head) + 1;
        React_TxtHd_NumReferences_Put (txt_head, nrefs);
        text->lit_refs = (String_t *) realloc (text->lit_refs, nrefs * sizeof (String_t));
        React_TxtRec_Reference_Put (text, nrefs - 1, modify_string);
        }
      else
        {
        nrefs = React_TxtHd_NumReferences_Get (txt_head);
        for (i = nrefs - 1; i >= 0 && String_Value_Get (React_TxtRec_Reference_Get (text, i))[0] != '\007'; i--);
        if (i < 0)
          {
          printf ("Warning: no <LIBRARY UPDATE> record exists for schema %d\n", NSch - 1);
          write_ref = FALSE;
          }
        else write_ref = TRUE;
        if (write_ref)
          {
          String_Destroy (React_TxtRec_Reference_Get (text, i));
          React_TxtRec_Reference_Put (text, i, modify_string);
          }
        }
      }
    }

  backup_rxnlib ();

  React_Schema_Write (NSch - 1);

  if (new_schema_flag)
    Persist_Add_Rxn (NSch - 1, FALSE, "refresh_schema");
  else
    {
    Persist_Update_Rxn (NSch - 1, schinx[schlist_pos - 1], FALSE, FALSE, "refresh_schema");
/*
    schema = React_Schema_Handle_Get (schinx[schlist_pos-1]);
    sch_head = React_Head_Get(schema);
    React_HeadFlags_Disabled_Put (sch_head, TRUE);
    React_Schema_Write (schinx[schlist_pos-1]);
*/
    }

  saved_schlist_pos = schlist_pos;
  lib_num=chap_num=sch_count[0][0]=sch_count[1][0]=sch_count[2][0]=0;
  update_schemata (FALSE);

  if (save_lib_num == 0) lib_num = 0;
  else lib_num=3;
  chap_num=save_chap_num;
  update_schemata (TRUE);
  if (new_schema_flag)
    {
    XmListSetPos(schlistw,schlist_bottom);
    XmListSelectPos(schlistw,schlist_bottom,True);
    }
  else
    {
    XmListSetPos(schlistw,saved_schlist_pos);
    XmListSelectPos(schlistw,saved_schlist_pos,True);
    }

  new_schema_flag = FALSE;
}

void no_schemata (char *func)
{
  char msgstr[100];
  XmString label, msg;

  label = XmStringCreateLocalized ("Error: No Schemata Listed");
  sprintf (msgstr, "There are no schemata listed - can't %s!", func);
  msg = XmStringCreateLocalized (msgstr);
  XtVaSetValues (mess_box,
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree (label);
  XmStringFree (msg);
  XtManageChild (mess_box);
}

void run_isthere (Boolean_t forward)
{
/*
  printf ("Dummy IsThere Execution\n");
*/
  IsThere (tl, schlC, schinx + schlist_pos - 1, &save_NSch, last_schema_closed, forward);
}

Boolean_t check_dots (int sch_num, char *buffer, Boolean_t new, Boolean_t tsd_altered)
{
  Tsd_t *gtsd, *sgtsd;
  Xtr_t *gxtr, *sgxtr;
  React_Record_t *schema;
  React_Head_t *sch_head;
  U32_t roots[MX_ROOTS];
  int i;

  schema = React_Schema_Handle_Get (sch_num);
  sch_head=React_Head_Get(schema);

  gtsd = React_Goal_Get (schema);

  if (!tsd_altered) for (i = 0; i < Tsd_NumAtoms_Get (gtsd); i++) if (Tsd_AtomFlags_Dot_Get (gtsd, i)) return (FALSE);

  sgtsd = React_Subgoal_Get (schema);

  gxtr = Tsd2Xtr (gtsd);
  sgxtr = Tsd2Xtr (sgtsd);

  for (i = 0; i < MX_ROOTS; i++) roots[i]=React_Head_RootNode_Get(sch_head, i);

  dots (gxtr, sgxtr, roots, buffer);
printf("%s\n",buffer);

  Xtr_Destroy (sgxtr);
  Tsd_Destroy (gtsd);

  React_Goal_Put (schema, Xtr2Tsd (gxtr));

  Xtr_Destroy (gxtr);

  if (new) return (FALSE);

  backup_rxnlib ();
  React_Schema_Write (sch_num);

  return (TRUE);
}

void backup_rxnlib ()
{
  FILE *source, *destination;
  int c;

  source = fopen (FCB_SEQDIR_RXNS ("/rkbstd.inx"), "rb");
  destination = fopen (FCB_SEQDIR_RXNS ("/rkbstd.inx.bak"), "wb");
  while ((c = getc (source)) != EOF) putc(c, destination);
  fclose (source);
  fclose (destination);

  source = fopen (FCB_SEQDIR_RXNS (REACT_DATANAME), "rb");
  destination = fopen (FCB_SEQDIR_RXNS (REACT_DATANAME ".bak"), "wb");
  while ((c = getc (source)) != EOF) putc(c, destination);
  fclose (source);
  fclose (destination);

  source = fopen (FCB_SEQDIR_RXNS (REACT_TEXTNAME), "rb");
  destination = fopen (FCB_SEQDIR_RXNS (REACT_TEXTNAME ".bak"), "wb");
  while ((c = getc (source)) != EOF) putc(c, destination);
  fclose (source);
  fclose (destination);
}
