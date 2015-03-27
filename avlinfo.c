/******************************************************************************
*
*  Copyright (C) 1993, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     AVLINFO.C
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
*    AviCmpInfo_Destroy
*    AviCmpInfo_Extract
*    AviCmpInfo_Extrude
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita	xxx
* 25-Feb-95  Krebsbach  Added string lengths into info datastructure.
* 10-Oct-93  Tito       Brought up to code spec.
*
******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif


/****************************************************************************
*
*  Function Name:                 AviCmpInfo_Destroy
*
*    Frees up the memory allocated to the slg_info structure.
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
void AviCmpInfo_Destroy
  (
  Avi_CmpInfo_t *cmp_info_p                /* Info record to destroy */
  )
{

  DEBUG (R_AVL, DB_AVCPINFDESTROY, TL_PARAMS, (outbuf,
    "Entering AviCmpInfo_Destroy, avl. comp. info addr %p", cmp_info_p));

  if (cmp_info_p == NULL)
    return;

#ifdef _MIND_MEM_
  mind_free ("Avi_CmpInfo_Sling_Get(cmp_info_p)", "avlinfo", Avi_CmpInfo_Sling_Get (cmp_info_p));
  mind_free ("Avi_CmpInfo_Name_Get(cmp_info_p)", "avlinfo", Avi_CmpInfo_Name_Get (cmp_info_p));
  mind_free ("Avi_CmpInfo_Catalog_Get(cmp_info_p)", "avlinfo", Avi_CmpInfo_Catalog_Get (cmp_info_p));
  mind_free ("cmp_info_p", "avlinfo", cmp_info_p);
#else
  Mem_Dealloc (Avi_CmpInfo_Sling_Get (cmp_info_p), 
    Avi_CmpInfo_SlgLen_Get (cmp_info_p) + 1, GLOBAL);
  Mem_Dealloc (Avi_CmpInfo_Name_Get (cmp_info_p), 
    Avi_CmpInfo_NameLen_Get (cmp_info_p) + 1, GLOBAL);
  Mem_Dealloc (Avi_CmpInfo_Catalog_Get (cmp_info_p), 
    Avi_CmpInfo_CatLen_Get (cmp_info_p) + 1, GLOBAL);
  Mem_Dealloc (cmp_info_p, AVI_CMPINFO_SZ, GLOBAL);
#endif

  DEBUG (R_AVL, DB_AVCPINFDESTROY, TL_PARAMS, (outbuf,
    "Leaving AviCmpInfo_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 AviCmpInfo_Extract
*
*    Parses the record buffer and stores the information in the newly 
*    created cmp_info structure.  
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
*    Address of new Avi_CmpInfo_t
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
Avi_CmpInfo_t *AviCmpInfo_Extract
  (
  U8_t         *buff_p                     /* Information rec buffer */
  )
{
  Avi_CmpInfo_t  *info_p;                  /* Temp comp info ptr */
  Avi_CmpHdr_t   *head_p;                  /* Comp hdr ptr */
  U8_t           *b_p;                     /* Ptr for buffer */
  U8_t           *s_p;                     /* Temp char ptr */

  DEBUG (R_AVL, DB_AVCPINFEXTRACT, TL_PARAMS, (outbuf,
    "Entering AviCmpInfo_Extract, buffer %p", buff_p));

  /*  Allocate structure, copy information from buffer into structure */

#ifdef _MIND_MEM_
  mind_malloc ("info_p", "avlinfo{1}", &info_p, AVI_CMPINFO_SZ);
#else
  Mem_Alloc (Avi_CmpInfo_t *, info_p, AVI_CMPINFO_SZ, GLOBAL);
#endif

  DEBUG (R_AVL, DB_AVCPINFEXTRACT, TL_MEMORY, (outbuf,
    "Allocated memory for an avail. comp. info. structure in AviCmpInfo_Extract\
 at %p", info_p));

  if (info_p == NULL)
    IO_Exit_Error (R_AVL, X_LIBCALL, 
      "No memory for avail. comp. info. structure in AviCmpInfo_Extract");

  head_p = (Avi_CmpHdr_t *)buff_p;
  b_p = buff_p + AVI_CMPHDR_SZ;

#ifdef _MIND_MEM_
  mind_malloc ("s_p", "avlinfo{1}", &s_p, Avi_CmpHdr_SlgLen_Get (head_p) + 1);
#else
  Mem_Alloc (U8_t *, s_p, Avi_CmpHdr_SlgLen_Get (head_p) + 1, GLOBAL);
#endif

  DEBUG (R_AVL, DB_AVCPINFEXTRACT, TL_MEMORY, (outbuf,
    "Allocated memory for a sling string in AviCmpInfo_Extract at %p", info_p));

  if (s_p == NULL)
    IO_Exit_Error (R_AVL, X_LIBCALL, 
      "No memory for sling string in AviCmpInfo_Extract");

  Avi_CmpInfo_Sling_Put (info_p, (char *)s_p);
  memcpy (s_p, b_p, Avi_CmpHdr_SlgLen_Get (head_p));
  s_p[Avi_CmpHdr_SlgLen_Get (head_p)] = '\0';
  b_p += Avi_CmpHdr_SlgLen_Get (head_p);
  Avi_CmpInfo_SlgLen_Put (info_p, Avi_CmpHdr_SlgLen_Get (head_p));

#ifdef _MIND_MEM_
  mind_malloc ("s_p", "avlinfo{1a}", &s_p, Avi_CmpHdr_NameLen_Get (head_p) + 1);
#else
  Mem_Alloc (U8_t *, s_p, Avi_CmpHdr_NameLen_Get (head_p) + 1, GLOBAL);
#endif

  DEBUG (R_AVL, DB_AVCPINFEXTRACT, TL_MEMORY, (outbuf,
    "Allocated memory for a sling string in AviCmpInfo_Extract at %p", info_p));

  if (s_p == NULL)
    IO_Exit_Error (R_AVL, X_LIBCALL, 
      "No memory for compound name in AviCmpInfo_Extract");

  Avi_CmpInfo_Name_Put (info_p, (char *)s_p);
  memcpy (s_p, b_p, Avi_CmpHdr_NameLen_Get (head_p));
  s_p[Avi_CmpHdr_NameLen_Get (head_p)] = '\0';
  b_p += Avi_CmpHdr_NameLen_Get (head_p);
  Avi_CmpInfo_NameLen_Put (info_p, Avi_CmpHdr_NameLen_Get (head_p));

#ifdef _MIND_MEM_
  mind_malloc ("s_p", "avlinfo{1b}", &s_p, Avi_CmpHdr_CatLen_Get (head_p) + 1);
#else
  Mem_Alloc (U8_t *, s_p, Avi_CmpHdr_CatLen_Get (head_p) + 1, GLOBAL);
#endif

  DEBUG (R_AVL, DB_AVCPINFEXTRACT, TL_MEMORY, (outbuf,
    "Allocated memory for a sling string in AviCmpInfo_Extract at %p", info_p));

  if (s_p == NULL)
    IO_Exit_Error (R_AVL, X_LIBCALL, 
      "No memory for catalogue name in AviCmpInfo_Extract");

  Avi_CmpInfo_Catalog_Put (info_p, (char *)s_p);
  memcpy (s_p, b_p, Avi_CmpHdr_CatLen_Get (head_p));
  s_p[Avi_CmpHdr_CatLen_Get (head_p)] = '\0';
  Avi_CmpInfo_CatLen_Put (info_p, Avi_CmpHdr_CatLen_Get (head_p));

  Avi_CmpInfo_Avail_Put (info_p, Avi_CmpHdr_Avail_Get (head_p));
  Avi_CmpInfo_Lib_Put (info_p, Avi_CmpHdr_Lib_Get (head_p));

  DEBUG (R_AVL, DB_AVCPINFEXTRACT, TL_PARAMS, (outbuf,
    "Leaving AviCmpInfo_Extract, address = %p", info_p));

  return info_p;
}

/****************************************************************************
*
*  Function Name:                 AviCmpInfo_Extrude
*
*    Sets up the buffer of the information record and extrudes onto it   
*    the data in the cmp_info structure.  
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
*    Length of compound information record.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t AviCmpInfo_Extrude
  (
  Avi_CmpInfo_t *info_p,                   /* Address of *Info_t to format */
  U8_t         *buff_p                     /* Address of output buffer */
  )
{
  Avi_CmpHdr_t *head_p;                   /* Record header */
  U8_t         *b_p;                      /* Ptr for buffer */
  U32_t         rec_len;                  /* length of comp info rec */

  DEBUG (R_AVL, DB_AVCPINFEXTRUDE, TL_PARAMS, (outbuf,
    "Entering AviCmpInfo_Extrude, avail. comp. info. addr %p, buffer %p",
    info_p, buff_p));

  /*  IMPORTANT NOTE:  the sling must be the first string after the 
      information header (SSlings_Get in avldict reads in the sling
      strings ignoring the other information).  Append any new strings
      at the end of the string buffer after the catalog string.

      Set up pointers,  copy info into the compound record header,
      copy sling, name and catalog strings to buffers. 
  */

  head_p = (Avi_CmpHdr_t *)buff_p;
  b_p = buff_p + AVI_CMPHDR_SZ;
  rec_len = AVI_CMPHDR_SZ;

  Avi_CmpHdr_Avail_Put (head_p, Avi_CmpInfo_Avail_Get (info_p));
  Avi_CmpHdr_Lib_Put (head_p, Avi_CmpInfo_Lib_Get (info_p));

  if (Avi_CmpInfo_Sling_Get (info_p) == NULL)
    {
    Avi_CmpHdr_SlgLen_Put (head_p, 0);
    }
  else
    {
    memcpy (b_p, Avi_CmpInfo_Sling_Get (info_p), 
      Avi_CmpInfo_SlgLen_Get (info_p));
    b_p += Avi_CmpInfo_SlgLen_Get (info_p);
    rec_len += Avi_CmpInfo_SlgLen_Get (info_p);
    Avi_CmpHdr_SlgLen_Put (head_p, Avi_CmpInfo_SlgLen_Get (info_p));
    }

  if (Avi_CmpInfo_Name_Get (info_p) == NULL)
    {
    Avi_CmpHdr_NameLen_Put (head_p, 0);
    }
  else
    {
    memcpy (b_p, Avi_CmpInfo_Name_Get (info_p), 
      Avi_CmpInfo_NameLen_Get (info_p));
    b_p += Avi_CmpInfo_NameLen_Get (info_p);
    rec_len += Avi_CmpInfo_NameLen_Get (info_p);
    Avi_CmpHdr_NameLen_Put (head_p, Avi_CmpInfo_NameLen_Get (info_p));
    }

  if (Avi_CmpInfo_Catalog_Get (info_p) == NULL)
    {
    Avi_CmpHdr_CatLen_Put (head_p, 0);
    }
  else
    {
    memcpy (b_p, Avi_CmpInfo_Catalog_Get (info_p), 
      Avi_CmpInfo_CatLen_Get (info_p));
    b_p += Avi_CmpInfo_CatLen_Get (info_p);
    rec_len += Avi_CmpInfo_CatLen_Get (info_p);
    Avi_CmpHdr_CatLen_Put (head_p, Avi_CmpInfo_CatLen_Get (info_p));
    }

  DEBUG (R_AVL, DB_AVCPINFEXTRUDE, TL_PARAMS, (outbuf,
    "Leaving AviCmpInfo_Extrude, formatted length of buffer %lu", rec_len));

  return rec_len;
}
/* End of AviCmpInfo_Extrude */

/* End of AVLINFO.C */
