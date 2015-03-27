/******************************************************************************
*
*  Copyright (C) 1995, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     APP_RESRC.C
*
*    This module defines the routines needed to initialize resources
*    (fonts, colors, pixmaps, etc.) used throughout the SYNCHEM GUI. 
*
*  Creation Date:
*
*    19-Jun-1995
*
*  Authors:
*
*    Daren Krebsbach
*
*  Routines:
*
*    SynAppR_PostInit
*    SynAppR_PreInit
*    SynAppR_Destroy
*
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 14-Sep-00  Miller     Added SynAppR_Manage_Windows which, by redefinition in
*                       app_resrc.h of XtManageChild and XtUnmanageChild as
*                       macros, will be invoked every time a widget's managed
*                       state changes (except for those which are created as
*                       managed widgets, which is not the case for applications
*                       and dialogs - the only ones of any consequence in this
*                       context.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>

#include <Xm/Xm.h>
#include <Xm/DialogS.h>
#include <X11/cursorfont.h>

#ifdef FALSE
#undef FALSE
#endif
#ifdef TRUE
#undef TRUE
#endif

#ifndef _H_SYNCHEM_
#include "synchem.h"
#endif

#define APPRESCR_GLOBALS 1
#ifndef _H_APP_RESRC_
#include "app_resrc.h"
#endif
#undef APPRESCR_GLOBALS

#ifndef _H_EXTERN_
#include "extern.h"
#endif

/* Static Variables */

static char    *color_error_msg = {
  "If you have some other color intensive application running, it is\n"
  "recommended that you exit that program and restart SynView.\n"
  };

static char    *color_names[SAR_CLR_MAXNUM] = {
  SAR_CLRN_DEFAULT,  SAR_CLRN_BLACK,    SAR_CLRN_BLUE,   SAR_CLRN_GOLD, 
  SAR_CLRN_GRAY,     SAR_CLRN_GREEN,    SAR_CLRN_RED,    SAR_CLRN_VIOLET, 
  SAR_CLRN_WHITE,    SAR_CLRN_AVL_VD,   SAR_CLRN_AVL_VE, SAR_CLRN_DEV_VD,
  SAR_CLRN_DEV_VE,   SAR_CLRN_SOL_VD,   SAR_CLRN_SOL_VE, SAR_CLRN_UND_VD,
  SAR_CLRN_UND_VE,   SAR_CLRN_UNS_VD,   SAR_CLRN_UNS_VE, SAR_CLRN_FOCUSBTM,
  SAR_CLRN_FOCUSMID, SAR_CLRN_FOCUSPTH, SAR_CLRN_MENUBG, SAR_CLRN_PSTDABG, 
  SAR_CLRN_PTHDABG,  SAR_CLRN_SURPDABG, SAR_CLRN_MARK,   SAR_CLRN_CYCLENUM
  };

static unsigned int cursor_fonts[SAR_CRSR_MAXNUM] = {
  SAR_CRSRN_DFLT,   SAR_CRSRN_CMPIVW, SAR_CRSRN_PSTVW,  SAR_CRSRN_PTHVW, 
  SAR_CRSRN_RXNPVW, SAR_CRSRN_RXNVW,  SAR_CRSRN_WAIT_W, SAR_CRSRN_CIRCLE, 
  SAR_CRSRN_CROSS,  SAR_CRSRN_PLUS,   SAR_CRSRN_TCROSS, SAR_CRSRN_UMBRLA, 
  SAR_CRSRN_ERASE, SAR_CRSRN_WRITE
  };

static char    *fontlist_names[SAR_FONTLIST_MAXNUM] = {
  SAR_FONTNAME_FDFLT_NML,  SAR_FONTNAME_FDFLT_LRG,
  SAR_FONTNAME_FDFLT_NML,  SAR_FONTNAME_FDFLT_SML,  SAR_FONTNAME_FDFLT_MIN,
  SAR_FONTNAME_ATOM_LRG,   SAR_FONTNAME_ATOM_NML, 
  SAR_FONTNAME_ATOM_SML,   SAR_FONTNAME_ATOM_MIN,
  SAR_FONTNAME_PSTLBL_NML, SAR_FONTNAME_PSTLBL_SML, SAR_FONTNAME_PSTLBL_MIN,
  SAR_FONTNAME_RXNLBL_NML, SAR_FONTNAME_RXNLBL_SML,
  SAR_FONTNAME_RXNNAM_NML, SAR_FONTNAME_RXNNAM_SML 
  };

