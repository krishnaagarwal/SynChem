/* shelley2.f -- translated by f2c (version 19950314).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)

  Edited after translation to remove f2c i/o calls.  
  Linking to f2c libraries is no longer needed.  
  Daren Krebsbach  9/15/95
*/

#include <stdio.h>
#include "f2c.h"

/* Table of constant values */

static integer c__255 = 255;
static integer c__200 = 200;
static integer c__2000 = 2000;

integer bmat_(integer *bm, integer *is)
{
    /* System generated locals */
    integer ret_val;

    /* Local variables */
    static integer i;


/* THIS RETURNS THE INDEX OF THE FIRST NUMBER IN BM THAT IS EQUAL TO IS */

    /* Parameter adjustments */
    --bm;

    /* Function Body */
    i = 1;
L10:
    if (bm[i] == *is) {
	goto L20;
    }
    ++i;
    goto L10;
L20:
    ret_val = i;
    return ret_val;
} /* bmat_ */

/* Subroutine */ int rdcy_(integer *at, integer *tpct, integer *tpatcn, 
	integer *nbrdcy, integer *rdcypt, integer *cy)
{

    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer nbrb, nbcy, from[255];
    extern /* Subroutine */ int trim_(integer *, integer *, integer *);
    static integer mnpt, cypt[200], mxpt, i, j, k, l, m, ct[1530]	/* 
	    was [255][6] */;
    extern /* Subroutine */ int delbnd_(integer *, integer *, integer *, 
	    integer *, integer *);
    static integer pt, nbatcn[255];
    extern /* Subroutine */ int st_(integer *, integer *, integer *, integer *
	    , integer *, integer *, integer *), findph_(integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *, 
	    integer *, integer *, integer *), reduce_(integer *, integer *, 
	    integer *, integer *, integer *, integer *);
    static integer inrgat[20], tergat[20], prnbcy, at1, at2, icn;


/*     THIS PROCEDURE FINDS REDUCED CYCLES IN THE DESIGNATED GRAPH. */
/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE GRAPH. */
/*  TPCT(I,J) - THE JTH ATOM CONNECTED TO ATOM I. */
/*  TPATCN(I) - THE NUMBER OF ATOMS CONNECTED TO THE ATOM I. */
/* OUTPUT: */
/*  NBCY - THE NUMBER OF CYCLES. */
/*  CYPT(I) - A POINTER TO THE LOCATION IN CY OF THE ITH CYCLE. */
/*  CY(CYPT(I)) - THE LENGTH OF THE ITH CYCLE. */
/*  CY(CYPT(I)+1) ...CY(CYPT(I)+CY(CYPT(I))) - A LIST OF ATOMS IN */
/*  THE ITH CYCLE, WHERE ADJACENT ATOMS IN THE LIST ARE CONNECTED, */
/*  AND THE FIRST AND LAST ATOMS IN THE LIST ARE ALSO CONNECTED. */


/* THE CONNECTION TABLE IS WRITE PROTECTED. */

    /* Parameter adjustments */
    --cy;
    --rdcypt;
    --tpatcn;
    tpct -= 256;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	icn = tpatcn[i];
	j = 1;
L20:
	if (j > icn) {
	    goto L30;
	}
	ct[i + j * 255 - 256] = tpct[i + j * 255];
	++j;
	goto L20;
L30:
	nbatcn[i - 1] = icn;
/* L10: */
    }

/* A POINTER TO THE INITIAL CYCLE STORAGE LOCATION IS INITIALIZED. */

    pt = 1;

/* THE NUMBER OF CYCLES IS ZEROED. */

    nbcy = 0;

/* A SPANNING TREE IS CONSTRUCTED FOR THE GRAPH AND A LIST OF RING */
/* BONDS ARE INITIALIZED. */

    st_(at, ct, nbatcn, from, &nbrb, inrgat, tergat);

/* IF THE MOLECULE DOES NOT CONTAIN RING BONDS THEN SKIP CYCLE */
/* PERCEPTION. */

    if (nbrb == 0) {
	goto L60;
    }

/* TRIM BRANCHES FROM THE CONNECTION TABLE. */

    trim_(at, ct, nbatcn);

/* DELETE RING BONDS FROM THE CONNECTION TABLE. */

    delbnd_(&nbrb, inrgat, tergat, ct, nbatcn);

/* FIND CYCLES FOR EACH RING BOND PAIR.  IF IN FUNDAMENTAL CYCLE */
/* PERCEPTION MODE (MODE=1) DO NOT RENTER RING BOND PAIR INTO THE CT. */

    i = 1;
L40:
    if (i > nbrb) {
	goto L50;
    }
    at1 = inrgat[i - 1];
    at2 = tergat[i - 1];
    prnbcy = nbcy;
    findph_(ct, nbatcn, &at1, &at2, &c__255, &c__200, &c__2000, &pt, &nbcy, 
	    cypt, &cy[1]);
    if (nbcy != 200 && pt <= 2000) {
	goto L90;
    }
    reduce_(at, &prnbcy, cypt, &cy[1], nbrdcy, &rdcypt[1]);
    pt = 1;
    i__1 = *nbrdcy;
    for (j = 1; j <= i__1; ++j) {
	mnpt = rdcypt[j];
	mxpt = cy[mnpt] + mnpt;
	cypt[j - 1] = pt;
	i__2 = mxpt;
	for (k = mnpt; k <= i__2; ++k) {
	    cy[pt] = cy[k];
	    ++pt;
/* L110: */
	}
/* L100: */
    }
    prnbcy = *nbrdcy;
    nbcy = prnbcy;
    findph_(ct, nbatcn, &at1, &at2, &c__255, &c__200, &c__2000, &pt, &nbcy, 
	    cypt, &cy[1]);
    if (nbcy != 200 && pt <= 2000) {
	goto L120;
    }
    fprintf (stderr, 
      "\nShelley code:  **ERROR** cycle storage exceeded in rdcy.\n");
    exit (-1);
L120:
L90:
    l = nbatcn[at1 - 1] + 1;
    nbatcn[at1 - 1] = l;
    m = nbatcn[at2 - 1] + 1;
    nbatcn[at2 - 1] = m;
    ct[at1 + l * 255 - 256] = at2;
    ct[at2 + m * 255 - 256] = at1;
    ++i;
    goto L40;
L50:
L60:
    reduce_(at, &nbcy, cypt, &cy[1], nbrdcy, &rdcypt[1]);
    return 0;
} /* rdcy_ */

