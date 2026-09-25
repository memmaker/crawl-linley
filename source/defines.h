/*
 *  File:       defines.h
 *  Summary:    Various definess used by Crawl.
 *  Written by: Linley Henzel
 *
 *      Abstract:       A variety of miscellaneous constant values are found here.
 *                      I think we should move the colors into an enum or something
 *                      because there are in numerical order.  But I'm too lazy to
 *                      do it myself.
 *
 *  Copyright ｩ 1999 Brian Robinson.  // Me?  How come?
 *
 *  Change History (most recent first):
 *
 *       <4>     7/29/00        JDJ     Renamed MNG NON_MONSTER, MNST MAX_MONSTERS, ITEMS MAX_ITEMS,
 *                                      ING NON_ITEM, CLOUDS MAX_CLOUDS, CNG EMPTY_CLOUD, NTRAPS MAX_TRAPS.
 *       <3>     9/25/99        CDL     linuxlib -> liblinux
 *       <2>     6/17/99        BCR     indented and added header
 *       <1>     --/--/--       LRH     Created
 */

#ifndef DEFINES_H
#define DEFINES_H

#define ESCAPE '\x1b'           // most ansi-friendly way I can think of defining this.

// there's got to be a better way...
#ifdef _LIBLINUX_IMPLEMENTATION
#elif macintosh
#else
    #ifndef TRUE
     #define TRUE 1
    #endif

    #ifndef FALSE
     #define FALSE 0
    #endif
#endif


// max size of inventory array {dlb}:
#define ENDOFPACK 52

// max size of monter array {dlb}:
#define MAX_MONSTERS 200
// number of monster enchantments
#define NUM_MON_ENCHANTS 6
// non-monster for mgrd[][] -- (MNST + 1) {dlb}:
#define NON_MONSTER 201

// max size of item list {dlb}:
#define MAX_ITEMS 500
// non-item -- (ITEMS + 1) {dlb}
#define NON_ITEM 501

// max size of cloud array {dlb}:
#define MAX_CLOUDS 100

// empty cloud -- (CLOUDS + 1) {dlb}:
#define EMPTY_CLOUD 101

// max x-bound for level generation {dlb}
#define GXM 80
// max y-bound for level generation {dlb}
#define GYM 70

// max traps per level
#define MAX_TRAPS 30

// max shops per level
#define MAX_SHOPS 5

// lowest grid value which can be passed by walking etc.
#define MINMOVE 31

// lowest grid value which can be seen through
#define MINSEE 11


// some shortcuts:
#define menv   env.mons
#define mitm   env.item
#define grd    env.grid
#define mgrd   env.mgrid
#define igrd   env.igrid


// (MNG) -- for a reason! see usage {dlb}:
#define MHITNOT 201
// (MNG + 1) -- for a reason! see usage {dlb}:
#define MHITYOU 202

// colors, such pretty colors ...
#ifndef DOS
#ifndef WIN32CONSOLE
    #define BLACK 0
    #define BLUE 1
    #define GREEN 2
    #define CYAN 3
    #define RED 4
    #define MAGENTA 5
    #define BROWN 6
    #define LIGHTGREY 7
    #define DARKGREY 8
    #define LIGHTBLUE 9
    #define LIGHTGREEN 10
    #define LIGHTCYAN 11
    #define LIGHTRED 12
    #define LIGHTMAGENTA 13
    #define YELLOW 14
    #define WHITE 15
#endif
#endif
//

#ifdef WIN32CONSOLE
#ifdef JP
    // WIN32CONSOLEでは黒い画面にBLUE 1の文字は暗すぎるのでLIGHTBLUE 9と同じ色に
    // 黒い画面にRED 4の文字は暗すぎるのでLIGHTRED 12と同じ色に
    #define BLUE 9
    #define RED 12
    //WIN32CONSOLEでのトウフ
    #define B_BLACK         0*16 + 0
    #define B_BLUE          1*16 + 1
    #define B_GREEN         2*16 + 2
    #define B_CYAN          3*16 + 3
    #define B_RED           4*16 + 4
    #define B_MAGENTA       5*16 + 5
    #define B_BROWN         6*16 + 6
    #define B_LIGHTGREY     7*16 + 7
    #define B_DARKGREY      8*16 + 8
    #define B_LIGHTBLUE     9*16 + 9
    #define B_LIGHTGREEN   10*16 +10
    #define B_LIGHTCYAN    11*16 +11
    #define B_LIGHTRED     12*16 +12
    #define B_LIGHTMAGENTA 13*16 +13
    #define B_YELLOW       14*16 +14
    #define B_WHITE 	   15*16 +15
    #define B_LIGHTGRAY    B_LIGHTGREY
    #define B_DARKGRAY     B_DARKGREY
