#ifndef _H_INFO_VIEW_
#define _H_INFO_VIEW_  1
/******************************************************************************
*
*  Copyright (C) 1996 Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     INFO_VIEW.H
*
*    This header file defines the data structure for managing
*    the information window. 
*
*
*  Creation Date:
*
*    16-Nov-1996
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

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_RCB_
#include "rcb.h"
#endif

/*** Literal Values ***/

#define SVI_INFO_NONE         0 
#define SVI_INFO_STAT         1
#define SVI_INFO_STAT_LOAD    2
#define SVI_INFO_COMPOUND     3
#define SVI_INFO_SUBGOAL      4
#define SVI_INFO_RXN          5

#define SVI_INFO_BUFSZ        8192

#define SVI_INFO_TITLE_NONE   "Information Window"
#define SVI_INFO_TITLE_STAT   "Information:  Run Controls and Statistics"
#define SVI_INFO_TITLE_STLD   "Information:  Status File Load"
#define SVI_INFO_TITLE_COMP   "Information:  Compound Node"
#define SVI_INFO_TITLE_SUBG   "Information:  Subgoal Node"
#define SVI_INFO_TITLE_RXN    "Information:  Reaction"

#define SVI_INFO_PB_DISMISS    "dismiss"
#define SVI_INFO_PB_LOAD       "load"
#define SVI_INFO_PB_SELECT     "select"
#define SVI_INFO_PB_SELECT_CMP "select compound"
#define SVI_INFO_PB_SELECT_SG  "select subgoal"

#define SVI_INFO_NUMCOLS      80
#define SVI_INFO_NUMROWS      25

#define SVI_INFO_ROWS_CMP     12
#define SVI_INFO_ROWS_RXN     10
#define SVI_INFO_ROWS_SG      8

/*** Data Structures ***/

typedef struct svi_infowin_s
  {
  Widget         infodlg;             /* Info window form dialog */ 
  Widget         selpb;               /* Comp/Subgoal select button */ 
  Widget         textwin;             /* Scrollable text window */
  char           filename[MX_FILENAME]; /* File to read info from */
  char           txtbuf[SVI_INFO_BUFSZ]; /* Buffer for text window */ 
  U8_t           type;                /* Which type of info? */
}  InfoWin_t;

/** Field Access Macros for InfoWin_t **/

/* Macro Prototypes

  char           *InfoWin_Filename_Get  (InfoWin_t);
  Widget          InfoWin_InfoDlg_Get   (InfoWin_t);
  void            InfoWin_InfoDlg_Put   (InfoWin_t, Widget);
  Widget          InfoWin_SelectPB_Get  (InfoWin_t);
  void            InfoWin_SelectPB_Put  (InfoWin_t, Widget);
  char           *InfoWin_TextBuf_Get   (InfoWin_t);
  Widget          InfoWin_TextWin_Get   (InfoWin_t);
  void            InfoWin_TextWin_Put   (InfoWin_t, Widget);
  U8_t            InfoWin_Type_Get      (InfoWin_t);
  void            InfoWin_Type_Put      (InfoWin_t, U8_t);
*/

#define InfoWin_Filename_Get(iw)\
  (iw).filename

#define InfoWin_InfoDlg_Get(iw)\
  (iw).infodlg
#define InfoWin_InfoDlg_Put(iw, value)\
  (iw).infodlg = (value)

#define InfoWin_SelectPB_Get(iw)\
  (iw).selpb
#define InfoWin_SelectPB_Put(iw, value)\
  (iw).selpb = (value)

#define InfoWin_TextBuf_Get(iw)\
  (iw).txtbuf

#define InfoWin_TextWin_Get(iw)\
  (iw).textwin
#define InfoWin_TextWin_Put(iw, value)\
  (iw).textwin = (value)

#define InfoWin_Type_Get(iw)\
  (iw).type
#define InfoWin_Type_Put(iw, value)\
  (iw).type = (value)

/*** Macros ***/

/* Macro Prototypes

*/

/*** Routine Prototypes ***/

void       InfoMess_Create          (Widget);
void       InfoMess_Dismiss_CB      (Widget, XtPointer, XtPointer);
void       InfoMess_Show            (char *);

void       InfoWarn_Create          (Widget);
void       InfoWarn_Dismiss_CB      (Widget, XtPointer, XtPointer);
void       InfoWarn_Show            (char *);

void       InfoWin_Create           (Widget, U8_t);
void       InfoWin_Dismiss_CB       (Widget, XtPointer, XtPointer);
InfoWin_t *InfoWin_Handle_Get       (U8_t);

void       InfoWin_Compound_Update  (Widget, XtPointer, XtPointer);
void       InfoWin_Reaction_Update  (void);
void       InfoWin_Subgoal_Update   (Widget, XtPointer, XtPointer);

void       InfoWin_Status_Load_CB   (Widget, XtPointer, XtPointer);
void       InfoWin_Status_Update    (Rcb_t *, RunStats_t *, char *);


#endif
/*  End of INFO_VIEW.H  */
