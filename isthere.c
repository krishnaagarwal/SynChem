/******************************************************************************
*
*  Copyright (C) 2000 Synchem Group at SUNY-Stony Brook, Jerry Miller
*
*  Module Name:                     ISTHERE.C
*
*    Utility for searching for schemata that match certain transform templates,
*    specified by drawings and/or lists of functional groups.
*
*  Creation Date:
*
*    19-Oct-2000
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
#include <sys/types.h>
#include <time.h>

#include <X11/Xlib.h>
#include <X11/Intrinsic.h>
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/Label.h>
#include <Xm/MessageB.h>
#include <Xm/Text.h>
#include <Xm/PushB.h>
#include <Xm/PushBG.h>

#ifdef TRUE
#undef TRUE
#endif

#ifdef FALSE
#undef FALSE
#endif

#include "synchem.h"
#include "synio.h"
#include "utl.h"
#include "array.h"
#include "app_resrc.h"

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_TSD2XTR_
#include "tsd2xtr.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTRFUNCGROUP_
#include "funcgroups.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_SUBGOALGENERATION_
#include "subgoalgeneration.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_SCHFORM_
#include "schform.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#include "rxnpatt_draw.h"
#include "extern.h"

void rxl_ist_refresh (int);

void IstSrch_CB (Widget, XtPointer, XtPointer);
void NMBDismiss_CB (Widget, XtPointer, XtPointer);

static time_t now;
static int current_schema_number = 0, numlibs = 3, actual_schema_number, nfg[2], nsl[2], fg[2][5], minin[2][5],
  *prev_schn, *save_NSch, last_viewed_pos = 1;
static Boolean_t last_schema_closed, glob_forward;
static Tsd_t *stsd[2][5] = {{NULL, NULL, NULL, NULL, NULL}, {NULL, NULL, NULL, NULL, NULL}},
  *pt[2] = {NULL, NULL}, *t[2] = {NULL, NULL};
static Xtr_t *xt[2] = {NULL, NULL};
static Widget tl, srch_popup, no_more_box;
static XtAppContext schlC;

Boolean_t try_match (int piece, int npieces, int gs, Array_t *in_ilat, Array_t *stopt)
{
  Xtr_t *gx, *gpx;
  Array_t local_ilat;
  int ii, jj, kk;
  Boolean_t found;
  MatchCB_t *mcb;
  Match_Node_t *node;
  Match_Leaf_t *leaf;

printf ("try_match (%d, %d, %d, ...)\n",piece,npieces,gs);
        gx = Tsd2Xtr (stsd[gs][piece]);
        gpx = xt[gs];
        Array_Copy (in_ilat, &local_ilat);
        jj = 0;
        found = FALSE;
        for (ii = 0; ii < Xtr_NumAtoms_Get (gpx) && !found; ii++) if (!Array_1d1_Get (&local_ilat, ii))
        {
          Array_CopyContents (in_ilat, &local_ilat);
printf("before SGFM\n");
          mcb = SubGenr_Fragment_Match (gpx, gx, stopt, &local_ilat, ii, jj, TRUE);
printf("before SGFM: mcb=%p\n",mcb);
          if (mcb != NULL)
          {
printf ("current node=%p\n", MatchCB_CurrentNode_Get (mcb));
            if (piece == npieces - 1)
            {
leaf = MatchCB_Leaf_Get (mcb);
node = Match_Leaf_Node_Get (leaf);
do
{
kk = Match_Node_GoalLink_Get (node);
printf ("%d: kk = %d; node = %p\n", Match_Node_PatternLink_Get (node), kk, node);
node = Match_Node_Back_Get (node);
printf ("node = %p\n", node);
}
while (node != NULL);
              found = TRUE;
            }
            else
            {
printf("setting local_ilat\n");
              leaf = MatchCB_Leaf_Get (mcb);
              node = Match_Leaf_Node_Get (leaf);
              do
              {
                kk = Match_Node_GoalLink_Get (node);
printf ("%d: kk = %d; node = %p\n", Match_Node_PatternLink_Get (node), kk, node);
                Array_1d1_Put (&local_ilat, kk, TRUE);
                node = Match_Node_Back_Get (node);
printf ("node = %p\n", node);
              }
              while (node != NULL);
printf("before try_match\n");
              found = try_match (piece + 1, npieces, gs, &local_ilat, stopt);
printf("after try_match\n");
            }
            SubGenr_MatchCB_Destroy (mcb);
          }
        }
        Array_Destroy (&local_ilat);
        Xtr_Destroy (gx);
        return (found);
}

void ist_fix_tsd (Tsd_t *t)
{
  int i, j;
  Boolean_t nbr_found;

  for (i = 0; i < Tsd_NumAtoms_Get (t); i++) if (Tsd_Atomid_Get (t, i) != 1)
  {
    for (j = 0, nbr_found = FALSE; j < 6 && !nbr_found; j++)
      if (Tsd_Atom_NeighborId_Get (t, i, j) != TSD_INVALID) nbr_found = TRUE;
    if (!nbr_found) Tsd_Atomid_Put (t, i, 1); /* no fg has sling "H" */
  }
}

