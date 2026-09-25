/*
 *  File:       food.cc
 *  Summary:    Functions for eating and butchering.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <2>      5/20/99        BWR           Added CRAWL_PIZZA.
 *      <1>      -/--/--        LRH           Created
 */

#include "AppHdr.h"
#include "food.h"

#include <string.h>
// required for abs() {dlb}:
#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "debug.h"
#include "delay.h"
#include "invent.h"
#include "items.h"
#include "itemname.h"
#include "item_use.h"
#include "it_use2.h"
#include "macro.h"
#include "misc.h"
#include "mon-util.h"
#include "mutation.h"
#include "player.h"
#include "religion.h"
#include "skills2.h"
#include "spells2.h"
#include "stuff.h"
#include "wpn-misc.h"

static bool  can_ingest(int what_isit, int kindof_thing, bool suppress_msg);
static bool  eat_from_floor(void);
static int   determine_chunk_effect(int which_chunk_type, bool rotten_chunk);
static void  eat_chunk( int chunk_effect );
static void  eat_from_inventory(int which_inventory_slot);
static void  eating(unsigned char item_class, int item_type);
static void  ghoul_eat_flesh( int chunk_effect );
static void  describe_food_change(int hunger_increment);
static bool  food_change(bool suppress_message);

/*
 **************************************************
 *                                                *
 *             BEGIN PUBLIC FUNCTIONS             *
 *                                                *
 **************************************************
*/

void make_hungry( int hunger_amount, bool suppress_msg )
{
    if (you.is_undead == US_UNDEAD)
        return;

#if DEBUG_DIAGNOSTICS
    set_redraw_status( REDRAW_HUNGER );
#endif

    you.hunger -= hunger_amount;

    if (you.hunger < 0)
        you.hunger = 0;

    // so we don't get two messages, ever.
    bool state_message = food_change(false);

    if (!suppress_msg && !state_message)
        describe_food_change( -hunger_amount );
}                               // end make_hungry()

void lessen_hunger( int satiated_amount, bool suppress_msg )
{
    if (you.is_undead == US_UNDEAD)
        return;

    you.hunger += satiated_amount;

    if (you.hunger > 12000)
        you.hunger = 12000;

    // so we don't get two messages, ever
    bool state_message = food_change(false);

    if (!suppress_msg && !state_message)
        describe_food_change(satiated_amount);
}                               // end lessen_hunger()

void set_hunger( int new_hunger_level, bool suppress_msg )
{
    if (you.is_undead == US_UNDEAD)
        return;

    int hunger_difference = (new_hunger_level - you.hunger);

    if (hunger_difference < 0)
        make_hungry( abs(hunger_difference), suppress_msg );
    else if (hunger_difference > 0)
        lessen_hunger( hunger_difference, suppress_msg );
}                               // end set_hunger()

// more of a "weapon_switch back from butchering" function, switching
// to a weapon is done using the wield_weapon code.
// special cases like staves of power or other special weps are taken
// care of by calling wield_effects()    {gdl}

void weapon_switch( int targ )
{
    if (targ == -1)
    {
#ifdef JP 
        mpr( "あなたは素手に戻った。" );
#else
        mpr( "You switch back to your bare hands." );
#endif
    }
    else
    {
        char buff[80];
        in_name( targ, DESC_NOCAP_A, buff );

        char let = index_to_letter( targ );

#ifdef JP 
        snprintf( info, INFO_SIZE, "%c - %sに武器を戻した。", let, buff );
#else
        snprintf( info, INFO_SIZE, "Switching back to %c - %s.", let, buff );
#endif
        mpr( info );
    }

    // unwield the old weapon and wield the new.
    // XXX This is a pretty dangerous hack;  I don't like it.--GDL
    //
    // Well yeah, but that's because interacting with the wielding
    // code is a mess... this whole function's purpose was to 
    // isolate this hack until there's a proper way to do things. -- bwr
    if (you.equip[EQ_WEAPON] != -1)
        unwield_item(you.equip[EQ_WEAPON]);

    you.equip[EQ_WEAPON] = targ;
#ifdef USE_TILE
    if (Options.use_tile)
        TilePlayerRefresh();
#endif
    // special checks: staves of power, etc
    if (targ != -1)
        wield_effects( targ, false );
}

