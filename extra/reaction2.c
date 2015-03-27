/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     REACTION.C
*
*    This module is the data-abstraction module for the reaction library
*    records.  A reaction description comprises TSDs for the goal and sub-goal
*    molecules, attribute lists for can't have/must have - any/all functional
*    groups.  It also has post-transform tests to determine if the embedding
*    is chemically possible, these are encoded as FSAs.
*
*    Originally: CHAPOK.PLI, SCHEMOK.PLI, XRLIB.PLI
*
*  Routines:
*
*    React_Init
*    React_NumSchemas_Get
*    React_Schema_Handle_Get
*    React_Schema_IsOk
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

#include <string.h>
#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#define REACTION_GLOBALS
#ifndef _H_REACTION_
#include "reaction.h"
#endif
#undef REACTION_GLOBALS

#ifndef _H_REACT_FILE_
#include "react_file.h"
#endif

#ifndef _H_PERSIST_
#include "persist.h"
#endif

#ifndef _H_XTR_
#include "xtr.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

static Boolean_t already_open = FALSE;

void Sprintf (char *, String_t);
void Sgets (char *, char **);


void React_Temp_Init
  (
  U8_t *dir_p
  )
{
  fprintf (stderr, "\n\n\n\t\t* * * React_Temp_Init () is not yet a working function * * *\n\n\n");
  exit (1);
}


/****************************************************************************
*
*  Function Name:                 React_Init
*
*    This routine opens all the pre-test files in the reaction library that
*    the user input to the SYNCHEM executive.
*
*  Used to be:
*
*    N/A: (many routines)
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
*    May call IO_Exit_Error
*
******************************************************************************/
void React_Init
  (
  U8_t          *dir_p                      /* Directory with RKB files */
  )
{
  React_Initialize (dir_p, FALSE);
}

void React_Open (U8_t *dir_p)
{
  React_Open_Files (dir_p, FALSE);
}

void React_Force_Open (U8_t *dir_p)
{
  React_Open_Files (dir_p, TRUE);
}

void React_Open_Files (U8_t *dir_p, Boolean_t force_open)
{
  Boolean_t      read_only;
  int            dir_offset;

  if (already_open && !force_open) return;

  if (strncasecmp (dir_p, "R+W", 3) == 0)
    {
    read_only = FALSE;
    dir_offset = 3;
    }
  else
    {
    read_only = TRUE;
    dir_offset = 0;
    }

  strncpy (IO_FileName_Get (Isam_File_Get (&SReactDataFile)),
    (char *) (dir_p + dir_offset),
    MX_FILENAME - 1);
  strncat (IO_FileName_Get (Isam_File_Get (&SReactDataFile)), REACT_DATANAME,
    MX_FILENAME - strlen ((char *) dir_p) - dir_offset - 1);
  Isam_Open (&SReactDataFile, ISAM_TYPE_RKBDATA,
    read_only ? ISAM_OPEN_READ : ISAM_OPEN_WRITE);

  strncpy (IO_FileName_Get (Isam_File_Get (&SReactTextFile)),
    (char *) (dir_p + dir_offset),
    MX_FILENAME - 1);
  strncat (IO_FileName_Get (Isam_File_Get (&SReactTextFile)), REACT_TEXTNAME,
    MX_FILENAME - strlen ((char *) dir_p) - dir_offset - 1);
  Isam_Open (&SReactTextFile, ISAM_TYPE_RKBTEXT,
    read_only ? ISAM_OPEN_READ : ISAM_OPEN_WRITE);

  already_open = TRUE;
}

void React_Initialize (U8_t *dir_p, Boolean_t persistent_read)
{
  React_Initialization (dir_p, persistent_read, FALSE);
}

void React_Force_Initialize (U8_t *dir_p, Boolean_t persistent_read)
{
  React_Initialization (dir_p, persistent_read, TRUE);
}

void React_Initialization (U8_t *dir_p, Boolean_t persistent_read, Boolean_t force_reopen)
{
  React_TextRec_t *trec_p;                  /* Temporary for reading file */
  U32_t          i;                         /* Counter */
  Boolean_t      read_only, is_temp;
  static Boolean_t already_read = FALSE;
  static Boolean_t isam_open = FALSE;
  static U32_t   num_schemas = 0;           /* # schemas in library */
#ifdef _MIND_MEM_
  char varname[100];
#endif

  if (already_read)
  {
    if (!force_reopen) return;

    if (isam_open)
    {
      num_schemas = React_NumSchemas_Get ();
      Isam_Close (&SReactDataFile);
      Isam_Close (&SReactTextFile);
      isam_open = FALSE;
    }
    /* else use static value */

#ifdef _MIND_MEM_
    for (i = 0; i < num_schemas; i++) if (SReactions[i] != NULL)
      {
      sprintf (varname, "SReactions[%d]", i);
      mind_free (varname, "reaction", SReactions[i]);
      }
    mind_free ("SReactions", "reaction", SReactions);
#else
    for (i = 0; i < num_schemas; i++) if (SReactions[i] != NULL)
      Mem_Dealloc (SReactions[i], REACTRECSIZE, GLOBAL);
    Mem_Dealloc (SReactions, curr_schemata_allocated * sizeof (void *), GLOBAL);
#endif
    Persist_Close ();
    already_open = FALSE;
    if (!Persist_Inx_OK (FCB_SEQDIR_RXNS ("/rkbstd.inx"),
      String_Value_Get (FileCB_DirStr_Get (Rcb_IthFileCB_Get (&GGoalRcb, FCB_IND_RXNS)))))
      exit (1);
  }

  React_Force_Open (dir_p);

  DEBUG (R_REACTION, DB_REACTINIT, TL_PARAMS, (outbuf,
    "Entering React_Initialize, directory = %s", dir_p));

  read_only = strncasecmp (dir_p, "R+W", 3) != 0;

  num_schemas = React_NumSchemas_Get ();

  curr_schemata_allocated = 2 * (num_schemas + 1);

#ifdef _MIND_MEM_
  mind_malloc ("SReactions", "reaction{1}", &SReactions, (curr_schemata_allocated) * sizeof (void *));
#else
  Mem_Alloc (React_Record_t **, SReactions, (curr_schemata_allocated) * sizeof (void *),
    GLOBAL);
#endif

  DEBUG (R_REACTION, DB_REACTINIT, TL_MEMORY, (outbuf,
    "Allocated memory for Reaction Library records array in React_Init at %p",
    SReactions));

  if (SReactions == NULL)
    IO_Exit_Error (R_REACTION, X_LIBCALL,
      "Failed to allocate memory for Reaction Library in React_Init");

  for (i = 0; i < num_schemas; i++)
    if (persistent_read && i != Persist_Current_Rec (PER_STD, i, /* PER_TIME_ANY, PER_TIME_ANY, */ glob_run_date, glob_run_time, &is_temp))
    SReactions[i] = NULL;
  else
    {
    SReactions[i] = React_Read (i, &SReactDataFile);
    trec_p = React_TextRead (i, &SReactTextFile,
      React_Head_NumTests_Get (React_Head_Get (SReactions[i])));
    React_Text_Put (SReactions[i], trec_p);
    }

  for (; i < curr_schemata_allocated; i++)
    SReactions[i] = NULL;

  if (read_only && !persistent_read)
  {
    Isam_Close (&SReactDataFile);
    Isam_Close (&SReactTextFile);
    isam_open = FALSE;
  }
  else isam_open = TRUE;

  DEBUG (R_REACTION, DB_REACTINIT, TL_INIT, (outbuf,
    "Reaction Library initialized successfully"));

  DEBUG (R_REACTION, DB_REACTINIT, TL_PARAMS, (outbuf,
    "Leaving React_Init, status = <void>"));

  already_read = TRUE;

  return;
}

void React_Check_Alloc (U32_t schema_index)
{
  React_TextRec_t *trec_p;                  /* Temporary for reading file */

  if (SReactions[schema_index] == NULL)
    {
    SReactions[schema_index] = React_Read (schema_index, &SReactDataFile);
    trec_p = React_TextRead (schema_index, &SReactTextFile,
      React_Head_NumTests_Get (React_Head_Get (SReactions[schema_index])));
    React_Text_Put (SReactions[schema_index], trec_p);
    }
}

