/*
 *  File:       chardump.cc
 *  Summary:    Dumps character info out to the morgue file.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *
 * <4> 19 June 2000    GDL Changed handles to FILE *
 * <3> 6/13/99         BWR Improved spell listing
 * <2> 5/30/99         JDJ dump_spells dumps failure rates (from Brent).
 * <1> 4/20/99         JDJ Reformatted, uses string objects, split out 7
 *                         functions from dump_char, dumps artifact info.
 */

#include "AppHdr.h"
#include "chardump.h"

#include <string>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <stdlib.h>
#if !(defined(__IBMCPP__) || defined(__BCPLUSPLUS__))
#include <unistd.h>
#endif
#include <ctype.h>

#ifdef USE_EMX
#include <sys/types.h>
#endif

#ifdef OS9
#include <stat.h>
#else
#include <sys/stat.h>
#endif

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "debug.h"
#include "describe.h"
#include "itemname.h"
#include "items.h"
#include "macro.h"
#include "mutation.h"
#include "output.h"
#include "player.h"
#include "religion.h"
#include "shopping.h"
#include "skills2.h"
#include "spl-book.h"
#include "spl-cast.h"
#include "spl-util.h"
#include "stash.h"
#include "stuff.h"
#include "version.h"
#include "view.h"

// Defined in view.cc
extern unsigned char (*mapch2) (unsigned char);


#ifdef JP
#ifdef LINUX
// VT100 ƒtƒHƒ“ƒg‚Ì“Áê•¶š‚ğg—p
static unsigned char ascii_to_zenkaku[2*256+1] ="\
@Ih¡“•fij–{C|D^\
‚O‚P‚Q‚R‚S‚T‚U‚V‚W‚XFGƒ„H\
—‚`‚a‚b‚c‚d‚e‚f‚g‚h‚i‚j‚k‚l‚m‚n\
‚o‚p‚q‚r‚s‚t‚u‚v‚w‚x‚ym_nOQ\
e‚‚‚‚ƒ‚„‚…‚†‚‡‚ˆ‚‰‚Š‚‹‚Œ‚‚‚\
‚‚‘‚’‚“‚”‚•‚–‚—‚˜‚™‚šobp`¡\
‰ñsEƒ®¿œ ßt¦¡¡¡›İñ";
#else /* LINUX */
static unsigned char ascii_to_zenkaku[2*256+1] ="\
@Ih¡“•fij–{C|D^\
‚O‚P‚Q‚R‚S‚T‚U‚V‚W‚XFGƒ„H\
—‚`‚a‚b‚c‚d‚e‚f‚g‚h‚i‚j‚k‚l‚m‚n\
‚o‚p‚q‚r‚s‚t‚u‚v‚w‚x‚ym_nOQ\
e‚‚‚‚ƒ‚„‚…‚†‚‡‚ˆ‚‰‚Š‚‹‚Œ‚‚‚\
‚‚‘‚’‚“‚”‚•‚–‚—‚˜‚™‚šobp`¡\
‰ñsEƒ®¿œ ßt¦¡¡¡›İñ";
#endif /* LINUX */
/*
 0 1 2 3 4 5 6 7 8 9 A B C D E F
128:‰ñ•Â‚¶‚½ƒhƒA 129:s“ÁêŠK’i 130:E°          131:ƒ®Õ’d
132:¿“üŒû       133:œ •¬…    134:  ŠJ‚¢‚½ƒhƒA 135:ß …
136:t“ÁêŠK’i   137:¦–I‚Ì‘ƒ   138:¡Î‚Ì•Ç      139:¡‹à‘®‚Ì•Ç
140:¡…»‚Ì•Ç   141:›İ —nŠâ    142:ñ€‘Ì
*/
#endif /* JP */


 // ========================================================================
 //      Internal Functions
 // ========================================================================

 // fillstring() is a hack to get around a missing constructor in
 // Borland C++ implementation of the STD basic_string.   Argh!!!
static std::string fillstring(size_t strlen, char filler)
{
    std::string s;

    for (size_t i=0; i<strlen; i++)
        s += filler;

    return s;
}

 //---------------------------------------------------------------
 //
 // munge_description
 //
 // Convert dollar signs to EOL and word wrap to 80 characters.
 // (for some obscure reason get_item_description uses dollar
 // signs instead of EOL).
 //  - It uses $ signs because they're easier to manipulate than the EOL
 //  macro, which is of uncertain length (well, that and I didn't know how
 //  to do it any better at the time) (LH)
 //---------------------------------------------------------------
std::string munge_description(const std::string & inStr)
{
    std::string modStr;
    std::string outStr;

    modStr = inStr;
    outStr.reserve(modStr.length() + 32);

    const long kIndent = 3;
    long lineLen = kIndent;

    long i = 0;

    outStr += fillstring(kIndent, ' ');

    while (modStr[modStr.length() - 1] == '$')    //!!!
        modStr.erase( (modStr.length() - 1), 1 );

    while (i < (long) modStr.length())
    {
        char ch = modStr[i];

        if (ch == '$')
        {
            if ( (i + 1) != (long)modStr.length() )
            {
                outStr += EOL;
                outStr += fillstring(kIndent, ' ');
                lineLen = kIndent;
            }

            while (modStr[++i] == '$')
            ;
        }
        else if (isspace(ch))
        {
            if (lineLen >= 79)
            {
                outStr += EOL;
                outStr += fillstring(kIndent, ' ');
                lineLen = kIndent;

            }
            else if (lineLen > 0)
            {
                outStr += ch;
                ++lineLen;
            }
            ++i;
        }
        else
        {
            std::string word;

            while (i < (long) modStr.length()
                   && lineLen + (long) word.length() < 79
                   && !isspace(modStr[i]) && modStr[i] != '$')
            {
                word += modStr[i++];
            }

            if (lineLen + word.length() >= 79)
            {
                outStr += EOL;
                outStr += fillstring(kIndent, ' ');
                lineLen = kIndent;
            }

            outStr += word;
            lineLen += word.length();
        }
    }

    outStr += EOL;

    return (outStr);
}                               // end munge_description()


 //---------------------------------------------------------------
 //
 // dump_screenshot
 //
 // Grabs a screenshot and appends the text into the given std::string,
 // using several ugly hacks in the process.
 //---------------------------------------------------------------
