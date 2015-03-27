/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     PERSIST.C
*
*    Manages the mapping by date context of the persistent knowledge base into
*    a snapshot of the knowledge base as it existed at the time specified.
*
*  Creation Date:
*
*    17-Feb-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
* 27-Aug-01  Miller     Closed rxnlib in Persist_Inx_OK() after determining
*                       number of schemata to avoid redundant buffer allocation.
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

#define _H_PERSIST_GLOBAL_
#include "persist.h"
#undef _H_PERSIST_GLOBAL_

#ifndef _H_REACTION_
#include "reaction.h"
#endif

#ifndef _H_EXTERN_
#include "extern.h"
#endif

Boolean_t Persist_Inx_OK (char *standard_inx, char *dir_p)
{
  int ninx_recs, nrxn_recs, i, j;

  rxn_inx = fopen (standard_inx, "r+b");
#ifdef _DEBUG_
printf("persist: rxn_inx=%p\n",rxn_inx);
#endif
  if (rxn_inx == NULL) return (FALSE);
  React_Open (dir_p);
  nrxn_recs = React_NumSchemas_Get ();
/* No need to keep rxnlib open after this - main app will reopen it and reallocate buffers. */
  React_Close ();
  fseek (rxn_inx, 0L, SEEK_END);
  ninx_recs = ftell (rxn_inx) / PER_RXNINFO_SIZE;
#ifdef _DEBUG_
printf("persist: %d vs %d recs\n",ninx_recs,nrxn_recs);
#endif
  if (ninx_recs != nrxn_recs)
    {
    fprintf (stderr, "Persist_Inx_OK: %d index records vs %d schemata\n",
      ninx_recs, nrxn_recs);
    Persist_Close ();
    return (FALSE);
    }
  clearerr (rxn_inx);
  fseek (rxn_inx, 0L, SEEK_SET);
  fread (rxn_info[PER_STD], PER_RXNINFO_SIZE, ninx_recs, rxn_inx);
  for (i = 0; i < ninx_recs; i++)
    {
    if (rxn_info[PER_STD][i].temp_replacement_rec != PER_NONE || rxn_info[PER_STD][i].replaces_which_temp_rec != PER_NONE)
      {
      fprintf (stderr, "Persist_Inx_OK: record %d refers to temp index\n", i);
      Persist_Close ();
      return (FALSE);
      }
    j = rxn_info[PER_STD][i].replacement_rec;
    if (j != PER_NONE && j != PER_DELETED && (j <= i || j >= ninx_recs || rxn_info[PER_STD][j].replaces_which_rec != i))
      {
      fprintf (stderr, "Persist_Inx_OK: records %d and %d are not mutually consistent\n", i, j);
      Persist_Close ();
      return (FALSE);
      }
    j = rxn_info[PER_STD][i].replaces_which_rec;
    if (j != PER_NONE && (j >= i || j < 0 || rxn_info[PER_STD][j].replacement_rec != i))
      {
      fprintf (stderr, "Persist_Inx_OK: records %d and %d are not mutually consistent\n", i, j);
      Persist_Close ();
      return (FALSE);
      }
    }
#ifdef _DEBUG_
printf("persist succeeded\n");
#endif

  for (; i < MX_RXN_INFO; i++)
    {
    rxn_info[PER_STD][i].date_last_mod = rxn_info[PER_STD][i].time_last_mod = PER_TIME_LEGACY;
    rxn_info[PER_STD][i].replacement_rec = rxn_info[PER_STD][i].temp_replacement_rec = rxn_info[PER_STD][i].replaces_which_rec =
      rxn_info[PER_STD][i].replaces_which_temp_rec = PER_NONE;
    }

  return (TRUE);
}

Date_t Persist_Today ()
{
  Persist_Recalc_Time ();

  return (today);
}

long Persist_Now ()
{
/* Assume always called after Persist_Today () - prevent midnight debacle ("H2.4K crisis")
  Persist_Recalc_Time ();
*/

  return (time_now);
}

