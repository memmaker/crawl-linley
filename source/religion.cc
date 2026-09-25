/*
 *  File:       religion.cc
 *  Summary:    Misc religion related functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *
 *   <7>   11jan2001     gdl    added M. Valvoda's changes to
 *                              god_colour() and god_name()
 *   <6>   06-Mar-2000   bwr    added penance, gift_timeout,
 *                              divine_retribution(), god_speaks()
 *   <5>   11/15/99      cdl    Fixed Daniel's yellow Xom patch  :)
 *                              Xom will sometimes answer prayers
 *   <4>   10/11/99      BCR    Added Daniel's yellow Xom patch
 *   <3>    6/13/99      BWR    Vehumet book giving code.
 *   <2>    5/20/99      BWR    Added screen redraws
 *   <1>    -/--/--      LRH    Created
 */

#include "AppHdr.h"
#include "religion.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "externs.h"

#include "abl-show.h"
#include "beam.h"
#include "debug.h"
#include "decks.h"
#include "describe.h"
#include "dungeon.h"
#include "effects.h"
#include "food.h"
#include "it_use2.h"
#include "itemname.h"
#include "items.h"
#include "misc.h"
#include "monplace.h"
#include "mutation.h"
#include "newgame.h"
#include "ouch.h"
#include "player.h"
#include "shopping.h"
#include "skills2.h"
#include "spells1.h"
#include "spells2.h"
#include "spells3.h"
#include "spl-cast.h"
#include "stuff.h"

const char *sacrifice[] = {
#ifdef JP
    "は銀色に輝いて消え去った。",
    "は煌く金色に輝いて消え去った。",
    "は一瞬のうちに朽ち果ててしまった。",
    "は塵へと分解された。",
    "は虫に食べられた。",    /* Xom - no sacrifices */
    "は爆発して消え去った。",
    "は炎の爆発で焼き尽された。",
    "は唸りを上げる炎の柱に焼き尽された。",
    "は一瞬だけかすかに輝くと消え去った。",
    "は唸りを上げる炎の柱に焼き尽された。",
    "は気味悪い色彩の虹に輝くと消え去った。",
    "は蒸発した。"
#else
    " glows silver and disappears.",
    " glows a brilliant golden colour and disappears.",
    " rots away in an instant.",
    " crumbles to dust.",
    " is eaten by a bug.",    /* Xom - no sacrifices */
    " explodes into nothingness.",
    " is consumed in a burst of flame.",
    " is consumed in a roaring column of flame.",
    " glows faintly for a moment, then is gone.",
    " is consumed in a roaring column of flame.",
    " glows with a rainbow of weird colours and disappears.",
    " evaporates."
#endif
};

void altar_prayer(void);
void dec_penance(int god, int val);
void divine_retribution(int god);
void inc_penance(int god, int val);
void inc_penance(int val);

void dec_penance(int god, int val)
{
    if (you.penance[god] > 0)
    {
        if (you.penance[god] <= val)
        {
#ifdef JP
            simple_god_message("は怒りを和らげたようだ。", god);
#else
            simple_god_message(" seems mollified.", god);
#endif
            you.penance[god] = 0;
        }
        else
            you.penance[god] -= val;
    }
}                               // end dec_penance()

void dec_penance(int val)
{
    dec_penance(you.religion, val);
}                               // end dec_penance()

void inc_penance(int god, int val)
{
    if ((int) you.penance[god] + val > 200)
        you.penance[god] = 200;
    else
        you.penance[god] += val;
}                               // end inc_penance()

void inc_penance(int val)
{
    inc_penance(you.religion, val);
}                               // end inc_penance()

static void inc_gift_timeout(int val)
{
    if ((int) you.gift_timeout + val > 200)
        you.gift_timeout = 200;
    else
        you.gift_timeout += val;
}                               // end inc_gift_timeout()

void pray(void)
{
    int            temp_rand = 0;
    unsigned char  was_praying = you.duration[DUR_PRAYER];
    bool           success = false;

    if (silenced(you.x_pos, you.y_pos))
    {
#ifdef JP
        mpr("あなたは声を上げることができない！");
#else
        mpr("You are unable to make a sound!");
#endif
        return;
    }

    // all prayers take time
    you.turn_is_over = 1;

    if (you.religion != GOD_NO_GOD
            && grd[you.x_pos][you.y_pos] == 179 + you.religion)
    {
        altar_prayer();
    }
    else if (grd[you.x_pos][you.y_pos] >= 180
                && grd[you.x_pos][you.y_pos] <= 199)
    {
        if (you.species == SP_DEMIGOD)
        {
#ifdef JP
            mpr("残念ながら、あなたのような地位にある存在はここで祈れない。");
#else
            mpr("Sorry, a being of your status cannot worship here.");
#endif
            return;
        }
        god_pitch(grd[you.x_pos][you.y_pos] - 179);
        return;
    }

    if (you.religion == GOD_NO_GOD)
    {
#ifdef JP
        strcpy(info, "あなたは一瞬、");
#else
        strcpy(info, "You spend a moment contemplating the meaning of ");
#endif

        if (you.is_undead)
#ifdef JP
            strcat(info, "死せることの意味について考えた。");
#else
            strcat(info, "un");
#endif

#ifdef JP
        else
            strcat(info, "生きることの意味について考えた。");
#else
        strcat(info, "life.");
#endif
        mpr(info);
        return;
    }
    else if (you.religion == GOD_XOM)
    {
        if (one_chance_in(100))
        {
            // Every now and then, Xom listens
            // This is for flavour, not effect, so praying should not be
            // encouraged.

            // Xom is nicer to experienced players
            bool nice = (27 <= random2( 27 + you.experience_level ));

            // and he's not very nice even then
            int sever = (nice) ? random2( random2( you.experience_level ) )
                               : you.experience_level;

            // bad results are enforced, good are not
            bool force = !nice;

            Xom_acts( nice, 1 + sever, force );
        }
        else
#ifdef JP
            mpr("ゾムはあなたを顧みない。");
#else
            mpr("Xom ignores you.");
#endif

        return;
    }

#ifdef JP
    strcpy( info, "あなたは" );
#else
    strcpy( info, "You offer a prayer to " );
#endif
    strcat( info, god_name( you.religion ) );
#ifdef JP
    strcat( info, "に祈りを捧げた。" );
#else
    strcat( info, "." );
#endif
    mpr(info);

    you.duration[DUR_PRAYER] = 9 + (random2(you.piety) / 20)
                                            + (random2(you.piety) / 20);

    if (player_under_penance())
#ifdef JP
        simple_god_message("は贖罪を求めている！");
#else
        simple_god_message(" demands penance!");
#endif
    else
    {
        strcpy(info, god_name(you.religion));
#ifdef JP
        strcat(info, "は");
#else
        strcat(info, " is ");
#endif

#ifdef JP
        strcat(info, (you.piety > 130) ? "あなたの崇拝を誇りにしている" :
                     (you.piety > 100) ? "あなたに極めて満足している" :
                     (you.piety >  70) ? "あなたに大いに満足している" :
                     (you.piety >  40) ? "あなたにかなり満足している" :
                     (you.piety >  20) ? "あなたに満足している" :
                     (you.piety >   5) ? "あなたに特別な関心を持っていない"
                                       : "あなたに不満がある");
#else
        strcat(info, (you.piety > 130) ? "exalted by your worship" :
                     (you.piety > 100) ? "extremely pleased with you" :
                     (you.piety >  70) ? "greatly pleased with you" :
                     (you.piety >  40) ? "most pleased with you" :
                     (you.piety >  20) ? "pleased with you" :
                     (you.piety >   5) ? "noncommittal"
                                       : "displeased");
#endif

#ifdef JP
        strcat(info, "。");
#else
        strcat(info, ".");
#endif
        god_speaks(you.religion, info);

        if (you.piety > 130)
            you.duration[DUR_PRAYER] *= 3;
        else if (you.piety > 70)
            you.duration[DUR_PRAYER] *= 2;
    }

#if DEBUG_DIAGNOSTICS
#ifdef JP
    snprintf( info, INFO_SIZE, "piety: %d", you.piety );
#else
    snprintf( info, INFO_SIZE, "piety: %d", you.piety );
#endif
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    // Consider a gift if we don't have a timeout and weren't
    // already praying when we prayed.
    if (!you.penance[you.religion] && !you.gift_timeout && !was_praying)
    {
        //   Remember to check for water/lava
        //jmf: "good" god will sometimes feed you (a la Nethack)
        if (you.religion == GOD_ZIN
            && you.hunger_state == HS_STARVING
            && random2(250) <= you.piety)
        {
#ifdef JP
            god_speaks(you.religion, "あなたの空腹は満たされた。");
#else
            god_speaks(you.religion, "Your stomach feels content.");
#endif
            set_hunger(6000, true);
            lose_piety(5 + random2avg(10, 2));
            inc_gift_timeout(30 + random2avg(10, 2));
            return;
        }

        if (you.religion == GOD_NEMELEX_XOBEH
            && random2(200) <= you.piety
            && (!you.attribute[ATTR_CARD_TABLE] || one_chance_in(3))
            && !you.attribute[ATTR_CARD_COUNTDOWN]
            && grd[you.x_pos][you.y_pos] != DNGN_LAVA
            && grd[you.x_pos][you.y_pos] != DNGN_DEEP_WATER)
        {
            int thing_created = NON_ITEM;
            unsigned char gift_type = MISC_DECK_OF_TRICKS;

            if (!you.attribute[ATTR_CARD_TABLE])
            {
                thing_created = items( 1, OBJ_MISCELLANY,
                                       MISC_PORTABLE_ALTAR_OF_NEMELEX,
                                       true, 1, 250 );

                if (thing_created != NON_ITEM)
                    you.attribute[ATTR_CARD_TABLE] = 1;
            }
            else
            {
                if (random2(200) <= you.piety && one_chance_in(4))
                    gift_type = MISC_DECK_OF_SUMMONINGS;
                if (random2(200) <= you.piety && coinflip())
                    gift_type = MISC_DECK_OF_WONDERS;
                if (random2(200) <= you.piety && one_chance_in(4))
                    gift_type = MISC_DECK_OF_POWER;

                thing_created = items( 1, OBJ_MISCELLANY, gift_type,
                                       true, 1, 250 );
            }

            if (thing_created != NON_ITEM)
            {
                move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

#ifdef JP
                simple_god_message("はあなたに贈り物を授けた！");
#else
                simple_god_message(" grants you a gift!");
#endif
                more();
                canned_msg(MSG_SOMETHING_APPEARS);

                you.attribute[ATTR_CARD_COUNTDOWN] = 10;
                inc_gift_timeout(5 + random2avg(9, 2));
            }
        }

        if ((you.religion == GOD_OKAWARU || you.religion == GOD_TROG)
            && you.piety > 130
            && random2(you.piety) > 120
            && grd[you.x_pos][you.y_pos] != DNGN_LAVA
            && grd[you.x_pos][you.y_pos] != DNGN_DEEP_WATER
            && one_chance_in(4))
        {
            if (you.religion == GOD_TROG
                || (you.religion == GOD_OKAWARU && coinflip()))
            {
                success = acquirement(OBJ_WEAPONS);
            }
            else
            {
                success = acquirement(OBJ_ARMOUR);
            }

            if (success)
            {
#ifdef JP
                simple_god_message("はあなたに贈り物を授けた！");
#else
                simple_god_message(" has granted you a gift!");
#endif
                more();

                inc_gift_timeout(30 + random2avg(19, 2));
            }
        }

        if (you.religion == GOD_YREDELEMNUL
            && random2(you.piety) > 80 && one_chance_in(5))
        {
            int thing_called = MONS_PROGRAM_BUG;  // error trapping {dlb}

            temp_rand = random2(100);
            thing_called = ((temp_rand > 66) ? MONS_WRAITH :            // 33%
                            (temp_rand > 52) ? MONS_WIGHT :             // 12%
                            (temp_rand > 40) ? MONS_SPECTRAL_WARRIOR :  // 16%
                            (temp_rand > 31) ? MONS_ROTTING_HULK :      //  9%
                            (temp_rand > 23) ? MONS_SKELETAL_WARRIOR :  //  8%
                            (temp_rand > 16) ? MONS_VAMPIRE :           //  7%
                            (temp_rand > 10) ? MONS_GHOUL :             //  6%
                            (temp_rand >  4) ? MONS_MUMMY               //  6%
                                             : MONS_FLAYED_GHOST);      //  5%

            if (create_monster( thing_called, 0, BEH_FRIENDLY,
                                you.x_pos, you.y_pos,
                                you.pet_target, 250 ) != -1)
            {
#ifdef JP
                simple_god_message("はあなたにアンデッドの従僕を授けた！");
#else
                simple_god_message(" grants you an undead servant!");
#endif
                more();
                inc_gift_timeout(4 + random2avg(7, 2));
            }
        }

        if ((you.religion == GOD_KIKUBAAQUDGHA
                || you.religion == GOD_SIF_MUNA
                || you.religion == GOD_VEHUMET)
            && you.piety > 160 && random2(you.piety) > 100)
        {
            unsigned int gift = NUM_BOOKS;

            switch (you.religion)
            {
            case GOD_KIKUBAAQUDGHA:     // gives death books
                if (!you.had_book[BOOK_NECROMANCY])
                    gift = BOOK_NECROMANCY;
                else if (!you.had_book[BOOK_DEATH])
                    gift = BOOK_DEATH;
                else if (!you.had_book[BOOK_UNLIFE])
                    gift = BOOK_UNLIFE;
                else if (!you.had_book[BOOK_NECRONOMICON])
                    gift = BOOK_NECRONOMICON;
                break;

            case GOD_SIF_MUNA:
                gift = OBJ_RANDOM;      // Sif Muna - gives any
                break;

            // Vehumet - gives conj/summ. books (higher skill first)
            case GOD_VEHUMET:
                if (!you.had_book[BOOK_CONJURATIONS_I])
                    gift = give_first_conjuration_book();
                else if (!you.had_book[BOOK_POWER])
                    gift = BOOK_POWER;
                else if (!you.had_book[BOOK_ANNIHILATIONS])
                    gift = BOOK_ANNIHILATIONS;  // conj books

                if (you.skills[SK_CONJURATIONS] < you.skills[SK_SUMMONINGS]
                    || gift == NUM_BOOKS)
                {
                    if (!you.had_book[BOOK_CALLINGS])
                        gift = BOOK_CALLINGS;
                    else if (!you.had_book[BOOK_SUMMONINGS])
                        gift = BOOK_SUMMONINGS;
                    else if (!you.had_book[BOOK_DEMONOLOGY])
                        gift = BOOK_DEMONOLOGY; // summoning bks
                }
                break;
            }

            if (gift != NUM_BOOKS
                && (grd[you.x_pos][you.y_pos] != DNGN_LAVA
                    && grd[you.x_pos][you.y_pos] != DNGN_DEEP_WATER))
            {
                if (gift == OBJ_RANDOM)
                    success = acquirement(OBJ_BOOKS);
                else
                {
                    int thing_created = items(1, OBJ_BOOKS, gift, true, 1, 250);
                    if (thing_created == NON_ITEM)
                        return;

                    move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

                    if (thing_created != NON_ITEM)
                        success = true;
                }

                if (success)
                {
#ifdef JP
                    simple_god_message("はあなたに贈り物を授けた！");
#else
                    simple_god_message(" has granted you a gift!");
#endif
                    more();

                    inc_gift_timeout(40 + random2avg(19, 2));
                }


                // Vehumet gives books less readily
                if (you.religion == GOD_VEHUMET && success)
                    inc_gift_timeout(10 + random2(10));
            }                   // end of giving book
        }                       // end of book gods
    }                           // end of gift giving
}                               // end pray()

