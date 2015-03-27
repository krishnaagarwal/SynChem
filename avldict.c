/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook, Daren Krebsbach
*
*  Module Name:                     AVLDICT.C
*
*    The Dictionary construction algorithm.  This module takes a runtime
*    version of a compound information file, extracts all of the slings
*    from the file, and creates a dictionary for the compression of the
*    slings, the encoding trie table.  It also creates a sorted list of
*    the compressed slings along with a parallel list of keys into the 
*    runtime Isam information file.  A future implementation will allow
*    an existing dictionary to be used to compress a new list of slings.
*
*  Creation Date:
*
*    01-Aug-1993
*
*  Routines:
*
*    main
*    
*  Authors:
*
*    Daren Krebsbach
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Lolita     xx
*
******************************************************************************/

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <malloc.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_AVLINFO_
#include "avlinfo.h"
#endif

#ifndef _H_AVLDICT_
#include "avldict.h"
#endif

#ifndef _H_AVLCOMP_
#include "avlcomp.h"
#endif

/* Static Variables */

static Avd_Dict_Control_t   SDictControl;     /* dict control structure */

static char     *avldict_usage = {
   "\nUsage: avldict dirpath \n"
     "           dirpath  dir where runtime avail lib files are located.\n\n"};

/*  Static Function Prototypes  */

static U32_t   SCands_Get           (Avd_Trie_Node_t *, Avd_Trie_Node_t *);
static U32_t   SCands_Extract       (Avd_Trie_Node_t *, U32_t);
static void    SCands_Patt_Merge    (S32_t, S32_t, S32_t);
static void    SCands_Patt_Sort     (S32_t, S32_t);
static void    SCands_Save_Merge    (S32_t, S32_t, S32_t);
static void    SCands_Save_Sort     (S32_t, S32_t);
static S32_t   SCount_Get           (Avd_Trie_Node_t *, U8_t *);
static Avd_Trie_Node_t *SCount_Reduce (Avd_Trie_Node_t *, U8_t *, S32_t);
static S16_t   SCSlgs_Comp          (Avc_Slg_Code_t *, Avc_Slg_Code_t *);
static void    SCSlg_Key_Merge      (S32_t, S32_t, S32_t, Avd_Slg_Key_t *);
static void    SCSlg_Key_Sort       (S32_t, S32_t, Avd_Slg_Key_t *);
static void    SPatt_Chain_Create   (U8_t *, S32_t, Avd_Trie_Node_t *);
static Avd_Queue_Node_t *SPatt_DeQueue (Avd_Queue_Node_t **,
  Avd_Queue_Node_t **);
static void    SPatt_EnQueue        (Avd_Trie_Node_t *, Avc_Trie_Table_t *,
  Avd_Queue_Node_t **, Avd_Queue_Node_t **);
static Avd_Queue_Node_t *SPatt_Queue_Node_Create (Avd_Trie_Node_t *, 
  Avc_Trie_Table_t *);
static size_t  SPatt_Table_Create   (Avd_Trie_Node_t  *, Avc_Trie_Table_t *,
  Avd_Queue_Node_t **, Avd_Queue_Node_t **);
static void    SPatt_Tree_Update    (U8_t *, S32_t, Avd_Trie_Node_t *);
static void    SPatts_Codes_Trie_Create (Boolean_t *, U32_t,
   Avd_Trie_Node_t *);
static S32_t   SPatts_Extract       (Avd_Trie_Node_t *, U8_t *);
static void    SSavings_Calc        (Avd_Trie_Node_t *);
static void    SSling_Chain_Create  (U8_t *, Avd_Trie_Node_t *);
static void    SSling_Tree_Create   (U32_t, Avd_Trie_Node_t *, Avd_Slg_Key_t *);
static void    SSling_Tree_Update   (U8_t *, Avd_Trie_Node_t *);
static void    SSlings_Compress     (Avd_Slg_Key_t *, U32_t, Avd_Trie_Node_t *);
static U32_t   SSlings_Get          (Avd_Slg_Key_t *, Boolean_t *);
static void    SSling_Subtrie_Merge (Avd_Trie_Node_t *, Avd_Trie_Node_t *,
  Avd_Trie_Node_t *, Avd_Trie_Node_t *);
static Boolean_t STabletops_Pop       (Avd_Trie_Node_t *);
static Avd_Trie_Node_t *STrie_Node_Create (U8_t, S32_t, Avd_Trie_Node_t *);
static void    STrie_Tree_Destroy   (Avd_Trie_Node_t *);

void main
  (
  int    argc,
  char **argv
  )
{
  char       *fn_p;                       /* temp filename ptr */
  char       *dp_p;                       /* dirpath ptr */
  FILE       *file_p;                     /* temp file ptr */
  size_t      dir_len;                    /* length of dirpath */
  U32_t       i;                          /* index */
  U32_t       num_slings;                 /* number of slings */
  U32_t       num_cands;                  /* number of candidates */
  Avc_Trie_Table_t patt_tbl[AVD_PATT_TBL_MAX_SZ]; /* encoding dict table */
  size_t     patt_tbl_sz;                 /* size of dict trie table */
  Avd_Queue_Node_t *patq_head_p;          /* queue for converting  */
  Avd_Queue_Node_t *patq_tail_p;          /* dict trie to table format */
  Boolean_t  char_exists[AVD_CHARS_MAX_NUM]; /* which chars occur in slgs? */
  Avd_Slg_Key_t sling_list[AVD_SLGS_MAX_NUM]; /* list of slings */
  Avd_Trie_Node_t *slgroot_p;              /* root of sling trie */
  Avd_Trie_Node_t *patroot_p;              /* root of pattern trie */
  Avd_Trie_Node_t *patcdroot_p;            /* root of patt/code trie */

  Debug_Init ();
  Trace_Init ();
  IO_Init ();
  IO_FileHandle_Put (&GStdErr, stdout);
  IO_FileHandle_Put (&GTraceFile, stdout);

  /*  Parse command line and open files. */

  if (argc < 2)
    {
    fprintf (stderr, "%s", avldict_usage);
     exit(0);
   }

  dp_p = *++argv;
  dir_len = strlen (dp_p);
  fn_p = IO_FileName_Get (Isam_File_Get (&Avd_DictCtrl_InfoFC_Get
    (SDictControl)));
  strncpy (fn_p, dp_p, dir_len);
  strncpy (fn_p + dir_len, AVI_DATAFILE_ISAMINFO, 
	   MX_FILENAME - (dir_len + 1));
  Isam_Open (&Avd_DictCtrl_InfoFC_Get (SDictControl), ISAM_TYPE_AVLCOMP, 
    ISAM_OPEN_READ);

  fn_p = IO_FileName_Get (&Avd_DictCtrl_PatTblFC_Get (SDictControl));
  strncpy (fn_p, dp_p, dir_len);
  strncpy (fn_p + dir_len, AVD_DATAFILE_DICT_TBL, 
	   MX_FILENAME - (dir_len + 1));
#ifdef _WIN32  
  file_p = fopen (gccfix (fn_p), "wb");
#else
  file_p = fopen (fn_p, "wb");
#endif
  if (file_p == NULL)
    {
    fprintf (stderr, "\navldict:  error opening dict tbl file:  %s.\n",
	     fn_p);
    exit (-1);
    }

  IO_FileHandle_Put (&Avd_DictCtrl_PatTblFC_Get (SDictControl), file_p);

  fn_p = IO_FileName_Get (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl));
  strncpy (fn_p, dp_p, dir_len);
  strncpy (fn_p + dir_len, AVD_DATAFILE_CSLGKEY, 
	   MX_FILENAME - (dir_len + 1));
#ifdef _WIN32
  file_p = fopen (gccfix (fn_p), "wb");
#else
  file_p = fopen (fn_p, "wb");