Boolean_t React_Schema_Copy (int source, int destination)
{
  int i;

#ifdef _MIND_MEM_
  char varname[100];
  void *varptr;

  if (destination < 0 || destination > React_NumSchemas_Get () ||
    SReactions[destination] != NULL) return (FALSE);

  sprintf (varname, "SReactions[%d]", destination);
  mind_malloc (varname, "reaction{2}", SReactions + destination, REACTRECSIZE);
  memcpy (SReactions[destination], SReactions[source], REACTRECSIZE);

  sprintf (varname, "React_Goal_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{2}", &varptr, TSDSIZE);
  React_Goal_Put (SReactions[destination], (Tsd_t *) varptr);
  memcpy (React_Goal_Get (SReactions[destination]),
    React_Goal_Get (SReactions[source]), TSDSIZE);
  sprintf (varname, "Tsd_AtomHandle_Get(React_Goal_Get(SReactions[%d]))", destination);
  mind_malloc (varname, "reaction{2}", &varptr, Tsd_NumAtoms_Get (React_Goal_Get (SReactions[source])) * TSDROWSIZE);
  Tsd_AtomHandle_Put (React_Goal_Get (SReactions[destination]), (TsdRow_t *) varptr);
  memcpy (Tsd_AtomHandle_Get (React_Goal_Get (SReactions[destination])),
    Tsd_AtomHandle_Get (React_Goal_Get (SReactions[source])),
    Tsd_NumAtoms_Get (React_Goal_Get (SReactions[source])) * TSDROWSIZE);

  sprintf (varname, "React_Subgoal_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{2}", &varptr, TSDSIZE);
  React_Subgoal_Put (SReactions[destination], (Tsd_t *) varptr);
  memcpy (React_Subgoal_Get (SReactions[destination]),
    React_Subgoal_Get (SReactions[source]), TSDSIZE);
  sprintf (varname, "Tsd_AtomHandle_Get(React_Subgoal_Get(SReactions[%d]))", destination);
  mind_malloc (varname, "reaction{2}", &varptr, Tsd_NumAtoms_Get (React_Subgoal_Get (SReactions[source])) * TSDROWSIZE);
  Tsd_AtomHandle_Put (React_Subgoal_Get (SReactions[destination]), (TsdRow_t *) varptr);
  memcpy (Tsd_AtomHandle_Get (React_Subgoal_Get (SReactions[destination])),
    Tsd_AtomHandle_Get (React_Subgoal_Get (SReactions[source])),
    Tsd_NumAtoms_Get (React_Subgoal_Get (SReactions[source])) * TSDROWSIZE);

  sprintf (varname, "React_Text_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{2}", &varptr, REACTEXTRECSIZE);
  React_Text_Put (SReactions[destination], (React_TextRec_t *) varptr);
  memcpy (React_TxtRec_Head_Get (React_Text_Get (SReactions[destination])),
    React_TxtRec_Head_Get (React_Text_Get (SReactions[source])), REACTEXTHEADSIZE);
  React_TxtRec_Name_Put (React_Text_Get (SReactions[destination]),
    String_Copy (React_TxtRec_Name_Get (React_Text_Get (SReactions[source]))));

  if (React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))) == 0)
    React_Text_Get (SReactions[destination])->comments = NULL;
  else
    {
    sprintf (varname, "React_Text_Get(SReactions[%d])->comments", destination);
    mind_malloc (varname, "reaction{2}", &React_Text_Get (SReactions[destination])->comments,
      React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (React_Text_Get (SReactions[source]))) * STRINGSIZE);
    }
  for (i = 0; i < React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))); i++)
    React_TxtRec_Comment_Put (React_Text_Get (SReactions[destination]), i,
    String_Copy (React_TxtRec_Comment_Get (React_Text_Get (SReactions[source]), i)));

  if (React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))) == 0)
    React_Text_Get (SReactions[destination])->lit_refs = NULL;
  else
    {
    sprintf (varname, "React_Text_Get(SReactions[%d])->lit_refs", destination);
    mind_malloc (varname, "reaction{2}", &React_Text_Get (SReactions[destination])->lit_refs,
      React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (React_Text_Get (SReactions[source]))) * STRINGSIZE);
    }
  for (i = 0; i < React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))); i++)
    React_TxtRec_Reference_Put (React_Text_Get (SReactions[destination]), i,
    String_Copy (React_TxtRec_Reference_Get (React_Text_Get (SReactions[source]), i)));

  if (React_Head_NumConditions_Get (React_Head_Get (SReactions[source])) == 0)
    SReactions[destination]->conditions = NULL;
  else
    {
    sprintf (varname, "SReactions[%d]->conditions", destination);
    mind_malloc (varname, "reaction{2}", &SReactions[destination]->conditions,
      React_Head_NumConditions_Get (React_Head_Get (SReactions[source])) * CONDITIONSIZE);
    memcpy (SReactions[destination]->conditions, SReactions[source]->conditions,
      React_Head_NumConditions_Get (React_Head_Get (SReactions[source])) *
      CONDITIONSIZE);
    }

  if (React_Head_NumTests_Get (React_Head_Get (SReactions[source])) == 0)
    {
    SReactions[destination]->tests = NULL;
    React_Text_Get (SReactions[destination])->reasons = NULL;
    React_Text_Get (SReactions[destination])->chemists = NULL;
    }
  else
    {
    sprintf (varname, "SReactions[%d]->tests", destination);
    mind_malloc (varname, "reaction{2}", &SReactions[destination]->tests,
      React_Head_NumTests_Get (React_Head_Get (SReactions[source])) * POSTTESTSIZE);
    memcpy (SReactions[destination]->tests, SReactions[source]->tests,
      React_Head_NumTests_Get (React_Head_Get (SReactions[source])) *
      POSTTESTSIZE);
    sprintf (varname, "React_Text_Get(SReactions[%d])->reasons", destination);
    mind_malloc (varname, "reaction{2}", &React_Text_Get (SReactions[destination])->reasons,
      React_Head_NumTests_Get (React_Head_Get (SReactions[source])) * STRINGSIZE);
    sprintf (varname, "React_Text_Get(SReactions[%d])->chemists", destination);
    mind_malloc (varname, "reaction{2}", &React_Text_Get (SReactions[destination])->chemists,
      React_Head_NumTests_Get (React_Head_Get (SReactions[source])) * STRINGSIZE);
    }
  for (i = 0; i < React_Head_NumTests_Get (
    React_Head_Get (SReactions[source])); i++)
    {
    sprintf (varname, "Post_OpHandle_Get(React_Test_Get(SReactions[%d],%d))", destination, i);
    mind_malloc (varname, "reaction{2}", &Post_OpHandle_Get (React_Test_Get (SReactions[destination], i)),
      Post_Length_Get (React_Test_Get (SReactions[source], i)));
    memcpy (Post_OpHandle_Get (React_Test_Get (SReactions[destination], i)),
      Post_OpHandle_Get (React_Test_Get (SReactions[source], i)),
      Post_Length_Get (React_Test_Get (SReactions[source], i)));
    React_TxtRec_Reason_Put (React_Text_Get (SReactions[destination]), i,
      String_Copy (React_TxtRec_Reason_Get (React_Text_Get (SReactions[source]), i)));
    React_TxtRec_Chemist_Put (React_Text_Get (SReactions[destination]), i,
      String_Copy (React_TxtRec_Chemist_Get (React_Text_Get (SReactions[source]), i)));
    }
#else
  if (destination < 0 || destination > React_NumSchemas_Get () ||
    SReactions[destination] != NULL) return (FALSE);

  SReactions[destination] = (React_Record_t *) malloc (REACTRECSIZE);
  memcpy (SReactions[destination], SReactions[source], REACTRECSIZE);

  React_Goal_Put (SReactions[destination], (Tsd_t *) malloc (TSDSIZE));
  memcpy (React_Goal_Get (SReactions[destination]),
    React_Goal_Get (SReactions[source]), TSDSIZE);
  Tsd_AtomHandle_Put (React_Goal_Get (SReactions[destination]),
    (TsdRow_t *) malloc (Tsd_NumAtoms_Get (React_Goal_Get (SReactions[source]))
    * TSDROWSIZE));
  memcpy (Tsd_AtomHandle_Get (React_Goal_Get (SReactions[destination])),
    Tsd_AtomHandle_Get (React_Goal_Get (SReactions[source])),
    Tsd_NumAtoms_Get (React_Goal_Get (SReactions[source])) * TSDROWSIZE);

  React_Subgoal_Put (SReactions[destination], (Tsd_t *) malloc (TSDSIZE));
  memcpy (React_Subgoal_Get (SReactions[destination]),
    React_Subgoal_Get (SReactions[source]), TSDSIZE);
  Tsd_AtomHandle_Put (React_Subgoal_Get (SReactions[destination]),
    (TsdRow_t *) malloc (Tsd_NumAtoms_Get
    (React_Subgoal_Get (SReactions[source])) * TSDROWSIZE));
  memcpy (Tsd_AtomHandle_Get (React_Subgoal_Get (SReactions[destination])),
    Tsd_AtomHandle_Get (React_Subgoal_Get (SReactions[source])),
    Tsd_NumAtoms_Get (React_Subgoal_Get (SReactions[source])) * TSDROWSIZE);

  React_Text_Put (SReactions[destination],
    (React_TextRec_t *) malloc (REACTEXTRECSIZE));
  memcpy (React_TxtRec_Head_Get (React_Text_Get (SReactions[destination])),
    React_TxtRec_Head_Get (React_Text_Get (SReactions[source])), REACTEXTHEADSIZE);
  React_TxtRec_Name_Put (React_Text_Get (SReactions[destination]),
    String_Copy (React_TxtRec_Name_Get (React_Text_Get (SReactions[source]))));

  if (React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))) == 0)
    React_Text_Get (SReactions[destination])->comments = NULL;
  else React_Text_Get (SReactions[destination])->comments =
    (String_t *) malloc (React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))) * STRINGSIZE);
  for (i = 0; i < React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))); i++)
    React_TxtRec_Comment_Put (React_Text_Get (SReactions[destination]), i,
    String_Copy (React_TxtRec_Comment_Get (React_Text_Get (SReactions[source]), i)));

  if (React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))) == 0)
    React_Text_Get (SReactions[destination])->lit_refs = NULL;
  else React_Text_Get (SReactions[destination])->lit_refs =
    (String_t *) malloc (React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))) * STRINGSIZE);
  for (i = 0; i < React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[source]))); i++)
    React_TxtRec_Reference_Put (React_Text_Get (SReactions[destination]), i,
    String_Copy (React_TxtRec_Reference_Get (React_Text_Get (SReactions[source]), i)));

  if (React_Head_NumConditions_Get (React_Head_Get (SReactions[source])) == 0)
    SReactions[destination]->conditions = NULL;
  else
    {
    SReactions[destination]->conditions =
      (Condition_t *) malloc (React_Head_NumConditions_Get (
      React_Head_Get (SReactions[source])) * CONDITIONSIZE);
    memcpy (SReactions[destination]->conditions, SReactions[source]->conditions,
      React_Head_NumConditions_Get (React_Head_Get (SReactions[source])) *
      CONDITIONSIZE);
    }

  if (React_Head_NumTests_Get (React_Head_Get (SReactions[source])) == 0)
    {
    SReactions[destination]->tests = NULL;
    React_Text_Get (SReactions[destination])->reasons = NULL;
    React_Text_Get (SReactions[destination])->chemists = NULL;
    }
  else
    {
    SReactions[destination]->tests =
      (Posttest_t *) malloc (React_Head_NumTests_Get (
      React_Head_Get (SReactions[source])) * POSTTESTSIZE);
    memcpy (SReactions[destination]->tests, SReactions[source]->tests,
      React_Head_NumTests_Get (React_Head_Get (SReactions[source])) *
      POSTTESTSIZE);
    React_Text_Get (SReactions[destination])->reasons =
      (String_t *) malloc (React_Head_NumTests_Get (
      React_Head_Get (SReactions[source])) * STRINGSIZE);
    React_Text_Get (SReactions[destination])->chemists =
      (String_t *) malloc (React_Head_NumTests_Get (
      React_Head_Get (SReactions[source])) * STRINGSIZE);
    }
  for (i = 0; i < React_Head_NumTests_Get (
    React_Head_Get (SReactions[source])); i++)
    {
    Post_OpHandle_Get (React_Test_Get (SReactions[destination], i)) =
      (U8_t *) malloc (Post_Length_Get
      (React_Test_Get (SReactions[source], i)));
    memcpy (Post_OpHandle_Get (React_Test_Get (SReactions[destination], i)),
      Post_OpHandle_Get (React_Test_Get (SReactions[source], i)),
      Post_Length_Get (React_Test_Get (SReactions[source], i)));
    React_TxtRec_Reason_Put (React_Text_Get (SReactions[destination]), i,
      String_Copy (React_TxtRec_Reason_Get (React_Text_Get (SReactions[source]), i)));
    React_TxtRec_Chemist_Put (React_Text_Get (SReactions[destination]), i,
      String_Copy (React_TxtRec_Chemist_Get (React_Text_Get (SReactions[source]), i)));
    }
