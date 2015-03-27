#ifndef _H_TSD2XTR_
#define _H_TSD2XTR_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     TSD2XTR.H
*
*    This module contains the conversion routine prototypes.
*
*    Routines can be found in TSD2XTR.C
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

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

/*** Literal Values ***/

/*** Data Structures ***/

/*** Macros ***/

/*** Routine Prototypes ***/

Xtr_t *Tsd2Xtr (Tsd_t *);
Tsd_t *Xtr2Tsd (Xtr_t *);

/*** Global Variables ***/

/* End of Tsd2Xtr.H */
#endif
