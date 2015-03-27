/******************************************************************************
*
*  Copyright (C) 1993-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     REACT_FILE.C
*
*    This module is the abstraction of the on-disk structure for the
*    Reaction Library.  The actual layout is described in REACT_FILE.H.
*
*  Routines:
*
*    React_Destroy
*    React_Dump
*    React_Read
*    React_TextRead
*    React_TextWrite
*    React_Write
*
*  Creation Date:
*
*    01-Jan-1993
*
*  Authors:
*
*    Tito Autrey
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <string.h>
#include <stdlib.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"

#ifndef _H_ARRAY_
#include "array.h"
#endif

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_TSD_
#include "tsd.h"
#endif

#ifndef _H_FUNCGROUPS_
#include "funcgroups.h"
#endif

#ifndef _H_POSTTEST_
#include "posttest.h"
#endif

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_REACT_FILE_
#include "react_file.h"
#endif


/****************************************************************************
*
*  Function Name:                 React_Destroy
*
*    This routine destroys the React_Record_t parameter.  This data-type
*    should only be created by calling React_Read.
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
void React_Destroy
  (
  React_Record_t *react_p                  /* React_Record_t to destroy */
  )
{
  React_TextRec_t *text_p;                 /* Text portion handle */
  React_TextHead_t *thead_p;               /* Text header handle */
  React_Head_t *head_p;                    /* Data header handle */
  U8_t          i;                         /* Counter */

  DEBUG (R_REACTION, DB_REACTDESTROY, TL_PARAMS, (outbuf,
    "Entering React_Destroy, record addr = %p", react_p));

  head_p = React_Head_Get (react_p);
  text_p = React_Text_Get (react_p);
  if (text_p != NULL)
    {
    thead_p = React_TxtRec_Head_Get (text_p);

    for (i = 0; i < React_Head_NumTests_Get (head_p); i++)
      {
      String_Destroy (React_TxtRec_Chemist_Get (text_p, i));
      String_Destroy (React_TxtRec_Reason_Get (text_p, i));
      }

    if (React_Head_NumTests_Get (head_p) != 0)
      {
#ifdef _MIND_MEM_
      mind_free ("text_p->chemists", "react_file", text_p->chemists);

      DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
        "Deallocated chemists array in React_Destroy at %p", text_p->chemists));

      mind_free ("text_p->reasons", "react_file", text_p->reasons);
#else
      Mem_Dealloc (text_p->chemists, React_Head_NumTests_Get (head_p) *
        STRINGSIZE, GLOBAL);

      DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
        "Deallocated chemists array in React_Destroy at %p", text_p->chemists));

      Mem_Dealloc (text_p->reasons, React_Head_NumTests_Get (head_p) *
        STRINGSIZE, GLOBAL);
#endif

      DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
        "Deallocated reasons array in React_Destroy at %p", text_p->reasons));
      }

    for (i = 0; i < React_TxtHd_NumComments_Get (thead_p); i++)
      String_Destroy (React_TxtRec_Comment_Get (text_p, i));

    if (React_TxtHd_NumComments_Get (thead_p) != 0)
      {
#ifdef _MIND_MEM_
      mind_free ("text_p->comments", "react_file", text_p->comments);
#else
      Mem_Dealloc (text_p->comments, React_TxtHd_NumComments_Get (thead_p) *
        STRINGSIZE, GLOBAL);
#endif

      DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
        "Deallocated comments array in React_Destroy at %p", text_p->comments));
      }

    for (i = i; i < React_TxtHd_NumReferences_Get (thead_p); i++)
      String_Destroy (React_TxtRec_Reference_Get (text_p, i));

    if (React_TxtHd_NumReferences_Get (thead_p) != 0)
      {
#ifdef _MIND_MEM_
      mind_free ("text_p->lit_refs", "react_file", text_p->lit_refs);
#else
      Mem_Dealloc (text_p->lit_refs, React_TxtHd_NumReferences_Get (thead_p) *
        STRINGSIZE, GLOBAL);
#endif

      DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
        "Deallocated literature references array in React_Destroy at %p",
        text_p->lit_refs));
      }

    String_Destroy (React_TxtRec_Name_Get (text_p));

    DEBUG_DO (DB_REACTDESTROY, TL_MEMORY,
      {
      memset (text_p, 0, REACTEXTRECSIZE);
      });

