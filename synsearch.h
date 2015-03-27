#ifndef _H_SYN_SEARCH_
#define _H_SYN_SEARCH_ 1
/******************************************************************************
*
*  Copyright (C) 1996 Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SYN_SEARCH.H
*
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 23-Sep-97  Krebsbach  Combined sequential and distributed versions
*                       of SYNCHEM into single file, making many changes to
*                       data structures and function prototypes.
* 27-Sep-96  Krebsbach  Encapsulated the search into a separate function.
*
******************************************************************************/

#ifndef _H_RCB_
#include "rcb.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/*** Routine Prototypes ***/

void      Synchem_Search   (Compound_t *, Rcb_t *, RunStats_t *);

#endif
/*  End of SYN_SEARCH.H  */
