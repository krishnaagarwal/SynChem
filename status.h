#ifndef _H_STATUS_
#define _H_STATUS_  1
/******************************************************************************
*
*  Copyright (C) 1995-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     STATUS.H
*    
*
*  Creation Date:
*
*    18-Jul-95
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 26-Sep-96  Krebsbach  Placed run statistics data in a separate structure.
*
******************************************************************************/

#ifndef _H_RCB_
#include "rcb.h"
#endif

/*** Routine Prototypes ***/
 
void Status_File_Peek           (char *, Rcb_t *, RunStats_t *);
void Status_File_Read           (char *, Rcb_t *, RunStats_t *);
void Status_File_Write          (char *, Rcb_t *, RunStats_t *);

#endif
/*  End of STATUS.H  */