Boolean_t Persist_Legacy_Rxn (long rec, Date_t up_to_date, long up_to_time, Boolean_t *is_temp)
{
  Boolean_t is_legacy, dummy_bool;

  is_legacy = rxn_info[PER_STD][rec].replaces_which_rec == PER_NONE &&

/* exclude reactions created since run */
    (rxn_info[PER_STD][rec].date_last_mod < up_to_date ||
    (rxn_info[PER_STD][rec].date_last_mod == up_to_date && rxn_info[PER_STD][rec].time_last_mod <= up_to_time)) &&
/***************************************/

    Persist_Current_Rec (PER_STD, rec, up_to_date, up_to_time, &dummy_bool) != PER_DELETED;
  return (is_legacy);
}

long Persist_ModTime (int which, long rec)
{
  long mod_time;

  mod_time = rxn_info[which][rec].time_last_mod;
  return (mod_time);
}

long Persist_Current_Rec (int which, long orig_rec, Date_t up_to_date, long up_to_time, Boolean_t *is_temp)
{
  long next_rec, temp_rec;
  Boolean_t dummy_bool;

  *is_temp = FALSE;

  temp_rec=rxn_info[which][orig_rec].temp_replacement_rec;

  if (temp_rec != PER_NONE)
    {
    *is_temp = TRUE;
    if (temp_rec == PER_DELETED) return (temp_rec);
    return (Persist_Current_Rec (PER_TEMP, temp_rec, today, time_now, &dummy_bool));
    }
  if (which == PER_TEMP) return (orig_rec);

  next_rec=rxn_info[which][orig_rec].replacement_rec;

  if (next_rec == PER_NONE) return (orig_rec);
  if (next_rec == PER_DELETED) return (next_rec);
  if (rxn_info[which][next_rec].date_last_mod > up_to_date) return (orig_rec);
  if (rxn_info[which][next_rec].date_last_mod == up_to_date && rxn_info[which][next_rec].time_last_mod > up_to_time)
    return (orig_rec);

  return (Persist_Current_Rec (which, next_rec, up_to_date, up_to_time, is_temp));
}

long Persist_Orig_Rec (int which, long curr_rec, Boolean_t *is_temp)
{
  long prev_rec, std_rec;
  Boolean_t dummy_bool;

  *is_temp = TRUE;

  std_rec = rxn_info[which][curr_rec].replaces_which_rec;

  if (std_rec != PER_NONE)
    {
    *is_temp = FALSE;
    return (Persist_Orig_Rec (PER_STD, std_rec, &dummy_bool));
    }
  if (which == PER_STD) return (curr_rec);

  prev_rec = rxn_info[which][curr_rec].replaces_which_temp_rec;

  if (prev_rec == PER_NONE) return (curr_rec);

  return (Persist_Orig_Rec (which, prev_rec, is_temp));
}

