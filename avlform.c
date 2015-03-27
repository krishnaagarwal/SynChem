/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     AVLFORM.C
*
*    The available compounds editor display and editing functions.
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
* 2/6/2001   Miller     Restore slings to all-upper before storing!
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

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_SUBMIT_
#include "submit.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_LOGIN_
#include "login.h"
#endif

#define DEL_AVL_LEV 8
#define ADD_AVL_LEV ADD_SCH_LEV
#define EDIT_AVL_LEV EDIT_TRANSFORM_LEV
#define AVL_ADD_FROM_SRCH 255

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

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_SLING2XTR_
#include "sling2xtr.h"
#endif

#ifndef _H_RXNPATT_DRAW_
#include "rxnpatt_draw.h"
#endif

#ifndef _H_AVLFORM_
#define _GLOBAL_DEF_
#include "avlform.h"
#undef _GLOBAL_DEF_
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

static char glob_avl_comp[512],
  *unspec[] = {"<<< UNSPECIFIED SUPPLIER >>>", "<<< COMPOUND NAME OMITTED >>>", "<<< STRUCTURE UNDEFINED >>>", ""};
static char *nmstr=NULL, *slstr=NULL;
static Boolean_t was_by_name = TRUE;
static char exe_dir[128];

void AvlEdit_Create_Form (Widget top_level, XtAppContext schlContext, char *avl_comp)
{
	ScreenAttr_t     *sca_p;
	XmFontList flhv14,flhv18,flco12,flco18,fl6x10;
	XmFontListEntry helv14,helv18,cour12,cour18,fs6x10;
	Widget frame, topform, avllist_form, avldisp_form, liblistwin, avllistwin, textwin,
		avlform,toppaned,ldummy,cdummy,editpb,helppb,srch_pb,srch_lbl,info_pb[4], parentw[3], vertsep;
	XtWidgetGeometry geom;
	int i;
	char itemstr[500], widname[32];
	XmString label, labels[41];
	Boolean_t first_avl;
int pos;
#ifdef _CYGHAT_
        Widget box; /* kludge for lesstif and OpenMotif */
Arg al[50];
int ac;
#endif

strcpy(exe_dir, getenv ("SYNEXE"));
  tl = top_level;
  schlC = schlContext;
  strcpy (glob_avl_comp, avl_comp);
  if (glob_avl_comp[strlen(glob_avl_comp)-1]==' ')
  {
    first_avl = FALSE;
    glob_avl_comp[strlen(glob_avl_comp)-1]=0;
  }
  else first_avl = TRUE;

  frame = XtVaCreateManagedWidget ("AvlEditFr",
    xmMainWindowWidgetClass,  top_level,
    XmNshadowType,       XmSHADOW_IN,
    XmNshadowThickness,  3,
    XmNmarginWidth,      AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight,     AppDim_MargFmFr_Get (&GAppDim),
    NULL);

  topform = XtVaCreateWidget ("form", xmFormWidgetClass, frame, NULL);

  toppaned = XtVaCreateWidget ("paned", xmPanedWindowWidgetClass, topform, NULL);

  misc_form = XtVaCreateWidget ("MiscFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     False,
    XmNpositionIndex,   0,
    XmNpaneMinimum,     60,
    XmNpaneMaximum,     60,
    XmNfractionBase,    800,
    NULL);

  avlform = XtVaCreateWidget ("AvlFm",
    xmFormWidgetClass,  toppaned,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNallowResize,     True,
    XmNpaneMaximum,     1600,
    XmNfractionBase,    800,
    NULL);

  avllist_form = XtVaCreateWidget ("AvlListFm",
    xmFormWidgetClass,  avlform,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    XmNfractionBase,    800,
    NULL);

  avldisp_form = XtVaCreateWidget ("RxlDispFm",
    xmFormWidgetClass,  avlform,
    XmNlabelFontList,   SynAppR_FontList_Get (&GSynAppR),
    XmNmarginHeight,    AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginWidth,     AppDim_MargFmFr_Get (&GAppDim),
    XmNresizable,       False,
    XmNfractionBase,    800,
    NULL);

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

  XtVaSetValues (avllist_form,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightPosition,    500,
    XmNrightAttachment,  XmATTACH_POSITION,
    NULL);

  XtVaSetValues (avldisp_form,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       avllist_form,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  helv14 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  helv18 = XmFontListEntryLoad (XtDisplay (topform), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  cour12 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-12-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  cour18 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  fs6x10 = XmFontListEntryLoad (XtDisplay (topform), "-*-courier-medium-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv14 = XmFontListAppendEntry (NULL, helv14);
  flhv18 = XmFontListAppendEntry (NULL, helv18);
  flco12 = XmFontListAppendEntry (NULL, cour12);
  flco18 = XmFontListAppendEntry (NULL, cour18);
  fl6x10 = XmFontListAppendEntry (NULL, fs6x10);

  ldummy = XmVaCreateSimplePulldownMenu (misc_form, "_lpulldown", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  label = XmStringCreateLocalized("Library");
  labels[0] = XmStringCreateLocalized("All");

  for (i = 0; i < 3; i++)
    {
    sprintf (itemstr, "%d: %s", i + 1, alibname[i]);
    labels[i + 1] = XmStringCreateLocalized (itemstr);
    }

  labels[4] = XmStringCreateLocalized("Exit");
  labels[5] = XmStringCreateLocalized("Exit - Force Update");

  liblistw = XmVaCreateSimpleOptionMenu (misc_form, "LibMenu", label, '\0', 0, AvLib_CB,
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
        XmVaPUSHBUTTON, labels[5], '\0', NULL, NULL,
        NULL);
  XmStringFree (label);
  XmStringFree (labels[0]);
  XmStringFree (labels[1]);
  XmStringFree (labels[2]);
  XmStringFree (labels[3]);
  XmStringFree (labels[4]);
  XmStringFree (labels[5]);

  cdummy = XmVaCreateSimplePulldownMenu (misc_form, "_cpulldown", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  for (i=0; i<250; i++) itemstr[i]=' ';
  itemstr[i]=0;

  for (i=0; i<41; i++)
    labels[i]=XmStringCreateLocalized(itemstr);

  editpb=XmCreatePushButton(misc_form,"EditPB",NULL,0);
  label=XmStringCreateLocalized(" Edit Compound ");
  XtVaSetValues(editpb,
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNlabelString, label,
	XmNfontList, flhv18,
        NULL);
  XmStringFree (label);

  XtAddCallback (editpb, XmNactivateCallback, AvEdit_CB, (XtPointer) NULL);

  helppb=XmCreatePushButton(misc_form,"HelpPB",NULL,0);
  label=XmStringCreateLocalized(" Help ");
  XtVaSetValues(helppb,
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNlabelString, label,
	XmNfontList, flhv18,
        NULL);
  XmStringFree (label);

  XtAddCallback (helppb, XmNactivateCallback, Help_CB, (XtPointer) "avlform:Available Compound Editor");

  label=XmStringCreateLocalized(" Additional Operations ");

  othermb=XmVaCreateSimpleMenuBar(misc_form,"othermb",
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmVaCASCADEBUTTON,label,'\0',
        NULL);

  XmStringFree (label);

  XtVaSetValues(XtNameToWidget(othermb,"button_0"),
    XmNfontList, flhv18,
    NULL);

  labels[0]=XmStringCreateLocalized("Add Compound");
  labels[1]=XmStringCreateLocalized("Delete Compound");
  labels[2]=XmStringCreateLocalized("Forward Search for Compound Name (from top)");
  labels[3]=XmStringCreateLocalized("Forward Search for Compound Name (below current position)");
  labels[4]=XmStringCreateLocalized("Continue Search Forward");
  labels[5]=XmStringCreateLocalized("Backward Search for Compound Name (from bottom)");
  labels[6]=XmStringCreateLocalized("Backward Search for Compound Name (above current position)");
  labels[7]=XmStringCreateLocalized("Continue Search Backward");
  labels[8]=XmStringCreateLocalized("Search by Structure");
  labels[9]=XmStringCreateLocalized("Close This Menu");

  otherpdm = XmVaCreateSimplePulldownMenu (othermb, "_opulldown", 0, AvOther_CB,
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
          SAR_CLRI_WHITE),
        XmNbuttonFontList, flhv18,
        XmNlabelString,    label,
        XmNtearOffModel, XmTEAR_OFF_ENABLED,
        XmVaPUSHBUTTON, labels[0], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[1], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[2], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[3], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[4], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[5], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[6], '\0', NULL, NULL,
        XmVaPUSHBUTTON, labels[7], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[8], '\0', NULL, NULL,
        XmVaSEPARATOR,
        XmVaPUSHBUTTON, labels[9], '\0', NULL, NULL,
        NULL);

  for (i=0; i<10; i++)
	XmStringFree(labels[i]);

  avllistwin = XmCreateScrolledWindow (avllist_form, "Compound List", NULL, 0);
  avllistw = XmCreateList (avllistwin, "Compound List", NULL, 0);

  XtVaSetValues(avllistw,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmNfontList,flco18,
        XmNselectionPolicy,          XmSINGLE_SELECT,
        XmNtitle,"Compounds",XmNx,0,XmNy,0,NULL);

  XtAddCallback (avllistw, XmNsingleSelectionCallback, AvlCmp_CB, (XtPointer) NULL);

  vertsep = XtVaCreateManagedWidget ("VSep",
    xmSeparatorWidgetClass, misc_form,
    XmNorientation, XmVERTICAL,
    NULL);

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

/*
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
    NULL);
*/

  XtVaSetValues (editpb,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       liblistw,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (helppb,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       editpb,
    XmNrightOffset,      0,
#ifdef _CYGWIN_
    XmNrightAttachment,  XmATTACH_NONE,
#else
    XmNrightAttachment,  XmATTACH_OPPOSITE_WIDGET,
    XmNrightWidget,      editpb,
#endif
    NULL);

  XtVaSetValues (othermb,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       helppb,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (vertsep,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_WIDGET,
    XmNleftWidget,       othermb,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_NONE,
    NULL);

  XtVaSetValues (avllistwin,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  textwin = XmCreateScrolledWindow (avldisp_form, "Compound Data", NULL, 0);

#ifdef _CYGHAT_
ac = 0;
XtSetArg(al[ac],
        XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_WHITE)); ac++;
XtSetArg(al[ac],
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR, SAR_CLRI_BLACK)); ac++;
XtSetArg(al[ac],
        XmNfontList, flco18); ac++;
XtSetArg(al[ac],
        XmNscrollingPolicy, XmAUTOMATIC); ac++;
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
        XmNvalue, avlbuf); ac++;
XtSetArg(al[ac],
        XmNmarginHeight, 0); ac++;
XtSetArg(al[ac],
        XmNmarginWidth, 0); ac++;
XtSetArg(al[ac],
        XmNtitle,"Compound Data"); ac++;
XtSetArg(al[ac],
        XmNx,0); ac++;
XtSetArg(al[ac],
        XmNy,0); ac++;
  textw = XmCreateText (textwin, "Compound Data", al, ac);
#else
  textw = XmCreateText (textwin, "Compound Data", NULL, 0);

  XtVaSetValues(textw,XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_WHITE),
        XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
        SAR_CLRI_BLACK),
        XmNfontList, flco18,
        XmNscrollingPolicy, XmAUTOMATIC,
        XmNscrollBarDisplayPolicy, XmAS_NEEDED,
#ifdef _CYGHAT_
        XmNscrollVertical, True,
#endif
        XmNeditMode, XmMULTI_LINE_EDIT,
        XmNeditable, False,
        XmNautoShowCursorPosition, False,
        XmNcursorPositionVisible, False,
        XmNvalue, avlbuf,
        XmNmarginHeight, 0,
        XmNmarginWidth, 0,
        XmNtitle,"Compound Data",
        XmNx,0,
        XmNy,0,
        NULL);
#endif

#ifdef _CYGHAT_
  box = XtVaCreateManagedWidget ("box", xmRowColumnWidgetClass, avldisp_form,
    XmNleftAttachment,   XmATTACH_FORM, /* and all this time I've been wasting my time putting in 0-offsets by example!! */
    XmNrightAttachment,  XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_POSITION,
    XmNbottomPosition,   320,
    XmNorientation,      XmHORIZONTAL,
    XmNheight,           1,
    NULL);
#endif

  XtVaSetValues (textwin,
#ifdef _CYGHAT_
    XmNscrollVertical,   True,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_WIDGET,
    XmNbottomWidget,     box,
#else
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_FORM,
    XmNbottomPosition,   320,
    XmNbottomAttachment, XmATTACH_POSITION,
#endif
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  srch_popup = XmCreateFormDialog (top_level, "SrchFmDg", NULL, 0);

  label = XmStringCreateLocalized ("Search");
  XtVaSetValues (srch_popup,
    XmNdialogTitle,  label,
    XmNdialogStyle,  XmDIALOG_MODELESS,
    XmNautoUnmanage, False,
    NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("Search compounds for:");
  srch_lbl =  XtVaCreateManagedWidget ("SearchLbl",
  xmLabelWidgetClass, srch_popup,
	XmNfontList, flhv18,
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
        XmNeditable, True,
	XmNfontList, flhv18,
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
	XmNfontList, flhv18,
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
  XtAddCallback (srch_pb, XmNactivateCallback, AvlSearch_CB, (XtPointer) NULL);

  label = XmStringCreateLocalized ("Draw New Structure");
  srch_draw_pb[0] =  XtVaCreateWidget ("SearchDrawPB0",
        xmPushButtonGadgetClass, srch_popup,
	XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   srch_txt,
        XmNtopOffset, 0,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, srch_pb,
        XmNrightOffset, 0,
        NULL);
  XmStringFree (label);
  XtAddCallback (srch_draw_pb[0], XmNactivateCallback, AvlDraw_CB, (XtPointer) srch_txt);

  label = XmStringCreateLocalized ("Modify Current Structure");
  modpb[2] = srch_draw_pb[1] =  XtVaCreateWidget ("SearchDrawPB1",
        xmPushButtonGadgetClass, srch_popup,
	XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   srch_txt,
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, srch_draw_pb[0],
        XmNrightOffset, 0,
        NULL);
  XmStringFree (label);
  XtAddCallback (srch_draw_pb[1], XmNactivateCallback, AvlDraw_CB, (XtPointer) srch_txt);

  info_popup = XmCreateFormDialog (top_level, "InfoFmDg", NULL, 0);

  label = XmStringCreateLocalized ("Compound Info");
  XtVaSetValues (info_popup,
    XmNdialogTitle,  label,
    XmNdialogStyle,  XmDIALOG_MODELESS,
    XmNautoUnmanage, False,
    XmNfractionBase, 200,
    NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("Name:");
  info_lbl[0] =  XtVaCreateManagedWidget ("InfoLbl0",
  xmLabelWidgetClass, info_popup,
	XmNfontList, flhv18,
        XmNlabelString,  label,
        XmNtopAttachment, XmATTACH_FORM,
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("Catalog #:");
  info_lbl[1] =  XtVaCreateManagedWidget ("InfoLbl1",
  xmLabelWidgetClass, info_popup,
	XmNfontList, flhv18,
        XmNlabelString,  label,
        XmNtopAttachment, XmATTACH_FORM,
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_POSITION,
        XmNleftPosition, 135,
        NULL);
  XmStringFree (label);

  info_txt[0] =  XtVaCreateManagedWidget ("InfoTxt0",
        xmTextWidgetClass, info_popup,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
	XmNfontList, flhv18,
        XmNresizeWidth, True,
        XmNmaxLength, 475,
        XmNvalue, "",
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, info_lbl[0],
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightOffset, 0,
        XmNrightWidget, info_lbl[1],
        NULL);

  info_txt[1] =  XtVaCreateManagedWidget ("InfoTxt1",
        xmTextWidgetClass, info_popup,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
	XmNfontList, flhv18,
        XmNresizeWidth, True,
        XmNmaxLength, 475,
        XmNvalue, "",
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, info_lbl[1],
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
        XmNleftOffset, 0,
        XmNleftWidget, info_lbl[1],
        NULL);

  XtVaSetValues (info_popup,
    XmNinitialFocus, info_txt[0],
    NULL);

  label = XmStringCreateLocalized ("Save Data");
  info_pb[1] =  XtVaCreateManagedWidget ("InfoPB1",
        xmPushButtonGadgetClass, info_popup,
	XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   info_txt[1],
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_OPPOSITE_WIDGET,
        XmNleftWidget,   info_txt[1],
        XmNleftOffset, 0,
        NULL);
  XmStringFree (label);
  XtAddCallback (info_pb[1], XmNactivateCallback, AvlStore_CB, (XtPointer) NULL);

  label = XmStringCreateLocalized ("Cancel");
  info_pb[2] =  XtVaCreateManagedWidget ("InfoPB2",
        xmPushButtonGadgetClass, info_popup,
	XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   info_txt[1],
        XmNtopOffset, 0,
        XmNrightAttachment, XmATTACH_FORM,
        XmNrightOffset, 0,
        NULL);
  XmStringFree (label);
  XtAddCallback (info_pb[2], XmNactivateCallback, AvlSkipInfo_CB, (XtPointer) NULL);

  label = XmStringCreateLocalized ("Draw New Structure");
  info_pb[0] =  XtVaCreateManagedWidget ("Info_PB0",
        xmPushButtonGadgetClass, info_popup,
	XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   info_txt[0],
        XmNtopOffset, 0,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, info_pb[1],
        XmNrightOffset, 0,
        NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("Modify Current Structure");
  modpb[1] = info_pb[3] =  XtVaCreateWidget ("InfoPB3",
        xmPushButtonGadgetClass, info_popup,
	XmNfontList, flhv18,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNlabelString, label,
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget,   info_txt[0],
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        XmNrightAttachment, XmATTACH_WIDGET,
        XmNrightWidget, info_pb[0],
        XmNrightOffset, 0,
        NULL);
  XmStringFree (label);

  info_txt[2] =  XtVaCreateManagedWidget ("InfoTxt2",
        xmTextWidgetClass, info_popup,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNeditable, False,
	XmNfontList, flco12,
        XmNresizeWidth, True,
        XmNmaxLength, 475,
        XmNvalue, "",
        XmNtopAttachment, XmATTACH_WIDGET,
        XmNtopWidget, info_pb[0],
        XmNtopOffset, 0,
        XmNleftAttachment, XmATTACH_FORM,
        XmNleftOffset, 0,
        NULL);
  XtAddCallback (info_pb[0], XmNactivateCallback, AvlDraw_CB, (XtPointer) info_txt[2]);
  XtAddCallback (info_pb[3], XmNactivateCallback, AvlDraw_CB, (XtPointer) info_txt[2]);

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

  parentw[0] = avldisp_form;
  parentw[1] = info_popup;
  parentw[2] = srch_popup;

  for (i=0; i<3; i++)
  {
    sprintf (widname, "AvlCompDrawingW%d", i);
    drawwin[i] = XtVaCreateWidget (widname,
      xmScrolledWindowWidgetClass, parentw[i],
      XmNscrollingPolicy,          XmAUTOMATIC,
      XmNscrollBarDisplayPolicy,   XmAS_NEEDED,
      NULL);

    sprintf (widname, "AvlCompDrawing%d", i);
    draww[i] = XtVaCreateManagedWidget (widname,
      xmDrawingAreaWidgetClass, drawwin[i],
      XmNbackground,        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
      XmNheight,            2 * PDRW_DA_PXMP_HT / (i + 2),
      XmNwidth,             2 * PDRW_DA_PXMP_WD / (i + 2),
      XmNfontList, fl6x10,
      XmNresize, False,
      NULL);


    XtAddCallback (draww[i],
      XmNexposeCallback,
      redraw_compound, (XtPointer) i);

    drawp[i] = XCreatePixmap (disp, RootWindow(disp, DefaultScreen(disp)),
      2 * PDRW_DA_PXMP_WD / (i + 2), 2 * PDRW_DA_PXMP_HT / (i + 2), Screen_Depth_Get (sca_p));

    XFillRectangle (disp, drawp[i], gc, 0, 0, 2 * PDRW_DA_PXMP_WD / (i + 2), 2 * PDRW_DA_PXMP_HT / (i + 2));
  }

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

  XtVaSetValues (drawwin[0],
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        textwin,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (drawwin[1],
    XmNwidth,            PDRW_DA_PXMP_WD / 2,
    XmNheight,           PDRW_DA_PXMP_HT / 2,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        info_txt[2],
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  XtVaSetValues (drawwin[2],
    XmNwidth,            PDRW_DA_PXMP_WD / 3,
    XmNheight,           PDRW_DA_PXMP_HT / 3,
    XmNtopOffset,        0,
    XmNtopAttachment,    XmATTACH_WIDGET,
    XmNtopWidget,        srch_pb,
    XmNbottomOffset,     0,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftOffset,       0,
    XmNleftAttachment,   XmATTACH_FORM,
    XmNrightOffset,      0,
    XmNrightAttachment,  XmATTACH_FORM,
    NULL);

  mess_box = XmCreateMessageDialog (top_level, "Message", NULL, 0);
  labels[0]=XmStringCreateLocalized("Dismiss");
  labels[1]=XmStringCreateLocalized("Add Compound");
  XtVaSetValues (mess_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, labels[0],
        XmNhelpLabelString, labels[1],
        NULL);
  XmStringFree(labels[0]);
  XmStringFree(labels[1]);

  XtUnmanageChild (XmMessageBoxGetChild (mess_box, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (mess_box, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (mess_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, AvlMBDismiss_CB,
    (XtPointer) NULL);
  XtAddCallback (XmMessageBoxGetChild (mess_box, XmDIALOG_HELP_BUTTON), XmNactivateCallback, AvOther_CB,
    (XtPointer) AVL_ADD_FROM_SRCH);
  XtUnmanageChild (mess_box);

  conf_box = XmCreateMessageDialog (top_level, "Confirm", NULL, 0);
  label=XmStringCreateLocalized("Confirm");
  XtVaSetValues (conf_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNdialogTitle, label,
        NULL);
  XmStringFree(label);

  XtUnmanageChild (XmMessageBoxGetChild (conf_box, XmDIALOG_HELP_BUTTON));

  XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, AvlConfirm_CB,
    (XtPointer) (int) TRUE);
  XtAddCallback (XmMessageBoxGetChild (conf_box, XmDIALOG_CANCEL_BUTTON), XmNactivateCallback, AvlConfirm_CB,
    (XtPointer) (int) FALSE);
  XtUnmanageChild (conf_box);

  XtManageChild (srch_txt);
  XtManageChild (srch_pb);

  XtManageChild (liblistw);

  XtVaSetValues(XmOptionLabelGadget(liblistw),
        XmNfontList,flhv18,
        NULL);

  XtManageChild(editpb);
  XtManageChild(helppb);

  XtManageChild(othermb);

  /* Set misc_form so that it can't be resized */
  geom.request_mode = CWHeight;
  XtQueryGeometry (liblistw, NULL, &geom);

  XtManageChild (misc_form);
               XtVaSetValues (misc_form,
               XmNpaneMinimum, geom.height,
               XmNpaneMaximum, geom.height,
               NULL);

  XtManageChild (textw);
  XtManageChild (draww[0]);
  XtManageChild (draww[1]);
  XtManageChild (draww[2]);

  XtManageChild (avllistw);

  XtManageChild (textwin);

  XtManageChild (drawwin[0]);

  XtManageChild (avllistwin);

  XtManageChild (avllist_form);

  XtManageChild (avldisp_form);

  XtManageChild (avlform);

  XtManageChild (toppaned);
  XtManageChild (topform);

  XtManageChild (frame);

  update_compounds(TRUE);

if (glob_avl_comp[0]!=0)
{
  srch_fwd=TRUE;
  was_by_name=FALSE;
  slstr=glob_avl_comp;
pos=AvSearch_Pos(slstr, srch_pos, srch_fwd, TRUE);
if (pos)
  {
  XmListSetPos(avllistw,pos);
  XmListSelectPos(avllistw,pos,True);
  }
else
  {
  glob_txt_widget=srch_txt;
  XmTextSetString (srch_txt, slstr);
  XtVaSetValues (srch_txt,
        XmNeditable, False,
        NULL);
  XtManageChild(srch_draw_pb[0]);
  XtManageChild(srch_draw_pb[1]);
  XtManageChild (drawwin[2]);
  XtManageChild(srch_popup);
  Avl_Drawn_Sling(slstr);
  }
}

  if (first_avl) InfoWarn_Show ("AvlEdit Notice~NOTE: In the event of an abnormal exit, the dictionary will not be updated, in which case you must restart and \"Exit - Force Update.\"");
}

void AvLib_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  char msg_str[600];
  XmString msg;
  XmRowColumnCallbackStruct *cbs = (XmRowColumnCallbackStruct *) call_data;

  if (cbs->reason != XmCR_ACTIVATE) return;

  alib_num = (int)client_data;

  if (alib_num == 4 || alib_num == 5)
    {
    exit_set = TRUE;
    if (alib_num == 5) db_modified = TRUE;
    sprintf (msg_str, "Are you sure you want to exit?");
    msg = XmStringCreateLocalized (msg_str);
    XtVaSetValues (conf_box,
        XmNmessageString, msg,
        NULL);
    XmStringFree (msg);
    XtManageChild (conf_box);
    }
  else update_compounds (TRUE);
}

void AvEdit_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  if (avllist_pos == 0)
    {
    no_avlcomps ("view");
    return;
    }
  if (logged_in) AvEdit_Cont ();
  else
    {
    if (login_failed) LoginFail (tl);
    else Login (tl, AvEdit_Cont, NULL);
    }
}

void AvEdit_Cont ()
{
  int *plist, pcount;
  Avl_Record_t *avlcmp;
  char *cat, *nam, *slg;

  if (clearance_level < EDIT_AVL_LEV)
    {
    LoginFail (tl);
    return;
    }

  XmListGetSelectedPos (avllistw, &plist, &pcount);
  editing_record = plist[0] - 1;
  XtFree ((char *) plist);
  if (Avl_Record_Empty (editing_record))
  {
    editing_record = -1;
    InfoWarn_Show ("AvlEdit Notice~This record is reserved for reuse and may not be edited.");
    return;
  }
  avlcmp = Avl_Record_Get (editing_record);
  cat=Avl_Comp_CatNumber_Get (avlcmp);
  if (strcmp(cat,unspec[0])==0) cat=unspec[3];
  nam=Avl_Comp_Name_Get (avlcmp);
  if (strcmp(nam,unspec[1])==0) nam=unspec[3];
  slg=Sling_Name_Get (Avl_Comp_CanSling_Get (avlcmp));
  strcpy (sshot_comp, slg);
  if (slg[0]==0)
  {
    slg=unspec[2];
    XtUnmanageChild (modpb[1]);
    XtUnmanageChild (drawwin[1]);
  }
  else
  {
    XtManageChild (modpb[1]);
    XtManageChild (drawwin[1]);
  }
  XmTextSetString (info_txt[0], nam);
  XmTextSetString (info_txt[1], cat);
  XmTextSetString (info_txt[2], slg);
  XtManageChild (info_popup);
  if (XtIsManaged (drawwin[1])) AvlCmp_CB (info_txt[2], slg, (XtPointer) NULL);
}

void AvOther_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  int c, pos, lib, chap, sch;
  Sling_t cmp;
  char string[500],*s, *lcl_srch_str, msg_str[600], *nam, *cat;
  XmString msg,label;
  Avl_Record_t *avlcmp;
  Widget pd_parent, pd_grandparent;
/*
  static char *nmstr=NULL, *slstr=NULL;
  static Boolean_t was_by_name = TRUE;
*/

  switch((int)client_data)
  {
  case 0:
  case AVL_ADD_FROM_SRCH:
    if ((int) client_data == AVL_ADD_FROM_SRCH) XtUnmanageChild (mess_box);
    else sshot_comp[0] = 0;
    if (logged_in) new_avlcmp ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, new_avlcmp, NULL);
      }
    break;
  case 1:
    if (avllist_pos == 0)
      {
      no_avlcomps ("delete");
      return;
      }

    delete_set = TRUE;
    avlcmp=Avl_Record_Get(avlinx[avllist_pos - 1]);
    lib = Avl_Comp_Library_Get (avlcmp);
    cmp = Avl_Comp_CanSling_Get(avlcmp);
    nam = Avl_Comp_Name_Get(avlcmp);
    cat = Avl_Comp_CatNumber_Get(avlcmp);
    sprintf (msg_str, "Are you ABSOLUTELY SURE you want to DELETE (PERMANENTLY) compound index %d (Lib %d: %s; %s: %s)?",
      avlinx[avllist_pos -1], lib, nam, cat, Sling_Name_Get (cmp));
    msg = XmStringCreateLocalized (msg_str);
    XtVaSetValues (conf_box,
      XmNmessageString, msg,
      NULL);
    XmStringFree (msg);
    XtManageChild (conf_box);
    break;
  case 2:
  case 8:
    if (avllist_pos == 0)
      {
      no_avlcomps ("search");
      return;
      }

    srch_fwd = TRUE;
    srch_pos = 0;
#ifdef _DEBUG_
printf ("search: case %d; was_by_name=%d; srch_struct_exists=%d\n", client_data, was_by_name, srch_struct_exists);
#endif
    if ((int)client_data == 8)
    {
      if (was_by_name)
      {
        if (nmstr != NULL)
        {
          XtFree (nmstr);
          nmstr = NULL;
        }
        nmstr=XmTextGetString (srch_txt);
        if (slstr == NULL)
        {
          slstr = XtMalloc (strlen (unspec[3]) + 1);
          strcpy (slstr, unspec[3]);
          srch_struct_exists = FALSE;
        }
        XmTextSetString (srch_txt, slstr);
        was_by_name = FALSE;
      }
      XtVaSetValues (srch_txt,
        XmNeditable, False,
        NULL);
      XtManageChild (srch_draw_pb[0]);
      if (!srch_struct_exists) XtUnmanageChild (srch_draw_pb[1]);
      else XtManageChild (srch_draw_pb[1]);
    }
    else if (!was_by_name)
    {
      if (slstr != NULL)
      {
        if (slstr != glob_avl_comp) XtFree (slstr);
        slstr = NULL;
      }
      slstr=XmTextGetString (srch_txt);
      if (nmstr == NULL)
      {
        nmstr = XtMalloc (strlen (unspec[3]) + 1);
        strcpy (nmstr, unspec[3]);
      }
      XmTextSetString (srch_txt, nmstr);
      was_by_name = TRUE;
    }
    if (was_by_name) XtUnmanageChild (drawwin[2]);
    if (was_by_name || srch_struct_exists) XtManageChild(srch_popup);
    else AvlDraw_CB (srch_draw_pb[0], (XtPointer) srch_txt, NULL);
    break;
  case 3:
    if (avllist_pos == 0)
      {
      no_avlcomps ("search");
      return;
      }

    srch_fwd = TRUE;
    srch_pos = avllist_pos;
    if (!was_by_name)
    {
      if (slstr != NULL) XtFree (slstr);
      slstr=XmTextGetString (srch_txt);
      if (nmstr == NULL)
      {
        nmstr = XtMalloc (strlen (unspec[3]) + 1);
        strcpy (nmstr, unspec[3]);
      }
      XmTextSetString (srch_txt, nmstr);
      was_by_name = TRUE;
    }
    XtUnmanageChild (drawwin[2]);
    XtManageChild(srch_popup);
    break;
  case 5:
    if (avllist_pos == 0)
      {
      no_avlcomps ("search");
      return;
      }

    srch_fwd = FALSE;
    srch_pos = avllist_bottom;
    if (!was_by_name)
    {
      if (slstr != NULL) XtFree (slstr);
      slstr=XmTextGetString (srch_txt);
      if (nmstr == NULL)
      {
        nmstr = XtMalloc (strlen (unspec[3]) + 1);
        strcpy (nmstr, unspec[3]);
      }
      XmTextSetString (srch_txt, nmstr);
      was_by_name = TRUE;
    }
    XtUnmanageChild (drawwin[2]);
    XtManageChild(srch_popup);
    break;
  case 6:
    if (avllist_pos == 0)
      {
      no_avlcomps ("search");
      return;
      }

    srch_fwd = FALSE;
    srch_pos = avllist_pos;
    if (!was_by_name)
    {
      if (slstr != NULL) XtFree (slstr);
      slstr=XmTextGetString (srch_txt);
      if (nmstr == NULL)
      {
        nmstr = XtMalloc (strlen (unspec[3]) + 1);
        strcpy (nmstr, unspec[3]);
      }
      XmTextSetString (srch_txt, nmstr);
      was_by_name = TRUE;
    }
    XtUnmanageChild (drawwin[2]);
    XtManageChild(srch_popup);
    break;
  case 4:
  case 7:
    if (avllist_pos == 0)
      {
      no_avlcomps ("search");
      return;
      }

    if (srch_str[0]=='\0' || !was_by_name)
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
    pos=AvSearch_Pos(srch_str,avllist_pos, (int)client_data == 4, FALSE);
    if (pos)
    {
      XmListSetPos(avllistw,pos);
      XmListSelectPos(avllistw,pos,True);
    }
    else
    {
      label = XmStringCreateLocalized ("Search Failed");
      sprintf (msg_str, "Search string \"%s\" not found in %s direction", srch_str, (int)client_data == 5 ? "forward" : "reverse");
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
    pd_parent = XtParent (otherpdm);
    pd_grandparent = XtParent (pd_parent);
    if (pd_grandparent != othermb)
      XUnmapWindow (XtDisplay(pd_parent),XtWindow(pd_parent));
    break;
  default:
    break;
  }
}

void AvlCmp_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  Avl_Record_t *avlcmp;
  int lib, chap, sch, year, hr, min, sec;
  Sling_t cmp;
  char *nam, *cat, string[25], *sbuf;
  Tsd_t *ttsd, *subgoal;
  Xtr_t *txtr;
  Dsp_Molecule_t *dsp;
  U32_t
        i, j, k, w, h, twh, x, y, txy,
        rtype,
        credate, moddate, cretime, modtime,
        num_comm, num_ref,
        roots[MX_ROOTS], synthemes[MX_ROOTS];
  Boolean_t rotate, f1, f2, f3, dummy;

  XmListCallbackStruct *cbs = (XmListCallbackStruct *) call_data;

if (widg == avllistw)
{
  i = 0;
  if (cbs->reason != XmCR_SINGLE_SELECT) return;

  avllist_pos = cbs->item_position;
  avlcmp=Avl_Record_Get(avlinx[avllist_pos - 1]);

  lib = Avl_Comp_Library_Get (avlcmp);
  cmp = Avl_Comp_CanSling_Get (avlcmp);
  nam = Avl_Comp_Name_Get (avlcmp);
  cat = Avl_Comp_CatNumber_Get (avlcmp);

  if (lib == 0) sprintf (avlbuf, "<<< EMPTY RECORD - %d TEXT BYTES RESERVED >>>\n", avlfree[avlinx[avllist_pos - 1]]);
  else sprintf (avlbuf, "\n Library %2d: %s\n %s\n Catalog Number: %s\n\n %s\n",
    lib, alibname[lib - 1], nam, cat, Sling_Name_Get (cmp));

  XmTextSetString (textw, avlbuf);

  if (lib == 0)
  {
    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_WHITE));

    XClearWindow (disp, XtWindow (draww[0]));
    XFillRectangle (disp, drawp[0], gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_BLACK));

    XFlush (disp);
    return;
  }
}
else if (widg == info_txt[2]) i = 1;
else if (widg == srch_txt) i = 2;
else return;

if (i != 0)
{
  Sling_Name_Put (cmp, client_data);
  Sling_Length_Put (cmp, strlen ((char *) client_data));
}

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
    SAR_CLRI_WHITE));

  XClearWindow (disp, XtWindow (draww[i]));
  XFillRectangle (disp, drawp[i], gc, 0, 0, 2 * PDRW_DA_PXMP_WD / (i + 2), 2 *PDRW_DA_PXMP_HT / (i + 2));

  XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
  SAR_CLRI_BLACK));

  txtr=Sling2Xtr(cmp);
  ttsd=Xtr2Tsd(txtr);
  dsp=Xtr2Dsp(txtr);

  if (dsp_Shelley(dsp))
    {
/*
    Dsp_Compress (dsp);
    avfixdsp (dsp);
*/
    h=dsp->molh;
    w=dsp->molw;
    if (rotate=w>h)
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
    cdraw(dsp,ttsd,disp,XtWindow(draww[i]),gc,25,50,275,200,TRUE);
    cdraw(dsp,ttsd,disp,drawp[i],gc,25,50,275,200,FALSE);
    free_Molecule (dsp);
    }
  Xtr_Destroy (txtr);
  Tsd_Destroy (ttsd);

  XFlush (disp);
}

