/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     PST_VIEW.C
*
*    This module defines the routines needed to view SYNCHEM's problem 
*    solving tree (graph). 
*
*  Creation Date:
*
*    10-Jun-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    
*    PathView_Preview_EH
*    PstView_Compound_Sel
*    PstView_Create
*    PstView_Destroy
*    PstView_Display
*    PstView_Focus_Draw
*    PstView_Focus_Undraw
*    PstView_Mark_Store
*    PstView_Mouse_Remove
*    PstView_Mouse_Reset
*    PstView_PostInit
*    PstView_Preview_EH
*    PstView_Reset
*    PstView_Trace_Store
*    PstView_Tree_Init
*    PstView_Unmark_Store
*    PstView_Visit_Store
*    PstVLvls_SetAll
*    PstVLvls_SetTwo
*    PstVLvls_Update
*    TreeLvl_Create
*    TreeLvl_Destroy
*    TreeLvls_Backup
*    
*
*    SCycle_Detect
*    SHead_Center
*    SLines_Draw
*    SPathSel_Draw
*    SPathLvlNums_Draw
*    SSelLine_Draw
*    SSelSym_Draw
*    SSelSym_Undraw
*    SSymbols_Draw
*    STopLvlBars_Draw
*    SUnmark_Downward
*
*    SPstView_BarLeftSet_CB
*    SPstView_PstExp_CB
*    SPstView_PstSel_CB
*    SPstView_PthExp_CB
*    SPstView_PthSel_CB
*    SPstView_Resize_CB
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 07-Dec-00  Miller     On-the-fly marking of dead-end paths due to "solution"
*                       exclusively via circularity
* 21-Dec-00  Miller     Allowed either button to select from pst path, with
*                       button 3 (right) toggling marks at the same time
* 21-Feb-01  Miller     Allowed button 2 to select from pst view, propagating
*                       marks upward to the target or unmarking node and all
*                       marked descendants, depending on the current mark state;
*                       also placed recursion limit on predictive circularity
* 19-Apr-01  Miller     Corrected uses of PstView_Display where PstView_Mouse_Remove
*                       had not first been called.  This is a very error-prone way
*                       of managing callbacks, with the potential for duplicate
*                       callback installations and PST havoc, and a rewrite that
*                       manages it through a bookkeeping method would be highly
*                       desirable!
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>

#include <Xm/DrawingA.h>
#include <Xm/Label.h>
#include <Xm/Form.h>
#include <Xm/Frame.h>
#include <Xm/PushB.h>
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

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif

#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif

#ifndef _H_INFO_VIEW_
#include "info_view.h"
#endif

#ifndef _H_MOL_VIEW_
#include "mol_view.h"
#endif

#ifndef _H_RXN_VIEW_
#include "rxn_view.h"
#endif

#ifndef _H_RXN_PREVIEW_
#include "rxn_preview.h"
#endif

#ifndef _H_SEL_TRACE_
#include "sel_trace.h"
#endif

#ifndef _H_PST_VIEW_
#include "pst_view.h"
#endif

#ifndef _H_SYN_VIEW_
#include "syn_view.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

/* Static Variables */

/* Static Routine Prototypes */

static Boolean_t SCycle_Detect  (PstView_t *, U32_t, U8_t, Boolean_t);
static void   SHead_Center      (PstView_t *, U8_t, U8_t);
static void   SLines_Draw       (PsvCmpInfo_t *, PsvViewLvl_t *, 
  PsvViewLvl_t *);
static void   SPathSel_Draw     (PstView_t *);
static void   SPathLvlNums_Draw (PsvViewLvl_t *);
static void   SSelLine_Draw     (PsvCmpInfo_t *, PsvViewLvl_t *, 
  PsvViewLvl_t *);
static void   SSelSym_Draw      (PsvViewLvl_t *);
static void   SSelSym_Undraw    (PsvViewLvl_t *);
static void   SSymbols_Draw     (PstView_t *, PsvCmpInfo_t *, PsvViewLvl_t *, Boolean_t);
static void   STopLvlBars_Draw  (PsvViewLvl_t *);
static void   SUnmark_Downward  (PsvCmpInfo_t *, Compound_t *);
static Boolean_t     PstView_AllSolCirc   (PstView_t *, Compound_t *, U8_t, Boolean_t);
static Boolean_t     PstView_CompSolNC    (Compound_t *, U32_t *, int);
static Boolean_t     PstView_SgSolNC      (Subgoal_t *, U32_t *, int);

/* Callback Routine Prototypes */

static void   SPstView_BarLeftSet_CB (Widget, XtPointer, XtPointer);
static void   SPstView_PstExp_CB     (Widget, XtPointer, XtPointer);
static void   SPstView_PstSel_CB     (Widget, XtPointer, XtPointer);
static void   SPstView_PthExp_CB     (Widget, XtPointer, XtPointer);
static void   SPstView_PthSel_CB     (Widget, XtPointer, XtPointer);
static void   SPstView_Resize_CB     (Widget, XtPointer, XtPointer);

/* Global array and counter to compensate for overflow of path display,
   which screws up backward propagation of marks */

static Subgoal_t **path_overflow = NULL;
static int num_overflowed = 0, overflow_level = 0;

/* Global flag to avoid adding a new parameter to PstView_Display */

static Boolean_t pvdisp_reset_mouse = TRUE;

/* Global flag to avoid repeating warning during a given call to PstView_Display */

static Boolean_t warning_shown = FALSE;

/* Another global flag: set when calling AvlEdit; reset from SPstView_PstExp_CB ... */

static Boolean_t avledit_called = FALSE;

/* ... and the function that sets it!!! */

void PstView_Set_Avledit_Called ()
{
  avledit_called = TRUE;
}


/****************************************************************************
*
*  Function Name:                 PathView_Preview_EH
*
*    This routine handles the mouse movement events for the path
*    drawing area.  When the pointer is moved into the area of a 
*    symbol, a focus box is drawn around it, and if appropriate, the
*    reaction preview is updated.  
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PathView_Preview_EH
  (
  Widget         w,                       /* Widget */
  XtPointer      client_data,             /* PstView */
  XEvent        *event,                   /* Event  */ 
  Boolean       *cont                     /* Continue to dispatch? */
  )
{
  PstView_t     *pv_p;
  PsvViewLvl_t  *vlvl_p;                  /* Path view level */
  PsvTreeLvl_t  *tlvl_p;                  /* Path tree level */ 
  RxnPreView_t  *rpv_p;
  XMotionEvent  *mevent;
  U16_t          curnode;

  pv_p = (PstView_t *) client_data;
  if (pv_p == NULL)
    return;

  vlvl_p = PstView_PathVLvl_Get (pv_p);
  /*  Only process motion events with no state.  */
  if (event->type == MotionNotify)
    {
    mevent = &event->xmotion;
    if (mevent->state == 0 && mevent->y >= ViewLvl_SymYBeg_Get (vlvl_p) 
       && mevent->y <= ViewLvl_SymYEnd_Get (vlvl_p))
      {
      /*  If the pointer is sitting in a valid node then redraw the focus 
          box and update the path previewer.
      */
      if ((mevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
            % AppDim_PstSymTotWd_Get (&GAppDim) 
            < AppDim_PstSymDim_Get (&GAppDim))
        {
        curnode = (U16_t)((mevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
          / AppDim_PstSymTotWd_Get (&GAppDim));
        rpv_p = ViewLvl_RxnPV_Get (vlvl_p);
        tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
        if (curnode < TreeLvl_NCmps_Get (tlvl_p)
           && curnode != TreeLvl_FocusNd_Get (tlvl_p))
          {
          React_Record_t *rxn_rec_p;
          U32_t           cursg;
          PsvTreeLvl_t   *psttlvl_p;

          if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
            {
            psttlvl_p = PstView_IthPTLvl_Get (pv_p, PstView_IthPthLN_Get (pv_p,
              TreeLvl_FocusNd_Get (tlvl_p)));
            if (TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p)) ==
                TreeLvl_IthCmpNd_Get (psttlvl_p, TreeLvl_SelNd_Get (psttlvl_p)))
              TreeLvl_SelNd_Put (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p));

            PstView_Focus_Undraw  (vlvl_p);
            }

          TreeLvl_FocusNd_Put (tlvl_p, curnode);
          psttlvl_p = PstView_IthPTLvl_Get (pv_p, PstView_IthPthLN_Get (pv_p,
            TreeLvl_FocusNd_Get (tlvl_p)));
          if (TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p)) ==
              TreeLvl_IthCmpNd_Get (psttlvl_p, TreeLvl_SelNd_Get (psttlvl_p)))
            TreeLvl_SelNd_Put (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p));

          PstView_Focus_Draw  (vlvl_p);
          TreeLvl_SelNd_Put (tlvl_p, PVW_NODE_NONE);

          /*  If this focused compound belongs to same subgoal, then only
              the compound molecule needs to be changed.  Otherwise,
              we need to update the reaction name, compound numbers,
              and subgoal merit as well.
          */
          cursg = PstSubg_Index_Get (PstComp_Father_Get 
            (TreeLvl_IthCmpNd_Get (tlvl_p, curnode)));
          if (cursg != TreeLvl_FocusSG_Get (tlvl_p))
            {
            TreeLvl_FocusSG_Put (tlvl_p, cursg);
            if (curnode == 0)
              {
              if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
                String_Destroy (RxnPreV_RxnName_Get (rpv_p));
              RxnPreV_RxnName_Put (rpv_p, String_Create (RPV_LABEL_TARGET, 0));
              RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, curnode), 
                TRUE);
              }
            else
              {
              rxn_rec_p = React_Schema_Handle_Get (
                PstSubg_Reaction_Schema_Get (PstComp_Father_Get (
                TreeLvl_IthCmpNd_Get (tlvl_p, curnode))));
              if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
                String_Destroy (RxnPreV_RxnName_Get (rpv_p));
              RxnPreV_RxnName_Put (rpv_p, String_Copy (React_TxtRec_Name_Get 
                (React_Text_Get (rxn_rec_p))));

              /* Quirk of String_Copy:  doesn't null terminate string!! */
              *(String_Value_Get (RxnPreV_RxnName_Get (rpv_p))
                + String_Length_Get (RxnPreV_RxnName_Get (rpv_p))) 
               = (U8_t) '\0';
              RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, curnode), 
                TRUE);
              }
            }
          else
            {
            RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, curnode), 
              FALSE);
            }
          }
        }
      }  /* End of if empty state */
    }  /* End of if motion event */

  return ;
}
/*  End of PathView_Preview_EH  */


/****************************************************************************
*
*  Function Name:                 PstView_Compound_Sel
*
*    This routine uses the XmTrackingEvent procedure to grab the pointer
*    and allow the user to select a compound in the pst view area.  The  
*    selected compound is stored in the PstView_t data structure so that  
*    the appropriate action can be performed after this function returns. 
*
*    Warning!  The tracking is done directly on the drawing area of the
*    pst viewing area, not on the clip window.  As such, the tracker is
*    under the illusion that the drawing area extends past the right edge
*    of the clip window.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Compound_Sel
  (
  PstView_t    *pv_p
  )
{
  PsvTreeLvl_t *tlvl_p;
  PsvViewLvl_t *vlvl_p;
  XEvent        event;
  Widget        w;
  int           rem;
  U16_t         curnode;
  U8_t          curlvl;

  /*  Try using Motif's tracking event procedure.  */
  XBell (Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR)), 25);
  w = XmTrackingEvent (DrawCxt_DA_Get (PstView_PstDC_Get (pv_p)), 
    SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_RXNPVW), True, &event);

  /*  Only process left button releases.  */
  if (w == DrawCxt_DA_Get (PstView_PstDC_Get (pv_p)) 
      && event.type == ButtonRelease && event.xbutton.button == Button1)
    {
    /*  Determine which level and node.  */
    rem = (event.xbutton.y % AppDim_PstLvlTotHt_Get (&GAppDim)) 
      + AppDim_PstLvlBdrOff_Get (&GAppDim);
    if (rem >= AppDim_PstLvlBdrTop_Get (&GAppDim) 
        && rem < AppDim_PstLvlBdrBtm_Get (&GAppDim))
      curlvl = (U8_t) (event.xbutton.y / AppDim_PstLvlTotHt_Get (&GAppDim));
    else
      curlvl = PVW_LEVEL_NONE;

    if ((event.xbutton.x - AppDim_PstSymSepWd_Get (&GAppDim)) 
         % AppDim_PstSymTotWd_Get (&GAppDim) 
         < AppDim_PstSymDim_Get (&GAppDim))
      curnode = (U16_t)((event.xbutton.x - AppDim_PstSymSepWd_Get (&GAppDim)) 
        / AppDim_PstSymTotWd_Get (&GAppDim));
    else
      curnode = PVW_NODE_NONE;
  
    if (curlvl != PVW_LEVEL_NONE && curnode != PVW_NODE_NONE)
      {
      vlvl_p = PstView_IthVLvl_Get (pv_p, curlvl);
      tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
      if (curnode >= TreeLvl_HeadNd_Get (tlvl_p) 
         && curnode < TreeLvl_HeadNd_Get (tlvl_p) 
           + TreeLvl_NCmps_Get (tlvl_p))
        {
        curnode -= TreeLvl_HeadNd_Get (tlvl_p);
        PstView_LastSelCmp_Put (pv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 
          curnode));
        return ;
        }
      }  
    }  /* End of if left button released */

  PstView_LastSelCmp_Put (pv_p, NULL);
  return ;
}
/*  End of PstView_Compound_Sel  */


/****************************************************************************
*
*  Function Name:                 PstView_Create
*
*    This routine initializes the data structures specific to displaying
*    the pst that can be or need to be done before the widgets are created.
*    Then it creates the widgets.
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Main menu widget.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Widget PstView_Create
  (
  Widget         top_level,
  Widget         parent,
  PstView_t     *pv_p
  )
{
  Widget         clipw;
  Widget         scrollbar;
  PsvTreeLvl_t **pthtlvls;                /* Temp tree lvl ptr list */
  DrawCxt_t     *pathdc_p;                /* Path drawing context */ 
  DrawCxt_t     *pstdc_p;                 /* Pst tree drawing context */
  PsvViewLvl_t  *pathvl_p;                /* Path view level for pst */
  PsvViewLvl_t  *vlvl_p;                  /* View level of pst */ 
  Widget         form;                    /* Parent form widget */
  XmString       path_str;                /* Path label string */
  XmString       nsgs_str;                /* Num of subgoals label */
  XmString       lvl_str;                 /* Level label string */
  Dimension      p_h, n_h, l_h;           /* String heights */
  Dimension      p_w, n_w, l_w;           /* String widths */
  Dimension      pst_minh, pth_minh;      /* Minumum height of pst and path */
  Dimension      pst_minw;                /* Minumum width of pst */
  Dimension      max_w;                   /* Maximum label width */
  Dimension      offset;                  /* Offset for label */
  int            y_pos;                   /* Y position in drawing area */
  U8_t           lvl_i;                   /* Ith pst view level */

  /*  Initialize those data structures for which the values are known
      or can be calculated before the widgets are created.  First initialize
      the drawing contexts for the view areas.
  */
  pathdc_p = PstView_PathDC_Get (pv_p);
  pstdc_p = PstView_PstDC_Get (pv_p);

  DrawCxt_BgPxl_Put (pathdc_p, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_PTHDABG));
  DrawCxt_BgPxl_Put (pstdc_p, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_PSTDABG));

  pst_minh = PVW_LEVEL_NUMOF * AppDim_PstSymDim_Get (&GAppDim)
    + (PVW_LEVEL_NUMOF - 1) * AppDim_PstLinesHt_Get (&GAppDim)  
    + PVW_LEVEL_NUMOF  * AppDim_PstBarHt_Get (&GAppDim) 
    + 2 * AppDim_PstMargHt_Get (&GAppDim) 
    + 2 * PVW_LEVEL_NUMOF * AppDim_PstSymSepHt_Get (&GAppDim)
    + AppDim_PstCycHt_Get (&GAppDim);

  pst_minw = AppDim_PstSymTotWd_Get (&GAppDim) * PVW_PST_NUMSYMS_INIT
    + AppDim_PstSymSepWd_Get (&GAppDim);

  pth_minh = AppDim_PstSymDim_Get (&GAppDim) + AppDim_PthBarHt_Get (&GAppDim)
    + AppDim_PthCycHt_Get (&GAppDim) + AppDim_PthLNumHt_Get (&GAppDim)
    + 5 * AppDim_PthMargHt_Get (&GAppDim);

  DrawCxt_DAH_Put (pathdc_p, pth_minh);
  DrawCxt_DAH_Put (pstdc_p, pst_minh);
  DrawCxt_DAW_Put (pathdc_p, 0);
  DrawCxt_DAW_Put (pstdc_p, pst_minw);
  DrawCxt_PmapW_Put (pathdc_p, 0);
  DrawCxt_PmapW_Put (pstdc_p, pst_minw);

  /*  Initialize the view levels for the pst.  
  */
  y_pos =  AppDim_PstMargHt_Get (&GAppDim);
  for (lvl_i = 0; lvl_i < PVW_LEVEL_NUMOF; lvl_i++)
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, lvl_i);
    ViewLvl_TreeLvl_Put (vlvl_p, TreeLvl_Create (NULL, 0, lvl_i, 
      PVW_TREETYPE_NONE));
    ViewLvl_DrawCxt_Put (vlvl_p, pstdc_p);
    ViewLvl_LvlNum_Put (vlvl_p, lvl_i);
    ViewLvl_SelClr_Put (vlvl_p, SynAppR_IthClrPx_Get (&GSynAppR, 
      SAR_CLRI_BLACK));

    /* Layout vertical postioning of symbols, lines and bars in drawing
       areas.
    */
    if (lvl_i == PVW_LEVEL_TOP)
      {
      ViewLvl_LnYBeg_Put (vlvl_p, 0);
      ViewLvl_LnYEnd_Put (vlvl_p, 0);
      }
    else
      {
      ViewLvl_LnYBeg_Put (vlvl_p, y_pos);       
      y_pos += AppDim_PstLinesHt_Get (&GAppDim);
      ViewLvl_LnYEnd_Put (vlvl_p, y_pos - 1);
      }

    ViewLvl_BarYBeg_Put (vlvl_p, y_pos);      
    y_pos += AppDim_PstBarHt_Get (&GAppDim);
    ViewLvl_BarYEnd_Put (vlvl_p, y_pos - 1);  
    y_pos += AppDim_PstSymSepHt_Get (&GAppDim);
    ViewLvl_SymYBeg_Put (vlvl_p, y_pos);      
    y_pos += AppDim_PstSymDim_Get (&GAppDim);
    ViewLvl_SymYEnd_Put (vlvl_p, y_pos - 1);  
    y_pos += AppDim_PstSymSepHt_Get (&GAppDim);

    /* If this a level with a previewer, create it.  */
    if (lvl_i == PVW_LEVEL_TOP)
      {
      RxnPreV_Which_Put (ViewLvl_RxnPV_Get (vlvl_p), RPV_PREVIEW_TOP);
      ViewLvl_FocusClr_Put (vlvl_p, SynAppR_IthClrPx_Get (&GSynAppR, 
        SAR_CLRI_FOCUSTOP));
      RxnPreV_Create (top_level, ViewLvl_RxnPV_Get (vlvl_p),
        ViewLvl_FocusClr_Get (vlvl_p));
      }
    else if (lvl_i == PVW_LEVEL_MID)
      {
      RxnPreV_Which_Put (ViewLvl_RxnPV_Get (vlvl_p), RPV_PREVIEW_MID);
      ViewLvl_FocusClr_Put (vlvl_p, SynAppR_IthClrPx_Get (&GSynAppR, 
        SAR_CLRI_FOCUSMID));
      RxnPreV_Create (top_level, ViewLvl_RxnPV_Get (vlvl_p),
        ViewLvl_FocusClr_Get (vlvl_p));
      }
    else if (lvl_i == PVW_LEVEL_BTM)
      {
      RxnPreV_Which_Put (ViewLvl_RxnPV_Get (vlvl_p), RPV_PREVIEW_BTM);
      ViewLvl_FocusClr_Put (vlvl_p, SynAppR_IthClrPx_Get (&GSynAppR, 
        SAR_CLRI_FOCUSBTM));
      RxnPreV_Create (top_level, ViewLvl_RxnPV_Get (vlvl_p),
        ViewLvl_FocusClr_Get (vlvl_p));
      }
    }

  /*  Create the  path tree level and initialize the view level.  */
  pathvl_p = PstView_PathVLvl_Get (pv_p);
  ViewLvl_TreeLvl_Put (pathvl_p, TreeLvl_Create (NULL, 
    PVW_PATH_NUMSYMS_INIT, PVW_LEVEL_PATH, PVW_TREETYPE_NONE));
  TreeLvl_NCmps_Put (ViewLvl_TreeLvl_Get (pathvl_p) , 0);
  ViewLvl_DrawCxt_Put (pathvl_p, pathdc_p);
  ViewLvl_SelClr_Put (pathvl_p, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_BLUE));
  ViewLvl_FocusClr_Put (pathvl_p, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_FOCUSPTH));
  ViewLvl_LvlNum_Put (pathvl_p, PVW_LEVEL_PATH);
  RxnPreV_Which_Put (ViewLvl_RxnPV_Get (pathvl_p), RPV_PREVIEW_PATH);
  RxnPreV_Create (top_level, ViewLvl_RxnPV_Get (pathvl_p),
    ViewLvl_FocusClr_Get (pathvl_p));

  /* Layout vertical postioning of symbols, lines and bars in drawing
     areas.
  */
  y_pos =  AppDim_PthMargHt_Get (&GAppDim);
  ViewLvl_LnYBeg_Put (pathvl_p, y_pos);      
    y_pos += AppDim_PthLNumHt_Get (&GAppDim);
  ViewLvl_LnYEnd_Put (pathvl_p, y_pos - 1);  
    y_pos += AppDim_PthMargHt_Get (&GAppDim);
  ViewLvl_BarYBeg_Put (pathvl_p, y_pos);     
    y_pos += AppDim_PthBarHt_Get (&GAppDim);
  ViewLvl_BarYEnd_Put (pathvl_p, y_pos - 1); 
    y_pos += AppDim_PthMargHt_Get (&GAppDim);
  ViewLvl_SymYBeg_Put (pathvl_p, y_pos);     
    y_pos += AppDim_PstSymDim_Get (&GAppDim);
  ViewLvl_SymYEnd_Put (pathvl_p, y_pos - 1); 
    y_pos += AppDim_PthMargHt_Get (&GAppDim);


  /* Finish initializing the main view data structure. */
  pthtlvls = (PsvTreeLvl_t **) malloc (PVW_PATH_NUMSYMS_INIT 
    * sizeof (PsvTreeLvl_t *));
  if (pthtlvls == NULL)
    {
    fprintf (stderr, 
      "PstView_Create:  unable to allocate memory for tree level ptr array.\n");
    exit (-1);
    }

  PstView_PthTLvls_Put (pv_p, pthtlvls);

  for (lvl_i = 0; lvl_i < PVW_PATH_NUMSYMS_INIT; lvl_i++)
    PstView_IthPTLvl_Put (pv_p, lvl_i, NULL);

  PstView_PstCB_Put (pv_p, NULL);
  PstView_CmpInfo_Put (pv_p, NULL);
  PstView_PstEHAct_Put (pv_p, FALSE);
  PstView_PthEHAct_Put (pv_p, FALSE);
  PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE);
  PstView_LastSelCmp_Put (pv_p, NULL);
  PstView_LeftNd_Put (pv_p, 0);
  PstView_NumActPV_Put (pv_p, 0);
  PstView_NumCmpI_Put (pv_p, 0);
  PstView_TreeType_Put (pv_p, PVW_TREETYPE_FULL);

  /*  Find the widest label and use it as the width of the bboard.  */

  path_str = XmStringCreateLtoR (PVW_LBL_PATH, AppDim_RxnLblTag_Get (&GAppDim));
  nsgs_str = XmStringCreateLtoR ("000", AppDim_PstNSGTag_Get (&GAppDim));
  lvl_str = XmStringCreateLtoR ("000", AppDim_PstLblTag_Get (&GAppDim));

  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), path_str, &p_w, &p_h);
  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), nsgs_str, &n_w, &n_h);
  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), lvl_str, &l_w, &l_h);
  max_w = (p_w > n_w) ? p_w : n_w;  max_w = (max_w > l_w) ? max_w : l_w;
  max_w += 2 * AppDim_PstLblMarg_Get (&GAppDim);
  XmStringFree (nsgs_str);
  XmStringFree (lvl_str);



  /*  Create the encapsulating frame and form first.  Then create the
      label bulletin board, path drawing area, scrolled window and
      tree drawing area widgets.  Because the PST tree viewing area
      should have a fixed height (but resizable width), the bulletin board
      that contains the labels is given a height that approximates the
      height of the path drawing area and scrolled window.  The height of 
      the path drawing area is known and fixed, but the height of the 
      scrolled window is estimated.  The width of the path drawing area 
      is dependent on the width of the  entire PST tree viewing area, 
      whereas the width of the pst drawing area is initialized to contain
      100 nodes.  
  */

  PstView_Frame_Put (pv_p, XtVaCreateWidget ("PstVFm",
    xmFrameWidgetClass,  parent,
    XmNshadowType, XmSHADOW_IN,
    XmNshadowThickness, 3,
    XmNmarginWidth, AppDim_MargFmFr_Get (&GAppDim),
    XmNmarginHeight, AppDim_MargFmFr_Get (&GAppDim),
    NULL));

  form = XmCreateForm (PstView_Frame_Get (pv_p), "PstVF", NULL, 0);
  PstView_Form_Put (pv_p, form);

  PstView_LabelBB_Put (pv_p, XtVaCreateWidget ("PstVBB", 
      xmBulletinBoardWidgetClass,  form, 
      XmNallowOverlap, False,
      XmNlabelFontList, SynAppR_FontList_Get (&GSynAppR),

      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNdefaultPosition, False,
      XmNmarginHeight, 0,
      XmNmarginWidth, 0,
      XmNnoResize, True,
      XmNresizePolicy, XmRESIZE_NONE,
      XmNshadowThickness, 0,
      XmNborderWidth, 0,
      XmNheight, pth_minh + pst_minh 
        + AppDim_XHtPvwBB_Get (&GAppDim),
      XmNwidth, max_w,
      NULL));

  DrawCxt_DA_Put (pathdc_p, XtVaCreateWidget ("PstVPDA", 
      xmDrawingAreaWidgetClass,  form, 

      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNheight, pth_minh,
      NULL));
  XtAddCallback (DrawCxt_DA_Get (pathdc_p), XmNresizeCallback, 
    SPstView_Resize_CB, (XtPointer) pv_p);
  XtAddCallback (DrawCxt_DA_Get (pathdc_p), XmNexposeCallback, 
    SPstView_PthExp_CB, (XtPointer) pathdc_p);

  PstView_ScrollWin_Put (pv_p, XtVaCreateWidget ("PstVSW", 
      xmScrolledWindowWidgetClass,  form, 
      XmNscrollingPolicy, XmAUTOMATIC,
      XmNspacing, 0,
      XmNscrolledWindowMarginHeight, 0,
      XmNscrolledWindowMarginWidth, 0,
      NULL));

  XtVaGetValues (PstView_ScrollWin_Get (pv_p),
    XmNclipWindow, &clipw,
    XmNhorizontalScrollBar, &scrollbar,
    NULL);

  PstView_ClipWin_Put (pv_p, clipw);
  PstView_ScrollBar_Put (pv_p, scrollbar);

  XtVaSetValues (scrollbar,
    XmNincrement, AppDim_PstSymTotWd_Get (&GAppDim),
    NULL);

  XtVaSetValues (clipw,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
    NULL);

  XtAddCallback (scrollbar, XmNdecrementCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNdragCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNincrementCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNpageDecrementCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNpageIncrementCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNtoBottomCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNtoTopCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);
  XtAddCallback (scrollbar, XmNvalueChangedCallback, 
    SPstView_BarLeftSet_CB, (XtPointer) pv_p);


  DrawCxt_DA_Put (pstdc_p, XtVaCreateWidget ("PstVTDA", 
      xmDrawingAreaWidgetClass,  PstView_ScrollWin_Get (pv_p), 
      XmNwidth, DrawCxt_DAW_Get (pstdc_p),
      XmNheight, DrawCxt_DAH_Get (pstdc_p),
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      NULL));
  XtAddCallback (DrawCxt_DA_Get (pstdc_p), XmNexposeCallback, 
    SPstView_PstExp_CB, (XtPointer) pv_p);

  offset = pth_minh - AppDim_PstLblMarg_Get (&GAppDim);

  /*  Create and manage the label widgets.  The level number label is
      used to label the path.
   */
  ViewLvl_TLNLbl_Put (pathvl_p, XtVaCreateManagedWidget ("PstVPathL", 
      xmLabelWidgetClass,  PstView_LabelBB_Get (pv_p), 
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNlabelType, XmSTRING,
      XmNlabelString, path_str,
      XmNalignment, XmALIGNMENT_BEGINNING,
      XmNmarginHeight, 0,
      XmNmarginWidth, AppDim_PstLblMarg_Get (&GAppDim),
      XmNrecomputeSize, False,
      XmNshadowThickness, 0,
      XmNheight, pth_minh,
      XmNwidth, max_w,
      XmNy, 0,
      XmNx, 0,
      NULL));
  XmStringFree (path_str);

  lvl_str = XmStringCreateLtoR (PVW_LBL_TLVL, AppDim_PstLblTag_Get (&GAppDim));
  ViewLvl_TLNLbl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), 
      XtVaCreateManagedWidget ("PstVLvLL", 
      xmLabelWidgetClass,  PstView_LabelBB_Get (pv_p), 
      XmNlabelType, XmSTRING,
      XmNalignment, XmALIGNMENT_END,
      XmNlabelString, lvl_str,
      XmNmarginHeight, 0,
      XmNmarginWidth, AppDim_PstLblMarg_Get (&GAppDim),
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNrecomputeSize, False,
      XmNshadowThickness, 0,
      XmNheight, (l_h > AppDim_PstSymDim_Get (&GAppDim)) 
        ? l_h : AppDim_PstSymDim_Get (&GAppDim),
      XmNwidth, max_w,
      XmNy, offset + ViewLvl_SymYBeg_Get (PstView_IthVLvl_Get (pv_p, 
        PVW_LEVEL_TOP)),
      XmNx, 0,
      NULL));
  XmStringFree (lvl_str);

  nsgs_str = XmStringCreateLtoR (PVW_LBL_TNSG, AppDim_PstNSGTag_Get (&GAppDim));
  ViewLvl_NSGLbl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID), 
      XtVaCreateManagedWidget ("PstVNSGL", 
      xmLabelWidgetClass,  PstView_LabelBB_Get (pv_p), 
      XmNlabelType, XmSTRING,
      XmNlabelString, nsgs_str,
      XmNalignment, XmALIGNMENT_CENTER,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNmarginHeight, 0,
      XmNmarginWidth, AppDim_PstLblMarg_Get (&GAppDim),
      XmNrecomputeSize, False,
      XmNshadowThickness, 0,
      XmNheight, n_h,
      XmNwidth, max_w,
      XmNy, offset + ((AppDim_PstBarHt_Get (&GAppDim) 
        + AppDim_PstLinesHt_Get (&GAppDim) - n_h) >> 1) 
        + ViewLvl_LnYBeg_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID)),
      XmNx, 0,
      NULL));
  XmStringFree (nsgs_str);

  lvl_str = XmStringCreateLtoR (PVW_LBL_MLVL, AppDim_PstLblTag_Get (&GAppDim));
  ViewLvl_TLNLbl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID), 
      XtVaCreateManagedWidget ("PstVLvLL", 
      xmLabelWidgetClass,  PstView_LabelBB_Get (pv_p), 
      XmNlabelType, XmSTRING,
      XmNlabelString, lvl_str,
      XmNalignment, XmALIGNMENT_END,
      XmNmarginHeight, 0,
      XmNmarginWidth, AppDim_PstLblMarg_Get (&GAppDim),
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNrecomputeSize, False,
      XmNshadowThickness, 0,
      XmNheight, (l_h > AppDim_PstSymDim_Get (&GAppDim)) 
        ? l_h : AppDim_PstSymDim_Get (&GAppDim),
      XmNwidth, max_w,
      XmNy, offset + ViewLvl_SymYBeg_Get (PstView_IthVLvl_Get (pv_p, 
        PVW_LEVEL_MID)),
      XmNx, 0,
      NULL));
  XmStringFree (lvl_str);

  nsgs_str = XmStringCreateLtoR (PVW_LBL_MNSG, AppDim_PstNSGTag_Get (&GAppDim));
  ViewLvl_NSGLbl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM), 
      XtVaCreateManagedWidget ("PstVNSGL", 
      xmLabelWidgetClass,  PstView_LabelBB_Get (pv_p), 
      XmNlabelType, XmSTRING,
      XmNalignment, XmALIGNMENT_CENTER,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNlabelString, nsgs_str,
      XmNmarginHeight, 0,
      XmNmarginWidth, AppDim_PstLblMarg_Get (&GAppDim),
      XmNrecomputeSize, False,
      XmNshadowThickness, 0,
      XmNheight, n_h,
      XmNwidth, max_w,
      XmNy, offset + ((AppDim_PstBarHt_Get (&GAppDim) 
        + AppDim_PstLinesHt_Get (&GAppDim) - n_h) >> 1) 
        + ViewLvl_LnYBeg_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM)),
      XmNx, 0,
      NULL));
  XmStringFree (nsgs_str);

  lvl_str = XmStringCreateLtoR (PVW_LBL_BLVL, AppDim_PstLblTag_Get (&GAppDim));
  ViewLvl_TLNLbl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM), 
      XtVaCreateManagedWidget ("PstVLvLL", 
      xmLabelWidgetClass,  PstView_LabelBB_Get (pv_p), 
      XmNlabelType, XmSTRING,
      XmNlabelString, lvl_str,
      XmNalignment, XmALIGNMENT_END,
      XmNmarginHeight, 0,
      XmNbackground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PSTDABG),
      XmNforeground, SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK),
      XmNmarginWidth, AppDim_PstLblMarg_Get (&GAppDim),
      XmNrecomputeSize, False,
      XmNshadowThickness, 0,
      XmNheight, (l_h > AppDim_PstSymDim_Get (&GAppDim)) 
        ? l_h : AppDim_PstSymDim_Get (&GAppDim),
      XmNwidth, max_w,
      XmNy, offset + ViewLvl_SymYBeg_Get (PstView_IthVLvl_Get (pv_p, 
        PVW_LEVEL_BTM)),
      XmNx, 0,
      NULL));
  XmStringFree (lvl_str);

  /* Set up the attachments and manage the children.  */
  XtVaSetValues (PstView_LabelBB_Get (pv_p),
    XmNtopAttachment, XmATTACH_FORM,
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_FORM,
    XmNrightAttachment, XmATTACH_OPPOSITE_FORM,
    XmNrightOffset, -max_w,
    NULL);

  XtVaSetValues (DrawCxt_DA_Get (pathdc_p),
    XmNtopAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, PstView_LabelBB_Get (pv_p),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtVaSetValues (PstView_ScrollWin_Get (pv_p),
    XmNtopAttachment, XmATTACH_WIDGET,
    XmNtopWidget, DrawCxt_DA_Get (pathdc_p),
    XmNbottomAttachment, XmATTACH_FORM,
    XmNleftAttachment, XmATTACH_WIDGET,
    XmNleftWidget, PstView_LabelBB_Get (pv_p),
    XmNrightAttachment, XmATTACH_FORM,
    NULL);

  XtManageChild (PstView_LabelBB_Get (pv_p));
  XtManageChild (DrawCxt_DA_Get (pathdc_p));
  XtManageChild (DrawCxt_DA_Get (pstdc_p));
  XtManageChild (PstView_ScrollWin_Get (pv_p));
  XtManageChild (form);

  return (PstView_Frame_Get (pv_p));
}
/*  End of PstView_Create  */