static char    *fontlist_tags[SAR_FONTLIST_MAXNUM] = {
  NULL,                     SAR_FONTTAG_FIXED_LRG,
  SAR_FONTTAG_FIXED_NML,    SAR_FONTTAG_FIXED_SML,  SAR_FONTTAG_FIXED_MIN,
  SAR_FONTTAG_ATM_LRG,      SAR_FONTTAG_ATM_NML, 
  SAR_FONTTAG_ATM_SML,      SAR_FONTTAG_MAP_SML,
  SAR_FONTTAG_PSTLBL_NML,   SAR_FONTTAG_PSTLBL_SML, SAR_FONTTAG_PTHLBL_SML,
  SAR_FONTTAG_RXNLBL_NML,   SAR_FONTTAG_RXNLBL_SML,
  SAR_FONTTAG_RXNNAM_NML,   SAR_FONTTAG_RXNNAM_SML 
  };

static char    *fontstruct_names[SAR_FONTSTRCTS_MAXNUM] = {
  SAR_FONTNAME_ATOM_LRG, SAR_FONTNAME_ATOM_NML, SAR_FONTNAME_ATOM_SML
  };

/*  Take advantage of automatic string concatenation.  */ /* except it only works for non-portable constant strings! */
static char    *pix_names[SAR_PIXMAP_MAXNUM] = {
  /*SAR_DIR_BITMAPS*/ SAR_PIXMAP_FNAME_FLASK, 
  /*SAR_DIR_BITMAPS*/ SAR_STIPPLE_FNAME_HORIZ, 
  /*SAR_DIR_BITMAPS*/ SAR_STIPPLE_FNAME_NEG, 
  /*SAR_DIR_BITMAPS*/ SAR_STIPPLE_FNAME_POS, 
  /*SAR_DIR_BITMAPS*/ SAR_STIPPLE_FNAME_VERT,
  /*SAR_DIR_BITMAPS*/ SAR_PIXMAP_FNAME_SGLARW,
  /*SAR_DIR_BITMAPS*/ SAR_PIXMAP_FNAME_DBLARW,
  /*SAR_DIR_BITMAPS*/ SAR_PIXMAP_FNAME_PLUS
  };


