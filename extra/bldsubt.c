/******************************************************************************
*
*  Copyright (C) 2000, Synchem Group at SUNY-Stony Brook
*
*  Module Name:                     BLDSUBT.C
*
*    Reads FG definitions from fgdata.isam and writes bldsubt.out, which is
*    read by the subsrch function initsub().  Should be run each time
*    fgdata.isam is updated by running fg2isam.
*    
*    Documentary PL/I code is excluded from compilation through the use of
*      #ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
*      #endif
*    blocks to avoid "commenting-out" problems.
*
*  Creation Date:
*
*    29-Nov-2000
*
*  Authors:
*
*    Gerald A. Miller
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* dd-mmm-yy  Fred       xxx
*
******************************************************************************/

#include <stdio.h>
#include <string.h>

#include "synchem.h"
#include "synio.h"
#include "debug.h"
#include "rcb.h"
#include "utl.h"
#include "sling.h"
#include "xtr.h"
#include "sling2xtr.h"
#include "isam.h"
#include "funcgroups.h"
#include "funcgroup_file.h"
#define _GLOBAL_DEF_
#include "extern.h"
#undef _GLOBAL_DEF_

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
 BLDSUBT: PROC OPTIONS(MAIN);

     /* COPYRIGHT 1976 - SUNY AT STONY BROOK SYNCHEM GROUP */

     /* PROGRAMMER -- RICK BOIVIE
        DATE -- 1976 SEPTEMBER 22
        PURPOSE -- TO BUILD THE TABLE OF SUBSTITUENTS USED BY SUBSRCH
        MODIFIED 11/21/77 JETS -- TO WRITE TABLES ON EXTERNAL FILES,
             INCLUDING A NEW TABLE OF ATTRIBUTE NAMES

        MODIFIED 1/18/83 GAM -- INCREASED TABLE SIZE TO 2000 TO
        PERMIT MORE DETAILED DESCRIPTION OF THE CHEMISTRY OF A
        GIVEN COMPOUND

        MODIFIED 5/16/83 GAM -- REDUCED FG_NAME UPPER BOUND TO 999
        TO PREVENT THE CREATION OF INACCESSIBLE ATTRIBUTE NUMBERS.
        ALSO ADDED CONVERSION RESPONSE AND NEW ERROR TRAPS.

        MODIFIED 7/31/84 GAM -- ADDED INVALID_ATTRIBUTE READ-IN AS AN
        IMPROVEMENT OVER THE PREVIOUS METHOD OF HARD-CODING (VMS VERSION)

     */

     /* FGFILE & FGNAME CONTAIN VARIABLE LENGTH, BLOCKED RECORDS.
        RECSIZE IS BASED ON THE 200 CHARACTER MAXIMUM ALLOWED FOR
        SUBSTRUCTURE STRINGS (PLUS 4 CONTROL BYTES FOR VARYING LENGTH).
        BLKSIZE ALLOWS FOR 15 RECORDS/BLOCK (PLUS   BYTES FOR THE BLOCK).
     */

     DCL FGFILE FILE SEQUENTIAL ENVIRONMENT
         (MAXIMUM_RECORD_SIZE(204),BLOCK_SIZE(3064),SCALARVARYING);
     DCL FGNAME FILE SEQUENTIAL ENVIRONMENT
         (MAXIMUM_RECORD_SIZE(204),BLOCK_SIZE(3064),SCALARVARYING);
     DCL SUBDOC FILE STREAM; /* USED FOR SUBSRCH DOCUMENTATION */

     DCL 1 TRACE EXT,
             (2 OPTIONS(100),
              2 PARMS(100)) BIN FIXED(15);

     /* IF MORE THAN 2000 SUBSTRUCTURES ARE DESIRED, THE
        FOLLOWING DECLARATIONS MUST BE CHANGED; HOWEVER,
        THE REACTION LIBRARY IN ITS PRESENT FORM WILL NOT
        SUPPORT ATTRIBUTE NUMBERS >1000 IN THE PRETRANSFORM
        TESTS, AND THE POSTTRANSFORM TESTS 'EXCESS' AND 'FGEQ'
        RECOGNIZE ONLY UP TO 3 DIGITS, SO THAT 999 IS A
        PRACTICAL UPPER LIMIT FOR THE FG_NAME ARRAY. */

     DCL 1 TABLE,
              2 REF(2000) BIN FIXED(15),
              2 PRESERVABLE(2000) BIT(1),
              2 PMAP(2000) BIN FIXED(15),
              2 SUB(2000) BIN FIXED(15),
              2 REFN(2000) BIN FIXED(15),
              2 SLING_LENGTH(2000) BIN FIXED(15),
              2 SUBSTRUCTURE_LENGTH(2000) BIN FIXED(15);

     DCL SLING(2000) CHAR(67) VAR;
     DCL MAX_SLING_LENGTH BIN FIXED(15) INIT(67);
     DCL SUBSTRUCTURE_STRING(2000) CHAR(200) VAR;
     DCL MAX_SUBSTRUCTURE_LENGTH BIN FIXED(15) INIT(200);
     DCL FGINFO_STRING CHAR(225) VAR;
     DCL FG_NAME(999) CHAR(200) VAR;
     DCL INVALID_ATTRIBUTE(0:5,999) BIN FIXED(15);
     DCL DUP CHAR(300) VAR BASED(DUPPTR);
     DCL DUPPTR PTR;

     /************************************************/

     DCL TABLE_BOUND BIN FIXED(15);
     DCL HBOUND BUILTIN;
     DCL LENGTH BUILTIN;
     DCL (I,J,K,L) BIN FIXED(15);
     DCL REFERENCE BIN FIXED(15);
     DCL VARSTRING CHAR(250) VAR;
     DCL XTR PTR;
     DCL SLNGXTR ENTRY(CHAR(*) VAR) RETURNS(PTR);
     DCL XLIFO ENTRY;
     DCL ALLOCT ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR,BIN FIXED(15),CHAR(*) VAR,
        BIN FIXED(15));
     DCL ALLOCN ENTRY(CHAR(*) VAR,CHAR(*) VAR) RETURNS(BIN FIXED(15));
     DCL FREE ENTRY(CHAR(*) VAR,CHAR(*) VAR,PTR);
     DCL NUM_ENTRIES BIN FIXED(15);
     DCL NREF BIN FIXED(15);
     DCL NREF_REF(999) BIN FIXED(15);
     DCL NUM_PRESERVE BIN FIXED(15) INIT(0);
     DCL TEMP BIN FIXED(15);
     DCL SMALLEST CHAR(250) VAR;
     DCL FREEXTR ENTRY(PTR);
     DCL XXTR ENTRY;
     DCL M BIN FIXED(15);
     DCL MOD BUILTIN;
     DCL MAX_REF BIN FIXED(15) INIT(0);

