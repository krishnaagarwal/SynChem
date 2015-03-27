#ifndef _H_SEL_TRACE_
#define _H_SEL_TRACE_  1
/******************************************************************************
*
*  Copyright (C) 1996 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SEL_TRACE.H
*
*    This header file defines the data structure for managing
*    the information needed to trace the selection of compounds
*    to expand in a given status file and display it in the PST
*    viewer. 
*
*
*  Creation Date:
*
*    10-Jan-1996
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
#include "synchem.h"

#ifndef _H_PST_
#include "pst.h"
#endif

/*** Literal Values ***/

#define SLT_CYCLE_NONE        (U32_t) -1
#define SLT_TEXT_BUFLEN       8

#define SLT_PUSHB_DISMISS     "dismiss"
#define SLT_PUSHB_CLEAR       "clear"
#define SLT_PUSHB_NUMALL      "num all"
#define SLT_TITLE             "Select Trace"
#define SLT_LABEL_CYCLE       "Cycle:  "
#define SLT_LABEL_CMP         "Compound:  "
#define SLT_LABEL_LVL         "Level:  "
#define SLT_LABEL_NUM         "00000000"
#define SLT_LABEL_SELECT      "select"

#define SLT_ERROR_NOSTRING\
          "Select Trace:  error.  Text window returned NULL string."
#define SLT_WARN_CONVERT\
          "Select Trace:  invalid entry.  Unable to convert string to integer."
#define SLT_WARN_INVALID\
          "Select Trace:  invalid cycle entered."
#define SLT_WARN_NOPST\
          "Select Trace:  not currently examining a PST."
#define SLT_WARN_NOCYCLES\
          "Select Trace:  no compounds have been expanded for the current PST."
#define SLT_WARN_NOTSOLVED\
          "Select Trace:  not all subgoals along the path from the root\n"\
          "to the selected compound are completely solved."
#define SLT_WARN_NOTSEL\
          "Select Trace:  the entered compound was not selected for expansion."
#define SLT_WARN_NOMORESOL\
          "Select Trace:  there are no more selected compounds\n"\
          "that are children of solved subgoals."
#define SLT_WARN_BADSEL\
          "Select Trace:  no compound was selected."

#define SLT_ARROW_DIM           25


typedef struct sel_trace_s
  {
  Compound_t   **sels;                 /* Array of selected compound ptrs */
  Compound_t    *curr_cmp;             /* Current compound */ 
  Widget         formdlg;              /* Selected trace form dialog */ 
  Widget         cmp_form;             /* Compound widgets form */ 
  Widget         cmplbl;               /* Compound label label */ 
  Widget         cmpsel_pb;            /* Compound select push button */ 
  Widget         cmptxt;               /* Compound number text--editable */ 
  Widget         cycle_form;           /* Cycle widgets form */ 
  Widget         cycle_lbl;            /* Current Cycle label */
  Widget         cyclenext_ab;         /* Next subgoal arrow button */ 
  Widget         cycleprev_ab;         /* Previous subgoal arrow button */ 
  Widget         cycletext;            /* Cycle text--editable */ 
  Widget         lvl_form;             /* Level widgets form */ 
  Widget         lvllbl;               /* Level label label */ 
  Widget         lvlnum;               /* Level number label */ 
  Widget         clear_pb;             /* Clear all pb in action area */ 
  Widget         dismiss_pb;           /* Dismiss pushbutton in action area */ 
  Widget         numall_pb;            /* Number all pb in action area */ 
  U32_t          curr_cycle;           /* Current cycle  */ 
  U32_t          num_cycles;           /* Number of completed cycles */
  char           cmpbuf[SLT_TEXT_BUFLEN];
  char           cyclebuf[SLT_TEXT_BUFLEN];
  Boolean_t      is_created;           /* Select Trace created? */
  }  SelTrace_t;
#define SLT_SELTRACE_SIZE  (sizeof (SelTrace_t))

/** Field Access Macros for SelTrace_t **/

