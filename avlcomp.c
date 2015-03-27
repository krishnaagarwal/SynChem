/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     AVLCOMP.C
*
*    This module contains the code for the abstraction of the Available
*    Compounds Library.  
*
*  Creation Date:
*
*    01-Aug-1993
*
*  Routines:
*
*    AvcLib_Control_Destroy
*    AvcLib_Exists
*    AvcLib_Info
*    AvcLib_Init
*    AvcLib_Key
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
* 24-May-95  Cheung     Convert all the characters into uppercase before 
*                       searching.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

/* Static Variables */

static Avc_Control_t    SAvcControl;

/* Static Routine Prototypes */

static void  SAvc_Control_Init (char *dirpath);
static S16_t SCSlgs_Compare    (Avc_Slg_Code_t *, Avc_Slg_Code_t *);
static void  SCSlgs_Init       (void);
static void  SEncode_Init      (void);
static void  SInfo_Init        (void);
static void  SKeys_Init        (void);


/****************************************************************************
*
*  Function Name:                 AvcLib_Control_Destroy
*
*    Frees memory allocated for dictionary encoding table, the list
*    of compressed slings, the list of keys, the Isam control block,
*    and the Avc_Control block itself.  Closes the Isam information
*    file.
*
*  Implicit Inputs:
*
*    The Avc_Control structure.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None.
*
*  Side Effects:
*
*    Deallocates memory
*    Closes Info file
*
******************************************************************************/
void AvcLib_Control_Destroy /* FORMERLY UNUSED ROUTINE THAT DIDN'T WORK */
  (
  void
  )
{
  U32_t           cslg_i;                  /* Counter for the comp slgs */
/*
  Avc_Slg_Code_t *cslgs_p; WRONG! NOT a STRING array, but a CHAR array!
*/
  Avc_Slg_Code_t **cslgs_p;                /* the array of cslgs */

  if (Avc_Control_DictTbl_Get (SAvcControl) != NULL)
    free (Avc_Control_DictTbl_Get (SAvcControl));

  if (Avc_Control_Keys_Get (SAvcControl) != NULL)
    free (Avc_Control_Keys_Get (SAvcControl));

  if (Avc_Control_InfoFICB_Get (SAvcControl) != NULL)
    {
    Isam_Close (Avc_Control_InfoFICB_Get (SAvcControl));
    free (Avc_Control_InfoFICB_Get (SAvcControl));
    }

  if (Avc_Control_CSlgs_Get (SAvcControl) != NULL)
    {
/*
    cslgs_p = *(Avc_Control_CSlgs_Get (SAvcControl)); WRONG! We want the STRING array!
*/
    cslgs_p = Avc_Control_CSlgs_Get (SAvcControl);
    for (cslg_i = 0; cslg_i < Avc_Control_SlgCnt_Get (SAvcControl); cslg_i++)
       {
/*
       free (cslgs_p); WRONG! Does not index into array!
       ++cslgs_p; WRONG! Too confusing! Why not use loop index??
*/
       free (cslgs_p[cslg_i]);
       }

    free (Avc_Control_CSlgs_Get (SAvcControl));
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 AvLib_Exists
*
*    Uses a binary search to locate the given sling among the list of
*    available compounds, which are in the form of compressed slings.  
*    The given sling is first compressed.
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
*    Assumes the Avc_Control structure has been initialized;
*    in particular, the list of compressed slings and the dictionary trie 
*    table.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    If the sling is in the list, the corresponding index for the sling into
*    the array of Isam keys.  Otherwise, AVC_INDEX_NOT_FOUND.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t AvcLib_Exists
  (
  char            *sling_p                      /* sling to find */
  )
{
  Avc_Slg_Code_t   this_cslg[AVD_SLING_LENMAX]; /* the compressed sling */
  Avc_Slg_Code_t **cslg_p;                      /* array of comp slgs */
  S16_t            result;                      /* result of comparision */ 
  U32_t            low;                         /* indices into the array */
  U32_t            mid;                         /*   of comp slings */
  U32_t            high;
  char            *temp_p;

  /* convert all the characters into uppercase */

  for (temp_p = sling_p; *temp_p != '\0'; ++temp_p)
    *temp_p = (char) toupper (*temp_p);

  /* Initialize variables and encode the sling. */

  AvcLib_Sling_Encode (sling_p, this_cslg, 
    Avc_Control_DictTbl_Get (SAvcControl));
  cslg_p = Avc_Control_CSlgs_Get (SAvcControl);
  low = 0;
  high = Avc_Control_SlgCnt_Get (SAvcControl) - 1;

  /* See if the sling compressed successfully.  Test boundry conditions. */

  if (*this_cslg == AVC_NULL_CODE)
    return AVC_INDEX_NOT_FOUND;

  result = SCSlgs_Compare (this_cslg, *cslg_p);
  if (result == AVC_COMPARE_LESS)
    return AVC_INDEX_NOT_FOUND;

  result = SCSlgs_Compare (this_cslg, *(cslg_p + high));
  if (result == AVC_COMPARE_GREATER)
    return AVC_INDEX_NOT_FOUND;

  /* Search array. */

  while (high - low > 1)
    {
    mid = (high + low) >> 1;
    result = SCSlgs_Compare (this_cslg, *(cslg_p + mid));
    if (result == AVC_COMPARE_EQUAL)
      return mid;

    else
      {
      if (result == AVC_COMPARE_GREATER)
	low = mid;

      else
	high = mid;
      }   
    }
  
  if (SCSlgs_Compare (this_cslg, *(cslg_p + high)) == AVC_COMPARE_EQUAL)
    return high;
  
  if (SCSlgs_Compare (this_cslg, *(cslg_p + low)) == AVC_COMPARE_EQUAL)
    return low;
  
  return AVC_INDEX_NOT_FOUND;
}

/****************************************************************************
*
*  Function Name:                 AvcLib_Info
*
*    Reads in the Isam information record for the given key and parses  
*    the record storing the information in cmp_info.  Assumes that the
*    Isam file has already been opened.
* 
*  Implicit Inputs:
*
*    The static Avc_Control structure.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Pointer to the compound information record.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Avi_CmpInfo_t *AvcLib_Info
  (
  U32_t           key                         /* key into Isam info file */
  )
{
  U8_t            buff[AVI_INFOREC_LENMAX];   /* buffer to store info rec */

  /*  Read in the record and extract the information into the cmp_info
      structure.
  */

  Isam_Read_Nobuffer (Avc_Control_InfoFICB_Get (SAvcControl), key,
     buff, AVI_INFOREC_LENMAX);

  return (AviCmpInfo_Extract (buff));

}

/****************************************************************************
*
*  Function Name:                 AvcLib_Init
*
*    Initializes either the files necessary to test the existence of a 
*    sling in a library, or initializes the files necessary to retrieve
*    information for a sling, depending on the value of the passed flag.
*    It is assumed that the files for existence testing have already been
*    initialized before an attempt is made to initialize the information 
*    files.  The Avc_Control structure is created when the first 
*    initialization is performed.
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None.
*
*  Side Effects:
*
*       
*    The data structures (dictionary trie table, array of compressed
*    slings, array of Isam keys) are read in from the appropriate 
*    files, and the Isam Control structure is created.
*
******************************************************************************/
void AvcLib_Init
  (
  char           *dirpath,                    /* directory path */
  U8_t            which                       /* exists or keys? */
  )
{

  if (which == AVC_INIT_EXISTS)
    {
    SAvc_Control_Init (dirpath);
    SEncode_Init ();
    SCSlgs_Init ();
    }
  else if (which == AVC_INIT_INFO)
    {
    SKeys_Init ();
    SInfo_Init ();
    }
  else
    IO_Exit_Error (R_AVL, X_SYNERR, 
      "Unrecognized flag for AvcLib_Init");

  DEBUG (R_AVL, DB_REACTINIT, TL_INIT, (outbuf,
    "Available Compounds Library initialized successfully"));

  return;
}

/****************************************************************************
*
*  Function Name:                 AvcLib_Key
*
*    Get the Isam key for the given sling.
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
*    Assumes the Avc_Control structure has been initialized, 
*    in particular, the list of compressed slings, the dictionary trie 
*    table, and the Isam keys array.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Zero, if the sling does not exist in the library file.  Otherwise,
*    it returns the positive key into the Isam info file.
*    
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t  AvcLib_Key
  (
  char           *sling_p                     /* sling to find */
  )
{
  U32_t           slg_i;                      /* index into Isam keys array */

  slg_i = AvcLib_Exists (sling_p);
  if (slg_i == AVC_INDEX_NOT_FOUND)
    return AVC_KEY_NOT_FOUND;

  else
    return *(Avc_Control_Keys_Get (SAvcControl) + slg_i);
}

