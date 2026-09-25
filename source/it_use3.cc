/*
 *  File:       it_use3.cc
 *  Summary:    Functions for using some of the wackier inventory items.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>     6/13/99         BWR             Auto ID Channel staff
 *      <3>     5/22/99         BWR             SPWLD_POWER is now HP/13 - 3
 *      <2>     5/20/99         BWR             Capped SPWLD_POWER to +20
 *      <1>     -/--/--         LRH             Created
 */

#include "AppHdr.h"
#include "it_use3.h"

#include <string.h>

#include "externs.h"

#include "beam.h"
#include "decks.h"
#include "direct.h"
#include "effects.h"
#include "fight.h"
#include "food.h"
#include "items.h"
#include "it_use2.h"
#include "itemname.h"
#include "misc.h"
#include "monplace.h"
#include "monstuff.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills.h"
#include "skills2.h"
#include "spells1.h"
#include "spells2.h"
#include "spl-book.h"
#include "spl-cast.h"
#include "spl-util.h"
#include "stuff.h"
#include "view.h"
#include "wpn-misc.h"

static bool ball_of_energy(void);
static bool ball_of_fixation(void);
static bool ball_of_seeing(void);
static bool box_of_beasts(void);
static bool disc_of_storms(void);
static bool efreet_flask(void);

