/* shelley1.f -- translated by f2c (version 19950314).
   You must link the resulting object file with the libraries:
	-lf2c -lm   (in that order)

  Edited after translation to remove f2c i/o calls.  
  Linking to f2c libraries is no longer needed.  
  Daren Krebsbach  9/15/95
*/

#include <stdio.h>
#include "f2c.h"
#include "kodata.h"

/* Common Block Declarations */

/* Table of constant values */

static integer c__0 = 0;
static real c_b53 = 1.5708f;
static real c_b54 = 0.f;
static real c_b56 = 1.f;
static integer c__3 = 3;
static integer c__255 = 255;
static integer c__1 = 1;

/* THIS VERSION WAS FIRST MODIFIED TO RUN ON A UNIX VAX 11/780 AND THEN */
/* PORTED OVER TO A VMS VAX 11/750. */

/* THE CHANGES BELOW WERE NEEDED TO RUN ON THE VMS SYSTEM. */
/* 1 - THERE ARE SEVERAL WRITE AND FORMAT STATEMENTS THAT INCLUDED */
/* THE " CHARACTER.  THESE STATEMENTS ARE NOW COMMENTED OUT. */
/* 2 - IN SUBROUTINE "ST" THE INTEGER DECLARATION HAD TWO INSTANCES OF */
/* THE VARIABLE "AT".  IT NOW HAS THE FRAGMENT "AT,NBRB,NBAT" INSTEAD OF */
/* "AT,NBRB,NBAT,AT". */
/* 3 - IN SUBROUTINE "SYMBOL" THE STATEMENT "DATA IESC/'<33><0>'/" IS */
/* COMMENTED OUT. */


/* Subroutine */ int dbsort_(integer *a, integer *c, integer *n)
{
    static integer b, d, i, k, m, p, s[1000], t;

/* OMMENT: SORTS THE N VALUES OF A INTO ASCENDING ORDER, AND REORDERS THE 
*/
/* OMMENT: N VALUES OF C INTO THE SAME ORDER. */
/* OMMENT: NOTE THAT THIS IS A QUICKSORT.  THE SIZE OF THE STACK (S) */
/* OMMENT: MUST BE INCREASED AS THE NUMBER OF VALUES TO BE SORTED */
/* OMMENT: INCREASES. */
/* ONVERSION TO EAGLE */
/*     INTEGER T,S(1000),P,D,C(*),B,A(*) */
    /* Parameter adjustments */
    --c;
    --a;

    /* Function Body */
    s[0] = 1;
    s[1] = *n;
    p = 2;
L10:
    if (p <= 0) {
	goto L70;
    }
    b = s[p - 2];
    i = s[p - 2];
    t = s[p - 1];
    m = s[p - 1];
    p += -2;
    d = 1;
L20:
    if (i >= m) {
	goto L40;
    }
    if (a[i] <= a[m]) {
	goto L30;
    }
    k = a[i];
    a[i] = a[m];
    a[m] = k;
    k = c[i];
    c[i] = c[m];
    c[m] = k;
    d = 3 - d;
L30:
    i = i + d - 1;
    m = m - 2 + d;
    goto L20;
L40:
    --i;
    ++m;
    if (b >= i) {
	goto L50;
    }
    s[p] = b;
    s[p + 1] = i;
    p += 2;
L50:
    if (m >= t) {
	goto L60;
    }
    s[p] = m;
    s[p + 1] = t;
    p += 2;
L60:
    goto L10;
L70:
    return 0;
} /* dbsort_ */

/* Subroutine */ int dbbbso_(integer *n, integer *b, integer *a)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, j, k, l, m, limit;
    static logical sw;

/* OMMENT: SUBROUTINE DOUBLE BUBBLE SORT (DBBBSO) SORTS THE N MEMBERS */
/* OMMENT: OF ARRAY B INTO DESCENDING ORDER AND KEEPS A RECORD OF THE */
/* OMMENT: PREVIOUS INDICES OF B IN ARRAY A. */
/* ONVERSION TO EAGLE */
/*     INTEGER N,B(*),A(*) */
    /* Parameter adjustments */
    --a;
    --b;

    /* Function Body */
    i__1 = *n;
    for (i = 1; i <= i__1; ++i) {
	a[i] = i;
/* L10: */
    }
    limit = *n - 1;
    j = 1;
L100:
    if (j > limit) {
	goto L120;
    }
    if (b[j + 1] <= b[j]) {
	goto L30;
    }
    l = b[j + 1];
    b[j + 1] = b[j];
    m = a[j + 1];
    sw = FALSE_;
    k = j - 1;
    a[j + 1] = a[j];
L40:
    if (k < 1 || sw) {
	goto L50;
    }
    if (l <= b[k]) {
	goto L60;
    }
    b[k + 1] = b[k];
    a[k + 1] = a[k];
    goto L70;
L60:
    sw = TRUE_;
    ++k;
L70:
    --k;
    goto L40;
L50:
    b[k + 1] = l;
    a[k + 1] = m;
L30:
    ++j;
    goto L100;
L120:
    return 0;
} /* dbbbso_ */

/* Subroutine */ int coord_(integer *at, integer *nbatcn, integer *ct, real *
	x, real *y, integer *nbprat, integer *prat, logical *erfl)
{
    /* System generated locals */
    integer i__1, i__2, i__3;

    /* Local variables */
    static integer nbfg, nbat;
    extern /* Subroutine */ int cnml_(integer *, integer *, integer *, 
	    integer *, integer *), drct_(integer *, integer *, integer *, 
	    real *, real *, integer *, logical *);
    static integer tpct[1530]	/* was [255][6] */, i, j, k, l, bm[255];
    static real dx, dy, sx, tx[255], ty[255], largex, largey;
    static integer tpnbcn[255], frtran[255];
    static real smallx, smally;
    static integer totran[255];


/* THIS PROCEDURE ASSIGNS COORDINATES FOR A CONNECTION TABLE.  THE */
/* CT CAN BE DISCONNECTED. */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE STRUCTURE */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO ATOM I */
/*  CT(I,J) - THE JTH ATOM CONNECTED TO ATOM I */
/* OUTPUT: */
/*  X(I),Y(I) - COORDINATE FOR ATOM I */
/*  NBPRAT,PRAT - AN ATOM IN THE PRIORITY RING SYSTEM OF EACH MOLECULE */
/*  OR FRAGMENT IN THE CONNECTION TABLE IS RETURNED.  NBPRAT IS THE */
/*  NUMBER OF PRIORITY ATOMS CONTAINED IN PRAT. */
/*  ERFL - IF TRUE, THIS PROCEDURE DID NOT SUCCESSFULLY ASSIGN */
/*  COORDINATES. */

    /* Parameter adjustments */
    --prat;
    --y;
    --x;
    ct -= 256;
    --nbatcn;

    /* Function Body */
    *nbprat = 0;

/* THIS STEP ASSIGNS EACH ATOM TO A FRAGMENT. */
/* NBFG IS THE NUMBER OF FRAGMENTS IN THE CONNECTION TABLE.  BM(I) IS */
/* THE FRAGMENT NUMBER FOR ATOM I. */

    cnml_(at, &nbatcn[1], &ct[256], &nbfg, bm);

/* LOOP OVER THE FRAGMENTS, ASSIGNING COORDINATES TO EACH. */

    sx = 0.f;
    i__1 = nbfg;
    for (i = 1; i <= i__1; ++i) {
	nbat = 0;

/* PUT THE ITH FRAGMENT IN A TEMPORARY CT.  FRTRAN AND TOTRAN ENABLE 
*/
/* THE TRANSLATION OF SEQUENCE NUMBER ASSIGNMENTS BETWEEN THE TWO */
/* CONNECTION TABLES. */

	i__2 = *at;
	for (j = 1; j <= i__2; ++j) {
	    if (bm[j - 1] != i) {
		goto L30;
	    }
	    ++nbat;
	    frtran[nbat - 1] = j;
	    totran[j - 1] = nbat;
	    l = nbatcn[j];
	    tpnbcn[nbat - 1] = l;
	    k = 1;
L40:
	    if (k > l) {
		goto L50;
	    }
	    tpct[nbat + k * 255 - 256] = ct[j + k * 255];
	    ++k;
	    goto L40;
L50:
L30:
/* L20: */
	    ;
	}
	if ((i__2 = nbat - 2) < 0) {
	    goto L60;
	} else if (i__2 == 0) {
	    goto L70;
	} else {
	    goto L80;
	}

/* A ONE ATOM FRAGMENT IS ASSIGNED A COORDINATE */

L60:
	sx += 2.f;
	x[frtran[0]] = sx;
	y[frtran[0]] = 0.f;
	goto L90;

/* A TWO ATOM FRAGMENT IS ASSIGNED COORDINATES. */

L70:
	sx += 2.f;
	x[frtran[0]] = sx;
	y[frtran[0]] = 0.f;
	sx += 1.f;
	x[frtran[1]] = sx;
	y[frtran[1]] = 0.f;
	goto L90;

/* 3 ATOM OR LARGER FRAGMENTS ARE ASSIGNED COORDINATES. */

L80:

/* THE SEQUENCE NUMBERS IN THE CT ARE ASSIGNED. */

	i__2 = nbat;
	for (j = 1; j <= i__2; ++j) {
	    l = tpnbcn[j - 1];
	    i__3 = l;
	    for (k = 1; k <= i__3; ++k) {
		tpct[j + k * 255 - 256] = totran[tpct[j + k * 255 - 256] - 1];
/* L110: */
	    }
/* L100: */
	}

/* COORDINATES ARE ASSIGNED TO THIS FRAGMENT. */

	++(*nbprat);
	drct_(&nbat, tpnbcn, tpct, tx, ty, &prat[*nbprat], erfl);
	if (*erfl) {
	    return 0;
	}

/* THE FRAGMENT IS TRANSLATED TO THE CORRECT POSITION. */

	largex = tx[0];
	smallx = tx[0];
	largey = ty[0];
	smally = ty[0];
	i__2 = nbat;
	for (j = 2; j <= i__2; ++j) {
	    if (ty[j - 1] > largey) {
		largey = ty[j - 1];
	    }
	    if (ty[j - 1] < smally) {
		smally = ty[j - 1];
	    }
	    if (tx[j - 1] > largex) {
		largex = tx[j - 1];
	    }
	    if (tx[j - 1] < smallx) {
		smallx = tx[j - 1];
	    }
/* L120: */
	}
	dx = sx - smallx + 2.f;
	dy = -(doublereal)((largey + smally) / 2.f);
	i__2 = nbat;
	for (j = 1; j <= i__2; ++j) {
	    x[frtran[j - 1]] = tx[j - 1] + dx;
	    y[frtran[j - 1]] = ty[j - 1] + dy;
/* L130: */
	}
	sx = largex + dx;
L90:
/* L10: */
	;
    }
    return 0;
} /* coord_ */

/* Subroutine */ int drsm_(integer *nbsm, integer *smfl, integer *nbrdcy, 
	integer *cysm, integer *cy, integer *rdcypt, integer *cyatrk, integer 
	*at, integer *atrk, integer *smatcn, real *x, real *y, real *a, 
	integer *smct)
{

    /* System generated locals */
    integer i__1;
    real r__1;

    /* Builtin functions */
    double atan2(doublereal, doublereal), cos(doublereal), sin(doublereal);

    /* Local variables */
    static real cyag[100];
    static integer nbrg, nbcy, smbm[200], lkrk[3];
    extern /* Subroutine */ int fuse_(real *, real *, real *, real *, integer 
	    *, real *, real *, real *);
    static integer cyln, cyrk[50], cysk[100], lkpt[3], rgto[20];
    static real cyxc[100];
    static integer mnpt;
    static real cyyc[100];
    extern /* Subroutine */ int drst_(integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, real *, 
	    real *, real *), stat_(integer *, integer *, integer *, integer *,
	     integer *, integer *, integer *, integer *, integer *);
    extern doublereal smen_(real *, real *, integer *, integer *, real *);
    static integer mxpt, bgat1, bgat2, rgcn1[20], rgcn2[20];
    static real dist1, dist2;
    static integer i, j, k, l, m, n;
    static real r;
    static integer t[3];
    static real theta;
    extern /* Subroutine */ int spiro_(real *, real *, real *, real *, 
	    integer *, real *, real *, real *), bicyc2_(integer *, integer *, 
	    real *, real *, real *, real *);
    static integer bm[255], ii, jj;
    static real ta[20];
    static integer lk[50], kk;
    static real dx, dy;
    static integer sn[20];
    static real atfact[255];
    static integer pt;
    extern /* Subroutine */ int bicycl_(integer *, integer *, real *, real *, 
	    real *, real *), codecy_(integer *, integer *, integer *, integer 
	    *);
    static real tx[20], ty[20];
    extern /* Subroutine */ int dbinso_(integer *, integer *, integer *, 
	    integer *, integer *);
    static real xx[255], yy[255];
    static integer length;
    extern /* Subroutine */ int atincy_(integer *, integer *, integer *, 
	    integer *);
    static integer rgfrom[20];
    extern /* Subroutine */ int rotate_(integer *, integer *);
    static integer bm2[255], cyskln;
    static real en1;
    static integer lkcypt[3];
    static real en2;
    extern /* Subroutine */ int slrgor_(integer *, integer *, integer *, 
	    integer *, integer *), nvertm_(integer *, integer *);
    static integer smcypt[50];
    extern /* Subroutine */ int rgstsr_(integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *);
    static integer pt1, pt2;
    static real taa[20];
    static integer inb, iat, irk, snn[20];
    static real txx[20], tyy[20];
    static integer iat1, iat2, iat3, iat4;


/* THIS PROCEDURE ASSIGNS COORDINATES TO RING SYSTEMS. */

/* INPUT: */
/*  NBSM - THE NUMBER OF RING SYSTEMS. */
/*  SMFL(I) - THE COMPLEXITY OF THE ITH RING SYSTEM, WHERE: SMFL(I)= */
/*  (NUMBER OF CYCLES IN SYSTEM I)-(NUMBER OF EDGES IN SYSTEM I)+ */
/*  (NUMBER OF ATOMS IN SYSTEM I)-(1). */
/*  NBRDCY - THE NUMBER OF CYCLES IN THE REDUCED SET. */
/*  CYSM - A POINTER TO THE RING SYSTEM IN WHICH THE REDUCED CYCLE */
/*  RESIDES. */
/*  CY(I) - THE LENGTH OF CYCLE I, WHERE I IS A POINTER. */
/*  CY(I+1)...CY(I+CY(I)) - THE ATOMS IN THE ITH CYCLE. */
/*  RDCYPT - POINTERS TO CYCLES IN THE REDUCED SET. */
/*  CYATRK(I) - A HASH CODE FOR ATOM I BASED ON THE CYCLIC PORTION */
/*  OF THE MOLECULE ONLY. */
/*  AT - THE NUMBER OF ATOMS IN A MOLECULE. */
/*  ATRK - A HASH CODE FOR ATOM I BASED ON THE ENTIRE MOLECULE. */
/*  SMATCN(I) - THE NUMBER OF CYCLIC EDGES ATTACHED TO ATOM I. */
/*  SMCT(I,J) - THE JTH CYCLIC ATOM CONNECTED TO ATOM I. */
/* OUTPUT: */
/*  X,Y - X AND Y COORDINATES. */
/*  A - THE ANGLE AT WHICH SUBSTITUENTS TO THIS ATOM SHOULD */
/*  BE DIRECTED. */


/* THE NUMBER OF CYCLES IN THE CYCLE STACK IS INITIALIZED TO ZERO. */

    /* Parameter adjustments */
    smct -= 256;
    --a;
    --y;
    --x;
    --smatcn;
    --atrk;
    --cyatrk;
    --rdcypt;
    --cy;
    --cysm;
    --smfl;

    /* Function Body */
    cyskln = 0;

/* AN X VALUE OF -100000 INDICATES THAT THE ATOM HAS NOT BEEN ASSIGNED */
/* A COORDINATE. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	atfact[i - 1] = 1.f;
	x[i] = -1e5f;
/* L1070: */
    }
    m = 1;
L600:
    if (m > *nbsm) {
	goto L10;
    }

/* TREAT RING SYSTEMS OF DIFFERENT COMPLEXITIES AS SEPARATE CASES. */
/* IF SMFL=0 THEN THE SYSTEM IS SIMPLE AND WILL BE TREATED ALGORITH- */
/* MICALLY.  IF SMFL=1 THEN THE SYSTEM COULD CONTAIN A SIMPLE BICYCLIC */
/* STRUCTURE (LIKE NORBORNANE), THIS CAN ALSO BE TREATED ALGORITHMICALLY 
*/
/* HOWEVER FURTHER PERCEPTION IS REQUIRED.  IF SMFL=2 THEN THE SYSTEM */
/* WILL BE TREATED WITH A TEMPLATE OR CLASSICAL MECHANICAL APPROACH. */

    if ((i__1 = smfl[m] - 1) < 0) {
	goto L20;
    } else if (i__1 == 0) {
	goto L30;
    } else {
	goto L40;
    }
L20:

/* THE RING SYSTEM IS SIMPLE.  CREATE A LIST (SMCYPT,NBCY) OF CYCLES */
/* IN THIS SYSTEM AND THE CORRESPONDING LIST OF IDENTIFIERS (CYRK) */
/* FOR THESE CYCLES.  ALSO, SAVE A POINTER (PT) TO THE CYCLE WITH THE */
/* LARGEST INTEGER CYCLE IDENTIFIER (THE PRIORITY CYCLE). */

    j = 0;
    nbcy = 0;
    k = 0;
    i__1 = *nbrdcy;
    for (i = 1; i <= i__1; ++i) {
	if (m != cysm[i]) {
	    goto L60;
	}
	++nbcy;
	codecy_(&cy[1], &rdcypt[i], &cyrk[nbcy - 1], &cyatrk[1]);
	smcypt[nbcy - 1] = rdcypt[i];
	if (k > cyrk[nbcy - 1]) {
	    goto L70;
	}
	if (k != cyrk[nbcy - 1]) {
	    goto L75;
	}
	codecy_(&cy[1], &rdcypt[i], &n, &atrk[1]);
	if (j >= n) {
	    goto L70;
	}
L75:
	j = n;
	l = nbcy;
	k = cyrk[nbcy - 1];
	pt = rdcypt[i];
L70:
L60:
/* L50: */
	;
    }

/* IN THE FOLLOWING SECTION, AN ORDERED SPANNING TREE IS CONSTRUCTED */
/* FOR THE CYCLES IN THE RING SYSTEM.  THE SPANNING TREE DETERMINES */
/* THE ORDER OF CONSTRUCTING COORDINATES FOR THE CYCLES OF THE RING */
/* SYSTEM.  THE PRIORITY CYCLE IS THE ROOT NODE IN THE TREE.  A BIT */
/* MAP FOR THE ATOMS IN THE PRIORITY CYCLE (BM) IS INITIALIZED. */
/* ANOTHER BITMAP FOR CYCLES IN THE SPANNING TREE (SMBM) IS INITIALIZED */
/* RGFROM AND RGTO ARE THE EDGES IN THE SPANNING TREE.  RGCN1(I) AND */
/* RGCN2(I) REPRESENT THE ATOM (RGCN1 ONLY) OR ATOMS (BOTH) IN COMMON */
/* FOR THE ITH PAIR OF CYCLES FOR THE ITH EDGE IN THE SPANNING TREE. */

    i__1 = nbcy;
    for (i = 1; i <= i__1; ++i) {
	smbm[i - 1] = 0;
/* L80: */
    }
    smbm[l - 1] = 1;
    mnpt = pt + 1;
    mxpt = pt + cy[pt];
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	bm[i - 1] = 0;
/* L90: */
    }
    i__1 = mxpt;
    for (i = mnpt; i <= i__1; ++i) {
	bm[cy[i] - 1] = 1;
/* L100: */
    }
    rgstsr_(&cy[1], bm, &nbcy, smbm, smcypt, cyrk, rgfrom, rgto, rgcn1, rgcn2,
	     &nbrg, at, &pt, &c__0, &c__0);

/* IN THIS SECTION THE ORIENTATION OF THE PRIORITY CYCLE IS SELECTED */
/* AND COORDINATES ASSIGNED.  FIRST, A PRIORITY ATOM IN THE PRIORITY */
/* CYCLE (L) IS SELECTED BASED ON AVAILABLE IDENTIFIERS. */

    k = 0;
    irk = 0;
    i__1 = mxpt;
    for (i = mnpt; i <= i__1; ++i) {
	if (cyatrk[cy[i]] == k) {
	    goto L120;
	}
	if (cyatrk[cy[i]] < k) {
	    goto L121;
	}
	k = cyatrk[cy[i]];
	irk = atrk[cy[i]];
	l = cy[i];
L121:
	goto L123;
L120:
	if (atrk[cy[i]] <= irk) {
	    goto L122;
	}
	k = cyatrk[cy[i]];
	irk = atrk[cy[i]];
	l = cy[i];
L122:
L123:
/* L110: */
	;
    }

/* SECOND, THE PREFERED ORIENTATION OF THE CYCLE IS SELECTED. */

    slrgor_(&cy[1], &pt, &l, &cyatrk[1], &atrk[1]);

/* THIRD, COORDINATES ARE ASSIGNED TO THE PRIORITY CYCLE. */

    cyln = cy[pt];
    spiro_(&c_b53, &c_b54, &c_b54, &c_b56, &cyln, cyxc, cyyc, cyag);
    i__1 = cyln;
    for (i = 1; i <= i__1; ++i) {
	x[cy[pt + i]] = cyxc[i - 1];
	y[cy[pt + i]] = cyyc[i - 1];
	a[cy[pt + i]] = cyag[i - 1];
/* L130: */
    }

/* NOW, COORDINATES ARE ASSIGNED TO ALL OTHER CYCLES IN THE RING */
/* SYSTEM USING THE SELECTED SPANNING TREE STRATEGY. */

    drst_(&nbrg, rgto, rgfrom, rgcn1, rgcn2, &cy[1], &cyatrk[1], &atrk[1], &x[
	    1], &y[1], &a[1]);
    goto L140;
L30:

/* SINCE SMFL=1, FURTHER ANALYSIS OF THE RING SYSTEM IS REQUIRED. */
/* INITIALIZE A LIST OF CYCLES IN THIS SYSTEM (N,SMCYPT) AND THE */
/* RANK FOR EACH CYCLE (CYRK). */

    n = 0;
    i__1 = *nbrdcy;
    for (j = 1; j <= i__1; ++j) {
	if (cysm[j] != m) {
	    goto L160;
	}
	++n;
	smcypt[n - 1] = rdcypt[j];
	codecy_(&cy[1], &rdcypt[j], &cyrk[n - 1], &cyatrk[1]);
L160:
/* L150: */
	;
    }

/* COUNT THE NUMBER OF ATOMS COMMON TO EACH PAIR OF CYCLES (K) UNTIL */
/* 2 CYCLES ARE FOUND THAT HAVE AT LEAST 3 ATOMS IN COMMON.  SAVE THE */
/* POINTERS TO THESE 2 CYCLES (PT1,PT2). */

    pt1 = 0;
    i = 1;
L170:
    if (i == n) {
	goto L180;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	bm[j - 1] = 0;
/* L190: */
    }
    mnpt = smcypt[i - 1];
    mxpt = cy[mnpt] + mnpt;
    ++mnpt;
    i__1 = mxpt;
    for (j = mnpt; j <= i__1; ++j) {
	bm[cy[j] - 1] = 1;
/* L200: */
    }
    j = i + 1;