Boolean_t find_next ()
{
  MatchCB_t *mcb;
  Array_t ilat, stopt;
  React_Record_t *schema;
  React_Head_t *sch_head;
  Xtr_t *gx, *gpx;
  Boolean_t is_temp, nota, found, lt;
  int gs, f, s, i, j, k, l, m, n, ii, jj, attr, savnode, loc_minin;
  static Boolean_t schedit_window_open = FALSE;

printf("current_schema_number=%d\n",current_schema_number);
  do
  {
savnode=0; /* purpose? */
   if (Persist_Legacy_Rxn (current_schema_number, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
   {
    actual_schema_number = Persist_Current_Rec (PER_STD, current_schema_number, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
    current_schema_number += (glob_forward ? 1 : -1); /* prepare for next iteration, since actual_schema_number is used for the remainder of this one */

    schema = React_Schema_Handle_Get (actual_schema_number);
    pt[0] = Tsd_Copy (React_Goal_Get (schema));
ist_fix_tsd(pt[0]);
    xt[0] = Tsd2Xtr (pt[0]);
    pt[1] = Tsd_Copy (React_Subgoal_Get (schema));
ist_fix_tsd(pt[1]);
    xt[1] = Tsd2Xtr (pt[1]);
sch_head = React_Head_Get (schema);
i = React_Head_SynthemeFG_Get (sch_head);

    for (gs = 1, found = TRUE; gs >= 0 && found; gs--) /* gs=0 FOR GOAL; gs=1 FOR SUBGOAL */
    /* SUBGOAL IS EXAMINED FIRST BECAUSE IT IS LESS LIKELY TO CONTAIN
       THE SYNTHEME FOR THE CHAPTER BEING SURVEYED */
    {
      if (nfg[gs] != 0) for (f = 0; f < nfg[gs] && found; f++)
      /* EXAMINE FOR EACH FUNCTIONAL GROUP */
      {
loc_minin = minin[gs][f];
lt = loc_minin < 0;
if (lt) loc_minin = -loc_minin;
        attr = fg[gs][f];
        nota = attr < 0;
        if (nota)
{
attr = -attr;
lt = TRUE;
loc_minin = 1;
}
        found = gs == 0 && attr == i && minin[gs][f] == 1;
        if (!found)
        {
          if (Xtr_FuncGroups_Get (xt[gs]) == NULL)
            Xtr_FuncGroups_Put (xt[gs], FuncGroups_Create (xt[gs]));
          found = lt ?
            Xtr_FuncGrp_NumInstances_Get (xt[gs], attr) < loc_minin :
            Xtr_FuncGrp_NumInstances_Get (xt[gs], attr) >= loc_minin;
        }
/*
        if (nota) found = !found;
*/
      }
      if (nsl[gs] != 0) for (s = 0; s < nsl[gs] && found; s++)
                    /* EXAMINE FOR EACH SLING */
      {
        t[gs] = stsd[gs][s];
        for (k = 0; k < Tsd_NumAtoms_Get (t[gs]) && found; k++)
        {
          found = FALSE;
          for (l = 0; l < Tsd_NumAtoms_Get (pt[gs]) && !found; l++)
          {
            found = Tsd_Atomid_Get (t[gs], k) == Tsd_Atomid_Get (pt[gs], l);
            for (m = 0; m < 6 && found; m++)
            {
              found = Tsd_Atom_NeighborBond_Get (t[gs], k, m) == 0;
              if (!found) found = Tsd_Atom_NeighborId_Get (t[gs], k, m) == TSD_INVALID;
              for (n = 0; n < 6 && !found; n++)
                if (Tsd_Atom_NeighborId_Get (pt[gs], l, n) != TSD_INVALID)
              {
                found = Tsd_Atom_NeighborBond_Get (pt[gs], l, n) != 0 &&
                  (Tsd_Atom_NeighborBond_Get (t[gs], k, m) == Tsd_Atom_NeighborBond_Get (pt[gs], l, n) ||
                  Tsd_Atom_NeighborBond_Get (t[gs], k, m) == BOND_VARIABLE ||
                  Tsd_Atom_NeighborBond_Get (pt[gs], l, n) == BOND_VARIABLE);
                if (found) found = Tsd_Atomid_Get (t[gs], Tsd_Atom_NeighborId_Get (t[gs], k, m)) ==
                  Tsd_Atomid_Get (pt[gs], Tsd_Atom_NeighborId_Get (pt[gs], l, n));
              }
            } 
            if (k == 0) savnode = l;
          }
        }
        if (found)
                     /* IF PRELIMINARY SCREENING IS PASSED,
                       USE FMATCH TO CONFIRM */
        {
printf("0.0");
fflush(stdout);
/*
        t[gs] = stsd[gs][0];
        gx = Tsd2Xtr (t[gs]);
*/
          gpx = xt[gs];
          Array_1d_Create (&ilat, Xtr_NumAtoms_Get (gpx), BITSIZE);
printf("0.1");
fflush(stdout);
          Array_Set (&ilat, FALSE);
printf("0.2");
fflush(stdout);
          Array_1d_Create (&stopt, Xtr_NumAtoms_Get (gpx), BITSIZE);
printf("0.3");
fflush(stdout);
          Array_Set (&stopt, FALSE);
/*
        jj = 0;
        found = FALSE;
        for (ii = savnode; ii < Xtr_NumAtoms_Get (gpx) && !found; ii++)
        {
          mcb = SubGenr_Fragment_Match (gpx, gx, &stopt, &ilat, ii, jj, TRUE);
          if (mcb != NULL)
          {
printf("*");
fflush(stdout);
            found = TRUE;
            SubGenr_MatchCB_Destroy (mcb);
          }
        }
*/
printf("1");
fflush(stdout);
found = try_match (0, nsl[gs], gs, &ilat, &stopt);
printf("2");
fflush(stdout);

          Array_Destroy (&ilat);
          Array_Destroy (&stopt);
/*
        Xtr_Destroy (gx);
*/
        }
      }
    }
    for (gs = 0; gs < 2; gs++)
    {
      Tsd_Destroy (pt[gs]);
      Xtr_Destroy (xt[gs]);
    }
    if (found)
    {
now=time(NULL);
fprintf(stderr,"before schedit: %s\n",ctime (&now));
/*
printf("tl=%p schlC=%p actual_schema_number=%d prev_schn=%d last_schema_closed=%d\n",tl,schlC,actual_schema_number,*prev_schn,last_schema_closed);
*/
    schedit_window_open=TRUE;
      SchEdit_Create_Form (tl, schlC, actual_schema_number, prev_schn, FALSE, last_schema_closed, FALSE);
last_viewed_pos=current_schema_number + (glob_forward ? 0 : 2);
now=time(NULL);
fprintf(stderr,"after schedit: %s\n",ctime (&now));
      return (TRUE);
    }
   }
   else current_schema_number += (glob_forward ? 1 : -1);
printf(" %d",current_schema_number);
fflush(stdout);
  }
/*
  while (current_schema_number < React_NumSchemas_Get ());
*/
  while ((glob_forward && current_schema_number < React_NumSchemas_Get ()) ||
    (!glob_forward && current_schema_number >= 0));

printf ("returning FALSE\n");
  if (schedit_window_open)
  { 
    schedit_window_close ();
    schedit_window_open=FALSE;
  }
  XtManageChild (no_more_box);
IsThere_Draw_Flag = FALSE;
glob_rxlform = TRUE;
  return (FALSE);
}

void isthere_cont (int mode)
{
int i, j;

now=time(NULL);
fprintf(stderr,"isthere_cont: %s\n",ctime (&now));
 if (mode == 0 && find_next ()) return;
printf("preparing to free stsd\n");
 for (i = 0; i < 2; i++) for (j = 0; j < 5; j++)
   if (stsd[i][j] != NULL) Tsd_Destroy (stsd[i][j]);
 if (mode != 0)
 {
  IsThere_Draw_Flag = FALSE;
  glob_rxlform = TRUE;
  rxl_ist_refresh (last_viewed_pos);
 }
}

void IsTGold ()
{
static RxnInfoCB_t rxninfo_p[2];

static Boolean_t first=TRUE;
static Widget srch_lbl, srch_txt, srch_pb, wait_lbl, drawtool;
XmString label, title, msg;
        XmFontList flhv18;
        XmFontListEntry helv18;

/*
  return ("C=C,-12 -> CCC*N");
  return ("2,3->31");
  if (first)
  {
  first=FALSE;
    return ("C=CC=C, C=C -> C=CCCCC/0");
return("C(BR)-1($2)->COH-2($2)");
    return ("C=O-1O, HCC=O-1O -> C=O-1C");
  }
  return("END");
*/

  IsThere_Draw_Flag = TRUE;

  drawtool = PDraw_Tool_Create(rxninfo_p,tl,&schlC,NULL,NULL,NULL,NULL,0);

 if (first)
 {
  first = FALSE;
  helv18 = XmFontListEntryLoad (XtDisplay (tl), "-*-helvetica-bold-r-normal-*-18-*-75-75-*-*-iso8859-1", XmFONT_IS_FONT,
    XmFONTLIST_DEFAULT_TAG_STRING);
  flhv18 = XmFontListAppendEntry (NULL, helv18);

  no_more_box = XmCreateMessageDialog (tl, "Message", NULL, 0);
  label=XmStringCreateLocalized("Dismiss");
  title=XmStringCreateLocalized("No more schemata");
  msg=XmStringCreateLocalized("No more matching schemata were found by IsThere.");
  XtVaSetValues (no_more_box,
        XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
        XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
        XmNokLabelString, label,
        XmNdialogTitle, title,
        XmNmessageString, msg,
        NULL);
  XmStringFree(label);
  XmStringFree(title);
  XmStringFree(msg);
  XtUnmanageChild (XmMessageBoxGetChild (no_more_box, XmDIALOG_CANCEL_BUTTON));
  XtUnmanageChild (XmMessageBoxGetChild (no_more_box, XmDIALOG_HELP_BUTTON));
  XtAddCallback (XmMessageBoxGetChild (no_more_box, XmDIALOG_OK_BUTTON), XmNactivateCallback, NMBDismiss_CB,
    (XtPointer) NULL);
  XtUnmanageChild (no_more_box);

/*
  srch_popup = XmCreateFormDialog (tl, "SrchFmDg", NULL, 0);

  label = XmStringCreateLocalized ("Search");
  XtVaSetValues (srch_popup,
    XmNdialogTitle,  label,
    XmNdialogStyle,  XmDIALOG_MODELESS,
    XmNautoUnmanage, False,
    NULL);
  XmStringFree (label);

  label = XmStringCreateLocalized ("IsThere search for:");
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
  XtAddCallback (srch_pb, XmNactivateCallback, IstSrch_CB,
    (XtPointer) NULL);
*/
 }
/*
 XtManageChild (srch_popup);
*/
}

React_Record_t *schm_find (int lib, int chap, int schm)
{
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *sch_text;
  React_TextHead_t *sch_txthd;
  int i, sch_num;
  Boolean_t is_temp;

  for (sch_num = 0; sch_num < React_NumSchemas_Get (); sch_num++)
    if (Persist_Legacy_Rxn (sch_num, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
  {
    i = Persist_Current_Rec (PER_STD, sch_num, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp);
    schema = React_Schema_Handle_Get (i);
    sch_head = React_Head_Get(schema);
    if (React_Head_Library_Get (sch_head) == lib && React_Head_SynthemeFG_Get (sch_head) == chap)
    {
      sch_text=React_Text_Get(schema);
      sch_txthd=React_TxtRec_Head_Get(sch_text);
      if (React_TxtHd_OrigSchema_Get (sch_txthd) == schm) return (schema);
    }
  }
  return (NULL);
}

Boolean_t num (char *str)
{
  if (atoi (str) != 0) return (TRUE);
  if (strcmp (str, "0") == 0) return (TRUE);
  return (FALSE);
}

Boolean_t check_nums (char *str, int *x)
{
  char nst[16], y[16], *i, *j, *k;

  *x = 1;
  if (str[0] == '\0' || str[0] == '-') return (num (str));
  i = strstr (str, "G");
  if (i == NULL) i = strstr (str, ">");
  j = strstr (str, "L");
  if (j == NULL) j = strstr (str, "<");
  if (i == NULL && j == NULL) return (num (str));
  if (i != NULL && j != NULL) return (FALSE);
  if (i == NULL) k = j;
  else k = i;
  if (k == str || k[1] == '\0') return (FALSE);
  *k = '\0';
  strcpy (nst, str);
  strcpy (y, k + 1);
  if (!num (nst) || !num (y)) return (FALSE);
  sprintf (str, "%s%s", k == j ? "-" : "", nst);
  if (k == i) ++*x;
  return (TRUE);
}

/*
HELP: PROC;
PUT EDIT(
   '     Use SLINGs to describe patterns which no FG# adequately represents.',
   'FG#''s have the advantages of allowing negation (preceded by a minus sign)'
   ,'and/or maxima/minima (followed by < or > and a number of instances).',
   '  Example 1: C=C,-12 -> CCC*N to find Michael addition of cyanide',
   '  Example 2: 2>1,-1 -> 2,1 to find acyloin condensation',
   'Queries are made for the purpose of finding an FG# whose associated name',
   'contains a given string. The string is followed by a question mark, e.g.:',
   'YLIDENE?')
   (8(SKIP,A));
PUT SKIP;
END HELP;
*/

void IsThere (Widget top_level, XtAppContext schlContext, int *ps, int *sns, Boolean_t lsc, Boolean_t forward)
{
 int i, j;

 tl = top_level;
 schlC = schlContext;
 prev_schn = ps;
 save_NSch = sns;
 last_schema_closed = lsc;

 for (i = 0; i < 2; i++) for (j = 0; j < 5; j++) stsd[i][j] = NULL;
 if (glob_forward = forward) current_schema_number = 0;
 else current_schema_number = React_NumSchemas_Get () - 1;

 IsTGold ();
}

/*
void IstSrch_CB (Widget w, XtPointer client_data, XtPointer call_data)
{
}
*/

void Ist_Info_Send (Tsd_t *gftsd, Tsd_t *sgftsd, int *goalfg, int *subgfg, int *gminin, int *sminin)
{
 char str[128], *si, *str2[2], *comma, attstr[8];
 String_t string;
 Sling_t sling;
 int f, s, i, j, k, l, m, n, gs, innum, fgn, attr, attn, libn, savnode, npc, map[200];
 U16_t ii, jj;
 Boolean_t all, exit_scan, inchap, done_inner, found, nota, was_all;
  char *lcl_srch_str;

  struct molecs
    {
    struct molecs *next;
    Tsd_t *tptr;
    }           *molecptr, *nextptr;

IsThere_Draw_Flag = FALSE;

  if (gftsd == NULL || sgftsd == NULL || goalfg == NULL || subgfg == NULL || gminin == NULL || sminin == NULL)
{
glob_rxlform = TRUE;
return;
}

  MSepMol (gftsd, &molecptr, &npc, map);
/*
  Tsd_Destroy (gftsd);
*/
  for (nsl[0]=0; nsl[0]<npc; nsl[0]++)
  {
    nextptr=molecptr->next;
    stsd[0][nsl[0]]=molecptr->tptr;
    free (molecptr);
    molecptr=nextptr;
  }
  if (molecptr!=NULL)
  {
    printf("Error in goal tsd list\n");
    exit(1);
  }

  MSepMol (sgftsd, &molecptr, &npc, map);
/*
  Tsd_Destroy (sgftsd);
*/
  for (nsl[1]=0; nsl[1]<npc; nsl[1]++)
  {
    nextptr=molecptr->next;
    stsd[1][nsl[1]]=molecptr->tptr;
    free (molecptr);
    molecptr=nextptr;
  }
  if (molecptr!=NULL)
  {
    printf("Error in subgoal tsd list\n");
    exit(1);
  }

/*
  lcl_srch_str = XmTextGetString(XtNameToWidget(srch_popup,"SearchTxt"));
  strcpy(str,lcl_srch_str);
strcpy(str,"c=c,c=o->cccccc/0");
strcpy(str,"c(cl)->ccc/0c/1");
  XtFree(lcl_srch_str);
  XtUnmanageChild(srch_popup);
fprintf(stderr,"\nGoal\n");
Tsd_Dump (gftsd, &GStdErr);
for (i=0; goalfg[i]; i++) fprintf(stderr, "FG #%d\n",goalfg[i]);
fprintf(stderr,"\nSubgoal\n");
Tsd_Dump (sgftsd,&GStdErr);
for (i=0; subgfg[i]; i++) fprintf(stderr, "FG #%d\n",subgfg[i]);

  while ((si = strstr (str, " ")) != NULL) strcpy (si, si + 1);
  if (strcmp (str, "END") == 0) return;

  si = strstr (str, "->");
  if (si == NULL)
  {
    fprintf (stderr, "Error - no arrow\n");
    exit (1);
  }
  *si = '\0';
  str2[0] = si + 2;
  str2[1] = str;
  for (gs = 1; gs >= 0; gs--)
  {
    strcat (str2[gs], ",");
    nfg[gs] = nsl[gs] = 0;
    while ((comma = strstr (str2[gs], ",")) != NULL)
    {
      *comma++ = '\0';
      strcpy (str, str2[gs]);
      str2[gs] = comma;
      if (check_nums (str, &innum))
      {
        if (++nfg[gs] > 5)
        {
          fprintf (stderr, "Error - >5 FG's\n");
          exit (1);
        }
        sscanf (str, "%d", fg[gs] + nfg[gs] - 1);
        if (fg[gs][nfg[gs] - 1] == 0)
          {
          fprintf (stderr, "Error - illegal FG#: 0\n");
          exit (1);
          }
        fgn = fg[gs][nfg[gs] - 1];
        minin[gs][nfg[gs] - 1] = innum;
      }
      else
      {
        if (++nsl[gs] > 5)
        {
          fprintf (stderr, "Error - >5 slings\n");
          exit (1);
        }
        string = String_Create ((const char *) str, 0);
        sling = String2Sling (string);
        if (!Sling_Validate (sling, NULL))
        {
          fprintf (stderr, "Error - invalid sling\n");
          exit (1);
        }
        stsd[gs][nsl[gs] - 1] = Sling2Tsd (sling);
Sling_Destroy(sling);
String_Destroy(string);
      }
    }
  }


  str[0] = '\0';
*/

for (nfg[0]=0; goalfg[nfg[0]]; nfg[0]++)
{
  fg[0][nfg[0]]=goalfg[nfg[0]];
  minin[0][nfg[0]]=gminin[nfg[0]];
}
for (nfg[1]=0; subgfg[nfg[1]]; nfg[1]++)
{
  fg[1][nfg[1]]=subgfg[nfg[1]];
  minin[1][nfg[1]]=sminin[nfg[1]];
}

for (i=0; i<2; i++)
{
fprintf(stderr,"%s Tsd(s):\n",i?"Subgoal":"Goal");
for (j=0; j<nsl[i]; j++) Tsd_Dump (stsd[i][j], &GStdErr);
fprintf(stderr,"%s FG(s):\n",i?"Subgoal":"Goal");
for (j=0; j<nfg[i]; j++) fprintf(stderr,"%d\n",fg[i][j]);
}

  for (gs = 0; gs < 2; gs++)
    for (i = 0; i < nfg[gs]; i++)
  {
    attn = abs (fg[gs][i]);
    /* minin[gs][i] = 1; *//* pending redesign of FG input widget */
    sprintf (attstr, " %d", attn);
    if (strstr (str, attstr + 1) == NULL)
      strcat (str, attstr);
  }
  find_next ();
}

void NMBDismiss_CB (Widget w, XtPointer clntd, XtPointer calld)
{
  XtUnmanageChild (no_more_box);
}