void special_wielded(void)
{
    const int wpn = you.equip[EQ_WEAPON];
    const int old_plus = you.inv[wpn].plus;
    const int old_plus2 = you.inv[wpn].plus2;
    const char old_colour = you.inv[wpn].colour;

    char str_pass[ ITEMNAME_SIZE ];
    int temp_rand = 0;          // for probability determination {dlb}
    bool makes_noise = (one_chance_in(20) && !silenced(you.x_pos, you.y_pos));

    switch (you.special_wield)
    {
    case SPWLD_SING:
        if (makes_noise)
        {
#ifdef JP 
            strcpy(info, "『謳う剣』は");
#else
            strcpy(info, "The Singing Sword ");
#endif
            temp_rand = random2(32);
            strcat(info,
#ifdef JP 
                     (temp_rand ==  0) ? "短い曲をハミングした。" :
                     (temp_rand ==  1) ? "壮麗な歌を歌い始めた。" :
                     (temp_rand ==  2) ? "歌った。" :
                     (temp_rand ==  3) ? "やかましく歌った。" :
                     (temp_rand ==  4) ? "メロディを鳴らした。" :
                     (temp_rand ==  5) ? "恐ろしい騒音を立てた。" :
                     (temp_rand ==  6) ? "調子外れに歌った。" :
                     (temp_rand ==  7) ? "『トゥラララ』と歌った。" :
                     (temp_rand ==  8) ? "陽気にゴロゴロと鳴った。" :
                     (temp_rand ==  9) ? "ゴロゴロと鳴った。" :
                     (temp_rand == 10) ? "突然金切り声を上げた！" :
                     (temp_rand == 11) ? "カラカラと笑い声を上げた。" :
                     (temp_rand == 12) ? "さえずるように歌った。" :
                     (temp_rand == 13) ? "美しい旋律を奏でた。" :
                     (temp_rand == 14) ? "美しい音楽を奏でた。" :
                     (temp_rand == 15) ? "やかましいオーケストラの和音を出した。" :
                     (temp_rand == 16) ? "悲しげに泣き言を並べた。" :
                     (temp_rand == 17) ? "チリンチリンと鳴った。" :
                     (temp_rand == 18) ? "ベルのように鳴った。" :
                     (temp_rand == 19) ? "悲しげに泣き叫んだ。" :
                     (temp_rand == 20) ? "音階の練習をはじめた。" :
                     (temp_rand == 21) ? "旋律豊かで軽快に歌った。" :
                     (temp_rand == 22) ? "調子外れにハミングした。" :
                     (temp_rand == 23) ? "溜め息をついた。" :
                     (temp_rand == 24) ? "深く唸るような音楽を奏でた。" :
                     (temp_rand == 25) ? "弾けるような音楽を奏でた。" :
                     (temp_rand == 26) ? "突然に短く断続的な音符で歌った。" :
                     (temp_rand == 27) ? "「やあ！わたしは『謳う剣』」と言った。" :
                     (temp_rand == 28) ? "何かを囁いた。" :
                     (temp_rand == 29) ? "たわ言を言った。" :
                     (temp_rand == 30) ? "支離滅裂にわめき散した。"
                                       : "何か気味の悪い言葉で叫んだ。");
#else
                     (temp_rand ==  0) ? "hums a little tune." :
                     (temp_rand ==  1) ? "breaks into glorious song!" :
                     (temp_rand ==  2) ? "sings." :
                     (temp_rand ==  3) ? "sings loudly." :
                     (temp_rand ==  4) ? "chimes melodiously." :
                     (temp_rand ==  5) ? "makes a horrible noise." :
                     (temp_rand ==  6) ? "sings off-key." :
                     (temp_rand ==  7) ? "sings 'tra-la-la'." :
                     (temp_rand ==  8) ? "burbles away merrily." :
                     (temp_rand ==  9) ? "gurgles." :
                     (temp_rand == 10) ? "suddenly shrieks!" :
                     (temp_rand == 11) ? "cackles." :
                     (temp_rand == 12) ? "warbles." :
                     (temp_rand == 13) ? "chimes harmoniously." :
                     (temp_rand == 14) ? "makes beautiful music." :
                     (temp_rand == 15) ? "produces a loud orchestral chord." :
                     (temp_rand == 16) ? "whines plaintively." :
                     (temp_rand == 17) ? "tinkles." :
                     (temp_rand == 18) ? "rings like a bell." :
                     (temp_rand == 19) ? "wails mournfully." :
                     (temp_rand == 20) ? "practices its scales." :
                     (temp_rand == 21) ? "lilts tunefully." :
                     (temp_rand == 22) ? "hums tunelessly." :
                     (temp_rand == 23) ? "sighs." :
                     (temp_rand == 24) ? "makes a deep moaning sound." :
                     (temp_rand == 25) ? "makes a popping sound." :
                     (temp_rand == 26) ? "sings a sudden staccato note." :
                     (temp_rand == 27) ? "says 'Hi! I'm the Singing Sword!'." :
                     (temp_rand == 28) ? "whispers something." :
                     (temp_rand == 29) ? "speaks gibberish." :
                     (temp_rand == 30) ? "raves incoherently."
                                       : "yells in some weird language.");
#endif
            mpr(info);
        }
        break;

    case SPWLD_CURSE:
        makes_noise = false;

        if (one_chance_in(30))
            curse_an_item(0, 0);
        break;

    case SPWLD_VARIABLE:
        makes_noise = false;

        do_uncurse_item( you.inv[wpn] );

        if (random2(5) < 2)     // 40% chance {dlb}
            you.inv[wpn].plus  += (coinflip() ? +1 : -1);

        if (random2(5) < 2)     // 40% chance {dlb}
            you.inv[wpn].plus2 += (coinflip() ? +1 : -1);

        if (you.inv[wpn].plus < -4)
            you.inv[wpn].plus = -4;
        else if (you.inv[wpn].plus > 16)
            you.inv[wpn].plus = 16;

        if (you.inv[wpn].plus2 < -4)
            you.inv[wpn].plus2 = -4;
        else if (you.inv[wpn].plus2 > 16)
            you.inv[wpn].plus2 = 16;

        you.inv[wpn].colour = random_colour();
        break;

    case SPWLD_TORMENT:
        makes_noise = false;

        if (one_chance_in(200))
        {
            torment( you.x_pos, you.y_pos );
            naughty( NAUGHTY_UNHOLY, 1 );
        }
        break;

    case SPWLD_ZONGULDROK:
        makes_noise = false;

        if (one_chance_in(5))
        {
            animate_dead( 1 + random2(3), BEH_HOSTILE, MHITYOU, 1 );
            naughty( NAUGHTY_NECROMANCY, 1 );
        }
        break;

    case SPWLD_POWER:
        makes_noise = false;

        you.inv[wpn].plus = stepdown_value( -4 + (you.hp / 5), 4, 4, 4, 20 );
        you.inv[wpn].plus2 = you.inv[wpn].plus;
        break;

    case SPWLD_OLGREB:
        makes_noise = false;

        // Giving Olgreb's staff a little lift since staves of poison have
        // been made better. -- bwr
        you.inv[wpn].plus = you.skills[SK_POISON_MAGIC] / 3;
        you.inv[wpn].plus2 = you.inv[wpn].plus;
        break;

    case SPWLD_WUCAD_MU:
        makes_noise = false;

        you.inv[wpn].plus  = ((you.intel > 25) ? 22 : you.intel - 3);
        you.inv[wpn].plus2 = ((you.intel > 25) ? 13 : you.intel / 2);
        break;

    case SPWLD_SHADOW:
        makes_noise = false;

        if (random2(8) <= player_spec_death())
        {
            naughty( NAUGHTY_NECROMANCY, 1 );
            create_monster( MONS_SHADOW, ENCH_ABJ_II, BEH_FRIENDLY,
                            you.x_pos, you.y_pos, you.pet_target, 250 );
        }

        show_green = DARKGREY;
        break;

    case SPWLD_HUM:
        if (makes_noise)
        {
            in_name(wpn, DESC_CAP_YOUR, str_pass);
            strcpy(info, str_pass);
#ifdef JP 
            strcat(info, "が気味の悪い唸りを漏らした。");
#else
            strcat(info, " lets out a weird humming sound.");
#endif
            mpr(info);
        }
        break;                  // to noisy() call at foot 2apr2000 {dlb}

    case SPWLD_CHIME:
        if (makes_noise)
        {
            in_name(wpn, DESC_CAP_YOUR, str_pass);
            strcpy(info, str_pass);
#ifdef JP 
            strcat(info, "が鐘のような美しい音を立てた。");
#else
            strcat(info, " chimes like a gong.");
#endif
            mpr(info);
        }
        break;

    case SPWLD_BECKON:
        if (makes_noise)
#ifdef JP 
            mpr("あなたの名前を呼ぶ声が聞こえた。");
#else
            mpr("You hear a voice call your name.");
#endif
        break;

    case SPWLD_SHOUT:
        if (makes_noise)
#ifdef JP 
            mpr("あなたは叫び声を耳にした。");
#else
            mpr("You hear a shout.");
#endif
        break;

    //case SPWLD_PRUNE:
    default:
        return;
    }

    if (makes_noise)
        noisy( 25, you.x_pos, you.y_pos );

    if (old_plus != you.inv[wpn].plus
        || old_plus2 != you.inv[wpn].plus2
        || old_colour != you.inv[wpn].colour)
    {
        you.wield_change = true;
    }

    return;
}                               // end special_wielded()