L210:
    if (j > n) {
	goto L220;
    }
    mnpt = smcypt[j - 1];
    mxpt = cy[mnpt] + mnpt;
    ++mnpt;
    k = 0;
    i__1 = mxpt;
    for (l = mnpt; l <= i__1; ++l) {
	if (bm[cy[l] - 1] == 0) {
	    goto L240;
	}
	++k;
L240:
/* L230: */
	;
    }
    if (k < 3) {
	goto L250;
    }
    pt1 = smcypt[i - 1];
    pt2 = smcypt[j - 1];
    i = n - 1;
    j = n;
L250:
    ++j;
    goto L210;
L220:
    ++i;
    goto L170;
L180:

/* SOME SYSTEMS WITH SMFL=1 DO NOT HAVE TWO CYCLES IN THE REDUCED */
/* CYCLE SET WITH 3 OR MORE ATOMS IN COMMON.  IF SUCH A SYSTEM IS */
/* ENCOUNTERED, SET THE SMFL FLAG TO 1000 AND PROCESS CYCLIC SYSTEM */
/* OVER AGAIN. */

    if (pt1 != 0) {
	goto L2100;
    }
    smfl[m] = 1000;
    goto L600;
L2100:

/* ROTATE THE SECOND CYCLE UNTIL THE FIRST ATOM IS AN ATOM NOT */
/* PRESENT IN THE FIRST CYCLE. */

L260:
    if (bm[cy[mnpt] - 1] == 0) {
	goto L270;
    }
    rotate_(&cy[1], &pt2);
    goto L260;
L270:

/* NOW, ROTATE THE SECOND CYCLE UNTIL THE FIRST ATOM IS AN ATOM */
/* PRESENT IN THE SECOND CYCLE. */

L280:
    rotate_(&cy[1], &pt2);
    if (bm[cy[mnpt] - 1] == 0) {
	goto L280;
    }

/* THE FIRST K ATOMS IN THE SECOND CYCLE ARE A POTENTIAL BRIDGE */
/* LINK IN A BICYCLIC SYSTEM, SO SAVE THIS PATH. */

    lkpt[0] = 1;
    lk[0] = k;
    i__1 = k;
    for (i = 1; i <= i__1; ++i) {
	lk[i] = cy[pt2 + i];
/* L290: */
    }
    i = k + 2;

/* THE LAST ATOMS IN THE SECOND CYCLE ARE NOT COMMON TO THE FIRST */
/* CYCLE AND COULD BE A POTENTIAL BRIDGE LINK IN A BICYCLIC SYSTEM, */
/* SO SAVE THIS LINK. */

    l = cy[pt2] - k;
    lkpt[1] = i;
    lk[i - 1] = l + 2;
    ++l;
    i__1 = l;
    for (j = 1; j <= i__1; ++j) {
	lk[i + j - 1] = cy[pt2 + k + j - 1];
/* L300: */
    }
    i = i + l + 1;
    lk[i - 1] = cy[mnpt];

/* INITIALIZE AN ATOM BITMAP FOR THE SECOND CYCLE. */

    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	bm[j - 1] = 0;
/* L310: */
    }
    i__1 = mxpt;
    for (j = mnpt; j <= i__1; ++j) {
	bm[cy[j] - 1] = 1;
/* L320: */
    }

/* NOW ROTATE THE FIRST CYCLE UNTIL THE FIRST ATOM IS AN ATOM */
/* PRESENT IN THE SECOND CYCLE. */

    mnpt = pt1 + 1;
L330:
    if (bm[cy[mnpt] - 1] == 1) {
	goto L340;
    }
    rotate_(&cy[1], &pt1);
    goto L330;
L340:

/* ROTATE THE FIRST CYCLE UNTIL THE SECOND ATOM IS AN ATOM NOT */
/* PRESENT IN THE SECOND CYCLE. */

    l = mnpt + 1;
L350:
    if (bm[cy[l] - 1] == 0) {
	goto L360;
    }
    rotate_(&cy[1], &pt1);
    goto L350;
L360:

/* THE FIRST ATOMS IN THE FIRST CYCLE ARE A POTENTIAL BRIDGE LINK */
/* IN A BICYCLIC SYSTEM, SO SAVE THIS PATH. */

    l = cy[pt1] - k;
    l += 2;
    ++i;
    lkpt[2] = i;
    lk[i - 1] = l;
    i__1 = l;
    for (j = 1; j <= i__1; ++j) {
	lk[i + j - 1] = cy[pt1 + j];
/* L370: */
    }

/* SELECT THE PRIORITY BRIDGE ATOM. */

    bgat1 = lk[1];
    bgat2 = lk[lk[0]];
    if (cyatrk[bgat1] == cyatrk[bgat2]) {
	goto L390;
    }
    if (cyatrk[bgat1] > cyatrk[bgat2]) {
	goto L400;
    }
    l = bgat1;
    bgat1 = bgat2;
    bgat2 = l;
L400:
    goto L410;
L390:
    if (atrk[bgat1] >= atrk[bgat2]) {
	goto L420;
    }
    l = bgat1;
    bgat1 = bgat2;
    bgat2 = l;
L420:
L410:

/* NOW LET US MAKE SURE THAT THE PATHS NOT COMMON IN THE TWO CYCLES */
/* CAN FORM A THIRD CYCLE THAT IS IN THE REDUCED CYCLE SET.  ANY */
/* BICYCLIC SYSTEM WILL FIT THIS CONDITION.  A SYSTEM SUCH AS CUBANE */
/* DOES NOT.  I AM ASSUMING THAT THIS CONDITION IS SUFFICIENT TO */
/* DETECT ALL BICYCLIC SYSTEMS.  ALSO, A SYSTEM BITMAP FOR CYCLES */
/* (FROM 1 TO NBRDCY) IS INITIALIZED WITH THE TWO CYCLES PREVIOUSLY */
/* PERCEIVED. */

    i__1 = *at;
    for (l = 1; l <= i__1; ++l) {
	bm[l - 1] = 0;
/* L440: */
    }
    for (l = 2; l <= 3; ++l) {
	mnpt = lkpt[l - 1];
	mxpt = mnpt + lk[mnpt - 1];
	++mnpt;
	i__1 = mxpt;
	for (j = mnpt; j <= i__1; ++j) {
	    bm[lk[j - 1] - 1] = 1;
/* L460: */
	}
/* L450: */
    }
    i__1 = n;
    for (i = 1; i <= i__1; ++i) {
	smbm[i - 1] = 0;
/* L475: */
    }
    k = 0;
    i = 1;
L470:
    if (i > n) {
	goto L480;
    }
    if (pt1 == smcypt[i - 1]) {
	goto L490;
    }
    if (pt2 == smcypt[i - 1]) {
	goto L490;
    }
    mnpt = smcypt[i - 1];
    mxpt = cy[mnpt] + mnpt;
    ++mnpt;
    i__1 = *at;
    for (l = 1; l <= i__1; ++l) {
	bm2[l - 1] = 0;
/* L455: */
    }
    i__1 = mxpt;
    for (l = mnpt; l <= i__1; ++l) {
	bm2[cy[l] - 1] = 1;
/* L465: */
    }
    l = 1;
L500:
    if (l > *at) {
	goto L510;
    }
    if (bm[l - 1] != bm2[l - 1]) {
	goto L510;
    }
    ++l;
    goto L500;
L510:
    if (l <= *at) {
	goto L520;
    }
    k = i;
    i = n;
L520:
    goto L530;
L490:
    smbm[i - 1] = 1;
L530:
    ++i;
    goto L470;
L480:

/* IF K=0 THIS IS NOT A BICYCLIC SYSTEM SO ASSIGN A LARGER COMPLEXITY */
/* AND PROCESS IT DIFFERENTLY. */

    if (k == 0) {
	goto L540;
    }

/* SET THE BIT FOR THE THIRD CYCLE. */

    smbm[k - 1] = 1;

/* NOW ORDER THE LINKS OF THE BICYCLIC SYSTEM BASED ON CYATRK AND */
/* THEN ON ATRK VALUES. */

    for (l = 1; l <= 3; ++l) {
	codecy_(lk, &lkpt[l - 1], &t[l - 1], &cyatrk[1]);
	mnpt = lkpt[l - 1];
	mxpt = mnpt + lk[mnpt - 1] - 1;
	mnpt += 2;
L900:
	if (mnpt > mxpt) {
	    goto L910;
	}
	if (smatcn[lk[mnpt - 1]] > 2) {
	    goto L910;
	}
	++mnpt;
	goto L900;
L910:
	if (mnpt <= mxpt) {
	    t[l - 1] += 1000000000;
	}
	codecy_(lk, &lkpt[l - 1], &lkrk[l - 1], &atrk[1]);
/* L550: */
    }
    lkcypt[0] = pt2;
    lkcypt[2] = pt1;
    lkcypt[1] = smcypt[k - 1];
    dbinso_(t, lkrk, &c__3, lkpt, lkcypt);

/* INVERT LINKS, IF NECESSARY, TO PREPARE FOR BICYCLIC COORDINATE */
/* GENERATION. */

    pt = lkpt[0];
    if (bgat1 != lk[pt]) {
	nvertm_(lk, &pt);
    }
    pt = lkpt[1];
    if (bgat2 != lk[pt]) {
	nvertm_(lk, &pt);
    }
    pt = lkpt[2];
    if (bgat1 != lk[pt]) {
	nvertm_(lk, &pt);
    }

/* IF A BRIDGEHEAD ATOM IS NOT FUSED TO ANOTHER RING THEN ASSIGN */
/* COORDINATES FOR THE FLAT BICYCLIC SYSTEM, OTHERWISE ASSIGN */
/* COORDINATES FOR THE PSEUDO THREE DIMENSIONAL DIAGRAM. */

    if (smatcn[bgat1] > 3) {
	goto L380;
    }
    if (smatcn[bgat2] > 3) {
	goto L380;
    }
    bicycl_(lk, lkpt, &c_b56, &x[1], &y[1], &a[1]);
    goto L610;
L380:
    bicyc2_(lk, lkpt, &c_b56, &x[1], &y[1], &a[1]);
L610:

/* NOW ASSIGN COORDINATES TO THE TREE OF CYCLES ATTACHED TO EACH OF */
/* THE THREE BRIDGE LINKS.  THIS IS DONE IN THE SAME MANNER AS FOR */
/* SIMPLE RING SYSTEMS.  EFFECTIVELY, THE THREE CYCLES IN THE */
/* BICYCLIC SYSTEM ARE EACH CONSIDERED AS PRIORITY CYCLES. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	bm[i - 1] = 0;
/* L580: */
    }
    for (i = 1; i <= 3; ++i) {
	pt = lkpt[i - 1];
	pt1 = lk[pt];
	pt2 = lk[pt + 1];
	pt = lkcypt[i - 1];
	l = pt + 1;
L710:
	if (cy[l] == pt1) {
	    goto L720;
	}
	rotate_(&cy[1], &pt);
	goto L710;
L720:
	if (cy[l + 1] != pt2) {
	    nvertm_(&cy[1], &pt);
	}
/* L700: */
    }
    l = 3;
L565:
    if (l == 0) {
	goto L560;
    }
    mnpt = lkpt[l - 1];
    mxpt = mnpt + lk[mnpt - 1];
    ++mnpt;
    i__1 = mxpt;
    for (i = mnpt; i <= i__1; ++i) {
	bm[lk[i - 1] - 1] = 1;
/* L570: */
    }

/* SELECT A SPANNING TREE STRATEGY FOR THIS LINK. */

    rgstsr_(&cy[1], bm, &n, smbm, smcypt, cyrk, rgfrom, rgto, rgcn1, rgcn2, &
	    nbrg, at, &lkcypt[l - 1], &bgat1, &bgat2);
    i = 1;

/* ASSIGN COORDINATES BASED ON THE SPANNING TREE STRATEGY. */

    drst_(&nbrg, rgto, rgfrom, rgcn1, rgcn2, &cy[1], &cyatrk[1], &atrk[1], &x[
	    1], &y[1], &a[1]);
    --l;
    goto L565;
L560:
    goto L590;
L540:

/* AN SMFL VALUE OF 1000 INDICATES A RING SYSTEM OF TYPE 1 WAS NOT */
/* BICYCLIC AND COORDINATES WERE NOT ASSIGNED. */

    smfl[m] = 1000;

/* THE RING SYSTEM POINTER IS DECREMENTED SO THAT THIS SYSTEM IS */
/* PROCESSED DIFFERENTLY THE NEXT TIME THROUGH THE LOOP. */

    --m;
L590:
    goto L140;
L40:

/* THIS IS A COMPLEX RING SYSTEM.  THE RING SYSTEM IS SEARCHED FOR AN */
/* ATOM OF CYCLIC DEGREE 2 THAT WHEN REMOVED FROM THE RING SYSTEM WILL */
/* REDUCE THE NUMBER OF CYCLES THE MOST.  IAT IS THE ATOM.  INB IS THE */
/* NUMBER OF CYCLES THAT WILL BE REMOVED FROM THE RING SYSTEM BY */
/* REMOVING THIS ATOM. */

    stat_(nbrdcy, &rdcypt[1], &cy[1], &cysm[1], &m, at, &cyatrk[1], &iat, &
	    inb);

/* WAS AN ATOM THAT COULD BE REMOVED FROM THE RING SYSTEM FOUND? */

    if (inb == 0) {
	goto L1060;
    }

/* YES, IF THE SYSTEM COMPLEXITY IS 1000 THEN CHANGE IT BACK TO */
/* ONE, WHICH IS THE TRUE COMPLEXITY. */

    if (smfl[m] == 1000) {
	smfl[m] = 1;
    }

/* CALCULATE THE COMPLEXITY AFTER REMOVING THE ATOM. */

    smfl[m] = smfl[m] + 1 - inb;

/* LOOP OVER THE REDUCED CYCLE SET.  SUBSTRACT 1000 FROM THE SYSTEM */
/* POINTER FOR EACH CYCLE IN THE RING SYSTEM THAT CONTAINS THE ATOM */
/* TO BE REMOVED.  ALSO, SAVE A POINTER (PT) TO THE SMALLEST CYCLE */
/* IN THE RING SYSTEM THAT CONTAINS THIS ATOM. */

    length = 10000;
    i__1 = *nbrdcy;
    for (i = 1; i <= i__1; ++i) {
	if (m != cysm[i]) {
	    goto L1010;
	}
	atincy_(&cy[1], &rdcypt[i], &iat, &j);
	if (j == 0) {
	    goto L1040;
	}
	cysm[i] += -1000;
	if (cy[rdcypt[i]] >= length) {
	    goto L1050;
	}
	pt = i;
	length = cy[rdcypt[i]];
L1050:
L1040:
L1010:
/* L1000: */
	;
    }

/* PUT THE POINTER TO THE SMALLEST CYCLE ON A STACK. */

    ++cyskln;
    cysk[cyskln - 1] = pt;

/* DECREMENT THE POINTER TO THE RING SYSTEM.  THE RING SYSTEM WILL */
/* BE PROCESSED OVER AGAIN IN THE NEXT ITERATION OF THE OUTERLOOP. */
/* HOWEVER, THE COMPLEXITY HAS BEEN REDUCED SUBSTANTIALLY.  THIS */
/* SIMPLIFYING PROCESS CONTINUES UNTIL THE RING SYSTEM CAN BE */
/* PROCESSED OR A RING SYSTEM NUCLEUS IS OBTAINED THAT CANNOT BE */
/* SIMPLIFIED.  EFFECTIVELY, OUTER CYCLES ARE REMOVED AND STORED */
/* IN THE STACK UNTIL COORDINATES CAN BE ASSIGNED TO IT. */
/* THE OUTER CYCLES IN THE STACK ARE PROCESSED IN THE REVERSE ORDER */
/* OF REMOVING THEM BEFORE THE PROCEDURE RETURNS. */

    --m;
    goto L1350;
L1060:

/* NO, OUTPUT ERROR MESSAGE. */

fprintf (stderr, "\nShelley code:  **ERROR** RING SYSTEM TOO COMPLEX\n");
L1350:
L140:
    ++m;
    goto L600;
L10:

/* LOOP OVER THE CYCLES IN THE STACK OF THOSE REMOVED FROM COMPLEX */
/* RING SYSTEMS; ASSIGN COORDINATES TO ATOMS IN THESE CYCLES THAT */
/* DO NOT HAVE COORDINATES ASSIGNED. */

    i = cyskln;
L1080:
    if (i == 0) {
	goto L1090;
    }
    pt = rdcypt[cysk[i - 1]];
    mxpt = pt + cy[pt];
    mnpt = pt + 1;

/* IT IS POSSIBLE TO TAKE CYCLES FROM A RING SYSTEM AND PUT THEM */
/* ON THE STACK TO SIMPLIFY THE RING SYSTEM BUT THE NUCLEUS OF THE */
/* RING SYSTEM CANNOT BE TREATED.  THEREFORE, CYCLES ON THE STACK */
/* WHICH DO NOT HAVE COORDINATES ASSIGNED TO ANY OF THEIR ATOMS */
/* SHOULD BE SKIPPED OVER. */

    k = mnpt;
L2110:
    if (k > mxpt) {
	goto L2120;
    }
    if (x[cy[k]] > -99999.f) {
	goto L2120;
    }
    ++k;
    goto L2110;
L2120:
    if (k > mxpt) {
	goto L2130;
    }
    en1 = 1e25f;
    dist1 = 1e25f;

/* SET THE ATOM MAP (BM) TO 1 FOR EACH ATOM IN THE RING SYSTEM */
/* WHICH HAS BEEN ASSIGNED A COORDINATE, OTHERWISE SET TO 0.  STORE */
/* COORDINATES (XX,YY) FOR EACH ATOM THAT HAS BEEN ASSIGNED */
/* COORDINATES IN THIS RING SYSTEM. */

    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	bm[j - 1] = 0;
/* L1390: */
    }
    j = 1;
L1400:
    if (j > *nbrdcy) {
	goto L1360;
    }
    if (cysm[j] != cysm[cysk[i - 1]] + 1000) {
	goto L1370;
    }
    m = rdcypt[j];
    n = m + cy[m];
    ++m;
    i__1 = n;
    for (ii = m; ii <= i__1; ++ii) {
	if (bm[cy[ii] - 1] == 1) {
	    goto L1385;
	}
	bm[cy[ii] - 1] = 1;
	xx[cy[ii] - 1] = x[cy[ii]];
	yy[cy[ii] - 1] = y[cy[ii]];
L1385:
/* L1380: */
	;
    }
L1370:
    ++j;
    goto L1400;
L1360:

/* THERE ARE FOUR POSSIBLE WAYS TO ASSIGN COORDINATES TO THE ATOMS */
/* IN THE CYCLE.  THE ONE THAT GIVES THE LOWEST ENERGY IS USED. */

    for (ii = 1; ii <= 2; ++ii) {

/* ROTATE THE CYCLE UNTIL AN ATOM WITHOUT AN ASSIGNED COORDINATE */
/* IS FIRST. */

L1100:
	if (x[cy[mnpt]] < -99999.f) {
	    goto L1110;
	}
	rotate_(&cy[1], &pt);
	goto L1100;
L1110:

/* ROTATE THE CYCLE UNTIL AN ATOM WITH AN ASSIGNED COORDINATE IS */
/* FIRST. */

L1120:
	if (x[cy[mnpt]] > -99999.f) {
	    goto L1130;
	}
	rotate_(&cy[1], &pt);
	goto L1120;
L1130:

/* COORDINATES ARE ASSIGNED CLOCKWISE, STARTING WITH THE FIRST ATOM */
/* AND PROCEEDING TO THE SECOND. */

	iat1 = cy[mnpt];
	iat3 = iat1;
	iat2 = cy[mnpt + 1];
	fuse_(&x[iat1], &y[iat1], &x[iat2], &y[iat2], &cy[pt], cyxc, cyyc, 
		cyag);
	jj = 0;
	j = cy[pt] - 1;
	n = mxpt;
L1200:
	if (x[cy[n]] > -99999.f) {
	    goto L1210;
	}
	++jj;
	sn[jj - 1] = cy[n];
	tx[jj - 1] = cyxc[j - 1];
	ty[jj - 1] = cyyc[j - 1];
	ta[jj - 1] = cyag[j - 1];
	bm[cy[n] - 1] = 1;
	xx[cy[n] - 1] = cyxc[j - 1];
	yy[cy[n] - 1] = cyyc[j - 1];
	--j;
	--n;
	goto L1200;
L1210:
	iat4 = cy[n];

/* THE ENERGY IS CALCULATED FOR THESE COORDINATES. */

	en2 = smen_(xx, yy, at, bm, atfact);

/* IF IT HAS DECREASED, THE COORDINATES ARE SAVED. */

	if (en2 > en1) {
	    goto L1240;
	}
	if (en1 > 1e4f) {
	    goto L1245;
	}
	dx = x[cy[n + 1]] - x[iat4];
	dy = y[cy[n + 1]] - y[iat4];
	r = dx * dx + dy * dy;
	if (r < .25f) {
	    goto L1240;
	}
	dist2 = (r__1 = 1.f - r, dabs(r__1));
	if (dist2 > dist1) {
	    goto L1240;
	}
L1245:
	en1 = en2;
	i__1 = jj;
	for (j = 1; j <= i__1; ++j) {
	    snn[j - 1] = sn[j - 1];
	    txx[j - 1] = tx[j - 1];
	    tyy[j - 1] = ty[j - 1];
	    taa[j - 1] = ta[j - 1];
/* L1250: */
	}
L1240:

/* COORDINATES ARE ASSIGNED COUNTERCLOCKWISE, STARTING WITH THE FIRST 
*/
/* ATOM AND PROCEEDING TO THE SECOND. */

	fuse_(&x[iat2], &y[iat2], &x[iat1], &y[iat1], &cy[pt], cyxc, cyyc, 
		cyag);
	jj = 0;
	j = 2;
	n = mxpt;
L1270:
	if (x[cy[n]] > -99999.f) {
	    goto L1280;
	}
	++jj;
	sn[jj - 1] = cy[n];
	tx[jj - 1] = cyxc[j - 1];
	ty[jj - 1] = cyyc[j - 1];
	ta[jj - 1] = cyag[j - 1];
	xx[cy[n] - 1] = cyxc[j - 1];
	yy[cy[n] - 1] = cyyc[j - 1];
	++j;
	--n;
	goto L1270;
L1280:

/* THE ENERGY IS CALCULATED FOR THESE COORDINATES. */

	en2 = smen_(xx, yy, at, bm, atfact);

/* IF THE ENERGY HAS DECREASED, THE COORDINATES ARE SAVED. */

	if (en2 > en1) {
	    goto L1290;
	}
	if (en1 > 1e4f) {
	    goto L1295;
	}
	dx = x[cy[n + 1]] - x[iat4];
	dy = y[cy[n + 1]] - y[iat4];
	r = dx * dx + dy * dy;
	if (r < .25f) {
	    goto L1290;
	}
	dist2 = (r__1 = 1.f - r, dabs(r__1));
	if (dist2 > dist1) {
	    goto L1290;
	}
L1295:
	en1 = en2;
	i__1 = jj;
	for (j = 1; j <= i__1; ++j) {
	    snn[j - 1] = sn[j - 1];
	    txx[j - 1] = tx[j - 1];
	    tyy[j - 1] = ty[j - 1];
	    taa[j - 1] = ta[j - 1];
/* L1300: */
	}
L1290:

/* THE LIST OF ATOMS IN THE CYCLE IS INVERTED AND THE PROCESS IS */
/* REPEATED. */

	nvertm_(&cy[1], &pt);
/* L1310: */
    }

/* THE LOWEST ENERGY SET OF COORDINATES IS USED. */

    i__1 = jj;
    for (j = 1; j <= i__1; ++j) {
	iat = snn[j - 1];
	x[iat] = txx[j - 1];
	y[iat] = tyy[j - 1];
	a[iat] = taa[j - 1];
/* L1320: */
    }