void update_compounds (Boolean_t update_list)
{
  static int prev_lib;
  int inx, i, j, l, c, tlib, tcmp;
  Avl_Record_t *avlcmp;
  char itemstr[500],name[200];
  XmString item;
  Boolean_t upd_count, is_temp;

  upd_count=alib_num==0 && cmp_count[0]==0 && cmp_count[1]==0 && cmp_count[2] == 0;
  if (upd_count) for (i=prev_lib=0; i<3; i++) cmp_count[i]=0;

  if (update_list)
    {
    XmListDeleteAllItems(avllistw);
    XmListSetAddMode(avllistw,TRUE);
    for (i = 0; i < 3; i++) last_cmpnum[i] = 0;
    }
  avllist_bottom = 0;

  for (inx=j=0; inx<slg_count; inx++) /* if (Persist_Legacy_Rxn (inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp)) */
    {
/*
    i = Persist_Current_Rec (PER_STD, inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp);
*/
    i = inx;
    avlcmp=Avl_Record_Get(i);

    if (update_list)
      {
      /* get last compound number in each library */
      tlib = Avl_Comp_Library_Get (avlcmp);
      if (last_cmpnum[tlib - 1] < tcmp) last_cmpnum[tlib - 1] = tcmp;
      }

    if ((alib_num == 0 || Avl_Comp_Library_Get (avlcmp) == alib_num))
      {
      if (alib_num == 0)
          sprintf(itemstr, "%d [%06d]/%s: %s",
                Avl_Comp_Library_Get (avlcmp), i+1,
                Avl_Comp_CatNumber_Get (avlcmp),
                Avl_Comp_Name_Get(avlcmp));
      else
            sprintf(itemstr, "[%06d]/%s: %s (%06d)", i+1,
                Avl_Comp_CatNumber_Get (avlcmp),
                Avl_Comp_Name_Get(avlcmp));

      if (update_list)
        {
        item=XmStringCreate(itemstr,XmSTRING_DEFAULT_CHARSET);
        XmListAddItem(avllistw,item,j+1);
        avlinx[j++] = i;
        XmStringFree(item);
        }
      }
    }

  if (upd_count || alib_num != prev_lib)
      prev_lib=alib_num;

  if (!update_list) return;

  XmListSetAddMode(avllistw,FALSE);
  avllist_bottom = j;

  if (avllist_bottom == 0)
    {
    avllist_pos = 0;

    avlbuf[0] = 0;
    XmTextSetString (textw, avlbuf);

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_WHITE));

      XFillRectangle (disp, drawp[0], gc, 0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT);
      XCopyArea (disp,
        drawp[0], XtWindow (draww[0]), gc,
        0, 0, PDRW_DA_PXMP_WD, PDRW_DA_PXMP_HT, 0, 0);

    XSetForeground (disp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
      SAR_CLRI_BLACK));

    return;
    }

  XmListSetBottomPos(avllistw, avllist_bottom);
  XmListSetPos(avllistw,1);
  XmListSelectPos(avllistw,1,True);
}