/****************************************************************************
*
*  Function Name:                 AppDim_Init
*
*    This routine initializes the application dimensions according to the type
*    of display (workstation, pc or mini).
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources
*    GAppDim, SynView application dimensions
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
void AppDim_Init
  (
  U8_t         type
  )
{
  ScreenAttr_t  *sca_p;
  Dimension      app_width;

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  if (Screen_Height_Get (sca_p) < SAR_PC_MIN_HT)
    type |= SAR_APPSIZE_MINI_HT;
  else if (Screen_Height_Get (sca_p) < SAR_PC_MAX_HT)
    type |= SAR_APPSIZE_PC_HT;
  else
    type |= SAR_APPSIZE_WS_HT;

  AppDim_AppSize_Put (&GAppDim, type);

  /*  Set the application window sizes.  */
  if (type & SAR_APPSIZE_MINI_WD)
    {
    app_width = SMI_APPSIZE_WD - SPC_APPSHELL_OFFX;
    if (type & SAR_APPSIZE_MINI_HT)
      AppDim_AppHeight_Put (&GAppDim, SMI_APPSIZE_HT - SPC_APPSHELL_OFFY);
   else  
    AppDim_AppHeight_Put (&GAppDim,
      Screen_Height_Get (sca_p) - SWS_APPSHELL_OFFY);
    }
   else
    {
    app_width = Screen_Width_Get (sca_p) - SWS_APPSHELL_OFFX;
    AppDim_AppHeight_Put (&GAppDim,
      Screen_Height_Get (sca_p) - SWS_APPSHELL_OFFY);
    }

  if (type & SAR_APPSIZE_HALF_WD)
    {
    app_width = app_width >> 1;
    }
  else if (type & SAR_APPSIZE_DFLT_WD)
    {
    app_width = (app_width >> 1) + (app_width >> 2);
    }

  AppDim_AppWidth_Put (&GAppDim, app_width);

  /*  Set the label, text and margin sizes.
      Set the pst view and reaction view sizes.  
      Set the submission panel sizes.
      Set the font sizes.
      Set the molecule sizes.
  */
  if (type & SAR_APPSIZE_MINI_HT || type & SAR_APPSIZE_MINI_WD)
    {
    AppDim_HtLblPB_Put (&GAppDim, SMI_LBL_PB_HT);
    AppDim_HtTxt_Put (&GAppDim, SMI_TXT_HT);
    AppDim_MargFmFr_Put (&GAppDim, SMI_FM_FR_MARG);
    AppDim_MargLblPB_Put (&GAppDim, SMI_LBL_PB_MARG);
    AppDim_SepLarge_Put (&GAppDim, SMI_LARGE_SEP);
    AppDim_SepSmall_Put (&GAppDim, SMI_SMALL_SEP);

    AppDim_PstBarHt_Put (&GAppDim, SMI_PST_BARS_HT);
    AppDim_PstCycHt_Put (&GAppDim, SMI_PST_CYC_HT);
    AppDim_PstFBoxOff_Put (&GAppDim, SMI_PST_FBOX_OFF);
    AppDim_PstLblMarg_Put (&GAppDim, SMI_PST_LBL_MARG);
    AppDim_PstLinesHt_Put (&GAppDim, SMI_PST_LINES_HT);
    AppDim_PstMargHt_Put (&GAppDim, SMI_PST_MARG_HT);
    AppDim_PstSBoxOff_Put (&GAppDim, SMI_PST_SBOX_OFF);
    AppDim_PstSymDim_Put (&GAppDim, SMI_PST_SYM_DIM);
    AppDim_PstSymMid_Put (&GAppDim, SMI_PST_SYM_MID);
    AppDim_PstSymSepHt_Put (&GAppDim, SMI_PST_SYM_SEP_HT);
    AppDim_PstSymSepWd_Put (&GAppDim, SMI_PST_SYM_SEP_WD);
    AppDim_PthBarHt_Put (&GAppDim, SMI_PTH_BARS_HT);
    AppDim_PthCycHt_Put (&GAppDim, SMI_PTH_CYC_HT);
    AppDim_PthLNumHt_Put (&GAppDim, SMI_PTH_LNUM_HT);
    AppDim_PthMargHt_Put (&GAppDim, SMI_PTH_MARG_HT);
    AppDim_RxnNameHt_Put (&GAppDim, SMI_RXN_NAME_HT);
    AppDim_RxnValHt_Put (&GAppDim, SMI_RXN_VAL_HT);
    AppDim_XHtPvwBB_Put (&GAppDim, SMI_PVW_BB_XHT);
    AppDim_XHtPvwDA_Put (&GAppDim, SMI_PVW_DA_XHT);

    AppDim_SbtExitWd_Put (&GAppDim, SMI_SBT_EXIT_WD);
    AppDim_SbtPanel_Wd_Put (&GAppDim, SMI_SBT_PANEL_WD);

    AppDim_AtmDftTag_Put (&GAppDim, SAR_FONTTAG_ATM_SML);
    AppDim_MapDftTag_Put (&GAppDim, SAR_FONTTAG_MAP_SML);
    AppDim_PstCycTag_Put (&GAppDim, SAR_FONTTAG_PSTCYC_SML);
    AppDim_PstLblTag_Put (&GAppDim, SAR_FONTTAG_PSTLBL_SML);
    AppDim_PstNSGTag_Put (&GAppDim, SAR_FONTTAG_PSTNSG_SML);
    AppDim_PthLblTag_Put (&GAppDim, SAR_FONTTAG_PTHLBL_SML);
    AppDim_RxnLblTag_Put (&GAppDim, SAR_FONTTAG_RXNLBL_SML);
    AppDim_RxnMrtTag_Put (&GAppDim, SAR_FONTTAG_RXNMRT_SML);
    AppDim_RxnNamTag_Put (&GAppDim, SAR_FONTTAG_RXNNAM_SML);
    AppDim_RxnValTag_Put (&GAppDim, SAR_FONTTAG_RXNVAL_SML);

    AppDim_BndDblOff_Put (&GAppDim, SMI_BOND_DBL_OFF);
    AppDim_BndTplOff_Put (&GAppDim, SMI_BOND_TPL_OFF);
    AppDim_MargMol_Put (&GAppDim, SMI_MOL_MARG);
    AppDim_MolScale_Put (&GAppDim, SMI_MOL_SCALE);
    }

  else if (type & SAR_APPSIZE_HALF_WD || type & SAR_APPSIZE_PC_HT)
    {
    AppDim_HtLblPB_Put (&GAppDim, SPC_LBL_PB_HT);
    AppDim_HtTxt_Put (&GAppDim, SPC_TXT_HT);
    AppDim_MargFmFr_Put (&GAppDim, SPC_FM_FR_MARG);
    AppDim_MargLblPB_Put (&GAppDim, SPC_LBL_PB_MARG);
    AppDim_SepLarge_Put (&GAppDim, SPC_LARGE_SEP);
    AppDim_SepSmall_Put (&GAppDim, SPC_SMALL_SEP);
 
    AppDim_PstBarHt_Put (&GAppDim, SPC_PST_BARS_HT);
    AppDim_PstCycHt_Put (&GAppDim, SPC_PST_CYC_HT);
    AppDim_PstFBoxOff_Put (&GAppDim, SPC_PST_FBOX_OFF);
    AppDim_PstLblMarg_Put (&GAppDim, SPC_PST_LBL_MARG);
    AppDim_PstLinesHt_Put (&GAppDim, SPC_PST_LINES_HT);
    AppDim_PstMargHt_Put (&GAppDim, SPC_PST_MARG_HT);
    AppDim_PstSBoxOff_Put (&GAppDim, SPC_PST_SBOX_OFF);
    AppDim_PstSymDim_Put (&GAppDim, SPC_PST_SYM_DIM);
    AppDim_PstSymMid_Put (&GAppDim, SPC_PST_SYM_MID);
    AppDim_PstSymSepHt_Put (&GAppDim, SPC_PST_SYM_SEP_HT);
    AppDim_PstSymSepWd_Put (&GAppDim, SPC_PST_SYM_SEP_WD);
    AppDim_PthBarHt_Put (&GAppDim, SPC_PTH_BARS_HT);
    AppDim_PthCycHt_Put (&GAppDim, SPC_PTH_CYC_HT);
    AppDim_PthLNumHt_Put (&GAppDim, SPC_PTH_LNUM_HT);
    AppDim_PthMargHt_Put (&GAppDim, SPC_PTH_MARG_HT);
    AppDim_RxnNameHt_Put (&GAppDim, SPC_RXN_NAME_HT);
    AppDim_RxnValHt_Put (&GAppDim, SPC_RXN_VAL_HT);
    AppDim_XHtPvwBB_Put (&GAppDim, SPC_PVW_BB_XHT);
    AppDim_XHtPvwDA_Put (&GAppDim, SPC_PVW_DA_XHT);

    AppDim_SbtExitWd_Put (&GAppDim, SPC_SBT_EXIT_WD);
    AppDim_SbtPanel_Wd_Put (&GAppDim, SPC_SBT_PANEL_WD);

    AppDim_AtmDftTag_Put (&GAppDim, SAR_FONTTAG_ATM_NML);
    AppDim_MapDftTag_Put (&GAppDim, SAR_FONTTAG_MAP_NML);
    AppDim_PstCycTag_Put (&GAppDim, SAR_FONTTAG_PSTCYC_NML);
    AppDim_PstLblTag_Put (&GAppDim, SAR_FONTTAG_PSTLBL_SML);
    AppDim_PstNSGTag_Put (&GAppDim, SAR_FONTTAG_PSTNSG_SML);
    AppDim_PthLblTag_Put (&GAppDim, SAR_FONTTAG_PTHLBL_SML);
    AppDim_RxnLblTag_Put (&GAppDim, SAR_FONTTAG_RXNLBL_SML);
    AppDim_RxnMrtTag_Put (&GAppDim, SAR_FONTTAG_RXNMRT_SML);
    AppDim_RxnNamTag_Put (&GAppDim, SAR_FONTTAG_RXNNAM_SML);
    AppDim_RxnValTag_Put (&GAppDim, SAR_FONTTAG_RXNVAL_SML);

    AppDim_BndDblOff_Put (&GAppDim, SPC_BOND_DBL_OFF);
    AppDim_BndTplOff_Put (&GAppDim, SPC_BOND_TPL_OFF);
    AppDim_MargMol_Put (&GAppDim, SPC_MOL_MARG);
    AppDim_MolScale_Put (&GAppDim, SPC_MOL_SCALE);
    }

  else
    {
    AppDim_HtLblPB_Put (&GAppDim, SWS_LBL_PB_HT);
    AppDim_HtTxt_Put (&GAppDim, SWS_TXT_HT);
    AppDim_MargFmFr_Put (&GAppDim, SWS_FM_FR_MARG);
    AppDim_MargLblPB_Put (&GAppDim, SWS_LBL_PB_MARG);
    AppDim_SepLarge_Put (&GAppDim, SWS_LARGE_SEP);
    AppDim_SepSmall_Put (&GAppDim, SWS_SMALL_SEP);

    AppDim_PstBarHt_Put (&GAppDim, SWS_PST_BARS_HT);
    AppDim_PstCycHt_Put (&GAppDim, SWS_PST_CYC_HT);
    AppDim_PstFBoxOff_Put (&GAppDim, SWS_PST_FBOX_OFF);
    AppDim_PstLblMarg_Put (&GAppDim, SWS_PST_LBL_MARG);
    AppDim_PstLinesHt_Put (&GAppDim, SWS_PST_LINES_HT);
    AppDim_PstMargHt_Put (&GAppDim, SWS_PST_MARG_HT);
    AppDim_PstSBoxOff_Put (&GAppDim, SWS_PST_SBOX_OFF);
    AppDim_PstSymDim_Put (&GAppDim, SWS_PST_SYM_DIM);
    AppDim_PstSymMid_Put (&GAppDim, SWS_PST_SYM_MID);
    AppDim_PstSymSepHt_Put (&GAppDim, SWS_PST_SYM_SEP_HT);
    AppDim_PstSymSepWd_Put (&GAppDim, SWS_PST_SYM_SEP_WD);
    AppDim_PthBarHt_Put (&GAppDim, SWS_PTH_BARS_HT);
    AppDim_PthCycHt_Put (&GAppDim, SWS_PTH_CYC_HT);
    AppDim_PthLNumHt_Put (&GAppDim, SWS_PTH_LNUM_HT);
    AppDim_PthMargHt_Put (&GAppDim, SWS_PTH_MARG_HT);
    AppDim_RxnNameHt_Put (&GAppDim, SWS_RXN_NAME_HT);
    AppDim_RxnValHt_Put (&GAppDim, SWS_RXN_VAL_HT);
    AppDim_XHtPvwBB_Put (&GAppDim, SWS_PVW_BB_XHT);
    AppDim_XHtPvwDA_Put (&GAppDim, SWS_PVW_DA_XHT);

    AppDim_SbtExitWd_Put (&GAppDim, SWS_SBT_EXIT_WD);
    AppDim_SbtPanel_Wd_Put (&GAppDim, SWS_SBT_PANEL_WD);

    AppDim_AtmDftTag_Put (&GAppDim, SAR_FONTTAG_ATM_LRG);
    AppDim_MapDftTag_Put (&GAppDim, SAR_FONTTAG_MAP_LRG);
    AppDim_PstCycTag_Put (&GAppDim, SAR_FONTTAG_PSTCYC_NML);
    AppDim_PstLblTag_Put (&GAppDim, SAR_FONTTAG_PSTLBL_NML);
    AppDim_PstNSGTag_Put (&GAppDim, SAR_FONTTAG_PSTNSG_NML);
    AppDim_PthLblTag_Put (&GAppDim, SAR_FONTTAG_PTHLBL_NML);
    AppDim_RxnLblTag_Put (&GAppDim, SAR_FONTTAG_RXNLBL_NML);
    AppDim_RxnMrtTag_Put (&GAppDim, SAR_FONTTAG_RXNMRT_NML);
    AppDim_RxnNamTag_Put (&GAppDim, SAR_FONTTAG_RXNNAM_NML);
    AppDim_RxnValTag_Put (&GAppDim, SAR_FONTTAG_RXNVAL_NML);

    AppDim_BndDblOff_Put (&GAppDim, SWS_BOND_DBL_OFF);
    AppDim_BndTplOff_Put (&GAppDim, SWS_BOND_TPL_OFF);
    AppDim_MargMol_Put (&GAppDim, SWS_MOL_MARG);
    AppDim_MolScale_Put (&GAppDim, SWS_MOL_SCALE);
    }

  AppDim_PstLvlTotHt_Put (&GAppDim, 2 * AppDim_PstSymSepHt_Get (&GAppDim)
    + AppDim_PstSymDim_Get (&GAppDim) + AppDim_PstBarHt_Get (&GAppDim) 
    + AppDim_PstLinesHt_Get (&GAppDim));

  AppDim_PstSymTotWd_Put (&GAppDim, AppDim_PstSymDim_Get (&GAppDim) 
    + AppDim_PstSymSepWd_Get (&GAppDim));

  AppDim_PstLvlBdrTop_Put (&GAppDim, AppDim_PstLvlTotHt_Get (&GAppDim)
    - AppDim_PstSymSepHt_Get (&GAppDim) - AppDim_PstSymDim_Get (&GAppDim));

  AppDim_PstLvlBdrBtm_Put (&GAppDim, AppDim_PstLvlTotHt_Get (&GAppDim)
    - AppDim_PstSymSepHt_Get (&GAppDim));

  AppDim_PstLvlBdrOff_Put (&GAppDim, AppDim_PstLinesHt_Get (&GAppDim)
    - AppDim_PstMargHt_Get (&GAppDim));

  return;
}
/*  End of AppDim_Init  */