/* THE ANGLE IS CORRECTED WHERE THE CYCLE IS FUSED TO THE NUCLEUS */
/* RING SYSTEM. */

L7040:
    en1 = 1e25f;
    j = 1;
    if (smatcn[iat3] != 3) {
	goto L7015;
    }
L7000:
    if (j > smatcn[iat3]) {
	goto L7010;
    }
    if (x[smct[iat3 + j * 255]] < -99999.f) {
	goto L7010;
    }
    theta = atan2(y[iat3] - y[smct[iat3 + j * 255]], x[iat3] - x[smct[iat3 + 
	    j * 255]]);
    kk = *at + 1;
    x[kk] = x[iat3] + cos(theta);
    y[kk] = y[iat3] + sin(theta);
    bm[kk - 1] = 1;
    atfact[kk - 1] = 1.f;
    en2 = smen_(&x[1], &y[1], &kk, bm, atfact);
    if (en2 >= en1) {
	goto L7005;
    }
    a[iat3] = theta;
    en1 = en2;
L7005:
    ++j;
    goto L7000;
L7010:
L7015:
    if (iat3 == iat4) {
	goto L7030;
    }
    iat3 = iat4;
    goto L7040;
L7030:
L2130:

/* THE CYCLE IS ADDED TO THE RING SYSTEM SINCE OTHER CYCLES MIGHT BE */
/* CONNECTED TO IT THAT ARE STILL ON THE STACK. */

    cysm[cysk[i - 1]] += 1000;
    --i;
    goto L1080;
L1090:
    return 0;
} /* drsm_ */

/* Subroutine */ int drct_(integer *at, integer *nbatcn, integer *ct, real *x,
	 real *y, integer *prat, logical *erfl)
{
    /* Initialized data */

    static real bdln = 1.f;

    /* System generated locals */
    integer i__1, i__2;

    /* Builtin functions */
    double cos(doublereal), sin(doublereal);

    /* Local variables */
    static integer nbbd, nbcn;
    extern integer bmat_(integer *, integer *);
    static integer nbat, cybm[255];
    extern /* Subroutine */ int flip_(real *, real *, real *, integer *, 
	    integer *, integer *);
    static integer nbpo, nbsm, cnls[6], smfl[40], atrk[255], atls[32];
    extern /* Subroutine */ int drsm_(integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *, 
	    integer *, real *, real *, real *, integer *);
    static integer smct[1530]	/* was [255][6] */;
    extern doublereal smen_(real *, real *, integer *, integer *, real *);
    static integer cysm[100], smrk[40], rkls[32], mnpt, mxpt;
    extern /* Subroutine */ int turn_(real *, real *, real *, integer *, 
	    integer *, integer *, real *);
    static real a[255];
    static integer d[5], i, j, k, l, m, n, t[5];
    static real range, theta, a1, a2, d1, d2;
    static integer sumat;
    static real x1, x2, y1, y2, a60, dd, a90, a36;
    static integer bm[255];
    static real ap[5], ta[255], dt;
    static integer cy[2000];
    static real dx, dy;
    static integer sm[255];
    static real atfact[255], xp[5], yp[5], tx[255], ty[255];
    extern /* Subroutine */ int rankem_(integer *, integer *, integer *, 
	    integer *);
    static integer nbrdcy, smatcn[255];
    extern /* Subroutine */ int modify_(integer *, integer *, integer *, 
	    integer *, integer *, integer *, real *, real *);
    static integer cyatrk[255];
    extern /* Subroutine */ int dbsort_(integer *, integer *, integer *);
    static integer smatrk[40];
    extern /* Subroutine */ int smtran_(real *, real *, integer *, integer *, 
	    integer *, real *, real *);
    static integer rdcypt[100];
    extern /* Subroutine */ int drstup_(integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *);
    static integer iat;


/* THIS PROCEDURE ASSIGNS X AND Y COORDINATES TO THE ATOMS OF A */
/* CONNECTION TABLE.  NOTE THAT DROPEN MUST BE CALLED ONCE BEFORE USING */
/* DRCT TO GENERATE COORDINATES FOR MOLECULES. */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/*  NBATCN(I) - THE NUMBER OF ATOMS ATTACHED TO THE ITH ATOM. */
/*  CT(I,J) - THE JTH ATOM ATTACHED TO THE ITH ATOM. */
/* OUTPUT: */
/*  X(I) - THE X COORDINATE FOR THE ITH ATOM. */
/*  Y(I) - THE Y COORDINATE FOR THE ITH ATOM. */
/*  PRAT - AN ATOM IN THE PRIORITY RING SYSTEM. */
/*  ERFL - IF COORDINATES WERE NOT ASSIGNED THEN ERFL IS TRUE, */
/*  OTHERWISE IT IS FALSE. */


/* INFORMATION ON RING SYSTEM ORIENTATION IS PASSED THROUGH COMMON. */
/* DROPEN MUST BE CALLED TO INITIALIZE THESE TWO ARRAYS. */


/* THIS IS THE BOND LENGTH.  DO NOT CHANGE BDLN TO INCREASE THE SIZE */
/* OF THE MOLECULE.  I HAVE NOT CONSISTENTLY USED BDLN.  AT TIMES, */
/* 1. IS ASSUMED TO BE THE STANDARD BOND LENGTH. */

    /* Parameter adjustments */
    --y;
    --x;
    ct -= 256;
    --nbatcn;

    /* Function Body */
    *erfl = TRUE_;

/* 60 AND 90 DEGREE ANGLE CONSTANTS ARE INITIALIZED. */

    a60 = 3.14159f;
    a90 = a60 / 2.f;
    a60 /= 3.f;
    a36 = .62832f;

/* THE NUMBER OF BONDS IN THE CONNECTION TABLE IS DETERMINED (NBBD/2). */

    nbbd = 0;
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	nbbd += nbatcn[i];
/* L10: */
    }

/* IS THE MOLECULE ACYCLIC? */

    if (nbbd / 2 == *at - 1) {
	goto L120;
    }

/* THE MOLECULE IS CYCLIC. */

    sumat = 0;

/* PERCEIVE STRUCTURAL FEATURES AND INITIALIZE ARRAYS IN PREPARATION */
/* TO ASSIGN COORDINATES TO ALL RING SYSTEMS. */

    drstup_(at, &ct[256], &nbatcn[1], cy, &nbrdcy, rdcypt, &nbsm, sm, smatcn, 
	    smct, cyatrk, smrk, cysm, smfl, atrk);

/* NOW ASSIGN COORDINATES TO THE RING SYSTEMS. */

    drsm_(&nbsm, smfl, &nbrdcy, cysm, cy, rdcypt, cyatrk, at, atrk, smatcn, &
	    x[1], &y[1], a, smct);

/* RETURN IF A RING SYSTEM WAS ENCOUNTERED THAT COORDINATES COULD NOT */
/* BE GENERATED FOR. */

    i = 1;
L90:
    if (i > nbsm) {
	goto L100;
    }
    if (smfl[i - 1] > 1) {
	goto L100;
    }
    ++i;
    goto L90;
L100:
    if (i <= nbsm) {
	return 0;
    }

/* ASSIGN A RANK TO EACH CYCLIC SYSTEM (SMATRK) BASED ON THE ENTIRE */
/* MOLECULE.  NOTE, THIS RANK IS DIFFERENT FROM SMRK IN THAT SMRK IS */
/* ONLY DEPENDENT ON THE RING SYSTEM.  INITIALIZE CYBM(I), WHERE CYBM(I) 
*/
/* IS THE LENGTH OF THE SMALLEST CYCLE IN WHICH ATOM I IS PRESENT. */

    i__1 = nbsm;
    for (i = 1; i <= i__1; ++i) {
	smatrk[i - 1] = 0;
/* L20: */
    }
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	j = sm[i - 1];
	cybm[i - 1] = 1000;
	if (j != 0) {
	    smatrk[j - 1] += atrk[i - 1];
	}
/* L70: */
    }
    l = 1;
L180:
    if (l > nbrdcy) {
	goto L190;
    }
    mnpt = rdcypt[l - 1];
    k = cy[mnpt - 1];
    mxpt = mnpt + k;
    ++mnpt;
    i__1 = mxpt;
    for (m = mnpt; m <= i__1; ++m) {
	if (k < cybm[cy[m - 1] - 1]) {
	    cybm[cy[m - 1] - 1] = k;
	}
/* L200: */
    }
    ++l;
    goto L180;
L190:

/* FIND THE "PRIORITY" CYCLIC SYSTEM (M).  THE PRIORITY SYSTEM IS THE */
/* ONE WITH THE LARGEST VALUE OF SMRK (SMATRK IS USED TO BREAK TIES. */
/* THE LARGEST VALUE IS AGAIN USED.) */

    j = smrk[0];
    k = smatrk[0];
    m = 1;
    l = 2;
L80:
    if (l > nbsm) {
	goto L30;
    }
    if (smrk[l - 1] < j) {
	goto L40;
    }
    if (smrk[l - 1] > j) {
	goto L50;
    }
    if (smatrk[l - 1] > k) {
	goto L50;
    }
    goto L60;
L50:
    m = l;
    j = smrk[l - 1];
    k = smatrk[l - 1];
L60:
L40:
    ++l;
    goto L80;
L30:

/* IF AN ORIENTATION FOR THE PRIORITY CYCLIC SYSTEM IS KNOWN, THEN */
/* ORIENT IT CORRECTLY.  SMRK(I) IS USED AS AN IDENTIFIER FOR THE ITH */
/* SYSTEM.  I BELIEVE THERE IS ONLY A VERY SMALL PROBABILITY FOR TWO */
/* DIFFERENT CYCLIC SYSTEMS TO HAVE THE SAME SMRK IDENTIFIER.  AT */
/* LEAST IT IS SUFFICIENTLY UNIQUE FOR ITS USE IN THIS APLICATION. */
/* THE LEAST SIGNIFICANT BYTE OF SMRK(I)+1 (K) IS USED AS A HASH */
/* TABLE INDEX.  THE LISTS ARE CONTAINED IN SMOR HAVING THE FOLLOWING */
/* FORMAT: (1) SYSTEM IDENTIFIER, (2) FLIP ARGUMENT (1=FLIP,0=NO FLIP), */
/* (3) TURN ARGUMENT (THE ANGLE IN DEGREES TO ROTATE (OR TURN) THE */
/* SYSTEM AROUND THE PRIORITY ATOM OF THE SYSTEM), (4) A POINTER TO */
/* THE NEXT SYSTEM IDENTIFIER OR "0" IF END OF LINKED LIST.  A FILE */
/* OF 1000 MOLECULES WERE USED TO "TRAIN" THE PROGRAM.  FROM THE 67 */
/* SYSTEMS IN THIS FILE, 39 LISTS OF LENGTH 1 WERE CREATED, 11 OF */
/* LENGTH 2, AND 2 OF LENGTH 3 WERE CREATED.  SMHSTB AND SMOR ARE */
/* INITIALIZED BY CALLING DROPEN. */


    k = smrk[m - 1] % 251 + 1;
    if (smhstb[k - 1] == 0) {
	goto L130;
    }
    j = smhstb[k - 1];
L140:
    if (smor[j + 2] == 0) {
	goto L150;
    }
    if (smor[j - 1] == smrk[m - 1]) {
	goto L150;
    }
    j = smor[j + 2];
    goto L140;
L150:
    if (smor[j - 1] != smrk[m - 1]) {
	goto L160;
    }
    if (smor[j + 1] != 0) {
	iat = bmat_(sm, &m);
    }
    if (smor[j] == 1) {
	flip_(&x[1], &y[1], a, sm, &m, at);
    }
    if (smor[j + 1] == 0) {
	goto L170;
    }
    theta = (real) smor[j + 1] / 180.f * 3.14159f;
    turn_(&x[1], &y[1], a, sm, at, &iat, &theta);
L170:
L160:
L130:

/* BM(I) IS ASSIGNED THE VALUE 1 IF COORDINATES HAVE BEEN PREVIOUSLY */
/* ASSIGNED TO ATOM I, OTHERWISE IT IS ASSIGNED 0.  THE NUMBER OF ATOMS */
/* ASSIGNED COORDINATES (SUMAT) IS UPDATED. */
/* NOTE HERE, THAT THESE ARE ABSOLUTE COORDINATES.  ALL RING SYSTEMS */
/* HAVE BEEN ASSIGNED RELATIVE COORDINATES BUT ANY SYSTEM OTHER THAN */
/* THE PRIORITY SYSTEM MAY NEED TO BE FLIPPED, TURNED AND/OR TRANSLATED */
/* BEFORE ABSOLUTE COORDINATES ARE ASSIGNED. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (sm[i - 1] == m) {
	    goto L230;
	}
	bm[i - 1] = 0;
	goto L240;
L230:
	bm[i - 1] = 1;
	++sumat;
	*prat = i;
L240:
/* L220: */
	;
    }
    goto L110;
L120:

/* THE MOLECULE IS ACYCLIC. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	sm[i - 1] = 0;
	bm[i - 1] = 0;
/* L210: */
    }

/* INITIALIZE THE IDENTIFIER FOR EACH ATOM (ATRK).  THIS IS USED AS */
/* A PRIORITY NUMBER FOR COORDINATE GENERATION.  LARGE NUMBERS */
/* INDICATE ATOMS THAT ARE MORE CENTRALLY LOCATED IN THE MOLECULE. */
/* THE ATOM WITH THE LARGEST ATRK IS ASSIGNED COORDINATE 0,0.  THE */
/* ATOM ATTACHED TO THIS ATOM WITH THE LARGEST ATRK VALUE IS ASSIGNED */
/* COORDINATE .866,.5. */

    rankem_(&ct[256], &nbatcn[1], at, atrk);
    j = 1;
    l = 2;
L250:
    if (l > *at) {
	goto L260;
    }
    if (atrk[l - 1] > atrk[j - 1]) {
	j = l;
    }
    ++l;
    goto L250;
L260:
    l = nbatcn[j];
    k = ct[j + 255];
    i = 2;
L280:
    if (i > l) {
	goto L290;
    }
    m = ct[j + i * 255];
    if (atrk[m - 1] > atrk[k - 1]) {
	k = m;
    }
    ++i;
    goto L280;
L290:
    bm[j - 1] = 1;
    bm[k - 1] = 1;
    x[j] = 0.f;
    y[j] = 0.f;
    *prat = j;
    x[k] = bdln * .866026f;
    y[k] = bdln * .5f;
    a[j - 1] = 3.66519f;
    a[k - 1] = .523598f;
    sumat = 2;
L110:

/* THE ARRAY ATFACT IS INITIALIZED, WHERE ATFACT(I)=(ATRK(I)-MINIMUM */
/* ATRK VALUE)/(MAXIMUM ATRK VALUE-MINIMUM ATRK VALUE)+.5.  IT IS A */
/* REAL NUMBER BETWEEN .5 AND 1.5 THAT IS A MEASURE OF CENTRALITY */
/* BASED ON ATRK VALUES.  IT IS USED TO DETERMINE PREFERED LOCATIONS */
/* WHERE A CHOICE IS ALLOWED. */

    i = atrk[0];
    j = atrk[0];
    i__1 = *at;
    for (k = 2; k <= i__1; ++k) {
	if (atrk[k - 1] > i) {
	    i = atrk[k - 1];
	}
	if (atrk[k - 1] < j) {
	    j = atrk[k - 1];
	}
/* L640: */
    }
    if (i == j) {
	goto L645;
    }
    range = (real) (i - j);
    i__1 = *at;
    for (k = 1; k <= i__1; ++k) {
	atfact[k - 1] = (real) (atrk[k - 1] - j) / range + .5f;
/* L610: */
    }
    goto L655;
L645:
    i__1 = *at;
    for (k = 1; k <= i__1; ++k) {
	atfact[k - 1] = 1.f;
/* L665: */
    }
L655:
L270:

/* HAVE ALL ATOMS BEEN ASSIGNED COORDINATES? */

    if (sumat == *at) {
	goto L1000;
    }

/* NO, EXTRACT A LIST OF ATOMS ALREADY ASSIGNED COORDINATES */
/* AND ATTACHED TO ATOMS NOT YET ASSIGNED COORDINATES */
/* (ATLS) AND THE CORRESPONDING ATRK VALUES (RKLS) FOR */
/* THESE ATOMS.  REORDER THE LIST, SUCH THAT ATOMS IN CYCLIC SYSTEMS */
/* ARE AT THE BOTTOM AND ATOMS WITH LARGER ATRK VALUES ARE CLOSER */
/* TO THE BOTTOM OF THE LIST.  IF ATOM I IS ATTACHED TO AN ATOM IN */
/* THIS LIST AND NOT ASSIGNED A COORDINATE, THEN BM(I) IS 3 IF THE */
/* ATOM IS IN A CYCLIC SYSTEM, OTHERWISE IT IS 2. */

    nbat = 0;
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (bm[i - 1] != 1) {
	    goto L310;
	}
	j = nbatcn[i];
	n = 0;
	i__2 = j;
	for (k = 1; k <= i__2; ++k) {
	    l = ct[i + k * 255];
	    if (bm[l - 1] != 0) {
		goto L330;
	    }
	    if (sm[l - 1] == 0) {
		goto L340;
	    }
	    bm[l - 1] = 3;
	    n = 1;
	    goto L350;
L340:
	    bm[l - 1] = 2;
	    n = 2;
L350:
L330:
/* L320: */
	    ;
	}
	if (n == 0) {
	    goto L360;
	}
	++nbat;
	atls[nbat - 1] = i;
	rkls[nbat - 1] = atrk[i - 1];
L360:
	if (n == 1) {
	    rkls[nbat - 1] += 1000000000;
	}
L310:
/* L300: */
	;
    }
    if (nbat > 1) {
	dbsort_(rkls, atls, &nbat);
    }

/* NOW LOOP THROUGH THIS LIST OF ATOMS FROM THE LAST ATOM IN THE */
/* LIST (NBAT) TO THE FIRST (1), ASSIGNING COORDINATES TO THE */
/* ADJACENT ATOMS NOT YET ASSIGNED COORDINATES. */

    i = nbat;
L370:
    if (i == 0) {
	goto L270;
    }

/* GET THE LIST OF ADJACENT ATOMS (NBCN,CNLS) AND ORDER THEM BASED */
/* ON ATRK VALUES AND WHETHER THEY ARE IN CYCLES.  ATOMS IN CYCLES */
/* GET THE HIGHEST PRIORITY FOR COORDINATE GENERATION, THEN THE */
/* PRIORITY IS BASED ON HIGH ATRK VALUES. */

    iat = atls[i - 1];
    l = nbatcn[iat];
    nbcn = 0;
    i__1 = l;
    for (k = 1; k <= i__1; ++k) {
	m = ct[iat + k * 255];
	if (bm[m - 1] < 2) {
	    goto L390;
	}
	++nbcn;
	cnls[nbcn - 1] = m;
	rkls[nbcn - 1] = atrk[m - 1];
	if (bm[m - 1] == 3) {
	    rkls[nbcn - 1] += 1000000000;
	}
L390:
/* L380: */
	;
    }
    if (nbcn > 1) {
	dbsort_(rkls, cnls, &nbcn);
    }

/* THE POSSIBLE POSITION ANGLES ARE DETERMINED. */

    if (sm[iat - 1] == 0) {
	goto L400;
    }
    if (nbcn == 1) {
	goto L410;
    }

/* POSITIONS ARE ASSIGNED FOR A CYCLIC ORIGINATING ATOM THAT NEEDS TO */
/* ATTACH TO 2 OTHER ATOMS.  THE POSITIONS ARE AT ANGLES OF A + OR - */
/* THETA.  A IS THE ANGLE FROM THE CENTER OF THE RING TO THE ATOM. */
/* THETA IS 180 DEGREES DIVIDED BY THE SIZE OF THE SMALLEST CYCLE IN */
/* WHICH THE ATOM IS CONTAINED. */

    theta = 3.14159f / (real) cybm[iat - 1];
    if ((i__1 = nbcn - 3) < 0) {
	goto L2000;
    } else if (i__1 == 0) {
	goto L2010;
    } else {
	goto L2020;
    }
L2000:
    ap[0] = a[iat - 1] - theta;
    ap[1] = a[iat - 1] + theta;
    nbpo = 2;
    goto L2030;
L2010:
    ap[0] = a[iat - 1] + a36;
    ap[1] = a[iat - 1] - a36;
    ap[2] = a[iat - 1];
    nbpo = 3;
    goto L2030;
L2020:
    ap[0] = a[iat - 1] - a36 / 2.f;
    ap[1] = a[iat - 1] + a36 / 2.f;
    ap[2] = a[iat - 1] - a36 * 1.5f;
    ap[3] = a[iat - 1] + a36 * 1.5f;
    nbpo = 4;
L2030:
    goto L420;
L410:

/* ONE POSITION IS ASSIGNED FOR A CYCLIC ORIGINATING ATOM THAT WILL */
/* HAVE ONE ATOM ATTACHED. */

    ap[0] = a[iat - 1];
    nbpo = 1;
L420:
    goto L430;
L400:
    if (nbatcn[iat] > 3) {
	goto L440;
    }

/* TWO POSITIONS ARE POSSIBLE IF THE ATOM HAS LESS THAN 4 ATOMS */
/* ATTACHED AND IS ACYCLIC. */

    nbpo = 2;
    ap[0] = a[iat - 1] - a60;
    ap[1] = a[iat - 1] + a60;
    goto L450;
L440:
    if ((i__1 = nbcn - 4) < 0) {
	goto L2040;
    } else if (i__1 == 0) {
	goto L2050;
    } else {
	goto L2060;
    }
L2040:
    nbpo = 3;
    ap[1] = a[iat - 1];
    ap[0] = a[iat - 1] + a90;
    ap[2] = a[iat - 1] - a90;
    goto L2070;
L2050:
    nbpo = 4;
    ap[0] = a[iat - 1] + a36;
    ap[1] = a[iat - 1] - a36;
    ap[2] = a[iat - 1] + a36 * 3.f;
    ap[3] = a[iat - 1] - a36 * 3.f;
    goto L2070;
L2060:
    nbpo = 5;
    ap[0] = a[iat - 1] - a60;
    ap[1] = a[iat - 1] + a60;
    ap[2] = a[iat - 1];
    ap[3] = a[iat - 1] + a60 * 2.f;
    ap[4] = a[iat - 1] - a60 * 2.f;
L2070:
L450:
L430:

/* COORDINATES ARE CALCULATED FOR EACH POSITION AND THE POSITIONS */
/* ARE RANKED FROM LOWEST TO HIGHEST ENERGY. */

    i__1 = nbpo;
    for (l = 1; l <= i__1; ++l) {
	xp[l - 1] = bdln * cos(ap[l - 1]) + x[iat];
	yp[l - 1] = bdln * sin(ap[l - 1]) + y[iat];
	dd = 0.f;
	i__2 = *at;
	for (k = 1; k <= i__2; ++k) {
	    if (bm[k - 1] != 1) {
		goto L475;
	    }
	    dx = xp[l - 1] - x[k];
	    dy = yp[l - 1] - y[k];
	    dt = dx * dx + dy * dy;
	    if (dt < 1e-4f) {
		dt = 1e-4f;
	    }
	    dd += atfact[k - 1] / dt;
L475:
/* L465: */
	    ;
	}
	d[l - 1] = (integer) (dd * 1e3f);
	t[l - 1] = l;
/* L460: */
    }
    if (nbpo > 1) {
	dbsort_(d, t, &nbpo);
    }

/* NOW ASSIGN EACH ATOM IN THE LIST OF ADJACENT ATOMS TO AN ABSOLUTE */
/* POSITION.  ATOMS WITH LARGER ATRK VALUES AND/OR IN CYCLES GET */
/* PREFERENCE FOR THE LOWEST "ENERGY" POSITION. */

    l = nbcn;
    k = 1;
