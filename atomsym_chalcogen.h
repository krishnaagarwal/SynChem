#ifndef _H_ATOMSYM_
#define _H_ATOMSYM_ 1
/******************************************************************************
*
*  Copyright (C) 1991-1996, Synchem Group at SUNY-Stony Brook
*
*
*  Module Name:                     ATOMSYM.H
*
*    This module contains the global information for the Atomic Symbol
*    manipulation abstraction.  This file contains all stuff related to
*    atomic symbols and atomic ids and weights and all translation between
*    the various sequences.
*
*    Routines are found in ATOMSYM.C
*
*  Creation Date:
*
*    01-Jan-1991
*
*  Authors:
*
*    Tito Autrey (rewritten based on others PLI code)
*
*  Modification History, reverse chronological
*
* Date       Author     Modification Description
*------------------------------------------------------------------------------
* 06-Jun-96  Krebsbach  Changed atom symols "Va" to "V" for Vanadium and
*                       "V" to "Px" to signify a polymer variable.
*
******************************************************************************/
/*** Literal Values ***/

#define WEIGHT_INVALID  -10000.0
#define VALENCE_INVALID  (U8_t)-100
#define MX_VALENCE       6

/* Atom name literals for references in the code */

#define HYDROGEN      1
#define CARBON        6
#define NITROGEN      7
#define OXYGEN        8
#define FLUORINE      9
#define SILICON       14
#define PHOSPHORUS    15
#define SULFUR        16
#define CHLORINE      17
#define SCANDIUM      21
#define SELENIUM      34
#define BROMINE       35
#define TELLURIUM     52
#define IODINE        53
#define POLONIUM      84
#define ASTATINE      85
#define LAWRENCIUM    103

#define POLYMER_VAR      110
#define ALKYL            133
#define GENERIC_HALOGEN  134
#define GENERIC_CHALCOGEN 135

/* Special types and variables
   Reserved ranges:
   000-127  -  Atoms
   128-144  -  Super Atoms  // #n
   145-160  -  BCG stuff
   161-176  -  Alkyl chains // &n
   177-192  -  Special symbols such as cation
   193-254  -  Variables    // $n
*/

#define ATOM_START       1
#define ATOM_END         127
#define SUPER_START      128
#define SUPER_END        144
#define BCG_START        145
#define BCG_END          160
#define ALKYL_START      161
#define ALKYL_END        176
#define SPECIAL_START    177
#define SPECIAL_END      192
#define VARIABLE_START   193
#define VARIABLE_END     254

/* Bond-Centered Graph Specials */

#define BCG_CorH     145
#define BCG_HALOGEN  146
#define BCG_CorX     147
#define BCG_CorHorX  148
#define BCG_NONHYDROGEN 149
#define BCG_ANYNOM   150
#define BCG_OM       151
#define BCG_ANYATOM  152

/* Special Symbols */

#define CATION       177  /* Used to be 110 and going up */
#define ANION        178
#define CARBENE      179
#define NITRENE      179
#define RADICAL      180

/*** Macros ***/

/* Macro Prototypes
   Boolean_t Atom_IsVariable (U16_t);
*/

#define Atom_IsVariable(id)\
  ((id) >= VARIABLE_START && (id) <= VARIABLE_END ? TRUE : FALSE)

/*** Routine Prototypes ***/

U8_t        Atomid_MaxValence (U16_t);
const char *Atomid2Symbol     (U16_t);
float       Atomid2Weight     (U16_t);
U16_t       Atomsymbol2Id     (U8_t *);
Boolean_t   Atomid_IsHalogen  (U16_t);
Boolean_t   Atomid_IsChalcogen (U16_t);

/*** Global Variables ***/

#ifdef ATOM_GLOBALS

const float GAtomicWeights[255] =
  { 0.00,   1.01,   4.00,   6.94,   9.01,  10.81,  12.01,  14.01,  16.00,
   19.00,  20.18,  22.99,  24.30,  26.98,  28.09,  30.97,  32.06,  35.45,
   39.95,  39.10,  40.08,  44.96,  47.90,  50.94,  52.00,  54.94,  55.85,
   58.93,  58.71,  63.55,  65.38,  69.72,  72.59,  74.92,  78.96,  79.90,
   83.80,  85.47,  87.62,  88.91,  91.22,  92.91,  95.94,  98.91, 101.07,
  102.91, 106.40, 107.87, 112.40, 114.82, 118.69, 121.75, 127.60, 126.90,
  131.30, 132.91, 137.34, 138.91, 140.12, 140.91, 144.24, 147.00, 150.40,
  151.96, 157.25, 158.93, 162.50, 164.93, 167.26, 168.93, 173.04, 174.97,
  178.49, 180.95, 183.85, 186.20, 190.20, 192.22, 195.09, 196.97, 200.59,
  204.37, 207.19, 208.98, 210.00, 210.00, 222.00, 223.00, 226.03, 227.00,
  232.04, 231.04, 238.03, 237.05, 244.00, 243.00, 247.00, 247.00, 251.00,
  254.00, 257.00, 258.00, 255.00, 256.00, 
    0.0   /* last zero is element 104 */  };