#endif

  React_Head_SchemaId_Put (React_Head_Get (SReactions[destination]),
    destination);

  return(TRUE);
}

Boolean_t React_Schema_Allocated (int schnum)
{
  return (SReactions[schnum] != NULL);
}

void React_Schema_Free (int schnum)
{
  int i;
#ifdef _MIND_MEM_
  char varname[100];

  if (React_Goal_Get (SReactions[schnum]) != NULL)
    {
    sprintf (varname, "Tsd_AtomHandle_Get(React_Goal_Get(SReactions[%d]))", schnum);
    mind_free (varname, "reaction", Tsd_AtomHandle_Get (React_Goal_Get (SReactions[schnum])));
    sprintf (varname, "React_Goal_Get(SReactions[%d])", schnum);
    mind_free (varname, "reaction", React_Goal_Get (SReactions[schnum]));
    }

  if (React_Subgoal_Get (SReactions[schnum]) != NULL)
    {
    sprintf (varname, "Tsd_AtomHandle_Get(React_Subgoal_Get(SReactions[%d]))", schnum);
    mind_free (varname, "reaction", Tsd_AtomHandle_Get (React_Subgoal_Get (SReactions[schnum])));
    sprintf (varname, "React_Suboal_Get(SReactions[%d])", schnum);
    mind_free (varname, "reaction", React_Subgoal_Get (SReactions[schnum]));
    }

  String_Destroy (React_TxtRec_Name_Get (
    React_Text_Get (SReactions[schnum])));

  for (i = 0; i < React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))); i++)
    String_Destroy (React_TxtRec_Comment_Get (
    React_Text_Get (SReactions[schnum]), i));

  if (React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))) != 0)
    {
    sprintf (varname, "React_Text_Get(SReactions[%d])->comments", schnum);
    mind_free (varname, "reaction", React_Text_Get (SReactions[schnum])->comments);
    }

  for (i = 0; i < React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))); i++)
    String_Destroy (React_TxtRec_Reference_Get (
    React_Text_Get (SReactions[schnum]), i));

  if (React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))) != 0)
    {
    sprintf (varname, "React_Text_Get(SReactions[%d])->lit_refs", schnum);
    mind_free (varname, "reaction", React_Text_Get (SReactions[schnum])->lit_refs);
    }

  for (i = 0; i < React_Head_NumTests_Get (
    React_Head_Get (SReactions[schnum])); i++)
    {
    sprintf (varname, "Post_OpHandle_Get(React_Test_Get(SReactions[%d],%d))", schnum, i);
    mind_free (varname, "reaction", Post_OpHandle_Get (React_Test_Get (SReactions[schnum], i)));
    String_Destroy (React_TxtRec_Reason_Get (
      React_Text_Get (SReactions[schnum]), i));
    String_Destroy (React_TxtRec_Chemist_Get (
      React_Text_Get (SReactions[schnum]), i));
    }

  if (React_Head_NumTests_Get (
    React_Head_Get (SReactions[schnum])) != 0)
    {
    sprintf (varname, "React_Text_Get(SReactions[%d])->reasons", schnum);
    mind_free (varname, "reaction", React_Text_Get (SReactions[schnum])->reasons);
    sprintf (varname, "React_Text_Get(SReactions[%d])->chemists", schnum);
    mind_free (varname, "reaction", React_Text_Get (SReactions[schnum])->chemists);
    }

  if (SReactions[schnum]->conditions != NULL)
    {
    sprintf (varname, "SReactions[%d]->conditions", schnum);
    mind_free (varname, "reaction", SReactions[schnum]->conditions);
    }
  if (SReactions[schnum]->tests != NULL)
    {
    sprintf (varname, "SReactions[%d]->tests", schnum);
    mind_free (varname, "reaction", SReactions[schnum]->tests);
    }
  sprintf (varname, "React_Text_Get(SReactions[%d])", schnum);
  mind_free (varname, "reaction", React_Text_Get (SReactions[schnum]));
  sprintf (varname, "SReactions[%d]", schnum);
  mind_free (varname, "reaction", SReactions[schnum]);
#else

  if (React_Goal_Get (SReactions[schnum]) != NULL)
    {
    free (Tsd_AtomHandle_Get (React_Goal_Get (SReactions[schnum])));
    free (React_Goal_Get (SReactions[schnum]));
    }

  if (React_Subgoal_Get (SReactions[schnum]) != NULL)
    {
    free (Tsd_AtomHandle_Get (React_Subgoal_Get (SReactions[schnum])));
    free (React_Subgoal_Get (SReactions[schnum]));
    }

  String_Destroy (React_TxtRec_Name_Get (
    React_Text_Get (SReactions[schnum])));

  for (i = 0; i < React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))); i++)
    String_Destroy (React_TxtRec_Comment_Get (
    React_Text_Get (SReactions[schnum]), i));

  if (React_TxtHd_NumComments_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))) != 0)
    free (React_Text_Get (SReactions[schnum])->comments);

  for (i = 0; i < React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))); i++)
    String_Destroy (React_TxtRec_Reference_Get (
    React_Text_Get (SReactions[schnum]), i));

  if (React_TxtHd_NumReferences_Get (React_TxtRec_Head_Get (
    React_Text_Get (SReactions[schnum]))) != 0)
    free (React_Text_Get (SReactions[schnum])->lit_refs);

  for (i = 0; i < React_Head_NumTests_Get (
    React_Head_Get (SReactions[schnum])); i++)
    {
    free (Post_OpHandle_Get (React_Test_Get (SReactions[schnum], i)));
    String_Destroy (React_TxtRec_Reason_Get (
      React_Text_Get (SReactions[schnum]), i));
    String_Destroy (React_TxtRec_Chemist_Get (
      React_Text_Get (SReactions[schnum]), i));
    }

  if (React_Head_NumTests_Get (
    React_Head_Get (SReactions[schnum])) != 0)
    {
    free (React_Text_Get (SReactions[schnum])->reasons);
    free (React_Text_Get (SReactions[schnum])->chemists);
    }

  if (SReactions[schnum]->conditions != NULL) free (SReactions[schnum]->conditions);
  if (SReactions[schnum]->tests != NULL) free (SReactions[schnum]->tests);
  free (React_Text_Get (SReactions[schnum]));
  free (SReactions[schnum]);
#endif

  SReactions[schnum] = NULL;
}

void React_Schema_Write (U32_t schnum)
{
  U32_t num_schemas;
  int i, new_alloc;

  num_schemas = (U32_t) React_NumSchemas_Get ();
  if (schnum < 0 || schnum > num_schemas)
    {
    printf("Invalid schema number in React_Schema_Write: %d\n", schnum);
    return;
    }
  React_Write (SReactions[schnum], &SReactDataFile);
  Isam_Flush (&SReactDataFile);
  React_TextWrite (SReactions[schnum], &SReactTextFile);
  Isam_Flush (&SReactTextFile);
  if (schnum == curr_schemata_allocated - 1)
    {
    new_alloc = (3 * curr_schemata_allocated + 1) / 2;
#ifdef _MIND_MEM_
    mind_realloc ("SReactions", "reaction{3}", &SReactions, SReactions, new_alloc * sizeof (void *));
#else
    SReactions = realloc (SReactions, new_alloc * sizeof (void *));
#endif
    for (i = curr_schemata_allocated; i < new_alloc; i++)
      SReactions[i] = NULL;
    curr_schemata_allocated = new_alloc;
    }
}

void React_Close ()
{
  Isam_Close (&SReactDataFile);
  Isam_Close (&SReactTextFile);
}