#ifdef _MIND_MEM_
    mind_free ("text_p", "react_file", text_p);

    DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
      "Deallocated a Reaction text record in React_Destroy at %p", text_p));
    }

  if (react_p->conditions != NULL)
    {
    mind_free ("react_p->conditions", "react_file", react_p->conditions);

    DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
      "Deallocated the Conditions array in React_Destroy at %p",
      react_p->conditions));
    }

  if (react_p->tests != NULL)
    {
    mind_free ("react_p->tests", "react_file", react_p->tests);

    DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
      "Deallocated the Posttests array in React_Destroy at %p",
      react_p->tests));
    }

  Tsd_Destroy (React_Goal_Get (react_p));
  Tsd_Destroy (React_Subgoal_Get (react_p));
  mind_Array_Destroy ("&react_p->mustall", "react_file", &react_p->mustall);
  mind_Array_Destroy ("&react_p->mustany", "react_file", &react_p->mustany);
  mind_Array_Destroy ("&react_p->notall", "react_file", &react_p->notall);
  mind_Array_Destroy ("&react_p->notany", "react_file", &react_p->notany);

  DEBUG_DO (DB_REACTDESTROY, TL_MEMORY,
    {
    memset (react_p, 0, REACTRECSIZE);
    });

  mind_free ("react_p", "react_file", react_p);
#else
    Mem_Dealloc (text_p, REACTEXTRECSIZE, GLOBAL);

    DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
      "Deallocated a Reaction text record in React_Destroy at %p", text_p));
    }

  if (react_p->conditions != NULL)
    {
    Mem_Dealloc (react_p->conditions, React_Head_NumConditions_Get (head_p) *
      CONDITIONSIZE, GLOBAL);

    DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
      "Deallocated the Conditions array in React_Destroy at %p",
      react_p->conditions));
    }

  if (react_p->tests != NULL)
    {
    Mem_Dealloc (react_p->tests, React_Head_NumTests_Get (head_p) *
      POSTTESTSIZE, GLOBAL);

    DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
      "Deallocated the Posttests array in React_Destroy at %p",
      react_p->tests));
    }

  Tsd_Destroy (React_Goal_Get (react_p));
  Tsd_Destroy (React_Subgoal_Get (react_p));
  Array_Destroy (&react_p->mustall);
  Array_Destroy (&react_p->mustany);
  Array_Destroy (&react_p->notall);
  Array_Destroy (&react_p->notany);

  DEBUG_DO (DB_REACTDESTROY, TL_MEMORY,
    {
    memset (react_p, 0, REACTRECSIZE);
    });

  Mem_Dealloc (react_p, REACTRECSIZE, GLOBAL);
