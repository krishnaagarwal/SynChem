/******************************************************************************
*
*  Copyright (C) 1997 by Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     SEARCH_COVER.C
*
*       This module implements the search cover exit condition for
*       the sequential version of synchem.  The symbol table of the 
*       to-cover run of the problem is read from the status table, the
*       developed  symbols are stored, and when the given percentage of
*       the to-cover symbols have also been developed by the covering 
*       sequential run, SearchSpace_Cover returns TRUE.
*       
*
*  Routines:
*
*    SearchSpace_Cover
*    
*    SSearchSpace_Load
*    SSling_Find
*
*
*  Creation Date:
*
*    13-Nov-96
*
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modifcation Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xxx
*
******************************************************************************/

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"


#ifndef _H_SLING_
#include "sling.h"
#endif

#ifndef _H_SEARCH_COVER_
#include "search_cover.h"
#endif

/* Static Routine Prototypes */

static void  SCoverTable_Merge  (CoverTbl_t *, S32_t, S32_t, S32_t);
static void  SCoverTable_Sort   (CoverTbl_t *, S32_t, S32_t);
static S32_t SSling_Find        (Sling_t, CoverTbl_t *);

/* Static Data Declarations */

static CoverTbl_t  SCoverTable;


/****************************************************************************
*
*  Function Name:                 SearchCoverTable_Load
*
*    This function reads in the developed symbols from the given file.
*
*  Implicit Inputs:
*
*    CoverTable
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    pointer to a task
*    NULL if no task is available
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SearchCoverTable_Load
  (
  char          *filename,
  U16_t          percent
  )
{
  FILE          *f_p;
  CoverEntry_t  *covers;
  char          *sling_p;
  S32_t          entry_i;
  S32_t          num_sol;
  S32_t          num_dev;
  S32_t          upbound;
  char           buff[MX_INPUT_SIZE+1];

  CoverTbl_NumCoverDev_Put (&SCoverTable, 0);
  CoverTbl_NumCoverSlv_Put (&SCoverTable, 0);
  CoverTbl_NumDev_Put (&SCoverTable, 0);
  CoverTbl_NumSlv_Put (&SCoverTable, 0);
  CoverTbl_Table_Put (&SCoverTable, NULL);

#ifdef _WIN32  
  f_p = fopen (gccfix (filename), "r");
#else
  f_p = fopen (filename, "r");
#endif
  if (f_p == NULL)
    {
    fprintf (stderr, "SearchCoverTable_Load:  unable to open file %s\n",
      filename);
    exit (-1);
    }

  num_sol = 0;

  if (fgets (buff, MX_INPUT_SIZE, f_p) == NULL)
    {
    fprintf (stderr, "SearchCoverTable_Load:  unable to read file %s\n",
      filename);
    exit (-1);
    }


  num_dev = (S32_t) atol (buff);
  
#ifdef _MIND_MEM_
  mind_malloc ("covers", "search_cover{1}", &covers, COVERENTRY_SIZE * num_dev);
#else
  covers = (CoverEntry_t *) malloc (COVERENTRY_SIZE * num_dev);
#endif
  if (covers == NULL)
    {
    fprintf (stderr, 
      "SCoverTable_Load:  unable to allocate memory for cover table.\n");
    exit (-1);
    }

  for (entry_i = 0; entry_i < num_dev; entry_i++)
    {
    CoverEntry_IsCovered_Put (covers[entry_i], FALSE);
    if (fgets (buff, MX_INPUT_SIZE, f_p) == NULL)
      {
      fprintf (stderr, "SearchCoverTable_Load:  unable to read file %s\n",
	filename);
      exit (-1);
      }

    if (*buff == 'S')
      {
      CoverEntry_WasSolved_Put (covers[entry_i], TRUE);
      num_sol++;
      }
    else
      CoverEntry_WasSolved_Put (covers[entry_i], FALSE);

    sling_p = strtok (buff + 1, "\n");
    CoverEntry_Sling_Put (covers[entry_i], Sling_Create (strlen (sling_p)));
    memcpy ((char *) Sling_Name_Get (CoverEntry_Sling_Get (covers[entry_i])), 
      sling_p, Sling_Length_Get (CoverEntry_Sling_Get (covers[entry_i])) + 1);
    }

  CoverTbl_Table_Put (&SCoverTable, covers);
  CoverTbl_NumDev_Put (&SCoverTable, num_dev);
  CoverTbl_NumSlv_Put (&SCoverTable, num_sol);

  if (percent > 0)
    {
    upbound = (S32_t) ((CoverTbl_NumDev_Get (&SCoverTable) * percent) / 100);
    CoverTbl_UpperBound_Put (&SCoverTable, upbound);
    }
  else
    {
    CoverTbl_UpperBound_Put (&SCoverTable, 0);
    }

  SCoverTable_Sort (&SCoverTable, 0, num_dev);

  fclose (f_p);

  return ;
}
/* End of SearchCoverTable_Load */

