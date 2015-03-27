/******************************************************************************
*
*  Copyright (C) 1992-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     FG2ISAM.C
*
*    This module converts the old text format of the Functional Group
*    Table into the ISAM file format.  The table was processed by hand
*    from the original very clumsy (but free-form) PL/1 file to the
*    following format:
*      - {; (semi-colon) for field separator}
*      - FG#[P], for preservable groups
*      - Name[s], could be more than 1
*      - Sling[s], usually more than 1, could be multiple lines as well
*        which are distinguished by NOT starting with a number
*
*    For the new format, see FUNCGROUPS_FILE.H.
*
*  Routines:
*
*    main
*
*  Creation Date:
*
*    01-Jan-1992
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
* 26-Feb-95  Krebsbach  To avoid problems with creating a nondense isam 
*                       file, write out initialized header as record zero.
* 19-Oct-00  Miller     Added IBM_compatible_fgets to compensate for the
*                       backward hacks at Microsoft; also allowed newline
*                       to be a valid separator token anywhere other than
*                       after a numeric field; also corrected inner while
*                       loop to prevent unpredictable behavior due to failure
*                       to test for EOF (sts == NULL); ALSO, made separate
*                       lines for slings a REQUIREMENT, so as not to have to
*                       "fudge" names like "Nitro"!
*
******************************************************************************/

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_UTL_
#include "utl.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

/* Static variables */

static Isam_Control_t   SIsam_CB;
static FuncGrp_Record_t SFgdsc;
static FuncGrp_Head_t  *SFhd_p;
static String_t         Snames[16];
static Sling_t          Sslings[128];

char *IBM_compatible_fgets (char *line, size_t maxlen, FILE *f)
{
  char *retval;
  int len;

  retval = fgets (line, maxlen, f);
  if (retval == NULL) return (NULL);
  len = strlen (line);
  if (len > 1 && line[len - 2] == '\r') strcpy (line + len - 2, line + len - 1);

  return (retval);
}