bool butchery(void)
{
    char str_pass[ ITEMNAME_SIZE ];
    int items_here = 0;
    int o = igrd[you.x_pos][you.y_pos];
    int k = 0;
    int item_got;
    unsigned char keyin;

    bool can_butcher = false;
    bool wpn_switch = false;
    bool new_cursed = false;
    int old_weapon = you.equip[EQ_WEAPON];

    bool barehand_butcher = (you.equip[ EQ_GLOVES ] == -1)
                && (you.species == SP_TROLL 
                    || you.species == SP_GHOUL
                    || you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS
                    || you.attribute[ATTR_TRANSFORMATION] == TRAN_DRAGON
                    || you.mutation[MUT_CLAWS]);


    if (igrd[you.x_pos][you.y_pos] == NON_ITEM)
    {
#ifdef JP 
        mpr("ここには何もない！");
#else
        mpr("There isn't anything here!");
#endif
        return (false);
    }

    if (player_is_levitating() && !wearing_amulet(AMU_CONTROLLED_FLIGHT))
    {
#ifdef JP 
        mpr("あなたは今いる高さから床に手が届かない。");
#else
        mpr("You can't reach the floor from up here.");
#endif
        return (false);
    }

    if (barehand_butcher)
        can_butcher = true;
    else
    {
        if (you.equip[EQ_WEAPON] != -1)
        {
            can_butcher = can_cut_meat( you.inv[you.equip[EQ_WEAPON]].base_type,
                                        you.inv[you.equip[EQ_WEAPON]].sub_type );
        }

        // Should probably check for cursed-weapons, bare hands and
        // non-weapons in hand here, but wield_weapon will be used for
        // this swap and it will do all that (although the player might
        // be annoyed with the excess prompt).
        if (Options.easy_butcher && !can_butcher)
        {
            //mv: check for berserk first
            if (you.berserker)
            {
#ifdef JP 
                mpr ("あなたは狂乱のあまり、肉を捌くナイフを探すことができない！");
#else
                mpr ("You are too berserk to search for a butchering knife!");
#endif
                return (false);
            }

             // We'll now proceed to look through the entire inventory for
             // choppers/slicers.  We'll skip special weapons because
             // wielding/unwielding a foo of distortion would be disastrous.
             for (int i = 0; i < ENDOFPACK; ++i)
             {
                 if (is_valid_item( you.inv[i] )
                         && can_cut_meat( you.inv[i].base_type,
                             you.inv[i].sub_type )
                         && you.inv[i].base_type == OBJ_WEAPONS
                         && item_known_uncursed(you.inv[i])
                         && item_ident( you.inv[i], ISFLAG_KNOW_TYPE )
                         && get_weapon_brand(you.inv[i])
                                 != SPWPN_DISTORTION
                         && can_wield( you.inv[i] ))
                 {
#ifdef JP 
                    mpr( "あなたは肉を捌くため武器を持ち替えた。" );
#else
                    mpr( "Switching to a butchering implement.");
#endif
                    wpn_switch = true;
                    wield_weapon( true, i );
                    break;
                  }
              }

            // if we didn't swap above, then we still can't cut... let's
            // call wield_weapon() in the "prompt the user" way...
            if (!wpn_switch)
            {
                // prompt for new weapon
#ifdef JP 
                mpr( "どの武器で肉を捌きますか？", MSGCH_PROMPT );
#else
                mpr( "What would you like to use?", MSGCH_PROMPT );
#endif
                wield_weapon( false );

                // let's see if the user did something...
                if (you.equip[EQ_WEAPON] != old_weapon)
                    wpn_switch = true;
            }
        }

        // weapon might have changed (to bare hands as well), we'll
        // update the can_butcher status accordingly (note: if we could
        // butcher with our bare hands we wouldn't be here) -- bwr
        if (wpn_switch && you.equip[EQ_WEAPON] != -1)
        {
            can_butcher = can_cut_meat( you.inv[you.equip[EQ_WEAPON]].base_type,
                                        you.inv[you.equip[EQ_WEAPON]].sub_type );
        }
    }

    // Account for the weapon switch above if it happened... we're 
    // doing this here since the above switch may reveal information
    // about the weapon (curse status, ego type).  So even if the 
    // character fails to or decides not to butcher past this point,
    // they have achieved something and there should be a cost.
    if (wpn_switch)
        start_delay( DELAY_UNINTERUPTABLE, 1, old_weapon );

    // check to see if the new implement is cursed - if so,  set a
    // flag indicating this.  If a player actually butchers anything,
    // this flag can be checked before switching back.
    int wpn = you.equip[EQ_WEAPON];

    if (wpn != -1 
        && you.inv[wpn].base_type == OBJ_WEAPONS
        && item_cursed( you.inv[wpn] ))
    {
        new_cursed = true;
    }

    // Final checks and clue-giving...
    if (!barehand_butcher && you.equip[EQ_WEAPON] == -1)
    {
        if (you.equip[ EQ_GLOVES ] == -1)
#ifdef JP 
            mpr("なんと。素手で？");
#else
            mpr("What, with your bare hands?");
#endif
        else
#ifdef JP 
            mpr("手が防具に包まれているため、爪を使えない！");
#else
            mpr("You can't use your claws with your gloves on!");
#endif

        // Switching back to avoid possible bug where player can use
        // this to switch weapons in zero time.
        if (wpn_switch)
            weapon_switch( old_weapon );

        return (false);
    }
    else if (!can_butcher)
    {
#ifdef JP 
        mpr("肉を捌くには鋭い武器を使う必要があります。");
#else
        mpr("Maybe you should try using a sharper implement.");
#endif

        // Switching back to avoid possible bug where player can use
        // this to switch weapons in zero time.
        if (wpn_switch && !new_cursed)
            weapon_switch( old_weapon );

        return (false);
    }

    // No turning back at this point, we better be qualified.
    ASSERT( can_butcher );

    int last_item = NON_ITEM;

    int objl = igrd[you.x_pos][you.y_pos];
    int hrg = 0;
    int counter = 0;

    while (objl != NON_ITEM)
    {
        counter++;

        last_item = objl;

        hrg = mitm[objl].link;
        objl = hrg;
        items_here++;

        if (counter > 1000)
        {
            error_message_to_player();

            if (wpn_switch && !new_cursed)
                weapon_switch( old_weapon );

            return (false);
        }
    }

    if (items_here == 1
            && (mitm[igrd[you.x_pos][you.y_pos]].base_type == OBJ_CORPSES &&
                mitm[igrd[you.x_pos][you.y_pos]].sub_type == CORPSE_BODY))
    {
#ifdef JP 
        strcpy(info, "");
#else
        strcpy(info, "Butcher ");
#endif
        it_name(igrd[you.x_pos][you.y_pos], DESC_NOCAP_A, str_pass);
        strcat(info, str_pass);
#ifdef JP 
        strcat(info, "を捌きますか？");
#else
        strcat(info, "\?");
#endif
        mpr(info, MSGCH_PROMPT);

        unsigned char keyin = getch();

        if (keyin == 0)
        {
            getch();
            keyin = 0;
        }

        if (keyin != 'y' && keyin != 'Y')
        {
            if (wpn_switch && !new_cursed)
                weapon_switch( old_weapon );

            return (false);
        }

        int item_got = igrd[you.x_pos][you.y_pos];

        last_item = NON_ITEM;

        if (barehand_butcher)
#ifdef JP 
            mpr("あなたは死体をバラバラに引き裂き始めた。");
#else
            mpr("You start tearing the corpse apart.");
#endif
        else
#ifdef JP 
            mpr("あなたは切り刻み始めた。");
#else
            mpr("You start hacking away.");
#endif

        if (you.duration[DUR_PRAYER]
            && (you.religion == GOD_OKAWARU
                || you.religion == GOD_MAKHLEB || you.religion == GOD_TROG))
        {
            offer_corpse(item_got);
            destroy_item(item_got);
            // XXX: need an extra turn here for weapon swapping?
        }
        else
        {
            int work_req = 3 - mitm[item_got].plus2;
            if (work_req < 0)
                work_req = 0;

            start_delay( DELAY_BUTCHER, work_req, item_got );
        }

        // cue up switching weapon back
        if (wpn_switch && !new_cursed)
            start_delay( DELAY_WEAPON_SWAP, 1, old_weapon );

        you.turn_is_over = 1;

        return (true);

    }                           // end "if items_here == 1"
    else if (items_here > 1)
    {
        last_item = NON_ITEM;
        o = igrd[you.x_pos][you.y_pos];

        for (k = 0; k < items_here; k++)
        {
            if (mitm[o].base_type != OBJ_CORPSES
                    || mitm[o].sub_type != CORPSE_BODY)
            {
                goto out_of_eating;
            }

#ifdef JP 
            strcpy(info, "");
#else
            strcpy(info, "Butcher ");
#endif
            it_name(o, DESC_NOCAP_A, str_pass);
            strcat(info, str_pass);
#ifdef JP 
            strcat(info, "を捌きますか？");
#else
            strcat(info, "\?");
#endif
            mpr(info, MSGCH_PROMPT);

            keyin = getch();
            if (keyin == 0)
            {
                getch();
                keyin = 0;
            }

            if (keyin == 'q')
            {
                if (wpn_switch && !new_cursed)
                    weapon_switch( old_weapon );

                return (false);
            }

            if (keyin == 'y')
            {
                item_got = o;

                if (barehand_butcher)
#ifdef JP 
                    mpr("あなたは死体をバラバラに引き裂き始めた。");
#else
                    mpr("You start tearing the corpse apart.");
#endif
                else
#ifdef JP 
                    mpr("あなたは切り刻み始めた。");
#else
                    mpr("You start hacking away.");
#endif

                if (you.duration[DUR_PRAYER]
                    && (you.religion == GOD_OKAWARU
                        || you.religion == GOD_MAKHLEB
                        || you.religion == GOD_TROG))
                {
                    offer_corpse(item_got);
                    destroy_item(item_got);
                    // XXX: need an extra turn here for weapon swapping?
                }
                else
                {
                    int work_req = 3 - mitm[item_got].plus2;
                    if (work_req < 0)
                        work_req = 0;

                    start_delay( DELAY_BUTCHER, work_req, item_got );
                }

                if (wpn_switch && !new_cursed)
                {
                    // weapon_switch( old_weapon );
                    // need to count the swap delay in this case
                    start_delay( DELAY_WEAPON_SWAP, 1, old_weapon );
                }

                you.turn_is_over = 1;
                return (true);
            }

          out_of_eating:

            if (is_valid_item( mitm[o] ))
                last_item = o;

            hrg = mitm[o].link;
            o = hrg;

            if (o == NON_ITEM)
                break;

            if (items_here == 0)
                break;
        }                       // end "for k" loop
    }

#ifdef JP 
    mpr("ここには解体するような物はない。");
#else
    mpr("There isn't anything to dissect here.");
#endif

    if (wpn_switch && !new_cursed)
        weapon_switch( old_weapon );

    return (false);
}                               // end butchery()

