#ifndef _H_RXN_VIEW_
#define _H_RXN_VIEW_  1
/******************************************************************************
*
*  Copyright (C) 1995 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     RXN_VIEW.H
*
*    This header file defines the data structures for managing
*    the information needed to view information for the selected
*    reaction during perusal of the pst. 
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

#ifndef _H_DSP_
#include "dsp.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

/*** Literal Values ***/

#define RVW_LBL_BUFLEN     256 //kka: was 128 - needed more to display SLING. 12/16/08

#define RVW_NUM_CHAPTERS   41
#define RVW_MAXNUM_MOLS    5

#define RVW_SIGN_HT        20
#define RVW_SIGN_WD        20


#define RVW_VAL_BUFLEN     32
#define RVW_VALUE_LIB      "         "
#define RVW_VALUE_MERIT    "         "
#define RVW_VALUE_NUM      "         "
#define RVW_VALUE_CHAP     "                                            "
#define RVW_VALUE_PARAM    "                  "
#define RVW_CHAP_LBL       "chapter:  "
#define RVW_CONF_LBL       "confidence:  "
#define RVW_EASE_LBL       "ease:  "
#define RVW_LIB_LBL        "library:  "
#define RVW_REACTION_NONE  "Reaction name"
#define RVW_SCHEMA_LBL     "schema:  "
#define RVW_SOLVED_LBL     "solved "
#define RVW_SGMRT_LBL      "subgoal merit:  "
#define RVW_YIELD_LBL      "yield:  "

/*** Data Structures ***/

typedef struct rxn_view_s
  {
  MolView_t             mols[RVW_MAXNUM_MOLS];  /* Molecule views */
  Widget                ch_lbl;             /* Chapter name label */ 
  Widget                ch_val;             /* Chapter name value label */ 
  Widget                conf_lbl;           /* Confidence label */ 
  Widget                conf_val;           /* Confidence value label */ 
  Widget                ease_lbl;           /* Ease label */ 
  Widget                ease_val;           /* Ease value label */ 
  Widget                form;               /* Form for the rxn view */ 
  Widget                frame;              /* Frame for the rxn view */ 
  Widget                lib_lbl;            /* Library name label */ 
  Widget                lib_val;            /* Library name value label */ 
  Widget                mol_form;           /* Mol bulletin board */ 
  Widget                name_lbl;           /* Rxn name label */ 
  Widget                schema_lbl;         /* Schema number label */ 
  Widget                schema_val;         /* Schema number value label */ 
  Widget                sgmrt_lbl;          /* Subgoal merit label */ 
  Widget                sgmrt_val;          /* Subgoal merit value label */ 
  Widget                solved_lbl;         /* Solved (subgoal) label */ 
  Widget                yield_lbl;          /* Yield label */ 
  Widget                yield_val;          /* Yield value label */ 
  Subgoal_t            *cursg_p;            /* Current subgoal pointer */
  Dimension             molform_w;          /* Width of mol bulletin board */
  Dimension             molda_h;            /* Mol drawing area height */   
  U8_t                  num_sgs;            /* Number of subgoals */ 
  }
  RxnView_t;

/** Field Access Macros for RxnView_t **/

