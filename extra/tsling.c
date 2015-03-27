#include <stdio.h>
#ifdef TRUE
#undef TRUE
#endif
#ifdef FALSE
#undef FALSE
#endif
#include "synchem.h"
#include "synio.h"
#include "utl.h"
#include "tsd.h"
#include "sling.h"
#include "debug.h"
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

main()
{
  String_t string;
  Sling_t sling;
  Tsd_t invalid_tsd;
  TsdRow_t invalid_rows[10];
  int i, j;

  IO_Init ();
  Trace_Init ();
  IO_Init_Files("tsling.out",FALSE);
  GTrace[DB_SLINGVALIDATE].options=TL_MAJOR;
  invalid_tsd.num_atoms=10;
  invalid_tsd.atoms=invalid_rows;
  for (i=0; i<10; i++)
  {
    invalid_rows[i].flags=0;
    for (j=0; j<6; j++)
    {
      invalid_rows[i].neighbors[j].id=TSD_INVALID;
      invalid_rows[i].neighbors[j].bond=0;
    }
  }
  invalid_rows[0].atomid=6;
  invalid_rows[0].neighbors[0].id=1;
  invalid_rows[0].neighbors[0].bond=1;
  invalid_rows[0].neighbors[1].id=2;
  invalid_rows[0].neighbors[1].bond=1;
  invalid_rows[0].neighbors[2].id=8;
  invalid_rows[0].neighbors[2].bond=1;
  invalid_rows[0].neighbors[3].id=9;
  invalid_rows[0].neighbors[3].bond=1;
  invalid_rows[1].atomid=1;
  invalid_rows[1].neighbors[0].id=0;
  invalid_rows[1].neighbors[0].bond=1;
  invalid_rows[2].atomid=6;
  invalid_rows[2].neighbors[0].id=0;
  invalid_rows[2].neighbors[0].bond=1;
  invalid_rows[2].neighbors[1].id=3;
  invalid_rows[2].neighbors[1].bond=1;
  invalid_rows[3].atomid=7;
  invalid_rows[3].neighbors[0].id=4;
  invalid_rows[3].neighbors[0].bond=1;
  invalid_rows[3].neighbors[1].id=3;
  invalid_rows[3].neighbors[1].bond=1;
  invalid_rows[3].neighbors[2].id=3;
  invalid_rows[3].neighbors[2].bond=1;
  invalid_rows[3].neighbors[3].id=5;
  invalid_rows[3].neighbors[3].bond=1;
  invalid_rows[3].neighbors[4].id=2;
  invalid_rows[3].neighbors[4].bond=1;
  invalid_rows[4].atomid=1;
  invalid_rows[4].neighbors[0].id=3;
  invalid_rows[4].neighbors[0].bond=1;
  invalid_rows[5].atomid=6;
  invalid_rows[5].neighbors[0].id=3;
  invalid_rows[5].neighbors[0].bond=1;
  invalid_rows[5].neighbors[1].id=6;
  invalid_rows[5].neighbors[1].bond=1;
  invalid_rows[6].atomid=6;
  invalid_rows[6].neighbors[0].id=5;
  invalid_rows[6].neighbors[0].bond=1;
  invalid_rows[6].neighbors[1].id=7;
  invalid_rows[6].neighbors[1].bond=1;
  invalid_rows[7].atomid=6;
  invalid_rows[7].neighbors[0].id=6;
  invalid_rows[7].neighbors[0].bond=1;
  invalid_rows[8].atomid=6;
  invalid_rows[8].neighbors[0].id=0;
  invalid_rows[8].neighbors[0].bond=1;
  invalid_rows[8].neighbors[1].id=9;
  invalid_rows[8].neighbors[1].bond=1;
  invalid_rows[9].atomid=6;
  invalid_rows[9].neighbors[0].id=8;
  invalid_rows[9].neighbors[0].bond=1;
  invalid_rows[9].neighbors[1].id=0;
  invalid_rows[9].neighbors[1].bond=1;
  sling=Tsd2Sling(&invalid_tsd);
  printf("%s\n",Sling_Name_Get(sling));
/*
string=String_Create("CCC/0CNH-1CCC",0);
sling=String2Sling(string);
*/
string=String_Create("C=CC=CC=C/0CCO/6-5OC=C/-1",0);
sling=String2Sling(string);
  if (Sling_Validate(sling,NULL)) printf("\nOK\n");
  else printf ("\nInvalid sling\n");
  IO_Close_Files ();
}