#endif
  if (file_p == NULL)
    {
    fprintf (stderr, "\navldict:  error opening cslg/key file:  %s.\n",
	     fn_p);
    exit (-1);
    }

  IO_FileHandle_Put (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl), file_p);

  /*  Intializations. */

  for (i = 0; i < AVD_CHARS_MAX_NUM; i++) 
    char_exists[i] = FALSE;

  for (i = 0; i < AVD_CHARS_MAX_NUM; i++)
    {
    *(Avd_PreTbl_Prefix_Get (Avd_DictCtrl_TblTops_Get (SDictControl)[i])) 
      = AVD_EOS;
    Avd_PreTbl_Save_Get (Avd_DictCtrl_TblTops_Get (SDictControl)[i]) = 0;
    Avd_PreTbl_Change_Get (Avd_DictCtrl_TblTops_Get (SDictControl)[i]) = TRUE;
    }

  slgroot_p = STrie_Node_Create (AVD_EOS, AVD_SLG_NODE_INIT_VAL, NULL);
  patroot_p = STrie_Node_Create (AVD_EOS, AVD_PATT_NODE_INIT_VAL, NULL);
  patcdroot_p = STrie_Node_Create (AVD_EOS, AVD_PATT_NODE_INIT_VAL, NULL);
 
  /* Read in the list of slings and check what characters occur in the
     slings.
  */

  num_slings = SSlings_Get (sling_list, char_exists);
  fprintf (stdout, "avldict:  total number of slings:  %1lu.\n", num_slings);

  /*  Create the sling prefix trie. */

  SSling_Tree_Create (num_slings, slgroot_p, sling_list);

  /* Extract list of candidate dictionary prefixes from the sling
     prefix trie (possible patterns are first placed in the pattern 
     trie, which accumulates estimated savings).
  */

  num_cands = SCands_Get (slgroot_p, patroot_p);

  /*  The sling and pattern tries are no longer needed--free their
      memory.
  */

  STrie_Tree_Destroy (slgroot_p);
  STrie_Tree_Destroy (patroot_p);

  /*  Sort the list of candidate dictionary prefixes according to
      estimated savings.  Build the dictionary trie using the best
      AVD_PATTS_MAX_NUM candidates.
  */

  SCands_Save_Sort (0, (S32_t) num_cands);
  SPatts_Codes_Trie_Create (char_exists, num_cands, patcdroot_p);

  /*  Convert the dictionary trie into a table and save it into the 
      appropriate file.
  */

  patt_tbl_sz = SPatt_Table_Create (patcdroot_p, patt_tbl, &patq_head_p, 
    &patq_tail_p);
  Avc_RecZero_VerNum_Put (Avd_DictCtrl_PatTblZR_Get (SDictControl),
    AVD_VERSION_DICT_TBL);
  Avc_RecZero_CntSz_Put (Avd_DictCtrl_PatTblZR_Get (SDictControl),
    patt_tbl_sz);
  if (fwrite (&Avd_DictCtrl_PatTblZR_Get (SDictControl), AVC_RECZERO_SZ, 
      1, IO_FileHandle_Get (&Avd_DictCtrl_PatTblFC_Get (SDictControl))) != 1)
   {
   fprintf (stderr, "\navldict:  Error writing out zero rec for patt tbl.\n");
   exit (-1);
   }

  if (fwrite (patt_tbl, 1, patt_tbl_sz, IO_FileHandle_Get 
      (&Avd_DictCtrl_PatTblFC_Get (SDictControl))) != patt_tbl_sz)
    {
    fprintf (stderr, "\navldict:  Error writing out pattern table.\n");
    exit (-1);
    }

  fclose (IO_FileHandle_Get (&Avd_DictCtrl_PatTblFC_Get (SDictControl)));

  /*  Compress the slings using the dictionary trie, destroy the 
      dictionary trie,  sort the compressed slings and store them,
      along with their keys into the runtime Isam information file,
      in the appropriate file.  
  */

  SSlings_Compress (sling_list, num_slings, patcdroot_p);
  STrie_Tree_Destroy (patcdroot_p);
  SCSlg_Key_Sort (0, (S32_t) num_slings, sling_list);
  Avc_RecZero_VerNum_Put (Avd_DictCtrl_CslgKeyZR_Get (SDictControl),
    AVD_VERSION_CSLGKEY);
  Avc_RecZero_CntSz_Put (Avd_DictCtrl_CslgKeyZR_Get (SDictControl),
    num_slings);
  if (fwrite (&Avd_DictCtrl_CslgKeyZR_Get (SDictControl), AVC_RECZERO_SZ, 
      1, IO_FileHandle_Get (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl))) != 1)
   {
   fprintf (stderr, "\navldict:  Error writing out zero rec for cslgs.\n");
   exit (-1);
   }

  /*  Store the keys first.  */

  for (i = 0; i < num_slings; i++)
    {
    if (fwrite (&Avd_SlgKey_Key_Get (sling_list[i]), sizeof (U32_t), 1,
	IO_FileHandle_Get (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl))) != 1)
      {
      fprintf (stderr, "\navldict:  Error writing out keys to cslgs.\n");
      exit (-1);
      }
    }

  /*  And then the lengths of the compressed slings.  */

  for (i = 0; i < num_slings; i++)
    {
    if (fwrite (&Avd_SlgKey_CLen_Get (sling_list[i]), sizeof (U16_t), 1,
	IO_FileHandle_Get (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl))) != 1)
      {
      fprintf (stderr, "\navldict:  Error writing out lens of cslgs.\n");
      exit (-1);
      }
    }

  /*  And then, the compressed slings themselves.  Free memory allocated
      to store the sling and compressed sling strings.
  */

  for (i = 0; i < num_slings; i++)
    {
    if (fwrite (Avd_SlgKey_CSlg_Get (sling_list[i]), sizeof (Avc_Slg_Code_t), 
	Avd_SlgKey_CLen_Get (sling_list[i]), 
	IO_FileHandle_Get (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl))) 
	!= Avd_SlgKey_CLen_Get (sling_list[i]))
      {
      fprintf (stderr, "\navldict:  Error writing out cslgs.\n");
      exit (-1);
      }

    free (Avd_SlgKey_Sling_Get (sling_list[i]));
    free (Avd_SlgKey_CSlg_Get (sling_list[i]));
    }

  fclose (IO_FileHandle_Get (&Avd_DictCtrl_CslgKeyFC_Get (SDictControl)));

  exit (0);

}  
/* End of main */

/****************************************************************************
*
*  Function Name:                 SCands_Get
*
*    Extract the best AVD_CANDS_MAX_NUM candidate dictionary prefixes
*    from the sling prefix trie.  The patterns with the highest 
*    estimated savings are removed from the sling trie and added to the 
*    pattern trie.  The candidates are selected from amongst these patterns. 
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
*      - cand_sorted
*      - cursave
*      - depth
*      - extract_patt
*      - extpat_p
*      - tbltops
*      - topprefix_p
*
*  Implicit Outputs:
*
*    SDictControl:
*      - candidates
*      - cand_sorted
*      - cursave
*      - depth
*      - extract_patt
*      - extpat_p
*      - tbltops
*      - topprefix_p
*
*  Return Values:
*
*    Number of candidates stored in list.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t SCands_Get
  (
  Avd_Trie_Node_t *slgroot_p,                 /* root of the sling trie */
  Avd_Trie_Node_t *patroot_p                  /* root of the pattern trie */
  )
 {
  U8_t            ch;                         /* temp char */
  U8_t            top_i;                      /* Ith element of tabletops */
  U8_t            max_top;                    /* tabletop with max savings */
  size_t          pat_len;                    /* Length of pattern */
  S32_t           max_save;                   /* maximium estimated savings */
  U32_t           num_cands;                  /* number of candidates */
  U32_t           cand_i;                     /* Ith candidate */
  U32_t           count;                      /* count of pattern in trie */
  Boolean_t       made_changes;               /* Made changes to trie */
  Avd_Trie_Node_t *tn_p;                      /* sling subtrie ptr */

  num_cands = 0;
  max_save = -1;

  /*  Search through the sling trie looking for the pattern with the 
      greatest estimated savings.  Extract the found pattern, and update 
      pattern trie with it.  After all the patterns with at least a count 
      of AVD_COUNT_CUTOFF have been removed, select the AVD_CANDS_MAX_NUM
      best ones to be the candidates.  Continue to extract any of the 
      chosen candidates that may have moved up to the top of the sling trie 
      during the extractions or during the removal of the first level of
      the sling trie (pop tops).
  */

  while (max_save)
    {
    /*  Traverse the top level of the trie.  If any of the prefix subtries
	have been altered by an extraction, find the new best prefix for 
	that subtrie.
    */

    tn_p = slgroot_p;
    while (tn_p != NULL)
      {
      ch = Avd_TrieNode_Char_Get (tn_p);
      if (Avd_PreTbl_Change_Get (Avd_DictCtrl_TblTops_Get (SDictControl)[ch]))
	{
	Avd_DictCtrl_Depth_Put (SDictControl ,1);
	Avd_DictCtrl_CurSave_Put(SDictControl ,0);
	Avd_DictCtrl_TopPref_Put (SDictControl, Avd_PreTbl_Prefix_Get
	  (Avd_DictCtrl_TblTops_Get (SDictControl)[ch]));
	*(Avd_DictCtrl_TopPref_Get (SDictControl)) =  ch;
	*(++Avd_DictCtrl_TopPref_Get (SDictControl)) = AVD_EOS;

	SSavings_Calc (Avd_TrieNode_Next_Get (tn_p));

	Avd_PreTbl_Save_Put (Avd_DictCtrl_TblTops_Get (SDictControl)[ch], 
	  Avd_DictCtrl_CurSave_Get (SDictControl));
	Avd_PreTbl_Change_Put (Avd_DictCtrl_TblTops_Get (SDictControl)[ch],
	  FALSE);
	}
     
     tn_p = Avd_TrieNode_List_Get (tn_p);
     }  /* End of while more changed TableTops need to be processed */

    /*  Find the candidate pattern with largest estimated savings.  If 
	the savings is nonzero, extract it and place it in the pattern
	trie.
    */

    max_save = 0;
    max_top = 0;
    for (top_i = 0; top_i < AVD_CHARS_MAX_NUM; top_i++)
      {
      if (Avd_PreTbl_Save_Get (Avd_DictCtrl_TblTops_Get (SDictControl)[top_i])
	  > max_save)
	{
	max_save = Avd_PreTbl_Save_Get (Avd_DictCtrl_TblTops_Get 
	  (SDictControl)[top_i]);
	max_top = top_i;
	}
      }
   
    if (max_save)
      {
      Avd_PreTbl_Change_Put (Avd_DictCtrl_TblTops_Get (SDictControl)[max_top],
	TRUE);
      SPatt_Tree_Update (Avd_PreTbl_Prefix_Get (Avd_DictCtrl_TblTops_Get 
	(SDictControl)[max_top]), Avd_PreTbl_Save_Get (Avd_DictCtrl_TblTops_Get 
	(SDictControl)[max_top]), patroot_p);
      count = SPatts_Extract (slgroot_p, Avd_PreTbl_Prefix_Get 
	(Avd_DictCtrl_TblTops_Get (SDictControl)[max_top]));
      }

    } /* End of while there are still patterns with a nonzero savings */

  /*  Store only the best AVD_CANDS_MAX_NUM of the prefix patterns as   
      possible candidates.  
  */

  Avd_DictCtrl_CandSort_Put (SDictControl,FALSE);
  Avd_DictCtrl_ExtPatPtr_Put (SDictControl, Avd_DictCtrl_ExtPatt_Get 
    (SDictControl));
  *(Avd_DictCtrl_ExtPatPtr_Get (SDictControl)++) = AVD_EOS;
  num_cands = SCands_Extract (patroot_p, num_cands);
 
  /*  Continue to pop and extract the chosen candidates until the 
      sling trie is empty, in case the savings of certain candidates 
      can be increased by the sling fragments left in the sling trie.  
  */

  SCands_Patt_Sort (0, (S32_t) num_cands);
  made_changes = TRUE;
  while (made_changes)
    {
    made_changes = FALSE;
    for (cand_i = 0; cand_i < num_cands; cand_i++)
      {
      count = SPatts_Extract (slgroot_p, Avd_Candidate_Patt_Get 
	(Avd_DictCtrl_Cands_Get (SDictControl)[cand_i]));

      if (count)
	{
	pat_len = strlen ((char *) Avd_Candidate_Patt_Get 
	  (Avd_DictCtrl_Cands_Get (SDictControl)[cand_i])) - 1;
	Avd_Candidate_Save_Get (Avd_DictCtrl_Cands_Get 
	  (SDictControl)[cand_i]) += pat_len * count;
	made_changes = TRUE;
	}
      }
    }  /* End of while made changes */

  made_changes = STabletops_Pop (slgroot_p);
  while (made_changes)
    {
    /*  Check to see if any existing patts have bubbled up to the top. */

    while (made_changes)
      {
      made_changes = FALSE;
      for (cand_i = 0; cand_i < num_cands; cand_i++)
	{
	count = SPatts_Extract (slgroot_p, Avd_Candidate_Patt_Get 
	  (Avd_DictCtrl_Cands_Get (SDictControl)[cand_i]));

	if (count)
	  {
	  pat_len = strlen ((char *) Avd_Candidate_Patt_Get 
	   (Avd_DictCtrl_Cands_Get (SDictControl)[cand_i])) - 1;
	  Avd_Candidate_Save_Get (Avd_DictCtrl_Cands_Get (SDictControl)[cand_i])
	    += pat_len * count;
	  made_changes = TRUE;
	  }
       }
    } /* End of while more have bubbled up */

    made_changes = STabletops_Pop (slgroot_p);

  } /* End of while more to pop */

  return num_cands;
}
/* End of SCands_Get  */