/****************************************************************************
*
*  Function Name:                 AvcLib_Sling_Encode
*
*    Encodes the given sling.  Assumes there is enough room to store the
*    compressed sling in the space given.
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
*    None.
*
*  Side Effects:
*
*    Places the compressed sling at the memory location referenced by 
*    encoded_p.  An AVC_NULL_CODE value will be stored there if the sling
*    cannot be compressed (it contains characters not seen in the list
*    of available compounds and hence is not in the list).
*
******************************************************************************/
void AvcLib_Sling_Encode
  (
  char           *sling_p,                    /* the sling to be encoded */
  Avc_Slg_Code_t *encoded_p,                  /* the encoded sling */
  Avc_Trie_Table_t *dict_p                    /* the dict trie table */
  )
{
  char           *end_p;                     /* end of curr longest patt */
  char           *head_p;                    /* look ahead ptr for the sling */
  char            abs_ch;                    /* abs val of the target char  */
  Avc_Slg_Code_t code;                       /* code for the curr long patt */
  Avc_Slg_Code_t *enc_slg_p;                 /* ptr for the encoded sling */
  Avc_Trie_Table_t *entry_p;                 /* ptr into the pattern table */

  /* Initialize variables. */

  entry_p = dict_p;
  end_p = sling_p;
  head_p = sling_p;
  enc_slg_p = encoded_p;
  code = AVC_NULL_CODE;

  /* Traverse the table to find the longest pattern that matches the 
     current prefix of the sling.  Continue until the entire sling is
     consumed.
  */

  while (*head_p)
    {
    /* Find the current sling char in this list.  The char of the last entry 
       in the list is negative.  Otherwise, the chars are ordered.  
    */

    while ((Avc_Trie_Char_Get (entry_p) > 0) 
	   && (*head_p > (char)Avc_Trie_Char_Get (entry_p)))
      {
      entry_p += AVC_TBL_ENTRY_LEN;
      } 

    /* If the char was in the list, and the table character has a non-zero
       code associated with it, then update code and end_p with values for
       the new longest pattern.  
    */

    abs_ch = (char) abs (Avc_Trie_Char_Get (entry_p));
    if (*head_p == abs_ch)
      {
      ++head_p;
      if (Avc_Trie_Code_Get (entry_p))
	{
	end_p = head_p;
	code = Avc_Trie_Code_Get (entry_p);
	}

      /* If the next field of the entry is not NULL, advance to the next list. 
	 Otherwise, we have hit a deadend, so save current code and backup to
	 the end of the pattern.   
      */

      if (Avc_Trie_Next_Get(entry_p))
	{
	entry_p += (Avc_Trie_Next_Get(entry_p) * AVC_TBL_ENTRY_LEN);
	}
      else
	{
	*(enc_slg_p++) = code;
	head_p = end_p;
	entry_p = dict_p;
	code = AVC_NULL_CODE;
	}
      }                        /* End of if sling char in list */
    else
      {
      /* Otherwise,  the sling char was not in the list--a deadend.  If
	 there is no code saved (it is still NULL), then we have found a
	 character in the sling which has no code.  So return a NULL code.
	 Otherwise, save the current code and backup to the end of that pattern
      */
   
      if (code == AVC_NULL_CODE)
	{
	*encoded_p = AVC_NULL_CODE;
	return;
	}
      else
	{
	*(enc_slg_p++) = code;
	head_p = end_p;
	entry_p = dict_p;
	code = AVC_NULL_CODE; 
	}     
      }

    /* If we are at the end of the sling, save the current code and backup to
       the end of the matched pattern.  
    */

    if (!*head_p)
      {
      *(enc_slg_p++) = code;
      head_p = end_p;
      entry_p = dict_p;
      code = AVC_NULL_CODE; 
      }
    }                         /* End of while not at end of sling */

  *enc_slg_p = AVC_NULL_CODE;

  return;
}