/****************************************************************************
*
*  Function Name:                 PstView_Destroy
*
*    This routine destroys the gc's and pixmaps used by PST View,
*    then free any memory allocated for its data structures.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Destroy
  (
  PstView_t   *pv_p
  )
{
  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  DrawCxt_t     *pathdc_p;                /* Path drawing context */ 
  DrawCxt_t     *pstdc_p;                 /* Pst tree drawing context */
  U8_t           lvl_i;                   /* Ith level of pst tree */

  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  pathdc_p = PstView_PathDC_Get (pv_p);
  pstdc_p = PstView_PstDC_Get (pv_p);

  XFreePixmap (Screen_TDisplay_Get (scra_p), DrawCxt_Pmap_Get (pathdc_p));
  XFreePixmap (Screen_TDisplay_Get (scra_p), DrawCxt_Pmap_Get (pstdc_p));

  XFreeGC (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p));
  XFreeGC (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p));

  if (PstView_PthTLvls_Get (pv_p) != NULL)
    {
    for (lvl_i = 0; lvl_i < PVW_PATH_NUMSYMS_INIT; lvl_i++)
      if (PstView_IthPTLvl_Get (pv_p, lvl_i) != NULL)
        TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, lvl_i));

    free (PstView_PthTLvls_Get (pv_p));
    }


  free (TreeLvl_CmpNodes_Get (ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get 
      (pv_p))));

  free (ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p)));

  if (PstView_CmpInfo_Get (pv_p) != NULL)
    free (PstView_CmpInfo_Get (pv_p));

  return ;
}
/*  End of PstView_Destroy  */


/****************************************************************************
*
*  Function Name:                 PstView_Display
*
*    This routine displays the pst viewing area.  It also makes calls
*    to update the two reaction previewers, and the reaction viewer.
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Display
  (
  PstView_t     *pv_p
  )
{
  PsvTreeLvl_t  *tlvl_p;                  /* Pst tree level */
  PsvTreeLvl_t  *pthtlvl_p;               /* Pst tree level */
  PsvViewLvl_t  *vlvl_p;                  /* Pst view level */
  RxnView_t     *rxnv_p;                  /* Reaction view */
  RxnPreView_t  *btmrxnpv_p;              /* Bottom reaction previewer */
  RxnPreView_t  *midrxnpv_p;              /* Middle reaction previewer */
  RxnPreView_t  *pthrxnpv_p;              /* Path previewer */
  RxnPreView_t  *toprxnpv_p;              /* Top compound previewer */
  DrawCxt_t     *pathdc_p;                /* Path drawing context */ 
  DrawCxt_t     *pstdc_p;                 /* Pst tree drawing context */
  Display       *disp;                    /* Selected kid compound */
  char           buff[16];                /* Label buffer */
  XmString       lbl_str;                 /* New label string */

  /*  Set up the tree levels, path and surrogate parent list, based on the 
      level of the selected node, then display them.
  */
  disp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  pathdc_p = PstView_PathDC_Get (pv_p);
  pstdc_p = PstView_PstDC_Get (pv_p);
  pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
  btmrxnpv_p = ViewLvl_RxnPV_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM));
  midrxnpv_p = ViewLvl_RxnPV_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID));
  pthrxnpv_p = ViewLvl_RxnPV_Get (PstView_PathVLvl_Get (pv_p));
  toprxnpv_p = ViewLvl_RxnPV_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP));

  /* Clear pst pixmap, draw each tree level, and then copy the pixmap 
     to drawing area.
  */ 
  if (DrawCxt_GCFill_Get (pstdc_p) != FillSolid)
    {
    XSetFillStyle (disp, DrawCxt_GC_Get (pstdc_p), FillSolid);
    DrawCxt_GCFill_Put (pstdc_p, (U8_t) FillSolid);
    }

  XSetForeground (disp, DrawCxt_GC_Get (pstdc_p), 
    DrawCxt_BgPxl_Get (pstdc_p));
  XFillRectangle (disp, DrawCxt_Pmap_Get (pstdc_p),
    DrawCxt_GC_Get (pstdc_p), 0, 0, DrawCxt_PmapW_Get (pstdc_p), 
    DrawCxt_DAH_Get (pstdc_p));
  XSetForeground (disp, DrawCxt_GC_Get (pstdc_p), 
    SynAppR_IthClrPx_Get (&GSynAppR, DrawCxt_GCFg_Get (pstdc_p)));

  /*  Top level bars, symbols, and label.  */
  STopLvlBars_Draw  (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP));
  SSymbols_Draw (pv_p, PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
    PVW_LEVEL_TOP), TRUE);
  SSelSym_Draw  (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP));

  if (TreeLvl_LvlNum_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, 
      PVW_LEVEL_TOP))) == 0)
    sprintf (buff, PVW_LBL_TLVL);
  else
    sprintf (buff, "%1hu", TreeLvl_LvlNum_Get (ViewLvl_TreeLvl_Get (
      PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP))));

  lbl_str = XmStringCreateLtoR (buff, AppDim_PstLblTag_Get (&GAppDim));
  XtVaSetValues (ViewLvl_TLNLbl_Get (PstView_IthVLvl_Get (pv_p, 
    PVW_LEVEL_TOP)),
    XmNlabelString, lbl_str,
    NULL);
    XmStringFree (lbl_str);

  /*  Top-mid level lines and label, and mid level symbols and label.  */
  SLines_Draw (PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
    PVW_LEVEL_TOP), PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID));
  if (PstView_SelNode_Get (pv_p, PVW_LEVEL_BTM) == PVW_NODE_NONE)
    SSelLine_Draw (PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
      PVW_LEVEL_TOP), PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID));

  SSymbols_Draw (pv_p, PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
    PVW_LEVEL_MID), TRUE);
  SSelSym_Draw  (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID));

  sprintf (buff, "%1hu", TreeLvl_LvlNum_Get (ViewLvl_TreeLvl_Get (
    PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID))));

  lbl_str = XmStringCreateLtoR (buff, AppDim_PstLblTag_Get (&GAppDim));
  XtVaSetValues (ViewLvl_TLNLbl_Get (PstView_IthVLvl_Get (pv_p, 
    PVW_LEVEL_MID)),
    XmNlabelString, lbl_str,
    NULL);
  XmStringFree (lbl_str);

  sprintf (buff, "%1u", TreeLvl_NSGs_Get (ViewLvl_TreeLvl_Get (
    PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID))));
  lbl_str = XmStringCreateLtoR (buff, AppDim_PstNSGTag_Get (&GAppDim));
  XtVaSetValues (ViewLvl_NSGLbl_Get (PstView_IthVLvl_Get (pv_p, 
    PVW_LEVEL_MID)),
    XmNlabelString, lbl_str,
    NULL);
  XmStringFree (lbl_str);

  /*  Mid-btm level lines and label, and btm level symbols and label.  */
  SLines_Draw (PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
    PVW_LEVEL_MID), PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM));
  if (TreeLvl_SelNd_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, 
      PVW_LEVEL_BTM))) != PVW_NODE_NONE)
    SSelLine_Draw (PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
      PVW_LEVEL_MID), PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM));
  SSymbols_Draw (pv_p, PstView_CmpInfo_Get (pv_p), PstView_IthVLvl_Get (pv_p,
    PVW_LEVEL_BTM), TRUE);
  SSelSym_Draw  (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM));

  sprintf (buff, "%1hu", TreeLvl_LvlNum_Get (ViewLvl_TreeLvl_Get (
    PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM))));

  lbl_str = XmStringCreateLtoR (buff, AppDim_PstLblTag_Get (&GAppDim));
  XtVaSetValues (ViewLvl_TLNLbl_Get (PstView_IthVLvl_Get (pv_p, 
    PVW_LEVEL_BTM)),
    XmNlabelString, lbl_str,
    NULL);
  XmStringFree (lbl_str);

  sprintf (buff, "%1u", TreeLvl_NSGs_Get (ViewLvl_TreeLvl_Get (
    PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM))));

  lbl_str = XmStringCreateLtoR (buff, AppDim_PstNSGTag_Get (&GAppDim));
    XtVaSetValues (ViewLvl_NSGLbl_Get (PstView_IthVLvl_Get (pv_p, 
    PVW_LEVEL_BTM)),
    XmNlabelString, lbl_str,
    NULL);
  XmStringFree (lbl_str);

  XCopyArea (disp, DrawCxt_Pmap_Get (pstdc_p), 
    XtWindow (DrawCxt_DA_Get (pstdc_p)), DrawCxt_GC_Get (pstdc_p), 
    0, 0, DrawCxt_DAW_Get (pstdc_p), DrawCxt_DAH_Get (pstdc_p), 0, 0);

  /*  Determine which reaction to display and display it.  */
  rxnv_p = PstView_RxnView_Get (pv_p);
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
    {
    if (avledit_called || RxnView_CurSG_Get (rxnv_p) == NULL
        || PstSubg_Index_Get (RxnView_CurSG_Get (rxnv_p)) 
        != TreeLvl_SelSG_Get (tlvl_p))
      RxnView_Update (rxnv_p, PstComp_Father_Get (TreeLvl_IthCmpNd_Get (
        tlvl_p, TreeLvl_SelNd_Get (tlvl_p))));
    }
  else 
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_SelNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      if (avledit_called || RxnView_CurSG_Get (rxnv_p) == NULL
          || PstSubg_Index_Get (RxnView_CurSG_Get (rxnv_p))
          != TreeLvl_SelSG_Get (tlvl_p))
        RxnView_Update (rxnv_p, PstComp_Father_Get (TreeLvl_IthCmpNd_Get (
          tlvl_p, TreeLvl_SelNd_Get (tlvl_p))));
      }
    }

  /* Clear path pixmap, draw the bars, symbols, and level numbers, 
     and then copy the pixmap to drawing area.
  */ 
  if (DrawCxt_GCFill_Get (pathdc_p) != FillSolid)
    {
    XSetFillStyle (disp, DrawCxt_GC_Get (pathdc_p), FillSolid);
    DrawCxt_GCFill_Put (pathdc_p, (U8_t) FillSolid);
    }

  XSetForeground (disp, DrawCxt_GC_Get (pathdc_p), 
    DrawCxt_BgPxl_Get (pathdc_p));
  XFillRectangle (disp, DrawCxt_Pmap_Get (pathdc_p),
    DrawCxt_GC_Get (pathdc_p), 0, 0, DrawCxt_PmapW_Get (pathdc_p), 
    DrawCxt_DAH_Get (pathdc_p));
  XSetForeground (disp, DrawCxt_GC_Get (pathdc_p), 
    SynAppR_IthClrPx_Get (&GSynAppR, DrawCxt_GCFg_Get (pathdc_p)));  

  STopLvlBars_Draw (PstView_PathVLvl_Get (pv_p));
  SSymbols_Draw (pv_p, PstView_CmpInfo_Get (pv_p), PstView_PathVLvl_Get (pv_p), FALSE);
  SPathLvlNums_Draw (PstView_PathVLvl_Get (pv_p));
  SPathSel_Draw (pv_p);
  XCopyArea (disp, DrawCxt_Pmap_Get (pathdc_p), 
    XtWindow (DrawCxt_DA_Get (pathdc_p)), DrawCxt_GC_Get (pathdc_p), 
    0, 0, DrawCxt_DAW_Get (pathdc_p), DrawCxt_DAH_Get (pathdc_p), 0, 0);

  /*  Update the four reaction previewers, if managed.  */
  if (XtIsManaged (RxnPreV_FormDlg_Get (pthrxnpv_p)))
    {
    vlvl_p = PstView_PathVLvl_Get (pv_p);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      React_Record_t *rxn_rec_p;

      PstView_Focus_Draw  (vlvl_p);
      if (TreeLvl_FocusNd_Get (tlvl_p) == 0)
        {
        if (String_Alloc_Get (RxnPreV_RxnName_Get (pthrxnpv_p)))
          String_Destroy (RxnPreV_RxnName_Get (pthrxnpv_p));
        RxnPreV_RxnName_Put (pthrxnpv_p, String_Create (RPV_LABEL_TARGET, 0));
        RxnPreV_Update (pthrxnpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 0), 
          TRUE);
        }
      else
        {
        rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (
          PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
          TreeLvl_FocusNd_Get (tlvl_p)))));
        if (String_Alloc_Get (RxnPreV_RxnName_Get (pthrxnpv_p)))
          String_Destroy (RxnPreV_RxnName_Get (pthrxnpv_p));
        RxnPreV_RxnName_Put (pthrxnpv_p, String_Copy (React_TxtRec_Name_Get 
          (React_Text_Get (rxn_rec_p))));

        /* Quirk of String_Copy:  doesn't null terminate string!! */
        *(String_Value_Get (RxnPreV_RxnName_Get (pthrxnpv_p))
          + String_Length_Get (RxnPreV_RxnName_Get (pthrxnpv_p))) 
          = (U8_t) '\0';
        RxnPreV_Update (pthrxnpv_p, 
          TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p)), TRUE);
        }
      }
    else
      {
      RxnPreV_Clear (pthrxnpv_p);
      }
    }

  if (XtIsManaged (RxnPreV_FormDlg_Get (btmrxnpv_p)))
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      React_Record_t *rxn_rec_p;

      PstView_Focus_Draw  (vlvl_p);
      rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
        TreeLvl_FocusNd_Get (tlvl_p)))));
      if (String_Alloc_Get (RxnPreV_RxnName_Get (btmrxnpv_p)))
        String_Destroy (RxnPreV_RxnName_Get (btmrxnpv_p));
      RxnPreV_RxnName_Put (btmrxnpv_p, String_Copy (React_TxtRec_Name_Get 
        (React_Text_Get (rxn_rec_p))));

      /* Quirk of String_Copy:  doesn't null terminate string!! */
      *(String_Value_Get (RxnPreV_RxnName_Get (btmrxnpv_p))
        + String_Length_Get (RxnPreV_RxnName_Get (btmrxnpv_p))) 
        = (U8_t) '\0';
      RxnPreV_Update (btmrxnpv_p, 
        TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p)), TRUE);
      }
    else
      {
      RxnPreV_Clear (btmrxnpv_p);
      }
    }

  if (XtIsManaged (RxnPreV_FormDlg_Get (midrxnpv_p)))
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      React_Record_t *rxn_rec_p;

      PstView_Focus_Draw  (vlvl_p);
      rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
        TreeLvl_FocusNd_Get (tlvl_p)))));
      if (String_Alloc_Get (RxnPreV_RxnName_Get (midrxnpv_p)))
        String_Destroy (RxnPreV_RxnName_Get (midrxnpv_p));
      RxnPreV_RxnName_Put (midrxnpv_p, String_Copy (React_TxtRec_Name_Get 
        (React_Text_Get (rxn_rec_p))));

      /* Quirk of String_Copy:  doesn't null terminate string!! */
      *(String_Value_Get (RxnPreV_RxnName_Get (midrxnpv_p))
        + String_Length_Get (RxnPreV_RxnName_Get (midrxnpv_p))) 
        = (U8_t) '\0';
      RxnPreV_Update (midrxnpv_p, 
        TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p)), TRUE);
      }
    else
      {
      RxnPreV_Clear (midrxnpv_p);
      }
    }

  if (XtIsManaged (RxnPreV_FormDlg_Get (toprxnpv_p)))
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (TreeLvl_FocusNd_Get (tlvl_p) != PVW_NODE_NONE)
      {
      React_Record_t *rxn_rec_p;

      PstView_Focus_Draw  (vlvl_p);
      if (TreeLvl_LvlNum_Get (tlvl_p) == 0)
        {
        if (String_Alloc_Get (RxnPreV_RxnName_Get (toprxnpv_p)))
          String_Destroy (RxnPreV_RxnName_Get (toprxnpv_p));
        RxnPreV_RxnName_Put (toprxnpv_p, String_Create (RPV_LABEL_TARGET, 0));
        RxnPreV_Update (toprxnpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, 0), 
          TRUE);
        }
      else
        {
        rxn_rec_p = React_Schema_Handle_Get (PstSubg_Reaction_Schema_Get (
          PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 
          TreeLvl_FocusNd_Get (tlvl_p)))));
        if (String_Alloc_Get (RxnPreV_RxnName_Get (toprxnpv_p)))
          String_Destroy (RxnPreV_RxnName_Get (toprxnpv_p));
        RxnPreV_RxnName_Put (toprxnpv_p, String_Copy (React_TxtRec_Name_Get 
          (React_Text_Get (rxn_rec_p))));

        /* Quirk of String_Copy:  doesn't null terminate string!! */
        *(String_Value_Get (RxnPreV_RxnName_Get (toprxnpv_p))
          + String_Length_Get (RxnPreV_RxnName_Get (toprxnpv_p))) 
          = (U8_t) '\0';
        RxnPreV_Update (toprxnpv_p, 
          TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_FocusNd_Get (tlvl_p)), TRUE);
        }
      }
    else
      {
      RxnPreV_Clear (midrxnpv_p);
      }
    }

  if (pvdisp_reset_mouse) PstView_Mouse_Reset (pv_p, disp);
  return;
}
/*  End of PstView_Display  */