char *god_name( int which_god, bool long_name ) // mv - rewritten
{
    static char godname_buff[80];

    switch (which_god)
    {
    case GOD_NO_GOD:
#ifdef JP
        sprintf(godname_buff, "無信仰");
#else
        sprintf(godname_buff, "No God");
#endif
        break;
    case GOD_ZIN:
#ifdef JP
        sprintf(godname_buff, "%sジン", long_name ? "秩序をもたらすもの" : "");
#else
        sprintf(godname_buff, "Zin%s", long_name ? " the Law-Giver" : "");
#endif
        break;
    case GOD_SHINING_ONE:
#ifdef JP
        sprintf(godname_buff, "輝けるもの");
#else
        sprintf(godname_buff, "The Shining One");
#endif
        break;
    case GOD_KIKUBAAQUDGHA:
#ifdef JP
        strcpy(godname_buff, "キクバークッグァ");
#else
        strcpy(godname_buff, "Kikubaaqudgha");
#endif
        break;
    case GOD_YREDELEMNUL:
#ifdef JP
        sprintf(godname_buff, "%sイレデレンヌル", long_name ? "暗黒の" : "");
#else
        sprintf(godname_buff, "Yredelemnul%s", long_name ? " the Dark" : "");
#endif
        break;
    case GOD_XOM:
#ifdef JP
        strcpy(godname_buff, "ゾム");
#else
        strcpy(godname_buff, "Xom");
#endif
        if (long_name)
        {
#ifdef JP
            strcat(godname_buff, "");
#else
            strcat(godname_buff, " ");
#endif
            switch(random2(1000))
            {
            default:
#ifdef JP
                strcpy(godname_buff, "混沌のゾム");
#else
                strcat(godname_buff, "of Chaos");
#endif
                break;
            case 1:
#ifdef JP
                strcpy(godname_buff, "無秩序のゾム");
#else
                strcat(godname_buff, "the Random");
#endif
                if (coinflip())
#ifdef JP
                    strcpy(godname_buff, coinflip()?"無秩序のあるじゾム":"乱数の神ゾム");
#else
                    strcat(godname_buff, coinflip()?"master":" Number God");
#endif
                break;
            case 2:
#ifdef JP
                strcpy(godname_buff, "トリックのゾム");
#else
                strcat(godname_buff, "the Tricky");
#endif
                break;
            case 3:
#ifdef JP
                sprintf( godname_buff, "%sのゾム", coinflip() ? "予測不可能"
                                                                    : "予測不能" );
#else
                sprintf( godname_buff, "Xom the %sredictible", coinflip() ? "Less-P"
                                                                    : "Unp" );
#endif
                break;
            case 4:
#ifdef JP
                strcpy(godname_buff, "あまたの扉のゾム");
#else
                strcat(godname_buff, "of Many Doors");
#endif
                break;
            case 5:
#ifdef JP
                strcpy(godname_buff, "気紛れのゾム");
#else
                strcat(godname_buff, "the Capricious");
#endif
                break;
            case 6:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "血塗られし奇想のゾム" : "奇想の執行者ゾム");
#else
                strcat(godname_buff, "of ");
                strcat(godname_buff, coinflip() ? "Bloodstained" : "Enforced");
                strcat(godname_buff, " Whimsey");
#endif
                break;
            case 7:
#ifdef JP
                strcpy(godname_buff, "ゾム『お前のユーザー名なんだったけ？』*カタカタカタ*");
#else
                strcat(godname_buff, "\"What was your username?\" *clickity-click*");
#endif
                break;
            case 8:
#ifdef JP
                strcpy(godname_buff, "残忍なる機知のゾム");
#else
                strcat(godname_buff, "of Bone-Dry Humour");
#endif
                break;
            case 9:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "邪悪なる戯れのゾム" : "悪意ある冗談のゾム");
#else
                strcat(godname_buff, "of ");
                strcat(godname_buff, coinflip() ? "Malevolent" : "Malicious");
                strcat(godname_buff, " Giggling");
#endif
                break;
            case 10:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "狂気のゾム" : "精神病質者ゾム");
#else
                strcat(godname_buff, "the Psycho");
                strcat(godname_buff, coinflip() ? "tic" : "path");
#endif
                break;
            case 11:
#ifdef JP
#else
                strcat(godname_buff, "of ");
#endif
                switch(random2(5))
                {
#ifdef JP
                case 0: strcpy(godname_buff, "戯れのゾム"); break;
                case 1: strcpy(godname_buff, "恐るべきゾム"); break;
                case 2: strcpy(godname_buff, "移り気のゾム"); break;
                case 3: strcpy(godname_buff, "前言撤回のゾム"); break;
                case 4: strcpy(godname_buff, "知られざる意図のゾム"); break;
#else
                case 0: strcat(godname_buff, "Gnomic"); break;
                case 1: strcat(godname_buff, "Ineffable"); break;
                case 2: strcat(godname_buff, "Fickle"); break;
                case 3: strcat(godname_buff, "Swiftly Tilting"); break;
                case 4: strcat(godname_buff, "Unknown"); break;
#endif
                }
#ifdef JP
                strcat(godname_buff, "");
#else
                strcat(godname_buff, " Intent");
#endif
                if (coinflip())
#ifdef JP
                    strcat(godname_buff, "");
#else
                    strcat(godname_buff, "ion");
#endif
                break;
            case 12:
#ifdef JP
                sprintf(godname_buff, "ゾム・メイスター");
#else
                sprintf(godname_buff, "The Xom-Meister");
#endif
                if (coinflip())
#ifdef JP
                    strcat(godname_buff, " ゾム-ア-ロム-ア-ディン-ドン");
#else
                    strcat(godname_buff, ", Xom-a-lom-a-ding-dong");
#endif
                else if (coinflip())
#ifdef JP
                    strcat(godname_buff, " ゾム-オ-ラマ");
#else
                    strcat(godname_buff, ", Xom-o-Rama");
#endif
                else if (coinflip())
#ifdef JP
                    strcat(godname_buff, " ゾム-ゾム-ボ-ボン バナナ-ファナ-フォ-フォム");
#else
                    strcat(godname_buff, ", Xom-Xom-bo-Bom, Banana-Fana-fo-Fom");
#endif
                break;
            case 13:
#ifdef JP
                strcpy(godname_buff, coinflip() ? "争乱をもたらすものゾム" :
                 "無秩序をもたらすものゾム");
#else
                strcat(godname_buff, "the Begetter of ");
                strcat(godname_buff, coinflip() ? "Turbulence" : "Discontinuities");
#endif
                break;
            }
        }
        break;
    case GOD_VEHUMET:
#ifdef JP
        strcpy(godname_buff, "ヴェフメット");
#else
        strcpy(godname_buff, "Vehumet");
#endif
        break;
    case GOD_OKAWARU:
#ifdef JP
        sprintf(godname_buff, "%sオカワル", long_name ? "軍神" : "");
#else
        sprintf(godname_buff, "%sOkawaru", long_name ? "Warmaster " : "");
#endif
        break;
    case GOD_MAKHLEB:
#ifdef JP
        sprintf(godname_buff, "%sマクレブ", long_name ? "破壊者" : "");
#else
        sprintf(godname_buff, "Makhleb%s", long_name ? " the Destroyer" : "");
#endif
        break;
    case GOD_SIF_MUNA:
#ifdef JP
        sprintf(godname_buff, "%sシフ・ムーナ", long_name ? "知識の守護者" : "");
#else
        sprintf(godname_buff, "Sif Muna%s", long_name ? " the Loreminder" : "");
#endif
        break;
    case GOD_TROG:
#ifdef JP
        sprintf(godname_buff, "%sトログ", long_name ? "怒れる" : "");
#else
        sprintf(godname_buff, "Trog%s", long_name ? " the Wrathful" : "");
#endif
        break;
    case GOD_NEMELEX_XOBEH:
#ifdef JP
        sprintf(godname_buff, "ネメレクス%s", long_name ? "・ソベー" : "");
#else
        strcpy(godname_buff, "Nemelex Xobeh");
#endif
        break;
    case GOD_ELYVILON:
#ifdef JP
        sprintf(godname_buff, "%sエリヴィロン", long_name ? "癒し手" : "");
#else
        sprintf(godname_buff, "Elyvilon%s", long_name ? " the Healer" : "");
#endif
        break;
    default:
#ifdef JP
        sprintf(godname_buff, "『バグれるもの』(%d)", which_god);
#else
        sprintf(godname_buff, "The Buggy One (%d)", which_god);