#endif

  DEBUG (R_REACTION, DB_REACTDESTROY, TL_MEMORY, (outbuf,
    "Deallocated a Reaction data record in React_Destroy at %p", react_p));

  DEBUG (R_REACTION, DB_REACTDESTROY, TL_PARAMS, (outbuf,
    "Leaving React_Destroy, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 React_Dump
*
*    This function prints a formatted dump of a React_Record_t to the
*    specified file.
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
void React_Dump
  (
  React_Record_t *react_p,                 /* Reaction to format */
  FileDsc_t    *filed_p                    /* File to dump to */
  )
{
  FILE         *f;                         /* Temporary */
  React_Head_t *head_p;                    /* Data header handle */
  React_TextRec_t *text_p;                 /* Text record handle */
  React_TextHead_t *thead_p;               /* Text header handle */
  U8_t          x;                         /* Temporary due to compiler */
  U8_t          i;                         /* Counter */
  Boolean_t     flag;                      /* True if comma needed */

  f = IO_FileHandle_Get (filed_p);
  if (react_p == NULL)
    {
    fprintf (f, "Attempted to dump a NULL Reaction Record\n");
    return;
    }

  DEBUG_ADDR (R_REACTION, DB_REACTREAD, react_p);
  fprintf (f, "Dump of Reaction Record\n");
  head_p = React_Head_Get (react_p);
  text_p = React_Text_Get (react_p);
  if (text_p != NULL)
    {
    thead_p = React_TxtRec_Head_Get (text_p);
    fprintf (f, "Reaction name is %s\n", String_Value_Get (
      React_TxtRec_Name_Get (text_p)));
    fprintf (f, "Created %ld\tLast mod. %ld\tOrig. schema %hu\n",
      React_TxtHd_Created_Get (thead_p), React_TxtHd_LastMod_Get (thead_p),
      React_TxtHd_OrigSchema_Get (thead_p));
    }
  else
    {
    fprintf (f, "Reaction text is Unavailable\n");
    thead_p = NULL;
    }

  fprintf (f, "Schema Id %lu\tReaction type %hu\tMax. Non-identical"
    " subgoals %hu\n", React_Head_SchemaId_Get (head_p), 
    React_Head_ReactionType_Get (head_p), React_Head_MaxNonident_Get (head_p));
  fprintf (f, "Ease %u\t\tYield %hu\tConfidence %u\tPattern Size %hu\n",
    React_Head_Ease_Get (head_p), React_Head_Yield_Get (head_p),
    React_Head_Confidence_Get (head_p), React_Head_GoalSize_Get (head_p));
  fprintf (f, "Syntheme FG %hu\tFlags %X",
    React_Head_SynthemeFG_Get (head_p), React_Head_Flags_Get (head_p));

  if (React_HeadFlags_Disabled_Get (head_p) == TRUE ||
      React_HeadFlags_Incomplete_Get (head_p) == TRUE ||
      React_HeadFlags_Protection_Get (head_p) == TRUE)
    {
    fprintf (f, " (");
    flag = FALSE;

    if (React_HeadFlags_Disabled_Get (head_p) == TRUE)
      fprintf (f, "Disabled"), flag = TRUE;

    if (React_HeadFlags_Incomplete_Get (head_p) == TRUE)
      {
      if (flag == TRUE)
        fprintf (f, ", ");

      fprintf (f, "Incomplete");
      flag = TRUE;
      }

    if (React_HeadFlags_Protection_Get (head_p) == TRUE)
      {
      if (flag == TRUE)
        fprintf (f, ", ");

      fprintf (f, "Protection");
      }

    fprintf (f, ")\n");
    }
  else
    fprintf (f, "\n");

  fprintf (f, "Root nodes ");
  for (i = 0; i < MX_ROOTS; i++)
    {
    x = React_Head_RootNode_Get (head_p, i);
    if (x != REACT_NODE_INVALID)
      fprintf (f, "\t%hu", x);
    else
      i = MX_ROOTS;
    }

  fprintf (f, "\nRoot synthemes ");
  for (i = 0; i < MX_ROOTS; i++)
    {
    x = React_Head_RootSyntheme_Get (head_p, i);
    if (x != 0)
      fprintf (f, "\t%hu", x);
    else
      i = MX_ROOTS;
    }
/*
  fprintf (f, "\n\nGoal TSD\n");
  Tsd_Dump (React_Goal_Get (react_p), filed_p);
  fprintf (f, "\nSubgoal TSD\n");
  Tsd_Dump (React_Subgoal_Get (react_p), filed_p);

  fprintf (f, "\nCan't have all\n");
  Array_Dump (&react_p->notall, filed_p);
  fprintf (f, "\nCan't have any\n");
  Array_Dump (&react_p->notany, filed_p);
  fprintf (f, "Must have all\n");
  Array_Dump (&react_p->mustall, filed_p);
  fprintf (f, "Must have any\n");
  Array_Dump (&react_p->mustany, filed_p);
*/
  fprintf (f, "\nPost-transform tests.  # conditions %hu.  # tests %hu.\n",
    React_Head_NumConditions_Get (head_p), React_Head_NumTests_Get (head_p));

  for (i = 0; i < React_Head_NumConditions_Get (head_p); i++)
    Condition_Dump (React_Condition_Get (react_p, i), filed_p);

  for (i = 0; i < React_Head_NumTests_Get (head_p); i++)
    {
    if (text_p != NULL)
      {
      Posttest_Dump (React_Test_Get (react_p, i), filed_p);
      fprintf (f, "Rationale for test  :\t%s\n", String_Value_Get (
        React_TxtRec_Reason_Get (text_p, i)));
      fprintf (f, "Chemist\t\t    :\t%s\n\n", String_Value_Get (
        React_TxtRec_Chemist_Get (text_p, i)));
      }
    }

  if (text_p != NULL)
    {
    fprintf (f, "# comments %hu.  # literature references %hu.\n",
      React_TxtHd_NumComments_Get (thead_p), React_TxtHd_NumReferences_Get (
      thead_p));

    for (i = 0; i < React_TxtHd_NumComments_Get (thead_p); i++)
      fprintf (f, "%s\n", String_Value_Get (React_TxtRec_Comment_Get (text_p,
        i)));

    if (React_TxtHd_NumComments_Get (thead_p) != 0 &&
        React_TxtHd_NumReferences_Get (thead_p) != 0)
      fprintf (f, "\n");

    for (i = 0; i < React_TxtHd_NumReferences_Get (thead_p); i++)
      fprintf (f, "%s\n", String_Value_Get (React_TxtRec_Reference_Get (text_p,
        i)));
    }

  fprintf (f, "\n");
  return;
}

/****************************************************************************
*
*  Function Name:                 React_Read
*
*    This function reads the Reaction Record with the specified key value.
*    It will read in the React_TextRec if specified, otherwise it will
*    save the VM by leaving it on disk.
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
*    Handle for new Reaction data record
*    NULL   for failure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
React_Record_t *React_Read
  (
  U32_t         key,                       /* Key of record to read */
  Isam_Control_t *file_p                   /* ISAM control block handle */
  )
{
  React_Record_t *react_p;                 /* Handle for new React_Record_t */
  React_Head_t *head_p;                    /* Temporary */
  Condition_t  *cond_p;                    /* Temporary */
  Posttest_t   *test_p;                    /* Temporary */
  Tsd_t        *tsd_p;                     /* For reading the TSDs */
  U8_t         *buf_p;                     /* Pointer to file buffer */
  U16_t         length;                    /* # U8_t's to copy */
  U8_t          i;                         /* Counter */

  DEBUG (R_REACTION, DB_REACTREAD, TL_PARAMS, (outbuf,
    "Entering React_Read, key = %lu, ISAM cb = %p", key, file_p));

  buf_p = (U8_t *) Isam_Read (file_p, key);
  if (buf_p == NULL)
    return NULL;

#ifdef _MIND_MEM_
  mind_malloc ("react_p", "react_file{1}", &react_p, REACTRECSIZE);
#else
  Mem_Alloc (React_Record_t *, react_p, REACTRECSIZE, GLOBAL);
#endif

  if (react_p == NULL)
    IO_Exit_Error (R_REACTION, X_LIBCALL,
      "Failed to allocate memory for Reaction record");

  DEBUG (R_REACTION, DB_REACTREAD, TL_MEMORY, (outbuf,
    "Allocated memory for a Reaction record in React_Read at %p", react_p));

  head_p = React_Head_Get (react_p);
  memcpy (head_p, buf_p, REACTHEADSIZE);
  buf_p += REACTHEADSIZE;

#ifdef _MIND_MEM_
  mind_Array_1d_Create ("&react_p->notall", "react_file{1}", &react_p->notall, MX_FUNCGROUPS, BITSIZE);
  mind_Array_1d_Create ("&react_p->notany", "react_file{1}", &react_p->notany, MX_FUNCGROUPS, BITSIZE);
  mind_Array_1d_Create ("&react_p->mustall", "react_file{1}", &react_p->mustall, MX_FUNCGROUPS, BITSIZE);
  mind_Array_1d_Create ("&react_p->mustany", "react_file{1}", &react_p->mustany, MX_FUNCGROUPS, BITSIZE);
#else
  Array_1d_Create (&react_p->notall, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&react_p->notany, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&react_p->mustall, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&react_p->mustany, MX_FUNCGROUPS, BITSIZE);
#endif

  length = ((MX_FUNCGROUPS + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) *
    BITUNITSIZE;

  memcpy (Array_Storage_Get (&react_p->notall), buf_p, length);
  buf_p += length;
  memcpy (Array_Storage_Get (&react_p->notany), buf_p, length);
  buf_p += length;
  memcpy (Array_Storage_Get (&react_p->mustall), buf_p, length);
  buf_p += length;
  memcpy (Array_Storage_Get (&react_p->mustany), buf_p, length);
  buf_p += length;

  tsd_p = Tsd_Create (React_Head_NumGoalAtoms_Get (head_p));
  length = React_Head_NumGoalAtoms_Get (head_p) * TSDROWSIZE;
  memcpy (Tsd_AtomHandle_Get (tsd_p), buf_p, length);
  buf_p += length;
  React_Goal_Put (react_p, tsd_p);

  tsd_p = Tsd_Create (React_Head_NumSubgoalAtoms_Get (head_p));
  length = React_Head_NumSubgoalAtoms_Get (head_p) * TSDROWSIZE;
  memcpy (Tsd_AtomHandle_Get (tsd_p), buf_p, length);
  buf_p += length;
  React_Subgoal_Put (react_p, tsd_p);

  if (React_Head_NumConditions_Get (head_p) != 0)
    {
#ifdef _MIND_MEM_
    mind_malloc ("cond_p", "react_file{1}", &cond_p, React_Head_NumConditions_Get (head_p) * CONDITIONSIZE);
#else
    Mem_Alloc (Condition_t *, cond_p, React_Head_NumConditions_Get (head_p) *
      CONDITIONSIZE, GLOBAL);
#endif

    if (cond_p == NULL)
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for conditions array");

    DEBUG (R_REACTION, DB_REACTREAD, TL_MEMORY, (outbuf,
      "Allocated memory for the condition array in React_Read at %p", cond_p));

    react_p->conditions = cond_p;
    }
  else
    react_p->conditions = NULL;

  for (i = 0; i < React_Head_NumConditions_Get (head_p); i++,
       buf_p += CONDITIONSIZE)
    memcpy (React_Condition_Get (react_p, i), buf_p, CONDITIONSIZE);

  if (React_Head_NumTests_Get (head_p) != 0)
    {
#ifdef _MIND_MEM_
    mind_malloc ("test_p", "react_file{1}", &test_p, React_Head_NumTests_Get (head_p) * POSTTESTSIZE);
#else
    Mem_Alloc (Posttest_t *, test_p, React_Head_NumTests_Get (head_p) *
      POSTTESTSIZE, GLOBAL);
#endif

    if (test_p == NULL)
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for post-tests array");

    DEBUG (R_REACTION, DB_REACTREAD, TL_MEMORY, (outbuf,
      "Allocated memory for the post-test array in React_Read at %p", test_p));

    react_p->tests = test_p;
    }
  else
    react_p->tests = NULL;

  for (i = 0; i < React_Head_NumTests_Get (head_p); i++, buf_p += length)
    {
    test_p = React_Test_Get (react_p, i);
    memcpy (Post_Head_Get (test_p), buf_p, POSTTESTHEADSIZE);
    buf_p += POSTTESTHEADSIZE;

    length = Post_Length_Get (test_p);
#ifdef _MIND_MEM_
    mind_malloc ("test_p->ops", "react_file{1}", &test_p->ops, length);
#else
    test_p->ops = (U8_t *)malloc (length);
#endif
    if (test_p->ops == NULL)
      {
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for post-test ops list");

      DEBUG (R_REACTION, DB_REACTREAD, TL_MEMORY, (outbuf,
        "Allocated memory for the post-test ops list in React_Read at %p",
         test_p->ops));
      }

    memcpy (test_p->ops, buf_p, length);
    }

  DEBUG (R_REACTION, DB_REACTREAD, TL_PARAMS, (outbuf,
    "Leaving React_Read, React_Record = %p", react_p));

  return react_p;
}

/****************************************************************************
*
*  Function Name:                 React_TextRead
*
*    This function will read just the React_TextRec_t from the Reaction
*    text file.  This function exists so that the textual information
*    can be brought in for the interpretation at the end.  At the very
*    least the schema name is useful.
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
*    Handle for new Reaction text record
*    NULL   for failure
*
*  Side Effects:
*
*    Allocates memory
*    May call IO_Exit_Error
*
******************************************************************************/
React_TextRec_t *React_TextRead
  (
  U32_t         key,                       /* Key of reaction to read */
  Isam_Control_t *file_p,                  /* ISAM control block handle */
  U8_t          num_tests                  /* # of tests for this reaction */
  )
{
  React_TextRec_t *react_p;                /* Handle for new React_TextRec_t */
  React_TextHead_t *head_p;                /* Temporary */
  String_t     *str_p;                     /* Temporary */
  U8_t         *buf_p;                     /* Pointer to file buffer */
  U16_t         length;                    /* # U8_t's to copy */
  U8_t          i;                         /* Counter */

  DEBUG (R_REACTION, DB_REACTXTREAD, TL_PARAMS, (outbuf,
    "Entering React_TextRead, key = %lu, ISAM cb = %p", key, file_p));

  buf_p = (U8_t *) Isam_Read (file_p, key);
  if (buf_p == NULL)
    return NULL;

#ifdef _MIND_MEM_
  mind_malloc ("react_p", "react_file{2}", &react_p, REACTEXTRECSIZE);
#else
  Mem_Alloc (React_TextRec_t *, react_p, REACTEXTRECSIZE, GLOBAL);
#endif

  if (react_p == NULL)
    IO_Exit_Error (R_REACTION, X_LIBCALL,
      "Failed to allocate memory for Reaction text record");

  DEBUG (R_REACTION, DB_REACTXTREAD, TL_MEMORY, (outbuf,
    "Allocated memory for Reaction text record in React_TextRead at %p",
    react_p));

  /* On-disk layout (roughly): header, lengths of - post test reasons,
     post test chemists, comments, lit. references; values of previous,
     schema name length and value.
  */

  head_p = React_TxtRec_Head_Get (react_p);
  memcpy (head_p, buf_p, REACTEXTHEADSIZE);
  buf_p += REACTEXTHEADSIZE;

  memcpy (&length, buf_p, sizeof (U16_t));
  buf_p += sizeof (U16_t);
  React_TxtRec_Name_Put (react_p, String_Create_nn ((char *) buf_p, length));
  buf_p += length;

  if (num_tests != 0)
    {
#ifdef _MIND_MEM_
    mind_malloc ("str_p", "react_file{2}", &str_p, num_tests * STRINGSIZE);
#else
    Mem_Alloc (String_t *, str_p, num_tests * STRINGSIZE, GLOBAL);
#endif

    if (str_p == NULL)
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for post-test reasons array");

    DEBUG (R_REACTION, DB_REACTXTREAD, TL_MEMORY, (outbuf,
      "Allocated memory for the post-test reasons array in"
      " React_TextRead at %p", str_p));

    react_p->reasons = str_p;

#ifdef _MIND_MEM_
    mind_malloc ("str_p", "react_file{2a}", &str_p, num_tests * STRINGSIZE);
#else
    Mem_Alloc (String_t *, str_p, num_tests * STRINGSIZE, GLOBAL);
#endif

    if (str_p == NULL)
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for post-test chemists array");

    DEBUG (R_REACTION, DB_REACTXTREAD, TL_MEMORY, (outbuf,
      "Allocated memory for the post-test chemists array in"
      " React_TextRead at %p", str_p));

    react_p->chemists = str_p;
    }
  else
    {
    react_p->reasons = NULL;
    react_p->chemists = NULL;
    }

  for (i = 0; i < num_tests; i++)
    {
    memcpy (&length, buf_p, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    React_TxtRec_Reason_Put (react_p, i, String_Create_nn ((char *) buf_p, 
      length));
    buf_p += length;
    }

  for (i = 0; i < num_tests; i++)
    {
    memcpy (&length, buf_p, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    React_TxtRec_Chemist_Put (react_p, i, String_Create_nn ((char *) buf_p, 
      length));
    buf_p += length;
    }

  if (React_TxtHd_NumComments_Get (head_p) != 0)
    {
#ifdef _MIND_MEM_
    mind_malloc ("str_p", "react_file{2b}", &str_p, React_TxtHd_NumComments_Get (head_p) * STRINGSIZE);
#else
    Mem_Alloc (String_t *, str_p, React_TxtHd_NumComments_Get (head_p) *
      STRINGSIZE, GLOBAL);
#endif

    if (str_p == NULL)
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for post-test comments array");

    DEBUG (R_REACTION, DB_REACTXTREAD, TL_MEMORY, (outbuf,
      "Allocated memory for the post-test comments array in"
      " React_TextRead at %p", str_p));

    react_p->comments = str_p;
    }
  else
    react_p->comments = NULL;

  for (i = 0; i < React_TxtHd_NumComments_Get (head_p); i++)
    {
    memcpy (&length, buf_p, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    React_TxtRec_Comment_Put (react_p, i, String_Create_nn ((char *) buf_p, 
      length));
    buf_p += length;
    }

  if (React_TxtHd_NumReferences_Get (head_p) != 0)
    {
#ifdef _MIND_MEM_
    mind_malloc ("str_p", "react_file{2c}", &str_p, React_TxtHd_NumReferences_Get (head_p) * STRINGSIZE);
#else
    Mem_Alloc (String_t *, str_p, React_TxtHd_NumReferences_Get (head_p) *
      STRINGSIZE, GLOBAL);
#endif

    if (str_p == NULL)
      IO_Exit_Error (R_REACTION, X_LIBCALL,
        "Failed to allocate memory for post-test literature references array");

    DEBUG (R_REACTION, DB_REACTXTREAD, TL_MEMORY, (outbuf,
      "Allocated memory for the post-test literature referencess array in"
      " React_TextRead at %p",
      str_p));

    react_p->lit_refs = str_p;
    }
  else
    react_p->lit_refs = NULL;

  for (i = 0; i < React_TxtHd_NumReferences_Get (head_p); i++)
    {
    memcpy (&length, buf_p, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    React_TxtRec_Reference_Put (react_p, i, String_Create_nn ((char *) buf_p, 
      length));
    buf_p += length;
    }

  DEBUG (R_REACTION, DB_REACTXTREAD, TL_PARAMS, (outbuf,
    "Leaving React_TextRead, Reaction record = %p", react_p));
  return react_p;
}

/****************************************************************************
*
*  Function Name:                 React_TextWrite
*
*    This routine writes out the React_TxtRec_t parameter.  Since the
*    the Reaction data and text go to different files, it takes two
*    separate write routines.
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
void React_TextWrite
  (
  React_Record_t *react_p,                 /* React_TextRec_t to write */
  Isam_Control_t *file_p                   /* ISAM control block handle */
  )
{
  React_TextRec_t *text_p;                 /* What we are really after */
  React_Head_t *head_p;                    /* React_Head_t handle */
  U8_t         *buf_p;                     /* Current write pointer */
  U32_t         key;                       /* Key to write to */
  React_TextHead_t *thead_p;               /* Real header */
  U16_t         length;                    /* # U8_t's to write */
  U8_t          i;                         /* Counter */
  U8_t          obuf[REACT_MX_BUFSIZE];    /* Output buffer */

  DEBUG (R_REACTION, DB_REACTXTWRITE, TL_PARAMS, (outbuf,
    "Entering React_TextWrite, record address = %p, ISAM cb = %p", react_p,
    file_p));

  /* On-disk layout is (roughly): header, schema name, post-test reasons,
     post-test chemists, comments, lit. refs.
  */

  buf_p = obuf;
  head_p = React_Head_Get (react_p);
  text_p = React_Text_Get (react_p);
  thead_p = React_TxtRec_Head_Get (text_p);
  *(React_TextHead_t *)buf_p = *thead_p;
  buf_p += REACTEXTHEADSIZE;

  length = String_Length_Get (React_TxtRec_Name_Get (text_p));
  memcpy (buf_p, &length, sizeof (U16_t));
  buf_p += sizeof (U16_t);
  memcpy (buf_p, String_Value_Get (React_TxtRec_Name_Get (text_p)), length);
  buf_p += length;

  for (i = 0; i < React_Head_NumTests_Get (head_p); i++)
    {
    length = String_Length_Get (React_TxtRec_Reason_Get (text_p, i));
    memcpy (buf_p, &length, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    memcpy (buf_p, String_Value_Get (React_TxtRec_Reason_Get (text_p, i)),
      length);
    buf_p += length;
    }

  for (i = 0; i < React_Head_NumTests_Get (head_p); i++)
    {
    length = String_Length_Get (React_TxtRec_Chemist_Get (text_p, i));
    memcpy (buf_p, &length, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    memcpy (buf_p, String_Value_Get (React_TxtRec_Chemist_Get (text_p, i)),
      length);
    buf_p += length;
    }

  for (i = 0; i < React_TxtHd_NumComments_Get (thead_p); i++)
    {
    length = String_Length_Get (React_TxtRec_Comment_Get (text_p, i));
    memcpy (buf_p, &length, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    memcpy (buf_p, String_Value_Get (React_TxtRec_Comment_Get (text_p, i)),
      length);
    buf_p += length;
    }

  for (i = 0; i < React_TxtHd_NumReferences_Get (thead_p); i++)
    {
    length = String_Length_Get (React_TxtRec_Reference_Get (text_p, i));
    memcpy (buf_p, &length, sizeof (U16_t));
    buf_p += sizeof (U16_t);
    memcpy (buf_p, String_Value_Get (React_TxtRec_Reference_Get (text_p, i)),
      length);
    buf_p += length;
    }

  key = React_Head_SchemaId_Get (head_p);
  length = buf_p - obuf;

  ASSERT (length <= REACT_MX_BUFSIZE,
    {
    IO_Exit_Error (R_REACTION, X_SYNERR, "React_TextWrite buffer overflowed");
    });

  if (Isam_Write (file_p, key, obuf, length) != SYN_NORMAL)
    IO_Put_Trace (R_REACTION,
      "Unexpected status returned from Isam_Write in React_TextWrite");

  DEBUG (R_REACTION, DB_REACTXTWRITE, TL_PARAMS, (outbuf,
    "Leaving React_TextWrite, status = <void>"));

  return;
}

/****************************************************************************
*
*  Function Name:                 React_Write
*
*    This function writes out the specified React_Record_t by laying it
*    out according to the on-disk structure in a buffer and then calling
*    Isam_Write to put it in an ISAM file.
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
*    Writes to file
*
******************************************************************************/
void React_Write
  (
  React_Record_t *react_p,                 /* React_Record_t to write */
  Isam_Control_t *file_p                   /* ISAM control block handle */
  )
{
  U8_t         *buf_p;                     /* Current write pointer */
  React_Head_t *head_p;                    /* Temporary */
  U32_t         key;                       /* Key for ISAM record */
  U16_t         length;                    /* # U8_t's to write */
  U8_t          i;                         /* Counter */
  U8_t          obuf[REACT_MX_BUFSIZE];    /* Output buffer */

  DEBUG (R_REACTION, DB_REACTWRITE, TL_PARAMS, (outbuf,
    "Entering React_Write, record addr = %p, ISAM cb = %p", react_p, file_p));

  /* On-disk layout is (roughly): header, pretest arrays, TSDs, condition(s),
     test(s).
  */
  buf_p = obuf;
  head_p = React_Head_Get (react_p);
  *(React_Head_t *)buf_p = *head_p;
  buf_p += REACTHEADSIZE;

  length = ((MX_FUNCGROUPS + (BITUNITSIZE * 8)) / (BITUNITSIZE * 8)) *
    BITUNITSIZE;

  memcpy (buf_p, Array_Storage_Get (&react_p->notall), length);
  buf_p += length;
  memcpy (buf_p, Array_Storage_Get (&react_p->notany), length);
  buf_p += length;
  memcpy (buf_p, Array_Storage_Get (&react_p->mustall), length);
  buf_p += length;
  memcpy (buf_p, Array_Storage_Get (&react_p->mustany), length);
  buf_p += length;

  length = React_Head_NumGoalAtoms_Get (head_p) * TSDROWSIZE;
  memcpy (buf_p, Tsd_AtomHandle_Get (React_Goal_Get (react_p)), length);
  buf_p += length;
  length = React_Head_NumSubgoalAtoms_Get (head_p) * TSDROWSIZE;
  memcpy (buf_p, Tsd_AtomHandle_Get (React_Subgoal_Get (react_p)), length);
  buf_p += length;

  /* Write out conditions here */

  for (i = 0; i < React_Head_NumConditions_Get (head_p); i++,
       buf_p += CONDITIONSIZE)
    memcpy (buf_p, React_Condition_Get (react_p, i), CONDITIONSIZE);

  /* Write out tests here */

  for (i = 0; i < React_Head_NumTests_Get (head_p); i++, buf_p += length)
    {
    memcpy (buf_p, Post_Head_Get (React_Test_Get (react_p, i)),
      POSTTESTHEADSIZE);
    buf_p += POSTTESTHEADSIZE;
    length = Post_Length_Get (React_Test_Get (react_p, i));
    memcpy (buf_p, Post_OpHandle_Get (React_Test_Get (react_p, i)), length);
    }

  key = React_Head_SchemaId_Get (head_p);
  length = buf_p - obuf;

  ASSERT (length <= REACT_MX_BUFSIZE,
    {
    IO_Exit_Error (R_REACTION, X_SYNERR, "React_Write buffer overflowed");
    });

  if (Isam_Write (file_p, key, obuf, length) != SYN_NORMAL)
    IO_Put_Trace (R_REACTION,
      "Unexpected status returned from Isam_Write in React_Write");

  DEBUG (R_REACTION, DB_REACTWRITE, TL_PARAMS, (outbuf,
    "Leaving React_Write, status = <void>"));

  return;
}
/* End of React_Write */
/* End of REACT_FILE.C */
