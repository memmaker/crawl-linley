/*
 *  File:       debug.cc
 *  Summary:    Debug and wizard related functions.
 *  Written by: Linley Henzell and Jesse Jones
 *
 *  Change History (most recent first):
 *
 *               <4>     14/12/99       LRH             Added cast_spec_spell_name()
 *               <3>     5/06/99        JDJ     Added TRACE.
 *               <2>     -/--/--        JDJ     Added a bunch od debugging macros.
 *                                                   Old code is now #if WIZARD.
 *               <1>     -/--/--        LRH     Created
 */

#include "AppHdr.h"
#include "debug.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "direct.h"
#include "dungeon.h"
#include "invent.h"
#include "itemname.h"
#include "items.h"
#include "misc.h"
#include "monplace.h"
#include "monstuff.h"
#include "mon-util.h"
#include "mutation.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills.h"
#include "skills2.h"
#include "spl-cast.h"
#include "spl-util.h"
#include "stuff.h"

#ifndef WIZARD
#define WIZARD
#endif

#if DEBUG && WIN
#define MyDebugBreak() _asm {int 3}
#endif

#ifdef USE_TILE
#define get_input_line(x,y) {mpr_on(MODE_MPR);get_input_line(x,y);mpr_on(MODE_CRT);}
#endif

//-----------------------------------
//      Internal Variables
//
#if WIN
static HANDLE sConsole = NULL;
#endif

// ========================================================================
//      Internal Functions
// ========================================================================

//---------------------------------------------------------------
//
// BreakStrToDebugger
//
//---------------------------------------------------------------
#if DEBUG
static void BreakStrToDebugger(const char *mesg)
{

#if OSX
    fprintf(stderr, mesg);
// raise(SIGINT);               // this is what DebugStr() does on OS X according to Tech Note 2030
    int* p = NULL;              // but this gives us a stack crawl...
    *p = 0;
#elif MAC
     unsigned char s[50];

     int len = strlen(mesg);

     if (len > 255)
         len = 255;

     s[0] = (Byte) len;
     BlockMoveData(mesg, s + 1, len);

     DebugStr(s);

#elif WIN
    MSG msg;    // remove pending quit messages so the message box displays

    bool quitting = (bool)::PeekMessage(&msg, NULL, WM_QUIT, WM_QUIT, PM_REMOVE);

    char text[2500];

    int flags = MB_YESNO +   // want abort and ignore buttons
                             // (too bad we can't ditch the retry button...)
        MB_ICONERROR +       // display the icon for errors
        MB_TASKMODAL +       // don't let the user do anything else in the app
        MB_SETFOREGROUND;    // bring the app to the front

    strcpy(text, mesg);
#ifdef JP /* 訳不用？ */
    strcat(text, "\nDo you want to drop into the debugger?");
#else
    strcat(text, "\nDo you want to drop into the debugger?");
#endif

#ifdef JP /* 訳不用？ */
    int result = MessageBoxA(NULL, text, "Debug Break", flags);
#else
    int result = MessageBoxA(NULL, text, "Debug Break", flags);
#endif

    if (result == IDYES)
        MyDebugBreak();

    if (quitting)
        PostQuitMessage(msg.wParam);

#else
    fprintf(stderr, "%s\n", mesg);
    abort();
#endif
}
#endif


//---------------------------------------------------------------
//
// IsDebuggerPresent95
//
// From March 1999 Windows Developer's Journal. This should only
// be called if we're running on Win 95 (normally I'd add an
// ASSERT, but that's a bit dicy since this is called by ASSERT...)
//
//---------------------------------------------------------------
#if WIN
static bool IsDebuggerPresent95()
{
    bool present = false;

    const DWORD kDebuggerPresentFlag = 0x000000001;
    const DWORD kProcessDatabaseBytes = 190;
    const DWORD kOffsetFlags = 8;

    DWORD threadID = GetCurrentThreadId();
    DWORD processID = GetCurrentProcessId();
    DWORD obfuscator = 0;

#if __MWERKS__
    asm
    {
        mov ax, fs
            mov es, ax
            mov eax, 0x18
            mov eax, es:[eax]
            sub eax, 0x10 xor eax,[threadID] mov[obfuscator], eax
    }

#else
    _asm
    {
        mov ax, fs
            mov es, ax
            mov eax, 18 h
            mov eax, es:[eax]
            sub eax, 10 h xor eax,[threadID] mov[obfuscator], eax
    }
#endif

    const DWORD *processDatabase =
                    reinterpret_cast< const DWORD * >(processID ^ obfuscator);

    if (!IsBadReadPtr(processDatabase, kProcessDatabaseBytes))
    {
        DWORD flags = processDatabase[kOffsetFlags];

        present = (flags & kDebuggerPresentFlag) != 0;
    }

    return present;
}
#endif


//---------------------------------------------------------------
//
// IsDebuggerPresent
//
//---------------------------------------------------------------
#if WIN
bool IsDebuggerPresent()
{
    bool present = false;

    typedef BOOL(WINAPI * IsDebuggerPresentProc) ();

    HINSTANCE kernelH = LoadLibrary("KERNEL32.DLL");

    if (kernelH != NULL)
    {                           // should never fail

        IsDebuggerPresentProc proc =
            (IsDebuggerPresentProc)::GetProcAddress( kernelH,
                                                     "IsDebuggerPresent" );

        if (proc != NULL)       // only present in NT and Win 98
            present = proc() != 0;
        else
            present = IsDebuggerPresent95();
    }

    return present;
}
#endif


//---------------------------------------------------------------
//
// CreateConsoleWindow
//
//---------------------------------------------------------------
#if WIN
static void CreateConsoleWindow()
{
    ASSERT(sConsole == NULL);

    // Create the console window
    if (::AllocConsole())
    {
        // Get the console window's handle
        sConsole =::GetStdHandle(STD_ERROR_HANDLE);
        if (sConsole == INVALID_HANDLE_VALUE)
            sConsole = NULL;

        // Set some options
        if (sConsole != NULL)
        {
            VERIFY(::SetConsoleTextAttribute(sConsole, FOREGROUND_GREEN));
            // green text on a black background (there doesn't appear to
            // be a way to get black text)

            VERIFY(::SetConsoleTitle("Debug Log"));

            COORD size = { 80, 120 };

            VERIFY(::SetConsoleScreenBufferSize(sConsole, size));
        }
        else
#ifdef JP /* 訳不用？ */
            DEBUGSTR(L "Couldn't get the console window's handle!");
#else
            DEBUGSTR(L "Couldn't get the console window's handle!");
#endif
    }
    else
#ifdef JP /* 訳不用？ */
        DEBUGSTR(L "Couldn't allocate the console window!");
#else
        DEBUGSTR(L "Couldn't allocate the console window!");
#endif
}
#endif


#if DEBUG
//---------------------------------------------------------------
//
// TraceString
//
//---------------------------------------------------------------
static void TraceString(const char *mesg)
{
    // Write the string to the debug window
#if WIN
    if (IsDebuggerPresent())
    {
        OutputDebugStringA(mesg);       // if you're using CodeWarrior you'll need to enable the "Log System Messages" checkbox to get this working
    }
    else
    {
        if (sConsole == NULL)   // otherwise we'll use a console window
            CreateConsoleWindow();

        if (sConsole != NULL)
        {
            unsigned long written;

            VERIFY(WriteConsoleA(sConsole, mesg, strlen(mesg), &written, NULL));
        }
    }
#else
    fprintf(stderr, "%s", mesg);
#endif

    // Write the string to the debug log
    static bool inited = false;
    static FILE *file = NULL;

    if (!inited)
    {
        ASSERT(file == NULL);

        const char *fileName = "DebugLog.txt";

        file = fopen(fileName, "w");
        ASSERT(file != NULL);

        inited = true;
    }

    if (file != NULL)
    {
        fputs(mesg, file);
        fflush(file);           // make sure all the output makes it to the file

    }
}
#endif

