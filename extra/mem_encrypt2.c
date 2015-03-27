#include <stdio.h>

#include "mem_encrypt.h"

static char keychars[] = {'\n', ' ', '-', '.', '1', '2', ':', '?',
  'A', 'Q', 'W', 'a', 'b', 'd', 'e', 'f', 'h', 'i', 'l', 'm', 'n', 'o', 'r', 's', 't', 'x', 'y', 'z', 'N', 'u', 'c', 'k', 'g',
  'S', 'J', '!', 'B', 'O', 'F', 'p', 'w', 'H', 'G', 'v', 'U'};
/*
Wenn ist das Nunstruck git und Slotermeyer?\nJa! ... Beiderhund das Oder die Flipperwaldt gersput!\n
*/
static unsigned char odd_array[] = {0, 98, 10, 14, 20, 20, 1, 17, 23, 24, 1, 13, 11, 23, 1, 28, 29, 20, 23, 24, 22, 29, 30, 31, 1,
  32, 17, 24, 1, 29, 20, 13, 1, 33, 18, 21, 24, 14, 22, 19, 14, 26, 14, 22, 7, 0, 34, 11, 35, 1, 3, 3, 3, 1, 36, 14, 17, 13, 14, 22,
  16, 29, 20, 13, 1, 13, 11, 23, 1, 37, 13, 14, 22, 1, 13, 17, 14, 1, 38, 18, 17, 39, 39, 14, 22, 40, 11, 18, 13, 24, 1, 32, 14, 22,
  23, 39, 29, 24, 35, 0};
/*
Herb Gambolputty de von Ausfern-schplenden-schlitter-crasscrenbon-fried-digger-dingle-dangle-dongle-dungle-
burstein-von-knacker-thrasher-apple-banger-horowitz-ticolensic-grander-knotty-spelltinkle-grandlich-
grumblemeyer-spelterwasser-kurstlich-himbleeisen-bahnwagen-gutenabend-bitte-ein-nurnburger-bratwustle-
gernspurten-mitz-weimache-luber-hundsfut-gumberaber-shonedanker-kalbsfleisch-mittler-aucher von Hautkopft Ulm
*/
static unsigned char even_array[] = {1, 162, 41, 14, 22, 12, 1, 42, 11, 19, 12, 21, 18, 39, 29, 24, 24, 26, 1, 13, 14, 1, 43, 21,
  20, 1, 8, 29, 23, 15, 14, 22, 20, 2, 23, 30, 16, 39, 18, 14, 20, 13, 14, 20, 2, 23, 30, 16, 18, 17, 24, 24, 14, 22, 2, 30, 22, 11,
  23, 23, 30, 22, 14, 20, 12, 21, 20, 2, 15, 22, 17, 14, 13, 2, 13, 17, 32, 32, 14, 22, 2, 13, 17, 20, 32, 18, 14, 2, 13, 11, 20,
  32, 18, 14, 2, 13, 21, 20, 32, 18, 14, 2, 13, 29, 20, 32, 18, 14, 2, 12, 29, 22, 23, 24, 14, 17, 20, 2, 43, 21, 20, 2, 31, 20, 11,
  30, 31, 14, 22, 2, 24, 16, 22, 11, 23, 16, 14, 22, 2, 11, 39, 39, 18, 14, 2, 12, 11, 20, 32, 14, 22, 2, 16, 21, 22, 21, 40, 17,
  24, 27, 2, 24, 17, 30, 21, 18, 14, 20, 23, 17, 30, 2, 32, 22, 11, 20, 13, 14, 22, 2, 31, 20, 21, 24, 24, 26, 2, 23, 39, 14, 18,
  18, 24, 17, 20, 31, 18, 14, 2, 32, 22, 11, 20, 13, 18, 17, 30, 16, 2, 32, 22, 29, 19, 12, 18, 14, 19, 14, 26, 14, 22, 2, 23, 39,
  14, 18, 24, 14, 22, 40, 11, 23, 23, 14, 22, 2, 31, 29, 22, 23, 24, 18, 17, 30, 16, 2, 16, 17, 19, 12, 18, 14, 14, 17, 23, 14, 20,
  2, 12, 11, 16, 20, 40, 11, 32, 14, 20, 2, 32, 29, 24, 14, 20, 11, 12, 14, 20, 13, 2, 12, 17, 24, 24, 14, 2, 14, 17, 20, 2, 20, 29,
  22, 20, 12, 29, 22, 32, 14, 22, 2, 12, 22, 11, 24, 40, 29, 23, 24, 18, 14, 2, 32, 14, 22, 20, 23, 39, 29, 22, 24, 14, 20, 2, 19,
  17, 24, 27, 2, 40, 14, 17, 19, 11, 30, 16, 14, 2, 18, 29, 12, 14, 22, 2, 16, 29, 20, 13, 23, 15, 29, 24, 2, 32, 29, 19, 12, 14,
  22, 11, 12, 14, 22, 2, 23, 16, 21, 20, 14, 13, 11, 20, 31, 14, 22, 2, 31, 11, 18, 12, 23, 15, 18, 14, 17, 23, 30, 16, 2, 19, 17,
  24, 24, 18, 14, 22, 2, 11, 29, 30, 16, 14, 22, 1, 43, 21, 20, 1, 41, 11, 29, 24, 31, 21, 39, 15, 24, 1, 44, 18, 19};

void Mem_Encrypt (U8_t *string, int offset, int recnum, int length)
{
  int i;
  unsigned short arrlen;
  unsigned char *array;

  array = recnum % 2 ? odd_array : even_array;
/* DO NOT USE - Machine-dependent!
  arrlen = *(unsigned short *) array;
*/
  arrlen = 256 * array[0] + array[1];
  array += 2;
  for (i = 0; i < length; i++) if (string[i] != 0 && string[i] != keychars[array[(i + offset) % arrlen]])
    string[i] ^= keychars[array[(i + offset) % arrlen]];
}

void Mem_Scramble (U8_t *string, int length)
{
  int i, j;

  if (length==0) return;
  for (i=0; i<length-3; i+=3)
  {
    j=string[i];
    string[i]=string[i+2];
    string[i+2]=j;
  }
  j=string[i];
  string[i]=string[length-1];
  string[length-1]=j;
}
