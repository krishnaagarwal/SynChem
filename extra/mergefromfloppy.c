#include <stdio.h>

main (int argc, char *argv[])
{
  FILE *in, *out;
  char buffer[1454080], name[100];
  int i, bytes;

  out = fopen (argv[1], "w");
  i = 1;
  do
  {
    sprintf (name, "%s.%d", argv[1], i);
    if ((in = fopen (name, "r")) == NULL) bytes = 0;
    else
    {
      bytes = fread (buffer, 1, 1454080, in);
      fwrite (buffer, 1, bytes, out);
      fclose (in);
    }
    i++;
  }
  while (bytes != 0);
  fclose (out);
}