/****************************************************************************
*
*  Function Name:                 SynAppR_PostInit
*
*    This routine initializes the pixmaps, which have to be done after
*    the widget hierarchy has been realized and we have a valid window.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources
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
void SynAppR_PostInit
  (
  Widget         top
  )
{
  GC             gc;
  ScreenAttr_t  *sca_p;
  Display       *tdisplay;
  Window         rootwin, twin;
  int            pixhotx, pixhoty;
  int            pixerror;
  U8_t           pix_i;

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  tdisplay = Screen_TDisplay_Get (sca_p);
  twin = XtWindow (top);
  Screen_TWindow_Put (sca_p, twin);
  rootwin = Screen_RootWin_Get (sca_p);

  for (pix_i = 0; pix_i < SAR_PIXMAP_MAXNUM; pix_i++)
    {
    pixerror = XReadBitmapFile (tdisplay, rootwin, SAR_DIR_BITMAPS (pix_names[pix_i]),
      &SynAppR_IthPMapW_Get (&GSynAppR, pix_i), 
      &SynAppR_IthPMapH_Get (&GSynAppR, pix_i),
      &SynAppR_IthPMap_Get (&GSynAppR, pix_i), &pixhotx, &pixhoty);
    if (pixerror != BitmapSuccess)
      {
      fprintf (stderr, 
      "\nError creating pixmap:  %s ...\n", SAR_DIR_BITMAPS (pix_names[pix_i]));
      switch (pixerror)
        {
        case BitmapOpenFailed:
          fprintf (stderr, "  Unable to open file.\n");
          break;
        case BitmapFileInvalid:
          fprintf (stderr, "  Wrong file format.\n");
          break;
        case BitmapNoMemory:
          fprintf (stderr, "  Unable to allocate memory.\n");
          break;
        default:
          fprintf (stderr, "  Unknown error.\n");
          break;
        }

      exit (-1);
      }
    }

  gc = XCreateGC (tdisplay, rootwin, 0, NULL);
  XSetBackground (tdisplay, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_WHITE));
  XSetForeground (tdisplay, gc, SynAppR_IthClrPx_Get (&GSynAppR, 
    SAR_CLRI_BLACK));
  XSetFillStyle (tdisplay, gc, FillSolid);
  XSetLineAttributes (tdisplay, gc, 1, LineSolid, CapButt, JoinMiter);
  XSetFont (tdisplay, gc, SynAppR_IthFont_Get (&GSynAppR, 
    SAR_FONTSTRCTS_NRML)->fid);
  SynAppR_MolGC_Put (&GSynAppR, gc);

  return ;
}

/*  End of SynAppR_PostInit  */


