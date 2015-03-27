/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:              PTDIST.C
*
*    This module provides for the display and editing of the posttransform
*    conditions for determining whether a given attribute with a specific
*    attachment exists at a given position.
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

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>
#include <Xm/RowColumn.h>
#include <Xm/Text.h>
#include <Xm/TextF.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifdef _H_UTL_
#include "utl.h"
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

#ifndef _H_POSTTEST_
#include "posttest.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_SSHOT_VIEW_
#include "sshot_view.h"
#endif

#ifndef NATTRIB
#define NATTRIB 65
#endif

extern char *atable[][2];
extern int cond_type;
extern Widget pttl, ptformdg, condform, condpb[2], emptytextmsg, nodexceedmsg, condlbl;
extern Boolean_t cond_add_canceled;

static Widget distform, distlbl[4], disttxt[4], distpb, distmenu, distdummy;
static Condition_t *glob_cond;
static int attr_num;
static char glob_condstr[1024];
static Boolean_t *glob_nu, new_cond, dist_disabled = FALSE;

void Post_Cond_Put (Condition_t *);

void fix_sling_case (char *);
void Dist_Display (char *, int, int, int, char *);
void DistOpt_CB (Widget, XtPointer, XtPointer);
void CondExit_CB (Widget, XtPointer, XtPointer);
void DistExit_CB (Widget, XtPointer, XtPointer);
void DistNode_CB (Widget, XtPointer, XtPointer);
void DistPB_CB (Widget, XtPointer, XtPointer);
void PTSetCondname (int, char *);
void getat (int);

static char *attr[NATTRIB + 1] =
  {"SUBSTITUENT WITH SLING ... ",
   "CARBONYL",
   "ENOLIZABLE CARBONYL",
   "ACIDIC NITRITE",
   "AROMATIC RING",
   "ALKENE",
   "ALKYNE",
   "NITRILE",
   "NITRO",
   "SULFONE",
   "HYDROGEN",
   "ESTER (ACYL)",
   "TERTIARY AMIDE",
   "ALPHA,BETA UNSATURATED CARBONYL",
   "ESTER (ALKYL)",
   "O ESTER",
   "FLUORINE",
   "CHLORINE",
   "BROMINE",
   "IODINE",
   "SULFUR",
   "HYDROXYL",
   "ETHER",
   "PRIMARY AMINE",
   "SECONDARY AMINE",
   "TERTIARY AMINE",
   "CARBOXYLIC ACID",
   "METHYL",
   "METHYLENE",
   "METHINYL",
   "QUATERNARY AMINE",
   "SULFONIUM",
   "SULFONATE ESTER",
   "PHENOXY",
   "ALKOXY",
   "NITRATE",
   "PHOSPHATE",
   "BORATE",
   "VINYL WITH GAMMA HYDROGEN",
   "SULFOXIDE",
   "PHOSPHOROUS OXIDES",
   "THIOPHENOL",
   "MERCAPTAN",
   "SULFIDE",
   "PHOSPHINE",
   "PHOSPHONATE ESTER AT P",
   "TRIALKYLSILYL",
   "ALPHA-PYRIDYL",
   "BETA-PYRIDYL",
   "GAMMA-PYRIDYL",
   "ALPHA-FURYL",
   "ALPHA-THIENYL",
   "ALPHA-PYRRYL",
   "BETA-FURYL, ALPHA,ALPHA-DISUBSTITUTED",
   "BETA-THIENYL, ALPHA,ALPHA-DISUBSTITUTED",
   "BETA-PYRRYL, ALPHA,ALPHA-DISUBSTITUTED",
   "PARA-HYDROXYPHENYL",
   "PARA-AMINOPHENYL",
   "ORTHO-HYDROXYPHENYL",
   "ORTHO-AMINOPHENYL",
   "TRIHALOMETHYL",
   "QUATERNARY CARBON",
   "OXIRANE",
   "1-PYRRYL",
   "1-IMIDAZOLYL",
   "1-PYRAZOLYL"};