L470:
    if (l == 0) {
	goto L480;
    }
    m = cnls[l - 1];
    if (bm[m - 1] == 3) {
	goto L490;
    }

/* THE ADJACENT ATOM IS ACYCLIC. */

    n = t[k - 1];
    x[m] = xp[n - 1];
    y[m] = yp[n - 1];
    a[m - 1] = ap[n - 1];
    ++sumat;
    bm[m - 1] = 1;
    goto L500;
L490:

/* THE ADJACENT ATOM IS CYCLIC. */

/* IF THE ADJACENT ATOM HAS MORE THAN ONE ACYCLIC EDGE THEN IT HAS */
/* TWO SUBSTITUENTS.  THIS CAUSES A FEW PROBLEMS. */

    if (nbatcn[m] - smatcn[m - 1] == 2) {
	goto L510;
    }
    a1 = ap[t[k - 1] - 1];
    goto L520;
L510:

/* POSITIONS MUST BE SELECTED FOR THE RING SYSTEM CONTAINING THE */
/* ADJACENT ATOM AND FOR ITS OTHER SUBSTITUENT.  THERE ARE TWO */
/* POSSIBILITIES.  THE RING IS "ROCKED" TO THE LOWEST ENERGY POSITION. */
/* THE SUBSTITUENT IS ASSIGNED THE HIGHEST ENERGY POSITION, UNLESS IT */
/* IS IN ANOTHER RING SYSTEM ITSELF. */

    theta = 3.14159f / (real) cybm[m - 1];
    theta *= 2.f;
    a1 = ap[t[k - 1] - 1] + 3.14159f;
    a2 = a1 + theta;
    a1 -= theta;
    x1 = xp[t[k - 1] - 1] + bdln * cos(a1);
    y1 = yp[t[k - 1] - 1] + bdln * sin(a1);
    x2 = xp[t[k - 1] - 1] + bdln * cos(a2);
    y2 = yp[t[k - 1] - 1] + bdln * sin(a2);
    d1 = 0.f;
    d2 = 0.f;
    i__1 = *at;
    for (n = 1; n <= i__1; ++n) {
	if (bm[n - 1] != 1) {
	    goto L630;
	}
	dx = x1 - x[n];
	dy = y1 - y[n];
	dt = dx * dx + dy * dy;
	if (dt < 1e-4f) {
	    dt = 1e-4f;
	}
	d1 += atfact[n - 1] / dt;
	dx = x2 - x[n];
	dy = y2 - y[n];
	dt = dx * dx + dy * dy;
	if (dt < 1e-4f) {
	    dt = 1e-4f;
	}
	d2 += atfact[n - 1] / dt;
L630:
/* L620: */
	;
    }
    if (d2 > d1) {
	goto L530;
    }
    a1 = a2;
    x1 = x2;
    y1 = y2;
L530:

/* THE ANGLE AND COORDINATES FOR THE SUBSTITUENT HAVE BEEN DETERMINED. */
/* NOW FIND THE SEQUENCE NUMBER OF THIS ATOM. */

    n = 1;
L540:
    j = ct[m + n * 255];
    if (bm[j - 1] == 0 && sm[m - 1] != sm[j - 1]) {
	goto L550;
    }
    ++n;
    goto L540;
L550:

/* IF THE ATOM IS IN A CYCLIC SYSTEM A COORDINATE WILL NOT BE ASSIGNED */
/* UNTIL LATER, BUT THE SUBSTITUENT ANGLE OF THE ORIGINATING ATOM WILL */
/* BE CHANGED SO THAT IT POINTS IN THE CORRECT DIRECTION, OTHERWISE */
/* GO AHEAD AND ASSIGN THE COORDINATE. */

    if (sm[j - 1] != 0) {
	goto L910;
    }
    ++sumat;
    x[j] = x1;
    y[j] = y1;
    bm[j - 1] = 1;
    a[j - 1] = a1;
    goto L920;
L910:

/* NOTE, SMATCN FOR THIS ATOM MUST BE CHANGED- TO AN INCORRECT VALUE */
/* TO TRICK THE ALGORITHM ABOVE. */
/* THE SUBSTITUENT RING SYSTEM SHOULD BE ASSIGNED CORRECTLY. */

    ++smatcn[m - 1];
    a2 = a1;
L920:
    a1 = (a1 + ap[t[k - 1] - 1] + 3.14159f) / 2.f;
    a1 += 3.14159f;
L520:

/* THE FOLLOWING SECTION PERFORMS A "DOCKING-LIKE" STEP TO MOVE */
/* THE RING SYSTEM INTO THE LOWEST ENERGY POSITION. */

    i__1 = *at;
    for (n = 1; n <= i__1; ++n) {
	if (sm[m - 1] != sm[n - 1]) {
	    goto L590;
	}
	bm[n - 1] = 1;
	++sumat;
L590:
/* L600: */
	;
    }

/* ROTATE SYSTEM */

    theta = a1 + 3.14159f - a[m - 1];
    turn_(&x[1], &y[1], a, sm, at, &m, &theta);

/* TRANSLATE SYSTEM INTO POSITION. */

    smtran_(&x[1], &y[1], at, sm, &m, &xp[t[k - 1] - 1], &yp[t[k - 1] - 1]);

/* CALCULATE THE ENERGY FOR THIS POSITION. */

    d1 = smen_(&x[1], &y[1], at, bm, atfact);

/* SAVE COORDINATES. */

    i__1 = *at;
    for (n = 1; n <= i__1; ++n) {
	tx[n - 1] = x[n];
	ty[n - 1] = y[n];
	ta[n - 1] = a[n - 1];
/* L560: */
    }

/* FLIP THE SYSTEM. */

    flip_(&x[1], &y[1], a, sm, &sm[m - 1], at);

/* ROTATE THE SYSTEM. */

    theta = a1 + 3.14159f - a[m - 1];
    turn_(&x[1], &y[1], a, sm, at, &m, &theta);

/* NOW TRANSLATE THE SYSTEM INTO POSITION. */

    smtran_(&x[1], &y[1], at, sm, &m, &xp[t[k - 1] - 1], &yp[t[k - 1] - 1]);

/* CALCULATE THE ENERGY FOR THE POSITION (D2). */

    d2 = smen_(&x[1], &y[1], at, bm, atfact);

/* ASSIGN COORDINATES FOR THE LOWEST ENERGY POSTION. */

    if (d2 <= d1) {
	goto L570;
    }
    i__1 = *at;
    for (n = 1; n <= i__1; ++n) {
	x[n] = tx[n - 1];
	y[n] = ty[n - 1];
	a[n - 1] = ta[n - 1];
/* L580: */
    }
L570:
    a[m - 1] = a2;
L500:
    --l;
    ++k;
    goto L470;
L480:
    --i;
    goto L370;
L1000:
    *erfl = FALSE_;

/* NOW THE MOLECULE IS CHECKED FOR A PAIR OF CLOSE ATOMS.  IF A PAIR */
/* IS FOUND, BONDS ARE STRETCHED, SUBSTITUENTS ON BONDS ARE BENT, */
/* AND SUBSTITUENTS ARE ROTATED AROUND BONDS TO TRY AND IMPROVE THE */
/* QUALITY OF THE STRUCTURAL DIAGRAM.  IN PRACTICE, MOST STRUCTURES */
/* DO NOT PRESENT A PROBLEM BECAUSE THE FLEEING PRINCIPLE IS USED. */

    modify_(&ct[256], &nbatcn[1], at, prat, smct, smatcn, &x[1], &y[1]);
    return 0;
} /* drct_ */

/* Subroutine */ int bend_(integer *ct, integer *nbatcn, integer *at, integer 
	*at1, integer *bm, real *x, real *y, real *tx, real *ty, real *dir)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double atan2(doublereal, doublereal), sqrt(doublereal), cos(doublereal), 
	    sin(doublereal);

    /* Local variables */
    static logical erfl;
    static real a;
    static integer i;
    static real r, ax, ay, dx, dy, phi;


/* THIS PROCEDURE ROTATES THE COORDINATES FOR DESIGNATED ATOMS ABOUT */
/* AN ATOM. */

/* INPUT: */
/*  CT(I,J) - THE JTH ATOM ATTACHED TO ATOM I. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO ATOM I. */
/*  AT - THE NUMBER OF ATOMS. */
/*  AT1 - THE ATOMS ABOUT WHICH ATOMS ARE TO BE ROTATED. */
/*  X(I),Y(I) - THE COORDINATE FOR ATOM I. */
/*  DIR - THIS ARGUMENT CHANGES THE DIRECTION AND MAGNITUDE OF THE BEND */
/*  EACH UNIT CORRESPONDS TO A 30 DEGREE ROTATION.  (-2 ROTATE THE */
/*  SPECIFIED ATOM -60 DEGREES AROUND AT1.) */
/*  BM(I) - IF BM(I) IS NOT EQUAL TO 0 THEN IT WILL BE ROTATED ABOUT */
/*  ATOM AT1. */
/* OUTPUT: */
/*  TX(I),TY(I) - NEW COORDINATE FOR ATOM I. */


/* THE BEND ANGLE IS ASSIGNED HERE. */

    /* Parameter adjustments */
    --ty;
    --tx;
    --y;
    --x;
    --bm;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    a = *dir * .523598f;
    ax = x[*at1];
    ay = y[*at1];

/* ROTATE ATOMS AROUND AT1. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (bm[i] == 0) {
	    goto L20;
	}
	dy = y[i] - ay;
	dx = x[i] - ax;
	if (dx < 1e-4f && dy < 1e-4f) {
	    goto L60;
	}
	phi = atan2(dy, dx) + a;
	goto L70;
L60:
	phi = atan2(dy + 1e-4f, dx + 1e-4f) + a;
L70:
	erfl = FALSE_;
	r = sqrt(dy * dy + dx * dx);
	tx[i] = ax + r * cos(phi);
	ty[i] = ay + r * sin(phi);
	goto L30;
L20:
	tx[i] = x[i];
	ty[i] = y[i];
L30:
/* L10: */
	;
    }
    return 0;
} /* bend_ */

/* Subroutine */ int bicycl_(integer *lk, integer *lkpt, real *bl, real *x, 
	real *y, real *a)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    extern /* Subroutine */ int fuse_(real *, real *, real *, real *, integer 
	    *, real *, real *, real *);
    static integer i, j, k, l, m, n, ii, pt;
    static real atp[90], xtp[90], ytp[90], xcp1[3], ycp1[3], xcp2[3], ycp2[3];



/* THIS PROCEDURE GENERATES COORDINATES FOR THE BICYCLIC SYSTEM (LK,LKPT) 
*/
/* WITH A BOND LENGTH BL. */
/* THE COORDINATE SYSTEM IS " PSEUDO" THREE DIMENSIONAL - SEE B.D. COX, */
/* PH.D. DISSERTATION, COMPUTER PROGRAM STR3, ARIZONA STATE UNIVERSITY, */
/* TEMPE, ARIZONA, 1973. */

/* INPUT: */
/*  LKPT(I) - A POINTER TO THE ITH LINK BETWEEN BRIDGE ATOMS (THERE ARE */
/*  3 LINKS, THEY ALL START WITH THE SAME BRIDGE ATOM, AND THEY ARE */
/*  ORDERED FROM LOWEST TO HIGHEST PRIORITY - TOP,RIGHT,LEFT) */
/*  LK(LKPT(I)) - THE LENGTH OF THE ITH LINK. */
/*  LK(LKPT(I)+1)... LK(LKPT(I)+LK(LKPT(I))) - THE ATOMS IN THE LINK, */
/*  WHERE SEQUENTIAL ATOMS ARE CONNECTED. */
/*  BL - THE STANDARD BOND LENGTH. */
/* OUTPUT: */
/*  X,Y - X AND Y COORDINATES FOR THE VERTICES IN THE BICYCLIC SYSTEM. */
/*  THE INDICES CORRESPOND TO ATOMIC SEQUENCE NUMBERS IN THE CONNECTION */
/*  TABLE. */
/*  A(I) - THE ANGLE, IN RADIANS, AT WHICH SUBSTITUENTS WOULD BE */
/*  DIRECTED FROM ATOM I. */


    /* Parameter adjustments */
    --a;
    --y;
    --x;
    --lkpt;
    --lk;

    /* Function Body */
    k = lkpt[1];
    i = lk[k + 1];
    j = lk[lk[k] + k];
    x[i] = 0.f;
    y[i] = 0.f;
    x[j] = *bl * .25882f;
    y[j] = *bl * .96593f;
    a[i] = 4.45059f;
    a[j] = 1.30833f;
    xcp1[0] = *bl * .38823f;
    ycp1[0] = *bl * 1.44854f;
    xcp2[0] = xcp1[0] - *bl * .86603f;
    ycp2[0] = ycp1[0] - *bl * .5f;
    xcp1[1] = *bl * .86603f;
    ycp1[1] = *bl * -.5f;
    xcp2[1] = x[j] + *bl * .86603f;
    ycp2[1] = y[j] - *bl * .5f;
    xcp2[2] = *bl * -.86603f;
    ycp2[2] = *bl * -.5f;
    xcp1[2] = xcp2[2] - *bl * .5f;
    ycp1[2] = ycp2[2] + *bl * .86603f;
    for (l = 1; l <= 3; ++l) {
	pt = lkpt[l];
	fuse_(&xcp1[l - 1], &ycp1[l - 1], &xcp2[l - 1], &ycp2[l - 1], &lk[pt],
		 xtp, ytp, atp);
	m = 2;
	k = pt + 2;
	n = pt + lk[pt] - 1;
	i__1 = n;
	for (ii = k; ii <= i__1; ++ii) {
	    x[lk[ii]] = xtp[m - 1];
	    y[lk[ii]] = ytp[m - 1];
	    a[lk[ii]] = atp[m - 1];
	    ++m;
/* L20: */
	}
/* L10: */
    }
    return 0;
} /* bicycl_ */

/* Subroutine */ int bicyc2_(integer *lk, integer *lkpt, real *bl, real *x, 
	real *y, real *a)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double atan2(doublereal, doublereal), sin(doublereal), cos(doublereal);

    /* Local variables */
    static real cyag[100];
    extern /* Subroutine */ int fuse_(real *, real *, real *, real *, integer 
	    *, real *, real *, real *);
    static real cyxc[100], cyyc[100];
    static integer i, j, k, l, m;
    extern /* Subroutine */ int spiro_(real *, real *, real *, real *, 
	    integer *, real *, real *, real *);
    static real bf, mx, my;
    static integer bg1, bg2;
    static real cpx1, cpx2, cpy1, cpy2;


/* THIS PROCEDURE GENERATES COORDINATES FOR THE BICYCLIC SYSTEM (LK,LKPT) 
*/
/* WITH A BOND LENGTH BL.  THE BICYCLIC SYSTEM IS "FLAT" IN COMPARISON */
/* TO THE ONE GENERATED BY SUBROUTINE BICYCL.  THE ROUTINE IS USED WHEN */
/* A RING IS FUSED TO A BRIDGEHEAD ATOM OF THE BICYCLIC SYSTEM. */
/* INPUT AND OUTPUT: */

/* INPUT: */
/*  LKPT(I) - A POINTER TO THE ITH LINK BETWEEN BRIDGE ATOMS (THERE ARE */
/*  3 LINKS, THEY ALL STAT WITH THE SAME BRIDGE ATOM, AND THEY ARE */
/*  ORDERED FROM LOWEST TO HIGHEST PRIORITY - TOP,RIGHT,LEFT) */
/*  LK(LKPT(I)) - THE LENGTH OF THE ITH LINK. */
/*  LK(LKPT(I)+1) ... LK(LKPT(I)+LK(LKPT(I))) - THE ATOMS IN THE LINK, */
/*  WHERE SEQUENTIAL ATOMS ARE CONNECTION */
/*  BL - THE STANDARD BOND LENGTH */

/* OUTPUT: */
/*  X,Y - X AND Y COORDINATES FOR THE VERTICES IN THE BICYCLIC SYSTEM */
/*  THE INDICES CORRESPOND TO THE SEQUENCE NUMBERS IN THE CONNECTION */
/*  TABLE. */
/*  A(I) - THE ANGLE, IN RADIANS, AT WHICH SUBSTITUENTS WOULD BE */
/*  DIRECTED FROM ATOM I. */


    /* Parameter adjustments */
    --a;
    --y;
    --x;
    --lkpt;
    --lk;

    /* Function Body */
    i = lk[lkpt[2]] + lk[lkpt[3]] - 2;
    spiro_(&c_b53, &c_b54, &c_b54, &c_b56, &i, cyxc, cyyc, cyag);
    j = lkpt[3];
    m = lk[j] + j;
    ++j;
    l = 1;
    i__1 = m;
    for (k = j; k <= i__1; ++k) {
	x[lk[k]] = cyxc[l - 1];
	y[lk[k]] = cyyc[l - 1];
	a[lk[k]] = cyag[l - 1];
	++l;
/* L10: */
    }
    j = lkpt[2];
    m = lk[j] + j - 1;
    j += 2;
    i__1 = m;
    for (k = j; k <= i__1; ++k) {
	x[lk[k]] = cyxc[l - 1];
	y[lk[k]] = cyyc[l - 1];
	a[lk[k]] = cyag[l - 1];
	++l;
/* L20: */
    }
    j = lkpt[1];
    l = lk[j] + j;
    bg2 = lk[j + 1];
    bg1 = lk[l];
    my = (y[bg1] + y[bg2]) / 2.f;
    mx = (x[bg1] + x[bg2]) / 2.f;
    bf = *bl;
    if (i - lk[j] == 1) {
	bf *= .8f;
    }
    if (i - lk[j] == 2) {
	bf *= .8f;
    }
    a[bg1] = atan2(y[bg1] - y[bg2], x[bg1] - x[bg2]);
    a[bg2] = a[bg1] + 3.14159f;
    if (lk[lkpt[2]] > lk[lkpt[3]]) {
	goto L50;
    }
    my += sin(a[bg1] - 1.5708f) * bf * .5f;
    mx += cos(a[bg1] - 1.5708f) * bf * .5f;
    cpx2 = bf * .5f * cos(a[bg2]) + mx;
    cpx1 = bf * .5f * cos(a[bg1]) + mx;
    cpy1 = bf * .5f * sin(a[bg1]) + my;
    cpy2 = bf * .5f * sin(a[bg2]) + my;
    fuse_(&cpx1, &cpy1, &cpx2, &cpy2, &lk[j], cyxc, cyyc, cyag);
    j += 2;
    --l;
    m = 2;
L30:
    if (j > l) {
	goto L40;
    }
    x[lk[j]] = cyxc[m - 1];
    y[lk[j]] = cyyc[m - 1];
    a[lk[j]] = cyag[m - 1];
    ++m;
    ++j;
    goto L30;
L40:
    goto L60;
L50:
    my += sin(a[bg1] + 1.5708f) * bf * .5f;
    mx += cos(a[bg1] + 1.5708f) * bf * .5f;
    cpx1 = bf * .5f * cos(a[bg2]) + mx;
    cpx2 = bf * .5f * cos(a[bg1]) + mx;
    cpy2 = bf * .5f * sin(a[bg1]) + my;
    cpy1 = bf * .5f * sin(a[bg2]) + my;
    fuse_(&cpx1, &cpy1, &cpx2, &cpy2, &lk[j], cyxc, cyyc, cyag);
    j += 2;
    --l;
    m = 2;
L80:
    if (l < j) {
	goto L90;
    }
    x[lk[l]] = cyxc[m - 1];
    y[lk[l]] = cyyc[m - 1];
    a[lk[l]] = cyag[m - 1];
    ++m;
    --l;
    goto L80;
L90:
L60:
    return 0;
} /* bicyc2_ */

/* Subroutine */ int branch_(integer *ct, integer *nbatcn, integer *at, 
	integer *at1, integer *at2, integer *bm)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, j, k, l, m, ls[255], pt1, pt2;


/* THE EDGE BETWEEN AT1 AND AT2 IS ACYCLIC.  THIS ROUTINE FINDS */
/* THE BRANCH STARTING WITH AT2 DIRECTED AWAY FROM AT1.  BM IS A */
/* BIT MAP DESIGNATING ATOMS IN THE BRANCH. */

/* INPUT: */
/*  CT,NBATCN,AT - THE CONNECTION TABLE. */
/*  AT1,AT2 - THE EDGE ENDPOINTS.  THE BRANCH STARTING WITH ATOM AT2 */
/*  IS FOUND. */
/* OUTPUT: */
/*  BM(I) - ZERO IF ATOM I IS NOT IN THE BRANCH, OTHERWISE ONE. */


/* THE BRANCH ATOM MAP IS INITIALIZED. */

    /* Parameter adjustments */
    --bm;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	bm[i] = 0;
/* L10: */
    }
    bm[*at1] = 1;
    bm[*at2] = 1;

/* ATOMS IN THE BRANCH ARE SEARCHED FOR BREADTH-FIRST USING A */
/* QUEUE.  PT1 IS THE REAR POINTER.  PT2 IS THE FRONT POINTER. */

    ls[0] = *at2;
    pt1 = 0;
    pt2 = 1;
L20:
    ++pt1;
    k = ls[pt1 - 1];
    l = nbatcn[k];
    j = 1;
L30:
    if (j > l) {
	goto L40;
    }
    m = ct[k + j * 255];
    if (bm[m] == 1) {
	goto L50;
    }
    bm[m] = 1;
    ++pt2;
    ls[pt2 - 1] = m;
L50:
    ++j;
    goto L30;
L40:

/* IF THERE ARE STILL ATOMS IN THE QUEUE THAT HAVE NOT BEEN SEARCHED */
/* THEN REPEAT THE LOOP. */

    if (pt1 != pt2) {
	goto L20;
    }
    bm[*at1] = 0;
    return 0;
} /* branch_ */

/* Subroutine */ int codecy_(integer *cy, integer *pt, integer *cyrk, integer 
	*rk)
{
    static integer i, j;


/* THIS PROCEDURE ASSIGNS A PRIORITY (CYRK) TO A CYCLE (PT,CY) */
/* THAT IS THE SUM OF COMPONENT ATOM PRIORITIES (RK). */

    /* Parameter adjustments */
    --rk;
    --cy;

    /* Function Body */
    *cyrk = 0;
    i = *pt + 1;
    j = cy[*pt] + *pt;
L10:
    if (i > j) {
	goto L20;
    }
    *cyrk += rk[cy[i]];
    ++i;
    goto L10;
L20:
    return 0;
} /* codecy_ */

logical cw_(integer *cy, integer *pt, integer *at1, integer *at2)
{
    /* System generated locals */
    logical ret_val;

    /* Local variables */
    static integer i;



/* THE CYCLE SPECIFIED BY CY AND PT CONTAINS ATOMS AT1 AND AT2. */
/* FUNCTION CW RETURNS A FALSE IF ATOM AT2 RESIDES TO THE "RIGHT" */
/* OF AT1 AND TRUE IF ATOM AT2 RESIDES TO THE "LEFT" OF AT1. */
/* LARGE INDICES OF THE ARRAY CY CORRESPOND TO THE "RIGHT" AND */
/* SMALL INDICES OF THE ARRAY CORRESPOND TO THE "LEFT".  FOR */
/* A CYCLE OF THE TYPE AT2 ... AT1 HOWEVER, AT2 RESIDES TO THE */
/* "RIGHT" OF AT1. */


    /* Parameter adjustments */
    --cy;

    /* Function Body */
    i = 1;
L10:
    if (cy[*pt + i] == *at1) {
	goto L20;
    }
    ++i;
    goto L10;
L20:
    ++i;
    if (i > cy[*pt]) {
	i = 1;
    }
    if (cy[*pt + i] == *at2) {
	goto L30;
    }

/* COUNTER CLOCKWISE ROTATION. */

    ret_val = TRUE_;
    goto L40;
L30:

/* CLOCKWISE ROTATION. */

    ret_val = FALSE_;
L40:
    return ret_val;
} /* cw_ */