#endif

static struct
  {
    U16_t ref[2000];
    Boolean_t preservable[2000];
    U16_t pmap[2000];
    U16_t sub[2000];
/*
    U16_t refn[2000];
*/
    U16_t sling_length[2000];
    U16_t substructure_length[2000];
  } table;

static char sling[2000][68];
static int max_sling_length = 67;
static char substructure_string[2000][201];
static int max_substructure_length = 200;
static char fginfo_string[226];
static char fg_name[1000][201];
static int invalid_attribute[1000][6];
static char **dup = NULL;

static int table_bound, i, j, k, l, reference;
static char varstring[251];
static Xtr_t *xtr;
static int num_entries, nref=0, nref_ref[1000], num_preserve = 0, ndups = 0;
static char *smallest;
static int m, max_ref = 0;
static U16_t *map;
static FILE *fgfile;
static Isam_Control_t *fgdata;
static FuncGrp_Record_t *fgrec;

static struct qst
{
  U16_t q;
  U16_t bond;
  U16_t atom;
} *queue;

void sort ();
void buildheap ();
void heapify (int, int);
void sort_ref (U16_t *, int);
void sbstrst (Xtr_t *, char *);
void bubble_sort (int, int);

main ()
{
  int i, j, crtref, lastref, fgnot;
  char last_sub_str[201];
  String_t string_st;
  Sling_t sling_st;

/*
     ON ENDFILE(SYSIN) GO TO START_SORT;
     ON ERROR PUT EDIT('FOLLOWING: REFERENCE=',REFERENCE,
        '; FG_NAME(NREF)=',FG_NAME(NREF))(A,F(5),A,A);

     OPEN FILE (FGFILE) OUTPUT;
     OPEN FILE (FGNAME) OUTPUT;

     PUT SKIP LIST('BLDSUBT INPUT:');
     PUT SKIP LIST('______________');
     PUT SKIP(2);
*/

IO_Init();

for (i=0; i<1000; i++) table.preservable[i] = FALSE;
fgdata = (Isam_Control_t *) malloc (ISAMCONTROLSIZE);
if (fgdata == NULL) IO_Exit_Error (R_AVL, X_SYSCALL,
        "Unable to allocate memory for Isam Control Block.");
strcpy(IO_FileName_Get (Isam_File_Get (fgdata)),
        FCB_SEQDIR_FNGP ("/fgdata.isam"));
Isam_Open(fgdata,ISAM_TYPE_FGINFO,ISAM_OPEN_READ);

for (i=1, num_entries=0; (fgrec=FuncGrp_Rec_Read(i,fgdata))!=NULL; i++)
{
        reference=FuncGrp_Head_FGNum_Get(FuncGrp_Rec_Head_Get(fgrec));
/*
	table.ref[i]=reference;
*/
	if (max_ref<reference) max_ref=reference;
	for (invalid_attribute[reference][0]=0;
	  (fgnot = FuncGrp_Head_NotGroup_Get (FuncGrp_Rec_Head_Get (fgrec), invalid_attribute[reference][0])) != 0;
	  invalid_attribute[reference][0]++)
	  invalid_attribute[reference][invalid_attribute[reference][0]+1]=fgnot;
	for (j=invalid_attribute[reference][0]+1; j < 6; j++) invalid_attribute[reference][j]=0;
        for (j=0; j<FuncGrp_Head_NumSlings_Get (FuncGrp_Rec_Head_Get (fgrec)); j++)
        {
		table.ref[++num_entries]=reference;
                strcpy (varstring, Sling_Name_Get(FuncGrp_Rec_Sling_Get(fgrec,j)));
/*
		if ((table.sling_length[i] = strlen(varstring)) > max_sling_length)
*/
		if ((table.sling_length[num_entries] = strlen(varstring)) > max_sling_length)
		{
                  printf ("BLDSUBT ERROR: SLING TOO LONG\n");
                  exit(1);
		}
/*
		strcpy(sling[i],varstring);
		string_st=String_Create((const char *)FuncGrp_Rec_Sling_Get(fgrec,j), 0);
		sling_st=String2Sling(string_st);
		String_Destroy(string_st);
                xtr=Sling2Xtr(sling_st);
		Sling_Destroy(sling_st);
*/
		strcpy(sling[num_entries],varstring);
		sling_st=FuncGrp_Rec_Sling_Get(fgrec,j);
                xtr=Sling2Xtr(sling_st);
		Sling_Destroy(sling_st);
                sbstrst(xtr, varstring);
                Xtr_Destroy (xtr);
/*
                if ((table.substructure_length[i] = strlen(varstring)) > max_substructure_length)
*/
                if ((table.substructure_length[num_entries] = strlen(varstring)) > max_substructure_length)
                {
                  printf ("BLDSUBT ERROR: SUBSTRUCTURE TOO LONG\n");
                  exit(1);
		}
/*
                strcpy (substructure_string[i],varstring);
*/
                strcpy (substructure_string[num_entries],varstring);
/*
if (strncmp(varstring,"00  1",5)==0)
{
  printf("substructure_string[%d]=\"%s\"\n",i,varstring);
  getchar ();
}
*/
/*
        }
        if (table.preservable[i] = FuncGrp_Head_FlagsPreserveable_Get (FuncGrp_Rec_Head_Get (fgrec)))
	{
		num_preserve++;
		table.pmap[num_preserve]=reference;
	}
*/
        	if (table.preservable[num_entries] = FuncGrp_Head_FlagsPreserveable_Get (FuncGrp_Rec_Head_Get (fgrec)))
		{
			num_preserve++;
			table.pmap[num_preserve]=reference;
		}
        }
}
Isam_Close(fgdata);

  fgfile=fopen(FCB_SEQDIR_FNGP ("/bldsubt.out"),"w");

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
     NREF = 0;
     INVALID_ATTRIBUTE=0;
     GET LIST(REFERENCE);
          DO WHILE(REFERENCE > 0);
          NREF = NREF + 1;
          IF NREF>HBOUND(FG_NAME,1)|REFERENCE>HBOUND(FG_NAME,1) THEN DO;
             PUT SKIP LIST('BLDSUBT ERROR: FG_NAME OVERFLOW');
             SIGNAL ERROR;
             END;
          REFN(NREF) = REFERENCE;
          NREF_REF(REFERENCE)=NREF; /* NEEDED FOR PRINTING NAMES OF INVALID
                                       ATTRIBUTES */
          GET LIST(FGINFO_STRING);
          I=INDEX(FGINFO_STRING,'~');
          IF I=0 THEN FG_NAME(NREF)=FGINFO_STRING;
          ELSE DO;
             FG_NAME(NREF)=SUBSTR(FGINFO_STRING,1,I-1);
             DO WHILE(I>0);
                INVALID_ATTRIBUTE(0,REFERENCE)=INVALID_ATTRIBUTE(0,REFERENCE)
                   +1;
                IF INVALID_ATTRIBUTE(0,REFERENCE)>5 THEN DO;
                   PUT SKIP LIST('BLDSUBT ERROR: OVERFLOW OF INVALID_ATTRIBUTE'
                      ||' ARRAY FOR '''||FG_NAME(NREF)||'''');
                   SIGNAL ERROR;
                   END;
                FGINFO_STRING=SUBSTR(FGINFO_STRING,I+1);
                I=INDEX(FGINFO_STRING,'~');
                IF I=0 THEN INVALID_ATTRIBUTE(INVALID_ATTRIBUTE(0,REFERENCE),
                   REFERENCE)=FGINFO_STRING;
                ELSE INVALID_ATTRIBUTE(INVALID_ATTRIBUTE(0,REFERENCE),
                   REFERENCE)=SUBSTR(FGINFO_STRING,1,I-1);
                END;
             END;
          GET LIST(REFERENCE);
          END;

     IF NREF > 0 THEN
          DO;
          CALL SORT_REF(REFN,NREF,'0'B);
               DO K = 1 TO NREF;
               WRITE FILE(FGNAME) FROM (REFN(SUB(K)));
               WRITE FILE(FGNAME) FROM (FG_NAME(SUB(K)));
               PUT SKIP EDIT (REFN(SUB(K)) ,FG_NAME(SUB(K)))(F(5),A);
               PUT FILE(SUBDOC) SKIP EDIT (REFN(SUB(K)))(F(5));
               PUT FILE(SUBDOC) EDIT (FG_NAME(SUB(K)))(X(5),A);
               END;
          END;

ON ERROR BEGIN;
   CALL RESIGNAL();
END;

     PRESERVABLE='0'B;
     PMAP = 0;
     TRACE.OPTIONS=0;
     TRACE.PARMS=0;
     VARSTRING='PRESERVE';
     TABLE_BOUND=HBOUND(REF,1);
     I=0;
          DO WHILE('1'B);
          IF VARSTRING='PRESERVE' THEN
             GET LIST(VARSTRING);
          I=I+1;
          IF I>TABLE_BOUND THEN
               DO;
               PUT SKIP LIST('BLDSUBT ERROR: TABLE OVERFLOW');
               SIGNAL ERROR;
               END;
          GET LIST(REF(I));
          IF REF(I)>MAX_REF THEN MAX_REF=REF(I);
          PUT SKIP EDIT(I,VARSTRING,REF(I))(F(5),X(2),A,X(2),F(5));
          SLING(I)=VARSTRING;
          SLING_LENGTH(I)=LENGTH(VARSTRING);
          IF SLING_LENGTH(I)>MAX_SLING_LENGTH THEN
               DO;
               PUT SKIP LIST('BLDSUBT ERROR: SLING TOO LONG');
               SIGNAL ERROR;
               END;
          XTR=SLNGXTR(VARSTRING);
          VARSTRING=SBSTRST(XTR);
          CALL FREEXTR(XTR);
          SUBSTRUCTURE_STRING(I)=VARSTRING;
          SUBSTRUCTURE_LENGTH(I)=LENGTH(VARSTRING);
          PUT SKIP EDIT(VARSTRING)(X(5),A);
          IF SUBSTRUCTURE_LENGTH(I)>MAX_SUBSTRUCTURE_LENGTH THEN
               DO;
               PUT SKIP LIST('BLDSUBT ERROR: SUBSTRUCTURE TOO LONG');
               SIGNAL ERROR;
               END;
          GET LIST(VARSTRING);
          IF VARSTRING='PRESERVE' THEN
               DO;
               PRESERVABLE(I)='1'B;
               NUM_PRESERVE = NUM_PRESERVE + 1;
               PMAP(NUM_PRESERVE) = REF(I);
               PUT SKIP LIST('***   PRESERVABLE   ***');
               END;
          END;
 START_SORT:
#endif

/*
num_entries=i-1;
*/
map = (U16_t *) malloc (num_entries * sizeof (U16_t));
last_sub_str[0] = 0;
crtref=lastref=0;
sort ();
printf("BLDSUBT TABLE OF SUBSTITUENT SUBSTRUCTURES\n\n\n\n\n\n");
fprintf (fgfile, "%d %d %d\n",num_entries,max_ref,num_preserve);
for (i = 1; i <= num_entries; i++)
{
  m=map[i];
  crtref=table.ref[m];
  printf("%5d  %s\n     %s  %5d\n",i,substructure_string[m],sling[m],table.ref[m]);
  fprintf(fgfile,"%s\n",substructure_string[m]);
  if (strcmp(last_sub_str,substructure_string[m])==0)
  {
	sprintf(varstring,"CONTENTION FOR '%s' BETWEEN %03d and %03d",last_sub_str,crtref,lastref);
	dup=(char **) realloc(dup,++ndups * sizeof (char *));
        dup[ndups-1]=(char *) malloc (strlen(varstring) + 1);
	strcpy(dup[ndups-1],varstring);
  }
  else
  {
	strcpy(last_sub_str,substructure_string[m]);
        lastref=table.ref[m];
  }
  fprintf(fgfile,"%d\n",table.ref[m]);
}
printf("\n\n***  PRESERVABLE INDICES  ***\n");
for (i=1; i<=num_preserve; i++)
{
	fprintf(fgfile,"%d\n",table.pmap[i]);
	printf("%d\n",table.pmap[i]);
}
for (i=1; i<=max_ref; i++)
{
  for (j=0; j<6; j++) fprintf(fgfile," %d",invalid_attribute[i][j]);
  putc('\n',fgfile);
}
sort_ref(table.ref,num_entries);
if (ndups!=0)
{
  printf("WARNING: DUPLICATE ATTRIBUTES DETECTED...\n");
  for (i=0; i<ndups; i++)
  {
    printf("%s\n",dup[i]);
    free(dup[i]);
  }
  free(dup);
}
printf("BLDSUBT EXIT\n");
free(map);
fclose(fgfile);

/*
     NUM_ENTRIES=I;
     BEGIN;
     DCL MAP(NUM_ENTRIES) BIN FIXED(15);
     DCL LAST_SUB_STR CHAR(200) VAR INIT('');
     DCL (CRTREF,LASTREF) PIC'999' INIT(0);
     CALL SORT;
     PUT PAGE LIST('BLDSUBT TABLE OF SUBSTITUENT SUBSTRUCTURES');
     WRITE FILE(FGFILE) FROM (NUM_ENTRIES);
     WRITE FILE(FGFILE) FROM (MAX_REF);
     WRITE FILE(FGFILE) FROM (NUM_PRESERVE);
     PUT SKIP(5);
 
         DO I=1 TO NUM_ENTRIES;
          M=MAP(I);
          CRTREF=REF(M);
          PUT SKIP EDIT(I,SUBSTRUCTURE_STRING(M),SLING(M),REF(M))(F(5),
            X(2),A,SKIP,X(5),A,X(2),F(5));
          WRITE FILE(FGFILE) FROM(SUBSTRUCTURE_STRING(M));
          IF LAST_SUB_STR=SUBSTRUCTURE_STRING(M) THEN DO;
             CALL ALLOCT('DUP','BLDSUBT',DUPPTR,0,'CHAR VAR',500);
             DUP='CONTENTION FOR '''||LAST_SUB_STR||''' BETWEEN '||CRTREF||
                ' AND '||LASTREF;
             END;
          ELSE DO;
             LAST_SUB_STR=SUBSTRUCTURE_STRING(M);
             LASTREF=REF(M);
             END;
          WRITE FILE(FGFILE) FROM(REF(M));
          END;

          PUT SKIP(3) LIST('***  PRESERVABLE INDICES  ***');
          DO I=1 TO NUM_PRESERVE;
          WRITE FILE(FGFILE) FROM(PMAP(I));
          PUT SKIP LIST(PMAP(I));
          END;

          PUT SKIP(3) LIST('INVALID ATTRIBUTES:');
          PUT SKIP;
          PUT FILE(SUBDOC) SKIP(3) LIST('INVALID ATTRIBUTES:');
          PUT FILE(SUBDOC) SKIP;
          DO I=1 TO MAX_REF;
          DO J=0 TO 5;
          WRITE FILE(FGFILE) FROM(INVALID_ATTRIBUTE(J,I));
          END;
          IF INVALID_ATTRIBUTE(0,I)>0 THEN DO;
             PUT SKIP LIST('FOR '''||FG_NAME(NREF_REF(I))||''':');
             PUT FILE(SUBDOC) SKIP LIST('FOR '''||
                FG_NAME(NREF_REF(I))||''':');
             DO J=1 TO INVALID_ATTRIBUTE(0,I);
                PUT SKIP LIST('     '||FG_NAME(NREF_REF(INVALID_ATTRIBUTE(J,
                   I))));
                PUT FILE(SUBDOC) SKIP LIST('     '||
                   FG_NAME(NREF_REF(INVALID_ATTRIBUTE(J,I))));
                END;
             END;
          END;

          CALL SORT_REF(REF,NUM_ENTRIES,'1'B);
          DO I=1 TO NUM_ENTRIES;
          PUT FILE(SUBDOC) SKIP EDIT(REF(SUB(I)))(F(5));
          IF PRESERVABLE(SUB(I)) THEN
             PUT FILE(SUBDOC) EDIT(' PRESRV ')(A(8));
          PUT FILE(SUBDOC) EDIT(SLING(SUB(I)))(COL(14),A);
          END;

IF ALLOCN('DUP','BLDSUBT')>0 THEN DO;
   PUT SKIP EDIT('WARNING: DUPLICATE ATTRIBUTES DETECTED...')(A);
   DO WHILE(ALLOCN('DUP','BLDSUBT')>0);
      PUT SKIP EDIT(DUP)(A);
      CALL FREE('DUP','BLDSUBT',DUPPTR);
      END;
   END;

     PUT SKIP(10) LIST('BLDSUBT EXIT');
     RETURN;
*/
}

void sort()
{
  int i, temp;

  for (i=1; i<=num_entries; i++) map[i]=i;
  buildheap ();
  for (i=num_entries; i>=2; i--)
  {
    temp=map[1];
    map[1]=map[i];
    map[i]=temp;
    heapify(1,i-1);
  }
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
 SORT:PROC;
      DO I=1 TO NUM_ENTRIES;
      MAP(I)=I;
      END;
 CALL BUILDHEAP;
      DO I=NUM_ENTRIES TO 2 BY -1;
      TEMP=MAP(1);
      MAP(1)=MAP(I);
      MAP(I)=TEMP;
      CALL HEAPIFY(1,I-1);
      END;
 END /*SORT */;
#endif

void buildheap ()
{
  int i, n;

  n = num_entries/2;
  for (i=n; i>=1; i--) heapify (i, num_entries);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
 BUILDHEAP:PROC;
     DCL I BIN FIXED(15);
     DCL N BIN FIXED(15);
     N=DIVIDE(NUM_ENTRIES,2,15);
          DO I=N TO 1 BY -1;
          CALL HEAPIFY(I,NUM_ENTRIES);
          END;
 END /* BUILDHEAP */;
#endif

void heapify (int i, int j)
{
  int k, son, temp;

  smallest=substructure_string[map[i]];
  k=0;
  for (son=2*i; son<=2*i+1 && son<=j; son++) if (strcmp(substructure_string[map[son]],smallest)<0)
  {
    k=son;
    smallest=substructure_string[map[son]];
  }
  if (k!=0)
  {
    temp=map[i];
    map[i]=map[k];
    map[k]=temp;
    heapify (k,j);
  }
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
 HEAPIFY: PROC(I,J) RECURSIVE;
     DCL (I,J,K) BIN FIXED(15);
     DCL SON BIN FIXED(15);

     SMALLEST=SUBSTRUCTURE_STRING(MAP(I));
     K=0;
          DO SON=2*I TO 2*I+1 WHILE(SON<=J);
          IF SUBSTRUCTURE_STRING(MAP(SON))<SMALLEST THEN
               DO;
               K=SON;
               SMALLEST=SUBSTRUCTURE_STRING(MAP(SON));
               END;
          END;
     IF K^=0 THEN
          DO;
          TEMP=MAP(I);
          MAP(I)=MAP(K);
          MAP(K)=TEMP;
          CALL HEAPIFY(K,J);
          END;
 END /* HEAPIFY */;
 END /* BEGIN */;
#endif

void sort_ref (U16_t *refr, int n_ref)
{
  int k,l,temp;
  Boolean_t SWITCH;

  for (k=1; k<=n_ref; k++) table.sub[k]=k;
  SWITCH=TRUE;
  for (k=2; k<=n_ref && SWITCH; k++)
  {
    SWITCH=FALSE;
    for (l=n_ref; l>=k; l--) if (refr[table.sub[l-1]]>refr[table.sub[l]])
    {
	SWITCH=TRUE;
	temp=table.sub[l-1];
	table.sub[l-1]=table.sub[l];
	table.sub[l]=temp;
    }
  }
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
 SORT_REF: PROC(REFR,N_REF,DUPS_OK);

     /* SORTS THE ARRAYS BY REFERENCE NUMBER, USING GLOBAL ARRAY SUB */
     DCL REFR(*) BIN FIXED(15);
     DCL N_REF BIN FIXED(15);
     DCL DUPS_OK BIT(1);
     DCL (K,L) BIN FIXED(15);
     DCL SWITCH BIT(1);

          DO K = 1 TO N_REF;
          SUB(K) = K;
          END;

     SWITCH = '1'B;
          DO K = 2 TO N_REF WHILE(SWITCH);
          SWITCH = '0'B;
               DO L = N_REF TO K BY -1;
               IF REFR(SUB(L-1)) > REFR(SUB(L)) THEN
                    DO;
                    SWITCH = '1'B;
                    TEMP = SUB(L-1);
                    SUB(L-1) = SUB(L);
                    SUB(L) = TEMP;
                    END;
               ELSE IF ^DUPS_OK & REFR(SUB(L-1))=REFR(SUB(L)) THEN
                    DO;
                    PUT SKIP LIST('BLDSUBT ERROR: TWO FGNAMES '||
                       'GIVEN SAME FG#');
                    SIGNAL ERROR;
                    END;
               END; /* L LOOP */
          END; /* K LOOP */

 END; /* SORT_REF */

 SBSTRST:PROC(XTR) RETURNS(CHAR(250) VAR);
     /* CONSTRUCTS A SUBSTRUCTURE SLING */
     DCL XTR PTR;
     DCL NUM_ATOMS BIN FIXED(15);
     DCL NMATOMS ENTRY(PTR) RETURNS(BIN FIXED(15));
     DCL NUM_EDGES BIN FIXED(15);
     DCL I BIN FIXED(15);
     DCL DEGREE ENTRY(PTR,BIN FIXED(15)) RETURNS(BIN FIXED(15));
     DCL QSIZE BIN FIXED(15);
     DCL (QHEAD,QTAIL) BIN FIXED(15);
     DCL SUBSTRUCTURE_STRING CHAR(250) VAR;
     DCL CHAR3VAR CHAR(3) VAR;
     DCL ATOM_ID ENTRY(PTR,BIN FIXED(15)) RETURNS(BIN FIXED(15));
     DCL NUM_NEIGHBORS BIN FIXED(15);
     DCL J BIN FIXED(15);
     DCL NEIGHBOR BIN FIXED(15);
     DCL OLD_QTAIL BIN FIXED(15);
     DCL BXMLTR ENTRY(PTR,BIN FIXED(15),BIN FIXED(15)) RETURNS(
       BIN FIXED(15));
     DCL BOND_STRING(16) CHAR(2) INIT('01','02','03','04','05','06','07'
       ,'00','09','10','11','12','13','14','15','16');
     DCL SUBSTR BUILTIN;
     DCL ENTER_COUNTER BIN FIXED(15);
     DCL NEIGHBR ENTRY(PTR,BIN FIXED(15),BIN FIXED(15)) RETURNS(
       BIN FIXED(15));

     NUM_ATOMS=NMATOMS(XTR);
     NUM_EDGES=0;
          DO I=1 TO NUM_ATOMS;
             NUM_EDGES=DEGREE(XTR,I)+NUM_EDGES;
          END;
     NUM_EDGES=DIVIDE(NUM_EDGES,2,15);
     QSIZE=NUM_EDGES+1;
     BEGIN;
     DCL NODE_ENTRD(NUM_ATOMS) BIN FIXED(15);
     DCL NODE_EXPANDED(NUM_ATOMS) BIT(1);
     DCL EDGE_MARK(NUM_ATOMS,NUM_ATOMS) BIT(1);
     DCL 1 QUEUE(0:QSIZE),
              2 Q BIN FIXED(15),
              2 BOND BIN FIXED(15),
              2 ATOM BIN FIXED(15);
#endif

void sbstrst (Xtr_t *xtr, char *substructure_string)
{
  int num_atoms, num_edges, i, qsize, qhead, qtail, num_neighbors, j, neighbor, old_qtail, enter_counter;
  char char3var[4];
  char *bond_string[] = {"", "01", "02", "03", "04", "05", "06", "07", "00", "09", "10", "11", "12", "13", "14", "15", "16"};
  int *node_entrd;
  Boolean_t *node_expanded, **edge_mark;

  num_atoms=Xtr_NumAtoms_Get (xtr);
  for (i=num_edges=0; i<num_atoms; i++) num_edges+=Xtr_Attr_NumNeighbors_Get(xtr,i);
  num_edges/=2;
  qsize=num_edges+1;
  queue = (struct qst *) malloc ((qsize+1) * sizeof (struct qst));
  node_entrd=(int *) malloc (num_atoms * sizeof (int));
  node_expanded=(Boolean_t *) malloc (num_atoms * sizeof (Boolean_t));
  edge_mark = (Boolean_t **) malloc (num_atoms * sizeof (Boolean_t *));
  for (i=0; i<num_atoms; i++) edge_mark[i] = (Boolean_t *) malloc (num_atoms * sizeof (Boolean_t));
  queue[1].q=0;
  qhead=qtail=1;
  for (i=0; i<num_atoms; i++)
  {
    node_entrd[i]=0;
    node_expanded[i]=FALSE;
    for (j=0; j<num_atoms; j++) edge_mark[i][j]=FALSE;
  }
  enter_counter=node_entrd[0]=1;
  strcpy(substructure_string,"00");
  sprintf(char3var,"%3d",Xtr_Attr_Atomid_Get (xtr, 0));
  strcat(substructure_string,char3var);
  while(qtail<qsize)
  {
    if (!node_expanded[queue[qhead].q])
    {
      old_qtail=qtail;
      i=queue[qhead].q;
      node_expanded[i]=TRUE;
      strcat (substructure_string, "*");
      num_neighbors=Xtr_Attr_NumNeighbors_Get (xtr, i);
      for (j=0; j<num_neighbors; j++)
      {
        neighbor=Xtr_Attr_NeighborId_Get (xtr, i, j);
        if (!edge_mark[i][neighbor])
        {
	  edge_mark[i][neighbor]=edge_mark[neighbor][i]=TRUE;
	  qtail++;
	  queue[qtail].q=neighbor;
	  queue[qtail].bond=Xtr_Attr_NeighborBond_Get (xtr, i, j);
	  queue[qtail].atom=Xtr_Attr_Atomid_Get (xtr, neighbor);
        }
      }
      if (qtail>old_qtail+1) bubble_sort(old_qtail+1,qtail);
      for (j=old_qtail+1; j<=qtail; j++)
      {
        i=queue[j].q;
        strcat(substructure_string,bond_string[queue[j].bond]);
        if (node_entrd[i]==0)
	{
	  enter_counter++;
	  node_entrd[i]=enter_counter;
          sprintf(char3var,"%3d",queue[j].atom);
	}
	else sprintf(char3var,"/%2d",node_entrd[i]);
	strcat(substructure_string,char3var);
      }
    }
    qhead++;
  }
  strcat (substructure_string, "#");

  free (queue);
  free (node_entrd);
  free (node_expanded);
  for (i=0; i<num_atoms; i++) free (edge_mark[i]);
  free (edge_mark);
}

#ifdef ABCDEFGHIJKLMNOPQRSTUVWXYZ
     Q(1)=1;
     QHEAD=1;
     QTAIL=1;
     NODE_ENTRD=0;
     ENTER_COUNTER=1;
     NODE_ENTRD(1)=1;
     EDGE_MARK='0'B;
     NODE_EXPANDED='0'B;
     SUBSTRUCTURE_STRING='00';
     PUT STRING(CHAR3VAR) EDIT(ATOM_ID(XTR,1))(F(3));
     SUBSTRUCTURE_STRING=SUBSTRUCTURE_STRING || CHAR3VAR;
          DO WHILE(QTAIL<QSIZE);
          IF ^NODE_EXPANDED(Q(QHEAD)) THEN
               DO;
               OLD_QTAIL=QTAIL;
               I=Q(QHEAD);
               NODE_EXPANDED(I)='1'B;
               SUBSTRUCTURE_STRING=SUBSTRUCTURE_STRING || '*';
               NUM_NEIGHBORS=DEGREE(XTR,I);
                    DO J=1 TO NUM_NEIGHBORS;
                    NEIGHBOR=NEIGHBR(XTR,I,J);
                    IF ^EDGE_MARK(I,NEIGHBOR) THEN
                         DO;
                         EDGE_MARK(I,NEIGHBOR)='1'B;
                         EDGE_MARK(NEIGHBOR,I)='1'B;
                         QTAIL=QTAIL+1;
                         Q(QTAIL)=NEIGHBOR;
                         BOND(QTAIL)=BXMLTR(XTR,I,J);
                         ATOM(QTAIL)=ATOM_ID(XTR,NEIGHBOR);
                         END;
                    END;
               IF QTAIL>OLD_QTAIL+1 THEN CALL BUBBLE_SORT(OLD_QTAIL+1,
                 QTAIL);
                    DO J=OLD_QTAIL+1 TO QTAIL;
                    I=Q(J);
                    SUBSTRUCTURE_STRING=SUBSTRUCTURE_STRING ||
                      BOND_STRING(BOND(J));
                    IF NODE_ENTRD(I)=0 THEN
                         DO;
                         ENTER_COUNTER=ENTER_COUNTER+1;
                         NODE_ENTRD(I)=ENTER_COUNTER;
                         PUT STRING(CHAR3VAR) EDIT(ATOM(J))(F(3));
                         END;
                    ELSE DO;
                         PUT STRING(CHAR3VAR) EDIT(NODE_ENTRD(I))(F(3));
                         SUBSTR(CHAR3VAR,1,1)='/';
                         END;
                    SUBSTRUCTURE_STRING=SUBSTRUCTURE_STRING || CHAR3VAR;
                    END;
               END;
          QHEAD=QHEAD+1;
          END /* DO WHILE */;
     SUBSTRUCTURE_STRING=SUBSTRUCTURE_STRING || '#';
/* GAM 2/2/84: SUBSTITUTED # FOR + TO MAKE TABLE COMPATIBLE WITH ASCII */
     RETURN(SUBSTRUCTURE_STRING);

 BUBBLE_SORT:PROC(START,FINISH);
     DCL (START,FINISH) BIN FIXED(15);
     DCL (I,J) BIN FIXED(15);

          DO I=START+1 TO FINISH;
               DO J=FINISH TO I BY -1;
               IF BOND(J)>BOND(J-1) | ((BOND(J)=BOND(J-1)) &(ATOM(J)>
                 ATOM(J-1))) THEN
                    DO;
                    QUEUE(0)=QUEUE(J);
                    QUEUE(J)=QUEUE(J-1);
                    QUEUE(J-1)=QUEUE(0);
                    END;
               END;
          END;
 END /* BUBBLE_SORT */;
 END /* BEGIN */;
 END /* SBSTRST */;
 /*$$$$$$*/ END /* BLDSUBT */;
#endif

void bubble_sort (int start, int finish)
{
  int i, j;

  for (i=start+1; i<=finish; i++) for (j=finish; j>=i; j--)
    if (queue[j].bond>queue[j-1].bond || (queue[j].bond==queue[j-1].bond && queue[j].atom>queue[j-1].atom))
  {
    queue[0]=queue[j];
    queue[j]=queue[j-1];
    queue[j-1]=queue[0];
  }
}