/****************************************************************************
*
*  Function Name:                 SearchSpace_Cover
*
*    This function determines when the developed slings from the covering
*    run covers the developed slings from the to-cover run, or a 
*    percentage thereof, or the slings from selected solved pathways.
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
*    Coverage status:
*      COVER_NOT_COVERED:  Neither solved paths nor developed space covered.
*      COVER_DEV_COVERED:  Developed space covered only.
*      COVER_SOL_COVERED:  Solved paths covered only.
*      COVER_BOTH_COVERED:  Both solved paths and developed space covered.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U8_t SearchSpace_Cover
  (
  Sling_t       sling
  )
{
  S32_t         index;
  U8_t          reason;

  reason = COVER_NOT_COVERED;

  index = SSling_Find (sling, &SCoverTable);

  if (index != COVER_NOT_FOUND)
    {
    if (CoverTbl_IthIsCovered_Get (&SCoverTable, index) == FALSE)
      {
      CoverTbl_NumCoverDev_Get (&SCoverTable)++;

      if (CoverTbl_IthWasSolved_Get (&SCoverTable, index) == TRUE)
	CoverTbl_NumCoverSlv_Get (&SCoverTable)++;

      CoverTbl_IthIsCovered_Put (&SCoverTable, index, TRUE);
      }
    }

  if (CoverTbl_NumCoverSlv_Get (&SCoverTable) 
      >= CoverTbl_NumSlv_Get (&SCoverTable))
    reason |= COVER_SOL_COVERED;

  if (CoverTbl_NumCoverDev_Get (&SCoverTable) 
      >= CoverTbl_UpperBound_Get (&SCoverTable))
    reason |= COVER_DEV_COVERED;

  return (reason);
}
/* End of SearchSpace_Cover */