/****************************************************************************
*
*  Function Name:                 SynAppR_PreInit
*
*    This routine initializes the colors and fonts, which has to be done 
*    after the top application widget has been initialized, but before 
*    the other widgets are created.  It also creates the cursors.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources
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
void SynAppR_PreInit
  (
  Widget         top,
  U8_t           type
  )
{
  ScreenAttr_t  *sca_p;
  XmFontListEntry tempfonts[SAR_FONTLIST_MAXNUM];
  Display        *tdisplay;
  Screen         *tscreen;
  Colormap        cmap;
  XmString        ls;
  Dimension       lsh, lsw;
  XColor          exact;
  U8_t            cerr_cnt;
  U8_t            color_i;
  U8_t            curs_i;
  U8_t            font_i;

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);

  /*  Store the top level application widget and values of the screen.  */
  tscreen = XtScreen (top);
#ifdef _LANGUAGE_C_PLUS_PLUS
  Screen_Class_Put (sca_p, (DefaultVisualOfScreen (tscreen))->c_class);
#else
  Screen_Class_Put (sca_p, (DefaultVisualOfScreen (tscreen))->class);
#endif
  tdisplay = DisplayOfScreen (tscreen);
  Screen_TScreen_Put (sca_p, tscreen);
  Screen_TDisplay_Put (sca_p, tdisplay);
  Screen_Width_Put (sca_p, WidthOfScreen (tscreen));
  Screen_Height_Put (sca_p, HeightOfScreen (tscreen));
  Screen_Depth_Put (sca_p, DefaultDepthOfScreen (tscreen));
  Screen_RootWin_Put (sca_p, RootWindowOfScreen (tscreen));

  AppDim_Init (type);

  /*  Get a handle on the default colormap and allocate the colors.  The 
      default color for the application is assumed to be the first color 
      (index 0).  
  */
  cmap = DefaultColormapOfScreen (tscreen);
  SynAppR_Colormap_Put (&GSynAppR, cmap);

  if (!XAllocNamedColor (tdisplay, cmap, color_names[SAR_CLRI_DEFAULT], 
    &SynAppR_IthColor_Get (&GSynAppR, SAR_CLRI_DEFAULT), &exact))
    {
    fprintf (stderr, "Unable to allocate default color:  %s\n", 
      SAR_CLRN_DEFAULT);
    fprintf (stderr, "%s\n", color_error_msg);
    exit (-1);
    };

  cerr_cnt = 0;
  for (color_i = 1; color_i < SAR_CLR_MAXNUM; color_i++)
    {
    if (!XAllocNamedColor (tdisplay, cmap, color_names[color_i], 
      &SynAppR_IthColor_Get (&GSynAppR, color_i), &exact))
      {
      fprintf (stderr, "Unable to allocate color:  %s ... using default.\n", 
        color_names[color_i]);
      cerr_cnt++;
      XAllocNamedColor (tdisplay, cmap, color_names[SAR_CLRI_DEFAULT], 
        &SynAppR_IthColor_Get (&GSynAppR, color_i), &exact);
      };
    };

  if (cerr_cnt > 0)
    fprintf (stderr, "%s\n", color_error_msg);

  /*  Load the first font (the default for labels, buttons and text),
      and intialize the Motif font list.  Then load the rest of the fonts.
      A few fonts stored in the X Windows structure are
      needed, so we will load those as well.  
  */ 
  if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_MINI_HT 
      || AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_MINI_WD)
    {
    tempfonts[0] = XmFontListEntryLoad (tdisplay, SAR_FONTNAME_FDFLT_SML, 
      XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
    }

  else if (AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_PC_HT 
      || AppDim_AppSize_Get (&GAppDim) & SAR_APPSIZE_HALF_WD)
    {
    tempfonts[0] = XmFontListEntryLoad (tdisplay, SAR_FONTNAME_FDFLT_NML, 
      XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
    }

  else
    {
    tempfonts[0] = XmFontListEntryLoad (tdisplay, SAR_FONTNAME_FDFLT_LRG, 
      XmFONT_IS_FONT, XmFONTLIST_DEFAULT_TAG_STRING);
    }

  SynAppR_FontList_Put (&GSynAppR, XmFontListAppendEntry (NULL, tempfonts[0]));
  for (font_i = 1; font_i < SAR_FONTLIST_MAXNUM; font_i++)
    {
    tempfonts[font_i] = XmFontListEntryLoad (tdisplay, 
      fontlist_names[font_i], XmFONT_IS_FONT, fontlist_tags[font_i]);
    SynAppR_FontList_Put (&GSynAppR, XmFontListAppendEntry 
      (SynAppR_FontList_Get (&GSynAppR), tempfonts[font_i]));
    }

  for (font_i = 0; font_i < SAR_FONTSTRCTS_MAXNUM; font_i++)
    SynAppR_IthFont_Put (&GSynAppR, font_i, XLoadQueryFont 
      (tdisplay, fontstruct_names[font_i]));


  /*  Calculate the maximum dimensions of an atom figure (with charge 
      and subscript) and set the default bond offset and molecule scale 
      factor.  
  */
  ls = XmStringCreateLtoR ("Mg", SAR_FONTTAG_ATM_SML);
  XmStringExtent (SynAppR_FontList_Get (&GSynAppR), ls, &lsw, &lsh);
  AppDim_AtmMaxH_Put (&GAppDim, lsw);
  AppDim_AtmMaxW_Put (&GAppDim, lsw);
  XmStringFree (ls);

    /*
    XmString       ls, lm;
    Dimension      lsh, lsw, lmh, lmw;

    ls = XmStringCreateLtoR ("Mg", AppDim_AtmDftTag_Get (&GAppDim));
    lm = XmStringCreateLtoR ("88", AppDim_MapDftTag_Get (&GAppDim));
    XmStringExtent (SynAppR_FontList_Get (&GSynAppR), ls, &lsw, &lsh);
    XmStringExtent (SynAppR_FontList_Get (&GSynAppR), lm, &lmw, &lmh);
    AppDim_AtmMaxH_Put (&GAppDim, lsw + 2 * lmh);
    AppDim_AtmMaxW_Put (&GAppDim, lsw + 2 * lmw);
    XmStringFree (ls);
    XmStringFree (lm);
    */


  /*  Create the cursors.  */
  for (curs_i = 0; curs_i < SAR_CRSR_MAXNUM; curs_i++)
    SynAppR_IthCursor_Put (&GSynAppR, curs_i, XCreateFontCursor 
      (tdisplay, cursor_fonts[curs_i]));

  return;
}
/*  End of SynAppR_PreInit  */


/****************************************************************************
*
*  Function Name:                 SynAppR_Destroy
*
*    This routine destroys the colors, pixmaps and fonts.
*
*
*  Implicit Inputs:
*
*    GSynAppR, SynView application resources
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
void SynAppR_Destroy
  (
  void
  )
{
  ScreenAttr_t  *sca_p;
  Display       *tdisplay;
  U8_t           font_i;
  U8_t           pix_i;

  sca_p = SynAppR_ScrnAtrb_Get (&GSynAppR);
  tdisplay = Screen_TDisplay_Get (sca_p);

  /*  Free the pixmaps and font structures.  I am not sure how to handle 
      the font list and colors, so we won't bother for now :-).
  */
  for (pix_i = 0; pix_i < SAR_PIXMAP_MAXNUM; pix_i++)
    XFreePixmap (tdisplay, SynAppR_IthPMap_Get (&GSynAppR, pix_i));
  
  for (font_i = 0; font_i < SAR_FONTSTRCTS_MAXNUM; font_i++)
    XFreeFont (tdisplay, SynAppR_IthFont_Get (&GSynAppR, font_i));

  return ;
}
/*  End of SynAppR_Destroy  */


