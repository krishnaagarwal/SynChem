/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     FUNCGROUPS.C
*
*    This module is the abstraction for the Functional Group structure.
*    This structure is a sub-structure of the XTR.  The Functional Groups
*    are known sets of atoms with particular properties.  It is important
*    to know where they are in a molecule in order to help know about
*    potential reaction sites.  The broad descriptions of each of the
*    Func. Groups is kept in the Func. Group datafile.  There is also a
*    record in this file which contains a Trie which encodes all of the
*    Func. Groups so that they may be efficiently searched for.
*
*  Routines:
*
*    FGEncode_Open
*    FGEncode_Read_Close
*    FGEncode_Write_Close
*    FuncGroups_Copy
*    FuncGroups_Create
*    FuncGroups_Destroy
*    FuncGroups_Dump
*    FuncGroups_Init
*    FuncGroups_Reset
*    Xtr_FuncGrp_Instance_IsDiff
*    Xtr_FuncGrps_Equiv
*    SExplore
*    SFGMatch
*    SLocal
*    SSearch
*    SValid
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
* 25-Jan-97  Krebsbach  Changed Xtr_FuncGrp_NumInstances_Get back to macro
*                       and make sure xtr has funcgroups before used.
* 11-May-95  Cheung     Changed Xtr_FuncGrp_NumInstances_Get from marco to
*                       routine and called FuncGroups_Create when needed.
* 07-Jul-95  Cheung     In SValid, the first bond in a preservable
*                       substructure is not marked as a preservable bond
* 23-Jul-95  Cheung     Added routines FGEncode_Open, FGEncode_Read_Close
*                       and FGEncode_Write_Close. Encoded table is now read 
*                       from other file.
* 13-Sep-00  Miller     Added FuncGroups_Reset to handle undoing of initialization
*                       for cleaner syn_view environment transitions.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_ATOMSYM_
#include "atomsym.h"
#endif

#ifndef _H_FUNCGROUPS_
#include "funcgroups.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

/* Static Routine Prototypes */

static void              SExplore (Xtr_t *, FuncGroups_t *, FuncGrp_Encode_t *,
  Array_t *, Array_t *, Array_t *, Array_t *, U16_t, U16_t, U16_t);
static void              SFGMatch (Xtr_t *, FuncGroups_t *);
static void              SLocal   (Xtr_t *, FuncGroups_t *, FuncGrp_Encode_t *,
  Array_t *, Array_t *, Array_t *, Array_t *, U16_t, U16_t, U16_t,
  U16_t[MX_NEIGHBORS][2], U8_t, U8_t);
static FuncGrp_Encode_t *SSearch  (FuncGrp_Encode_t *, FuncGrp_Encode_t *);
static Boolean_t         SValid   (Xtr_t *, FuncGroups_t *, FuncGrp_Encode_t *,
  Array_t *);

int  initsub ();
void resetsub ();
void subsrch (Xtr_t *, FuncGroups_t *);

/* Static Data Structures */

static FuncGrp_Record_t **SFuncGroups = NULL;
static FuncGrp_Encode_t  *SFuncGrpTable = NULL;
static List_t            *SFuncGrpInvalid = NULL;
static Array_t            SFuncGrpPreserve;
static Array_t            SFuncGrpNum2Key;
static Isam_Control_t     SFuncGrpFile;
static int                num_ref = 0;
static int                num_groups = 0;


/*****************************************************************************
*
*  Function Name:                 FGEncode_Open
*
*    This routine opens a file to store or read in the encoded table.
*
*  Used to be:
*
*    N/A
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
*    A pointer to the file descriptor if file can be opened.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
FILE  *FGEncode_Open
  (
  char  *filename_p,
  U8_t  flag
  )
{
  FILE          *file_p;

  if (flag == READ_ONLY)
#ifdef _WIN32
    file_p = fopen (gccfix (filename_p), "rb");
  else
    if (flag == WRITE_ONLY)
      file_p = fopen (gccfix (filename_p), "wb");  
#else
    file_p = fopen (filename_p, "rb");
  else
    if (flag == WRITE_ONLY)
      file_p = fopen (filename_p, "wb");  
#endif
    else
      {
      printf ("Invalid flag used in FGEncode_Open!!\n");
      exit (-1);
      }

  return file_p;
}

/*****************************************************************************
*
*  Function Name:                 FGEncode_Read_Close
*
*    This routine read in an encoded table and then close the file.
*
*  Used to be:
*
*    N/A
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
FuncGrp_Encode_t *FGEncode_Read_Close
  (
  FILE          *file_p
  )
{
  FuncGrp_Encode_t      *table_p; 
  U32_t                 size;

  if (fread (&size, 1, sizeof (U32_t), file_p) != sizeof (U32_t))
    {
    printf ("Cannot read file!!\n");
    exit (-1);
    }

#ifdef _MIND_MEM_
  mind_malloc ("table_p", "funcgroups{1}", &table_p, size * FUNCGRPENCODESIZE);
#else
  Mem_Alloc (FuncGrp_Encode_t *, table_p, size * FUNCGRPENCODESIZE, GLOBAL);
#endif

  if (fread (table_p, 1, FUNCGRPENCODESIZE * size, file_p) != 
       FUNCGRPENCODESIZE * size)
    {
    printf ("Cannot read file!!\n");
    exit (-1);
    }
   
  fclose (file_p);

  return table_p;
}

/*****************************************************************************
*
*  Function Name:                 FGEncode_Write_Close
*
*    This routine writes the encoded table to a file and then close the file.
*
*  Used to be:
*
*    N/A
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
void FGEncode_Write_Close
  (
  FILE                  *file_p,
  FuncGrp_Encode_t      *data_p,
  U32_t                 size
  )
{
  if (fwrite (&size, 1, sizeof (U32_t), file_p) != sizeof (U32_t))
    {
    printf ("Cannot write output file!!\n");
    exit (-1);
    }
 
  if (fwrite (data_p, 1, size * FUNCGRPENCODESIZE, file_p) != 
      (size * FUNCGRPENCODESIZE)) 
    {
    printf ("Cannot write output file!!\n");
    exit (-1);
    }

  fclose (file_p);
}


/****************************************************************************
*
*  Function Name:                 FuncGroups_Copy
*
*    This function copies an FuncGroups_t.
*
*  Used to be:
*
*    nfgroup:
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
*    Address of newly created copy
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
FuncGroups_t *FuncGroups_Copy
  (
  FuncGroups_t *fungroup_p                 /* Func. Group descriptor to copy */
  )
{
  FuncGroups_t *fungroup_tmp;              /* Temporary */

  if (fungroup_p == NULL)
    return NULL;

  DEBUG (R_XTR, DB_FNGRPCOPY, TL_PARAMS, (outbuf,
    "Entering FuncGroups_Copy, Func Group = %p", fungroup_p));

#ifdef _MIND_MEM_
  mind_malloc ("funcgroup_tmp", "funcgroups{2}", &fungroup_tmp, FUNCGROUPSSIZE);
#else
  Mem_Alloc (FuncGroups_t *, fungroup_tmp, FUNCGROUPSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Func. Group desc. in FuncGroups_Copy at %p",
    fungroup_tmp));

  if (fungroup_tmp == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for Functional Group desc. in FuncGroups_Copy");

  FuncGrp_NumNonHydrogen_Put (fungroup_tmp, FuncGrp_NumNonHydrogen_Get (
    fungroup_p));

#ifdef _MIND_MEM_
  mind_Array_Copy ("FuncGrp_Substructures_Get(fungroup_tmp)", "funcgroups{2}", FuncGrp_Substructures_Get (fungroup_p),
    FuncGrp_Substructures_Get (fungroup_tmp));
  mind_Array_Copy ("FuncGrp_Preservable_Get(fungroup_tmp)", "funcgroups{2}", FuncGrp_Preservable_Get (fungroup_p),
    FuncGrp_Preservable_Get (fungroup_tmp));
#else
  Array_Copy (FuncGrp_Substructures_Get (fungroup_p),
    FuncGrp_Substructures_Get (fungroup_tmp));
  Array_Copy (FuncGrp_Preservable_Get (fungroup_p),
    FuncGrp_Preservable_Get (fungroup_tmp));
#endif

  DEBUG (R_XTR, DB_FNGRPCOPY, TL_PARAMS, (outbuf,
    "Leaving FuncGroups_Copy, Func Group = %p", fungroup_tmp));

  return fungroup_tmp;
}