/****************************************************************************
*
*  Function Name:                 PstView_Focus_Draw
*
*    Draw the focus box at given node on the given level.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Focus_Draw
  (
  PsvViewLvl_t  *vlvl_p                 /* Pst level */
  )
{
  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */ 
  Display       *dsp;
  DrawCxt_t     *dc_p;
  XPoint         pts[5];
  short int      x;

  if (vlvl_p == NULL)
    return;

  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);

  if (TreeLvl_FocusNd_Get (tlvl_p) == PVW_NODE_NONE)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (vlvl_p);
  x = AppDim_PstSymSepWd_Get (&GAppDim) + AppDim_PstSymTotWd_Get (&GAppDim) 
    * (TreeLvl_HeadNd_Get (tlvl_p) + TreeLvl_FocusNd_Get (tlvl_p));
  pts[0].x = x - AppDim_PstFBoxOff_Get (&GAppDim);
  pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p) - AppDim_PstFBoxOff_Get (&GAppDim);
  pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1) 
    + AppDim_PstFBoxOff_Get (&GAppDim);
  pts[1].y = pts[0].y;
  pts[2].x = pts[1].x;
  pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p) + AppDim_PstFBoxOff_Get (&GAppDim);
  pts[3].x = pts[0].x;
  pts[3].y = pts[2].y;
  pts[4].x = pts[0].x;
  pts[4].y = pts[0].y;

  if (DrawCxt_GCLineW_Get (dc_p) != PVW_FOCUSBOX_LNWD)
    {
    XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_FOCUSBOX_LNWD,
      LineSolid, CapButt, JoinMiter);
    DrawCxt_GCLineW_Put (dc_p, PVW_FOCUSBOX_LNWD);
    }

  XSetForeground (dsp, DrawCxt_GC_Get (dc_p), ViewLvl_FocusClr_Get (vlvl_p));
  XDrawLines (dsp, XtWindow (DrawCxt_DA_Get (dc_p)), DrawCxt_GC_Get (dc_p),
    pts, 5, CoordModeOrigin);

  XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
    SynAppR_IthClrPx_Get (&GSynAppR, DrawCxt_GCFg_Get (dc_p)));

  return ;
}
/*  End of PstView_Focus_Draw  */


/****************************************************************************
*
*  Function Name:                 PstView_Focus_Undraw
*
*    Draw over the existing focus box in the appropriate background color
*    at the given node on the given level.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Focus_Undraw
  (
  PsvViewLvl_t  *vlvl_p                 /* Pst view level */
  )
{
  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */ 
  Display       *dsp;
  DrawCxt_t     *dc_p;
  XPoint         pts[5];
  short int      x;

  if (vlvl_p == NULL)
    return;

  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);

  if (TreeLvl_FocusNd_Get (tlvl_p) == PVW_NODE_NONE)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (vlvl_p);

  /*  Redraw as selected node if it has been selected.  */
  if (TreeLvl_FocusNd_Get (tlvl_p) == TreeLvl_SelNd_Get (tlvl_p))
    {
    SSelSym_Draw  (vlvl_p);
    return;
    }

  x = AppDim_PstSymSepWd_Get (&GAppDim) + AppDim_PstSymTotWd_Get (&GAppDim)
    * (TreeLvl_HeadNd_Get (tlvl_p) + TreeLvl_FocusNd_Get (tlvl_p));
  pts[0].x = x - AppDim_PstFBoxOff_Get (&GAppDim);
  pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p) - AppDim_PstFBoxOff_Get (&GAppDim);
  pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1) 
    + AppDim_PstFBoxOff_Get (&GAppDim);
  pts[1].y = pts[0].y;
  pts[2].x = pts[1].x;
  pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p) + AppDim_PstFBoxOff_Get (&GAppDim);
  pts[3].x = pts[0].x;
  pts[3].y = pts[2].y;
  pts[4].x = pts[0].x;
  pts[4].y = pts[0].y;

  if (DrawCxt_GCLineW_Get (dc_p) != PVW_FOCUSBOX_LNWD)
    {
    XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_FOCUSBOX_LNWD,
      LineSolid, CapButt, JoinMiter);
    DrawCxt_GCLineW_Put (dc_p, PVW_FOCUSBOX_LNWD);
    }

  XSetForeground (dsp, DrawCxt_GC_Get (dc_p), DrawCxt_BgPxl_Get (dc_p));
  XDrawLines (dsp, XtWindow (DrawCxt_DA_Get (dc_p)), DrawCxt_GC_Get (dc_p),
    pts, 5, CoordModeOrigin);
  XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
    SynAppR_IthClrPx_Get (&GSynAppR, DrawCxt_GCFg_Get (dc_p)));

  return ;
}
/*  End of PstView_Focus_Undraw  */


/****************************************************************************
*
*  Function Name:                 PstView_Mark_Store
*
*    For all other instances in the pst of the given marked compound, 
*    store the fact that it was marked elsewhere in the list of compound
*    information.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Mark_Store
  (
  PsvCmpInfo_t  *cmpinfos,              /* Compound infos */
  Compound_t    *cmp_p                  /* Visited compound */
  )
{
  Compound_t    *else_p;                /* Other instances */

  if (cmp_p == NULL)
    return;

  PsvCmpI_Marked_Set (cmpinfos[PstComp_Index_Get (cmp_p)]);

/* Just mark the single instance - forget about marking elsewhere! */

/*
  else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
  while (else_p != NULL)
    {
    if (else_p == cmp_p)
      PsvCmpI_Marked_Set (cmpinfos[PstComp_Index_Get (else_p)]);
    else
      PsvCmpI_MarkElse_Set (cmpinfos[PstComp_Index_Get (else_p)]);

    else_p = PstComp_Prev_Get (else_p);
    }
*/

  return ;
}
/*  End of PstView_Mark_Store  */


/****************************************************************************
*
*  Function Name:                 PstView_Mouse_Remove
*
*    This routine removes the mouse eventhandler and callbacks so that
*    such events are not recognized or attempted to be processed while
*    the pst tree is being updated.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Mouse_Remove
  (
  PstView_t *pv_p
  )
{

  /*  Remove selection event callbacks and pointer eventhandler.  */
  XtRemoveCallback (DrawCxt_DA_Get (PstView_PathDC_Get (pv_p)),
    XmNinputCallback, SPstView_PthSel_CB, (XtPointer) pv_p);
  XtRemoveCallback (DrawCxt_DA_Get (PstView_PstDC_Get (pv_p)), 
    XmNinputCallback, SPstView_PstSel_CB, (XtPointer) pv_p);

  /*  Remove callbacks for Select Trace if has been created and is 
      currently being used.  disabled - DK 
  
   SelTrace_t    *selt_p;

  selt_p = PstView_SelTrace_Get (pv_p);
  if (SelTrace_IsCreated_Get (selt_p) 
      && XtIsManaged (SelTrace_FormDlg_Get (selt_p))) 
    {
    XtRemoveCallback (SelTrace_CmpSelPB_Get (selt_p), XmNactivateCallback, 
      SelTrace_CmpSel_CB, (XtPointer) pv_p);
    XtRemoveCallback (SelTrace_CmpText_Get (selt_p), XmNactivateCallback, 
      SelTrace_CmpText_CB, (XtPointer) pv_p);
    XtRemoveCallback (SelTrace_CycleNextAB_Get (selt_p), XmNactivateCallback, 
      SelTrace_CycleNext_CB, (XtPointer) pv_p);
    XtRemoveCallback (SelTrace_CyclePrevAB_Get (selt_p), XmNactivateCallback, 
      SelTrace_CyclePrev_CB, (XtPointer) pv_p);
    XtRemoveCallback (SelTrace_CycleText_Get (selt_p), XmNactivateCallback, 
      SelTrace_CycleText_CB, (XtPointer) pv_p);
    XtRemoveCallback (SelTrace_DismissPB_Get (selt_p), XmNactivateCallback, 
      SelTrace_Dismiss_CB, (XtPointer) selt_p);
    }
  */

  return;
}
/*  End of PstView_Mouse_Remove  */


/****************************************************************************
*
*  Function Name:                 PstView_Mouse_Reset
*
*    This routine reinstalls the mouse eventhandler and callbacks.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Mouse_Reset
  (
  PstView_t *pv_p,
  Display   *disp
  )
{
 XEvent         event;                   /* Used to flush select events */

  /*  Remove select events from the event queue and restore callbacks.  */
  while (XCheckTypedEvent (disp, ButtonPress | ButtonRelease, &event)) 
    ;

  XtAddCallback (DrawCxt_DA_Get (PstView_PstDC_Get (pv_p)), 
    XmNinputCallback, SPstView_PstSel_CB, (XtPointer) pv_p);
  XtAddCallback (DrawCxt_DA_Get (PstView_PathDC_Get (pv_p)),
    XmNinputCallback, SPstView_PthSel_CB, (XtPointer) pv_p);

  /*  Restore callbacks for Select Trace if has been created and is 
      currently being used.  disabled - DK
  
 SelTrace_t    *selt_p;

  selt_p = PstView_SelTrace_Get (pv_p);
  if (SelTrace_IsCreated_Get (selt_p) 
      && XtIsManaged (SelTrace_FormDlg_Get (selt_p))) 
    {
    XtAddCallback (SelTrace_CmpSelPB_Get (selt_p), XmNactivateCallback, 
      SelTrace_CmpSel_CB, (XtPointer) pv_p);
    XtAddCallback (SelTrace_CmpText_Get (selt_p), XmNactivateCallback, 
      SelTrace_CmpText_CB, (XtPointer) pv_p);
    XtAddCallback (SelTrace_CycleNextAB_Get (selt_p), XmNactivateCallback, 
      SelTrace_CycleNext_CB, (XtPointer) pv_p);
    XtAddCallback (SelTrace_CyclePrevAB_Get (selt_p), XmNactivateCallback, 
      SelTrace_CyclePrev_CB, (XtPointer) pv_p);
    XtAddCallback (SelTrace_CycleText_Get (selt_p), XmNactivateCallback, 
      SelTrace_CycleText_CB, (XtPointer) pv_p);
    XtAddCallback (SelTrace_DismissPB_Get (selt_p), XmNactivateCallback, 
      SelTrace_Dismiss_CB, (XtPointer) selt_p);
    }
  */


  /*  Reposition pointer to follow selected node.  */
  if (PstView_ActVLvl_Get (pv_p) != PVW_LEVEL_NONE)
    {
    PsvTreeLvl_t  *tlvl_p;                /* Tree view level  */ 
    PsvViewLvl_t  *vlvl_p;                /* Pst view level */
    int            new_x, new_y;

    vlvl_p = PstView_IthVLvl_Get (pv_p, PstView_ActVLvl_Get (pv_p));
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    new_x = (TreeLvl_SelNd_Get (tlvl_p) + TreeLvl_HeadNd_Get (tlvl_p))
      * AppDim_PstSymTotWd_Get (&GAppDim) + AppDim_PstSymDim_Get (&GAppDim) - 1;
    new_y = ViewLvl_SymYBeg_Get (vlvl_p) + AppDim_PstSymMid_Get (&GAppDim) - 1;
    XWarpPointer (disp, 
      XtWindow (DrawCxt_DA_Get (ViewLvl_DrawCxt_Get (vlvl_p))),
      XtWindow (DrawCxt_DA_Get (ViewLvl_DrawCxt_Get (vlvl_p))),
      0, 0, 0, 0, new_x, new_y);

    PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE);
    }

  return;
}
/*  End of PstView_Mouse_Reset  */


/****************************************************************************
*
*  Function Name:                 PstView_PostInit
*
*    This routine is called after the application is realized.  The
*    height of the drawing area and pixmaps for the PST need to be
*    calculated after the clip window of the scrolled window is 
*    realized.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_PostInit
  (
  Widget         top_level,
  PstView_t     *pv_p
  )
{
  XSetWindowAttributes winattr;
  ScreenAttr_t  *scra_p;                  /* Screen Attritbutes */
  DrawCxt_t     *pathdc_p;                /* Path drawing context */ 
  DrawCxt_t     *pstdc_p;                 /* Pst tree drawing context */
  RxnView_t     *rxnv_p;
  Dimension      cw_h, cw_w;
  Dimension      mf_h, mf_w;
  Dimension      pthda_w;
  U16_t          num_vsb;

  scra_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  pathdc_p = PstView_PathDC_Get (pv_p);
  pstdc_p = PstView_PstDC_Get (pv_p);
  rxnv_p = PstView_RxnView_Get (pv_p);

  /*  Set window gravity on reaction application window.  */
  winattr.bit_gravity = ForgetGravity;
  XChangeWindowAttributes (XtDisplay (top_level), 
    XtWindow (top_level), 
    CWBitGravity, &winattr);

  /*  Initialize size variables.  */
  XtVaGetValues (PstView_ClipWin_Get (pv_p),
    XmNheight, &cw_h,
    XmNwidth, &cw_w,
    NULL);

  PstView_ClipH_Put (pv_p, cw_h);
  PstView_ClipW_Put (pv_p, cw_w);

  XtVaGetValues (RxnView_MolForm_Get (rxnv_p),
    XmNheight, &mf_h,
    XmNwidth, &mf_w,
    NULL);

  RxnView_MolDAH_Put (rxnv_p, mf_h - RxnV_IthM_LblsH_Get (rxnv_p, 0));
  RxnView_MolFormW_Put (rxnv_p, mf_w);

  if (cw_h > DrawCxt_DAH_Get (pstdc_p))
    {
    DrawCxt_DAH_Put (pstdc_p, cw_h - AppDim_XHtPvwDA_Get (&GAppDim));
    XtVaSetValues (DrawCxt_DA_Get (pstdc_p),
    XmNheight, DrawCxt_DAH_Get (pstdc_p),
    NULL);
    }

  num_vsb = (U16_t) ((cw_w < AppDim_PstSymSepWd_Get (&GAppDim)) ? 0
    : (cw_w - AppDim_PstSymSepWd_Get (&GAppDim)) 
    / AppDim_PstSymTotWd_Get (&GAppDim));
  DrawCxt_NumVsbNd_Put (pstdc_p, num_vsb);

  XtVaGetValues (DrawCxt_DA_Get (pathdc_p),
    XmNwidth, &pthda_w,
    NULL);
  DrawCxt_DAW_Put (pathdc_p, pthda_w);
  DrawCxt_PmapW_Put (pathdc_p, pthda_w);

  num_vsb = (U16_t) (pthda_w < AppDim_PstSymSepWd_Get (&GAppDim)) ? 0
    : (pthda_w - AppDim_PstSymSepWd_Get (&GAppDim)) 
    / AppDim_PstSymTotWd_Get (&GAppDim);
  DrawCxt_NumVsbNd_Put (pathdc_p, num_vsb);


  /*  Create the PST tree view pixmap and graphic context.  */
  DrawCxt_Pmap_Put (pstdc_p, XCreatePixmap (Screen_TDisplay_Get (scra_p),
    Screen_RootWin_Get (scra_p), DrawCxt_DAW_Get (pstdc_p), 
    DrawCxt_DAH_Get (pstdc_p), Screen_Depth_Get (scra_p)));

  DrawCxt_GC_Put (pstdc_p, XCreateGC (Screen_TDisplay_Get (scra_p),
    XtWindow (DrawCxt_DA_Get (pstdc_p)), 0, NULL));
  XSetLineAttributes (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p),
    PVW_DEFAULT_LNWD, LineSolid, CapButt, JoinMiter);
  XSetBackground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p),
    DrawCxt_BgPxl_Get (pstdc_p));
  XSetFont (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p),
    SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML)->fid);

  /*  Initialize the pixmap with some stipple pattern.  */
  XSetForeground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p), 
    DrawCxt_BgPxl_Get (pstdc_p));
  XSetFillStyle (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p), 
    FillSolid);
  XFillRectangle (Screen_TDisplay_Get (scra_p), DrawCxt_Pmap_Get (pstdc_p),
    DrawCxt_GC_Get (pstdc_p), 0, 0, DrawCxt_PmapW_Get (pstdc_p), 
    DrawCxt_DAH_Get (pstdc_p));

  XSetForeground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p), 
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VE));
  XSetFillStyle (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p), 
    FillStippled);
  XSetStipple (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p), 
    SynAppR_IthPMap_Get (&GSynAppR, SAR_PIXMAP_FLASK));
  XFillRectangle (Screen_TDisplay_Get (scra_p), DrawCxt_Pmap_Get (pstdc_p),
    DrawCxt_GC_Get (pstdc_p), 0, 0, DrawCxt_PmapW_Get (pstdc_p), 
    DrawCxt_DAH_Get (pstdc_p));

  XSetFillStyle (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p),
    FillSolid);
  XSetForeground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pstdc_p),
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  DrawCxt_GCFg_Put (pstdc_p, SAR_CLRI_BLACK);
  DrawCxt_GCFill_Put (pstdc_p, (U8_t) FillSolid);
  DrawCxt_GCLineW_Put (pstdc_p, PVW_DEFAULT_LNWD);
  DrawCxt_GCLnSty_Put (pstdc_p, LineSolid);

  /*  Create the Path pixmap and graphic context.  */
  DrawCxt_Pmap_Put (pathdc_p, XCreatePixmap (Screen_TDisplay_Get (scra_p),
    Screen_RootWin_Get (scra_p), DrawCxt_PmapW_Get (pathdc_p), 
    DrawCxt_DAH_Get (pathdc_p), Screen_Depth_Get (scra_p)));

  DrawCxt_GC_Put (pathdc_p, XCreateGC (Screen_TDisplay_Get (scra_p),
    XtWindow (DrawCxt_DA_Get (pathdc_p)), 0, NULL));
  XSetLineAttributes (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p),
    PVW_DEFAULT_LNWD, LineSolid, CapButt, JoinMiter);
  XSetBackground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p),
    DrawCxt_BgPxl_Get (pathdc_p));
  XSetFont (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p),
    SynAppR_IthFont_Get (&GSynAppR, SAR_FONTSTRCTS_NRML)->fid);

  /*  Initialize the pixmap clear with background color.  */
  XSetForeground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p), 
    DrawCxt_BgPxl_Get (pathdc_p));
  XSetFillStyle (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p), 
    FillSolid);
  XFillRectangle (Screen_TDisplay_Get (scra_p), DrawCxt_Pmap_Get (pathdc_p),
    DrawCxt_GC_Get (pathdc_p), 0, 0, DrawCxt_PmapW_Get (pathdc_p), 
    DrawCxt_DAH_Get (pathdc_p));

  XSetForeground (Screen_TDisplay_Get (scra_p), DrawCxt_GC_Get (pathdc_p),
    SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
  DrawCxt_GCFg_Put (pathdc_p, SAR_CLRI_BLACK);
  DrawCxt_GCFill_Put (pathdc_p, (U8_t) FillSolid);
  DrawCxt_GCLineW_Put (pathdc_p, PVW_DEFAULT_LNWD);
  DrawCxt_GCLnSty_Put (pathdc_p, LineSolid);


  /*  Change cursor for the pst view drawing area.  */
  XDefineCursor (XtDisplay (DrawCxt_DA_Get (pstdc_p)), 
    XtWindow (DrawCxt_DA_Get (pstdc_p)), 
    SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PSTVW));

  /*  Change cursor for the path view drawing area.  */
  XDefineCursor (Screen_TDisplay_Get (scra_p), 
    XtWindow (DrawCxt_DA_Get (pathdc_p)), 
    SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_PTHVW));

  /*  Change cursor for the reaction view drawing area.  */
  XDefineCursor (Screen_TDisplay_Get (scra_p), 
    XtWindow (RxnView_Form_Get (rxnv_p)), 
    SynAppR_IthCursor_Get (&GSynAppR, SAR_CRSRI_RXNVW));

  return ;
}
/*  End of PstView_PostInit  */


/****************************************************************************
*
*  Function Name:                 PstView_Preview_EH
*
*    This routine handles the mouse movement events for the pst tree
*    drawing area.  When the pointer is moved into the area of a 
*    symbol, a focus box is drawn around it, and if appropriate, the
*    reaction preview is updated.  
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Preview_EH
  (
  Widget     w,                           /* Widget */
  XtPointer  client_data,                 /* PstView */
  XEvent    *event,                       /* Event  */ 
  Boolean   *cont                         /* Continue to dispatch? */
  )
{
  PstView_t    *pv_p;
  XMotionEvent *mevent;
  int           rem;
  U16_t         curvnode;
  U8_t          curlvl;

  pv_p = (PstView_t *) client_data;

  /*  Only process motion events with no state.  */
  if (event->type == MotionNotify)
    {
    mevent = &event->xmotion;
    if (mevent->state == 0)
      {
      /*  Determine which level and node.  */
      rem = (mevent->y % AppDim_PstLvlTotHt_Get (&GAppDim)) 
        + AppDim_PstLvlBdrOff_Get (&GAppDim);
      if (rem >= AppDim_PstLvlBdrTop_Get (&GAppDim) 
          && rem < AppDim_PstLvlBdrBtm_Get (&GAppDim))
        curlvl = (U8_t) (mevent->y / AppDim_PstLvlTotHt_Get (&GAppDim));
      else
        curlvl = PVW_LEVEL_NONE;

      if ((mevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
            % AppDim_PstSymTotWd_Get (&GAppDim) 
            < AppDim_PstSymDim_Get (&GAppDim))
        curvnode = (U16_t)((mevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
          / AppDim_PstSymTotWd_Get (&GAppDim));
      else
        curvnode = PVW_NODE_NONE;

      /*  If the pointer is sitting in a valid node, and the previewer for
          this level is active, then redraw the focus box and update the 
          reaction preview (if appropriate).
      */
      if (curlvl != PVW_LEVEL_NONE && curvnode != PVW_NODE_NONE)
        {
        PsvTreeLvl_t  *tlvl_p;                /* Tree level */ 
        PsvViewLvl_t  *vlvl_p;                /* Pst view level */
        RxnPreView_t  *rpv_p;

        vlvl_p = PstView_IthVLvl_Get (pv_p, curlvl);
        rpv_p = ViewLvl_RxnPV_Get (vlvl_p);
        tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
        if (curvnode >= TreeLvl_HeadNd_Get (tlvl_p) 
           && curvnode < TreeLvl_HeadNd_Get (tlvl_p) 
           + TreeLvl_NCmps_Get (tlvl_p)
           && curvnode != TreeLvl_HeadNd_Get (tlvl_p) 
             + TreeLvl_FocusNd_Get (tlvl_p)
           && XtIsManaged (RxnPreV_FormDlg_Get (rpv_p)))
          {
          React_Record_t *rxn_rec_p;
          U32_t           cursg;
          U16_t           curtnode;

          curtnode = curvnode - TreeLvl_HeadNd_Get (tlvl_p);
          PstView_Focus_Undraw  (vlvl_p);
          TreeLvl_FocusNd_Put (tlvl_p, curtnode);
          PstView_Focus_Draw  (vlvl_p);

          /*  If this focused compound belongs to same subgoal, then no
              changes to the previewer need to be made.  Otherwise,
              we need to update the reaction name, compound numbers,
              and reaction center.  The target molecule stays the same.
          */
          cursg = PstSubg_Index_Get (PstComp_Father_Get 
            (TreeLvl_IthCmpNd_Get (tlvl_p, curtnode)));
          if (cursg != TreeLvl_FocusSG_Get (tlvl_p))
            {
            TreeLvl_FocusSG_Put (tlvl_p, cursg);
            rxn_rec_p = React_Schema_Handle_Get 
              (PstSubg_Reaction_Schema_Get (PstComp_Father_Get 
              (TreeLvl_IthCmpNd_Get (tlvl_p, curtnode))));
            if (String_Alloc_Get (RxnPreV_RxnName_Get (rpv_p)))
              String_Destroy (RxnPreV_RxnName_Get (rpv_p));
            RxnPreV_RxnName_Put (rpv_p, String_Copy (React_TxtRec_Name_Get 
              (React_Text_Get (rxn_rec_p))));

            /* Quirk of String_Copy:  doesn't null terminate string!! */
            *(String_Value_Get (RxnPreV_RxnName_Get (rpv_p))
              + String_Length_Get (RxnPreV_RxnName_Get (rpv_p))) 
              = (U8_t) '\0';
            RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, curtnode), 
              TRUE);
            }
          else
            {
            RxnPreV_Update (rpv_p, TreeLvl_IthCmpNd_Get (tlvl_p, curtnode), 
              FALSE);
            }
          }
        }
      }  /* End of if empty state */
    }  /* End of if motion event */

  return ;
}
/*  End of PstView_Preview_EH  */