void cdraw(Dsp_Molecule_t *mol_p,Tsd_t *patt,Display *dsp,/* Drawable */ Window dawin,GC gc,
        int X1,int Y1,int X2,int Y2, Boolean_t iswin)
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
                s=0;
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
                          XSetForeground (dsp, gc, SynAppR_IthClrPx_Get (&GSynAppR,
                            SAR_CLRI_BLACK));
                          }
                        if (s) XDrawRectangle(dsp,dawin,gc,ax+X1,ay+Y1,aw,ah);

                        XDrawString(dsp,dawin,gc,ax+aw/4+X1,ay+3*ah/4+Y1,
                                atomsym,strlen(atomsym));
                }
                ++atom_p;
        }
}

/*
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
*/

void avfixdsp (Dsp_Molecule_t *dsp)
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

void redraw_compound
  (
  Widget      drawing_a,
  XtPointer   client_data,
  XtPointer   call_data
  )

{
  int which;
  XmDrawingAreaCallbackStruct *cbs =
    (XmDrawingAreaCallbackStruct *) call_data;

  which = (int) client_data;
  XCopyArea (cbs->event->xexpose.display,
    drawp[which], XtWindow (draww[which]), gc,
    0, 0, 2 * PDRW_DA_PXMP_WD / (which + 2), 2 * PDRW_DA_PXMP_HT / (which + 2), 0, 0);
}