const char *GAtomicSymbols[255] =
  { "Err",  "H",    "He",   "Li",   "Be",   "B",    "C",    "N",    "O",
    "F",    "Ne",   "Na",   "Mg",   "Al",   "Si",   "P",    "S",    "Cl",
    "Ar",   "K",    "Ca",   "Sc",   "Ti",   "V",    "Cr",   "Mn",   "Fe",
    "Co",   "Ni",   "Cu",   "Zn",   "Ga",   "Ge",   "As",   "Se",   "Br",
    "Kr",   "Rb",   "Sr",   "Y",    "Zr",   "Nb",   "Mo",   "Tc",   "Ru",
    "Rh",   "Pd",   "Ag",   "Cd",   "In",   "Sn",   "Sb",   "Te",   "I",
    "Xe",   "Cs",   "Ba",   "La",   "Ce",   "Pr",   "Nd",   "Pm",   "Sm",
    "Eu",   "Gd",   "Tb",   "Dy",   "Ho",   "Er",   "Tm",   "Yb",   "Lu",
    "Hf",   "Ta",   "W",    "Re",   "Os",   "Ir",   "Pt",   "Au",   "Hg",
    "Tl",   "Pb",   "Bi",   "Po",   "At",   "Rn",   "Fr",   "Ra",   "Ac",
    "Th",   "Pa",   "U",    "Np",   "Pu",   "Am",   "Cm",   "Bk",   "Cf",
    "Es",   "Fm",   "Md",   "No",   "Lw",   "104",  "105",  "106",  "107",
    "X",    "D",    "Px",   "T",    "R",    "113",  "114",  "115",  "116",
    "117",  "118",  "119",  "120",  "121",  "122",  "123",  "124",  "125",
    "126",  "127",  "#q",   "#v",   "#w",   "#m",   "#z",   "#a",   "#j",
    "#k",   "#x",   "#y",   "#r",   "139",  "140",  "141",  "142",  "143",
    "144",  "145",  "146",  "147",  "148",  "149",  "150",  "151",  "152",
    "153",  "154",  "155",  "156",  "157",  "158",  "159",  "160",  "&0",
    "&1",   "&2",   "&3",   "&4",   "&5",   "&6",   "&7",   "&8",   "&9",
    "&10",  "&11",  "&12",  "&13",  "&14",  "&15",  "+`",   "-`",   ":`",
    ".`",   ", ",   "182",  "183",  "184",  "185",  "186",  "187",  "188",
    "189",  "190",  "191",  "192",  "$0",   "$1",   "$2",   "$3",   "$4",
    "$5",   "$6",   "$7",   "$8",   "$9",   "$10",  "$11",  "$12",  "$13",
    "$14",  "$15",  "$16",  "$17",  "$18",  "$19",  "$20",  "$21",  "$22",
    "$23",  "$24",  "$25",  "$26",  "$27",  "$28",  "$29",  "$30",  "$31",
    "225",  "226",  "227",  "228",  "229",  "230",  "231",  "232",  "233",
    "234",  "235",  "236",  "237",  "238",  "239",  "240",  "241",  "242",
    "243",  "244",  "245",  "246",  "247",  "248",  "249",  "250",  "251",
    "252",  "253",  "254" };

const char GSym2IdMap[340] = {
  " h  he li be b  c  n  o  f  ne na mg al si\
 p  s  cl ar k  ca sc ti v  cr mn fe co ni cu zn ga ge as se br kr rb sr y  zr\
 nb mo tc ru rh pd ag cd in sn sb te i  xe cs ba la ce pr nd pm sm eu gd tb\
 dy ho er tm yb lu hf ta w  re os ir pt au hg tl pb bi po at rn fr ra ac th\
 pa u  np pu am cm bk cf es fm md no lw .4 .5 .6 .7 x  d  px t  r  vv" };

const U8_t GValence[20] = {
   1,  0,  1,  2,  3,  4,  5,  2,  1,  0,  1,  2,  3,  4,  5,  6,  1,  0,  1,
   2 };
/* H  He  Li  Be   B   C   N   O   F  Ne  Na  Mg  Al  Si   P   S  Cl  Ar   K
   Ca */

#else
extern const float GAtomicWeights[255];
extern const char *GAtomicSymbols[255];
extern const char *GSym2IdMap[340];
#endif

/* End of AtomSym.H */
#endif