/* Subroutine */ int drst_(integer *nbrg, integer *rgto, integer *rgfrom, 
	integer *rgcn1, integer *rgcn2, integer *cy, integer *cyatrk, integer 
	*atrk, real *x, real *y, real *a)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double atan2(doublereal, doublereal);

    /* Local variables */
    static real cyag[100];
    extern /* Subroutine */ int fuse_(real *, real *, real *, real *, integer 
	    *, real *, real *, real *);
    static integer cyln;
    static real cyxc[100], cyyc[100];
    static integer i, j, k;
    extern /* Subroutine */ int spiro_(real *, real *, real *, real *, 
	    integer *, real *, real *, real *);
    extern logical cw_(integer *, integer *, integer *, integer *);
    static integer pt;
    extern /* Subroutine */ int rotate_(integer *, integer *);
    static integer at1, at2;
    extern /* Subroutine */ int slrgor_(integer *, integer *, integer *, 
	    integer *, integer *), nvertm_(integer *, integer *);



/* THIS PROCEDURE GENERATES COORDINATES FOR CYCLES DESCRIBED BY */
/* A SPANNING TREE. */

/* INPUT: */
/*  NBRG - THE NUMBER OF EDGES IN THE CYCLE SPANNING TREE. */
/*  RGTO - THE "TO" LIST FOR SPANNING TREE CYCLES. */
/*  RGFROM - THE "FROM" LIST FOR SPANNING TREE CYCLES. */
/*  RGCN1,RGCN2 - COMMON VERTICES BETWEEN THE CORRESPONDING CYCLES IN */
/*  THE FROM AND TO LISTS, NOTE THAT RGCN2 IS ZERO IF ONLY ONE VERTEX */
/*  IS IN COMMON. */
/*  CY(I) - THE LENGTH OF CYCLE I. */
/*  CY(I+1)...CY(I+CY(I)) - ATOMS IN CYCLE I. */
/*  ATRK(I) - A MORGAN-LIKE RANK FOR ATOMS. */
/*  X,Y - X AND Y COORDINATES FOR THE VERTICES. */
/* OUTPUT: */
/*  X,Y - X AND Y COORDINATES FOR THE VERTICES. */
/*  A(I) - THE ANGLE, IN RADIANS, AT WHICH SUBSTITUENTS WOULD BE */
/*  DIRECTED FROM ATOM I. */



/*  LOOP THROUGH "FROM" AND "TO" LISTS FOR THE SPANNING TREE. */

    /* Parameter adjustments */
    --a;
    --y;
    --x;
    --atrk;
    --cyatrk;
    --cy;
    --rgcn2;
    --rgcn1;
    --rgfrom;
    --rgto;

    /* Function Body */
    i = 1;
L530:
    if (i > *nbrg) {
	goto L540;
    }
    pt = rgto[i];
    cyln = cy[pt];

/* AT1 AND AT2 ARE THE ATOMS WHICH REPRESENT THE EDGE (FUSED) OR */
/* AT1 ONLY FOR THE VERTEX (SPIRO) FOR THE CONNECTION BETWEEN */
/* THESE TWO CYCLES. */

    at1 = rgcn1[i];
    at2 = rgcn2[i];
    if (at2 == 0) {
	goto L550;
    }

/* ***** A FUSED SYSTEM ***** */
/* COORDINATES FOR CYCLES ARE ALWAYS SELECTED GOING "CLOCKWISE". */
/* FOR EXAMPLE, A-B-C-D-E-F IS THE "TO" CYCLE.  G-H-I-C-D IS THE */
/* FROM CYCLE.  THE TO CYCLE AFTER PROPER INVERSION AND/OR */
/* ROTATION IS D-C-B-A-F-E. */

    if (cw_(&cy[1], &rgfrom[i], &at1, &at2)) {
	goto L560;
    }
    j = at1;
    at1 = at2;
    at2 = j;
L560:
L570:
L580:
    rotate_(&cy[1], &pt);
    if (cy[pt + 1] != at1) {
	goto L580;
    }
    if (cy[pt + 2] == at2) {
	goto L590;
    }
    nvertm_(&cy[1], &pt);
    goto L570;
L590:
    fuse_(&x[at1], &y[at1], &x[at2], &y[at2], &cyln, cyxc, cyyc, cyag);
    i__1 = cyln;
    for (k = 3; k <= i__1; ++k) {
	x[cy[pt + k]] = cyxc[k - 2];
	y[cy[pt + k]] = cyyc[k - 2];
	a[cy[pt + k]] = cyag[k - 2];
/* L600: */
    }

/* CHANGE ANGLE FOR ATOMS AT1 AND AT2. */
/* THE NEW ANGLE FOR SUBSTITUENTS IS IN LINE WITH THE FUSED BOND. */

    a[at1] = atan2(y[at1] - y[at2], x[at1] - x[at2]);
    a[at2] = a[at1] + 3.14159f;
    goto L610;
L550:

/* ***** A SPIRO SYSTEM ***** */
/* ONE OF THE TWO SPIRO RING ORIENTATIONS ARE SELECTED BASED ON */
/* ATOM RANK AT PRESENT. */

    slrgor_(&cy[1], &pt, &at1, &cyatrk[1], &atrk[1]);
    spiro_(&a[at1], &x[at1], &y[at1], &c_b56, &cyln, cyxc, cyyc, cyag);
    i__1 = cyln;
    for (j = 2; j <= i__1; ++j) {
	x[cy[pt + j]] = cyxc[j - 1];
	y[cy[pt + j]] = cyyc[j - 1];
	a[cy[pt + j]] = cyag[j - 1];
/* L620: */
    }
L610:
    ++i;
    goto L530;
L540:
    return 0;
} /* drst_ */

/* Subroutine */ int drstup_(integer *at, integer *ct, integer *nbatcn, 
	integer *cy, integer *nbrdcy, integer *rdcypt, integer *nbsm, integer 
	*sm, integer *smatcn, integer *smct, integer *cyatrk, integer *smrk, 
	integer *cysm, integer *smfl, integer *atrk)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer smod[100], atsk[255];
    extern /* Subroutine */ int rdcy_(integer *, integer *, integer *, 
	    integer *, integer *, integer *);
    static integer i, j, k, l, m, n, nbsmbd[100];
    extern logical bdonct_(integer *, integer *, integer *, integer *);
    extern /* Subroutine */ int rankem_(integer *, integer *, integer *, 
	    integer *);
    static integer nbsmat[100], icn, iat1, iat2;



/* THIS PROCEDURE PERCEIVES FEATURES AND INITIALIZES VARIABLES */
/* IN PREPARATION FOR COORDINATE GENERATION. */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/*  CT(I,J) - THE JTH ATOM CONNECTED TO ATOM I, THE CONNECTION TABLE. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO ATOM I. */
/* OUTPUT: */
/*  NBRDCY - THE NUMBER OF CYCLES IN THE REDUCED CYCLE SET. */
/*  RDCYPT(I) - A POINTER TO THE LOCATION IN CY OF THE ITH REDUCED */
/*  CYCLE. */
/*  CY(I) - THE LENGTH OF CYCLE I WHERE I IS A POINTER TO THE CYCLE. */
/*  CY(I+1)...CY(I+CY(I)) - THE ATOMS IN CYCLE I, ATOMS LISTED */
/*  SEQUENTIALLY ARE CONNECTED AND THE FIRST AND LAST ATOMS ARE ALSO */
/*  CONNECTED. */
/*  NBSM - THE NUMBER OF RING SYSTEMS.  CYCLES WITH COMMON ATOMS OR */
/*  BONDS ARE IN THE SAME SYSTEM. */
/*  SM(I) - A POINTER TO THE RING SYSTEM IN WHICH ATOM I RESIDES */
/*  (ACYCLIC=0). */
/*  SMCT,SMATCN - THE SAME AS CT AND NBATCN EXCEPT THAT ACYCLIC EDGES */
/*  HAVE BEEN REMOVED. */
/*  CYATRK - A MORGAN-LIKE RANK FOR CYCLIC ATOMS. */
/*  ATRK - A MORGAN-LIKE RANK FOR ALL ATOMS. */
/*  SMRK - THE SUM OF CYATRK VALUES FOR ATOMS IN THE RING SYSTEM. */
/*  THIS NUMBER IS USED AS A RING SYSTEM HASH CODE AND AS A PRIORITY */
/*  NUMBER. */
/*  CYSM - A POINTER TO THE RING SYSTEM IN WHICH THE REDUCED CYCLE */
/*  RESIDES. */
/*  SMFL(I) - THE COMPLEXITY OF THE ITH RING SYSTEM, WHERE: */
/*  SMFL(I)=(NUMBER OF CYCLES IN SYSTEM I)-(NUMBER OF EDGES IN SYSTEM I) 
*/
/*  +(NUMBER OF ATOMS IN SYSTEM I)-(1). */



/* PERCEIVE THE "REDUCE" SET OF CYCLES IN THE MOLECULE. */

    /* Parameter adjustments */
    --atrk;
    --smfl;
    --cysm;
    --smrk;
    --cyatrk;
    smct -= 256;
    --smatcn;
    --sm;
    --rdcypt;
    --cy;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    rdcy_(at, &ct[256], &nbatcn[1], nbrdcy, &rdcypt[1], &cy[1]);

/* CREATE A CONNECTION TABLE (SMCY,SMATCN) THAT CONTAINS ONLY */
/* EDGES THAT ARE CYCLIC USING THE REDUCED CYCLE SET. */

    *nbsm = 0;
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	sm[i] = 0;
	smatcn[i] = 0;
/* L100: */
    }

/* LOOP OVER ALL REDUCED CYCLES. */

    i__1 = *nbrdcy;
    for (i = 1; i <= i__1; ++i) {
	j = rdcypt[i];
	k = cy[j] + j;
	++j;
	i__2 = k;
	for (l = j; l <= i__2; ++l) {
	    m = l - 1;
	    if (l == j) {
		m = k;
	    }
	    iat1 = cy[m];
	    iat2 = cy[l];

/* IF THIS BOND IS ON THE CT THEN SKIP IT. */

	    if (bdonct_(&smatcn[1], &smct[256], &iat1, &iat2)) {
		goto L200;
	    }
	    ++smatcn[iat1];
	    ++smatcn[iat2];
	    smct[iat1 + smatcn[iat1] * 255] = iat2;
	    smct[iat2 + smatcn[iat2] * 255] = iat1;
L200:
/* L120: */
	    ;
	}
/* L110: */
    }

/* INITIALIZE CYCLIC ATOM RANK FOR EACH ATOM. */

    rankem_(&smct[256], &smatcn[1], at, &cyatrk[1]);

/* PERCEIVE VARIOUS PROPERTIES ABOUT THE RING SYSTEMS. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (sm[i] != 0) {
	    goto L760;
	}
	if (smatcn[i] == 0) {
	    goto L760;
	}
	++(*nbsm);
	smod[*nbsm - 1] = 0;
	nbsmbd[*nbsm - 1] = smatcn[i];
	nbsmat[*nbsm - 1] = 1;
	smrk[*nbsm] = cyatrk[i];
	sm[i] = *nbsm;
	atsk[0] = i;
	n = 1;
	m = 0;
L810:
	if (n == m) {
	    goto L820;
	}
	++m;
	iat1 = atsk[m - 1];
	icn = smatcn[iat1];
	j = 1;
L830:
	if (j > icn) {
	    goto L840;
	}
	iat2 = smct[iat1 + j * 255];
	if (sm[iat2] != 0) {
	    goto L850;
	}
	nbsmbd[*nbsm - 1] += smatcn[iat2];
	++nbsmat[*nbsm - 1];
	smrk[*nbsm] += cyatrk[iat2];
	sm[iat2] = *nbsm;
	++n;
	atsk[n - 1] = iat2;
L850:
	++j;
	goto L830;
L840:
	goto L810;
L820:
L760:
/* L750: */
	;
    }
    i__1 = *nbrdcy;
    for (i = 1; i <= i__1; ++i) {
	j = rdcypt[i] + 1;
	m = sm[cy[j]];
	++smod[m - 1];
	cysm[i] = m;
/* L710: */
    }
    i__1 = *nbsm;
    for (i = 1; i <= i__1; ++i) {
	nbsmbd[i - 1] /= 2;
	smfl[i] = smod[i - 1] - nbsmbd[i - 1] + nbsmat[i - 1] - 1;
/* L740: */
    }

/* INITIALIZE RANKS FOR ALL ATOMS, ALL EDGES ARE IN THIS CONNECTION */
/* TABLE. */

    rankem_(&ct[256], &nbatcn[1], at, &atrk[1]);
    return 0;
} /* drstup_ */

doublereal energy_(real *x, real *y, integer *at)
{
    /* System generated locals */
    integer i__1, i__2;
    real ret_val;

    /* Local variables */
    static integer i, j, l;
    static real dd;
    static integer jj;
    static real dx, dy;


/* THIS FUNCTION CALCULATES THE "ENERGY" OF A MOLECULE.  THE ENERGY IS */
/* THE SUM OVER ALL PAIRS OF ATOMS OF 1 DIVIDED BY THE SQUARE OF THE */
/* DISTANCE BETWEEN THE ATOMS. */
/* INPUT: */
/*  X,Y - COORDINATES */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/* OUTPUT: */
/*  ENERGY - THE "ENERGY" OF THE MOLECULE. */

    /* Parameter adjustments */
    --y;
    --x;

    /* Function Body */
    ret_val = 0.f;
    l = *at - 1;
    i__1 = l;
    for (i = 1; i <= i__1; ++i) {
	jj = i + 1;
	i__2 = *at;
	for (j = jj; j <= i__2; ++j) {
	    dx = x[i] - x[j];
	    dy = y[i] - y[j];
	    dd = dx * dx + dy * dy;

/* THIS STATEMENT AVOIDS DIVISION BY ZERO AND EXCESSIVELY LARGE */
/* ENERGY DUE TO ATOMS WITH THE SAME COORDINATE. */

	    if (dd < 1e-4f) {
		dd = 1e-4f;
	    }
	    ret_val += 1.f / dd;
/* L20: */
	}
/* L10: */
    }
    return ret_val;
} /* energy_ */

/* Subroutine */ int findph_(integer *ct, integer *nbatcn, integer *at1, 
	integer *at2, integer *mxln, integer *mxph, integer *mxst, integer *
	pt, integer *nbph, integer *phpt, integer *ph)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer path[255], teat, atpt[255], i, i1, i2;
    static logical sw;
    static integer length;
    static logical donesw;


/*     THIS PROCEDURE PERFORMS A DEPTH-FIRST TREE SEARCH TO FIND ALL */
/* PATHS BETWEEN ATOMS AT1 AND AT2 IN THE GRAPH DEFINED BY CT AND */
/* NBATCN. */
/* INPUT: */
/*  CT(I,J) - THE JTH ATOM CONNECTED TO THE ATOM WITH THE ITH SEQUENCE */
/*  NUMBER. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO THE ITH ATOM. */
/*  AT1 - A TERMINAL ATOM IN THE PATH. */
/*  AT2 - A TERMINAL ATOM IN THE PATH. */
/*  MXLN - THE MAXIMUM LENGTH OF PATHS TO BE FOUND. */
/*  MXPH - THE MAXIMUM NUMBER OF PATHS TO BE FOUND. */
/*  MXST - THE LENGTH OF THE PH (STORAGE) ARRAY. */
/*  PT - A POINTER TO THE LOCATION IN ARRAY PH WHERE PATHS WILL */
/*  BE STORED. */
/*  NBPH - THE CURRENT NUMBER OF PATHS STORED IN PH. */
/* OUTPUT: */
/*  PT - A POINTER TO THE LAST LOCATION IN ARRAY PH WHERE PATHS */
/*  WERE STORED PLUS ONE. */
/*  NBPH - THE NUMBER OF PATHS PERCEIVED PLUS THOSE PREVIOUSLY */
/*  PRESENT. */
/*  PHPT(I) - A POINTER TO THE LOCATION IN PH OF THE ITH PATH. */
/*  PH(PHPT(I)) - THE LENGTH OF THE ITH PATH. */
/*  PH(PHPT(I)+1) ...PH(PHPT(I)+PH(PHPT(I))) - A LIST OF ATOMS IN THE */
/*  ITH PATH, WHERE ADJACENT ATOMS IN THE LIST ARE CONNECTED, AND THE */
/*  FIRST AND LAST ATOMS IN THE LIST ARE ALSO CONNECTED. */

    /* Parameter adjustments */
    --ph;
    --phpt;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    donesw = FALSE_;
    sw = TRUE_;
    length = 1;

/* THE SEARCH FOR PATHS BEGINS WITH THE ATOM ATTACHED TO THE */
/* SMALLEST NUMBER OF ATOMS. */

    if (nbatcn[*at1] > nbatcn[*at2]) {
	goto L10;
    }
    path[length - 1] = *at1;
    teat = *at2;
    goto L20;
L10:
    path[length - 1] = *at2;
    teat = *at1;
L20:
L30:

/* IF ALL PATHS HAVE BEEN FOUND (DONESW=TRUE) THEN RETURN. */

    if (donesw) {
	goto L40;
    }

/* IF SW THEN INITIALIZE ATOM POINTER (ATPT) ELSE BACKUP ONE */
/* ATOM IN THE PATH AND INCREMENT THE ATOM POINTER. */

    if (sw) {
	goto L50;
    }
    --length;
    ++atpt[length - 1];
    goto L60;
L50:
    atpt[length - 1] = 1;
L60:
    i2 = atpt[length - 1];
    i1 = path[length - 1];

/* IS THE PATH LENGTH GREATER THAN THE MAXIMUM OR HAVE ALL ATOMS AT */
/* NODE PATH(LENGTH) BEEN EXAMINED? */

    if (length == *mxln) {
	goto L70;
    }
    if (i2 > nbatcn[i1]) {
	goto L70;
    }

/* NO, EXAMINE NEXT ATOM ATTACHED TO PATH(LENGTH). */

    ++length;
    path[length - 1] = ct[i1 + i2 * 255];

/* IS THIS ATOM ALREADY IN THE PATH (IF I=LENGTH THEN NOT IN PATH)? */

    i = length - 1;
L80:
    if (i == 0) {
	goto L90;
    }
    if (path[i - 1] == path[length - 1]) {
	goto L90;
    }
    --i;
    goto L80;
L90:
    if (i != 0) {
	goto L100;
    }

/* HAS THE TERMINAL ATOM OF THE PATH BEEN FOUND? */

    if (path[length - 1] != teat) {
	goto L110;
    }

/* YES, STORE THIS PATH. */

    sw = FALSE_;
    ++(*nbph);

/* IF INSUFFICIENT STORAGE IS LEFT THEN RETURN */

    if (*nbph <= *mxph && *pt + length <= *mxst) {
	goto L120;
    }
    *pt += length;
    donesw = TRUE_;
    goto L125;
L120:
    phpt[*nbph] = *pt;
    i__1 = length;
    for (i = 1; i <= i__1; ++i) {
	ph[*pt + i] = path[i - 1];
/* L140: */
    }
    ph[*pt] = length;
    *pt = *pt + length + 1;
L125:
    goto L150;
L110:

/* NO, CONTINUE SEARCHING FOR THE TERMINAL ATOM. */

    sw = TRUE_;
L150:
    goto L160;
L100:

/* THIS ATOM ALREADY OCCURRS IN THE PATH, BACKUP ONE ATOM. */

    sw = FALSE_;
L160:
    goto L170;
L70:

/* YES, ALL ATOMS HAVE BEEN EXAMINED. IS THIS NODE THE INITIAL ATOM */
/* IN THE PATH (IF LENGTH=1 THEN YES)? */

    if (length == 1) {
	goto L180;
    }

/* NO, BACKUP ONE ATOM. */

    sw = FALSE_;
    goto L190;
L180:

/* YES, THE SUBROUTINE IS DONE, SET FLAG. */

    donesw = TRUE_;
L190:
L170:
    goto L30;
L40:
    return 0;
} /* findph_ */

/* Subroutine */ int flip_(real *x, real *y, real *a, integer *bm, integer *
	is, integer *at)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double cos(doublereal), sin(doublereal), atan2(doublereal, doublereal);

    /* Local variables */
    static integer i;
    static real sx, sy;


/* THIS PROCEDURE REFLECTS THE DESIGNATED RING SYSTEM THROUGH THE */
/* X AXIS.  THUS, X COORDINATES STAY THE SAME AND NEW Y COORDINATES */
/* HAVE THE OPPOSITE SIN OF OLD Y COORDINATES. */
/* INPUT: */
/*  X,Y - COORDINATES. */
/*  A - SUBSTITUENT ANGLES. */
/*  BM(I),BM(J) - IF EQUAL, ATOMS I AND J ARE IN THE SAME RING SYSTEM. */
/*  IS - THE NUMBER ASSIGNED TO THE RING SYSTEM TO BE "FLIPPED." */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */

/* OUTPUT: */
/*  X,Y - NEW COORDINATES. */
/*  A - NEW SUBSTITUENT ANGLES. */

    /* Parameter adjustments */
    --bm;
    --a;
    --y;
    --x;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (bm[i] != *is) {
	    goto L20;
	}
	sx = x[i] + cos(a[i]);
	sy = y[i] + sin(a[i]);
	y[i] = -(doublereal)y[i];
	sy = -(doublereal)sy;
	a[i] = atan2(sy - y[i], sx - x[i]);
L20:
/* L10: */
	;
    }
    return 0;
} /* flip_ */

/* Subroutine */ int fuse_(real *x1, real *y1, real *x2, real *y2, integer *
	nbat, real *x, real *y, real *a)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double sqrt(doublereal), sin(doublereal), atan2(doublereal, doublereal), 
	    cos(doublereal);

    /* Local variables */
    static real size;
    static integer i;
    static real lamda, t, theta, bl, pi, xc, dx, dy, yc, radius, atn;



/* THIS PROCEDURE CALCULATES COORDINATES FOR EACH VERTEX OF A */
/* REGULAR POLYGON GIVEN COORDINATES FOR TWO ADJACENT VERTICES IN */
/* THE POLYGON.  THE POLYGON IS CONSTRUCTED IN A "CLOCKWISE" */
/* DIRECTION. */

/* INPUT: */
/*  X1,Y1 - COORDINATES OF POINT 1. */
/*  X2,Y2 - COORDINATES OF POINT 2. */
/*  NBAT - THE NUMBER OF VERTICES IN THE POLYGON. */
/* OUTPUT: */
/*  X,Y - X AND Y COORDINATES FOR THE VERTICES OF A REGULAR */
/*  POLYGON STARTING WITH X2,Y2 AND ENDING WITH X1,Y1. */
/*  A - THE ANGLE FROM THE CENTER OF THE POLYGON TO THE VERTEX. */


    /* Parameter adjustments */
    --a;
    --y;
    --x;

    /* Function Body */
    size = (real) (*nbat);
    pi = 3.14159f;
    theta = pi * 2.f / size;
    dx = *x2 - *x1;
    dy = *y2 - *y1;
    bl = sqrt(dx * dx + dy * dy);
    radius = bl / (sin(theta / 2.f) * 2.f);
    atn = atan2(dy, dx);
    t = atn + (theta - pi) / 2;
    lamda = atn + (pi - theta) / 2;
    xc = *x1 + radius * cos(t);
    yc = *y1 + radius * sin(t);
    x[1] = *x2;
    y[1] = *y2;
    a[1] = lamda;
    i__1 = *nbat;
    for (i = 2; i <= i__1; ++i) {
	lamda -= theta;
	x[i] = xc + radius * cos(lamda);
	y[i] = yc + radius * sin(lamda);
	a[i] = lamda;
/* L10: */
    }
    return 0;
} /* fuse_ */

