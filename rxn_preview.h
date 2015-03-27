#ifndef _H_RXN_PREVIEW_
#define _H_RXN_PREVIEW_  1
/******************************************************************************
*
*  Copyright (C) 1995 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     RXN_PREVIEW.H
*
*    This header file defines the data structures for managing
*    the information needed to preview a reaction by showing the
*    bonds that would be changed in the target molecule. 
*
*
*  Creation Date:
*
*    10-Jun-1995
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

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_DSP_
#include "dsp.h"
#endif

/*** Literal Values ***/

#define RPV_PREVIEW_TOP         0    /* Top level Previewer */
#define RPV_PREVIEW_MID         1    /* Middle level Previewer */
#define RPV_PREVIEW_BTM         2    /* Bottom level Previewer */
#define RPV_PREVIEW_PATH        3    /* Path level Previewer */

#define RPV_PREVIEW_TITLE_PATH  "Path Previewer"
#define RPV_PREVIEW_TITLE_TOPL  "Subtree Root Previewer"
#define RPV_PREVIEW_TITLE_PRIM  "Primary Reaction Previewer"
#define RPV_PREVIEW_TITLE_SEC   "Secondary Reaction Previewer"

#define RPV_LABEL_BUFLEN        64
#define RPV_LABEL_CMPNUM        "Compound Numbers"
#define RPV_LABEL_CMPN_PLUS     " + "
#define RPV_LABEL_CMPN_ARRW     "   =>   "
#define RPV_LABEL_CMPNWARN      "(No precursors)"
#define RPV_LABEL_NAME          "Reaction Name"
#define RPV_LABEL_NONAMEWARN    "(No name for this reaction)"
#define RPV_LABEL_CMPMRT        "cmp merit:  "
#define RPV_LABEL_SGMRT         "sgl merit:  "
#define RPV_LABEL_TARGET        "Target Compound"

#define RPV_LABEL_NAME_LINES    3

#define RPV_PUSHB_DISMISS       "dismiss"
#define RPV_PUSHB_RESET         "reset"


/*** Data Structures ***/

typedef struct rxn_preview_s
  {
  Widget         formdlg;             /* Rxn previewer form dialog */ 
  Widget         mol_da;              /* Target mol drawing area */ 
  Widget         name_lbl;            /* Reaction name label */ 
  Widget         cmpnum_lbl;          /* Compound numbers label */ 
  Widget         dismiss_pb;          /* Dismiss pushbutton */ 
  Widget         reset_pb;            /* Reset size default pushbutton  */ 
  Widget         sgmrt_lbl;           /* Subgoal merit label */
  Dsp_Molecule_t *mol_p;              /* Display molecule */
  String_t       rxn_name;            /* Name of reaction */
  Dimension      da_h;                /* Drawing area height */
  Dimension      da_w;                /* Drawing area width */
  Dimension      dflt_h;              /* Default drawing area dimensions */
  Dimension      dflt_w;              /* Default drawing area dimensions */
  Dimension      char_w;              /* Width of character in rxn name */
  Dimension      rem_ht;              /* Remaining height of dialog form */
  Dimension      rem_wd;              /* Remaining width of dialog form */
  U16_t          target_idx;          /* Target molecule index */
  U8_t           line_w;              /* Number of chars per line */
  U8_t           which;               /* Which preview, top or middle? */
  Boolean_t      is_new;              /* Preview newly created? */
}  RxnPreView_t;

/** Field Access Macros for RxnPreView_t **/

/* Macro Prototypes

  Dimension       RxnPreV_CharWd_Get    (RxnPreView_t *);
  void            RxnPreV_CharWd_Put    (RxnPreView_t *, Dimension);
  Widget          RxnPreV_CmpNumLbl_Get (RxnPreView_t *);
  void            RxnPreV_CmpNumLbl_Put (RxnPreView_t *, Widget);
  Dimension       RxnPreV_DAHt_Get      (RxnPreView_t *);
  void            RxnPreV_DAHt_Put      (RxnPreView_t *, Dimension);
  Dimension       RxnPreV_DAWd_Get      (RxnPreView_t *);
  void            RxnPreV_DAWd_Put      (RxnPreView_t *, Dimension);
  Dimension       RxnPreV_DfltHt_Get    (RxnPreView_t *);
  void            RxnPreV_DfltHt_Put    (RxnPreView_t *, Dimension);
  Dimension       RxnPreV_DfltWd_Get    (RxnPreView_t *);
  void            RxnPreV_DfltWd_Put    (RxnPreView_t *, Dimension);
  Widget          RxnPreV_DismissPB_Get (RxnPreView_t *);
  void            RxnPreV_DismissPB_Put (RxnPreView_t *, Widget);
  Widget          RxnPreV_FormDlg_Get   (RxnPreView_t *);
  void            RxnPreV_FormDlg_Put   (RxnPreView_t *, Widget);
  Boolean_t       RxnPreV_IsNew_Get     (RxnPreView_t *);
  void            RxnPreV_IsNew_Put     (RxnPreView_t *, Boolean_t);
  U8_t            RxnPreV_LineWd_Get    (RxnPreView_t *);
  void            RxnPreV_LineWd_Put    (RxnPreView_t *, U8_t);
  Dsp_Molecule_t *RxnPreV_Mol_Get       (RxnPreView_t *);
  void            RxnPreV_Mol_Put       (RxnPreView_t *, Dsp_Molecule_t *);
  Widget          RxnPreV_MolDA_Get     (RxnPreView_t *);
  void            RxnPreV_MolDA_Put     (RxnPreView_t *, Widget);
  Widget          RxnPreV_NameLbl_Get   (RxnPreView_t *);
  void            RxnPreV_NameLbl_Put   (RxnPreView_t *, Widget);
  Dimension       RxnPreV_RemHt_Get     (RxnPreView_t *);
  void            RxnPreV_RemHt_Put     (RxnPreView_t *, Dimension);
  Dimension       RxnPreV_RemWd_Get     (RxnPreView_t *);
  void            RxnPreV_RemWd_Put     (RxnPreView_t *, Dimension);
  Widget          RxnPreV_ResetPB_Get   (RxnPreView_t *);
  void            RxnPreV_ResetPB_Put   (RxnPreView_t *, Widget);
  String_t        RxnPreV_RxnName_Get   (RxnPreView_t *);
  void            RxnPreV_RxnName_Put   (RxnPreView_t *, String_t);
  Widget          RxnPreV_SGMrtLbl_Get  (RxnPreView_t *);
  void            RxnPreV_SGMrtLbl_Put  (RxnPreView_t *, Widget);
  U8_t            RxnPreV_Which_Get     (RxnPreView_t *);
  void            RxnPreV_Which_Put     (RxnPreView_t *, U8_t);
*/

