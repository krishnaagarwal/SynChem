#include <stdio.h>
#include <string.h>

main()
{
  char *s[5000], line[200], entry_name[2][100];
  int nent, i;
  FILE *f;

  nent=0;
  f=fopen("nm.out","r");
  fgets(line,199,f);
  fgets(line,199,f);
  while(fgets(line,199,f) && line[0]>' ')
  {
    sscanf(line,"%s",entry_name[0]);
    for (i=0; i<nent; i++) if (strstr (s[i], entry_name[0]) == s[i])
	{
		sscanf(s[i], "%s", entry_name[1]);
		if (!strcmp (entry_name[0], entry_name[1])) printf ("%s%s\n", s[i], line);
	}
    s[nent]=(char *)malloc(strlen(line)+1);
    strcpy(s[nent],line);
    nent++;
  }
  fclose(f);
  printf("%d entries\n",nent);
}
