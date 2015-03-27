/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     ISAM2FG.C
*
*    This module is the inverse of fg2isam, and dumps the contents of
*    the functional group isam file for verification.
*
*  Routines:
*
*    xxx
*
*  Creation Date:
*
*    27-Feb-1995
*
*  Authors:
*
*    Daren Krebsbach 
*
*  Modification History, reverse chronological
*
* Date       Author	Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred	xxx
*
******************************************************************************/
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "debug.h"
#include "synio.h"

#ifndef _H_ISAM_
#include "isam.h"
#endif

#ifndef _H_FUNCGROUPS_
#include "funcgroups.h"
#endif

#ifndef _H_FUNCGROUP_FILE_
#include "funcgroup_file.h"
#endif

int main
  (
  int           argc,                       /* # arguments */
  char        **argv,                       /* Argument vector */
  char        **envp                        /* Environment vector */
  )
{
  Isam_Control_t if_p;                      /* ISAM file to read from */
  FILE          *fp;                        /* Temp file ptr */
  FileDsc_t     ofd_p;                      /* File to dump to */
  FuncGrp_Record_t *fgrec_p;                /* Functional Group Record */
  U32_t         key;                        /* Which ISAM key value */

  Debug_Init ();
  Trace_Init ();
  IO_Init ();
  GStdErr.handle = stdout;
  GTraceFile.handle = stdout;

  if (argc != 3)
    {
    printf ("Usage: isam2fg isamdir dumpfile\n");
    exit (-1);
    }

  if ((fp = fopen (argv[2], "w")) == NULL)
    {
    perror ("Failed to open output file");
    exit (-1);
    }

  strncpy (IO_FileName_Get (&ofd_p), argv[2], MX_FILENAME);
  IO_FileHandle_Put (&ofd_p, fp);
  
  strncpy (IO_FileName_Get (Isam_File_Get (&if_p)), argv[1], MX_FILENAME - 1);
  strncat (IO_FileName_Get (Isam_File_Get (&if_p)), FUNCGRP_FILENAME,
    MX_FILENAME - strlen (argv[1]) - 1);
  Isam_Open (&if_p, ISAM_TYPE_FGINFO, ISAM_OPEN_READ);

  for (key = 1; key < Isam_NextKey_Get (&if_p); key++)
    {
    fgrec_p = FuncGrp_Rec_Read (key, &if_p);
    FuncGrp_Rec_Dump (fgrec_p, &ofd_p);
    FuncGrp_Rec_Destroy (fgrec_p);
    }

  Isam_Close (&if_p);
  if (fclose (fp) < 0)
    perror ("Error closing input file");

  exit (0);
}
