/*
 *  File:       initfile.cc
 *  Summary:    Simple reading of an init file and system variables
 *  Written by: David Loewenstern
 *
 *  Change History (most recent first):
 *
 *      <3>     5 May 2000      GDL             Add field stripping for 'name'
 *      <2>     6/12/99         BWR             Added get_system_environment
 *      <1>     6/9/99          DML             Created
 */

#include "AppHdr.h"
#include "initfile.h"

#include <stdio.h>
#include <stdlib.h>
#include <string>
#include <ctype.h>

#include "Kills.h"
#include "externs.h"
#include "defines.h"
#include "player.h"
#include "stash.h"
#include "stuff.h"
#include "travel.h"
#include "items.h"
#include "view.h"

game_options    Options;

extern void (*viewwindow) (char, bool);
extern unsigned char (*mapch) (unsigned char);
extern unsigned char (*mapch2) (unsigned char);
extern unsigned char mapchar3(unsigned char ldfk);
extern unsigned char mapchar4(unsigned char ldfk);

#if defined(LINUX) && !defined(USE_X11)
extern int character_set;       // unices only
#endif

static std::string & tolower_string( std::string &str );

const static char *obj_syms = ")([/%.?=!.+\\0}X$";
const static int   obj_syms_len = 16;

// also used with macros
std::string & trim_string( std::string &str )
{
    // OK,  this is really annoying.  Borland C++ seems to define
    // basic_string::erase to take iterators,  and basic_string::remove
    // to take size_t or integer.  This is ass-backwards compared to
    // nearly all other C++ compilers.  Crap.             (GDL)
    //
    // Borland 5.5 does this correctly now... leaving the old code
    // around for now in case anyone needs it.  -- bwr
// #ifdef __BCPLUSPLUS__
//     str.remove( 0, str.find_first_not_of( " \t\n\r" ) );
//     str.remove( str.find_last_not_of( " \t\n\r" ) + 1 );
// #else
    str.erase( 0, str.find_first_not_of( " \t\n\r" ) );
    str.erase( str.find_last_not_of( " \t\n\r" ) + 1 );
// #endif

    return (str);
}

// returns -1 if unmatched else returns 0-15
static short str_to_colour( const std::string &str )
{
    int ret;

    const std::string cols[16] =
    {
#ifdef JP /* 訳不用？ */
        "black", "blue", "green", "cyan", "red", "magenta", "brown",
        "lightgrey", "darkgrey", "lightblue", "lightgreen", "lightcyan",
        "lightred", "lightmagenta", "yellow", "white"
#else
        "black", "blue", "green", "cyan", "red", "magenta", "brown",
        "lightgrey", "darkgrey", "lightblue", "lightgreen", "lightcyan",
        "lightred", "lightmagenta", "yellow", "white"
#endif
    };

    for (ret = 0; ret < 16; ret++)
    {
        if (str == cols[ret])
            break;
    }

    // check for alternate spellings
    if (ret == 16)
    {
#ifdef JP /* 訳不用？ */
        if (str == "lightgray")
#else
        if (str == "lightgray")
#endif
            ret = 7;
#ifdef JP /* 訳不用？ */
        else if (str == "darkgray")
#else
        else if (str == "darkgray")
#endif
            ret = 8;
    }

    return ((ret == 16) ? -1 : ret);
}

// returns -1 if unmatched else returns 0-15
static short str_to_channel_colour( const std::string &str )
{
    int ret = str_to_colour( str );

    if (ret == -1)
    {
#ifdef JP /* 訳不用？ */
        if (str == "mute")
#else
        if (str == "mute")
#endif
            ret = MSGCOL_MUTED;
#ifdef JP /* 訳不用？ */
        else if (str == "plain" || str == "off")
#else
        else if (str == "plain" || str == "off")
#endif
            ret = MSGCOL_PLAIN;
#ifdef JP /* 訳不用？ */
        else if (str == "default" || str == "on")
#else
        else if (str == "default" || str == "on")
#endif
            ret = MSGCOL_DEFAULT;
#ifdef JP /* 訳不用？ */
        else if (str == "alternate")
#else
        else if (str == "alternate")
#endif
            ret = MSGCOL_ALTERNATE;
    }

    return (ret);
}

// returns -1 if unmatched else returns 0-15
static short str_to_channel( const std::string &str )
{
    short       ret;

    const std::string cols[ NUM_MESSAGE_CHANNELS ] =
    {
#ifdef JP /* 訳不用？ */
        "plain", "prompt", "god", "duration", "danger", "warning", "food",
        "recovery", "talk", "intrinsic_gain", "mutation", "monster_spell",
        "monster_enchant", "monster_damage", "rotten_meat", "equipment",
        "diagnostic",
#else
        "plain", "prompt", "god", "duration", "danger", "warning", "food",
        "recovery", "talk", "intrinsic_gain", "mutation", "monster_spell",
        "monster_enchant", "monster_damage", "rotten_meat", "equipment",
        "diagnostic",
#endif
    };

    for (ret = 0; ret < NUM_MESSAGE_CHANNELS; ret++)
    {
        if (str == cols[ret])
            break;
    }

    return (ret == NUM_MESSAGE_CHANNELS ? -1 : ret);
}