/****************************************************************************
*
*  Function Name:                 React_NumSchemas_Get
*
*    This routine returns the number of schemas available to this
*    computation.
*
*  Used to be:
*
*    num_schemas:
*
*  Implicit Inputs:
*
*    SReactDataFile - reaction data file
*
*  Implicit Outputs:
*
*    N/A
*
*  Return Values:
*
*    Next valid key in data file
*
*  Side Effects:
*
*    N/A
*
******************************************************************************/
U32_t React_NumSchemas_Get
  (
  )
{
  return Isam_NextKey_Get (&SReactDataFile);
}

/****************************************************************************
*
*  Function Name:                 React_Schema_Handle_Get
*
*    This function returns a handle from the module-level static storage
*    for the reaction library to the caller.  This gives an ADT flavor.
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
React_Record_t *React_Schema_Handle_Get
  (
  U32_t          schema                     /* Schema index to check */
  )
{
  return   SReactions[schema];
}

/****************************************************************************
*
*  Function Name:                 React_Schema_Handle_Set
*
*    This function creates a handle in the module-level static storage
*    for the reaction library.
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
React_Record_t *React_Schema_Handle_Set
  (
  U32_t          schema                     /* Schema index to check */
  )
{
#ifdef _MIND_MEM_
  char varname[100];

  sprintf (varname, "SReactions[%d]", schema);
  mind_malloc (varname, "reaction{4}", SReactions + schema, REACTRECSIZE);
#else
  Mem_Alloc (React_Record_t *, SReactions[schema], REACTRECSIZE, GLOBAL);
#endif
}

/****************************************************************************
*
*  Function Name:                 React_Schema_IsOk
*
*     This function checks whether a given reaction schema may be
*     applied in a valid fashion to a given molecule.  The top set of
*     checks are drawn from the old CHAPOK routine.  The represent
*     limitations based on the old chapter breakdown of the reaction 
*     library.  The second set of checks represent the pre-transform
*     tests.  At the moment there are a few Disabled or Incomplete
*     reactions in the library, eventually there should be none as
*     they can be eliminated before runtime.  This routine also rejects
*     Protection reactions as they should be included when a proper
*     strategic approach is determined for handling protection.
*
*     Eventually this routine should go away.  The idea is to develope
*     a decision tree that will combine the rules encoded here with
*     all the reaction pre-transform tests to produce a routine that
*     can accept a molecule and return a prioritized list of reaction
*     schemas to apply.
*
*  Used to be:
*
*    schemok, chapok:
*
*  Implicit Inputs:
*
*    SReactions - in memory table of reactions
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
Boolean_t React_Schema_IsOk_Buffer
  (
  Xtr_t         *xtr_p,
  U32_t          schema,
  char          *buffer
  )
{
  Boolean_t      is_ok;

  if (schema == PER_DELETED)
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Bypassing deleted schema"));
    sprintf (rxn_buffer, "Bypassing deleted schema");
    strcpy (buffer, rxn_buffer);

    return FALSE;
    }

  is_ok = React_Schema_IsOk (xtr_p, (U32_t)(-1 - schema));

  if (!is_ok) strcpy (buffer, rxn_buffer);
  else buffer[0] = '\0';

  return (is_ok);
}

Boolean_t React_Schema_IsOk
  (
  Xtr_t         *xtr_p,                     /* Compound to check */
  U32_t          schema_index               /* Schema index to check */
  )  
{
  React_Record_t *react_p;                  /* Schema handle */
  React_Head_t  *head_p;                    /* Reaction header handle */
  FuncGroups_t  *funcgrp_p;                 /* FG data-struct handle */
  U32_t          schema;
  U16_t          rxn_funcgrp;               /* Syntheme FG # */
  U16_t          i;                         /* Counter */
  Boolean_t      flag;                      /* Flag for loop termination */
  Boolean_t      buffer_trace;

  if (schema == PER_DELETED)
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Bypassing deleted schema"));
    sprintf (rxn_buffer, "Bypassing deleted schema");

    return FALSE;
    }

  if ((int)schema_index < 0)
    {
    schema = -1 - (int)schema_index;
    buffer_trace = TRUE;
    }
  else
    {
    schema = schema_index;
    buffer_trace = FALSE;
    }

  DEBUG (R_REACTION, DB_REACTSCHEMAOK, TL_PARAMS, (outbuf,
    "Entering React_Schema_IsOk, Xtr = %p, schema = %u", xtr_p, schema));

  /* First perform the checks based on the original chapter.
     The tests will be reordered so that they fail-out early.
  */

  react_p = React_Schema_Handle_Get (schema);
  head_p = React_Head_Get (react_p);
  rxn_funcgrp = React_Head_SynthemeFG_Get (head_p);
  funcgrp_p = Xtr_FuncGroups_Get (xtr_p);

  if (FuncGroups_Substructure_Exists (funcgrp_p, rxn_funcgrp) == FALSE)
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Schema %u rejected - no syntheme root for reaction", schema));
    if (buffer_trace) sprintf (rxn_buffer,
      "Schema %u rejected - no syntheme root for reaction", schema);

    return FALSE;
    }

  /* Certain types of reagents require special chemistry:
     - Peroxides, Arynes, Grignards (Organomagnesium?), Organolithium,
       Carbenes, Nitrenes
     - Ketene only with C-C double bond reactions
     - Epoxide only with Ether reactions
     - Hemiacteal only with Alcohol reactions
     - Anhydride only with Carboxylic Ester reactions
     - Acyl Halide only with Halogen reactions
     - Haloformate only with Halogen reactions
     - Hypohalite only with Halogen reactions
     - Diazonium or Diazo only with N-N double or triple bond reactions
     - Isocyanate only with C-N double bond reactions
     - Thioacid only with Organosulfur reactions
     - Isothiocyanate only with C-N double bond or Organosulfur reactions
     - Enethiol only with C-C double bond or Organosulfur reactions
     - Episulfide and Thione only with Organosulfur reactions
     - Sulfenyl and Sulfinyl Halide only with Halogen or Organosulfur reactions
     - Aziridine and Diaziridine only with C-N single bond reactions
     - Isonitrile only with C-N triple bond reactions
     - Carbodiimide only with C-N double bond reactions
     - Ensure that only compounds with Aldehydes or Ketones use Carbonyl
       reactions
     - Ensure there is an independent C-N single bond and that it is not a
       sub-substructure of a Carboxylic Amide
     - (tests similiar to the above but for Alcohol vs Carboxylic Acid and
       Carbonyl and Carboxylic Ester were eliminated because FuncGroups_Find
       does this already)
  */

  if ((FuncGroups_Substructure_Exists (funcgrp_p, PEROXIDE) == TRUE &&
      rxn_funcgrp != PEROXIDE) ||
      (FuncGroups_Substructure_Exists (funcgrp_p, ARYNE) == TRUE &&
      rxn_funcgrp != ARYNE) ||
      (FuncGroups_Substructure_Exists (funcgrp_p, ORGANOMAGNESIUM) == TRUE &&
      rxn_funcgrp != ORGANOMAGNESIUM) ||
      (FuncGroups_Substructure_Exists (funcgrp_p, ORGANOLITHIUM) == TRUE &&
      rxn_funcgrp != ORGANOLITHIUM) ||
      (FuncGroups_Substructure_Exists (funcgrp_p, CARBENE_NITRENE) == TRUE &&
      rxn_funcgrp != CARBENE_NITRENE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Target molecule contains functional group requiring specific chemistry,"
      " and schema %u is not suitable", schema));
    if (buffer_trace) sprintf (rxn_buffer,
      "Target molecule contains functional group requiring specific chemistry,"
      " and schema %u is not suitable", schema);

    return FALSE;
    }

  if (rxn_funcgrp != ALCOHOL &&
      (FuncGroups_Substructure_Exists (funcgrp_p, HEMIACETAL) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Hemiacetal groups only with Alcohol reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Hemiacetal groups only with Alcohol reactions");

    return FALSE;
    }

  if (rxn_funcgrp != CC_DOUBLE &&
      (FuncGroups_Substructure_Exists (funcgrp_p, KETENE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Ketene groups only with C-C double bond reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Ketene groups only with C-C double bond reactions");

    return FALSE;
    }

  if (rxn_funcgrp != ETHER &&
      (FuncGroups_Substructure_Exists (funcgrp_p, EPOXIDE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Epoxide groups only with Ether reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Epoxide groups only with Ether reactions");

    return FALSE;
    }

  /* From the CHAPOK.PLI comments: use pre-transform tests to rule out 
     Anhydrides ...
  */

  if (rxn_funcgrp != CARBOXYLIC_ESTER &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ANHYDRIDE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Anhydride groups only with Carboxylic Ester reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Anhydride groups only with Carboxylic Ester reactions");

    return FALSE;
    }

  if (rxn_funcgrp != HALOGEN &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ACYL_HALIDE) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, HALOFORMATE) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, HYPOHALITE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Acyl Halide, Haloformate, Hypohalite groups only with"
      " Halogen reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Acyl Halide, Haloformate, Hypohalite groups only with"
      " Halogen reactions");

    return FALSE;
    }

  if (rxn_funcgrp != CN_SINGLE &&
      (FuncGroups_Substructure_Exists (funcgrp_p, AZIRIDINE) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, DIAZIRIDINE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Aziridine and Diaziridine groups only with C-N single bond reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Aziridine and Diaziridine groups only with C-N single bond reactions");

    return FALSE;
    }

  if (rxn_funcgrp != CN_DOUBLE &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ISOCYANATE) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, CARBODIIMIDE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Isocyanate, Carbodiimide groups only with C-N double bond reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Isocyanate, Carbodiimide groups only with C-N double bond reactions");

    return FALSE;
    }

  if (rxn_funcgrp != CN_TRIPLE &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ISONITRILE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Isonitrile groups only with C-N triple bond reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Isonitrile groups only with C-N triple bond reactions");

    return FALSE;
    }

  if (rxn_funcgrp != NN_DOUBLE_TRIPLE &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ARYL_DIAZONIUM) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, DIAZO) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Aryl Diazonium, Diazo only with N-N double or triple bond reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Aryl Diazonium, Diazo only with N-N double or triple bond reactions");

    return FALSE;
    }

  if (rxn_funcgrp != ORGANOSULFUR &&
      (FuncGroups_Substructure_Exists (funcgrp_p, THIOACID) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, EPISULFIDE) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, THIONE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Thioacid, Episulfide, Thione groups only with Organosulfur reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Thioacid, Episulfide, Thione groups only with Organosulfur reactions");

    return FALSE;
    }

  /* Some Func. Groups can go with two chapters rather than one */

  if ((rxn_funcgrp != CC_DOUBLE && rxn_funcgrp != ORGANOSULFUR) &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ENETHIOL) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Enethiol groups only with C-C double bond or Organosulfur reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Enethiol groups only with C-C double bond or Organosulfur reactions");

    return FALSE;
    }

  if ((rxn_funcgrp != HALOGEN && rxn_funcgrp != ORGANOSULFUR) &&
      (FuncGroups_Substructure_Exists (funcgrp_p, SULFENYL_HALIDE) == TRUE ||
      FuncGroups_Substructure_Exists (funcgrp_p, SULFINYL_HALIDE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Sulfenyl and Sulfinyl Halide groups only with Halogen or"
      " Organosulfur reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Sulfenyl and Sulfinyl Halide groups only with Halogen or"
      " Organosulfur reactions");

    return FALSE;
    }

  if ((rxn_funcgrp != CN_DOUBLE && rxn_funcgrp != ORGANOSULFUR) &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ISOTHIOCYANATE) == TRUE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Isothiocyanate groups only with C-N double bond or"
      " Organosulfur reactions"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Isothiocyanate groups only with C-N double bond or"
      " Organosulfur reactions");

    return FALSE;
    }

  /* Below are a positive tests for two chapters */

  if (rxn_funcgrp == CARBONYL &&
      (FuncGroups_Substructure_Exists (funcgrp_p, ALDEHYDE) == FALSE &&
      FuncGroups_Substructure_Exists (funcgrp_p, KETONE) == FALSE &&
      FuncGroups_Substructure_Exists (funcgrp_p, TRIMETHYLSILYL_ETHER) == FALSE))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Aldehyde or Ketone group needed for Carbonyl reaction to go"));
    if (buffer_trace) sprintf (rxn_buffer,
      "Aldehyde or Ketone group needed for Carbonyl reaction to go");

    return FALSE;
    }

  if (rxn_funcgrp == CN_SINGLE &&
      (FuncGrp_SubstructureCount_Get (funcgrp_p, CN_SINGLE) == 
      FuncGrp_SubstructureCount_Get (funcgrp_p, CARBOXYLIC_AMIDE)))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "All C-N single bonds tied up in Carboxylic Acid groups so C-N"
      " single bond reaction won't go"));
    if (buffer_trace) sprintf (rxn_buffer,
      "All C-N single bonds tied up in Carboxylic Acid groups so C-N"
      " single bond reaction won't go");

    return FALSE;
    }

  /* If we have reached this point, then the reaction schema and the
     goal compound have passed the chapok tests.

     - Check that schema is complete, enabled, and not a PROTECTION reaction
     - Check can't have any (has protection failure limit of 2)
     - Check must have any
     - Check can't have all
     - Check must have all (has projection failure limit of 4)
  */

  /*
    At this stage, Auto protection is assume off 
  */