#endif
    }

    return (godname_buff);
}                               // end god_name()

void god_speaks( int god, const char *mesg )
{
    mpr( mesg, MSGCH_GOD, god );
}                               // end god_speaks()

void Xom_acts(bool niceness, int sever, bool force_sever)
{
    // niceness = false - bad, true - nice
    int temp_rand;              // probability determination {dlb}
    bool done_bad = false;      // flag to clarify logic {dlb}
    bool done_good = false;     // flag to clarify logic {dlb}

    struct bolt beam;

    if (sever < 1)
        sever = 1;

    if (!force_sever)
        sever = random2(sever);

    if (sever == 0)
        return;

  okay_try_again:

    if (!niceness || one_chance_in(3))
    {
        // begin "Bad Things"
        done_bad = false;

        // this should always be first - it will often be called
        // deliberately, with a low sever value
        if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

#ifdef JP
            god_speaks(GOD_XOM,
                (temp_rand == 0) ? "ゾムはあなたに目をとめた。" :
                (temp_rand == 1) ? "ゾムは一瞬だけあなたに注目した。":
                (temp_rand == 2) ? "ゾムの力が一瞬だけあなたに触れた。"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
            god_speaks(GOD_XOM,
                (temp_rand == 0) ? "Xom notices you." :
                (temp_rand == 1) ? "Xom's attention turns to you for a moment.":
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            miscast_effect( SPTYP_RANDOM, 5 + random2(10), random2(100), 0,
#ifdef JP
                            "ゾムの気まぐれ" );
#else
                            "the capriciousness of Xom" );
#endif

            done_bad = true;
        }
        else if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『苦しめ！』" :
                (temp_rand == 1) ? "ゾムの悪意ある注視が、一瞬だけあなたに注がれた。" :
                (temp_rand == 2) ? "ゾムの力が一瞬だけあなたに触れた。"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
                (temp_rand == 0) ? "\"Suffer!\"" :
                (temp_rand == 1) ? "Xom's malign attention turns to you for a moment." :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            lose_stat(STAT_RANDOM, 1 + random2(3), true);

            done_bad = true;
        }
        else if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "ゾムはあなたに目をとめた。" :
                (temp_rand == 1) ? "ゾムは一瞬だけあなたに注目した。":
                (temp_rand == 2) ? "ゾムの力が一瞬だけあなたに触れた。"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
                (temp_rand == 0) ? "Xom notices you." :
                (temp_rand == 1) ? "Xom's attention turns to you for a moment.":
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            miscast_effect( SPTYP_RANDOM, 5 + random2(15), random2(250), 0,
#ifdef JP
                            "ゾムの気まぐれ" );
#else
                            "the capriciousness of Xom" );
#endif

            done_bad = true;
        }
        else if (!you.is_undead && random2(sever) <= 3)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『定命の者よ！お前には若干の修正が必要だな！』" :
                (temp_rand == 1) ? "『お前の貧相な体を改造してやろう』" :
                (temp_rand == 2) ? "ゾムの力が一瞬だけあなたに触れた。"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
                (temp_rand == 0) ? "\"You need some minor adjustments, mortal!\"" :
                (temp_rand == 1) ? "\"Let me alter your pitiful body.\"" :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

#ifdef JP
            mpr("あなたの肉体は歪曲のエネルギーに覆われた。");
#else
            mpr("Your body is suffused with distortional energy.");
#endif

            set_hp(1 + random2(you.hp), false);
            deflate_hp(you.hp_max / 2, true);

            bool failMsg = true;
            for (int i = 0; i < 4; i++)
            {
                if (!mutate(100, failMsg))
                    failMsg = false;
            }

            done_bad = true;
        }
        else if (!you.is_undead && random2(sever) <= 3)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『定命の者よ、お前ってば癇にさわるんだよ』" :
                (temp_rand == 1) ? "『お前は無価値な分際でいい気になりすぎているな』" :
                (temp_rand == 2) ? "ゾムの力が一瞬だけあなたに触れた。"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
                (temp_rand == 0) ? "\"You have displeased me, mortal.\"" :
                (temp_rand == 1) ? "\"You have grown too confident for your meagre worth.\"" :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal laughter.");
#endif

            if (one_chance_in(4))
            {
                drain_exp();
                if (random2(sever) > 3)
                    drain_exp();
                if (random2(sever) > 3)
                    drain_exp();
            }
            else
            {
#ifdef JP
                mpr("苦痛の波動があなたの全身を引き裂いた！");
#else
                mpr("A wave of agony tears through your body!");
#endif
                set_hp(1 + (you.hp / 2), false);
            }

            done_bad = true;
        }
        else if (random2(sever) <= 3)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『お楽しみの時間だ！』" :
                (temp_rand == 1) ? "『定命の者よ、戦って生き延びて見せよ』" :
                (temp_rand == 2) ? "『お前の強さが生き残るに充分か試してみよう』"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
                (temp_rand == 0) ? "\"Time to have some fun!\"" :
                (temp_rand == 1) ? "\"Fight to survive, mortal.\"" :
                (temp_rand == 2) ? "\"Let's see if it's strong enough to survive yet.\""
                                 : "You hear Xom's maniacal laughter.");
#endif

            if (one_chance_in(4))
                dancing_weapon(100, true);      // nasty, but fun
            else
            {
                create_monster(MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                    BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(3))
                    create_monster(MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(4))
                    create_monster(MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(3))
                    create_monster(MONS_HELLION + random2(10), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);

                if (one_chance_in(4))
                    create_monster(MONS_HELLION + random2(10), ENCH_ABJ_III,
                        BEH_HOSTILE, you.x_pos, you.y_pos, MHITNOT, 250);
            }

            done_bad = true;
        }
        else if (you.your_level == 0)
        {
            // this should remain the last possible outcome {dlb}
            temp_rand = random2(3);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『定命の者よ！お前はそのちっぽけな世界で居心地良くしすぎているな』" :
                (temp_rand == 1) ? "ゾムがあなたをアビスに放り込んだ！"
                                 : "ゾムの狂気の哄笑が耳に響くとともに、世界がクルクルと回りだした。");
#else
                (temp_rand == 0) ? "\"You have grown too comfortable in your little world, mortal!\"" :
                (temp_rand == 1) ? "Xom casts you into the Abyss!"
                                 : "The world seems to spin as Xom's maniacal laughter rings in your ears.");
#endif

            banished(DNGN_ENTER_ABYSS);

            done_bad = true;
        }
    }                           // end "Bad Things"
    else
    {
        // begin "Good Things"
        done_good = false;

// Okay, now for the nicer stuff (note: these things are not necessarily nice):
        if (random2(sever) <= 2)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『行きて、そして破壊せよ！』" :
                (temp_rand == 1) ? "『定命の者よ、行きて、そして破壊せよ！』" :
                (temp_rand == 2) ? "ゾムはあなたに幾許かの好意を示した。"
                                 : "ゾムはあなたに微笑みかけている。");
#else
                (temp_rand == 0) ? "\"Go forth and destroy!\"" :
                (temp_rand == 1) ? "\"Go forth and destroy, mortal!\"" :
                (temp_rand == 2) ? "Xom grants you a minor favour."
                                 : "Xom smiles on you.");
#endif

            switch (random2(7))
            {
            case 0:
                potion_effect(POT_HEALING, 150);
                break;
            case 1:
                potion_effect(POT_HEAL_WOUNDS, 150);
                break;
            case 2:
                potion_effect(POT_SPEED, 150);
                break;
            case 3:
                potion_effect(POT_MIGHT, 150);
                break;
            case 4:
                potion_effect(POT_INVISIBILITY, 150);
                break;
            case 5:
                if (one_chance_in(6))
                    potion_effect(POT_EXPERIENCE, 150);
                else
                {
                    you.berserk_penalty = NO_BERSERK_PENALTY;
                    potion_effect(POT_BERSERK_RAGE, 150);
                }
                break;
            case 6:
                you.berserk_penalty = NO_BERSERK_PENALTY;
                potion_effect(POT_BERSERK_RAGE, 150);
                break;
            }

            done_good = true;
        }
        else if (random2(sever) <= 4)
        {
            temp_rand = random2(3);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『わが子らよ、この定命の者に仕えよ』" :
                (temp_rand == 1) ? "ゾムはあなたに一時的な手助けを与えた。"
                                 : "ゾムは門を開いた。");
#else
                (temp_rand == 0) ? "\"Serve the mortal, my children!\"" :
                (temp_rand == 1) ? "Xom grants you some temporary aid."
                                 : "Xom opens a gate.");
#endif

            create_monster( MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                            BEH_FRIENDLY, you.x_pos, you.y_pos,
                            you.pet_target, 250 );

            create_monster( MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                            BEH_FRIENDLY, you.x_pos, you.y_pos,
                            you.pet_target, 250 );

            if (random2( you.experience_level ) >= 8)
            {
                create_monster( MONS_NEQOXEC + random2(5), ENCH_ABJ_III,
                                BEH_FRIENDLY, you.x_pos, you.y_pos,
                                you.pet_target, 250 );
            }

            if (random2( you.experience_level ) >= 8)
            {
                create_monster( MONS_HELLION + random2(10), ENCH_ABJ_III,
                                BEH_FRIENDLY, you.x_pos, you.y_pos,
                                you.pet_target, 250 );
            }

            if (random2( you.experience_level ) >= 8)
            {
                create_monster( MONS_HELLION + random2(10), ENCH_ABJ_III,
                                BEH_FRIENDLY, you.x_pos, you.y_pos,
                                you.pet_target, 250 );
            }

            done_good = true;
        }
        else if (random2(sever) <= 3)
        {
            temp_rand = random2(3);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『我が好意の印にこれを受け取れ』" :
                (temp_rand == 1) ? "ゾムはあなたに贈り物を授けた！"
                                 : "ゾムは気前の良さを披露した。");
#else
                (temp_rand == 0) ? "\"Take this token of my esteem.\"" :
                (temp_rand == 1) ? "Xom grants you a gift!"
                                 : "Xom's generous nature manifests itself.");
#endif

            if (grd[you.x_pos][you.y_pos] == DNGN_LAVA
                || grd[you.x_pos][you.y_pos] == DNGN_DEEP_WATER)
            {
                // How unfortunate. I'll bet Xom feels sorry for you.
#ifdef JP
                mpr("あなたは水飛沫の音を耳にした。");
#else
                mpr("You hear a splash.");
#endif
            }
            else
            {
                int thing_created = items(1, OBJ_RANDOM, OBJ_RANDOM, true,
                                              you.experience_level * 3, 250);

                move_item_to_grid( &thing_created, you.x_pos, you.y_pos );

                if (thing_created != NON_ITEM)
                {
                    canned_msg(MSG_SOMETHING_APPEARS);
                    more();
                }
            }

            done_good = true;
        }
        else if (random2(sever) <= 4)
        {
            const int demon = (random2(you.experience_level) < 6)
                                                ? MONS_WHITE_IMP + random2(5)
                                                : MONS_NEQOXEC + random2(5);

            if (create_monster( demon, 0, BEH_FRIENDLY, you.x_pos, you.y_pos,
                                                 you.pet_target, 250 ) != -1)
            {
                temp_rand = random2(3);

                god_speaks(GOD_XOM,
#ifdef JP
                    (temp_rand == 0) ? "『わが子よ、この定命の者に仕えよ』" :
                    (temp_rand == 1) ? "ゾムがあなたに悪魔の召使を貸し与えた。"
                                     : "ゾムは門を開いた。");
#else
                    (temp_rand == 0) ? "\"Serve the mortal, my child!\"" :
                    (temp_rand == 1) ? "Xom grants you a demonic servitor."
                                     : "Xom opens a gate.");
#endif
            }

            done_good = true;   // well, for Xom, trying == doing {dlb}
        }
        else if (random2(sever) <= 4)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『この武器を手に破壊をもたらせ！』" :
                (temp_rand == 1) ? "『お前は贈り物を受けるに値する』" :
                (temp_rand == 2) ? "ゾムはあなたに、死をもたらす武器を与えた。"
                                 : "ゾムはあなたに微笑みかけている。");
