#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main (int argc, char *argv[])
{
  int i;
  char command[256], filename[128], path[128], *slash, command_path[128];

  strcpy (command_path, argv[0]);
  for (i = strlen (command_path); i > 0 && command_path[i - 1] != '/'; i--);
  command_path[i] = 0;
  for (i = 1; i < argc - 2; i++)
  {
    strcpy (path, argv[i]);
    if ((slash = strstr (path, "/data/status/_")) != NULL && strstr (path, ".status") != NULL)
    {
      *slash++ = 0;
      slash = strstr (slash, "_");
      strcpy (filename, slash);
      sprintf (command, "%sstatus_fix %s %s %s %s", command_path, path, filename, argv[argc - 2], argv[argc - 1]);
      system (command);
    }
    else if ((slash = strstr (path, "/data/submit/_")) != NULL && strstr (path, ".submit") != NULL)
    {
      *slash++ = 0;
      slash = strstr (slash, "_");
      strcpy (filename, slash);
      sprintf (command, "%ssubmit_fix %s %s %s %s", command_path, path, filename, argv[argc - 2], argv[argc - 1]);
      system (command);
    }
    else fprintf (stderr, "Error: Can't identify %s as either status or submit!\n", path);
  }
}