/*
  if (React_HeadFlags_Disabled_Get (head_p) == TRUE ||
      React_HeadFlags_Incomplete_Get (head_p) == TRUE ||
      React_HeadFlags_Protection_Get (head_p) == TRUE)
*/
  if (React_HeadFlags_Disabled_Get (head_p) == TRUE ||
      (React_HeadFlags_Incomplete_Get (head_p) == TRUE && !bypass_incomplete_flag))
    {
    TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
      "Schema %u is disabled, incomplete or a protection reaction", schema));
    if (buffer_trace) sprintf (rxn_buffer,
      "Schema %u is disabled, incomplete or a protection reaction", schema);

    return FALSE;
    }

  if (React_HeadFlags_CantAny_Get (head_p) == TRUE)
    {
    for (i = 0; i < MX_FUNCGROUPS; i++)
      if (FuncGroups_Substructure_Exists (funcgrp_p, i) == TRUE &&
          React_CantAny_Get (react_p, i) == TRUE)
        {
        TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
          "Schema %u failed Can't-Have-Any test for %u group", schema, i));
        if (buffer_trace) sprintf (rxn_buffer,
          "Schema %u failed Can't-Have-Any test for %u group", schema, i);

        return FALSE;
        }
    }

  if (React_HeadFlags_MustAny_Get (head_p) == TRUE)
    {
    for (flag = FALSE,i = 0; i < MX_FUNCGROUPS && flag == FALSE; i++)
      if (FuncGroups_Substructure_Exists (funcgrp_p, i) == TRUE &&
          React_MustAny_Get (react_p, i) == TRUE)
        flag = TRUE;

    if (flag == FALSE)
      {
      TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
        "Schema %u failed Must-Have-Any test", schema));
      if (buffer_trace) sprintf (rxn_buffer,
        "Schema %u failed Must-Have-Any test", schema);

      return FALSE;
      }
    }

  if (React_HeadFlags_CantAll_Get (head_p) == TRUE)
    {
    for (flag = TRUE,i = 0; i < MX_FUNCGROUPS && flag == TRUE; i++)
      if (FuncGroups_Substructure_Exists (funcgrp_p, i) == FALSE &&
          React_CantAll_Get (react_p, i) == TRUE)
        flag = FALSE;

    if (flag == TRUE)
      {
      TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
        "Schema %u failed Can't-Have-All test", schema));
      if (buffer_trace) sprintf (rxn_buffer,
        "Schema %u failed Can't-Have-All test", schema);

      return FALSE;
      }
    }

  if (React_HeadFlags_MustAll_Get (head_p) == TRUE)
    {
    for (i = 0; i < MX_FUNCGROUPS; i++)
      if (FuncGroups_Substructure_Exists (funcgrp_p, i) == FALSE &&
          React_MustAll_Get (react_p, i) == TRUE)
        {
        TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
          "Schema %u failed Must-Have-All test for %u group", schema, i));
        if (buffer_trace) sprintf (rxn_buffer,
          "Schema %u failed Must-Have-All test for %u group", schema, i);

        return FALSE;
        }
    }

  TRACE (R_REACTION, DB_CHEMISTRY, TL_DETAIL, (outbuf,
    "Schema %d passed all pre-transform tests", schema));

  DEBUG (R_REACTION, DB_REACTSCHEMAOK, TL_PARAMS, (outbuf,
    "Leaving React_Schema_Ok, status = TRUE"));

  return TRUE;
}
/* End of React_Schema_Ok */