/* Subroutine */ int nvertm_(integer *cy, integer *pt)
{
    static integer i, l, m, n;



/* THIS PROCEDURE INVERTS THE CYCLE (CY,PT).  FOR EXAMPLE, THE */
/* CYCLE 1-2-3-4-5-6 WOULD BE INVERTED TO 6-5-4-3-2-1. */


    /* Parameter adjustments */
    --cy;

    /* Function Body */
    m = cy[*pt];
    n = m / 2;
    i = n + 1;
    if (n << 1 != m) {
	++i;
    }
L10:
    if (n == 0) {
	goto L20;
    }
    l = cy[*pt + i];
    cy[*pt + i] = cy[*pt + n];
    cy[*pt + n] = l;
    --n;
    ++i;
    goto L10;
L20:
    return 0;
} /* nvertm_ */

/* Subroutine */ int modify_(integer *ct, integer *nbatcn, integer *at, 
	integer *prat, integer *smct, integer *smatcn, real *x, real *y)
{
    /* System generated locals */
    integer i__1;
    real r__1;

    /* Local variables */
    static integer nbbd;
    extern /* Subroutine */ int bend_(integer *, integer *, integer *, 
	    integer *, integer *, real *, real *, real *, real *, real *), 
	    cmbm_(integer *, integer *);
    static integer nbph, inat[255], teat[255];
    static real dist, sten;
    static integer phpt[1], mnpt, mxpt, i, j, k, l, m, n, limit;
    extern /* Subroutine */ int rotat_(integer *, integer *, integer *, 
	    integer *, integer *, real *, real *, integer *, real *, real *), 
	    stret_(integer *, integer *, integer *, integer *, integer *, 
	    real *, real *, integer *, real *, real *);
    static real be, dd;
    static integer bm[255];
    static logical fl;
    static integer jj, ph[255];
    static real te, dx, dy;
    extern /* Subroutine */ int branch_(integer *, integer *, integer *, 
	    integer *, integer *, integer *), findph_(integer *, integer *, 
	    integer *, integer *, integer *, integer *, integer *, integer *, 
	    integer *, integer *, integer *);
    static real sx[255], tx[255], ty[255], sy[255], factor;
    extern doublereal energy_(real *, real *, integer *);
    static integer at1, at2;
    static real txx[255], tyy[255];
    static integer iat1, iat2;


/* THIS PROCEDURE CHECKS FOR ATOMS WITHIN THE PROXIMITY OF ANOTHER, */
/* AND IF FOUND, IT WILL STRETCH, BEND AND ROTATE SUBSTITUENTS TO */
/* IMPROVE THE APPEARANCE OF THE STRUCTURAL DIAGRAM. */
/* INPUT: */
/*  CT,NBATCN,AT - THE MOLECULE'S CONNECTION TABLE, IN STANDARD FORMAT. */
/*  SMCT,SMATCN - THE CONNECTION TABLE FOR THE MOLECULE'S RING SYSTEMS. */
/*  X,Y - INPUT COORDINATES. */
/*  PRAT - AN ATOM IN THE PRIORITY RING SYSTEM. */
/* OUTPUT: */
/*  X,Y - IF THE MOLECULE WAS MODIFIED THESE ARE THE NEW X,Y */
/*  COORDINATES. */

    /* Parameter adjustments */
    --y;
    --x;
    --smatcn;
    smct -= 256;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    fl = FALSE_;
L10:

/* ASSIGNED ATOM COORDINATES ARE CHECKED FOR ANY TWO ATOMS SEPARATED */
/* BY A "SHORT" DISTANCE (.6 BOND UNITS). */

    nbbd = 0;
    limit = *at - 1;
    i = 1;
L20:
    if (i > limit) {
	goto L30;
    }
    j = i + 1;
L40:
    if (j > *at) {
	goto L50;
    }
    dx = x[i] - x[j];
    dy = y[i] - y[j];
    dd = dx * dx + dy * dy;

/* ARE THE ATOMS TOO CLOSE? */

    if (dd > .36f) {
	goto L60;
    }

/* YES, THE ATOMS ARE TO CLOSE.  A LIST OF ACYCLIC BONDS IS CREATED */
/* WHICH LIE IN A PATH BETWEEN K AND M. */

    at1 = i;
    at2 = j;
    dist = dd;
    k = i;
    m = j;
    n = 1;

/* FIND A PATH BETWEEN THESE ATOMS. */

    nbph = 0;
    findph_(&ct[256], &nbatcn[1], &k, &m, &c__255, &c__1, &c__255, &n, &nbph, 
	    phpt, ph);
    mnpt = phpt[0];
    mxpt = mnpt + ph[mnpt - 1] - 1;
    ++mnpt;
    i__1 = mxpt;
    for (jj = mnpt; jj <= i__1; ++jj) {
	iat1 = ph[jj - 1];
	iat2 = ph[jj];
	l = smatcn[iat1];

/* IF CYCLIC, SKIP THIS BOND. */

	k = 1;
L110:
	if (k > l) {
	    goto L120;
	}
	if (smct[iat1 + k * 255] == iat2) {
	    goto L120;
	}
	++k;
	goto L110;
L120:
	if (k <= l) {
	    goto L130;
	}

/* IF ALREADY IN LIST DO NOT ENTER AGAIN. */

	k = 1;
L125:
	if (k > nbbd) {
	    goto L135;
	}
	if (inat[k - 1] == iat1 && teat[k - 1] == iat2) {
	    goto L135;
	}
	if (inat[k - 1] == iat2 && teat[k - 1] == iat1) {
	    goto L135;
	}
	++k;
	goto L125;
L135:
	if (k <= nbbd) {
	    goto L145;
	}

/* ENTER BOND IN THE LIST. */

	++nbbd;
	inat[nbbd - 1] = iat1;
	teat[nbbd - 1] = iat2;
L145:
L130:
/* L80: */
	;
    }
L60:
    ++j;
    goto L40;
L50:
    ++i;
    goto L20;
L30:

/* NBBD IS THE NUMBER OF ACYCLIC BONDS IN PATHS BETWEEN ATOMS */
/* THAT ARE CLOSE TOGETHER.  IF THERE ARE NO SUCH BONDS THEN */
/* THE COORDINATES CANNOT BE REFINED SO RETURN. */

    if (nbbd == 0) {
	return 0;
    }

/* THE INPUT COORDINATES ARE SAVED. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	tx[i - 1] = x[i];
	ty[i - 1] = y[i];
/* L500: */
    }

/* THE ENERGY OF THE DISPLAY IS CALCULATED. */

    be = energy_(&x[1], &y[1], at);

/* AN ATTEMPT IS MADE TO REDUCE THE ENERGY OF THE STRUCTURE.  FIRST, */
/* ROTATING A SUBSTITUENT AROUND AN ACYCLIC BOND IS ATTEMPTED.  IF THIS */
/* WAS JUST TRIED (FL=TRUE), BONDS ARE BENT OR STRETCHED.  THE PROCESS */
/* ALTERNATES BETWEEN ROTATING AND BENDING-STRETCHING OPERATIONS UNTIL */
/* THE ENERGY OF THE STRUCTURE DIAGRAM CANNOT BE REDUCED FURTHER. */

    if (fl) {
	goto L290;
    }
    fl = TRUE_;

/* LOOP OVER THE ACYCLIC BOND LIST. */

    i = 1;
L240:
    if (i > nbbd) {
	goto L250;
    }
    iat1 = inat[i - 1];
    iat2 = teat[i - 1];
    if (nbatcn[iat1] == 1) {
	goto L150;
    }
    if (nbatcn[iat2] == 1) {
	goto L150;
    }

/* ATOMS IN THE SUBSTITUENT TO BE ROTATED ARE ASSIGNED A 1 IN BM; */
/* ALL OTHERS ARE ASSIGNED A 0.  THE SUBSTITUENT STARTS WITH ATOM */
/* TEAT(I) AND IS DIRECTED AWAY FROM INAT(I). */

    branch_(&ct[256], &nbatcn[1], at, &iat1, &iat2, bm);

/* IF AN ATOM IN THE PRIORITY RING SYSTEM IS IN THE SUBSTITUENT */
/* TO BE ROTATED THEN THE SUBSTITUENT STARTS WITH INAT(I) AND IS */
/* DIRECTED AWAY FROM TEAT(I). */

    if (bm[*prat - 1] == 0) {
	goto L1120;
    }
    cmbm_(bm, at);
    iat1 = iat2;
    iat2 = inat[i - 1];
L1120:

/* ROTATE THE SUBSTITUENT. */

    rotat_(&ct[256], &nbatcn[1], at, &iat1, &iat2, &x[1], &y[1], bm, txx, tyy)
	    ;

/* CALCULATE THE ENERGY AND SAVE THE COORDINATES IF A LOWER ENERGY */
/* IS OBTAINED. */

    te = energy_(txx, tyy, at);
    if (te > be - .001f) {
	goto L260;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	tx[j - 1] = txx[j - 1];
	ty[j - 1] = tyy[j - 1];
/* L270: */
    }
    be = te;
L260:
L150:
    ++i;
    goto L240;
L250:

/* SAVE THE BEST DISPLAY COORDINATES. */

    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	x[j] = tx[j - 1];
	y[j] = ty[j - 1];
/* L280: */
    }
    goto L300;
L290:
    fl = TRUE_;
    sten = 1e25f;

/* LOOP OVER THE LIST OF ACYCLIC BONDS.  BEND SUBSTITUENTS AND STRETCH */
/* BONDS TO ATTEMPT TO DECREASE THE ENERGY AND INCREASE THE DISTANCE */
/* BETWEEN THE CLOSE PAIR OF ATOMS. */

    i = 1;
L340:
    if (i > nbbd) {
	goto L350;
    }

/* FIND THE ATOMS IN THE SUBSTITUENT STARTING WITH TEAT(I) AND DIRECTED */
/* AWAY FROM INAT(I). */

    iat1 = inat[i - 1];
    iat2 = teat[i - 1];
    branch_(&ct[256], &nbatcn[1], at, &iat1, &iat2, bm);

/* STRETCH THE BOND. */

    stret_(&ct[256], &nbatcn[1], at, &iat1, &iat2, &x[1], &y[1], bm, txx, tyy)
	    ;

/* THE ENERGY OF A STRETCH IS CALCULATED.  40 ENERGY UNITS ARE ADDED */
/* TO THIS QUANTITY TO FAVOR A BEND OVER A STRETCH. */

    te = energy_(txx, tyy, at) + 40;

/* THE DISTANCE BETWEEN THE CLOSE PAIR OF ATOMS IS CALCULATED. */

    dx = txx[at1 - 1] - txx[at2 - 1];
    dy = tyy[at1 - 1] - tyy[at2 - 1];
    dd = dx * dx + dy * dy;

/* IF THE DISTANCE INCREASED AND THE ENERGY DECREASED THEN SAVE */
/* THESE COORDINATES. */

    if (dd < dist + .001f) {
	goto L160;
    }
    if (te > be - .001f) {
	goto L160;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	tx[j - 1] = txx[j - 1];
	ty[j - 1] = tyy[j - 1];
/* L170: */
    }
    fl = FALSE_;
    be = te;
L160:

/* IF THE DISTANCE INCREASED AND THIS IS THE LOWEST ENERGY STRETCH */
/* THEN SAVE COORDINATES FOR THE STRETCH. */

    if (dd < dist + .001f) {
	goto L2050;
    }
    if (te > sten - .001f) {
	goto L2050;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	sx[j - 1] = txx[j - 1];
	sy[j - 1] = tyy[j - 1];
/* L2060: */
    }
    sten = te;
L2050:

/* MAKE SURE THAT THE PRIORITY RING SYSTEM DOES NOT MOVE. */

    if (bm[*prat - 1] == 1) {
	cmbm_(bm, at);
    }

/* BEND SUBSTITUENTS ON INAT(I) FIRST AND THEN THOSE ON TEAT(I). */

L490:

/* IF THIS IS A ROTATION RATHER THAN A BEND THEN SKIP SECTION. */

    if (nbatcn[iat1] == 1) {
	goto L165;
    }
    factor = .33f;

/* BEND SUBSTITUENTS ON ATOM CLOCKWISE. */

    r__1 = factor * -1.f;
    bend_(&ct[256], &nbatcn[1], at, &iat1, bm, &x[1], &y[1], txx, tyy, &r__1);

/* CALCULATE ENERGY. */

    te = energy_(txx, tyy, at);

/* CALCULATE DISTANCE BETWEEN CLOSE ATOMS. */

    dx = txx[at1 - 1] - txx[at2 - 1];
    dy = tyy[at1 - 1] - tyy[at2 - 1];
    dd = dx * dx + dy * dy;

/* IF THE DISTANCE INCREASED AND THE ENERGY DECREASED THEN SAVE */
/* THESE COORDINATES. */

    if (dd < dist + .001f) {
	goto L360;
    }
    if (te > be - .001f) {
	goto L360;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	tx[j - 1] = txx[j - 1];
	ty[j - 1] = tyy[j - 1];
/* L370: */
    }
    fl = FALSE_;
    be = te;
L360:

/* BEND SUBSTITUENTS ON ATOM COUNTER COUNTERCLOCKWISE. */

    bend_(&ct[256], &nbatcn[1], at, &iat1, bm, &x[1], &y[1], txx, tyy, &
	    factor);

/* CALCULATE THE ENERGY. */

    te = energy_(txx, tyy, at);

/* CALCULATE THE DISTANCE BETWEEN CLOSE ATOMS. */

    dx = txx[at1 - 1] - txx[at2 - 1];
    dy = tyy[at1 - 1] - tyy[at2 - 1];
    dd = dx * dx + dy * dy;

/* IF THE DISTANCE INCREASED AND THE ENERGY DECREASED THEN SAVE THESE */
/* COORDINATES. */

    if (dd < dist + .001f) {
	goto L460;
    }
    if (te > be - .001f) {
	goto L460;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	tx[j - 1] = txx[j - 1];
	ty[j - 1] = tyy[j - 1];
/* L470: */
    }
    fl = FALSE_;
    be = te;
L460:
L165:

/* HAVE BENDS BEEN ATTEMPTED FOR SUBSTITUENTS ON BOTH INAT(I) AND */
/* TEAT(I)? */

    if (iat1 != inat[i - 1]) {
	goto L480;
    }

/* NO, TRY BENDS FOR THE TERMINATING ATOM OF THE ACYCLIC BOND. */

    iat1 = teat[i - 1];
    iat2 = inat[i - 1];

/* MARK ATOMS IN SUBSTITUENTS TO BE BENT. */

    branch_(&ct[256], &nbatcn[1], at, &iat1, &iat2, bm);

/* MAKE SURE THAT THE PRIORITY RING SYSTEM DOES NOT MOVE. */

    if (bm[*prat - 1] == 1) {
	cmbm_(bm, at);
    }
    goto L490;
L480:
    ++i;
    goto L340;
L350:

/* DID A STRETCH OR BEND DECREASE THE ENERGY AND INCREASE THE DISTANCE? */

    if (! fl) {
	goto L2000;
    }

/* NO, IF A STRETCH INCREASED THE DISTANCE THEN ASSIGN COORDINATES FOR */
/* THE LOWEST ENERGY STRETCH. */

    if (sten > 1e25f) {
	return 0;
    }
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	x[j] = sx[j - 1];
	y[j] = sy[j - 1];
/* L2070: */
    }
    fl = FALSE_;
    goto L2010;
L2000:

/* YES, ASSIGN THE COORDINATES FOR THE LOWEST ENERGY DIAGRAM. */

    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	x[j] = tx[j - 1];
	y[j] = ty[j - 1];
/* L180: */
    }
L2010:
L300:

/* PROCESS THE STRUCTURAL DIAGRAM OVER AGAIN. */

    goto L10;
} /* modify_ */

/* Subroutine */ int rankem_(integer *ct, integer *nbatcn, integer *at, 
	integer *rk)
{
    /* System generated locals */
    integer i__1, i__2;

    /* Local variables */
    static integer nbat, trrk[255], i, j, k, iat;


/*  THIS SUBROUTINE ASSIGNS EACH ATOM IN THE CT A RANKING (OR IDENTIFIER) 
*/
/*  BASED ON A MORGAN LIKE ALGORITHM.  ATOMS WITH HIGH RANKS */
/*  ARE MORE CENTRALLY LOCATED IN THE MOLECULE. */

/* INPUT: */
/*  CT,NBATCN,AT - THE CONNECTION TABLE. */
/* OUTPUT: */
/*  RK(I) - THE ATOM IDENTIFIER (RANK) FOR THE ITH ATOM. */

/* *********WARNING- CHANGING THIS SUBROUTINE CAN AFFECT MANY */
/* FEATURES OF THE DISPLAY PROGRAM.  LET THE CHANGER BEWARE. */
    /* Parameter adjustments */
    --rk;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	rk[i] = nbatcn[i];
/* L10: */
    }
    k = 1;
L60:
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {

/*  THIS STATEMENT COULD BE CHANGED TO ALTER THE WEIGHT OF */
/*  PREVIOUS CLASS MEMBERSHIP IN THE RANK ASSIGNMENT PROCESS. */

	trrk[i - 1] = rk[i] * 3;
	nbat = nbatcn[i];
	i__2 = nbat;
	for (j = 1; j <= i__2; ++j) {
	    iat = ct[i + j * 255];
	    trrk[i - 1] += rk[iat];
/* L40: */
	}
/* L30: */
    }
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	rk[i] = trrk[i - 1];
/* L50: */
    }

/* THIS STATEMENT CONTROLS THE NUMBER OF ITERATIONS. EIGHT */
/* ITERATIONS (K=8) IS PROBABLY OPTIMAL. */

    if (k == 8) {
	goto L70;
    }
    ++k;
    goto L60;
L70:
    return 0;
} /* rankem_ */

/* Subroutine */ int rgstsr_(integer *cy, integer *bm, integer *nbsmcy, 
	integer *smbm, integer *smcypt, integer *cyrk, integer *rgfrom, 
	integer *rgto, integer *rgcn1, integer *rgcn2, integer *topt, integer 
	*at, integer *rtfrom, integer *bgat1, integer *bgat2)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer from, cyln, tprk[10], tpto[10], tpcn1[10], tpcn2[10], a[10]
	    , j, k, l;
    extern /* Subroutine */ int dbbbso_(integer *, integer *, integer *);
    static integer pt, frompt;



/* THIS PROCEDURE GETS A SPANNING TREE STRATEGY FOR CYCLES */
/* IN THE CYCLIC SYSTEM NOT ALREADY ASSIGNED A STRATEGY. */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/*  CY(I) - THE LENGTH OF CYCLE I, WHERE I IS A POINTER TO */
/*  THE CYCLE. */
/*  CY(I+1)... CY(I+CY(I)) - A LIST OF ATOMS IN CYCLE I, WHERE */
/*  ADJACENT ATOMS IN THE LIST ARE CONNECTED AND THE FIRST AND */
/*  LAST ATOMS IN THE LIST ARE ALSO CONNECTED. */
/*  BM(I) - A BIT MAP FOR ATOMS IN THE ROOT NODE. */
/*  NBSMCY - THE NUMBER OF CYCLES IN THIS CYCLIC SYSTEM. */
/*  SMBM - A BITMAP FOR CYCLES WHERE A STRATEGY HAS ALREADY BEEN */
/*  FOUND. */
/*  SMCYPT(I) - THE POINTER TO THE ITH CYCLE IN THIS CYCLIC SYSTEM. */
/*  CYRK(I) - THE HASH CODE (RANK) FOR THE ITH CYCLE IN THE */
/*  CYCLIC SYSTEM. */
/*  RTFROM - THE POINTER TO THE ROOT NODE (CYCLE) IN THE TREE. */
/*  BGAT1,BGAT2 - DO NOT ASSIGN A CYCLE TO THE TREE IF EITHER */
/*  OF THESE ATOMS ARE CONNECTING THE CYCLE TO THE PARENT CYCLE */
/*  AND THE CYCLE IS SPIRO TO THE PARENT.  (THESE PARAMETERS ARE */
/*  USED FOR BICYCLIC SYSTEMS.) */
/* OUTPUT: */
/*  RGFROM - THE FROM LIST FOR THE SPANNING TREE STRATEGY */
/*  RGTO - THE TO LIST FOR THE SPANNING TREE STRATEGY */
/*  RGCN1,RGCN2 - COMMON VERTICES BETWEEN THE CORRESPONDING */
/*  CYCLES IN THE FROM AND TO LISTS, NOTE RGCN2 IS ZERO IF */
/*  ONLY ONE VERTEX IS IN COMMON. */
/*  BM - NOTE THAT THIS ARRAY IS USED, HOWEVER THE OUTPUT IS */
/*  MEANINGLESS */
/*  SMBM - BITS CORRESPONDING TO CYCLES ENTERED INTO THE RGFROM */
/*  AND RGTO LISTS ARE SET. */
/*  TOPT - THE NUMBER OF NODES IN THE SPANNING TREE. */

    /* Parameter adjustments */
    --rgcn2;
    --rgcn1;
    --rgto;
    --rgfrom;
    --cyrk;
    --smcypt;
    --smbm;
    --bm;
    --cy;

    /* Function Body */
    from = *rtfrom;
    *topt = 0;
    frompt = 0;
L10:

/* FIND THE LIST OF CYCLES ATTACHED TO THIS ONE THAT DO NOT */
/* HAVE A SPANNING TREE STRATEGY ASSIGNED. */

    l = 0;
    i__1 = *nbsmcy;
    for (j = 1; j <= i__1; ++j) {
	if (smbm[j] == 1) {
	    goto L50;
	}
	++l;
	tpcn1[l - 1] = 0;
	tpcn2[l - 1] = 0;
	pt = smcypt[j];
	cyln = cy[pt];
	k = 1;
L60:
	if (k > cyln) {
	    goto L70;
	}
	if (tpcn2[l - 1] != 0) {
	    goto L70;
	}
	if (bm[cy[pt + k]] == 0) {
	    goto L80;
	}
	if (tpcn1[l - 1] == 0) {
	    goto L90;
	}
	tpcn2[l - 1] = cy[pt + k];
	goto L100;
L90:
	tpcn1[l - 1] = cy[pt + k];
L100:
L80:
	++k;
	goto L60;
L70:
	if (tpcn1[l - 1] == 0) {
	    goto L110;
	}
	if (tpcn2[l - 1] == 0 && (*bgat1 == tpcn1[l - 1] || *bgat2 == tpcn1[l 
		- 1])) {
	    goto L110;
	}
	tpto[l - 1] = pt;
	tprk[l - 1] = cyrk[j];
	smbm[j] = 1;
	a[l - 1] = j;
	goto L120;
L110:
	--l;
L120:
L50:
/* L40: */
	;
    }
    if (l == 0) {
	goto L130;
    }
    if (l == 1) {
	goto L160;
    }

/* IF A CYCLE IS SPIRO TO THE PARENT CYCLE THEN ANOTHER FUSED */
/* OFFSPRING OF THE PARENT CYCLE CANNOT HAVE A "FUSED" ATOM */
/* COMMON TO THE "SPIRO" ATOM. */

/* LOOP OVER POTENTIAL OFFSPRING CYCLES. */

    j = 1;
L250:
    if (j > l) {
	goto L170;
    }
    if (tpcn2[j - 1] != 0) {
	goto L180;
    }

/* THIS CYCLE IS SPIRO.  COMPARE TO ALL OTHER CYCLES. */

    k = 1;
L190:
    if (k > l) {
	goto L200;
    }
    if (k == j) {
	goto L210;
    }
    if (tpcn1[j - 1] == tpcn1[k - 1]) {
	goto L200;
    }
    if (tpcn1[j - 1] == tpcn2[k - 1]) {
	goto L200;
    }