int main
  (
  int argc,
  char **argv
  )
{
  char       *sts;                         /* Result of IBM_compatible_fgets */
  FILE       *inf;                         /* Input file handle */
  char       *str_p;                       /* Temporary for parsing input */
  U32_t       key;                         /* What key */
  U16_t       index;                       /* Index of ~ in name */
  U16_t       name_cnt;                    /* Counter */
  U16_t       sling_cnt;                   /* Counter */
  U16_t       i;                           /* Counter */
  String_t    str_tmp;                     /* Temporary for converting input */
  Sling_t     sling_tmp;                   /* Temporary for converting input */
  char        ibuf[MX_INPUT_SIZE];         /* Input buffer */

  /* Call all the SYNCHEM init routines
     - Open the file, name must be passed as the 1st argument
     - Set up the static data-structures
     - Open the output ISAM file
     - Loop through all lines of the input file
       - Init input Funcgroup_Record_t
       - Find 1st field separator, check for Preservable flag
         set number and new FG record key
       - Loop over field separators
         - Check for Slings (if not a Sling, then a name)
         - Keep count of names and Slings and store them as they are found
       - Check for invalid groups (not-groups) in names
       - Write out record
       - Destroy names and Slings
     - Close both files
     - Return
  */

  Debug_Init ();
  Trace_Init ();
  IO_Init ();
  IO_FileHandle_Put (&GStdErr, stdout);
  IO_FileHandle_Put (&GTraceFile, stdout);

  if (argc != 3)
    {
    printf ("Usage: fg2isam textname isamname\n");
    exit (-1);
    }

  if ((inf = fopen (argv[1], "r")) == NULL)
    {
    perror ("Failed to open input file");
    exit (-1);
    }

  SFgdsc.names = Snames;
  SFgdsc.slings = Sslings;
  SFhd_p = FuncGrp_Rec_Head_Get (&SFgdsc);
  key = 1;
  strcpy (IO_FileName_Get (Isam_File_Get (&SIsam_CB)), argv[2]);
  Isam_Open (&SIsam_CB, ISAM_TYPE_FGINFO, ISAM_OPEN_INIT);

  /*  Store empty header as zero record of isam file (DK).  */
  FuncGrp_Head_FGNum_Put (SFhd_p, 0);
  FuncGrp_Head_Flags_Put (SFhd_p, 0);
  FuncGrp_Head_NumNames_Put (SFhd_p, 0);
  FuncGrp_Head_NumSlings_Put (SFhd_p, 0);
  for (i = 0; i < FG_MX_NOTGRP; i++)
    FuncGrp_Head_NotGroup_Put (SFhd_p, i, 0);
  if (Isam_Write (&SIsam_CB, 0, SFhd_p, FUNCGRPHEADSIZE) !=
      SYN_NORMAL)
    IO_Put_Trace (R_ISAM,
      "Unexpected status returned from Isam_Write in main (fg2isam)");

  sts = IBM_compatible_fgets (ibuf, sizeof (ibuf), inf);
  while (sts != NULL)
    {
    name_cnt = sling_cnt = 0;
    FuncGrp_Rec_Key_Put (&SFgdsc, 0);
    FuncGrp_Head_Flags_Put (SFhd_p, 0);
    FuncGrp_Head_NumNames_Put (SFhd_p, 0);
    FuncGrp_Head_NumSlings_Put (SFhd_p, 0);
    for (i = 0; i < FG_MX_NOTGRP; i++)
      FuncGrp_Head_NotGroup_Put (SFhd_p, i, 0);

    /* Have initialized counters and data areas.  Now to parse off the
       functional group number and preservable flags.
    */

    str_p = strtok (ibuf, ";");
    if (!(isdigit (ibuf[strlen (str_p) - 1])))
      {
      FuncGrp_Head_FlagsPreserveable_Put (SFhd_p, TRUE);
      ibuf[strlen (str_p)] = '\0';
      }

    FuncGrp_Head_FGNum_Put (SFhd_p, atoi (str_p));
    FuncGrp_Rec_Key_Put (&SFgdsc, key);
    key++;
    do
      {
      if (!(isdigit (ibuf[0])))
        str_p = strtok (ibuf, ";\n"); /* Allow for a more readable input: each sling on a separate line! */
      else
        str_p = strtok (NULL, ";\n"); /* Allow for a more readable input: each sling on a separate line! */

      while (str_p != NULL)
        {
        /* Loop while more strings, could be names or Slings.
        */

        str_tmp = String_Create (str_p, 64);
        sling_tmp = String2Sling (str_tmp);
if (!(isdigit (ibuf[0])))
{
        if (Sling_Validate (sling_tmp, &i) == TRUE)
	  {
          FuncGrp_Rec_Sling_Put (&SFgdsc, sling_cnt, sling_tmp);
          sling_cnt++;
	  }
        else
          {
          printf ("Error in sling \"%s\"\n", sling_tmp);
          exit (-1);
          }
}
        else
          {
          FuncGrp_Rec_Name_Put (&SFgdsc, name_cnt, str_tmp);
          name_cnt++;
	  }

        /* More strings possible for this input line, then more input lines
           possible for this func. group, a new group starts with a number as
           the first field in the record.
        */

        str_p = strtok (NULL, ";\n");
        }                                  /* End of while-more strings */
      sts = IBM_compatible_fgets (ibuf, sizeof (ibuf), inf);
      }                                    /* End of do-while block */
    while (sts != NULL /* must test for EOF condition!!! */ && !(isdigit (ibuf[0])));

    /* Fill in the data-structure, then call the formatter to write it out to
       the ISAM file.
       Then free the allocated storage.
    */

    FuncGrp_Head_NumNames_Put (SFhd_p, name_cnt);
    FuncGrp_Head_NumSlings_Put (SFhd_p, sling_cnt);

    /* Check names for negative groups, distinguished by ~'s.
       Parse them out, convert them to binary and drop them from the 
       name.
    */

    for (name_cnt = 0; name_cnt < FuncGrp_Head_NumNames_Get (SFhd_p);
         name_cnt++)
      {
      str_tmp = FuncGrp_Rec_Name_Get (&SFgdsc, name_cnt);
      index = String_Index_c (str_tmp, "~");
      if (index != (U16_t) INFINITY)
        {
        str_p = strtok ((char *) String_Value_Get (str_tmp), "~");
        str_p = strtok (NULL, "~");
        i = 0;
        while (str_p != NULL)
          {
          FuncGrp_Head_NotGroup_Put (SFhd_p, i, atoi (str_p));
          str_p = strtok (NULL, "~");
          i++;
          }

        String_Truncate (str_tmp, String_Length_Get (str_tmp) - index - 1);
	}  /* The above works due to copying of pointer to value storage */
      }

    FuncGrp_Rec_Write (&SFgdsc, &SIsam_CB);
/*    FuncGrp_Rec_Dump (&SFgdsc, &GStdOut); */
    for (i = 0; i < name_cnt; i++)
      String_Destroy (Snames[i]);

    for (i = 0; i < sling_cnt; i++)
      Sling_Destroy (Sslings[i]);
    }                                      /* End of while more input block */

  if (fclose (inf) < 0)
    perror ("Error closing input file");

  Isam_Close (&SIsam_CB);

  return 0;
}