/* Macro Prototypes

  Widget          RxnView_ChapLbl_Get    (RxnView_t *);
  void            RxnView_ChapLbl_Put    (RxnView_t *, Widget);
  Widget          RxnView_ChapVal_Get    (RxnView_t *);
  void            RxnView_ChapVal_Put    (RxnView_t *, Widget);
  Widget          RxnView_ConfLbl_Get    (RxnView_t *);
  void            RxnView_ConfLbl_Put    (RxnView_t *, Widget);
  Widget          RxnView_ConfVal_Get    (RxnView_t *);
  void            RxnView_ConfVal_Put    (RxnView_t *, Widget);
  Subgoal_t      *RxnView_CurSG_Get      (RxnView_t *);
  void            RxnView_CurSG_Put      (RxnView_t *, Subgoal_t *);
  U32_t           RxnView_CurSubgoal_Get (RxnView_t *);
  void            RxnView_CurSubgoal_Put (RxnView_t *, U32_t);
  Widget          RxnView_EaseLbl_Get    (RxnView_t *);
  void            RxnView_EaseLbl_Put    (RxnView_t *, Widget);
  Widget          RxnView_EaseVal_Get    (RxnView_t *);
  void            RxnView_EaseVal_Put    (RxnView_t *, Widget);
  Widget          RxnView_Form_Get       (RxnView_t *);
  void            RxnView_Form_Put       (RxnView_t *, Widget);
  Widget          RxnView_Frame_Get      (RxnView_t *);
  void            RxnView_Frame_Put      (RxnView_t *, Widget);
  MolView_t       RxnView_IthMol_Get     (RxnView_t *, U8_t);
  void            RxnView_IthMol_Put     (RxnView_t *, U8_t, MolView_t);
  Widget          RxnV_IthM_DA_Get       (RxnView_t *, U8_t);
  void            RxnV_IthM_DA_Put       (RxnView_t *, U8_t, Widget);
  unsigned int    RxnV_IthM_DAW_Get      (RxnView_t *, U8_t);
  void            RxnV_IthM_DAW_Put      (RxnView_t *, U8_t, unsigned int);
  Widget          RxnV_IthM_Form_Get     (RxnView_t *, U8_t);
  void            RxnV_IthM_Form_Put     (RxnView_t *, U8_t, Widget);
  Dimension       RxnV_IthM_LblsH_Get    (RxnView_t *, U8_t);
  void            RxnV_IthM_LblsH_Put    (RxnView_t *, U8_t, Dimension);
  Dimension       RxnV_IthM_LblW_Get     (RxnView_t *, U8_t);
  void            RxnV_IthM_LblW_Put     (RxnView_t *, U8_t, Dimension);
  Dsp_Molecule_t *RxnV_IthM_Mol_Get      (RxnView_t *, U8_t);
  void            RxnV_IthM_Mol_Put      (RxnView_t *, U8_t, Dsp_Molecule_t *);
  Widget          RxnV_IthM_MrtLbl_Get   (RxnView_t *, U8_t);
  void            RxnV_IthM_MrtLbl_Put   (RxnView_t *, U8_t, Widget);
  Widget          RxnV_IthM_NameLbl_Get  (RxnView_t *, U8_t);
  void            RxnV_IthM_NameLbl_Put  (RxnView_t *, U8_t, Widget);
  Widget          RxnV_IthM_Sign_Get     (RxnView_t *, U8_t);
  void            RxnV_IthM_Sign_Put     (RxnView_t *, U8_t, Widget);
  Widget          RxnView_LibLbl_Get     (RxnView_t *);
  void            RxnView_LibLbl_Put     (RxnView_t *, Widget);
  Widget          RxnView_LibVal_Get     (RxnView_t *);
  void            RxnView_LibVal_Put     (RxnView_t *, Widget);
  Dimension       RxnView_MolDAH_Get     (RxnView_t *);
  void            RxnView_MolDAH_Put     (RxnView_t *, Dimension);
  Widget          RxnView_MolForm_Get    (RxnView_t *);
  void            RxnView_MolForm_Put    (RxnView_t *, Widget);
  Dimension       RxnView_MolFormW_Get   (RxnView_t *);
  void            RxnView_MolFormW_Put   (RxnView_t *, Dimension);
  MolView_t      *RxnView_Mols_Get       (RxnView_t *);
  Widget          RxnView_NameLbl_Get    (RxnView_t *);
  void            RxnView_NameLbl_Put    (RxnView_t *, Widget);
  U8_t            RxnView_NumSGs_Get     (RxnView_t *);
  void            RxnView_NumSGs_Put     (RxnView_t *, U8_t);
  Widget          RxnView_SchemaLbl_Get  (RxnView_t *);
  void            RxnView_SchemaLbl_Put  (RxnView_t *, Widget);
  Widget          RxnView_SchemaVal_Get  (RxnView_t *);
  void            RxnView_SchemaVal_Put  (RxnView_t *, Widget);
  Widget          RxnView_SGMeritLbl_Get (RxnView_t *);
  void            RxnView_SGMeritLbl_Put (RxnView_t *, Widget);
  Widget          RxnView_SGMeritVal_Get (RxnView_t *);
  void            RxnView_SGMeritVal_Put (RxnView_t *, Widget);
  Widget          RxnView_SolvedLbl_Get  (RxnView_t *);
  void            RxnView_SolvedLbl_Put  (RxnView_t *, Widget);
  Widget          RxnView_YieldLbl_Get   (RxnView_t *);
  void            RxnView_YieldLbl_Put   (RxnView_t *, Widget);
  Widget          RxnView_YieldVal_Get   (RxnView_t *);
  void            RxnView_YieldVal_Put   (RxnView_t *, Widget);
*/