void eat_food(void)
{
    int which_inventory_slot;

    if (you.is_undead == US_UNDEAD)
    {
#ifdef JP 
        mpr("あなたは食べることができない。");
#else
        mpr("You can't eat.");
#endif
        return;
    }

    if (you.hunger >= 11000)
    {
#ifdef JP 
        mpr("あなたは満腹で食べることができない。");
#else
        mpr("You're too full to eat anything.");
#endif
        return;
    }

    if (igrd[you.x_pos][you.y_pos] != NON_ITEM)
    {
        if (eat_from_floor())
        {
            burden_change();    // ghouls regain strength from rotten food
            return;
        }
    }

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }

#ifdef JP 
    which_inventory_slot = prompt_invent_item( "どのアイテムを食べますか？", OBJ_FOOD );
#else
    which_inventory_slot = prompt_invent_item( "Eat which item?", OBJ_FOOD );
#endif
    if (which_inventory_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    // this conditional can later be merged into food::can_ingest() when
    // expanded to handle more than just OBJ_FOOD 16mar200 {dlb}
    if (you.inv[which_inventory_slot].base_type != OBJ_FOOD)
    {
#ifdef JP 
        mpr("あなたはそれを食べられない!");
#else
        mpr("You can't eat that!");
#endif
        return;
    }

    if (!can_ingest( you.inv[which_inventory_slot].base_type,
                        you.inv[which_inventory_slot].sub_type, false ))
    {
        return;
    }

    eat_from_inventory(which_inventory_slot);

    burden_change();
    you.turn_is_over = 1;
}                               // end eat_food()

/*
 **************************************************
 *                                                *
 *              END PUBLIC FUNCTIONS              *
 *                                                *
 **************************************************
*/