/****************************************************************************
*
*  Function Name:                 SAvc_Control_Init
*
*    Initializes the appropriate fields.
*
*  Implicit Inputs:
*
*    The static pointer to the Avc_Control structure.
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    None.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SAvc_Control_Init
  (
  char           *dirpath                     /* dir path to avlib datafiles */
  )
{
  Avc_Control_CSlgs_Put (SAvcControl, NULL);
  Avc_Control_DictTbl_Put (SAvcControl, NULL);
  Avc_Control_Dir_Put (SAvcControl, dirpath);
  Avc_Control_InfoFICB_Put (SAvcControl, NULL);
  Avc_Control_Keys_Put (SAvcControl, NULL);
  Avc_Control_SlgCnt_Put (SAvcControl, 0);

  return;
}

/****************************************************************************
*
*  Function Name:                 SCSlgs_Compare
*
*    Compare the compressed slings.
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
*    0  if the compressed slings are equal.
*    1  if the first is greater than the second, or the second is a prefix
*         of the first.
*   -1  if the second is greater than the first, or the first is a prefix
*         of the second.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
S16_t SCSlgs_Compare
  (
  Avc_Slg_Code_t *cslg1_p,                    /* comp slings to be compared */
  Avc_Slg_Code_t *cslg2_p
  )
{

  while (*cslg1_p && *cslg2_p &&  *cslg1_p == *cslg2_p)
    {
    ++cslg1_p;
    ++cslg2_p;
    }

  if (*cslg1_p == *cslg2_p)
    return AVC_COMPARE_EQUAL;
  else
    if (!*cslg1_p && *cslg2_p)
      return AVC_COMPARE_LESS;
  else
    if (*cslg1_p && !*cslg2_p)
      return AVC_COMPARE_GREATER;
  else
    if (*cslg1_p > *cslg2_p)
      return AVC_COMPARE_GREATER;
  else
      return AVC_COMPARE_LESS;

}