/****************************************************************************
*
*  Function Name:                 PstView_Reset
*
*    This routine destroys the data structures specific to a particular
*    status file so that a new status file can be viewed.  This also
*    closes or resets tools associated with the given tree.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    Frees memory.
*
******************************************************************************/
void PstView_Reset
  (
  PstView_t   *pv_p
  )
{
  PsvViewLvl_t  *vlvl_p;                  /* View level of pst */ 
  CmpInst_t     *cinst_p;
  SelTrace_t    *selt_p;
  U8_t           lvl_i;                   /* Ith level of pst tree */

  warning_shown = FALSE; /* reset global flag to re-enable circularity check overflow warning */


  /*  Free up the list of compound information nodes.  */
  if (PstView_CmpInfo_Get (pv_p) != NULL)
    {
    free (PstView_CmpInfo_Get (pv_p));
    PstView_CmpInfo_Put (pv_p, NULL);
    PstView_NumCmpI_Put (pv_p, 0);
    }

  /*  Free up the path level nodes and reinitialize the path view level.  */

  if (PstView_PthTLvls_Get (pv_p) != NULL)
    {
    for (lvl_i = 0; lvl_i < PVW_PATH_NUMSYMS_INIT; lvl_i++)
      if (PstView_IthPTLvl_Get (pv_p, lvl_i) != NULL)
        {
        TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, lvl_i));
        PstView_IthPTLvl_Put (pv_p, lvl_i, NULL);
        }
    }

  /*  Reinitialize the view level nodes.  */
  for (lvl_i = 0; lvl_i < PVW_LEVEL_NUMOF; lvl_i++)
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, lvl_i);
    ViewLvl_TreeLvl_Put (vlvl_p, TreeLvl_Create (NULL, 0, lvl_i, 
      PVW_TREETYPE_NONE));
    }

  /*  Reinitialize the path view level.  */
  vlvl_p = PstView_PathVLvl_Get (pv_p);
  TreeLvl_NCmps_Put (ViewLvl_TreeLvl_Get (vlvl_p), 0);
  TreeLvl_NSGs_Put (ViewLvl_TreeLvl_Get (vlvl_p), 0);

  PstView_PathLen_Put (pv_p, 0);
  PstView_TreeType_Put (pv_p, PVW_TREETYPE_FULL);
  PstView_LeftNd_Put (pv_p, 0);
  PstView_LastSelCmp_Put (pv_p, NULL);
  PstView_PstCB_Put (pv_p, NULL);

  selt_p = PstView_SelTrace_Get (pv_p);
  if (SelTrace_IsCreated_Get (selt_p) 
      && XtIsManaged (SelTrace_FormDlg_Get (selt_p)))
    {
    XtUnmanageChild (SelTrace_FormDlg_Get (selt_p));
    SelTrace_Reset (selt_p);
    }

  cinst_p = PstView_CmpInsts_Get (pv_p);
  if (CmpInst_IsCreated_Get (cinst_p) 
      && XtIsManaged (CmpInst_FormDlg_Get (cinst_p)))
    CmpInst_Reset (cinst_p);

  return ;
}
/*  End of PstView_Reset  */


/****************************************************************************
*
*  Function Name:                 PstView_Trace_Store
*
*    Store the fact that the given compound has been traced (cycle trace or
*    instance trace).  If others is true,  then for all other instances in 
*    the PST of the given traced compound, store the fact that it was traced 
*    elsewhere in the list of compound information.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Trace_Store
  (
  PsvCmpInfo_t  *cmpinfos,              /* Compound infos */
  Compound_t    *cmp_p,                 /* Visited compound */
  U32_t          cycle_instance,
  Boolean_t      others
  )
{
  Compound_t    *else_p;                /* Other instances */

  if (cmp_p == NULL)
    return;

  else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
  while (else_p != NULL)
    {
    if (else_p == cmp_p)
      {
      PsvCmpI_Traced_Set (cmpinfos[PstComp_Index_Get (else_p)]);
      PsvCmpI_Cycle_Put (cmpinfos[PstComp_Index_Get (else_p)], cycle_instance);
      }
    else if (others)
      {
      PsvCmpI_TracElse_Set (cmpinfos[PstComp_Index_Get (else_p)]);
      PsvCmpI_Cycle_Put (cmpinfos[PstComp_Index_Get (else_p)], cycle_instance);
      }

    else_p = PstComp_Prev_Get (else_p);
    }

  return ;
}
/*  End of PstView_Trace_Store  */


/****************************************************************************
*
*  Function Name:                 PstView_Tree_Init
*
*    This routine loads in the problems solving tree in preparation for
*    displaying it.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstView_Tree_Init
  (
  PstView_t     *pv_p, 
  PstCB_t       *pcb_p
  )
{
  PsvCmpInfo_t  *cmp_infos;               /* Compound info records */
  U32_t          cmp_i;                   /* Ith compound info record */
  
  /*  First make sure that there is a current nonempty problem solving tree
      to display.
  */
  if (pcb_p == NULL)
    {
    fprintf (stderr, 
      "PstView_Tree_Init:  null pst control block pointer.\n");
    return (FALSE);
    }

  if (PstCB_TotalExpandedCompounds_Get (pcb_p) == 0)
    {
    fprintf (stderr, 
      "PstView_Tree_Load:  no compounds in problem solving tree.\n");
    return (FALSE);
    }

  PstView_PstCB_Put (pv_p, pcb_p);
  PstView_NumCmpI_Put (pv_p, PstCB_CompoundIndex_Get (pcb_p));
  cmp_infos = (PsvCmpInfo_t *) malloc (PstView_NumCmpI_Get (pv_p)
    * sizeof (PsvCmpInfo_t));
  if (cmp_infos == NULL)
    {
    fprintf (stderr, 
      "PstView_Tree_Init:  unable to allocate memory for cmp info recs.\n");
    exit (-1);
    }

  PstView_CmpInfo_Put (pv_p, cmp_infos);

  /*  Initialize compound information nodes.  */
  for (cmp_i = 0; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
    {
    PsvCmpI_RxnCmmt_Put (cmp_infos[cmp_i], NULL);
    PsvCmpI_CmtLen_Put (cmp_infos[cmp_i], 0);
    PsvCmpI_Cycle_Put (cmp_infos[cmp_i], 0);
    PsvCmpI_MidBar_Put (cmp_infos[cmp_i], 0);
    PsvCmpI_Status_Put (cmp_infos[cmp_i], PVW_STATUS_NONE);
    }

  return (TRUE);
}
/*  End of PstView_Tree_Init  */


/****************************************************************************
*
*  Function Name:                 PstView_Unmark_Store
*
*    For all other instances in the pst of the given marked compound, 
*    store the fact that it was marked elsewhere in the list of compound
*    information.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Unmark_Store
  (
  PsvCmpInfo_t  *cmpinfos,              /* Compound infos */
  Compound_t    *cmp_p                  /* Visited compound */
  )
{
  Compound_t    *else_p;                /* Other instances */

  if (cmp_p == NULL)
    return;

  PsvCmpI_Marked_Unset (cmpinfos[PstComp_Index_Get (cmp_p)]);

/* Just unmark the single instance - forget about unmarking elsewhere! */

/*
  else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
  while (else_p != NULL)
    {
    PsvCmpI_Marked_Unset (cmpinfos[PstComp_Index_Get (else_p)]);
    PsvCmpI_MarkElse_Unset (cmpinfos[PstComp_Index_Get (else_p)]);

    else_p = PstComp_Prev_Get (else_p);
    }
*/

  return ;
}
/*  End of PstView_Unmark_Store  */


/****************************************************************************
*
*  Function Name:                 PstView_Visit_Store
*
*    For all other instances in the pst of the given visited compound, 
*    store the fact that it was visited elsewhere in the list of compound
*    information.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_Visit_Store
  (
  PsvCmpInfo_t  *cmpinfos,              /* Compound infos */
  Compound_t    *cmp_p                  /* Visited compound */
  )
{
  Compound_t    *else_p;                /* Other instances */

  if (cmp_p == NULL)
    return;

  else_p = SymTab_FirstComp_Get (PstComp_SymbolTable_Get (cmp_p));
  while (else_p != NULL)
    {
    if (else_p == cmp_p)
      PsvCmpI_Visited_Set (cmpinfos[PstComp_Index_Get (else_p)]);
    else
      PsvCmpI_VstElse_Set (cmpinfos[PstComp_Index_Get (else_p)]);

    else_p = PstComp_Prev_Get (else_p);
    }

  return ;
}
/*  End of PstView_Visit_Store  */


/****************************************************************************
*
*  Function Name:                 PstVLvls_SetAll
*
*    This routine sets all three pst view levels starting at the 
*    given tree level.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstVLvls_SetAll
  (
  PstView_t     *pv_p,
  Compound_t    *cmp_p,
  U16_t          tlvl_num
  )
{
  PsvTreeLvl_t  *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t  *vlvl_p;                  /* Pst view level */
  Compound_t    *pop_p;                   /* Selected compound */
  U16_t          cmp_i;                   /* Ith compound in level */
  U16_t          num_vis;                 /* Ith compound in level */

  /*  Set up the tree levels, path and surrogate parent list, based on the 
      level of the selected node, then display them.
  */

  if (cmp_p == NULL)
    return;

  PstView_ActVLvl_Put (pv_p, PVW_LEVEL_TOP);
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP);

  /*  There are two cases:  1) we are at the root of the pst tree,
      in which case we store only the target compound in the top
      viewing level, and 2) we are somewhere in the middle of the 
      pst tree, so we should store all the subgoals in the top
      viewing level.
    */
  if (tlvl_num == 0)
    {
    /*  Set up top view level and then create two bottom levels.  */
    if (PstView_IthPTLvl_Get (pv_p, 0) != NULL)
      TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, 0));
    tlvl_p = TreeLvl_Create (NULL, 1, tlvl_num, PstView_TreeType_Get (pv_p));
    PstView_IthPTLvl_Put (pv_p, 0, tlvl_p);
    PstView_PathLen_Put (pv_p, 1);
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), tlvl_p);
    TreeLvl_IthCmpNd_Put (tlvl_p, 0, cmp_p);
    TreeLvl_SelNd_Put (tlvl_p, 0);
    TreeLvl_NSGs_Put (tlvl_p, 1);
    TreeLvl_SelSG_Put (tlvl_p, PstSubg_Index_Get (PstComp_Father_Get (cmp_p)));
    TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
    PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), cmp_p);
    TreeLvl_HeadNd_Put (tlvl_p, 
      DrawCxt_NumVsbNd_Get (PstView_PstDC_Get (pv_p)) >> 1);
    PstVLvls_SetTwo (pv_p);

    /*  Set path focus to none.  */
    tlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
    TreeLvl_SelNd_Put (tlvl_p, PVW_NODE_NONE);
    TreeLvl_SelSG_Put (tlvl_p, 0);
    TreeLvl_FocusNd_Put (tlvl_p, PVW_NODE_NONE);
    TreeLvl_FocusSG_Put (tlvl_p, 0);
    }
  else
    {
    if (PstComp_Father_Get (cmp_p) == NULL)
      {
      return ;
      }
    
    pop_p = PstSubg_Father_Get (PstComp_Father_Get (cmp_p));
    if (pop_p == NULL)
      {
      return ;
      }

    /*  Create new top level.  */
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP);
    if (PstView_IthPTLvl_Get (pv_p, tlvl_num) != NULL)
      TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, tlvl_num));
    tlvl_p = TreeLvl_Create (pop_p, 0, tlvl_num, PstView_TreeType_Get (pv_p));
    PstView_IthPTLvl_Put (pv_p, tlvl_num, tlvl_p);
    PstView_PathLen_Put (pv_p, tlvl_num + 1);
    ViewLvl_TreeLvl_Put (vlvl_p, tlvl_p);

    /* Find the location of the given compound in the array of compnodes.  */
    for (cmp_i = 0; cmp_i < TreeLvl_NCmps_Get (tlvl_p) 
        && cmp_p != TreeLvl_IthCmpNd_Get (tlvl_p, cmp_i); cmp_i++)
      ;

    TreeLvl_SelNd_Put (tlvl_p, cmp_i);
    TreeLvl_SelSG_Put (tlvl_p, 
      PstSubg_Index_Get (PstComp_Father_Get (cmp_p)));
    TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
    PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), cmp_p);
    num_vis = DrawCxt_NumVsbNd_Get (PstView_PstDC_Get (pv_p));
    if (num_vis > TreeLvl_NCmps_Get (tlvl_p))
      TreeLvl_HeadNd_Put (tlvl_p, (num_vis - TreeLvl_NCmps_Get (tlvl_p)) >> 1);
    else
      TreeLvl_HeadNd_Put (tlvl_p, 0);

    PstVLvls_SetTwo (pv_p);
    }

  return;
}
/*  End of PstVLvls_SetAll  */


/****************************************************************************
*
*  Function Name:                 PstVLvls_SetTwo
*
*    This routine sets up the bottom two pst view levels.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstVLvls_SetTwo
  (
  PstView_t     *pv_p
  )
{
  PsvTreeLvl_t  *tlvl_p;                  /* Pst tree level */
  PsvViewLvl_t  *vlvl_p;                  /* Pst view level */
  Compound_t    *selcmp_p;                /* Selected compound */
  Compound_t    *selkid_p;                /* Selected kid compound */
  U16_t          kidcmp_i;                 /* Ith compound in level */
  U16_t          tlvl_num;

  /*  Set up the bottom two pst tree levels.  */
  tlvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP));
  tlvl_num = TreeLvl_LvlNum_Get (tlvl_p);
  selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
  if (selcmp_p == NULL)
    return ;

  /*  Create new mid level.  */
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
  if (PstView_IthPTLvl_Get (pv_p, tlvl_num + 1) != NULL)
    TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, tlvl_num + 1));
  tlvl_p = TreeLvl_Create (selcmp_p, 0, tlvl_num + 1, 
    PstView_TreeType_Get (pv_p));
  PstView_IthPTLvl_Put (pv_p, tlvl_num + 1, tlvl_p);
  ViewLvl_TreeLvl_Put (vlvl_p, tlvl_p);

  /* Select a child of the subgoal with the highest merit to expand.  */
  if (TreeLvl_CmpNodes_Get (tlvl_p) != NULL)
    {
    PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) + 1);
    kidcmp_i = 0;
    selkid_p = TreeLvl_IthCmpNd_Get (tlvl_p, kidcmp_i);
    while (kidcmp_i < TreeLvl_NCmps_Get (tlvl_p) 
        && (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get 
        (selkid_p)) == NULL || SCycle_Detect (pv_p, SymTab_Index_Get
        (PstComp_SymbolTable_Get (selkid_p)), tlvl_num + 1, FALSE)))
      {
      kidcmp_i++;
      selkid_p = TreeLvl_IthCmpNd_Get (tlvl_p, kidcmp_i);
      }

    if (kidcmp_i < TreeLvl_NCmps_Get (tlvl_p))
      {
      TreeLvl_SelNd_Put (tlvl_p, kidcmp_i);
      TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (selkid_p)));
      TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
      PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selkid_p);
      }
    else
      {
      selkid_p = TreeLvl_IthCmpNd_Get (tlvl_p, 0);
      TreeLvl_SelNd_Put (tlvl_p, 0);
      TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (selkid_p)));
      TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
      PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selkid_p);
      selkid_p = NULL;
      }
    }

  else
    selkid_p = NULL;

  SHead_Center (pv_p, PVW_LEVEL_TOP, PVW_LEVEL_MID);

  /*  Create new bottom level.  */
  vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
  if (PstView_IthPTLvl_Get (pv_p, tlvl_num + 2) != NULL)
    TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, tlvl_num + 2));
  tlvl_p = TreeLvl_Create (selkid_p, 0, tlvl_num + 2, 
    PstView_TreeType_Get (pv_p));
  PstView_IthPTLvl_Put (pv_p, tlvl_num + 2, tlvl_p);
  ViewLvl_TreeLvl_Put (vlvl_p, tlvl_p);
  SHead_Center (pv_p, PVW_LEVEL_MID, PVW_LEVEL_BTM);
  if (TreeLvl_NCmps_Get (tlvl_p) > 0)
    {
    PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) + 1);
    TreeLvl_FocusNd_Put (tlvl_p, 0);
    TreeLvl_FocusSG_Put (tlvl_p, PstSubg_Index_Get (
      PstComp_Father_Get (TreeLvl_IthCmpNd_Get (tlvl_p, 0))));
    }
        
  return;
}
/*  End of PstVLvls_SetTwo  */


