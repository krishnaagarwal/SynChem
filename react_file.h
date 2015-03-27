#ifndef _H_REACT_FILE_
#define _H_REACT_FILE_ 1
/******************************************************************************
*
*  Copyright (C) 1993-1996 by Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     REACT_FILE.H
*
*    This module is the abstraction for the on-disk structure of the
*    Reaction Library.  The Reaction Library is an ISAM file.  Each
*    record has a fixed header followed by some specifically layed
*    out information.  The data file follows:
*    - Data file header
*    - Can't have any data
*    - Must have all data
*    - Must have any data
*    - Goal TSD
*    - Sub-goal TSD
*    - Post-xform conditions
*    - Post-xform tests
*
*    The text file follows:
*    - Text file header
*    - Schema name length and name value
*    - Post test reason(s) length, value
*    - Post test chemist(s) length, value
*    - Comment(s) length, value
*    - Reference(s) length, value
*
*    Routines are found in REACT_FILE.C unless otherwise noted.
*
*  Creation Date:
*
*    01-Jan-1993
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

#ifndef _H_SYNIO_
#include "synio.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

/*** Literal Values ***/

#define REACT_MX_BUFSIZE  4096

/*** Data Structures ***/

/*** Macros ***/

/*** Routine Prototypes ***/

void             React_Destroy   (React_Record_t *);
void             React_Dump      (React_Record_t *, FileDsc_t *);
React_Record_t  *React_Read      (U32_t, Isam_Control_t *);
React_TextRec_t *React_TextRead  (U32_t, Isam_Control_t *, U8_t);
void             React_TextWrite (React_Record_t *, Isam_Control_t *);
void             React_Write     (React_Record_t *, Isam_Control_t *);

/*** Global Variables ***/

/* End of React_File.H */
#endif
