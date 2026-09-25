/*
 *  File:       it_use2.cc
 *  Summary:    Functions for using wands, potions, and weapon/armour removal.4\3
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      26jun2000   jmf   added ZAP_MAGMA
 *  <4> 19mar2000   jmf   Added ZAP_BACKLIGHT and ZAP_SLEEP
 *  <3>     10/1/99         BCR             Changed messages for speed and
 *                                          made amulet resist slow up speed
 *  <2>     5/20/99         BWR             Fixed bug with RAP_METABOLISM
 *                                          and RAP_NOISES artefacts/
 *  <1>     -/--/--         LRH             Created
 */

#include "AppHdr.h"
#include "it_use2.h"

#include <stdio.h>
#include <string.h>

#include "externs.h"

#include "beam.h"
#include "effects.h"
#include "food.h"
#include "itemname.h"
#include "misc.h"
#include "mutation.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills2.h"
#include "spells2.h"
#include "spl-cast.h"
#include "stuff.h"
#include "view.h"

// From an actual potion, pow == 40 -- bwr
bool potion_effect( char pot_eff, int pow )
{
    bool effect = true;  // current behaviour is all potions id on quaffing

    int new_value = 0;
    unsigned char i;

    if (pow > 150)
        pow = 150;

    switch (pot_eff)
    {
    case POT_HEALING:
#ifdef JP
        mpr("あなたは気分が良くなった。");
#else
        mpr("You feel better.");
#endif
        inc_hp(5 + random2(7), false);

        // only fix rot when healed to full
        if (you.hp == you.hp_max)
        {
            unrot_hp(1);
            set_hp(you.hp_max, false);
        }

        you.poison = 0;
        you.rotting = 0;
        you.disease = 0;
        you.conf = 0;
        break;

    case POT_HEAL_WOUNDS:
#ifdef JP
        mpr("あなたはかなり体調が良くなった。");
#else
        mpr("You feel much better.");
#endif
        inc_hp(10 + random2avg(28, 3), false);

        // only fix rot when healed to full
        if (you.hp == you.hp_max)
        {
            unrot_hp( 2 + random2avg(5, 2) );
            set_hp(you.hp_max, false);
        }
        break;

    case POT_SPEED:
        haste_player( 40 + random2(pow) );
        break;

    case POT_MIGHT:
        {
            bool were_mighty = (you.might > 0);

            if (!were_mighty)
#ifdef JP
                mpr( "あなたは突然、非常に力強くなった。" );
#else
                mpr( "You feel very mighty all of a sudden." );
#endif
            else
            {
#ifdef JP
                mpr( "あなたの力強さは今も続いている。" );
#else
                mpr( "You still feel pretty mighty." );
#endif
                contaminate_player(1);
            }

            // conceivable max gain of +184 {dlb}
            you.might += 35 + random2(pow);

            if (!were_mighty)
                modify_stat(STAT_STRENGTH, 5, true);

            // files.cc permits values up to 215, but ... {dlb}
            if (you.might > 80)
                you.might = 80;

            naughty( NAUGHTY_STIMULANTS, 4 + random2(4) );
        }
        break;

    case POT_GAIN_STRENGTH:
        mutate(MUT_STRONG);
        break;

    case POT_GAIN_DEXTERITY:
        mutate(MUT_AGILE);
        break;

    case POT_GAIN_INTELLIGENCE:
        mutate(MUT_CLEVER);
        break;

    case POT_LEVITATION:
#ifdef JP
        strcpy(info, "あなたの体は");
        strcat(info, (!player_is_levitating()) ? "かなりの" : "更に");
        strcat(info, "浮力を持った。");
#else
        strcpy(info, "You feel");
        strcat(info, (!player_is_levitating()) ? " very" : " more");
        strcat(info, " buoyant.");
#endif
        mpr(info);

        if (!player_is_levitating())
#ifdef JP
            mpr("あなたは床から緩やかに上昇していった。");
#else
            mpr("You gently float upwards from the floor.");
#endif

        you.levitation += 25 + random2(pow);

        if (you.levitation > 100)
            you.levitation = 100;

        burden_change();
        break;

    case POT_POISON:
    case POT_STRONG_POISON:
        if (player_res_poison())
        {
#ifdef JP
            snprintf( info, INFO_SIZE, "あなたは%s吐き気を催した。",
                        (pot_eff == POT_POISON) ? "軽い" : "ひどい" );
#else
            snprintf( info, INFO_SIZE, "You feel %s nauseous.",
                        (pot_eff == POT_POISON) ? "slightly" : "quite" );
#endif

            mpr(info);
        }
        else
        {
#ifdef JP
            snprintf( info, INFO_SIZE, "この液体は%sひどい味がする。",
                        (pot_eff == POT_POISON) ? "とても" : "極めて" );
#else
            snprintf( info, INFO_SIZE, "That liquid tasted %s nasty...",
                        (pot_eff == POT_POISON) ? "very" : "extremely" );
#endif

            mpr(info);

            poison_player( ((pot_eff == POT_POISON) ? 1 + random2avg(5, 2)
                                                    : 3 + random2avg(13, 2)) );
        }
        break;

    case POT_SLOWING:
        slow_player( 10 + random2(pow) );
        break;

    case POT_PARALYSIS:
#ifdef JP
        snprintf( info, INFO_SIZE, "あなたは体の自由を%s！",
                    (you.paralysis) ? "いまだに失っている" : "突然に失った" );
#else
        snprintf( info, INFO_SIZE, "You %s the ability to move!",
                    (you.paralysis) ? "still haven't" : "suddenly lose" );
#endif

        mpr( info, MSGCH_WARN );

        new_value = 2 + random2( 6 + you.paralysis );

        if (new_value > you.paralysis)
            you.paralysis = new_value;

        if (you.paralysis > 13)
            you.paralysis = 13;
        break;

    case POT_CONFUSION:
        confuse_player( 3 + random2(8) );
        break;

    case POT_INVISIBILITY:
#ifdef JP
        mpr( (!you.invis) ? "あなたは透明になっていった！"
                          : "あなたは安全に身を隠したままだ。" );
#else
        mpr( (!you.invis) ? "You fade into invisibility!"
                          : "You feel safely hidden away." );
#endif

        // now multiple invisiblity casts aren't as good -- bwr
        if (!you.invis)
            you.invis = 15 + random2(pow);
        else
            you.invis += random2(pow);

        if (you.invis > 100)
            you.invis = 100;
        break;

    // carnivore check here? {dlb}
    case POT_PORRIDGE:          // oatmeal - always gluggy white/grey?
#ifdef JP
        mpr("この薬は粥そのものの味がする！");
#else
        mpr("That potion was really gluggy!");
#endif
        lessen_hunger(6000, true);
        break;

    case POT_DEGENERATION:
#ifdef JP
        mpr("この液体は何かとてつもなく危険な代物だ！");
#else
        mpr("There was something very wrong with that liquid!");
#endif
        lose_stat(STAT_RANDOM, 1 + random2avg(4, 2));
        break;

    // Don't generate randomly - should be rare and interesting
    case POT_DECAY:
        if (you.is_undead)
#ifdef JP
            mpr( "あなたはひどい気分になった。" );
#else
            mpr( "You feel terrible." );
#endif
        else
            rot_player( 10 + random2(10) );
        break;

    case POT_WATER:
#ifdef JP
        mpr("水のような味がする。");
#else
        mpr("This tastes like water.");
#endif
        // we should really separate thirst from hunger {dlb}
        // Thirst would just be annoying for the player, the
        // 20 points here doesn't represesent real food anyways -- bwr
        lessen_hunger(20, true);
        break;

    case POT_EXPERIENCE:
        if (you.experience_level < 27)
        {
#ifdef JP
            mpr("あなたはより経験を積んだように感じる！");
#else
            mpr("You feel more experienced!");
#endif

            you.experience = 1 + exp_needed( 2 + you.experience_level );
            level_change();
        }
        else
#ifdef JP
            mpr("記憶の洪水があなたを洗い流した。");
#else
            mpr("A flood of memories washes over you.");
#endif
        break;                  // I'll let this slip past robe of archmagi

    case POT_MAGIC:
#ifdef JP
        mpr( "あなたは魔力に満たされた！" );
#else
        mpr( "You feel magical!" );
#endif
        new_value = 5 + random2avg(19, 2);

        // increase intrinsic MP points
        if (you.magic_points + new_value > you.max_magic_points)
        {
            new_value = (you.max_magic_points - you.magic_points)
                + (you.magic_points + new_value - you.max_magic_points) / 4 + 1;
        }

        inc_mp( new_value, true );
        break;

    case POT_RESTORE_ABILITIES:
        // messaging taken care of within function {dlb}
        // not quite true... if no stat's are restore = no message, and
        // that's just confusing when trying out random potions (this one
        // still auto-identifies so we know what the effect is, but it
        // shouldn't require bringing up the descovery screen to do that -- bwr
        if (restore_stat(STAT_ALL, false) == false)
#ifdef JP
            mpr( "あなたは気分爽快になった。" );
#else
            mpr( "You feel refreshed." );
#endif
        break;

    case POT_BERSERK_RAGE:
        go_berserk(true);
        break;

    case POT_CURE_MUTATION:
#ifdef JP
        mpr("とても清浄な味がする。");
#else
        mpr("It has a very clean taste.");
#endif
        for (i = 0; i < 7; i++)
        {
            if (random2(10) > i)
                delete_mutation(100);
        }
        break;

    case POT_MUTATION:
#ifdef JP
        mpr("あなたは極めて奇妙な気分になった。");
#else
        mpr("You feel extremely strange.");
#endif
        for (i = 0; i < 3; i++)
        {
            mutate(100, false);
        }

        naughty(NAUGHTY_STIMULANTS, 4 + random2(4));
        break;
    }

    return (effect);
}                               // end potion_effect()