int AvSearch_Pos (char *string, int prev_pos, Boolean_t forward, Boolean_t sling_search)
{
  int inx,i,j,c,pos,reclib;
  Avl_Record_t *avlcmp;
  char str[500],txt[500],*bar;
  Boolean_t is_temp;
  Sling_t sling_st;
  Xtr_t *xtr;

  if (sling_search)
    {
    Sling_Name_Put (sling_st, string);
    Sling_Length_Put (sling_st, strlen(string));
    xtr = Sling2Xtr_PlusHydrogen (sling_st);
/*
    Sling_Destroy (sling_st);
*/
    Xtr_Attr_ResonantBonds_Set (xtr);
    Xtr_Name_Set (xtr, NULL);
    sling_st = Name_Sling_Get (xtr, TRUE);
    Xtr_Destroy(xtr);
    strcpy (str, Sling_Name_Get (sling_st));
    Sling_Destroy (sling_st);
    bar=strstr(str,"|");
    if (bar!=NULL) *bar=0;
    }
  else for (i=0; i<=strlen(string); i++) str[i]=tolower(string[i]);

  for (inx=pos=0; inx<slg_count && (forward || pos<prev_pos); inx++) /* if (Persist_Legacy_Rxn (inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp)) */
    {
/*
    i = Persist_Current_Rec (PER_STD, inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp);
*/
    i = inx;
    avlcmp=Avl_Record_Get(i);
reclib = Avl_Comp_Library_Get (avlcmp);
    if ((alib_num == 0 || reclib == alib_num) && ++pos>prev_pos)
      {
      if (reclib == 0);
      else if (sling_search)
        {
        if (strcasecmp (str, Sling_Name_Get(Avl_Comp_CanSling_Get(avlcmp))) == 0) return (pos);
        }
      else
        {
        strcpy(txt,Avl_Comp_Name_Get(avlcmp));
        for (j=0; j<strlen(txt); j++) txt[j]=tolower(txt[j]);
        if (strstr(txt,str)) return(pos);
        }
      }
    }

  if (forward) return(0);

  for (inx-=2; inx>=0; inx--) /* if (Persist_Legacy_Rxn (inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp)) */
    {
/*
    i = Persist_Current_Rec (PER_STD, inx, PER_TIME_ANY, PER_TIME_ANY, &is_temp);
*/
    i = inx;
    avlcmp=Avl_Record_Get(i);
reclib = Avl_Comp_Library_Get (avlcmp);
if (reclib == 0) continue;
    if ((alib_num == 0 || reclib == alib_num))
      {
      pos--;
      if (reclib != 0)
        {
        strcpy(txt,Avl_Comp_Name_Get(avlcmp));
        for (j=0; j<strlen(txt); j++) txt[j]=tolower(txt[j]);
        if (strstr(txt,str)) return(pos);
        }
      }
    }

  return(0);
}

