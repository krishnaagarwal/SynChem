#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

typedef union bigarg
{
  double         dval;
  float          fval[2];
  long           lval[2];
  unsigned short sval[4];
  unsigned char  bval[8];
} BIGARG;

void tva_arg (void *stvarg, ...)
{
  va_list  ap;
  int      i;
  BIGARG   val;

  va_start (ap, stvarg);
  do
  {
    val = va_arg (ap, BIGARG);
    if (val.lval[0] != 0) printf ("char: {%d %d %d %d %d %d %d %d}\nshort: {%d %d %d %d}\nlong: {%d %d}\nfloat: {%f %f}\ndouble: %lf\n\n",
      val.bval[0], val.bval[1], val.bval[2], val.bval[3], val.bval[4], val.bval[5], val.bval[6], val.bval[7],
      val.sval[0], val.sval[1], val.sval[2], val.sval[3], val.lval[0], val.lval[1], val.fval[0],val.fval[1], val.dval);
  }
  while (val.lval[0] != 0);
}

main ()
{
  tva_arg (NULL, (unsigned char) 1, (unsigned short) 2, (long) 3, (unsigned char) 4, (unsigned short) 5, (long) 6, 0);
}
