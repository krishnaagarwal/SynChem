#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef union bigarg
{
  long           lval;
  unsigned short sval[2];
  unsigned char  bval[4];
} BIGARG;

void tva_arg (void *stvarg, ...)
{
  va_list  ap;
  int      i;
  BIGARG   val;
  long    *loc_lval;

  va_start (ap, stvarg);
  do
  {
/*
    loc_lval = va_arg (ap, long);
    val.lval = (long) loc_lval;
*/
val.sval[0]=0;
val.sval[1]=va_arg(ap,int);
printf("val.sval[1]=%d\n",val.sval[1]);
    if (val.lval != 0) printf ("char: {%d %d %d %d}\nshort: {%d %d}\nlong: %d\n\n",
      val.bval[0], val.bval[1], val.bval[2], val.bval[3], val.sval[0], val.sval[1], val.lval);
  }
  while (val.lval != 0);
  va_end (ap);
}

main ()
{
  tva_arg (NULL, (unsigned char) 1, (unsigned short) 2, (long) 3, (unsigned char) 4, (unsigned short) 5, (long) 6, 0);
}