static void reaching_weapon_attack(void)
{
    struct dist beam;
    int x_distance, y_distance;
    int x_middle, y_middle;
    int skill;

#ifdef JP 
    mpr("どれを攻撃しますか？", MSGCH_PROMPT);
#else
    mpr("Attack whom?", MSGCH_PROMPT);
#endif

    direction( beam, DIR_TARGET, TARG_ENEMY );
    if (!beam.isValid)
        return;

    if (beam.isMe)
    {
        canned_msg(MSG_UNTHINKING_ACT);
        return;
    }

    x_distance = abs(beam.tx - you.x_pos);
    y_distance = abs(beam.ty - you.y_pos);

    if (x_distance > 2 || y_distance > 2)
#ifdef JP 
        mpr("あなたの武器はそこまで遠くには伸びない！");
#else
        mpr("Your weapon cannot reach that far!");
#endif
    else if (mgrd[beam.tx][beam.ty] == NON_MONSTER)
    {
#ifdef JP 
        mpr("あなたは何もない空間を攻撃した。");
#else
        mpr("You attack empty space.");
#endif
    }
    else
    {
        /* BCR - Added a check for monsters in the way.  Only checks cardinal
         *       directions.  Knight moves are ignored.  Assume the weapon
         *       slips between the squares.
         */

        // if we're attacking more than a space away
        if ((x_distance > 1) || (y_distance > 1))
        {

#define MAX(x,y) (((x) > (y)) ? (x) : (y))
            x_middle = MAX(beam.tx, you.x_pos) - (x_distance / 2);
            y_middle = MAX(beam.ty, you.y_pos) - (y_distance / 2);
#undef MAX

            // if either the x or the y is the same, we should check for
            // a monster:
            if (((beam.tx == you.x_pos) || (beam.ty == you.y_pos))
                    && (mgrd[x_middle][y_middle] != NON_MONSTER))
            {
                skill = weapon_skill( you.inv[you.equip[EQ_WEAPON]].base_type,
                                      you.inv[you.equip[EQ_WEAPON]].sub_type );

                if ((5 + (3 * skill)) > random2(100))
                {
#ifdef JP 
                    mpr("あなたの攻撃が届いた！");
#else
                    mpr("You reach to attack!");
#endif
                    you_attack(mgrd[beam.tx][beam.ty], false);
                }
                else
                {
#ifdef JP 
                    mpr("あなたの攻撃はそこまで届かなかった。");
#else
                    mpr("You could not reach far enough!");
#endif
                    you_attack(mgrd[x_middle][y_middle], false);
                }
            }
            else
            {
#ifdef JP 
                mpr("あなたの攻撃が届いた！");
#else
                mpr("You reach to attack!");
#endif
                you_attack(mgrd[beam.tx][beam.ty], false);
            }
        }
        else
        {
            you_attack(mgrd[beam.tx][beam.ty], false);
        }
    }

    return;
}                               // end reaching_weapon_attack()