void Persist_Open (char *standard_inx, char *temp_inx, char *temp_rxnlib, Boolean_t fixit)
{
  int i, j, k, num_ref, lib, chap, sch, prevlib, prevchap, prevsch;
  Date_t cre_date, mod_date;
  char trxnlib[128], *ref, *last_upd;
  React_Record_t *schema;
  React_Head_t *sch_head;
  React_TextRec_t *text;
  React_TextHead_t *txt_head;
  Boolean_t found;
String_t unencrypted, temp;

  Persist_Recalc_Time ();

  if (temp_inx == NULL)
    {
    temp_exists = FALSE;
    temp_rxn_inx = NULL;
    }
  else
    {
    temp_rxn_inx = fopen (temp_inx, "r+b");
    if (temp_rxn_inx == NULL) temp_rxn_inx = fopen (temp_inx, "w+b");
    temp_exists = temp_rxn_inx != NULL;
    if (!temp_exists) fprintf (stderr, "WARNING: Error creating %s as temporary index\n", temp_inx);
    else
      {
      sprintf (trxnlib, "R+W%s", temp_rxnlib);
      React_Temp_Init (trxnlib);
      }
    }
  if (!temp_exists) i = 0;
  else for (i = 0; i < MX_RXN_INFO; i++)
    if (fread (rxn_info[PER_TEMP] + i, PER_RXNINFO_SIZE, 1, temp_rxn_inx) == 0) break;

  recs_in_temp_inx = i;

  for (; i < MX_RXN_INFO; i++)
    {
    rxn_info[PER_TEMP][i].date_last_mod = rxn_info[PER_TEMP][i].time_last_mod = PER_TIME_LEGACY;
    rxn_info[PER_TEMP][i].replacement_rec = rxn_info[PER_TEMP][i].temp_replacement_rec = rxn_info[PER_TEMP][i].replaces_which_rec =
      rxn_info[PER_TEMP][i].replaces_which_temp_rec = PER_NONE;
    }

  if (rxn_inx != NULL) return;

  rxn_inx = fopen (standard_inx, "r+b");
  if (rxn_inx == NULL)
    {
    rxn_inx = fopen (standard_inx, "w+b");
    if (rxn_inx == NULL)
      {
      fprintf (stderr, "FATAL ERROR: unsuccessful attempt to create %s as standard index\n", standard_inx);
      exit (1);
      }
    i = 0;
    }
  else
    {
    for (i = 0; i < MX_RXN_INFO; i++)
      if (fread (rxn_info[PER_STD] + i, PER_RXNINFO_SIZE, 1, rxn_inx) == 0) break;
    if (i != (j = React_NumSchemas_Get ()))
      {
      if (fixit && i < j)
        {
        fprintf (stderr, "Persist_Open: attempting to fix %s\n", standard_inx);
        for (; i < j; i++)
          {
          last_upd = NULL;
          schema = React_Schema_Handle_Get (i);
          sch_head = React_Head_Get (schema);
          text = React_Text_Get (schema);
          txt_head = React_TxtRec_Head_Get (text);
          lib = React_Head_Library_Get (sch_head);
          chap = React_Head_SynthemeFG_Get (sch_head);
          sch = React_TxtHd_OrigSchema_Get (txt_head);
          num_ref = React_TxtHd_NumReferences_Get (txt_head);
          cre_date = React_TxtHd_Created_Get (txt_head);
          mod_date = React_TxtHd_LastMod_Get (txt_head);
          if (cre_date == (Date_t) 0)
            {
            fprintf (stderr, "Error in reaction library found when trying to update persistent index: schema %d (l%d/c%d/s%d) "
              "has no creation date.\n", i, lib, chap, sch);
            exit (1);
            }
          for (k = 0; k < num_ref; k++)
            {
            unencrypted = String_Encrypt (React_TxtRec_Reference_Get (text, k), k, i);
            ref = String_Value_Get (unencrypted);
            if (ref[0] == '\007' || strncmp (ref, "Schema modified ", 16) == 0 || strncmp (ref, "Schema created ", 15) == 0)
              {
              if (last_upd != NULL) String_Destroy (temp);
              temp = String_Copy (unencrypted);
              last_upd = String_Value_Get (temp);
              }
            String_Destroy (unencrypted);
            }
          if (mod_date == (Date_t) 0)
            {
            found = TRUE;
            Persist_LinkRecs (rxn_inx, PER_STD, i, PER_NONE, cre_date, last_upd, FALSE);
            }
          else for (k = i - 1, found = FALSE; k >= 0 && !found; k--)
            {
            schema = React_Schema_Handle_Get (k);
            sch_head = React_Head_Get (schema);
            text = React_Text_Get (schema);
            txt_head = React_TxtRec_Head_Get (text);
            prevlib = React_Head_Library_Get (sch_head);
            prevchap = React_Head_SynthemeFG_Get (sch_head);
            prevsch = React_TxtHd_OrigSchema_Get (txt_head);
            if (prevlib == lib && prevchap == chap && prevsch == sch)
              {
              found = TRUE;
              Persist_LinkRecs (rxn_inx, PER_STD, i, k, mod_date, last_upd, FALSE);
              }
            }
          if (!found)
            {
            fprintf (stderr, "Error in reaction library found when trying to update persistent index: schema %d (l%d/c%d/s%d) "
              "does not replace any earlier vesrion.\n", i, lib, chap, sch);
            exit (1);
            }
          if (last_upd != NULL) String_Destroy (temp);
          }
        }
      else if (fixit && i > j)
        {
        fclose (rxn_inx);
        rxn_inx = fopen (standard_inx, "w+b");
        fwrite (rxn_info[PER_STD], PER_RXNINFO_SIZE, j, rxn_inx);
        i = j;
        }
      else
        {
        printf ("Persist_Open error: rkbstd.inx has %s records than reaction library (%d vs %d)\n",
          i > j ? "more" : "fewer", i, j);
        printf ("\tThe three (3) files in %s having the \".bak\" extension should contain the version\n"
          "\tthat predates whatever abnormal termination led to this inconsistency.\n", FCB_SEQDIR_RXNS (""));
        exit (1);
        }
      }
    }

  recs_in_inx = i;

/*
printf ("read %d records of %d\n",i, React_NumSchemas_Get ());
exit(0);
*/
  for (; i < MX_RXN_INFO; i++)
    {
    rxn_info[PER_STD][i].date_last_mod = rxn_info[PER_STD][i].time_last_mod = PER_TIME_LEGACY;
    rxn_info[PER_STD][i].replacement_rec = rxn_info[PER_STD][i].temp_replacement_rec = rxn_info[PER_STD][i].replaces_which_rec =
      rxn_info[PER_STD][i].replaces_which_temp_rec = PER_NONE;
    }

  j = React_NumSchemas_Get ();
  if (recs_in_inx < j && fixit)
    {
    clearerr (rxn_inx);
    fseek (rxn_inx, recs_in_inx * PER_RXNINFO_SIZE, SEEK_SET);
    fwrite (rxn_info[PER_STD] + recs_in_inx, PER_RXNINFO_SIZE, j - recs_in_inx, rxn_inx);
    }
}