/* Macro Prototypes

  Widget          SelTrace_ClearPB_Get   (SelTrace_t *);
  void            SelTrace_ClearPB_Put   (SelTrace_t *, Widget);
  char           *SelTrace_CmpBuf_Get    (SelTrace_t *);
  Widget          SelTrace_CmpForm_Get   (SelTrace_t *);
  void            SelTrace_CmpForm_Put   (SelTrace_t *, Widget);
  Widget          SelTrace_CmpLbl_Get    (SelTrace_t *);
  void            SelTrace_CmpLbl_Put    (SelTrace_t *, Widget);
  Widget          SelTrace_CmpSelPB_Get  (SelTrace_t *);
  void            SelTrace_CmpSelPB_Put  (SelTrace_t *, Widget);
  Widget          SelTrace_CmpText_Get   (SelTrace_t *);
  void            SelTrace_CmpText_Put   (SelTrace_t *, Widget);
  Compound_t     *SelTrace_CurrCmp_Get   (SelTrace_t *);
  void            SelTrace_CurrCmp_Put   (SelTrace_t *, Compound_t *);
  U32_t           SelTrace_CurrCycle_Get (SelTrace_t *);
  void            SelTrace_CurrCycle_Put (SelTrace_t *, U32_t);
  char           *SelTrace_CycleBuf_Get  (SelTrace_t *);
  Widget          SelTrace_CycleForm_Get (SelTrace_t *);
  void            SelTrace_CycleForm_Put (SelTrace_t *, Widget);
  Widget          SelTrace_CycleLbl_Get  (SelTrace_t *);
  void            SelTrace_CycleLbl_Put  (SelTrace_t *, Widget);
  Widget          SelTrace_CycleNextAB_Get (SelTrace_t *);
  void            SelTrace_CycleNextAB_Put (SelTrace_t *, Widget);
  Widget          SelTrace_CyclePrevAB_Get (SelTrace_t *);
  void            SelTrace_CyclePrevAB_Put (SelTrace_t *, Widget);
  Widget          SelTrace_CycleText_Get (SelTrace_t *);
  void            SelTrace_CycleText_Put (SelTrace_t *, Widget);
  Widget          SelTrace_DismissPB_Get (SelTrace_t *);
  void            SelTrace_DismissPB_Put (SelTrace_t *, Widget);
  Widget          SelTrace_FormDlg_Get   (SelTrace_t *);
  void            SelTrace_FormDlg_Put   (SelTrace_t *, Widget);
  Boolean_t       SelTrace_IsCreated_Get (SelTrace_t *);
  void            SelTrace_IsCreated_Put (SelTrace_t *, Boolean_t);
  Compound_t     *SelTrace_IthSelCmp_Get (SelTrace_t *, U32_t);
  void            SelTrace_IthSelCmp_Put (SelTrace_t *, U32_t, Compound_t *);
  Widget          SelTrace_LvlForm_Get   (SelTrace_t *);
  void            SelTrace_LvlForm_Put   (SelTrace_t *, Widget);
  Widget          SelTrace_LvlLblL_Get   (SelTrace_t *);
  void            SelTrace_LvlLblL_Put   (SelTrace_t *, Widget);
  Widget          SelTrace_LvlNumL_Get   (SelTrace_t *);
  void            SelTrace_LvlNumL_Put   (SelTrace_t *, Widget);
  Widget          SelTrace_NumAllPB_Get  (SelTrace_t *);
  void            SelTrace_NumAllPB_Put  (SelTrace_t *, Widget);
  U32_t           SelTrace_NumCycles_Get (SelTrace_t *);
  void            SelTrace_NumCycles_Put (SelTrace_t *, U32_t);
  Compound_t    **SelTrace_SelCmps_Get   (SelTrace_t *);
  void            SelTrace_SelCmps_Put   (SelTrace_t *, Compound_t **);
*/

#define SelTrace_ClearPB_Get(stp)\
  (stp)->clear_pb
#define SelTrace_ClearPB_Put(stp, value)\
  (stp)->clear_pb = (value)
#define SelTrace_CmpBuf_Get(stp)\
  (stp)->cmpbuf
#define SelTrace_CmpForm_Get(stp)\
  (stp)->cmp_form
#define SelTrace_CmpForm_Put(stp, value)\
  (stp)->cmp_form = (value)
#define SelTrace_CmpLbl_Get(stp)\
  (stp)->cmplbl
#define SelTrace_CmpLbl_Put(stp, value)\
  (stp)->cmplbl = (value)
#define SelTrace_CmpSelPB_Get(stp)\
  (stp)->cmpsel_pb
#define SelTrace_CmpSelPB_Put(stp, value)\
  (stp)->cmpsel_pb = (value)
#define SelTrace_CmpText_Get(stp)\
  (stp)->cmptxt
#define SelTrace_CmpText_Put(stp, value)\
  (stp)->cmptxt = (value)
#define SelTrace_CurrCmp_Get(stp)\
  (stp)->curr_cmp