/****************************************************************************
*
*  Function Name:                 SCands_Extract
*
*    Recursively traverse the pattern trie extracting those patterns
*    with high expected savings and storing them in the list of candidates.
*
*    
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
*      - extract_patt
*      - extpat_p
*      - cand_sorted
*
*  Implicit Outputs:
*
*    SDictControl:
*      - candidates
*      - extract_patt
*      - extpat_p
*      - cand_sorted
*
*  Return Values:
*
*    Number of candidates saved in the list.
*
*  Side Effects:
*
*    Allocates memory for candidate string:  may exit on error.
*
******************************************************************************/
U32_t SCands_Extract
  (
  Avd_Trie_Node_t *pn_p,                      /* for traversing patt trie */
  U32_t           cand_i                      /* number of candidates saved */
  )
{
 
  while (pn_p != NULL)
    {
    *(Avd_DictCtrl_ExtPatPtr_Get (SDictControl) - 1) 
      = Avd_TrieNode_Char_Get (pn_p);
    *Avd_DictCtrl_ExtPatPtr_Get (SDictControl) = AVD_EOS;

    /*  If worthwhile, attempt to store the pattern ending at this node. */

    if (Avd_TrieNode_Value_Get (pn_p))
      {
      /*  If there is room in the candidate list,  store this pattern. */

      if (cand_i < AVD_CANDS_MAX_NUM)
	{
	size_t    pat_len;                    /* temp length of pattern */  
   
	pat_len = strlen ((char *) Avd_DictCtrl_ExtPatt_Get (SDictControl)) 
	  + 1;
	Avd_Candidate_Patt_Put (Avd_DictCtrl_Cands_Get 
	  (SDictControl)[cand_i], (U8_t *) malloc (pat_len));
	strcpy ((char *) Avd_Candidate_Patt_Get (Avd_DictCtrl_Cands_Get 
	  (SDictControl)[cand_i]), (char *) Avd_DictCtrl_ExtPatt_Get 
	  (SDictControl));
	Avd_Candidate_Save_Put (Avd_DictCtrl_Cands_Get 
	  (SDictControl)[cand_i],  Avd_TrieNode_Value_Get (pn_p));
	++cand_i;
	} /* End of if room to store */
     
      /*  Otherwise, try to replace a candidate with smaller savings with
	  this one.
      */
      else
	{
	S32_t     cand_j;                     /* temp index into cand list */
	S32_t     cand_k;                     /* temp index into cand list */
	size_t    pat_len;                    /* length of candidate patt */

	/*  Sort the candidates in decreasing order according to savings.
	    If the smallest has less savings, throw it away and insert the
	    new candidate in its proper place in the list.
	*/

	if (!Avd_DictCtrl_CandSort_Get (SDictControl))
	  {
	  SCands_Save_Sort (0, AVD_CANDS_MAX_NUM);
	
	  Avd_DictCtrl_CandSort_Put (SDictControl, TRUE);
	  }

	if (Avd_TrieNode_Value_Get (pn_p) > Avd_Candidate_Save_Get 
	    (Avd_DictCtrl_Cands_Get (SDictControl)[AVD_CANDS_MAX_NUM - 1]))
	  {
	  free (Avd_Candidate_Patt_Get (Avd_DictCtrl_Cands_Get 
	    (SDictControl)[AVD_CANDS_MAX_NUM - 1]));
	  cand_j = AVD_CANDS_MAX_NUM - 1;

	  while ((cand_j >= 0) 
	      && (Avd_Candidate_Save_Get (Avd_DictCtrl_Cands_Get
	      (SDictControl)[cand_j]) < Avd_TrieNode_Value_Get (pn_p)))
	    --cand_j;

	  ++cand_j;
	  for (cand_k = AVD_CANDS_MAX_NUM - 1; cand_k > cand_j; cand_k--)
	    Avd_DictCtrl_Cands_Get (SDictControl)[cand_k]
	      = Avd_DictCtrl_Cands_Get (SDictControl)[cand_k - 1];

	  pat_len = strlen ((char *) Avd_DictCtrl_ExtPatt_Get (SDictControl))
	    + 1;
	  Avd_Candidate_Patt_Put (Avd_DictCtrl_Cands_Get 
	    (SDictControl)[cand_j], (U8_t *) malloc (pat_len));
	  strcpy ((char *) Avd_Candidate_Patt_Get (Avd_DictCtrl_Cands_Get
	    (SDictControl)[cand_j]), (char *) Avd_DictCtrl_ExtPatt_Get 
	    (SDictControl));
	  Avd_Candidate_Save_Put (Avd_DictCtrl_Cands_Get
	    (SDictControl)[cand_j], Avd_TrieNode_Value_Get (pn_p));         
	  }
	}  /*  End of else replace cand with smaller save  */
      } /* End of if room to store pattern */

    ++Avd_DictCtrl_ExtPatPtr_Get (SDictControl);

    cand_i = SCands_Extract (Avd_TrieNode_Next_Get (pn_p), cand_i);

    --Avd_DictCtrl_ExtPatPtr_Get (SDictControl);

    pn_p = Avd_TrieNode_List_Get (pn_p);
    }  /* End of while more nodes in list */

  return cand_i;
}
/* End of SCands_Extract  */

/****************************************************************************
*
*  Function Name:                  SCands_Patt_Merge
*
*    Merge sort algorithm:  merges two presorted sublists of candidates.
*    Takes advantage of the fact that the two sublists are adjacent.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
*
*  Implicit Outputs:
*
*    SDictControl:
*      - candidates
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
void SCands_Patt_Merge
  (
  S32_t           low,                        /* low range of low sublist */
  S32_t           mid,                        /* mid range of sublists */
  S32_t           high                        /* high range of high sublist */
  )
{
  S32_t           l_i;                        /* index for low sublist */
  S32_t           m_i;                        /* index for high sublist */
  S32_t           n_i;                        /* index for temp sublist */
  Avd_Candidate_t temp[AVD_CANDS_MAX_NUM];

  l_i = low;
  m_i = mid;
  n_i = low;

  while ((l_i < mid) && (m_i < high))
    {
    if (strcmp((char *) Avd_Candidate_Patt_Get (Avd_DictCtrl_Cands_Get 
	(SDictControl)[l_i]), (char *) Avd_Candidate_Patt_Get 
	(Avd_DictCtrl_Cands_Get (SDictControl)[m_i])) > 0)
      temp[n_i++] = Avd_DictCtrl_Cands_Get (SDictControl)[l_i++];

    else
      temp[n_i++] = Avd_DictCtrl_Cands_Get (SDictControl)[m_i++];
    }

  if (l_i >= mid)
    for (; n_i < high; n_i++)
      temp[n_i] = Avd_DictCtrl_Cands_Get (SDictControl)[m_i++];

  else
    for (; n_i < high; n_i++)
      temp[n_i] = Avd_DictCtrl_Cands_Get (SDictControl)[l_i++];

  for (n_i = low; n_i < high; n_i++)
    Avd_DictCtrl_Cands_Get (SDictControl)[n_i] = temp[n_i];

  return;
}
/* End of SCands_Patt_Merge  */

/****************************************************************************
*
*  Function Name:                  SCands_Patt_Sort
*
*    Sort the list of candidates in decreasing order according to the 
*    patterns.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
*
*  Implicit Outputs:
*
*    SDictControl:
*      - candidates
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
void SCands_Patt_Sort
  (
  S32_t           low,                        /* low range of sublist */
  S32_t           high                        /* high range of sublist */
  )
{
  S32_t           mid;                        /* mid range of sublists */

  mid = (low + high)/2;
  if (low < mid-1)
    SCands_Patt_Sort (low, mid);

  if (mid < high-1)
    SCands_Patt_Sort (mid, high);

  SCands_Patt_Merge (low, mid, high); 

  return;
}
/* End of SCands_Patt_Sort */

