#include <stdio.h>

/*
static char *key="Q: What is meant by six of one and a half-dozen of the other?\nA: 12.\n";
*/
static char keychars[] = {'\n', ' ', '-', '.', '1', '2', ':', '?',
  'A', 'Q', 'W', 'a', 'b', 'd', 'e', 'f', 'h', 'i', 'l', 'm', 'n', 'o', 'r', 's', 't', 'x', 'y', 'z'};
static char key_array[] = {69, 9, 6, 1, 10, 16, 11, 24, 1, 17, 23, 1, 19, 14, 11, 20, 24, 1, 12, 26, 1, 23, 17, 25, 1, 21, 15, 1,
  21, 20, 14, 1, 11, 20, 13, 1, 11, 1, 16, 11, 18, 15, 2, 13, 21, 27, 14, 20, 1, 21, 15, 1, 24, 16, 14, 1, 21, 24, 16, 14, 22, 7, 0,
  8, 6, 1, 4, 5, 3, 0};

main (int argc, char *argv[])
{
  int i, enclen, reclen;
  FILE *in, *out;
  char *buffer, encval;

  enclen = key_array[0];

/*
  for (i=1; i<=enclen; i++) putchar(keychars[key_array[i]]);
*/

  if (argc != 3)
  {
    printf ("Usage: %s <input_file> <output_file>\n", argv[0]);
    exit (1);
  }

  if ((in = fopen (argv[1], "r")) == NULL)
  {
    printf ("Error opening %s for input\n", argv[1]);
    exit (1);
  }

  if ((out = fopen (argv[2], "r")) != NULL)
  {
    printf ("Error: %s already exists.\n", argv[2]);
    fclose (in);
    fclose (out);
    exit (1);
  }

  if ((out = fopen (argv[2], "w")) == NULL)
  {
    printf ("Error opening %s for input\n", argv[2]);
    fclose (in);
    exit (1);
  }

  buffer = (char *) malloc (enclen);

  do
  {
    reclen = fread (buffer, 1, enclen, in);
    for (i = 0; i < reclen; i++)
    {
      encval = keychars[key_array[i + 1]];
      if (buffer[i] != 0 && buffer[i] != encval) buffer[i] ^= encval;
    }
    fwrite (buffer, 1, reclen, out);
  }
  while (enclen == reclen);

  fclose (in);
  fclose (out);
  free (buffer);
}
