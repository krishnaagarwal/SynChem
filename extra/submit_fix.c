#include <stdio.h>
#include <stdlib.h>
#include <string.h>

main (int argc, char *argv[])
{
  char old_filename[128], new_filename[128], line[200], *oldinx, temp_line[200],
    name[64];
  FILE *old, *new;

  strcpy (name, argv[2]);
  sprintf (old_filename, "%s/data/submit/%s", argv[1], name);
  sprintf (new_filename, "%s/data/submit/%s", argv[4], name + 1);
  oldinx = strstr (name, ".");
  *oldinx = 0;
  old = fopen (old_filename, "r");
  new = fopen (new_filename, "w");
  while (fgets (line, 200, old) != NULL)
  {
    if ((oldinx = strstr (line, name)) != NULL) strcpy (oldinx, oldinx + 1);
    else if ((oldinx = strstr (line, argv[3])) != NULL)
    {
      strcpy (temp_line, oldinx + strlen (argv[3]));
      strcpy (oldinx, argv[4]);
      strcat (line, temp_line);
    }
    fprintf (new, line);
  }
  fclose (old);
  fclose (new);
}
