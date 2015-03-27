#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_
#endif

main (int argc, char *argv[])
{
  char old_filename[128], new_filename[128], line[200], *oldinx, temp_line[200];
  FILE *old, *new;
  int i, c;
  Rcb_t rcb;
  Filecb_t fcb;
  String_t old_str, new_str, user, name, comment, dir[FCB_IND_NUMOF], file[FCB_IND_NUMOF];
  Sling_t goal;

  sprintf (old_filename, "%s/data/status/%s", argv[1], argv[2]);
  sprintf (new_filename, "%s/data/status/%s", argv[4], argv[2] + 1);

  old = fopen (old_filename, "r");
  new = fopen (new_filename, "w");

  fread (&rcb, 1, RCBSIZE, old);

  goal = Rcb_Goal_Get (&rcb);
  Sling_Name_Put (goal, (U8_t *) malloc (Sling_Length_Get (goal) + 1));
  fread (Sling_Name_Get (goal), 1, Sling_Length_Get (goal) + 1, old);
  Rcb_Goal_Put (&rcb, goal);

  user = Rcb_User_Get (&rcb);
  String_Value_Get (user) = (U8_t *) malloc (String_Length_Get (user) + 1);
  String_Alloc_Put (user, String_Length_Get (user) + 1);
  fread (String_Value_Get (user), 1, String_Length_Get (user) + 1, old);
  Rcb_User_Put (&rcb, user);

  name = Rcb_Name_Get (&rcb);
  getc (old); /* skip underscore */
  String_Value_Get (name) = (U8_t *) malloc (String_Length_Get (name));
  String_Alloc_Put (name, String_Length_Get (name));
  String_Length_Put (name, String_Length_Get (name) - 1);
  fread (String_Value_Get (name), 1, String_Length_Get (name) + 1, old);
  Rcb_Name_Put (&rcb, name);

  comment = Rcb_Comment_Get (&rcb);
  String_Value_Get (comment) = (U8_t *) malloc (String_Length_Get (comment) + 1);
  String_Alloc_Put (comment, String_Length_Get (comment) + 1);
  fread (String_Value_Get (comment), 1, String_Length_Get (comment) + 1, old);
  Rcb_Comment_Put (&rcb, comment);

  for (i = 0; i < FCB_IND_NUMOF; i++)
  {
    fcb = Rcb_IthFileCB_Get (&rcb, i);
    old_str = FileCB_DirStr_Get (fcb);
    fread (line, 1, String_Length_Get (old_str) + 1, old);
    if ((oldinx = strstr (line, argv[3])) != NULL)
    {
      strcpy (temp_line, oldinx + strlen (argv[3]));
      strcpy (oldinx, argv[4]);
      strcat (line, temp_line);
    }
    new_str = String_Create (line, 0);
    FileCB_DirStr_Put (fcb, new_str);
    old_str = FileCB_FileStr_Get (fcb);
    fread (line, 1, String_Length_Get (old_str) + 1, old);
    if ((oldinx = strstr (line, argv[3])) != NULL)
    {
      strcpy (temp_line, oldinx + strlen (argv[3]));
      strcpy (oldinx, argv[4]);
      strcat (line, temp_line);
    }
    sprintf (temp_line, "_%s", String_Value_Get (name));
    if ((oldinx = strstr (line, temp_line)) != NULL) strcpy (oldinx, oldinx + 1);
    new_str = String_Create (line, 0);
    FileCB_FileStr_Put (fcb, new_str);
    Rcb_IthFileCB_Get (&rcb, i) = fcb;
  }

  fwrite (&rcb, 1, RCBSIZE, new);

  fwrite (Sling_Name_Get (goal), 1, Sling_Length_Get (goal) + 1, new);

  fwrite (String_Value_Get (user), 1, String_Length_Get (user) + 1, new);

  fwrite (String_Value_Get (name), 1, String_Length_Get (name) + 1, new);

  fwrite (String_Value_Get (comment), 1, String_Length_Get (comment) + 1, new);

  for (i = 0; i < FCB_IND_NUMOF; i++)
  {
    fcb = Rcb_IthFileCB_Get (&rcb, i);
    new_str = FileCB_DirStr_Get (fcb);
    fwrite (String_Value_Get (new_str), 1, String_Length_Get (new_str) + 1, new);

    new_str = FileCB_FileStr_Get (fcb);
    fwrite (String_Value_Get (new_str), 1, String_Length_Get (new_str) + 1, new);
  }

  while ((c = getc (old)) != EOF) putc (c, new);
  fclose (old);
  fclose (new);
}