void AvlSearch_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  int pos;
  char *lcl_srch_str, msg_str[600];
  XmString msg, label;
  Boolean_t sling_search;

  lcl_srch_str = XmTextGetString(XtNameToWidget(srch_popup,"SearchTxt"));
  strcpy(srch_str,lcl_srch_str);
  XtFree(lcl_srch_str);
  if (srch_str[0] == 0)
  {
    InfoWarn_Show ("AvlEdit Notice~No search string was supplied.");
    return;
  }
  if (XtIsManaged (srch_draw_pb[0]))
  {
    XtUnmanageChild (srch_draw_pb[0]);
    XtUnmanageChild (srch_draw_pb[1]);
    sling_search = TRUE;
  }
  else sling_search = FALSE;
  XtUnmanageChild(srch_popup);
  XtVaSetValues (srch_txt,
    XmNeditable, True,
    NULL);
  pos=AvSearch_Pos(srch_str, srch_pos, srch_fwd, sling_search);
  if (pos)
    {
    XmListSetPos(avllistw,pos);
    XmListSelectPos(avllistw,pos,True);
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
    if (sling_search)
    {
      strcpy (sshot_comp, srch_str);
      XtManageChild (XmMessageBoxGetChild (mess_box, XmDIALOG_HELP_BUTTON));
    }
    else XtUnmanageChild (XmMessageBoxGetChild (mess_box, XmDIALOG_HELP_BUTTON));
    XtManageChild (mess_box);
    }
}

