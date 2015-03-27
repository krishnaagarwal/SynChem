#ifndef _H_SLING2XTR_
#define _H_SLING2XTR_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SLING2XTR.H
*
*    This module contains the conversion routine prototypes.  This is
*    to make many of the test executables smaller since many need Sling
*    input but not all need XTRs.
*
*    Routines are found in SLING2XTR.C
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/*** Macros ***/

/*** Routine Prototypes ***/

Xtr_t   *Sling_CanonicalName2Xtr (Sling_t);
Xtr_t   *Sling2Xtr               (Sling_t);
Xtr_t   *Sling2Xtr_PlusHydrogen  (Sling_t);
Sling_t  Xtr2Sling               (Xtr_t *);

/*** Global Variables ***/

/* End of Sling2Xtr.H */
#endif