Boolean_t React_Schema_Init (int destination, int library, int chapter, int sch_num, char *created_by)
{
  int i,
      day,
      hour,
      minute,
      second,
      year,
      month,
      num1,
      num2;
  React_Head_t *shead;
  React_TextRec_t *stxt;
  React_TextHead_t *sthead;
  Date_t cur_date;
  String_t create_string;
  time_t now;
  char date_str[32],
       day_of_week[8],
       month_name[8],
       creation[128],
       *sts,
       *tmp;
  static char *month_string = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";
  static U16_t SMonths[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304,
    334 };
#ifdef _MIND_MEM_
  char varname[100];
  void *varptr;

  if (destination < 0 || destination > React_NumSchemas_Get () ||
    SReactions[destination] != NULL) return (FALSE);

  sprintf (varname, "SReactions[%d]", destination);
  mind_malloc (varname, "reaction{5}", SReactions + destination, REACTRECSIZE);
#else

  if (destination < 0 || destination > React_NumSchemas_Get () ||
    SReactions[destination] != NULL) return (FALSE);

  SReactions[destination] = (React_Record_t *) malloc (REACTRECSIZE);
#endif
  shead = React_Head_Get (SReactions[destination]);

  React_Head_SchemaId_Put (shead, destination);
  React_Head_RootNode_Put (shead, 0, REACT_NODE_INVALID);
  React_Head_Flags_Put (shead, 0);
  React_HeadFlags_Incomplete_Put (shead, TRUE);
  React_Head_Ease_Put (shead, 0);
  React_Head_Yield_Put (shead, 0);
  React_Head_Confidence_Put (shead, 0);
  React_Head_NumConditions_Put (shead, 0);
  React_Head_NumTests_Put (shead, 0);
  React_Head_ReactionType_Put (shead, REACT_SING_APPL_ONLY);
  React_Head_MaxNonident_Put (shead, 1);
  React_Head_NumGoalAtoms_Put (shead, 0);
  React_Head_NumSubgoalAtoms_Put (shead, 0);
  React_Head_GoalSize_Put (shead, 0);
  React_Head_SynthemeFG_Put (shead, chapter);
  React_Head_Library_Put (shead, library);

  React_Goal_Put (SReactions[destination], NULL);
  React_Subgoal_Put (SReactions[destination], NULL);
  SReactions[destination]->conditions = NULL;
  SReactions[destination]->tests = NULL;

#ifdef _MIND_MEM_
  sprintf (varname, "React_Text_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{5}", &varptr, REACTEXTRECSIZE);
  React_Text_Put (SReactions[destination], (React_TextRec_t *) varptr);
#else
  React_Text_Put (SReactions[destination],
    (React_TextRec_t *) malloc (REACTEXTRECSIZE));
#endif
  stxt = React_Text_Get (SReactions[destination]);
  sthead = React_TxtRec_Head_Get (stxt);

  now = time (NULL);
  strcpy (date_str, ctime (&now));
  sscanf (date_str, "%s %s %d %d:%d:%d %d", day_of_week, month_name, &day, &hour, &minute, &second, &year);

  tmp = strstr (month_string, month_name);
  sts = month_string;
  num1 = day + SMonths[(tmp - sts) / 4];
  num2 = (year - 1900) * 365 + (year - 1900) / 4;  /* Hack */
  cur_date = num2 + num1;
  React_TxtHd_Created_Put (sthead, cur_date);
  React_TxtHd_LastMod_Put (sthead, (Date_t) 0);
  React_TxtHd_OrigSchema_Put (sthead, sch_num);
  React_TxtHd_NumComments_Put (sthead, 0);
  React_TxtHd_NumReferences_Put (sthead, 1);

  stxt->reasons = stxt->chemists = stxt->comments = NULL;
#ifdef _MIND_MEM_
  mind_malloc ("stxt->lit_refs", "reaction{5}", &stxt->lit_refs, sizeof (String_t));
  sprintf (creation, "\007Schema created by %s on %d/%d/%d at %02d%02d", created_by, (tmp - sts) / 4 + 1, day, year, hour, minute);
  create_string = String_Create ((const char *) creation, 0);
  React_TxtRec_Reference_Put (stxt, 0, create_string);
  React_TxtRec_Name_Put (stxt, String_Create ("", 0));

  sprintf (varname, "&SReactions[%d]->notany", destination);
  mind_Array_1d_Create (varname, "reaction{5}", &SReactions[destination]->notany, MX_FUNCGROUPS, BITSIZE);
  sprintf (varname, "&SReactions[%d]->notall", destination);
  mind_Array_1d_Create (varname, "reaction{5}", &SReactions[destination]->notall, MX_FUNCGROUPS, BITSIZE);
  sprintf (varname, "&SReactions[%d]->mustall", destination);
  mind_Array_1d_Create (varname, "reaction{5}", &SReactions[destination]->mustall, MX_FUNCGROUPS, BITSIZE);
  sprintf (varname, "&SReactions[%d]->mustany", destination);
  mind_Array_1d_Create (varname, "reaction{5}", &SReactions[destination]->mustany, MX_FUNCGROUPS, BITSIZE);
#else
  stxt->lit_refs = (String_t *) malloc (sizeof (String_t));
  sprintf (creation, "\007Schema created by %s on %d/%d/%d at %02d%02d", created_by, (tmp - sts) / 4 + 1, day, year, hour, minute);
  create_string = String_Create ((const char *) creation, 0);
  React_TxtRec_Reference_Put (stxt, 0, create_string);
  React_TxtRec_Name_Put (stxt, String_Create ("", 0));

  Array_1d_Create (&SReactions[destination]->notany, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&SReactions[destination]->notall, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&SReactions[destination]->mustall, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&SReactions[destination]->mustany, MX_FUNCGROUPS, BITSIZE);
#endif
  for (i = 0; i < MX_FUNCGROUPS; i++)
    {
    React_CantAll_Put (SReactions[destination], i, FALSE);
    React_CantAny_Put (SReactions[destination], i, FALSE);
    React_MustAll_Put (SReactions[destination], i, FALSE);
    React_MustAny_Put (SReactions[destination], i, FALSE);
    }

  return(TRUE);
}

void React_Schema_Export (int source, char *buffer)
{
  int               i, j;
  unsigned char     op;
  char              line[1024],
                    test_expr[512];
  React_Record_t   *schema;
  React_Head_t     *sch_head;
  React_TextRec_t  *text;
  React_TextHead_t *txt_head;
  Tsd_t            *goal,
                   *subgoal;
  String_t          condstr;
  Posttest_t       *test;
  static char      *type[] = {"SAO", "MPS", "MNP", "MPM", "MAO"};

  schema = SReactions[source];
  sch_head = React_Head_Get (schema);
  text = React_Text_Get (schema);
  txt_head = React_TxtRec_Head_Get (text);
  goal = React_Goal_Get (schema);
  subgoal = React_Subgoal_Get (schema);

  sprintf (buffer, "%06d (%06d): %02d/%03d/%06d (0x%02x)\n", source, React_Head_SchemaId_Get (sch_head),
    React_Head_Library_Get (sch_head), React_Head_SynthemeFG_Get (sch_head), React_TxtHd_OrigSchema_Get (txt_head),
    React_Head_Flags_Get (sch_head));
  Sprintf (line, React_TxtRec_Name_Get (text));
  strcat (buffer, line);
  sprintf (line, "Cre: 0x%08x; Mod: 0x%08x\n", React_TxtHd_Created_Get (txt_head), React_TxtHd_LastMod_Get (txt_head));
  strcat (buffer, line);
  sprintf (line, "%03d %03d %03d %s %d\n", React_Head_Ease_Get (sch_head), React_Head_Yield_Get (sch_head),
    React_Head_Confidence_Get (sch_head), type[React_Head_ReactionType_Get (sch_head) - REACT_SING_APPL_ONLY],
    React_Head_MaxNonident_Get (sch_head));
  strcat (buffer, line);

  sprintf (line, "%02d Com:\n", React_TxtHd_NumComments_Get (txt_head));
  strcat (buffer, line);
  for (i = 0; i < React_TxtHd_NumComments_Get (txt_head); i++)
    {
    strcat (buffer, "\t");
    Sprintf (line, React_TxtRec_Comment_Get (text, i));
    strcat (buffer, line);
    }

  sprintf (line, "%02d Ref:\n", React_TxtHd_NumReferences_Get (txt_head));
  strcat (buffer, line);
  for (i = 0; i < React_TxtHd_NumReferences_Get (txt_head); i++)
    {
    strcat (buffer, "\t");
    Sprintf (line, React_TxtRec_Reference_Get (text, i));
    strcat (buffer, line);
    }

  strcpy (line, "CL: ");
  for (j = Array_Columns_Get (&schema->notall); j > 0 && !React_CantAll_Get (schema, j - 1); j--);
  for (i = 0; i < j; i++) line[i + 4] = React_CantAll_Get (schema, i) ? '1' : '0';
  line[i + 4] = '\0';
  strcat (line, "\n");
  strcat (buffer, line);

  strcpy (line, "CN: ");
  for (j = Array_Columns_Get (&schema->notany); j > 0 && !React_CantAny_Get (schema, j - 1); j--);
  for (i = 0; i < j; i++) line[i + 4] = React_CantAny_Get (schema, i) ? '1' : '0';
  line[i + 4] = '\0';
  strcat (line, "\n");
  strcat (buffer, line);

  strcpy (line, "ML: ");
  for (j = Array_Columns_Get (&schema->mustall); j > 0 && !React_MustAll_Get (schema, j - 1); j--);
  for (i = 0; i < j; i++) line[i + 4] = React_MustAll_Get (schema, i) ? '1' : '0';
  line[i + 4] = '\0';
  strcat (line, "\n");
  strcat (buffer, line);

  strcpy (line, "MN: ");
  for (j = Array_Columns_Get (&schema->mustany); j > 0 && !React_MustAny_Get (schema, j - 1); j--);
  for (i = 0; i < j; i++) line[i + 4] = React_MustAny_Get (schema, i) ? '1' : '0';
  line[i + 4] = '\0';
  strcat (line, "\n");
  strcat (buffer, line);

  for (j = MX_ROOTS; j > 0 && React_Head_RootNode_Get (sch_head, j - 1) == REACT_NODE_INVALID; j--);
  for (i = 0; i < j; i++) sprintf (line + 8 * i, " %03d:%03d", React_Head_RootNode_Get (sch_head, i),
    React_Head_RootSyntheme_Get (sch_head, i));
  strcat (line, "\n");
  strcat (buffer, line);

  sprintf (line, "Goal: %03d (%03d) atoms\n", React_Head_NumGoalAtoms_Get (sch_head), React_Head_GoalSize_Get (sch_head));
  strcat (buffer, line);
  for (i = 0; i < React_Head_NumGoalAtoms_Get (sch_head); i++)
    {
    sprintf (line, "\t%03d (0x%02x)", Tsd_Atomid_Get (goal, i), Tsd_Atom_Flags_Get (goal, i));
    for (j = 0; j < MX_NEIGHBORS; j++)
      sprintf (line + 12 * j + 11, " 0x%04x:0x%02x", Tsd_Atom_NeighborId_Get (goal, i, j),
      Tsd_Atom_NeighborBond_Get (goal, i, j));
    line[12 * j + 10] = '\0';
    strcat (line, "\n");
    strcat (buffer, line);
    }

  sprintf (line, "Subgoal: %03d atoms\n", React_Head_NumSubgoalAtoms_Get (sch_head));
  strcat (buffer, line);
  for (i = 0; i < React_Head_NumSubgoalAtoms_Get (sch_head); i++)
    {
    sprintf (line, "\t%03d (0x%02x)", Tsd_Atomid_Get (subgoal, i), Tsd_Atom_Flags_Get (subgoal, i));
    for (j = 0; j < MX_NEIGHBORS; j++)
      sprintf (line + 12 * j + 11, " 0x%04x:0x%02x", Tsd_Atom_NeighborId_Get (subgoal, i, j),
      Tsd_Atom_NeighborBond_Get (subgoal, i, j));
    line[12 * j + 10] = '\0';
    strcat (line, "\n");
    strcat (buffer, line);
    }

  sprintf (line, "%03d Conds:\n", React_Head_NumConditions_Get (sch_head));
  strcat (buffer, line);
  for (i = 0; i < React_Head_NumConditions_Get (sch_head); i++)
    {
    condition_import (React_Condition_Get (schema, i), &condstr, NULL);
    sprintf (line, "\t%s\n", String_Value_Get (condstr));
    strcat (buffer, line);
    }

  sprintf (line, "%03d Tests:\n", React_Head_NumTests_Get (sch_head));
  strcat (buffer, line);
  for (i = 0; i < React_Head_NumTests_Get (sch_head); i++)
    {
    test = React_Test_Get (schema, i);
    for (j = test_expr[0] = 0; j < Post_Length_Get (test); j++)
      {
      op = Post_Op_Get (test, j);
      if (op < PT_TEST_ADD) sprintf (test_expr + strlen (test_expr), "C%02d", op + 1);
      else switch (op)
        {
      case OP_AND:
        strcat (test_expr, "&");
        break;
      case OP_OR:
        strcat (test_expr, "|");
        break;
      case OP_NOT:
        strcat (test_expr, "~");
        break;
      case OP_NOPASS:
        strcat (test_expr, "NOP");
        break;
      case BOOLOP_EQ:
        strcat (test_expr, "=");
        break;
      case BOOLOP_XOR:
        strcat (test_expr, "#");
        break;
      default:
        sprintf (test_expr + strlen (test_expr), "T%02d", op - PT_TEST_ADD + 1);
        break;
        }
      }
    sprintf (line, "\t%s %1d%+04d%+04d%+04d\n", test_expr, Post_Head_Get (test)->flags & 3, Post_EaseAdj_Get (test),
      Post_YieldAdj_Get (test), Post_ConfidenceAdj_Get (test));
    strcat (buffer, line);
    strcat (buffer, "\t\t");
    Sprintf (line, React_TxtRec_Reason_Get (text, i));
    strcat (buffer, line);
    strcat (buffer, "\t\t");
    Sprintf (line, React_TxtRec_Chemist_Get (text, i));
    strcat (buffer, line);
    }
  strcat (buffer, "\n");
}