void unwield_item(char unw)
{
    you.special_wield = SPWLD_NONE;
    you.wield_change = true;

    if (you.inv[unw].base_type == OBJ_WEAPONS)
    {
        if (is_fixed_artefact( you.inv[unw] ))
        {
            switch (you.inv[unw].special)
            {
            case SPWPN_SINGING_SWORD:
#ifdef JP
                mpr("『謳う剣』は溜息を漏らした。");
#else
                mpr("The Singing Sword sighs.");
#endif
                break;
            case SPWPN_WRATH_OF_TROG:
#ifdef JP
                mpr("あなたの凶暴さは幾分収まった。");
#else
                mpr("You feel less violent.");
#endif
                break;
            case SPWPN_SCYTHE_OF_CURSES:
            case SPWPN_STAFF_OF_OLGREB:
                you.inv[unw].plus = 0;
                you.inv[unw].plus2 = 0;
                break;
            case SPWPN_STAFF_OF_WUCAD_MU:
                you.inv[unw].plus = 0;
                you.inv[unw].plus2 = 0;
#ifdef JP
                miscast_effect( SPTYP_DIVINATION, 9, 90, 100, "ウカド・ムーの杖" );
#else
                miscast_effect( SPTYP_DIVINATION, 9, 90, 100, "the Staff of Wucad Mu" );
#endif
                break;
            default:
                break;
            }

            return;
        }

        int brand = get_weapon_brand( you.inv[unw] );

        if (is_random_artefact( you.inv[unw] ))
            unuse_randart(unw);

        if (brand != SPWPN_NORMAL)
        {
            char str_pass[ ITEMNAME_SIZE ];
            in_name(unw, DESC_CAP_YOUR, str_pass);
            strcpy(info, str_pass);

            switch (brand)
            {
            case SPWPN_SWORD_OF_CEREBOV:
            case SPWPN_FLAMING:
#ifdef JP
                strcat(info, "は燃えるのを止めた。");
#else
                strcat(info, " stops flaming.");
#endif
                mpr(info);
                break;

            case SPWPN_FREEZING:
            case SPWPN_HOLY_WRATH:
#ifdef JP
                strcat(info, "は輝きを止めた。");
#else
                strcat(info, " stops glowing.");
#endif
                mpr(info);
                break;

            case SPWPN_ELECTROCUTION:
#ifdef JP
                strcat(info, "は爆ぜるのを止めた。");
#else
                strcat(info, " stops crackling.");
#endif
                mpr(info);
                break;

            case SPWPN_VENOM:
#ifdef JP
                strcat(info, "は毒を滲ませるのを止めた。");
#else
                strcat(info, " stops dripping with poison.");
#endif
                mpr(info);
                break;

            case SPWPN_PROTECTION:
#ifdef JP
                mpr("あなたの護りは薄くなったようだ。");
#else
                mpr("You feel less protected.");
#endif
                you.redraw_armour_class = 1;
                break;

            case SPWPN_VAMPIRICISM:
#ifdef JP
                mpr("あなたは奇妙な飢えが去っていくのを感じた。");
#else
                mpr("You feel the strange hunger wane.");
#endif
                break;

            /* case 8: draining
               case 9: speed, 10 slicing etc */

            case SPWPN_DISTORTION:
                // Removing the translocations skill reduction of effect,
                // it might seem sensible, but this brand is supposted
                // to be dangerous because it does large bonus damage,
                // as well as free teleport other side effects, and
                // even with the miscast effects you can rely on the
                // occasional spatial bonus to mow down some opponents.
                // It's far too powerful without a real risk, especially
                // if it's to be allowed as a player spell. -- bwr

                // int effect = 9 - random2avg( you.skills[SK_TRANSLOCATIONS] * 2, 2 );
#ifdef JP
                miscast_effect( SPTYP_TRANSLOCATION, 9, 90, 100, "歪曲の効果" );
#else
                miscast_effect( SPTYP_TRANSLOCATION, 9, 90, 100, "a distortion effect" );
#endif
                break;

                // when more are added here, *must* duplicate unwielding
                // effect in vorpalise weapon scroll effect in read_scoll
            }                   // end switch


            if (you.duration[DUR_WEAPON_BRAND])
            {
                you.duration[DUR_WEAPON_BRAND] = 0;
                set_item_ego_type( you.inv[unw], OBJ_WEAPONS, SPWPN_NORMAL );
#ifdef JP
                mpr("あなたの武器にかかっていた魔法が切れた。");
#else
                mpr("Your branding evaporates.");
#endif
            }
        }                       // end if
    }

    if (player_equip( EQ_STAFF, STAFF_POWER ))
    {
        // XXX: Ugly hack so that thhis currently works (don't want to
        // mess with the fact that currently this function doesn't
        // actually unwield the item, but we need it out of the player's
        // hand for this to work. -- bwr
        int tmp = you.equip[ EQ_WEAPON ];

        you.equip[ EQ_WEAPON ] = -1;
        calc_mp();
        you.equip[ EQ_WEAPON ] = tmp;
    }

    return;
}                               // end unwield_item()