void Persist_Close()
{
  if (temp_exists && temp_rxn_inx != NULL) fclose (temp_rxn_inx);
  if (rxn_inx != NULL) fclose (rxn_inx);
  rxn_inx = temp_rxn_inx = NULL;
}

void Persist_LinkRecs (FILE *f, int which_info, int current, int previous, Date_t mod_date, char *last_update, Boolean_t is_templib)
{
  long mod_time;

  if (previous == PER_NONE) fprintf (stderr, "Persist_LinkRecs: adding record %d as new schema\n", current);
  else fprintf (stderr, "Persist_linkRecs: adding record %d as replacement for %d\n", current, previous);
  if (sscanf (last_update + strlen (last_update) - 8, " at %d", &mod_time) != 1)
    {
    fprintf (stderr, "Persist_LinkRecs error: badly formatted <LIBRARY UPDATE> string:\n\t%s\n",last_update);
    exit (1);
    }
  mod_time *= 60; /* add back truncated seconds */

  rxn_info[which_info][current].date_last_mod = mod_date;
  rxn_info[which_info][current].time_last_mod = mod_time;
  rxn_info[which_info][current].replacement_rec = PER_NONE;
  rxn_info[which_info][current].temp_replacement_rec = PER_NONE;
  if (is_templib)
    {
    rxn_info[which_info][current].replaces_which_rec = PER_NONE;
    rxn_info[which_info][current].replaces_which_temp_rec = previous;
    }
  else
    {
    rxn_info[which_info][current].replaces_which_rec = previous;
    rxn_info[which_info][current].replaces_which_temp_rec = PER_NONE;
    }

  clearerr (f);
  fseek (f, current * PER_RXNINFO_SIZE, SEEK_SET);

  if (fwrite (rxn_info[which_info] + current, PER_RXNINFO_SIZE, 1, f) != 1)
    {
    fprintf (stderr, "Persist_LinkRecs: Error writing record\n");
    exit (1);
    }
  fflush (f);

  fprintf (stderr, "Done writing new record\n");

  if (previous == PER_NONE) return;

  clearerr (f);
  fseek (f, previous * PER_RXNINFO_SIZE, SEEK_SET);

  fread (rxn_info[which_info] + previous, PER_RXNINFO_SIZE, 1, f);

  if (is_templib)
    {
    rxn_info[which_info][previous].replacement_rec = PER_NONE;
    rxn_info[which_info][previous].temp_replacement_rec = current;
    }
  else
    {
    rxn_info[which_info][previous].replacement_rec = current;
    rxn_info[which_info][previous].temp_replacement_rec = PER_NONE;
    }

  clearerr (f);
  fseek (f, previous * PER_RXNINFO_SIZE, SEEK_SET);

  fwrite (rxn_info[which_info] + previous, PER_RXNINFO_SIZE, 1, f);
  fflush (f);

  fprintf (stderr, "Done updating old record\n");
}

void Persist_Init_RxnInx ()
{
  int i, NSch;

  React_Init (FCB_SEQDIR_RXNS (""));
  NSch = React_NumSchemas_Get ();
  Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);
  if (recs_in_inx != 0)
    {
    fprintf (stderr, "Persist_Init_RxnInx error: file is not empty\n");
    exit (1);
    }
  for (i = 0; i < NSch; i++) fwrite (rxn_info[PER_STD] + i, PER_RXNINFO_SIZE, 1, rxn_inx);
  Persist_Close ();
}