/****************************************************************************
*
*  Function Name:                 SCSlgs_Init
*
*    Initialize the array of compressed slings.
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
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
*    Memory for the array is dynamically allocated.  The Avc_Control record
*    is updated with the pointer to the array of comp slings and the count 
*    of slings in this file.
*
******************************************************************************/
void SCSlgs_Init
  (
  void
  )
{
  Avc_RecZero_t   reczero;                    /* version and num of cslgs */
  U32_t           cslg_i;                     /* counter for the comp slgs */
  U16_t          *cslg_len_p;                 /* temp array for cslgs lens */
  U16_t          *cl_p;                       /* temp ptr into cslg lens */
  Avc_Slg_Code_t **cslgs_p;                   /* the array of cslgs */
  char            filename[MX_FILENAME];      /* for constructing filename */ 
  size_t          path_len;                   /* length of pathdir */
  size_t          name_len;                   /* length of filename */
  FILE           *cslgfile_p;                 /*  temp file ptr */

  /*  Open compressed slings/offsets file */

  path_len = strlen (Avc_Control_Dir_Get (SAvcControl));
  strncpy (filename, Avc_Control_Dir_Get (SAvcControl), path_len);
  name_len = MX_FILENAME - (path_len + 1);
  strncpy (&filename[path_len], AVD_DATAFILE_CSLGKEY, name_len);

#ifdef _WIN32
  cslgfile_p = fopen (gccfix (filename), "rb");
#else
  cslgfile_p = fopen (filename, "rb");
#endif
  if (cslgfile_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to open compressed slings/key file.");

  /*  Get zero record of file and test version number for correctness. */

  if (fread (&reczero, AVC_RECZERO_SZ, 1, cslgfile_p) != 1)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to read zero record of comp slings/key file.");

  if (Avc_RecZero_VerNum_Get (reczero) != AVD_VERSION_CSLGKEY)
    IO_Exit_Error (R_AVL, X_SYNERR, 
      "SCSlgs_Init:  Incorrect version number of compressed slings/key file.");

  /* Save the number of compressed slings/offsets in the file record.  
     Allocate the memory for the compressed slings array and save pointer 
     to array.  
  */

  Avc_Control_SlgCnt_Put (SAvcControl, Avc_RecZero_CntSz_Get (reczero));
#ifdef _MIND_MALLOC_
  mind_malloc ("cslgs_p", "avlcomp{1}", &cslgs_p, Avc_RecZero_CntSz_Get (reczero) * (sizeof (Avc_Slg_Code_t *)));
#else
  cslgs_p = (Avc_Slg_Code_t **) malloc (Avc_RecZero_CntSz_Get (reczero)
    * (sizeof (Avc_Slg_Code_t *)));
#endif
  if (cslgs_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to allocate array for compressed slings.");

  Avc_Control_CSlgs_Put (SAvcControl, cslgs_p);

  /* First skip keys, and read in array of compressed sling lengths, 
     and then read in the compressed slings into the cslgs array.
  */

#ifdef _MIND_MEM_
  mind_malloc ("cslg_len_p", "avlcomp{1}", &cslg_len_p, Avc_RecZero_CntSz_Get (reczero) * (sizeof (U16_t)));
#else
  cslg_len_p = (U16_t *) malloc (Avc_RecZero_CntSz_Get (reczero)
    * (sizeof (U16_t)));
#endif
  if (cslg_len_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to allocate array for cslg lengths.");

  if (fseek (cslgfile_p, (S32_t) (Avc_RecZero_CntSz_Get (reczero) 
      * sizeof (U32_t)), 1))
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to skip over keys for compressed slings.");

  if (fread (cslg_len_p, sizeof (U16_t), Avc_RecZero_CntSz_Get (reczero),
       cslgfile_p) != Avc_RecZero_CntSz_Get (reczero))
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to read in lengths of compressed slings.");

  cl_p = cslg_len_p;
  for (cslg_i = 0; cslg_i < Avc_RecZero_CntSz_Get (reczero); cslg_i++)
    {
#ifdef _MIND_MEM_
    mind_malloc ("*cslgs_p", "avlcomp{1}", cslgs_p, *cl_p + AVC_TBL_CODE_SZ);
#else
    *cslgs_p = (Avc_Slg_Code_t *) malloc (*cl_p + AVC_TBL_CODE_SZ);
#endif
    if (fread (*cslgs_p, sizeof (Avc_Slg_Code_t), *cl_p, cslgfile_p)
	!= *cl_p)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to read in compressed sling.");

    *(*cslgs_p + *cl_p) = AVC_NULL_CODE;
    ++cslgs_p;
    ++cl_p;

    }  /* End for cslg_i */   

  free (cslg_len_p);
  fclose (cslgfile_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 SEncode_Init
*
*    Initialize the pattern/code table for compressing the slings.
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
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
*    Memory for the pattern/code table is dynamically allocated.
*    Updates the Avc_Control structure with the pointer to the 
*    pattern/code table.
*
******************************************************************************/
void SEncode_Init
  (
  void
  )
{
  Avc_RecZero_t   reczero;                    /* version and sizeof dict tbl */
  Avc_Trie_Table_t *tbl_p;                    /* temp trie table ptr */
  char            filename[MX_FILENAME];      /* for constructing filename */ 
  size_t          path_len;                   /* length of pathdir */
  size_t          name_len;                   /* length of filename */
  FILE           *tblfile_p;                  /* temp file ptr */

  /* Open the encoding trie table file */
  
  path_len = strlen (Avc_Control_Dir_Get (SAvcControl));
  strncpy (filename, Avc_Control_Dir_Get (SAvcControl), path_len);
  name_len = MX_FILENAME - (path_len + 1);
  strncpy (&filename[path_len], AVD_DATAFILE_DICT_TBL, name_len);
printf("kka:%d,%s,%d",path_len,filename,name_len);

#ifdef _WIN32
  tblfile_p = fopen (gccfix (filename), "rb");
#else
  tblfile_p = fopen (filename, "rb");
#endif
  if (tblfile_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SEncode_Init:  Unable to open dictionary encoding file.");

  /* Get zero record of file and test version number for correctness. */

  if (fread (&reczero, AVC_RECZERO_SZ, 1, tblfile_p) != 1)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SEncode_Init:  Unable to read zero record of dict encoding file.");

  if (Avc_RecZero_VerNum_Get (reczero) != AVD_VERSION_DICT_TBL)
    IO_Exit_Error (R_AVL, X_SYNERR, 
      "SEncode_Init:  Incorrect version number of dict encoding file.");


  /* Allocate the memory for the table and save pointer to table. 
     Read in the table.
  */

#ifdef _MIND_MEM_
  mind_malloc ("tbl_p", "avlcomp{2}", &tbl_p, Avc_RecZero_CntSz_Get (reczero));
#else
  tbl_p = (Avc_Trie_Table_t *) malloc (Avc_RecZero_CntSz_Get (reczero));
#endif
  if (tbl_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SEncode_Init:  Unable to allocate memory for dict encoding table.");

  Avc_Control_DictTbl_Put (SAvcControl, tbl_p);
  if (fread (tbl_p, 1, Avc_RecZero_CntSz_Get (reczero), tblfile_p)
      != Avc_RecZero_CntSz_Get (reczero))
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SEncode_Init:  Unable to read in the dict encoding table.");

  fclose (tblfile_p);

  return;
}

/****************************************************************************
*
*  Function Name:                 SInfo_Init
*
*    Initialize the file containing the information for each sling.  
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
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
*    Creates the Isam_Control_t block for the Isam file containing the 
*    sling information.  The pointer is saved in the Avc_Control structure.
*
******************************************************************************/
void SInfo_Init
  (
  void
  )
{
  Avc_RecZero_t   zrec;                       /* record zero of isam file */
  char            *filename_p;                /* for constructing filename */ 
  size_t          path_len;                   /* length of pathdir */
  size_t          name_len;                   /* length of filename */
  Isam_Control_t  *icf_p;                     /* temp Isam ctrl blk ptr */

  /* Initialize Isam control block, set up filename, and open file.
     Save Isam control block ptr in Avc_Control structure.
  */
 
#ifdef _MIND_MEM_
  mind_malloc ("icf_p", "avlcomp{3}", &icf_p, ISAMCONTROLSIZE);
#else
  icf_p = (Isam_Control_t *) malloc (ISAMCONTROLSIZE);
#endif
  if (icf_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SInfo_Init:  Unable to allocate memory for Isam Control Block.");

  filename_p = IO_FileName_Get (Isam_File_Get (icf_p));
  path_len = strlen (Avc_Control_Dir_Get (SAvcControl));
  strncpy (filename_p, Avc_Control_Dir_Get (SAvcControl), path_len);
  name_len = MX_FILENAME - (path_len + 1);
  strncpy (filename_p + path_len, AVI_DATAFILE_ISAMINFO, name_len);
  Isam_Open (icf_p, ISAM_TYPE_AVLCOMP, ISAM_OPEN_READ);
  Avc_Control_InfoFICB_Put (SAvcControl, icf_p);

  /* Get record zero and verify the version number and number of slings */

  Isam_Read_Nobuffer (icf_p, 0, &zrec, AVC_RECZERO_SZ);
  if (Avc_RecZero_VerNum_Get (zrec) != AVI_VERSION_ISAMINFO)
    IO_Exit_Error (R_AVL, X_SYNERR, 
      "SInfo_Init:  Incorrect version number of Isam information file.");

/* This is no longer valid -- there can be reserved (empty) records due to updates
  if (Avc_RecZero_CntSz_Get (zrec) != Avc_Control_SlgCnt_Get (SAvcControl))
    IO_Exit_Error (R_AVL, X_SYNERR, 
      "SInfo_Init:  The number of slg recs and comp slgs do not match.");
*/

  return;
}

/****************************************************************************
*
*  Function Name:                 SKeys_Init
*
*    Initialize the array of keys into the Isam information file.
*
*  Implicit Inputs:
*
*    The static Avc_Control structure.
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
*    Memory for the array of keys is dynamically allocated.
*    Updates the Avc_Control structure with the pointer to the 
*    array of keys.
*
******************************************************************************/
void SKeys_Init
  (
  void
  )
{
  Avc_RecZero_t   reczero;                    /* version and num of cslgs */
  U32_t           cslg_count;                 /* number of cslgs in the file */
  U32_t          *keys_p;                     /* ptr to the array of keys */
  char            filename[MX_FILENAME];      /* for constructing filename */ 
  size_t          path_len;                   /* length of pathdir */
  size_t          name_len;                   /* length of filename */
  FILE           *cslgfile_p;                 /* temp file ptr */

  /* Open compressed slings/keys file.  The version number has already
     been verified.
  */

  path_len = strlen (Avc_Control_Dir_Get (SAvcControl));
  strncpy (filename, Avc_Control_Dir_Get (SAvcControl), path_len);
  name_len = MX_FILENAME - (path_len + 1);
  strncpy (&filename[path_len], AVD_DATAFILE_CSLGKEY, name_len);

#ifdef _WIN32  
  cslgfile_p = fopen (gccfix (filename), "rb");
#else
  cslgfile_p = fopen (filename, "rb");
#endif
  if (cslgfile_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SKeys_Init:  Unable to open compressed slings/key file.");

  /* Allocate the memory for the keys array, save the pointer in the
     Avc_Control structure, and read in the array.  
  */

  if (fread (&reczero, AVC_RECZERO_SZ, 1, cslgfile_p) != 1)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SCSlgs_Init:  Unable to zero rec number of comp slings/key file.");

  cslg_count = Avc_RecZero_CntSz_Get (reczero);
#ifdef _MIND_MEM_
  mind_malloc ("keys_p", "avlcomp{4}", &keys_p, cslg_count * (sizeof (U32_t)));
#else
  keys_p = (U32_t *) malloc (cslg_count * (sizeof (U32_t)));
#endif
  if (keys_p == NULL)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SKeys_Init:  Unable to allocate memory for the array of keys.");

  Avc_Control_Keys_Put (SAvcControl, keys_p);
  if (fread (keys_p, sizeof (U32_t), cslg_count, cslgfile_p)
	!= cslg_count)
    IO_Exit_Error (R_AVL, X_SYSCALL, 
      "SKeys_Init:  Unable to read in the array of keys.");

  fclose (cslgfile_p);
  return;
}
/* End of SKeys_Init */
/* End of AVLCOMP.C */