#define SelTrace_CurrCmp_Put(stp, value)\
  (stp)->curr_cmp = (value)
#define SelTrace_CurrCycle_Get(stp)\
  (stp)->curr_cycle
#define SelTrace_CurrCycle_Put(stp, value)\
  (stp)->curr_cycle = (value)
#define SelTrace_CycleBuf_Get(stp)\
  (stp)->cyclebuf
#define SelTrace_CycleForm_Get(stp)\
  (stp)->cycle_form
#define SelTrace_CycleForm_Put(stp, value)\
  (stp)->cycle_form = (value)
#define SelTrace_CycleLbl_Get(stp)\
  (stp)->cycle_lbl
#define SelTrace_CycleLbl_Put(stp, value)\
  (stp)->cycle_lbl = (value)
#define SelTrace_CycleNextAB_Get(stp)\
  (stp)->cyclenext_ab
#define SelTrace_CycleNextAB_Put(stp, value)\
  (stp)->cyclenext_ab = (value)
#define SelTrace_CyclePrevAB_Get(stp)\
  (stp)->cycleprev_ab
#define SelTrace_CyclePrevAB_Put(stp, value)\
  (stp)->cycleprev_ab = (value)
#define SelTrace_CycleText_Get(stp)\
  (stp)->cycletext
#define SelTrace_CycleText_Put(stp, value)\
  (stp)->cycletext = (value)
#define SelTrace_DismissPB_Get(stp)\
  (stp)->dismiss_pb
#define SelTrace_DismissPB_Put(stp, value)\
  (stp)->dismiss_pb = (value)
#define SelTrace_FormDlg_Get(stp)\
  (stp)->formdlg
#define SelTrace_FormDlg_Put(stp, value)\
  (stp)->formdlg = (value)
#define SelTrace_IsCreated_Get(stp)\
  (stp)->is_created
#define SelTrace_IsCreated_Put(stp, value)\
  (stp)->is_created = (value)
#define SelTrace_IthSelCmp_Get(stp, ith)\
  (stp)->sels[ith]
#define SelTrace_IthSelCmp_Put(stp, ith, value)\
  (stp)->sels[ith] = (value)
#define SelTrace_LvlForm_Get(stp)\
  (stp)->lvl_form
#define SelTrace_LvlForm_Put(stp, value)\
  (stp)->lvl_form = (value)
#define SelTrace_LvlLblL_Get(stp)\
  (stp)->lvllbl
#define SelTrace_LvlLblL_Put(stp, value)\
  (stp)->lvllbl = (value)
#define SelTrace_LvlNumL_Get(stp)\
  (stp)->lvlnum
#define SelTrace_LvlNumL_Put(stp, value)\
  (stp)->lvlnum = (value)
#define SelTrace_NumAllPB_Get(stp)\
  (stp)->numall_pb
#define SelTrace_NumAllPB_Put(stp, value)\
  (stp)->numall_pb = (value)
#define SelTrace_NumCycles_Get(stp)\
  (stp)->num_cycles
#define SelTrace_NumCycles_Put(stp, value)\
  (stp)->num_cycles = (value)
#define SelTrace_SelCmps_Get(stp)\
  (stp)->sels
#define SelTrace_SelCmps_Put(stp, value)\
  (stp)->sels = (value)
/*** Macros ***/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

void         SelTrace_Init       (SelTrace_t *);
void         SelTrace_Reset      (SelTrace_t *);

/*  Routines defined in pst_view.h  (to prevent circular includes) 
void         SelTrace_Create     (Widget, PstView_t *);
Boolean_t    SelTrace_Cycles_Set (PstView_t *);
Boolean_t    SelTrace_Setup      (PstView_t *);
void         SelTrace_Update     (PstView_t *, U32_t);
*/

void SelTrace_Clear_CB        (Widget, XtPointer, XtPointer);
void SelTrace_CmpSel_CB       (Widget, XtPointer, XtPointer);
void SelTrace_CmpText_CB      (Widget, XtPointer, XtPointer);
void SelTrace_CycleNext_CB    (Widget, XtPointer, XtPointer);
void SelTrace_CyclePrev_CB    (Widget, XtPointer, XtPointer);
void SelTrace_CycleText_CB    (Widget, XtPointer, XtPointer);
void SelTrace_Dismiss_CB      (Widget, XtPointer, XtPointer);
void SelTrace_NumAll_CB       (Widget, XtPointer, XtPointer);

#endif
/*  End of SEL_TRACE.H  */