/****************************************************************************
*
*  Function Name:                 SynAppR_Manage_Windows
*
*    This routine restacks the windows in an intuitive fashion.  It is called
*    immediately after each call to XtManageChild with the widget that was
*    just managed, as well as after each XtUnmanageChild with a NULL widget.
*
*    The development of this function has emphasized just how flaky Motif
*    and the Hummingbird Window Manager are.  Restacking is impossible,
*    because the root window has forgotten that the windows that are
*    supposed to be underneath are even among its children, and those
*    windows often become unmanaged, despite the fact that no call to
*    XtUnmanageChild has been made - and they are not message dialogs
*    for which the OK button unmanages them.  In fact, these widgets are
*    both visible and supposedly unmanaged!  K.I.S.S.!!
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
void SynAppR_Manage_Windows
  (
  Widget    w,
  Boolean_t manage
  )
{
  Display *disp;
  Widget   parent;
  Window   root, pwin, rw, pw, *cw;
  XWindowAttributes watt;
  int      screen, plen, i, j, nc, counter;
  char     *pname;
  Boolean_t ispopup, found, redundant, all_parented, reverse;
  static Widget managed_widgets[128];
  static int nmanaged=0;

parent=XtParent(w);
disp=XtDisplay(parent);
screen=DefaultScreen(disp);
root=RootWindow(disp,screen);
pname=XtName(parent);
plen=strlen(pname);
pwin=XtWindow(parent);

ispopup=plen>6 && strcmp(pname+plen-6,"_popup")==0;

if (!manage)
  {
  redundant=!XtIsManaged(w);
/*
  if (ispopup && XtIsRealized(w)) printf("before XtUnmanageChild (%p->%s [%d])%s\n", w, pname, pwin, redundant?" redundant":"");
*/
  XtUnmanageChild (w);
  if (!ispopup || !XtIsRealized(w)) return;
  if (nmanaged==0)
    {
/*
    printf("SynAppR_Manage_Windows: Attempt to unmanage unknown widget\n");
*/
    return;
    }
  }