/****************************************************************************
*
*  Function Name:                  SCoverTable_Merge
*
*    Merge sort algorithm:  merges two presorted sublists of the cover table.
*    Takes advantage of the fact that the two sublists are adjacent in the 
*    table.
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
*    N/A
*
******************************************************************************/
static void SCoverTable_Merge
  (
  CoverTbl_t     *cover_table,
  S32_t           low,                        /* low range of low sublist */
  S32_t           mid,                        /* mid range of sublists */
  S32_t           high                        /* high range of high sublist */
  )
{
  CoverEntry_t   *covers;
  S32_t           l_i;                        /* index for low sublist */
  S32_t           m_i;                        /* index for high sublist */
  S32_t           n_i;                        /* index for temp sublist */
  S32_t           subsize;                    /* size of temp sublist */

  l_i = low;
  m_i = mid;
  n_i = 0;
  subsize = high - low;

#ifdef _MIND_MEM_
  mind_malloc ("covers", "search_cover{2}", &covers, COVERENTRY_SIZE * subsize);
#else
  covers = (CoverEntry_t *) malloc (COVERENTRY_SIZE * subsize);
#endif
  if (covers == NULL)
    {
    fprintf (stderr, 
      "SCoverTable_Merge:  unable to allocate memory for temp covers.\n");
    exit (-1);
    }

  while ((l_i < mid) && (m_i < high))
    {
    if (strcmp ((char *) Sling_Name_Get (CoverTbl_IthSling_Get (cover_table, 
	 l_i)), (char *) Sling_Name_Get (CoverTbl_IthSling_Get (cover_table, 
	 m_i))) < 0)
      {
      covers[n_i] = CoverTbl_IthCover_Get (cover_table, l_i);
      l_i++;
      n_i++;
      }

    else
      {
      covers[n_i] = CoverTbl_IthCover_Get (cover_table, m_i);
      m_i++;
      n_i++;
      }

    }

  if (l_i >= mid)
    for (; n_i < subsize; n_i++, m_i++)
      covers[n_i] = CoverTbl_IthCover_Get (cover_table, m_i);

  else
    for (; n_i < subsize; n_i++, l_i++)
      covers[n_i] = CoverTbl_IthCover_Get (cover_table, l_i);

  for (n_i = 0, l_i = low; n_i < subsize; n_i++, l_i++)
    CoverTbl_IthCover_Put (cover_table, l_i, covers[n_i]);


#ifdef _MIND_MEM_
  mind_free ("covers", "search_cover", covers);
#else
  free (covers);
#endif

  return;
}
/* End of SCands_Save_Merge  */

/****************************************************************************
*
*  Function Name:                  SCoverTable_Sort
*
*    Sort the list of cover table entries according to the sling entry.
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
*    N/A
*
******************************************************************************/
static void SCoverTable_Sort
  (
  CoverTbl_t     *cover_table,
  S32_t           low,                        /* low range of sublist */
  S32_t           high                        /* high range of sublist */
  )
{
  S32_t           mid;                        /* mid range of sublists */

  mid = (low + high) >> 1;

  if (low < mid - 1)
    SCoverTable_Sort (cover_table, low, mid);

  if (mid < high - 1)
    SCoverTable_Sort (cover_table, mid, high);

  SCoverTable_Merge (cover_table, low, mid, high); 

  return;
}
/* End of SCoverTable_Sort  */

/****************************************************************************
*
*  Function Name:                 SSling_Find
*
*    A binary search of the new developed sling in the cover table.
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
*    Non-negative index if found.
*    COVER_NOT_FOUND, otherwise.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
static S32_t SSling_Find
  (
  Sling_t       sling,
  CoverTbl_t   *cover_table
  )
{
  S32_t         low;
  S32_t         mid;
  S32_t         high;
  S16_t         result;

  low = 0;
  high = CoverTbl_NumDev_Get (cover_table) - 1;

  if (strcmp ((char *) Sling_Name_Get (sling), (char *) Sling_Name_Get (
      CoverTbl_IthSling_Get (cover_table, low))) < 0)
  return COVER_NOT_FOUND;

  if (strcmp ((char *) Sling_Name_Get (sling), (char *) Sling_Name_Get (
      CoverTbl_IthSling_Get (cover_table, high))) > 0)
  return COVER_NOT_FOUND;


  /* Search array. */

  while (high - low > 1)
    {
    mid = (high + low) >> 1;
    result = strcmp ((char *) Sling_Name_Get (sling), 
      (char *) Sling_Name_Get (CoverTbl_IthSling_Get (cover_table, mid)));

    if (result == 0)
      return mid;

    if (result > 0)
      low = mid;
    else
      high = mid;
    }
  
  if (strcmp ((char *) Sling_Name_Get (sling), 
      (char *) Sling_Name_Get (CoverTbl_IthSling_Get (cover_table, high)))
       == 0)
    return high;
  
  if (strcmp ((char *) Sling_Name_Get (sling), 
      (char *) Sling_Name_Get (CoverTbl_IthSling_Get (cover_table, low)))
       == 0)
    return low;
  
  return COVER_NOT_FOUND;
}

/* End of SSling_Find */

/* End of SEARCH_COVER.CL */