/****************************************************************************
*
*  Function Name:                  SCands_Save_Merge
*
*    Merge sort algorithm:  merges two presorted sublists of candidates.
*    Takes advantage of the fact that the two sublists are adjacent.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
*
*  Implicit Outputs:
*
*    SDictControl:
*      - candidates
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
void SCands_Save_Merge
  (
  S32_t           low,                        /* low range of low sublist */
  S32_t           mid,                        /* mid range of sublists */
  S32_t           high                        /* high range of high sublist */
  )
{
  S32_t           l_i;                        /* index for low sublist */
  S32_t           m_i;                        /* index for high sublist */
  S32_t           n_i;                        /* index for temp sublist */
  Avd_Candidate_t temp[AVD_CANDS_MAX_NUM];

  l_i = low;
  m_i = mid;
  n_i = low;

  while ((l_i < mid) && (m_i < high))
    {
    if (Avd_Candidate_Save_Get (Avd_DictCtrl_Cands_Get (SDictControl)[l_i])
      > Avd_Candidate_Save_Get (Avd_DictCtrl_Cands_Get (SDictControl)[m_i]))
      temp[n_i++] = Avd_DictCtrl_Cands_Get (SDictControl)[l_i++];

    else
      temp[n_i++] = Avd_DictCtrl_Cands_Get (SDictControl)[m_i++];
    }

  if (l_i >= mid)
    for (; n_i < high; n_i++)
      temp[n_i] = Avd_DictCtrl_Cands_Get (SDictControl)[m_i++];

  else
    for (; n_i < high; n_i++)
      temp[n_i] = Avd_DictCtrl_Cands_Get (SDictControl)[l_i++];

  for (n_i = low; n_i < high; n_i++)
    Avd_DictCtrl_Cands_Get (SDictControl)[n_i] = temp[n_i];

  return;
}
/* End of SCands_Save_Merge  */

/****************************************************************************
*
*  Function Name:                  SCands_Save_Sort
*
*    Sort the list of candidates in decreasing order according to the 
*    expected savings.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
*
*  Implicit Outputs:
*
*    SDictControl:
*      - candidates
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
void SCands_Save_Sort
  (
  S32_t           low,                        /* low range of sublist */
  S32_t           high                        /* high range of sublist */
  )
{
  S32_t           mid;                        /* mid range of sublists */

  mid = (low + high)/2;
  if (low < mid-1)
    SCands_Save_Sort (low, mid);

  if (mid < high-1)
    SCands_Save_Sort (mid, high);

  SCands_Save_Merge (low, mid, high); 

  return;
}
/* End of SCands_Save_Sort  */

/****************************************************************************
*
*  Function Name:                 SCount_Get
*
*    Get the count for the given sling substring.
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
*    Count of substring saved in sling trie.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
S32_t SCount_Get
  (
  Avd_Trie_Node_t *tn_p,                      /* trie to search through */
  U8_t           *ch_p                        /* substring to search for */
  )
{

  while (*ch_p)
    {

    /*  Search through trie list for current character. */

    while ((tn_p != NULL) && (Avd_TrieNode_Char_Get (tn_p) < *ch_p))  
     tn_p = Avd_TrieNode_List_Get (tn_p);

    /*  If the string is not in current trie, the count is zero. */

    if (tn_p == NULL)
      return 0;                     

   /*  If the char was in the list, then if we are at the end of the substring
       return the count, otherwise move down the trie.
   */

    if (Avd_TrieNode_Char_Get (tn_p) == *ch_p)                       
      {
      if (!*++ch_p)
	return Avd_TrieNode_Value_Get (tn_p);  

      else
	tn_p = Avd_TrieNode_Next_Get (tn_p);
      }

    /*  Otherwise, the character was not in the list, so count is zero. */

    else           
      return 0;

   }  /* End of while not at end of string */

  return 0;
}
/* End of SCount_Get */

/****************************************************************************
*
*  Function Name:                 SCount_Reduce
*
*    Traverse the path of the trie for the given substring and decrement
*    the values for the node along that path.  Return the subtrie rooted
*    at the node containing the last character of substring so that it 
*    can be merged with trie.
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
*    Subtrie rooted at the node containing the last character of substring.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Avd_Trie_Node_t *SCount_Reduce
  (
  Avd_Trie_Node_t *tn_p,                      /* trie to search through */
  U8_t           *ch_p,                       /* substring to search for */
  S32_t           decrmt                      /* amount to decrement */
  )
{

  while (*ch_p)
    {

    /*  Search through trie list for current character. */

    while ((tn_p != NULL) && (Avd_TrieNode_Char_Get (tn_p) < *ch_p))  
      tn_p = Avd_TrieNode_List_Get (tn_p);

    /*  If the string is not in current trie, the subtrie is empty. */

    if (tn_p == NULL)
      return NULL;                  

    /*  If the char was in the list, decrement value of node.  If this is
	the end of the substring, return the subtrie rooted at this node.  
	Otherwise, continue down trie.
    */

    if (Avd_TrieNode_Char_Get (tn_p) == *ch_p)                       
      {
      Avd_TrieNode_Value_Get (tn_p) -= decrmt;

      /*  If we are at the end of the string, return end node. */

      if (!*++ch_p)
	return tn_p; 

      else
	tn_p = Avd_TrieNode_Next_Get (tn_p);
      }

    /*  Otherwise, the substring is not in the trie, so return an empty 
	subtrie.
    */

    else           
      return NULL;
    }  /*  End of while not at end of string  */

  return NULL;
}
/* End of SCount_Reduce */

/****************************************************************************
*
*  Function Name:                 SCSlgs_Comp
*
*    Compare the two compressed slings.
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
S16_t SCSlgs_Comp
  (
  Avc_Slg_Code_t *cslg1_p,                    /* compressed sling */
  Avc_Slg_Code_t *cslg2_p                     /* compressed sling */
  )
{

  while (*cslg1_p && *cslg2_p &&  *cslg1_p ==  *cslg2_p)
    {
    ++cslg1_p;
    ++cslg2_p;
    }

  if (*cslg1_p ==  *cslg2_p)
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
/*  End of SCSlgs_Comp  */

/****************************************************************************
*
*  Function Name:                  SCSlg_Key_Merge
*
*    Merge sort algorithm:  merges two presorted sublists of slg_keys.
*    Takes advantage of the fact that the two sublists are adjacent.
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
void SCSlg_Key_Merge
  (
  S32_t           low,                        /* low range of low sublist */
  S32_t           mid,                        /* mid range of sublists */
  S32_t           high,                       /* high range of high sublist */
  Avd_Slg_Key_t   slings[]
  )
{
  S32_t           l_i;                        /* index for low sublist */
  S32_t           m_i;                        /* index for high sublist */
  S32_t           n_i;                        /* index for temp sublist */
  Avd_Slg_Key_t   temp[AVD_SLGS_MAX_NUM];

  l_i = low;
  m_i = mid;
  n_i = low;

  while ((l_i < mid) && (m_i < high))
    {
    if (SCSlgs_Comp (Avd_SlgKey_CSlg_Get (slings[l_i]), 
	Avd_SlgKey_CSlg_Get (slings[m_i])) < 0)
      temp[n_i++] = slings[l_i++];
    else
      temp[n_i++] = slings[m_i++];
    }

  if (l_i >= mid)
    for (; n_i < high; n_i++)
      temp[n_i] = slings[m_i++];
  else
    for (; n_i < high; n_i++)
      temp[n_i] = slings[l_i++];

  for (n_i = low; n_i < high; n_i++)
    slings[n_i] = temp[n_i];

  return;
}
/* End of SCSlg_Key_Merge  */

/****************************************************************************
*
*  Function Name:                  SCSlg_Key_Sort
*
*    Sort the list of compressed slings in increasing order.
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
void SCSlg_Key_Sort
  (
  S32_t           low,                        /* low range of sublist */
  S32_t           high,                       /* high range of sublist */
  Avd_Slg_Key_t   slings[]
  )
{
  S32_t           mid;                        /* mid range of sublists */

  mid = (low + high)/2;
  if (low < mid-1)
    SCSlg_Key_Sort (low, mid, slings);

  if (mid < high-1)
    SCSlg_Key_Sort (mid, high, slings);

  SCSlg_Key_Merge (low, mid, high, slings); 

  return;
}
/* End of SCSlg_Key_Sort */

/****************************************************************************
*
*  Function Name:                  SPatt_Chain_Create
*
*    Builds the chain of new nodes in the trie for the new suffix of a string.
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
void SPatt_Chain_Create
  (
  U8_t           *newch_p,                    /* new suffix of string */
  S32_t           save,                       /* savings to add in last node */
  Avd_Trie_Node_t *tn_p                       /* root of new chain */
  )
{

  while (*newch_p)
    {
    Avd_TrieNode_Next_Put (tn_p, 
      STrie_Node_Create (*newch_p, AVD_PATT_NODE_INIT_VAL, NULL));
    tn_p = Avd_TrieNode_Next_Get (tn_p);
   
    /*  If at end of string, add savings. */

    if (!*++newch_p)
      {
      Avd_TrieNode_Value_Get (tn_p) += save;
      return;
      }
   } 

  return;
} 
/* End of SPatt_Chain_Create  */

/****************************************************************************
*
*  Function Name:                 SPatt_DeQueue
*
*    Removes the head of the queue and returns a pointer to the entry.
*    Assumes that the calling function will eventually free the memory
*    allocated to the node.
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
*    A pointer to the queue entry removed from the queue.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/ 
Avd_Queue_Node_t *SPatt_DeQueue
  (
  Avd_Queue_Node_t **queue_head_p,            /* head of queue */
  Avd_Queue_Node_t **queue_tail_p             /* tail of queue */
  )
{
  Avd_Queue_Node_t *this_one_p;               /* node to be removed */

  /*  Make sure queue is not empty. */

  if (*queue_head_p == NULL)
    return NULL;

  /*  If this is the last element of queue, reset head and tail. */

  if (*queue_head_p == *queue_tail_p)
    {
    this_one_p = *queue_head_p;
    *queue_head_p = NULL;
    *queue_tail_p = NULL;
    return this_one_p;
    }

  /*  Otherwise, pop off head of queue and advance head. */

  this_one_p = *queue_head_p;
  *queue_head_p = Avd_QueueNode_Next_Get (*queue_head_p);

  return this_one_p;
}
/* End of SPatt_DeQueue */