static void dump_screenshot( std::string &text )
{
    FixedVector < char, 1500 > buffy;  //[800]; //392];
    int bufcount = 0;
    unsigned short ch, color;
    int count_x, count_y;

    // A little message history:
    if (Options.dump_message_count > 0)
    {
#ifdef JP
        text += "ÅV‚ÌƒƒbƒZ[ƒW:";
#else
        text += "  Last messages:";
#endif
        text += EOL;
        text += get_last_messages(Options.dump_message_count);
        text += EOL;
    }
#ifdef JP
    text += "ü•Ó‚Ì—lq:";
#endif
    text += EOL;
    // Urg, ugly screen capture. CVS Crawl may have a better way of doing this,
    // but until the next release...
    for (count_y = (you.y_pos - 8); (count_y < you.y_pos + 9); count_y++)
    {
        bufcount += 8;
        for (count_x = (you.x_pos - 8); (count_x < you.x_pos + 9); count_x++)
        {
            if (count_x == you.x_pos && count_y == you.y_pos)
            {
                extern unsigned char your_sign;
                ch = your_sign;
            }
            else
            {
                unsigned int object = env.show[count_x - you.x_pos + 9]
                    [count_y - you.y_pos + 9];
                get_non_ibm_symbol(object, &ch, &color);
            }

            buffy[bufcount++] = (char) ch;

        }
        bufcount += 8;
    }

    int maxbuf = bufcount;
    bufcount = 0;

    for (count_y = 0; count_y < 17; count_y++)
    {
        for (count_x = 0; count_x < 33; count_x++)
        {
            if (count_x + you.x_pos - 17 < 3
                    || count_y + you.y_pos - 9 < 3
                    || count_x + you.x_pos - 14 > (GXM - 3)
                    || count_y + you.y_pos - 9 > (GYM - 3))
            {
                buffy[bufcount++] = ' ';
                continue;
            }

            if (count_x >= 8 && count_x <= 24 && count_y >= 0
                    && count_y <= 16 && buffy[bufcount] != 0)
            {
                bufcount++;
                continue;
            }

            unsigned char envc = env.map[count_x + you.x_pos - 17]
                       [count_y + you.y_pos - 9];
            if (envc)
            {
                // If it's printable, use it directly.
                if (envc < 127 && envc >= 32)
                    ch = envc;
                else {
                    // Otherwise get what's on the grid and get an ASCII
                    // character for that.
                    unsigned int object = grd[count_x + you.x_pos - 16]
                        [count_y + you.y_pos - 8];

                    // Special case secret doors so that monsters that open
                    // doors out of hero's LOS don't reveal the secret door in
                    // the dump
                    if (envc == mapch2(DNGN_SECRET_DOOR))
                        object = DNGN_SECRET_DOOR;
                    get_non_ibm_symbol(object, &ch, &color);
                }

                buffy[bufcount++] = (char) ch;

            }
            else
            {
                buffy[bufcount++] = ' ';
            }
        }
    }

    if (bufcount > maxbuf) maxbuf = bufcount;

    // 33 columns and a null terminator. More hardcoding. :-(
    char buf [34];
    char buf2[34];
    char buf3[68];
    char *s = buf;
    int  count;
    for (int i = 0; i < maxbuf; )
    {
        *s++ = buffy[i]? buffy[i] : ' ';

        ++i;
        if (!(i % 33) || i >= maxbuf)
        {
            *s = 0;
            while (s > buf && *--s == ' ')
                *s = 0;
            snprintf(buf2, 34, "%-33s\0", buf);
#ifdef JP
#ifdef USE_TILE
            if (Options.use_zenkaku || Options.use_tile)
#else
            if (Options.use_zenkaku)
#endif //USE_TILE
            {
                for(count = 0; count < 34; count++)
                {
                    if ( buf2[count] >= ' ' )
                    {
                        snprintf(&buf3[count * 2], 3, "%2s",
                                 &ascii_to_zenkaku[(buf2[count] - ' ') * 2]);
                    }
                    else
                    {
                        snprintf(&buf3[count * 2], 3, "  ");
                    }
                }
                buf3[66] = '\0';
                text += buf3;
            }
            else
            {
                text += buf2;
            }
#else
            text += buf2;
#endif //defined(JP)&&(defined(WIN32CONSOLE)||defined(WINDOWS))
            text += EOL;
            s = buf;
        }
    }
}

 //---------------------------------------------------------------
 //
 // dump_stats
 //
 //---------------------------------------------------------------
