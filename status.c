/******************************************************************************
*
*  Copyright (C) 1995-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     STATUS.C
*
*    The layout of the status file is:
*
*    Rcb
*    Run Status
*    PstCB
*    Subgoal Nodes
*    Compound Nodes
*    Symbol Table entries
*    Slings in the symbol Table
*
*  Creation Date:
*
*    18-Jul-95
*
*  Routines:
*
*    Status_File_Peek
*    Status_File_Read    
*    Status_File_Write
*
*  Static Routines:
*
*    Status_Byte_Read
*    Status_FCBs_Read
*    Status_FCBs_Write
*    Status_Pst_Modify
*    Status_Pst_Construct
*
*  Authors:
*
*    Daren Krebsbach
*    Shu Cheung
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 22-Jun-98  Krebsbach  Nondestructive saves of PST.
* 26-Sep-96  Krebsbach  No longer store shelved merits in status file.
* 26-Sep-96  Krebsbach  Modified Status_File_Read and Status_File_Write
*                       to read and write Run Status from/to status file.
* 16-Mar-01  Miller     Removed static attribute from Status_Pst_Serialize,
*                       so that other modules don't have to use convoluted
*                       and incomplete ways to associate a compound index
*                       with a pointer!
*
******************************************************************************/
 
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>
#include <errno.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_TIMER_
#include "timer.h"
#endif

#ifndef _H_PST_
#include "pst.h"
#endif
 
#ifndef _H_RCB_
#include "rcb.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_STATUS_
#include "status.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

/* static routines prototypes */

static U8_t *Status_Byte_Read       (U32_t, FILE *);
static void  Status_FCBs_Read       (Rcb_t *, FILE *);
static void  Status_FCBs_Write      (Rcb_t *, FILE *);
/* static */ void  Status_Pst_Serialize   (Array_t *, Array_t *, PstNode_t); /* should NOT be static! */
static void  Status_Pst_Construct   (Array_t *, Array_t *, Array_t *, 
               U8_t *, PstCB_t *);

/****************************************************************************
*
*  Function Name:                 Status_File_Peek
*
*    This function only reads in the run control block (and associated 
*    strings) and the run statistics (and strings).
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
*    May exit program
*
******************************************************************************/
void Status_File_Peek
  (
  char          *filename,              /* status file name */
  Rcb_t         *rcb_p,                 /* run control block */
  RunStats_t    *rstat_p                /* run statistics */
  )
{
  String_t       goal_sling;
  FILE          *f;
  U8_t          *string_p;
  U8_t          *tempstr_p;
  U32_t          size;
  size_t         length;

#ifdef _WIN32
  f = fopen (gccfix (filename), "rb");
#else
  f = fopen (filename, "rb");
#endif
  if (f == NULL)
    {
    perror ("Failed to open status file");
    exit (-1);
    } 

  /* read in the content of Rcb */

  if (fread (rcb_p, 1, RCBSIZE, f) != RCBSIZE)
    {
    perror ("Failed to read status file"); 
    exit (-1); 
    } 

  /*  Extract the strings from the Rcb, and then for the file
      control blocks.
  */
  size = Sling_Length_Get (Rcb_Goal_Get (rcb_p)) + 1 +
         String_Length_Get (Rcb_User_Get (rcb_p)) + 1 +
         String_Length_Get (Rcb_Name_Get (rcb_p)) + 1 +
         String_Length_Get (Rcb_Comment_Get (rcb_p)) + 1;

  string_p = Status_Byte_Read (size, f);

  tempstr_p = string_p;
  length = Sling_Length_Get (Rcb_Goal_Get (rcb_p));
  goal_sling = String_Create ((char *) tempstr_p, length);
  Rcb_Goal_Put (rcb_p, String2Sling (goal_sling));
  String_Destroy (goal_sling);

  tempstr_p += length + 1;
  length = String_Length_Get (Rcb_User_Get (rcb_p));
  Rcb_User_Put (rcb_p, String_Create ((char *) tempstr_p, length));

  tempstr_p += length + 1;
  length = String_Length_Get (Rcb_Name_Get (rcb_p));
  Rcb_Name_Put (rcb_p, String_Create ((char *) tempstr_p, length));

  tempstr_p += length + 1;
  length = String_Length_Get (Rcb_Comment_Get (rcb_p));
  Rcb_Comment_Put (rcb_p, String_Create ((char *) tempstr_p, length));

#ifdef _MIND_MEM_
  mind_free ("string_p", "status", string_p);
#else
  Mem_Dealloc (string_p, size, GLOBAL);
#endif

  /*  Read in the file and directory names.  */
  Status_FCBs_Read (rcb_p, f);

  /*  Read in the content of Run Status and the exit condition string.  */
  if (fread (rstat_p, 1, RUNSTATS_SIZE, f) != RUNSTATS_SIZE)
    {
    perror ("Failed to read status file\n");
    exit (-1);
    }
 
  size = String_Length_Get (RunStats_ExitCond_Get (rstat_p));
  string_p = Status_Byte_Read (size + 1, f);
  RunStats_ExitCond_Put (rstat_p, 
    String_Create ((char *) string_p, (U16_t) size));
#ifdef _MIND_MEM_
  mind_free ("string_p", "status", string_p);
#else
  Mem_Dealloc (string_p, size + 1, GLOBAL);
#endif

  if (fclose (f) == EOF)
    {
    perror ("Failed to close status file\n");
    exit (-1);
    }

  return;
}
/* End of Status_File_Peek */