static bool food_change(bool suppress_message)
{
    char newstate = HS_ENGORGED;
    bool state_changed = false;

    // this case shouldn't actually happen:
    if (you.is_undead == US_UNDEAD)
        you.hunger = 6000;

    // take care of ghouls - they can never be 'full'
    if (you.species == SP_GHOUL && you.hunger > 6999) 
        you.hunger = 6999;

    // get new hunger state
    if (you.hunger <= 1000)
        newstate = HS_STARVING;
    else if (you.hunger <= 2600)
        newstate = HS_HUNGRY;
    else if (you.hunger < 7000)
        newstate = HS_SATIATED;
    else if (you.hunger < 11000)
        newstate = HS_FULL;

    if (newstate != you.hunger_state)
    {
        state_changed = true;
        you.hunger_state = newstate;
        set_redraw_status( REDRAW_HUNGER );

        // Stop the travel command, if it's in progress and we just got hungry
        if (newstate < HS_SATIATED && you.running < 0) 
            you.running = 0;

        if (suppress_message == false)
        {
            switch (you.hunger_state)
            {
            case HS_STARVING:
#ifdef JP 
                mpr("あなたはひどく飢えている！", MSGCH_FOOD);
#else
                mpr("You are starving!", MSGCH_FOOD);
#endif
                break;
            case HS_HUNGRY:
#ifdef JP 
                mpr("あなたは空腹を感じている。", MSGCH_FOOD);
#else
                mpr("You are feeling hungry.", MSGCH_FOOD);
#endif
                break;
            default:
                break;
            }
        }
    }

    return (state_changed);
}                               // end food_change()


// food_increment is positive for eating,  negative for hungering
static void describe_food_change(int food_increment)
{
    int magnitude = (food_increment > 0)?food_increment:(-food_increment);

    if (magnitude == 0)
        return;

#ifdef JP 
    strcpy(info, (magnitude <= 100) ? "あなたはやや" :
                 (magnitude <= 350) ? "あなたは幾分" :
                 (magnitude <= 800) ? "あなたはかなり"
                                    : "あなたはとても");
#else
    strcpy(info, (magnitude <= 100) ? "You feel slightly " :
                 (magnitude <= 350) ? "You feel somewhat " :
                 (magnitude <= 800) ? "You feel a quite a bit "
                                    : "You feel a lot ");
#endif

#ifdef JP 
#else
    if ((you.hunger_state > HS_SATIATED) ^ (food_increment < 0))
        strcat(info, "more ");
    else
        strcat(info, "less ");
#endif

#ifdef JP 
    strcat(info, (you.hunger_state > HS_SATIATED) ? "満腹感が"
                                                  : "空腹感が");
    if ((you.hunger_state > HS_SATIATED) ^ (food_increment < 0))
        strcat(info, "増した。");    
    else
        strcat(info, "薄れた。");
#else
    strcat(info, (you.hunger_state > HS_SATIATED) ? "full."
                                                  : "hungry.");
#endif
    mpr(info);
}                               // end describe_food_change()

static void eat_from_inventory(int which_inventory_slot)
{
    if (you.inv[which_inventory_slot].sub_type == FOOD_CHUNK)
    {
        // this is a bit easier to read... most compilers should
        // handle this the same -- bwr
        const int mons_type = you.inv[ which_inventory_slot ].plus;
        const int chunk_type = mons_corpse_thingy( mons_type );
        const bool rotten = (you.inv[which_inventory_slot].special < 100);

        eat_chunk( determine_chunk_effect( chunk_type, rotten ) );
    }
    else
    {
        eating( you.inv[which_inventory_slot].base_type,
                you.inv[which_inventory_slot].sub_type );
    }

    dec_inv_item_quantity( which_inventory_slot, 1 );
}                               // end eat_from_inventory()


static bool eat_from_floor(void)
{
    char str_pass[ ITEMNAME_SIZE ];

    if (player_is_levitating() && !wearing_amulet(AMU_CONTROLLED_FLIGHT))
        return (false);

    for (int o = igrd[you.x_pos][you.y_pos]; o != NON_ITEM; o = mitm[o].link)
    {
        if (mitm[o].base_type != OBJ_FOOD)
            continue;

        it_name( o, DESC_NOCAP_A, str_pass );
#ifdef JP 
        snprintf( info, INFO_SIZE, "%s%s食べますか？", str_pass, 
                 (mitm[o].quantity > 1) ? "から1個" : "を" );
#else
        snprintf( info, INFO_SIZE, "Eat %s%s?", (mitm[o].quantity > 1) ? "one of " : "", 
                 str_pass );
#endif
        mpr( info, MSGCH_PROMPT );

        unsigned char keyin = tolower( getch() );

        if (keyin == 0)
        {
            getch();
            keyin = 0;
        }

        if (keyin == 'q')
            return (false);

        if (keyin == 'y')
        {
            if (!can_ingest( mitm[o].base_type, mitm[o].sub_type, false ))
                return (false);

            if (mitm[o].sub_type == FOOD_CHUNK)
            {
                const int chunk_type = mons_corpse_thingy( mitm[o].plus );
                const bool rotten = (mitm[o].special < 100);

                eat_chunk( determine_chunk_effect( chunk_type, rotten ) );
            }
            else
            {
                eating( mitm[o].base_type, mitm[o].sub_type );
            }

            you.turn_is_over = 1;

            dec_mitm_item_quantity( o, 1 );

            return (true);
        }
    }

    return (false);
}