void PTDist (Condition_t *cond, Boolean_t *node_used, int condnum, Boolean_t new)
{
  char *condstr, tmpstr[128], *sling;
  String_t string;
  XmString label[NATTRIB + 1], title;
  int i, sel, node, dist, root;
/*
  XmFontList flhv18;
  XmFontListEntry helv18;
*/
  XmFontList flhv14;
  XmFontListEntry helv14;

  cond_type = PT_TYPE_DIST;

/*
  if (cond == NULL)
*/
  if (new)
    {
    new_cond = TRUE;
/*
    cond = (Condition_t *) malloc (sizeof (Condition_t));
*/
    condstr = (char *) malloc (100);
    strcpy (condstr, "1040100100");
    String_Alloc_Put (string, 100);
    String_Value_Put (string, condstr);
    }
  else
    {
    new_cond = FALSE;
    condition_import (cond, &string, NULL);
    condstr = String_Value_Get (string);
    }

  glob_cond = cond;
  strcpy (glob_condstr, condstr);
  glob_nu = node_used;

  String_Destroy (string);

/*
  helv18 = XmFontListEntryLoad (XtDisplay (pttl), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv18 = XmFontListAppendEntry (NULL, helv18);
*/
  helv14 = XmFontListEntryLoad (XtDisplay (pttl), "-*-helvetica-bold-r-normal-*-14-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);

  flhv14 = XmFontListAppendEntry (NULL, helv14);

  PTSetCondname (condnum, tmpstr);

  title = XmStringCreateLocalized (tmpstr);
  XtVaSetValues (condlbl,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);

  distform = XmCreateForm (condform, "distform", NULL, 0);
  XtVaSetValues (distform,
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
    XmNfractionBase, 200,
    NULL);

  title = XmStringCreateLocalized ("The directed attribute");
  distlbl[0] = XtVaCreateWidget ("distlbl0",
               xmLabelWidgetClass, distform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_FORM,
               NULL);
  XmStringFree (title);

  strncpy (tmpstr, glob_condstr + 3, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &node);
  strncpy (tmpstr, glob_condstr + 5, 3);
  tmpstr[3] = '\0';
  sscanf (tmpstr, "%d", &attr_num);
  strncpy (tmpstr, glob_condstr + 8, 2);
  tmpstr[2] = '\0';
  sscanf (tmpstr, "%d", &dist);
  if (strlen (glob_condstr) == 10)
    {
    root = 0;
    sling = glob_condstr + 10;
    }
  else
    {
    strncpy (tmpstr, glob_condstr + 10, 2);
    tmpstr[2] = '\0';
    sscanf (tmpstr, "%d", &root);
    sling = glob_condstr + 12;
    }

  dist_disabled = attr_num == 0;

  if (dist_disabled) title = XmStringCreateLocalized (attr[0]);
  else title = XmStringCreateLocalized (atable[attr_num-1][1]);
  distpb = XtVaCreateWidget ("distpb",
               xmPushButtonGadgetClass, distform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
/*
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_FORM,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_NONE,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, distlbl[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_FORM,
*/
               NULL);
  XmStringFree (title);

  XtAddCallback (distpb, XmNactivateCallback, DistPB_CB, (XtPointer) &attr_num);

  XtVaSetValues (distpb,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, distlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 125,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  disttxt[0] = XmCreateTextField (distform, "disttxt0", NULL, 0);

  XtVaSetValues (disttxt[0],
/*
    XmNfontList, flhv18,
*/
    XmNfontList, flhv14,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, distlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, distpb,
    XmNrightPosition, 150,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" AND ROOT ");
  distlbl[1] = XtVaCreateWidget ("distlbl1",
               xmLabelWidgetClass, distform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, distlbl[0],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, disttxt[0],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, disttxt[0],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  disttxt[1] = XmCreateTextField (distform, "disttxt1", NULL, 0);

  XtVaSetValues (disttxt[1],
/*
    XmNfontList, flhv18,
*/
    XmNfontList, flhv14,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, distlbl[0],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, distlbl[1],
    XmNrightPosition, 200,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized ("");
  label[0] = XmStringCreateLocalized ("IS");
  label[1] = XmStringCreateLocalized ("IS NOT");

/*
  distdummy = XmVaCreateSimplePulldownMenu (distform, "_distpulldown1", 0, NULL, XmNbuttonFontList, flhv18, NULL);
*/
  distdummy = XmVaCreateSimplePulldownMenu (distform, "_distpulldown1", 0, NULL, XmNbuttonFontList, flhv14, NULL);

  if (glob_condstr[0] == '0') sel = 1;
  else sel = 0;

  distmenu = XmVaCreateSimpleOptionMenu (distform, "distmenu1", title, '\0', sel, DistOpt_CB,
      XmNsubMenuId, distdummy,
/*
      XmNlabelFontList, flhv18,
      XmNbuttonFontList, flhv18,
*/
      XmNlabelFontList, flhv14,
      XmNbuttonFontList, flhv14,
      XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_WHITE),
      XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
      SAR_CLRI_BLACK),
      XmVaPUSHBUTTON, label[0], '\0', NULL, NULL,
      XmVaPUSHBUTTON, label[1], '\0', NULL, NULL,
      NULL);

  XmStringFree (title);
  XmStringFree (label[0]);
  XmStringFree (label[1]);

  XtVaSetValues (distmenu,
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, distpb,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" present at distance");
  distlbl[2] = XtVaCreateWidget ("distlbl2",
               xmLabelWidgetClass, distform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, distpb,
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, distmenu,
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, distmenu,
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  disttxt[2] = XmCreateTextField (distform, "disttxt2", NULL, 0);

  XtVaSetValues (disttxt[2],
/*
    XmNfontList, flhv18,
*/
    XmNfontList, flhv14,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, distmenu,
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftPosition, 12,
    XmNleftAttachment, XmATTACH_POSITION,
    XmNrightPosition, 50,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  title = XmStringCreateLocalized (" from NODE ");
  distlbl[3] = XtVaCreateWidget ("distlbl3",
               xmLabelWidgetClass, distform,
/*
               XmNfontList, flhv18,
*/
               XmNfontList, flhv14,
               XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_WHITE),
               XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
               SAR_CLRI_BLACK),
               XmNlabelString, title,
               XmNtopOffset, 0,
               XmNtopAttachment, XmATTACH_WIDGET,
               XmNtopWidget, distlbl[2],
               XmNbottomOffset, 0,
               XmNbottomAttachment, XmATTACH_OPPOSITE_WIDGET,
               XmNbottomWidget, disttxt[2],
               XmNleftOffset, 0,
               XmNleftAttachment, XmATTACH_WIDGET,
               XmNleftWidget, disttxt[2],
               XmNrightOffset, 0,
               XmNrightAttachment, XmATTACH_NONE,
               NULL);
  XmStringFree (title);

  disttxt[3] = XmCreateTextField (distform, "disttxt3", NULL, 0);

  XtVaSetValues (disttxt[3],
/*
    XmNfontList, flhv18,
*/
    XmNfontList, flhv14,
    XmNbackground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_WHITE),
    XmNforeground,SynAppR_IthClrPx_Get(&GSynAppR,
    SAR_CLRI_BLACK),
    XmNtopOffset, 0,
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, distlbl[2],
    XmNbottomOffset, 0,
    XmNbottomAttachment, XmATTACH_NONE,
    XmNleftOffset, 0,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, distlbl[3],
    XmNrightPosition, 150,
    XmNrightAttachment, XmATTACH_POSITION,
    NULL);

  for (i = 0; i < 4; i++)
    XtAddCallback (disttxt[i], XmNvalueChangedCallback, DistNode_CB, (XtPointer) i);

  Dist_Display (glob_condstr, root, dist, node, sling);
}

void Dist_Display (char *v, int root, int dist, int node, char *sling)
{
  int i, sel;
  char nodelist[256], tmpstr[5];

  XtManageChild (distlbl[0]);

  XtManageChild (distpb);

  XtVaSetValues (disttxt[0],
    XmNvalue, sling,
    NULL);

  XtManageChild (disttxt[0]);

  XtManageChild (distlbl[1]);

  if (root != 0)
    {
    sprintf (nodelist, "%02d", root);

    XtVaSetValues (disttxt[1],
      XmNvalue, nodelist,
      NULL);
    }

  XtManageChild (disttxt[1]);

  XtManageChild (distmenu);

  XtManageChild (distlbl[2]);

  sprintf (nodelist, "%02d", dist);

  XtVaSetValues (disttxt[2],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (disttxt[2]);

  XtManageChild (distlbl[3]);

  sprintf (nodelist, "%02d", node);

  XtVaSetValues (disttxt[3],
    XmNvalue, nodelist,
    NULL);

  XtManageChild (disttxt[3]);

  XtManageChild (distform);
  XtManageChild (ptformdg);

  if (root == 0)
    {
    XtUnmanageChild (disttxt[0]);
    XtUnmanageChild (distlbl[1]);
    XtUnmanageChild (disttxt[1]);
    }
}

void DistOpt_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *wname, tmpstr[128], *ts, *comma, *space;
  int sel, sel2, nnodes, nnodes2;

  sel = (int) client_data;

  glob_condstr[0] = sel == 0 ? '1' : '0';
}

void DistNode_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  char *lcl_node_str, node_str[128];
  int i, len;
  Boolean_t ok;
  static Boolean_t internal_change = FALSE;

  if (internal_change) return; /* ignore callback due to field truncation */

  lcl_node_str = XmTextGetString(w);

  len = strlen (lcl_node_str);
  if ((int) client_data == 0)
    {
    if (len == 8)
      {
      strcpy (node_str, lcl_node_str);
      node_str[7] = 0;
      XtFree(lcl_node_str);
      internal_change = TRUE;
      XmTextReplace (w,0,len,node_str);
      internal_change = FALSE;
      XBell (XtDisplay (w), 10);
      }
    else XtFree(lcl_node_str);
    return;
    }
  else if ((int) client_data == 2 && dist_disabled && strcmp (lcl_node_str, "00") != 0)
    {
    XtFree(lcl_node_str);
    internal_change = TRUE;
    if (len < 2) len = 2;
    XmTextReplace (w,0,len,"00");
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    return;
    }
  else
    for (i = 0, ok = TRUE; i < len && ok; i++)
    {
    ok = i < 2 && lcl_node_str[i] >= '0' && lcl_node_str[i] <= '9';
    if (!ok) i--; /* adjust prior to increment */
    }

  if (!ok)
    {
    strcpy (node_str, lcl_node_str);
    strcpy (node_str + i, node_str + i + 1);
    XtFree(lcl_node_str);
    internal_change = TRUE;
    XmTextReplace (w,0,len,node_str);
    internal_change = FALSE;
    XBell (XtDisplay (w), 10);
    }
  else XtFree(lcl_node_str);
}