#else
    #define BLUE 1
    #define RED 4
#endif
    #define BLACK 0
    //BLUE
    #define GREEN 2
    #define CYAN 3
    //RED
    #define MAGENTA 5
    #define BROWN 6
    #define LIGHTGREY 7
    #define DARKGREY 8
    #define LIGHTBLUE 9
    #define LIGHTGREEN 10
    #define LIGHTCYAN 11
    #define LIGHTRED 12
    #define LIGHTMAGENTA 13
    #define YELLOW 14
    #define WHITE 15
#endif
//
#ifdef DOS
    #include <conio.h>
    #define LIGHTGREY LIGHTGRAY
    #define DARKGREY DARKGRAY
#else
    #define LIGHTGRAY LIGHTGREY
    #define DARKGRAY DARKGREY
#endif

// Graphics Colors
#define PIX_BLACK 0
#define PIX_BLUE 1
#define PIX_GREEN 2
#define PIX_CYAN 3
#define PIX_RED 4
#define PIX_MAGENTA 5
#define PIX_BROWN 6
#define PIX_LIGHTGREY 7
#define PIX_DARKGREY 8
#define PIX_LIGHTBLUE 9
#define PIX_LIGHTGREEN 10
#define PIX_LIGHTCYAN 11
#define PIX_LIGHTRED 12
#define PIX_LIGHTMAGENTA 13
#define PIX_YELLOW 14
#define PIX_WHITE 15

// Colour options... these are used as bit flags along with the colour
// value in the low byte.

// This is used to signal curses (which has seven base colours) to
// try to get a brighter version using recommisioned attribute flags.
#define COLFLAG_CURSES_BRIGHTEN          0x0080

#ifdef USE_COLOUR_OPTS

    #define COLFLAG_FRIENDLY_MONSTER         0x0100

    enum CHAR_ATTRIBUTES
    {
        CHATTR_NORMAL,
        CHATTR_STANDOUT,
        CHATTR_BOLD,
        CHATTR_BLINK,
        CHATTR_UNDERLINE,
        CHATTR_REVERSE,
        CHATTR_DIM
    };

#endif

// required for stuff::coinflip()
#define IB1 1
#define IB2 2
#define IB5 16
#define IB18 131072
#define MASK (IB1 + IB2 + IB5)
// required for stuff::coinflip()

#define MINIMUM( xxx, yyy )     (((xxx) < (yyy)) ? (xxx) : (yyy))
#define MAXIMUM( xxx, yyy )     (((xxx) > (yyy)) ? (xxx) : (yyy))

// Convert capital letters into mystic numbers representing 
// CTRL sequences.  This is a macro because a lot of the type 
// it wants to be used in case labels.
#define CONTROL( xxx )          (xxx - 'A' + 1)

#ifdef USE_TILE
#define MODE_CRT   0  //cprintf > crt
#define MODE_MPR   1  //cprintf > message
#define MODE_STAT  2  //cprintf > character status
#define MODE_DNGN  3  //cprintf > dungeon screen

// MOUSE
#define KEYIN_MODE_NONE       0
#define KEYIN_MODE_COMMAND    1
#define KEYIN_MODE_TARGET     2
#define KEYIN_MODE_TARGET_DIR 3
#define KEYIN_MODE_TARGET_PRE 4
#define KEYIN_MODE_INVENT     5
#define KEYIN_MODE_MORE       6
#define KEYIN_MODE_MAP        7
#define KEYIN_MODE_PAPER_DOLL 8
#define KEYIN_MODE_END        9
#define KEYIN_MODE_MSG_REPLAY 10
#define KEYIN_MODE_ITEMLIST_COMMAND 11
#define KEYIN_MODE_MULTIPICK  12

#else
  // Not USE_TILE
  #define set_keyin_mode(x) {/*x*/}
#endif

#endif