Boolean_t React_Schema_Import (int destination, char *buffer)
{
  int               i, j, testnum, source[2], inputs[10];
  unsigned char     op;
  char              line[1024],
                    temp[4],
                    opstr[512],
                    test_expr[512],
                   *buf;
  React_Record_t   *schema;
  React_Head_t     *sch_head;
  React_TextRec_t  *text;
  React_TextHead_t *txt_head;
  Tsd_t            *goal,
                   *subgoal;
  String_t          condstr;
  Posttest_t       *test;
  static char      *type[] = {"SAO", "MPS", "MNP", "MPM", "MAO"};

#ifdef _MIND_MEM_
  char varname[100];
  void *varptr;

  if (destination < 0 || destination > React_NumSchemas_Get () ||
    SReactions[destination] != NULL) return (FALSE);

  sprintf (varname, "SReactions[%d]", destination);
  mind_malloc (varname, "reaction{6}", SReactions + destination, REACTRECSIZE);
  schema = SReactions[destination];
  sch_head = React_Head_Get (schema);
  sprintf (varname, "React_Text_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{6}", &varptr, REACTEXTRECSIZE);
  React_Text_Put (SReactions[destination], (React_TextRec_t *) varptr);
  text = React_Text_Get (schema);
  txt_head = React_TxtRec_Head_Get (text);
  sprintf (varptr, "React_Goal_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{6}", &varptr, TSDSIZE);
  React_Goal_Put (SReactions[destination], (Tsd_t *) varptr);
  goal = React_Goal_Get (schema);
  sprintf (varname, "React_Subgoal_Get(SReactions[%d])", destination);
  mind_malloc (varname, "reaction{6}", &varptr, TSDSIZE);
  React_Subgoal_Put (SReactions[destination], (Tsd_t *) varptr);
  subgoal = React_Subgoal_Get (schema);
  mind_Array_1d_Create ("&schema->notany", "reaction{6}", &schema->notany, MX_FUNCGROUPS, BITSIZE);
  mind_Array_1d_Create ("&schema->notall", "reaction{6}", &schema->notall, MX_FUNCGROUPS, BITSIZE);
  mind_Array_1d_Create ("&schema->musall", "reaction{6}", &schema->mustall, MX_FUNCGROUPS, BITSIZE);
  mind_Array_1d_Create ("&schema->musany", "reaction{6}", &schema->mustany, MX_FUNCGROUPS, BITSIZE);
#else
  if (destination < 0 || destination > React_NumSchemas_Get () ||
    SReactions[destination] != NULL) return (FALSE);

  SReactions[destination] = (React_Record_t *) malloc (REACTRECSIZE);
  schema = SReactions[destination];
  sch_head = React_Head_Get (schema);
  React_Text_Put (SReactions[destination], (React_TextRec_t *) malloc (REACTEXTRECSIZE));
  text = React_Text_Get (schema);
  txt_head = React_TxtRec_Head_Get (text);
  React_Goal_Put (SReactions[destination], (Tsd_t *) malloc (TSDSIZE));
  goal = React_Goal_Get (schema);
  React_Subgoal_Put (SReactions[destination], (Tsd_t *) malloc (TSDSIZE));
  subgoal = React_Subgoal_Get (schema);
  Array_1d_Create (&schema->notany, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&schema->notall, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&schema->mustall, MX_FUNCGROUPS, BITSIZE);
  Array_1d_Create (&schema->mustany, MX_FUNCGROUPS, BITSIZE);
#endif

  buf = buffer;
  Sgets (line, &buf);
  sscanf (line, "%06d (%06d): %02d/%03d/%06d (0x%02x)", inputs, inputs + 1, inputs + 2, inputs + 3, inputs + 4, inputs + 5);
  for (i = 0; i < 2; i++) source[i] = inputs[i];
  React_Head_SchemaId_Put (sch_head, destination);
  React_Head_Library_Put (sch_head, inputs[2]);
  React_Head_SynthemeFG_Put (sch_head, inputs[3]);
  React_TxtHd_OrigSchema_Put (txt_head, inputs[4]);
  React_Head_Flags_Put (sch_head, inputs[5]);

  Sgets (line, &buf);
  React_TxtRec_Name_Put (text, String_Create (line, 0));

  Sgets (line, &buf);
  sscanf (line, "Cre: 0x%08x; Mod: 0x%08x", inputs, inputs + 1);
  React_TxtHd_Created_Put (txt_head, inputs[0]);
  React_TxtHd_LastMod_Put (txt_head, inputs[1]);

  Sgets (line, &buf);
  sscanf (line, "%03d %03d %03d %s %d", inputs, inputs + 1, inputs + 2, temp, inputs + 4);
  for (inputs[3] = 0; strcmp (temp, type[inputs[3]]) != 0; inputs[3]++);
  React_Head_Ease_Put (sch_head, inputs[0]);
  React_Head_Yield_Put (sch_head, inputs[1]);
  React_Head_Confidence_Put (sch_head, inputs[2]);
  React_Head_ReactionType_Put (sch_head, inputs[3] + REACT_SING_APPL_ONLY);
  React_Head_MaxNonident_Put (sch_head, inputs[4]);

  Sgets (line, &buf);
  sscanf (line, "%02d Com:", inputs);
  React_TxtHd_NumComments_Put (txt_head, inputs[0]);

  if (React_TxtHd_NumComments_Get (txt_head) == 0) text->comments = NULL;
#ifdef _MIND_MEM_
  else mind_malloc ("text->comments", "reaction{6}", &text->comments, React_TxtHd_NumComments_Get (txt_head) * STRINGSIZE);
#else
  else text->comments = (String_t *) malloc (React_TxtHd_NumComments_Get (txt_head) * STRINGSIZE);
#endif
  for (i = 0; i < React_TxtHd_NumComments_Get (txt_head); i++)
    {
    Sgets (line, &buf);
    React_TxtRec_Comment_Put (text, i, String_Create (line + 1, 0));
    }

  Sgets (line, &buf);
  sscanf (line, "%02d Ref:", inputs);
  React_TxtHd_NumReferences_Put (txt_head, inputs[0]);

  if (React_TxtHd_NumReferences_Get (txt_head) == 0) text->lit_refs = NULL;
#ifdef _MIND_MEM_
  else mind_malloc ("text->lit_refs", "reaction{6}", &text->lit_refs, React_TxtHd_NumReferences_Get (txt_head) * STRINGSIZE);
#else
  else text->lit_refs = (String_t *) malloc (React_TxtHd_NumReferences_Get (txt_head) * STRINGSIZE);
#endif
  for (i = 0; i < React_TxtHd_NumReferences_Get (txt_head); i++)
    {
    Sgets (line, &buf);
    React_TxtRec_Reference_Put (text, i, String_Create (line + 1, 0));
    }

  Sgets (line, &buf);
  for (i = 0; line[i + 4] != '\0'; i++) React_CantAll_Put (schema, i, line[i + 4] == '1');
  for (; i < Array_Columns_Get (&schema->notall); i++) React_CantAll_Put (schema, i, FALSE);

  Sgets (line, &buf);
  for (i = 0; line[i + 4] != '\0'; i++) React_CantAny_Put (schema, i, line[i + 4] == '1');
  for (; i < Array_Columns_Get (&schema->notany); i++) React_CantAny_Put (schema, i, FALSE);

  Sgets (line, &buf);
  for (i = 0; line[i + 4] != '\0'; i++) React_MustAll_Put (schema, i, line[i + 4] == '1');
  for (; i < Array_Columns_Get (&schema->mustall); i++) React_MustAll_Put (schema, i, FALSE);

  Sgets (line, &buf);
  for (i = 0; line[i + 4] != '\0'; i++) React_MustAny_Put (schema, i, line[i + 4] == '1');
  for (; i < Array_Columns_Get (&schema->mustany); i++) React_MustAny_Put (schema, i, FALSE);

  Sgets (line, &buf);
  for (i = 0; line[8 * i] != '\0'; i++) sscanf (line + 8 * i, " %03d:%03d", inputs + 2 * i, inputs + 2 * i + 1);
  for (; i < MX_ROOTS; i++)
    {
    inputs[2 * i] = REACT_NODE_INVALID;
    inputs[2 * i + 1] = 0;
    }
  for (i = 0; i < MX_ROOTS; i++)
    {
    React_Head_RootNode_Put (sch_head, i, inputs[2 * i]);
    React_Head_RootSyntheme_Put (sch_head, i, inputs[2 * i + 1]);
    }

  Sgets (line, &buf);
  sscanf (line, "Goal: %03d (%03d) atoms", inputs, inputs + 1);
  React_Head_NumGoalAtoms_Put (sch_head, inputs[0]);
  Tsd_NumAtoms_Put (goal, inputs[0]);
  React_Head_GoalSize_Put (sch_head, inputs[1]);
#ifdef _MIND_MEM_
  mind_malloc ("Tsd_AtomHandle_Get(goal)", "reaction{6}", &varptr, Tsd_NumAtoms_Get (goal) * TSDROWSIZE);
  Tsd_AtomHandle_Put (goal, (TsdRow_t *) varptr);
#else
  Tsd_AtomHandle_Put (goal, (TsdRow_t *) malloc (Tsd_NumAtoms_Get (goal) * TSDROWSIZE));
#endif

  for (i = 0; i < React_Head_NumGoalAtoms_Get (sch_head); i++)
    {
    Sgets (line, &buf);
    sscanf (line, "\t%03d (0x%02x)", inputs, inputs + 1);
    Tsd_Atomid_Put (goal, i, inputs[0]);
    Tsd_Atom_Flags_Put (goal, i, inputs[1]);
    for (j = 0; j < MX_NEIGHBORS; j++)
      {
      sscanf (line + 12 * j + 11, " 0x%04x:0x%02x", inputs, inputs + 1);
      Tsd_Atom_NeighborId_Put (goal, i, j, inputs[0]);
      Tsd_Atom_NeighborBond_Put (goal, i, j, inputs[1]);
      }
    }

  Sgets (line, &buf);
  sscanf (line, "Subgoal: %03d atoms", inputs);
  React_Head_NumSubgoalAtoms_Put (sch_head, inputs[0]);
  Tsd_NumAtoms_Put (subgoal, inputs[0]);
#ifdef _MIND_MEM_
  mind_malloc ("Tsd_AtomHandle_Get(subgoal)", "reaction{6}", &varptr, Tsd_NumAtoms_Get (subgoal) * TSDROWSIZE);
  Tsd_AtomHandle_Put (subgoal, (TsdRow_t *) varptr);
#else
  Tsd_AtomHandle_Put (subgoal, (TsdRow_t *) malloc (Tsd_NumAtoms_Get (subgoal) * TSDROWSIZE));
#endif

  for (i = 0; i < React_Head_NumSubgoalAtoms_Get (sch_head); i++)
    {
    Sgets (line, &buf);
    sscanf (line, "\t%03d (0x%02x)", inputs, inputs + 1);
    Tsd_Atomid_Put (subgoal, i, inputs[0]);
    Tsd_Atom_Flags_Put (subgoal, i, inputs[1]);
    for (j = 0; j < MX_NEIGHBORS; j++)
      {
      sscanf (line + 12 * j + 11, " 0x%04x:0x%02x", inputs, inputs + 1);
      Tsd_Atom_NeighborId_Put (subgoal, i, j, inputs[0]);
      Tsd_Atom_NeighborBond_Put (subgoal, i, j, inputs[1]);
      }
    }

  Sgets (line, &buf);
  sscanf (line, "%03d Conds:", inputs);
  React_Head_NumConditions_Put (sch_head, inputs[0]);
  if (React_Head_NumConditions_Get (sch_head) == 0) schema->conditions = NULL;
#ifdef _MIND_MEM_
  else mind_malloc ("schema->conditions", "reaction{6}", &schema->conditions, React_Head_NumConditions_Get (sch_head) * CONDITIONSIZE);
#else
  else schema->conditions = (Condition_t *) malloc (React_Head_NumConditions_Get (sch_head) * CONDITIONSIZE);
#endif

  for (i = 0; i < React_Head_NumConditions_Get (sch_head); i++)
    {
    Sgets (line, &buf);
    condstr = String_Create (line + 1, 0);
    condition_export_refresh (React_Condition_Get (schema, i), &condstr, NULL, FALSE);
    }

  Sgets (line, &buf);
  sscanf (line, "%03d Tests:", inputs);
  React_Head_NumTests_Put (sch_head, inputs[0]);
  if (React_Head_NumTests_Get (sch_head) == 0)
    {
    schema->tests = NULL;
    text->reasons = NULL;
    text->chemists = NULL;
    }
  else
    {
#ifdef _MIND_MEM_
    mind_malloc ("schema_tests", "reaction{6}", &schema->tests, React_Head_NumTests_Get (sch_head) * POSTTESTSIZE);
    mind_malloc ("text->reasons", "reaction{6}", &text->reasons, React_Head_NumTests_Get (sch_head) * STRINGSIZE);
    mind_malloc ("text->chemists", "reaction{6}", &text->chemists, React_Head_NumTests_Get (sch_head) * STRINGSIZE);
#else
    schema->tests = (Posttest_t *) malloc (React_Head_NumTests_Get (sch_head) * POSTTESTSIZE);
    text->reasons = (String_t *) malloc (React_Head_NumTests_Get (sch_head) * STRINGSIZE);
    text->chemists = (String_t *) malloc (React_Head_NumTests_Get (sch_head) * STRINGSIZE);
#endif
    }

  for (testnum = 0; testnum < React_Head_NumTests_Get (sch_head); testnum++)
    {
    test = React_Test_Get (schema, testnum);

    Sgets (line, &buf);
    sscanf (line, "\t%s %1d%04d%04d%04d", test_expr, inputs + 3, inputs, inputs + 1, inputs + 2);
    Post_Head_Get (test)->flags = inputs[3];

    Post_EaseAdj_Put (test, inputs[0]);
    Post_YieldAdj_Put (test, inputs[1]);
    Post_ConfidenceAdj_Put (test, inputs[2]);

    for (i = j = 0; i < strlen (test_expr); i++) switch (test_expr[i])
      {
      case '&':
        opstr[j++] = OP_AND;
        break;
      case '|':
        opstr[j++] = OP_OR;
        break;
      case '~':
        opstr[j++] = OP_NOT;
        break;
      case '=':
        opstr[j++] = BOOLOP_EQ;
        break;
      case '#':
        opstr[j++] = BOOLOP_XOR;
        break;
      case 'N':
        if (strncmp (test_expr + i, "NOP", 3) != 0) return (FALSE);
        i += 2;
        opstr[j++] = OP_NOPASS;
        break;
      case 'C':
        sscanf (test_expr + i + 1, "%d", inputs);
        i += 2;
        opstr[j++] = inputs[0] - 1;
        break;
      case 'T':
        sscanf (test_expr + i + 1, "%d", inputs);
        i += 2;
        opstr[j++] = inputs[0] + PT_TEST_ADD - 1;
        break;
      default:
        return (FALSE);
        break;
      }

#ifdef _MIND_MEM_
    mind_malloc ("Post_OpHandle_Get(test)", "reaction{6}", &Post_OpHandle_Get (test), j);
#else
    Post_OpHandle_Get (test) = (char *) malloc (j);
#endif
    Post_Length_Put (test, j);
    for (i = 0; i < j; i++) Post_Op_Put (test, i, opstr[i]);

    Sgets (line, &buf);
    React_TxtRec_Reason_Put (text, testnum, String_Create (line + 2, 0));

    Sgets (line, &buf);
    React_TxtRec_Chemist_Put (text, testnum, String_Create (line + 2, 0));
    }

  return (TRUE);
}

