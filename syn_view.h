#ifndef _H_SYN_VIEW_
#define _H_SYN_VIEW_  1
/******************************************************************************
*
*  Copyright (C) 1995 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     SYN_VIEW.H
*
*    This is a header contains global declarations used by the GUI for
*    the SYNCHEM expert system. 
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

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SSHOT_VIEW_
#include "sshot_view.h"
#endif

#ifndef _H_SUBMIT_
#include "submit.h"
#endif

/*** Literal Values ***/


/*** Data Structures ***/


typedef struct syn_view_s
  {
  Widget         cycle_dg;              /* Cycle count dialog widget */
  Widget         legend;                /* Pstview symbol legend */ 
  Widget         top_app;               /* Top level application widget */
  Widget         top_form;              /* Application form widget */
  PstView_t      pstview;               /* Problem solving tree viewer */ 
  SShotView_t    sshotview;             /* Singleshot viewer */ 
  SubmitCB_t     job_submit;            /* Job submission module */
  XtAppContext   acon;                  /* Application context */
  U32_t          flags;                 /* Flags to control application. */
  }  SynViewCB_t;

/** Field Access Macros for SynViewCB_t **/

/* Macro Prototypes

  XtAppContext  *SynVCB_AppCon_Get    (SynViewCB_t *);
  Widget         SynVCB_CycleDg_Get   (SynViewCB_t *);
  void           SynVCB_CycleDg_Put   (SynViewCB_t *, Widget);
  Widget         SynVCB_FileDg_Get    (SynViewCB_t *);
  void           SynVCB_FileDg_Put    (SynViewCB_t *, Widget);
  U32_t          SynVCB_Flags_Get     (SynViewCB_t *);
  void           SynVCB_Flags_Put     (SynViewCB_t *, U32_t);
  Boolean_t      SynVCB_FlagExit_Get  (SynViewCB_t *);
  void           SynVCB_FlagExit_Put  (SynViewCB_t *, Boolean_t);
  Boolean_t      SynVCB_FlagParallel_Get (SynViewCB_t *);
  void           SynVCB_FlagParallel_Put (SynViewCB_t *, Boolean_t);
  SubmitCB_t    *SynVCB_JobSubmit_Get (SynViewCB_t *);
  Widget         SynVCB_Legend_Get    (SynViewCB_t *);
  PstView_t     *SynVCB_PstView_Get   (SynViewCB_t *);
  SShotView_t   *SynVCB_SShotView_Get (SynViewCB_t *);
  Widget         SynVCB_TopApp_Get    (SynViewCB_t *);
  void           SynVCB_TopApp_Put    (SynViewCB_t *, Widget);
  Widget         SynVCB_TopForm_Get   (SynViewCB_t *);
  void           SynVCB_TopForm_Put   (SynViewCB_t *, Widget);
*/

#define SynVCBM_Exit            0x1
#define SynVCBM_Parallel        0x10

#define SynVCB_AppCon_Get(svp)\
  &(svp)->acon
#define SynVCB_CycleDg_Get(svp)\
  (svp)->cycle_dg
#define SynVCB_CycleDg_Put(svp, value)\
  (svp)->cycle_dg = (value)
#define SynVCB_Flags_Get(svp)\
  (svp)->flags
#define SynVCB_Flags_Put(svp, value)\
  (svp)->flags = (value)

#define SynVCB_FlagExit_Get(svp)\
  ((svp)->flags & SynVCBM_Exit ? TRUE : FALSE)
#define SynVCB_FlagExit_Put(svp, value)\
  { if ((value) == TRUE)\
    (svp)->flags |= SynVCBM_Exit;\
  else\
    (svp)->flags &= ~SynVCBM_Exit; }

#define SynVCB_FlagParallel_Get(svp)\
  ((svp)->flags & SynVCBM_Parallel ? TRUE : FALSE)
#define SynVCB_FlagParallel_Put(svp, value)\
  { if ((value) == TRUE)\
    (svp)->flags |= SynVCBM_Parallel;\
  else\
    (svp)->flags &= ~SynVCBM_Parallel; }

#define SynVCB_JobSubmit_Get(svp)\
  &((svp)->job_submit)
#define SynVCB_Legend_Get(svp)\
  (svp)->legend
#define SynVCB_PstView_Get(svp)\
  &((svp)->pstview)
#define SynVCB_SShotView_Get(svp)\
  &((svp)->sshotview)
#define SynVCB_TopApp_Get(svp)\
  (svp)->top_app
#define SynVCB_TopApp_Put(svp, value)\
  (svp)->top_app = (value)
#define SynVCB_TopForm_Get(svp)\
  (svp)->top_form
#define SynVCB_TopForm_Put(svp, value)\
  (svp)->top_form = (value)

/*** Macros ***/

/*** Routine Prototypes ***/

PstView_t      *PstView_Handle_Grab   (void);
SShotView_t    *SShotView_Handle_Grab (void);
SubmitCB_t     *Submit_Handle_Grab    (void);
SynViewCB_t    *SynView_Handle_Grab   (void);
void            SynView_Exit          (void);

#endif
/*  End of SYN_VIEW.H  */