static void dump_stats( std::string & text )
{
    char st_prn[] = "";

#ifdef JP
    text += player_title();
    text += "w";
    text += you.your_name;
    text += "x";
#else
    text += you.your_name;
    text += " the ";
    text += player_title();
    text += " (";
    text += species_name(you.species, you.experience_level);
    text += ")";
#endif
    text += EOL;

#ifdef JP
    text += "ƒŒƒxƒ‹ ";
    itoa(you.experience_level, st_prn, 10);
    text += st_prn;
    text += "  ";
    text += species_name(you.species, you.experience_level);
    text += "  ";
    text += you.class_name;
    text += EOL EOL;
#else
    text += "(Level ";
    itoa(you.experience_level, st_prn, 10);
    text += st_prn;
    text += " ";
    text += you.class_name;
    text += ")";
    text += EOL EOL;
#endif

#ifdef JP
    text += "‚g‚o ";
#else
    text += "HP   ";
#endif
    snprintf(st_prn, 10, "%3d", you.hp);
    text += st_prn;

    int max_max_hp = you.hp_max + player_rotted();

    text += "/";
    snprintf(st_prn, 10, "%3d", you.hp_max);
    text += st_prn;

    if (max_max_hp != you.hp_max)
    {
        text += "(";
        snprintf(st_prn, 10, "%3d", max_max_hp);
        text += st_prn;
        text += ")";
#ifdef JP
#else
        if (you.hp < 1)
        {
            text += " ";
            text += ((!you.deaths_door) ? "(dead)" : "(almost dead)");
        }
#endif
    }
    text += EOL;

#ifdef JP
    text += "‚l‚o ";
#else
    text += "MP   ";
#endif
    snprintf(st_prn, 10, "%3d", you.magic_points);
    text += st_prn;
    text += "/";
    snprintf(st_prn, 10, "%3d", you.max_magic_points);
    text += st_prn;
    text += EOL;

#ifdef JP
    text += "˜r—Í ";
#else
    text += "Str  ";
#endif
    snprintf(st_prn, 10, "%2d", you.strength);
    text += st_prn;
    if (you.strength < you.max_strength)
    {
        text += "/";
        snprintf(st_prn, 10, "%2d", you.max_strength);
        text += st_prn;
    }
    else
    {
    text += "   ";
    }

#ifdef JP
    text += "  Ší—p ";
#else
    text += "  Dex  ";
#endif
    snprintf(st_prn, 10, "%2d", you.dex);
    text += st_prn;
    if (you.dex < you.max_dex)
    {
        text += "/";
        snprintf(st_prn, 10, "%2d", you.max_dex);
        text += st_prn;
    }
    else
    {
    text += "   ";
    }

#ifdef JP
    text += "  ’m—Í ";
#else
    text += "  Int  ";
#endif
    snprintf(st_prn, 10, "%2d", you.intel);
    text += st_prn;
    if (you.intel < you.max_intel)
    {
        text += "/";
        snprintf(st_prn, 10, "%2d", you.max_intel);
        text += st_prn;
    }
    else
    {
    //text += "   ";
    }
    text += EOL;

#ifdef JP
    text += "‚`‚b ";
#else
    text += "AC   ";
#endif
    snprintf(st_prn, 10, "%2d", player_AC() );
    text += st_prn;

#ifdef JP
    text += "     ‰ñ”ğ ";
#else
    text += "     EV   ";
#endif
    snprintf(st_prn, 10, "%2d", player_evasion() );
    text += st_prn;

#ifdef JP
    text += "     ‚   ";
#else
    text += "     SH   ";
#endif
    snprintf(st_prn, 10, "%2d", player_shield_class() );
    text += st_prn;
    text += EOL EOL;
#ifdef JP
    text += "ŒoŒ±’l ";
    itoa(you.experience_level, st_prn, 10);
    text += st_prn;
    text += "/";
    itoa(you.experience, st_prn, 10);
    text += st_prn;
    text += EOL;
    text += "Š‹à ";
    itoa( you.gold, st_prn, 10 );
    text += st_prn;
#else
    text += "Gold : ";
    itoa( you.gold, st_prn, 10 );
    text += st_prn;
    text += EOL;

    text += "Experience : ";
    itoa(you.experience_level, st_prn, 10);
    text += st_prn;
    text += "/";
    itoa(you.experience, st_prn, 10);
    text += st_prn;
#endif
    text += EOL;

    if (you.real_time != -1)
    {
        const time_t curr = you.real_time + (time(NULL) - you.start_time);
        char buff[200];

        make_time_string( curr, buff, sizeof(buff) );

#ifdef JP
        text += "ƒvƒŒƒCŠÔ ";
#else
        text += "Play time: ";
#endif
        text += buff;

#ifdef JP
        text += "(Œo‰ßƒ^[ƒ“”";
        itoa( you.num_turns, st_prn, 10 );
        text += st_prn;
        text += ")";
#else
        text += "(Number of turns: ";
        itoa( you.num_turns, st_prn, 10 );
        text += st_prn;
        text += ")";
#endif
        text += EOL EOL;
    }

#ifdef JP
        if (you.hp < 1)
        {
            text += ((!you.deaths_door) ? "‚ ‚È‚½‚Í€‚ñ‚Å‚¢‚éB" : "‚ ‚È‚½‚Í€‚Ì‚Æ‚ÎŒû‚É—Õ‚ñ‚Å‚¢‚éB");
            text += EOL;
        }
#endif
}                               // end dump_stats()

 //---------------------------------------------------------------
 //
 // dump_stats2
 //
 //---------------------------------------------------------------
static void dump_stats2( std::string & text, bool calc_unid)
{
    char buffer[25*3][45];
    char str_pass[80];
    char* ptr_n;
    int i;
    int dump_x = 1;
    int dump_y = 1;

    get_full_detail(&buffer[0][0], calc_unid);

    for(i=0; i<25; i++)
    {
        //if (buffer[i][0] != '\0')
        ptr_n = &buffer[i+0][0];
        if (buffer[i+25][0] == '\0' && buffer[i+50][0] == '\0')
            snprintf(&str_pass[0], 45, "%s", ptr_n);
        else
            snprintf(&str_pass[0], 45, "%-28s", ptr_n);
        text += str_pass;

        ptr_n = &buffer[i+25][0];
        if (buffer[i+50][0] == '\0')
            snprintf(&str_pass[0], 45, "%s", ptr_n);
        else
            snprintf(&str_pass[0], 45, "%-20s", ptr_n);
        text += str_pass;

        ptr_n = &buffer[i+50][0];
        if (buffer[i+50][0] != '\0')
        {
            snprintf(&str_pass[0], 45, "%s", ptr_n);
            text += str_pass;
        }
        //text += &buffer[i+25][0];
        //text += &buffer[i+48][0];
        text += EOL;
    }

    text += EOL EOL;
#ifdef JP
    if (you.hp < 1)
    {
        text += ((!you.deaths_door) ? "‚ ‚È‚½‚Í€‚ñ‚Å‚¢‚éB" : "‚ ‚È‚½‚Í€‚Ì‚Æ‚ÎŒû‚É—Õ‚ñ‚Å‚¢‚éB");
        text += EOL;
    }
#endif
}
 //---------------------------------------------------------------
 //
 // dump_location
 //
 //---------------------------------------------------------------