#else
                (temp_rand == 0) ? "\"Take this instrument of destruction!\"" :
                (temp_rand == 1) ? "\"You have earned yourself a gift.\"" :
                (temp_rand == 2) ? "Xom grants you an implement of death."
                                 : "Xom smiles on you.");
#endif

            if (acquirement(OBJ_WEAPONS))
                more();

            done_good = true;
        }
        else if (!you.is_undead && random2(sever) <= 5)
        {
            temp_rand = random2(4);

            god_speaks(GOD_XOM,
#ifdef JP
                (temp_rand == 0) ? "『定命の者よ！お前には若干の修正が必要だな！』" :
                (temp_rand == 1) ? "『お前の貧相な体を改造してやろう』" :
                (temp_rand == 2) ? "ゾムの力が一瞬だけあなたに触れた。"
                                 : "あなたはゾムの狂気の哄笑を耳にした。");
#else
                (temp_rand == 0) ? "\"You need some minor adjustments, mortal!\"" :
                (temp_rand == 1) ? "\"Let me alter your pitiful body.\"" :
                (temp_rand == 2) ? "Xom's power touches on you for a moment."
                                 : "You hear Xom's maniacal chuckling.");
#endif

#ifdef JP
            mpr("あなたの肉体は歪曲のエネルギーに覆われた。");
#else
            mpr("Your body is suffused with distortional energy.");
#endif

            set_hp(1 + random2(you.hp), false);
            deflate_hp(you.hp_max / 2, true);

            if (coinflip() || !give_cosmetic_mutation())
                give_good_mutation();

            done_good = true;
        }
        else if (random2(sever) <= 2)
        {
            // this should remain the last possible outcome {dlb}
            if (!one_chance_in(8))
                you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] = 1;

#ifdef JP
            god_speaks(GOD_XOM, "一帯に神の稲妻が降り注いだ！");
#else
            god_speaks(GOD_XOM, "The area is suffused with divine lightning!");
#endif

            beam.beam_source = NON_MONSTER;
            beam.type = SYM_BURST;
            beam.damage = dice_def( 3, 30 );
            beam.flavour = BEAM_ELECTRICITY;
            beam.target_x = you.x_pos;
            beam.target_y = you.y_pos;
#ifdef JP
            strcpy(beam.beam_name, "稲妻の放射");
#else
            strcpy(beam.beam_name, "blast of lightning");
#endif
            beam.colour = LIGHTCYAN;
            beam.thrower = KILL_YOU;    // your explosion
#ifdef JP
            beam.aux_source = "ゾムの雷霆の一撃";
#else
            beam.aux_source = "Xom's lightning strike";
#endif
            beam.ex_size = 2;
            beam.isTracer = false;

            explosion(beam);

            if (you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] == 1)
            {
#ifdef JP
                mpr("あなたから神による防護が消え去った。");
#else
                mpr("Your divine protection wanes.");
#endif
                you.attribute[ATTR_DIVINE_LIGHTNING_PROTECTION] = 0;
            }

            done_good = true;
        }
    }                           // end "Good Things"

    if (done_bad || done_good || one_chance_in(4))
        return;
    else
        goto okay_try_again;
}                               // end Xom_acts()