// never called directly - chunk_effect values must pass
// through food::determine_chunk_effect() first {dlb}:
static void eat_chunk( int chunk_effect )
{

    bool likes_chunks = (you.species == SP_KOBOLD || you.species == SP_OGRE
                         || you.species == SP_TROLL
                         || you.mutation[MUT_CARNIVOROUS] > 0);

    if (you.species == SP_GHOUL)
    {
        ghoul_eat_flesh( chunk_effect );
        start_delay( DELAY_EAT, 2 );
        lessen_hunger( 1000, true );
    }
    else
    {
        switch (chunk_effect)
        {
        case CE_MUTAGEN_RANDOM:
#ifdef JP 
            mpr("この肉はとても奇妙な味がする。");
#else
            mpr("This meat tastes really weird.");
#endif
            mutate(100);
            break;

        case CE_MUTAGEN_BAD:
#ifdef JP 
            mpr("この肉は*とても*奇妙な味がする。");
#else
            mpr("This meat tastes *really* weird.");
#endif
            give_bad_mutation();
            break;

        case CE_HCL:
            rot_player( 10 + random2(10) );
            disease_player( 50 + random2(100) );
            break;

        case CE_POISONOUS:
#ifdef JP 
            mpr("うへ！  この肉は有毒だ！");
#else
            mpr("Yeeuch - this meat is poisonous!");
#endif
            poison_player( 3 + random2(4) );
            break;

        case CE_ROTTEN:
        case CE_CONTAMINATED:
#ifdef JP 
            mpr("この肉はどうも悪くなっているようだ。");
#else
            mpr("There is something wrong with this meat.");
#endif
            disease_player( 50 + random2(100) );
            break;

        // note that this is the only case that takes time and forces redraw
        case CE_CLEAN:
#ifdef JP 
            strcpy(info, "この肉は");
#else
            strcpy(info, "This raw flesh ");
#endif

#ifdef JP 
            strcat(info, (likes_chunks) ? "おいしい。"
                                        : "あまり食欲をそそらない。");
#else
            strcat(info, (likes_chunks) ? "tastes good."
                                        : "is not very appetising.");
#endif
            mpr(info);

            start_delay( DELAY_EAT, 2 );
            lessen_hunger( 1000, true );
            break;
        }
    }

    return;
}                               // end eat_chunk()

static void ghoul_eat_flesh( int chunk_effect )
{
    bool healed = false;

    if (chunk_effect != CE_ROTTEN && chunk_effect != CE_CONTAMINATED)
    {
#ifdef JP 
        mpr("この肉はおいしい。");
#else
        mpr("This raw flesh tastes good.");
#endif

        if (!one_chance_in(5))
            healed = true;

        if (player_rotted() && !one_chance_in(3))
        {
#ifdef JP 
            mpr("あなたは元気になったように感じた。");
#else
            mpr("You feel more resilient.");
#endif
            unrot_hp(1);
        }
    }
    else
    {
        if (chunk_effect == CE_ROTTEN)
#ifdef JP 
            mpr( "この腐った肉はとてもおいしい！" );
#else
            mpr( "This rotting flesh tastes delicious!" );
#endif
        else // CE_CONTAMINATED
#ifdef JP 
            mpr( "この肉はとてもおいしい！" );
#else
            mpr( "This flesh tastes delicious!" );
#endif

        healed = true;

        if (player_rotted() && !one_chance_in(4))
        {
#ifdef JP 
            mpr("あなたは元気になったように感じた。");
#else
            mpr("You feel more resilient.");
#endif
            unrot_hp(1);
        }
    }

    if (you.strength < you.max_strength && one_chance_in(5))
    {
#ifdef JP 
        mpr("あなたは腕力が回復した。");
#else
        mpr("You feel your strength returning.");
#endif
        you.strength++;
        you.redraw_strength = 1;
    }

    if (healed && you.hp < you.hp_max)
        inc_hp(1 + random2(5) + random2(1 + you.experience_level), false);

    calc_hp();

    return;
}                               // end ghoul_eat_flesh()