#define RxnView_ChapLbl_Get(rvp)\
  (rvp)->ch_lbl
#define RxnView_ChapLbl_Put(rvp, value)\
  (rvp)->ch_lbl = (value)
#define RxnView_ChapVal_Get(rvp)\
  (rvp)->ch_val
#define RxnView_ChapVal_Put(rvp, value)\
  (rvp)->ch_val = (value)
#define RxnView_ConfLbl_Get(rvp)\
  (rvp)->conf_lbl
#define RxnView_ConfLbl_Put(rvp, value)\
  (rvp)->conf_lbl = (value)
#define RxnView_ConfVal_Get(rvp)\
  (rvp)->conf_val
#define RxnView_ConfVal_Put(rvp, value)\
  (rvp)->conf_val = (value)
#define RxnView_CurSG_Get(rvp)\
  (rvp)->cursg_p
#define RxnView_CurSG_Put(rvp, value)\
  (rvp)->cursg_p = (value)
#define RxnView_EaseLbl_Get(rvp)\
  (rvp)->ease_lbl
#define RxnView_EaseLbl_Put(rvp, value)\
  (rvp)->ease_lbl = (value)
#define RxnView_EaseVal_Get(rvp)\
  (rvp)->ease_val
#define RxnView_EaseVal_Put(rvp, value)\
  (rvp)->ease_val = (value)
#define RxnView_Form_Get(rvp)\
  (rvp)->form
#define RxnView_Form_Put(rvp, value)\
  (rvp)->form = (value)
#define RxnView_Frame_Get(rvp)\
  (rvp)->frame
#define RxnView_Frame_Put(rvp, value)\
  (rvp)->frame = (value)
#define RxnView_IthMol_Get(rvp, ith)\
  (rvp)->mols[ith]
#define RxnView_IthMol_Put(rvp, ith, value)\
  (rvp)->mols[ith] = (value)

#define RxnV_IthM_DA_Get(rvp, ith)\
  MolView_DA_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_DA_Put(rvp, ith, value)\
  MolView_DA_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_DAW_Get(rvp, ith)\
  MolView_DAW_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_DAW_Put(rvp, ith, value)\
  MolView_DAW_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_Form_Get(rvp, ith)\
  MolView_Form_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_Form_Put(rvp, ith, value)\
  MolView_Form_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_LblsH_Get(rvp, ith)\
  MolView_LblsH_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_LblsH_Put(rvp, ith, value)\
  MolView_LblsH_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_LblW_Get(rvp, ith)\
  MolView_LblW_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_LblW_Put(rvp, ith, value)\
  MolView_LblW_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_Mol_Get(rvp, ith)\
  MolView_Mol_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_Mol_Put(rvp, ith, value)\
  MolView_Mol_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_MrtLbl_Get(rvp, ith)\
  MolView_MrtLbl_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_MrtLbl_Put(rvp, ith, value)\
  MolView_MrtLbl_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_NameLbl_Get(rvp, ith)\
  MolView_NameLbl_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_NameLbl_Put(rvp, ith, value)\
  MolView_NameLbl_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnV_IthM_Sign_Get(rvp, ith)\
  MolView_Sign_Get (RxnView_IthMol_Get (rvp, ith))

