#include <stdio.h>

main ()
{
  FILE *p;
  char command[1001];
  int c;

  printf ("popen or system? ");
  c=getchar();
  while(getchar()!='\n');
  printf ("Command: ");
  fgets (command, 1000, stdin);
  while (command[0] && command[strlen(command)-1]<' ') command[strlen(command)-1]='\0';
  printf("\n%s\n\n",command);
  if (c=='p')
  {
    p = popen (command, "r");
    while ((c = getc(p))!=EOF) putchar(c);
    pclose(p);
  }
  else system (command);
}
