#include <stdio.h>
#include <stdlib.h>

main()
{
  FILE *f;

  f=popen("ls","r");
  pclose(f);
}