/****************************************************************************
*
*  Function Name:                 PstVLvls_Update
*
*    This routine updates the pst view levels based on the user's 
*    selection.  Both the tree and path are updated.
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstVLvls_Update
  (
  PstView_t     *pv_p,
  U16_t          newnode,
  U8_t           vlevel
  )
{
  PsvTreeLvl_t  *tlvl_p;                  /* Pst tree level */
  PsvTreeLvl_t  *newtlvl_p;               /* New pst tree level */
  PsvTreeLvl_t  *pthtlvl_p;
  PsvViewLvl_t  *vlvl_p;                  /* Pst view level */
  Compound_t    *selcmp_p;                /* Selected compound */
  U16_t          tlvl_num;
  U16_t          cmp_i;
  U16_t          num_bros;

  /*  Set up the tree levels and path based on the level of the selected 
      node.
  */

  if (vlevel == PVW_LEVEL_TOP)
    {
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);

    /*  There are four cases:  1) we are at the root, so we do nothing;  
        2) the user has selected the same nonroot node, in which case we 
        move up one level; 3) the user has selected a new node at the 
        top, so we need to load new middle and bottom levels; and 4) 
        the user has selected a new node at the top which has no children.
    */

    /*----------------------- case 1 --------------------------------*/

    if (TreeLvl_LvlNum_Get (tlvl_p) == 0)
      {
      PstView_ActVLvl_Put (pv_p, PVW_LEVEL_TOP);
      return FALSE;
      }

    else  /*  Same node selected in top view level, but not root node.  */
      {
      /*----------------------- case 2 --------------------------------*/

      if (newnode == TreeLvl_SelNd_Get (tlvl_p))
        {
        PsvTreeLvl_t  *switchtl_p;             /* Pst tree level */

        /*  Move up one level.  */
        PstView_ActVLvl_Put (pv_p, PVW_LEVEL_MID);
        switchtl_p = PstView_IthPTLvl_Get (pv_p, 
          TreeLvl_LvlNum_Get (tlvl_p) - 1);
        ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP),
          switchtl_p);
        TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
        TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

        switchtl_p = PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p));
        ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID),
          switchtl_p);
        TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
        TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

        switchtl_p = PstView_IthPTLvl_Get (pv_p, 
          TreeLvl_LvlNum_Get (tlvl_p) + 1);
        ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM),
          switchtl_p);
        TreeLvl_SelNd_Put (switchtl_p, PVW_NODE_NONE);
        TreeLvl_SelSG_Put (switchtl_p, 0);
        if (TreeLvl_NCmps_Get (switchtl_p) > 0)
          {
          PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) - 1);
          TreeLvl_FocusNd_Put (switchtl_p, 0);
          TreeLvl_FocusSG_Put (switchtl_p, PstSubg_Index_Get (
            PstComp_Father_Get (TreeLvl_IthCmpNd_Get (switchtl_p, 0))));
          }
        else
          {
          TreeLvl_FocusNd_Put (switchtl_p, PVW_NODE_NONE);
          TreeLvl_FocusSG_Put (switchtl_p, 0);
          }
        }

  /*----------------------- cases 3 & 4 ------------------------------*/

      else  /* A new node at the top level was selected. */
        {
        PstView_ActVLvl_Put (pv_p, PVW_LEVEL_TOP);

        /*  Modify the top tree level and replace the middle and bottom  
            levels.
        */ 
        selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, newnode);
        if (SCycle_Detect (pv_p, SymTab_Index_Get (PstComp_SymbolTable_Get (
            selcmp_p)), TreeLvl_LvlNum_Get (tlvl_p), TRUE))
          {
          PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE);
          return FALSE;
          }

        SSelSym_Undraw (vlvl_p);
        TreeLvl_SelNd_Put (tlvl_p, newnode);
        SSelSym_Draw (vlvl_p);
        TreeLvl_SelSG_Put (tlvl_p, 
          PstSubg_Index_Get (PstComp_Father_Get (selcmp_p)));
        TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
        TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
        PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selcmp_p);
        PstView_PathLen_Put (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1);
        PstVLvls_SetTwo (pv_p);
        if (TreeLvl_NCmps_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (
            pv_p, PVW_LEVEL_MID))) == 0)
          {
          PsvTreeLvl_t  *switchtl_p;

          /*  Move up one level.  */
          PstView_ActVLvl_Put (pv_p, PVW_LEVEL_MID);
          switchtl_p = PstView_IthPTLvl_Get (pv_p, 
            TreeLvl_LvlNum_Get (tlvl_p) - 1);
          ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP),
            switchtl_p);
          TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
          TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

          switchtl_p = PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p));
          ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID),
            switchtl_p);
          TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
          TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

          switchtl_p = PstView_IthPTLvl_Get (pv_p, 
            TreeLvl_LvlNum_Get (tlvl_p) + 1);
          ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM),
            switchtl_p);
          TreeLvl_SelNd_Put (switchtl_p, PVW_NODE_NONE);
          TreeLvl_SelSG_Put (switchtl_p, 0);
          if (TreeLvl_NCmps_Get (switchtl_p) > 0)
            {
            TreeLvl_FocusNd_Put (switchtl_p, 0);
            TreeLvl_FocusSG_Put (switchtl_p, PstSubg_Index_Get (
              PstComp_Father_Get (TreeLvl_IthCmpNd_Get (switchtl_p, 0))));
            }
          else
            {
            TreeLvl_FocusNd_Put (switchtl_p, PVW_NODE_NONE);
            TreeLvl_FocusSG_Put (switchtl_p, 0);
            }
          }
        }  /* End of else new node at top level */
      }  /* End of else not root node */
    }  /*  End of if top level */

  /*--------------------------------------------------------------------*/
  /*--------------------------------------------------------------------*/
  else if (vlevel == PVW_LEVEL_MID)
    {
    /*  This is the simplest case:  if the new node is different from
        the old one, we replace the bottom level with a new level.  If 
        the node is the same and there is a selected node in the bottom 
        level, we unselect the bottom level node--this allows the user 
        to change the reaction that is displayed without moving up the tree.
        In either case, the path doesn't change so we can just return.
    */
    PstView_ActVLvl_Put (pv_p, PVW_LEVEL_MID);
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    if (newnode == TreeLvl_SelNd_Get (tlvl_p))
      {
      PsvTreeLvl_t  *bottlvl_p;             /* Pst tree level */

      bottlvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, 
        PVW_LEVEL_BTM));
      if (TreeLvl_SelNd_Get (bottlvl_p) != PVW_NODE_NONE)
        {
        TreeLvl_SelNd_Put (bottlvl_p, PVW_NODE_NONE);
        TreeLvl_SelSG_Put (bottlvl_p, 0);
        return TRUE;
        }
      else
        return FALSE;
      }
    
    selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, newnode);
    if (SCycle_Detect (pv_p, SymTab_Index_Get (PstComp_SymbolTable_Get (
        selcmp_p)), TreeLvl_LvlNum_Get (tlvl_p), TRUE))
      {
      PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE);
      return FALSE;
      }

    SSelSym_Undraw (vlvl_p);
    TreeLvl_SelNd_Put (tlvl_p, newnode);
    TreeLvl_SelSG_Put (tlvl_p, 
      PstSubg_Index_Get (PstComp_Father_Get (selcmp_p)));
    PstView_PathLen_Put (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1);
    SSelSym_Draw (vlvl_p);

    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
    if (PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1) != NULL)
      TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, 
        TreeLvl_LvlNum_Get (tlvl_p) + 1));
    newtlvl_p = TreeLvl_Create (selcmp_p, 0, 
      TreeLvl_LvlNum_Get (tlvl_p) + 1, PstView_TreeType_Get (pv_p));
    PstView_IthPTLvl_Put (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1, 
      newtlvl_p);
    ViewLvl_TreeLvl_Put (vlvl_p, newtlvl_p);
    SHead_Center (pv_p, PVW_LEVEL_MID, PVW_LEVEL_BTM);
    PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selcmp_p);
    if (TreeLvl_NCmps_Get (newtlvl_p) > 0)
      {
      TreeLvl_FocusNd_Put (newtlvl_p, 0);
      TreeLvl_FocusSG_Put (newtlvl_p, PstSubg_Index_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (newtlvl_p, 0))));
      PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) + 1);
      }

    return TRUE;
    }

  /*--------------------------------------------------------------------*/
  /*--------------------------------------------------------------------*/
  else if (vlevel == PVW_LEVEL_BTM)
    {
    /*  There are three cases:  the new selected node is the same as the 
        old selected node, in which case we do nothing,  Otherwise, if the 
        node has children then we move down one level.  If the node has no 
        children, we simply show that the node was selected so that the 
        reaction will be displayed and it can be included in a path trace.
    */
    PstView_ActVLvl_Put (pv_p, PVW_LEVEL_BTM);
    vlvl_p = PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);

  /*----------------------- case 1 --------------------------------*/
    if (newnode == TreeLvl_SelNd_Get (tlvl_p))
      {
      return FALSE;
      }
    
    selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, newnode);
    if (SCycle_Detect (pv_p, SymTab_Index_Get (PstComp_SymbolTable_Get (
        selcmp_p)), TreeLvl_LvlNum_Get (tlvl_p), TRUE))
      {
      PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE);
      return FALSE;
      }

  /*----------------------- case 2 --------------------------------*/

    if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (selcmp_p)) == NULL)
      {
      SSelSym_Undraw (vlvl_p);
      TreeLvl_SelNd_Put (tlvl_p, newnode);
      TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (selcmp_p)));
      TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
      SSelSym_Draw (vlvl_p);
      PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selcmp_p);
     
     /*  Also make sure middle level focus is moved to selected node.  */
     tlvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID));
     TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
     TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
     }

  /*----------------------- case 3 --------------------------------*/
    else
      {
      PsvTreeLvl_t  *switchtl_p;

      /*  Move down one level.  */
      SSelSym_Undraw (vlvl_p);
      PstView_ActVLvl_Put (pv_p, PVW_LEVEL_MID);
      TreeLvl_SelNd_Put (tlvl_p, newnode);
      TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (selcmp_p)));
      TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
      SSelSym_Draw (vlvl_p);

      switchtl_p = PstView_IthPTLvl_Get (pv_p, 
        TreeLvl_LvlNum_Get (tlvl_p) - 1);
      ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP),
        switchtl_p);
      TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
      TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

      switchtl_p = PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p));
      ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID),
        switchtl_p);
      TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
      TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

      if (PstView_IthPTLvl_Get (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1) != NULL)
        TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, 
          TreeLvl_LvlNum_Get (tlvl_p) + 1));
      newtlvl_p = TreeLvl_Create (selcmp_p, 0, 
        TreeLvl_LvlNum_Get (tlvl_p) + 1, PstView_TreeType_Get (pv_p));
      PstView_IthPTLvl_Put (pv_p, TreeLvl_LvlNum_Get (tlvl_p) + 1, newtlvl_p);
      ViewLvl_TreeLvl_Put (vlvl_p, newtlvl_p);
      SHead_Center (pv_p, PVW_LEVEL_MID, PVW_LEVEL_BTM);
      PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selcmp_p);
      if (TreeLvl_NCmps_Get (newtlvl_p) > 0)
        {
        TreeLvl_FocusNd_Put (newtlvl_p, 0);
        TreeLvl_FocusSG_Put (newtlvl_p, PstSubg_Index_Get (
          PstComp_Father_Get (TreeLvl_IthCmpNd_Get (newtlvl_p, 0))));
        PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) + 1);
        }
      }
    }

  /*--------------------------------------------------------------------*/
  /*--------------------------------------------------------------------*/
  else if (vlevel == PVW_LEVEL_PATH)
    {
    /*  Move tree levels up.  If the new node is already a selected
        node on the path, we don't need to worry about creating tree
        levels.  Otherwise, we have to setup the bottom two levels.
    */
    PstView_ActVLvl_Put (pv_p, PVW_LEVEL_TOP);
    pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
    selcmp_p = TreeLvl_IthCmpNd_Get (pthtlvl_p, newnode);
    tlvl_num = PstView_IthPthLN_Get (pv_p, newnode);
    tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num);
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), tlvl_p);
    TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
    PstView_PathLen_Put (pv_p, tlvl_num + 1);
    if (selcmp_p == TreeLvl_IthCmpNd_Get (tlvl_p, 
        TreeLvl_SelNd_Get (tlvl_p)))
      {
      PsvTreeLvl_t  *switchtl_p;             /* Pst tree level */

      switchtl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num + 1);
      ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID),
        switchtl_p);
      TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
      TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));

      switchtl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num + 2);
      ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM),
        switchtl_p);
      TreeLvl_SelNd_Put (switchtl_p, PVW_NODE_NONE);
      TreeLvl_SelSG_Put (switchtl_p, 0);
      TreeLvl_FocusNd_Put (switchtl_p, 0);
      TreeLvl_FocusSG_Put (switchtl_p, PstSubg_Index_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (switchtl_p, 0))));
      PstView_PathLen_Put (pv_p, PstView_PathLen_Get (pv_p) + 2);
      }  /* End of if newnode on path */

    else 
      {
      U16_t          nd_i;

      /*  Modify the top tree level and replace the middle and bottom  
          levels.
      */ 
      if (SCycle_Detect (pv_p, SymTab_Index_Get (PstComp_SymbolTable_Get (
          selcmp_p)), tlvl_num, TRUE))
        {
        SSelSym_Undraw (PstView_PathVLvl_Get (pv_p));
        PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE);
        return FALSE;
        }

      /*  Find position of new selected node in tree level.  */
      for (nd_i = 0; nd_i < TreeLvl_NCmps_Get (tlvl_p)
           && selcmp_p != TreeLvl_IthCmpNd_Get (tlvl_p, nd_i) ; nd_i++)
        ;

      TreeLvl_SelNd_Put (tlvl_p, nd_i);
      TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (selcmp_p)));
      TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
      PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), selcmp_p);
      PstVLvls_SetTwo (pv_p);
      if (TreeLvl_NCmps_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (
          pv_p, PVW_LEVEL_MID))) == 0)
        {
        PsvTreeLvl_t  *switchtl_p;

        /*  Move up one level.  */
        PstView_ActVLvl_Put (pv_p, PVW_LEVEL_MID);
        switchtl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num - 1);
        ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP),
          switchtl_p);
        TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
        TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));
        switchtl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num);
        ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID),
          switchtl_p);
        TreeLvl_FocusNd_Put (switchtl_p, TreeLvl_SelNd_Get (switchtl_p));
        TreeLvl_FocusSG_Put (switchtl_p, TreeLvl_SelSG_Get (switchtl_p));
        switchtl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num + 1);
        ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_BTM),
          switchtl_p);
        TreeLvl_SelNd_Put (switchtl_p, PVW_NODE_NONE);
        TreeLvl_SelSG_Put (switchtl_p, 0);
        if (TreeLvl_NCmps_Get (switchtl_p) > 0)
          {
          TreeLvl_FocusNd_Put (switchtl_p, 0);
          TreeLvl_FocusSG_Put (switchtl_p, PstSubg_Index_Get (
            PstComp_Father_Get (TreeLvl_IthCmpNd_Get (switchtl_p, 0))));
          }
        else
          {
          TreeLvl_FocusNd_Put (switchtl_p, PVW_NODE_NONE);
          TreeLvl_FocusSG_Put (switchtl_p, 0);
          }
        }
      }  /* End of else new node at top level */
    }

  /*--------------------------------------------------------------------*/
  /*--------------------------------------------------------------------*/

  /*  Update the path.  */
  pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
  tlvl_num = TreeLvl_LvlNum_Get (ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (
    pv_p, PVW_LEVEL_TOP)));
  if (tlvl_num < overflow_level + num_overflowed)
    {
    if (tlvl_num <= overflow_level)
      {
      free (path_overflow);
      path_overflow = NULL;
      num_overflowed = overflow_level = 0;
/*
printf("overflow freed\n");
*/
      }
    else
      {
      num_overflowed = tlvl_num - overflow_level;
      path_overflow = (Subgoal_t **) realloc (path_overflow, num_overflowed * sizeof (Subgoal_t *));
/*
printf("overflows beyond %d reduced to %d\n",overflow_level,num_overflowed);
*/
      }
    }
  if (tlvl_num < TreeLvl_NSGs_Get (pthtlvl_p))
    {
    /*  Remove extra subgoals by skipping left past the compounds with 
        a higher level.  
    */ 
    cmp_i = TreeLvl_NCmps_Get (pthtlvl_p) - 1;
    while (cmp_i > 0 && PstView_IthPthLN_Get (pv_p, cmp_i) >= tlvl_num)
      cmp_i--;

    if (PstView_IthPthLN_Get (pv_p, cmp_i) >= tlvl_num)
      TreeLvl_NCmps_Put (pthtlvl_p, cmp_i);
    else
      TreeLvl_NCmps_Put (pthtlvl_p, cmp_i + 1);

    TreeLvl_NSGs_Put (pthtlvl_p, tlvl_num);
    if (TreeLvl_FocusNd_Get (pthtlvl_p) >= TreeLvl_NCmps_Get (pthtlvl_p))
      {
      if (TreeLvl_NCmps_Get (pthtlvl_p) > 0)
        {
        PsvTreeLvl_t  *psttlvl_p;                /* Tree level */ 
        U16_t          nd_i;
        
        nd_i = TreeLvl_NCmps_Get (pthtlvl_p) - 1;
        psttlvl_p = PstView_IthPTLvl_Get (pv_p, PstView_IthPthLN_Get (pv_p, 
        nd_i));
        while (TreeLvl_IthCmpNd_Get (pthtlvl_p, nd_i) 
            != TreeLvl_IthCmpNd_Get (psttlvl_p, TreeLvl_SelNd_Get (psttlvl_p)))
          {
          nd_i--;
          psttlvl_p = PstView_IthPTLvl_Get (pv_p, PstView_IthPthLN_Get (pv_p, 
          nd_i));
          }

        TreeLvl_FocusNd_Put (pthtlvl_p, nd_i);
        TreeLvl_FocusSG_Put (pthtlvl_p, PstSubg_Index_Get (
            PstComp_Father_Get (TreeLvl_IthCmpNd_Get (pthtlvl_p, nd_i))));
        }
      else
        {
        TreeLvl_FocusNd_Put (pthtlvl_p, PVW_NODE_NONE);
        TreeLvl_FocusSG_Put (pthtlvl_p, 0);
        }
      }
    }
  
  else if (TreeLvl_NSGs_Get (pthtlvl_p) < tlvl_num)
    {
    DrawCxt_t       *pthdc_p;               
    Compound_t      *cmp_p;                   /* Selected compound */
    U16_t            tlvl_i;
    U16_t            selcmp_i;

    /*  Add subgoals to path.  */
    pthdc_p = ViewLvl_DrawCxt_Get (PstView_PathVLvl_Get (pv_p));
    tlvl_i = TreeLvl_NSGs_Get (pthtlvl_p);
    tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_i);
    selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    num_bros = PstSubg_NumSons_Get (PstComp_Father_Get (selcmp_p));
    cmp_i = TreeLvl_NCmps_Get (pthtlvl_p);
    selcmp_i = PVW_NODE_NONE;
    while (tlvl_i < tlvl_num && num_bros + cmp_i 
        < DrawCxt_NumVsbNd_Get (pthdc_p))
      {
      cmp_p = PstSubg_Son_Get (PstComp_Father_Get (selcmp_p));
      while (cmp_p != NULL)
        {
        TreeLvl_IthCmpNd_Put (pthtlvl_p, cmp_i, cmp_p);
        PstView_IthPthLN_Put (pv_p, cmp_i, tlvl_i);
        if (cmp_p == selcmp_p)
          selcmp_i = cmp_i;

        cmp_p = PstComp_Brother_Get (cmp_p);
        cmp_i++;
        }

      tlvl_i++;
      tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_i);
      selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      num_bros = PstSubg_NumSons_Get (PstComp_Father_Get (selcmp_p));
      }

    if (tlvl_i < tlvl_num)
      {
      if (path_overflow == NULL)
        {
if (tlvl_num-tlvl_i!=1) printf("error in allocation logic\n");
        num_overflowed = 1;
        overflow_level = tlvl_i;
        path_overflow = (Subgoal_t **) malloc (sizeof (Subgoal_t *));
/*
printf("path_overflow initialized at level %d\n",overflow_level);
*/
        }
      else if (tlvl_num-tlvl_i > num_overflowed)
        {
        num_overflowed++;
        path_overflow = (Subgoal_t **) realloc (path_overflow, num_overflowed * sizeof (Subgoal_t *));
/*
printf("path_overflow reallocated to %d\n",num_overflowed);
*/
        }
      tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_num - 1);
      selcmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
      path_overflow[num_overflowed - 1] = PstComp_Father_Get (selcmp_p);
      }

    TreeLvl_NSGs_Put (pthtlvl_p, tlvl_i);
    TreeLvl_NCmps_Put (pthtlvl_p, cmp_i);
    if (selcmp_i != PVW_NODE_NONE)
      {
      TreeLvl_FocusNd_Put (pthtlvl_p, selcmp_i);
      TreeLvl_FocusSG_Put (pthtlvl_p, PstSubg_Index_Get (
        PstComp_Father_Get (TreeLvl_IthCmpNd_Get (pthtlvl_p, selcmp_i))));
      }
    }

  return TRUE;
}
/*  End of PstVLvls_Update  */


/****************************************************************************
*
*  Function Name:                 TreeLvl_Create
*
*    This routine allocates memory for and initializes the tree view data 
*    structure and the list of compound pointers.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
PsvTreeLvl_t *TreeLvl_Create
  (
  Compound_t    *pop_p,                   /* Parent compound */
  U16_t          num_nodes,               /* Number of nodes to initialize */ 
  U16_t          level_num,               /* Tree level number */ 
  U8_t           tree_type                /* Tree type */ 
  )
{
  Compound_t   **cmp_pp;                  /* Temp cmp ptr array ptr */
  PsvTreeLvl_t  *tlvl_p;                  /* Pst view level */

  tlvl_p = (PsvTreeLvl_t *) malloc (PSVTREELVL_SIZE);
  if (tlvl_p == NULL)
    {
    fprintf (stderr, 
      "TreeLvl_Create:  unable to allocate memory for tree level.\n");
    exit (-1);
    }

  if (pop_p != NULL 
      && SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (pop_p)) != NULL)
    {
    Compound_t  *tmp_cmp_p;              /* Temp compound ptr */
    Subgoal_t  **sgl_pp;                 /* Temp subgoal ptr array ptr */
    Subgoal_t   *tmp_sg_p;               /* Temp subgoal ptr */
    S16_t        c_i;                    /* Counts index */
    U16_t        cnts[103];              /* Merit counts (and indices) */
    U16_t        index, offset;          /* Index and offset into cmp array */
    U16_t        sg_i, num_sgls;         /* Ith subgoal and number of sgls */
    U16_t        num_cmps;               /* Ith cmp and number of cmps */

    /*  Determine the number of subgoals.  Create and sort the list of 
        subgoals, then create and initialize the array of compound ptrs.
    */
    pop_p = SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (pop_p));
    num_sgls = PstComp_NumSons_Get (pop_p);
    if (num_sgls > 0)
      {
      sgl_pp = (Subgoal_t **) malloc (num_sgls * sizeof (Subgoal_t *));
      if (sgl_pp == NULL)
        {
        fprintf (stderr, 
          "TreeLvl_Create:  unable to allocate memory for subgoals.\n");
        exit (-1);
        }

      /*  Since we know the range of subgoal merits (-2 .. 100), we can  
          sort the subgoals by counting the number of occurrences of each   
          merit value, and using the counts to index into the sorted array.
      */
      for (c_i = 0; c_i < 103; c_i++)  cnts[c_i] = 0;
      tmp_sg_p = PstComp_Son_Get (pop_p);
      while (tmp_sg_p != NULL)
        {
        cnts[((PstSubg_Merit_Solved_Get (tmp_sg_p) != SUBG_MERIT_INIT)
          ? PstSubg_Merit_Solved_Get (tmp_sg_p) 
          : PstSubg_Merit_Main_Get (tmp_sg_p)) + 2]++;
        tmp_sg_p = PstSubg_Brother_Get (tmp_sg_p);
        }

      /*  Sort in increasing order.  */
      for (index = 0, c_i = 102; c_i >= 0; c_i--)  
        {
        offset = cnts[c_i];
        cnts[c_i] = index;
        index += offset;
        }

      if (index != num_sgls)
        fprintf (stderr, 
          "TreeLvl_Create:  oops, index (%1u) != num_sgls (%1u).\n",
          index, num_sgls);

      tmp_sg_p = PstComp_Son_Get (pop_p);
      while (tmp_sg_p != NULL)
        {
        sgl_pp[cnts[((PstSubg_Merit_Solved_Get (tmp_sg_p) != SUBG_MERIT_INIT)
          ? PstSubg_Merit_Solved_Get (tmp_sg_p) 
          : PstSubg_Merit_Main_Get (tmp_sg_p)) + 2]++] = tmp_sg_p;
        tmp_sg_p = PstSubg_Brother_Get (tmp_sg_p);
        }

        /*  Now that the subgoals are sorted, store the compound ptrs.  */
      if (tree_type == PVW_TREETYPE_FULL)
        {
        /*  All compounds are displayed in a full tree.  */
        num_cmps = PstComp_NumGrandSons_Get (pop_p);
        if (num_cmps > 0)
          {
          cmp_pp = (Compound_t **) malloc (num_cmps * sizeof (Compound_t *));
          if (cmp_pp == NULL)
            {
            fprintf (stderr, 
              "TreeLvl_Create:  unable to allocate memory for nodes.\n");
            exit (-1);
            }

          TreeLvl_CmpNodes_Put (tlvl_p, cmp_pp);
          for (sg_i = 0; sg_i < num_sgls; sg_i++)  
            {
            tmp_cmp_p = PstSubg_Son_Get (sgl_pp[sg_i]);
            while (tmp_cmp_p != NULL)
              {
              *(cmp_pp++) = tmp_cmp_p;
              tmp_cmp_p = PstComp_Brother_Get (tmp_cmp_p);
              }
            }
          }

        else
          TreeLvl_CmpNodes_Put (tlvl_p, NULL);

        }  /* End of if tree type is full */

       if (tree_type == PVW_TREETYPE_DEV)
        {
        Boolean_t    dev_kid;
        U16_t        num_dsgs;               /* Num of dev sbgoals */

        /*  Only those subgoals with at least one developed compound 
            are used.
        */ 
        num_cmps = 0;
        num_dsgs = 0;
        for (sg_i = 0; sg_i < num_sgls; sg_i++)  
          {
          dev_kid = FALSE;
          tmp_cmp_p = PstSubg_Son_Get (sgl_pp[sg_i]);
          while (!dev_kid && tmp_cmp_p != NULL)
            {
            if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (tmp_cmp_p)) 
                != NULL)
              dev_kid = TRUE;
            tmp_cmp_p = PstComp_Brother_Get (tmp_cmp_p);
            }
          
          if (dev_kid || Pst_All_Solved (sgl_pp[sg_i]))
            {
            num_cmps += PstSubg_NumSons_Get (sgl_pp[sg_i]);
            num_dsgs++;
            }
          else
            sgl_pp[sg_i] = NULL;
          }

        if (num_cmps > 0)
          {
          cmp_pp = (Compound_t **) malloc (num_cmps * sizeof (Compound_t *));
          if (cmp_pp == NULL)
            {
            fprintf (stderr, 
              "TreeLvl_Create:  unable to allocate memory for nodes.\n");
            exit (-1);
            }

          TreeLvl_CmpNodes_Put (tlvl_p, cmp_pp);
          for (sg_i = 0; sg_i < num_sgls; sg_i++)  
            {
            if (sgl_pp[sg_i] != NULL)
              {
              tmp_cmp_p = PstSubg_Son_Get (sgl_pp[sg_i]);
              while (tmp_cmp_p != NULL)
                {
                *(cmp_pp++) = tmp_cmp_p;
                tmp_cmp_p = PstComp_Brother_Get (tmp_cmp_p);
                }
              }
            }
          }

        else
          TreeLvl_CmpNodes_Put (tlvl_p, NULL);
           
        num_sgls = num_dsgs;
        }  /* End of if tree type is developed */

      if (tree_type == PVW_TREETYPE_SOLV)
        {
        U16_t        num_ssgs;               /* Num of solved sbgoals */

        /*  Only those subgoals with all compounds either solved or 
            available are used.
        */ 
        num_cmps = 0;
        num_ssgs = 0;
        for (sg_i = 0; sg_i < num_sgls; sg_i++)  
          {
          if (Pst_All_Solved (sgl_pp[sg_i]))
            {
            num_cmps += PstSubg_NumSons_Get (sgl_pp[sg_i]);
            num_ssgs++;
            }
          else
            sgl_pp[sg_i] = NULL;
          }

        if (num_cmps > 0)
          {
          cmp_pp = (Compound_t **) malloc (num_cmps * sizeof (Compound_t *));
          if (cmp_pp == NULL)
            {
            fprintf (stderr, 
              "TreeLvl_Create:  unable to allocate memory for nodes.\n");
            exit (-1);
            }

          TreeLvl_CmpNodes_Put (tlvl_p, cmp_pp);
          for (sg_i = 0; sg_i < num_sgls; sg_i++)  
            {
            if (sgl_pp[sg_i] != NULL)
              {
              tmp_cmp_p = PstSubg_Son_Get (sgl_pp[sg_i]);
              while (tmp_cmp_p != NULL)
                {
                *(cmp_pp++) = tmp_cmp_p;
                tmp_cmp_p = PstComp_Brother_Get (tmp_cmp_p);
                }
              }
            }
          }

        else
          TreeLvl_CmpNodes_Put (tlvl_p, NULL);

        num_sgls = num_ssgs;
        }  /* End of if tree type is solved */

       free (sgl_pp);
        }  /* If nonzero number of subgoals  */

      else
        {
        TreeLvl_CmpNodes_Put (tlvl_p, NULL);
        num_cmps = 0;
        }

      TreeLvl_NCmps_Put (tlvl_p, num_cmps);
      TreeLvl_NSGs_Put (tlvl_p, num_sgls);
      TreeLvl_LvlNum_Put (tlvl_p, level_num);
      TreeLvl_FocusNd_Put (tlvl_p, PVW_NODE_NONE);
      TreeLvl_FocusSG_Put (tlvl_p, 0);
      TreeLvl_HeadNd_Put (tlvl_p, 0);
      TreeLvl_SelNd_Put (tlvl_p, PVW_NODE_NONE);
      TreeLvl_SelSG_Put (tlvl_p, 0);
    }  /* End of if non-null pop compound pointer */

  else  /*  Parent compound is NULL */
    {
    if (num_nodes > 0)
      {
      cmp_pp = (Compound_t **) malloc (num_nodes * sizeof (Compound_t *));
      if (cmp_pp == NULL)
        {
        fprintf (stderr, 
          "PstView_Create:  unable to allocate memory cmp nodes.\n");
        exit (-1);
        }

      TreeLvl_CmpNodes_Put (tlvl_p, cmp_pp);
      }

    else
      {
      TreeLvl_CmpNodes_Put (tlvl_p, NULL);
      }

    TreeLvl_NCmps_Put (tlvl_p, num_nodes);
    TreeLvl_NSGs_Put (tlvl_p, 0);
    TreeLvl_LvlNum_Put (tlvl_p, level_num);
    TreeLvl_FocusNd_Put (tlvl_p, PVW_NODE_NONE);
    TreeLvl_FocusSG_Put (tlvl_p, 0);
    TreeLvl_HeadNd_Put (tlvl_p, 0);
    TreeLvl_SelNd_Put (tlvl_p, PVW_NODE_NONE);
    TreeLvl_SelSG_Put (tlvl_p, 0);
    }

  return (tlvl_p);
}
/*  End of TreeLvl_Create  */


/****************************************************************************
*
*  Function Name:                 TreeLvl_Destroy
*
*    Free the array of compound pointers and then the tree view
*    data structure itself.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void TreeLvl_Destroy
  (
  PsvTreeLvl_t  *tlvl_p                 /* Pst tree level */
  )
{
  if (TreeLvl_CmpNodes_Get (tlvl_p) != NULL)
    free (TreeLvl_CmpNodes_Get (tlvl_p));

  free (tlvl_p);
  return ;
}
/*  End of TreeLvl_Destroy  */