static int str_to_weapon( const std::string &str )
{
#ifdef JP /* 訳不用？ */
    if (str == "shortsword" || str == "short sword")
#else
    if (str == "shortsword" || str == "short sword")
#endif
        return (WPN_SHORT_SWORD);
#ifdef JP /* 訳不用？ */
    else if (str == "mace")
#else
    else if (str == "mace")
#endif
        return (WPN_MACE);
#ifdef JP /* 訳不用？ */
    else if (str == "spear")
#else
    else if (str == "spear")
#endif
        return (WPN_SPEAR);
#ifdef JP /* 訳不用？ */
    else if (str == "trident")
#else
    else if (str == "trident")
#endif
        return (WPN_TRIDENT);
#ifdef JP /* 訳不用？ */
    else if (str == "hand axe" || str == "handaxe")
#else
    else if (str == "hand axe" || str == "handaxe")
#endif
        return (WPN_HAND_AXE);
#ifdef JP /* 訳不用？ */
    else if (str == "random")
#else
    else if (str == "random")
#endif
        return (WPN_RANDOM);

    return (WPN_UNKNOWN);
}

static unsigned int str_to_fire_types( const std::string &str )
{
#ifdef JP /* 訳不用？ */
    if (str == "launcher")
#else
    if (str == "launcher")
#endif
        return (FIRE_LAUNCHER);
#ifdef JP /* 訳不用？ */
    else if (str == "dart")
#else
    else if (str == "dart")
#endif
        return (FIRE_DART);
#ifdef JP /* 訳不用？ */
    else if (str == "stone")
#else
    else if (str == "stone")
#endif
        return (FIRE_STONE);
#ifdef JP /* 訳不用？ */
    else if (str == "dagger")
#else
    else if (str == "dagger")
#endif
        return (FIRE_DAGGER);
#ifdef JP /* 訳不用？ */
    else if (str == "spear")
#else
    else if (str == "spear")
#endif
        return (FIRE_SPEAR);
#ifdef JP /* 訳不用？ */
    else if (str == "hand axe" || str == "handaxe")
#else
    else if (str == "hand axe" || str == "handaxe")
#endif
        return (FIRE_HAND_AXE);
#ifdef JP /* 訳不用？ */
    else if (str == "club")
#else
    else if (str == "club")
#endif
        return (FIRE_CLUB);

    return (FIRE_NONE);
}

static void str_to_fire_order( const std::string &str,
                               FixedVector< int, NUM_FIRE_TYPES > &list )
{
    int i;
    size_t pos = 0;
    std::string item = "";

    for (i = 0; i < NUM_FIRE_TYPES; i++)
    {
        // get next item from comma delimited list
        const size_t end = str.find( ',', pos );
        item = str.substr( pos, end - pos );
        trim_string( item );

        list[i] = str_to_fire_types( item );

        if (end == std::string::npos)
            break;
        else
            pos = end + 1;
    }
}

static char str_to_race( const std::string &str )
{
    int index = -1;

    if (str.length() == 1)      // old system of using menu letter
        return (str[0]);
    else if (str.length() == 2) // scan abbreviations
        index = get_species_index_by_abbrev( str.c_str() );

    // if we don't have a match, scan the full names
    if (index == -1)
        index = get_species_index_by_name( str.c_str() );

    // skip over the extra draconians here
    if (index > SP_RED_DRACONIAN)
        index -= (SP_CENTAUR - SP_RED_DRACONIAN - 1);

    // SP_HUMAN is at 1, therefore we must subtract one.
    return ((index != -1) ? index_to_letter( index - 1 ) : '\0');
}

static char str_to_class( const std::string &str )
{
    int index = -1;

    if (str.length() == 1)      // old system of using menu letter
        return (str[0]);
    else if (str.length() == 2) // scan abbreviations
        index = get_class_index_by_abbrev( str.c_str() );

    // if we don't have a match, scan the full names
    if (index == -1)
        index = get_class_index_by_name( str.c_str() );

    return ((index != -1) ? index_to_letter( index ) : '\0');
}

static std::string & tolower_string( std::string &str )
{
    if (str.length())
    {
        for (std::string::iterator cp = str.begin(); cp != str.end(); cp++)
        {
            *cp = tolower( *cp );
        }
    }

    return (str);
}

static bool read_bool( const std::string &field, bool def_value )
{
    bool ret = def_value;

#ifdef JP /* 訳不用？ */
    if (field == "true" || field == "1")
#else
    if (field == "true" || field == "1")
#endif
        ret = true;

#ifdef JP /* 訳不用？ */
    if (field == "false" || field == "0")
#else
    if (field == "false" || field == "0")
#endif
        ret = false;

    return (ret);
}