/****************************************************************************
*
*  Function Name:                 SPatt_EnQueue
*
*    Adds a new queue entry at the tail of the queue.
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
void SPatt_EnQueue
  (
  Avd_Trie_Node_t *tn_p,                      /* rest of trie list */
  Avc_Trie_Table_t *tb_entry_p,               /* table entry for thsi node */
  Avd_Queue_Node_t **queue_head_p,            /* queue head */
  Avd_Queue_Node_t **queue_tail_p             /* queue tail */
  )
{
  /*  If the queue is empty, initialize it with this node.
      Otherwise, add node to end of queue.
  */

  if (*queue_tail_p == NULL)
    {
    *queue_tail_p = SPatt_Queue_Node_Create (tn_p, tb_entry_p);
    *queue_head_p = *queue_tail_p;
    }
  else
    {
    Avd_QueueNode_Next_Put (*queue_tail_p, 
      SPatt_Queue_Node_Create (tn_p, tb_entry_p));
    *queue_tail_p = Avd_QueueNode_Next_Get (*queue_tail_p);
    }

  return;
}
/* End of SPatt_EnQueue */

/****************************************************************************
*
*  Function Name:                 SPatt_Queue_Node_Create
*
*    Creates a new node for the queue and initializes the fields.
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
*     Pointer to the new node with fields initialized.
*
*  Side Effects:
*
*    Allocates memory for the queue node--may exit on error.
*
******************************************************************************/
Avd_Queue_Node_t *SPatt_Queue_Node_Create 
  (
  Avd_Trie_Node_t *pn_p,                      /* rest of trie list */
  Avc_Trie_Table_t *tb_entry_p                /* table entry for this node */ 
  )
{
  Avd_Queue_Node_t *new_node_p;               /* new quewue node */

  new_node_p = (Avd_Queue_Node_t *) malloc (AVD_QUEUE_NODE_SZ);
  if (new_node_p == NULL)
    {
    fprintf (stderr, "\nSPatt_Queue_Node_Create:  memory allocation error.\n");
    exit (-1);
    }

  Avd_QueueNode_TrList_Put (new_node_p, pn_p);
  Avd_QueueNode_TbEnt_Put (new_node_p, tb_entry_p);
  Avd_QueueNode_Next_Put (new_node_p, NULL);
  return new_node_p;
}
/* End of SPatt_Queue_Node_Create */

/****************************************************************************
*
*  Function Name:                 SPatt_Table_Create
*
*    Converts the pattern tree constructed using pointers into a flat
*    character array, the pattern table, which can be traversed in a
*    similar manner.  Entries into the table all have the same format:
*
*    signed char   next/offset   code            explanation
*     +(char)        0--255     0--255    This entry has no next or code if 
*                                           those field values are zero.
*     -(char)        0--255     0--255    This entry is the last in the list.
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
*    The size of the table in bytes.
*
*  Side Effects:
*
*    May exit with an error if offset into patt table is too large.
*
******************************************************************************/
size_t SPatt_Table_Create
  (
  Avd_Trie_Node_t *patcdroot_p,               /* root of patt/code trie */
  Avc_Trie_Table_t *table_p,                  /* patt encode dict table */
  Avd_Queue_Node_t **queue_head_p,            /* head of queue */
  Avd_Queue_Node_t **queue_tail_p             /* tail of queue */
  )
{
  Avd_Trie_Node_t *patt_list_p;               /* rest of patt/cd trie list */
  Avc_Trie_Table_t *cur_tb_entry_p;           /* cur entry into patt table */
  Avd_Queue_Node_t *cur_qnode_p;              /* cur node of queue */
  ptrdiff_t       temp_offset;                /* store calculated offset */
 
  /*  Initialize variables and queue. */

  *queue_head_p = NULL;
  *queue_tail_p = NULL;
  cur_tb_entry_p = table_p;

  SPatt_EnQueue (Avd_TrieNode_List_Get (patcdroot_p), cur_tb_entry_p,
    queue_head_p, queue_tail_p);

  /*  Continue to pop the head node off the queue and process that node
    until the queue is empty.  The root node, which contains no pattern
    information, is ignored.    
  */

  cur_qnode_p = SPatt_DeQueue (queue_head_p, queue_tail_p);

  while (cur_qnode_p != NULL)
    {
    /*  If the offset for the next field of the parent node, which is 
	calculated as the number of entries, not bytes, is too large to 
	store, exit with error message.  Otherwise, store offset in next 
	field and process nonempty list of patt_list nodes.   
    */

    temp_offset = (cur_tb_entry_p - (Avd_QueueNode_TbEnt_Get (cur_qnode_p))) 
      / AVC_TBL_ENTRY_LEN;
    if (temp_offset > AVD_PATT_TBL_MAX_OFFSET)
      {
      fprintf (stderr, "\nSPatt_Table_Create:  ERROR---offset too large.\n");
      exit(-1);
      }

    else
      {
      *((Avd_QueueNode_TbEnt_Get (cur_qnode_p)) + AVC_TBL_NEXT_OFF) = 
	(Avc_Slg_Next_t) temp_offset;
      }

    /*  Process the list of patt_nodes:  Place each non-null next node 
	on the queue; then enter the character and the code (using the
	value field of the patt_node) values in the appropriate fields
	of the table entry.
    */

    patt_list_p = Avd_QueueNode_TrList_Get (cur_qnode_p);
    free (cur_qnode_p);

    while (patt_list_p != NULL)
      {
      if (Avd_TrieNode_Next_Get (patt_list_p) == NULL)
	*((cur_tb_entry_p) + AVC_TBL_NEXT_OFF) = AVC_NULL_NEXT;

      else
	SPatt_EnQueue (Avd_TrieNode_Next_Get (patt_list_p), cur_tb_entry_p,
	  queue_head_p, queue_tail_p);

      *(cur_tb_entry_p + AVC_TBL_CHAR_OFF) 
	= (Avc_Slg_Char_t) Avd_TrieNode_Char_Get (patt_list_p);
      *(cur_tb_entry_p + AVC_TBL_CODE_OFF) 
	= (Avc_Slg_Code_t) Avd_TrieNode_Value_Get (patt_list_p);
      cur_tb_entry_p += AVC_TBL_ENTRY_LEN;
	
      patt_list_p = Avd_TrieNode_List_Get (patt_list_p);
      }   /* End of while more elements in patt list */  

    /*  When the end of the list is reached, go back and mark the previous
	entry as the last in the list (the char is negated).  
    */

    *(cur_tb_entry_p - AVC_TBL_ENTRY_LEN + AVC_TBL_CHAR_OFF) 
       = - *(cur_tb_entry_p - AVC_TBL_ENTRY_LEN + AVC_TBL_CHAR_OFF);

    cur_qnode_p = SPatt_DeQueue (queue_head_p, queue_tail_p);
    }    /* End of while more nodes in the queue */

  /*  Append null entry at end of table. */

  *cur_tb_entry_p = AVC_NULL_CHAR;
  cur_tb_entry_p += AVC_TBL_CHAR_SZ;
  *cur_tb_entry_p = AVC_NULL_NEXT;
  cur_tb_entry_p += AVC_TBL_NEXT_SZ;
  *cur_tb_entry_p = AVC_NULL_CODE;
  cur_tb_entry_p += AVC_TBL_CODE_SZ;

 return (cur_tb_entry_p - table_p);
}
/* End of SPatt_Table_Create */

/****************************************************************************
*
*  Function Name:                  SPatt_Tree_Update
*
*    If the pattern is new, insert if into the prefix trie tree.  
*    Otherwise, update the existing trie by adding the savings to the
*    node containing the last character of the pattern.
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
*     None.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SPatt_Tree_Update
  (  
  U8_t           *ch_p,                       /* pattern to insert */
  S32_t           save,                       /* savings for pattern */
  Avd_Trie_Node_t  *cn_p                      /* current trie node */
  ) 
{
  Avd_Trie_Node_t  *bn_p;                     /* ptr to prev node in list */
  Avd_Trie_Node_t  *un_p;                     /* ptr to prev node in chain */

  bn_p = cn_p;
  un_p = NULL;

  while (*ch_p)
    { 

    /*  Search through ordered list for current char in pattern. */

    while ((Avd_TrieNode_Char_Get (cn_p) < *ch_p) 
	&& (Avd_TrieNode_List_Get (cn_p) != NULL))
      {  
      bn_p = cn_p;
      cn_p = Avd_TrieNode_List_Get (cn_p);
      }

    /*  If the char was in the list ... */

    if (Avd_TrieNode_Char_Get (cn_p) == *ch_p)
      {

      /*  And are at the end of the string, increment savings. */

      if (!*++ch_p)
	{
	Avd_TrieNode_Value_Get (cn_p) += save;
	return;
	}

      /*  Otherwise, if at end of a chain in pattern tree, create a new chain 
	  with rest of pattern string.
      */
      else
	{
	if (Avd_TrieNode_Next_Get (cn_p) == NULL)
	  { 
	  SPatt_Chain_Create (ch_p, save, cn_p);
	  return;                            
	  }

	/*  Otherwise,  there is room to move down the tree, so move down a
	    level.  
	*/
	else
	  {   
	  un_p = cn_p;
	  cn_p = Avd_TrieNode_Next_Get (cn_p);
	  bn_p = cn_p;
	  }
	} 
      }

    /*  Otherwise, the character was not in list, so insert it in proper 
	position in the list.  
    */
    else                                
      {
      /*  If at end of the list, append new node. */

      if (*ch_p > Avd_TrieNode_Char_Get (cn_p)) 
	{
	Avd_TrieNode_List_Put (cn_p, 
	  STrie_Node_Create (*ch_p, AVD_PATT_NODE_INIT_VAL, NULL));
	cn_p = Avd_TrieNode_List_Get (cn_p);
	}

      /*  Otherwise, either prepend or insert node. */

      else                                       
	{
	/*  If at the beginning of list, prepend the new node. */

	if (bn_p == cn_p)
	  {
	  Avd_TrieNode_Next_Put (un_p,
	    STrie_Node_Create (*ch_p, AVD_PATT_NODE_INIT_VAL, cn_p));
	  cn_p = Avd_TrieNode_Next_Get (un_p);
	  }

	/*  Otherwise, insert it between the backnode and the current node. */
	else             
	  {
	  Avd_TrieNode_List_Put (bn_p,
	    STrie_Node_Create (*ch_p, AVD_PATT_NODE_INIT_VAL, cn_p));
	  cn_p = Avd_TrieNode_List_Get (bn_p);
	  }
	}   /* End of else prepend or insert node */

      /*  If at end of the string, increment savings. */

      if (!*++ch_p)
	{
	Avd_TrieNode_Value_Get (cn_p) += save;
	return;
	}

      /*  Otherwise, finish building chain of new nodes and return 
	  since we are at the end of the string.
      */
      else
	{
	SPatt_Chain_Create (ch_p, save, cn_p);
	return;  
	}
      }   /* End of else char was not in list */
   }   /* End of while not at the end of the pattern */

  return;
} 
/* End of  SPatt_Tree_Update */