void DistExit_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  String_t string;
  char sling[128], tmpstr[128], tmpstr2[128], savestr[128], *ts, *ts2, *comma, *space, *newstr;
  int i, node, root, dist;

  if ((Boolean_t) (int) client_data)
    {
/* first get final results of text fields */
    if (XtIsManaged (disttxt[0]) && XtIsManaged (disttxt[1]))
      {
      XtVaGetValues (disttxt[0],
        XmNvalue, &ts,
        NULL);

      if (ts[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      strcpy (sling, ts);

      fix_sling_case (sling);

      XtVaGetValues (disttxt[1],
        XmNvalue, &ts2,
        NULL);

      if (ts2[0] == '\0')
        {
        XtManageChild (emptytextmsg);
        return;
        }
      sscanf (ts2, "%d", &root);
      sprintf (tmpstr, "%02d%s", root, sling);
      }
    else tmpstr[0] = '\0';

/* incorporate into string */
    strcpy (glob_condstr + 10, tmpstr);

    XtVaGetValues (disttxt[2],
      XmNvalue, &ts,
      NULL);

    if (ts[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts, "%d", &dist);

    XtVaGetValues (disttxt[3],
      XmNvalue, &ts2,
      NULL);

    if (ts2[0] == '\0')
      {
      XtManageChild (emptytextmsg);
      return;
      }
    sscanf (ts2, "%d", &node);
    strncpy (tmpstr2, glob_condstr + 5, 3);
    tmpstr2[3] = '\0';
    sscanf (tmpstr2, "%d", &attr_num);

    sprintf (tmpstr, "%02d%03d%02d", node, attr_num, dist);

    strncpy (glob_condstr + 3, tmpstr, 7);

    newstr = (char *) malloc (strlen (glob_condstr) + 1);
    strcpy (newstr, glob_condstr);
    String_Value_Put (string, newstr);
    String_Length_Put (string, strlen (newstr));
    String_Alloc_Put (string, strlen (newstr) + 1);

    condition_export (glob_cond, &string, glob_nu);
    }
  else if (new_cond) cond_add_canceled = TRUE;

  XtUnmanageChild (ptformdg);
  XtDestroyWidget (distform);
}

void fix_sling_case (char *sling)
{
  int i, pos;

  for (i = 0; i < strlen (sling); i++)
    {
    if (sling[i] == '(' && i < strlen (sling) - 3 && sling[i + 3] == ')')
      {
      sling[i + 1] = toupper (sling [i + 1]);
      sling[i + 2] = tolower (sling [i + 2]);
      i += 3;
      }
    else
      sling[i] = toupper (sling [i]);
    }
}

void DistPB_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
  AT_Form_Create (XtParent (XtParent (XtParent (XtParent (w)))), *(int *) client_data, getat);
}

void getat (int atnum)
{
  XmString title;
  char tmpstr[5], *ts;

  attr_num = atnum;
  dist_disabled = attr_num == 0;

  if (dist_disabled)
    {
    title = XmStringCreateLocalized (attr[0]);
    XtManageChild (disttxt[0]);
    XtManageChild (distlbl[1]);
    XtManageChild (disttxt[1]);
    XtVaGetValues (disttxt[1],
      XmNvalue, &ts,
      NULL);
    if (ts[0] == '\0')
      strncpy (glob_condstr + 10, "00", 2);
    else
      {
      if (ts[1] == '\0')
        {
        ts[2] = ts[1];
        ts[1] = ts[0];
        ts[0] = '0';
        }
      strncpy (glob_condstr + 10, ts, 2);
      }
    XtVaGetValues (disttxt[0],
      XmNvalue, &ts,
      NULL);
    strcpy (glob_condstr + 12, ts);
    strcpy (tmpstr, "00");
    XmTextReplace (disttxt[2], 0, 2, tmpstr);
    strncpy (glob_condstr + 8, tmpstr, 2);
    }
  else
    {
    title = XmStringCreateLocalized (atable[attr_num - 1][1]);
    XtUnmanageChild (disttxt[0]);
    XtUnmanageChild (distlbl[1]);
    XtUnmanageChild (disttxt[1]);
    }
  XtVaSetValues (distpb,
    XmNlabelString, title,
    NULL);
  XmStringFree (title);

  sprintf (tmpstr, "%03d", attr_num);
  strncpy (glob_condstr + 5, tmpstr, 3);
}
