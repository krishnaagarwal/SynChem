/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     FUNCGROUP_FILE.C
*
*    This module is the implementation of the Functional Groups information
*    file interface.  The layout for the on-disk structure is as follows:
*    - FuncGrp_Head_t
*    - name length[s], 16-bits, values
*    - Sling length[s], 16-bits, values
*
*  Routines:
*
*    FuncGrp_Rec_Destroy
*    FuncGrp_Rec_Dump
*    FuncGrp_Rec_Read
*    FuncGrp_Rec_Write
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PL/I code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif


/****************************************************************************
*
*  Function Name:                 FuncGrp_Rec_Destroy
*
*    This function destroys a FuncGrp_Record_t.  These records are
*    created when a record is read from the data file.  There is no
*    corresponding create routine.
*
*  Used to be:
*
*    N/A:
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
*    Deallocates memory
*
******************************************************************************/
void FuncGrp_Rec_Destroy
  (
  FuncGrp_Record_t *fgrec_p                /* Func. group record to dump */
  )
{
  FuncGrp_Head_t *head_p;                  /* Head of record */
  U32_t         max;                       /* # items to do it to */
  U8_t          i;                         /* Counter */
  String_t      string;                    /* For names to delete */
  Sling_t       sling;                     /* For slings to delete */

  DEBUG (R_XTR, DB_FNGRPRECDESTROY, TL_PARAMS, (outbuf,
    "Entering FuncGrp_Rec_Destroy, record address %p", fgrec_p));

  head_p = FuncGrp_Rec_Head_Get (fgrec_p);
  max = FuncGrp_Head_NumNames_Get (head_p);
  for (i = 0; i < max; i++)
    {
    string = FuncGrp_Rec_Name_Get (fgrec_p, i);
    String_Destroy (string);
    }

  DEBUG_DO (DB_FNGRPRECDESTROY, TL_MEMORY,
    {
    memset (FuncGrp_RecName_Get (fgrec_p), 0, max * STRINGSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("FuncGrp_RecName_Get(fgrec_p)", "funcgroup_file", FuncGrp_RecName_Get (fgrec_p));
#else
  Mem_Dealloc (FuncGrp_RecName_Get (fgrec_p), max * STRINGSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPRECDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Func. Group record names array at %p",
    FuncGrp_RecName_Get (fgrec_p)));

  max = FuncGrp_Head_NumSlings_Get (head_p);
  for (i = 0; i < max; i++)
    {
    sling = FuncGrp_Rec_Sling_Get (fgrec_p, i);
    Sling_Destroy (sling);
    }

  DEBUG_DO (DB_FNGRPRECDESTROY, TL_MEMORY,
    {
    memset (FuncGrp_RecSling_Get (fgrec_p), 0, max * SLINGSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("FuncGrp_RecSling_Get(fgrec_p)", "funcgroup_file", FuncGrp_RecSling_Get (fgrec_p));
#else
  Mem_Dealloc (FuncGrp_RecSling_Get (fgrec_p), max * SLINGSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPRECDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Func. Group record slings array at %p",
    FuncGrp_RecSling_Get (fgrec_p)));

  DEBUG_DO (DB_FNGRPRECDESTROY, TL_MEMORY,
    {
    memset (FuncGrp_RecSling_Get (fgrec_p), 0, FUNCGRPRECORDSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("fgrec_p", "funcgroup_file", fgrec_p);
#else
  Mem_Dealloc (fgrec_p, FUNCGRPRECORDSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPRECDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Func. Group record %p", fgrec_p));

  DEBUG (R_XTR, DB_FNGRPRECDESTROY, TL_PARAMS, (outbuf,
    "Leaving FuncGrp_Rec_Destroy, status NULL"));

  return;
}

/****************************************************************************
*
*  Function Name:                 FuncGrp_Rec_Dump
*
*    This function prints a formatted dump of the specified data-structure to
*    the specified file.
*
*  Used to be:
*
*    N/A:
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
void FuncGrp_Rec_Dump
  (
  FuncGrp_Record_t *fgrec_p,               /* Func. group record to dump */
  FileDsc_t    *filed_p                    /* File to dump to */
  )
{
  FILE         *f;                         /* Temporary file handle */
  FuncGrp_Head_t *head_p;                  /* Temporary */
  U8_t          i;                         /* Counter */

  f = IO_FileHandle_Get (filed_p);
  if (fgrec_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Functional Group Record\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_FNGRPRECREAD, fgrec_p);
  fprintf (f, "Dump of Functional Group Record\n");
  head_p = FuncGrp_Rec_Head_Get (fgrec_p);

  fprintf (f, "F.G. # %lu\tFlags %08x\t# Names %hu\t# Slings %hu\n",
    FuncGrp_Head_FGNum_Get (head_p), FuncGrp_Head_Flags_Get (head_p),
    FuncGrp_Head_NumNames_Get (head_p), FuncGrp_Head_NumSlings_Get (head_p));

  /* Check for not-groups, print them */

  if (FuncGrp_Head_NotGroup_Get (head_p, 0) != 0)
    {
    fprintf (f, "Not groups are: %u", FuncGrp_Head_NotGroup_Get (head_p, 0));
    i = 1;
    while ((i < FG_MX_NOTGRP) && FuncGrp_Head_NotGroup_Get (head_p, i) != 0)
      {
      fprintf (f, " %u", FuncGrp_Head_NotGroup_Get (head_p, i));
      i++;
      }

    fprintf (f, "\n");
    }

  fprintf (f, "\nNames for this functional group:\n");
  for (i = 0; i < FuncGrp_Head_NumNames_Get (head_p); i++)
    fprintf (f, "\t%s\n", String_Value_Get (FuncGrp_Rec_Name_Get (fgrec_p, i)));

  fprintf (f, "\nSlings for this functional group:\n");
  for (i = 0; i < FuncGrp_Head_NumSlings_Get (head_p); i++)
    fprintf (f, "\t%s\n", Sling_Name_Get (FuncGrp_Rec_Sling_Get (fgrec_p, i)));

  fprintf (f, "\n");

  return;
}


/****************************************************************************
*
*  Function Name:                 FuncGrp_Rec_Read
*
*    This function creates a FuncGrp_Record_t structure and then reads
*    the record with the specified key into this structure.  The format
*    is described in the module header.
*
*  Used to be:
*
*    N/A:
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
FuncGrp_Record_t *FuncGrp_Rec_Read
  (
  U32_t           key,                     /* Key (FG#) to read */
  Isam_Control_t *file_p                   /* ISAM file to read from */
  )
{
  FuncGrp_Record_t *fgrec_p;               /* Record to "create/read" */
  U8_t           *buf_p;                   /* Start of record pointer */
  FuncGrp_Head_t *head_p;                  /* Temporary */
  U16_t           length;                  /* # bytes in this buffer */
  U8_t            i;                       /* Counter */

  DEBUG (R_XTR, DB_FNGRPRECREAD, TL_PARAMS, (outbuf,
    "Entering FuncGrp_Rec_Read, ISAM cb %p", file_p));

#ifdef _MIND_MEM_
  mind_malloc ("fgrec_p", "funcgroup_file{1}", &fgrec_p, FUNCGRPRECORDSIZE);
#else
  Mem_Alloc (FuncGrp_Record_t *, fgrec_p, FUNCGRPRECORDSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPRECREAD, TL_MEMORY, (outbuf,
    "Allocated memory for Functional Group Record in FuncGrp_Rec_Read at %p",
    fgrec_p));

  if (fgrec_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL, "No memory for Func. Group record");

  head_p = FuncGrp_Rec_Head_Get (fgrec_p);
  buf_p = (U8_t *) Isam_Read (file_p, key);

  if (buf_p == NULL)
    {
#ifdef _MIND_MEM_
    mind_free ("fgrec_p", "funcgroup_file", fgrec_p);
#else
    Mem_Dealloc (fgrec_p, FUNCGRPRECORDSIZE, GLOBAL);
#endif

    DEBUG (R_XTR, DB_FNGRPRECREAD, TL_MEMORY, (outbuf,
      "Deallocated memory for Functional Group Record in FuncGrp_Rec_Read"
      " at %p", fgrec_p));

    return NULL;
    }

  /* Use memcpy to allow for the buffer to appear at any address */

  memcpy (head_p, buf_p, FUNCGRPHEADSIZE);
  buf_p += FUNCGRPHEADSIZE;

#ifdef _MIND_MEM_
  mind_malloc ("FuncGrp_RecName_Get(fgrec_p)", "funcgroup_file{1}", &FuncGrp_RecName_Get (fgrec_p), STRINGSIZE *
    FuncGrp_Head_NumNames_Get (head_p));
#else
  Mem_Alloc (String_t *, FuncGrp_RecName_Get (fgrec_p), STRINGSIZE *
    FuncGrp_Head_NumNames_Get (head_p), GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPRECREAD, TL_MEMORY, (outbuf,
    "Allocated memory for names array in FuncGrp_Rec_Read at %p",
    FuncGrp_RecName_Get (fgrec_p)));

  if (FuncGrp_RecName_Get (fgrec_p) == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL, "No memory for Func. Group names array");

#ifdef _MIND_MEM_
  mind_malloc ("FuncGrp_RecSling_Get(fgrec_p)", "funcgroup_file{1}", &FuncGrp_RecSling_Get (fgrec_p), SLINGSIZE *
    FuncGrp_Head_NumSlings_Get (head_p));
#else
  Mem_Alloc (Sling_t *, FuncGrp_RecSling_Get (fgrec_p), SLINGSIZE *
    FuncGrp_Head_NumSlings_Get (head_p), GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPRECREAD, TL_MEMORY, (outbuf,
    "Allocated memory for Slings array in FuncGrp_Rec_Read at %p",
    FuncGrp_RecSling_Get (fgrec_p)));

  if (FuncGrp_RecSling_Get (fgrec_p) == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL, "No memory for Func. Group Slings array");

  /* Read in the lengths (16-bit integers) of the names, and then the
     values.  Repeat for Slings.
  */

  for (i = 0; i < FuncGrp_Head_NumNames_Get (head_p); i++, buf_p += length)
    {
    memcpy (&length, buf_p, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    FuncGrp_Rec_Name_Put (fgrec_p, i, String_Create_nn ((char *)buf_p, 
      length));
    }

  /* Have read in the name lengths, now do the same for the Sling lengths. 
     Yes, the code is virtually identical.
  */

  for (i = 0; i < FuncGrp_Head_NumSlings_Get (head_p); i++, buf_p += length)
    {
    memcpy (&length, buf_p, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    FuncGrp_Rec_Sling_Put (fgrec_p, i, Sling_Create (length));
    memcpy (Sling_Name_Get (FuncGrp_Rec_Sling_Get (fgrec_p, i)), buf_p,
      length);
    }

  DEBUG (R_XTR, DB_FNGRPRECREAD, TL_PARAMS, (outbuf,
    "Leaving FuncGrp_Rec_Read, status = <void>"));

  return fgrec_p;
}

/****************************************************************************
*
*  Function Name:                 FuncGrp_Rec_Write
*
*    This function writes to the specified file the specified functional group
*    information record.  The data-structure is layed out in a buffer
*    using the format described in the module header.  The buffer is then
*    written to the ISAM file.
*
*  Used to be:
*
*    N/A:
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
*    Writes to the Func. Group ISAM file
*
******************************************************************************/
void FuncGrp_Rec_Write
  (
  FuncGrp_Record_t *fgrec_p,               /* Func. group record to write */
  Isam_Control_t *file_p                   /* ISAM file to write to */
  )
{
  U8_t         *buf_p;                     /* Current write pointer */
  FuncGrp_Head_t *head_p;                  /* Temporary */
  U16_t         length;                    /* # U8_t's to write */
  U8_t          i;                         /* Counter */
  U8_t          obuf[FG_MX_BUFSIZE];       /* Output buffer */

  DEBUG (R_XTR, DB_FNGRPRECWRITE, TL_PARAMS, (outbuf,
    "Entering FuncGrp_Rec_Write, record address = %p, ISAM cb = %p", fgrec_p,
    file_p));

  /* The on-disk layout is (roughly): header, names length+value,
     slings length+value
  */

  buf_p = obuf;
  head_p = FuncGrp_Rec_Head_Get (fgrec_p);
  *(FuncGrp_Head_t *)buf_p = *head_p;
  buf_p += FUNCGRPHEADSIZE;

  for (i = 0; i < FuncGrp_Head_NumNames_Get (head_p); i++,
       buf_p += length)
    {
    length = String_Length_Get (FuncGrp_Rec_Name_Get (fgrec_p, i));
    memcpy (buf_p, &length, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    memcpy (buf_p, String_Value_Get (FuncGrp_Rec_Name_Get (fgrec_p, i)),
      length);
    }

  for (i = 0; i < FuncGrp_Head_NumSlings_Get (head_p); i++,
       buf_p += length)
    {
    length = Sling_Length_Get (FuncGrp_Rec_Sling_Get (fgrec_p, i));
    memcpy (buf_p, &length, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    memcpy (buf_p, Sling_Name_Get (FuncGrp_Rec_Sling_Get (fgrec_p, i)),
      length);
    }

  length = buf_p - obuf;

  ASSERT (length <= FG_MX_BUFSIZE,
    {
    IO_Exit_Error (R_XTR, X_SYNERR, "Funcgrp_Rec_Write buffer overflowed");
    });

  if (Isam_Write (file_p, FuncGrp_Rec_Key_Get (fgrec_p), obuf, length) !=
      SYN_NORMAL)
    IO_Put_Trace (R_XTR,
      "Unexpected status returned from Isam_Write in Funcgrp_Rec_Write");

  DEBUG (R_XTR, DB_FNGRPRECWRITE, TL_PARAMS, (outbuf,
    "Leaving FuncGrp_Rec_Write, status = <void>"));

  return;
}
/* End of FuncGrp_Rec_Write */
/* End of FUNCGROUPS_FILE.C */