/****************************************************************************
*
*  Function Name:                 SPatts_Codes_Trie_Create
*
*    Builds the pattern dictionary trie usning the list of candidates.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - candidates
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
void SPatts_Codes_Trie_Create
  (
  Boolean_t       chars[],                    /* chars occurring in slings */
  U32_t           num_cands,                  /* number of candidates */
  Avd_Trie_Node_t *patcdroot_p                /* root of patt code trie */
  )
{ 
  U8_t            ch;                         /* index into chars array */
  U32_t           cand_i;                     /* Ith candidate */
  S32_t           next_code;                  /* next available code */
  U8_t            ch_str[2];                  /* temp char string */

  next_code = 1;
  ch_str[1] = AVD_EOS;

  /*  First create codes for all the characters which occurred in the 
      slings.  
  */

  for (ch = 0; ch < AVD_CHARS_MAX_NUM; ch++)
    {
    if (chars[ch])
      {
      *ch_str = ch;
      SPatt_Tree_Update (ch_str, next_code, patcdroot_p);
      next_code++;
      }
    }

  /*  Then take the best of the candidates for remaining patterns. */

  if (num_cands > (AVD_PATTS_MAX_NUM - next_code))
    {
    cand_i = 0;
    for ( ; next_code <= AVD_PATTS_MAX_NUM; next_code++)
      {
      SPatt_Tree_Update (Avd_Candidate_Patt_Get (Avd_DictCtrl_Cands_Get 
	(SDictControl)[cand_i]), next_code, patcdroot_p);
      ++cand_i;
      }
    }
  else
    {
    for (cand_i = 0; cand_i < num_cands; cand_i++)
      {
      SPatt_Tree_Update (Avd_Candidate_Patt_Get (Avd_DictCtrl_Cands_Get 
	(SDictControl)[cand_i]), next_code, patcdroot_p);
      ++next_code;
      }
    }

  return;
}  
/* End of  SPatts_Codes_Trie_Create */

/****************************************************************************
*
*  Function Name:                 SPatts_Extract
*
*    Extract the given pattern from the pattern trie.  Reduce the counts 
*    of the nodes on the path through the trie containing the pattern, 
*    and merge the dangling subtrie with the main trie.
*
*
*  Implicit Inputs:
*
*    SDictControl:
*      - tbltops
*
*  Implicit Outputs:
*
*    SDictControl:
*      - tbltops
*
*  Return Values:
*
*    The count of number of occurrances of pattern.
*
*  Side Effects:
*
*    Exit program if we fall out of the prefix trie.
*
******************************************************************************/
S32_t SPatts_Extract
  (
  Avd_Trie_Node_t *tn_p,                      /* trie to extract pat from */
  U8_t           *ch_p                        /* pattern to extract */
  )
{
  S32_t           count;                      /* count of the pattern */
  Avd_Trie_Node_t *end_p;                     /* node of last char */
  Avd_Trie_Node_t *toplist_p;                 /* top list of trie */
  Avd_Trie_Node_t *endnext_p;                 /* root of dangling subtrie */

  /*  If this subtrie has not been altered, there is no need to attempt 
      to extract the pattern.
  */

  if (!Avd_PreTbl_Change_Get (Avd_DictCtrl_TblTops_Get 
      (SDictControl)[*(ch_p)]))
    return 0;

  /*  If there is no count for this pattern, then there is no
      need to extract it.
  */

  count = SCount_Get (tn_p, ch_p);
  if (count == 0)                    
    return 0;

  /*  Otherwise, reduce the counts of the nodes on the path through
      the trie containing the pattern, and merge the dangling subtrie
      with the main trie.
  */

  end_p = SCount_Reduce (tn_p, ch_p, count);
  if (end_p == NULL)
    {
    fprintf (stderr, "\n\nSPatts_Extract:  fell out of prefix tree.\n\n");
    exit (-1);
    }

  /*  Disconnect subtree from prefix tree, and merge with tree. */

  endnext_p = Avd_TrieNode_Next_Get (end_p);
  Avd_TrieNode_Next_Put (end_p, NULL);    
  if (endnext_p != NULL)
    {

    /*  First mark TableTops of top list of subtree as changed. */

    toplist_p = endnext_p;
    while (toplist_p != NULL)
      {
      Avd_PreTbl_Change_Put (Avd_DictCtrl_TblTops_Get 
	(SDictControl)[Avd_TrieNode_Char_Get (toplist_p)], TRUE);
      toplist_p = Avd_TrieNode_List_Get (toplist_p);
      }

    SSling_Subtrie_Merge (Avd_TrieNode_List_Get (tn_p), tn_p, 
      NULL, endnext_p);
    }

  return count;
}
/* End of SPatts_Extract  */

/****************************************************************************
*
*  Function Name:                 SSavings_Calc
*
*    Recursively find the prefix in the given sling subtrie which has the 
*    greatest estimated savings.  The savings, depth and prefix are
*    stored in the control structure.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - cursave
*      - depth
*      - topprefix_p
*
*  Implicit Outputs:
*
*    SDictControl:
*      - cursave
*      - depth
*      - topprefix_p
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
void SSavings_Calc
  (
  Avd_Trie_Node_t *tn_p                       /* sling subtrie */
  )
{

  /*  If we have hit a deadend, backup. */

  if (tn_p == NULL) 
    {
    --Avd_DictCtrl_Depth_Get (SDictControl);
    --Avd_DictCtrl_TopPref_Get (SDictControl);
    return;
    }

  /*  If the pathway down through this node leads to prefixes with smaller
      estimated savings, skip and continue the search through the other
      nodes in this list.
  */

  if ((Avd_TrieNode_Value_Get (tn_p) < AVD_COUNT_CUTOFF) 
      || (Avd_DictCtrl_CurSave_Get (SDictControl) 
      > (Avd_TrieNode_Value_Get (tn_p) 
      * Avd_DictCtrl_Depth_Get (SDictControl))))
    {
    SSavings_Calc (Avd_TrieNode_List_Get (tn_p));
    return;
    }

  /*  Otherwise, select this as the current prefix, update depth, savings
      and the prefix and move down the trie.
  */ 

  Avd_DictCtrl_CurSave_Put (SDictControl, Avd_TrieNode_Value_Get (tn_p)
    * Avd_DictCtrl_Depth_Get (SDictControl));
  Avd_DictCtrl_Depth_Get (SDictControl)++;
  *(Avd_DictCtrl_TopPref_Get (SDictControl)++) = Avd_TrieNode_Char_Get (tn_p);
  *Avd_DictCtrl_TopPref_Get (SDictControl) = AVD_EOS;
  SSavings_Calc (Avd_TrieNode_Next_Get (tn_p));

  /*  Continue to traverse the rest of the subtrie, in case there is a 
      better prefix we haven't seen yet.
  */ 

  SSavings_Calc (Avd_TrieNode_List_Get (tn_p));
  return;
}
/* End of SSavings_Calc */

/****************************************************************************
*
*  Function Name:                  SSling_Chain_Create
*
*    Builds the chain of new nodes in the trie for the new suffix of a sling.
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
void SSling_Chain_Create
  (
  U8_t           *newch_p,                    /* new sling suffix */
  Avd_Trie_Node_t *tn_p                       /* head of the trie chain */
  )
{

 while (*newch_p)
   {
   Avd_TrieNode_Next_Put (tn_p, 
     STrie_Node_Create (*newch_p, AVD_SLG_NODE_INIT_VAL, NULL));
   newch_p++;
   tn_p = Avd_TrieNode_Next_Get (tn_p);
   }

  return;
} 
/* End of SSling_Chain_Create  */

/****************************************************************************
*
*  Function Name:                 SSling_Tree_Create
*
*    Build the prefix tree using the list of slings.
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
void SSling_Tree_Create
  (
  U32_t           sling_count,                /* number of slings */
  Avd_Trie_Node_t *slgroot_p,                 /* root of sling trie */
  Avd_Slg_Key_t  *slings                      /* list of slings */
  )
{

  for (; sling_count > 0; sling_count--)
    {
    SSling_Tree_Update (Avd_SlgKey_Sling_Get (*slings), slgroot_p);
    ++slings;
    }  

  return;
} 
/* End of SSling_Tree_Create  */