static void eating(unsigned char item_class, int item_type)
{
    int temp_rand;              // probability determination {dlb}
    int food_value = 0;
    int how_herbivorous = you.mutation[MUT_HERBIVOROUS];
    int how_carnivorous = you.mutation[MUT_CARNIVOROUS];
    int carnivore_modifier = 0;
    int herbivore_modifier = 0;

    switch (item_class)
    {
    case OBJ_FOOD:
        // apply base sustenance {dlb}:
        switch (item_type)
        {
        case FOOD_MEAT_RATION:
        case FOOD_ROYAL_JELLY:
            food_value = 5000;
            break;
        case FOOD_BREAD_RATION:
            food_value = 4400;
            break;
        case FOOD_HONEYCOMB:
            food_value = 2000;
            break;
        case FOOD_SNOZZCUMBER:  // maybe a nasty side-effect from RD's book?
        case FOOD_PIZZA:
        case FOOD_BEEF_JERKY:
            food_value = 1500;
            break;
        case FOOD_CHEESE:
        case FOOD_SAUSAGE:
            food_value = 1200;
            break;
        case FOOD_ORANGE:
        case FOOD_BANANA:
        case FOOD_LEMON:
            food_value = 1000;
            break;
        case FOOD_PEAR:
        case FOOD_APPLE:
        case FOOD_APRICOT:
            food_value = 700;
            break;
        case FOOD_CHOKO:
        case FOOD_RAMBUTAN:
        case FOOD_LYCHEE:
            food_value = 600;
            break;
        case FOOD_STRAWBERRY:
            food_value = 200;
            break;
        case FOOD_GRAPE:
            food_value = 100;
            break;
        case FOOD_SULTANA:
            food_value = 70;     // will not save you from starvation
            break;
        default:
            break;
        }                       // end base sustenance listing {dlb}

        // next, sustenance modifier for carnivores/herbivores {dlb}:
        // for some reason, sausages do not penalize herbivores {dlb}:
        switch (item_type)
        {
        case FOOD_MEAT_RATION:
            carnivore_modifier = 500;
            herbivore_modifier = -1500;
            break;
        case FOOD_BEEF_JERKY:
            carnivore_modifier = 200;
            herbivore_modifier = -200;
            break;
        case FOOD_BREAD_RATION:
            carnivore_modifier = -1000;
            herbivore_modifier = 500;
            break;
        case FOOD_BANANA:
        case FOOD_ORANGE:
        case FOOD_LEMON:
            carnivore_modifier = -300;
            herbivore_modifier = 300;
            break;
        case FOOD_PEAR:
        case FOOD_APPLE:
        case FOOD_APRICOT:
        case FOOD_CHOKO:
        case FOOD_SNOZZCUMBER:
        case FOOD_RAMBUTAN:
        case FOOD_LYCHEE:
            carnivore_modifier = -200;
            herbivore_modifier = 200;
            break;
        case FOOD_STRAWBERRY:
            carnivore_modifier = -50;
            herbivore_modifier = 50;
            break;
        case FOOD_GRAPE:
        case FOOD_SULTANA:
            carnivore_modifier = -20;
            herbivore_modifier = 20;
            break;
        default:
            carnivore_modifier = 0;
            herbivore_modifier = 0;
            break;
        }                    // end carnivore/herbivore modifier listing {dlb}

        // next, let's take care of messaging {dlb}:
        if (how_carnivorous > 0 && carnivore_modifier < 0)
#ifdef JP 
            mpr("オエ……。あなたには肉が必要だ！");
#else
            mpr("Blech - you need meat!");
#endif
        else if (how_herbivorous > 0 && herbivore_modifier < 0)
#ifdef JP 
            mpr("オエ……。あなたには野菜が必要だ！");
#else
            mpr("Blech - you need greens!");
#endif

        if (how_herbivorous < 1)
        {
            switch (item_type)
            {
            case FOOD_MEAT_RATION:
#ifdef JP 
                mpr("肉の保存食は実に満足の行く食事だ！");
#else
                mpr("That meat ration really hit the spot!");
#endif
                break;
            case FOOD_BEEF_JERKY:
#ifdef JP 
                strcpy(info, "このビーフジャーキーは");
                strcat(info, (one_chance_in(4)) ? "噛み応えがある"
                                                : "おいしい");
                strcat(info, "！");
#else
                strcpy(info, "That beef jerky was ");
                strcat(info, (one_chance_in(4)) ? "jerk-a-riffic"
                                                : "delicious");
                strcat(info, "!");
#endif
                mpr(info);
                break;
            default:
                break;
            }
        }

        if (how_carnivorous < 1)
        {
            switch (item_type)
            {
            case FOOD_BREAD_RATION:
#ifdef JP 
                mpr("パンの保存食は実に満足の行く食事だ！");
#else
                mpr("That bread ration really hit the spot!");
#endif
                break;
            case FOOD_PEAR:
            case FOOD_APPLE:
            case FOOD_APRICOT:
#ifdef JP 
                strcpy(info, "むむむ……おいしい");
                strcat(info, (item_type == FOOD_APPLE)   ? "リンゴだ。" :
                             (item_type == FOOD_PEAR)    ? "洋ナシだ。" :
                             (item_type == FOOD_APRICOT) ? "アンズだ。"
                                                         : "果物だ。");
#else
                strcpy(info, "Mmmm... Yummy ");
                strcat(info, (item_type == FOOD_APPLE)   ? "apple." :
                             (item_type == FOOD_PEAR)    ? "pear." :
                             (item_type == FOOD_APRICOT) ? "apricot."
                                                         : "fruit.");
#endif
                mpr(info);
                break;
            case FOOD_CHOKO:
#ifdef JP 
                mpr("このハヤトウリはまるで味がしない。");
#else
                mpr("That choko was very bland.");
#endif
                break;
            case FOOD_SNOZZCUMBER:
#ifdef JP 
                mpr("クサキウリはまさしく腐った味がする！");
#else
                mpr("That snozzcumber tasted truly putrid!");
#endif
                break;
            case FOOD_ORANGE:
#ifdef JP 
                strcpy(info, "このオレンジはとてもおいしい！");
#else
                strcpy(info, "That orange was delicious!");
#endif
                if (one_chance_in(8))
#ifdef JP 
                    strcat(info, "皮までもおいしかった！");
#else
                    strcat(info, " Even the peel tasted good!");
#endif
                mpr(info);
                break;
            case FOOD_BANANA:
#ifdef JP 
                strcpy(info, "このバナナはとてもおいしい！");
#else
                strcpy(info, "That banana was delicious!");
#endif
                if (one_chance_in(8))
#ifdef JP 
                    strcat(info, "皮までもおいしかった！");
#else
                    strcat(info, " Even the peel tasted good!");
#endif
                mpr(info);
                break;
            case FOOD_STRAWBERRY:
#ifdef JP 
                mpr("このイチゴはとてもおいしい！");
#else
                mpr("That strawberry was delicious!");
#endif
                break;
            case FOOD_RAMBUTAN:
#ifdef JP 
                mpr("このランブータンはとてもおいしい！");
#else
                mpr("That rambutan was delicious!");
#endif
                break;
            case FOOD_LEMON:
#ifdef JP 
                mpr("このレモンは酸っぱい……だがとてもおいしい！");
#else
                mpr("That lemon was rather sour... But delicious nonetheless!");
#endif
                break;
            case FOOD_GRAPE:
#ifdef JP 
                mpr("このブドウはとてもおいしい！");
#else
                mpr("That grape was delicious!");
#endif
                break;
            case FOOD_SULTANA:
#ifdef JP 
                mpr("この種なし干しブドウはとてもおいしい！ (だが小さすぎる)");
#else
                mpr("That sultana was delicious! (but very small)");
#endif
                break;
            case FOOD_LYCHEE:
#ifdef JP 
                mpr("このライチはとてもおいしい！");
#else
                mpr("That lychee was delicious!");
#endif
                break;
            default:
                break;
            }
        }

        switch (item_type)
        {
        case FOOD_HONEYCOMB:
#ifdef JP 
            mpr("この蜂の巣はとてもおいしい。");
#else
            mpr("That honeycomb was delicious.");
#endif
            break;
        case FOOD_ROYAL_JELLY:
#ifdef JP 
            mpr("このロイヤルゼリーはとてもおいしい！");
#else
            mpr("That royal jelly was delicious!");
#endif
            restore_stat(STAT_ALL, false);
            break;
        case FOOD_PIZZA:
#ifdef JP 
            strcpy(info, "むむむ……");
#else
            strcpy(info, "Mmm... ");
#endif

            if (SysEnv.crawl_pizza && !one_chance_in(3))
                strcat(info, SysEnv.crawl_pizza);
            else
            {
                temp_rand = random2(9);

#ifdef JP 
                strcat(info, (temp_rand == 0) ? "ハム＆パイナップルピザだ。" :
                             (temp_rand == 1) ? "とても厚いピザだ。" :
                             (temp_rand == 2) ? "ベジタブルピザだ。" :
                             (temp_rand == 3) ? "ペパローニピザだ。" :
                             (temp_rand == 4) ? "うへ。アンチョビーピザだ！" :
                             (temp_rand == 5) ? "チーズたっぷりのピザだ。" :
                             (temp_rand == 6) ? "最高にうまいピザだ。" :
                             (temp_rand == 7) ? "超最高にうまいピザだ！"
                                              : "チキンピザだ。");
#else
                strcat(info, (temp_rand == 0) ? "Ham and pineapple." :
                             (temp_rand == 1) ? "Extra thick crust." :
                             (temp_rand == 2) ? "Vegetable." :
                             (temp_rand == 3) ? "Pepperoni." :
                             (temp_rand == 4) ? "Yeuchh - Anchovies!" :
                             (temp_rand == 5) ? "Cheesy." :
                             (temp_rand == 6) ? "Supreme." :
                             (temp_rand == 7) ? "Super Supreme!"
                                              : "Chicken.");
#endif
            }
            mpr(info);
            break;
        case FOOD_CHEESE:
#ifdef JP 
            strcpy(info, "むむむ……");
#else
            strcpy(info, "Mmm... ");
#endif
            temp_rand = random2(9);

#ifdef JP 
            strcat(info, (temp_rand == 0) ? "チェーダーチーズだ" :
                         (temp_rand == 1) ? "エダムチーズだ" :
                         (temp_rand == 2) ? "ウェンズレデールチーズだ" :
                         (temp_rand == 3) ? "カマンベールだ" :
                         (temp_rand == 4) ? "ヤギ乳のチーズだ" :
                         (temp_rand == 5) ? "フルーツチーズだ" :
                         (temp_rand == 6) ? "モツァレラだ" :
                         (temp_rand == 7) ? "ヒツジ乳のチーズだ"
                                          : "ヤク乳のチーズだ");

            strcat(info, "。");
#else
            strcat(info, (temp_rand == 0) ? "Cheddar" :
                         (temp_rand == 1) ? "Edam" :
                         (temp_rand == 2) ? "Wensleydale" :
                         (temp_rand == 3) ? "Camembert" :
                         (temp_rand == 4) ? "Goat cheese" :
                         (temp_rand == 5) ? "Fruit cheese" :
                         (temp_rand == 6) ? "Mozzarella" :
                         (temp_rand == 7) ? "Sheep cheese"
                                          : "Yak cheese");

            strcat(info, ".");
#endif
            mpr(info);
            break;
        case FOOD_SAUSAGE:
#ifdef JP 
            mpr("このソーセージはとてもおいしい！");
#else
            mpr("That sausage was delicious!");
#endif
            break;
        default:
            break;
        }

        // finally, modify player's hunger level {dlb}:
        if (carnivore_modifier && how_carnivorous > 0)
            food_value += (carnivore_modifier * how_carnivorous);

        if (herbivore_modifier && how_herbivorous > 0)
            food_value += (herbivore_modifier * how_herbivorous);

        if (food_value > 0)
        {
            if (item_type == FOOD_MEAT_RATION || item_type == FOOD_BREAD_RATION)
                start_delay( DELAY_EAT, 3 );
            else
                start_delay( DELAY_EAT, 1 );

            lessen_hunger( food_value, true );
        }
        break;

    default:
        break;
    }

    return;
}                               // end eating()