static void dump_location( std::string & text )
{
    if (you.level_type != LEVEL_DUNGEON || you.your_level != -1)
#ifdef JP
        text += "‚ ‚È‚½‚Í";
#else
        text += "You are ";
#endif

    if (you.level_type == LEVEL_PANDEMONIUM)
#ifdef JP
        text += "ƒpƒ“ƒfƒ‚ƒjƒEƒ€‚É‚¢‚é";
#else
        text += "in Pandemonium";
#endif
    else if (you.level_type == LEVEL_ABYSS)
#ifdef JP
        text += "ƒAƒrƒX‚É‚¢‚é";
#else
        text += "in the Abyss";
#endif
    else if (you.level_type == LEVEL_LABYRINTH)
#ifdef JP
        text += "ƒ‰ƒrƒŠƒ“ƒX‚É‚¢‚é";
#else
        text += "in a labyrinth";
#endif
    else if (you.where_are_you == BRANCH_DIS)
#ifdef JP
        text += "ƒfƒB[ƒX‚É‚¢‚é";
#else
        text += "in Dis";
#endif
    else if (you.where_are_you == BRANCH_GEHENNA)
#ifdef JP
        text += "ƒQƒwƒi‚É‚¢‚é";
#else
        text += "in Gehenna";
#endif
    else if (you.where_are_you == BRANCH_VESTIBULE_OF_HELL)
#ifdef JP
        text += "’n–‚Ì“ü‚èŒû‚É‚¢‚é";
#else
        text += "in the Vestibule of Hell";
#endif
    else if (you.where_are_you == BRANCH_COCYTUS)
#ifdef JP
        text += "ƒRƒLƒ…[ƒgƒX‚É‚¢‚é";
#else
        text += "in Cocytus";
#endif
    else if (you.where_are_you == BRANCH_TARTARUS)
#ifdef JP
        text += "ƒ^ƒ‹ƒ^ƒƒX‚É‚¢‚é";
#else
        text += "in Tartarus";
#endif
    else if (you.where_are_you == BRANCH_INFERNO)
#ifdef JP
        text += "ƒCƒ“ƒtƒFƒ‹ƒm‚É‚¢‚é";
#else
        text += "in the Inferno";
#endif
    else if (you.where_are_you == BRANCH_THE_PIT)
#ifdef JP
        text += "â{‚É‚¢‚é";
#else
        text += "in the Pit";
#endif
    else if (you.where_are_you == BRANCH_ORCISH_MINES)
#ifdef JP
        text += "ƒI[ƒN‚ÌB“¹‚É‚¢‚é";
#else
        text += "in the Mines";
#endif
    else if (you.where_are_you == BRANCH_HIVE)
#ifdef JP
        text += "–I‚Ì‘ƒ‚É‚¢‚é";
#else
        text += "in the Hive";
#endif
    else if (you.where_are_you == BRANCH_LAIR)
#ifdef JP
        text += "b‚Ì±‚İˆ‚É‚¢‚é";
#else
        text += "in the Lair";
#endif
    else if (you.where_are_you == BRANCH_SLIME_PITS)
#ifdef JP
        text += "ƒXƒ‰ƒCƒ€‚ÌŒŠ‚®‚ç‚É‚¢‚é";
#else
        text += "in the Slime Pits";
#endif
    else if (you.where_are_you == BRANCH_VAULTS)
#ifdef JP
        text += "•ó•¨ŒÉ‚É‚¢‚é";
#else
        text += "in the Vaults";
#endif
    else if (you.where_are_you == BRANCH_CRYPT)
#ifdef JP
        text += "’n‰º•æ’n‚É‚¢‚é";
#else
        text += "in the Crypt";
#endif
    else if (you.where_are_you == BRANCH_HALL_OF_BLADES)
#ifdef JP
        text += "n‚ÌLŠÔ‚É‚¢‚é";
#else
        text += "in the Hall of Blades";
#endif
    else if (you.where_are_you == BRANCH_HALL_OF_ZOT)
#ifdef JP
        text += "ƒ]ƒbƒg‚Ì—Ìˆæ‚É‚¢‚é";
#else
        text += "in the Hall of Zot";
#endif
    else if (you.where_are_you == BRANCH_ECUMENICAL_TEMPLE)
#ifdef JP
        text += "”@”h‚Ì›‰@‚É‚¢‚é";
#else
        text += "in the Ecumenical Temple";
#endif
    else if (you.where_are_you == BRANCH_SNAKE_PIT)
#ifdef JP
        text += "ÖŒŠ‚É‚¢‚é";
#else
        text += "in the Snake Pit";
#endif
    else if (you.where_are_you == BRANCH_ELVEN_HALLS)
#ifdef JP
        text += "ƒGƒ‹ƒt‚Ì‘åLŠÔ‚É‚¢‚é";
#else
        text += "in the Elven Halls";
#endif
    else if (you.where_are_you == BRANCH_TOMB)
#ifdef JP
        text += "—ì•_‚É‚¢‚é";
#else
        text += "in the Tomb";
#endif
    else if (you.where_are_you == BRANCH_SWAMP)
#ifdef JP
        text += "À‚É‚¢‚é";
#else
        text += "in the Swamp";
#endif
    else
    {
        if (you.your_level == -1)
#ifdef JP
            text += "‚ ‚È‚½‚Íƒ_ƒ“ƒWƒ‡ƒ“‚©‚ç’Eo‚µ‚½";
#else
            text += "You escaped";
#endif
        else
        {
#ifdef JP
            text += "’n‰º";
#else
            text += "on level ";
#endif

            char st_prn[20];
            itoa(you.your_level + 1, st_prn, 10);
            text += st_prn;
#ifdef JP
            text += "ŠK‚É‚¢‚é";
#endif
        }
    }

#ifdef JP
    text += "B";
#else
    text += ".";
#endif
    text += EOL;
}                               // end dump_location()

 //---------------------------------------------------------------
 //
 // dump_religion
 //
 //---------------------------------------------------------------