/****************************************************************************
*
*  Function Name:                  SSling_Tree_Update
*
*    Recursively insert given sling and update count values along the path   
*    in the sling trie.
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
*     None.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSling_Tree_Update
  (  
  U8_t           *ch_p,                       /* sling ptr */
  Avd_Trie_Node_t *cn_p                       /* sling trie ptr */
  ) 
{
  Avd_Trie_Node_t *bn_p;                      /* ptr to prev node in list */
  Avd_Trie_Node_t *un_p;                      /* ptr to prev node in chain */

  bn_p = cn_p;
  un_p = NULL;

  while (*ch_p)
    { 
    /*  Search through ordered list for current char in sublist.  If the 
	character is in the list, increment the count for the matching node.  
	If there is no next node, create a chain with the remaining sling
	string; otherwise, move down a level and continue.  
    */

    while ((Avd_TrieNode_Char_Get (cn_p) < *ch_p) 
	&& (Avd_TrieNode_List_Get (cn_p) != NULL))
      {  
      bn_p = cn_p;
      cn_p = Avd_TrieNode_List_Get (cn_p);
      }

    if (Avd_TrieNode_Char_Get (cn_p) == *ch_p)
      {
      Avd_TrieNode_Value_Get (cn_p)++;
      ch_p++;
      if (Avd_TrieNode_Next_Get (cn_p) == NULL)
	{ 
	SSling_Chain_Create (ch_p, cn_p);
	return;
	}

      else 
	{  
	un_p = cn_p;
	cn_p = Avd_TrieNode_Next_Get (cn_p); 
	bn_p = cn_p;
	}
      } 

    /*  Character was not in list:  So if it at the end of list, append 
	new node;  if it at the beginning of list, prepend it; otherwise, 
	insert the new node between backnode and current node.  
    */
    else                                     
      {
      if (*ch_p > Avd_TrieNode_Char_Get (cn_p))         
	{
	Avd_TrieNode_List_Put (cn_p, 
	  STrie_Node_Create (*ch_p, AVD_SLG_NODE_INIT_VAL, NULL));
	cn_p = Avd_TrieNode_List_Get (cn_p);
	}

      else
	{
	if (bn_p == cn_p)     
	  {
	  Avd_TrieNode_Next_Put (un_p,
	    STrie_Node_Create (*ch_p, AVD_SLG_NODE_INIT_VAL, cn_p));
	  cn_p = Avd_TrieNode_Next_Get (un_p);
	  }
	else
	  {
	  Avd_TrieNode_List_Put (bn_p,
	    STrie_Node_Create (*ch_p, AVD_SLG_NODE_INIT_VAL, cn_p));
	  cn_p = Avd_TrieNode_List_Get (bn_p);
	  }
	}   /* End of else  prepend node. */

      ++ch_p;

      /*  We have inserted a new node, so finish building chain of new 
	  nodes.
      */

      SSling_Chain_Create (ch_p, cn_p);
      return;  
      }   /* End of else char was not in list */
    }   /*  End of while not at end of sling  */

  return;
} 
/* End of  SSling_Tree_Update */

/****************************************************************************
*
*  Function Name:                 SSlings_Compress
*
*    Compress the given sling using the pattern/code dictionary trie.
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
*    Length of the compressed sling.
*
*  Side Effects:
*
*    Allocates memory for the compressed sling--may exit on error.
*
******************************************************************************/
void SSlings_Compress
  (
  Avd_Slg_Key_t  *slings,                     /* sling list to compress */
  U32_t           num_slgs,                   /* number of slings */
  Avd_Trie_Node_t *patcdroot_p                /* patt/code dict trie */
  )
{
  Avc_Slg_Code_t  comp_buf[AVD_SLING_LENMAX]; /* temp buff for comp sling */
  Avc_Slg_Code_t *cb_p;                       /* comp sling buff ptr */
  Avc_Slg_Code_t *comp;                       /* comp sling */
  Avc_Slg_Code_t  code;                       /* code for curr slg prefix */
  U8_t           *end_p;                      /* end of curr prefix */
  U8_t           *slg_p;                      /* slg ptr */
  U32_t           slg_i;                      /* Ith sling */
  U16_t           length;                     /* length of comp sling */
  Avd_Trie_Node_t *pcn_p;                     /* patt/code trie ptr */

  for (slg_i = 0; slg_i < num_slgs; slg_i++)
    {
    cb_p = comp_buf;
    slg_p = Avd_SlgKey_Sling_Get (slings[slg_i]);
    end_p = slg_p;
    pcn_p = patcdroot_p;

    while (*slg_p)
      {
      /*  Search through trie list for current char in sling. */

      while ((Avd_TrieNode_Char_Get (pcn_p) != *slg_p) 
	  && (Avd_TrieNode_List_Get (pcn_p) != NULL))
	pcn_p = Avd_TrieNode_List_Get (pcn_p);

      /*  If the char was in the list, check to see if there is a code
	  associated with this substring; if we are not at end of a pattern
	  and not at the end of the sling, continue matching chars.  
      */

      if (Avd_TrieNode_Char_Get (pcn_p) == *slg_p)
	{
	slg_p++;
	if (Avd_TrieNode_Value_Get (pcn_p) != AVC_NULL_CODE)
	  {
	  end_p = slg_p;
	  code = (Avc_Slg_Code_t) Avd_TrieNode_Value_Get (pcn_p);
	  }
	    
	if ((Avd_TrieNode_Next_Get (pcn_p) != NULL) && (*slg_p))
	  {
	  pcn_p = Avd_TrieNode_Next_Get (pcn_p);
	  continue;
	  }
	}  /* End of if char was in list  */

      /*  Otherwise, the char was not in list, or we are at the end of the
	  subpattern, so store the current best code and start finding new a
	  subpattern.  
      */
	 
      *(cb_p++) = code;
      slg_p = end_p;
      pcn_p = patcdroot_p;
      }  /*  End of while not at end of sling  */
      
    /*  Copy the compressed sling into the sling structure. */ 

    *cb_p = AVC_NULL_CODE;
    length = (U16_t) (cb_p - comp_buf);
    comp = (Avc_Slg_Code_t *) malloc (length + AVC_TBL_CODE_SZ);
    if (comp == NULL)
      {
      fprintf (stderr, "SSling_Compress:  memory allocation error.");
      exit (-1);
      }

    memcpy (comp, comp_buf, length + AVC_TBL_CODE_SZ);
    Avd_SlgKey_CSlg_Put (slings[slg_i], comp);
    Avd_SlgKey_CLen_Put (slings[slg_i], length);
    }  /* End of for each sling */

  return;
}
/*  End of SSlings_Compress  */

/****************************************************************************
*
*  Function Name:                 SSlings_Get
*
*    Verifies the version number of the Isam sling info file.
*    Reads in the slings from the Isam sling info file into the sling list.
*    Initializes the char_exists vector based on the characters found in 
*    the slings.
*
*  Implicit Inputs:
*
*    SDictControl:
*      - info_b
*
*  Implicit Outputs:
*
*    SDictControl:
*      - info_b
*      - infozrec_b
*
*  Return Values:
*
*    Number of slings read in.
*
*  Side Effects:
*
*    Allocates memory for the slings:  may exit on error.
*    Closes the Isam sling info file.
*
******************************************************************************/
U32_t SSlings_Get
  (
  Avd_Slg_Key_t   slings[],                   /* list of slings */
  Boolean_t       exists[]                    /* char vector */ 
  )
{

  U32_t           slg_count;                  /* number of slings in file */
  U32_t           valid_slg_count;            /* number of slings in file */
  U32_t           slg_i;                      /* Ith sling */
  Avi_CmpHdr_t   *sh_p;                       /* comp info record header */
/**/U8_t          buff[AVI_INFOREC_LENMAX];   /* comp info record buffer */
  U8_t           *b_p;                        /* buffer ptr */
  U8_t           *slg_p;                      /* sling ptr */

  /*  Read in zero record of Isam sling info file.  Verify version
      number and get number of slings.  Read in the slings.
  */

  Isam_Read_Nobuffer (&Avd_DictCtrl_InfoFC_Get (SDictControl), 0, 
    &Avd_DictCtrl_InfoZR_Get (SDictControl), AVC_RECZERO_SZ);

  if (Avc_RecZero_VerNum_Get (Avd_DictCtrl_InfoZR_Get (SDictControl))
      != AVI_VERSION_ISAMINFO)
    {
    fprintf (stderr, "\nSSlings_Get:  incorrect Isam info file version.\n");
    exit (-1);
    }

  slg_count = Avc_RecZero_CntSz_Get (Avd_DictCtrl_InfoZR_Get (SDictControl));

/**/sh_p = (Avi_CmpHdr_t *) buff;
  for (slg_i = 1, valid_slg_count = 0; slg_i <= slg_count; slg_i++)
    {
/**/Isam_Read_Nobuffer (&Avd_DictCtrl_InfoFC_Get (SDictControl), slg_i, 
/**/buff, AVI_INFOREC_LENMAX);

    if (Avi_CmpHdr_Avail_Get (sh_p) == AVI_AVAIL_JUNK_CD) continue;

/*  sh_p = (Avi_CmpHdr_t *) Isam_Read (&Avd_DictCtrl_InfoFC_Get
      (SDictControl), slg_i);  */
    b_p = (U8_t *) (sh_p + 1);
    slg_p = (U8_t *) malloc (Avi_CmpHdr_SlgLen_Get (sh_p) + 1);
    if (slg_p == NULL)
      {
      fprintf (stderr, "\nSSlings_Get:  memory allocation error.\n");
      exit (-1);
      }
    memcpy (slg_p, b_p, Avi_CmpHdr_SlgLen_Get (sh_p));
    *(slg_p + Avi_CmpHdr_SlgLen_Get (sh_p)) = AVD_EOS;
/*
    Avd_SlgKey_Sling_Put (slings[slg_i-1], slg_p);
    Avd_SlgKey_Key_Put (slings[slg_i-1], slg_i);
*/
    Avd_SlgKey_Sling_Put (slings[valid_slg_count], slg_p);
    Avd_SlgKey_Key_Put (slings[valid_slg_count], slg_i);

    valid_slg_count++;

    /*  Mark characters in sling as seen. */

    while (*slg_p)
      exists[*(slg_p++)] = TRUE; 
    }

  Isam_Close (&Avd_DictCtrl_InfoFC_Get (SDictControl));
  return valid_slg_count;
} 
/* End of SSlings_Get  */