/****************************************************************************
*
*  Function Name:                 TreeLvls_Backup
*
*    Back up the path from the given node to root node storing the new 
*    tree levels.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U16_t TreeLvls_Backup
  (
  PstView_t       *pv_p,                  /* Pst view */
  Compound_t      *selcmp_p
  )
{
  Compound_t      *cmp_p;
  Compound_t      *pop_p;
  PsvTreeLvl_t    *tlvl_p;                /* Pst tree level */
  PsvTreeLvl_t    *pthtlvl_p;             /* Path tree level */
  DrawCxt_t       *pthdc_p;               
  U16_t            node_i;
  U16_t            tlvl_i, last_tlvl;
  U16_t            cmp_i;
  U16_t            num_bros;
  Boolean_t        add_more;

  /*  First back up the path from the given node to root node storing
      the new tree levels.
  */
  last_tlvl = PstSubg_Level_Get (PstComp_Father_Get (selcmp_p));
  cmp_p = selcmp_p;
  pop_p = PstSubg_Father_Get (PstComp_Father_Get (cmp_p));
  for (tlvl_i = last_tlvl; tlvl_i > 0; tlvl_i--)
    {
    if (PstView_IthPTLvl_Get (pv_p, tlvl_i) != NULL)
      TreeLvl_Destroy (PstView_IthPTLvl_Get (pv_p, tlvl_i));
    tlvl_p = TreeLvl_Create (pop_p, 0, tlvl_i, PstView_TreeType_Get (pv_p));
    for (node_i = 0; node_i < TreeLvl_NCmps_Get (tlvl_p) 
        && cmp_p != TreeLvl_IthCmpNd_Get (tlvl_p, node_i); node_i++)
      ;

    TreeLvl_SelNd_Put (tlvl_p, node_i);
    TreeLvl_SelSG_Put (tlvl_p, 
        PstSubg_Index_Get (PstComp_Father_Get (cmp_p)));
    TreeLvl_FocusNd_Put (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    TreeLvl_FocusSG_Put (tlvl_p, TreeLvl_SelSG_Get (tlvl_p));
    PstView_Visit_Store (PstView_CmpInfo_Get (pv_p), cmp_p);
    PstView_IthPTLvl_Put (pv_p, tlvl_i, tlvl_p);

    cmp_p = pop_p;
    pop_p = PstSubg_Father_Get (PstComp_Father_Get (cmp_p));
    }

  /*  Move back down the path centering the levels and updating path tree
      level.  
  */
  pthtlvl_p = ViewLvl_TreeLvl_Get (PstView_PathVLvl_Get (pv_p));
  pthdc_p = ViewLvl_DrawCxt_Get (PstView_PathVLvl_Get (pv_p));
  add_more = TRUE;
  for (cmp_i = 0, tlvl_i = 0; tlvl_i < last_tlvl; tlvl_i++)
    {
    tlvl_p = PstView_IthPTLvl_Get (pv_p, tlvl_i);
    cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p));
    num_bros = PstSubg_NumSons_Get (PstComp_Father_Get (cmp_p));
    if (add_more && cmp_i + num_bros < DrawCxt_NumVsbNd_Get (pthdc_p))
      {
      cmp_p = PstSubg_Son_Get (PstComp_Father_Get (cmp_p));
      while (cmp_p != NULL)
        {
        TreeLvl_IthCmpNd_Put (pthtlvl_p, cmp_i, cmp_p);
        PstView_IthPthLN_Put (pv_p, cmp_i, tlvl_i);
        cmp_p = PstComp_Brother_Get (cmp_p);
        cmp_i++;
        }
      }
    else
      add_more = FALSE;

    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), 
      PstView_IthPTLvl_Get (pv_p, tlvl_i));
    ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_MID), 
      PstView_IthPTLvl_Get (pv_p, tlvl_i + 1));
    SHead_Center (pv_p, PVW_LEVEL_TOP, PVW_LEVEL_MID);
    }

  TreeLvl_NCmps_Put (pthtlvl_p, cmp_i);
  TreeLvl_NSGs_Put (pthtlvl_p, tlvl_i);

  ViewLvl_TreeLvl_Put (PstView_IthVLvl_Get (pv_p, PVW_LEVEL_TOP), 
    PstView_IthPTLvl_Get (pv_p, last_tlvl));

  return (last_tlvl);
}
/*  End of TreeLvls_Backup  */


/****************************************************************************
*
*  Function Name:                 SCycle_Detect
*
*    Search the select nodes along the current path to determine
*    if the given compound index is already present.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t SCycle_Detect
  (
  PstView_t     *pv_p,                  /* Pst view */
  U32_t          cmp_ind,               /* Index of selected compound */
  U8_t           tlvl,                  /* Tree level of selected cmp */
  Boolean_t      mess
  )
{
  static char    buff[128];

  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */
  U8_t           lvl_i;                 /* Ith level */
  Boolean_t      found;

  found = FALSE;
  lvl_i = 0;
  while (!found && lvl_i < tlvl)
    {
    tlvl_p = PstView_IthPTLvl_Get (pv_p, lvl_i);
    found = (cmp_ind == SymTab_Index_Get (PstComp_SymbolTable_Get (
      TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p)))));
    lvl_i++;
    }

  /*  If found, then display message if requested.  */
  if (found && mess)
    {
    sprintf (buff, "%s%1hu.", PVW_CYCLE_WARN, lvl_i - 1);
    InfoWarn_Show (buff);
    }

  return (found);
}
/*  End of SCycle_Detect  */


/****************************************************************************
*
*  Function Name:                 SHead_Center
*
*    Center the list of compounds in the given level, and store the head 
*    position of the compound list.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SHead_Center
  (
  PstView_t     *pv_p,                  /* Pst view */
  U8_t           plevel,                /* Parent level */
  U8_t           klevel                 /* Level to center */
  )
{
  PsvTreeLvl_t  *klvl_p;                /* Pst level to center */
  PsvTreeLvl_t  *plvl_p;                /* Pst parent level */
  U16_t          leftmost;              /* Left most visible node */
  U16_t          numvis;                /* number of visible nodes */
  U16_t          pcenter;               /* Parent center position */
  U16_t          pivot;                 /* Pivot position of level */
  U16_t          head;                  /* Head position */
  U16_t          shift;                 /* Position shift */

  if (pv_p == NULL)
    return;

  klvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, klevel));
  plvl_p = ViewLvl_TreeLvl_Get (PstView_IthVLvl_Get (pv_p, plevel));
  if (klvl_p == NULL || plvl_p == NULL)
    return;

  if (TreeLvl_SelNd_Get (plvl_p) == PVW_NODE_NONE)
    {
    TreeLvl_HeadNd_Put (klvl_p, 0);
    return;
    }

  numvis = DrawCxt_NumVsbNd_Get (PstView_PstDC_Get (pv_p));
  if (numvis > 0 
      && PstView_LeftEdge_Get (pv_p) % AppDim_PstSymTotWd_Get (&GAppDim) 
        < AppDim_PstSymSepWd_Get (&GAppDim) - 1)
    numvis -= 1;

  leftmost = PstView_LeftNd_Get (pv_p);
  pcenter = TreeLvl_HeadNd_Get (plvl_p) + TreeLvl_SelNd_Get (plvl_p);
  pivot = TreeLvl_NCmps_Get (klvl_p) >> 1;
  shift = (pcenter + pivot > leftmost + numvis) ? (pcenter + pivot) 
    - (leftmost + numvis) : 0;
  if (pcenter - pivot > 0)
    {
    head = pcenter - pivot;
    if (head - shift >= 0)
      head -= shift;
    else
      head = 0;
    }
  else
    head = 0;

  TreeLvl_HeadNd_Put (klvl_p, head);
  return ;
}
/*  End of SHead_Center  */


/****************************************************************************
*
*  Function Name:                 SLines_Draw
*
*    This routine draws the lines between the given parent level 
*    and given kid level of the pst viewer.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SLines_Draw
  (
  PsvCmpInfo_t  *cmpinfs,               /* Compound info recs */
  PsvViewLvl_t  *pvlvl_p,               /* Parent pst view level */
  PsvViewLvl_t  *kvlvl_p                /* Kids pst view level */
  )
{
  Display       *dsp;
  DrawCxt_t     *dc_p;
  PsvTreeLvl_t  *ptlvl_p;               /* Tree level for parent view level */ 
  PsvTreeLvl_t  *ktlvl_p;               /* Tree level for kids view level */ 
  U16_t          nd_i, sg_i;
  U16_t          num_sibs, sib_i;
  short int      hx, kx, ky, mx, px, py;

  ptlvl_p = ViewLvl_TreeLvl_Get (pvlvl_p);
  ktlvl_p = ViewLvl_TreeLvl_Get (kvlvl_p);

  /*  Don't draw any lines if there is no selected node in the parent
      level, or no compounds in the kid level.
  */
  if (TreeLvl_SelNd_Get (ptlvl_p)  == PVW_NODE_NONE
      || TreeLvl_NCmps_Get (ktlvl_p) == 0)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (pvlvl_p);

  /*  Draw the lines connecting the two levels.  Determine first the 
      center coordinates of the parent node, and the starting position
      of the first kid node.  Also set the gc for the proper line type
      and color.
  */
  px = AppDim_PstSymMid_Get (&GAppDim) + AppDim_PstSymSepWd_Get (&GAppDim) 
    + AppDim_PstSymTotWd_Get (&GAppDim) * (TreeLvl_HeadNd_Get (ptlvl_p) 
    + TreeLvl_SelNd_Get (ptlvl_p));

  /*  If the cycle number of top compound is nonzero, then assume that 
      the cycle numbers are being displayed, so move the top of the 
      lines down some.
  */
  if (PsvCmpI_Cycle_Get (PstView_IthCmpI_Get (PstView_Handle_Grab (), 
      PstComp_Index_Get (TreeLvl_IthCmpNd_Get (ptlvl_p, 
      TreeLvl_SelNd_Get (ptlvl_p))))) != 0)
    py = ViewLvl_LnYBeg_Get (kvlvl_p) + AppDim_PstCycHt_Get (&GAppDim);
  else
    py = ViewLvl_LnYBeg_Get (kvlvl_p);

  hx = AppDim_PstSymMid_Get (&GAppDim) + AppDim_PstSymSepWd_Get (&GAppDim) 
    + AppDim_PstSymTotWd_Get (&GAppDim) * TreeLvl_HeadNd_Get (ktlvl_p);
  kx = hx;
  ky = ViewLvl_LnYEnd_Get (kvlvl_p);

  if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
    {
    XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
    }
  if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
    {
    XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
      LineSolid, CapButt, JoinMiter);
    DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
    }

  /*  For each subgoal, draw the crossbar if it is needed, draw the 
      line from the center of the parent node to the center of the 
      subgoal nodes, and then draw the vertical bar for each compound
      node in the subgoal.
  */
  for (nd_i = 0, sg_i = 0; sg_i < TreeLvl_NSGs_Get (ktlvl_p); sg_i++)
    {
    num_sibs = PstSubg_NumSons_Get (PstComp_Father_Get (TreeLvl_IthCmpNd_Get 
      (ktlvl_p, nd_i)));
    if (num_sibs > 1)
      {
      XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
        kx, ViewLvl_BarYBeg_Get (kvlvl_p), kx + (num_sibs - 1) 
        * AppDim_PstSymTotWd_Get (&GAppDim), ViewLvl_BarYBeg_Get (kvlvl_p));
      }

    mx = kx - hx + (((num_sibs - 1) * AppDim_PstSymTotWd_Get (&GAppDim)) >> 1);
    XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
      px, py, mx + hx, ky);

    for (sib_i = 0; sib_i < num_sibs; sib_i++)
      {
      XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
        kx, ViewLvl_BarYBeg_Get (kvlvl_p), kx, ViewLvl_BarYEnd_Get (kvlvl_p));
      PsvCmpI_MidBar_Put (cmpinfs[PstComp_Index_Get (TreeLvl_IthCmpNd_Get
       (ktlvl_p, nd_i))], mx);
      nd_i++;
      kx += AppDim_PstSymTotWd_Get (&GAppDim);
      }
    }


  return ;
}
/*  End of SLines_Draw  */


/****************************************************************************
*
*  Function Name:                 SPathSel_Draw
*
*    This routine draws a focus box around those compounds which 
*    actually lie along the path.
*
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPathSel_Draw
  (
  PstView_t     *pv_p                 
  )
{
  PsvViewLvl_t  *pthvlvl_p;             /* Path view level */
  PsvTreeLvl_t  *pthtlvl_p;             /* Path tree level */ 
  PsvTreeLvl_t  *tlvl_p;                /* Tree level */ 
  U16_t          nd_i;

  pthvlvl_p = PstView_PathVLvl_Get (pv_p);
  pthtlvl_p = ViewLvl_TreeLvl_Get (pthvlvl_p);

  for (nd_i = 0; nd_i < TreeLvl_NCmps_Get (pthtlvl_p); nd_i++)
    {
    tlvl_p = PstView_IthPTLvl_Get (pv_p, PstView_IthPthLN_Get (pv_p, nd_i));
    if (TreeLvl_IthCmpNd_Get (pthtlvl_p, nd_i) 
        == TreeLvl_IthCmpNd_Get (tlvl_p, TreeLvl_SelNd_Get (tlvl_p)))
      {
      TreeLvl_SelNd_Put (pthtlvl_p, nd_i);
      SSelSym_Draw (pthvlvl_p);
      }
    }

  TreeLvl_SelNd_Put (pthtlvl_p, PVW_NODE_NONE);
  return ;
}
/*  End of SPathSel_Draw  */


/****************************************************************************
*
*  Function Name:                 SPathLvlNums_Draw
*
*    This routine draws the level numbers in the path viewer.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPathLvlNums_Draw
  (
  PsvViewLvl_t  *pthvlvl_p              /* Path view level */
  )
{
  Display       *dsp;
  DrawCxt_t     *dc_p;
  PsvTreeLvl_t  *pthtlvl_p;             /* Tree level for path view level */ 
  Window         dawin;
  XmString       num_str;
  U16_t          nd_i, sg_i;
  U16_t          num_sibs;
  Position       x;
  Dimension      width;
  char           buff[8];

  pthtlvl_p = ViewLvl_TreeLvl_Get (pthvlvl_p);
  dc_p = ViewLvl_DrawCxt_Get (pthvlvl_p);
  dawin = XtWindow (DrawCxt_DA_Get (dc_p));
  dsp = XtDisplay (DrawCxt_DA_Get (dc_p));
  if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
    {
    XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
    }

  /*  Create the string and draw in it the box determined by the
      beginning of the first symbol in the subgoal and end of the
      last symbol of the subgoal.
  */
  x = 0;
  for (nd_i = 0, sg_i = 0; sg_i < TreeLvl_NSGs_Get (pthtlvl_p); sg_i++)
    {
    if (sg_i == 0)
      {
      num_str = XmStringCreateLtoR ("tgt", AppDim_PthLblTag_Get (&GAppDim));
      }
    else
      {
      sprintf (buff, "%1u", sg_i);
      num_str = XmStringCreateLtoR (buff, AppDim_PthLblTag_Get (&GAppDim));
      }

    num_sibs = PstSubg_NumSons_Get (PstComp_Father_Get (
      TreeLvl_IthCmpNd_Get (pthtlvl_p, nd_i)));
    width = num_sibs * AppDim_PstSymTotWd_Get (&GAppDim);
    XmStringDrawImage (dsp, DrawCxt_Pmap_Get (dc_p), 
      SynAppR_FontList_Get (&GSynAppR), num_str, DrawCxt_GC_Get (dc_p), x, 
      ViewLvl_LnYBeg_Get (pthvlvl_p), 
      width + AppDim_PstSymSepWd_Get (&GAppDim), 
      XmALIGNMENT_CENTER, XmSTRING_DIRECTION_L_TO_R, NULL); 
    x += width;
    nd_i += num_sibs;
    XmStringFree (num_str);
    }

  return ;
}
/*  End of SPathLvlNums_Draw  */


/****************************************************************************
*
*  Function Name:                 SSelLine_Draw
*
*    This routine thickens the lower half of the line between the selected
*    parent node and the selected kid subgoal nodes.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSelLine_Draw
  (
  PsvCmpInfo_t  *cmpinfs,               /* Compound info recs */
  PsvViewLvl_t  *pvlvl_p,               /* Parent pst view level */
  PsvViewLvl_t  *kvlvl_p                /* Kids pst view level */
  )
{
  Display       *dsp;
  DrawCxt_t     *dc_p;
  PsvTreeLvl_t  *ptlvl_p;               /* Tree level for parent view level */ 
  PsvTreeLvl_t  *ktlvl_p;               /* Tree level for kids view level */ 
  U32_t          cmp_i;
  short int      kx, ky, px, py;

  ptlvl_p = ViewLvl_TreeLvl_Get (pvlvl_p);
  ktlvl_p = ViewLvl_TreeLvl_Get (kvlvl_p);

  /*  Don't draw the line if there is no selected node in the parent
      level or the kid level.
  */
  if (TreeLvl_SelNd_Get (ptlvl_p) == PVW_NODE_NONE 
      || TreeLvl_SelNd_Get (ktlvl_p) == PVW_NODE_NONE)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (pvlvl_p);

  /*  Determine first the center coordinates of the parent node and the
      kid subgoal.  Calculate the center of the connecting line, set the 
      gc for the proper line type and color, and then draw lines.
  */
  px = AppDim_PstSymMid_Get (&GAppDim) + AppDim_PstSymSepWd_Get (&GAppDim) 
    + AppDim_PstSymTotWd_Get (&GAppDim) * (TreeLvl_HeadNd_Get (ptlvl_p) 
    + TreeLvl_SelNd_Get (ptlvl_p));

  /*  If the cycle number of top compound is nonzero, then assume that 
      the cycle numbers are being displayed, so move the top of the 
      lines down some.
  */
  if (PsvCmpI_Cycle_Get (PstView_IthCmpI_Get (PstView_Handle_Grab (), 
      PstComp_Index_Get (TreeLvl_IthCmpNd_Get (ptlvl_p, 
      TreeLvl_SelNd_Get (ptlvl_p))))) != 0)
    py = ViewLvl_LnYBeg_Get (kvlvl_p) + AppDim_PstCycHt_Get (&GAppDim);
  else
    py = ViewLvl_LnYBeg_Get (kvlvl_p);

  cmp_i = PstComp_Index_Get (TreeLvl_IthCmpNd_Get (ktlvl_p, 
    TreeLvl_SelNd_Get (ktlvl_p)));
  kx = AppDim_PstSymMid_Get (&GAppDim) + AppDim_PstSymSepWd_Get (&GAppDim) 
    + PsvCmpI_MidBar_Get (cmpinfs[cmp_i])
    + AppDim_PstSymTotWd_Get (&GAppDim) * TreeLvl_HeadNd_Get (ktlvl_p);
  ky = ViewLvl_LnYEnd_Get (kvlvl_p);
  if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
    {
    XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
    }
  if (DrawCxt_GCLineW_Get (dc_p) != PVW_SELECTLN_LNWD)
    {
    XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SELECTLN_LNWD,
      LineSolid, CapButt, JoinMiter);
    DrawCxt_GCLineW_Put (dc_p, PVW_SELECTLN_LNWD);
    }

  XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
    px, py, kx - 1, ky);
  XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
    px, py, kx + 1, ky);

  return ;
}
/*  End of SSelLine_Draw  */


/****************************************************************************
*
*  Function Name:                 SSelSym_Draw
*
*    This routine draws a black box around the selected node.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSelSym_Draw
  (
  PsvViewLvl_t  *vlvl_p                 /* Pst view level */
  )
{
  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */ 
  Display       *dsp;
  DrawCxt_t     *dc_p;
  XPoint         pts[5];
  short int      x;

  if (vlvl_p == NULL)
    return;

  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) == PVW_NODE_NONE)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (vlvl_p);

  /*  Because of the way lines of width two are drawn, we need
      to make some adjustments on the select box coordinates.
  */
  x = AppDim_PstSymSepWd_Get (&GAppDim) + AppDim_PstSymTotWd_Get (&GAppDim)
    * (TreeLvl_HeadNd_Get (tlvl_p) + TreeLvl_SelNd_Get (tlvl_p));
  pts[0].x = x - AppDim_PstSBoxOff_Get (&GAppDim) + 1;
  pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p)
    - AppDim_PstSBoxOff_Get (&GAppDim) + 1;
  pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1) 
    + AppDim_PstSBoxOff_Get (&GAppDim);
  pts[1].y = pts[0].y;
  pts[2].x = pts[1].x;
  pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p) + AppDim_PstSBoxOff_Get (&GAppDim);
  pts[3].x = pts[0].x;
  pts[3].y = pts[2].y;
  pts[4].x = pts[0].x;
  pts[4].y = pts[0].y;
  if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
    {
    XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
    }

  XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p),
    PVW_SELECTBOX_LNW, LineSolid, CapButt, JoinMiter);

  XDrawLines (dsp, XtWindow (DrawCxt_DA_Get (dc_p)), DrawCxt_GC_Get (dc_p),
    pts, 5, CoordModeOrigin);
  XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
    pts, 5, CoordModeOrigin);
  XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p),
    PVW_DEFAULT_LNWD, LineSolid, CapButt, JoinMiter);

  return ;
}
/*  End of SSelSym_Draw  */


/****************************************************************************
*
*  Function Name:                 SSelSym_Undraw
*
*    Draw over the existing selection box in the PST View background color
*    at the given node on the given level of the PST View tree.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSelSym_Undraw
  (
  PsvViewLvl_t  *vlvl_p                 /* Pst view level */
  )
{
  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */ 
  Display       *dsp;
  DrawCxt_t     *dc_p;
  XPoint         pts[5];
  short int      x;

  if (vlvl_p == NULL)
    return;

  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  if (TreeLvl_SelNd_Get (tlvl_p) == PVW_NODE_NONE)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (vlvl_p);

  /*  Because of the way lines of width two are drawn, we need
      to make some adjustments on the select box coordinates.
  */
  x = AppDim_PstSymSepWd_Get (&GAppDim) + AppDim_PstSymTotWd_Get (&GAppDim)
    * (TreeLvl_HeadNd_Get (tlvl_p) + TreeLvl_SelNd_Get (tlvl_p));
  pts[0].x = x - AppDim_PstSBoxOff_Get (&GAppDim) + 1;
  pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p) 
    - AppDim_PstSBoxOff_Get (&GAppDim) + 1;
  pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1) 
    + AppDim_PstSBoxOff_Get (&GAppDim);
  pts[1].y = pts[0].y;
  pts[2].x = pts[1].x;
  pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p) + AppDim_PstSBoxOff_Get (&GAppDim);
  pts[3].x = pts[0].x;
  pts[3].y = pts[2].y;
  pts[4].x = pts[0].x;
  pts[4].y = pts[0].y;
  if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_WHITE)
    {
    XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_WHITE));
    DrawCxt_GCFg_Put (dc_p, SAR_CLRI_WHITE);
    }

  XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p),
    PVW_SELECTBOX_LNW, LineSolid, CapButt, JoinMiter);
  XDrawLines (dsp, XtWindow (DrawCxt_DA_Get (dc_p)), DrawCxt_GC_Get (dc_p),
    pts, 5, CoordModeOrigin);
  XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
    pts, 5, CoordModeOrigin);
  XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p),
    PVW_DEFAULT_LNWD, LineSolid, CapButt, JoinMiter);

  return ;
}
/*  End of SSelSym_Undraw  */


