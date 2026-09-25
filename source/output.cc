/*
 *  File:       output.cc
 *  Summary:    Functions used to print player related info.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <2>      5/20/99        BWR             Efficiency changes for curses.
 *      <1>      -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "output.h"

#include <stdlib.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "itemname.h"
#include "ouch.h"
#include "player.h"
#include "skills2.h"
#include "religion.h"
#include "stuff.h"
#include "fight.h"

static int bad_ench_colour( int lvl, int orange, int red )
{
    if (lvl > red)
        return (RED);
    else if (lvl > orange)
        return (LIGHTRED);

    return (YELLOW);
}

static void dur_colour( int colour, bool running_out )
{
    if (running_out)
        textcolor( colour );
    else
    {
        switch (colour)
        {
        case GREEN:     textcolor( LIGHTGREEN );        break;
        case BLUE:      textcolor( LIGHTBLUE );         break;
        case MAGENTA:   textcolor( LIGHTMAGENTA );      break;
        case LIGHTGREY: textcolor( WHITE );             break;
        }
    }
}

#ifdef USE_TILE // HP BAR
void draw_hp_bar(int hp, int maxhp){
#define HPBAR_OX 60
#define HPBAR_OY 3
#define HPBAR_WIDTH (80-HPBAR_OX)

    static int oldhp = 0;
    static int oldhx = 0;
    int cx;
    int hx = HPBAR_WIDTH *hp/maxhp;

    gotoxy(HPBAR_OX, HPBAR_OY);
#ifdef USE_TILE
    mpr_on(MODE_STAT);
#endif

    for (cx=0;cx<HPBAR_WIDTH;cx++)
    {
        textcolor(BLACK + DARKGREY*16);

        if(cx<hx)
            textcolor(BLACK + GREEN*16);
        else
        if(oldhp>hp && oldhx>hx && cx< oldhx && cx>=hx)
            textcolor(BLACK + RED*16);

        putch(' ');
    }

    textcolor(LIGHTGREY);
    oldhp=hp;
    oldhx=hx;
#ifdef USE_TILE
    mpr_on(MODE_CRT);
#endif
}
#endif

void print_stats(void)
{
#ifdef USE_TILE
    mpr_on(MODE_STAT);
#endif

    textcolor(LIGHTGREY);

    if (you.redraw_hit_points)
    {
        const int max_max_hp = you.hp_max + player_rotted();
        const int hp_warn = MAXIMUM( 25, Options.hp_warning );
        const int hp_attent = MAXIMUM( 10, Options.hp_attention );
#ifdef JP
        gotoxy(45, 3);
#else
        gotoxy(44, 3);
#endif
        if (you.hp <= (you.hp_max * hp_warn) / 100)
            textcolor(RED);
        else if (you.hp <= (you.hp_max * hp_attent) / 100)
            textcolor(YELLOW);
#ifdef JP
        cprintf( "%3d", you.hp );

        textcolor(LIGHTGREY);
        cprintf( "/%3d", you.hp_max );

        if (max_max_hp != you.hp_max)
        {
            cprintf( " (%3d)", max_max_hp );
        }
#else
        {
        cprintf( "%d", you.hp );
        }
        textcolor(LIGHTGREY);
        cprintf( "/%d", you.hp_max );

        if (max_max_hp != you.hp_max)
        {
            cprintf( " (%d)", max_max_hp );
        }
#endif

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf("        ");
#endif

#ifdef USE_TILE //HPBAR
        draw_hp_bar(you.hp, you.hp_max);
        mpr_on(MODE_STAT);
#endif

        you.redraw_hit_points = 0;
    }

    if (you.redraw_magic_points)
    {
#ifdef JP
        gotoxy(45, 4);
        cprintf( "%3d/%3d", you.magic_points, you.max_magic_points );
#else
        gotoxy(47, 4);
        cprintf( "%d/%d", you.magic_points, you.max_magic_points );
#endif

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf("        ");
#endif

        you.redraw_magic_points = 0;
    }

    if (you.redraw_strength)
    {
        if (you.strength < 0)
            you.strength = 0;
        else if (you.strength > 72)
            you.strength = 72;

        if (you.max_strength > 72)
            you.max_strength = 72;
#ifdef JP
        gotoxy(45, 7);
#else
        gotoxy(45, 7);
#endif
        if (you.might)
            textcolor(LIGHTBLUE);  // no end of effect warning
        else if (you.strength < you.max_strength)
            textcolor(YELLOW);
#ifdef JP
        cprintf( "%2d", you.strength );
        textcolor(LIGHTGREY);

        if (you.strength != you.max_strength)
        {
            cprintf( " (%2d)", you.max_strength );
        }
#else
        cprintf( "%d", you.strength );
        textcolor(LIGHTGREY);

        if (you.strength != you.max_strength)
        {
            cprintf( " (%d)  ", you.max_strength );
        }
#endif
        else
        {
            cprintf( "       " );
        }

        you.redraw_strength = 0;

        if (you.strength < 1)
            ouch(-9999, 0, KILLED_BY_WEAKNESS);

        burden_change();
#ifdef USE_TILE
        mpr_on(MODE_STAT);
#endif
    }

    if (you.redraw_intelligence)
    {
        if (you.intel < 0)
            you.intel = 0;
        else if (you.intel > 72)
            you.intel = 72;

        if (you.max_intel > 72)
            you.max_intel = 72;
#ifdef JP
        gotoxy(45, 8);
#else
        gotoxy(45, 8);
#endif
        if (you.intel < you.max_intel)
            textcolor(YELLOW);
#ifdef JP
        cprintf( "%2d", you.intel );
        textcolor(LIGHTGREY);

        if (you.intel != you.max_intel)
        {
            cprintf( " (%2d)", you.max_intel );
        }
#else
        {
            cprintf( "%d", you.intel );
        }
        textcolor(LIGHTGREY);

        if (you.intel != you.max_intel)
        {
            cprintf( " (%d)  ", you.max_intel );
        }
#endif
        else
        {
            cprintf( "       " );
        }

        you.redraw_intelligence = 0;

        if (you.intel < 1)
            ouch(-9999, 0, KILLED_BY_STUPIDITY);
    }

    if (you.redraw_dexterity)
    {
        if (you.dex < 0)
            you.dex = 0;
        else if (you.dex > 72)
            you.dex = 72;

        if (you.max_dex > 72)
            you.max_dex = 72;
#ifdef JP
        gotoxy(45, 9);
#else
        gotoxy(45, 9);
#endif
        if (you.dex < you.max_dex)
            textcolor(YELLOW);
#ifdef JP
        cprintf( "%2d", you.dex );
        textcolor(LIGHTGREY);

        if (you.dex != you.max_dex)
        {
            cprintf( " (%2d)", you.max_dex );
        }
#else
        cprintf( "%d", you.dex );
        textcolor(LIGHTGREY);

        if (you.dex != you.max_dex)
        {
            cprintf( " (%d)  ", you.max_dex );
        }
#endif
        else
        {
            cprintf( "       " );
        }

        you.redraw_dexterity = 0;

        if (you.dex < 1)
            ouch(-9999, 0, KILLED_BY_CLUMSINESS);
    }

    if (you.redraw_armour_class)
    {
#ifdef JP
        gotoxy(45, 5);
#else
        gotoxy(44, 5);
#endif

        if (you.duration[DUR_STONEMAIL])
            dur_colour( BLUE, (you.duration[DUR_STONEMAIL] <= 6) );
        else if (you.duration[DUR_ICY_ARMOUR] || you.duration[DUR_STONESKIN])
            textcolor( LIGHTBLUE );  // no end of effect warning
#ifdef JP
        cprintf( "%2d  ", player_AC() );
#else
        cprintf( "%d  ", player_AC() );
#endif

        textcolor( LIGHTGREY );
#ifdef JP
        gotoxy(48, 5);
        if (you.duration[DUR_CONDENSATION_SHIELD])      //jmf: added 24mar2000
            textcolor( LIGHTBLUE );  // no end of effect warning
        cprintf( "(%d) ", player_shield_class() );
#else
        gotoxy(50, 5);
        if (you.duration[DUR_CONDENSATION_SHIELD])      //jmf: added 24mar2000
            textcolor( LIGHTBLUE );  // no end of effect warning
        cprintf( "(%d) ", player_shield_class() );
#endif

        textcolor( LIGHTGREY );

        you.redraw_armour_class = 0;
    }

    if (you.redraw_evasion)
    {
#ifdef JP
        gotoxy(45, 6);
        if (you.duration[DUR_FORESCRY])
            textcolor(LIGHTBLUE);  // no end of effect warning

        cprintf( "%2d  ", player_evasion() );
#else
        gotoxy(44, 6);
        if (you.duration[DUR_FORESCRY])
            textcolor(LIGHTBLUE);  // no end of effect warning

        cprintf( "%d  ", player_evasion() );
#endif
        textcolor(LIGHTGREY);

        you.redraw_evasion = 0;
    }

    if (you.redraw_gold)
    {
#ifdef JP
        gotoxy(47, 10);
        cprintf( "%d     ", you.gold );
#else
        gotoxy(46, 10);
        cprintf( "%d     ", you.gold );
#endif
        you.redraw_gold = 0;
    }

    if (you.redraw_experience)
    {
#ifdef JP
        gotoxy(47, 11);
#else
        gotoxy(52, 11);
#endif
#if DEBUG_DIAGNOSTICS
        cprintf( "%d/%d  (%d/%d)",
                 you.experience_level, you.experience,
                 you.skill_cost_level, you.exp_available );
#else
#ifdef JP
        cprintf( "%d/%d (%d)",
                 you.experience_level, you.experience, you.exp_available );
#else
        cprintf( "%d/%d  (%d)",
                 you.experience_level, you.experience, you.exp_available );
#endif
#endif

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf("     ");
#endif
        you.redraw_experience = 0;
    }

    if (you.wield_change)
    {
        gotoxy(40, 13);
#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf("                                       ");
#endif

        if (you.equip[EQ_WEAPON] != -1)
        {
            gotoxy(40, 13);
            textcolor(you.inv[you.equip[EQ_WEAPON]].colour);

            char str_pass[ ITEMNAME_SIZE ];
            in_name( you.equip[EQ_WEAPON], DESC_INVENTORY, str_pass, Options.terse_hand );
            str_pass[39] = '\0';

            cprintf(str_pass);
            textcolor(LIGHTGREY);
        }
        else
        {
            gotoxy(40, 13);

            if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
            {
                textcolor(RED);
#ifdef JP
                cprintf("ênÇÃéË");
#else
                cprintf("Blade Hands");
#endif
                textcolor(LIGHTGREY);
            }
            else
            {
                textcolor(LIGHTGREY);
#ifdef JP
                cprintf("âΩÇ‡éËÇ…ÇµÇƒÇ¢Ç»Ç¢");
#else
                cprintf("Nothing wielded");
#endif
            }
        }
        you.wield_change = false;
    }

    // The colour scheme for these flags is currently:
    //
    // - yellow, "orange", red      for bad conditions
    // - light grey, white          for god based conditions
    // - green, light green         for good conditions
    // - blue, light blue           for good enchantments
    // - magenta, light magenta     for "better" enchantments (deflect, fly)

    if (you.redraw_status_flags & REDRAW_LINE_1_MASK)
    {
        gotoxy(40, 14);

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf( "                                       " );
        gotoxy(40, 14);
#endif

        switch (you.burden_state)
        {
        case BS_OVERLOADED:
            textcolor( RED );
#ifdef JP
            cprintf( "èdó ÇÃí¥âﬂ " );
#else
            cprintf( "Overloaded " );
#endif
            break;

        case BS_ENCUMBERED:
            textcolor( LIGHTRED );
#ifdef JP
            cprintf( "èdó ÇÃïâíS " );
#else
            cprintf( "Encumbered " );
#endif
            break;

        case BS_UNENCUMBERED:
            break;
        }

        switch (you.hunger_state)
        {
        case HS_ENGORGED:
            textcolor( LIGHTGREEN );
#ifdef JP
            cprintf( "êHÇ◊âﬂÇ¨" );
#else
            cprintf( "Engorged" );
#endif
            break;

        case HS_FULL:
            textcolor( GREEN );
#ifdef JP
            cprintf( "ñûï†" );
#else
            cprintf( "Full" );
#endif
            break;

        case HS_SATIATED:
            break;

        case HS_HUNGRY:
            textcolor( YELLOW );
#ifdef JP
            cprintf( "ãÛï†" );
#else
            cprintf( "Hungry" );
#endif
            break;

        case HS_STARVING:
            textcolor( RED );
#ifdef JP
            cprintf( "ãQâÏ" );
#else
            cprintf( "Starving" );
#endif
            break;
        }

        textcolor( LIGHTGREY );

#if DEBUG_DIAGNOSTICS
        // debug mode hunger-o-meter
        cprintf( " (%d:%d) ", you.hunger - you.old_hunger, you.hunger );
#endif
    }

    if (you.redraw_status_flags & REDRAW_LINE_2_MASK)
    {
        gotoxy(40, 15);

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf( "                                       " );
        gotoxy(40, 15);
#endif

        // Max length of this line = 8 * 5 - 1 = 39

        if (you.duration[DUR_PRAYER])
        {
            textcolor( WHITE );  // no end of effect warning
#ifdef JP
            cprintf( "ãFÇË " );
#else
            cprintf( "Pray " );
#endif
        }

        if (you.duration[DUR_REPEL_UNDEAD])
        {
            dur_colour( LIGHTGREY, (you.duration[DUR_REPEL_UNDEAD] <= 4) );
#ifdef JP
            cprintf( "ê_êπ " );
#else
            cprintf( "Holy " );
#endif
        }

        if (you.duration[DUR_DEFLECT_MISSILES])
        {

            dur_colour( MAGENTA, (you.duration[DUR_DEFLECT_MISSILES] <= 6) );
#ifdef JP
            cprintf( "ïŒå¸ " );
#else
            cprintf( "DMsl " );
#endif
        }
        else if (you.duration[DUR_REPEL_MISSILES])
        {
            dur_colour( BLUE, (you.duration[DUR_REPEL_MISSILES] <= 6) );
#ifdef JP
            cprintf( "îÚñh " );
#else
            cprintf( "RMsl " );
#endif
        }

        if (you.duration[DUR_REGENERATION])
        {
            dur_colour( BLUE, (you.duration[DUR_REGENERATION] <= 6) );
#ifdef JP
            cprintf( "çƒê∂ " );
#else
            cprintf( "Regen " );
#endif
        }

        if (you.duration[DUR_INSULATION])
        {
            dur_colour( BLUE, (you.duration[DUR_INSULATION] <= 6) );
#ifdef JP
            cprintf( "ê‚âè " );
#else
            cprintf( "Ins " );
#endif
        }

        if (player_is_levitating())
        {
            bool perm = (you.species == SP_KENKU && you.experience_level >= 15)
                        || (player_equip_ego_type( EQ_BOOTS, SPARM_LEVITATION ))
                        || (you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON);

            if (wearing_amulet( AMU_CONTROLLED_FLIGHT ))
            {
                dur_colour( MAGENTA, (you.levitation <= 10 && !perm) );
#ifdef JP
                cprintf( "îÚçs " );
#else
                cprintf( "Fly " );
#endif
            }
            else
            {
                dur_colour( BLUE, (you.levitation <= 10 && !perm) );
#ifdef JP
                cprintf( "ïÇóV " );
#else
                cprintf( "Lev " );
#endif
            }
        }

        if (you.invis)
        {
            dur_colour( BLUE, (you.invis <= 6) );
#ifdef JP
            cprintf( "ìßñæ " );
#else
            cprintf( "Invis " );
#endif
        }

        // Perhaps this should be reversed to show when it can be used?
        // In that case, it should be probably be GREEN, and we'd have
        // to check to see if the player does have a breath weapon. -- bwr
        if (you.duration[DUR_BREATH_WEAPON])
        {
            textcolor( YELLOW );  // no warning
#ifdef JP
            cprintf( "öbãC" );
#else
            cprintf( "BWpn" );
#endif
        }

        textcolor( LIGHTGREY );
    }

    if (you.redraw_status_flags & REDRAW_LINE_3_MASK)
    {
        gotoxy(40, 16);

#ifdef LINUX
        clear_to_end_of_line();
#else
        cprintf( "                                       " );
        gotoxy(40, 16);
#endif
        // Max length of this line = 7 * 5 + 3 - 1 = 37

        // Note the usage of bad_ench_colour() correspond to levels that
        // can be found in player.cc, ie those that the player can tell by
        // using the '@' command.  Things like confusion and sticky flame
        // hide their amounts and are thus always the same colour (so
        // we're not really exposing any new information). --bwr
        if (you.conf)
        {
            textcolor( RED );   // no different levels
#ifdef JP
            cprintf( "ç¨óê " );
#else
            cprintf( "Conf " );
#endif
        }

        if (you.duration[DUR_LIQUID_FLAMES])
        {
            textcolor( RED );   // no different levels
#ifdef JP
            cprintf( "èƒàŒ " );
#else
            cprintf( "Fire " );
#endif
        }

        if (you.poison)
        {
            // We skip marking "quite" poisoned and instead mark the
            // levels where the rules for dealing poison damage change
            // significantly.  See acr.cc for that code. -- bwr
            textcolor( bad_ench_colour( you.poison, 5, 10 ) );
#ifdef JP
            cprintf( "ì≈ " );
#else
            cprintf( "Pois " );
#endif
        }

        if (you.disease)
        {
            textcolor( bad_ench_colour( you.disease, 40, 120 ) );
#ifdef JP
            cprintf( "ïaãC " );
#else
            cprintf( "Sick " );
#endif
        }

        if (you.rotting)
        {
            textcolor( bad_ench_colour( you.rotting, 4, 8 ) );
#ifdef JP
            cprintf( "ïÖîs " );
#else
            cprintf( "Rot " );
#endif
        }

        if (you.magic_contamination > 5)
        {
            textcolor( bad_ench_colour( you.magic_contamination, 15, 25 ) );
#ifdef JP
            cprintf( "î≠åı " );
#else
            cprintf( "Glow " );
#endif
        }

        if (you.duration[DUR_SWIFTNESS])
        {
            dur_colour( BLUE, (you.duration[DUR_SWIFTNESS] <= 6) );
#ifdef JP
            cprintf( "èrë´ " );
#else
            cprintf( "Swift " );
#endif
        }

        if (you.slow && !you.haste)
        {
            textcolor( RED );  // no end of effect warning
#ifdef JP
            cprintf( "å∏ë¨" );
#else
            cprintf( "Slow" );
#endif
        }
        else if (you.haste && !you.slow)
        {
            dur_colour( BLUE, (you.haste <= 6) );
#ifdef JP
            cprintf( "â¡ë¨" );
#else
            cprintf( "Fast" );
#endif
        }

        textcolor( LIGHTGREY );
    }

    you.redraw_status_flags = 0;

#if DEBUG_DIAGNOSTICS
    // debug mode GPS
    gotoxy(40, 17);
#ifdef JP
    cprintf( "Position (%2d,%2d)", you.x_pos, you.y_pos );
#else
    cprintf( "Position (%2d,%2d)", you.x_pos, you.y_pos );
#endif
#endif

#ifdef LINUX
    // get curses to redraw screen
    update_screen();
#endif

#ifdef USE_TILE
    mpr_on(MODE_CRT);
#endif

}                               // end print_stats()

#ifdef JP
unsigned char* itosym1(int stat)
{
    return (unsigned char*)( (stat >= 1) ? "Åõ" : "ÅE" );
}

unsigned char* itosym3(int stat)
{
    return (unsigned char*)( (stat >= 3) ? "ÅõÅõÅõ" :
             (stat == 2) ? "ÅõÅõÅE" :
             (stat == 1) ? "ÅõÅEÅE" :
             (stat == 0) ? "ÅEÅEÅE" :
                           "Å~ÅEÅE");
}
#else
unsigned char* itosym1(int stat)
{
    return (unsigned char*)( (stat >= 1) ? "+  " : ".  " );
}

unsigned char* itosym3(int stat)
{
    return (unsigned char*)( (stat >= 3) ? "+ + +" :
             (stat == 2) ? "+ + ." :
             (stat == 1) ? "+ . ." :
             (stat == 0) ? ". . ." :
                           "x . .");
}
#endif


#ifdef JP
void get_full_detail(char* buffer, bool calc_unid)
{
#define FIR_AD buffer,44
#define CUR_AD &buffer[++lines*45],44
#define BUF_SIZE 25*3*45
    int lines =0, i = 0, j = 0;
    char str_pass[ITEMNAME_SIZE];

    for(i=0; i < BUF_SIZE; i++)
        buffer[i] = '\0';

    snprintf(CUR_AD, "%sÅw%sÅx\0", player_title(), you.your_name);
    lines++;
    snprintf(CUR_AD, "éÌë∞     : %s\0", species_name(you.species,you.experience_level) );
    snprintf(CUR_AD, "êEã∆     : %s\0", you.class_name);
    snprintf(CUR_AD, "êMã¬     : %s\0", god_name(you.religion) );
    snprintf(CUR_AD, "\0");
    snprintf(CUR_AD, "ÉåÉxÉã     %7d\0", you.experience_level);
    snprintf(CUR_AD, "åoå±íl     %7d\0", you.experience);

    if (you.experience_level < 27)
    {
        int xp_needed = (exp_needed(you.experience_level+2) - you.experience) + 1;
        snprintf(CUR_AD, "éüÉåÉxÉã   %7d\0", exp_needed(you.experience_level + 2) + 1);
        snprintf(CUR_AD, "ïKóvåoå±íl %7d\0", xp_needed);
    }
    else
    {
        snprintf(CUR_AD, "éüÉåÉxÉã   *******\0");
        snprintf(CUR_AD, "ïKóvåoå±íl *******\0");
    }

    snprintf(CUR_AD, "écÇËãLâØóÕ %7d\0", player_spell_levels() );

    snprintf(CUR_AD, "èäéùã‡     %7d\0", you.gold );

    lines++;
/*
    snprintf(str_pass, ITEMNAME_SIZE, "%s\0",
             ( (you.hp > 0)       ? "":
               (you.deaths_door)  ? " (ïméÄ)":
                                    " (éÄñS)" ) );
*/
    if ( (you.hp_max + player_rotted() ) == you.hp_max )
    {
        snprintf(CUR_AD, "ÇgÇo     : %3d/%3d\0",
                 you.hp, you.hp_max);
    }
    else
    {
        snprintf(CUR_AD, "ÇgÇo     : %3d/%3d (%3d)\0",
                 you.hp, you.hp_max,
                 you.hp_max + player_rotted() );
    }

    snprintf(CUR_AD, "ÇlÇo     : %3d/%3d\0", you.magic_points, you.max_magic_points);

    if (you.strength == you.max_strength)
    {
        snprintf(CUR_AD, "òróÕ     : %3d\0", you.strength);
    }
    else
    {
        snprintf(CUR_AD, "òróÕ     : %3d (%2d)\0", you.strength, you.max_strength);
    }

    if (you.intel == you.max_intel)
    {
        snprintf(CUR_AD, "ímóÕ     : %3d\0", you.intel);
    }
    else
    {
        snprintf(CUR_AD, "ímóÕ     : %3d (%2d)\0", you.intel, you.max_intel);
    }

    if (you.dex == you.max_dex)
    {
        snprintf(CUR_AD, "äÌópÇ≥   : %3d\0", you.dex);
    }
    else
    {
        snprintf(CUR_AD, "äÌópÇ≥   : %3d (%2d)\0", you.dex, you.max_dex);
    }

    snprintf(CUR_AD, "Ç`Çb     : %3d\0", player_AC() );
    snprintf(CUR_AD, "âÒî     : %3d\0", player_evasion() );
    snprintf(CUR_AD, "èÇ       : %3d\0", player_shield_class() );
    lines++;

    if (you.real_time != -1)
    {
        const time_t curr = you.real_time + (time(NULL) - you.start_time);
        char buff[200];
        make_time_string( curr, buff, sizeof(buff) );

        snprintf(CUR_AD, "ÉvÉåÉCéûä‘ : %10s\0", buff);
        snprintf(CUR_AD, "åoâﬂÉ^Å[Éì : %10d\0", you.num_turns );
    }

    lines = 27;

    snprintf(CUR_AD, "âŒâäëœê´ : %s\0", itosym3( player_res_fire(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_fire() );
    snprintf(CUR_AD, "ó‚ãCëœê´ : %s\0", itosym3( player_res_cold(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_cold() );
    snprintf(CUR_AD, "êäé„ëœê´ : %s\0", itosym3( player_prot_life(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_prot_life() );
    snprintf(CUR_AD, "ì≈ëfëœê´ : %s\0", itosym1( player_res_poison(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_poison() );
    snprintf(CUR_AD, "ìdåÇëœê´ : %s\0", itosym1( player_res_electricity(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_electricity() );
    lines++;

    snprintf(CUR_AD, "î\óÕà€éù : %s\0", itosym1( player_sust_abil(calc_unid) ) );
    snprintf(CUR_AD, "ëœïœàŸ   : %s\0", itosym1( wearing_amulet( AMU_RESIST_MUTATION, calc_unid) ) );
    snprintf(CUR_AD, "ëœå∏ë¨   : %s\0", itosym1( wearing_amulet( AMU_RESIST_SLOW, calc_unid) ) );
    snprintf(CUR_AD, "ñæù     : %s\0", itosym1( wearing_amulet( AMU_CLARITY, calc_unid) ) );
    lines++;
    lines++;
/*
    if (you.wizard)
    {
        lines++;
        snprintf(CUR_AD, "çUåÇë¨ìx : %2d\0", check_weapon_speed() );
        snprintf(CUR_AD, "à⁄ìÆë¨ìx : %2d\0", player_movement_speed() );
        snprintf(CUR_AD, "ñÇñ@íÔçR : %2d\0", player_res_magic(calc_unid) / 10 );
        //snprintf(CUR_AD, " /%2d", (player_res_magic() + 1) / 10 );
        snprintf(CUR_AD, "âBñßê´Å@ : %2d\0", check_stealth(calc_unid) / 10 );
        lines++;
    }
    else
*/
    {
        const char *e_items[] = {"ïêäÌ  ", "äZ    ", "èÇ    ", "äï    ", "äOìÖ  ",
                                 "è¨éË  ", "åC    ", "åÏïÑ  ", "éwó÷  ", "éwó÷  "};
        const int   e_order[] = {EQ_WEAPON, EQ_BODY_ARMOUR, EQ_SHIELD, EQ_HELMET, EQ_CLOAK,
                                 EQ_GLOVES, EQ_BOOTS, EQ_AMULET, EQ_RIGHT_RING, EQ_LEFT_RING};

        for(i=0; i < NUM_EQUIP; i++)
        {
            if ( you.equip[ e_order[i] ] != -1)
            {
                in_name( you.equip[ e_order[i] ], DESC_PLAIN, str_pass, Options.terse_hand );
                snprintf(CUR_AD, "%s%s\0", e_items[i], &str_pass);
            }
            else
            {
                if (e_order[i] == EQ_WEAPON)
                {
                    if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
                        snprintf(CUR_AD, "%sênÇÃéË\0", e_items[i]);
                    else if (you.skills[SK_UNARMED_COMBAT])
                        snprintf(CUR_AD, "%sìkéËäiì¨\0", e_items[i]);
                    else
                        snprintf(CUR_AD, "%sÇ»Çµ\0", e_items[i]);
                }
                else
                {
                        snprintf(CUR_AD, "%sÇ»Çµ\0", e_items[i]);
                }
            }
        }
    }

    lines = 52;
    snprintf(CUR_AD, "óÏéã     : %s\0", itosym1( player_see_invis(calc_unid) ) );
    snprintf(CUR_AD, "åãäE     : %s\0", itosym1( wearing_amulet(AMU_WARDING, calc_unid)
                                               ||(you.religion == GOD_VEHUMET && you.duration[DUR_PRAYER]
                                               && (!player_under_penance() && you.piety >= 75) )
                                               ) );
    snprintf(CUR_AD, "ï€ëS     : %s\0", itosym1( wearing_amulet( AMU_CONSERVATION, calc_unid) ) );
    snprintf(CUR_AD, "ïÖêIëjé~ : %s\0", itosym1( wearing_amulet( AMU_RESIST_CORROSION, calc_unid) ) );

    if ( !wearing_amulet( AMU_THE_GOURMAND, calc_unid) )
    {
        switch (you.species)
        {
        case SP_GHOUL:
            snprintf(CUR_AD, "ïÖì˜ãÚÇ¢ : %s\0", itosym3(3) );
            break;

        case SP_KOBOLD:
        case SP_TROLL:
            snprintf(CUR_AD, "ïÖì˜ãÚÇ¢ : %s\0", itosym3(2) );
            break;

        case SP_HILL_ORC:
        case SP_OGRE:
            snprintf(CUR_AD, "ïÖì˜ãÚÇ¢ : %s\0", itosym3(1) );
            break;
#ifdef V_FIX
        case SP_OGRE_MAGE:
            snprintf(CUR_AD, "ëÂêH     : %s\0", itosym1(1) );
            break;
#endif
        default:
            snprintf(CUR_AD, "à´êH     : %s\0", itosym1(0) );
            break;
        }
    }
    else
    {
        snprintf(CUR_AD, "à´êH     : %s\0", itosym1( wearing_amulet( AMU_THE_GOURMAND, calc_unid) ) );
    }

    lines++;

    if ( scan_randarts(RAP_PREVENT_TELEPORTATION, calc_unid) )
        snprintf(CUR_AD, "ì]à ëjäQ : %s\0", itosym1( scan_randarts(RAP_PREVENT_TELEPORTATION, calc_unid) ) );
    else
    {
      //snprintf(CUR_AD, "ì]à óUî≠ : %s\0", itosym3( (player_teleport(calc_unid) + 11) / 12 ) );
        snprintf(CUR_AD, "ì]à óUî≠ : %s\0", itosym1( player_teleport(calc_unid) ) );
    }

    if ( ( you.attribute[ATTR_CONTROL_TELEPORT] <= 1 )
       &&( !player_equip( EQ_RINGS, RING_TELEPORT_CONTROL, calc_unid) ) )
        snprintf(CUR_AD, "ì]à êßå‰ : %s\0", itosym1( 0 ) );
    else
        snprintf(CUR_AD, "ì]à êßå‰ : %s\0", itosym1( 1 ) );

    snprintf(CUR_AD, "ïÇóV     : %s\0", itosym1( player_is_levitating() ) );
    snprintf(CUR_AD, "îÚçsêßå‰ : %s\0", itosym1( wearing_amulet(AMU_CONTROLLED_FLIGHT, calc_unid) ) );

    lines++;

    return;
}

#else
void get_full_detail(char* buffer, bool calc_unid)
{
#define FIR_AD buffer,44
#define CUR_AD &buffer[++lines*45],44
#define BUF_SIZE 25*3*45
    int lines =0, i = 0, j = 0;

    for(i=0; i < BUF_SIZE; i++)
        buffer[i] = '\0';

    snprintf(CUR_AD, "%s the %s\0", you.your_name, player_title());
    lines++;
    snprintf(CUR_AD, "Race     : %s\0", species_name(you.species,you.experience_level) );
    snprintf(CUR_AD, "Class    : %s\0", you.class_name);
    snprintf(CUR_AD, "Worship  : %s\0", god_name(you.religion) );
    snprintf(CUR_AD, "\0");
    snprintf(CUR_AD, "Level      %7d\0", you.experience_level);
    snprintf(CUR_AD, "Exp        %7d\0", you.experience);

    if (you.experience_level < 27)
    {
        int xp_needed = (exp_needed(you.experience_level+2) - you.experience) + 1;
        snprintf(CUR_AD, "Next Level %7d\0", exp_needed(you.experience_level + 2) + 1);
        snprintf(CUR_AD, "Exp Needed %7d\0", xp_needed);
    }
    else
    {
        snprintf(CUR_AD, "Next Level *******\0");
        snprintf(CUR_AD, "Exp Needed *******\0");
    }

    snprintf(CUR_AD,     "Spls.Left  %7d\0", player_spell_levels() );
    snprintf(CUR_AD,     "Gold       %7d\0", you.gold );

    lines++;

    if ( (you.hp_max + player_rotted() ) == you.hp_max )
    {
        snprintf(CUR_AD, "HP       : %3d/%3d\0", you.hp, you.hp_max);
    }
    else
    {
        snprintf(CUR_AD,  "HP       : %3d/%3d (%3d)\0",
        you.hp, you.hp_max, you.hp_max + player_rotted() );
    }

    snprintf(CUR_AD, "MP       : %3d/%3d\0", you.magic_points, you.max_magic_points);

    if (you.strength == you.max_strength)
    {
        snprintf(CUR_AD, "Str      : %3d\0", you.strength);
    }
    else
    {
        snprintf(CUR_AD, "Str      : %3d (%2d)\0", you.strength, you.max_strength);
    }

    if (you.intel == you.max_intel)
    {
        snprintf(CUR_AD, "Int      : %3d\0", you.intel);
    }
    else
    {
        snprintf(CUR_AD, "Int      : %3d (%2d)\0", you.intel, you.max_intel);
    }

    if (you.dex == you.max_dex)
    {
        snprintf(CUR_AD, "Dex      : %3d\0", you.dex);
    }
    else
    {
        snprintf(CUR_AD, "Dex      : %3d (%2d)\0", you.dex, you.max_dex);
    }

    snprintf(CUR_AD, "Armour Cl: %3d\0", player_AC() );
    snprintf(CUR_AD, "Evasion  : %3d\0", player_evasion() );
    snprintf(CUR_AD, "Shield Cl: %3d\0", player_shield_class() );
    lines++;

    if (you.real_time != -1)
    {
        const time_t curr = you.real_time + (time(NULL) - you.start_time);
        char buff[200];
        make_time_string( curr, buff, sizeof(buff) );

        snprintf(CUR_AD, "Play time  : %10s\0", buff);
        snprintf(CUR_AD, "Turns      : %10d\0", you.num_turns );
    }

    lines = 27;

    snprintf(CUR_AD, "Res.Fire  : %s\0", itosym3( player_res_fire(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_fire() );
    snprintf(CUR_AD, "Res.Cold  : %s\0", itosym3( player_res_cold(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_cold() );
    snprintf(CUR_AD, "Life Prot.: %s\0", itosym3( player_prot_life(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_prot_life() );
    snprintf(CUR_AD, "Res.Poison: %s\0", itosym1( player_res_poison(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_poison() );
    snprintf(CUR_AD, "Res.Elec. : %s\0", itosym1( player_res_electricity(calc_unid) ) );
    //snprintf(CUR_AD, " /%2d", player_res_electricity() );
    lines++;

    snprintf(CUR_AD, "Sust.Abil.: %s\0", itosym1( player_sust_abil(calc_unid) ) );
    snprintf(CUR_AD, "Res.Mut.  : %s\0", itosym1( wearing_amulet( AMU_RESIST_MUTATION, calc_unid) ) );
    snprintf(CUR_AD, "Res.Slow  : %s\0", itosym1( wearing_amulet( AMU_RESIST_SLOW, calc_unid) ) );
    snprintf(CUR_AD, "Clarity   : %s\0", itosym1( wearing_amulet( AMU_CLARITY, calc_unid) ) );
    lines++;
    lines++;

    {
        char str_pass[ITEMNAME_SIZE];
        const char *e_items[] = {"Weapon ", "Armour ", "Shield ", "Helmet ", "Cloak  ",
                                 "Gloves ", "Boots  ", "Amulet ", "Ring   ", "Ring   "};
        const int   e_order[] = {EQ_WEAPON, EQ_BODY_ARMOUR, EQ_SHIELD, EQ_HELMET, EQ_CLOAK,
                                 EQ_GLOVES, EQ_BOOTS, EQ_AMULET, EQ_RIGHT_RING, EQ_LEFT_RING};

        for(i=0; i < NUM_EQUIP; i++)
        {
            if ( you.equip[ e_order[i] ] != -1)
            {
                in_name( you.equip[ e_order[i] ], DESC_PLAIN, str_pass, Options.terse_hand );
                snprintf(CUR_AD, "%s%s\0", e_items[i], &str_pass);
            }
            else
            {
                if (e_order[i] == EQ_WEAPON)
                {
                    if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
                        snprintf(CUR_AD, "%sBlade Hands\0", e_items[i]);
                    else if (you.skills[SK_UNARMED_COMBAT])
                        snprintf(CUR_AD, "%sUnarmed\0", e_items[i]);
                    else
                        snprintf(CUR_AD, "%snone\0", e_items[i]);
                }
                else
                {
                        snprintf(CUR_AD, "%snone\0", e_items[i]);
                }
            }
        }
    }

    lines = 52;
    snprintf(CUR_AD, "See Invis. : %s\0", itosym1( player_see_invis(calc_unid) ) );
    snprintf(CUR_AD, "Warding    : %s\0", itosym1( wearing_amulet(AMU_WARDING, calc_unid)
                                               ||(you.religion == GOD_VEHUMET && you.duration[DUR_PRAYER]
                                               && (!player_under_penance() && you.piety >= 75) )
                                               ) );
    snprintf(CUR_AD, "Conserve   : %s\0", itosym1( wearing_amulet( AMU_CONSERVATION, calc_unid) ) );
    snprintf(CUR_AD, "Res.Corr.  : %s\0", itosym1( wearing_amulet( AMU_RESIST_CORROSION, calc_unid) ) );

    if ( !wearing_amulet( AMU_THE_GOURMAND, calc_unid) )
    {
        switch (you.species)
        {
        case SP_GHOUL:
            snprintf(CUR_AD, "Saprovore  : %s\0", itosym3(3) );
            break;

        case SP_KOBOLD:
        case SP_TROLL:
            snprintf(CUR_AD, "Saprovore  : %s\0", itosym3(2) );
            break;

        case SP_HILL_ORC:
        case SP_OGRE:
            snprintf(CUR_AD, "Saprovore  : %s\0", itosym3(1) );
            break;
#ifdef V_FIX
        case SP_OGRE_MAGE:
            snprintf(CUR_AD, "Voracious  : %s\0", itosym1(1) );
            break;
#endif
        default:
            snprintf(CUR_AD, "Gourmand   : %s\0", itosym1(0) );
            break;
        }
    }
    else
    {
        snprintf(CUR_AD, "Gourmand   : %s\0", itosym1( wearing_amulet( AMU_THE_GOURMAND, calc_unid) ) );
    }

    lines++;

    if ( scan_randarts(RAP_PREVENT_TELEPORTATION, calc_unid) )
        snprintf(CUR_AD, "Prev.Telep.: %s\0", itosym1( scan_randarts(RAP_PREVENT_TELEPORTATION, calc_unid) ) );
    else
    {
      //snprintf(CUR_AD, "Rnd.Telep. : %s\0", itosym3( (player_teleport(calc_unid) + 11) / 12 ) );
        snprintf(CUR_AD, "Rnd.Telep. : %s\0", itosym1( player_teleport(calc_unid) ) );
    }

    if ( ( you.attribute[ATTR_CONTROL_TELEPORT] <= 1 )
       &&( !player_equip( EQ_RINGS, RING_TELEPORT_CONTROL, calc_unid) ) )
        snprintf(CUR_AD, "Ctrl.Telep.: %s\0", itosym1( 0 ) );
    else
        snprintf(CUR_AD, "Ctrl.Telep.: %s\0", itosym1( 1 ) );

    snprintf(CUR_AD, "Ctrl.Telep.: %s\0", itosym1( you.attribute[ATTR_CONTROL_TELEPORT] ) );
    snprintf(CUR_AD, "Levitation : %s\0", itosym1( player_is_levitating() ) );
    snprintf(CUR_AD, "Ctrl.Flight: %s\0", itosym1( wearing_amulet(AMU_CONTROLLED_FLIGHT, calc_unid) ) );
    lines++;

    return;
}
#endif
