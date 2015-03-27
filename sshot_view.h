#ifndef _H_SSHOT_VIEW_
#define _H_SSHOT_VIEW_  1
/******************************************************************************
*
*  Copyright (C) 1995 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SSHOT_VIEW.H
*
*    This header file defines the data structure for managing
*    the information needed to view an application of singleshot
*    to the selected reaction in the PST view. 
*
*
*  Creation Date:
*
*    26-Dec-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
*
******************************************************************************/

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_SGLSHOT_
#include "sglshot.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

/*** Literal Values ***/

#define SSV_DLGSHELL_OFFX      15
#define SSV_RXNMOL_BRDR        30

#define SSV_TEXTBUFF_SIZE      40960

#define SSV_LABEL_TESTS       "Post-transform Tests"
#define SSV_MOL_TRGT          "Target"
#define SSV_MOL_PREC          "Precursor "
#define SSV_MOL_MRT           " "
#define SSV_PUSHB_DISMISS     "dismiss"
#define SSV_PUSHB_NUMON       "numbering on"
#define SSV_PUSHB_NUMOFF      "numbering off"
#define SSV_PUSHB_NEXT        "next"
#define SSV_PUSHB_PREVIOUS    "previous"
#define SSV_TITLE             "Single Shot"
#define SSV_WARN_NORXN        "No reaction is currently selected in the PST."

#define SSV_WARN_INVALID      "An invalid precursor compound was generated."
#define SSV_WARN_NO_INFO      "Warning!  No information for generated subgoal.\n(This message is usually a cryptic way of stating either that the target does not match the goal pattern or that a preservable bond would have been broken!)"
#define SSV_WARN_NO_TESTS     "No Post-transform Tests for this reaction."
#define SSV_WARN_PRET_FAIL    "Target compound failed the Pre-transform Tests."
#define SSV_WARN_MRTUP_FAIL   "Generated subgoal failed Merit Update."
#define SSV_WARN_MAXAPP_FAIL\
  "Generated subgoal failed the Maximum Application Test."
#define SSV_WARN_MAXNONID_FAIL\
  "Generated subgoal failed the Maximum Nonidentical Test."

#define SSV_PARVAL_FAIL         "fail"

#define SSV_TESTNUM              "Test #"
#define SSV_TESTMESS_ERROR       "Error!  Unknown test result:  "
#define SSV_TESTMESS_FAIL        "Test failed."
#define SSV_TESTMESS_FALSE       " not applied--condition not satisfied."
#define SSV_TESTMESS_NONE        "Test skipped or unreached."
#define SSV_TESTMESS_PASS        "Condition satisfied"
#define SSV_TESTMESS_CONFDEC     "CONF - "
#define SSV_TESTMESS_CONFINC     "CONF + "
#define SSV_TESTMESS_EASEDEC     "EASE - "
#define SSV_TESTMESS_EASEINC     "EASE + "
#define SSV_TESTMESS_YLDDEC      "YIELD - "
#define SSV_TESTMESS_YLDINC      "YIELD + "



typedef struct sshot_view_s
  {
  RxnView_t      rxnvw;               /* Reaction view */
  React_Record_t *rxn_rec_p;          /* Reaction record */
  Xtr_t         *target_p;            /* Target Xtr ptr */
  SShotGenSGs_t  gensgs;              /* Generated subgoals */
  char           text_buf[2][SSV_TEXTBUFF_SIZE];
  Widget         formdlg[2];          /* Singleshot view form dialogs */ 
  Widget         sc_text[2];          /* Scrolled text for posttran tests */ 
  Widget         dismiss_pb[2];       /* Dismiss pushbutton in action area */ 
  Widget         numtogl_pb;          /* Numbering toggle pushbutton in action area */ 
  Widget         next_pb;             /* Next subgoal pushbutton */ 
  Widget         prev_pb;             /* Previous subgoal pushbutton */ 
  Widget         test_lbl;            /* posttran tests label */
  U8_t           curr_sg;             /* Current generated subgoal */ 
  Boolean_t      is_created;          /* SShot view created? */
  }  SShotView_t;
#define SSV_SSHOTVIEW_SIZE  (sizeof (SShotView_t))

/** Field Access Macros for SShotView_t **/