void Persist_Dump_RxnInx (FILE *output)
{
  int i, j, rep[4];
  char repstr[4][16];

  React_Init (FCB_SEQDIR_RXNS (""));
  Persist_Open (FCB_SEQDIR_RXNS ("/rkbstd.inx"), NULL, NULL, FALSE);
  fprintf (output, "%s contains %d records\n", FCB_SEQDIR_RXNS ("/rkbstd.inx"), recs_in_inx);
  fprintf (output, "\nrec#\tdate\ttime\trepby\ttrepby\treps\trepst\n\n");

  for (i = 0; i < recs_in_inx; i++)
    {
    rep[0] = rxn_info[PER_STD][i].replacement_rec;
    rep[1] = rxn_info[PER_STD][i].temp_replacement_rec;
    rep[2] = rxn_info[PER_STD][i].replaces_which_rec;
    rep[3] = rxn_info[PER_STD][i].replaces_which_temp_rec;
    for (j = 0; j < 4; j++)
      {
      if (rep[j] == PER_NONE) strcpy (repstr[j], "none");
      else sprintf (repstr[j], "%d", rep[j]);
      }
    fprintf (output, "%d\t%x\t%x\t%s\t%s\t%s\t%s\n", i, rxn_info[PER_STD][i].date_last_mod, rxn_info[PER_STD][i].time_last_mod,
      repstr[0], repstr[1], repstr[2], repstr[3]);
    }
}

void Persist_Update_Rxn (long new_rec, long prev_rec, Boolean_t new_is_temp, Boolean_t prev_is_temp, char *debug_msg)
{
  int which_rec[2], i, lib[2], chap[2], sch[2];
  FILE *which_file[2];
  React_Record_t *schema[2];
  React_Head_t *sch_head[2];
  React_TextRec_t *text[2];
  React_TextHead_t *txt_head[2];

  if (new_rec == prev_rec && new_is_temp != prev_is_temp)
    {
    fprintf (stderr, "Persist_Update_Rxn error: inconsistent boolean parameters for update-in-place\n\t(called from %s)\n",
      debug_msg);
    exit(1);
    }

  if (new_rec >= React_NumSchemas_Get ())
    {
    fprintf (stderr, "Persist_Update_Rxn error: attempting to create record (%d<-%d) beyond rxnlib count\n\t(called from %s)\n",
      new_rec, prev_rec, debug_msg);
    exit (1);
    }

/* Assume always called after Persist_Today () - prevent midnight debacle ("H2.4K crisis")
  Persist_Recalc_Time ();
*/

  which_rec[PER_NEW] = new_is_temp ? PER_TEMP : PER_STD;
  which_file[PER_NEW] = new_is_temp ? temp_rxn_inx : rxn_inx;

  if (new_rec != prev_rec)
    {
    schema[0] = React_Schema_Handle_Get (new_rec);
    schema[1] = React_Schema_Handle_Get (prev_rec);
    for (i = 0; i < 2; i++)
      {
      sch_head[i] = React_Head_Get (schema[i]);
      text[i] = React_Text_Get (schema[i]);
      txt_head[i] = React_TxtRec_Head_Get (text[i]);
      lib[i] = React_Head_Library_Get (sch_head[i]);
      chap[i] = React_Head_SynthemeFG_Get (sch_head[i]);
      sch[i] = React_TxtHd_OrigSchema_Get (txt_head[i]);
      }

    if ((lib[0] != lib[1] || chap[0] != chap[1] || sch[0] != sch[1]) && strcmp (debug_msg, "move_schema") != 0)
      {
      fprintf (stderr, "Persist_Update_Rxn error: attempting to replace %d/%d/%d with %d/%d/%d\n\t(called from %s)\n",
        lib[1], chap[1], sch[1], lib[0], chap[0], sch[0], debug_msg);
      exit (1);
      }

    which_rec[PER_PREV] = prev_is_temp ? PER_TEMP : PER_STD;
    which_file[PER_PREV] = prev_is_temp ? temp_rxn_inx : rxn_inx;

    if (new_is_temp)
      rxn_info[which_rec[PER_PREV]][prev_rec].temp_replacement_rec = new_rec;
    else
      rxn_info[which_rec[PER_PREV]][prev_rec].replacement_rec = new_rec;

    clearerr (which_file[PER_PREV]);
    fseek (which_file[PER_PREV], prev_rec * PER_RXNINFO_SIZE, SEEK_SET);
    fwrite (rxn_info[which_rec[PER_PREV]] + prev_rec, PER_RXNINFO_SIZE, 1, which_file[PER_PREV]);
    fflush (which_file[PER_PREV]); /* This is necessary to maintain consistency with the reaction library files. */

    if (prev_is_temp)
      rxn_info[which_rec[PER_NEW]][new_rec].replaces_which_temp_rec = prev_rec;
    else
      rxn_info[which_rec[PER_NEW]][new_rec].replaces_which_rec = prev_rec;
    }

  rxn_info[which_rec[PER_NEW]][new_rec].date_last_mod = today;
  rxn_info[which_rec[PER_NEW]][new_rec].time_last_mod = time_now;

  clearerr (which_file[PER_NEW]);
  fseek (which_file[PER_NEW], new_rec * PER_RXNINFO_SIZE, SEEK_SET);
  fwrite (rxn_info[which_rec[PER_NEW]] + new_rec, PER_RXNINFO_SIZE, 1, which_file[PER_NEW]);
  fflush (which_file[PER_NEW]); /* This is necessary to maintain consistency with the reaction library files. */
}