/****************************************************************************
*
*  Function Name:                 FuncGroups_Create
*
*    This function allocates the memory for a Func. Group Descriptor.  It
*    also calls the search routines to set all the information about which
*    Functional Groups exist in this molecule.  For orthogonalities sake
*    the allocated FuncGroup_t is NOT entered into the XTR.
*
*  Used to be:
*
*    build_fgroup:
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
*    Address of newly created Functional Groups descriptor
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
FuncGroups_t *FuncGroups_Create
  (
  Xtr_t        *xtr_p                      /* M'cule to look at */
  )
{
  FuncGroups_t *fungrp_p;                  /* Temporary */
  U16_t         i;                         /* Counter */
  U16_t         nonh_cnt;                  /* Counter */

  DEBUG (R_XTR, DB_FNGRPCREATE, TL_PARAMS, (outbuf,
    "Entering FuncGroups_Create, xtr addr = %p", xtr_p));

#ifdef _MIND_MEM_
  mind_malloc ("fungrp_p", "funcgroups{3}", &fungrp_p, FUNCGROUPSSIZE);
#else
  Mem_Alloc (FuncGroups_t *, fungrp_p, FUNCGROUPSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPCREATE, TL_MEMORY, (outbuf,
    "Allocated memory for a Func. Group desc. in FuncGroups_Create at %p",
    fungrp_p));

  if (fungrp_p == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "No memory for Func Group in FuncGroups_Create");

  /* Count how many non-Hydrogen atoms there are in this molecule */

  for (i = 0, nonh_cnt = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    if (Xtr_Attr_Atomid_Get (xtr_p, i) != HYDROGEN)
      nonh_cnt++;

  FuncGrp_NumNonHydrogen_Put (fungrp_p, nonh_cnt);

  /* Initialize the array of substructure instances.  Set row 0 to 0
     since that is the row with the counts in it.
  */

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("FuncGrp_Substructures_Get(fungrp_p)", "funcgroups{3}", FuncGrp_Substructures_Get (fungrp_p),
     FuncGrp_NumNonHydrogen_Get (fungrp_p) + 1, MX_FUNCGROUPS,
     WORDSIZE);
#else
  Array_2d_Create (FuncGrp_Substructures_Get (fungrp_p),
     FuncGrp_NumNonHydrogen_Get (fungrp_p) + 1, MX_FUNCGROUPS,
     WORDSIZE);
#endif

  Array_Set (FuncGrp_Substructures_Get (fungrp_p), XTR_INVALID);
  for (i = 0; i < MX_FUNCGROUPS; i++)
    {
    Array_2d16_Put (FuncGrp_Substructures_Get (fungrp_p), 0, i, 0);
    }

#ifdef _MIND_MEM_
  mind_Array_2d_Create ("FuncGrp_Preservable_Get(fungrp_p)", "funcgroups{3}", FuncGrp_Preservable_Get (fungrp_p),
     Xtr_NumAtoms_Get (xtr_p), MX_NEIGHBORS, BITSIZE);
#else
  Array_2d_Create (FuncGrp_Preservable_Get (fungrp_p),
     Xtr_NumAtoms_Get (xtr_p), MX_NEIGHBORS, BITSIZE);
#endif
  Array_Set (FuncGrp_Preservable_Get (fungrp_p), FALSE);

  /* This call will discover all the Functional Groups in the molecule */

/*
  SFGMatch (xtr_p, fungrp_p);
*/
  subsrch (xtr_p, fungrp_p);

  DEBUG (R_XTR, DB_FNGRPCREATE, TL_PARAMS, (outbuf,
    "Leaving FuncGroups_Create, Func Group = %p", fungrp_p));

  return fungrp_p;
}

/****************************************************************************
*
*  Function Name:                 FuncGroups_Destroy
*
*    This routine deallocates the memory associated with a FuncGroups_t.
*
*  Used to be:
*
*    zfgroup:
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
void FuncGroups_Destroy
  (
  FuncGroups_t *fungrp_p                   /* Func. Group desc. to destroy */
  )
{
  if (fungrp_p == NULL)
    return;

  DEBUG (R_XTR, DB_FNGRPDESTROY, TL_PARAMS, (outbuf,
    "Entering FuncGroups_Destroy, Func Group = %p", fungrp_p));

#ifdef _MIND_MEM_
  mind_Array_Destroy ("FuncGrp_Substructures_Get(fungrp_p)", "funcgroups", FuncGrp_Substructures_Get (fungrp_p));
  mind_Array_Destroy ("FuncGrp_Preservable_Get(fungrp_p)", "funcgroups", FuncGrp_Preservable_Get (fungrp_p));
#else
  Array_Destroy (FuncGrp_Substructures_Get (fungrp_p));
  Array_Destroy (FuncGrp_Preservable_Get (fungrp_p));
#endif


  DEBUG_DO (DB_FNGRPDESTROY, TL_MEMORY,
    {
    memset (fungrp_p, 0, FUNCGROUPSSIZE);
    });

#ifdef _MIND_MEM_
  mind_free ("fungrp_p", "funcgroups", fungrp_p);
#else
  Mem_Dealloc (fungrp_p, FUNCGROUPSSIZE, GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPDESTROY, TL_MEMORY, (outbuf,
    "Deallocated memory for Func. Group desc. in FuncGroups_Destroy at %p",
    fungrp_p));

  DEBUG (R_XTR, DB_FNGRPDESTROY, TL_PARAMS, (outbuf,
    "Leaving FuncGroups_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 FuncGroups_Dump
*
*    This routine prints a formatted dump of a FuncGroups_t.
*
*  Used to be:
*
*    pfgroup:
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
void FuncGroups_Dump
  (
  FuncGroups_t  *fungrp_p,                   /* Instance to dump */
  FileDsc_t     *filed_p                     /* File to dump to */
  )
{
  FILE          *f;                          /* Temporary */
  U16_t          i, j;                       /* Counters */
  U16_t          icnt;                       /* # instances */

  f = IO_FileHandle_Get (filed_p);
  if (fungrp_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Functional Groups\n\n");
    return;
    }

  DEBUG_ADDR (R_XTR, DB_FNGRPCREATE, fungrp_p);
  fprintf (f, "Dump of Functional Group Descriptor\nSubstructure Table :\n");
  for (i = 0; i < FuncGrp_NumSubstructures_Get (fungrp_p); i++)
    {
    icnt = FuncGrp_SubstructureCount_Get (fungrp_p, i);
    if (icnt != 0)
      {
      fprintf (f, "Number of instances of substructure #%u = %u\n", i, icnt);
      fprintf (f, "Root atoms for syntheme instances: ");
      for (j = 1; j <= icnt; j++)
	fprintf (f, "%3u ", FuncGrp_SubstructureInstance_Get (fungrp_p, i, j));

      fprintf (f, "\n");
      }
     }

  fprintf (f, "Preservable bond array\n");
  Array_Dump (FuncGrp_Preservable_Get (fungrp_p), filed_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 FuncGroups_Init
*
*    This routine reads in the Functional Group information from the
*    Functional Group datafile at startup.  The file layout is described
*    in FUNCGROUP_FILE.C.  A list of invalid groups (Not-Groups) is made
*    as well as an array of flags indicating which groups are preservable.
*
*  Used to be:
*
*    initsub:
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
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
void FuncGroups_Init
  (
  U8_t          *dir_p                       /* Directory with FG data file */
  )
{
  FuncGrp_Head_t *head_p;                    /* Pointer to head of FG record */
  ListElement_t *tail_p;                     /* Pointer to tail of list */
  U32_t          i;                          /* Counter */
  U32_t          num_fgs;                    /* # FGs in file */
  U32_t          max_fgnum;                  /* Biggest FG # */
  Boolean_t      first;                      /* Tells state of invalid list */
  FILE           *file_p;
  String_t      filename;

  DEBUG (R_XTR, DB_FNGRPINIT, TL_PARAMS, (outbuf,
    "Entering FuncGroups_Init, directory name = %s", dir_p));

  strcpy (IO_FileName_Get (Isam_File_Get (&SFuncGrpFile)), (char *)dir_p);
  strcat (IO_FileName_Get (Isam_File_Get (&SFuncGrpFile)), FUNCGRP_FILENAME);
  Isam_Open (&SFuncGrpFile, ISAM_TYPE_FGINFO, ISAM_OPEN_READ);

  num_fgs = FuncGroups_NumGroups_Get ();
num_groups = num_fgs; /* Need to know how for passing to Mem_Dealloc later */
  SFuncGrpInvalid = List_Create (LIST_SCALAR);

#ifdef _MIND_MEM_
  mind_malloc ("SFuncGroups", "funcgroups{4}", &SFuncGroups, num_fgs * sizeof (void *));
#else
  Mem_Alloc (FuncGrp_Record_t **, SFuncGroups, num_fgs * sizeof (void *),
    GLOBAL);
#endif

  DEBUG (R_XTR, DB_FNGRPINIT, TL_MEMORY, (outbuf,
    "Allocated memory for Func. Group records array in FuncGrp_Init at %p",
    SFuncGroups));

  if (SFuncGroups == NULL)
    IO_Exit_Error (R_XTR, X_LIBCALL,
      "Failed to allocate memory for Func. Groups array in FuncGrp_Init");

  /* The decoding Trie is stored in separate file (Cheung) */

/****** Not used - replaced by initsub/subsrch ******
  filename = String_Create ((char *)dir_p, strlen ((char *)dir_p));
  String_Concat_c (&filename, ENCODE_FILENAME);
  file_p = FGEncode_Open ((char *)String_Value_Get (filename), READ_ONLY);
  SFuncGrpTable = FGEncode_Read_Close (file_p);
  String_Destroy (filename);
****** Not used - replaced by initsub/subsrch ******/

  for (i = 1; i < num_fgs; i++)
    SFuncGroups[i] = FuncGrp_Rec_Read (i, &SFuncGrpFile);

  /* After reading in all the FuncGrp_Record_t's from the file, we need
     to go through and find the invalid groups and the preservable ones.
  */

  head_p = FuncGrp_Rec_Head_Get (SFuncGroups[num_fgs - 1]);
  max_fgnum = FuncGrp_Head_FGNum_Get (head_p) + 1;
  if (max_fgnum > MX_FUNCGROUPS)
    IO_Exit_Error (R_XTR, X_SYNERR, "Compile-time max. # FGs is too small");

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&SFuncGrpPreserve", "funcgroups{4}", &SFuncGrpPreserve, MX_FUNCGROUPS, BITSIZE);
  Array_Set (&SFuncGrpPreserve, FALSE);
  mind_Array_1d_Create ("&SFuncGrpNum2Key", "funcgroups{4}", &SFuncGrpNum2Key, MX_FUNCGROUPS, LONGSIZE);
  Array_Set (&SFuncGrpNum2Key, ISAM_INVALID);
#else
  Array_1d_Create (&SFuncGrpPreserve, MX_FUNCGROUPS, BITSIZE);
  Array_Set (&SFuncGrpPreserve, FALSE);
  Array_1d_Create (&SFuncGrpNum2Key, MX_FUNCGROUPS, LONGSIZE);
  Array_Set (&SFuncGrpNum2Key, ISAM_INVALID);
#endif

  for (i = 1, first = FALSE, tail_p = NULL; i < num_fgs; i++)
    {
    head_p = FuncGrp_Rec_Head_Get (SFuncGroups[i]);
    Array_1d32_Put (&SFuncGrpNum2Key, FuncGrp_Head_FGNum_Get (head_p), i);
    if (FuncGrp_Head_FlagsPreserveable_Get (head_p) == TRUE)
      {
      Array_1d1_Put (&SFuncGrpPreserve, FuncGrp_Head_FGNum_Get (head_p), TRUE);
      }

    if (FuncGrp_Head_NotGroup_Get (head_p, 0) != 0)
      {
      if (first == FALSE)
	{
	List_InsertU16 (SFuncGrpInvalid, NULL, 
	  (U16_t)FuncGrp_Head_FGNum_Get (head_p));
	first = TRUE;
	}
      else
	List_InsertU16 (SFuncGrpInvalid, tail_p, 
	  (U16_t)FuncGrp_Head_FGNum_Get (head_p));

      tail_p = List_Tail_Get (SFuncGrpInvalid);
      }
    }

  Isam_Close (&SFuncGrpFile);

  num_ref = initsub ();

  DEBUG (R_XTR, DB_FNGRPINIT, TL_PARAMS, (outbuf,
    "Leaving FuncGroups_Init, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 FuncGroups_Reset
*
*    This routine undoes the above initialization.  This is needed in order
*    to have syn_view automatically handle potential KB conflicts during a
*    job submission or when opening a new status file.
*
*  Used to be:
*
*    N/A
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
void FuncGroups_Reset
  (
  )
{
  int i;

#ifdef _DEBUG_
printf("FG_R 1\n");
#endif
  if (SFuncGroups != NULL)
  {
    for (i = 1; i < num_groups; i++) FuncGrp_Rec_Destroy (SFuncGroups[i]);
#ifdef _DEBUG_
printf("FG_R 2\n");
#endif
#ifdef _MIND_MEM_
    mind_free ("SFuncGroups", "funcgroups", SFuncGroups);
#else
    Mem_Dealloc (SFuncGroups, num_groups * sizeof (void *), GLOBAL);
#endif
#ifdef _DEBUG_
printf("FG_R 3\n");
#endif
  }
  SFuncGroups = NULL;

  if (SFuncGrpInvalid != NULL) List_Destroy (SFuncGrpInvalid);
#ifdef _DEBUG_
printf("FG_R 4\n");
#endif
  SFuncGrpInvalid = NULL;

  resetsub ();
#ifdef _DEBUG_
printf("FG_R 5\n");
#endif
}

/****************************************************************************
*
*  Function Name:                 Xtr_FuncGrp_Instance_IsDiff
*
*    This function checks to see if a given instance of a substructure is
*    constitutionally the same as all previous instances.
*
*  Used to be:
*
*    inst_ok: mainent=true, pres_ent=false
*    ins_okp: mainent=false, pres_ent=true
*    subi_ok: mainent=false, pres_ent=false
*
*  Implicit Inputs:
*
*    Xtr_Name_Set must have been called before calling this routine.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    TRUE  - this instance is constitutionally the same
*    FALSE - it is not
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Xtr_FuncGrp_Instance_IsDiff
  (
  Xtr_t        *xtr_p,                     /* M'cule to look at */
  U16_t         substructure,              /* Which functional group */
  U16_t         instance                   /* Which instance to get */
  )
{
  FuncGroups_t *fungrp_p;                  /* Functional group address */
  U16_t         instance_atom;             /* Temporary */
  U16_t         i;                         /* Counter */

  fungrp_p = Xtr_FuncGroups_Get (xtr_p);

# ifdef XTR_DEBUG
  if (fungrp_p == NULL)
    return FALSE;
# endif

  DEBUG (R_XTR, DB_FNGRPSUBINSTOK, TL_PARAMS, (outbuf,
    "Entering Xtr_FuncGrp_Instance_IsDiff, Xtr = %p, substruct = %u,\
 instance = %u", xtr_p, substructure, instance));

  instance_atom = FuncGrp_SubstructureInstance_Get (fungrp_p, substructure,
    instance);

  /* The number of instances is stored in the zeroeth column, so the instances
     run from 1 (the first, to whatever)
  */

  for (i = 1; i < instance; i++)
    if (Name_ConstitutionalEquivilance_Get (xtr_p, instance_atom,
	  FuncGrp_SubstructureInstance_Get (fungrp_p, substructure, i)) == TRUE)
      {
      DEBUG (R_XTR, DB_FNGRPSUBINSTOK, TL_PARAMS, (outbuf,
	"Leaving Xtr_FuncGrp_Instance_IsDiff, status = FALSE"));

      return FALSE;
      }

  DEBUG (R_XTR, DB_FNGRPSUBINSTOK, TL_PARAMS, (outbuf,
    "Leaving Xtr_FuncGrp_Instance_IsDiff, status = TRUE"));

  return TRUE;
}

/*****************************************************************************
*
*  Function Name:                 Xtr_FuncGrps_Equiv
*
*
* 
*  Implicit Inputs:
*
*    Assumes function groups have been created for both xtr's.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t Xtr_FuncGrps_Equiv
  (
  Xtr_t         *xtr_p,
  Xtr_t         *cxtr_p
  )
{
  U16_t         fgnum;

  for (fgnum = 0; fgnum < MX_FUNCGROUPS; fgnum++)
    if (FuncGrp_SubstructureCount_Get (Xtr_FuncGroups_Get (xtr_p), fgnum)
	!= FuncGrp_SubstructureCount_Get (Xtr_FuncGroups_Get (cxtr_p), fgnum))
    return FALSE;

  return TRUE;
}


/****************************************************************************
*
*  Function Name:                 SExplore
*
*    This routine a pointer to a FuncGrp_Encode_t that represents a
*    traversal of the molecule that is known to be in the encoding Trie.
*    It calls to see if it is a valid (meaning is an FG) decoding and then
*    it explores the neighborhood of the atom at the head of the Breadth-
*    First Search Queue (BFS Q).  It sorts all the outgoing bonds that
*    haven't been crossed so far in decode order and if there are a non-
*    zero number then it calls SLocal to expand the set of neighbors.
*
*  Used to be:
*
*    analyze:
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
static void SExplore
  (
  Xtr_t        *xtr_p,                     /* Molecule to search */
  FuncGroups_t *fungrp_p,                  /* Functional group address */
  FuncGrp_Encode_t *base_enc_p,            /* Address of cur. spot in Trie */
  Array_t      *bfsq_p,                    /* 2-d word, queue of atoms */
  Array_t      *enter_p,                   /* 1-d word, enter order of atoms */
  Array_t      *expand_p,                  /* 1-d bit, expand flag for atoms */
  Array_t      *bonds_p,                   /* 2-d bit, bond expand flags */
  U16_t         head,                      /* Head of queue */
  U16_t         tail,                      /* Tail of queue */
  U16_t         enter_cnt                  /* How many atoms have been seen */
  )
{
  U16_t         neigh[MX_NEIGHBORS][2];    /* Neighbors */
  U16_t         atom_idx;                  /* Atom index we are exploring */
  U16_t         next_atom;                 /* Atom on other end of bond */
  U16_t         i, j;                      /* Counter */
  U16_t         num_seen;                  /* Num. neighbors seen already */
  U16_t         swap;                      /* Temp. for sorting neigh[] */
  U8_t          num_neigh;                 /* # neighbors (compiler bug) */
  Boolean_t     leaf;                      /* Is this a leaf node */

  /* Prune over-zealous expansion which is indicated by attempting to
     look at a node beyond the end of the BFS Q
  */

  if (head > tail)
    return;

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Entering SExplore, Xtr = %p, fun.grp. = %p, cur. enc. = %p, bfs q = %p,"
    " enter = %p, expand = %p, bonds = %p, head = %u, tail = %u,"
    " enter cnt = %u", xtr_p, fungrp_p, base_enc_p, bfsq_p, enter_p, expand_p, 
    bonds_p, head, tail, enter_cnt));

  /*
     First call SValid to see if we have a valid node (info on leaf status).
     Next fill in the breadth-ordered search of this atom (mark it as expanded)
     Pass over bonds (atoms) that have been crossed before.
     Then sort it by the table ordering (ascending order everywhere!)
     Copy the bonds array so that it doesn't get improperly overwritten
     Unmark it as expanded before leaving
  */

  leaf = SValid (xtr_p, fungrp_p, base_enc_p, bonds_p);
  atom_idx = Array_2d16_Get (bfsq_p, head, FG_ATOM_IDX);
  if (Array_1d1_Get (expand_p, atom_idx) == TRUE || leaf == TRUE)
    return;

  Array_1d1_Put (expand_p, atom_idx, TRUE);
  num_neigh = Xtr_Attr_NumNeighbors_Get (xtr_p, atom_idx);
  for (i = 0, j = 0, num_seen = 0; i < num_neigh; i++)
    if (Array_2d1_Get (bonds_p, atom_idx, i) == FALSE)
      {
      next_atom = Xtr_Attr_NeighborId_Get (xtr_p, atom_idx, i);
      neigh[j][FG_ATOM_IDX] = next_atom;
      neigh[j][FG_ATOMBOND] = Xtr_Attr_NeighborBond_Get (xtr_p, atom_idx, i) |
	(Xtr_Attr_Atomid_Get (xtr_p, next_atom) << 8);
      j++;
      }
    else
      num_seen++;

  /* Update the number of neighbors to be only those which have not been
     seen before.
  */

  num_neigh -= num_seen;
  for (i = 0; i < num_neigh; i++)
    {
    for (j = i, next_atom = i; j < num_neigh; j++)
      {
      if (neigh[next_atom][FG_ATOMBOND] > neigh[j][FG_ATOMBOND])
	next_atom = j;
      }

    if (next_atom != i)
      {
      swap = neigh[i][FG_ATOM_IDX];
      neigh[i][FG_ATOM_IDX] = neigh[next_atom][FG_ATOM_IDX];
      neigh[next_atom][FG_ATOM_IDX] = swap;

      swap = neigh[i][FG_ATOMBOND];
      neigh[i][FG_ATOMBOND] = neigh[next_atom][FG_ATOMBOND];
      neigh[next_atom][FG_ATOMBOND] = swap;
      }
    }

  /* Now the expansion order is correctly set */

  SLocal (xtr_p, fungrp_p, base_enc_p, bfsq_p, enter_p, expand_p, bonds_p,
    head, tail, enter_cnt, neigh, 0, num_neigh);

  Array_1d1_Put (expand_p, atom_idx, FALSE);

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Leaving SExplore, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SFGMatch
*
*    This routine is the top-level search routine.  The idea is to look for
*    Func. Groups based at each atom in the molecule.  After they are have
*    all been discovered, then any invalid ones must be purged.
*
*  Used to be:
*
*    subsrch, presrch:
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
static void SFGMatch
  (
  Xtr_t        *xtr_p,                     /* Molecule to search */
  FuncGroups_t *fungrp_p                   /* Functional group address */
  )
{
  FuncGrp_Encode_t *base_p;                /* Current base to look in */
  FuncGrp_Head_t *head_p;                  /* Head block of FG record */
  ListElement_t *list_p;                   /* List traversal pointer */
  U32_t         test_key;                  /* ISAM key for FG# */
  U16_t         num_atoms;                 /* # atoms in molecule */
  U16_t         i, j, k, l;                /* Counters */
  U16_t         count;                     /* Sum of items */
  U16_t         test_fg;                   /* Func. Group # */
  U16_t         invalid_fg;                /* Invalid FG # */
  U16_t         inv_cnt;                   /* # invalid instances of an FG */
  Boolean_t     invalid;                   /* Flag for invalid attr search */
  FuncGrp_Encode_t cur_enc;                /* Encoded atom */
  Array_t       expanded;                  /* 1-d bit, expanded predicate */
  Array_t       bonds;                     /* 2-d bit, crossed bond pred. */
  Array_t       bfs_q;                     /* 2-d word, queue of atoms to xp */
  Array_t       entered;                   /* 1-d word, enumeration of enter */

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Entering SFGMatch, Xtr = %p, fun. grp = %p", xtr_p, fungrp_p));

  /* Figure out the number of bonds to cross, this indicates how large the
     Breadth First Search Queue (BFS Q) has to be.  Create all the data-
     structures that will be used in the search.

     Then loop through each atom and try to find Functional Groups based
     there.
  */

  num_atoms = Xtr_NumAtoms_Get (xtr_p);
  for (i = 0, count = 0; i < num_atoms; i++)
    count += Xtr_Attr_NumNeighbors_Get (xtr_p, i);

  count = (count >> 1) + 1;
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&entered", "funcgroups{5}", &entered, num_atoms, WORDSIZE);
  mind_Array_1d_Create ("&expanded", "funcgroups{5}", &expanded, num_atoms, BITSIZE);
  mind_Array_2d_Create ("&bonds", "funcgroups{5}", &bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  mind_Array_2d_Create ("&bfs_q", "funcgroups{5}", &bfs_q, count, 2, WORDSIZE);
#else
  Array_1d_Create (&entered, num_atoms, WORDSIZE);
  Array_1d_Create (&expanded, num_atoms, BITSIZE);
  Array_2d_Create (&bonds, num_atoms, MX_NEIGHBORS, BITSIZE);
  Array_2d_Create (&bfs_q, count, 2, WORDSIZE);
#endif

  for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
    {
    /* Encode the root atom
       - Initialize the search data-structures
       - Record the current root atom where it can be found (mild hack)
       - Start the search with an exploration of the root's neighborhood
    */

    FuncGrp_Enc_Whole_Put (&cur_enc, 0);
    FuncGrp_Enc_Bond_Put (&cur_enc, 0);
    FuncGrp_Enc_Atomid_Put (&cur_enc, Xtr_Attr_Atomid_Get (xtr_p, i));
    base_p = SSearch (SFuncGrpTable, &cur_enc);
    if (base_p != NULL)
      {
      Array_Set (&entered, 0);
      Array_Set (&expanded, FALSE);
      Array_Set (&bonds, FALSE);
      Array_2d16_Put (&bfs_q, 0, FG_ATOM_IDX, i);
      Array_2d16_Put (&bfs_q, 0, FG_ATOMBOND, Xtr_Attr_Atomid_Get (xtr_p, i));
      FuncGrp_NumNonHydrogen_Put (fungrp_p, i);

      SExplore (xtr_p, fungrp_p, base_p, &bfs_q, &entered,
	&expanded, &bonds, 0, 0, 1);
      }
    }

  /* Reset the number of non-hydrogen atoms to its correct value */

  FuncGrp_NumNonHydrogen_Put (fungrp_p, (U16_t) Array_Rows_Get (
    FuncGrp_Substructures_Get (fungrp_p)) - 1);

  /* Fixup invalid functional groups.
     - Traverse list of groups which should be tested (test_fg)
     - Check each conflicting group number (invalid_fg)
     - For each instance of the conflicting group check all instances
       of the test group WHOSE atoms are lessEqual than that instance
     - If one is found, move all higher instances down one
  */

  list_p = List_Front_Get (SFuncGrpInvalid);
  while (list_p != NULL)
    {
    test_fg = LstElem_ValueU16_Get (list_p);
    if (FuncGroups_Substructure_Exists (fungrp_p, test_fg) == TRUE)
      {
      test_key = Array_1d32_Get (&SFuncGrpNum2Key, test_fg);
      head_p = FuncGrp_Rec_Head_Get (SFuncGroups[test_key]);
      for (i = 0; i < FG_MX_NOTGRP; i++)
	{
	invalid_fg = FuncGrp_Head_NotGroup_Get (head_p, i);
	if (invalid_fg == 0)
	  {
	  i = FG_MX_NOTGRP;
	  continue;
	  }

	count = FuncGrp_SubstructureCount_Get (fungrp_p, invalid_fg);
	for (j = 1, inv_cnt = 0; j <= count - inv_cnt; )
	  {
	  for (k = 1, invalid = FALSE; (k <= FuncGrp_SubstructureCount_Get (
	       fungrp_p, test_fg)) && (invalid == FALSE); k++)
	    {
	    if (FuncGrp_SubstructureInstance_Get (fungrp_p, test_fg, k) ==
		FuncGrp_SubstructureInstance_Get (fungrp_p, invalid_fg, j))
	      invalid = TRUE;
	    else
	      if (FuncGrp_SubstructureInstance_Get (fungrp_p, test_fg, k) >
		FuncGrp_SubstructureInstance_Get (fungrp_p, invalid_fg, j))
		k = FuncGrp_SubstructureCount_Get (fungrp_p, test_fg) + 1;
	    }

	  /* At loop exit, j marks invalid instance.  Move the remaining
	     ones down, and decrement the count
	  */

	  if (invalid == TRUE)
	    {
	    for (l = j; l < count; l++)
	      FuncGrp_SubstructureInstance_Put (fungrp_p, invalid_fg, l,
		FuncGrp_SubstructureInstance_Get (fungrp_p, invalid_fg, l + 1));

	    inv_cnt++;
	    }
	  else
	    ++j;
	  }                                 /* For j loop */

	FuncGrp_SubstructureCount_Put (fungrp_p, invalid_fg, count - inv_cnt);
	}                                   /* For i loop */
      }

    list_p = LstElem_Next_Get (list_p);
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&entered", "funcgroups", &entered);
  mind_Array_Destroy ("&expanded", "funcgroups", &expanded);
  mind_Array_Destroy ("&bonds", "funcgroups", &bonds);
  mind_Array_Destroy ("&bfs_q", "funcgroups", &bfs_q);
#else
  Array_Destroy (&entered);
  Array_Destroy (&expanded);
  Array_Destroy (&bonds);
  Array_Destroy (&bfs_q);
#endif

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Leaving SFGMatch, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SLocal
*
*    This routine is the heart of the search for Functional Groups.  The
*    basic idea is to traverse the molecule from the root atom in a depth-
*    first search and to match this search to paths in the decoding Trie.
*    So the search is constrained in a sense by what it can possibly hope
*    to find.  For greater detail see Rick Boivie's thesis, "Heuristic
*    Search Guidance in Synchem".
*
*    The algorithm starts by finding the next level of the Trie to look
*    at.  Then it enters a loop over all the neighbors, for each one it
*    will look at all compatible entries in this level of the Trie, ie
*    those for which the atom and bond match, but which may have different
*    numbers for eXpanded atoms.  For each such entry, if the eXpanded 
*    number is 0 then there are more neighbors to go, recursively call
*    SLocal to look at them, otherwise call SExplore to make sure that
*    all possible permutations of the neighbors array is looked at.  If
*    the number of eXpanded atoms is non-zero, then call SExplore to look
*    further down the BFS Q beyond the number of eXpanded atoms.
*
*  Used to be:
*
*    super_analyze:
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
static void SLocal
  (
  Xtr_t        *xtr_p,                     /* Molecule to search */
  FuncGroups_t *fungrp_p,                  /* Functional group address */
  FuncGrp_Encode_t *base_enc_p,            /* Current level's count entry */
  Array_t      *bfsq_p,                    /* 2-d word, queue of atoms */
  Array_t      *enter_p,                   /* 1-d word, enter order of atoms */
  Array_t      *expand_p,                  /* 1-d bit, expand flag for atoms */
  Array_t      *bonds_p,                   /* 2-d bit, bond expand flags */
  U16_t         head,                      /* Head of queue */
  U16_t         tail,                      /* Tail of queue */
  U16_t         enter_cnt,                 /* How many atoms have been seen */
  U16_t         neigh[MX_NEIGHBORS][2],    /* Inspection order of neighbors */
  U8_t          first_neigh,               /* Index of first neighbor */
  U8_t          num_neigh                  /* # valid entries in neigh[][] */
  )
{
  FuncGrp_Encode_t *xnode_p;               /* Pointer to new current node */
  U16_t         atom_idx;                  /* Atom we are expanding */
  U16_t         level_size;                /* # entries on this level */
  U16_t         level_idx;                 /* Current index to this level */
  U16_t         next_atom;                 /* Temporary */
  U16_t         i;                         /* Counter */
  U8_t          entered;                   /* Did we enter a new atom here */
  Boolean_t     found;                     /* Match found at current level */
  FuncGrp_Encode_t cur_enc;                /* New node in encoding */
  U16_t         prev_enter_cnt;

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Entering SLocal, Xtr = %p, fun. grp. = %p, cur. enc. = %p, bfs q = %p,"
    " enter = %p, expand = %p, bonds = %p, head = %u, tail = %u,"
    " enter cnt = %u, neigh = %p, 1st neigh = %u, # neigh = %u", xtr_p, 
    fungrp_p, base_enc_p, bfsq_p, enter_p, expand_p, bonds_p, head, tail, 
    enter_cnt, neigh, first_neigh, num_neigh));
 
  /* 
     - Find minimum place in current level.  Remember compressed levels.
     - Enter atoms from the sorted neighbor list into the BFS queue.
     - Mark their bonds as crossed at this point.  We know they are NOT
       yet crossed at this point in the recursion.
     - Call SLocal for recursion on NON-expanded nodes
     - Call SExplore for recursion with 1+ expanded nodes
     - Undo bond marks for next atom
  */

  if (FuncGrp_Enc_Compress_Get (base_enc_p) == TRUE)
    {
    base_enc_p = &SFuncGrpTable[FuncGrp_Enc_Next_Get (base_enc_p)];
    base_enc_p--;
    level_size = 1;
    }
  else
    {
    base_enc_p = &SFuncGrpTable[FuncGrp_Enc_Next_Get (base_enc_p)];
    level_size = FuncGrp_Enc_Next_Get (base_enc_p);
    }

  /* Make sure to loop around and try all possible expand counts for a given
     node.
  */

  atom_idx = Array_2d16_Get (bfsq_p, head, FG_ATOM_IDX);
  for (i = first_neigh, found = FALSE; i < num_neigh; )
    {
    if (found == FALSE)
      {
      prev_enter_cnt = Array_1d16_Get (enter_p, neigh[i][FG_ATOM_IDX]);

      level_idx = 1;
      FuncGrp_Enc_Whole_Put (&cur_enc, 0);
      next_atom = neigh[i][FG_ATOMBOND];
      FuncGrp_Enc_Bond_Put (&cur_enc, next_atom & 0xff);

      if (Array_1d16_Get (enter_p, neigh[i][FG_ATOM_IDX]) == 0)
	{
	FuncGrp_Enc_Atomid_Put (&cur_enc, next_atom >> 8);
	Array_1d16_Put (enter_p, neigh[i][FG_ATOM_IDX], enter_cnt);
	entered = 1;
	}
      else
	{
	FuncGrp_Enc_Atomid_Put (&cur_enc, Array_1d16_Get (enter_p,
	   neigh[i][FG_ATOM_IDX]));
	FuncGrp_Enc_Entered_Put (&cur_enc, TRUE);
	entered = 0;
	}
      }

    /* Find index of next base.  If you fail to find such an index, try
       the next neighbor.
    */

    found = FALSE;
    while (level_idx <= level_size && !found)
      {
      if ((((U32_t *)base_enc_p)[level_idx] & FuncGrp_EncM_Decode) <
	  (((U32_t *)&cur_enc)[0] & FuncGrp_EncM_Decode))
	level_idx++;
      else
	if ((((U32_t *)base_enc_p)[level_idx] & FuncGrp_EncM_Decode) ==
	    (((U32_t *)&cur_enc)[0] & FuncGrp_EncM_Decode))
	  found = TRUE;
      else
	break;
      }

    if (found == FALSE)
      {
      Array_1d16_Put (enter_p, neigh[i][FG_ATOM_IDX], prev_enter_cnt);
      i++;
      continue;
      }

 
    /* We are adding an atom, so mark the bond as crossed, update the 
       current place in the decoding Trie.  If we are still on the same
       atom (Xpand count = 0) then call SLocal as long as it will be
       useful, otherwise call SExplore to handle the case where we need
       to explore the neighborhood in a non-canonical order.  If the
       Xpand count is non-zero, then we are done with this atom and
       possibly others, so skip over them and call SExplore to look
       at the next viable candidate atom as determined by the Trie.
    */

    next_atom = neigh[i][FG_ATOM_IDX];
    if (entered == 1)
      {
    Array_2d16_Put (bfsq_p, tail + 1, FG_ATOM_IDX, next_atom);
    Array_2d16_Put (bfsq_p, tail + 1, FG_ATOMBOND, neigh[i][FG_ATOMBOND]);
      }
    Array_2d1_Put (bonds_p, atom_idx, Xtr_Attr_NeighborIndex_Find (xtr_p,
      atom_idx, next_atom), TRUE);
    Array_2d1_Put (bonds_p, next_atom, Xtr_Attr_NeighborIndex_Find (xtr_p,
      next_atom, atom_idx), TRUE);

    xnode_p = &base_enc_p[level_idx];

    if (FuncGrp_Enc_Xpanded_Get (xnode_p) + head > tail + entered)
      {
      printf ("access beyond the bfsq\n");
      exit (-1);
      }

    if (FuncGrp_Enc_Xpanded_Get (xnode_p) == 0)
      {
      /* Recurse, note that we can only look at neighbors that we have
	 NOT yet looked at, hence the i + 1 for first neighbor.  Otherwise
	 use SExplore to get full permutation of neigh array (don't unmark
	 bond array) but do reset expand array.
      */

      if (i + 1 == num_neigh)
	{
	Array_1d1_Put (expand_p, atom_idx, FALSE);
	SExplore (xtr_p, fungrp_p, xnode_p, bfsq_p, enter_p, expand_p, bonds_p,
	  head, tail + entered, enter_cnt + entered);
	Array_1d1_Put (expand_p, atom_idx, TRUE);
	}
      else
	SLocal (xtr_p, fungrp_p, xnode_p, bfsq_p, enter_p, expand_p, bonds_p,
	  head, tail + entered, enter_cnt + entered, neigh, i + 1, num_neigh);
      }
    else
      {
      SExplore (xtr_p, fungrp_p, xnode_p, bfsq_p, enter_p, expand_p, bonds_p,
	head + FuncGrp_Enc_Xpanded_Get (xnode_p), tail + entered,
	enter_cnt + entered);
      }

    Array_2d1_Put (bonds_p, atom_idx, Xtr_Attr_NeighborIndex_Find (xtr_p,
      atom_idx, next_atom), FALSE);
    Array_2d1_Put (bonds_p, next_atom, Xtr_Attr_NeighborIndex_Find (xtr_p,
      next_atom, atom_idx), FALSE);

    /* Attempt to prevent duplicates of symmetric FGs and hit all possible
       eXpand counts for a given atom.
    */

    level_idx++;
    }

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Leaving SLocal, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 SSearch
*
*    This function is mildly vestigial.  It is called in only one place,
*    SFGMatch, to find the index of the root atom in the first level of
*    the Trie.  Hopefully a good compiler will simply fold it up and place
*    it in SFGMatch.  Since each level of the Trie is sorted, the search
*    is a trivial binary/linear search.
*
*  Used to be:
*
*    binary_search:
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
static FuncGrp_Encode_t *SSearch
  (
  FuncGrp_Encode_t *base_p,                /* Current base to look in */
  FuncGrp_Encode_t *match_p                /* Current node to match */
  )
{
  U16_t         low;                         /* Low index */
  U16_t         high;                        /* High index */
  U16_t         mid;                         /* Middle index */

  /* Use linear search below a certain size.
     - First get bounds on array
     - Point to first (zeroth) entry in array
     - Sort in ascending order!
  */

  low = 1;
  high = FuncGrp_Enc_Next_Get (base_p) + 1;

  while (high - low > FG_MIN_LINEAR)
    {
    mid = ((high - low + 1) >> 1) + low;
    if ((((U32_t *)base_p)[mid] & FuncGrp_EncM_Decode) >
	(*(U32_t *)match_p & FuncGrp_EncM_Decode))
      high = mid;
    else
      low = mid;
    }

  while (low <= high)
    {
    if ((((U32_t *)base_p)[low] & FuncGrp_EncM_Decode) ==
	(*(U32_t *)match_p & FuncGrp_EncM_Decode))
      return &base_p[low];

    if ((((U32_t *)base_p)[low] & FuncGrp_EncM_Decode) > 
	(*(U32_t *)match_p & FuncGrp_EncM_Decode))
      return NULL;

    low++;
    }

  return NULL;
}

/****************************************************************************
*
*  Function Name:                 SValid
*
*    This routine checks if the current place in the decoding Trie marks
*    a valid Functional Group.  If so, then it records it in the
*    substructures array and if it is a preservable Func. Group then it
*    or's the current bond array into the preservable bond array.
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
*    TRUE  - this is a leaf node, ie you can't go below here in the Trie
*    FALSE - NOT a leaf node, do whatever you want
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static Boolean_t SValid
  (
  Xtr_t        *xtr_p,                     /* Molecule to enhance */
  FuncGroups_t *fungrp_p,                  /* Functional group address */
  FuncGrp_Encode_t *base_enc_p,            /* Address of cur. spot in Trie */
  Array_t      *bonds_p                    /* 2-d bit, bond expand flags */
  )
{
  U16_t         count;                     /* # of instances this FG */
  U32_t         fg_num;                    /* This FG # */
  U16_t         i, j;                      /* Counters */
  Boolean_t     status;                    /* Output */

  /* First check if this is a Valid node.
     - Next get the FG#
     - Check if a preservable group (record bonds)
  */

  if (FuncGrp_Enc_Valid_Get (base_enc_p) == FALSE)
    return FALSE;

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Entering SValid, Xtr = %p, fun. grp. = %p, cur. enc. = %p, bonds = %p",
    xtr_p, fungrp_p, base_enc_p, bonds_p));

  if (FuncGrp_Enc_Continue_Get (base_enc_p) == TRUE)
    {
    fg_num = FuncGrp_Enc_Whole_Get (&SFuncGrpTable[FuncGrp_Enc_Next_Get (
      base_enc_p)]) >> 16;
    status = FALSE;
    }
  else
    {
    fg_num = FuncGrp_Enc_Next_Get (base_enc_p);
    status = TRUE;
    }

  /* Mark the FG as starting at the root atom of this search.  The root
     atom index is stored in the numnon-h field (hack).  Check for case
     of trying to put same FG more than once at a given root (yes, it is
     very rare).
  */

  count = FuncGrp_SubstructureCount_Get (fungrp_p, fg_num);
  if (FuncGrp_SubstructureInstance_Get (fungrp_p, fg_num, count) ==
      FuncGrp_NumNonHydrogen_Get (fungrp_p) && count != 0)
    {
    DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
      "Leaving SValid, status = %hu", status));

    return status;
    }

  count++;
  FuncGrp_SubstructureCount_Put (fungrp_p, fg_num, count);
  FuncGrp_SubstructureInstance_Put (fungrp_p, fg_num, count,
    FuncGrp_NumNonHydrogen_Get (fungrp_p));

  /* Save the preservable bonds if necessary */

  if (Array_1d1_Get (&SFuncGrpPreserve, fg_num) == TRUE)
    {
    for (i = 0; i < Xtr_NumAtoms_Get (xtr_p); i++)
      {
      if (i != FuncGrp_NumNonHydrogen_Get (fungrp_p))
	for (j = 0; j < MX_NEIGHBORS; j++)
	  {
	  if (Xtr_Attr_NeighborId_Get (xtr_p, i, j) != 
	       FuncGrp_NumNonHydrogen_Get (fungrp_p))
	    if (Array_2d1_Get (bonds_p, i, j) == TRUE)
	      FuncGrp_PreservableBond_Put (fungrp_p, i, j, 
		Array_2d1_Get (bonds_p, i, j)); 
	  }
      }
    }

  DEBUG (R_XTR, DB_FNGRPSTATIC, TL_PARAMS, (outbuf,
    "Leaving SValid, status = %hu", status));

  return status;
}
/* End of SValid */


/* End of FUNCGROUPS.C */
