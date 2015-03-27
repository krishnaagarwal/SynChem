#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <time.h>

typedef struct
{
	char *val;
	int len;
} Str;

int *hash[65536], hashcount[65536], hashalloc[65536];

unsigned char gifbuff[200000], main_gifbuff[792]=
{'G', 'I', 'F', '8', '9', 'a',
 /* width: 6,7 */ 0x00, 0x05, /* height: 8,9 */ 0x80, 0x00,
 0x80, 0x00, 0x00,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 2 bits */
 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 3 bits */
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 4 bits */
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 5 bits */
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 6 bits */
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 7 bits */
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0,
 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, 0,0,0, /* 8 bits */
 0x2c,
 0x00, 0x00, 0x00, 0x00,
 /* width: 786,787 */ 0x00, 0x05, /* height: 788,789 */ 0x80, 0x00,
 0x00,
 0x02};

int bytes_used=0, gifsize=792, nbmask;
char buff[254];

Str CompTable[4096];
Str BuildString;
char BuildStringVal[65536];
char EncChar;
int OldIndex, ZeroCode, EOICode;
static int NumBits=2, CurrNumBits=3, ClearCode=4, bitpos=0, savedvalue=0;

unsigned char **bitmap;

int width,height;

void gifsend (unsigned char *, int, char *);

void init_hash ()
{
  int i;

  for (i=0; i<65536; i++)
  {
    hash[i]=(int *) malloc(32*sizeof(int));
    hashalloc[i]=32;
    hashcount[i]=0;
  }
}

int hash_value (Str arg, int include_enc)
{
  int i, len;
  unsigned short xor[4];

  for (i=0; i<4; i++) xor[i]=0;
  for (i=0; i<arg.len; i++) xor[i%4]^=arg.val[i]&15;
  if (include_enc) xor[i%4]^=EncChar&15;
  i = xor[0] | (xor[1]<<4) | (xor[2]<<8) | (xor[3]<<12);
  len = arg.len + include_enc;
  i ^= len;
  i ^= len<<6;
  return (i & 0xffff);
}

void add_to_hash ()
{
  int i, hashval;

  hashval = hash_value (CompTable[OldIndex - 1], 0);
  i = hashcount[hashval]++;
  if (hashcount[hashval]==hashalloc[hashval])
  {
    hashalloc[hashval]*=2;
    hash[hashval]=
      (int *) realloc(hash[hashval], hashalloc[hashval] * sizeof (int));
  }
  hash[hashval][i]=OldIndex-1;
}

void clear_hash ()
{
  int i;

  for (i=0; i<65536; i++) hashcount[i] = 0;
}

void restore_hash ()
{
  for (OldIndex=1; OldIndex<=ClearCode; OldIndex++) add_to_hash ();
}

int hash_find (int include_enc, int *foundinx)
{
  int hashval, i, j, k, found;

  hashval=hash_value(BuildString,0);
  for (i=found=0; i<hashcount[hashval] && !found; i++)
  {
    if (CompTable[hash[hashval][i]].len==BuildString.len)
    {
      for (j=0, k=BuildString.len-1, found=1; j<=k && found; j++, k--)
      {
        if (CompTable[hash[hashval][i]].val[j]!=BuildString.val[j] ||
          CompTable[hash[hashval][i]].val[k]!=BuildString.val[k]) found=0;
      }
    }
    if (found) *foundinx=hash[hashval][i];
  }
  if (!include_enc) return (found);

  hashval=hash_value(BuildString,1);
  for (i=found=0; i<hashcount[hashval] && !found; i++)
  {
    if (CompTable[hash[hashval][i]].len==BuildString.len+1 &&
      CompTable[hash[hashval][i]].val[BuildString.len]==EncChar)
    {
      for (j=0, k=BuildString.len-1, found=1; j<=k && found; j++, k--)
      {
        if (CompTable[hash[hashval][i]].val[j]!=BuildString.val[j] ||
          CompTable[hash[hashval][i]].val[k]!=BuildString.val[k]) found=0;
      }
    }
  }
  return (found);
}