#define RxnPreV_CharWd_Get(pvp)\
  (pvp)->char_w
#define RxnPreV_CharWd_Put(pvp, value)\
  (pvp)->char_w = (value)
#define RxnPreV_CmpNumLbl_Get(pvp)\
  (pvp)->cmpnum_lbl
#define RxnPreV_CmpNumLbl_Put(pvp, value)\
  (pvp)->cmpnum_lbl = (value)
#define RxnPreV_DAHt_Get(pvp)\
  (pvp)->da_h
#define RxnPreV_DAHt_Put(pvp, value)\
  (pvp)->da_h = (value)
#define RxnPreV_DAWd_Get(pvp)\
  (pvp)->da_w
#define RxnPreV_DAWd_Put(pvp, value)\
  (pvp)->da_w = (value)
#define RxnPreV_DfltHt_Get(pvp)\
  (pvp)->dflt_h
#define RxnPreV_DfltHt_Put(pvp, value)\
  (pvp)->dflt_h = (value)
#define RxnPreV_DfltWd_Get(pvp)\
  (pvp)->dflt_w
#define RxnPreV_DfltWd_Put(pvp, value)\
  (pvp)->dflt_w = (value)
#define RxnPreV_DismissPB_Get(pvp)\
  (pvp)->dismiss_pb
#define RxnPreV_DismissPB_Put(pvp, value)\
  (pvp)->dismiss_pb = (value)
#define RxnPreV_FormDlg_Get(pvp)\
  (pvp)->formdlg
#define RxnPreV_FormDlg_Put(pvp, value)\
  (pvp)->formdlg = (value)
#define RxnPreV_IsNew_Get(pvp)\
  (pvp)->is_new
#define RxnPreV_IsNew_Put(pvp, value)\
  (pvp)->is_new = (value)
#define RxnPreV_LineWd_Get(pvp)\
  (pvp)->line_w
#define RxnPreV_LineWd_Put(pvp, value)\
  (pvp)->line_w = (value)
#define RxnPreV_Mol_Get(pvp)\
  (pvp)->mol_p
#define RxnPreV_Mol_Put(pvp, value)\
  (pvp)->mol_p = (value)
#define RxnPreV_MolDA_Get(pvp)\
  (pvp)->mol_da
#define RxnPreV_MolDA_Put(pvp, value)\
  (pvp)->mol_da = (value)
#define RxnPreV_NameLbl_Get(pvp)\
  (pvp)->name_lbl
#define RxnPreV_NameLbl_Put(pvp, value)\
  (pvp)->name_lbl = (value)
#define RxnPreV_RemHt_Get(pvp)\
  (pvp)->rem_ht
#define RxnPreV_RemHt_Put(pvp, value)\
  (pvp)->rem_ht = (value)
#define RxnPreV_RemWd_Get(pvp)\
  (pvp)->rem_wd
#define RxnPreV_RemWd_Put(pvp, value)\
  (pvp)->rem_wd = (value)
#define RxnPreV_ResetPB_Get(pvp)\
  (pvp)->reset_pb
#define RxnPreV_ResetPB_Put(pvp, value)\
  (pvp)->reset_pb = (value)
#define RxnPreV_RxnName_Get(pvp)\
  (pvp)->rxn_name
#define RxnPreV_RxnName_Put(pvp, value)\
  (pvp)->rxn_name = (value)
#define RxnPreV_SGMrtLbl_Get(pvp)\
  (pvp)->sgmrt_lbl
#define RxnPreV_SGMrtLbl_Put(pvp, value)\
  (pvp)->sgmrt_lbl = (value)
#define RxnPreV_Which_Get(pvp)\
  (pvp)->which
#define RxnPreV_Which_Put(pvp, value)\
  (pvp)->which = (value)

/*** Macros ***/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

void RxnPreV_Clear          (RxnPreView_t *);
void RxnPreV_Create         (Widget, RxnPreView_t *, Pixel);
void RxnPreV_Destroy        (RxnPreView_t *);
void RxnPreV_RxnName_Redo   (RxnPreView_t *, char *);
void RxnPreV_Update         (RxnPreView_t *, Compound_t *, Boolean_t);

void RxnPreV_ResetPB_CB     (Widget, XtPointer, XtPointer);


#endif
/*  End of RXN_PREVIEW.H  */