void Persist_Add_Rxn (long rec, Boolean_t is_temp, char *debug_msg)
{
  int which_info;
  FILE *which_file;

  if (rec >= React_NumSchemas_Get ())
    {
    fprintf (stderr, "Persist_Add_Rxn error: attempting to create record (%d) beyond rxnlib count\n\t(called from %s)\n",
      rec, debug_msg);
    exit (1);
    }

  Persist_Recalc_Time ();

  if (is_temp)
    {
    which_info = PER_TEMP;
    which_file = temp_rxn_inx;
    }
  else
    {
    which_info = PER_STD;
    which_file = rxn_inx;
    }
  rxn_info[which_info][rec].date_last_mod = today;
  rxn_info[which_info][rec].time_last_mod = time_now;

  clearerr (which_file);
  fseek (which_file, rec * PER_RXNINFO_SIZE, SEEK_SET);
  fwrite (rxn_info[which_info] + rec, PER_RXNINFO_SIZE, 1, which_file);
  fflush (which_file); /* This is necessary to maintain consistency with the reaction library files. */
}

void Persist_Delete_Rxn (long rec, Boolean_t is_temp)
{
  int which_info;
  FILE *which_file;

  if (is_temp)
    {
    which_info = PER_TEMP;
    which_file = temp_rxn_inx;
    }
  else
    {
    which_info = PER_STD;
    which_file = rxn_inx;
    }

  if (is_temp)
    rxn_info[which_info][rec].temp_replacement_rec = PER_DELETED;
  else
    rxn_info[which_info][rec].replacement_rec = PER_DELETED;

  clearerr (which_file);
  fseek (which_file, rec * PER_RXNINFO_SIZE, SEEK_SET);
  fwrite (rxn_info[which_info] + rec, PER_RXNINFO_SIZE, 1, which_file);
  fflush (which_file); /* This is necessary to maintain consistency with the reaction library files. */
}

void Persist_Recalc_Time ()
{
  int year,
      day,
      hour,
      minute,
      second,
      num1,
      num2;
  time_t now;
  char date_str[32],
       day_of_week[8],
       month_name[8],
       *sts,
       *tmp;
  static U16_t SMonths[12] = {0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304,
    334};
  static char *month_string = "Jan Feb Mar Apr May Jun Jul Aug Sep Oct Nov Dec";

  now = time (NULL);
  strcpy (date_str, ctime (&now));
  sscanf (date_str, "%s %s %d %d:%d:%d %d", day_of_week, month_name, &day, &hour, &minute, &second, &year);

  tmp = strstr (month_string, month_name);
  sts = month_string;
  num1 = day + SMonths[(tmp - sts) / 4];
  num2 = (year - 1900) * 365 + (year - 1900) / 4;  /* Hack */
  today = num2 + num1;
  time_now = 3600 * hour + 60 * minute + second;
#ifdef _DEBUG_
printf("today=%d time_now=%d\n",today,time_now);
#endif
}