void init_rest (int wbytes)
{
  int i,ncmap,reduction,imgw;

  imgw=wbytes*8/NumBits; /* true width */
  bitmap=(unsigned char **)malloc(height*sizeof(unsigned char *));
  for (i=0; i<height; i++) bitmap[i]=(unsigned char *)malloc(wbytes);
  for (i=0; i<791; i++) gifbuff[i]=main_gifbuff[i];
  gifbuff[i]=NumBits;
  gifbuff[10]|=17*(NumBits-1);
  gifbuff[6]=gifbuff[786]=imgw%256;
  gifbuff[7]=gifbuff[787]=imgw/256;
  gifbuff[8]=gifbuff[788]=height%256;
  gifbuff[9]=gifbuff[789]=height/256;
  ncmap=3<<NumBits;
  reduction=768-ncmap;
  for (i=0; i<reduction; i++) gifbuff[ncmap+i+13]=gifbuff[i+781];
  gifsize=792-reduction;
}

void InitVariables (int all)
{
  int i, j, bitmask;

  for (i=0, bitmask=1; i<CurrNumBits; i++, bitmask<<=1)
  {
    if (ClearCode & bitmask) savedvalue |= 1<<bitpos;
    bitpos++;
    if (bitpos==8)
    {
      buff[bytes_used++]=savedvalue;
      if (bytes_used==254)
      {
        gifbuff[gifsize++]=254;
        for (j=0; j<254; j++, gifsize++) gifbuff[gifsize] = buff[j];
        bytes_used=0;
      }
      bitpos=savedvalue=0;
    }
  }
  CurrNumBits=NumBits+1;
  ClearCode=1<<NumBits;
  EOICode=ClearCode+1;
  ZeroCode=ClearCode+2;
  BuildString.val=BuildStringVal;
  BuildString.len=0;
  clear_hash ();
  if (!all)
  {
    for (i=ZeroCode; i<OldIndex; i++)
    {
      free(CompTable[i].val);
      CompTable[i].val=NULL;
      CompTable[i].len=0;
    }
    restore_hash ();
    OldIndex=ZeroCode;
    return;
  }
  for (i=0; i<ClearCode; i++)
  {
    CompTable[i].val=(char *) malloc(1);
    CompTable[i].val[0]=i;
    CompTable[i].len=1;
  }
  restore_hash ();
  OldIndex=ZeroCode;
}