static bool can_ingest(int what_isit, int kindof_thing, bool suppress_msg)
{
    bool survey_says = false;

    bool ur_carnivorous = (you.species == SP_GHOUL
                           || you.species == SP_KOBOLD
                           || you.mutation[MUT_CARNIVOROUS] == 3);

    bool ur_herbivorous = (you.mutation[MUT_HERBIVOROUS] > 1);

    // ur_chunkslover not defined in terms of ur_carnivorous because
    // a player could be one and not the other IMHO - 13mar2000 {dlb}
#ifdef V_FIX
    bool ur_chunkslover = (you.hunger_state <= HS_HUNGRY
                           || wearing_amulet(AMU_THE_GOURMAND)
                           || you.species == SP_KOBOLD
                           || you.species == SP_OGRE
                           || you.species == SP_OGRE_MAGE
                           || you.species == SP_TROLL
                           || you.species == SP_GHOUL
                           || you.mutation[MUT_CARNIVOROUS]);
#else
    bool ur_chunkslover = (you.hunger_state <= HS_HUNGRY
                           || wearing_amulet(AMU_THE_GOURMAND)
                           || you.species == SP_KOBOLD
                           || you.species == SP_OGRE
                           || you.species == SP_TROLL
                           || you.species == SP_GHOUL
                           || you.mutation[MUT_CARNIVOROUS]);
#endif

    switch (what_isit)
    {
    case OBJ_FOOD:
        switch (kindof_thing)
        {
        case FOOD_BREAD_RATION:
        case FOOD_PEAR:
        case FOOD_APPLE:
        case FOOD_CHOKO:
        case FOOD_SNOZZCUMBER:
        case FOOD_PIZZA:
        case FOOD_APRICOT:
        case FOOD_ORANGE:
        case FOOD_BANANA:
        case FOOD_STRAWBERRY:
        case FOOD_RAMBUTAN:
        case FOOD_LEMON:
        case FOOD_GRAPE:
        case FOOD_SULTANA:
        case FOOD_LYCHEE:
            if (ur_carnivorous)
            {
                survey_says = false;
                if (!suppress_msg)
#ifdef JP 
                    mpr("残念ながら、あなたは肉食性だ。");
#else
                    mpr("Sorry, you're a carnivore.");
#endif
            }
            else
                survey_says = true;
            break;

        case FOOD_CHUNK:
            if (ur_herbivorous)
            {
                survey_says = false;
                if (!suppress_msg)
#ifdef JP 
                    mpr("あなたは生肉を食べることができない！");
#else
                    mpr("You can't eat raw meat!");
#endif
            }
            else if (!ur_chunkslover)
            {
                survey_says = false;
                if (!suppress_msg)
#ifdef JP 
                    mpr("あなたはそれを食べられるほど腹が空いていない！");
#else
                    mpr("You aren't quite hungry enough to eat that!");
#endif
            }
            else
                survey_says = true;
            break;

        default:
            return (true);
        }
        break;

    // other object types are set to return false for now until
    // someone wants to recode the eating code to permit consumption
    // of things other than just food -- corpses first, then more
    // exotic stuff later would be good to explore - 13mar2000 {dlb}
    case OBJ_CORPSES:
    default:
        return (false);
    }

    return (survey_says);
}                               // end can_ingest()