else
  {
  redundant=XtIsManaged(w);
  XtManageChild (w);
  if (!ispopup || !XtIsRealized(w)) return;
/*
  printf("after XtManageChild (%p->%s [%d])%s\n", w, pname, pwin, redundant?" redundant":"");
*/
  if (nmanaged==127)
    {
    printf("SynAppR_Manage_Windows: Attempt to overflow managed widget array\n");
    return;
    }
  }

if (XtClass(parent) != xmDialogShellWidgetClass) return;

if (manage)
  {
  for (i=0, found=FALSE; i<nmanaged && !found; i++) if (managed_widgets[i]==parent) found=TRUE;
  if (!found)
    {
    for (j=nmanaged++; j>0; j--) managed_widgets[j]=managed_widgets[j-1];
    managed_widgets[0]=parent;
    }
/*
  else printf("already in list\n");
*/
  }
else
  {
  for (i=0, found=FALSE; i<nmanaged && !found; i++) if (managed_widgets[i]==parent)
    {
    found=TRUE;
    nmanaged--;
    for (j=i; j<nmanaged; j++) managed_widgets[j]=managed_widgets[j+1];
    }
/*
  if (!found) printf("not found in list\n");
*/
  }

if (!manage && nmanaged>2)
  {
  XQueryTree(disp,root,&rw,&pw,&cw,&nc);
  for (i=j=0, found=FALSE; j<nc && !found; j++) if (cw[j]==XtWindow(managed_widgets[i])) found=TRUE;
  if (!found)
    {
    if (!XtIsManaged(XtWindowToWidget(disp,XtWindow(managed_widgets[i])))) XtManageChild(managed_widgets[i]);
    XGetWindowAttributes(disp,XtWindow(managed_widgets[i]),&watt);
    XReparentWindow(disp,XtWindow(managed_widgets[i]),root,watt.x,watt.y);
    }
  XFree(cw);
  XRaiseWindow(disp,XtWindow(managed_widgets[i]));
  }

/*
XQueryTree(disp,root,&rw,&pw,&cw,&nc);
for (i=0; i<nmanaged; i++)
  {
  printf("\t%3d: %s window=%d\n",i,XtName(managed_widgets[i]),XtWindow(managed_widgets[i]));
  for (j=0, found=FALSE; j<nc && !found; j++) if (cw[j]==XtWindow(managed_widgets[i]))
    {
    found=TRUE;
    printf("\t\t(Child #%d of root)\n",j);
    }
  if (!found) printf("\t\t(%s - Not found among children of root!)\n",
    XtIsManaged(managed_widgets[i])?"Managed":"Unmanaged");
  }
XFree(cw);
*/
}

/*  End of APP_RESRC.C  */