static void dump_religion( std::string & text )
{
    if (you.religion != GOD_NO_GOD)
    {
#ifdef JP
        text += "‚ ‚È‚½‚Í";
#else
        text += "You worship ";
#endif
        text += god_name(you.religion);
#ifdef JP
        text += "‚ğM‹Â‚µ‚Ä‚¢‚éB";
#else
        text += ".";
#endif
        text += EOL;

        if (!player_under_penance())
        {
            if (you.religion != GOD_XOM)
            {                   // Xom doesn't care
                text += god_name(you.religion);
#ifdef JP
                text += "‚Í";
                text += ((you.piety <= 5) ? "‚ ‚È‚½‚É• ‚ğ—§‚Ä‚Ä‚¢‚é" :
                         (you.piety <= 20) ? "‚ ‚È‚½‚É“Á•Ê‚ÈŠÖS‚ğ‚Á‚Ä‚¢‚È‚¢" :
                         (you.piety <= 40) ? "‚ ‚È‚½‚É–‘«‚µ‚Ä‚¢‚é" :
                         (you.piety <= 70) ? "‚ ‚È‚½‚É‚©‚È‚è–‘«‚µ‚Ä‚¢‚é" :
                         (you.piety <= 100) ? "‚ ‚È‚½‚É‘å‚¢‚É–‘«‚µ‚Ä‚¢‚é" :
                         (you.piety <= 130) ? "‚ ‚È‚½‚É‹É‚ß‚Ä–‘«‚µ‚Ä‚¢‚é"
                         : "‚ ‚È‚½‚Ì’”q‚ğŒÖ‚è‚É‚µ‚Ä‚¢‚é");
                text += "B";
#else
                text += " is ";
                text += ((you.piety <= 5) ? "displeased" :
                         (you.piety <= 20) ? "noncommittal" :
                         (you.piety <= 40) ? "pleased with you" :
                         (you.piety <= 70) ? "most pleased with you" :
                         (you.piety <= 100) ? "greatly pleased with you" :
                         (you.piety <= 130) ? "extremely pleased with you"
                         : "exalted by your worship");
                text += ".";
#endif
                text += EOL;
            }
        }
        else
        {
            text += god_name(you.religion);
#ifdef JP
            text += "‚Í‚ ‚È‚½‚Éœğ‰÷‚ğ–½‚¶‚Ä‚¢‚éB";
#else
            text += " is demanding penance.";
#endif
            text += EOL;
        }
    }
}                               // end dump_religion()

 //---------------------------------------------------------------
 //
 // dump_inventory
 //
 //---------------------------------------------------------------
static void dump_inventory( std::string & text, bool show_prices )
{
    int i, j;
    char temp_id[4][50];

    std::string text2;

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 50; j++)
        {
            temp_id[i][j] = 1;
        }
    }

    char st_pass[ ITEMNAME_SIZE ] = "";
    int inv_class2[OBJ_GOLD];
    int inv_count = 0;
    char tmp_quant[20];

    for (i = 0; i < OBJ_GOLD; i++)
    {
        inv_class2[i] = 0;
    }

    for (i = 0; i < ENDOFPACK; i++)
    {
        if (is_valid_item( you.inv[i] ))
        {
            // adds up number of each class in invent.
            inv_class2[you.inv[i].base_type]++;
            inv_count++;
        }
    }

    if (!inv_count)
    {
#ifdef JP
        text += "‚ ‚È‚½‚ÍƒAƒCƒeƒ€‚ğ‰½‚à‚Á‚Ä‚¢‚È‚¢B";
#else
        text += "You aren't carrying anything.";
#endif
        text += EOL;
    }
    else
    {
#ifdef JP
        text += "Š•iˆê——:";
#else
        text += "  Inventory:";
#endif
        text += EOL;

        for (i = 0; i < OBJ_GOLD; i++)
        {
            if (inv_class2[i] != 0)
            {
                switch (i)
                {
#ifdef JP
                case OBJ_WEAPONS:    text += "[‹ßÚ•Ší]";        break;
                case OBJ_MISSILES:   text += "[”ò‚Ñ“¹‹ï]";        break;
                case OBJ_ARMOUR:     text += "[–h‹ï]";            break;
                case OBJ_WANDS:      text += "[–‚–@‚Ì“¹‹ï]";      break;
                case OBJ_FOOD:       text += "[H—¿•i]";          break;
                case OBJ_SCROLLS:    text += "[Šª•¨]";            break;
                case OBJ_JEWELLERY:  text += "[‘•g‹ï]";          break;
                case OBJ_POTIONS:    text += "[–ò]";              break;
                case OBJ_BOOKS:      text += "[‘•¨]";            break;
                case OBJ_STAVES:     text += "[–‚–@‚Ìñ]";        break;
                case OBJ_ORBS:       text += "[—Í‚Ì•óì]";        break;
                case OBJ_MISCELLANY: text += "[‚»‚Ì‘¼]";          break;
                case OBJ_CORPSES:    text += "[€‘Ì]";            break;
#else
                case OBJ_WEAPONS:    text += "Hand weapons";    break;
                case OBJ_MISSILES:   text += "Missiles";        break;
                case OBJ_ARMOUR:     text += "Armour";          break;
                case OBJ_WANDS:      text += "Magical devices"; break;
                case OBJ_FOOD:       text += "Comestibles";     break;
                case OBJ_SCROLLS:    text += "Scrolls";         break;
                case OBJ_JEWELLERY:  text += "Jewellery";       break;
                case OBJ_POTIONS:    text += "Potions";         break;
                case OBJ_BOOKS:      text += "Books";           break;
                case OBJ_STAVES:     text += "Magical staves";  break;
                case OBJ_ORBS:       text += "Orbs of Power";   break;
                case OBJ_MISCELLANY: text += "Miscellaneous";   break;
                case OBJ_CORPSES:    text += "Carrion";         break;
#endif

                default:
#ifdef JP
                    DEBUGSTR("Bad item class");
#else
                    DEBUGSTR("Bad item class");
#endif
                }
                text += EOL;

                for (j = 0; j < ENDOFPACK; j++)
                {
                    if (is_valid_item(you.inv[j]) && you.inv[j].base_type == i)
                    {
                        bool iseq = false;
                        for(int k = EQ_WEAPON; k < NUM_EQUIP; k++)
                        {
                            if (you.inv[j].link == you.equip[k])
                                iseq = true;
                        }
                        if (iseq)
                            text += "+";
                        else
                            text += " ";

                        in_name( j, DESC_INVENTORY_EQUIP, st_pass );
                        text += st_pass;

                        inv_count--;

                        if (show_prices)
                        {
                            text += " (";

                            itoa( item_value( you.inv[j], temp_id, true ),
                                  tmp_quant, 10 );

                            text += tmp_quant;
#ifdef JP
                            text += " ƒS[ƒ‹ƒh)";
#else
                            text += " gold)";
#endif
                        }

                        if (is_dumpable_artifact( you.inv[j],
                                                  Options.verbose_dump ))
                        {
                            text2 = get_item_description( you.inv[j],
                                                          Options.verbose_dump,
                                                          true );
#ifdef JP
                        std::string text3;
                        text3 += munge_description(text2);
                        for (int i = text3.length() - 2; i >= 0; --i)
                            if (text3[i] == '\n')
                                text3.insert(i + 1, "  ");
                            text += text3;
#else
                            text += munge_description(text2);
#endif

                        }
                        else
                        {
                            text += EOL;
                        }
                    }
                }
            }
        }
    }
}                               // end dump_inventory()