void done_good(char thing_done, int pgain)
{
    if (you.religion == GOD_NO_GOD)
        return;

    switch (thing_done)
    {
    case GOOD_KILLED_LIVING:
        switch (you.religion)
        {
        case GOD_ELYVILON:
#ifdef JP
            simple_god_message("はそれを喜ばない！");
#else
            simple_god_message(" did not appreciate that!");
#endif
            naughty(NAUGHTY_KILLING, 10);
            break;
        case GOD_KIKUBAAQUDGHA:
        case GOD_YREDELEMNUL:
        case GOD_VEHUMET:
        case GOD_OKAWARU:
        case GOD_MAKHLEB:
        case GOD_TROG:
#ifdef JP
            simple_god_message("はあなたの殺しを善しとした。");
#else
            simple_god_message(" accepts your kill.");
#endif
            if (random2(18 + pgain) > 5)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_KILLED_UNDEAD:
        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_SHINING_ONE:
        case GOD_VEHUMET:
        case GOD_MAKHLEB:
        case GOD_OKAWARU:
#ifdef JP
            simple_god_message("はあなたの殺しを善しとした。");
#else
            simple_god_message(" accepts your kill.");
#endif
            if (random2(18 + pgain) > 4)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_KILLED_DEMON:
        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_SHINING_ONE:
        case GOD_VEHUMET:
        case GOD_MAKHLEB:
        case GOD_OKAWARU:
#ifdef JP
            simple_god_message("はあなたの殺しを善しとした。");
#else
            simple_god_message(" accepts your kill.");
#endif
            if (random2(18 + pgain) > 3)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_KILLED_ANGEL_I:
    case GOOD_KILLED_ANGEL_II:
        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_SHINING_ONE:
        case GOD_ELYVILON:
#ifdef JP
            simple_god_message("はそれを見咎めた！");
#else
            simple_god_message(" did not appreciate that!");
#endif
            naughty(NAUGHTY_ATTACK_HOLY, (you.conf ? 3 : pgain * 3));
            break;
        }
        break;

    case GOOD_KILLED_WIZARD:
        // hooking this up, but is it too good?
        // enjoy it while you can -- bwr
        if (you.religion == GOD_TROG)
        {
#ifdef JP
            simple_god_message( "はあなたが魔法の遣い手を殺したことを喜んだ。" );
#else
            simple_god_message( " appreciates your killing of a magic user." );
#endif

            if (random2( 5 + pgain ) > 5)
                gain_piety(1);
        }
        break;

    case GOOD_HACKED_CORPSE:    // NB - pgain is you.experience_level (maybe)
        switch (you.religion)
        {
        // case GOD_KIKUBAAQUDGHA:
        case GOD_OKAWARU:
        case GOD_MAKHLEB:
        case GOD_TROG:
#ifdef JP
            simple_god_message("はあなたの捧げ物を受け容れた。");
#else
            simple_god_message(" accepts your offering.");
#endif
            if (random2(10 + pgain) > 5)
                gain_piety(1);
            break;

        // case GOD_ZIN:
        // case GOD_SHINING_ONE:
        case GOD_ELYVILON:
#ifdef JP
            simple_god_message("はそれを喜ばない！");
#else
            simple_god_message(" did not appreciate that!");
#endif

            naughty(NAUGHTY_BUTCHER, 8);
            break;
        }
        break;

    case GOOD_OFFER_STUFF:
#ifdef JP
        simple_god_message("はあなたの捧げ物に満足した。");
#else
        simple_god_message(" is pleased with your offering.");
#endif

        gain_piety(1);
        break;

    case GOOD_SLAVES_KILL_LIVING:
        switch (you.religion)
        {
        case GOD_KIKUBAAQUDGHA:
        case GOD_YREDELEMNUL:
        case GOD_VEHUMET:
#ifdef JP
            simple_god_message("はあなたの奴隷の殺しを善しとした。");
#else
            simple_god_message(" accepts your slave's kill.");
#endif

            if (random2(pgain + 18) > 5)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_SERVANTS_KILL:
        switch (you.religion)
        {
        case GOD_VEHUMET:
        case GOD_MAKHLEB:
#ifdef JP
            simple_god_message("はあなたのしもべの殺しを善しとした。");
#else
            simple_god_message(" accepts your collateral kill.");
#endif

            if (random2(pgain + 18) > 5)
                gain_piety(1);
            break;
        }
        break;

    case GOOD_CARDS:
        switch (you.religion)
        {
        case GOD_NEMELEX_XOBEH:
            gain_piety(pgain);
            break;
        }
        break;
    // Offering at altars is covered in another function.
    }
}                               // end done_good()

void gain_piety(char pgn)
{
    // check to see if we owe anything first
    if (you.penance[you.religion] > 0)
    {
        dec_penance(pgn);
        return;
    }
    else if (you.gift_timeout > 0)
    {
        if (you.gift_timeout > pgn)
            you.gift_timeout -= pgn;
        else
            you.gift_timeout = 0;

        // Slow down piety gain to account for the fact that gifts
        // no longer have a piety cost for getting them
        if (!one_chance_in(8))
            return;
    }

    // slow down gain at upper levels of piety
    if (you.piety > 199
        || (you.piety > 150 && one_chance_in(3))
        || (you.piety > 100 && one_chance_in(3)))
        return;

    int old_piety = you.piety;

    you.piety += pgn;

    if (you.piety >= 30 && old_piety < 30)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_NEMELEX_XOBEH:
        case GOD_SIF_MUNA:
            break;
        default:
#ifdef JP
            strcpy(info, "あなたは今や");
#else
            strcpy(info, "You can now ");
#endif
            strcat(info,
                    (you.religion == GOD_ZIN || you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "アンデッドを撃退することができる" :
#else
                            ? "repel the undead" :
#endif

                    (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                            ? "アンデッドのしもべを近くに呼び集めることができる" :
#else
                            ? "recall your undead slaves" :
#endif
                    (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "死体を操ることができる" :
#else
                            ? "animate corpses" :
#endif
                    (you.religion == GOD_VEHUMET)
#ifdef JP
                            ? "ヴェフメットの名に於いての殺しによって力を得られる" :
#else
                            ? "gain power from killing in Vehumet's name" :
#endif
                    (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "マクレブの名に於いての殺しによって力を得られる" :
#else
                            ? "gain power from killing in Makhleb's name" :
#endif
                    (you.religion == GOD_OKAWARU)
#ifdef JP
                            ? "一時的ではあるが強大な腕力を引き出すことができる" :
#else
                            ? "give your great, but temporary, body strength" :
#endif
                    (you.religion == GOD_TROG)
#ifdef JP
                            ? "意思によってバーサーク状態になることができる" :
#else
                            ? "go berserk at will" :
#endif
                    (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "小さき癒しをエリヴィロンに請うことができる"
#else
                            ? "call upon Elyvilon for minor healing"
#endif
                // Unknown god
#ifdef JP
                            : "このプログラムバグに耐えうる @30");
#else
                            : "endure this program bug @30");
#endif

#ifdef JP
            strcat(info, "。");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 50 && old_piety < 50)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_NEMELEX_XOBEH:
            break;
        case GOD_KIKUBAAQUDGHA:
#ifdef JP
            simple_god_message("があなたに、死の魔術の副作用からの保護を与えている。");
#else
            simple_god_message(" is protecting you from some side-effects of death magic.");
#endif
            break;

        case GOD_VEHUMET:
#ifdef JP
            god_speaks(you.religion, "あなたはヴェフメットへの祈りによって破壊の魔法の手助けを請うことができる。");
#else
            god_speaks(you.religion, "You can call upon Vehumet to aid your destructive magics with prayer.");
#endif
            break;

        default:
#ifdef JP
            strcpy(info, "あなたは今や");
#else
            strcpy(info, "You can now ");
#endif

            strcat(info,
                   (you.religion == GOD_ZIN)
#ifdef JP
                           ? "小さき癒しをジンに請うことができる" :
#else
                           ? "call upon Zin for minor healing" :
#endif
                   (you.religion == GOD_SHINING_ONE)
#ifdef JP
                           ? "あなたの敵に打擲を与えることができる" :
#else
                           ? "smite your foes" :
#endif
                   (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                           ? "アンデッドのしもべを近くに呼び集めることができる" :
#else
                           ? "recall your undead slaves" :
#endif
                   (you.religion == GOD_OKAWARU)
#ifdef JP
                           ? "小さき癒しをオカワルに請うことができる" :
#else
                           ? "call upon Okawaru for minor healing" :
#endif
                   (you.religion == GOD_MAKHLEB)
#ifdef JP
                           ? "マクレブの破壊の力を扱うことができる" :
#else
                           ? "harness Makhleb's destructive might" :
#endif
                   (you.religion == GOD_SIF_MUNA)
#ifdef JP
                           ? "新しい呪文を覚えるために記憶を抹消することができる" :
#else
                           ? "freely open your mind to new spells" :
#endif
                   (you.religion == GOD_TROG)
#ifdef JP
                           ? "一時的ではあるが強大な腕力を引き出すことができる" :
#else
                           ? "give your body great, but temporary, strength" :
#endif
                   (you.religion == GOD_ELYVILON)
#ifdef JP
                           ? "肉体の清浄化をエリヴィロンに請うことができる"
#else
                           ? "call upon Elyvilon for purification"
#endif
                   // Unknown god
#ifdef JP
                           : "このプログラムバグに耐えうる @50");
#else
                           : "endure this program bug @50");
#endif

#ifdef JP
            strcat(info, "。");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 75 && old_piety < 75)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_OKAWARU:
        case GOD_NEMELEX_XOBEH:
        case GOD_SIF_MUNA:
        case GOD_TROG:
            break;
        case GOD_VEHUMET:
#ifdef JP
            god_speaks(you.religion,"あなたは祈りの間、召換されたモンスターからある程度護られる。");
#else
            god_speaks(you.religion,"During prayer you have some protection from summoned creatures.");
#endif
            break;

        default:
#ifdef JP
            strcpy(info, "あなたは今や");
#else
            strcpy(info, "You can now ");
#endif
            strcat(info,
                     (you.religion == GOD_ZIN)
#ifdef JP
                                 ? "悪疫の発生を請うことができる" :
#else
                                 ? "call down a plague" :
#endif
                     (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                 ? "アンデッドを解呪することができる" :
#else
                                 ? "dispel the undead" :
#endif
                     (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                 ? "アンデッドを永続的な奴隷とすることができる" :
#else
                                 ? "permanently enslave the undead" :
#endif
                     (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                 ? "死者の軍勢を操ることができる" :
#else
                                 ? "animate legions of the dead" :
#endif
                     (you.religion == GOD_MAKHLEB)
#ifdef JP
                                 ? "マクレブの小さきしもべを召換することができる" :
#else
                                 ? "summon a lesser servant of Makhleb" :
#endif
                     (you.religion == GOD_ELYVILON)
#ifdef JP
                                 ? "癒しをエリヴィロンに請うことができる"
#else
                                 ? "call upon Elyvilon for moderate healing"
#endif
                     // Unknown god
#ifdef JP
                                 : "このプログラムバグに耐えうる @75");
#else
                                 : "endure this program bug @75");
#endif
#ifdef JP
            strcat(info, "。");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 100 && old_piety < 100)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_OKAWARU:
        case GOD_NEMELEX_XOBEH:
        case GOD_KIKUBAAQUDGHA:
            break;
        case GOD_SIF_MUNA:
            simple_god_message
#ifdef JP
                ("があなたに、魔術の副作用からの保護を与えている。");
#else
                (" is protecting you from some side-effects of spellcasting.");
#endif
            break;

        default:
#ifdef JP
            strcpy(info, "あなたは今や");
#else
            strcpy(info, "You can now ");
#endif

            strcat(info,
                        (you.religion == GOD_ZIN)
#ifdef JP
                                ? "聖なる御言葉を口にすることができる" :
#else
                                ? "utter a Holy Word" :
#endif
                        (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "神の怒りの矢を放つことができる" :
#else
                                ? "hurl bolts of divine anger" :
#endif
                        (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "周囲から生命力を吸い上げることができる" :
#else
                                ? "drain ambient lifeforce" :
#endif
                        (you.religion == GOD_VEHUMET)
#ifdef JP
                                ? "周辺から魔力を吸い上げることができる" :
#else
                                ? "tap ambient magical fields" :
#endif
                        (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "マクレブの大いなる破壊を放つことができる" :
#else
                                ? "hurl Makhleb's greater destruction" :
#endif
                        (you.religion == GOD_TROG)
#ifdef JP
                                ? "自身を加速することができる" :
#else
                                ? "haste yourself" :
#endif
                        (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "能力の回復をエリヴィロンに請うことができる"
#else
                                ? "call upon Elyvilon to restore your abilities"
#endif
                        // Unknown god
#ifdef JP
                                : "このプログラムバグに耐えうる @100");
#else
                                : "endure this program bug @100");
#endif

#ifdef JP
            strcat(info, "。");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }

    if (you.piety >= 120 && old_piety < 120)
    {
        switch (you.religion)
        {
        case GOD_NO_GOD:
        case GOD_XOM:
        case GOD_NEMELEX_XOBEH:
        case GOD_VEHUMET:
        case GOD_SIF_MUNA:
        case GOD_TROG:
            break;
        default:
#ifdef JP
            strcpy(info, "あなたは今や");
#else
            strcpy(info, "You can now ");
#endif

            strcat(info,
                     (you.religion == GOD_ZIN)
#ifdef JP
                                ? "守護天使を呼び出すことができる" :
#else
                                ? "summon a guardian angel" :
#endif
                     (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "聖なる戦士を呼び出すことができる" :
#else
                                ? "summon a divine warrior" :
#endif
                     (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                ? "死の御遣いを呼び出すことができる" :
#else
                                ? "summon an emissary of Death" :
#endif
                     (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "アンデッドに支配を及ぼすことができる" :
#else
                                ? "control the undead" :
#endif
                     (you.religion == GOD_OKAWARU)
#ifdef JP
                                ? "自身を加速することができる" :
#else
                                ? "haste yourself" :
#endif
                     (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "マクレブの大いなるしもべを呼び出すことができる" :
#else
                                ? "summon a greater servant of Makhleb" :
#endif
                     (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "大いなる癒しをエリヴィロンに請うことができる"
#else
                                ? "call upon Elyvilon for incredible healing"
#endif
                     // Unknown god
#ifdef JP
                                : "このプログラムバグに耐えうる @120");
#else
                                : "endure this program bug @120");
#endif

#ifdef JP
            strcat(info, "。");
#else
            strcat(info, ".");
#endif
            god_speaks(you.religion, info);
            break;
        }
    }
}                               // end gain_piety()

void naughty(char type_naughty, int naughtiness)
{
    int penance = 0;
    int piety_loss = 0;

    // if you currently worship no deity in particular, exit function {dlb}
    if (you.religion == GOD_NO_GOD)
        return;

    switch (you.religion)
    {
    case GOD_ZIN:
        switch (type_naughty)
        {
        case NAUGHTY_NECROMANCY:
        case NAUGHTY_UNHOLY:
        case NAUGHTY_ATTACK_HOLY:
            piety_loss = naughtiness;
            penance = piety_loss * 2;
            break;
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            break;
        case NAUGHTY_BUTCHER:
            piety_loss = naughtiness;
            if (one_chance_in(3))
                penance = piety_loss;
            break;
        }
        break;

    case GOD_SHINING_ONE:
        switch (type_naughty)
        {
        case NAUGHTY_NECROMANCY:
        case NAUGHTY_UNHOLY:
        case NAUGHTY_ATTACK_HOLY:
            piety_loss = naughtiness;
            penance = piety_loss;
            break;
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            break;
        case NAUGHTY_BUTCHER:
            piety_loss = naughtiness;
            if (one_chance_in(3))
                penance = piety_loss;
            break;
        case NAUGHTY_STABBING:
            piety_loss = naughtiness;
            if (one_chance_in(5))       // can be accidental so we're nice here
                penance = piety_loss;
            break;
        case NAUGHTY_POISON:
            piety_loss = naughtiness;
            penance = piety_loss * 2;
            break;
        }
        break;

    case GOD_ELYVILON:
        switch (type_naughty)
        {
        case NAUGHTY_NECROMANCY:
        case NAUGHTY_UNHOLY:
        case NAUGHTY_ATTACK_HOLY:
            piety_loss = naughtiness;
            penance = piety_loss;
            break;
        case NAUGHTY_KILLING:
            piety_loss = naughtiness;
            penance = piety_loss * 2;
            break;
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        // Healer god gets a bit more upset since you should have
        // used your healing powers to save them.
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            penance = piety_loss;
            break;
        case NAUGHTY_BUTCHER:
            piety_loss = naughtiness;
            if (one_chance_in(3))
                penance = piety_loss;
            break;
        }
        break;

    case GOD_OKAWARU:
        switch (type_naughty)
        {
        case NAUGHTY_ATTACK_FRIEND:
            piety_loss = naughtiness;
            penance = piety_loss * 3;
            break;
        case NAUGHTY_FRIEND_DIES:
            piety_loss = naughtiness;
            break;
        }
        break;

    case GOD_TROG:
        switch (type_naughty)
        {
        case NAUGHTY_SPELLCASTING:
            piety_loss = naughtiness;
            // This penance isn't so bad since its much easier to
            // gain piety with Trog than the other gods in this function.
            penance = piety_loss * 10;
            break;
        }
        break;
    }

    // exit function early iff piety loss is zero:
    if (piety_loss < 1)
        return;

    // output guilt message:
#ifdef JP
    strcpy(info, "あなたは");
#else
    strcpy(info, "You feel");
#endif

#ifdef JP
    strcat(info, (piety_loss == 1) ? "ほんの少し" :
                 (piety_loss <  5) ? "" :
                 (piety_loss < 10) ? "強い"
                                   : "極めて強い");
#else
    strcat(info, (piety_loss == 1) ? " a little " :
                 (piety_loss <  5) ? " " :
                 (piety_loss < 10) ? " very "
                                   : " extremely ");
#endif

#ifdef JP
    strcat(info, "罪の意識を覚えた。");
#else
    strcat(info, "guilty.");
#endif
    mpr(info);

    lose_piety(piety_loss);

    if (you.piety < 1)
        excommunication();
    else if (penance)       // Don't bother unless we're not kicking them out
    {
        //jmf: FIXME: add randomness to following message:
#ifdef JP
        god_speaks(you.religion, "『定命の者よ、お前は罪の代価を支払うことになろう！！』");
#else
        god_speaks(you.religion, "\"You will pay for your transgression, mortal!\"");
#endif
        inc_penance(penance);
    }
}                               // end naughty()

void lose_piety(char pgn)
{
    int old_piety = you.piety;

    if (you.piety - pgn < 0)
        you.piety = 0;
    else
        you.piety -= pgn;

    // Don't bother printing out these messages if you're under
    // penance, you wouldn't notice since all these abilities
    // are withheld.
    if (!player_under_penance() && you.piety != old_piety)
    {
        if (you.piety < 120 && old_piety >= 120)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_NEMELEX_XOBEH:
            case GOD_VEHUMET:
            case GOD_SIF_MUNA:
            case GOD_TROG:
                break;
            default:
#ifdef JP
                strcpy(info, "あなたはもはや");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                           (you.religion == GOD_ZIN)
#ifdef JP
                                ? "守護天使を呼び出すことができない" :
#else
                                ? "summon guardian angels" :
#endif
                           (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "聖なる戦士を呼び出すことができない" :
#else
                                ? "summon divine warriors" :
#endif
                           (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                ? "死の御遣いを呼び出すことができない" :
#else
                                ? "summon Death's emissaries" :
#endif
                           (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "アンデッドに支配を及ぼすことができない" :
#else
                                ? "control undead beings" :
#endif
                           (you.religion == GOD_OKAWARU)
#ifdef JP
                                ? "自身を加速することができない" :
#else
                                ? "haste yourself" :
#endif
                           (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "マクレブの大いなるしもべを呼び出すことができない" :
#else
                                ? "summon a greater servant of Makhleb" :
#endif
                           (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "大いなる癒しをエリヴィロンに請うことができない"
#else
                                ? "call upon Elyvilon for incredible healing"
#endif
                           // Unknown god
#ifdef JP
                                : "このプログラムバグに耐えることができない @120");
#else
                                : "endure this program bug @120");
#endif

#ifdef JP
                strcat(info, "。");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 100 && old_piety >= 100)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_OKAWARU:
            case GOD_NEMELEX_XOBEH:
            case GOD_KIKUBAAQUDGHA:
                break;
            case GOD_SIF_MUNA:
#ifdef JP
                god_speaks(you.religion,"シフ・ムーナはもはや、あなたを呪文の失敗から護っていない。");
#else
                god_speaks(you.religion,"Sif Muna is no longer protecting you from miscast magic.");
#endif
                break;
            default:
#ifdef JP
                strcpy(info, "あなたはもはや");
#else
                strcpy(info, "You can no longer ");
#endif
                strcat(info,
                        (you.religion == GOD_ZIN)
#ifdef JP
                            ? "聖なる御言葉を口にすることができない" :
#else
                            ? "utter a Holy Word" :
#endif
                        (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "能力の回復をエリヴィロンに請うことができない" :
#else
                            ? "call upon Elyvilon to restore your abilities" :
#endif
                        (you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "神の怒りの矢を放つことができない" :
#else
                            ? "hurl bolts of divine anger" :
#endif
                        (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "周囲から生命力を吸い上げることができない" :
#else
                            ? "drain ambient life force" :
#endif
                        (you.religion == GOD_VEHUMET)
#ifdef JP
                            ? "周辺から魔力を吸い上げることができない" :
#else
                            ? "tap ambient magical fields" :
#endif
                        (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "マクレブの大いなる破壊を放つことができない" :
#else
                            ? "direct Makhleb's greater destructive powers" :
#endif
                        (you.religion == GOD_TROG)
#ifdef JP
                            ? "自身を加速することができない"
#else
                            ? "haste yourself"
#endif
                        // Unknown god
#ifdef JP
                            : "このプログラムバグに耐えることができない @100");
#else
                            : "endure this program bug @100");
#endif

#ifdef JP
                strcat(info, "。");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 75 && old_piety >= 75)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_OKAWARU:
            case GOD_NEMELEX_XOBEH:
            case GOD_SIF_MUNA:
            case GOD_TROG:
                break;
            case GOD_VEHUMET:
#ifdef JP
                simple_god_message("はもはや、召換されたモンスターからあなたを護らない。");
#else
                simple_god_message(" will longer shield you from summoned creatures.");
#endif
                break;
            default:
#ifdef JP
                strcpy(info, "あなやはもはや");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                       (you.religion == GOD_ZIN)
#ifdef JP
                                ? "悪疫の発生を請うことができない" :
#else
                                ? "call down a plague" :
#endif
                       (you.religion == GOD_SHINING_ONE)
#ifdef JP
                                ? "アンデッドを解呪することができない" :
#else
                                ? "dispel undead" :
#endif
                       (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                                ? "アンデッドを奴隷とすることができない" :
#else
                                ? "enslave undead" :
#endif
                       (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                                ? "死者の軍勢を操ることができない" :
#else
                                ? "animate legions of the dead" :
#endif
                       (you.religion == GOD_MAKHLEB)
#ifdef JP
                                ? "マクレブのしもべを召換することができない" :
#else
                                ? "summon a servant of Makhleb" :
#endif
                       (you.religion == GOD_ELYVILON)
#ifdef JP
                                ? "癒しをエリヴィロンに請うことができない"
#else
                                ? "call upon Elyvilon for moderate healing"
#endif
                       // Unknown god
#ifdef JP
                                : "endure this program bug @75");
#else
                                : "endure this program bug @75");
#endif

#ifdef JP
                strcat(info, "。");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 50 && old_piety >= 50)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_NEMELEX_XOBEH:
                break;
            case GOD_KIKUBAAQUDGHA:
#ifdef JP
                simple_god_message("はもはや、あなたを死の魔術の失敗から護っていない。");
#else
                simple_god_message(" is no longer shielding you from miscast death magic.");
#endif
                break;
            case GOD_VEHUMET:
#ifdef JP
                simple_god_message("はもはや、あなたの破壊の魔法を手助けしない。");
#else
                simple_god_message(" will no longer aid your destructive magics.");
#endif
                break;

            default:
#ifdef JP
                strcpy(info, "あなやはもはや");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                       (you.religion == GOD_ZIN)
#ifdef JP
                            ? "小さき癒しをジンに請うことができない" :
#else
                            ? "call upon Zin for minor healing" :
#endif
                       (you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "あなたの敵に打擲を与えることができない" :
#else
                            ? "smite your foes" :
#endif
                       (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "アンデッドのしもべを近くに呼び集めることができない" :
#else
                            ? "recall your undead slaves" :
#endif
                       (you.religion == GOD_OKAWARU)
#ifdef JP
                            ? "小さき癒しをオカワルに請うことができない" :
#else
                            ? "call upon Okawaru for minor healing" :
#endif
                       (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "マクレブの破壊の力を扱うことができない" :
#else
                            ? "hurl Makhleb's destruction" :
#endif
                       (you.religion == GOD_SIF_MUNA)
#ifdef JP
                            ? "意思の力で呪文を忘れることができない" :
#else
                            ? "forget spells at will" :
#endif
                       (you.religion == GOD_TROG)
#ifdef JP
                            ? "強大な腕力を引き出すことができない" :
#else
                            ? "give your body great, but temporary, strength" :
#endif
                       (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "肉体の清浄化をエリヴィロンに請うことができない"
#else
                            ? "call upon Elyvilon for Purification"
#endif
                       // Unknown god
#ifdef JP
                            : "このプログラムバグに耐えうことができない @50");
#else
                            : "endure this program bug @50");
#endif

#ifdef JP
                strcat(info, "。");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }

        if (you.piety < 30 && old_piety >= 30)
        {
            switch (you.religion)
            {
            case GOD_NO_GOD:
            case GOD_XOM:
            case GOD_NEMELEX_XOBEH:
            case GOD_SIF_MUNA:
                break;
            default:
#ifdef JP
                strcpy(info, "あなたはもはや");
#else
                strcpy(info, "You can no longer ");
#endif

                strcat(info,
                    (you.religion == GOD_ZIN || you.religion == GOD_SHINING_ONE)
#ifdef JP
                            ? "アンデッドを撃退することができない" :
#else
                            ? "repel the undead" :
#endif
                    (you.religion == GOD_KIKUBAAQUDGHA)
#ifdef JP
                            ? "アンデッドのしもべを近くに呼び集めることができない" :
#else
                            ? "recall your undead slaves" :
#endif
                    (you.religion == GOD_YREDELEMNUL)
#ifdef JP
                            ? "死体を操ることができない" :
#else
                            ? "animate corpses" :
#endif
                    (you.religion == GOD_VEHUMET)
#ifdef JP
                            ? "ヴェフメットの名に於いての殺しによって力を得ることができない" :
#else
                            ? "gain power from killing in Vehumet's name" :
#endif
                    (you.religion == GOD_MAKHLEB)
#ifdef JP
                            ? "マクレブの名に於いての殺しによって力を得ることができない" :
#else
                            ? "gain power from killing in Makhleb's name" :
#endif
                    (you.religion == GOD_OKAWARU)
#ifdef JP
                            ? "強大な腕力を引き出すことができない" :
#else
                            ? "give your body great, but temporary, strength" :
#endif
                    (you.religion == GOD_TROG)
#ifdef JP
                            ? "意思によってバーサーク状態になることができない" :
#else
                            ? "go berserk at will" :
#endif
                    (you.religion == GOD_ELYVILON)
#ifdef JP
                            ? "小さき癒しをエリヴィロンに請うことができない"
#else
                            ? "call upon Elyvilon for minor healing."
#endif
                    // Unknown god
#ifdef JP
                            : "このプログラムバグに耐えることができない @30");
#else
                            : "endure this program bug @30");
#endif

#ifdef JP
                strcat(info, "。");
#else
                strcat(info, ".");
#endif
                god_speaks(you.religion, info);
                break;
            }
        }
    }
}                               // end lose_piety()

void divine_retribution( int god )
{
    ASSERT(god != GOD_NO_GOD);

    int loopy = 0;              // general purpose loop variable {dlb}
    int temp_rand = 0;          // probability determination {dlb}
    int punisher = MONS_PROGRAM_BUG;
    bool success = false;
    int how_many = 0;
    int divine_hurt = 0;

    // Good gods don't use divine retribution on their followers, they
    // will consider it for those who have gone astray however.
    if (god == you.religion)
    {
        if (god == GOD_SHINING_ONE || god == GOD_ZIN || god == GOD_ELYVILON)
            return;
    }

    // Just the thought of retribution (getting this far) mollifies the
    // god by a point... the punishment might reduce penance further.
    dec_penance( god, 1 + random2(3) );

    switch (god)
    {
    case GOD_XOM:
        {
            // One in ten chance that Xom might do something good...
            // but that isn't forced, bad things are though
            bool nice = one_chance_in(10);
            bool force = !nice;

            Xom_acts(nice, you.experience_level, force);
        }
        break;

    case GOD_SHINING_ONE:
        // daeva/smiting theme
        // Doesn't care unless you've gone over to evil/destructive gods
        if (you.religion == GOD_KIKUBAAQUDGHA || you.religion == GOD_MAKHLEB
            || you.religion == GOD_YREDELEMNUL || you.religion == GOD_VEHUMET)
        {
            if (coinflip())
            {
                success = false;
                how_many = 1 + random2(you.experience_level / 5) + random2(3);

                for (loopy = 0; loopy < how_many; loopy++)
                {
                    if (create_monster( MONS_DAEVA, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                    {
                        success = true;
                    }
                }

                if (success)
                {
#ifdef JP
                    simple_god_message( "はあなたの悪行を処罰するために、神の代行者を送り込んだ！", god );
#else
                    simple_god_message( " sends the divine host to punish you for your evil ways!", god );
#endif
                }
            }
            else
            {
                divine_hurt = 10 + random2(10);

                for (loopy = 0; loopy < 5; loopy++)
                    divine_hurt += random2( you.experience_level );

                if (!player_under_penance() && you.piety > random2(400))
                {
#ifdef JP
                    strcpy(info, "定命の者よ、『輝けるもの』の怒りを逸らしてやろう"
#else
                    strcpy(info, "Mortal, I have averted the wrath of "
#endif
#ifdef JP
                        "……今回はな。");
#else
                        "the Shining One... this time.");
#endif
                    god_speaks(you.religion, info);
                }
                else
                {
#ifdef JP
                    simple_god_message( "はあなたに一撃を見舞った！", god );
#else
                    simple_god_message( " smites you!", god );
#endif
                    ouch( divine_hurt, 0, KILLED_BY_TSO_SMITING );
                    dec_penance( GOD_SHINING_ONE, 1 );
                }
            }
        }
        break;

    case GOD_ZIN:
        // angels/creeping doom theme:
        // Doesn't care unless you've gone over to evil
        if (you.religion == GOD_KIKUBAAQUDGHA || you.religion == GOD_MAKHLEB
            || you.religion == GOD_YREDELEMNUL || you.religion == GOD_VEHUMET)
        {
            if (random2(you.experience_level) > 7 && !one_chance_in(5))
            {
                success = false;
                how_many = 1 + (you.experience_level / 10) + random2(3);

                for (loopy = 0; loopy < how_many; loopy++)
                {
                    if (create_monster(MONS_ANGEL, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                    {
                        success = true;
                    }
                }

                if (success)
                {
#ifdef JP
                    simple_god_message("はあなたの悪業を処罰するために、神の代行者を送り込んだ！", god);
#else
                    simple_god_message(" sends the divine host to punish you for your evil ways!", god);
#endif
                }
            }
            else
            {
                // god_gift == false gives unfriendly
                summon_swarm( you.experience_level * 20, true, false );
#ifdef JP
                simple_god_message("はあなたに悪疫の猛威を放った！", god);
#else
                simple_god_message(" sends a plague down upon you!", god);
#endif
            }
        }
        break;

    case GOD_MAKHLEB:
        // demonic servant theme
        if (random2(you.experience_level) > 7 && !one_chance_in(5))
        {
            if (create_monster(MONS_EXECUTIONER + random2(5), 0,
                               BEH_HOSTILE, you.x_pos, you.y_pos,
                               MHITYOU, 250) != -1)
            {
#ifdef JP
                simple_god_message("はあなたの後釜に、大いなるしもべを送り込んだ！",
#else
                simple_god_message(" sends a greater servant after you!",
#endif
                                   god);
            }
        }
        else
        {
            success = false;
            how_many = 1 + (you.experience_level / 7);

            for (loopy = 0; loopy < how_many; loopy++)
            {
                if (create_monster(MONS_NEQOXEC + random2(5), 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("はあなたを処罰するために手先を送り込んだ。", god);
#else
                simple_god_message(" sends minions to punish you.", god);
#endif
        }
        break;

    case GOD_KIKUBAAQUDGHA:
        // death/necromancy theme
        if (random2(you.experience_level) > 7 && !one_chance_in(5))
        {
            success = false;
            how_many = 1 + (you.experience_level / 5) + random2(3);

            for (loopy = 0; loopy < how_many; loopy++)
            {
                if (create_monster(MONS_REAPER, 0, BEH_HOSTILE, you.x_pos,
                                   you.y_pos, MHITYOU, 250) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("はあなたの元に死神を放った！", god);
#else
                simple_god_message(" unleashes Death upon you!", god);
#endif
        }
        else
        {
#ifdef JP
            god_speaks(god, (coinflip()) ? "あなたはキクバークッグァが甲高く笑い声を上げるのを耳にした。"
                                         : "キクバークッグァの悪意があなたに集中した。");
#else
            god_speaks(god, (coinflip()) ? "You hear Kikubaaqudgha cackling."
                                         : "Kikubaaqudgha's malice focuses upon you.");
#endif

            miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                            random2avg(88, 3), 100, "キクバークッグァの悪意" );
#else
                            random2avg(88, 3), 100, "the malice of Kikubaaqudgha" );
#endif
        }
        break;

    case GOD_YREDELEMNUL:
        // undead theme
        if (random2(you.experience_level) > 4)
        {
            success = false;
            how_many = 1 + random2(1 + (you.experience_level / 5));

            for (loopy = 0; loopy < how_many; loopy++)
            {
                temp_rand = random2(100);

                punisher = ((temp_rand > 66) ? MONS_WRAITH :            // 33%
                            (temp_rand > 52) ? MONS_WIGHT :             // 12%
                            (temp_rand > 40) ? MONS_SPECTRAL_WARRIOR :  // 16%
                            (temp_rand > 31) ? MONS_ROTTING_HULK :      //  9%
                            (temp_rand > 23) ? MONS_SKELETAL_WARRIOR :  //  8%
                            (temp_rand > 16) ? MONS_VAMPIRE :           //  7%
                            (temp_rand > 10) ? MONS_GHOUL :             //  6%
                            (temp_rand >  4) ? MONS_MUMMY               //  6%
                                             : MONS_FLAYED_GHOST);      //  5%

                if (create_monster( punisher, 0, BEH_HOSTILE,
                                    you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("はあなたを処罰するためにしもべを送り込んだ。", god);
#else
                simple_god_message(" sends a servant to punish you.", god);
#endif
        }
        else
        {
#ifdef JP
            simple_god_message("の怒りが一瞬あなたに向けられた。",
#else
            simple_god_message("'s anger turns toward you for a moment.",
#endif
                               god);

            miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                            random2avg(88, 3), 100, "イレデレンヌルの怒り" );
#else
                            random2avg(88, 3), 100, "the anger of Yredelemnul" );
#endif
        }
        break;

    case GOD_TROG:
        // physical/berserk theme
        switch (random2(6))
        {
        case 0:
        case 1:
        case 2:
            {
                // Would be better if berserking monsters were available,
                // we just send some big bruisers for now.
                success = false;

                int points = 3 + you.experience_level * 3;

                while (points > 0)
                {
                    if (points > 20 && coinflip())
                    {
                        // quick reduction for large values
                        punisher = MONS_DEEP_TROLL;
                        points -= 15;
                        break;
                    }
                    else
                    {
                        switch (random2(10))
                        {
                        case 0:
                            punisher = MONS_IRON_TROLL;
                            points -= 10;
                            break;

                        case 1:
                            punisher = MONS_ROCK_TROLL;
                            points -= 10;
                            break;

                        case 2:
                            punisher = MONS_TROLL;
                            points -= 6;
                            break;

                        case 3:
                        case 4:
                            punisher = MONS_MINOTAUR;
                            points -= 3;
                            break;

                        case 5:
                        case 6:
                            punisher = MONS_TWO_HEADED_OGRE;
                            points -= 4;
                            break;

                        case 7:
                        case 8:
                        case 9:
                        default:
                            punisher = MONS_OGRE;
                            points -= 3;
                        }
                    }

                    if (create_monster(punisher, 0, BEH_HOSTILE, you.x_pos,
                                       you.y_pos, MHITYOU, 250) != -1)
                    {
                        success = true;
                    }
                }

                if (success)
#ifdef JP
                    simple_god_message("はあなたを処罰するためにモンスターを送り込んだ。", god);
#else
                    simple_god_message(" sends monsters to punish you.", god);
#endif
            }
            break;

        case 3:
        case 4:
#ifdef JP
            simple_god_message("の声が轟いた。『我が怒りを思い知れ！』", god );
#else
            simple_god_message("'s voice booms out, \"Feel my wrath!\"", god );
#endif

            // A collection of physical effects that might be better
            // suited to Trog than wild fire magic... messages could
            // be better here... something more along the lines of apathy
            // or loss of rage to go with the anti-berzerk effect-- bwr
            switch (random2(6))
            {
            case 0:
                potion_effect( POT_DECAY, 100 );
                break;

            case 1:
            case 2:
                lose_stat(STAT_STRENGTH, 1 + random2(you.strength / 5), true);
                break;

            case 3:
                if (!you.paralysis)
                {
                    dec_penance(GOD_TROG, 3);
#ifdef JP
                    mpr( "あなたは突然、気を失った！", MSGCH_WARN );
#else
                    mpr( "You suddenly pass out!", MSGCH_WARN );
#endif
                    you.paralysis = 2 + random2(6);
                }
                break;

            case 4:
            case 5:
                if (you.slow < 90)
                {
                    dec_penance( GOD_TROG, 1 );
#ifdef JP
                    mpr( "あなたは突然に疲労困憊した！", MSGCH_WARN );
#else
                    mpr( "You suddenly feel exhausted!", MSGCH_WARN );
#endif
                    you.exhausted = 100;
                    slow_player( 100 );
                }
                break;
            };
            break;
        //jmf: returned Trog's old Fire damage
        // -- actually, this function partially exists to remove that,
        //    we'll leave this effect in, but we'll remove the wild
        //    fire magic. -- bwr
        case 5:
            dec_penance(GOD_TROG, 2);
#ifdef JP
            mpr( "あなたはトログの燃え盛る怒りが振り下ろされるのを感じた！", MSGCH_WARN );
#else
            mpr( "You feel Trog's fiery rage upon you!", MSGCH_WARN );
#endif
            miscast_effect( SPTYP_FIRE, 8 + you.experience_level,
#ifdef JP
                            random2avg(98, 3), 100, "トログの燃え盛る怒り" );
#else
                            random2avg(98, 3), 100, "the fiery rage of Trog" );
#endif
            break;
        }
        break;

    case GOD_OKAWARU:
        {
            // warrior theme:
            success = false;
            how_many = 1 + (you.experience_level / 5);

            for (loopy = 0; loopy < how_many; loopy++)
            {
                temp_rand = random2(100);

                punisher = ((temp_rand > 84) ? MONS_ORC_WARRIOR :
                            (temp_rand > 69) ? MONS_ORC_KNIGHT :
                            (temp_rand > 59) ? MONS_NAGA_WARRIOR :
                            (temp_rand > 49) ? MONS_CENTAUR_WARRIOR :
                            (temp_rand > 39) ? MONS_STONE_GIANT :
                            (temp_rand > 29) ? MONS_FIRE_GIANT :
                            (temp_rand > 19) ? MONS_FROST_GIANT :
                            (temp_rand >  9) ? MONS_CYCLOPS :
                            (temp_rand >  4) ? MONS_HILL_GIANT
                                             : MONS_TITAN);

                if (create_monster(punisher, 0, BEH_HOSTILE,
                                   you.x_pos, you.y_pos, MHITYOU, 250) != -1)
                {
                    success = true;
                }
            }

            if (success)
#ifdef JP
                simple_god_message("があなたに対して軍勢を送り込んだ！", god);
#else
                simple_god_message(" sends forces against you!", god);
#endif
        }
        break;

    case GOD_VEHUMET:
        // conjuration and summoning theme
#ifdef JP
        simple_god_message("の報復があなたを捉えた。", god);
#else
        simple_god_message("'s vengence finds you.", god);
#endif
        miscast_effect( coinflip() ? SPTYP_CONJURATION : SPTYP_SUMMONING,
                        8 + you.experience_level, random2avg(98, 3), 100,
#ifdef JP
                        "ヴェフメットの怒り" );
#else
                        "the wrath of Vehumet" );
#endif
        break;

    case GOD_NEMELEX_XOBEH:
        // like Xom, this might actually help the player -- bwr`
#ifdef JP
        simple_god_message("はあなたが処罰のデッキからカードを引くように仕向けた。",
#else
        simple_god_message(" makes you to draw from the Deck of Punishment.",
#endif
                           god);
        deck_of_cards(DECK_OF_PUNISHMENT);
        break;

    case GOD_SIF_MUNA:
#ifdef JP
        simple_god_message("の怒りがあなたを捉えた。", god);
#else
        simple_god_message("'s wrath finds you.", god);
#endif
        dec_penance(GOD_SIF_MUNA, 1);

        // magic and intelligence theme:
        switch (random2(10))
        {
        case 0:
        case 1:
            lose_stat(STAT_INTELLIGENCE, 1 + random2( you.intel / 5 ), true);
            break;

        case 2:
        case 3:
        case 4:
            confuse_player( 3 + random2(10), false );
            break;

        case 5:
        case 6:
#ifdef JP
            //miscast_effect(SK_DIVINATIONS, 9, 90, 100, "シフ・ムーナの意思");
            miscast_effect(SPTYP_DIVINATION, 9, 90, 100, "シフ・ムーナの意思"); //fixed 2005/04/02
#else
            //miscast_effect(SK_DIVINATIONS, 9, 90, 100, "the will of Sif Muna");
            miscast_effect(SPTYP_DIVINATION, 9, 90, 100, "the will of Sif Muna"); //fixed 2005/04/02
#endif
            break;

        case 7:
        case 8:
            if (you.magic_points)
            {
                dec_mp( 100 );  // this should zero it.
#ifdef JP
                mpr( "あなたは突然に魔力を吸い取られた！",
#else
                mpr( "You suddenly feel drained of magical energy!",
#endif
                     MSGCH_WARN );
            }
            break;

        case 9:
            // This will set all the extendable duration spells to
            // a duration of one round, thus potentially exposing
            // the player to real danger.
            antimagic();
#ifdef JP
            mpr( "あなたは魔法が消沈するのを知覚した。", MSGCH_WARN );
#else
            mpr( "You sense a dampening of magic.", MSGCH_WARN );
#endif
            break;
        }
        break;

    case GOD_ELYVILON:  // Elyvilon doesn't seek revenge
    default:
        return;
    }

    // Sometimes divine experiences are overwelming...
    if (one_chance_in(5) && you.experience_level < random2(37))
    {
        if (coinflip())
            confuse_player( 3 + random2(10) );
        else
        {
            if (you.slow < 90)
            {
#ifdef JP
                mpr( "神との接触があなたを疲労困憊させた！",
#else
                mpr( "The divine experience leaves you feeling exhausted!",
#endif
                     MSGCH_WARN );

                slow_player( random2(20) );
            }
        }
    }

    return;
}                               // end divine_retribution()

void excommunication(void)
{
    const int old_god = you.religion;

    you.duration[DUR_PRAYER] = 0;
    you.religion = GOD_NO_GOD;
    you.piety = 0;
    redraw_skill( you.your_name, player_title() );

#ifdef JP
    mpr("あなたは今までの信仰を棄てた！");
#else
    mpr("You have lost your religion!");
#endif
    more();

    switch (old_god)
    {
    case GOD_XOM:
        Xom_acts( false, (you.experience_level * 2), true );
        inc_penance( old_god, 50 );
        break;

    case GOD_KIKUBAAQUDGHA:
#ifdef JP
        simple_god_message( "はあなたの信仰放棄を快く思っていない！", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                        random2avg(88, 3), 100, "キクバークッグァの悪意" );
#else
                        random2avg(88, 3), 100, "the malice of Kikubaaqudgha" );
#endif
        inc_penance( old_god, 30 );
        break;

    case GOD_YREDELEMNUL:
#ifdef JP
        simple_god_message( "はあなたの信仰放棄を快く思っていない！", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( SPTYP_NECROMANCY, 5 + you.experience_level,
#ifdef JP
                        random2avg(88, 3), 100, "イレデレンヌルの立腹" );
#else
                        random2avg(88, 3), 100, "the anger of Yredelemnul" );
#endif
        inc_penance( old_god, 30 );
        break;

    case GOD_VEHUMET:
#ifdef JP
        simple_god_message( "はあなたの信仰放棄を快く思っていない！", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( (coinflip() ? SPTYP_CONJURATION : SPTYP_SUMMONING),
                        8 + you.experience_level, random2avg(98, 3), 100,
#ifdef JP
                        "ヴェフメットの激怒" );
#else
                        "the wrath of Vehumet" );
#endif
        inc_penance( old_god, 25 );
        break;

    case GOD_MAKHLEB:
#ifdef JP
        simple_god_message( "はあなたの信仰放棄を快く思っていない！", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif
        miscast_effect( (coinflip() ? SPTYP_CONJURATION : SPTYP_SUMMONING),
                        8 + you.experience_level, random2avg(98, 3), 100,
#ifdef JP
                        "マクレブの憤怒" );
#else
                        "the fury of Makhleb" );
#endif
        inc_penance( old_god, 25 );
        break;

    case GOD_TROG:
#ifdef JP
        simple_god_message( "はあなたの信仰放棄を快く思っていない！", old_god );
#else
        simple_god_message( " does not appreciate desertion!", old_god );
#endif

        // Penence has to come before retribution to prevent "mollify"
        inc_penance( old_god, 50 );
        divine_retribution( old_god );
        break;

    // these like to haunt players for a bit more than the standard
    case GOD_NEMELEX_XOBEH:
    case GOD_SIF_MUNA:
        inc_penance( old_god, 50 );
        break;

    default:
        inc_penance( old_god, 25 );
        break;

    case GOD_ELYVILON:  // never seeks revenge
        break;
    }
}                               // end excommunication()


void altar_prayer(void)
{
    int   i, j, next;
    char  subst_id[4][50];
    char str_pass[ ITEMNAME_SIZE ];

    for (i = 0; i < 4; i++)
    {
        for (j = 0; j < 50; j++)
        {
            subst_id[i][j] = 1;
        }
    }

#ifdef JP
    mpr( "あなたは祭壇に跪いて、祈りを捧げた。" );
#else
    mpr( "You kneel at the altar and pray." );
#endif

    if (you.religion == GOD_SHINING_ONE || you.religion == GOD_XOM)
        return;

    i = igrd[you.x_pos][you.y_pos];
    while (i != NON_ITEM)
    {
        if (one_chance_in(1000))
            break;

        next = mitm[i].link;  // in case we can't get it later.

        const int value = item_value( mitm[i], subst_id, true );

        switch (you.religion)
        {
        case GOD_ZIN:
        case GOD_OKAWARU:
        case GOD_MAKHLEB:
        case GOD_NEMELEX_XOBEH:
#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            if (mitm[i].base_type == OBJ_CORPSES
                || random2(value) >= 50
                || player_under_penance())
            {
                gain_piety(1);
            }

            destroy_item(i);
            break;

        case GOD_SIF_MUNA:
#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            if (value >= 150)
                gain_piety(1 + random2(3));

            destroy_item(i);
            break;

        case GOD_KIKUBAAQUDGHA:
        case GOD_TROG:
            if (mitm[i].base_type != OBJ_CORPSES)
                break;

#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            gain_piety(1);
            destroy_item(i);
            break;

        case GOD_ELYVILON:
            if (mitm[i].base_type != OBJ_WEAPONS
                && mitm[i].base_type != OBJ_MISSILES)
            {
                break;
            }

#ifdef JP
            it_name(i, DESC_PLAIN, str_pass);
#else
            it_name(i, DESC_CAP_THE, str_pass);
#endif
            strcpy(info, str_pass);
            strcat(info, sacrifice[you.religion - 1]);
            mpr(info);

            if (random2(value) >= random2(50)
                || (mitm[i].base_type == OBJ_WEAPONS
                    && (you.piety < 30 || player_under_penance())))
            {
                gain_piety(1);
            }

            destroy_item(i);
            break;

        default:
            break;
        }

        i = next;
    }
}                               // end altar_prayer()

void god_pitch(unsigned char which_god)
{
#ifdef JP
    strcpy(info, "あなたは");
#else
    strcpy(info, "You kneel at the altar of ");
#endif
    strcat(info, god_name(which_god));
#ifdef JP
    strcat(info, "の祭壇に跪いた。");
#else
    strcat(info, ".");
#endif
    mpr(info);

    more();

    // Note: using worship we could make some gods not allow followers to
    // return, or not allow worshippers from other religions.  -- bwr

    if ((you.is_undead || you.species == SP_DEMONSPAWN)
        && (which_god == GOD_ZIN || which_god == GOD_SHINING_ONE
            || which_god == GOD_ELYVILON))
    {
#ifdef JP
        simple_god_message("はあなたのような存在からの崇拝は受け容れない！",
#else
        simple_god_message(" does not accept worship from those such as you!",
#endif
                           which_god);
        return;
    }

    describe_god( which_god, false );

#ifdef JP
    snprintf( info, INFO_SIZE, "あなたはこの宗派を%s信仰することを望みますか？",
              (you.worshipped[which_god]) ? "再び" : "" );
#else
    snprintf( info, INFO_SIZE, "Do you wish to %sjoin this religion?",
              (you.worshipped[which_god]) ? "re" : "" );
#endif

    if (!yesno( info ))
    {
        redraw_screen();
        return;
    }

#ifdef JP
    if (!yesno("信仰の意思は確かですか？"))
#else
    if (!yesno("Are you sure?"))
#endif
    {
        redraw_screen();
        return;
    }

    redraw_screen();
    if (you.religion != GOD_NO_GOD)
        excommunication();

    you.religion = which_god;   //jmf: moved up so god_speaks gives right colour
    you.piety = 15;             // to prevent near instant excommunication
    you.gift_timeout = 0;
    set_god_ability_slots();    // remove old god's slots, reserve new god's

#ifdef JP
    snprintf( info, INFO_SIZE, "はあなたの入信を%s歓迎した！",
              (you.worshipped[which_god]) ? "再び" : "" );
#else
    snprintf( info, INFO_SIZE, " welcomes you%s!",
              (you.worshipped[which_god]) ? " back" : "" );
#endif

    simple_god_message( info );
    more();

    if (you.worshipped[you.religion] < 100)
        you.worshipped[you.religion]++;

    // Currently penance is just zeroed, this could be much more interesting.
    you.penance[you.religion] = 0;

    if (you.religion == GOD_KIKUBAAQUDGHA || you.religion == GOD_YREDELEMNUL
        || you.religion == GOD_VEHUMET || you.religion == GOD_MAKHLEB)
    {
        // Note:  Using worshipped[] we could make this sort of grudge
        // permanent instead of based off of penance. -- bwr
        if (you.penance[GOD_SHINING_ONE] > 0)
        {
            inc_penance(GOD_SHINING_ONE, 30);
#ifdef JP
            god_speaks(GOD_SHINING_ONE, "『定命の者よ、お前は悪業の代価を支払うことになろう！！』");
#else
            god_speaks(GOD_SHINING_ONE, "\"You will pay for your evil ways, mortal!\"");
#endif
        }
    }
    redraw_skill( you.your_name, player_title() );
}                               // end god_pitch()

void offer_corpse(int corpse)
{
    char str_pass[ ITEMNAME_SIZE ];
#ifdef JP
    it_name(corpse, DESC_PLAIN, str_pass);
#else
    it_name(corpse, DESC_CAP_THE, str_pass);
#endif
    strcpy(info, str_pass);
    strcat(info, sacrifice[you.religion - 1]);
    mpr(info);

    done_good(GOOD_HACKED_CORPSE, 10);
}                               // end offer_corpse()

//jmf: moved stuff from items::handle_time()
void handle_god_time(void)
{
    if (one_chance_in(100))
    {
        // Choose a god randomly from those to whom we owe penance.
        //
        // Proof: (By induction)
        //
        // 1) n = 1, probability of choosing god is one_chance_in(1)
        // 2) Asuume true for n = k (ie. prob = 1 / n for all n)
        // 3) For n = k + 1,
        //
        //      P:new-found-god = 1 / n (see algorithm)
        //      P:other-gods = (1 - P:new-found-god) * P:god-at-n=k
        //                             1        1
        //                   = (1 - -------) * ---
        //                           k + 1      k
        //
        //                          k         1
        //                   = ----------- * ---
        //                        k + 1       k
        //
        //                       1       1
        //                   = -----  = ---
        //                     k + 1     n
        //
        // Therefore, by induction the probability is uniform.  As for
        // why we do it this way... it requires only one pass and doesn't
        // require an array.

        int which_god = GOD_NO_GOD;
        unsigned int count = 0;

        for (int i = GOD_NO_GOD; i < NUM_GODS; i++)
        {
            if (you.penance[i])
            {
                count++;
                if (one_chance_in(count))
                    which_god = i;
            }
        }

        if (which_god != GOD_NO_GOD)
            divine_retribution(which_god);
    }

    // Update the god's opinion of the player
    if (you.religion != GOD_NO_GOD)
    {
        switch (you.religion)
        {
        case GOD_XOM:
            if (one_chance_in(75))
                Xom_acts(true, you.experience_level + random2(15), true);
            break;

        case GOD_ZIN:           // These gods like long-standing worshippers
        case GOD_ELYVILON:
            if (you.piety < 150 && one_chance_in(20))
                gain_piety(1);
            break;

        case GOD_SHINING_ONE:
            if (you.piety < 150 && one_chance_in(15))
                gain_piety(1);
            break;

        case GOD_YREDELEMNUL:
        case GOD_KIKUBAAQUDGHA:
        case GOD_VEHUMET:
            if (one_chance_in(17))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_OKAWARU: // These gods accept corpses, so they time-out faster
        case GOD_TROG:
            if (one_chance_in(14))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_MAKHLEB:
            if (one_chance_in(16))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_SIF_MUNA:
            if (one_chance_in(20))
                lose_piety(1);
            if (you.piety < 1)
                excommunication();
            break;

        case GOD_NEMELEX_XOBEH: // relatively patient
            if (one_chance_in(35))
                lose_piety(1);
            if (you.attribute[ATTR_CARD_COUNTDOWN] > 0 && coinflip())
                you.attribute[ATTR_CARD_COUNTDOWN]--;
            if (you.piety < 1)
                excommunication();
            break;

        default:
#ifdef JP
            DEBUGSTR("まずい神だ。主教もいない！");
#else
            DEBUGSTR("Bad god, no bishop!");
#endif
        }
    }
}                               // end handle_god_time()

// yet another wrapper for mpr() {dlb}:
void simple_god_message(const char *event, int which_deity)
{
    char buff[ INFO_SIZE ];

    if (which_deity == GOD_NO_GOD)
        which_deity = you.religion;

    snprintf( buff, sizeof(buff), "%s%s", god_name( which_deity ), event );

    god_speaks( which_deity, buff );
}

char god_colour( char god ) //mv - added
{
    switch (god)
    {
    case GOD_SHINING_ONE:
    case GOD_ZIN:
    case GOD_ELYVILON:
    case GOD_OKAWARU:
        return(CYAN);

    case GOD_YREDELEMNUL:
    case GOD_KIKUBAAQUDGHA:
    case GOD_MAKHLEB:
    case GOD_VEHUMET:
    case GOD_TROG:
        return(LIGHTRED);

    case GOD_XOM:
        return(YELLOW);

    case GOD_NEMELEX_XOBEH:
        return(LIGHTMAGENTA);

    case GOD_SIF_MUNA:
        return(LIGHTBLUE);

    case GOD_NO_GOD:
    default:
        break;
    }

    return(YELLOW);
}