// returns true if item successfully evoked.
bool evoke_wielded( void )
{
    char opened_gates = 0;
    unsigned char spell_casted = random2(21);
    int count_x, count_y;
    int temp_rand = 0;      // for probability determination {dlb}
    int power = 0;

    int pract = 0;
    bool did_work = false;  // used for default "nothing happens" message

    char str_pass[ ITEMNAME_SIZE ];

    int wield = you.equip[EQ_WEAPON];

    if (you.berserker)
    {
        canned_msg( MSG_TOO_BERSERK );
        return (false);
    }
    else if (wield == -1)
    {
#ifdef JP 
        mpr("あなたは何も手にしていない！");
#else
        mpr("You aren't wielding anything!");
#endif
        return (false);
    }

    switch (you.inv[wield].base_type)
    {
    case OBJ_WEAPONS:
        if (get_weapon_brand( you.inv[wield] ) == SPWPN_REACHING
            && enough_mp(1, false))
        {
            // needed a cost to prevent evocation training abuse -- bwr
            dec_mp(1);
            make_hungry( 50, false );  
            reaching_weapon_attack();
            pract = (one_chance_in(5) ? 1 : 0);
            did_work = true;
        }
        else if (is_fixed_artefact( you.inv[wield] ))
        {
            switch (you.inv[wield].special)
            {
            case SPWPN_STAFF_OF_DISPATER:
                if (you.deaths_door || !enough_hp(11, true) 
                    || !enough_mp(5, true))
                {
                    break;
                }

#ifdef JP 
                mpr("杖があなたのエネルギーを消費した！");
#else
                mpr("You feel the staff feeding on your energy!");
#endif

                dec_hp( 5 + random2avg(19, 2), false );
                dec_mp( 2 + random2avg(5, 2) );
                make_hungry( 100, false );

                power = you.skills[SK_EVOCATIONS] * 8;
                your_spells( SPELL_HELLFIRE, power, false );
                pract = (coinflip() ? 2 : 1);
                did_work = true;
                break;

            // let me count the number of ways spell_casted is
            // used here ... one .. two .. three ... >CRUNCH<
            // three licks to get to the center of a ... {dlb}
            case SPWPN_SCEPTRE_OF_ASMODEUS:
                spell_casted = random2(21);

                if (spell_casted == 0)
                    break;

                make_hungry( 200, false );
                pract = 1;

                if (spell_casted < 2)   // summon devils, maybe a Fiend
                {

                    spell_casted = (one_chance_in(4) ? MONS_FIEND
                                                 : MONS_HELLION + random2(10));

                    bool good_summon = (create_monster( spell_casted, 
                                            ENCH_ABJ_VI, BEH_HOSTILE,
                                            you.x_pos, you.y_pos, 
                                            MHITYOU, 250) != -1);

                    if (good_summon)
                    {
                        if (spell_casted == MONS_FIEND)
#ifdef JP 
                            mpr("『人間よ！汝の不敬に有罪を宣告する！』");
#else
                            mpr("\"Your arrogance condemns you, mortal!\"");
#endif
                        else
#ifdef JP 
                            mpr("王笏はしもべのひとつを召換した。");
#else
                            mpr("The Sceptre summons one of its servants.");
#endif
                    }

                    did_work = true;
                    break;
                }

                temp_rand = random2(240);

                if (temp_rand > 125)
                    spell_casted = SPELL_BOLT_OF_FIRE;      // 114 in 240
                else if (temp_rand > 68)
                    spell_casted = SPELL_LIGHTNING_BOLT;    //  57 in 240
                else if (temp_rand > 11)
                    spell_casted = SPELL_BOLT_OF_DRAINING;  //  57 in 240
                else
                    spell_casted = SPELL_HELLFIRE;          //  12 in 240

                power = you.skills[SK_EVOCATIONS] * 8;
                your_spells( spell_casted, power, false );
                did_work = true;
                break;

            case SPWPN_STAFF_OF_OLGREB:
                if (!enough_mp( 4, true ) 
                    || you.skills[SK_EVOCATIONS] < random2(6))
                {
                    break;
                }

                dec_mp(4);
                make_hungry( 50, false );
                pract = 1;
                did_work = true;

                power = 10 + you.skills[SK_EVOCATIONS] * 8;

                your_spells( SPELL_OLGREBS_TOXIC_RADIANCE, power, false );

                if (you.skills[SK_EVOCATIONS] >= random2(10))
                    your_spells( SPELL_VENOM_BOLT, power, false );
                break;

            case SPWPN_STAFF_OF_WUCAD_MU:
                if (you.magic_points == you.max_magic_points
                    || you.skills[SK_EVOCATIONS] < random2(25))
                {
                    break;
                }

#ifdef JP 
                mpr("魔法のエネルギーがあなたの精神を駆け巡る！");
#else
                mpr("Magical energy flows into your mind!");
#endif

                inc_mp( 3 + random2(5) + you.skills[SK_EVOCATIONS] / 3, false );
                make_hungry( 50, false );
                pract = 1;
                did_work = true;

                if (one_chance_in(3))
                {
                    miscast_effect( SPTYP_DIVINATION, random2(9), 
#ifdef JP 
                                    random2(70), 100, "ウカド・ムーの杖" );
#else
                                    random2(70), 100, "the Staff of Wucad Mu" );
#endif
                }
                break;

            default:
                break;
            }
        }
        break;

    case OBJ_STAVES:
        if (item_is_rod( you.inv[wield] ))
        {
            pract = staff_spell( wield );
            did_work = true;  // staff_spell() will handle messages
        } 
        else if (you.inv[wield].sub_type == STAFF_CHANNELING)
        {
            if (you.magic_points < you.max_magic_points 
                && you.skills[SK_EVOCATIONS] >= random2(30)) 
            {
#ifdef JP 
                mpr("あなたは幾らかの魔力を取り込んだ。");
#else
                mpr("You channel some magical energy.");
#endif
                inc_mp( 1 + random2(3), false );
                make_hungry( 50, false );
                pract = (one_chance_in(5) ? 1 : 0);
                did_work = true;

                if (item_not_ident( you.inv[you.equip[EQ_WEAPON]], 
                                    ISFLAG_KNOW_TYPE ))
                {
                    set_ident_flags( you.inv[you.equip[EQ_WEAPON]], 
                                     ISFLAG_KNOW_TYPE );

#ifdef JP 
                    strcpy( info, "あなたは" );
#else
                    strcpy( info, "You are wielding " );
#endif
                    in_name( you.equip[EQ_WEAPON], DESC_NOCAP_A, str_pass );
                    strcat( info, str_pass );
#ifdef JP 
                    strcat( info, "を手にしている。" );
#else
                    strcat( info, "." );
#endif

                    mpr( info );
                    more();

                    you.wield_change = true;
                }
            }
        }
        break;

    case OBJ_MISCELLANY:
        did_work = true; // easier to do it this way for misc items
        switch (you.inv[wield].sub_type)
        {
        case MISC_BOTTLED_EFREET:
            if (efreet_flask())
                pract = 2;
            break;

        case MISC_CRYSTAL_BALL_OF_SEEING:
            if (ball_of_seeing())
                pract = 1;
            break;

        case MISC_AIR_ELEMENTAL_FAN:
            if (you.skills[SK_EVOCATIONS] <= random2(30))
                canned_msg(MSG_NOTHING_HAPPENS);
            else
            {
                summon_elemental(100, MONS_AIR_ELEMENTAL, 4);
                pract = (one_chance_in(5) ? 1 : 0);
            }
            break;

        case MISC_LAMP_OF_FIRE:
            if (you.skills[SK_EVOCATIONS] <= random2(30))
                canned_msg(MSG_NOTHING_HAPPENS);
            else
            {
                summon_elemental(100, MONS_FIRE_ELEMENTAL, 4);
                pract = (one_chance_in(5) ? 1 : 0);
            }
            break;

        case MISC_STONE_OF_EARTH_ELEMENTALS:
            if (you.skills[SK_EVOCATIONS] <= random2(30))
                canned_msg(MSG_NOTHING_HAPPENS);
            else
            {
                summon_elemental(100, MONS_EARTH_ELEMENTAL, 4);
                pract = (one_chance_in(5) ? 1 : 0);
            }
            break;

        case MISC_HORN_OF_GERYON:
            // Note: This assumes that the Vestibule has not been changed.
            if (player_in_branch( BRANCH_VESTIBULE_OF_HELL ))
            {
#ifdef JP 
                mpr("あなたは不気味かつ陰鬱な音色を吹き鳴らした。");
#else
                mpr("You produce a weird and mournful sound.");
#endif

                for (count_x = 0; count_x < GXM; count_x++)
                {
                    for (count_y = 0; count_y < GYM; count_y++)
                    {
                        if (grd[count_x][count_y] == DNGN_STONE_ARCH)
                        {
                            opened_gates++;

                            // this may generate faulty [][] values {dlb}
                            switch (grd[count_x + 2][count_y])
                            {
                            case DNGN_FLOOR:
                                grd[count_x][count_y] = DNGN_ENTER_DIS;
                                break;
                            case DNGN_LAVA:
                                grd[count_x][count_y] = DNGN_ENTER_GEHENNA;
                                break;
                            case DNGN_ROCK_WALL:
                                grd[count_x][count_y] = DNGN_ENTER_TARTARUS;
                                break;
                            case DNGN_DEEP_WATER:
                                grd[count_x][count_y] = DNGN_ENTER_COCYTUS;
                                break;
                            }
                        }
                    }
                }

                if (opened_gates)
                {
#ifdef JP 
                    mpr("あなたの前に門が開かれた。");
#else
                    mpr("Your way has been unbarred.");
#endif
                    pract = 1;
                }
            }
            else
            {
#ifdef JP 
                mpr("あなたは忌まわしくも凄まじい騒音を吹き鳴らした！");
#else
                mpr("You produce a hideous howling noise!");
#endif
                pract = (one_chance_in(3) ? 1 : 0);
                create_monster( MONS_BEAST, ENCH_ABJ_IV, BEH_HOSTILE, 
                                you.x_pos, you.y_pos, MHITYOU, 250 );
            }
            break;

        case MISC_DECK_OF_WONDERS:
            deck_of_cards(DECK_OF_WONDERS);
            pract = 1;
            break;

        case MISC_DECK_OF_SUMMONINGS:
            deck_of_cards(DECK_OF_SUMMONING);
            pract = 1;
            break;

        case MISC_DECK_OF_TRICKS:
            deck_of_cards(DECK_OF_TRICKS);
            pract = 1;
            break;

        case MISC_DECK_OF_POWER:
            deck_of_cards(DECK_OF_POWER);
            pract = 1;
            break;

        case MISC_BOX_OF_BEASTS:
            if (box_of_beasts())
                pract = 1;
            break;

        case MISC_CRYSTAL_BALL_OF_ENERGY:
            if (ball_of_energy())
                pract = 1;
            break;

        case MISC_CRYSTAL_BALL_OF_FIXATION:
            if (ball_of_fixation())
                pract = 1;
            break;

        case MISC_DISC_OF_STORMS:
            if (disc_of_storms())
                pract = (coinflip() ? 2 : 1);
            break;

        case MISC_PORTABLE_ALTAR_OF_NEMELEX:
            if (player_in_branch( BRANCH_ECUMENICAL_TEMPLE ))
            {
#ifdef JP 
                mpr( "既にこの階には十分な数の祭壇があると思わないか？" );
#else
                mpr( "Don't you think this level already has more than "
                     "enough altars?" );
#endif
            }
            else if (grd[you.x_pos][you.y_pos] != DNGN_FLOOR)
#ifdef JP 
                mpr("このアイテムの設置には片付いた空間が必要だ。");
#else
                mpr("You need a clear area to place this item.");
#endif
            else
            {
#ifdef JP 
                mpr("あなたは携帯式祭壇を展開して、床に設置した。");
#else
                mpr("You unfold the altar and place it on the floor.");
#endif
                grd[you.x_pos][you.y_pos] = DNGN_ALTAR_NEMELEX_XOBEH;
                dec_inv_item_quantity( you.equip[EQ_WEAPON], 1 );
            }
            break;

        default:
            did_work = false;
            break;
        }
        break;

    default:
        break;
    }

    if (!did_work)
        canned_msg(MSG_NOTHING_HAPPENS);
    else if (pract > 0)
        exercise( SK_EVOCATIONS, pract );

    you.turn_is_over = 1;

    return (did_work);
}                               // end evoke_wielded()