// see if you can follow along here -- except for the Amulet of the Gourmand
// addition (long missing and requested), what follows is an expansion of how
// chunks were handled in the codebase up to this date -- I have never really
// understood why liches are hungry and not true undead beings ... {dlb}:
static int determine_chunk_effect(int which_chunk_type, bool rotten_chunk)
{
    int poison_resistance_level = player_res_poison();
    int this_chunk_effect = which_chunk_type;

    // determine the initial effect of eating a particular chunk {dlb}:
    switch (this_chunk_effect)
    {
    case CE_HCL:
    case CE_MUTAGEN_RANDOM:
        if (you.species == SP_GHOUL
                || you.attribute[ATTR_TRANSFORMATION] == TRAN_LICH)
        {
            this_chunk_effect = CE_CLEAN;
        }
        break;

    case CE_POISONOUS:
        if (you.species == SP_GHOUL
                || you.attribute[ATTR_TRANSFORMATION] == TRAN_LICH
                || poison_resistance_level > 0)
        {
            this_chunk_effect = CE_CLEAN;
        }
        break;

    case CE_CONTAMINATED:
        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_LICH)
            this_chunk_effect = CE_CLEAN;
        else
        {
            switch (you.species)
            {
            case SP_GHOUL:
                // Doing this here causes a odd message later. -- bwr
                // this_chunk_effect = CE_ROTTEN;
                break;

            case SP_KOBOLD:
            case SP_TROLL:
                if (!one_chance_in(45))
                    this_chunk_effect = CE_CLEAN;
                break;

            case SP_HILL_ORC:
            case SP_OGRE:
                if (!one_chance_in(15))
                    this_chunk_effect = CE_CLEAN;
                break;

            default:
                if (!one_chance_in(3))
                    this_chunk_effect = CE_CLEAN;
                break;
            }
        }
        break;

    default:
        break;
    }

    // determine effects of rotting on base chunk effect {dlb}:
    if (rotten_chunk)
    {
        switch (this_chunk_effect)
        {
        case CE_CLEAN:
        case CE_CONTAMINATED:
            this_chunk_effect = CE_ROTTEN;
            break;
        case CE_MUTAGEN_RANDOM:
            this_chunk_effect = CE_MUTAGEN_BAD;
            break;
        default:
            break;
        }
    }

    // one last chance for some species to safely eat rotten food {dlb}:
    if (this_chunk_effect == CE_ROTTEN)
    {
        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_LICH)
            this_chunk_effect = CE_CLEAN;
        else
        {
            switch (you.species)
            {
            case SP_KOBOLD:
            case SP_TROLL:
                if (!one_chance_in(15))
                    this_chunk_effect = CE_CLEAN;
                break;
            case SP_HILL_ORC:
            case SP_OGRE:
                if (!one_chance_in(5))
                    this_chunk_effect = CE_CLEAN;
                break;
            default:
                break;
            }
        }
    }

    // the amulet of the gourmad will permit consumption of rotting meat as
    // though it were "clean" meat - ghouls can expect the reverse, as they
    // prize rotten meat ... yum! {dlb}:
    if (wearing_amulet(AMU_THE_GOURMAND))
    {
#ifdef V_FIX
        if (you.species == SP_GHOUL)
        {
            if ( (this_chunk_effect == CE_CLEAN)||(this_chunk_effect == CE_CONTAMINATED) )
                this_chunk_effect = CE_ROTTEN;
        }
        else
        {
            if ( (this_chunk_effect == CE_ROTTEN)||(this_chunk_effect == CE_CONTAMINATED) )
                this_chunk_effect = CE_CLEAN;
        }
#else
        if (you.species == SP_GHOUL)
        {
            if (this_chunk_effect == CE_CLEAN)
                this_chunk_effect = CE_ROTTEN;
        }
        else
        {
            if (this_chunk_effect == CE_ROTTEN)
                this_chunk_effect = CE_CLEAN;
        }
#endif
    }

    return (this_chunk_effect);
}                               // end determine_chunk_effect()