/****************************************************************************
*
*  Function Name:                 SSymbols_Draw
*
*    This routine draws the row of symbols for one of the view levels.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSymbols_Draw
  (
  PstView_t     *pv_p,                  /* Needed for circularity coloration check */
  PsvCmpInfo_t  *cmpinfs,               /* Compound info recs */
  PsvViewLvl_t  *vlvl_p,                /* Pst view level */
  Boolean_t      in_view
  )
{
  Compound_t    *cmp_p;
  Display       *dsp;
  DrawCxt_t     *dc_p;
  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */ 
  SymTab_t      *stab_p;                /* Symbol table ptr */
  XPoint         pts[5];
  U32_t          cmp_i;
  U16_t          sym_i;
  U8_t           tlvl_num;
  short int      x;

  if (vlvl_p == NULL)
    return;

  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);

  /*  Don't draw any symbols if there are no compounds in the level.  */
  if (tlvl_p == NULL || TreeLvl_NCmps_Get (tlvl_p) == 0)
    return;

  tlvl_num = TreeLvl_LvlNum_Get (tlvl_p);

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (vlvl_p);

  /*  Draw each symbol in list.  Determine */
  x = AppDim_PstSymSepWd_Get (&GAppDim) + AppDim_PstSymTotWd_Get (&GAppDim)  
    * TreeLvl_HeadNd_Get (tlvl_p);
  for (sym_i = 0; sym_i < TreeLvl_NCmps_Get (tlvl_p); sym_i++)
    {
    cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, sym_i);
    cmp_i = PstComp_Index_Get (cmp_p);
    stab_p = PstComp_SymbolTable_Get (cmp_p);
    if (SymTab_Flags_Available_Get (stab_p))
      {
      /*  Available node:  draw blue square.  */
      if (PsvCmpI_Visited_Get (cmpinfs[cmp_i]))
        {
        /*  Available node:  fill visited square dark blue.  */
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_AVL_VD)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VD));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_AVL_VD);
          }
        if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
          {
          XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
          DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
          }
        pts[0].x = x;
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[1].y = pts[0].y;
        pts[2].x = pts[1].x;
        pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[3].x = pts[0].x;
        pts[3].y = pts[2].y;
        pts[4].x = pts[0].x;
        pts[4].y = pts[0].y;
        XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
          pts, 5, Convex, CoordModeOrigin);
        }  /* End of if visited */

      else 
        {
        if (PsvCmpI_VstElse_Get (cmpinfs[cmp_i]))
          {
          /*  Available node:  fill visited elsewhere square light blue.  */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_AVL_VE)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VE));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_AVL_VE);
            }
          if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
            {
            XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
            DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
            }
          pts[0].x = x;
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[1].y = pts[0].y;
          pts[2].x = pts[1].x;
          pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[3].x = pts[0].x;
          pts[3].y = pts[2].y;
          pts[4].x = pts[0].x;
          pts[4].y = pts[0].y;
          XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            pts, 5, Convex, CoordModeOrigin);

          if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
            }
          }
        else
          {
          if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
            }
          }

        /*  Available node:  draw square using dark blue lines.  */
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_AVL_VD)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_AVL_VD));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_AVL_VD);
          }
        pts[0].x = x;
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[1].y = pts[0].y;
        pts[2].x = pts[1].x;
        pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[3].x = pts[0].x;
        pts[3].y = pts[2].y;
        pts[4].x = pts[0].x;
        pts[4].y = pts[0].y;
        XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
          pts, 5, CoordModeOrigin);
        }
      }  /* End of if available */

    else if (SymTab_Flags_Solved_Get (stab_p))
      {
      /*  Solved node:  draw green circle.  */
      if (PsvCmpI_Visited_Get (cmpinfs[cmp_i]))
        {
        /*  Solved node:  fill visited circle dark green.  */
        if (DrawCxt_GCFg_Get (dc_p) != (PstView_AllSolCirc (pv_p, cmp_p, tlvl_num, in_view) ?
          SAR_CLRI_STK_VD : SAR_CLRI_SOL_VD))
          {
          if (PstView_AllSolCirc (pv_p, cmp_p, tlvl_num, in_view))
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_STK_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_STK_VD);
            }
          else
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_SOL_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_SOL_VD);
            }
          }
        if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
          {
          XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
          DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
          }
        XFillArc (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
          x, ViewLvl_SymYBeg_Get (vlvl_p), 
          AppDim_PstSymDim_Get (&GAppDim), AppDim_PstSymDim_Get (&GAppDim),
          0, 360 * 64);
        }

      else 
        {
        if (PsvCmpI_VstElse_Get (cmpinfs[cmp_i]))
          {
          /*  Solved node:  fill visited elsewhere circle light green.  */
          if (DrawCxt_GCFg_Get (dc_p) != (PstView_AllSolCirc (pv_p, cmp_p, tlvl_num, in_view) ?
            SAR_CLRI_STK_VE : SAR_CLRI_SOL_VE))
            {
            if (PstView_AllSolCirc (pv_p, cmp_p, tlvl_num, in_view))
              {
              XSetForeground (dsp, DrawCxt_GC_Get (dc_p), SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_STK_VE));
              DrawCxt_GCFg_Put (dc_p, SAR_CLRI_STK_VE);
              }
            else
              {
              XSetForeground (dsp, DrawCxt_GC_Get (dc_p), SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_SOL_VE));
              DrawCxt_GCFg_Put (dc_p, SAR_CLRI_SOL_VE);
              }
            }
          if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
            {
            XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
            DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
            }
          XFillArc (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            x, ViewLvl_SymYBeg_Get (vlvl_p), AppDim_PstSymDim_Get (&GAppDim), 
            AppDim_PstSymDim_Get (&GAppDim), 0, 360 * 64);

          if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
            }
          }
        else
          {
          if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
            }
          }
        /*  Solved node:  draw circle using a dark green line.  */
        if (DrawCxt_GCFg_Get (dc_p) != (PstView_AllSolCirc (pv_p, cmp_p, tlvl_num, in_view) ?
          SAR_CLRI_STK_VD : SAR_CLRI_SOL_VD))
          {
          if (PstView_AllSolCirc (pv_p, cmp_p, tlvl_num, in_view))
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_STK_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_STK_VD);
            }
          else
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_SOL_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_SOL_VD);
            }
          }
        XDrawArc (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
          x, ViewLvl_SymYBeg_Get (vlvl_p), 
          AppDim_PstSymDim_Get (&GAppDim), AppDim_PstSymDim_Get (&GAppDim),
          0, 360 * 64);
        }
      }  /* End of else if solved */

    else if (SymTab_Flags_Stuck_Get (stab_p))
      {
      /*  Stuck node:  draw red up triangle.  */
      if (PsvCmpI_Visited_Get (cmpinfs[cmp_i]))
        {
        /*  Stuck node:  fill visited up triangle dark red.  */
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_STK_VD)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_STK_VD));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_STK_VD);
          }
        if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
          {
          XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
          DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
          }
        pts[0].x = x + AppDim_PstSymMid_Get (&GAppDim);
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[1].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[2].x = x;
        pts[2].y = pts[1].y;
        pts[3].x = pts[0].x;
        pts[3].y = pts[0].y;
        XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
          pts, 4, Convex, CoordModeOrigin);
        }

      else 
        {
        if (PsvCmpI_VstElse_Get (cmpinfs[cmp_i]))
          {
          /*  Stuck node:  fill visited elsewhere up triangle light red.  */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_STK_VE)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_STK_VE));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_STK_VE);
            }
          if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
            {
            XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
            DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
            }
          pts[0].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[1].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[2].x = x;
          pts[2].y = pts[1].y;
          pts[3].x = pts[0].x;
          pts[3].y = pts[0].y;
          XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            pts, 4, Convex, CoordModeOrigin);

          if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
            }
          }
        else
          {
          if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
            }
          }

        /*  Stuck node:  draw up triangle using dark red lines.  */
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_STK_VD)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_STK_VD));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_STK_VD);
          }
        pts[0].x = x + AppDim_PstSymMid_Get (&GAppDim);
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[1].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[2].x = x;
        pts[2].y = pts[1].y;
        pts[3].x = pts[0].x;
        pts[3].y = pts[0].y;
        XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
          pts, 4, CoordModeOrigin);
        }
      }  /* End of else if stuck */

    else if (SymTab_Flags_Unsolveable_Get (stab_p))
      {
      /*  Unsolvable node:  draw red bowtie.  */
      if (PsvCmpI_Visited_Get (cmpinfs[cmp_i]))
        {
        /*  Unsolvable node:  fill visited bowtie dark red.  */
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_UNS_VD)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UNS_VD));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_UNS_VD);
          }
        if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
          {
          XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
          DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
          }
        pts[0].x = x;
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + AppDim_PstSymMid_Get (&GAppDim);
        pts[1].y = pts[0].y + AppDim_PstSymMid_Get (&GAppDim);
        pts[2].x = pts[0].x;
        pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[3].x = pts[0].x;
        pts[3].y = pts[0].y;
        XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
          pts, 4, Convex, CoordModeOrigin);
        pts[0].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[1].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[2].x = x + AppDim_PstSymMid_Get (&GAppDim);
        pts[2].y = pts[0].y + AppDim_PstSymMid_Get (&GAppDim);
        pts[3].x = pts[0].x;
        pts[3].y = pts[0].y;
        XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
          pts, 4, Convex, CoordModeOrigin);
        }

      else 
        {
        if (PsvCmpI_VstElse_Get (cmpinfs[cmp_i]))
          {
          /*  Unsolvable node:  fill visited elsewhere bowtie light 
              red.  
          */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_UNS_VE)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UNS_VE));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_UNS_VE);
            }
          if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
            {
            XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
            DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
            }
          pts[0].x = x;
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[1].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[1].y = pts[0].y + AppDim_PstSymMid_Get (&GAppDim);
          pts[2].x = pts[0].x;
          pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[3].x = pts[0].x;
          pts[3].y = pts[0].y;
          XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            pts, 4, Convex, CoordModeOrigin);
          pts[0].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[1].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[2].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[2].y = pts[0].y + AppDim_PstSymMid_Get (&GAppDim);
          pts[3].x = pts[0].x;
          pts[3].y = pts[0].y;
          XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            pts, 4, Convex, CoordModeOrigin);
  
          if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
            }
          }
        else
          {
          if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
            {
            XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
              LineSolid, CapButt, JoinMiter);
            DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
            }
          }
        /*  Unsolvable node:  draw bowtie using dark red lines.  */
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_UNS_VD)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UNS_VD));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_UNS_VD);
          }
        pts[0].x = x;
        pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
        pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
        pts[1].y = ViewLvl_SymYEnd_Get (vlvl_p);
        pts[2].x = pts[1].x;
        pts[2].y = pts[0].y;
        pts[3].x = pts[0].x;
        pts[3].y = pts[1].y;
        pts[4].x = pts[0].x;
        pts[4].y = pts[0].y;
        XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
          pts, 5, CoordModeOrigin);
        }
      }  /* End of else if unsolvable */
    else if (SymTab_Flags_Open_Get (stab_p))
      {
      if (SymTab_DevelopedComp_Get (stab_p) != NULL)
        {
        /*  Developed node:  draw brown down triangle.  */
        if (PsvCmpI_Visited_Get (cmpinfs[cmp_i]))
          {
          /*  Developed node:  fill visited down triangle dark brown.  */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_DEV_VD)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_DEV_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_DEV_VD);
            }
          if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
            {
            XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
            DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
            }
          pts[0].x = x;
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[1].y = pts[0].y;
          pts[2].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[3].x = pts[0].x;
          pts[3].y = pts[0].y;
          XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            pts, 4, Convex, CoordModeOrigin);
          }

        else 
          {
          if (PsvCmpI_VstElse_Get (cmpinfs[cmp_i]))
            {
            /*  Developed node:  fill visited elsewhere down triangle 
                light brown.  
             */
            if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_DEV_VE)
              {
              XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
                SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_DEV_VE));
              DrawCxt_GCFg_Put (dc_p, SAR_CLRI_DEV_VE);
              }
            if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
              {
              XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
              DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
              }
            pts[0].x = x;
            pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
            pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
            pts[1].y = pts[0].y;
            pts[2].x = x + AppDim_PstSymMid_Get (&GAppDim);
            pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
            pts[3].x = pts[0].x;
            pts[3].y = pts[0].y;
            XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
              pts, 4, Convex, CoordModeOrigin);

            if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
             {
              XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
                LineSolid, CapButt, JoinMiter);
              DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
              }
            }
          else
            {
            if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
              {
              XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
                LineSolid, CapButt, JoinMiter);
              DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
              }
            }
          /*  Developed node:  draw down triangle using dark brown lines.  */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_DEV_VD)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_DEV_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_DEV_VD);
            }
          pts[0].x = x;
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[1].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[1].y = pts[0].y;
          pts[2].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[2].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[3].x = pts[0].x;
          pts[3].y = pts[0].y;
          XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
            pts, 4, CoordModeOrigin);
          }
        }  /* End of else if open and developed */

      else
        {
        /*  Undeveloped node:  draw violet diamond.  */
        if (PsvCmpI_Visited_Get (cmpinfs[cmp_i]))
          {
          /*  Undeveloped node:  fill visited diamond dark violet.  */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_UND_VD)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UND_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_UND_VD);
            }
          if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
            {
            XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
            DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
            }
          pts[0].x = x;
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p) 
            + AppDim_PstSymMid_Get (&GAppDim);
          pts[1].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[1].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[2].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[2].y = pts[0].y;
          pts[3].x = pts[1].x;
          pts[3].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[4].x = pts[0].x;
          pts[4].y = pts[0].y;
          XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
            pts, 5, Convex, CoordModeOrigin);
          }

        else 
          {
          if (PsvCmpI_VstElse_Get (cmpinfs[cmp_i]))
            {
            /*  Undeveloped node:  fill visited elsewhere diamond light 
                violet.  
            */
            if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_UND_VE)
              {
              XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
                SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UND_VE));
              DrawCxt_GCFg_Put (dc_p, SAR_CLRI_UND_VE);
              }
            if (DrawCxt_GCFill_Get (dc_p) != (U8_t) FillSolid)
              {
              XSetFillStyle (dsp, DrawCxt_GC_Get (dc_p), FillSolid);
              DrawCxt_GCFill_Put (dc_p, (U8_t) FillSolid);
              }
            pts[0].x = x;
            pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p) 
              + AppDim_PstSymMid_Get (&GAppDim);
            pts[1].x = x + AppDim_PstSymMid_Get (&GAppDim);
            pts[1].y = ViewLvl_SymYBeg_Get (vlvl_p);
            pts[2].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
            pts[2].y = pts[0].y;
            pts[3].x = pts[1].x;
            pts[3].y = ViewLvl_SymYEnd_Get (vlvl_p);
            pts[4].x = pts[0].x;
            pts[4].y = pts[0].y;
            XFillPolygon (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
              pts, 5, Convex, CoordModeOrigin);

            if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
              {
              XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
                LineSolid, CapButt, JoinMiter);
              DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
              }
            }
          else
            {
            if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
              {
              XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
                LineSolid, CapButt, JoinMiter);
              DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
              }
            }
          /*  Undeveloped node:  draw diamond using dark violet lines.  */
          if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_UND_VD)
            {
            XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
              SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_UND_VD));
            DrawCxt_GCFg_Put (dc_p, SAR_CLRI_UND_VD);
            }
          pts[0].x = x;
          pts[0].y = ViewLvl_SymYBeg_Get (vlvl_p) 
            + AppDim_PstSymMid_Get (&GAppDim);
          pts[1].x = x + AppDim_PstSymMid_Get (&GAppDim);
          pts[1].y = ViewLvl_SymYBeg_Get (vlvl_p);
          pts[2].x = x + (AppDim_PstSymDim_Get (&GAppDim) - 1);
          pts[2].y = pts[0].y;
          pts[3].x = pts[1].x;
          pts[3].y = ViewLvl_SymYEnd_Get (vlvl_p);
          pts[4].x = pts[0].x;
          pts[4].y = pts[0].y;
          XDrawLines (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
            pts, 5, CoordModeOrigin);
          }
        }  /* End of else if open and undeveloped */
      }  /* End of else if open */

    /*  If compound has been marked, draw mark.  */
    if (PsvCmpI_Marked_Get (cmpinfs[cmp_i]) 
         || PsvCmpI_MarkElse_Get (cmpinfs[cmp_i]))
      {
      if (PsvCmpI_Marked_Get (cmpinfs[cmp_i]))
        {
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
          }
        }
      else
        {
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_MARK)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_MARK));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_MARK);
          }
        }

      if (DrawCxt_GCLineW_Get (dc_p) != PVW_SYMBOL_LNWD)
        {
        XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_SYMBOL_LNWD,
          LineSolid, CapButt, JoinMiter);
        DrawCxt_GCLineW_Put (dc_p, PVW_SYMBOL_LNWD);
        }
 
      XDrawLine(dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
        x, ViewLvl_SymYBeg_Get (vlvl_p) - AppDim_PstSymSepHt_Get (&GAppDim), 
        x + (AppDim_PstSymDim_Get (&GAppDim) - 1), ViewLvl_SymYEnd_Get (vlvl_p) 
        + AppDim_PstSymSepHt_Get (&GAppDim));
      XDrawLine(dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p), 
        x, ViewLvl_SymYEnd_Get (vlvl_p) + AppDim_PstSymSepHt_Get (&GAppDim), 
        x + (AppDim_PstSymDim_Get (&GAppDim) - 1), ViewLvl_SymYBeg_Get (vlvl_p) 
        - AppDim_PstSymSepHt_Get (&GAppDim));
      }

    /*  If compound has been selected via select trace, 
        and draw cycle number.  
    */
    if (PsvCmpI_Traced_Get (cmpinfs[cmp_i]) 
        || PsvCmpI_TracElse_Get (cmpinfs[cmp_i]))
      {
      XmString   num_str;
      char       buff[16];

      if (PsvCmpI_Traced_Get (cmpinfs[cmp_i]))
        {
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
          }
        }
      else
        {
        if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_CYCLENUM)
          {
          XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
            SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_CYCLENUM));
          DrawCxt_GCFg_Put (dc_p, SAR_CLRI_CYCLENUM);
          }
        }

      sprintf (buff, "%1lu", PsvCmpI_Cycle_Get (cmpinfs[cmp_i]));
      num_str = XmStringCreateLtoR (buff, AppDim_PstCycTag_Get (&GAppDim));
      XmStringDrawImage (dsp, DrawCxt_Pmap_Get (dc_p), 
        SynAppR_FontList_Get (&GSynAppR), num_str, DrawCxt_GC_Get (dc_p), 
        x, ViewLvl_SymYEnd_Get (vlvl_p) + AppDim_PstSymSepHt_Get (&GAppDim), 
        AppDim_PstSymDim_Get (&GAppDim), 
        XmALIGNMENT_CENTER, XmSTRING_DIRECTION_L_TO_R, NULL); 
      XmStringFree (num_str);

      }

    x += AppDim_PstSymTotWd_Get (&GAppDim);
    }

  return ;
}
/*  End of SSymbols_Draw  */


/****************************************************************************
*
*  Function Name:                 STopLvlBars_Draw
*
*    This routine draws the subgoal bars for the top level nodes.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void STopLvlBars_Draw
  (
  PsvViewLvl_t  *tvlvl_p                /* Top pst view level */
  )
{
  Display       *dsp;
  DrawCxt_t     *dc_p;
  PsvTreeLvl_t  *ttlvl_p;               /* Tree level for top view level */ 
  U16_t          nd_i, sg_i;
  U16_t          num_sibs, sib_i;
  short int      x;

  if (tvlvl_p == NULL)
    return;

  ttlvl_p = ViewLvl_TreeLvl_Get (tvlvl_p);

  /*  Don't draw any lines if there is no subgoal node in the top
      level.
  */
  if (TreeLvl_NSGs_Get (ttlvl_p) == 0)
    return;

  dsp = Screen_TDisplay_Get (SynAppR_ScrnAtrb_Get (&GSynAppR));
  dc_p = ViewLvl_DrawCxt_Get (tvlvl_p);

  /*  Set the gc for the proper line type and color.
  */
  x = AppDim_PstSymMid_Get (&GAppDim) + AppDim_PstSymSepWd_Get (&GAppDim) 
    + AppDim_PstSymTotWd_Get (&GAppDim) * TreeLvl_HeadNd_Get (ttlvl_p);

  if (DrawCxt_GCFg_Get (dc_p) != SAR_CLRI_BLACK)
    {
    XSetForeground (dsp, DrawCxt_GC_Get (dc_p), 
      SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_BLACK));
    DrawCxt_GCFg_Put (dc_p, SAR_CLRI_BLACK);
    }
  if (DrawCxt_GCLineW_Get (dc_p) != PVW_DEFAULT_LNWD)
    {
    XSetLineAttributes (dsp, DrawCxt_GC_Get (dc_p), PVW_DEFAULT_LNWD,
      LineSolid, CapButt, JoinMiter);
    DrawCxt_GCLineW_Put (dc_p, PVW_DEFAULT_LNWD);
    }

  /*  For each subgoal with more than one compound, draw the crossbar and 
      vertical bars for each compound node in the subgoal.
  */
  for (nd_i = 0, sg_i = 0; sg_i < TreeLvl_NSGs_Get (ttlvl_p); sg_i++)
    {
    num_sibs = PstSubg_NumSons_Get (PstComp_Father_Get (TreeLvl_IthCmpNd_Get 
      (ttlvl_p, nd_i)));
    if (num_sibs > 1)
      {
      XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
        x, ViewLvl_BarYBeg_Get (tvlvl_p), x + (num_sibs - 1) 
        * AppDim_PstSymTotWd_Get (&GAppDim), ViewLvl_BarYBeg_Get (tvlvl_p));
      for (sib_i = 0; sib_i < num_sibs; sib_i++)
        {
        XDrawLine (dsp, DrawCxt_Pmap_Get (dc_p), DrawCxt_GC_Get (dc_p),
          x, ViewLvl_BarYBeg_Get (tvlvl_p), x, ViewLvl_BarYEnd_Get (tvlvl_p));
        nd_i++;
        x += AppDim_PstSymTotWd_Get (&GAppDim);
        }
      }
    else
      {
      nd_i++;
      x += AppDim_PstSymTotWd_Get (&GAppDim);
      }
    }  /* For each subgoal */

  return ;
}
/*  End of STopLvlBars_Draw  */


/****************************************************************************
*
*  Function Name:                 SUnmark_Downward
*
*    This routine recursively removes marks from a node and its marked
*    descendants.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SUnmark_Downward (PsvCmpInfo_t *cmp_infos, Compound_t *cmp_p)
{
  SymTab_t   *stab_p;
  Compound_t *dev_p, *desc_p;
  Subgoal_t  *subg_p;

  PstView_Unmark_Store (cmp_infos, cmp_p);
  stab_p = PstComp_SymbolTable_Get (cmp_p);
  dev_p = SymTab_DevelopedComp_Get (stab_p);
  if (dev_p == NULL) return;

  for (subg_p = PstComp_Son_Get (dev_p); subg_p != NULL; subg_p = PstSubg_Brother_Get (subg_p))
    {
    for (desc_p = PstSubg_Son_Get (subg_p); desc_p != NULL; desc_p = PstComp_Brother_Get (desc_p))
      if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (desc_p)]))
      SUnmark_Downward (cmp_infos, desc_p);
    }
}


/****************************************************************************
*
*  Function Name:                 SPstView_BarLeftSet_CB
*
*    This routine determines the position of the pst tree in the view port.  
*
*  Implicit Inputs:
*
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPstView_BarLeftSet_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XmScrollBarCallbackStruct *cb_p;
  PstView_t    *pview_p;

  cb_p = (XmScrollBarCallbackStruct *) call_p;
  pview_p = (PstView_t *) client_p;

  if (cb_p->reason == XmCR_DECREMENT || cb_p->reason == XmCR_PAGE_DECREMENT 
      || cb_p->reason == XmCR_INCREMENT || cb_p->reason == XmCR_PAGE_INCREMENT
      || cb_p->reason == XmCR_VALUE_CHANGED || cb_p->reason == XmCR_TO_TOP
      || cb_p->reason == XmCR_TO_BOTTOM)
    {
    PstView_LeftEdge_Put (pview_p, (Dimension) (cb_p->value));
    PstView_LeftNd_Put (pview_p, 
      (U16_t) (cb_p->value / AppDim_PstSymTotWd_Get (&GAppDim)));
    if (cb_p->value % AppDim_PstSymTotWd_Get (&GAppDim) 
        > AppDim_PstSymSepWd_Get (&GAppDim) - 1)
      {
      PstView_LeftNd_Put (pview_p, PstView_LeftNd_Get (pview_p) + 1);
      }
    }


  return ;
}
/*  End of SPstView_BarLeftSet_CB  */


/****************************************************************************
*
*  Function Name:                 SPstView_PstExp_CB
*
*    This routine handles the expose events for the pst tree drawing
*    area.  On an expose event for either drawing area, the corresponding
*    pixmap is copies over to the drawing area and any focus or select
*    nodes are rehighlighted.  
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPstView_PstExp_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  PstView_t       *pv_p;
  DrawCxt_t       *dc_p;
  XExposeEvent    *event;
  U8_t             lvl_i;

  cb_p = (XmDrawingAreaCallbackStruct *) call_p;
  if (cb_p->reason == XmCR_EXPOSE)
    {
    pv_p = (PstView_t *) client_p;
    dc_p = PstView_PstDC_Get (pv_p);

    /* Since we have a pixmap that stores the drawing of the symbols, 
       we need only copy the exposed area of the pixmap to the drawing
       area.  However, since we do not draw the focus nodes to the pixmap,
       we have to redraw them directly to the drawing area.
    */
    event = &cb_p->event->xexpose;
    PstView_LeftEdge_Put(pv_p, (Dimension)(event->x));

    XCopyArea (event->display, DrawCxt_Pmap_Get (dc_p), event->window,
      DrawCxt_GC_Get (dc_p), event->x, event->y, event->width, event->height,
      event->x, event->y);
    for (lvl_i = 1; lvl_i < PVW_LEVEL_NUMOF; lvl_i++)
      if (XtIsManaged (RxnPreV_FormDlg_Get (ViewLvl_RxnPV_Get 
          (PstView_IthVLvl_Get (pv_p, lvl_i)))))
        PstView_Focus_Draw  (PstView_IthVLvl_Get (pv_p, lvl_i));
    if (avledit_called)
      {
      AvcLib_Control_Destroy ();
      AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_EXISTS);
      AvcLib_Init (FCB_SEQDIR_AVLC (""), AVC_INIT_INFO);
      PstView_Mouse_Remove (pv_p); /* Avoid screwing up levels through multiple callbacks!!! */
      PstView_Display (pv_p); /* Force update of rxn_view display */
      avledit_called = FALSE;
      }
    }

  return ;
}
/*  End of SPstView_PstExp_CB  */