void read_init_file(void)
{
    unsigned int i;

    // Option initialization
    Options.autopickups            = 0x0000;
    Options.verbose_dump           = false;
    Options.colour_map             = false;
    Options.clean_map              = false;
    Options.show_uncursed          = true;
    Options.always_greet           = false;
    Options.easy_open              = true;
    Options.easy_armour            = true;
    Options.easy_butcher           = false;
    Options.easy_confirm           = CONFIRM_SAFE_EASY;
    Options.easy_quit_item_prompts = false;
    Options.weapon                 = WPN_UNKNOWN;
    Options.random_pick            = false;
    Options.chaos_knight           = GOD_NO_GOD;
    Options.death_knight           = DK_NO_SELECTION;
    Options.priest                 = GOD_NO_GOD;
    Options.hp_warning             = 10;
    Options.hp_attention           = 25;
    Options.race                   = '\0';
    Options.cls                    = '\0';
    Options.terse_hand             = true;
    Options.auto_list              = false;
    Options.delay_message_clear    = false;
    Options.pickup_dropped         = true;
    Options.travel_colour          = true;
    Options.travel_delay           = -1;
    Options.travel_stair_cost      = 500;

#ifdef STASH_TRACKING
    Options.stash_tracking         = STM_NONE;
#endif
    Options.explore_item_stop      = true;
    Options.target_zero_exp        = true;
    Options.dump_kill_places       = KDO_ONE_PLACE;
    Options.dump_message_count     = 4;

    Options.flush_input[ FLUSH_ON_FAILURE ]     = true;
    Options.flush_input[ FLUSH_BEFORE_COMMAND ] = false;
    Options.flush_input[ FLUSH_ON_MESSAGE ]     = false;

    Options.lowercase_invocations  = false;

    // Note: These fire options currently match the old behaviour. -- bwr
    Options.fire_items_start       = 0;           // start at slot 'a'

    Options.fire_order[0] = FIRE_LAUNCHER;      // fire first from bow...
    Options.fire_order[1] = FIRE_DART;          // then only consider darts

    // clear the reast of the list
    for (i = 2; i < NUM_FIRE_TYPES; i++)
        Options.fire_order[i] = FIRE_NONE;

    // These are only used internally, and only from the commandline:
    // XXX: These need a better place.
    Options.sc_entries             = 0;
    Options.sc_format              = SCORE_REGULAR;

#ifdef USE_COLOUR_OPTS
    Options.friend_brand  = CHATTR_NORMAL;
    Options.no_dark_brand = 0;
#endif

#ifdef WIZARD
    Options.wiz_mode      = WIZ_NO;
#endif

#ifdef JP
    Options.use_zenkaku   = false;
#endif

#if 1 //JP
    Options.use_cake      = false;
#endif

#ifdef USE_TILE
    Options.use_tile      = true;
    Options.use_qv_mode   = false;
    Options.rotate_numpad = true;
    Options.rotate_minimap = false;

#if 1 //Slot
    Options.show_items[0] = '0';
#endif
#endif

    // map each colour to itself as default
#ifdef USE_8_COLOUR_TERM_MAP
    for (i = 0; i < 16; i++)
        Options.colour[i] = i % 8;

    Options.colour[ DARKGREY ] = COL_TO_REPLACE_DARKGREY;
#else
    for (i = 0; i < 16; i++)
        Options.colour[i] = i;
#endif

    Options.pick_items_start  = 0; // start at slot 'a'
    Options.enter_latest_name = true;

    // Setup travel information. What's a better place to do this?
    initialise_travel();

    // map each channel to plain (well, default for now since I'm testing)
    for (int i = 0; i < NUM_MESSAGE_CHANNELS; i++)
        Options.channels[i] = MSGCOL_DEFAULT;

    FILE *f;
    char s[255];
    unsigned int line = 0;
    int j;
    char name_buff[kPathLen];

    you.your_name[0] = '\0';

    if (SysEnv.crawl_rc)
    {
        f = fopen(SysEnv.crawl_rc, "r");
    }
    else if (SysEnv.crawl_dir)
    {
        strncpy(name_buff, SysEnv.crawl_dir, kPathLen);
        name_buff[ kPathLen - 1 ] = '\0';
        strncat(name_buff, "init.txt", kPathLen);
        name_buff[ kPathLen - 1 ] = '\0';

        f = fopen(name_buff, "r");
    }
#ifdef MULTIUSER
    else if (SysEnv.home)
    {
        // init.txt isn't such a good choice if we're looking in
        // the user's home directory, we'll use Un*x standard
        strncpy(name_buff, SysEnv.home, kPathLen);
        name_buff[ kPathLen - 1 ] = '\0';

        // likely not to have a closing slash so we'll insert one...
        strncat(name_buff, "/.crawlrc", kPathLen);
        name_buff[ kPathLen - 1 ] = '\0';

        f = fopen(name_buff, "r");
    }
#endif
    else
    {
        f = fopen("init.txt", "r");
    }

    if (f == NULL)
        return;

    while ( !feof(f) )
    //while (fgets(s, 255, f) != NULL)
    {
        s[0] = '\0';

        if ( fgets(s, 255, f) == NULL )
            break;

        line++;

        std::string str = s;
        trim_string( str );

        // This is to make some efficient comments
        if (s[0] == '#' || s[0] == '\0')
            continue;

        std::string key = "";
        std::string subkey = "";
        std::string field = "";

        int first_equals = str.find('=');
        int first_dot = str.find('.');

        // all lines with no equal-signs we ignore
        if (first_equals < 0)
            continue;

        if (first_dot > 0 && first_dot < first_equals)
        {
            key    = str.substr( 0, first_dot );
            subkey = str.substr( first_dot + 1, first_equals - first_dot - 1 );
            field  = str.substr( first_equals + 1 );
        }
        else
        {
            // no subkey (dots are okay in value field)
            key    = str.substr( 0, first_equals );
            subkey = "";
            field  = str.substr( first_equals + 1 );
        }

        // Clean up our data...
        tolower_string( trim_string( key ) );
        tolower_string( trim_string( subkey ) );

        // some fields want capitals... none care about external spaces
        trim_string( field );
        if (key != "name" && key != "crawl_dir"
            && key != "race" && key != "class" && key != "ban_pickup"
            && key != "stop_travel" && key != "sound")
        {
            tolower_string( field );
        }

        // everything not a valid line is treated as a comment
        if (key == "autopickup")
        {
            for (i = 0; i < field.length(); i++)
            {
                char type = field[i];

                // Make the amulet symbol equiv to ring -- bwross
                switch (type)
                {
                case '"':
                    // also represents jewellery
                    type = '=';
                    break;

                case '|':
                    // also represents staves
                    type = '\\';
                    break;

                case ':':
                    // also represents books
                    type = '+';
                    break;

                case 'x':
                    // also corpses
                    type = 'X';
                    break;
                }

                for (j = 0; j < obj_syms_len && type != obj_syms[j]; j++)
                    ;

                if (j < obj_syms_len)
                    Options.autopickups |= (1L << j);
                else
                {
#ifdef JP /* 訳不用？ */
                    fprintf( stderr, "Bad object type '%c' for autopickup.\n",
#else
                    fprintf( stderr, "Bad object type '%c' for autopickup.\n",
#endif
                             type );
                }
            }
        }
        else if (key == "name")
        {
            // field is already cleaned up from trim_string()
            strncpy(you.your_name, field.c_str(), kNameLen);
            you.your_name[ kNameLen - 1 ] = '\0';
        }
        else if (key == "verbose_dump")
        {
            // gives verbose info in char dumps
            Options.verbose_dump = read_bool( field, Options.verbose_dump );
        }
        else if (key == "clean_map")
        {
            // removes monsters/clouds from map
            Options.clean_map = read_bool( field, Options.clean_map );
        }
        else if (key == "colour_map" || key == "color_map")
        {
            // colour-codes play-screen map
            Options.colour_map = read_bool( field, Options.colour_map );
        }
        else if (key == "easy_confirm")
        {
            // allows both 'Y'/'N' and 'y'/'n' on yesno() prompts
            if (field == "none")
                Options.easy_confirm = CONFIRM_NONE_EASY;
            else if (field == "safe")
                Options.easy_confirm = CONFIRM_SAFE_EASY;
            else if (field == "all")
                Options.easy_confirm = CONFIRM_ALL_EASY;
        }
        else if (key == "easy_quit_item_lists")
        {
            // allow aborting of item lists with space
            Options.easy_quit_item_prompts = read_bool( field,
                                            Options.easy_quit_item_prompts );
        }
        else if (key == "easy_open")
        {
            // automatic door opening with movement
            Options.easy_open = read_bool( field, Options.easy_open );
        }
        else if (key == "easy_armour" || key == "easy_armour")
        {
            // automatic removal of armour when dropping
            Options.easy_armour = read_bool( field, Options.easy_armour );
        }
        else if (key == "easy_butcher")
        {
            // automatic knife switching
            Options.easy_butcher = read_bool( field, Options.easy_butcher );
        }
        else if (key == "colour" || key == "color")
        {
            const int orig_col   = str_to_colour( subkey );
            const int result_col = str_to_colour( field );

            if (orig_col != -1 && result_col != -1)
                Options.colour[orig_col] = result_col;
            else
            {
                fprintf( stderr, "Bad colour -- %s=%d or %s=%d\n",
                         subkey.c_str(), orig_col, field.c_str(), result_col );
            }
        }
        else if (key == "channel")
        {
            const int chnl = str_to_channel( subkey );
            const int col  = str_to_channel_colour( field );

            if (chnl != -1 && col != -1)
                Options.channels[chnl] = col;
            else if (chnl == -1)
                fprintf( stderr, "Bad channel -- %s\n", subkey.c_str() );
            else if (col == -1)
                fprintf( stderr, "Bad colour -- %s\n", field.c_str() );
        }
        else if (key == "background")
        {
            // change background colour
            // Experimental! This may look really bad!
            const int col = str_to_colour( field );

            if (col != -1)
                Options.background = col;
            else
                fprintf( stderr, "Bad colour -- %s\n", field.c_str() );

        }
#ifdef USE_COLOUR_OPTS
        else if (key == "friend_brand")
        {
            // Use curses attributes to mark friend
            // Some may look bad on some terminals.
            // As a suggestion, try "rxvt -rv -fn 10x20" under Un*xes
            if (field == "standout")               // probably reverses
                Options.friend_brand = CHATTR_STANDOUT;
            else if (field == "bold")              // probably brightens fg
                Options.friend_brand = CHATTR_BOLD;
            else if (field == "blink")             // probably brightens bg
                Options.friend_brand = CHATTR_BLINK;
            else if (field == "underline")
                Options.friend_brand = CHATTR_UNDERLINE;
            else if (field == "reverse")
                Options.friend_brand = CHATTR_REVERSE;
            else if (field == "dim")
                Options.friend_brand = CHATTR_DIM;
            else
#ifdef JP /* 訳不用？ */
                fprintf( stderr, "Bad colour -- %s\n", field.c_str() );
#else
                fprintf( stderr, "Bad colour -- %s\n", field.c_str() );
#endif
        }
        else if (key == "no_dark_brand")
        {
            // This is useful for terms where dark grey does
            // not have standout modes (since it's black on black).
            // This option will use light-grey instead in these cases.
            Options.no_dark_brand = read_bool( field, Options.no_dark_brand );
        }
#endif
        else if (key == "show_uncursed")
        {
            // label known uncursed items as "uncursed"
            Options.show_uncursed = read_bool( field, Options.show_uncursed );
        }
        else if (key == "always_greet")
        {
            // show greeting when reloading game
            Options.always_greet = read_bool( field, Options.always_greet );
        }
        else if (key == "weapon")
        {
            // choose this weapon for classes that get choice
            Options.weapon = str_to_weapon( field );
        }
        else if (key == "chaos_knight")
        {
            // choose god for Chaos Knights
            if (field == "xom")
                Options.chaos_knight = GOD_XOM;
            else if (field == "makhleb")
                Options.chaos_knight = GOD_MAKHLEB;
            else if (field == "random")
                Options.chaos_knight = GOD_RANDOM;
        }
        else if (key == "death_knight")
        {
            if (field == "necromancy")
                Options.death_knight = DK_NECROMANCY;
            else if (field == "yredelemnul")
                Options.death_knight = DK_YREDELEMNUL;
            else if (field == "random")
                Options.death_knight = DK_RANDOM;
        }
        else if (key == "priest")
        {
            // choose this weapon for classes that get choice
            if (field == "zin")
                Options.priest = GOD_ZIN;
            else if (field == "yredelemnul")
                Options.priest = GOD_YREDELEMNUL;
            else if (field == "random")
                Options.priest = GOD_RANDOM;
        }
        else if (key == "fire_items_start")
        {
            if (isalpha( field[0] ))
                Options.fire_items_start = letter_to_index( field[0] );
            else
            {
#ifdef JP /* 訳不用？ */
                fprintf( stderr, "Bad fire item start index -- %s\n",
#else
                fprintf( stderr, "Bad fire item start index -- %s\n",
#endif
                         field.c_str() );
            }
        }
        else if (key == "fire_order")
        {
            str_to_fire_order( field, Options.fire_order );
        }
        else if (key == "random_pick")
        {
            // randomly generate character
            Options.random_pick = read_bool( field, Options.random_pick );
        }
        else if (key == "hp_warning")
        {
            Options.hp_warning = atoi( field.c_str() );
            if (Options.hp_warning < 0 || Options.hp_warning > 100)
            {
                Options.hp_warning = 0;
#ifdef JP /* 訳不用？ */
                fprintf( stderr, "Bad HP warning percentage -- %s\n",
#else
                fprintf( stderr, "Bad HP warning percentage -- %s\n",
#endif
                         field.c_str() );
            }
        }
        else if (key == "hp_attention")
        {
            Options.hp_attention = atoi( field.c_str() );
            if (Options.hp_attention < 0 || Options.hp_attention > 100)
            {
                Options.hp_attention = 0;
#ifdef JP /* 訳不用？ */
                fprintf( stderr, "Bad HP attention percentage -- %s\n",
#else
                fprintf( stderr, "Bad HP attention percentage -- %s\n",
#endif
                         field.c_str() );
            }
        }
        else if (key == "crawl_dir")
        {
            // We shouldn't bother to allocate this a second time
            // if the user puts two crawl_dir lines in the init file.
            if (!SysEnv.crawl_dir)
                SysEnv.crawl_dir = (char *) calloc(kPathLen, sizeof(char));

            if (SysEnv.crawl_dir)
            {
                strncpy(SysEnv.crawl_dir, field.c_str(), kNameLen - 1);
                SysEnv.crawl_dir[ kNameLen - 1 ] = '\0';
            }
        }
        else if (key == "race")
        {
            Options.race = str_to_race( field );

            if (Options.race == '\0')
#ifdef JP /* 訳不用？ */
                fprintf( stderr, "Unknown race choice: %s\n", field.c_str() );
#else
                fprintf( stderr, "Unknown race choice: %s\n", field.c_str() );
#endif
        }
        else if (key == "class")
        {
            Options.cls = str_to_class( field );

            if (Options.cls == '\0')
#ifdef JP /* 訳不用？ */
                fprintf( stderr, "Unknown class choice: %s\n", field.c_str() );
#else
                fprintf( stderr, "Unknown class choice: %s\n", field.c_str() );
#endif
        }
        else if (key == "ban_pickup")
        {
            std::string::size_type pos;
            while ((pos = field.find(",", 0)) != std::string::npos)
            {
                if (pos > 0)
                    Options.banned_objects.push_back(field.substr(0, pos));
                field.erase(0, pos + 1);
            }
            if (field.length() > 0)
                Options.banned_objects.push_back(field);
        }
        else if (key == "pickup_thrown")
        {
            Options.pickup_thrown = read_bool(field, Options.pickup_thrown);
        }
        else if (key == "pickup_dropped")
        {
            Options.pickup_dropped = read_bool(field, Options.pickup_dropped);
        }
        else if (key == "travel_delay")
        {
            // Read travel delay in milliseconds.
            Options.travel_delay = atoi( field.c_str() );
            if (Options.travel_delay < -1)
                Options.travel_delay = -1;
            if (Options.travel_delay > 2000)
                Options.travel_delay = 2000;
        }
        else if (key == "travel_stair_cost")
        {
            Options.travel_stair_cost = atoi( field.c_str() );
            if (Options.travel_stair_cost < 1)
                Options.travel_stair_cost = 1;
            else if (Options.travel_stair_cost > 1000)
                Options.travel_stair_cost = 1000;
        }
        else if (key == "stop_travel")
        {
            std::string::size_type pos;
            while ((pos = field.find(",", 0)) != std::string::npos)
            {
                if (pos > 0)
                    Options.stop_travel.push_back(field.substr(0, pos));
                field.erase(0, pos + 1);
            }
            if (field.length() > 0)
                Options.stop_travel.push_back(field);
        }
#if 1 //MONOLITH
        else if (key == "travel_avoid_terrain")
        {
            std::string::size_type pos;
            while ((pos = field.find(",", 0)) != std::string::npos) {
                if (pos > 0)
                    prevent_travel_to(field.substr(0, pos));
                field.erase(0, pos + 1);
            }
            if (field.length() > 0)
                prevent_travel_to(field);
        }
        else if (key == "travel_colour")
        {
            Options.travel_colour = read_bool(field, Options.travel_colour);
        }
        else if (key == "explore_item_stop")
        {
            Options.explore_item_stop = read_bool(field, Options.explore_item_stop);
        }
#ifdef STASH_TRACKING
        else if (key == "stash_tracking")
        {
            Options.stash_tracking =
                 field == "explicit"? STM_EXPLICIT :
                 field == "dropped" ? STM_DROPPED  :
                 field == "all"     ? STM_ALL      :
                                      STM_NONE;
        }
        else if (key == "stash_filter")
        {
            std::string::size_type pos;
            while ((pos = field.find(",", 0)) != std::string::npos) {
                if (pos > 0)
                    Stash::filter(field.substr(0, pos));
                field.erase(0, pos + 1);
            }
            if (field.length() > 0)
                Stash::filter(field);
        }
#endif
        else if (key == "sound")
        {
            std::string::size_type pos = field.find(",", 0);
            while (pos != std::string::npos)
            {
                if (pos > 0)
                {
                    std::string sub = field.substr(0, pos);
                    std::string::size_type cpos = sub.find(":", 0);
                    if (cpos != std::string::npos)
                    {
                        FixedVector<std::string, 2> mapping;
                        mapping[0] = sub.substr(0, cpos);
                        mapping[1] = sub.substr(cpos + 1);
                        Options.sound_mappings.push_back(mapping);
                    }
                }
                field.erase(0, pos + 1);
            }
            if (field.length() > 0)
            {
                std::string::size_type cpos = field.find(":", 0);
                if (cpos != std::string::npos)
                {
                    FixedVector<std::string, 2> mapping;
                    mapping[0] = field.substr(0, cpos);
                    mapping[1] = field.substr(cpos + 1);
                    Options.sound_mappings.push_back(mapping);
                }
            }
        }
        else if (key == "dump_kill_places")
        {
            Options.dump_kill_places =
                field == "none"? KDO_NO_PLACES :
                field == "all" ? KDO_ALL_PLACES :
                                 KDO_ONE_PLACE;
        }
        else if (key == "dump_message_count")
        {
            // Capping is implicit
            Options.dump_message_count = atoi( field.c_str() );
            if (Options.dump_message_count > 50)
                Options.dump_message_count = 50;
        }
        else if (key == "target_zero_exp")
        {
            Options.target_zero_exp = read_bool(field, Options.target_zero_exp);
        }
#endif //MONOLITH
        else if (key == "auto_list")
        {
            Options.auto_list = read_bool( field, Options.auto_list );
        }
        else if (key == "delay_message_clear")
        {
            Options.delay_message_clear = read_bool( field, Options.delay_message_clear );
        }
        else if (key == "terse_hand")
        {
            Options.terse_hand = read_bool( field, Options.terse_hand );
        }
        else if (key == "flush")
        {
            if (subkey == "failure")
            {
                Options.flush_input[FLUSH_ON_FAILURE]
                    = read_bool(field, Options.flush_input[FLUSH_ON_FAILURE]);
            }
            else if (subkey == "command")
            {
                Options.flush_input[FLUSH_BEFORE_COMMAND]
                    = read_bool(field, Options.flush_input[FLUSH_BEFORE_COMMAND]);
            }
            else if (subkey == "message")
            {
                Options.flush_input[FLUSH_ON_MESSAGE]
                    = read_bool(field, Options.flush_input[FLUSH_ON_MESSAGE]);
            }
        }
        else if (key == "lowercase_invocations")
        {
            Options.lowercase_invocations
                    = read_bool(field, Options.lowercase_invocations);
        }
        else if (key == "wiz_mode")
        {
            // wiz_mode is recognized as a legal key in all compiles -- bwr
#ifdef WIZARD
            if (field == "never")
                Options.wiz_mode = WIZ_NEVER;
            else if (field == "no")
                Options.wiz_mode = WIZ_NO;
            else if (field == "yes")
                Options.wiz_mode = WIZ_YES;
            else
#ifdef JP /* 訳不用？ */
                fprintf(stderr, "Unknown wiz_mode option: %s\n", field.c_str());
#else
                fprintf(stderr, "Unknown wiz_mode option: %s\n", field.c_str());
#endif
#endif
        }

#ifdef JP
        //!!!!全角か半角か
        else if (key == "use_zenkaku" )
        {
            Options.use_zenkaku = read_bool( field, Options.use_zenkaku );
        }
        //!!!!トウフを使用するか否か
#endif
#if 1 //JP
        else if (key == "use_cake" )
        {
            Options.use_cake = read_bool( field, Options.use_cake );
        }
#endif

#ifdef USE_TILE
        else if (key == "use_tile" )
        {
            Options.use_tile = read_bool( field, Options.use_tile );
        }

        else if (key == "use_qv_mode" )
        {
            Options.use_qv_mode = read_bool( field, Options.use_qv_mode );
        }

        else if (key == "use_iso_mode" )
        {
            Options.use_qv_mode = read_bool( field, Options.use_qv_mode );
        }

        else if (key == "rotate_numpad" )
        {
            Options.rotate_numpad = read_bool( field, Options.rotate_numpad );
        }
        else if (key == "rotate_minimap" )
        {
            Options.rotate_minimap = read_bool( field, Options.rotate_minimap );
        }

#if 1 //Slot
        else if (key == "show_items" )
        {
            strncpy(Options.show_items, field.c_str(), 18);
        }
#endif
#endif

        else if (key == "stress_cursed" )
        {
            Options.stress_cursed = read_bool( field, Options.stress_cursed );
        }
        else if (key == "enter_latest_name")
        {
            Options.enter_latest_name = read_bool( field, Options.enter_latest_name );
        }
        else if (key == "pick_items_start")
        {
            if (isalpha( field[0] ))
                Options.pick_items_start = letter_to_index( field[0] );
            else
            {
#ifdef JP
                fprintf( stderr, "Bad pick item start index -- %s\n",
#else
                fprintf( stderr, "Bad pick item start index -- %s\n",
#endif
                         field.c_str() );
            }
        }
    }

    std::string str(255, ' '); //caract

    fclose(f);
    return;
}                               // end read_init_file()

void get_system_environment(void)
{
    // The player's name
    SysEnv.crawl_name = getenv("CRAWL_NAME");

    // The player's pizza
    SysEnv.crawl_pizza = getenv("CRAWL_PIZZA");

    // The directory which contians init.txt, macro.txt, morgue.txt
    // This should end with the appropriate path delimiter.
    SysEnv.crawl_dir = getenv("CRAWL_DIR");

    // The full path to the init file -- this over-rides CRAWL_DIR
    SysEnv.crawl_rc = getenv("CRAWL_RC");

    // rename giant and giant spiked clubs
    SysEnv.board_with_nail = (getenv("BOARD_WITH_NAIL") != NULL);

#ifdef MULTIUSER
    // The user's home directory (used to look for ~/.crawlrc file)
    SysEnv.home = getenv("HOME");
#endif
}                               // end get_system_environment()


// parse args, filling in Options and game environment as we go.
// returns true if no unknown or malformed arguments were found.

#ifdef JP /* 訳不用？ */
static const char *cmd_ops[] = { "scores", "name", "race", "class",
                                 "pizza", "plain", "dir", "rc", "tscores",
                                 "vscores" };
#else
static const char *cmd_ops[] = { "scores", "name", "race", "class",
                                 "pizza", "plain", "dir", "rc", "tscores",
                                 "vscores" };
#endif

const int num_cmd_ops = 10;
bool arg_seen[num_cmd_ops];

bool parse_args( int argc, char **argv, bool rc_only )
{
    if (argc < 2)           // no args!
        return (true);

    char *arg, *next_arg;
    int current = 1;
    bool nextUsed = false;
    int ecount;

    // initialize
    for(int i=0; i<num_cmd_ops; i++)
        arg_seen[i] = false;

    while(current < argc)
    {
        // get argument
        arg = argv[current];

        // next argument (if there is one)
        if (current+1 < argc)
            next_arg = argv[current+1];
        else
            next_arg = NULL;

        nextUsed = false;

        // arg MUST begin with '-' or '/'
        char c = arg[0];
        if (c != '-' && c != '/')
            return (false);

        // look for match (now we also except --scores)
        if (arg[1] == '-')
            arg = &arg[2];
        else
            arg = &arg[1];

        int o;
        for(o = 0; o < num_cmd_ops; o++)
        {
            if (stricmp(cmd_ops[o], arg) == 0)
                break;
        }

        if (o == num_cmd_ops)
            return (false);

        // disallow options specified more than once.
        if (arg_seen[o] == true)
            return (false);

        // set arg to 'seen'
        arg_seen[o] = true;

        // partially parse next argument
        bool next_is_param = false;
        if (next_arg != NULL)
        {
            if (next_arg[0] != '-' && next_arg[0] != '/')
                next_is_param = true;
        }

        //.take action according to the cmd chosen
        switch(o)
        {
        case 0:             // scores
        case 8:             // tscores
        case 9:             // vscores
            if (!next_is_param)
                ecount = SCORE_FILE_ENTRIES;            // default
            else // optional number given
            {
                ecount = atoi(next_arg);
                if (ecount < 1)
                    ecount = 1;

                if (ecount > SCORE_FILE_ENTRIES)
                    ecount = SCORE_FILE_ENTRIES;

                nextUsed = true;
            }

            if (!rc_only)
            {
                Options.sc_entries = ecount;

                if (o == 8)
                    Options.sc_format = SCORE_TERSE;
                else if (o == 9)
                    Options.sc_format = SCORE_VERBOSE;

            }
            break;

        case 1:             // name
            if (!next_is_param)
                return (false);

            if (!rc_only)
            {
                strncpy(you.your_name, next_arg, kNameLen);
                you.your_name[ kNameLen - 1 ] = '\0';
            }

            nextUsed = true;
            break;

        case 2:             // race
        case 3:             // class
            if (!next_is_param)
                return (false);

            // if (strlen(next_arg) != 1)
            //    return (false);

            if (!rc_only)
            {
                if (o == 2)
                    Options.race = str_to_race( std::string( next_arg ) );

                if (o == 3)
                    Options.cls = str_to_class( std::string( next_arg ) );
            }
            nextUsed = true;
            break;

        case 4:             // pizza
            if (!next_is_param)
                return (false);

            if (!rc_only)
                SysEnv.crawl_pizza = next_arg;

            nextUsed = true;
            break;

        case 5:             // plain
            if (next_is_param)
                return (false);

            if (!rc_only)
            {
                viewwindow = &viewwindow3;
                mapch = &mapchar3;
                mapch2 = &mapchar4;

#if defined(LINUX) && !defined(USE_X11)
                character_set = 0;
#endif
            }
            break;

        case 6:             // dir
            // ALWAYS PARSE
            if (!next_is_param)
                return (false);

            SysEnv.crawl_dir = next_arg;
            nextUsed = true;
            break;

        case 7:
            // ALWAYS PARSE
            if (!next_is_param)
                return (false);

            SysEnv.crawl_rc = next_arg;
            nextUsed = true;
            break;
        } // end switch -- which option?

        // update position
        current++;
        if (nextUsed)
            current++;
    }

    return (true);
}