void AvlMBDismiss_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (mess_box);
  if (!add_set)
    {
    if (disable_set || delete_set)
      {
      disable_set = delete_set = FALSE;
      update_compounds (TRUE);
      }
    return;
    }

  add_set = FALSE;
  if (logged_in) new_avlcmp ();
  else
    {
    if (login_failed) LoginFail (tl);
    else Login (tl, new_avlcmp, NULL);
    }
}

void AvlConfirm_CB (Widget widg, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (conf_box);
  if (exit_set)
    {
    exit_set = FALSE;
    if ((Boolean_t) (int) client_data)
      {
/*
      Persist_Close ();
*/
      Isam_Close (&Avd_DictCtrl_InfoFC_Get (SDictControl));
      if (db_modified)
        {
#ifdef _DEBUG_
debug_print(AVL_DICT_COMMAND);
#endif
        system (AVL_DICT_COMMAND);
        }
      exit (0);
      }
    }
  else if (delete_set)
    {
/* defer until AvlMBDismiss_CB unless canceled
    delete_set = FALSE;
*/
    if (!(Boolean_t) (int) client_data)
      {
      delete_set = FALSE;
      return;
      }
/* begin disabling of schema */
    if (logged_in) delete_avlcmp ();
    else
      {
      if (login_failed) LoginFail (tl);
      else Login (tl, delete_avlcmp, NULL);
      }
    }
  else printf ("Unexpected call to AvConfirm_CB\n");
}