L210:
    ++k;
    goto L190;
L200:
    if (k > l) {
	goto L220;
    }

/* THE FUSED CYCLE TAKES PRIORITY OVER THE SPIRO CYCLE.  REMOVE THE */
/* SPIRO CYCLE FROM THE LIST OF OFFSPRING CYCLES. */

    k = j;
    smbm[a[j - 1]] = 0;
L230:
    if (k == l) {
	goto L240;
    }
    tprk[k - 1] = tprk[k];
    tpcn1[k - 1] = tpcn1[k];
    tpcn2[k - 1] = tpcn2[k];
    a[k - 1] = a[k];
    tpto[k - 1] = tpto[k];
    ++k;
    goto L230;
L240:
    --j;
    --l;
L220:
L180:
    ++j;
    goto L250;
L170:
L160:

/* NODES (CYCLES) ARE ORDERED IN THE SPANNING TREE ON THE BASIS */
/* OF HASH CODES, THIS PROBABLY WILL NEVER MAKE A DIFFERENCE. */

    a[0] = 1;
    if (l > 1) {
	dbbbso_(&l, tprk, a);
    }
    j = l;
L150:
    if (j == 0) {
	goto L140;
    }
    ++frompt;
    rgcn1[frompt] = tpcn1[a[j - 1] - 1];
    rgcn2[frompt] = tpcn2[a[j - 1] - 1];
    rgto[frompt] = tpto[a[j - 1] - 1];
    rgfrom[frompt] = from;
    --j;
    goto L150;
L140:
L130:

/* IF THE TO AND FROM LISTS ARE THE SAME LENGTH THEN RETURN. */

    if (*topt == frompt) {
	return 0;
    }
    ++(*topt);

/* A BITMAP IS INITIALIZED FOR THE NEXT CYCLE TO BE EXAMINED */
/* FOR DESCENDENT NODES. */

    from = rgto[*topt];
    cyln = cy[from];
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	bm[j] = 0;
/* L20: */
    }
    i__1 = cyln;
    for (j = 1; j <= i__1; ++j) {
	bm[cy[from + j]] = 1;
/* L30: */
    }
    goto L10;
} /* rgstsr_ */

/* Subroutine */ int rotat_(integer *ct, integer *nbatcn, integer *at, 
	integer *at1, integer *at2, real *x, real *y, integer *bm, real *tx, 
	real *ty)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double atan2(doublereal, doublereal), cos(doublereal), sin(doublereal);

    /* Local variables */
    extern /* Subroutine */ int turn_(real *, real *, real *, integer *, 
	    integer *, integer *, real *);
    static real a[255];
    static integer i;
    static real theta, x1, y1, sx, sy;
    extern /* Subroutine */ int smtran_(real *, real *, integer *, integer *, 
	    integer *, real *, real *);


/* THIS PROCEDURE ROTATES A SUBSTITUENT.  THE SUBSTITUENT IS CONNECTED */
/* BY THE EDGE BETWEEN ATOMS AT1 AND AT2.  ATOM AT2 IS THE FIRST ATOM */
/* (OR ROOT) OF THE SUBSTITUENT.  THE SUBSTITUENT IS ROTATED AROUND */
/* THIS EDGE BY 180 DEGREES ("FLIPPED"). */

/* INPUT: */
/*  CT(I,J) - THE JTH ATOM ATTACHED TO ATOM I. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO ATOM I. */
/*  AT - THE NUMBER OF ATOMS */
/*  AT1,AT2 - THE ENDPOINTS OF THE BOND CONNECTING THE SUBSTITUENT TO */
/*  BE ROTATED. */
/*  X(I),Y(I) - THE COORDINATE FOR ATOM I. */
/*  BM(I) - A BIT MAP FOR ATOMS IN THE BRANCH TO BE ROTATED. */
/* OUTPUT: */
/*  TX(I),TY(I) - NEW COORDINATE FOR ATOM I. */


/* THE SUBSTITUENT IS ROTATED. */

    /* Parameter adjustments */
    --ty;
    --tx;
    --bm;
    --y;
    --x;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    a[*at2 - 1] = atan2(y[*at1] - y[*at2], x[*at1] - x[*at2]);
    a[*at1 - 1] = atan2(y[*at2] - y[*at1], x[*at2] - x[*at1]);
    x1 = x[*at2];
    y1 = y[*at2];
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	tx[i] = x[i];
	ty[i] = y[i];
/* ONVERSION TO MV */
	if (bm[i] == 1) {
	    goto L20;
	}
	ty[i] = y[i];
	goto L30;
L20:
	ty[i] = -(doublereal)y[i];
L30:
/* L10: */
	;
    }
/*     CALL FLIP(TX,TY,A,BM,1,AT) */
/* ONVERSION TO MV */
    sx = x[*at2] + cos(a[*at2 - 1]);
    sy = -(doublereal)y[*at2] - sin(a[*at2 - 1]);
    a[*at2 - 1] = atan2(sy - ty[*at2], sx - tx[*at2]);
    theta = a[*at1 - 1] + 3.14159f - a[*at2 - 1];
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	a[i - 1] = 0.f;
/* L50: */
    }
    turn_(&tx[1], &ty[1], a, &bm[1], at, at2, &theta);
    smtran_(&tx[1], &ty[1], at, &bm[1], at2, &x1, &y1);
    return 0;
} /* rotat_ */

/* Subroutine */ int rotate_(integer *cy, integer *pt)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, l, m;


/* THIS PROCEDURE ROTATES A CYCLE "COUNTER CLOCKWISE" (TO THE "LEFT"). */
/* FOR EXAMPLE, THE CYCLE 1-2-3-4-5-6 WOULD BE ROTATED TO 2-3-4-5-6-1. */

    /* Parameter adjustments */
    --cy;

    /* Function Body */
    l = cy[*pt + 1];
    m = cy[*pt];
    i__1 = m;
    for (i = 2; i <= i__1; ++i) {
	cy[*pt + i - 1] = cy[*pt + i];
/* L10: */
    }
    cy[*pt + m] = l;
    return 0;
} /* rotate_ */

/* Subroutine */ int spiro_(real *phi, real *x1, real *y1, real *bl, integer *
	nbat, real *x, real *y, real *a)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double sin(doublereal), cos(doublereal);

    /* Local variables */
    static real size;
    static integer i;
    static real lamda, theta, pa, pi, xc, yc, radius;



/* THIS PROCEDURE CALCULATES COORDINATES FOR EACH VERTEX OF A */
/* REGULAR POLYGON GIVEN COORDINATES FOR ONE VERTEX, THE ANGLE */
/* FROM THIS VERTEX TO THE CENTER OF THE POLYGON, THE LENGTH OF */
/* A SIDE, AND THE NUMBER OF VERTICES IN THE POLYGON. */

/* INPUT: */
/*  PHI - ANGLE FROM X1,Y1 TO CENTER */
/*  X1,Y1 - COORDINATES OF A VERTEX */
/*  BL - LENGTH OF POLYGON SIDE */
/*  NBAT - NUMBER OF VERTICES IN THE POLYGON */
/* OUTPUT: */
/*  X,Y - X AND Y COORDINATES FOR THE VERTICES OF A REGULAR */
/*  POLYGON STARTING WITH X1,Y1. */
/*  A - THE ANGLE FROM THE CENTER OF THE POLYGON TO THE VERTEX. */


    /* Parameter adjustments */
    --a;
    --y;
    --x;

    /* Function Body */
    size = (real) (*nbat);
    pi = 3.1416f;
    theta = pi * 2.f / size;
    lamda = pi + *phi;
    radius = *bl / (sin(theta / 2.f) * 2.f);
    xc = *x1 + radius * cos(*phi);
    yc = *y1 + radius * sin(*phi);
    x[1] = *x1;
    y[1] = *y1;
    pa = *phi + pi;
    a[1] = pa;
    i__1 = *nbat;
    for (i = 2; i <= i__1; ++i) {
	lamda -= theta;
	pa -= theta;
	x[i] = xc + radius * cos(lamda);
	y[i] = yc + radius * sin(lamda);
	a[i] = pa;
/* L10: */
    }
    return 0;
} /* spiro_ */

/* Subroutine */ int st_(integer *at, integer *ct, integer *nbatcn, integer *
	from, integer *nbrb, integer *inrgat, integer *tergat)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer nbat, i, j, i1, i2, pt, lookat[255];


/*     THIS PROCEDURE FINDS A SPANNING TREE (FROM) FOR THE GRAPH */
/* (AT,CT,NBATCN) AND THE RING BONDS WHICH COMPLETE THE GRAPH FOR THE */
/* CORRESPONDING SPANNING TREE. */
/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE GRAPH. */
/*  CT(I,J) - THE JTH ATOM CONNECTED TO THE ATOM WITH THE ITH SEQUENCE */
/*  NUMBER. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO THE ITH ATOM. */
/* OUTPUT: */
/*  FROM - THE SPANNING TREE DEFINED AS A FROM LIST, WHERE FROM(1) */
/*  IS THE ROOT ATOM IN THE SPANNING TREE, AND WHERE ALL OTHER FROM(I) */
/*  VALUES ARE ZERO (IF NO ATOMS ARE CONNECTED TO I) OR THE SEQUENCE */
/*  NUMBER OF AN ATOM CONNECTED TO I. */
/*  NBRB - THE NUMBER OF RING BONDS IN THE GRAPH. */
/*  INRGAT(I) - THE INITIATING ATOM OF THE ITH RING BOND. */
/*  TERGAT(I) - THE TERMINATING ATOM OF THE ITH RING BOND. */

    /* Parameter adjustments */
    --tergat;
    --inrgat;
    --from;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	from[j] = 0;
/* L70: */
    }
    *nbrb = 0;
L100:
    i = 1;
L80:
    if (i > *at) {
	goto L90;
    }
    if (from[i] == 0 && nbatcn[i] != 0) {
	goto L90;
    }
    ++i;
    goto L80;
L90:
    if (i > *at) {
	return 0;
    }

/* LOOKAT IS A QUEUE CONTAINING ATOMS IN THE TREE WHICH HAVE NOT BEEN */
/* CHECKED FOR DESCENDENT ATOMS.  NBAT IS THE NUMBER OF ATOMS IN LOOKAT */
/* AND PT IS A POINTER TO THE ATOM CURRENTLY BEING CHECKED FOR */
/* DESCENDENTS. */

    nbat = 1;
    from[1] = i;
    pt = 0;
    lookat[0] = i;
L10:
    ++pt;
    i1 = lookat[pt - 1];
    j = 1;
L20:
    if (j > nbatcn[i1]) {
	goto L30;
    }
    i2 = ct[i1 + j * 255];
    if (from[i2] != 0) {
	goto L40;
    }
    ++nbat;
    lookat[nbat - 1] = i2;
    from[i2] = i1;
    goto L50;
L40:
    if (i1 > i2) {
	goto L60;
    }
    if (i2 == from[i1]) {
	goto L60;
    }

/* STORE THE RING BOND. */

    ++(*nbrb);
    inrgat[*nbrb] = i1;
    tergat[*nbrb] = i2;
L60:
L50:
    ++j;
    goto L20;
L30:
    if (pt != nbat) {
	goto L10;
    }
    goto L100;
} /* st_ */

/* Subroutine */ int stret_(integer *ct, integer *nbatcn, integer *at, 
	integer *at1, integer *at2, real *x, real *y, integer *bm, real *tx, 
	real *ty)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double atan2(doublereal, doublereal), cos(doublereal), sin(doublereal);

    /* Local variables */
    static real a;
    static integer j;
    static real dx, dy;


/* THIS PROCEDURE STRETCHES THE BOND BETWEEN ATOMS AT1 AND AT2. */
/* INPUT: */
/*  CT,NBATCN - THE MOLECULES CONNECTION TABLE. */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/*  AT1,AT2 - THE ATOM ENDPOINTS OF THE BOND TO BE STRETCHED. */
/*  X,Y - COORDINATES. */
/*  BM(I) - A BIT MAP FOR ATOMS IN THE BRANCH TO BE STRETCHED. */
/* OUTPUT: */
/*  TX,TY - COORDINATES FOR THE STRETCHED MOLECULE. */

    /* Parameter adjustments */
    --ty;
    --tx;
    --bm;
    --y;
    --x;
    --nbatcn;
    ct -= 256;

    /* Function Body */
    a = atan2(y[*at2] - y[*at1], x[*at2] - x[*at1]);
    dx = cos(a) * 1.18f;
    dy = sin(a) * 1.18f;
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	if (bm[j] == 0) {
	    goto L20;
	}
	tx[j] = x[j] + dx;
	ty[j] = y[j] + dy;
	goto L30;
L20:
	tx[j] = x[j];
	ty[j] = y[j];
L30:
/* L10: */
	;
    }
    return 0;
} /* stret_ */

/* Subroutine */ int cnml_(integer *at, integer *nbatcn, integer *ct, integer 
	*nbfg, integer *cnmap)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, j, k, l, m, ls[255], pt1, pt2;


/* THIS PROCEDURE FINDS THE NUMBER OF FRAGMENTS IN A CONNECTION TABLE */
/* AND FLAGS ATOMS CORRESPONDING TO EACH FRAGMENT. */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO ATOM I. */
/*  CT(I,J)- THE JTH ATOM CONNECTED TO ATOM I. */
/* OUTPUT: */
/*  NBFG - THE NUMBER OF FRAGMENTS */
/*  CNMAP(I) - THE FRAGMENT NUMBER OF ATOM I. */


/* CNMAP IS ZEROED TO INDICATE THAT ATOMS HAVE NOT BEEN ASSIGNED TO A */
/* FRAGMENT.  THE NUMBER OF FRAGMENTS IS ZEROED. */

    /* Parameter adjustments */
    --cnmap;
    ct -= 256;
    --nbatcn;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	cnmap[i] = 0;
/* L10: */
    }
    *nbfg = 0;
    i = 1;
L20:

/* THE NUMBER OF FRAGMENTS IS INCREASED. */

    ++(*nbfg);

/* ATOM I IS ASSIGNED TO THE FRAGMENT AND ADDED TO A QUEUE OF ATOMS IN */
/* THIS FRAGMENT. */

    cnmap[i] = *nbfg;
    ls[0] = i;

/* PT1 IS A POINTER TO THE LAST ATOM PROCESSED AND PT2 IS A POINTER TO */
/* THE LAST ATOM IN THE QUEUE. */

    pt1 = 0;
    pt2 = 1;
L30:

/* ATOMS ADJACENT TO THIS ATOM ARE ASSIGNED TO THE FRAGMENT. */

    ++pt1;
    k = ls[pt1 - 1];
    l = nbatcn[k];
    j = 1;
L40:
    if (j > l) {
	goto L50;
    }
    m = ct[k + j * 255];
    if (cnmap[m] != 0) {
	goto L60;
    }
    cnmap[m] = *nbfg;
    ++pt2;
    ls[pt2 - 1] = m;
L60:
    ++j;
    goto L40;
L50:

/* IF NOT ALL ATOMS IN THE FRAGMENT HAVE BEEN CHECKED FOR NEIGHBORS */
/* WHICH HAVE NOT BEEN ASSIGNED TO A FRAGMENT THEN REPEAT THE PROCESS. */

    if (pt1 != pt2) {
	goto L30;
    }

/* SEARCH FOR AN ATOM NOT ASSIGNED TO A FRAGMENT.  REPEAT THE ABOVE */
/* PROCESS IF ONE IS FOUND. */

    i = 2;
L70:
    if (i > *at) {
	goto L80;
    }
    if (cnmap[i] == 0) {
	goto L80;
    }
    ++i;
    goto L70;
L80:
    if (i <= *at) {
	goto L20;
    }
    return 0;
} /* cnml_ */

/* Subroutine */ int slrgor_(integer *cy, integer *pt, integer *atom, integer 
	*cyatrk, integer *rk)
{
    static integer i, j;
    extern /* Subroutine */ int rotate_(integer *, integer *), nvertm_(
	    integer *, integer *);


/* THIS PROCEDURE IS USED TO SELECT A RING ORIENTATION WHERE THERE */
/* ARE TWO POSSIBILITIES BASED ON ATOM RANKING (RK). FOR EXAMPLE, */
/* A-B-C-D-E-F IS THE CYCLE.  ATOM IS THE PRIORITY ATOM IN THE */
/* CYCLE, THAT IS IT MUST APPEAR FIRST IN THE CYCLE ATOM LIST. */
/* THUS IF ATOM=D THEN THE CYCLE IS ROTATED TO D-E-F-A-B-C. */
/* NOW THE RANK OF ATOM E MUST BE GREATER THAN OR EQUAL TO THE */
/* RANK OF ATOM C, IF THIS IS CORRECT THE PROCESS IS COMPLETE. */
/* OTHERWISE, THE CYCLE IS INVERTED TO C-B-A-F-E-D AND THEN */
/* ROTATED SO THAT ATOM D IS IN THE FIRST POSITION, I.E., */
/* D-C-B-A-F-E. */

    /* Parameter adjustments */
    --rk;
    --cyatrk;
    --cy;

    /* Function Body */
L20:
L10:
    rotate_(&cy[1], pt);
    if (cy[*pt + 1] != *atom) {
	goto L10;
    }
    i = cy[*pt + 2];
    j = cy[*pt + cy[*pt]];
    if (cyatrk[i] == cyatrk[j]) {
	goto L30;
    }
    if (cyatrk[i] > cyatrk[j]) {
	return 0;
    }
    goto L40;
L30:
    if (rk[i] >= rk[j]) {
	return 0;
    }
L40:
    nvertm_(&cy[1], pt);
    goto L20;
} /* slrgor_ */

/* Subroutine */ int dbinso_(integer *a, integer *b, integer *n, integer *c, 
	integer *d)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i, j, ia, ib, ic, id;


/* THIS IS A STRAIGHT INSERTION SORT.  A(I) IS THE MOST SIGNIFICANT */
/* WORD AND B(I) IS THE LEAST SIGNIFICANT WORD.  THIS TWO-WORD SORT */
/* WILL ARRANGE THE VALUES IN NONDESCENDING ORDER (INCREASING). */
/* THE VALUES IN C AND D ARE REARRANGED IN THE SAME ORDER.  THERE */
/* ARE N TWO-WORD NUMBERS TO BE SORTED. */

/* ONVERSION TO MV/8000 */
/*     INTEGER A(*),B(*),C(*),D(*) */
    /* Parameter adjustments */
    --d;
    --c;
    --b;
    --a;

    /* Function Body */
    i__1 = *n;
    for (j = 2; j <= i__1; ++j) {
	i = j - 1;
	ia = a[j];
	ib = b[j];
	ic = c[j];
	id = d[j];
L30:
	if (ia > a[i]) {
	    goto L20;
	}
	if (ia < a[i]) {
	    goto L21;
	}
	if (ib >= b[i]) {
	    goto L20;
	}
L21:
	b[i + 1] = b[i];
	c[i + 1] = c[i];
	d[i + 1] = d[i];
	a[i + 1] = a[i];
	--i;
	if (i > 0) {
	    goto L30;
	}
L20:
	b[i + 1] = ib;
	c[i + 1] = ic;
	d[i + 1] = id;
	a[i + 1] = ia;
/* L10: */
    }
    return 0;
} /* dbinso_ */

/* Subroutine */ int stat_(integer *nbrdcy, integer *rdcypt, integer *cy, 
	integer *cysm, integer *smnb, integer *at, integer *cyatrk, integer *
	iat1, integer *nbcy)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer nbbd, nbfg, atcn[255];
    extern /* Subroutine */ int cnml_(integer *, integer *, integer *, 
	    integer *, integer *);
    static integer inat[255], teat[255], atcy[255], atrk, smct[1530]	/* 
	    was [255][6] */, mnpt, mxpt, i, j, k, l, m, n, bm[255], pt;
    extern logical bdonct_(integer *, integer *, integer *, integer *);
    static integer smatcn[255], map[255], iat2;


/* THE RING SYSTEM IS SEARCHED FOR AN ATOM OF CYCLIC DEGREE TWO THAT */
/* IF REMOVED FROM THE RING SYSTEM WILL REDUCE THE NUMBER OF CYCLES */
/* THE MOST.  IAT1 IS THE ATOM.  NBCY IS THE NUMBER OF CYCLES THAT WILL */
/* BE REMOVED FROM THE RING SYSTEM BY REMOVING THIS ATOM. */

/* INPUT: */
/*  NBRDCY - THE NUMBER OF CYCLES IN THE REDUCED SET. */
/*  RDCYPT(I) - A POINTER TO THE ITH CYCLE OF THE REDUCED SET IN CY. */
/*  CY(I) - THE LENGTH OF CYCLE I, WHERE I IS A POINTER TO THE CYCLE. */
/*  CY(I+1)....CY(I+CY(I)) - THE ATOMS IN THE ITH CYCLE. */
/*  CYSM(I) - A POINTER TO THE RING SYSTEM FOR THE ITH CYCLE IN THE */
/*  REDUCED SET. */
/*  SMNB - A POINTER TO THE RING SYSTEM TO BE PROCESSED. */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/*  CYATRK(I) - A CODE FOR ATOM I BASED ON THE CYCLIC TOPOLOGY */
/*  OF THE MOLECULE. */

/* OUTPUT: */
/*  IAT1 - THE STRATEGIC ATOM FOR SIMPLIFYING THE RING SYSTEM. */
/*  NBCY - THE NUMBER OF CYCLES CONTAINING THE STRATEGIC ATOM IAT1. */

    /* Parameter adjustments */
    --cyatrk;
    --cysm;
    --cy;
    --rdcypt;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	smatcn[i - 1] = 0;
	atcy[i - 1] = 0;
	bm[i - 1] = 0;
/* L190: */
    }

/* CREATE A LIST OF BONDS FOR THE RING SYSTEM.  FOR EACH BOND COUNT */
/* THE NUMBER OF CYCLES THAT CONTAIN THE BOND.  THE ENDPOINT WITH THE */
/* SMALLEST SEQUENCE NUMBER IS PUT IN INAT AND THE ENDPOINT WITH THE */
/* LARGEST SEQUENCE NUMBER IS PUT IN TEAT.  NBBD IS THE NUMBER OF BONDS */
/* IN THE LIST.  ATCY(I) IS THE CYCLE COUNT FOR THE ITH ATOM. */

    nbbd = 0;
    i__1 = *nbrdcy;
    for (i = 1; i <= i__1; ++i) {
	if (*smnb != cysm[i]) {
	    goto L10;
	}
	pt = rdcypt[i];
	mnpt = pt + 1;
	mxpt = pt + cy[pt];
	j = mnpt;
L20:
	if (j > mxpt) {
	    goto L30;
	}
	k = j + 1;
	if (k > mxpt) {
	    k = mnpt;
	}
	if (cy[k] > cy[j]) {
	    goto L50;
	}
	*iat1 = cy[k];
	iat2 = cy[j];
	goto L60;
L50:
	*iat1 = cy[j];
	iat2 = cy[k];
L60:
	++atcy[cy[k] - 1];
	k = 0;
	l = 1;
L70:
	if (l > nbbd) {
	    goto L80;
	}
	if (*iat1 != inat[l - 1]) {
	    goto L90;
	}
	if (iat2 != teat[l - 1]) {
	    goto L90;
	}
	l = nbbd;
	k = 1;
L90:
	++l;
	goto L70;
L80:
	if (k == 1) {
	    goto L100;
	}
	++nbbd;
	inat[nbbd - 1] = *iat1;
	teat[nbbd - 1] = iat2;
L100:
	++j;
	goto L20;
L30:
L10:
/* L110: */
	;
    }