/****************************************************************************
*
*  Function Name:                 SSling_Subtrie_Merge
*
*    Merges the guest subtrie into the host subtrie.
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
*    
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
void SSling_Subtrie_Merge
  (
  Avd_Trie_Node_t *cn_p,                      /* root of host subtrie */
  Avd_Trie_Node_t *bn_p,                      /* back node of host */
  Avd_Trie_Node_t *un_p,                      /* up node of host */
  Avd_Trie_Node_t *stn_p                      /* root of guest subtree */
  ) 
{

  /*  Find the char of guest subtree in the host subtrie list. */

  while ((Avd_TrieNode_Char_Get (cn_p) < Avd_TrieNode_Char_Get (stn_p)) 
      && (Avd_TrieNode_List_Get (cn_p) != NULL))
    {  
    bn_p = cn_p;
    cn_p = Avd_TrieNode_List_Get (cn_p);
    }

  /*  If the char was in the list, update count and continue down both 
      tries.
  */

  if (Avd_TrieNode_Char_Get (cn_p) == Avd_TrieNode_Char_Get (stn_p))
    {
    Avd_TrieNode_Value_Get (cn_p) += Avd_TrieNode_Value_Get (stn_p);

    /*  First, process the rest of the lists:  either append the nonempty
	guest subtrie to end of the host subtrie list, or if the guest
	subtrie is not empty, merge the rest of guest subtrie list with the 
	host subtrie.  
    */
    if (Avd_TrieNode_List_Get (cn_p) == NULL)
      { 
      if (Avd_TrieNode_List_Get (stn_p) != NULL)
	Avd_TrieNode_List_Put (cn_p, Avd_TrieNode_List_Get (stn_p));
      }

    else
      {
      if (Avd_TrieNode_List_Get (stn_p) != NULL)
	SSling_Subtrie_Merge (Avd_TrieNode_List_Get (cn_p), cn_p, 
	  NULL, Avd_TrieNode_List_Get (stn_p));
      }

    /*  Second, process the next char in the guest subtrie:  either connect 
	the nonempty guest subtrie to the end of the chain of the host 
	subtrie, or if the guest subtree is not empty, merge the rest of the
	guest subtre chain with the host subtrie.  
    */

    if (Avd_TrieNode_Next_Get (cn_p) == NULL)
      {
      if (Avd_TrieNode_Next_Get (stn_p) != NULL)
	Avd_TrieNode_Next_Put (cn_p, Avd_TrieNode_Next_Get (stn_p));
      }

    else
      {
      if (Avd_TrieNode_Next_Get (stn_p) != NULL)
       SSling_Subtrie_Merge (Avd_TrieNode_Next_Get (cn_p), 
	 Avd_TrieNode_Next_Get (cn_p), cn_p, Avd_TrieNode_Next_Get (stn_p));
      }
   
    free (stn_p);
    return;
    }  /* End of if guest char was in the host list */
  else
    {
    /*  The guest char was not in the host list: so either append the 
	guest subtrie, prepend the guest subtrie, or insert it between 
	the backnode and the current node.
    */

    if (Avd_TrieNode_Char_Get (stn_p) > Avd_TrieNode_Char_Get (cn_p))
      {
      Avd_TrieNode_List_Put (cn_p, stn_p);
      return;
      }
    else
      {
      if (bn_p == cn_p)
	{
	Avd_TrieNode_Next_Put (un_p, stn_p);
	bn_p = stn_p;
	stn_p = Avd_TrieNode_List_Get (stn_p);
	Avd_TrieNode_List_Put (bn_p, cn_p);
	if (stn_p != NULL)
	  SSling_Subtrie_Merge (cn_p, bn_p, NULL, stn_p);

	return;
	}
      else
	{
	Avd_TrieNode_List_Put (bn_p, stn_p);
	bn_p = stn_p;
	stn_p = Avd_TrieNode_List_Get (stn_p);
	Avd_TrieNode_List_Put (bn_p, cn_p);
	if (stn_p != NULL)
	  SSling_Subtrie_Merge (cn_p, bn_p, NULL, stn_p);

	return;
	}

      }   /* End of else not at end of list */
    }   /* End of else char was not in list */
}
/* End of SSling_Subtrie_Merge */

/****************************************************************************
*
*  Function Name:                 STabletops_Pop
*
*    Removes the top level of the given trie (when no more candidates
*    can be found in the trie, this alters the trie as if a single 
*    character was matched).
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
*    TRUE, if the trie was altered (there were nodes with a non-zero
*    count to remove);  FALSE, otherwise.
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
Boolean_t STabletops_Pop
  (
  Avd_Trie_Node_t *slgroot_p                  /* root of trie to pop */
  )
{
  Avd_Trie_Node_t *cn_p;                      /* current node of trie */
  Avd_Trie_Node_t *bn_p;                      /* back node of trie */
  Avd_Trie_Node_t *toplist_p;                 /* top list (level) of trie */

  /*  If the trie is empty, return FALSE */

  if ((slgroot_p == NULL) || (Avd_TrieNode_List_Get (slgroot_p) == NULL))
    return FALSE;

  /*  Disconnect the top list (level) from the slgroot_p. */

  cn_p = Avd_TrieNode_List_Get (slgroot_p);
  bn_p = cn_p;
  Avd_TrieNode_List_Put (slgroot_p, NULL);

  /*  Find the first node in the top list with a nonempty subtree 
      below it.  Free the empty subtries.
  */

  while ((cn_p != NULL) && (Avd_TrieNode_Next_Get (cn_p) == NULL))
    {
    cn_p = Avd_TrieNode_List_Get (cn_p);
    free (bn_p);
    bn_p = cn_p;
    }

  /*  If there are no non-empty subtries, return FALSE. */

  if (cn_p == NULL)                        
    return FALSE; 

  /*  Otherwise, make this subtrie the new root of the trie and disconnect 
      it from the list of old top nodes.  
  */
  Avd_TrieNode_List_Put (slgroot_p, Avd_TrieNode_Next_Get (cn_p));

  /*  Mark the toplist of the tree as changed first. */

  toplist_p = Avd_TrieNode_Next_Get (cn_p);
  while (toplist_p != NULL)
    {
    Avd_PreTbl_Change_Put (Avd_DictCtrl_TblTops_Get (SDictControl)
      [Avd_TrieNode_Char_Get (toplist_p)], TRUE);
    toplist_p = Avd_TrieNode_List_Get (toplist_p);
    }

  Avd_TrieNode_Next_Put (cn_p, NULL);
  cn_p = Avd_TrieNode_List_Get (cn_p);
  free (bn_p);
  bn_p = cn_p;

  /*  Build up the new prefix tree using the popped subtrees. */

  while (cn_p != NULL)
    {
    if (cn_p->next != NULL)
      {

      /*  Mark the toplist of the subtree as changed first. */

      toplist_p = Avd_TrieNode_Next_Get (cn_p);
      while (toplist_p != NULL)
	{
	Avd_PreTbl_Change_Put (Avd_DictCtrl_TblTops_Get (SDictControl)
	 [Avd_TrieNode_Char_Get (toplist_p)], TRUE);
	toplist_p = Avd_TrieNode_List_Get (toplist_p);
	}

      SSling_Subtrie_Merge (Avd_TrieNode_List_Get (slgroot_p), 
	slgroot_p, NULL, Avd_TrieNode_Next_Get (cn_p));
      }

    cn_p = Avd_TrieNode_List_Get (cn_p);
    free (bn_p);
    bn_p = cn_p;
    }

  return TRUE;
}
/* End of STabletops_Pop */

/****************************************************************************
*
*  Function Name:                 STrie_Node_Create
*
*    Create a new node in the pattern tree.
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
*    Pointer to newly created node.
*
*  Side Effects:
*
*    Allocates memory for the new node:  may exit on error.
*
******************************************************************************/
Avd_Trie_Node_t *STrie_Node_Create
  (
  U8_t            ch,                         /* initial char value */
  S32_t           value,                      /* initial value value */
  Avd_Trie_Node_t *list                       /* initial list ptr value */ 
  )
{
  Avd_Trie_Node_t *new_p;                     /* the new node */

  new_p = (Avd_Trie_Node_t *) malloc (AVD_TRIE_NODE_SZ);
  if (new_p == NULL) 
    {
    fprintf (stderr, "STrie_Node_Create:  memory allocation error.\n");
    exit (-1);
    }

  Avd_TrieNode_Char_Put (new_p, ch);
  Avd_TrieNode_Value_Put (new_p, value);
  Avd_TrieNode_List_Put (new_p, list);
  Avd_TrieNode_Next_Put (new_p, NULL);

  return new_p;
} 
/* End of  STrie_Node_Create */

/****************************************************************************
*
*  Function Name:                  STrie_Tree_Destroy
*
*    Traverse the trie freeing the memory allocated to the nodes.
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
*     None.
*
*  Side Effects:
*
*    Frees memory allocated for the trie nodes.
*
******************************************************************************/
void STrie_Tree_Destroy
  (  
  Avd_Trie_Node_t *pn_p                       /* trie to destroy */
  )
{
  if (pn_p == NULL)
    return;

  STrie_Tree_Destroy (Avd_TrieNode_Next_Get (pn_p));
  STrie_Tree_Destroy (Avd_TrieNode_List_Get (pn_p));
  free (pn_p);

  return;
} 
/* End of STrie_Tree_Destroy  */
/* End of AVLDICT.C */