void new_avlcmp()
{
  char *nam, *cat, *slg;

  if (clearance_level < ADD_AVL_LEV)
    {
    LoginFail (tl);
    return;
    }

/*
  save_NCmp = NCmp;
*/

  editing_record = -1;
  cat=nam=unspec[3];
  if (sshot_comp[0] == 0) slg=unspec[2];
  else slg=sshot_comp;
  XmTextSetString (info_txt[0], nam);
  XmTextSetString (info_txt[1], cat);
  XmTextSetString (info_txt[2], slg);
  if (sshot_comp[0] == 0)
  {
    XtUnmanageChild (drawwin[1]);
    XtUnmanageChild (modpb[1]);
  }
  else
  {
    XtManageChild (drawwin[1]);
    XtManageChild (modpb[1]);
  }
  XtManageChild (info_popup);
  if (sshot_comp[0] != 0) AvlCmp_CB (info_txt[2], slg, (XtPointer) NULL);
}

void delete_avlcmp()
{
  int c, pos, lib;
  char string[500],*s, *lcl_srch_str, msg_str[600], *nam, *cat;
  XmString msg, label;
  Avl_Record_t *avlcmp;
  Sling_t cmp;

  if (clearance_level < DEL_AVL_LEV)
    {
    LoginFail (tl);
    return;
    }

  avlcmp=Avl_Record_Get(avlinx[avllist_pos - 1]);
  lib = Avl_Comp_Library_Get (avlcmp);
  cmp = Avl_Comp_CanSling_Get (avlcmp);
  nam = Avl_Comp_Name_Get (avlcmp);
  cat = Avl_Comp_CatNumber_Get (avlcmp);

  /* create msg_str BEFORE action, because update_schemata resets schpos and updates schinx array */
  sprintf (msg_str, "Compound index %d (%s: %s [%s]) has been deleted from library %d.",
    avlinx[avllist_pos], cat, nam, Sling_Name_Get (cmp), lib);

  backup_avllib ();

  Avl_Delete (avlinx[avllist_pos - 1], TRUE);

  label = XmStringCreateLocalized ("Compound Deleted");
  msg = XmStringCreateLocalized (msg_str);
  XtVaSetValues (mess_box,
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree (label);
  XmStringFree (msg);
  XtManageChild (mess_box);
}

void Avl_Delete (int which, Boolean_t rewrite)
{
  Avi_CmpInfo_Avail_Put (avinfo_p[which], AVI_AVAIL_JUNK_CD);
  avlfree[which] = Avl_Free_VarLen (which);
  if (rewrite) Avl_Write_Record (which);
}

void no_avlcomps (char *func)
{
  char msgstr[100];
  XmString label, msg;

  label = XmStringCreateLocalized ("Error: No Compounds Listed");
  sprintf (msgstr, "There are no compounds listed - can't %s!", func);
  msg = XmStringCreateLocalized (msgstr);
  XtVaSetValues (mess_box,
    XmNdialogTitle, label,
    XmNmessageString, msg,
    NULL);
  XmStringFree (label);
  XmStringFree (msg);
  XtManageChild (mess_box);
}

U16_t Avl_Free_VarLen (int which)
{
  Avi_CmpInfo_t *ap;

  ap=avinfo_p[which];
  if (Avi_CmpInfo_Avail_Get (ap) != AVI_AVAIL_JUNK_CD) return (0);
  return (Avi_CmpInfo_CatLen_Get (ap) + Avi_CmpInfo_NameLen_Get (ap) + Avi_CmpInfo_SlgLen_Get (ap));
}

Avl_Record_t *Avl_Record_Get (int which)
{
  Avi_CmpInfo_t *ap;
  U8_t avail_flag;
  static Avl_Record_t art;
  static char cat[64], nam[256], slg[512];

  ap=avinfo_p[which];
  avail_flag=Avi_CmpInfo_Avail_Get (ap);
  art.catalogn=cat;
  art.name=nam;

  if (avail_flag == AVI_AVAIL_JUNK_CD)
  {
    art.library = 0;
    strcpy (cat, "<<< EMPTY >>>");
    strcpy (nam, "<<< RESERVED RECORD - ENTRY DELETED OR MOVED >>>");
    Sling_Name_Put (art.sling, unspec[3]);
    Sling_Length_Put (art.sling, 0);
  }
  else
  {
    art.library=Avi_CmpInfo_Lib_Get (ap);
    if (Avi_CmpInfo_CatLen_Get (ap) == 0) strcpy (cat, unspec[0]);
    else
    {
      strncpy (art.catalogn, Avi_CmpInfo_Catalog_Get (ap), Avi_CmpInfo_CatLen_Get (ap));
      art.catalogn[Avi_CmpInfo_CatLen_Get (ap)]=0;
    }
    if (Avi_CmpInfo_NameLen_Get (ap) == 0) strcpy (nam, unspec[1]);
    else
    {
      strncpy (art.name, Avi_CmpInfo_Name_Get (ap), Avi_CmpInfo_NameLen_Get (ap));
      art.name[Avi_CmpInfo_NameLen_Get (ap)]=0;
    }
    strncpy (slg, Avi_CmpInfo_Sling_Get (ap), Avi_CmpInfo_SlgLen_Get (ap));
    slg[Avi_CmpInfo_SlgLen_Get (ap)]=0;
    Sling_Name_Put (art.sling, slg);
    Sling_Length_Put (art.sling, strlen(slg));
  }

  return(&art);
}

void backup_avllib ()
{
  static Boolean_t first_time = TRUE;
  char dummy[3],files[2][128];
  FILE *in, *out;
  int c;

  if (!first_time) return;
  first_time = FALSE;
  db_modified = TRUE;
/*
  system (AVL_BKUP_COMMAND);
*/
  if (sscanf(AVL_BKUP_COMMAND, "%s %s %s", dummy, files[0], files[1]) != 3 || strcmp (dummy, "cp"))
  {
#ifdef _DEBUG_
debug_print(AVL_BKUP_COMMAND);
#endif
    system (AVL_BKUP_COMMAND);
    return;
  }
  in = fopen (files[0], "r");
  out = fopen (files[1], "w");
  while ((c=getc(in))!=EOF) putc(c,out);
  fclose(out);
  fclose(in);
}

void Avl_Record_Write (int i)
{
}

void AvlSkipInfo_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  XtUnmanageChild (info_popup);
}