static bool efreet_flask(void)
{
    const int behaviour = ((you.skills[SK_EVOCATIONS] > random2(20)) 
                                ? BEH_FRIENDLY : BEH_HOSTILE);

#ifdef JP 
    mpr("あなたは瓶を開けた……。");
#else
    mpr("You open the flask...");
#endif

    dec_inv_item_quantity( you.equip[EQ_WEAPON], 1 );

    if (create_monster( MONS_EFREET, ENCH_ABJ_V, behaviour, 
                        you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
    {
#ifdef JP 
        mpr( "……すると巨大なイフリートが中から出てきた。" );
#else
        mpr( "...and a huge efreet comes out." );
#endif

#ifdef JP 
        mpr( (behaviour == BEH_FRIENDLY) ? "「俺を解放してくれてありがとうよ！」"
                                         : "それは狂ったように喚いた！" );
#else
        mpr( (behaviour == BEH_FRIENDLY) ? "\"Thank you for releasing me!\""
                                         : "It howls insanely!" );
#endif
    }
    else
        canned_msg(MSG_NOTHING_HAPPENS);

    return (true);
}                               // end efreet_flask()

static bool ball_of_seeing(void)
{
    int use = 0;
    bool ret = false;

#ifdef JP 
    mpr("あなたは水晶球にじっと見入った。");
#else
    mpr("You gaze into the crystal ball.");
#endif

    use = ((!you.conf) ? random2(you.skills[SK_EVOCATIONS] * 6) : random2(5));

    if (use < 2)
    {
        lose_stat( STAT_INTELLIGENCE, 1 );
    }
    else if (use < 5 && enough_mp(1, true))
    {
#ifdef JP 
        mpr("あなたは魔力が吸い取られていくのを感じた！");
#else
        mpr("You feel power drain from you!");
#endif
        set_mp(0, false);
    }
    else if (use < 10)
    {
        confuse_player( 10 + random2(10), false );
    }
    else if (use < 15
             || you.level_type == LEVEL_LABYRINTH
             || you.level_type == LEVEL_ABYSS || coinflip())
    {
#ifdef JP 
        mpr("あなたには何も見えなかった。");
#else
        mpr("You see nothing.");
#endif
    }
    else
    {
#ifdef JP 
        mpr("あなたは周辺の地図を霊視した！");
#else
        mpr("You see a map of your surroundings!");
#endif
        magic_mapping( 15, 50 + random2( you.skills[SK_EVOCATIONS] ) );
        ret = true;
    }

    return (ret);
}                               // end ball_of_seeing()

static bool disc_of_storms(void)
{
    int temp_rand = 0;          // probability determination {dlb}
    struct bolt beam;
    int disc_count = 0;
    unsigned char which_zap = 0;

    const int fail_rate = (30 - you.skills[SK_EVOCATIONS]);
    bool ret = false;

    if (player_res_electricity() || (random2(100) < fail_rate))
        canned_msg(MSG_NOTHING_HAPPENS);
    else if (random2(100) < fail_rate)
#ifdef JP 
        mpr("円盤は一瞬だけ輝いたが、すぐに光は消えた。");
#else
        mpr("The disc glows for a moment, then fades.");
#endif
    else if (random2(100) < fail_rate)
#ifdef JP 
        mpr("小さな電光が円盤の周りでバチバチと爆ぜた。");
#else
        mpr("Little bolts of electricity crackle over the disc.");
#endif
    else
    {
#ifdef JP 
        mpr("円盤は電気の爆発を放射した！");
#else
        mpr("The disc erupts in an explosion of electricity!");
#endif

        disc_count = roll_dice( 2, 1 + you.skills[SK_EVOCATIONS] / 7 );

        while (disc_count)
        {
            temp_rand = random2(3);

            which_zap = ((temp_rand > 1) ? ZAP_LIGHTNING :
                         (temp_rand > 0) ? ZAP_ELECTRICITY
                                         : ZAP_ORB_OF_ELECTRICITY);

            beam.source_x = you.x_pos;
            beam.source_y = you.y_pos;
            beam.target_x = you.x_pos + random2(13) - 6;
            beam.target_y = you.y_pos + random2(13) - 6;

            zapping( which_zap, 30 + you.skills[SK_EVOCATIONS] * 2, beam );

            disc_count--;
        }

        ret = true;
    }

    return (ret);
}                               // end disc_of_storms()

void tome_of_power(char sc_read_2)
{
    int temp_rand = 0;          // probability determination {dlb}

    int powc = 5 + you.skills[SK_EVOCATIONS] 
                 + roll_dice( 5, you.skills[SK_EVOCATIONS] ); 

    int spell_casted = 0;
    struct bolt beam;

#ifdef JP 
    strcpy(info, "本は");
#else
    strcpy(info, "The book opens to a page covered in ");
#endif

    char wc[30];
    weird_writing( wc );
    strcat( info, wc );
#ifdef JP 
    strcat( info, "で書かれたページを開いた。" );
#else
    strcat( info, "." );
#endif
    mpr( info );

    you.turn_is_over = 1;

#ifdef JP 
    if (!yesno("読みますか？"))
#else
    if (!yesno("Read it?"))
#endif
        return;

    set_ident_flags( you.inv[sc_read_2], ISFLAG_IDENT_MASK );

    if (you.mutation[MUT_BLURRY_VISION] > 0
        && random2(4) < you.mutation[MUT_BLURRY_VISION])
    {
#ifdef JP 
        mpr("このページは文字がかすれていて読めない。");
#else
        mpr("The page is too blurry for you to read.");
#endif
        return;
    }

#ifdef JP 
    mpr("あなたは気がつくと魔法の言葉を詠唱していた！");
#else
    mpr("You find yourself reciting the magical words!");
#endif
    exercise( SK_EVOCATIONS, 1 ); 

    temp_rand = random2(50) + random2( you.skills[SK_EVOCATIONS] / 3 ); 

    switch (random2(50))
    {
    case 0:
    case 3:
    case 4:
    case 6:
    case 7:
    case 8:
    case 9:
#ifdef JP 
        mpr("気味悪い煙の雲が本のページから立ち昇った！");
#else
        mpr("A cloud of weird smoke pours from the book's pages!");
#endif
        big_cloud( CLOUD_GREY_SMOKE + random2(3), you.x_pos, you.y_pos, 20,
                                                          10 + random2(8) );
        return;
    case 1:
    case 14:
#ifdef JP 
        mpr("窒息ガスだ！");
#else
        mpr("A cloud of choking fumes pours from the book's pages!");
#endif
        big_cloud(CLOUD_POISON, you.x_pos, you.y_pos, 20, 7 + random2(5));
        return;

    case 2:
    case 13:
#ifdef JP 
        mpr("凍結ガスだ！");
#else
        mpr("A cloud of freezing gas pours from the book's pages!");
#endif
        big_cloud(CLOUD_COLD, you.x_pos, you.y_pos, 20, 8 + random2(5));
        return;

    case 5:
    case 11:
    case 12:
        if (one_chance_in(5))
        {
#ifdef JP 
            mpr("本は強烈な爆発で消し飛んだ！");
#else
            mpr("The book disappears in a mighty explosion!");
#endif
            dec_inv_item_quantity( sc_read_2, 1 );
        }

        beam.type = SYM_BURST;
        beam.damage = dice_def( 3, 15 );
        // unsure about this    // BEAM_EXPLOSION instead? [dlb]
        beam.flavour = BEAM_FIRE;
        beam.target_x = you.x_pos;
        beam.target_y = you.y_pos;
#ifdef JP 
        strcpy( beam.beam_name, "炎の爆発" );
#else
        strcpy( beam.beam_name, "fiery explosion" );
#endif
        beam.colour = RED;
        // your explosion, (not someone else's explosion)
        beam.beam_source = NON_MONSTER;
        beam.thrower = KILL_YOU;
#ifdef JP 
        beam.aux_source = "力の書の爆発";
#else
        beam.aux_source = "an exploding Tome of Power";
#endif
        beam.ex_size = 2;
        beam.isTracer = false;

        explosion(beam);
        return;


    case 10:
        if (create_monster( MONS_ABOMINATION_SMALL, ENCH_ABJ_VI, BEH_HOSTILE,
                            you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
        {
#ifdef JP 
            mpr("おぞましい存在が現れた！");
            mpr("それはとても友好的には見えない。");
#else
            mpr("A horrible Thing appears!");
            mpr("It doesn't look too friendly.");
#endif
        }
        return;
    }

    viewwindow(1, false);

    temp_rand = random2(23) + random2( you.skills[SK_EVOCATIONS] / 3 );

    if (temp_rand > 25)
        temp_rand = 25;

    spell_casted = ((temp_rand > 19) ? SPELL_FIREBALL :
                    (temp_rand > 16) ? SPELL_BOLT_OF_FIRE :
                    (temp_rand > 13) ? SPELL_BOLT_OF_COLD :
                    (temp_rand > 11) ? SPELL_LIGHTNING_BOLT :
                    (temp_rand > 10) ? SPELL_LEHUDIBS_CRYSTAL_SPEAR :
                    (temp_rand >  9) ? SPELL_VENOM_BOLT :
                    (temp_rand >  8) ? SPELL_BOLT_OF_DRAINING :
                    (temp_rand >  7) ? SPELL_BOLT_OF_INACCURACY :
                    (temp_rand >  6) ? SPELL_STICKY_FLAME :
                    (temp_rand >  5) ? SPELL_TELEPORT_SELF :
                    (temp_rand >  4) ? SPELL_CIGOTUVIS_DEGENERATION :
                    (temp_rand >  3) ? SPELL_POLYMORPH_OTHER :
                    (temp_rand >  2) ? SPELL_MEPHITIC_CLOUD :
                    (temp_rand >  1) ? SPELL_THROW_FLAME :
                    (temp_rand >  0) ? SPELL_THROW_FROST
                                     : SPELL_MAGIC_DART);

    your_spells( spell_casted, powc, false );
}                               // end tome_of_power()

void skill_manual(char sc_read_2)
{
    char skname[30];

    set_ident_flags( you.inv[sc_read_2], ISFLAG_IDENT_MASK );

    strcpy(skname, skill_name(you.inv[sc_read_2].plus));

#ifdef JP 
    strcpy(info, "これは");
#else
    strcpy(info, "This is a manual of ");
#endif
    strcat(info, skname);
#ifdef JP 
    strcat(info, "の虎の巻だ！");
#else
    strcat(info, "!");
#endif
    mpr(info);

    you.turn_is_over = 1;

#ifdef JP 
    if (!yesno("読みますか？"))
#else
    if (!yesno("Read it?"))
#endif
        return;

#ifdef JP 
    strcpy(info, "あなたは");
    strcat(info, skname);
    strcat(info, "について読んだ。");
#else
    strcpy(info, "You read about ");
    strcat(info, strlwr(skname));
    strcat(info, ".");
#endif
    mpr(info);

    exercise( you.inv[sc_read_2].plus, 500 );

    if (one_chance_in(10))
    {
#ifdef JP 
        mpr("本は塵となって崩れてしまった。");
#else
        mpr("The book crumbles into dust.");
#endif
        dec_inv_item_quantity( sc_read_2, 1 );
    }
    else
    {
#ifdef JP 
        mpr("本は前よりも幾らか擦り切れたようだ。");
#else
        mpr("The book looks somewhat more worn.");
#endif
    }
}                               // end skill_manual()

static bool box_of_beasts(void)
{
    int beasty = MONS_PROGRAM_BUG;      // error trapping {dlb}
    int temp_rand = 0;          // probability determination {dlb}

    int ret = false;

#ifdef JP 
    mpr("あなたは蓋を開いた……。");
#else
    mpr("You open the lid...");
#endif

    if (random2(100) < 60 + you.skills[SK_EVOCATIONS])
    {
        temp_rand = random2(11);

        beasty = ((temp_rand == 0) ? MONS_GIANT_BAT :
                  (temp_rand == 1) ? MONS_HOUND :
                  (temp_rand == 2) ? MONS_JACKAL :
                  (temp_rand == 3) ? MONS_RAT :
                  (temp_rand == 4) ? MONS_ICE_BEAST :
                  (temp_rand == 5) ? MONS_SNAKE :
                  (temp_rand == 6) ? MONS_YAK :
                  (temp_rand == 7) ? MONS_BUTTERFLY :
                  (temp_rand == 8) ? MONS_HELL_HOUND :
                  (temp_rand == 9) ? MONS_BROWN_SNAKE 
                                   : MONS_GIANT_LIZARD);

        int beh = (one_chance_in(you.skills[SK_EVOCATIONS] + 5) ? BEH_HOSTILE
                                                                : BEH_FRIENDLY);

        if (create_monster( beasty, ENCH_ABJ_II + random2(4), beh,
                            you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
        {
#ifdef JP 
            mpr("……すると何かが飛び出してきた！");
#else
            mpr("...and something leaps out!");
#endif
            ret = true;
        }
    }
    else
    {
        if (!one_chance_in(6))
#ifdef JP 
            mpr("……しかし何事も起こらなかった！");
#else
            mpr("...but nothing happens.");
#endif
        else
        {
#ifdef JP 
            mpr("……しかし匣の中は空のようだ。");
#else
            mpr("...but the box appears empty.");
#endif
            you.inv[you.equip[EQ_WEAPON]].sub_type = MISC_EMPTY_EBONY_CASKET;
        }
    }

    return (ret);
}                               // end box_of_beasts()

static bool ball_of_energy(void)
{
    int use = 0;
    int proportional = 0;

    bool ret = false;

#ifdef JP 
    mpr("あなたは水晶球にじっと見入った。");
#else
    mpr("You gaze into the crystal ball.");
#endif

    use = ((!you.conf) ? random2(you.skills[SK_EVOCATIONS] * 6) : random2(6));

    if (use < 2 || you.max_magic_points == 0)
    {
        lose_stat(STAT_INTELLIGENCE, 1);
    }
    else if ((use < 4 && enough_mp(1, true))
             || you.magic_points == you.max_magic_points)
    {
#ifdef JP 
        mpr( "あなたは魔力が吸い取られていくのを感じた！" );
#else
        mpr( "You feel your power drain away!" );
#endif
        set_mp( 0, false );
    }
    else if (use < 6)
    {
        confuse_player( 10 + random2(10), false );
    }
    else
    {
        proportional = you.magic_points * 100;
        proportional /= you.max_magic_points;

        if (random2avg(77 - you.skills[SK_EVOCATIONS] * 2, 4) > proportional 
            || one_chance_in(25))
        {
#ifdef JP 
            mpr( "あなたは魔力が吸い取られていくのを感じた！" );
#else
            mpr( "You feel your power drain away!" );
#endif
            set_mp( 0, false );
        }
        else
        {
#ifdef JP 
            mpr( "あなたは魔力に満たされた！" );
#else
            mpr( "You are suffused with power!" );
#endif
            inc_mp( 6 + roll_dice( 2, you.skills[SK_EVOCATIONS] ), false );

            ret = true;
        }
    }

    return (ret);
}                               // end ball_of_energy()

static bool ball_of_fixation(void)
{
#ifdef JP 
    mpr("あなたは水晶球にじっと見入った。");
    mpr("あなたは煌く虹に幻惑されてしまった！");
#else
    mpr("You gaze into the crystal ball.");
    mpr("You are mesmerised by a rainbow of scintillating colours!");
#endif

    you.paralysis = 100;
    you.slow = 100;

    return (true);
}                               // end ball_of_fixation()