/****************************************************************************
*
*  Function Name:                 SPstView_PstSel_CB
*
*    This routine handles the selection events for the pst tree 
*    drawing area.  When a symbol is selected (the left mouse button 
*    pressed), the tree is updated to reflect the selection.  Action 
*    is taken on button presses--button releases are ignored.  The
*    function has been modified to allow marking of compounds using
*    the right mouse button.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPstView_PstSel_CB
  (
  Widget     w,                           /* Widget */
  XtPointer  client_data,                 /* PstView */
  XtPointer  call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  PstView_t    *pv_p;
  PsvTreeLvl_t *tlvl_p;
  PsvViewLvl_t *vlvl_p;
  XButtonEvent *bevent;
  int           rem;
  U16_t         curnode;
  U8_t          curlvl;
  Boolean_t     cont;

  cb_p = (XmDrawingAreaCallbackStruct *) call_p;
  pv_p = (PstView_t *) client_data;

    /*  Only process left button presses with no state.  */
  if (cb_p->reason == XmCR_INPUT && cb_p->event->type == ButtonPress)
    {
    bevent = &cb_p->event->xbutton;
    if (bevent->state == 0 && (bevent->button == Button1
        || bevent->button == Button2
        || bevent->button == Button3))
      {
      /*  Determine which level and node.  */
      rem = (bevent->y % AppDim_PstLvlTotHt_Get (&GAppDim)) 
        + AppDim_PstLvlBdrOff_Get (&GAppDim);
      if (rem >= AppDim_PstLvlBdrTop_Get (&GAppDim) 
          && rem < AppDim_PstLvlBdrBtm_Get (&GAppDim))
        curlvl = (U8_t) (bevent->y / AppDim_PstLvlTotHt_Get (&GAppDim));
      else
        curlvl = PVW_LEVEL_NONE;

      if ((bevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
            % AppDim_PstSymTotWd_Get (&GAppDim) 
            < AppDim_PstSymDim_Get (&GAppDim))
        curnode = (U16_t)((bevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
          / AppDim_PstSymTotWd_Get (&GAppDim));
      else
        curnode = PVW_NODE_NONE;
  
      if (curlvl != PVW_LEVEL_NONE && curnode != PVW_NODE_NONE)
        {
        vlvl_p = PstView_IthVLvl_Get (pv_p, curlvl);
        tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
        if (curnode >= TreeLvl_HeadNd_Get (tlvl_p) 
           && curnode < TreeLvl_HeadNd_Get (tlvl_p) 
             + TreeLvl_NCmps_Get (tlvl_p))
          {
          curnode -= TreeLvl_HeadNd_Get (tlvl_p);
          PstView_Mouse_Remove (pv_p);
          if (bevent->button == Button1)
            {
            if (PstVLvls_Update (pv_p, curnode, curlvl))
              PstView_Display (pv_p);
            else
              PstView_Mouse_Reset (pv_p, Screen_TDisplay_Get (
                SynAppR_ScrnAtrb_Get (&GSynAppR)));
            }
          else if (bevent->button == Button2)
            {
            Compound_t    *cmp_p, *prev_cmp_p;
            Subgoal_t     *sg_p;
            PsvCmpInfo_t  *cmp_infos;
int i;

            cmp_infos = PstView_CmpInfo_Get (pv_p);
            cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
            if (cmp_p != NULL)
              {
              if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
                {
                SUnmark_Downward (cmp_infos, cmp_p);

PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE); /* prevent pointer from jumping to last selected node! */
                PstView_Display (pv_p);
                }
              else if (!PsvCmpI_MarkElse_Get (cmp_infos[PstComp_Index_Get (
                   cmp_p)]))
                {
                PstView_Mark_Store (cmp_infos, cmp_p);
                cont = TRUE;
                while (curlvl != 0 && cont)
                  {
                  curlvl--;
                  vlvl_p = PstView_IthVLvl_Get (pv_p, curlvl);
                  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
                  curnode = TreeLvl_SelNd_Get (tlvl_p);
                  cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
                  if (cmp_p == NULL || PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
                    cont = FALSE;
                  else PstView_Mark_Store (cmp_infos, cmp_p);
                  }

/* This extra step is necessitated by the potential truncation of the path thread */
                for (curnode = num_overflowed; curnode != 0 && cont;)
                  {
                  curnode--;
                  sg_p = path_overflow[curnode];
                  prev_cmp_p = PstSubg_Son_Get (sg_p);
                  while (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (prev_cmp_p)) !=
                    PstSubg_Father_Get (PstComp_Father_Get (cmp_p)))
                    {
                    prev_cmp_p = PstComp_Brother_Get (prev_cmp_p);
                    if (prev_cmp_p == NULL)
                      {
                      printf ("Error in path_overflow lineage\n");
for (i=0; i<num_overflowed; i++) printf("SG %d at level %d\n",PstSubg_Index_Get(path_overflow[i]),overflow_level+i);
                      }
                    }
                  if (prev_cmp_p == NULL) cont = FALSE;
                  else
                    {
                    cmp_p = prev_cmp_p;
                    if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
                      cont = FALSE;
                    else PstView_Mark_Store (cmp_infos, cmp_p);
                    }
                  }

                vlvl_p = PstView_PathVLvl_Get (pv_p);
                tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
                for (curnode = TreeLvl_NCmps_Get (tlvl_p); curnode != 0 && cont;)
                  {
                  curnode--;
                  prev_cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
                  if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (prev_cmp_p)) ==
                    PstSubg_Father_Get (PstComp_Father_Get (cmp_p)))
                    {
                    cmp_p = prev_cmp_p;
                    if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
                      cont = FALSE;
                    else PstView_Mark_Store (cmp_infos, cmp_p);
                    }
                  }
PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE); /* prevent pointer from jumping to last selected node! */
                PstView_Display (pv_p);
                }
              else
                {
                PstView_Mouse_Reset (pv_p, Screen_TDisplay_Get (
                  SynAppR_ScrnAtrb_Get (&GSynAppR)));
                }
              }
            else
              {
              PstView_Mouse_Reset (pv_p, Screen_TDisplay_Get (
                SynAppR_ScrnAtrb_Get (&GSynAppR)));
              }
            }
          else
            {
            Compound_t    *cmp_p;
            PsvCmpInfo_t  *cmp_infos;

            cmp_infos = PstView_CmpInfo_Get (pv_p);
            cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
            if (cmp_p != NULL)
              {
              if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
                PstView_Unmark_Store (cmp_infos, cmp_p);
              else if (PsvCmpI_MarkElse_Get (cmp_infos[PstComp_Index_Get ( 
                   cmp_p)]))
                PsvCmpI_MarkElse_Unset (cmp_infos[PstComp_Index_Get ( 
                   cmp_p)]);
              else
                PstView_Mark_Store (cmp_infos, cmp_p);

PstView_ActVLvl_Put (pv_p, PVW_LEVEL_NONE); /* prevent pointer from jumping to last selected node! */
              PstView_Display (pv_p);
              }
            else
              {
              PstView_Mouse_Reset (pv_p, Screen_TDisplay_Get (
                SynAppR_ScrnAtrb_Get (&GSynAppR)));
              }
            }
          }
        }  
      }  /* End of if empty state and left button pressed */
    }  /* End of if input is button press */

  return ;
}
/*  End of SPstView_PstSel_CB  */


/****************************************************************************
*
*  Function Name:                 PstView_SelInRow
*
*    This routine mimics the incremental downward selection of compounds
*    in the interactive continuation path.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void PstView_SelInRow (PstView_t *pv_p, U8_t curlvl, U32_t cmpinx)
{
  PsvTreeLvl_t *tlvl_p;
  PsvViewLvl_t *vlvl_p;
  XEvent        event;
  Widget        w;
  int           rem;
  U32_t         cmp_i;
  U16_t         curnode;
  Compound_t   *cmp_p;
  PsvCmpInfo_t *cmp_infos;
  PstCB_t      *pcb_p;

/* Superfluous and redundant: PstView_Tree_Init is called from Submission_Execute after Synchem_Search
  if (curlvl == PVW_LEVEL_TOP)
    {
    pcb_p = PstView_PstCB_Get (pv_p);
    PstView_NumCmpI_Put (pv_p, PstCB_CompoundIndex_Get (pcb_p));
    cmp_infos = (PsvCmpInfo_t *) realloc (PstView_CmpInfo_Get (pv_p),
      PstView_NumCmpI_Get (pv_p) * sizeof (PsvCmpInfo_t));
    if (cmp_infos == NULL)
      {
      fprintf (stderr, 
        "PstView_Tree_Init:  unable to allocate memory for cmp info recs.\n");
      exit (-1);
      }

    PstView_CmpInfo_Put (pv_p, cmp_infos);
*/
    /*  Initialize compound information nodes.  */
/*
    for (cmp_i = 0; cmp_i < PstView_NumCmpI_Get (pv_p); cmp_i++)
      {
      PsvCmpI_RxnCmmt_Put (cmp_infos[cmp_i], NULL);
      PsvCmpI_CmtLen_Put (cmp_infos[cmp_i], 0);
      PsvCmpI_Cycle_Put (cmp_infos[cmp_i], 0);
      PsvCmpI_MidBar_Put (cmp_infos[cmp_i], 0);
      PsvCmpI_Status_Put (cmp_infos[cmp_i], PVW_STATUS_NONE);
      }
    }
*/

  vlvl_p = PstView_IthVLvl_Get (pv_p, curlvl);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  for (curnode = 0; curnode < TreeLvl_NCmps_Get (tlvl_p); curnode++)
  {
    cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
    if (PstComp_Index_Get (cmp_p) == cmpinx)
    {
      if (PstVLvls_Update (pv_p, curnode, curlvl))
      {
        pvdisp_reset_mouse = FALSE;
        PstView_Display (pv_p);
        pvdisp_reset_mouse = TRUE;
      }
      return ;
    }
  }

  printf("Error in PstView_SelInRow (pv_p=%p curlvl=%d, cmpinx=%d)\n", pv_p, curlvl, cmpinx);
  return ;
}


/****************************************************************************
*
*  Function Name:                 PstView_SelCmp
*
*    This routine mimics the upward propagation event for the pst tree 
*    drawing area, as if the middle button were clicked.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t PstView_SelCmp
  (
  PstView_t  *pv_p,
  Compound_t *incmp_p
  )
{
  PsvTreeLvl_t *tlvl_p;
  PsvViewLvl_t *vlvl_p;
  int           i, j;
  U16_t         curnode;
  U8_t          curlvl;
  Boolean_t     cont;
  Compound_t   *cmp_p, *prev_cmp_p;
  Subgoal_t    *sg_p;
  PsvCmpInfo_t *cmp_infos;

  cmp_p = incmp_p;

  for (i=0, curlvl=0xff; i<3 && curlvl==0xff; i++)
  {
    vlvl_p = PstView_IthVLvl_Get (pv_p, i);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    for (j=0, curnode=0xffff; j<TreeLvl_NCmps_Get (tlvl_p) && curnode==0xffff; j++)
    {
      if (cmp_p==TreeLvl_IthCmpNd_Get (tlvl_p, j))
      {
        curlvl=i;
        curnode=j;
      }
    }
  }

  if (curlvl==0xff || curnode==0xffff)
  {
    InfoWarn_Show ("Marked compound is not visible - cannot create path file.");
    return (FALSE);
  }

  PstView_Mouse_Remove (pv_p);

  cmp_infos = PstView_CmpInfo_Get (pv_p);
  PstView_Mark_Store (cmp_infos, cmp_p);
  cont = TRUE;
  while (curlvl != 0 && cont)
    {
    curlvl--;
    vlvl_p = PstView_IthVLvl_Get (pv_p, curlvl);
    tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
    curnode = TreeLvl_SelNd_Get (tlvl_p);
    cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
    if (cmp_p == NULL || PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
      cont = FALSE;
    else PstView_Mark_Store (cmp_infos, cmp_p);
    }

/* This extra step is necessitated by the potential truncation of the path thread */
  for (curnode = num_overflowed; curnode != 0 && cont;)
    {
    curnode--;
    sg_p = path_overflow[curnode];
    prev_cmp_p = PstSubg_Son_Get (sg_p);
    while (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (prev_cmp_p)) !=
      PstSubg_Father_Get (PstComp_Father_Get (cmp_p)))
      {
      prev_cmp_p = PstComp_Brother_Get (prev_cmp_p);
      if (prev_cmp_p == NULL)
        {
        printf ("Error in path_overflow lineage\n");
for (i=0; i<num_overflowed; i++) printf("SG %d at level %d\n",PstSubg_Index_Get(path_overflow[i]),overflow_level+i);
        }
      }
    if (prev_cmp_p == NULL) cont = FALSE;
    else
      {
      cmp_p = prev_cmp_p;
      if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
        cont = FALSE;
      else PstView_Mark_Store (cmp_infos, cmp_p);
      }
    }

  vlvl_p = PstView_PathVLvl_Get (pv_p);
  tlvl_p = ViewLvl_TreeLvl_Get (vlvl_p);
  for (curnode = TreeLvl_NCmps_Get (tlvl_p); curnode != 0 && cont;)
    {
    curnode--;
    prev_cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
    if (SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (prev_cmp_p)) ==
      PstSubg_Father_Get (PstComp_Father_Get (cmp_p)))
      {
      cmp_p = prev_cmp_p;
      if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
        cont = FALSE;
      else PstView_Mark_Store (cmp_infos, cmp_p);
      }
    }

  PstView_Display (pv_p);

  return (TRUE);
}
/*  End of PstView_SelCmp  */


/****************************************************************************
*
*  Function Name:                 SPstView_PthExp_CB
*
*    This routine handles the expose events for the pst tree drawing
*    area.  On an expose event for either drawing area, the corresponding
*    pixmap is copies over to the drawing area and any focus or select
*    nodes are rehighlighted.  
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPstView_PthExp_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  DrawCxt_t       *dc_p;
  XExposeEvent    *event;

  cb_p = (XmDrawingAreaCallbackStruct *) call_p;
  if (cb_p->reason == XmCR_EXPOSE)
    {
    dc_p = (DrawCxt_t *) client_p;

    /* Since we have a pixmap that stores the drawing of the symbols, 
       we need only copy the exposed area of the pixmap to the drawing
       area.
    */
    event = &cb_p->event->xexpose;
    XCopyArea (event->display, DrawCxt_Pmap_Get (dc_p), event->window,
      DrawCxt_GC_Get (dc_p), event->x, event->y, event->width, event->height,
      event->x, event->y);
    }

  return ;
}
/*  End of SPstView_PthExp_CB  */


/****************************************************************************
*
*  Function Name:                 SPstView_PthSel_CB
*
*    This routine handles the selection events for the pst path 
*    drawing area.  When a symbol is selected (the left mouse button 
*    pressed), the tree is updated to reflect the selection.  Action 
*    is taken on button presses--button releases are ignored. 
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPstView_PthSel_CB
  (
  Widget     w,                           /* Widget */
  XtPointer  client_data,                 /* PstView */
  XtPointer  call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  XButtonEvent *bevent;
  PsvTreeLvl_t *tlvl_p;
  PsvViewLvl_t *plvl_p;
  PstView_t    *pv_p;
  U16_t         curnode;

  cb_p = (XmDrawingAreaCallbackStruct *) call_p;
  pv_p = (PstView_t *) client_data;
  plvl_p = PstView_PathVLvl_Get (pv_p);
  tlvl_p = ViewLvl_TreeLvl_Get (plvl_p);

    /*  Only process left button presses with no state.  */
  if (cb_p->reason == XmCR_INPUT && cb_p->event->type == ButtonPress)
    {
    bevent = &cb_p->event->xbutton;
    if (bevent->state == 0 && (bevent->button == Button1 || bevent->button == Button3))
      {
      /*  Determine which node.  */
      if (bevent->y >= ViewLvl_SymYBeg_Get (plvl_p) 
          && bevent->y <= ViewLvl_SymYEnd_Get (plvl_p)
          && (bevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
          % AppDim_PstSymTotWd_Get (&GAppDim) 
          < AppDim_PstSymDim_Get (&GAppDim))
        {
        curnode = (U16_t)((bevent->x - AppDim_PstSymSepWd_Get (&GAppDim)) 
          / AppDim_PstSymTotWd_Get (&GAppDim));

        if (curnode < TreeLvl_NCmps_Get (tlvl_p))
          {
          TreeLvl_SelNd_Put (tlvl_p, curnode);
          SSelSym_Draw  (plvl_p); 
          PstView_Mouse_Remove (pv_p);
          if (PstVLvls_Update (pv_p, curnode, PVW_LEVEL_PATH))
            {
            pvdisp_reset_mouse = (bevent->button == Button1);
            PstView_Display (pv_p);
            if (bevent->button == Button3)
              {
              Compound_t    *cmp_p;
              PsvCmpInfo_t  *cmp_infos;

              cmp_infos = PstView_CmpInfo_Get (pv_p);
              cmp_p = TreeLvl_IthCmpNd_Get (tlvl_p, curnode);
              if (cmp_p != NULL)
                {
                plvl_p = PstView_PathVLvl_Get (pv_p);
                tlvl_p = ViewLvl_TreeLvl_Get (plvl_p);
                if (PsvCmpI_Marked_Get (cmp_infos[PstComp_Index_Get (cmp_p)]))
                  PstView_Unmark_Store (cmp_infos, cmp_p);
                else if (PsvCmpI_MarkElse_Get (cmp_infos[PstComp_Index_Get (
                     cmp_p)]))
                  PsvCmpI_MarkElse_Unset (cmp_infos[PstComp_Index_Get (
                     cmp_p)]);
                else
                  PstView_Mark_Store (cmp_infos, cmp_p);

                pvdisp_reset_mouse = TRUE;
                PstView_Display (pv_p);
                }
              else
                {
                PstView_Mouse_Reset (pv_p, Screen_TDisplay_Get (
                  SynAppR_ScrnAtrb_Get (&GSynAppR)));
                }
              }
            }
          else
            PstView_Mouse_Reset (pv_p, Screen_TDisplay_Get (
              SynAppR_ScrnAtrb_Get (&GSynAppR)));
          }
        }
      }
    }

  return ;
}
/*  End of SPstView_PthSel_CB  */


/****************************************************************************
*
*  Function Name:                 SPstView_Resize_CB
*
*    This routine handles the resize events for the two drawing areas of
*    the Pst viewing area.  Resizing is controlled by the Path DA,
*    so only it has the callback defined for resize events.  On a resize, 
*    the new width of the path drawing area is saved and it is checked 
*    whether it is necessary to increase the size of the corresponding 
*    pixmap.  The number of visible path nodes is recalculated and stored.
*    The new size of the clip window for the tree viewing area is retrieved 
*    and used to recalculate the number of visible nodes for the tree viewing 
*    area.  
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPstView_Resize_CB
  (
  Widget    w, 
  XtPointer client_p, 
  XtPointer call_p  
  )
{
  XmDrawingAreaCallbackStruct *cb_p;
  SynViewCB_t    *svcb_p;
  DrawCxt_t      *dc_p;
  PstView_t      *pv_p;
  RxnView_t      *rxnv_p;
  Dimension       da_wd, cw_w;
  Dimension       mf_h, mf_w;

  cb_p = (XmDrawingAreaCallbackStruct *) call_p;
  pv_p = (PstView_t *) client_p;
  rxnv_p = PstView_RxnView_Get (pv_p);
  svcb_p = SynView_Handle_Grab ();

  if (cb_p->reason == XmCR_RESIZE)
    {
    /*  First adjust the Path values.  */

  XtVaSetValues (SynVCB_TopApp_Get (svcb_p),
    XmNresizePolicy, XmRESIZE_ANY,
    NULL);

    dc_p = PstView_PathDC_Get (pv_p);
    XtVaGetValues (DrawCxt_DA_Get (dc_p),
      XmNwidth, &da_wd,
      NULL);

    /*  Reset the sizes of the mol forms and drawing areas in rxn view.  */
    XtVaGetValues (RxnView_MolForm_Get (rxnv_p),
      XmNheight, &mf_h,
      XmNwidth, &mf_w,
      NULL);

    RxnView_MolDAH_Put (rxnv_p, mf_h - RxnV_IthM_LblsH_Get (rxnv_p, 0));
    RxnView_MolFormW_Put (rxnv_p, mf_w);

    /* Check to see if the drawing area is wider than the existing
       pixmap.  If so, we have to create a new pixmap.  Otherwise, we
       can continue to use the same one.  We need to recalculate the 
       number of visible nodes in either case.  
    */
    if (DrawCxt_Pmap_Get (dc_p) && da_wd > DrawCxt_PmapW_Get (dc_p))
      {
      Pixmap        temp_pm;
      Display      *temp_dsp;
      Screen       *temp_scn;
      ScreenAttr_t *sca_p;

      sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
      temp_dsp = Screen_TDisplay_Get (sca_p);
      temp_scn = Screen_TScreen_Get (sca_p);
      temp_pm = XCreatePixmap (temp_dsp, Screen_RootWin_Get (sca_p), 
        da_wd, DrawCxt_DAH_Get (dc_p), Screen_Depth_Get (sca_p));

      /*  Copy contents of old pixmap over to the new one, then clear the
          remaining part of the new pixmap.
      */
      XCopyArea (temp_dsp, DrawCxt_Pmap_Get (dc_p), temp_pm, 
        DrawCxt_GC_Get (dc_p), 0, 0, DrawCxt_PmapW_Get (dc_p), 
        DrawCxt_DAH_Get (dc_p), 0, 0);
      XSetForeground (temp_dsp, DrawCxt_GC_Get (dc_p), 
        SynAppR_IthClrPx_Get (&GSynAppR, SAR_CLRI_PTHDABG));
      XSetFillStyle (temp_dsp, DrawCxt_GC_Get (dc_p), FillSolid);
      XFillRectangle (temp_dsp, temp_pm,
       DrawCxt_GC_Get (dc_p), DrawCxt_PmapW_Get (dc_p), 0, 
       da_wd - DrawCxt_PmapW_Get (dc_p), DrawCxt_DAH_Get (dc_p));
      XSetForeground (temp_dsp, DrawCxt_GC_Get (dc_p), 
        SynAppR_IthClrPx_Get (&GSynAppR, DrawCxt_GCFg_Get (dc_p)));
      XSetFillStyle (temp_dsp, DrawCxt_GC_Get (dc_p), 
        DrawCxt_GCFill_Get (dc_p));

      XFreePixmap (temp_dsp, DrawCxt_Pmap_Get (dc_p));
      DrawCxt_Pmap_Put (dc_p, temp_pm);
      DrawCxt_PmapW_Put (dc_p, da_wd);
      }

    DrawCxt_DAW_Put (dc_p, da_wd);
    DrawCxt_NumVsbNd_Put (dc_p, 
      (U16_t) ((da_wd > AppDim_PstSymSepWd_Get (&GAppDim)) ? (da_wd -
      AppDim_PstSymSepWd_Get (&GAppDim)) / AppDim_PstSymTotWd_Get (&GAppDim)
      : 0));

    /*  Now adjust the Tree values.  */
    dc_p = PstView_PstDC_Get (pv_p);
    XtVaGetValues (PstView_ClipWin_Get (pv_p),
      XmNwidth, &cw_w,
      NULL);
    PstView_ClipW_Put (pv_p, cw_w);
    DrawCxt_NumVsbNd_Put (dc_p, 
      (U16_t) ((cw_w > AppDim_PstSymSepWd_Get (&GAppDim)) ? (cw_w 
      - AppDim_PstSymSepWd_Get (&GAppDim)) / AppDim_PstSymTotWd_Get (&GAppDim)
      : 0));
    }

  XtVaSetValues (SynVCB_TopApp_Get (svcb_p),
    XmNresizePolicy, XmRESIZE_ANY,
    NULL);
    XFlush (XtDisplay (SynVCB_TopApp_Get (svcb_p)));

  RxnView_Update (rxnv_p, RxnView_CurSG_Get (rxnv_p));
  return ;
}
/*  End of SPstView_Resize_CB  */

Boolean_t PstView_AllSolCirc (PstView_t *pv_p, Compound_t *cmp_p, U8_t tlvl_num, Boolean_t in_view)
{
  U8_t           lvl_i;
  PsvTreeLvl_t  *tlvl_p;                /* Tree level for this view level */
  U32_t          cmp_ind;               /* Index of selected compound */
  U32_t          inxs[256];
  U16_t          sel_node;
  Subgoal_t     *sg;
  Boolean_t      ncsol;
  Compound_t    *dev_inst;

  if (!in_view) return (FALSE); /* oversimplified, but info not directly available! */

  cmp_ind = SymTab_Index_Get (PstComp_SymbolTable_Get (cmp_p));

  for (lvl_i = 0; lvl_i < tlvl_num && lvl_i < 255; lvl_i++)
    {
    tlvl_p = PstView_IthPTLvl_Get (pv_p, lvl_i);
    sel_node = TreeLvl_SelNd_Get (tlvl_p);

    if (sel_node == PVW_NODE_NONE) /* This may only have occurred as a result of a memory error in an earlier version */
{
printf("\007in_view=%s - lvl_i = %d - returning\n",in_view?"TRUE":"FALSE",lvl_i);
InfoWarn_Show ("Unexpected occurrence in circularity filtering - not all circularities may be marked.");
return (FALSE); /* default: non-circular until proven circular */
}

    inxs[lvl_i] = SymTab_Index_Get (PstComp_SymbolTable_Get (TreeLvl_IthCmpNd_Get (tlvl_p, sel_node)));

    if (cmp_ind == inxs[lvl_i]) return (TRUE);
    }

  if (lvl_i < tlvl_num)
    {
    InfoWarn_Show ("Excessive tree depth in circularity filtering - not all circularities may be marked.");
    return (FALSE); /* default: non-circular until proven circular */
    }

  inxs[lvl_i] = cmp_ind;

  return (!PstView_CompSolNC (cmp_p, inxs, lvl_i + 1));
}

Boolean_t PstView_CompSolNC (Compound_t *cmp_p, U32_t *inxs, int ninxs)
{
  Subgoal_t *sg;
  Compound_t *dev_inst;
  static int level = 0;
  static Boolean_t short_circuit = FALSE;

  if (level == 50) /* arbitrary cutoff to prevent excessive recursion */
  {
    if (!warning_shown)
    {
      InfoWarn_Show ("Predictive circularity checking is excessive - not all circular nodes detected.\n"
        "(See SynView Help->Intro for explanation.)");
      warning_shown = TRUE;
    }
    short_circuit = TRUE;
    return (TRUE);
  }
  level++;

  dev_inst = SymTab_DevelopedComp_Get (PstComp_SymbolTable_Get (cmp_p));
  if (dev_inst == NULL) /* available */
  {
    if (--level == 0) short_circuit = FALSE;
    return (TRUE);
  }
  sg = PstComp_Son_Get (dev_inst);

  while (sg != NULL && !short_circuit)
  {
    if (PstSubg_Merit_Solved_Get (sg) != SUBG_MERIT_INIT && PstView_SgSolNC (sg, inxs, ninxs))
    {
      if (--level == 0) short_circuit = FALSE;
      return (TRUE);
    }
    sg = PstSubg_Brother_Get (sg);
  }

  level--;
  if (!short_circuit) return (FALSE);
  if (level == 0) short_circuit = FALSE;
  return (TRUE);
}

Boolean_t PstView_SgSolNC (Subgoal_t *sg, U32_t *inxs, int ninxs) /* context-specific rendering of circularity */
{
  Compound_t *conj;
  int i;

  conj = PstSubg_Son_Get (sg);

  while (conj != NULL)
  {
    if (!SymTab_Flags_Solved_Get(PstComp_SymbolTable_Get(conj))) return(FALSE);
    inxs[ninxs] = SymTab_Index_Get (PstComp_SymbolTable_Get (conj));
    for (i = 0; i < ninxs; i++) if (inxs[i] == inxs[ninxs]) return (FALSE);
    if (ninxs == 255)
    {
      InfoWarn_Show ("Recursion too deep in circularity filtering - not all circularities may be marked.");
      return (TRUE); /* default: solved until proven unsolved */
    }
    conj = PstComp_Brother_Get (conj);
  }

  conj = PstSubg_Son_Get (sg);

  while (conj != NULL)
  {
    inxs[ninxs] = SymTab_Index_Get (PstComp_SymbolTable_Get (conj));
    if (!PstView_CompSolNC (conj, inxs, ninxs + 1)) return (FALSE);
    conj = PstComp_Brother_Get (conj);
  }

  return (TRUE);
}

/*  End of PST_VIEW.C  */