//---------------------------------------------------------------
//
// dump_skills
//
//---------------------------------------------------------------
static void dump_skills( std::string & text )
{
    char tmp_quant[20];

    text += EOL;
    text += EOL;
#ifdef JP
    text += "ƒXƒLƒ‹:";
#else
    text += "   Skills:";
#endif
    text += EOL;

    for (unsigned char i = 0; i < 50; i++)
    {
        if (you.skills[i] > 0)
        {
            text += ( (you.skills[i] == 27)   ? " * " :
                      (you.practise_skill[i]) ? " + "
                                              : " - " );

#ifdef JP
            text += "LV ";
#else
            text += "Level ";
#endif
            snprintf(tmp_quant, 10, "%2d", you.skills[i]);
            text += tmp_quant;
            text += " ";
            text += skill_name(i);
            text += EOL;
        }
    }

    text += EOL;
    text += EOL;
}                               // end dump_skills()

//---------------------------------------------------------------
//
// Return string of the i-th spell type, with slash if required
//
//---------------------------------------------------------------
static std::string spell_type_name(int spell_class, bool slash)
{
    std::string ret;

    if (slash)
        ret = "/";

    ret += spelltype_name(spell_class);

    return (ret);
}                               // end spell_type_name()

//---------------------------------------------------------------
//
// dump_spells
//
//---------------------------------------------------------------
static void dump_spells( std::string & text )
{
    char tmp_quant[20];

// This array helps output the spell types in the traditional order.
// this can be tossed as soon as I reorder the enum to the traditional order {dlb}
    const int spell_type_index[] = {
        SPTYP_HOLY,
        SPTYP_POISON,
        SPTYP_FIRE,
        SPTYP_ICE,
        SPTYP_EARTH,
        SPTYP_AIR,
        SPTYP_CONJURATION,
        SPTYP_ENCHANTMENT,
        SPTYP_DIVINATION,
        SPTYP_TRANSLOCATION,
        SPTYP_SUMMONING,
        SPTYP_TRANSMIGRATION,
        SPTYP_NECROMANCY,
        0
    };

    int spell_levels = player_spell_levels();
    if (spell_levels == 1)
#ifdef JP
        text += "‚ ‚È‚½‚Í1ô•¶ƒŒƒxƒ‹‚ğc‚µ‚Ä‚¢‚éB";
#else
        text += "You have one spell level left.";
#endif
    else if (spell_levels == 0)
#ifdef JP
        text += "‚ ‚È‚½‚ÍV‚µ‚¢ô•¶‚ğŠo‚¦‚é‚±‚Æ‚ª‚Å‚«‚È‚¢B";
#else
        text += "You cannot memorise any spells.";
#endif
    else
    {
#ifdef JP
        text += "‚ ‚È‚½‚Í";
#else
        text += "You have ";
#endif
        itoa( spell_levels, tmp_quant, 10 );
        text += tmp_quant;
#ifdef JP
        text += "ô•¶ƒŒƒxƒ‹‚ğc‚µ‚Ä‚¢‚éB";
#else
        text += " spell levels left.";
#endif
    }

    text += EOL;

    if (!you.spell_no)
    {
#ifdef JP
        text += "‚ ‚È‚½‚Íô•¶‚ğˆêØ’m‚ç‚È‚¢B";
#else
        text += "You don't know any spells.";
#endif
        text += EOL;

    }
    else
    {
#ifdef JP
        text += "‚ ‚È‚½‚ÍˆÈ‰º‚Ìô•¶‚ğ‹L‰¯‚µ‚Ä‚¢‚é:" EOL;
#else
        text += "You know the following spells:" EOL;
#endif
        text += EOL;

#ifdef JP
        text += "ô•¶                              Œn“                    ¬Œ÷—¦     LV" EOL;

#else
        text += "  Your Spells                       Type                  Success   Level" EOL;

#endif
        for (int j = 0; j < 52; j++)
        {
            const char letter = index_to_letter( j );
            const int  spell  = get_spell_by_letter( letter );

            if (spell != SPELL_NO_SPELL)
            {
                std::string spell_line = " ";

                char strng[2];
                strng[0] = letter;
                strng[1] = '\0';

                spell_line += strng;
                spell_line += " - ";
                spell_line += spell_title( spell );

                for (int i = spell_line.length(); i < 34; i++)
                {
                    spell_line += ' ';
                }

                bool already = false;

                for (int i = 0; spell_type_index[i] != 0; i++)
                {
                    if (spell_typematch( spell, spell_type_index[i] ))
                    {
                        spell_line +=
                                spell_type_name(spell_type_index[i], already);
                        already = true;
                    }
                }

                for (int i = spell_line.length(); i < 58; i++)
                {
                    spell_line += ' ';
                }

                int fail_rate = spell_fail( spell );

#ifdef JP
                spell_line += (fail_rate == 100) ? "•s‰Â”\"    :
                              (fail_rate >   90) ? "‹É‚ß‚Äˆ«‚¢":
                              (fail_rate >   80) ? "‚©‚È‚èˆ«‚¢":
                              (fail_rate >   70) ? "ˆ«‚¢"      :
                              (fail_rate >   60) ? "—ò‚é"      :
                              (fail_rate >   50) ? "‚â‚â—ò‚é"  :
                              (fail_rate >   40) ? "‚â‚â—Ç‚¢"  :
                              (fail_rate >   30) ? "—Ç‚¢"      :
                              (fail_rate >   20) ? "‚Æ‚Ä‚à—Ç‚¢":
                              (fail_rate >   10) ? "‘f°‚ç‚µ‚¢":
                              (fail_rate >   0)  ? "‘ì‰z"
                                                 : "Š®àø" ;
#else
                spell_line += (fail_rate == 100) ? "Useless"   :
                              (fail_rate >   90) ? "Terrible"  :
                              (fail_rate >   80) ? "Cruddy"    :
                              (fail_rate >   70) ? "Bad"       :
                              (fail_rate >   60) ? "Very Poor" :
                              (fail_rate >   50) ? "Poor"      :
                              (fail_rate >   40) ? "Fair"      :
                              (fail_rate >   30) ? "Good"      :
                              (fail_rate >   20) ? "Very Good" :
                              (fail_rate >   10) ? "Great"     :
                              (fail_rate >   0)  ? "Excellent"
                                                 : "Perfect";
#endif

                for (int i = spell_line.length(); i < 70; i++)
                    spell_line += ' ';

                itoa((int) spell_difficulty( spell ), tmp_quant, 10 );
                spell_line += tmp_quant;
                spell_line += EOL;

                text += spell_line;
            }
        }
    }
}                               // end dump_spells()


