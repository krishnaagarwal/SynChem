/* draw2.f -- translated by f2c (version 19950314).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)

  Edited after translation to remove f2c i/o calls.  
  Linking to f2c libraries is no longer needed.  
  Daren Krebsbach  9/15/95
*/

#include <stdio.h>
#include "f2c.h"

/* Common Block Declarations */

struct {
    integer nterm;
} trmtyp_;

#define trmtyp_1 trmtyp_

/* Subroutine */ int draw2_(integer *icon, integer *nat, real *x, real *y)
{

    /* System generated locals */
    integer i__1;

    /* Local variables */
    static logical erfl;
    static integer prat[10];
    extern /* Subroutine */ int sort_(integer *, integer *);
    static integer i, j, k, l;
    extern /* Subroutine */ int coord_(integer *, integer *, integer *, real *
	    , real *, integer *, integer *, logical *);
    static integer cn[6], ct[1530]	/* was [255][6] */, nbatcn[255], 
	    nbprat;



/* THIS SUBROUTINE INTERFACES THE VT125 REGIS VECTOR GRAPHICS */
/* DISPLAY WITH THE COORDINATE GENERATION PROGRAM USED IN THE */
/* STRUCTURE DISPLAY PROGRAM OF C.A. SHELLEY.  (BASED ON DRAW1, */
/* THE CARHART DRAWING INTERFACE.) */

    /* Parameter adjustments */
    --y;
    --x;
    icon -= 7;

    /* Function Body */
    trmtyp_1.nterm = 1;
    for (i = 1; i <= 255; ++i) {
	x[i] = 0.f;
/* L1: */
	y[i] = 0.f;
    }
    if (*nat != 2) {
	goto L10;
    }
    x[1] = 2.f;
    x[2] = 3.f;
    goto L40;
L10:
    i__1 = *nat;
    for (i = 1; i <= i__1; ++i) {
	j = 1;
L60:
	if (j > 6) {
	    goto L70;
	}
	if (icon[j + i * 6] == 0) {
	    goto L70;
	}
	cn[j - 1] = icon[j + i * 6];
	++j;
	goto L60;
L70:
	--j;
	sort_(cn, &j);
	k = 2;
	l = 0;
L80:
	if (k > j) {
	    goto L90;
	}
	if (cn[k - 1] == cn[k - 2]) {
	    goto L100;
	}
	++l;
	ct[i + l * 255 - 256] = cn[k - 2];
L100:
	++k;
	goto L80;
L90:
	++l;
	ct[i + l * 255 - 256] = cn[j - 1];
/* L50: */
	nbatcn[i - 1] = l;
    }
    coord_(nat, nbatcn, ct, &x[1], &y[1], &nbprat, prat, &erfl);
    if (! erfl) {
	goto L4321;
    }
    fprintf (stderr, "\nShelley code:  ***TEMPLATE NOT FOUND***\n");
    x[1] = 0.f;
    y[1] = 0.f;
    x[2] = 0.f;
    y[2] = 0.f;
    goto L40;

/* INVERT Y COORDINATES TO GET CORRECT ORIENTATION */

L4321:
    i__1 = *nat;
    for (i = 1; i <= i__1; ++i) {
/* L200: */
	y[i] = -(doublereal)y[i];
    }
L40:
    return 0;
} /* draw2_ */

/* SORT IS CURRENTLY A DUMMY PROGRAM */
/* Subroutine */ int sort_(integer *cn, integer *j)
{
    /* Parameter adjustments */
    --cn;

    /* Function Body */
    return 0;
} /* sort_ */