/* Macro Prototypes

  U8_t            SShotV_CurrGenSG_Get (SShotView_t *);
  void            SShotV_CurrGenSG_Put (SShotView_t *, U8_t);
  Widget          SShotV_DismissPB_Get (SShotView_t *, int);
  void            SShotV_DismissPB_Put (SShotView_t *, int, Widget);
  Widget          SShotV_NumToglPB_Get (SShotView_t *);
  void            SShotV_NumToglPB_Put (SShotView_t *, Widget);
  Widget          SShotV_FormDlg_Get   (SShotView_t *, int);
  void            SShotV_FormDlg_Put   (SShotView_t *, int, Widget);
  SShotGenSGs_t  *SShotV_GenSGs_Get    (SShotView_t *);
  Boolean_t       SShotV_IsCreated_Get (SShotView_t *);
  void            SShotV_IsCreated_Put (SShotView_t *, Boolean_t);
  Widget          SShotV_NextPB_Get    (SShotView_t *);
  void            SShotV_NextPB_Put    (SShotView_t *, Widget);
  Widget          SShotV_PrevPB_Get    (SShotView_t *);
  void            SShotV_PrevPB_Put    (SShotView_t *, Widget);
  React_Record_t *SShotV_RxnRec_Get    (SShotView_t *);
  void            SShotV_RxnRec_Put    (SShotView_t *, React_Record_t *);
  RxnView_t      *SShotV_RxnView_Get   (SShotView_t *);
  Widget          SShotV_ScrlText_Get  (SShotView_t *, int);
  void            SShotV_ScrlText_Put  (SShotView_t *, int, Widget);
  Widget          SShotV_TestLbl_Get   (SShotView_t *);
  void            SShotV_TestLbl_Put   (SShotView_t *, Widget);
  char           *SShotV_TextBuf_Get   (SShotView_t *, int);
  Xtr_t          *SShotV_TgtXtr_Get    (SShotView_t *);
  void            SShotV_TgtXtr_Put    (SShotView_t *, Xtr_t *);
*/

#define SShotV_CurrGenSG_Get(svp)\
  (svp)->curr_sg
#define SShotV_CurrGenSG_Put(svp, value)\
  (svp)->curr_sg = (value)
#define SShotV_DismissPB_Get(svp, num)\
  (svp)->dismiss_pb[(num)]
#define SShotV_DismissPB_Put(svp, num, value)\
  (svp)->dismiss_pb[(num)] = (value)
#define SShotV_NumToglPB_Get(svp)\
  (svp)->numtogl_pb
#define SShotV_NumToglPB_Put(svp, value)\
  (svp)->numtogl_pb = (value)
#define SShotV_FormDlg_Get(svp, num)\
  (svp)->formdlg[(num)]
#define SShotV_FormDlg_Put(svp, num, value)\
  (svp)->formdlg[(num)] = (value)
#define SShotV_GenSGs_Get(svp)\
  &((svp)->gensgs)
#define SShotV_IsCreated_Get(svp)\
  (svp)->is_created
#define SShotV_IsCreated_Put(svp, value)\
  (svp)->is_created = (value)
#define SShotV_NextPB_Get(svp)\
  (svp)->next_pb
#define SShotV_NextPB_Put(svp, value)\
  (svp)->next_pb = (value)
#define SShotV_PrevPB_Get(svp)\
  (svp)->prev_pb
#define SShotV_PrevPB_Put(svp, value)\
  (svp)->prev_pb = (value)
#define SShotV_RxnRec_Get(svp)\
  (svp)->rxn_rec_p
#define SShotV_RxnRec_Put(svp, value)\
  (svp)->rxn_rec_p = (value)
#define SShotV_RxnView_Get(svp)\
  &((svp)->rxnvw)
#define SShotV_ScrlText_Get(svp, num)\
  (svp)->sc_text[(num)]
#define SShotV_ScrlText_Put(svp, num, value)\
  (svp)->sc_text[(num)] = (value)
#define SShotV_TestLbl_Get(svp)\
  (svp)->test_lbl
#define SShotV_TestLbl_Put(svp, value)\
  (svp)->test_lbl = (value)
#define SShotV_TextBuf_Get(svp, num)\
  (svp)->text_buf[(num)]
#define SShotV_TgtXtr_Get(svp)\
  (svp)->target_p
#define SShotV_TgtXtr_Put(svp, value)\
  (svp)->target_p = (value)

/*** Macros ***/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

void         SShotView_Create  (Widget, SShotView_t *);
void         SShotView_Reset   (SShotView_t *);
void         SShotView_Setup   (SShotView_t *);
void         SShotView_Update  (SShotView_t *, U8_t);

void SShotView_Add_Pattern_Nums (Dsp_Molecule_t *, SShotInfo_t *, U16_t);

void SShotView_DismissPB_CB   (Widget, XtPointer, XtPointer);
void SShotView_NumToglPB_CB   (Widget, XtPointer, XtPointer);
void SShotView_NextPB_CB      (Widget, XtPointer, XtPointer);
void SShotView_PrevPB_CB      (Widget, XtPointer, XtPointer);
void SShotView_ScrlText_CB    (Widget, XtPointer, XtPointer);
void SShotView_DismissPTPB_CB (Widget, XtPointer, XtPointer);

#ifdef _GLOBAL_SSV_DEF_
static int            ncond = 0, ntest = 0;
static Condition_t   *cond;
static Posttest_t    *test;
static String_t      *reas, *chem;
static char        ***condv[2], ***testv[2];
static Boolean_t      new_view = FALSE, numbering_on = FALSE;
static Dimension     *targ_x = NULL, *targ_y = NULL;
#endif

#endif
/*  End of SSHOT_VIEW.H  */