/* LOOP OVER THE ATOMS AND ASSIGN THE NUMBER OF ATOMS CONNECTED TO */
/* EACH ATOM BY A CYCLIC EDGE (SMATCN). */

    i = 1;
L170:
    if (i > nbbd) {
	goto L180;
    }
    *iat1 = inat[i - 1];
    ++smatcn[*iat1 - 1];
    iat2 = teat[i - 1];
    ++smatcn[iat2 - 1];
    ++i;
    goto L170;
L180:

/* LOOP OVER THE ATOMS AND SELECT THE ONE WITH TWO CYCLIC ATOMS */
/* ATTACHED AND CONTAINED IN THE LARGEST NUMBER OF CYCLES. */
/* THE LARGEST ATOM CODE IS USED TO BREAK TIES. */
/* ALSO, WHEN THE CYCLES CONTAINING THE ATOM ARE REMOVED FROM */
/* THE RING SYSTEM, ONE CONNECTED RING SYSTEM MUST BE OBTAINED. */

L380:
    *nbcy = 0;
    *iat1 = 0;
    atrk = 2147483647;
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (bm[i - 1] == 1) {
	    goto L130;
	}
	if (smatcn[i - 1] != 2) {
	    goto L130;
	}
	if (atcy[i - 1] < *nbcy) {
	    goto L130;
	}
	if (atcy[i - 1] == *nbcy && cyatrk[i] >= atrk) {
	    goto L130;
	}
	atrk = cyatrk[i];
	*nbcy = atcy[i - 1];
	*iat1 = i;
L130:
/* L200: */
	;
    }

/* ONVERSION TO EAGLE */

    if (*iat1 == 0) {
	return 0;
    }

/* CREATE A CONNECTION TABLE FOR THE RING SYSTEM(S) THAT RESULT */
/* WHEN THE ATOM IS REMOVED. */

    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	atcn[i - 1] = 0;
/* L370: */
    }
    i = 1;
L300:
    if (i > *nbrdcy) {
	goto L310;
    }
    if (*smnb != cysm[i]) {
	goto L500;
    }
    pt = rdcypt[i];
    mnpt = pt + 1;
    mxpt = pt + cy[pt];
    j = mnpt;
L320:
    if (j > mxpt) {
	goto L330;
    }
    if (*iat1 == cy[j]) {
	goto L330;
    }
    ++j;
    goto L320;
L330:
    if (j <= mxpt) {
	goto L340;
    }
    j = mnpt;
L350:
    if (j > mxpt) {
	goto L360;
    }
    k = j + 1;
    if (k > mxpt) {
	k = mnpt;
    }
    m = cy[j];
    n = cy[k];
    if (bdonct_(atcn, smct, &m, &n)) {
	goto L400;
    }
    ++atcn[m - 1];
    ++atcn[n - 1];
    smct[m + atcn[m - 1] * 255 - 256] = n;
    smct[n + atcn[n - 1] * 255 - 256] = m;
L400:
    ++j;
    goto L350;
L360:
L340:
L500:
    ++i;
    goto L300;
L310:

/* FIND THE NUMBER OF RING SYSTEMS (NBFG). */

    cnml_(at, atcn, smct, &nbfg, map);
    bm[*iat1 - 1] = 1;

/* IF THERE IS MORE THAN ONE RING SYSTEM THEN REPEAT THE PROCESS. */

    m = 0;
    i = 1;
L410:
    if (i > *at) {
	goto L420;
    }
    if (atcn[i - 1] == 0) {
	goto L430;
    }
    if (m == 0) {
	m = map[i - 1];
    }
    if (m != map[i - 1]) {
	goto L420;
    }
L430:
    ++i;
    goto L410;
L420:
    if (i <= *at) {
	goto L380;
    }
    return 0;
} /* stat_ */

/* Subroutine */ int atincy_(integer *cy, integer *pt, integer *at, integer *
	j)
{
    static integer mnpt, mxpt;


/* THIS PROCEDURE RETURNS A POINTER TO ATOM AT IN THE DESIGNATED */
/* CYCLE IF IT IS PRESENT.  OTHERWISE A ZERO IS RETURNED. */

/* INPUT: */
/*  PT - A POINTER TO THE CYCLE IN CY. */
/*  CY(PT) - THE LENGTH OF THE CYCLE. */
/*  CY(PT+1)...CY(PT+CY(PT)) - THE ATOMS IN THE CYCLE. */
/*  AT - THE ATOM TO BE SEARCHED FOR. */

/* OUTPUT: */
/*  J - IF THE ATOM IS PRESENT IN THE CYCLE J IS A POINTER TO IT, */
/*  OTHERWISE IT IS ZERO. */
    /* Parameter adjustments */
    --cy;

    /* Function Body */
    mnpt = *pt + 1;
    mxpt = *pt + cy[*pt];
    *j = mnpt;
L10:
    if (*j > mxpt) {
	goto L20;
    }
    if (cy[*j] == *at) {
	goto L20;
    }
    ++(*j);
    goto L10;
L20:
    if (*j > mxpt) {
	*j = 0;
    }
    return 0;
} /* atincy_ */

doublereal smen_(real *x, real *y, integer *at, integer *bm, real *atfact)
{
    /* System generated locals */
    integer i__1;
    real ret_val;

    /* Local variables */
    static integer i, j, l;
    static real dd, dx, dy;


/* THIS FUNCTION CALCULATES THE ENERGY OF THE ATOMS FLAGGED */
/* IN THE ATOM MAP (BM). */

/* INPUT: */
/*  X,Y - X AND Y COORDINATES. */
/*  AT - THE NUMBER OF ATOMS IN THE CONNECTION TABLE */
/*  BM(I) - 1 IF ATOM I SHOULD BE USED IN THE ENERGY CALCULATION, */
/*  OTHERWISE IT IS 0. */
/*  ATFACT(I) - A WEIGHT FACTOR FOR ATOM I. */

/* OUTPUT: */
/*  SMEN - THE CALCULATED ENERGY. */

    /* Parameter adjustments */
    --atfact;
    --bm;
    --y;
    --x;

    /* Function Body */
    ret_val = 0.f;
    l = *at - 1;
    i__1 = l;
    for (i = 1; i <= i__1; ++i) {
	if (bm[i] == 0) {
	    goto L30;
	}
	j = i + 1;
L50:
	if (j > *at) {
	    goto L20;
	}
	if (bm[j] == 0) {
	    goto L40;
	}
	dx = x[i] - x[j];
	if (dabs(dx) > 100.f) {
	    dx = 100.f;
	}
	dy = y[i] - y[j];
	if (dabs(dy) > 100.f) {
	    dy = 100.f;
	}
	dd = dx * dx + dy * dy;
	if (dd < 1e-4f) {
	    dd = 1e-4f;
	}
	ret_val += atfact[i] * atfact[j] / dd;
L40:
	++j;
	goto L50;
L20:
L30:
/* L10: */
	;
    }
    return ret_val;
} /* smen_ */

/* Subroutine */ int turn_(real *x, real *y, real *a, integer *bm, integer *
	at, integer *atom, real *theta)
{
    /* System generated locals */
    integer i__1;

    /* Builtin functions */
    double cos(doublereal), sin(doublereal), atan2(doublereal, doublereal), 
	    sqrt(doublereal);

    /* Local variables */
    extern logical zero_(real *);
    static integer i;
    static real r, ax, ay;
    static integer is;
    static real dy, dx, sx, sy, phi;


/* THIS PROCEDURE ROTATES A RING SYSTEM AROUND THE PRIORITY ATOM. */
/* INPUT: */
/*  X,Y - COORDINATES. */
/*  A - SUBSTITUENT ANGLES. */
/*  BM(I),BM(J) - IF EQUAL, ATOMS I AND J ARE IN THE SAME RING SYSTEM. */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/*  ATOM - THE PRIORITY ATOM SEQUENCE NUMBER. */
/*  THETA - THE ANGLE TO ROTATE THE MOLECULE. */
/* OUTPUT: */
/*  X,Y - NEW COORDINATES. */
/*  A - NEW SUBSTITUENT ANGLES. */

    /* Parameter adjustments */
    --bm;
    --a;
    --y;
    --x;

    /* Function Body */
    is = bm[*atom];
    a[*atom] = *theta + a[*atom];
    ax = x[*atom];
    ay = y[*atom];
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (*atom == i) {
	    goto L20;
	}
	if (bm[i] != is) {
	    goto L20;
	}
	sx = x[i] + cos(a[i]);
	sy = y[i] + sin(a[i]);
	dy = sy - ay;
	dx = sx - ax;
	phi = 0.f;
	if (zero_(&dx) && zero_(&dy)) {
	    goto L30;
	}
	phi = atan2(dy, dx) + *theta;
L30:
	r = sqrt(dy * dy + dx * dx);
	sx = ax + r * cos(phi);
	sy = ay + r * sin(phi);
	dy = y[i] - ay;
	dx = x[i] - ax;
	phi = atan2(dy, dx) + *theta;
	r = sqrt(dy * dy + dx * dx);
	x[i] = ax + r * cos(phi);
	y[i] = ay + r * sin(phi);
	a[i] = atan2(sy - y[i], sx - x[i]);
L20:
/* L10: */
	;
    }
    return 0;
} /* turn_ */

/* Subroutine */ int trim_(integer *at, integer *ct, integer *nbatcn)
{
    static integer i, j, iflag, iat;


/*     THIS PROCEDURE TRIMS BRANCHES FROM THE CONNECTION TABLE (AT, */
/* CT,NBATCN). */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE GRAPH. */
/*  CT(I,J) - THE JTH ATOM CONNECTED TO THE ATOM WITH THE ITH SEQUENCE */
/*  NUMBER. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO THE ITH ATOM. */
/* OUTPUT: */
/*  CT(I,J) - THE SAME AS IN INPUT.  HOWEVER, CONNECTED ATOMS NOT IN */
/*  BRANCHES CORRESPOND TO THE SMALLEST VALUES OF J. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO THE ITH ATOM THAT ARE */
/*  NOT CONTAINED IN BRANCHES. */

    /* Parameter adjustments */
    --nbatcn;
    ct -= 256;

    /* Function Body */
L10:

/* IFLAG IS ONE IF TERMINAL ATOMS WERE PRUNED DURING THIS PASS THROUGH */
/* THE CONNECTION TABLE ELSE IT IS ZERO. */

    iflag = 0;
    i = 1;
L20:

/* HAVE ALL ATOMS BEEN EXAMINED? */

    if (i > *at) {
	goto L30;
    }

/* IS THIS ATOM NOT TERMINAL? */

    if (nbatcn[i] != 1) {
	goto L80;
    }

/* THIS IS A TERMINAL ATOM.  SET THE NUMBER OF ATOMS CONNECTED TO THIS */
/* ATOM TO ZERO. */

    iflag = 1;
    nbatcn[i] = 0;

/* MOVE THE TERMINAL ATOM, IN THE CT ROW OF THE ATOM TO WHICH IT IS */
/* ATTACHED, TO THE LAST POSITION, THEN DECREASE THE NUMBER OF ATOMS */
/* CONNECTED TO THIS ATOM BY ONE. */

    iat = ct[i + 255];
    j = 1;
L40:
    if (i == ct[iat + j * 255]) {
	goto L50;
    }
    ++j;
    goto L40;
L50:
    ++j;
L60:
    if (j > nbatcn[iat]) {
	goto L70;
    }
    ct[iat + (j - 1) * 255] = ct[iat + j * 255];
    ++j;
    goto L60;
L70:
    ct[iat + (j - 1) * 255] = i;
    --nbatcn[iat];
L80:
    ++i;
    goto L20;
L30:

/* IF TERMINAL ATOMS WERE PRUNED FROM THE MOLECULE THEN REPEAT THE */
/* ENTIRE PROCESS. */

    if (iflag == 1) {
	goto L10;
    }
    return 0;
} /* trim_ */

/* Subroutine */ int delbnd_(integer *nbbd, integer *inat, integer *teat, 
	integer *ct, integer *nbatcn)
{
    static integer i, j, at1, at2, icn;


/*     THIS PROCEDURE DELETES THE SPECIFIED BONDS (NBBD,INAT,TEAT) */
/* FROM THE CONNECTION TABLE (CT,NBATCN). */

/* INPUT: */
/*  NBBD - THE NUMBER OF BONDS TO BE DELETED. */
/*  INAT(I), TEAT(I) - THE PAIR OF ATOMS CORRESPONDING TO THE ITH */
/*  BOND. */
/*  CT(I,J) - THE JTH ATOM CONNECTED TO THE ATOM WITH THE ITH SEQUENCE */
/*  NUMBER. */
/*  NBATCN(I) - THE NUMBER OF ATOMS CONNECTED TO ATOM I. */
/* OUTPUT: */
/*  CT - THE SAME AS ABOVE BUT WITH THE SPECIFIED BONDS DELETED. */
/*  NBATCN - THE SAME AS ABOVE BUT WITH THE SPECIFIED BONDS DELETED. */

/* ONVERSION TO EAGLE */
/*     INTEGER NBBD,INAT(*),TEAT(*),CT(255,6),NBATCN(255),AT1,AT2,ICN */
    /* Parameter adjustments */
    --nbatcn;
    ct -= 256;
    --teat;
    --inat;

    /* Function Body */
    i = 1;
L10:
    if (i > *nbbd) {
	goto L20;
    }
    at1 = inat[i];
    at2 = teat[i];
    j = 1;
L30:
    if (ct[at1 + j * 255] == at2) {
	goto L40;
    }
    ++j;
    goto L30;
L40:
    icn = nbatcn[at1];
L50:
    ++j;
    if (j > icn) {
	goto L60;
    }
    ct[at1 + (j - 1) * 255] = ct[at1 + j * 255];
    goto L50;
L60:
    nbatcn[at1] = icn - 1;
    j = 1;
L70:
    if (ct[at2 + j * 255] == at1) {
	goto L80;
    }
    ++j;
    goto L70;
L80:
    icn = nbatcn[at2];
L90:
    ++j;
    if (j > icn) {
	goto L100;
    }
    ct[at2 + (j - 1) * 255] = ct[at2 + j * 255];
    goto L90;
L100:
    nbatcn[at2] = icn - 1;
    ++i;
    goto L10;
L20:
    return 0;
} /* delbnd_ */

logical bdonct_(integer *nbatcn, integer *ct, integer *iat1, integer *iat2)
{
    /* System generated locals */
    logical ret_val;

    /* Local variables */
    static integer i, icn;



/* THIS FUNCTION (BOND ON CONNECTION TABLE) IS TRUE IF THERE */
/* IS A BOND BETWEEN ATOMS IAT1 AND IAT2 IN THE CONNECTION */
/* TABLE (CT,NBATCN), OTHERWISE IT RETURNS A FALSE. */


    /* Parameter adjustments */
    ct -= 256;
    --nbatcn;

    /* Function Body */
    ret_val = FALSE_;
    i = 1;
    icn = nbatcn[*iat1];
L10:
    if (i > icn) {
	goto L20;
    }
    if (ct[*iat1 + i * 255] == *iat2) {
	goto L20;
    }
    ++i;
    goto L10;
L20:
    if (i <= icn) {
	ret_val = TRUE_;
    }
    return ret_val;
} /* bdonct_ */

/* Subroutine */ int cmbm_(integer *bm, integer *at)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i;


/* THE ARRAY BM CONTAINS AT VALUES OF EITHER ONE OR ZERO. */
/* THIS PROCEDURE RETURNS THE COMPLEMENT. */

    /* Parameter adjustments */
    --bm;

    /* Function Body */
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (bm[i] == 1) {
	    goto L20;
	}
	bm[i] = 1;
	goto L30;
L20:
	bm[i] = 0;
L30:
/* L10: */
	;
    }
    return 0;
} /* cmbm_ */

/* Subroutine */ int smtran_(real *x, real *y, integer *at, integer *bm, 
	integer *at1, real *x1, real *y1)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer i;
    static real xs, ys;


/* THIS SUBROUTINE TRANSLATES A RING SYSTEM. */
/* INPUT: */
/*  X,Y - COORDINATES FOR ATOMS IN THE CONNECTION TABLE. */
/*  BM(I),BM(J) - IF EQUAL, ATOMS I AND J ARE IN THE SAME RING SYSTEM. */
/*  AT1,X1,Y1 - X1 AND Y1 ARE THE TRANSLATED COORDINATES FOR ATOM AT1 */
/*  WHICH IS IN THE RING SYSTEM TO BE TRANSLATED. */
/*  AT - THE NUMBER OF ATOMS IN THE MOLECULE. */
/* OUTPUT: */
/*  X,Y - COORDINATES, TRANSLATED FOR THE RING SYSTEM ATOMS. */

/* ONVERSION TO EAGLE */
/*     INTEGER AT,BM(*),AT1 */
/*     REAL X(*),Y(*),X1,Y1 */
    /* Parameter adjustments */
    --bm;
    --y;
    --x;

    /* Function Body */
    xs = *x1 - x[*at1];
    ys = *y1 - y[*at1];
    i__1 = *at;
    for (i = 1; i <= i__1; ++i) {
	if (bm[*at1] != bm[i]) {
	    goto L20;
	}
	x[i] += xs;
	y[i] += ys;
L20:
/* L10: */
	;
    }
    return 0;
} /* smtran_ */

logical zero_(real *v)
{
    /* System generated locals */
    logical ret_val;


/* RETURNS A TRUE IF THE REAL NUMBER V IS APPROXIMATELY ZERO */

    ret_val = FALSE_;
    if (*v < 1e-4f && *v > -1e-4f) {
	ret_val = TRUE_;
    }
    return ret_val;
} /* zero_ */

/* Subroutine */ int reduce_(integer *at, integer *nbcy, integer *cypt, 
	integer *cy, integer *nbrdcy, integer *rdcypt)
{
    /* System generated locals */
    integer i__1;

    /* Local variables */
    static integer cyln, rdpt, i, j, k, rdset[255], limit1, limit2, fl, pt;
    static logical sw;
    static integer rdcyln, iat, set[255];


/*     THIS PROCEDURE PERCEIVES A REDUCE CYCLE SET.  A CYCLE IS IN */
/* THE REDUCED CYCLE SET IF IT IS NOT A ATOM SUPERSET OF ANOTHER */
/* CYCLE. */

/* INPUT: */
/*  AT - THE NUMBER OF ATOMS IN THE CONNECTION TABLE. */
/*  NBCY - THE NUMBER OF CYCLES. */
/*  CYPT(I) - A POINTER TO THE LOCATION IN CY OF THE ITH CYCLE. */
/*  CY(CYPT(I)) - THE LENGTH OF THE ITH CYCLE. */
/*  CY(CYPT(I)+1)...CY(CYPT(I)+CY(CYPT(I))) - A LIST OF ATOMS IN THE */
/*  ITH CYCLE, WHERE ADJACENT ATOMS IN THE LIST ARE CONNECTED AND THE */
/*  FIRST AND LAST ATOMS IN THE LIST ARE ALSO CONNECTED. */

/* OUTPUT: */
/*  NBRDCY - THE NUMBER OF CYCLES IN THE REDUCED SET. */
/*  RDCYPT(I) - USED FOR THE REDUCED CYCLE SET WITH ARRAY CY IN THE */
/*  SAME MANNER AS CYPT IS USED FOR THE INPUT CYCLE SET. */

/* ONVERSION TO EAGLE */
/*     INTEGER NBCY,CYPT(*),CY(*),NBRDCY,RDCYPT(*),AT,SET(255),CYLN, */
/*    * PT,RDCY,RDCYLN,IAT,RDSET(255),FL */


/* THE REDUCED SET MUST CONTAIN AT LEAST ONE CYCLE.  ASSUME THE */
/* FIRST CYCLE INPUT IS IN THE REDUCED SET. */

    /* Parameter adjustments */
    --rdcypt;
    --cy;
    --cypt;

    /* Function Body */
    *nbrdcy = 1;
    rdcypt[1] = cypt[1];
    i = 2;
L10:
    if (i > *nbcy) {
	goto L20;
    }

/* INITIALIZE A BITMAP (SET) FOR THE ITH CYCLE IN THE INPUT SET OF */
/* CYCLES, WHERE SET(J) IS ONE IF THE ATOM IS IN THE SET, OTHERWISE */
/* ZERO. */

    pt = cypt[i];
    cyln = cy[pt];
    limit1 = pt + cyln;
    limit2 = pt + 1;
    i__1 = *at;
    for (j = 1; j <= i__1; ++j) {
	set[j - 1] = 0;
/* L30: */
    }
    i__1 = limit1;
    for (j = limit2; j <= i__1; ++j) {
	iat = cy[j];
	set[iat - 1] = 1;
/* L40: */
    }
    sw = FALSE_;

/* NOW COMPARE THIS CYCLE WITH THE CYCLES IN THE REDUCED SET AND */
/* MODIFY THE REDUCED SET WHERE NECESSARY. */

    j = 1;
L170:
    if (j > *nbrdcy) {
	goto L50;
    }

/* INITIALIZE A BITMAP (RDSET) FOR THE JTH CYCLE IN THE REDUCED */
/* SET OF CYCLES, WHERE RDSET(J) IS ONE IF THE ATOM IS IN THE */
/* SET, OTHERWISE ZERO. */

    rdpt = rdcypt[j];
    rdcyln = cy[rdpt];
    if (rdcyln == cyln) {
	goto L190;
    }
    limit1 = rdpt + rdcyln;
    limit2 = rdpt + 1;
    i__1 = *at;
    for (k = 1; k <= i__1; ++k) {
	rdset[k - 1] = 0;
/* L60: */
    }
    i__1 = limit1;
    for (k = limit2; k <= i__1; ++k) {
	iat = cy[k];
	rdset[iat - 1] = 1;
/* L70: */
    }

/* THE FOLLOWING CODE PERCEIVES THE RELATIONSHIP BETWEEN THIS */
/* "REDUCED" CYCLE AND THIS "INPUT" CYCLE.  FOUR CONDITIONS ARE */
/* POSSIBLE: */
/* 1. THE REDUCED CYCLE IS A SUPERSET OF THE INPUT CYCLE(FL=1) */
/* 2. THE REDUCED CYCLE IS A SUBSET OF THE INPUT CYCLE (FL=2) */
/* 3. THE TWO SETS INTERSECT BUT NEITHER IS A SUBSET OF THE */
/* OTHER (FL=3) */
/* 4. THE TWO SETS DO NOT INTERSECT (FL=0) */
/* BASED ON THE RELATIONSHIP BETWEEN THE TWO SETS THE REDUCED */
/* CYCLE SET IS EITHER LEFT UNCHANGED OR MODIFIED APPROPRIATELY. */

    fl = 0;
    k = 1;
L80:
    if (k > *at) {
	goto L90;
    }
    if (fl == 3) {
	goto L90;
    }
    if (fl == 1) {
	goto L100;
    }
    if (rdset[k - 1] == 1 && set[k - 1] == 0) {
	++fl;
    }
L100:
    if (fl == 2) {
	goto L110;
    }
    if (rdset[k - 1] == 0 && set[k - 1] == 1) {
	fl += 2;
    }
L110:
    ++k;
    goto L80;
L90:
    if (fl == 1) {
	goto L120;
    }
    if (fl != 2) {
	goto L130;
    }
    sw = TRUE_;
    j = *nbrdcy;
L130:
    goto L140;
L120:
    k = j;
L150:
    ++k;
    if (k > *nbrdcy) {
	goto L160;
    }
    rdcypt[k - 1] = rdcypt[k];
    goto L150;
L160:
    --(*nbrdcy);
L140:
L190:
    ++j;
    goto L170;
L50:
    if (sw) {
	goto L180;
    }
    ++(*nbrdcy);
    rdcypt[*nbrdcy] = pt;
L180:
    ++i;
    goto L10;
L20:
    return 0;
} /* reduce_ */