/****************************************************************************
*
*  Function Name:                 Status_File_Read
*
*    This function reads in a status file, called filename.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    PstCB is initialized.
*
*  Return Values:
*
*    N/A
*
*  Side Effects:
*
*    May exit program
*
******************************************************************************/
void Status_File_Read
  (
  char          *filename,              /* status file name */
  Rcb_t         *rcb_p,                 /* run control block */
  RunStats_t    *rstat_p                /* run status */
  )
{
  Array_t       compounds;
  Array_t       subgoals;
  Array_t       symtabs;
  String_t      goal_sling;
  FILE         *f;
  Compound_t   *newcomp_p;
  PstCB_t      *pstcb_p;
  Subgoal_t    *newsubg_p;
  SymTab_t     *newsymtab_p;
  SymTab_t     *symtab_p;
  U8_t         *temp_p;
  U8_t         *buf_p;
  U8_t         *string_p;
  U8_t         *tempstr_p;
  size_t        length;
  U32_t         i;
  U32_t         position;
  U32_t         size;
  U32_t         orig_schema, new_schema;
  Boolean_t     is_temp;
  char          date_str[32],
                day_of_week[8],
                month_name[8],
               *sts,
               *tmp;
  Date_t        run_date;
  int           day,
                hour,
                minute,
                second,
                year,
                month,
                num1,
                num2;
  long          run_time;
  static char  *month_string = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";
  static U16_t  SMonths[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334 };
  static Boolean_t
                save_rxnlib_state = TRUE;

#ifdef _WIN32  
  f = fopen (gccfix (filename), "rb");
#else
  f = fopen (filename, "rb");
#endif
  if (f == NULL)
    {
    perror ("Failed to open status file");
    exit (-1);
    } 

  /* Read in the content of Rcb. */

  if (fread (rcb_p, 1, RCBSIZE, f) != RCBSIZE)
    {
    perror ("Failed to read status file"); 
    exit (-1); 
    } 

  /*  Extract the strings from the Rcb, and then for the file
      control blocks.
  */
  strcpy (date_str, ctime (&((Rcb_Date_Get (rcb_p)).tv_sec)));
  sscanf (date_str, "%s %s %d %d:%d:%d %d", day_of_week, month_name, &day, &hour, &minute, &second, &year);

  tmp = strstr (month_string, month_name);
  sts = month_string;
  num1 = day + SMonths[(tmp - sts) / 4];
  num2 = (year - 1900) * 365 + (year - 1900) / 4;  /* Hack */
  run_date = num2 + num1;
  run_time = 3600 * hour + 60 * minute + second;

  size = Sling_Length_Get (Rcb_Goal_Get (rcb_p)) + 1 +
     String_Length_Get (Rcb_User_Get (rcb_p)) + 1 +
      String_Length_Get (Rcb_Name_Get (rcb_p)) + 1 +
      String_Length_Get (Rcb_Comment_Get (rcb_p)) + 1;
  string_p = Status_Byte_Read (size, f);
  tempstr_p = string_p;
  length = Sling_Length_Get (Rcb_Goal_Get (rcb_p));
  goal_sling = String_Create ((char *) tempstr_p, length);
  Rcb_Goal_Put (rcb_p, String2Sling (goal_sling));
  String_Destroy (goal_sling);

  tempstr_p += length + 1;
  length = String_Length_Get (Rcb_User_Get (rcb_p));
  Rcb_User_Put (rcb_p, String_Create ((char *) tempstr_p, length));

  tempstr_p += length + 1;
  length = String_Length_Get (Rcb_Name_Get (rcb_p));
  Rcb_Name_Put (rcb_p, String_Create ((char *) tempstr_p, length));

  tempstr_p += length + 1;
  length = String_Length_Get (Rcb_Comment_Get (rcb_p));
  Rcb_Comment_Put (rcb_p, String_Create ((char *) tempstr_p, length));

#ifdef _MIND_MEM_
  mind_free ("string_p", "status", string_p);
#else
  Mem_Dealloc (string_p, size, GLOBAL);
#endif

  /*  Read in the file and directory names.  */
  Status_FCBs_Read (rcb_p, f);

  /*  Read in the content of Run Status and the exit condition string.  */
  if (fread (rstat_p, 1, RUNSTATS_SIZE, f) != RUNSTATS_SIZE)
    {
    perror ("Failed to read status file\n");
    exit (-1);
    }
 
  size = String_Length_Get (RunStats_ExitCond_Get (rstat_p));
  string_p = Status_Byte_Read (size + 1, f);
  RunStats_ExitCond_Put (rstat_p, 
    String_Create ((char *) string_p, (U16_t) size));
#ifdef _MIND_MEM_
  mind_free ("string_p", "status", string_p);
#else
  Mem_Dealloc (string_p, size + 1, GLOBAL);
#endif

  /*  Read in the contents of the Pst control block.  */

  pstcb_p = Pst_ControlHandle_Get ();
  if (fread (pstcb_p, 1, PSTCBSIZE, f) != PSTCBSIZE)
    {
    perror ("Failed to read status file\n");
    exit (-1);
    }

    if (save_rxnlib_state)
{
glob_run_date=run_date;
glob_run_time=run_time;
}
  /* read in all the subgoals and allocate memory for them */
/*
  React_Initialize (String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);
*/
  React_Force_Initialize (String_Value_Get (FileCB_DirStr_Get (
    Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS))), TRUE);

  buf_p = Status_Byte_Read (
    (PstCB_SubgoalIndex_Get (pstcb_p) - 1) * PSTSUBGOALSIZE, f); 
  temp_p = buf_p;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&subgoals", "status{1}", &subgoals, PstCB_SubgoalIndex_Get (pstcb_p) - 1, ADDRSIZE);
  for (i = 0; i < PstCB_SubgoalIndex_Get (pstcb_p) - 1; ++i)
    {
    mind_malloc ("newsubg_p", "status{1}", &newsubg_p, PSTSUBGOALSIZE);
#else
  Array_1d_Create (&subgoals, PstCB_SubgoalIndex_Get (pstcb_p) - 1, ADDRSIZE);
  for (i = 0; i < PstCB_SubgoalIndex_Get (pstcb_p) - 1; ++i)
    {
    Mem_Alloc (Subgoal_t  *, newsubg_p, PSTSUBGOALSIZE, GLOBAL);
#endif
    memcpy (newsubg_p, buf_p, PSTSUBGOALSIZE);
    orig_schema = PstSubg_Reaction_Schema_Get (newsubg_p);
    if (save_rxnlib_state)
      new_schema = Persist_Current_Rec (PER_STD, orig_schema,
      run_date, run_time, &is_temp);
    else
      new_schema = Persist_Current_Rec (PER_STD, orig_schema,
      PER_TIME_ANY, PER_TIME_ANY, &is_temp);
    if (new_schema == PER_DELETED)
      React_Check_Alloc (orig_schema);
    else
      {
      React_Check_Alloc (new_schema);
      PstSubg_Reaction_Schema_Put (newsubg_p, new_schema);
      }
    Array_1dAddr_Put (&subgoals, i, newsubg_p);
    buf_p += PSTSUBGOALSIZE;
    }      

#ifdef _MIND_MEM_
  mind_free ("temp_p", "status", temp_p);
#else
  Mem_Dealloc (temp_p, (PstCB_SubgoalIndex_Get (pstcb_p) - 1) *
    PSTSUBGOALSIZE, GLOBAL);
#endif


  /* read in all the compounds and allocate memory for them */

  buf_p = Status_Byte_Read (
    (PstCB_CompoundIndex_Get (pstcb_p) - 1) * PSTCOMPOUNDSIZE, f); 
  temp_p = buf_p;
      
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&compounds", "status{1}", &compounds, PstCB_CompoundIndex_Get (pstcb_p) - 1, ADDRSIZE);
  for (i = 0; i < PstCB_CompoundIndex_Get (pstcb_p) - 1; ++i)
    {
    mind_malloc ("newcomp_p", "status{1}", &newcomp_p, PSTCOMPOUNDSIZE);
#else
  Array_1d_Create (&compounds, PstCB_CompoundIndex_Get (pstcb_p) - 1, ADDRSIZE);
  for (i = 0; i < PstCB_CompoundIndex_Get (pstcb_p) - 1; ++i)
    {
    Mem_Alloc (Compound_t *, newcomp_p, PSTCOMPOUNDSIZE, GLOBAL);
#endif
    memcpy (newcomp_p, buf_p, PSTCOMPOUNDSIZE);
    Array_1dAddr_Put (&compounds, i, newcomp_p);
    buf_p += PSTCOMPOUNDSIZE;
    }

#ifdef _MIND_MEM_
  mind_free ("temp_p", "status", temp_p);
#else
  Mem_Dealloc (temp_p, (PstCB_CompoundIndex_Get (pstcb_p) - 1) * 
    PSTCOMPOUNDSIZE, GLOBAL);
#endif
 

  /* read in all the symbol table entries and allocate memory for them*/

  buf_p = Status_Byte_Read (
    (PstCB_SymtabIndex_Get (pstcb_p) - 1) * SYMTABSIZE, f); 
  temp_p = buf_p;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&symtabs", "status{1}", &symtabs, PstCB_SymtabIndex_Get (pstcb_p) - 1, ADDRSIZE);
  for (i = 0; i < PstCB_SymtabIndex_Get (pstcb_p) - 1; ++i)
    {
    mind_malloc ("newsymtab_p", "status{1}", &newsymtab_p, SYMTABSIZE);
#else
  Array_1d_Create (&symtabs, PstCB_SymtabIndex_Get (pstcb_p) - 1, ADDRSIZE);
  for (i = 0; i < PstCB_SymtabIndex_Get (pstcb_p) - 1; ++i)
    {
    Mem_Alloc (SymTab_t *, newsymtab_p, SYMTABSIZE, GLOBAL);
#endif
    memcpy (newsymtab_p, buf_p, SYMTABSIZE);
    Array_1dAddr_Put (&symtabs, i, newsymtab_p);
    buf_p += SYMTABSIZE;
    }
    
#ifdef _MIND_MEM_
  mind_free ("temp_p", "status", temp_p);
#else
  Mem_Dealloc (temp_p, (PstCB_SymtabIndex_Get (pstcb_p) - 1) *
    SYMTABSIZE, GLOBAL);
#endif

  /* construct the symbol table */

  for (i = 0; i < MX_SYMTAB_BUCKETS; ++i)
    SymTab_HashBucketHead_Put (i, NULL);

  for (i = 0, size = 0; i < PstCB_SymtabIndex_Get (pstcb_p) - 1; ++i)
    {
    newsymtab_p = (SymTab_t *) Array_1dAddr_Get (&symtabs, i);
    size += Sling_Length_Get (SymTab_Sling_Get (newsymtab_p));
 

    position = SymTab_HashChainIndex_Get (newsymtab_p);
    SymTab_HashChain_Put (newsymtab_p, NULL);
    symtab_p = SymTab_HashBucketHead_Get (position);
    if (symtab_p == NULL)
      SymTab_HashBucketHead_Put (position, newsymtab_p);
    else
      {
      while (SymTab_HashChain_Get (symtab_p) != NULL)
        symtab_p = SymTab_HashChain_Get (symtab_p);
    
      SymTab_HashChain_Put (symtab_p, newsymtab_p);
      }
    }
     
  /* read in all the slings in the symbol table */

  string_p = Status_Byte_Read (size, f);

  Status_Pst_Construct (&subgoals, &compounds, &symtabs, string_p, 
    pstcb_p);

#ifdef _MIND_MEM_
  mind_free ("string_p", "status", string_p);
  mind_Array_Destroy ("&subgoals", "status", &subgoals);
  mind_Array_Destroy ("&compounds", "status", &compounds);
  mind_Array_Destroy ("&symtabs", "status", &symtabs);
#else
  Mem_Dealloc (string_p, size, GLOBAL);
  Array_Destroy (&subgoals);
  Array_Destroy (&compounds);
  Array_Destroy (&symtabs);
#endif

  if (fclose (f) == EOF)
    {
    perror ("Failed to close status file\n");
    exit (-1);
    }
  
  return;
}
/* End of Status_File_Read */


/****************************************************************************
*
*  Function Name:                 Status_File_Write
*
*    This routine creates a status file which contains the whole Pst, and the
*    data needed to restart the search.
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
*    May call exit ()
*
******************************************************************************/

void Status_File_Write
  (
  char          *filename,
  Rcb_t         *rcb_p,                 /* run control block */
  RunStats_t    *rstat_p
  )
{
  Array_t        subgoal_list;
  Array_t        compound_list;
  Array_t        symtab_list;
  Array_t        symtab_bucket;
  PstNode_t      node;
  char          *output_p;
  char          *temp_p;
  FILE          *f;
  PstCB_t       *pstcb_p;
  SymTab_t      *symtab_p;
  PstCB_t        pstcb_copy;
  SymTab_t       symtab_copy;
  U32_t          i;
  U32_t          size;
  U16_t          length;

  /*  Open the status to write.  */
#ifdef _WIN32  
  f = fopen (gccfix (filename), "wb");
#else
  f = fopen (filename, "wb");
#endif
  if (f == NULL) if (write_fail_fatal)
    {
    perror ("Failed to create status file");
    exit (-1);
    } 
  else
    {
    sprintf (write_fail_str, "Failed to create status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
  
  /*  Store the contents of the Rcb.  */
  output_p = (char *)rcb_p;
  size = RCBSIZE;
  
  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    {
    perror ("Failed to write rcb to status file"); 
    exit (-1); 
    } 
  else
    {
    sprintf (write_fail_str, "Failed to write rcb to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
  
  output_p = (char *)Sling_Name_Get (Rcb_Goal_Get (rcb_p));
  size = Sling_Length_Get (Rcb_Goal_Get (rcb_p)) + 1;

  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    { 
    perror ("Failed to write sling to status file");  
    exit (-1);  
    }
  else
    {
    sprintf (write_fail_str, "Failed to write sling to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }

  output_p = (char *)String_Value_Get (Rcb_User_Get (rcb_p));
  size = String_Length_Get (Rcb_User_Get (rcb_p)) + 1;

  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    { 
    perror ("Failed to write user to status file");  
    exit (-1);  
    }
  else
    {
    sprintf (write_fail_str, "Failed to write user to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
  
  output_p = (char *)String_Value_Get (Rcb_Name_Get (rcb_p));
  size = String_Length_Get (Rcb_Name_Get (rcb_p)) + 1;
 
  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    { 
    perror ("Failed to write name to status file");  
    exit (-1);  
    }
  else
    {
    sprintf (write_fail_str, "Failed to write name to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
  
  output_p = (char *)String_Value_Get (Rcb_Comment_Get (rcb_p));
  size = String_Length_Get (Rcb_Comment_Get (rcb_p)) + 1;
 
  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    { 
    perror ("Failed to write comment to status file");  
    exit (-1);  
    }
  else
    {
    sprintf (write_fail_str, "Failed to write comment to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
  
  /*  Store the file and directory names.  */
  Status_FCBs_Write (rcb_p, f);

  /*  Store the content of the Run Status and the exit condition string.  */
  if (fwrite (rstat_p, 1, RUNSTATS_SIZE, f) != RUNSTATS_SIZE) if (write_fail_fatal)
    {
    perror ("Failed to write run status to status file");
    exit (-1);
    }
  else
    {
    sprintf (write_fail_str, "Failed to write run status to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
 
  output_p = (char *) String_Value_Get (RunStats_ExitCond_Get (rstat_p));
  size = String_Length_Get (RunStats_ExitCond_Get (rstat_p)) + 1;
 
  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    { 
    perror ("Failed to write exit condition to status file");  
    exit (-1);  
    }
  else
    {
    sprintf (write_fail_str, "Failed to write exit condition to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }

  /*  Store the content of the Pst control block.  */
  pstcb_p = Pst_ControlHandle_Get ();
  PstNode_Subgoal_Put (&node, PstCB_Root_Get (pstcb_p));

  pstcb_copy = *pstcb_p;

  PstCB_RootIndex_Put (&pstcb_copy, 
    PstSubg_Index_Get (PstCB_Root_Get (pstcb_p)));

  /*  A NULL current compound pointer means that the search is stuck.
      Use index of 0.
  */
  if (PstCB_CurrentComp_Get (pstcb_p) == NULL)
    PstCB_CurrentCompIndex_Put (&pstcb_copy, 0);
  else
    PstCB_CurrentCompIndex_Put (&pstcb_copy, PstComp_Index_Get (
      PstCB_CurrentComp_Get (pstcb_p)));

  if (PstCB_LastExpandedCompound_Get (pstcb_p) == NULL)
    PstCB_LastExpandedCompoundIndex_Put (&pstcb_copy, 0);
  else
    PstCB_LastExpandedCompoundIndex_Put (&pstcb_copy, PstComp_Index_Get (
      PstCB_LastExpandedCompound_Get (pstcb_p)));

  if (fwrite (&pstcb_copy, 1, PSTCBSIZE, f) != PSTCBSIZE) if (write_fail_fatal)
    {
    perror ("Failed to write pstcb to status file");
    exit (-1);
    }
  else
    {
    sprintf (write_fail_str, "Failed to write pstcb to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }
  
#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&subgoal_list", "status{2}", &subgoal_list, PstCB_SubgoalIndex_Get (pstcb_p) - 1, 
     ADDRSIZE);
  mind_Array_1d_Create ("&compound_list", "status{2}", &compound_list, PstCB_CompoundIndex_Get (pstcb_p) - 1, 
    ADDRSIZE);
  mind_Array_1d_Create ("&symtab_list", "status{2}", &symtab_list, PstCB_SymtabIndex_Get (pstcb_p) - 1,
    ADDRSIZE);
  mind_Array_1d_Create ("&symtab_bucket", "status{2}", &symtab_bucket, PstCB_SymtabIndex_Get (pstcb_p) - 1,
    LONGSIZE);
#else
  Array_1d_Create (&subgoal_list, PstCB_SubgoalIndex_Get (pstcb_p) - 1, 
     ADDRSIZE);
  Array_1d_Create (&compound_list, PstCB_CompoundIndex_Get (pstcb_p) - 1, 
    ADDRSIZE);
  Array_1d_Create (&symtab_list, PstCB_SymtabIndex_Get (pstcb_p) - 1,
    ADDRSIZE);
  Array_1d_Create (&symtab_bucket, PstCB_SymtabIndex_Get (pstcb_p) - 1,
    LONGSIZE);
#endif
  Array_Set (&subgoal_list, 0);
  Array_Set (&compound_list, 0);
  Array_Set (&symtab_list, 0);
  Array_Set (&symtab_bucket, 0);

  Status_Pst_Serialize (&subgoal_list, &compound_list, node);
 
  /*  Write all the subgoal nodes to the status file.  */
  for (i = 0; i < Array_Columns_Get (&subgoal_list); ++i)
    {
    Subgoal_t        *subg_p;
    Subgoal_t         sg_copy;

    subg_p = (Subgoal_t *)Array_1d32_Get (&subgoal_list, i);
    sg_copy = *subg_p;

    if (PstSubg_Father_Get (subg_p) != NULL)
      PstSubg_FatherIndex_Put (&sg_copy, PstComp_Index_Get (
        PstSubg_Father_Get (subg_p)));

    PstSubg_SonIndex_Put (&sg_copy, PstComp_Index_Get (
      PstSubg_Son_Get (subg_p)));
   
    if (PstSubg_Brother_Get (subg_p) != NULL)
      PstSubg_BrotherIndex_Put (&sg_copy, PstSubg_Index_Get (
        PstSubg_Brother_Get (subg_p)));

    if (fwrite (&sg_copy, 1, PSTSUBGOALSIZE, f) != PSTSUBGOALSIZE) if (write_fail_fatal)
      {
      perror ("Failed to write subgoal to status file");
      exit (-1);
      }
    else
      {
      sprintf (write_fail_str, "Failed to write subgoal to status file: %s.\n(Correct error and try again.)", strerror (errno));
      return;
      }
    }

  /*  Write all the compound nodes to the status file.  */

  for (i = 0; i < Array_Columns_Get (&compound_list); ++i)
    {
    Compound_t       *comp_p;
    Compound_t        comp_copy;

    comp_p = (Compound_t *) Array_1d32_Get (&compound_list, i);
    comp_copy = *comp_p;

    PstComp_FatherIndex_Put (&comp_copy, PstSubg_Index_Get (
    PstComp_Father_Get (comp_p)));

    if (PstComp_Son_Get (comp_p) != NULL)
      PstComp_SonIndex_Put (&comp_copy, PstSubg_Index_Get (
        PstComp_Son_Get (comp_p)));

    if (PstComp_Brother_Get (comp_p) != NULL)
      PstComp_BrotherIndex_Put (&comp_copy, PstComp_Index_Get (
        PstComp_Brother_Get (comp_p)));

    if (PstComp_Prev_Get (comp_p) != NULL)
      PstComp_PrevIndex_Put (&comp_copy, PstComp_Index_Get (
        PstComp_Prev_Get (comp_p)));

    if (PstComp_Next_Get (comp_p) != NULL)
      PstComp_NextIndex_Put (&comp_copy, PstComp_Index_Get (
        PstComp_Next_Get (comp_p)));

    PstComp_SymbolTableIndex_Put (&comp_copy, SymTab_Index_Get (
      PstComp_SymbolTable_Get (comp_p)));

    if (fwrite (&comp_copy, 1, PSTCOMPOUNDSIZE, f) != PSTCOMPOUNDSIZE) if (write_fail_fatal)
       {
       perror ("Failed to write compound to status file");
       exit (-1);
       }
    else
      {
      sprintf (write_fail_str, "Failed to write compound to status file: %s.\n(Correct error and try again.)", strerror (errno));
      return;
      }
    }

  /*  Serialize the symbol table.  To ensure compatability with the
      destructive save method, we need to keep track of the bucket
      index of each symtab.  
  */

  for (i = 0, size = 0; i < SymTab_HashSize_Get (); ++i)
    for (symtab_p = SymTab_HashBucketHead_Get (i); symtab_p != NULL;
         symtab_p = SymTab_HashChain_Get (symtab_p))
      {
      size += Sling_Length_Get (SymTab_Sling_Get (symtab_p));
      Array_1d32_Put (&symtab_list, SymTab_Index_Get (symtab_p) - 1, symtab_p);
      Array_1d32_Put (&symtab_bucket, SymTab_Index_Get (symtab_p) - 1, i);
      }

#ifdef _MIND_MEM_
  mind_malloc ("output_p", "status{2}", &output_p, size);
#else
  Mem_Alloc (char *, output_p, size, GLOBAL);
#endif

  /*  Write symbol table entries to the status file.  */

  for (i = 0, temp_p = output_p; i < PstCB_SymtabIndex_Get (pstcb_p) - 1; ++i)
    {
    symtab_p = (SymTab_t *) Array_1d32_Get (&symtab_list, i);
    symtab_copy = *symtab_p;

    if (SymTab_FirstComp_Get (symtab_p) != NULL)
      SymTab_FirstCompIndex_Put (&symtab_copy, PstComp_Index_Get (
        SymTab_FirstComp_Get (symtab_p)));
      
    if (SymTab_DevelopedComp_Get (symtab_p) != NULL)
      SymTab_DevelopedCompIndex_Put (&symtab_copy, PstComp_Index_Get (
         SymTab_DevelopedComp_Get (symtab_p)));
   
    if (SymTab_Current_Get (symtab_p) != NULL)
      SymTab_CurrentIndex_Put (&symtab_copy, PstComp_Index_Get (
         SymTab_Current_Get (symtab_p)));

    SymTab_HashChainIndex_Put (&symtab_copy, 
      Array_1d32_Get (&symtab_bucket, i));

    if (fwrite (&symtab_copy, 1, SYMTABSIZE, f) != SYMTABSIZE) if (write_fail_fatal)
      {
      perror ("Failed to write symtab to status file");
      exit (-1);
      }
    else
      {
      sprintf (write_fail_str, "Failed to write symtab to status file: %s.\n(Correct error and try again.)", strerror (errno));
      return;
      }

    length = Sling_Length_Get (SymTab_Sling_Get (symtab_p));
    strncpy (temp_p, (char *) Sling_Name_Get (SymTab_Sling_Get (symtab_p)), 
      length);
    temp_p += length;
    } 

  /*  Write the slings in the symbol table to the status file.  */

  if (fwrite (output_p, 1, size, f) != size) if (write_fail_fatal)
    {
    perror ("Failed to write sling buffer to status file");
    exit (-1);
    }
  else
    {
    sprintf (write_fail_str, "Failed to write sling buffer to status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }

#ifdef _MIND_MEM_
  mind_Array_Destroy ("&subgoal_list", "status", &subgoal_list);
  mind_Array_Destroy ("&compound_list", "status", &compound_list);
  mind_Array_Destroy ("&symtab_list", "status", &symtab_list);
  mind_Array_Destroy ("&symtab_bucket", "status", &symtab_bucket);
  mind_free ("output_p", "status", output_p);
#else
  Array_Destroy (&subgoal_list);
  Array_Destroy (&compound_list);
  Array_Destroy (&symtab_list);
  Array_Destroy (&symtab_bucket);
  Mem_Dealloc (output_p, size, GLOBAL);
#endif

  if (fclose (f) == EOF) if (write_fail_fatal)
    {
    perror ("Failed to close status file");
    exit (-1);
    }
  else
    {
    sprintf (write_fail_str, "Failed to close status file: %s.\n(Correct error and try again.)", strerror (errno));
    return;
    }

  return;
}
/* End of Status_File_Write */


/****************************************************************************
*
*  Function Name:                 Status_Byte_Read
*
*    This function reads in a block of the status file.
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
static U8_t *Status_Byte_Read
  (
  U32_t        size,
  FILE        *f
  )
{
  U8_t        *buf_p;

#ifdef _MIND_MEM_
  mind_malloc ("buf_p", "status{3}", &buf_p, size);
#else
  Mem_Alloc (U8_t *, buf_p, size, GLOBAL);
#endif
  if (buf_p == NULL)
    {
    perror ("Cannot allocate sufficient memory");
    exit (-1);
    }

  if (fread (buf_p, 1, size, f) != size)
    {
    perror ("Failed to read status file");
    exit (-1);
    }
  
  return buf_p;
}
/* End of Status_Byte_Read */


/****************************************************************************
*
*  Function Name:                 Status_FCBs_Read
*
*    This function reads in the directory names from the status file.
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
*    May exit program
*
******************************************************************************/
void Status_FCBs_Read
  (
  Rcb_t         *rcb_p,                 /* run control block */
  FILE          *f_p
  )
{
  U8_t          *buffer;
  U8_t          *buff_p;
  size_t         length;
  U32_t          size;
  U8_t           fcb_i;

  size = 0;
  for (fcb_i = 0; fcb_i < FCB_IND_NUMOF; fcb_i++)
    {
    size += String_Length_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
       fcb_i))) + 1;
    size += String_Length_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p,
       fcb_i))) + 1;
    }

  buffer = Status_Byte_Read (size, f_p);
  buff_p = buffer;

  for (fcb_i = 0; fcb_i < FCB_IND_NUMOF; fcb_i++)
    {
    length = String_Length_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
       fcb_i)));
    FileCB_DirStr_Put (Rcb_IthFileCB_Get (rcb_p, fcb_i), 
      String_Create ((char *) buff_p, length));
    buff_p += length + 1;
    length = String_Length_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p,
       fcb_i)));
    FileCB_FileStr_Put (Rcb_IthFileCB_Get (rcb_p, fcb_i), 
      String_Create ((char *) buff_p, length));
    buff_p += length + 1;
    }

#ifdef _MIND_MEM_
  mind_free ("buffer", "status", buffer);
#else
  Mem_Dealloc (buffer, size, GLOBAL);
#endif
  return ;
}
/* End of Status_FCBs_Read */


/****************************************************************************
*
*  Function Name:                 Status_FCBs_Write
*
*    This routine stores the directory names to the status file.
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
*    May call exit ()
*
******************************************************************************/

void Status_FCBs_Write
  (
  Rcb_t         *rcb_p,                 /* run control block */
  FILE          *f_p
  )
{
  char          *output_p;
  U32_t          size;
  U8_t           fcb_i;

  for (fcb_i = 0; fcb_i < FCB_IND_NUMOF; fcb_i++)
    {
    output_p = (char *) String_Value_Get (FileCB_DirStr_Get (
      Rcb_IthFileCB_Get (rcb_p, fcb_i)));
    size = String_Length_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (rcb_p,
       fcb_i))) + 1;

    if (fwrite (output_p, 1, size, f_p) != size) 
      { 
      perror ("Failed to write status file");  
      exit (-1);  
      }

    output_p = (char *) String_Value_Get (FileCB_FileStr_Get (
      Rcb_IthFileCB_Get (rcb_p, fcb_i)));
    size = String_Length_Get (FileCB_FileStr_Get (Rcb_IthFileCB_Get (rcb_p,
       fcb_i))) + 1;

    if (fwrite (output_p, 1, size, f_p) != size) 
      { 
      perror ("Failed to write status file");  
      exit (-1);  
      }
    }

  return ;
}
/* End of Status_FCBs_Write */

/****************************************************************************
*
*  Function Name:                 Status_Pst_Construct
*
*    This function constructs a pst and symbol table.
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

static void Status_Pst_Construct
  (
  Array_t       *subgoals_p,
  Array_t       *compounds_p,
  Array_t       *symtabs_p,
  U8_t          *string_p,
  PstCB_t       *pstcb_p 
  )
{
  Compound_t    *comp_p;
  Subgoal_t     *subg_p;
  SymTab_t      *sym_p;
  U8_t          *temp_p;
  U32_t          i;
  U32_t          size;

  /* restore the Pst control block */
  PstCB_Root_Put (pstcb_p, (Subgoal_t *)Array_1dAddr_Get (subgoals_p, 0));

  /*  A current compound index of 0 means a stuck search, and should be
      set to NULL.
  */
  if (PstCB_CurrentCompIndex_Get (pstcb_p) == 0)
    PstCB_CurrentComp_Put (pstcb_p, NULL);
  else
    PstCB_CurrentComp_Put (pstcb_p, (Compound_t *) 
      Array_1dAddr_Get (compounds_p, 
      PstCB_CurrentCompIndex_Get (pstcb_p) - 1)); 

  if (PstCB_LastExpandedCompoundIndex_Get (pstcb_p) == 0)
    PstCB_LastExpandedCompound_Put (pstcb_p, NULL);
  else
    PstCB_LastExpandedCompound_Put (pstcb_p, (Compound_t *) 
      Array_1dAddr_Get (compounds_p, 
      PstCB_LastExpandedCompoundIndex_Get (pstcb_p) - 1)); 

  for (i = 0; i < PstCB_SubgoalIndex_Get (pstcb_p) - 1; ++i)
    {
    subg_p = (Subgoal_t *)Array_1dAddr_Get (subgoals_p, i);

    if (PstSubg_Father_Get (subg_p) != NULL)
      PstSubg_Father_Put (subg_p, (Compound_t *)Array_1dAddr_Get (compounds_p, 
      PstSubg_FatherIndex_Get (subg_p) - 1)); 

    PstSubg_Son_Put (subg_p, (Compound_t *)Array_1dAddr_Get (compounds_p,
        PstSubg_SonIndex_Get (subg_p) - 1));

    if (PstSubg_Brother_Get (subg_p) != NULL)
      PstSubg_Brother_Put (subg_p, (Subgoal_t *)Array_1dAddr_Get (subgoals_p,
          PstSubg_BrotherIndex_Get (subg_p) - 1));
    }

  for (i = 0; i < PstCB_CompoundIndex_Get (pstcb_p) - 1; ++i)
    {
    comp_p = (Compound_t *)Array_1dAddr_Get (compounds_p, i);
    PstComp_Father_Put (comp_p, (Subgoal_t *)Array_1dAddr_Get (subgoals_p,
        PstComp_FatherIndex_Get (comp_p) - 1));
   
    if (PstComp_Son_Get (comp_p) != NULL)
      PstComp_Son_Put (comp_p, (Subgoal_t *)Array_1dAddr_Get (subgoals_p,
          PstComp_SonIndex_Get (comp_p) - 1));
 
    if (PstComp_Prev_Get (comp_p) != NULL)
      PstComp_Prev_Put (comp_p, (Compound_t *)Array_1dAddr_Get (compounds_p,
          PstComp_PrevIndex_Get (comp_p) - 1));

    if (PstComp_Next_Get (comp_p) != NULL)
      PstComp_Next_Put (comp_p, (Compound_t *)Array_1dAddr_Get (compounds_p,
          PstComp_NextIndex_Get (comp_p) - 1));

    if (PstComp_Brother_Get (comp_p) != NULL)
      PstComp_Brother_Put (comp_p, (Compound_t *)Array_1dAddr_Get (compounds_p,
          PstComp_BrotherIndex_Get (comp_p) - 1));

    PstComp_SymbolTable_Put (comp_p, (SymTab_t *)Array_1dAddr_Get (symtabs_p,
        PstComp_SymbolTableIndex_Get (comp_p) - 1));
    }

  for (i = 0;  i < PstCB_SymtabIndex_Get (pstcb_p) - 1; ++i)
    {
    sym_p = (SymTab_t *)Array_1dAddr_Get (symtabs_p, i);
    if (SymTab_FirstComp_Get (sym_p) != NULL)
      SymTab_FirstComp_Put (sym_p, (Compound_t *)Array_1dAddr_Get (compounds_p,
           SymTab_FirstCompIndex_Get (sym_p) - 1));
    
    if (SymTab_DevelopedComp_Get (sym_p) != NULL)
      SymTab_DevelopedComp_Put (sym_p, 
    (Compound_t *)Array_1dAddr_Get (compounds_p,
        SymTab_DevelopedCompIndex_Get (sym_p) - 1));

    if (SymTab_Current_Get (sym_p) != NULL)
      SymTab_Current_Put (sym_p, (Compound_t *)Array_1dAddr_Get (compounds_p,
          SymTab_CurrentIndex_Get (sym_p)  - 1));

    size = Sling_Length_Get (SymTab_Sling_Get (sym_p));
#ifdef _MIND_MEM_
    mind_malloc ("temp_p", "status", &temp_p, size + 1);
#else
    Mem_Alloc (U8_t *, temp_p, size + 1, GLOBAL);
#endif
    memcpy (temp_p, string_p, size);
    temp_p[size] = '\0';
    Sling_Name_Put (SymTab_Sling_Get (sym_p), temp_p);
    string_p += size;
    }

  return;
}

/****************************************************************************
*
*  Function Name:                 Status_Pst_Serialize
*
*    This function changes all the pointers in the pst to their corresponding
*    index, and stores the pointers to subgoals and compounds in two separate
*    arrays.
*
*  Implicit Inputs:
*
*    N/A
*
*  Implicit Outputs:
*
*    A modified Pst, subgoals and compounds lists
*
*  Return Values:
*
*    N/A 
*
*  Side Effects:
*
*     N/A
*
******************************************************************************/

void Status_Pst_Serialize 
  (
  Array_t      *subgoals_p,
  Array_t      *compounds_p,
  PstNode_t     node
  )
{
  Subgoal_t    *subg_p;
  Compound_t   *comp_p;
  PstNode_t     newnode;

  while (PstNode_Compound_Get (&node) != NULL)
    {
    newnode = Pst_Son_Get (&node);
    Status_Pst_Serialize (subgoals_p, compounds_p, newnode);
    newnode = Pst_Brother_Get (&node);

    if (PstNode_Type_Get (&node) == PST_COMPOUND)
      {
      comp_p = PstNode_Compound_Get (&node);
      Array_1d32_Put (compounds_p, PstComp_Index_Get (comp_p) - 1, comp_p);
      }

    else
      {
      subg_p = PstNode_Subgoal_Get (&node);
      Array_1d32_Put (subgoals_p, PstSubg_Index_Get (subg_p) - 1, subg_p); 
      }

    node = newnode;
    }

  return;
}