void LZWCompress (int wbytes,char *name)
{
FILE *f;
  int x,y,i,j,k,c,found,foundinx,bitmask;
  time_t now;

now = time (NULL);
fprintf(stderr,"start of LZWCompress: %s\n",ctime (&now));
/* Create string table "CompTable" (with all permissible data) */
	/* Note 1: not necessarily 0-255, e.g., plain text */
/* Create String "BuildString" (empty), character "EncChar" */
  InitVariables (1);
/* [1] Char EncChar = next char in datastream (if no more, goto [2]) */
  for (x=0; x<height*wbytes; x++)
  {
    c=bitmap[x/wbytes][x%wbytes];
    for (y=8-NumBits; y>=0; y-=NumBits)
    {
      EncChar=(c>>y)&nbmask;
now = time (NULL);
fprintf(stderr,"before hash_find: %s\n",ctime (&now));
      found = hash_find (1, &foundinx);
now = time (NULL);
fprintf(stderr,"after hash_find: %s\n",ctime (&now));
/*     If (BuildString + EncChar) is in CompTable { */
/*       BuildString = BuildString + EncChar, goto [1] */
/*     } (end if) */
      if (found) BuildString.val[BuildString.len++]=EncChar;
/*     output index of BuildString in CompTable */
/*     BuildString = EncChar */
/*     goto [1] */
      else
      {
        for (i=0, bitmask=1; i<CurrNumBits; i++, bitmask<<=1)
        {
          if (foundinx & bitmask) savedvalue |= 1<<bitpos;
          bitpos++;
          if (bitpos==8)
          {
            buff[bytes_used++]=savedvalue;
            if (bytes_used==254)
            {
              gifbuff[gifsize++]=254;
              for (j=0; j<254; j++, gifsize++) gifbuff[gifsize]=buff[j];
              bytes_used=0;
            }
            bitpos=savedvalue=0;
          }
        }
        /* ??? (following not explicitly in algorithm as described, but is necessary) */
        if (BuildString.len!=0)
        {
          if (OldIndex==(1<<(CurrNumBits))) CurrNumBits++;
          CompTable[OldIndex].val=(char *) malloc (BuildString.len+1);
          CompTable[OldIndex].len=BuildString.len+1;
          for (i=0; i<BuildString.len; i++) CompTable[OldIndex].val[i]=BuildString.val[i];
          CompTable[OldIndex].val[i]=EncChar;
now = time (NULL);
fprintf(stderr,"before add_to_hash/InitVariables: %s\n",ctime (&now));
          if (++OldIndex==4096) InitVariables(0);
          else add_to_hash ();
now = time (NULL);
fprintf(stderr,"after add_to_hash/InitVariables: %s\n",ctime (&now));
        }
        /* ??? (preceding not explicitly in algorithm as described, but is necessary) */
        BuildString.val[0]=EncChar;
        BuildString.len=1;
      }
    }
  }


now = time (NULL);
fprintf(stderr,"start of [2]: %s\n",ctime (&now));
/* [2] Output index of BuildString in CompTable */
  found = hash_find (0, &foundinx);
now = time (NULL);
fprintf(stderr,"after hash_find: %s\n",ctime (&now));
  for (i=0, bitmask=1; i<CurrNumBits; i++, bitmask<<=1)
  {
    if (foundinx & bitmask) savedvalue |= 1<<bitpos;
    bitpos++;
    if (bitpos==8)
    {
      buff[bytes_used++]=savedvalue;
      if (bytes_used==254)
      {
        gifbuff[gifsize++]=254;
        for (j=0; j<254; j++, gifsize++) gifbuff[gifsize]=buff[j];
        bytes_used=0;
      }
      bitpos=savedvalue=0;
    }
  }
  for (i=0, bitmask=1; i<CurrNumBits || bitpos<8; i++, bitmask<<=1)
  {
    if (EOICode & bitmask) savedvalue |= 1<<bitpos;
    bitpos++;
    if (bitpos==8)
    {
      buff[bytes_used++]=savedvalue;
      if (bytes_used==254)
      {
        gifbuff[gifsize++]=254;
        for (j=0; j<254; j++, gifsize++) gifbuff[gifsize]=buff[j];
        bytes_used=0;
      }
      if (i<CurrNumBits) bitpos=savedvalue=0;
    }
  }
  bitpos=savedvalue=0; /* reset for next time */
  if (bytes_used)
  {
    gifbuff[gifsize++]=bytes_used;
    for (j=0; j<bytes_used; j++, gifsize++) gifbuff[gifsize]=buff[j];
    bytes_used=0;
  }
  gifbuff[gifsize++]=0;
  gifbuff[gifsize++]=';';
  gifsend(gifbuff,gifsize,name);
}

void gifimg(unsigned char *bmap,int bmwidth,int bmheight,unsigned char cmp[][3],
  int ncolors,char *name)
{
  int x,y,shift,i,j,c,nbits,wbytes;
FILE *f;
char newname[10];

strcpy(newname,name);
*strstr(newname,".")=0;
f=fopen(newname,"wb");
fwrite(bmap,1,bmwidth*bmheight,f);
fclose(f);
  nbits=2;
  nbmask=3;
  if (ncolors>4)
  {
    nbits=4;
    nbmask=0xf;
  }
  if (ncolors>16)
  {
    nbits=8;
    nbmask=0xff;
  }
  NumBits=nbits;
  CurrNumBits=nbits+1;
  ClearCode=1<<nbits;
  width=bmwidth;
  height=bmheight;
  wbytes=(nbits*width+7)/8;
  init_hash ();
  init_rest (wbytes);
  for (i=0; i<ncolors; i++) for (j=0; j<3; j++) gifbuff[3*i+j+13]=cmp[i][j];

/* pack bits */
  for (y=0; y<height; y++)
  {
    for (x=0; x<wbytes; x++) bitmap[y][x]=0;
    for (i=x=0, shift=8-NumBits; i<width; i++)
    {
      c=bmap[y*width+i];
      c<<=shift;
      bitmap[y][x]|=c;
      shift-=NumBits;
      if (shift<0)
      {
        shift+=8;
        x++;
      }
    }
  }

/* then call LZWCompress */

  LZWCompress (wbytes,name);
  for (i=0; i<height; i++) free (bitmap[i]);
  free (bitmap);
  for (i=ZeroCode; i<OldIndex; i++)
    if (CompTable[i].val!=NULL)
  {
    free(CompTable[i].val);
    CompTable[i].val=NULL;
  }
  for (i=0; i<256; i++) free (hash[i]);
}