// This does *not* call ev_mod!
void unwear_armour(char unw)
{
    you.redraw_armour_class = 1;
    you.redraw_evasion = 1;

    switch (get_armour_ego_type( you.inv[unw] ))
    {
    case SPARM_RUNNING:
#ifdef JP
        mpr("あなたは動きが緩慢になったようだ。");
#else
        mpr("You feel rather sluggish.");
#endif
        break;

    case SPARM_FIRE_RESISTANCE:
#ifdef JP
        mpr("『さっきからこんなに暑かったっけ？』");
#else
        mpr("\"Was it this warm in here before?\"");
#endif
        break;

    case SPARM_COLD_RESISTANCE:
#ifdef JP
        mpr("あなたは少し肌寒さを覚えた。");
#else
        mpr("You catch a bit of a chill.");
#endif
        break;

    case SPARM_POISON_RESISTANCE:
        if (!player_res_poison())
#ifdef JP
            mpr("あなたは頑健さが減退したようだ。");
#else
            mpr("You feel less healthy.");
#endif
        break;

    case SPARM_SEE_INVISIBLE:
        if (!player_see_invis())
#ifdef JP
            mpr("あなたは知覚力が低下したようだ。");
#else
            mpr("You feel less perceptive.");
#endif
        break;

    case SPARM_DARKNESS:        // I do not understand this {dlb}
        if (you.invis)
            you.invis = 1;
        break;

    case SPARM_STRENGTH:
        modify_stat(STAT_STRENGTH, -3, false);
        break;

    case SPARM_DEXTERITY:
        modify_stat(STAT_DEXTERITY, -3, false);
        break;

    case SPARM_INTELLIGENCE:
        modify_stat(STAT_INTELLIGENCE, -3, false);
        break;

    case SPARM_PONDEROUSNESS:
#ifdef JP
        mpr("足取りの軽さがあなたに戻ってきた。");
#else
        mpr("That put a bit of spring back into your step.");
#endif
        // you.speed -= 2;
        break;

    case SPARM_LEVITATION:
        //you.levitation++;
        if (you.levitation)
            you.levitation = 1;
        break;

    case SPARM_MAGIC_RESISTANCE:
#ifdef JP
        mpr("あなたは魔法への抵抗力が減退したようだ。");
#else
        mpr("You feel less resistant to magic.");
#endif
        break;

    case SPARM_PROTECTION:
#ifdef JP
        mpr("あなたの護りは薄くなったようだ。");
#else
        mpr("You feel less protected.");
#endif
        break;

    case SPARM_STEALTH:
#ifdef JP
        mpr("あなたの隠密性は低下したようだ。");
#else
        mpr("You feel less stealthy.");
#endif
        break;

    case SPARM_RESISTANCE:
#ifdef JP
        mpr("あなたは全身に熱さと冷気を感じた。");
#else
        mpr("You feel hot and cold all over.");
#endif
        break;

    case SPARM_POSITIVE_ENERGY:
#ifdef JP
        mpr("あなたは無防備になったように感じた。");
#else
        mpr("You feel vulnerable.");
#endif
        break;

    case SPARM_ARCHMAGI:
#ifdef JP
        mpr("あなたは奇妙な喪失感を覚えた。");
#else
        mpr("You feel strangely numb.");
#endif
        break;
    }

    if (is_random_artefact( you.inv[unw] ))
        unuse_randart(unw);

    return;
}                               // end unwear_armour()

void unuse_randart(unsigned char unw)
{
    ASSERT( is_random_artefact( you.inv[unw] ) );

    FixedVector< char, RA_PROPERTIES > proprt;
    randart_wpn_properties( you.inv[unw], proprt );

    if (proprt[RAP_AC])
        you.redraw_armour_class = 1;

    if (proprt[RAP_EVASION])
        you.redraw_evasion = 1;

    // modify ability scores
    modify_stat( STAT_STRENGTH,     -proprt[RAP_STRENGTH],     true );
    modify_stat( STAT_INTELLIGENCE, -proprt[RAP_INTELLIGENCE], true );
    modify_stat( STAT_DEXTERITY,    -proprt[RAP_DEXTERITY],    true );

    if (proprt[RAP_NOISES] != 0)
        you.special_wield = SPWLD_NONE;
}                               // end unuse_randart()