#if MAC
#pragma mark -
#endif

// ========================================================================
//      Global Functions
// ========================================================================

//---------------------------------------------------------------
//
// AssertFailed
//
//---------------------------------------------------------------
#if DEBUG
void AssertFailed(const char *expr, const char *file, int line)
{
    char mesg[512];

#if MAC
#ifdef JP /* 訳不用？ */
    sprintf(mesg, "ASSERT(%s) in %s at line %d failed.", expr, file, line);
#else
    sprintf(mesg, "ASSERT(%s) in %s at line %d failed.", expr, file, line);
#endif

#else
    const char *fileName = file + strlen(file); // strip off path

    while (fileName > file && fileName[-1] != '\\')
        --fileName;

#ifdef JP /* 訳不用？ */
    sprintf(mesg, "ASSERT(%s) in '%s' at line %d failed.", expr, fileName,
#else
    sprintf(mesg, "ASSERT(%s) in '%s' at line %d failed.", expr, fileName,
#endif
            line);
#endif

    BreakStrToDebugger(mesg);
}
#endif


//---------------------------------------------------------------
//
// DEBUGSTR
//
//---------------------------------------------------------------
#if DEBUG
void DEBUGSTR(const char *format, ...)
{
    char mesg[2048];

    va_list args;

    va_start(args, format);
    vsprintf(mesg, format, args);
    va_end(args);

    BreakStrToDebugger(mesg);
}
#endif


//---------------------------------------------------------------
//
// TRACE
//
//---------------------------------------------------------------
#if DEBUG
void TRACE(const char *format, ...)
{
    char mesg[2048];

    va_list args;

    va_start(args, format);
    vsprintf(mesg, format, args);
    va_end(args);

    TraceString(mesg);
}
#endif // DEBUG

//---------------------------------------------------------------
//
// debug_prompt_for_monster
//
//---------------------------------------------------------------
#ifdef WIZARD
static int debug_prompt_for_monster( void )
{
    char  specs[80];
    char  obj_name[ ITEMNAME_SIZE ];
    char *ptr;

#ifdef JP
    mpr( "(ヒント: 『オークのゾンビ』などは生成できない)", MSGCH_PROMPT );
    mpr( "生成するモンスターの名前は？", MSGCH_PROMPT );
#else
    mpr( "(Hint: 'generated' names, eg 'orc zombie', won't work)", MSGCH_PROMPT );
    mpr( "Which monster by name? ", MSGCH_PROMPT );
#endif

    get_input_line( specs, sizeof( specs ) );

    if (specs[0] == '\0')
        return (-1);

    int mon = -1;

    for (int i = 0; i < NUM_MONSTERS; i++)
    {
        moname( i, true, DESC_PLAIN, obj_name );
#ifdef JP
        ptr = strstr( obj_name, specs );
#else
        ptr = strstr( strlwr(obj_name), strlwr(specs) );
#endif
        if (ptr != NULL)
        {
            mpr( obj_name );
            if (ptr == obj_name)
            {
                // we prefer prefixes over partial matches
                mon = i;
                break;
            }
            else
                mon = i;
        }
    }

    return (mon);
}
#endif

//---------------------------------------------------------------
//
// debug_prompt_for_skill
//
//---------------------------------------------------------------
#ifdef WIZARD
static int debug_prompt_for_skill( const char *prompt )
{
    char specs[80];

    mpr( prompt, MSGCH_PROMPT );
    get_input_line( specs, sizeof( specs ) );

    if (specs[0] == '\0')
        return (-1);

    int skill = -1;

    for (int i = 0; i < NUM_SKILLS; i++)
    {
        // avoid the bad values:
        if (i == SK_UNUSED_1 || (i > SK_UNARMED_COMBAT && i < SK_SPELLCASTING))
            continue;

        char sk_name[80];
        strncpy( sk_name, skill_name(i), sizeof( sk_name ) );
#ifdef JP
        char *ptr = strstr( sk_name, specs );
#else
        char *ptr = strstr( strlwr(sk_name), strlwr(specs) );
#endif
        if (ptr != NULL)
        {
            if (ptr == sk_name && strlen(specs) > 0)
            {
                // we prefer prefixes over partial matches
                skill = i;
                break;
            }
            else
                skill = i;
        }
    }

    return (skill);
}
#endif

//---------------------------------------------------------------
//
// debug_change_species
//
//---------------------------------------------------------------
#ifdef WIZARD
void debug_change_species( void )
{
    char specs[80];
    int i;

#ifdef JP
    mpr( "どの種族に変更したいのですか？ " , MSGCH_PROMPT );
#else
    mpr( "What species would you like to be now? " , MSGCH_PROMPT );
#endif
    get_input_line( specs, sizeof( specs ) );

    if (specs[0] == '\0')
        return;

    int sp = -1;

    for (int i = SP_HUMAN; i < NUM_SPECIES; i++)
    {
        char sp_name[80];
        strncpy( sp_name, species_name(i, you.experience_level), sizeof( sp_name ) );
#ifdef JP
        char *ptr = strstr( sp_name, specs );
#else
        char *ptr = strstr( strlwr(sp_name), strlwr(specs) );
#endif
        if (ptr != NULL)
        {
            if (ptr == sp_name && strlen(specs) > 0)
            {
                // we prefer prefixes over partial matches
                sp = i;
                break;
            }
            else
                sp = i;
        }
    }

    if (sp == -1)
#ifdef JP
        mpr( "その種族には変更できない。" );
#else
        mpr( "That species isn't available." );
#endif
    else
    {
        for (i = 0; i < NUM_SKILLS; i++)
        {
            you.skill_points[i] *= species_skills( i, sp );
            you.skill_points[i] /= species_skills( i, you.species );
        }

        you.species = sp;

        redraw_screen();
    }
}
#endif
//---------------------------------------------------------------
//
// debug_prompt_for_int
//
// If nonneg, then it returns a non-negative number or -1 on fail
// If !nonneg, then it returns an integer, and 0 on fail
//
//---------------------------------------------------------------
#ifdef WIZARD
static int debug_prompt_for_int( const char *prompt, bool nonneg )
{
    char specs[80];

    mpr( prompt, MSGCH_PROMPT );
#ifdef USE_TILE
    mpr_on(MODE_MPR);
    get_input_line( specs, sizeof( specs ) );
    mpr_on(MODE_CRT);
#else
    get_input_line( specs, sizeof( specs ) );
#endif
    if (specs[0] == '\0')
        return (nonneg ? -1 : 0);

    char *end;
    int   ret = strtol( specs, &end, 10 );

    if ((ret < 0 && nonneg) || (ret == 0 && end == specs))
        ret = (nonneg ? -1 : 0);

    return (ret);
}
#endif

/*
   Some debugging functions, accessable through keys like %, $, &, ) etc when
   a section of code in input() in acr.cc is uncommented.
 */

//---------------------------------------------------------------
//
// cast_spec_spell
//
//---------------------------------------------------------------
#ifdef WIZARD
void cast_spec_spell(void)
{
#ifdef JP
    int spell = debug_prompt_for_int( "どの番号の呪文を唱えますか？ ", true );
#else
    int spell = debug_prompt_for_int( "Cast which spell by number? ", true );
#endif

    if (spell == -1)
        canned_msg( MSG_OK );
    else
        your_spells( spell, 0, false );
}
#endif


//---------------------------------------------------------------
//
// cast_spec_spell_name
//
//---------------------------------------------------------------
#ifdef WIZARD
void cast_spec_spell_name(void)
{
    int i = 0;
    char specs[80];
    char spname[80];

#ifdef JP
    mpr( "どの名前の呪文を唱えますか？ ", MSGCH_PROMPT );
#else
    mpr( "Cast which spell by name? ", MSGCH_PROMPT );
#endif
    get_input_line( specs, sizeof( specs ) );

    for (i = 0; i < NUM_SPELLS; i++)
    {
        strncpy( spname, spell_title(i), sizeof( spname ) );
#ifdef JP
        if (strstr( spname, specs ) != NULL)
#else
        if (strstr( strlwr(spname), strlwr(specs) ) != NULL)
#endif
        {
            your_spells(i, 0, false);
            return;
        }
    }

#ifdef JP
    mpr((one_chance_in(20)) ? "魔法学校からやり直したまえ。"
                            : "そのような呪文は見当も付かない。");
#else
    mpr((one_chance_in(20)) ? "Maybe you should go back to WIZARD school."
                            : "I couldn't find that spell.");
#endif
}
#endif


//---------------------------------------------------------------
//
// create_spec_monster
//
//---------------------------------------------------------------
#ifdef WIZARD
void create_spec_monster(void)
{
#ifdef JP
    int mon = debug_prompt_for_int( "どの番号のモンスターを生成しますか？ ", true );
#else
    int mon = debug_prompt_for_int( "Create which monster by number? ", true );
#endif

    if (mon == -1)
        canned_msg( MSG_OK );
    else
        create_monster( mon, 0, BEH_SLEEP, you.x_pos, you.y_pos, MHITNOT, 250 );
}                               // end create_spec_monster()
#endif


//---------------------------------------------------------------
//
// create_spec_monster_name
//
//---------------------------------------------------------------
#ifdef WIZARD
void create_spec_monster_name(void)
{
    int mon = debug_prompt_for_monster();

    if (mon == -1)
    {
#ifdef JP
        mpr("そのようなモンスターは見当も付かない。");
#else
        mpr("I couldn't find that monster.");
#endif

        if (one_chance_in(20))
#ifdef JP
            mpr("そいつはどこかに隠れてるんじゃないかな。");
#else
            mpr("Maybe it's hiding.");
#endif
    }
    else
    {
        create_monster(mon, 0, BEH_SLEEP, you.x_pos, you.y_pos, MHITNOT, 250);
    }
}                               // end create_spec_monster_name()
#endif


//---------------------------------------------------------------
//
// level_travel
//
//---------------------------------------------------------------
#ifdef WIZARD
void level_travel( int delta )
{
    int   old_level = you.your_level;
    int   new_level = you.your_level + delta;

    if (delta == 0)
    {
#ifdef JP
        new_level = debug_prompt_for_int( "どの階に移動しますか？ ", true ) - 1;
#else
        new_level = debug_prompt_for_int( "Travel to which level? ", true ) - 1;
#endif
    }

    if (new_level < 0 || new_level >= 50)
    {
#ifdef JP
        mpr( "その階は範囲外です。" );
#else
        mpr( "That level is out of bounds." );
#endif
        return;
    }

    you.your_level = new_level - 1;
    grd[you.x_pos][you.y_pos] = DNGN_STONE_STAIRS_DOWN_I;
    down_stairs(true, old_level);
    untag_followers();
}                               // end level_travel()
#endif


//---------------------------------------------------------------
//
// create_spec_object
//
//---------------------------------------------------------------
#ifdef WIZARD
void create_spec_object(void)
{
    static int max_subtype[] =
    {
        NUM_WEAPONS,
        NUM_MISSILES,
        NUM_ARMOURS,
        NUM_WANDS,
        NUM_FOODS,
        0,              // unknown I
        NUM_SCROLLS,
        NUM_JEWELLERY,
        NUM_POTIONS,
        0,              // unknown II
        NUM_BOOKS,
        NUM_STAVES,
        0,              // Orbs         -- only one, handled specially
        NUM_MISCELLANY,
        0,              // corpses      -- handled specially
        0,              // gold         -- handled specially
        0,              // "gemstones"  -- no items of type
    };

    char           specs[80];
    char           obj_name[ ITEMNAME_SIZE ];
    char           keyin;

    char *         ptr;
    int            best_index;
    int            mon;
    int            i;

    int            class_wanted   = OBJ_UNASSIGNED;
    int            type_wanted    = -1;
    int            special_wanted = 0;

    int            thing_created;

    for (;;)
    {
#ifdef JP
        mpr(") - 武器        ( - 飛び道具  [ - 防具    / - ワンド   ?  - 巻物",
#else
        mpr(") - weapons     ( - missiles  [ - armour  / - wands    ?  - scrolls",
#endif
             MSGCH_PROMPT);
#ifdef JP
        mpr("= - 装身具      ! - 薬        : - 本      | - 杖       0  - オーブ",
#else
        mpr("= - jewellery   ! - potions   : - books   | - staves   0  - The Orb",
#endif
             MSGCH_PROMPT);
#ifdef JP
        mpr("} - 宝物        X - 死体      %% - 食料    $ - 金貨    ESC - 中止",
#else
        mpr("} - miscellany  X - corpses   %% - food    $ - gold    ESC - exit",
#endif
             MSGCH_PROMPT);

#ifdef JP
        mpr("どの種類のアイテムですか？ ", MSGCH_PROMPT);
#else
        mpr("What class of item? ", MSGCH_PROMPT);
#endif

        keyin = toupper( get_ch() );

        if (keyin == ')')
            class_wanted = OBJ_WEAPONS;
        else if (keyin == '(')
            class_wanted = OBJ_MISSILES;
        else if (keyin == '[' || keyin == ']')
            class_wanted = OBJ_ARMOUR;
        else if (keyin == '/' || keyin == '\\')
            class_wanted = OBJ_WANDS;
        else if (keyin == '?')
            class_wanted = OBJ_SCROLLS;
        else if (keyin == '=' || keyin == '"')
            class_wanted = OBJ_JEWELLERY;
        else if (keyin == '!')
            class_wanted = OBJ_POTIONS;
        else if (keyin == ':')
            class_wanted = OBJ_BOOKS;
        else if (keyin == '|')
            class_wanted = OBJ_STAVES;
        else if (keyin == '0' || keyin == 'O')
            class_wanted = OBJ_ORBS;
        else if (keyin == '}' || keyin == '{')
            class_wanted = OBJ_MISCELLANY;
        else if (keyin == 'X')
            class_wanted = OBJ_CORPSES;
        else if (keyin == '%')
            class_wanted = OBJ_FOOD;
        else if (keyin == '$')
            class_wanted = OBJ_GOLD;
        else if (keyin == ESCAPE || keyin == ' '
                || keyin == '\r' || keyin == '\n')
        {
            canned_msg( MSG_OK );
            return;
        }

        if (class_wanted != OBJ_UNASSIGNED)
            break;
    }

    // allocate an item to play with:
    thing_created = get_item_slot();
    if (thing_created == NON_ITEM)
    {
#ifdef JP
        mpr( "アイテムを配置できなかった。" );
#else
        mpr( "Could not allocate item." );
#endif
        return;
    }

    // turn item into appropriate kind:
    if (class_wanted == OBJ_ORBS)
    {
        mitm[thing_created].base_type = OBJ_ORBS;
        mitm[thing_created].sub_type  = ORB_ZOT;
        mitm[thing_created].quantity  = 1;
    }
    else if (class_wanted == OBJ_GOLD)
    {
#ifdef JP
        int amount = debug_prompt_for_int( "何枚の金貨を？ ", true );
#else
        int amount = debug_prompt_for_int( "How much gold? ", true );
#endif
        if (amount <= 0)
        {
            canned_msg( MSG_OK );
            return;
        }

        mitm[thing_created].base_type = OBJ_GOLD;
        mitm[thing_created].sub_type = 0;
        mitm[thing_created].quantity = amount;
    }
    else if (class_wanted == OBJ_CORPSES)
    {
        mon = debug_prompt_for_monster();

        if (mon == -1)
        {
#ifdef JP
            mpr( "存在しないモンスターです。" );
#else
            mpr( "No such monster." );
#endif
            return;
        }

        mitm[thing_created].base_type = OBJ_CORPSES;
        mitm[thing_created].sub_type  = CORPSE_BODY;
        mitm[thing_created].plus      = mon;
        mitm[thing_created].plus2     = 0;
        mitm[thing_created].special   = 210;
        mitm[thing_created].colour    = mons_colour(mon);;
        mitm[thing_created].quantity  = 1;
        mitm[thing_created].flags     = 0;
    }
    else
    {
#ifdef USE_X11
// SWIN_LIST に一覧を表示
        if (!mitm[thing_created].sub_type)
        {
            mpr_on(MODE_CRT);
            mitm[thing_created].base_type = class_wanted;
            mitm[thing_created].sub_type  = 0;
            mitm[thing_created].plus      = 0;
            mitm[thing_created].plus2     = 0;
            mitm[thing_created].special   = 0;
            mitm[thing_created].flags     = 0;
            mitm[thing_created].quantity  = 1;
            set_ident_flags( mitm[thing_created], ISFLAG_IDENT_MASK );

        textcolor(PIX_LIGHTGREY);
        gotoxy(1, 1);
            for (i = 0; i < max_subtype[ class_wanted ]; i++)
            {
                mitm[thing_created].sub_type = i;
                item_name( mitm[thing_created], DESC_PLAIN, obj_name );
                if ((i&1)==0)
                {
                    gotoxy(1, wherey());
            clear_to_end_of_line();
                    cprintf("%02d %s",i,obj_name);
                }
                else
                {
                    gotoxy(41, wherey());
                    cprintf("%02d %s\n",i,obj_name);
                }
            }
            mitm[thing_created].sub_type = 0;
        }
#endif

#ifdef JP
        mpr( "どの名前のアイテムですか？ ", MSGCH_PROMPT );
#else
        mpr( "What type of item? ", MSGCH_PROMPT );
#endif
        get_input_line( specs, sizeof( specs ) );

        if (specs[0] == '\0')
        {
            canned_msg( MSG_OK );
            return;
        }

        // In order to get the sub-type, we'll fill out the base type...
        // then we're going to iterate over all possible subtype values
        // and see if we get a winner. -- bwr
        mitm[thing_created].base_type = class_wanted;
        mitm[thing_created].sub_type  = 0;
        mitm[thing_created].plus      = 0;
        mitm[thing_created].plus2     = 0;
        mitm[thing_created].special   = 0;
        mitm[thing_created].flags     = 0;
        mitm[thing_created].quantity  = 1;
        set_ident_flags( mitm[thing_created], ISFLAG_IDENT_MASK );

        if (class_wanted == OBJ_ARMOUR)
        {
#ifdef JP
            if (strstr( "ナーガの", specs ))
#else
            if (strstr( "naga barding", specs ))
#endif
            {
                mitm[thing_created].sub_type = ARM_BOOTS;
                mitm[thing_created].plus2 = TBOOT_NAGA_BARDING;
            }
#ifdef JP
            else if (strstr( "セントールの", specs ))
#else
            else if (strstr( "centaur barding", specs ))
#endif
            {
                mitm[thing_created].sub_type = ARM_BOOTS;
                mitm[thing_created].plus2 = TBOOT_CENTAUR_BARDING;
            }
#ifdef JP
            else if (strstr( "魔法帽", specs ))
#else
            else if (strstr( "wizard's hat", specs ))
#endif
            {
                mitm[thing_created].sub_type = ARM_HELMET;
                mitm[thing_created].plus2 = THELM_WIZARD_HAT;
            }
#ifdef JP
            else if (strstr( "帽子", specs ))
#else
            else if (strstr( "cap", specs ))
#endif
            {
                mitm[thing_created].sub_type = ARM_HELMET;
                mitm[thing_created].plus2 = THELM_CAP;
            }
#ifdef JP
            else if (strstr( "兜", specs ))
#else
            else if (strstr( "helm", specs ))
#endif
            {
                mitm[thing_created].sub_type = ARM_HELMET;
                mitm[thing_created].plus2 = THELM_HELM;
            }
        }

        if (!mitm[thing_created].sub_type)
        {
            type_wanted = -1;
            best_index  = 10000;

            for (i = 0; i < max_subtype[ mitm[thing_created].base_type ]; i++)
            {
                mitm[thing_created].sub_type = i;
                item_name( mitm[thing_created], DESC_PLAIN, obj_name );
#ifdef JP
                ptr = strstr( obj_name, specs );
#else
                ptr = strstr( strlwr(obj_name), strlwr(specs) );
#endif
                if (ptr != NULL)
                {
                    // earliest match is the winner
                    if (ptr - obj_name < best_index)
                    {
                        mpr( obj_name );
                        type_wanted = i;
                        best_index = ptr - obj_name;
                    }
                }
#ifdef USE_X11
        //数字での入力
                if (i==atoi(specs) && i!=0)type_wanted = i;
                if (specs[0]=='0' && specs[1]<32) type_wanted = 0;
#endif
            }

            if (type_wanted == -1)
            {
#ifdef JP
                mpr( "存在しないアイテムです。" );
#else
                mpr( "No such item." );
#endif
                return;
            }

            mitm[thing_created].sub_type = type_wanted;
        }

        switch (mitm[thing_created].base_type)
        {
        case OBJ_MISSILES:
            mitm[thing_created].quantity = 30;
            // intentional fall-through
        case OBJ_WEAPONS:
        case OBJ_ARMOUR:
#ifdef JP
            mpr( "どの名前のエゴですか？ ", MSGCH_PROMPT );
#else
            mpr( "What ego type? ", MSGCH_PROMPT );
#endif
            get_input_line( specs, sizeof( specs ) );

            if (specs[0] != '\0')
            {
                special_wanted = 0;
                best_index = 10000;

                for (i = 1; i < 25; i++)
                {
                    mitm[thing_created].special = i;
                    item_name( mitm[thing_created], DESC_PLAIN, obj_name );
#ifdef JP
                    ptr = strstr( obj_name, specs );
#else
                    ptr = strstr( strlwr(obj_name), strlwr(specs) );
#endif
                    if (ptr != NULL)
                    {
                        // earliest match is the winner
                        if (ptr - obj_name < best_index)
                        {
                            mpr( obj_name );
                            special_wanted = i;
                            best_index = ptr - obj_name;
                        }
                    }
                }

                mitm[thing_created].special = special_wanted;
            }
            break;

        case OBJ_BOOKS:
            if (mitm[thing_created].sub_type == BOOK_MANUAL)
            {
#ifdef JP
                special_wanted = debug_prompt_for_skill( "どのスキルの虎の巻ですか？ " );
#else
                special_wanted = debug_prompt_for_skill( "A manual for which skill? " );
#endif
                if (special_wanted != -1)
                    mitm[thing_created].plus = special_wanted;
                else
#ifdef JP
                    mpr( "残念ながら、その技能の本は在庫にない。" );
#else
                    mpr( "Sorry, no books on that skill today." );
#endif
            }
            break;

        case OBJ_WANDS:
            mitm[thing_created].plus = 24;
            break;

        case OBJ_MISCELLANY:
            // Runes to "demonic", decks have 50 cards, ignored elsewhere?
            mitm[thing_created].plus = 50;
            break;

        case OBJ_FOOD:
        case OBJ_SCROLLS:
        case OBJ_POTIONS:
            mitm[thing_created].quantity = 12;
            break;

        default:
            break;
        }
    }

    item_colour( mitm[thing_created] );

    move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

    if (thing_created != NON_ITEM)
        canned_msg( MSG_SOMETHING_APPEARS );

}
#endif


//---------------------------------------------------------------
//
// create_spec_object
//
//---------------------------------------------------------------
#ifdef WIZARD
void tweak_object(void)
{
    char specs[50];
    char keyin;

#ifdef JP
    int item = prompt_invent_item( "どのアイテムを改造しますか？ ", -1 );
#else
    int item = prompt_invent_item( "Tweak which item? ", -1 );
#endif
    if (item == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    if (item == you.equip[EQ_WEAPON])
        you.wield_change = true;

    for (;;)
    {
        void *field_ptr = NULL;

        for (;;)
        {
            item_name( you.inv[item], DESC_INVENTORY_EQUIP, info );
            mpr( info );

#ifdef JP
            mpr( "a - 修正値1  b - 修正値2  c - 特性  d - 個数  ESC - 中止",
#else
            mpr( "a - plus  b - plus2  c - special  d - quantity  ESC - exit",
#endif
                 MSGCH_PROMPT );
#ifdef JP
            mpr( "どの点を改造しますか？ ", MSGCH_PROMPT );
#else
            mpr( "Which field? ", MSGCH_PROMPT );
#endif

            keyin = tolower( get_ch() );

            if (keyin == 'a')
                field_ptr = &(you.inv[item].plus);
            else if (keyin == 'b')
                field_ptr = &(you.inv[item].plus2);
            else if (keyin == 'c')
                field_ptr = &(you.inv[item].special);
            else if (keyin == 'd')
                field_ptr = &(you.inv[item].quantity);
            else if (keyin == ESCAPE || keyin == ' '
                    || keyin == '\r' || keyin == '\n')
            {
                canned_msg( MSG_OK );
                return;
            }

            if (keyin >= 'a' && keyin <= 'd')
                break;
        }

        if (keyin != 'c')
        {
            const short *const ptr = static_cast< short * >( field_ptr );
#ifdef JP
            snprintf( info, INFO_SIZE, "元の数値: %d (0x%04x)", *ptr, *ptr );
#else
            snprintf( info, INFO_SIZE, "Old value: %d (0x%04x)", *ptr, *ptr );
#endif
        }
        else
        {
            const long *const ptr = static_cast< long * >( field_ptr );
#ifdef JP
            snprintf( info, INFO_SIZE, "元の数値: %ld (0x%08lx)", *ptr, *ptr );
#else
            snprintf( info, INFO_SIZE, "Old value: %ld (0x%08lx)", *ptr, *ptr );
#endif
        }

        mpr( info );

#ifdef JP
        mpr( "新しい数値は？ ", MSGCH_PROMPT );
#else
        mpr( "New value? ", MSGCH_PROMPT );
#endif
        get_input_line( specs, sizeof( specs ) );

        if (specs[0] == '\0')
            return;

        char *end;
        int   new_value = strtol( specs, &end, 10 );

        if (new_value == 0 && end == specs)
            return;

        if (keyin != 'c')
        {
            short *ptr = static_cast< short * >( field_ptr );
            *ptr = new_value;
        }
        else
        {
            long *ptr = static_cast< long * >( field_ptr );
            *ptr = new_value;
        }
    }
}
#endif


//---------------------------------------------------------------
//
// stethoscope
//
//---------------------------------------------------------------
#if DEBUG_DIAGNOSTICS

static const char *enchant_names[] =
{
#ifdef JP /* 訳不用？ */
    "None",
    "Slow", "Haste", "*BUG-3*", "Fear", "Conf", "Invis",
    "YPois-1", "YPois-2", "YPois-3", "YPois-4",
    "YShug-1", "YShug-2", "YShug-3", "YShug-4",
    "YRot-1", "YRot-2", "YRot-3", "YRot-4",
    "Summon", "Abj-1", "Abj-2", "Abj-3", "Abj-4", "Abj-5", "Abj-6",
    "Corona-1", "Corona-2", "Corona-3", "Corona-4",
    "Charm", "YSticky-1", "YSticky-2", "YSticky-3", "YSticky-4",
    "*BUG-35*", "*BUG-36*", "*BUG-37*",
    "GlowShapeshifter", "Shapeshifter",
    "Tele-1", "Tele-2", "Tele-3", "Tele-4",
    "*BUG-44*", "*BUG-45*", "*BUG-46*", "*BUG-47*", "*BUG-48*", "*BUG-49*",
    "*BUG-50*", "*BUG-51*", "*BUG-52*", "*BUG-53*", "*BUG-54*", "*BUG-55*",
    "*BUG-56*",
    "Pois-1", "Pois-2", "Pois-3", "Pois-4",
    "Sticky-1", "Sticky-2", "Sticky-3", "Sticky-4",
    "OldAbj-1", "OldAbj-2", "OldAbj-3", "OldAbj-4", "OldAbj-5", "OldAbj-6",
    "OldCreatedFriendly", "SleepWary", "Submerged", "Short Lived",
    "*BUG-too big*"
#else
    "None",
    "Slow", "Haste", "*BUG-3*", "Fear", "Conf", "Invis",
    "YPois-1", "YPois-2", "YPois-3", "YPois-4",
    "YShug-1", "YShug-2", "YShug-3", "YShug-4",
    "YRot-1", "YRot-2", "YRot-3", "YRot-4",
    "Summon", "Abj-1", "Abj-2", "Abj-3", "Abj-4", "Abj-5", "Abj-6",
    "Corona-1", "Corona-2", "Corona-3", "Corona-4",
    "Charm", "YSticky-1", "YSticky-2", "YSticky-3", "YSticky-4",
    "*BUG-35*", "*BUG-36*", "*BUG-37*",
    "GlowShapeshifter", "Shapeshifter",
    "Tele-1", "Tele-2", "Tele-3", "Tele-4",
    "*BUG-44*", "*BUG-45*", "*BUG-46*", "*BUG-47*", "*BUG-48*", "*BUG-49*",
    "*BUG-50*", "*BUG-51*", "*BUG-52*", "*BUG-53*", "*BUG-54*", "*BUG-55*",
    "*BUG-56*",
    "Pois-1", "Pois-2", "Pois-3", "Pois-4",
    "Sticky-1", "Sticky-2", "Sticky-3", "Sticky-4",
    "OldAbj-1", "OldAbj-2", "OldAbj-3", "OldAbj-4", "OldAbj-5", "OldAbj-6",
    "OldCreatedFriendly", "SleepWary", "Submerged", "Short Lived",
    "*BUG-too big*"
#endif
};

void stethoscope(int mwh)
{
    struct dist stth;
    int steth_x, steth_y;
    int i, j;

    if (mwh != RANDOM_MONSTER)
        i = mwh;
    else
    {
#ifdef JP /* 訳不用？ */
        mpr( "Which monster?", MSGCH_PROMPT );
#else
        mpr( "Which monster?", MSGCH_PROMPT );
#endif

        direction( stth );

        if (!stth.isValid)
            return;

        if (stth.isTarget)
        {
            steth_x = stth.tx;
            steth_y = stth.ty;
        }
        else
        {
            steth_x = you.x_pos + stth.dx;
            steth_y = you.x_pos + stth.dy;
        }

        if (env.cgrid[steth_x][steth_y] != EMPTY_CLOUD)
        {
#ifdef JP /* 訳不用？ */
            snprintf( info, INFO_SIZE, "cloud type: %d delay: %d",
#else
            snprintf( info, INFO_SIZE, "cloud type: %d delay: %d",
#endif
                     env.cloud[ env.cgrid[steth_x][steth_y] ].type,
                     env.cloud[ env.cgrid[steth_x][steth_y] ].decay );

            mpr( info, MSGCH_DIAGNOSTICS );
        }

        if (mgrd[steth_x][steth_y] == NON_MONSTER)
        {
#ifdef JP /* 訳不用？ */
            snprintf( info, INFO_SIZE, "item grid = %d", igrd[steth_x][steth_y] );
#else
            snprintf( info, INFO_SIZE, "item grid = %d", igrd[steth_x][steth_y] );
#endif
            mpr( info, MSGCH_DIAGNOSTICS );
            return;
        }

        i = mgrd[steth_x][steth_y];
    }

    // print type of monster
#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE, "%s (id #%d; type=%d loc=(%d,%d) align=%s)",
#else
    snprintf( info, INFO_SIZE, "%s (id #%d; type=%d loc=(%d,%d) align=%s)",
#endif
              monam( menv[i].number, menv[i].type, true, DESC_CAP_THE ),
              i, menv[i].type,
              menv[i].x, menv[i].y,
#ifdef JP /* 訳不用？ */
              ((menv[i].attitude == ATT_FRIENDLY) ? "friendly" :
               (menv[i].attitude == ATT_HOSTILE)  ? "hostile" :
               (menv[i].attitude == ATT_NEUTRAL)  ? "neutral"
                                                  : "unknown alignment") );
#else
              ((menv[i].attitude == ATT_FRIENDLY) ? "friendly" :
               (menv[i].attitude == ATT_HOSTILE)  ? "hostile" :
               (menv[i].attitude == ATT_NEUTRAL)  ? "neutral"
                                                  : "unknown alignment") );
#endif

    mpr( info, MSGCH_DIAGNOSTICS );

    // print stats and other info
#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE,"HD=%d HP=%d/%d AC=%d EV=%d MR=%d SP=%d energy=%d num=%d flags=%02x",
#else
    snprintf( info, INFO_SIZE,"HD=%d HP=%d/%d AC=%d EV=%d MR=%d SP=%d energy=%d num=%d flags=%02x",
#endif
             menv[i].hit_dice,
             menv[i].hit_points, menv[i].max_hit_points,
             menv[i].armour_class, menv[i].evasion,
             mons_resist_magic( &menv[i] ),
             menv[i].speed, menv[i].speed_increment,
             menv[i].number, menv[i].flags );

    mpr( info, MSGCH_DIAGNOSTICS );

    // print behaviour information

    const int hab = monster_habitat( menv[i].type );

#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE, "hab=%s beh=%s(%d) foe=%s(%d) mem=%d target=(%d,%d)",
             ((hab == DNGN_DEEP_WATER)            ? "water" :
              (hab == DNGN_LAVA)                  ? "lava"
                                                  : "floor"),

             ((menv[i].behaviour == BEH_SLEEP)    ? "sleep" :
              (menv[i].behaviour == BEH_WANDER)   ? "wander" :
              (menv[i].behaviour == BEH_SEEK)     ? "seek" :
              (menv[i].behaviour == BEH_FLEE)     ? "flee" :
              (menv[i].behaviour == BEH_CORNERED) ? "cornered"
                                                  : "unknown"),
             menv[i].behaviour,

             ((menv[i].foe == MHITYOU)            ? "you" :
              (menv[i].foe == MHITNOT)            ? "none" :
              (menv[menv[i].foe].type == -1)      ? "unassigned monster"
                 : monam( menv[menv[i].foe].number, menv[menv[i].foe].type,
                          true, DESC_PLAIN )),
             menv[i].foe,
             menv[i].foe_memory,

             menv[i].target_x, menv[i].target_y );
#else
    snprintf( info, INFO_SIZE, "hab=%s beh=%s(%d) foe=%s(%d) mem=%d target=(%d,%d)",
             ((hab == DNGN_DEEP_WATER)            ? "water" :
              (hab == DNGN_LAVA)                  ? "lava"
                                                  : "floor"),

             ((menv[i].behaviour == BEH_SLEEP)    ? "sleep" :
              (menv[i].behaviour == BEH_WANDER)   ? "wander" :
              (menv[i].behaviour == BEH_SEEK)     ? "seek" :
              (menv[i].behaviour == BEH_FLEE)     ? "flee" :
              (menv[i].behaviour == BEH_CORNERED) ? "cornered"
                                                  : "unknown"),
             menv[i].behaviour,

             ((menv[i].foe == MHITYOU)            ? "you" :
              (menv[i].foe == MHITNOT)            ? "none" :
              (menv[menv[i].foe].type == -1)      ? "unassigned monster"
                 : monam( menv[menv[i].foe].number, menv[menv[i].foe].type,
                          true, DESC_PLAIN )),
             menv[i].foe,
             menv[i].foe_memory,

             menv[i].target_x, menv[i].target_y );
#endif

    mpr( info, MSGCH_DIAGNOSTICS );

    // print resistances
#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE, "resist: fire=%d cold=%d elec=%d pois=%d neg=%d",
#else
    snprintf( info, INFO_SIZE, "resist: fire=%d cold=%d elec=%d pois=%d neg=%d",
#endif
              mons_res_fire( &menv[i] ),
              mons_res_cold( &menv[i] ),
              mons_res_elec( &menv[i] ),
              mons_res_poison( &menv[i] ),
              mons_res_negative_energy( &menv[i] ) );

    mpr( info, MSGCH_DIAGNOSTICS );


    // print enchantments
#ifdef JP /* 訳不用？ */
    strncpy( info, "ench: ", INFO_SIZE );
#else
    strncpy( info, "ench: ", INFO_SIZE );
#endif
    for (j = 0; j < 6; j++)
    {
        if (menv[i].enchantment[j] >= NUM_ENCHANTMENTS)
            strncat( info, enchant_names[ NUM_ENCHANTMENTS ], INFO_SIZE );
        else
            strncat( info, enchant_names[ menv[i].enchantment[j] ], INFO_SIZE );

        if (strlen( info ) <= 70)
            strncat( info, " ", INFO_SIZE );
        else if (j < 5)
        {
            mpr( info, MSGCH_DIAGNOSTICS );
#ifdef JP /* 訳不用？ */
            strncpy( info, "ench: ", INFO_SIZE );
#else
            strncpy( info, "ench: ", INFO_SIZE );
#endif
        }
    }

    mpr( info, MSGCH_DIAGNOSTICS );

    if (menv[i].type == MONS_PLAYER_GHOST
        || menv[i].type == MONS_PANDEMONIUM_DEMON)
    {
#ifdef JP /* 訳不用？ */
        snprintf( info, INFO_SIZE, "Ghost damage: %d; brand: %d",
#else
        snprintf( info, INFO_SIZE, "Ghost damage: %d; brand: %d",
#endif
                  ghost.values[ GVAL_DAMAGE ], ghost.values[ GVAL_BRAND ] );
        mpr( info, MSGCH_DIAGNOSTICS );
    }
}                               // end stethoscope()
#endif

#if DEBUG_ITEM_SCAN
//---------------------------------------------------------------
//
// dump_item
//
//---------------------------------------------------------------
static void dump_item( const char *name, int num, const item_def &item )
{
    mpr( name, MSGCH_WARN );

#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE, "    item #%d:  base: %d; sub: %d; plus: %d; plus2: %d; special: %ld",
#else
    snprintf( info, INFO_SIZE, "    item #%d:  base: %d; sub: %d; plus: %d; plus2: %d; special: %ld",
#endif
             num, item.base_type, item.sub_type,
             item.plus, item.plus2, item.special );

    mpr( info );

#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE, "    quant: %d; colour: %d; ident: 0x%08lx; ident_type: %d",
#else
    snprintf( info, INFO_SIZE, "    quant: %d; colour: %d; ident: 0x%08lx; ident_type: %d",
#endif
             item.quantity, item.colour, item.flags,
             get_ident_type( item.base_type, item.sub_type ) );

    mpr( info );

#ifdef JP /* 訳不用？ */
    snprintf( info, INFO_SIZE, "    x: %d; y: %d; link: %d",
#else
    snprintf( info, INFO_SIZE, "    x: %d; y: %d; link: %d",
#endif
             item.x, item.y, item.link );

    mpr( info );
}

//---------------------------------------------------------------
//
// debug_item_scan
//
//---------------------------------------------------------------
void debug_item_scan( void )
{
    int   i;
    char  name[256];

    // unset marks
    for (i = 0; i < MAX_ITEMS; i++)
        mitm[i].flags &= (~ISFLAG_DEBUG_MARK);

    // First we're going to check all the stacks on the level:
    for (int x = 0; x < GXM; x++)
    {
        for (int y = 0; y < GYM; y++)
        {
            // These are unlinked monster inventory items -- skip them:
            if (x == 0 && y == 0)
                continue;

            // Looking for infinite stacks (ie more links than tems allowed)
            // and for items which have bad coordinates (can't find their stack)
            for (int obj = igrd[x][y]; obj != NON_ITEM; obj = mitm[obj].link)
            {
                // Check for invalid (zero quantity) items that are linked in
                if (!is_valid_item( mitm[obj] ))
                {
#ifdef JP /* 訳不用？ */
                    snprintf( info, INFO_SIZE, "Linked invalid item at (%d,%d)!", x, y);
#else
                    snprintf( info, INFO_SIZE, "Linked invalid item at (%d,%d)!", x, y);
#endif
                    mpr( info, MSGCH_WARN );
                    item_name( mitm[obj], DESC_PLAIN, name );
                    dump_item( name, obj, mitm[obj] );
                }

                // Check that item knows what stack it's in
                if (mitm[obj].x != x || mitm[obj].y != y)
                {
#ifdef JP /* 訳不用？ */
                    snprintf( info, INFO_SIZE, "Item position incorrect at (%d,%d)!", x, y);
#else
                    snprintf( info, INFO_SIZE, "Item position incorrect at (%d,%d)!", x, y);
#endif
                    mpr( info, MSGCH_WARN );
                    item_name( mitm[obj], DESC_PLAIN, name );
                    dump_item( name, obj, mitm[obj] );
                }

                // If we run into a premarked item we're in real trouble,
                // this will also keep this from being an infinite loop.
                if (mitm[obj].flags & ISFLAG_DEBUG_MARK)
                {
#ifdef JP /* 訳不用？ */
                    snprintf( info, INFO_SIZE, "Potential INFINITE STACK at (%d, %d)", x, y);
#else
                    snprintf( info, INFO_SIZE, "Potential INFINITE STACK at (%d, %d)", x, y);
#endif
                    mpr( info, MSGCH_WARN );
                    break;
                }

                mitm[obj].flags |= ISFLAG_DEBUG_MARK;
            }
        }
    }

    // Now scan all the items on the level:
    for (i = 0; i < MAX_ITEMS; i++)
    {
        if (!is_valid_item( mitm[i] ))
            continue;

        item_name( mitm[i], DESC_PLAIN, name );

        // Don't check (-1,-1) player items or (0,0) monster items
        if ((mitm[i].x > 0 || mitm[i].y > 0)
            && !(mitm[i].flags & ISFLAG_DEBUG_MARK))
        {
#ifdef JP /* 訳不用？ */
            mpr( "Unlinked item:", MSGCH_WARN );
#else
            mpr( "Unlinked item:", MSGCH_WARN );
#endif
            dump_item( name, i, mitm[i] );

#ifdef JP /* 訳不用？ */
            snprintf( info, INFO_SIZE, "igrd(%d,%d) = %d", mitm[i].x, mitm[i].y,
#else
            snprintf( info, INFO_SIZE, "igrd(%d,%d) = %d", mitm[i].x, mitm[i].y,
#endif
                     igrd[ mitm[i].x ][ mitm[i].y ] );
            mpr( info );

            // Let's check to see if it's an errant monster object:
            for (int j = 0; j < MAX_MONSTERS; j++)
            {
                for (int k = 0; k < NUM_MONSTER_SLOTS; k++)
                {
                    if (menv[j].inv[k] == i)
                    {
#ifdef JP /* 訳不用？ */
                        snprintf( info, INFO_SIZE, "Held by monster #%d: %s at (%d,%d)",
#else
                        snprintf( info, INFO_SIZE, "Held by monster #%d: %s at (%d,%d)",
#endif
                                 j, ptr_monam( &menv[j], DESC_CAP_A ),
                                 menv[j].x, menv[j].y );

                        mpr( info );
                    }
                }
            }
        }

        // Current bad items of interest:
        //   -- armour and weapons with large enchantments/illegal special vals
        //
        //   -- items described as questionable (the class 100 bug)
        //
        //   -- eggplant is an illegal throwing weapon
        //
        //   -- bola is an illegal fixed artefact
        //
        //   -- items described as buggy (typically adjectives out of range)
        //      (note: covers buggy, bugginess, buggily, whatever else)
        //
#ifdef JP /* 訳不用？ */
        if (strstr( name, "questionable" ) != NULL
            || strstr( name, "eggplant" ) != NULL
            || strstr( name, "bola" ) != NULL
            || strstr( name, "bugg" ) != NULL)
#else
        if (strstr( name, "questionable" ) != NULL
            || strstr( name, "eggplant" ) != NULL
            || strstr( name, "bola" ) != NULL
            || strstr( name, "bugg" ) != NULL)
#endif
        {
#ifdef JP /* 訳不用？ */
            mpr( "Bad item:", MSGCH_WARN );
#else
            mpr( "Bad item:", MSGCH_WARN );
#endif
            dump_item( name, i, mitm[i] );
        }
        else if ((mitm[i].base_type == OBJ_WEAPONS
                && (abs(mitm[i].plus) > 30
                    || abs(mitm[i].plus2) > 30
                    || (!is_random_artefact( mitm[i] )
                        && (mitm[i].special >= 30
                            && mitm[i].special < 181))))

            || (mitm[i].base_type == OBJ_MISSILES
                && (abs(mitm[i].plus) > 25
                    || (!is_random_artefact( mitm[i] )
                        && mitm[i].special >= 30)))

            || (mitm[i].base_type == OBJ_ARMOUR
                && (abs(mitm[i].plus) > 25
                    || (!is_random_artefact( mitm[i] )
                        && mitm[i].sub_type != ARM_HELMET
                        && mitm[i].special >= 30))))
        {
#ifdef JP /* 訳不用？ */
            mpr( "Bad plus or special value:", MSGCH_WARN );
#else
            mpr( "Bad plus or special value:", MSGCH_WARN );
#endif
            dump_item( name, i, mitm[i] );
        }
    }

    // Don't want debugging marks interfering with anything else.
    for (i = 0; i < MAX_ITEMS; i++)
        mitm[i].flags &= (~ISFLAG_DEBUG_MARK);

    // Quickly scan monsters for "program bug"s.
    for (i = 0; i < MAX_MONSTERS; i++)
    {
        const struct monsters *const monster = &menv[i];

        if (monster->type == -1)
            continue;

        moname( monster->type, true, DESC_PLAIN, name );

#ifdef JP /* 訳不用？ */
        if (strcmp( name, "program bug" ) == 0)
#else
        if (strcmp( name, "program bug" ) == 0)
#endif
        {
#ifdef JP /* 訳不用？ */
            mpr( "Program bug detected!", MSGCH_WARN );
#else
            mpr( "Program bug detected!", MSGCH_WARN );
#endif

            snprintf( info, INFO_SIZE,
#ifdef JP /* 訳不用？ */
                      "Buggy monster detected: monster #%d; position (%d,%d)",
#else
                      "Buggy monster detected: monster #%d; position (%d,%d)",
#endif
                      i, monster->x, monster->y );

            mpr( info, MSGCH_WARN );
        }
    }
}
#endif

//---------------------------------------------------------------
//
// debug_add_skills
//
//---------------------------------------------------------------
#ifdef WIZARD
void debug_add_skills(void)
{
#ifdef JP
    int skill = debug_prompt_for_skill( "どのスキルですか？(名前で入力) " );
#else
    int skill = debug_prompt_for_skill( "Which skill (by name)? " );
#endif

    if (skill == -1)
#ifdef JP
        mpr("そのスキルは存在しないようだ。");
#else
        mpr("That skill doesn't seem to exist.");
#endif
    else
    {
#ifdef JP
        mpr("練習中……。");
#else
        mpr("Exercising...");
#endif
        exercise(skill, 100);
    }
}                               // end debug_add_skills()
#endif

//---------------------------------------------------------------
//
// debug_set_skills
//
//---------------------------------------------------------------
#ifdef WIZARD
void debug_set_skills(void)
{
#ifdef JP
    int skill = debug_prompt_for_skill( "どのスキルですか？(名前で入力) " );
#else
    int skill = debug_prompt_for_skill( "Which skill (by name)? " );
#endif

    if (skill == -1)
#ifdef JP
        mpr("そのスキルは存在しないようだ。");
#else
        mpr("That skill doesn't seem to exist.");
#endif
    else
    {
        mpr( skill_name(skill) );
#ifdef JP
        int amount = debug_prompt_for_int( "何レベルにしますか？ ", true );
#else
        int amount = debug_prompt_for_int( "To what level? ", true );
#endif

        if (amount == -1)
            canned_msg( MSG_OK );
        else
        {
            const int points = (skill_exp_needed( amount + 1 )
                                * species_skills( skill, you.species )) / 100;

            you.skill_points[skill] = points + 1;
            you.skills[skill] = amount;

            calc_total_skill_points();

            redraw_skill( you.your_name, player_title() );

            switch (skill)
            {
            case SK_FIGHTING:
                calc_hp();
                break;

            case SK_SPELLCASTING:
            case SK_INVOCATIONS:
            case SK_EVOCATIONS:
                calc_mp();
                break;

            case SK_DODGING:
                you.redraw_evasion = 1;
                break;

            case SK_ARMOUR:
                you.redraw_armour_class = 1;
                you.redraw_evasion = 1;
                break;

            default:
                break;
            }
        }
    }
}                               // end debug_add_skills()
#endif


//---------------------------------------------------------------
//
// debug_set_all_skills
//
//---------------------------------------------------------------
#ifdef WIZARD
void debug_set_all_skills(void)
{
    int i;
#ifdef JP
    int amount = debug_prompt_for_int( "全スキルを何レベルに変更しますか？ ", true );
#else
    int amount = debug_prompt_for_int( "Set all skills to what level? ", true );
#endif

    if (amount < 0)             // cancel returns -1 -- bwr
        canned_msg( MSG_OK );
    else
    {
        if (amount > 27)
            amount = 27;

        for (i = SK_FIGHTING; i < NUM_SKILLS; i++)
        {
            if (i == SK_UNUSED_1
                || (i > SK_UNARMED_COMBAT && i < SK_SPELLCASTING))
            {
                continue;
            }

            const int points = (skill_exp_needed( amount + 1 )
                                * species_skills( i, you.species )) / 100;

            you.skill_points[i] = points + 1;
            you.skills[i] = amount;
        }

        redraw_skill( you.your_name, player_title() );

        calc_total_skill_points();

        calc_hp();
        calc_mp();

        you.redraw_armour_class = 1;
        you.redraw_evasion = 1;
    }
}                               // end debug_add_skills()
#endif


//---------------------------------------------------------------
//
// debug_add_mutation
//
//---------------------------------------------------------------
#ifdef WIZARD
bool debug_add_mutation(void)
{
    bool success = false;
    char specs[80];

    // Yeah, the gaining message isn't too good for this... but
    // there isn't an array of simple mutation names. -- bwr
#ifdef JP
    mpr( "どの突然変異ですか？(変異獲得時のメッセージで指定) \n", MSGCH_PROMPT );
#else
    mpr( "Which mutation (by message when getting mutation)? \n", MSGCH_PROMPT );
#endif
    get_input_line( specs, sizeof( specs ) );

    if (specs[0] == '\0')
        return (false);

    int mutation = -1;

    for (int i = 0; i < NUM_MUTATIONS; i++)
    {
        char mut_name[80];
        strncpy( mut_name, mutation_name( i, 1 ), sizeof( mut_name ) );
#ifdef JP
        char *ptr = strstr( mut_name, specs );
#else
        char *ptr = strstr( strlwr(mut_name), strlwr(specs) );
#endif
        if (ptr != NULL)
        {
            // we take the first mutation that matches
            mutation = i;
            break;
        }
    }

    if (mutation == -1)
#ifdef JP
        mpr("あなたをそんな風には改変できない！");
#else
        mpr("I can't warp you that way!");
#endif
    else
    {
#ifdef JP
        snprintf( info, INFO_SIZE, "発見: %s", mutation_name( mutation, 1 ) );
#else
        snprintf( info, INFO_SIZE, "Found: %s", mutation_name( mutation, 1 ) );
#endif
        mpr( info );

#ifdef JP
        int levels = debug_prompt_for_int( "何段階変異しますか？ ", false );
#else
        int levels = debug_prompt_for_int( "How many levels? ", false );
#endif

        if (levels == 0)
        {
            canned_msg( MSG_OK );
            success = false;
        }
        else if (levels > 0)
        {
            for (int i = 0; i < levels; i++)
            {
                if (mutate( mutation ))
                    success = true;
            }
        }
        else
        {
            for (int i = 0; i < -levels; i++)
            {
                if (delete_mutation( mutation ))
                    success = true;
            }
        }
    }

    return (success);
}                               // end debug_add_mutation()
#endif


//---------------------------------------------------------------
//
// debug_get_religion
//
//---------------------------------------------------------------
#ifdef WIZARD
void debug_get_religion(void)
{
    char specs[80];

#ifdef JP
    mpr( "どの神を信仰しますか？(名前で指定) ", MSGCH_PROMPT );
#else
    mpr( "Which god (by name)? ", MSGCH_PROMPT );
#endif
    get_input_line( specs, sizeof( specs ) );

    if (specs[0] == '\0')
        return;

    int god = -1;

    for (int i = 1; i < NUM_GODS; i++)
    {
        char name[80];
        strncpy( name, god_name(i), sizeof( name ) );
#ifdef JP
        char *ptr = strstr( name, specs );
#else
        char *ptr = strstr( strlwr(name), strlwr(specs) );
#endif
        if (ptr != NULL)
        {
            god = i;
            break;
        }
    }

    if (god == -1)
#ifdef JP
        mpr( "その神は今日は信徒を募集していないようだ。" );
#else
        mpr( "That god doesn't seem to be taking followers today." );
#endif
    else
    {
        grd[you.x_pos][you.y_pos] = 179 + god;
        god_pitch(god);
    }
}                               // end debug_add_skills()
#endif


void error_message_to_player(void)
{
#ifdef JP
    mpr("おお友よ、何かしらのバグが発生したようである。");
    mpr("そこからは可及的速やかに逃げ去るべきではないか。");
#else
    mpr("Oh dear. There appears to be a bug in the program.");
    mpr("I suggest you leave this level then save as soon as possible.");
#endif

}                               // end error_message_to_player()