void Sprintf (char *string, String_t value)
{
  char *valstr;
  int   vallen, i, j;

  valstr = String_Value_Get (value);
  vallen = String_Length_Get (value);
  for (i = j = 0; i < vallen; i++)
    if (valstr[i] == '\\')
      {
      strcpy (string + j, "\\\\");
      j += 2;
      }
    else if (valstr[i] >= ' ' && valstr[i] <= '~')
      string[j++] = valstr[i];
    else switch (valstr[i])
      {
    case '\b':
      strcpy (string + j, "\\b");
      j += 2;
      break;
    case '\e':
      strcpy (string + j, "\\e");
      j += 2;
      break;
    case '\n':
      strcpy (string + j, "\\n");
      j += 2;
      break;
    case '\r':
      strcpy (string + j, "\\r");
      j += 2;
      break;
    case '\t':
      strcpy (string + j, "\\t");
      j += 2;
      break;
    default:
      sprintf (string + j, "\\%03o", valstr[i]);
      j += 4;
      break;
      }

  strcpy (string + j, "\n");
}

void Sgets (char *dest, char **src)
{
  int c;
  char *d;

  for (d = dest; **src != '\n';) *d++ = *(*src)++;
  *d = '\0';
  (*src)++;

  for (d = dest; *d != '\0'; d++) if (d[0] == '\\') switch (d[1])
    {
  case '\\':
    strcpy (d, d + 1);
    break;
  case 'b':
    strcpy (d, d + 1);
    d[0] = '\b';
    break;
  case 'e':
    strcpy (d, d + 1);
    d[0] = '\e';
    break;
  case 'n':
    strcpy (d, d + 1);
    d[0] = '\n';
    break;
  case 'r':
    strcpy (d, d + 1);
    d[0] = '\r';
    break;
  case 't':
    strcpy (d, d + 1);
    d[0] = '\t';
    break;
  default:
    sscanf (d, "\\%03o", &c);
    strcpy (d, d + 3);
    d[0] = c;
    break;
    }
}

/* End of REACTION.C */