//---------------------------------------------------------------
//
// dump_kills
//
//---------------------------------------------------------------
static void dump_kills( std::string & text )
{
    text += you.kills.kill_info();
    return;
}

//---------------------------------------------------------------
//
// dump_mutations
//
//---------------------------------------------------------------
static void dump_mutations( std::string & text )
{
    // Can't use how_mutated() here, as it doesn't count demonic powers
    int xz = 0;

    for (int xy = 0; xy < 100; xy++)
    {
        if (you.mutation[xy] > 0)
            xz++;
    }

    if (xz > 0)
    {
        text += "";
        text += EOL;
        text += EOL;
#ifdef JP
        text += "“Ë‘R•ÏˆÙ/’´©‘R‚Ì”\—Í";
#else
        text += "           Mutations & Other Weirdness";
#endif
        text += EOL;

        for (int j = 0; j < 100; j++)
        {
            if (you.mutation[j])
            {
                if (you.demon_pow[j] > 0)
                    text += "* ";
                else
                    text += "  ";
                text += mutation_name(j);
                text += EOL;
            }
        }
    }
}                               // end dump_mutations()

#if MAC
#pragma mark -
#endif


// ========================================================================
//      Public Functions
// ========================================================================

//---------------------------------------------------------------
//
// dump_char
//
// Creates a disk record of a character. Returns true if the
// character was successfully saved.
//
//---------------------------------------------------------------
bool dump_char( const char fname[30], bool show_prices )  // $$$ a try block?
{
    bool succeeded = false;

    std::string text;

    // start with enough room for 100 80 character lines
    text.reserve(200 * 80);

    text += "Dungeon Crawl " VERSION;
#ifdef V_FIX
    text += "f";
#endif

#ifdef USE_TILE
#ifdef WINDOWS
    text += "(Windows/";
#endif
#ifdef USE_X11
    text += "(X11/";
#endif
    if (Options.use_tile)
    {
        text += "Tile";
        if (Options.use_qv_mode)
#ifdef JP
            text +="_QV";
#else
            text +="_Iso";
#endif
    }
    else
    {
        text += "Text";
    }
        text += ")";
#else
#ifdef WIN32CONSOLE
    text += "(Win32Console)";
#endif
#endif

#ifdef JP
    text += " ƒLƒƒƒ‰ƒNƒ^[î•ñ";
    text += EOL;
    text += "[Build " BUILD_DATE "]";
    text += EOL;
    text += EOL;
#else
    text += " character file.";
    text += EOL;
    text += "[Last build " BUILD_DATE "]";
    text += EOL;
    text += EOL;
#endif

    //dump_stats(text);

    if (strcmp(fname,"morgue.txt")==0)
        dump_stats2(text, true);
    else
        dump_stats2(text, false);
    dump_location(text);
    dump_religion(text);

    switch (you.burden_state)
    {
    case BS_OVERLOADED:
#ifdef JP
        text += "‚ ‚È‚½‚Í‰×•¨‚Ìd‚³‚Å‰Ÿ‚µ’×‚³‚ê‚»‚¤‚¾B";
#else
        text += "You are overloaded with stuff.";
#endif
        text += EOL;
        break;
    case BS_ENCUMBERED:
#ifdef JP
        text += "‚ ‚È‚½‚Í‰×•¨‚Ìd‚³‚ª•‰’S‚É‚È‚Á‚Ä‚¢‚éB";
#else
        text += "You are encumbered.";
#endif
        text += EOL;
        break;
    }

#ifdef JP
    text += "‚ ‚È‚½‚Í";
#else
    text += "You are ";
#endif

#ifdef JP
    text += ((you.hunger <= 1000) ? "‹Q‰ìó‘Ô‚¾" :
             (you.hunger <= 2600) ? "‹ó• ‚¾" :
             (you.hunger < 7000) ? "‹ó• ‚Å‚Í‚È‚¢" :
             (you.hunger < 11000) ? "–• ‚¾" : "Š®‘S‚É–• ‚¾");
#else
    text += ((you.hunger <= 1000) ? "starving" :
             (you.hunger <= 2600) ? "hungry" :
             (you.hunger < 7000) ? "not hungry" :
             (you.hunger < 11000) ? "full" : "completely stuffed");
#endif

#ifdef JP
    text += "B";
#else
    text += ".";
#endif
    text += EOL;
    text += EOL;

    if (you.attribute[ATTR_TRANSFORMATION])
    {
        switch (you.attribute[ATTR_TRANSFORMATION])
        {
        case TRAN_SPIDER:
#ifdef JP
            text += "‚ ‚È‚½‚Í’wå‚ÌŒ`‘Ô‚ğæ‚Á‚Ä‚¢‚éB";
#else
            text += "You are in spider-form.";
#endif
            break;
        case TRAN_BLADE_HANDS:
#ifdef JP
            text += "‚ ‚È‚½‚Íè‚ğn‚É•Ï‰»‚³‚¹‚Ä‚¢‚éB";
#else
            text += "Your hands are blades.";
#endif
            break;
        case TRAN_STATUE:
#ifdef JP
            text += "‚ ‚È‚½‚ÍÎ‘œ‚É•Ïg‚µ‚Ä‚¢‚éB";
#else
            text += "You are a stone statue.";
#endif
            break;
        case TRAN_ICE_BEAST:
#ifdef JP
            text += "‚ ‚È‚½‚Í•X‚Ìƒ‚ƒ“ƒXƒ^[‚É•Ïg‚µ‚Ä‚¢‚éB";
#else
            text += "You are a creature of crystalline ice.";
#endif
            break;
        case TRAN_DRAGON:
#ifdef JP
            text += "‚ ‚È‚½‚Í‹°‚é‚×‚«ƒhƒ‰ƒSƒ“‚É•Ïg‚µ‚Ä‚¢‚éB";
#else
            text += "You are a fearsome dragon!";
#endif
            break;
        case TRAN_LICH:
#ifdef JP
            text += "‚ ‚È‚½‚ÍƒŠƒbƒ`‚ÌŒ`‘Ô‚ğæ‚Á‚Ä‚¢‚éB";
#else
            text += "You are in lich-form.";
#endif
            break;
        case TRAN_SERPENT_OF_HELL:
#ifdef JP
            text += "‚ ‚È‚½‚Í‹‘å‚Åˆ«–‚“I‚È‘åÖ‚É•Ïg‚µ‚Ä‚¢‚éB";
#else
            text += "You are a huge, demonic serpent!";
#endif
            break;
        case TRAN_AIR:
#ifdef JP
            text += "‚ ‚È‚½‚Í‹C‘Ì‚É•Ïg‚µ‚Ä‚¢‚éB";
#else
            text += "You are a cloud of diffuse gas.";
#endif
            break;
        }

        text += EOL;
        text += EOL;
    }
    text += EOL;

    dump_inventory(text, show_prices);

    char tmp_quant[20];

    text += EOL;
    text += EOL;
#ifdef JP
    text += "‚ ‚È‚½‚Í";
#else
    text += " You have ";
#endif
    itoa( you.exp_available, tmp_quant, 10 );
    text += tmp_quant;
#ifdef JP
    text += "ƒ|ƒCƒ“ƒg‚Ì‹Z”\ŒoŒ±’l‚ğ–¢g—p‚¾B";
#else
    text += " experience left.";
#endif

    dump_skills(text);
    dump_spells(text);
    dump_mutations(text);

    text += EOL;
    text += EOL;

    dump_screenshot(text);

    text += EOL;
    text += EOL;

    dump_kills(text);

    text += EOL;
    text += EOL;

    char file_name[kPathLen] = "\0";

    if (SysEnv.crawl_dir)
        strncpy(file_name, SysEnv.crawl_dir, kPathLen);

    strncat(file_name, fname, kPathLen);

#ifdef STASH_TRACKING
    char stash_file_name[kPathLen] = "";

    strncpy(stash_file_name, file_name, kPathLen);
    if (strcmp(fname, "morgue.txt") != 0)
    {
        strncat(file_name, ".txt", kPathLen);
        strncat(stash_file_name, ".lst", kPathLen);
        stashes.dump(stash_file_name);
    }
    else
    {
        strncpy(stash_file_name, "morgue.lst", kPathLen);
        stashes.dump(stash_file_name);
    }//morgue.lst
#endif

    FILE *handle = fopen(file_name, "wb");

#if DEBUG_DIAGNOSTICS
#ifdef JP
    strcpy( info, "ƒtƒ@ƒCƒ‹ƒl[ƒ€: " );
#else
    strcpy( info, "File name: " );
#endif
    strcat( info, file_name );
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    if (handle != NULL)
    {
        size_t begin = 0;
        size_t end = text.find(EOL);

        while (end != std::string::npos)
        {
            end += strlen(EOL);

            size_t len = end - begin;

            if (len > 80)
                len = 80;

            fwrite(text.c_str() + begin, len, 1, handle);

            begin = end;
            end = text.find(EOL, end);
        }

        fclose(handle);
        succeeded = true;
    }
    else
#ifdef JP
        mpr("Error opening file.");
#else
        mpr("Error opening file.");
#endif

    return (succeeded);
}                               // end dump_char()