void AvlDraw_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  static SubmitCB_t scbt;
  static Boolean_t first_time = TRUE;

  glob_txt_widget = (Widget) client_data;
  if (w != modpb[1] && w != modpb[2]) sshot_comp[0] = 0;
  if (first_time)
    {
    first_time = FALSE;
    Submit_Created_Put (&scbt, FALSE);
    }

  ValCompEntry_Create (&scbt, tl, &schlC, -1);

  XtManageChild (Submit_FormDlg_Get (&scbt));
}

void AvlStore_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *cat, *nam, *slg, sling_text[512], *bar;
  int newrec, textlen, padding, namlen, catlen, slglen, i, found_pos;
  Sling_t sling_st;
  Xtr_t *xtr;
  Avi_CmpInfo_t *ap;
  Boolean_t add_rec;

  nam = XmTextGetString (info_txt[0]);
  cat = XmTextGetString (info_txt[1]);
  slg = XmTextGetString (info_txt[2]);

  XtUnmanageChild (info_popup);

  if (strcmp (slg, unspec[2]) == 0)
  {
    XtFree (nam);
    XtFree (cat);
    XtFree (slg);
    InfoWarn_Show ("AvlEdit Notice~No structure was entered.");
    return;
  }

  namlen = strlen (nam);
  catlen = strlen (cat);

    Sling_Name_Put (sling_st, slg);
    Sling_Length_Put (sling_st, strlen(slg));
    xtr = Sling2Xtr_PlusHydrogen (sling_st);

  XtFree (slg);

    Xtr_Attr_ResonantBonds_Set (xtr);
    Xtr_Name_Set (xtr, NULL);
    sling_st = Name_Sling_Get (xtr, TRUE);
    Xtr_Destroy(xtr);
    strcpy (sling_text, Sling_Name_Get (sling_st));
    Sling_Destroy (sling_st);
    bar=strstr(sling_text,"|");
    if (bar!=NULL) *bar=0;

  slglen = strlen (sling_text);
  if (slglen == 0)
  {
    InfoWarn_Show ("AvlEdit Notice~Invalid structure given - data not stored.");
    return;
  }
  textlen = namlen + catlen + slglen;

  if (editing_record >= 0) Avl_Delete (editing_record, FALSE);
  newrec = Avl_Best_FreeRec (textlen);
  if (newrec < 0)
  {
    add_rec = TRUE;
    newrec = slg_count++;
    avlfree[newrec] = textlen;
    avinfo_p[newrec] = (Avi_CmpInfo_t *) malloc (sizeof (Avi_CmpInfo_t));
    Avi_CmpInfo_Avail_Put (avinfo_p[newrec], AVI_AVAIL_JUNK_CD);
    Avi_CmpInfo_Lib_Put (avinfo_p[newrec], 0);
  }
  else add_rec = FALSE;
  padding = avlfree[newrec] - textlen;
  for (i = 0; i < padding; i++) sling_text[++slglen] = 0;

  slg = (char *) malloc (slglen + 2);
  strncpy (slg, sling_text, slglen + 1);
  /* fix for incompatibility of legacy data with new atomsym */
  for (i = 0; i < slglen; i++) slg[i] = toupper (slg[i]);

  ap=avinfo_p[newrec];
  Avi_CmpInfo_Catalog_Put (ap, cat);
  Avi_CmpInfo_CatLen_Put (ap, catlen);
  Avi_CmpInfo_Name_Put (ap, nam);
  Avi_CmpInfo_NameLen_Put (ap, namlen);
  Avi_CmpInfo_Sling_Put (ap, slg);
  Avi_CmpInfo_SlgLen_Put (ap, slglen);

  if (editing_record < 0 && (found_pos = AvSearch_Pos (slg, 0, TRUE, TRUE)) > 0)
  {
    XmListSetPos(avllistw, found_pos);
    XmListSelectPos(avllistw, found_pos, True);
    InfoWarn_Show ("AvlEdit Notice~Compound record already exists.");
    if (add_rec) free (avinfo_p[--slg_count]);
    free (slg);
    XtFree (nam);
    XtFree (cat);
    return;
  }

  Avi_CmpInfo_Avail_Put (ap, AVI_AVAIL_DFLT_CD);
  Avi_CmpInfo_Lib_Put (ap, 1);

  backup_avllib ();

  Avl_Write_Record (newrec);
  avlfree[newrec] = 0;
  if (editing_record >= 0 && newrec != editing_record) Avl_Write_Record (editing_record);

  update_compounds (TRUE);
  XmListSetPos(avllistw, newrec + 1);
  XmListSelectPos(avllistw, newrec + 1, True);
}

void Avl_Drawn_Sling (char *sling)
{
  int which_child;

  if (glob_txt_widget == info_txt[2]) which_child = 1;
  else if (glob_txt_widget == srch_txt) which_child = 2;
  else
  {
    InfoWarn_Show ("AvlEdit Error~Avl_Drawn_Sling called with unexpected widget.");
    return;
  }

  XmTextSetString (glob_txt_widget, sling);
  if (sling[0])
  {
    strcpy (sshot_comp, sling);
    if (which_child == 2) srch_struct_exists = TRUE;
    if (!XtIsManaged (drawwin[which_child]) || !XtIsManaged (modpb[which_child]))
    {
      XtUnmanageChild (XtParent (drawwin[which_child]));
      XtManageChild (drawwin[which_child]);
      XtManageChild (modpb[which_child]);
      XtManageChild (XtParent (drawwin[which_child]));
    }
    AvlCmp_CB (glob_txt_widget, (XtPointer) sling, NULL);
  }
  else if (XtIsManaged (drawwin[which_child]) || XtIsManaged (modpb[which_child]))
  {
    XtUnmanageChild (XtParent (drawwin[which_child]));
    XtUnmanageChild (drawwin[which_child]);
    XtUnmanageChild (modpb[which_child]);
    XtManageChild (XtParent (drawwin[which_child]));
  }
}

void Avl_Init_Free_VarLen ()
{
  int i;

  for (i = 0; i < slg_count; i++) avlfree[i] = Avl_Free_VarLen (i);
}

int Avl_Best_FreeRec (int varlen)
{
  int bestrec, i;

  for (i = 0, bestrec = -1; i < slg_count - 1; i++) if (avlfree[i] >= varlen)
  {
    if (bestrec < 0) bestrec = i;
    else if (avlfree[i] < avlfree[bestrec]) bestrec = i;
  }

  if (avlfree[i] > 0 && (bestrec < 0 || avlfree[bestrec] > varlen)) avlfree[bestrec = i] = varlen;

  return (bestrec);
}

void Avl_Write_Record (int which)
{
  U16_t         rec_len;
  U8_t          buff[AVI_INFOREC_LENMAX];   /* comp info record buffer */

      rec_len = AviCmpInfo_Extrude (avinfo_p[which], buff);
      Isam_Write (&Avd_DictCtrl_InfoFC_Get (SDictControl), which + 1, buff, rec_len);
      Isam_Flush (&Avd_DictCtrl_InfoFC_Get (SDictControl));

      Avc_RecZero_CntSz_Put (Avd_DictCtrl_InfoZR_Get (SDictControl), slg_count);
      Isam_Write (&Avd_DictCtrl_InfoFC_Get (SDictControl), 0, &Avd_DictCtrl_InfoZR_Get (SDictControl), AVC_RECZERO_SZ);
      Isam_Flush (&Avd_DictCtrl_InfoFC_Get (SDictControl));
}

Boolean_t Avl_Record_Empty (int which)
{
  return (Avi_CmpInfo_Avail_Get (avinfo_p[which]) == AVI_AVAIL_JUNK_CD);
}