#define RxnV_IthM_Sign_Put(rvp, ith, value)\
  MolView_Sign_Put (RxnView_IthMol_Get (rvp, ith), (value))

#define RxnView_LibLbl_Get(rvp)\
  (rvp)->lib_lbl
#define RxnView_LibLbl_Put(rvp, value)\
  (rvp)->lib_lbl = (value)
#define RxnView_LibVal_Get(rvp)\
  (rvp)->lib_val
#define RxnView_LibVal_Put(rvp, value)\
  (rvp)->lib_val = (value)
#define RxnView_MolDAH_Get(rvp)\
  (rvp)->molda_h
#define RxnView_MolDAH_Put(rvp, value)\
  (rvp)->molda_h = (value)
#define RxnView_MolForm_Get(rvp)\
  (rvp)->mol_form
#define RxnView_MolForm_Put(rvp, value)\
  (rvp)->mol_form = (value)
#define RxnView_MolFormW_Get(rvp)\
  (rvp)->molform_w
#define RxnView_MolFormW_Put(rvp, value)\
  (rvp)->molform_w = (value)
#define RxnView_Mols_Get(rvp)\
  (rvp)->mols
#define RxnView_NameLbl_Get(rvp)\
  (rvp)->name_lbl
#define RxnView_NameLbl_Put(rvp, value)\
  (rvp)->name_lbl = (value)
#define RxnView_NumSGs_Get(rvp)\
  (rvp)->num_sgs
#define RxnView_NumSGs_Put(rvp, value)\
  (rvp)->num_sgs = (value)
#define RxnView_SchemaLbl_Get(rvp)\
  (rvp)->schema_lbl
#define RxnView_SchemaLbl_Put(rvp, value)\
  (rvp)->schema_lbl = (value)
#define RxnView_SchemaVal_Get(rvp)\
  (rvp)->schema_val
#define RxnView_SchemaVal_Put(rvp, value)\
  (rvp)->schema_val = (value)
#define RxnView_SGMeritLbl_Get(rvp)\
  (rvp)->sgmrt_lbl
#define RxnView_SGMeritLbl_Put(rvp, value)\
  (rvp)->sgmrt_lbl = (value)
#define RxnView_SGMeritVal_Get(rvp)\
  (rvp)->sgmrt_val
#define RxnView_SGMeritVal_Put(rvp, value)\
  (rvp)->sgmrt_val = (value)
#define RxnView_SolvedLbl_Get(rvp)\
  (rvp)->solved_lbl
#define RxnView_SolvedLbl_Put(rvp, value)\
  (rvp)->solved_lbl = (value)
#define RxnView_YieldLbl_Get(rvp)\
  (rvp)->yield_lbl
#define RxnView_YieldLbl_Put(rvp, value)\
  (rvp)->yield_lbl = (value)
#define RxnView_YieldVal_Get(rvp)\
  (rvp)->yield_val
#define RxnView_YieldVal_Put(rvp, value)\
  (rvp)->yield_val = (value)

/*** Macros ***/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

char **RxnView_ChapNames_Grab (void);
Widget RxnView_Create         (Widget, RxnView_t *);
void   RxnView_Destroy        (RxnView_t *, Display *);
void   RxnView_Mols_Scale     (RxnView_t *);
void   RxnView_Update         (RxnView_t *, Subgoal_t *);

#endif
/*  End of RXN_VIEW.H  */
