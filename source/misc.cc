/*
 *  File:       misc.cc
 *  Summary:    Misc functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *   <3>   11/14/99      cdl    evade with random40(ev) vice random2(ev)
 *   <2>    5/20/99      BWR    Multi-user support, new berserk code.
 *   <1>    -/--/--      LRH    Created
 */


#include "AppHdr.h"
#include "misc.h"

#include <string.h>
#if !(defined(__IBMCPP__) || defined(__BCPLUSPLUS__))
#include <unistd.h>
#endif

#ifdef __MINGW32__
#include <io.h>
#endif

#include <stdlib.h>
#include <stdio.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "cloud.h"
#include "delay.h"
#include "fight.h"
#include "files.h"
#include "food.h"
#include "it_use2.h"
#include "items.h"
#include "itemname.h"
#include "lev-pand.h"
#include "macro.h"
#include "monplace.h"
#include "mon-util.h"
#include "monstuff.h"
#include "ouch.h"
#include "player.h"
#include "shopping.h"
#include "skills.h"
#include "skills2.h"
#include "spells3.h"
#include "spl-cast.h"
#include "stuff.h"
#include "transfor.h"
#include "travel.h"
#include "view.h"

#ifdef USE_TILE
#include "tiles.h"
#endif

/* these are defined in view.cc: */
extern unsigned char (*mapch) (unsigned char);
extern unsigned char (*mapch2) (unsigned char);

bool scramble(void);
bool trap_item(char base_type, char sub_type, char beam_x, char beam_y);
static void dart_trap(bool trap_known, int trapped, struct bolt &pbolt, bool poison);

// void place_chunks(int mcls, unsigned char rot_status, unsigned char chx,
//                   unsigned char chy, unsigned char ch_col)
void turn_corpse_into_chunks( item_def &item )
{
    const int mons_class = item.plus;
    const int max_chunks = mons_weight( mons_class ) / 150;

    ASSERT( item.base_type == OBJ_CORPSES );

    item.base_type = OBJ_FOOD;
    if ( item.sub_type == CORPSE_SKELETON )
        item.special = 0;
    item.sub_type = FOOD_CHUNK;
    item.quantity = 1 + random2( max_chunks );

    item.quantity = stepdown_value( item.quantity, 4, 4, 12, 12 );

    // seems to me that this should come about only
    // after the corpse has been butchered ... {dlb}
    if (monster_descriptor( mons_class, MDSC_LEAVES_HIDE ) && !one_chance_in(3))
    {
        int o = get_item_slot( 100 + random2(200) );
        if (o == NON_ITEM)
            return;

        mitm[o].quantity = 1;

        // these values are common to all: {dlb}
        mitm[o].base_type = OBJ_ARMOUR;
        mitm[o].plus = 0;
        mitm[o].plus2 = 0;
        mitm[o].special = 0;
        mitm[o].flags = 0;
        mitm[o].colour = mons_colour( mons_class );

        // these values cannot be set by a reasonable formula: {dlb}
        switch (mons_class)
        {
        case MONS_DRAGON:
            mitm[o].sub_type = ARM_DRAGON_HIDE;
            break;
        case MONS_TROLL:
            mitm[o].sub_type = ARM_TROLL_HIDE;
            break;
        case MONS_ICE_DRAGON:
            mitm[o].sub_type = ARM_ICE_DRAGON_HIDE;
            break;
        case MONS_STEAM_DRAGON:
            mitm[o].sub_type = ARM_STEAM_DRAGON_HIDE;
            break;
        case MONS_MOTTLED_DRAGON:
            mitm[o].sub_type = ARM_MOTTLED_DRAGON_HIDE;
            break;
        case MONS_STORM_DRAGON:
            mitm[o].sub_type = ARM_STORM_DRAGON_HIDE;
            break;
        case MONS_GOLDEN_DRAGON:
            mitm[o].sub_type = ARM_GOLD_DRAGON_HIDE;
            break;
        case MONS_SWAMP_DRAGON:
            mitm[o].sub_type = ARM_SWAMP_DRAGON_HIDE;
            break;
        default:
            // future implementation {dlb}
            mitm[o].sub_type = ARM_ANIMAL_SKIN;
            break;
        }

        move_item_to_grid( &o, item.x, item.y );
    }
}                               // end place_chunks()

void search_around(void)
{
    char srx = 0;
    char sry = 0;
    int i;

    // Never if doing something else... this prevents a slight asymetry
    // where using autopickup was giving free searches in comparison to
    // not using autopickup.  -- bwr
    if (you_are_delayed())
        return;

    for (srx = you.x_pos - 1; srx < you.x_pos + 2; srx++)
    {
        for (sry = you.y_pos - 1; sry < you.y_pos + 2; sry++)
        {
            // don't exclude own square; may be levitating
            if (grd[srx][sry] == DNGN_SECRET_DOOR
                && random2(17) <= 1 + you.skills[SK_TRAPS_DOORS])
            {
                grd[srx][sry] = DNGN_CLOSED_DOOR;
#ifdef JP
                mpr("あなたは隠し扉を発見した！");
#else
                mpr("You found a secret door!");
#endif
                exercise(SK_TRAPS_DOORS, ((coinflip())? 2 : 1));
            }

            if (grd[srx][sry] == DNGN_UNDISCOVERED_TRAP
                && random2(17) <= 1 + you.skills[SK_TRAPS_DOORS])
            {
                i = trap_at_xy(srx, sry);

                if (i != -1)
                    grd[srx][sry] = trap_category(env.trap[i].type);

#ifdef JP
                mpr("あなたは罠を発見した！");
#else
                mpr("You found a trap!");
#endif
            }
        }
    }

    return;
}                               // end search_around()

void in_a_cloud(void)
{
    int cl = env.cgrid[you.x_pos][you.y_pos];
    int hurted = 0;
    int resist;

    if (you.duration[DUR_CONDENSATION_SHIELD] > 0)
    {
#ifdef JP
        mpr("あなたの氷の盾は砕け散ってしまった！", MSGCH_DURATION);
#else
        mpr("Your icy shield dissipates!", MSGCH_DURATION);
#endif
        you.duration[DUR_CONDENSATION_SHIELD] = 0;
        you.redraw_armour_class = 1;
    }

    switch (env.cloud[cl].type)
    {
    case CLOUD_FIRE:
    case CLOUD_FIRE_MON:
        if (you.fire_shield)
            return;

#ifdef JP
        mpr("あなたは唸りを上げる炎に巻き込まれた！");
#else
        mpr("You are engulfed in roaring flames!");
#endif

        resist = player_res_fire();

        if (resist <= 0)
        {
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;

            if (resist < 0)
                hurted += ((random2avg(14, 2) + 3) * you.time_taken) / 10;

            hurted -= random2(player_AC());

            if (hurted < 1)
                hurted = 0;
            else
#ifdef JP
                ouch( hurted, cl, KILLED_BY_CLOUD, "炎" );
#else
                ouch( hurted, cl, KILLED_BY_CLOUD, "flame" );
#endif
        }
        else
        {
            canned_msg(MSG_YOU_RESIST);
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;
            hurted /= (1 + resist * resist);
#ifdef JP
            ouch( hurted, cl, KILLED_BY_CLOUD, "炎" );
#else
            ouch( hurted, cl, KILLED_BY_CLOUD, "flame" );
#endif
        }
        scrolls_burn(7, OBJ_SCROLLS);
        break;

    case CLOUD_STINK:
    case CLOUD_STINK_MON:
        // If you don't have to breathe, unaffected
#ifdef JP
        mpr("あなたは有毒な煙に巻かれた！");
#else
        mpr("You are engulfed in noxious fumes!");
#endif
        if (player_res_poison())
            break;

        hurted += (random2(3) * you.time_taken) / 10;
        if (hurted < 1)
            hurted = 0;
        else
            ouch( (hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD,
#ifdef JP
                    "有毒な煙" );
#else
                    "noxious fumes" );
#endif

        if (1 + random2(27) >= you.experience_level)
        {
#ifdef JP
            mpr("あなたは悪臭に息が詰まった！");
#else
            mpr("You choke on the stench!");
#endif
            confuse_player( (coinflip() ? 3 : 2) );
        }
        break;

    case CLOUD_COLD:
    case CLOUD_COLD_MON:
#ifdef JP
        mpr("あなたは凍てつく気体に包み込まれた！！");
#else
        mpr("You are engulfed in freezing vapours!");
#endif

        resist = player_res_cold();

        if (resist <= 0)
        {
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;

            if (resist < 0)
                hurted += ((random2avg(14, 2) + 3) * you.time_taken) / 10;

            hurted -= random2(player_AC());
            if (hurted < 0)
                hurted = 0;

            ouch( (hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD,
#ifdef JP
                  "凍てつく気体" );
#else
                  "freezing vapour" );
#endif
        }
        else
        {
            canned_msg(MSG_YOU_RESIST);
            hurted += ((random2avg(23, 3) + 10) * you.time_taken) / 10;
            hurted /= (1 + resist * resist);
#ifdef JP
            ouch( hurted, cl, KILLED_BY_CLOUD, "凍てつく気体" );
#else
            ouch( hurted, cl, KILLED_BY_CLOUD, "freezing vapour" );
#endif
        }
        scrolls_burn(7, OBJ_POTIONS);
        break;

    case CLOUD_POISON:
    case CLOUD_POISON_MON:
        // If you don't have to breathe, unaffected
#ifdef JP
        mpr("あなたは毒ガスに包み込まれた！");
#else
        mpr("You are engulfed in poison gas!");
#endif
        if (!player_res_poison())
        {
            ouch( (random2(10) * you.time_taken) / 10, cl, KILLED_BY_CLOUD,
#ifdef JP
                  "poison gas" );
#else
                  "poison gas" );
#endif
            poison_player(1);
        }
        break;

    case CLOUD_GREY_SMOKE:
    case CLOUD_BLUE_SMOKE:
    case CLOUD_PURP_SMOKE:
    case CLOUD_BLACK_SMOKE:
    case CLOUD_GREY_SMOKE_MON:
    case CLOUD_BLUE_SMOKE_MON:
    case CLOUD_PURP_SMOKE_MON:
    case CLOUD_BLACK_SMOKE_MON:
#ifdef JP
        mpr("あなたは煙の中に巻かれた！");
#else
        mpr("You are engulfed in a cloud of smoke!");
#endif
        break;

    case CLOUD_STEAM:
    case CLOUD_STEAM_MON:
#ifdef JP
        mpr("あなたは高熱の蒸気に包み込まれた！");
#else
        mpr("You are engulfed in a cloud of scalding steam!");
#endif
        if (you.species == SP_PALE_DRACONIAN && you.experience_level > 5)
        {
#ifdef JP
            mpr("しかしあなたには効果がないようだ。");
#else
            mpr("It doesn't seem to affect you.");
#endif
            return;
        }

        if (player_equip( EQ_BODY_ARMOUR, ARM_STEAM_DRAGON_ARMOUR ))
        {
#ifdef JP
            mpr("しかしあなたには効果がないようだ。");
#else
            mpr("It doesn't seem to affect you.");
#endif
            return;
        }

        hurted += (random2(6) * you.time_taken) / 10;
        if (hurted < 0 || player_res_fire() > 0)
            hurted = 0;

#ifdef JP
        ouch( (hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD, "毒ガス" );
#else
        ouch( (hurted * you.time_taken) / 10, cl, KILLED_BY_CLOUD, "poison gas" );
#endif
        break;

    case CLOUD_MIASMA:
    case CLOUD_MIASMA_MON:
#ifdef JP
        mpr("あなたは暗黒の瘴気に包まれた。");
#else
        mpr("You are engulfed in a dark miasma.");
#endif

        if (player_prot_life() > random2(3))
            return;

        poison_player(1);

        hurted += (random2avg(12, 3) * you.time_taken) / 10;    // 3

        if (hurted < 0)
            hurted = 0;

#ifdef JP
        ouch( hurted, cl, KILLED_BY_CLOUD, "悪疫の瘴気" );
#else
        ouch( hurted, cl, KILLED_BY_CLOUD, "foul pestilence" );
#endif
        potion_effect(POT_SLOWING, 5);

        if (you.hp_max > 4 && coinflip())
            rot_hp(1);

        break;
    }

    return;
}                               // end in_a_cloud()


void merfolk_start_swimming(void)
{
    FixedVector < char, 8 > removed;

    if (you.attribute[ATTR_TRANSFORMATION] != TRAN_NONE)
        untransform();

    for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
    {
        removed[i] = 0;
    }

    if (you.equip[EQ_BOOTS] != -1)
        removed[EQ_BOOTS] = 1;

    // Perhaps a bit to easy for the player, but we allow merfolk
    // to slide out of heavy body armour freely when entering water,
    // rather than handling emcumbered swimming. -- bwr
    if (!player_light_armour())
    {
        // Can't slide out of just the body armour, cloak comes off -- bwr
        if (you.equip[EQ_CLOAK])
            removed[EQ_CLOAK] = 1;

        removed[EQ_BODY_ARMOUR] = 1;
    }

    remove_equipment(removed);

#ifdef USE_TILE
    if (Options.use_tile)
        TilePlayerRefresh();
#endif
}

void up_stairs(void)
{
    unsigned char stair_find = grd[you.x_pos][you.y_pos];
    char old_where = you.where_are_you;
    bool was_a_labyrinth = false;

    if (stair_find == DNGN_ENTER_SHOP)
    {
        shop();
        return;
    }

    // probably still need this check here (teleportation) -- bwr
    if ((stair_find < DNGN_STONE_STAIRS_UP_I
            || stair_find > DNGN_ROCK_STAIRS_UP)
        && (stair_find < DNGN_RETURN_FROM_ORCISH_MINES || stair_find >= 150))
    {
#ifdef JP
        mpr("あなたはここから上の階に行くことはできない。");
#else
        mpr("You can't go up here.");
#endif
        return;
    }

    // Since the overloaded message set turn_is_over, I'm assuming that
    // the overloaded character makes an attempt... so we're doing this
    // check before that one. -- bwr
    if (!player_is_levitating()
        && you.conf
        && (stair_find >= DNGN_STONE_STAIRS_UP_I
            && stair_find <= DNGN_ROCK_STAIRS_UP)
        && random2(100) > you.dex)
    {
#ifdef JP
        mpr("混乱のあまりあなたは躓いて、階段から転がり落ちてしまった。");
#else
        mpr("In your confused state, you trip and fall back down the stairs.");
#endif

        ouch( roll_dice( 3 + you.burden_state, 5 ), 0,
              KILLED_BY_FALLING_DOWN_STAIRS );

        you.turn_is_over = 1;
        return;
    }

    if (you.burden_state == BS_OVERLOADED)
    {
#ifdef JP
        mpr("あなたは荷物が重すぎて階段を上ることができない。");
#else
        mpr("You are carrying too much to climb upwards.");
#endif
        you.turn_is_over = 1;
        return;
    }

    if (you.your_level == 0
#ifdef JP
            && !yesno("本当にダンジョンから去りますか？", false))
#else
            && !yesno("Are you sure you want to leave the Dungeon?", false))
#endif
    {
#ifdef JP
        mpr("了解しました。探索を続けてください！");
#else
        mpr("Alright, then stay!");
#endif
        return;
    }

    unsigned char old_level = you.your_level;

    // Interlevel travel data:
    bool collect_travel_data = you.level_type != LEVEL_LABYRINTH
                && you.level_type != LEVEL_ABYSS
                && you.level_type != LEVEL_PANDEMONIUM;

    level_id  old_level_id    = level_id::get_current_level_id();
    LevelInfo &old_level_info = travel_cache.get_level_info(old_level_id);
    int stair_x = you.x_pos, stair_y = you.y_pos;
    if (collect_travel_data)
        old_level_info.update();

    // Make sure we return to our main dungeon level... labyrinth entrances
    // in the abyss or pandemonium a bit trouble (well the labyrinth does
    // provide a way out of those places, its really not that bad I suppose)
    if (you.level_type == LEVEL_LABYRINTH)
    {
        you.level_type = LEVEL_DUNGEON;
        was_a_labyrinth = true;
    }

    you.your_level--;

    int i = 0;

    if (you.your_level < 0)
    {
#ifdef JP
        mpr("あなたはダンジョンから脱出した！");
#else
        mpr("You have escaped!");
#endif

        for (i = 0; i < ENDOFPACK; i++)
        {
            if (is_valid_item( you.inv[i] )
                && you.inv[i].base_type == OBJ_ORBS)
            {
                ouch(-9999, 0, KILLED_BY_WINNING);
            }
        }

        ouch(-9999, 0, KILLED_BY_LEAVING);
    }

#ifdef JP
    mpr("あなたは階段を進んでいった……。");
#else
    mpr("Entering...");
#endif

    //今移動した階の＠マークに消しを入れる
    env.map[you.x_pos - 1][you.y_pos - 1] = mapch2( env.grid[you.x_pos][you.y_pos] );

    you.prev_targ = MHITNOT;
    you.pet_target = MHITNOT;

    if (player_in_branch( BRANCH_VESTIBULE_OF_HELL ))
    {
#ifdef JP
        mpr("地獄へのご来場ありがとうございました。すぐにまたお越しください。");
#else
        mpr("Thank you for visiting Hell. Please come again soon.");
#endif
        you.where_are_you = BRANCH_MAIN_DUNGEON;
        you.your_level = you.hell_exit;
        stair_find = DNGN_STONE_STAIRS_UP_I;
    }

    if (player_in_hell())
    {
        you.where_are_you = BRANCH_VESTIBULE_OF_HELL;
        you.your_level = 27;
    }

    switch (stair_find)
    {
    case DNGN_RETURN_FROM_ORCISH_MINES:
    case DNGN_RETURN_FROM_HIVE:
    case DNGN_RETURN_FROM_LAIR:
    case DNGN_RETURN_FROM_VAULTS:
    case DNGN_RETURN_FROM_TEMPLE:
    case DNGN_RETURN_FROM_ZOT:
#ifdef JP
        mpr("ダンジョンに戻ってきた！");
#else
        mpr("Welcome back to the Dungeon!");
#endif
        you.where_are_you = BRANCH_MAIN_DUNGEON;
        break;
    case DNGN_RETURN_FROM_SLIME_PITS:
    case DNGN_RETURN_FROM_SNAKE_PIT:
    case DNGN_RETURN_FROM_SWAMP:
#ifdef JP
        mpr("獣の棲み処に戻ってきた！");
#else
        mpr("Welcome back to the Lair of Beasts!");
#endif
        you.where_are_you = BRANCH_LAIR;
        break;
    case DNGN_RETURN_FROM_CRYPT:
    case DNGN_RETURN_FROM_HALL_OF_BLADES:
#ifdef JP
        mpr("宝物庫に戻ってきた！");
#else
        mpr("Welcome back to the Vaults!");
#endif
        you.where_are_you = BRANCH_VAULTS;
        break;
    case DNGN_RETURN_FROM_TOMB:
#ifdef JP
        mpr("地下墓地に戻ってきた！");
#else
        mpr("Welcome back to the Crypt!");
#endif
        you.where_are_you = BRANCH_CRYPT;
        break;
    case DNGN_RETURN_FROM_ELVEN_HALLS:
#ifdef JP
        mpr("オークの坑道に戻ってきた！");
#else
        mpr("Welcome back to the Orcish Mines!");
#endif
        you.where_are_you = BRANCH_ORCISH_MINES;
        break;
    }

    unsigned char stair_taken = stair_find;

    if (player_is_levitating())
    {
        if ( you.duration[DUR_CONTROLLED_FLIGHT] || wearing_amulet(AMU_CONTROLLED_FLIGHT) )
#ifdef JP
            mpr("あなたは上の階へと飛行した。");
#else
            mpr("You fly upwards.");
#endif
        else
#ifdef JP
            mpr("あなたは上昇していった……そして天井にぶつかってしまった！");
#else
            mpr("You float upwards... And bob straight up to the ceiling!");
#endif
    }
    else
#ifdef JP
        mpr("あなたは階段を昇り終えた。");
#else
        mpr("You climb upwards.");
#endif

    load(stair_taken, LOAD_ENTER_LEVEL, was_a_labyrinth, old_level, old_where);

    you.turn_is_over = 1;

    save_game(false);

    new_level();

#ifdef USE_TILE
    if (Options.use_tile)
    {
        TileLoadWall(false);
        tile_clear_buf();
    }
#endif

    viewwindow(1, true);


    if (you.skills[SK_TRANSLOCATIONS] > 0 && !allow_control_teleport( true ))
#ifdef JP
        mpr( "あなたは強力な魔法が空間を歪めているのを感じ取った。", MSGCH_WARN );
#else
        mpr( "You sense a powerful magical force warping space.", MSGCH_WARN );
#endif
    if (collect_travel_data) {
        // Update stair information for the stairs we just ascended, and the
        // down stairs we're currently on.
        level_id  new_level_id    = level_id::get_current_level_id();

        if (you.level_type != LEVEL_PANDEMONIUM &&
                you.level_type != LEVEL_ABYSS &&
                you.level_type != LEVEL_LABYRINTH)
        {
            LevelInfo &new_level_info =
                        travel_cache.get_level_info(new_level_id);
            new_level_info.update();

            // First we update the old level's stair.
            level_pos lp;
            lp.id  = new_level_id;
            lp.pos.x = you.x_pos;
            lp.pos.y = you.y_pos;

            bool guess = false;
            // Ugly hack warning:
            // The stairs in the Vestibule of Hell exhibit special behaviour:
            // they always lead back to the dungeon level that the player
            // entered the Vestibule from. This means that we need to pretend
            // we don't know where the upstairs from the Vestibule go each time
            // we take it. If we don't, interlevel travel may try to use portals
            // to Hell as shortcuts between dungeon levels, which won't work,
            // and will confuse the dickens out of the player (well, it confused
            // the dickens out of me when it happened).
            if (new_level_id.branch == BRANCH_MAIN_DUNGEON &&
                    old_level_id.branch == BRANCH_VESTIBULE_OF_HELL) {
                lp.id.depth = -1;
                lp.pos.x = lp.pos.y = -1;
                guess = true;
            }

            old_level_info.update_stair(stair_x, stair_y, lp, guess);

            // We *guess* that going up a staircase lands us on a downstair,
            // and that we can descend that downstair and get back to where we
            // came from. This assumption is guaranteed false when climbing out
            // of one of the branches of Hell.
            if (new_level_id.branch != BRANCH_VESTIBULE_OF_HELL) {
                // Set the new level's stair, assuming arbitrarily that going
                // downstairs will land you on the same upstairs you took to
                // begin with (not necessarily true).
                lp.id = old_level_id;
                lp.pos.x = stair_x;
                lp.pos.y = stair_y;
                new_level_info.update_stair(you.x_pos, you.y_pos, lp, true);
            }
        }
    }
}                               // end up_stairs()

void down_stairs( bool remove_stairs, int old_level )
{
    int i;
    char old_level_type = you.level_type;
    bool was_a_labyrinth = false;
    const unsigned char stair_find = grd[you.x_pos][you.y_pos];

    //int old_level = you.your_level;
    bool leave_abyss_pan = false;
    char old_where = you.where_are_you;

#ifdef SHUT_LABYRINTH
    if (stair_find == DNGN_ENTER_LABYRINTH)
    {
#ifdef JP
        mpr("残念ながら、ダンジョンのこの区画は燻蒸のために封鎖されている。");
        mpr("");
#else
        mpr("Sorry, this section of the dungeon is closed for fumigation.");
        mpr("Try again next release.");
#endif
        return;
    }
#endif

    // probably still need this check here (teleportation) -- bwr
    if ((stair_find < DNGN_ENTER_LABYRINTH
            || stair_find > DNGN_ROCK_STAIRS_DOWN)
        && stair_find != DNGN_ENTER_HELL
        && ((stair_find < DNGN_ENTER_DIS
                || stair_find > DNGN_TRANSIT_PANDEMONIUM)
            && stair_find != DNGN_STONE_ARCH)
        && !(stair_find >= DNGN_ENTER_ORCISH_MINES
            && stair_find < DNGN_RETURN_FROM_ORCISH_MINES))
    {
#ifdef JP
        mpr( "あなたはここから下の階に降りることはできない！" );
#else
        mpr( "You can't go down here!" );
#endif
        return;
    }

    if (stair_find >= DNGN_ENTER_LABYRINTH
        && stair_find <= DNGN_ROCK_STAIRS_DOWN
        && player_in_branch( BRANCH_VESTIBULE_OF_HELL ))
    {
#ifdef JP
        mpr("神秘的な力があなたが階段を降りるのを妨げた。");
#else
        mpr("A mysterious force prevents you from descending the staircase.");
#endif
        return;
    }                           /* down stairs in vestibule are one-way */

    if (stair_find == DNGN_STONE_ARCH)
    {
#ifdef JP
        mpr("あなたはここから下の階に降りることはできない！");
#else
        mpr("You can't go down here!");
#endif
        return;
    }

    if (player_is_levitating() && !wearing_amulet(AMU_CONTROLLED_FLIGHT))
    {
#ifdef JP
        mpr("あなたは床から高くに浮いたままだ！");
#else
        mpr("You're floating high up above the floor!");
#endif
        return;
    }

    if (stair_find == DNGN_ENTER_ZOT)
    {
        int num_runes = 0;

        for (i = 0; i < ENDOFPACK; i++)
        {
            if (is_valid_item( you.inv[i] )
                && you.inv[i].base_type == OBJ_MISCELLANY
                && you.inv[i].sub_type == MISC_RUNE_OF_ZOT)
            {
                num_runes += you.inv[i].quantity;
            }
        }

        if (num_runes < NUMBER_OF_RUNES_NEEDED)
        {
            switch (NUMBER_OF_RUNES_NEEDED)
            {
            case 1:
#ifdef JP
                mpr("あなたがこの場所に入るには1個のルーンが必要だ。");
#else
                mpr("You need a Rune to enter this place.");
#endif
                break;

            default:
                snprintf( info, INFO_SIZE,
#ifdef JP
                          "あなたがこの場所に入るには少なくとも%d個のルーンが必要だ。",
#else
                          "You need at least %d Runes to enter this place.",
#endif
                          NUMBER_OF_RUNES_NEEDED );

                mpr(info);
            }
            return;
        }
    }

    // Interlevel travel data:
    bool collect_travel_data = you.level_type != LEVEL_LABYRINTH
                && you.level_type != LEVEL_ABYSS
                && you.level_type != LEVEL_PANDEMONIUM;

    level_id  old_level_id    = level_id::get_current_level_id();
    LevelInfo &old_level_info = travel_cache.get_level_info(old_level_id);
    int stair_x = you.x_pos, stair_y = you.y_pos;
    if (collect_travel_data)
        old_level_info.update();


    if (you.level_type == LEVEL_PANDEMONIUM
            && stair_find == DNGN_TRANSIT_PANDEMONIUM)
    {
        was_a_labyrinth = true;
    }
    else
    {
        if (you.level_type != LEVEL_DUNGEON)
            was_a_labyrinth = true;

        you.level_type = LEVEL_DUNGEON;
    }

#ifdef JP
    mpr("あなたは階段を進んでいった……。");
#else
    mpr("Entering...");
#endif

    //今移動した階の＠マークに消しを入れる
    env.map[you.x_pos - 1][you.y_pos - 1] = mapch2( env.grid[you.x_pos][you.y_pos] );

    you.prev_targ = MHITNOT;
    you.pet_target = MHITNOT;

    if (stair_find == DNGN_ENTER_HELL)
    {
        you.where_are_you = BRANCH_VESTIBULE_OF_HELL;
        you.hell_exit = you.your_level;

#ifdef JP
        mpr("地獄にようこそ！");
        mpr("滞在をお楽しみください。");
#else
        mpr("Welcome to Hell!");
        mpr("Please enjoy your stay.");
#endif

        more();

        you.your_level = 26;    // = 59;
    }

    if ((stair_find >= DNGN_ENTER_DIS
            && stair_find <= DNGN_ENTER_TARTARUS)
        || (stair_find >= DNGN_ENTER_ORCISH_MINES
            && stair_find < DNGN_RETURN_FROM_ORCISH_MINES))
    {
        // no idea why such a huge switch and not 100-grd[][]
        // planning ahead for re-organizaing grd[][] values - 13jan2000 {dlb}
#ifdef JP
        strcpy( info, "" );
#else
        strcpy( info, "Welcome to " );
#endif
        switch (stair_find)
        {
        case DNGN_ENTER_DIS:
#ifdef JP
            strcat(info, "鉄の都ディースにようこそ！");
#else
            strcat(info, "the Iron City of Dis!");
#endif
            you.where_are_you = BRANCH_DIS;
            you.your_level = 26;
            break;
        case DNGN_ENTER_GEHENNA:
#ifdef JP
            strcat(info, "ゲヘナにようこそ！");
#else
            strcat(info, "Gehenna!");
#endif
            you.where_are_you = BRANCH_GEHENNA;
            you.your_level = 26;
            break;
        case DNGN_ENTER_COCYTUS:
#ifdef JP
            strcat(info, "コキュートスにようこそ！");
#else
            strcat(info, "Cocytus!");
#endif
            you.where_are_you = BRANCH_COCYTUS;
            you.your_level = 26;
            break;
        case DNGN_ENTER_TARTARUS:
#ifdef JP
            strcat(info, "タルタロスにようこそ！");
#else
            strcat(info, "Tartarus!");
#endif
            you.where_are_you = BRANCH_TARTARUS;
            you.your_level = 26;
            break;
        case DNGN_ENTER_ORCISH_MINES:
#ifdef JP
            strcat(info, "オークの坑道にようこそ！");
#else
            strcat(info, "the Orcish Mines!");
#endif
            you.where_are_you = BRANCH_ORCISH_MINES;
            break;
        case DNGN_ENTER_HIVE:
#ifdef JP
            strcpy(info, "あなたは全ての方角からブーンと唸る音を耳にした。");
#else
            strcpy(info, "You hear a buzzing sound coming from all directions.");
#endif
            you.where_are_you = BRANCH_HIVE;
            break;
        case DNGN_ENTER_LAIR:
#ifdef JP
            strcat(info, "獣の棲み処にようこそ！");
#else
            strcat(info, "the Lair of Beasts!");
#endif
            you.where_are_you = BRANCH_LAIR;
            break;
        case DNGN_ENTER_SLIME_PITS:
#ifdef JP
            strcat(info, "スライムの穴ぐらにようこそ！");
#else
            strcat(info, "the Pits of Slime!");
#endif
            you.where_are_you = BRANCH_SLIME_PITS;
            break;
        case DNGN_ENTER_VAULTS:
#ifdef JP
            strcat(info, "宝物庫にようこそ！");
#else
            strcat(info, "the Vaults!");
#endif
            you.where_are_you = BRANCH_VAULTS;
            break;
        case DNGN_ENTER_CRYPT:
#ifdef JP
            strcat(info, "地下墓地にようこそ！");
#else
            strcat(info, "the Crypt!");
#endif
            you.where_are_you = BRANCH_CRYPT;
            break;
        case DNGN_ENTER_HALL_OF_BLADES:
#ifdef JP
            strcat(info, "刃の広間にようこそ！");
#else
            strcat(info, "the Hall of Blades!");
#endif
            you.where_are_you = BRANCH_HALL_OF_BLADES;
            break;
        case DNGN_ENTER_ZOT:
#ifdef JP
            strcat(info, "ゾットの領域にようこそ！");
#else
            strcat(info, "the Hall of Zot!");
#endif
            you.where_are_you = BRANCH_HALL_OF_ZOT;
            break;
        case DNGN_ENTER_TEMPLE:
#ifdef JP
            strcat(info, "諸宗派の寺院にようこそ！");
#else
            strcat(info, "the Ecumenical Temple!");
#endif
            you.where_are_you = BRANCH_ECUMENICAL_TEMPLE;
            break;
        case DNGN_ENTER_SNAKE_PIT:
#ifdef JP
            strcat(info, "蛇穴にようこそ！");
#else
            strcat(info, "the Snake Pit!");
#endif
            you.where_are_you = BRANCH_SNAKE_PIT;
            break;
        case DNGN_ENTER_ELVEN_HALLS:
#ifdef JP
            strcat(info, "エルフの大広間にようこそ！");
#else
            strcat(info, "the Elven Halls!");
#endif
            you.where_are_you = BRANCH_ELVEN_HALLS;
            break;
        case DNGN_ENTER_TOMB:
#ifdef JP
            strcat(info, "霊廟にようこそ！");
#else
            strcat(info, "the Tomb!");
#endif
            you.where_are_you = BRANCH_TOMB;
            break;
        case DNGN_ENTER_SWAMP:
#ifdef JP
            strcat(info, "沼にようこそ！");
#else
            strcat(info, "the Swamp!");
#endif
            you.where_are_you = BRANCH_SWAMP;
            break;
        }

        mpr(info);
    }
    else if (stair_find == DNGN_ENTER_LABYRINTH)
    {
        you.level_type = LEVEL_LABYRINTH;
        grd[you.x_pos][you.y_pos] = DNGN_FLOOR;
    }
    else if (stair_find == DNGN_ENTER_ABYSS)
    {
        you.level_type = LEVEL_ABYSS;
    }
    else if (stair_find == DNGN_ENTER_PANDEMONIUM)
    {
        you.level_type = LEVEL_PANDEMONIUM;
    }

    if (you.level_type == LEVEL_LABYRINTH || you.level_type == LEVEL_ABYSS
        || you.level_type == LEVEL_PANDEMONIUM)
    {
        char glorpstr[kFileNameSize];
        char del_file[kFileNameSize];
        int sysg;

#ifdef SAVE_DIR_PATH
        snprintf( glorpstr, sizeof(glorpstr),
                  SAVE_DIR_PATH "%s%d", you.your_name, (int) getuid() );
#else
        strncpy(glorpstr, you.your_name, kFileNameLen);

        // glorpstr [strlen(glorpstr)] = 0;
        // This is broken. Length is not valid yet! We have to check if we got
        // a trailing NULL; if not, write one:
        /* is name 6 chars or more? */
        if (strlen(you.your_name) > kFileNameLen - 1)
            glorpstr[kFileNameLen] = '\0';
#endif

        strcpy(del_file, glorpstr);
        strcat(del_file, ".lab");

#ifdef DOS
        strupr(del_file);
#endif
        sysg = unlink(del_file);

#if DEBUG_DIAGNOSTICS
#ifdef JP
        strcpy( info, "Deleting: " );
#else
        strcpy( info, "Deleting: " );
#endif
        strcat( info, del_file );
        mpr( info, MSGCH_DIAGNOSTICS );
        more();
#endif
    }

    if (stair_find == DNGN_EXIT_ABYSS || stair_find == DNGN_EXIT_PANDEMONIUM)
    {
        leave_abyss_pan = true;
#ifdef JP
        mpr("あなたは門を抜け、階段を上りつめた。");
#else
        mpr("You pass through the gate, and find yourself at the top of a staircase.");
#endif
        more();
    }

    if (!player_is_levitating()
        && you.conf
        && (stair_find >= DNGN_STONE_STAIRS_DOWN_I
            && stair_find <= DNGN_ROCK_STAIRS_DOWN)
        && random2(100) > you.dex)
    {
#ifdef JP
        mpr("混乱のあまりあなたは躓いて、階段から転がり落ちてしまった。");
#else
        mpr("In your confused state, you trip and fall down the stairs.");
#endif

        // Nastier than when climbing stairs, but you'll aways get to
        // your destination, -- bwr
        ouch( roll_dice( 6 + you.burden_state, 10 ), 0,
              KILLED_BY_FALLING_DOWN_STAIRS );
    }

    if (you.level_type == LEVEL_DUNGEON)
        you.your_level++;

    int stair_taken = stair_find;

    //unsigned char save_old = 1;

    if (you.level_type == LEVEL_LABYRINTH || you.level_type == LEVEL_ABYSS)
        stair_taken = DNGN_FLOOR;       //81;

    if (you.level_type == LEVEL_PANDEMONIUM)
        stair_taken = DNGN_TRANSIT_PANDEMONIUM;

    if (remove_stairs)
        grd[you.x_pos][you.y_pos] = DNGN_FLOOR;

    switch (you.level_type)
    {
    case LEVEL_LABYRINTH:
#ifdef JP
        mpr("あなたは暗く不気味なラビリンスへと踏み込んでいった。");
#else
        mpr("You enter a dark and forbidding labyrinth.");
#endif
        break;

    case LEVEL_ABYSS:
#ifdef JP
        mpr("あなたはアビスに訪れた！");
        mpr("戻るためには外に続く門を探さなければならない。");
#else
        mpr("You enter the Abyss!");
        mpr("To return, you must find a gate leading back.");
#endif
        break;

    case LEVEL_PANDEMONIUM:
        if (old_level_type == LEVEL_PANDEMONIUM)
#ifdef JP
            mpr("あなたはパンデモニウムの別の領域に踏み込んでいった。");
#else
            mpr("You pass into a different region of Pandemonium.");
#endif
        else
        {
#ifdef JP
            mpr("あなたはパンデモニウムの大広間に足を踏み入れた！");
            mpr("戻るためには外に続く門を探さなければならない。");
#else
            mpr("You enter the halls of Pandemonium!");
            mpr("To return, you must find a gate leading back.");
#endif
        }
        break;

    default:
#ifdef JP
        mpr("あなたは階段を降り終えた。");
#else
        mpr("You climb downwards.");
#endif
        break;
    }

    load(stair_taken, LOAD_ENTER_LEVEL, was_a_labyrinth, old_level, old_where);

    unsigned char pc = 0;
    unsigned char pt = random2avg(28, 3);

    switch (you.level_type)
    {
    case LEVEL_LABYRINTH:
        you.your_level++;
        break;

    case LEVEL_ABYSS:
        grd[you.x_pos][you.y_pos] = DNGN_FLOOR;

        //if (old_level_type != LEVEL_PANDEMONIUM)
        //    you.your_level--;   // Linley-suggested addition 17jan2000 {dlb}
        if (old_level_type == LEVEL_LABYRINTH)
            you.your_level -= 2;
        else if (old_level_type != LEVEL_ABYSS
              && old_level_type != LEVEL_PANDEMONIUM)
            you.your_level--; // 05/03/05fixed

        init_pandemonium();     /* colours only */

        if (player_in_hell())
        {
            you.where_are_you = BRANCH_MAIN_DUNGEON;
            you.your_level = you.hell_exit - 1;
        }
        break;

    case LEVEL_PANDEMONIUM:
        if (old_level_type == LEVEL_PANDEMONIUM)
        {
            init_pandemonium();
            for (pc = 0; pc < pt; pc++)
                pandemonium_mons();
        }
        else
        {
            // Linley-suggested addition 17jan2000 {dlb}
            if (old_level_type != LEVEL_ABYSS)
                you.your_level--;

            init_pandemonium();

            for (pc = 0; pc < pt; pc++)
                pandemonium_mons();

            if (player_in_hell())
            {
                you.where_are_you = BRANCH_MAIN_DUNGEON;
                you.hell_exit = 26;
                you.your_level = 26;
            }
        }
        break;

    default:
        break;
    }

    you.turn_is_over = 1;

    save_game(false);

    new_level();

#ifdef USE_TILE
    if (Options.use_tile)
    {
        TileLoadWall(false);
        tile_clear_buf();
    }
#endif

    viewwindow(1, true);

    if (you.skills[SK_TRANSLOCATIONS] > 0 && !allow_control_teleport( true ))
#ifdef JP
        mpr( "あなたは強力な魔法が空間を歪めているのを感じ取った。", MSGCH_WARN );
#else
        mpr( "You sense a powerful magical force warping space.", MSGCH_WARN );
#endif
    if (collect_travel_data) {
        // Update stair information for the stairs we just descended, and the
        // upstairs we're currently on.
        level_id  new_level_id    = level_id::get_current_level_id();

        if (you.level_type != LEVEL_PANDEMONIUM &&
                you.level_type != LEVEL_ABYSS &&
                you.level_type != LEVEL_LABYRINTH)
        {
            LevelInfo &new_level_info =
                            travel_cache.get_level_info(new_level_id);
            new_level_info.update();

            // First we update the old level's stair.
            level_pos lp;
            lp.id  = new_level_id;
            lp.pos.x = you.x_pos;
            lp.pos.y = you.y_pos;

            old_level_info.update_stair(stair_x, stair_y, lp);

            // Then the new level's stair, assuming arbitrarily that going
            // upstairs will land you on the same downstairs you took to begin
            // with (not necessarily true).
            lp.id = old_level_id;
            lp.pos.x = stair_x;
            lp.pos.y = stair_y;
            new_level_info.update_stair(you.x_pos, you.y_pos, lp, true);
        }
    }
}                               // end down_stairs()

void new_level(void)
{
    int curr_subdungeon_level = you.your_level + 1;

#ifdef USE_TILE
    mpr_on(MODE_STAT);
#endif

    textcolor(LIGHTGREY);

    // maybe last part better expresssed as <= PIT {dlb}
    if (player_in_hell() || player_in_branch( BRANCH_VESTIBULE_OF_HELL ))
        curr_subdungeon_level = you.your_level - 26;

    /* Remember, must add this to the death_string in ouch */
    if (you.where_are_you >= BRANCH_ORCISH_MINES
        && you.where_are_you <= BRANCH_SWAMP)
    {
        curr_subdungeon_level = you.your_level
                                    - you.branch_stairs[you.where_are_you - 10];
    }
#ifdef JP
    gotoxy(47, 12);
#else
    gotoxy(46, 12);
#endif

#if DEBUG_DIAGNOSTICS
    cprintf( "(%d) ", you.your_level + 1 );
#endif

    env.floor_colour = LIGHTGREY;
    env.rock_colour  = BROWN;

    if (you.level_type == LEVEL_PANDEMONIUM)
    {
#ifdef JP
        cprintf("パンデモニウム           ");
#else
        cprintf("- Pandemonium            ");
#endif
        env.floor_colour = (mcolour[env.mons_alloc[9]] == BLACK)
                                    ? LIGHTGREY : mcolour[env.mons_alloc[9]];
        env.rock_colour = (mcolour[env.mons_alloc[8]] == BLACK)
                                    ? LIGHTGREY : mcolour[env.mons_alloc[8]];

    }
    else if (you.level_type == LEVEL_ABYSS)
    {
#ifdef JP
        cprintf("アビス                    ");
#else
        cprintf("- The Abyss               ");
#endif
        env.floor_colour = (mcolour[env.mons_alloc[9]] == BLACK)
                                    ? LIGHTGREY : mcolour[env.mons_alloc[9]];
        env.rock_colour = (mcolour[env.mons_alloc[8]] == BLACK)
                                    ? LIGHTGREY : mcolour[env.mons_alloc[8]];

    }
    else if (you.level_type == LEVEL_LABYRINTH)
    {
#ifdef JP
        cprintf("ラビリンス              ");
#else
        cprintf("- a Labyrinth           ");
#endif
    }
    else
    {
#ifdef JP
        // level_type == LEVEL_DUNGEON
        /*
        if (!player_in_branch( BRANCH_VESTIBULE_OF_HELL ))
            cprintf( "%d", curr_subdungeon_level );
        */
#else
        // level_type == LEVEL_DUNGEON
        if (!player_in_branch( BRANCH_VESTIBULE_OF_HELL ))
            cprintf( "%d", curr_subdungeon_level );
#endif

        switch (you.where_are_you)
        {
        case BRANCH_MAIN_DUNGEON:
#ifdef JP
            cprintf("地下%d階                  ", curr_subdungeon_level);
#else
            cprintf(" of the Dungeon           ");
#endif
            break;
        case BRANCH_DIS:
            env.floor_colour = CYAN;
            env.rock_colour = CYAN;

#ifdef JP
            cprintf("ディース%d階                ", curr_subdungeon_level);
#else
            cprintf(" of Dis                   ");
#endif
            break;
        case BRANCH_GEHENNA:
            env.floor_colour = DARKGREY;
            env.rock_colour = RED;

#ifdef JP
            cprintf("ゲヘナ%d階                ", curr_subdungeon_level);
#else
            cprintf(" of Gehenna               ");
#endif
            break;
        case BRANCH_VESTIBULE_OF_HELL:
            env.floor_colour = LIGHTGREY;
            env.rock_colour = LIGHTGREY;

#ifdef JP
            cprintf("- 地獄の入り口                     ");
#else
            cprintf("- the Vestibule of Hell            ");
#endif
            break;
        case BRANCH_COCYTUS:
            env.floor_colour = LIGHTBLUE;
            env.rock_colour = LIGHTCYAN;

#ifdef JP
            cprintf("コキュートス%d階              ", curr_subdungeon_level);
#else
            cprintf(" of Cocytus                   ");
#endif
            break;
        case BRANCH_TARTARUS:
            env.floor_colour = DARKGREY;
            env.rock_colour = DARKGREY;

#ifdef JP
            cprintf("タルタロス%d階              ", curr_subdungeon_level);
#else
            cprintf(" of Tartarus                ");
#endif
            break;
        case BRANCH_INFERNO:
            env.floor_colour = LIGHTRED;
            env.rock_colour = RED;

#ifdef JP
            cprintf("火炎地獄%d階                  ", curr_subdungeon_level);
#else
            cprintf(" of the Inferno               ");
#endif
            break;
        case BRANCH_THE_PIT:
            env.floor_colour = RED;
            env.rock_colour = DARKGREY;

#ifdef JP
            cprintf("窖%d階                   ", curr_subdungeon_level);
#else
            cprintf(" of the Pit              ");
#endif
            break;
        case BRANCH_ORCISH_MINES:
            env.floor_colour = BROWN;
            env.rock_colour = BROWN;

#ifdef JP
            cprintf("オークの坑道%d階              ", curr_subdungeon_level);
#else
            cprintf(" of the Orcish Mines          ");
#endif
            break;
        case BRANCH_HIVE:
            env.floor_colour = YELLOW;
            env.rock_colour = BROWN;

#ifdef JP
            cprintf("蜂の巣%d階                    ", curr_subdungeon_level);
#else
            cprintf(" of the Hive                  ");
#endif
            break;
        case BRANCH_LAIR:
            env.floor_colour = GREEN;
            env.rock_colour = BROWN;

#ifdef JP
            cprintf("獣の棲み処%d階                ", curr_subdungeon_level);
#else
            cprintf(" of the Lair                  ");
#endif
            break;
        case BRANCH_SLIME_PITS:
            env.floor_colour = GREEN;
            env.rock_colour = LIGHTGREEN;

#ifdef JP
            cprintf("スライムの穴ぐら%d階          ", curr_subdungeon_level);
#else
            cprintf(" of the Slime Pits            ");
#endif
            break;
        case BRANCH_VAULTS:
            env.floor_colour = LIGHTGREY;
            env.rock_colour = BROWN;

#ifdef JP
            cprintf("宝物庫%d階                    ", curr_subdungeon_level);
#else
            cprintf(" of the Vaults                ");
#endif
            break;
        case BRANCH_CRYPT:
            env.floor_colour = LIGHTGREY;
            env.rock_colour = LIGHTGREY;

#ifdef JP
            cprintf("地下墓地%d階                  ", curr_subdungeon_level);
#else
            cprintf(" of the Crypt                 ");
#endif
            break;
        case BRANCH_HALL_OF_BLADES:
            env.floor_colour = LIGHTGREY;
            env.rock_colour = LIGHTGREY;

#ifdef JP
            cprintf("刃の広間%d階                  ", curr_subdungeon_level);
#else
            cprintf(" of the Hall of Blades        ");
#endif
            break;

        case BRANCH_HALL_OF_ZOT:
            if (you.your_level - you.branch_stairs[7] <= 1)
            {
                env.floor_colour = LIGHTGREY;
                env.rock_colour = LIGHTGREY;

            }
            else
            {
                switch (you.your_level - you.branch_stairs[7])
                {
                case 2:
                    env.rock_colour = LIGHTGREY;
                    env.floor_colour = BLUE;
                    break;
                case 3:
                    env.rock_colour = BLUE;
                    env.floor_colour = LIGHTBLUE;
                    break;
                case 4:
                    env.rock_colour = LIGHTBLUE;
                    env.floor_colour = MAGENTA;

                    break;
                case 5:
                    env.rock_colour = MAGENTA;
                    env.floor_colour = LIGHTMAGENTA;
                    break;
                }
            }
#ifdef JP
            cprintf("ゾットの領域%d階              ", curr_subdungeon_level);
#else
            cprintf(" of the Realm of Zot          ");
#endif
            break;

        case BRANCH_ECUMENICAL_TEMPLE:
            env.floor_colour = LIGHTGREY;
            env.rock_colour = LIGHTGREY;

#ifdef JP
            cprintf("諸宗派の寺院                  ");
#else
            cprintf(" of the Temple                ");
#endif
            break;
        case BRANCH_SNAKE_PIT:
            env.floor_colour = LIGHTGREEN;
            env.rock_colour = YELLOW;

#ifdef JP
            cprintf("蛇穴%d階                      ", curr_subdungeon_level);
#else
            cprintf(" of the Snake Pit             ");
#endif
            break;
        case BRANCH_ELVEN_HALLS:
            env.floor_colour = DARKGREY;
            env.rock_colour = LIGHTGREY;

#ifdef JP
            cprintf("エルフの大広間%d階            ", curr_subdungeon_level);
#else
            cprintf(" of the Elven Halls           ");
#endif
            break;
        case BRANCH_TOMB:
            env.floor_colour = YELLOW;
            env.rock_colour = LIGHTGREY;

#ifdef JP
            cprintf("霊廟%d階                      ", curr_subdungeon_level);
#else
            cprintf(" of the Tomb                  ");
#endif
            break;
        case BRANCH_SWAMP:
            env.floor_colour = BROWN;
            env.rock_colour = BROWN;

#ifdef JP
            cprintf("沼%d階                        ", curr_subdungeon_level);
#else
            cprintf(" of the Swamp                 ");
#endif
            break;
        }
    }                           // end else

#ifdef USE_TILE
    mpr_on(MODE_CRT);

    // init micromap
    init_gmap();
#endif

}                               // end new_level()

static void dart_trap( bool trap_known, int trapped, struct bolt &pbolt,
                       bool poison )
{
    int damage_taken = 0;
    int trap_hit, your_dodge;

    if (random2(10) < 2 || (trap_known && !one_chance_in(4)))
    {
#ifdef JP
        snprintf( info, INFO_SIZE, "あなたは%sの罠をやり過ごした。",
#else
        snprintf( info, INFO_SIZE, "You avoid triggering a%s trap.",
#endif
                                    pbolt.beam_name );
        mpr(info);
        return;
    }

    if (you.equip[EQ_SHIELD] != -1 && one_chance_in(3))
        exercise( SK_SHIELDS, 1 );

#ifdef JP
    snprintf( info, INFO_SIZE, "%sが撃ち出されて", pbolt.beam_name );
#else
    snprintf( info, INFO_SIZE, "A%s shoots out and ", pbolt.beam_name );
#endif

    if (random2( 50 + 10 * you.shield_blocks * you.shield_blocks )
                                                < player_shield_class())
    {
        you.shield_blocks++;
#ifdef JP
        strcat( info, "あなたの盾に当たった。" );
#else
        strcat( info, "hits your shield." );
#endif
        mpr(info);
        goto out_of_trap;
    }

    // note that this uses full ( not random2limit(foo,40) ) player_evasion.
    trap_hit = (20 + (you.your_level * 2)) * random2(200) / 100;

    your_dodge = player_evasion() + random2(you.dex) / 3
                            - 2 + (you.duration[DUR_REPEL_MISSILES] * 10);

    if (trap_hit >= your_dodge && you.duration[DUR_DEFLECT_MISSILES] == 0)
    {
#ifdef JP
        strcat( info, "あなたに命中した！" );
#else
        strcat( info, "hits you!" );
#endif
        mpr(info);

        if (poison && random2(100) < 50 - (3 * player_AC()) / 2
                && !player_res_poison())
        {
            poison_player( 1 + random2(3) );
        }

        damage_taken = roll_dice( pbolt.damage );
        damage_taken -= random2( player_AC() + 1 );

        if (damage_taken > 0)
            ouch( damage_taken, 0, KILLED_BY_TRAP, pbolt.beam_name );
    }
    else
    {
#ifdef JP
        strcat( info, "あなたから外れた。" );
#else
        strcat( info, "misses you." );
#endif
        mpr(info);
    }

    if (player_light_armour() && coinflip())
        exercise( SK_DODGING, 1 );

  out_of_trap:

    pbolt.target_x = you.x_pos;
    pbolt.target_y = you.y_pos;

    if (coinflip())
        itrap( pbolt, trapped );
}                               // end dart_trap()

//
// itrap takes location from target_x, target_y of bolt strcture.
//

void itrap( struct bolt &pbolt, int trapped )
{
    int base_type = OBJ_MISSILES;
    int sub_type = MI_DART;

    switch (env.trap[trapped].type)
    {
    case TRAP_DART:
        base_type = OBJ_MISSILES;
        sub_type = MI_DART;
        break;
    case TRAP_ARROW:
        base_type = OBJ_MISSILES;
        sub_type = MI_ARROW;
        break;
    case TRAP_BOLT:
        base_type = OBJ_MISSILES;
        sub_type = MI_BOLT;
        break;
    case TRAP_SPEAR:
        base_type = OBJ_WEAPONS;
        sub_type = WPN_SPEAR;
        break;
    case TRAP_AXE:
        base_type = OBJ_WEAPONS;
        sub_type = WPN_HAND_AXE;
        break;
    case TRAP_NEEDLE:
        base_type = OBJ_MISSILES;
        sub_type = MI_NEEDLE;
        break;
    default:
        return;
    }

    trap_item( base_type, sub_type, pbolt.target_x, pbolt.target_y );

    return;
}                               // end itrap()

void handle_traps(char trt, int i, bool trap_known)
{
    struct bolt beam;

    switch (trt)
    {
    case TRAP_DART:
#ifdef JP
        strcpy(beam.beam_name, "投げ矢");
#else
        strcpy(beam.beam_name, " dart");
#endif
        beam.damage = dice_def( 1, 4 + (you.your_level / 2) );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_NEEDLE:
#ifdef JP
        strcpy(beam.beam_name, "吹き矢針");
#else
        strcpy(beam.beam_name, " needle");
#endif
        beam.damage = dice_def( 1, 0 );
        dart_trap(trap_known, i, beam, true);
        break;

    case TRAP_ARROW:
#ifdef JP
        strcpy(beam.beam_name, "矢");
#else
        strcpy(beam.beam_name, "n arrow");
#endif
        beam.damage = dice_def( 1, 7 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_BOLT:
#ifdef JP
        strcpy(beam.beam_name, "クロスボウの矢");
#else
        strcpy(beam.beam_name, " bolt");
#endif
        beam.damage = dice_def( 1, 13 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_SPEAR:
#ifdef JP
        strcpy(beam.beam_name, "槍");
#else
        strcpy(beam.beam_name, " spear");
#endif
        beam.damage = dice_def( 1, 10 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_AXE:
#ifdef JP
        strcpy(beam.beam_name, "斧");
#else
        strcpy(beam.beam_name, "n axe");
#endif
        beam.damage = dice_def( 1, 15 + you.your_level );
        dart_trap(trap_known, i, beam, false);
        break;

    case TRAP_TELEPORT:
#ifdef JP
        mpr("あなたはテレポートの罠に踏み込んだ！");
#else
        mpr("You enter a teleport trap!");
#endif

        if (scan_randarts(RAP_PREVENT_TELEPORTATION))
#ifdef JP
            mpr("あなたは奇妙な安定感を覚えた。");
#else
            mpr("You feel a weird sense of stasis.");
#endif
        else
            you_teleport2( true );
        break;

    case TRAP_AMNESIA:
#ifdef JP
        mpr("あなたは瞬間的な見当識喪失に陥った。");
#else
        mpr("You feel momentarily disoriented.");
#endif
        if (!wearing_amulet(AMU_CLARITY))
            forget_map(random2avg(100, 2));
        break;

    case TRAP_BLADE:
        if (trap_known && one_chance_in(3))
#ifdef JP
            mpr("あなたは刃の罠をやり過ごした。");
#else
            mpr("You avoid triggering a blade trap.");
#endif
        else if (random2limit(player_evasion(), 40)
                        + (random2(you.dex) / 3) + (trap_known ? 3 : 0) > 8)
        {
#ifdef JP
            mpr("巨大な刃があなたの通り過ぎた後を薙ぎ払った！");
#else
            mpr("A huge blade swings just past you!");
#endif
        }
        else
        {
#ifdef JP
            mpr("巨大な刃が振りかかり、あなたに突き刺さった！");
#else
            mpr("A huge blade swings out and slices into you!");
#endif
            ouch( (you.your_level * 2) + random2avg(29, 2)
#ifdef JP
                    - random2(1 + player_AC()), 0, KILLED_BY_TRAP, "刃" );
#else
                    - random2(1 + player_AC()), 0, KILLED_BY_TRAP, " blade" );
#endif
        }
        break;

    case TRAP_ZOT:
    default:
#ifdef JP
        mpr((trap_known) ? "あなたはゾットの罠に踏み込んだ。"
                         : "まずい！あなたはゾットの罠に踏み込んでしまった！");
#else
        mpr((trap_known) ? "You enter the Zot trap."
                         : "Oh no! You have blundered into a Zot trap!");
#endif
        miscast_effect( SPTYP_RANDOM, random2(30) + you.your_level,
#ifdef JP
                        75 + random2(100), 3, "ゾットの罠" );
#else
                        75 + random2(100), 3, "a Zot trap" );
#endif
        break;
    }
}                               // end handle_traps()

void disarm_trap( struct dist &disa )
{
    if (you.berserker)
    {
        canned_msg(MSG_TOO_BERSERK);
        return;
    }

    int i, j;

    for (i = 0; i < MAX_TRAPS; i++)
    {
        if (env.trap[i].x == you.x_pos + disa.dx
            && env.trap[i].y == you.y_pos + disa.dy)
        {
            break;
        }

        if (i == MAX_TRAPS - 1)
        {
#ifdef JP
            mpr("Error - couldn't find that trap.");
#else
            mpr("Error - couldn't find that trap.");
#endif
            return;
        }
    }

    if (trap_category(env.trap[i].type) == DNGN_TRAP_MAGICAL)
    {
#ifdef JP
        mpr("あなたはその罠を解除することはできない。");
#else
        mpr("You can't disarm that trap.");
#endif
        return;
    }

    if (random2(you.skills[SK_TRAPS_DOORS] + 2) <= random2(you.your_level + 5))
    {
#ifdef JP
        mpr("あなたは罠の解除に失敗した。");
#else
        mpr("You failed to disarm the trap.");
#endif

        you.turn_is_over = 1;

        if (random2(you.dex) > 5 + random2(5 + you.your_level))
            exercise(SK_TRAPS_DOORS, 1 + random2(you.your_level / 5));
        else
        {
            handle_traps(env.trap[i].type, i, false);

            if (coinflip())
                exercise(SK_TRAPS_DOORS, 1);
        }

        return;
    }

#ifdef JP
    mpr("あなたは罠を解除した。");
#else
    mpr("You have disarmed the trap.");
#endif

    struct bolt beam;

    beam.target_x = you.x_pos + disa.dx;
    beam.target_y = you.y_pos + disa.dy;

    if (env.trap[i].type != TRAP_BLADE
        && trap_category(env.trap[i].type) == DNGN_TRAP_MECHANICAL)
    {
        for (j = 0; j < 20; j++)
        {
            // places items (eg darts), which will automatically stack
            itrap(beam, i);

            if (j > 10 && one_chance_in(3))
                break;
        }
    }

    grd[you.x_pos + disa.dx][you.y_pos + disa.dy] = DNGN_FLOOR;
    env.trap[i].type = TRAP_UNASSIGNED;
    you.turn_is_over = 1;

    // reduced from 5 + random2(5)
    exercise(SK_TRAPS_DOORS, 1 + random2(5) + (you.your_level / 5));
}                               // end disarm_trap()

void manage_clouds(void)
{
    // amount which cloud dissipates - must be unsigned! {dlb}
    unsigned int dissipate = 0;

    for (unsigned char cc = 0; cc < MAX_CLOUDS; cc++)
    {
        if (env.cloud[cc].type == CLOUD_NONE)   // no cloud -> next iteration
            continue;

        dissipate = you.time_taken;

        // water -> flaming clouds:
        // lava -> freezing clouds:
        if ((env.cloud[cc].type == CLOUD_FIRE
                || env.cloud[cc].type == CLOUD_FIRE_MON)
            && grd[env.cloud[cc].x][env.cloud[cc].y] == DNGN_DEEP_WATER)
        {
            dissipate *= 4;
        }
        else if ((env.cloud[cc].type == CLOUD_COLD
                    || env.cloud[cc].type == CLOUD_COLD_MON)
                && grd[env.cloud[cc].x][env.cloud[cc].y] == DNGN_LAVA)
        {
            dissipate *= 4;
        }

        // double the amount when slowed - must be applied last(!):
        if (you.slow)
            dissipate *= 2;

        // apply calculated rate to the actual cloud:
        env.cloud[cc].decay -= dissipate;

        // check for total dissipatation and handle accordingly:
        if (env.cloud[cc].decay < 1)
            delete_cloud( cc );
    }

    return;
}                               // end manage_clouds()

void weird_writing(char stringy[40])
{
    int temp_rand = 0;          // for probability determinations {dlb}

    temp_rand = random2(15);

    // you'll see why later on {dlb}
#ifdef JP
    strcpy(stringy, (temp_rand == 0) ? "ひん曲がった" :
                    (temp_rand == 1) ? "くっきりした" :
                    (temp_rand == 2) ? "かすれた" :
                    (temp_rand == 3) ? "くねくねした" :
                    (temp_rand == 4) ? "むらのある" :
                    (temp_rand == 5) ? "角張った" :
                    (temp_rand == 6) ? "揺らめく" :
                    (temp_rand == 7) ? "輝く" : "");
#else
    strcpy(stringy, (temp_rand == 0) ? "writhing" :
                    (temp_rand == 1) ? "bold" :
                    (temp_rand == 2) ? "faint" :
                    (temp_rand == 3) ? "spidery" :
                    (temp_rand == 4) ? "blocky" :
                    (temp_rand == 5) ? "angular" :
                    (temp_rand == 6) ? "shimmering" :
                    (temp_rand == 7) ? "glowing" : "");
#endif

    if (temp_rand < 8)
        strcat(stringy, "");   // see above for reasoning {dlb}

    temp_rand = random2(14);

#ifdef JP
    strcat(stringy, (temp_rand ==  0) ? "黄色い" :
                    (temp_rand ==  1) ? "茶色い" :
                    (temp_rand ==  2) ? "黒い" :
                    (temp_rand ==  3) ? "紫の" :
                    (temp_rand ==  4) ? "オレンジの" :
                    (temp_rand ==  5) ? "黄緑の" :
                    (temp_rand ==  6) ? "青い" :
                    (temp_rand ==  7) ? "灰色の" :
                    (temp_rand ==  8) ? "銀色の" :
                    (temp_rand ==  9) ? "金色の" :
                    (temp_rand == 10) ? "琥珀色の" :
                    (temp_rand == 11) ? "炭色の" :
                    (temp_rand == 12) ? "淡い" :
                    (temp_rand == 13) ? "藤色の"
                                      : "無色の");
#else
    strcat(stringy, (temp_rand ==  0) ? "yellow" :
                    (temp_rand ==  1) ? "brown" :
                    (temp_rand ==  2) ? "black" :
                    (temp_rand ==  3) ? "purple" :
                    (temp_rand ==  4) ? "orange" :
                    (temp_rand ==  5) ? "lime-green" :
                    (temp_rand ==  6) ? "blue" :
                    (temp_rand ==  7) ? "grey" :
                    (temp_rand ==  8) ? "silver" :
                    (temp_rand ==  9) ? "gold" :
                    (temp_rand == 10) ? "umber" :
                    (temp_rand == 11) ? "charcoal" :
                    (temp_rand == 12) ? "pastel" :
                    (temp_rand == 13) ? "mauve"
                                      : "colourless");
#endif

    strcat(stringy, "");

    temp_rand = random2(14);

#ifdef JP
    strcat(stringy, (temp_rand == 0) ? "書き置き" :
                    (temp_rand == 1) ? "走り書き" :
                    (temp_rand == 2) ? "印章" :
                    (temp_rand == 3) ? "ルーン文字" :
                    (temp_rand == 4) ? "象形文字" :
                    (temp_rand == 5) ? "走り書き" :
                    (temp_rand == 6) ? "活字" :
                    (temp_rand == 7) ? "二進符合" :
                    (temp_rand == 8) ? "絵文字" :
                    (temp_rand == 9) ? "記号"
                                     : "文");
#else
    strcat(stringy, (temp_rand == 0) ? "writing" :
                    (temp_rand == 1) ? "scrawl" :
                    (temp_rand == 2) ? "sigils" :
                    (temp_rand == 3) ? "runes" :
                    (temp_rand == 4) ? "hieroglyphics" :
                    (temp_rand == 5) ? "scrawl" :
                    (temp_rand == 6) ? "print-out" :
                    (temp_rand == 7) ? "binary code" :
                    (temp_rand == 8) ? "glyphs" :
                    (temp_rand == 9) ? "symbols"
                                     : "text");
#endif

    return;
}                               // end weird_writing()

// must be a better name than 'place' for the first parameter {dlb}
void fall_into_a_pool(bool place, unsigned char terrain)
{
    bool escape = false;
    FixedVector< char, 2 > empty;

    if (you.species == SP_MERFOLK && terrain == DNGN_DEEP_WATER)
    {
        // These can happen when we enter deep water directly -- bwr
        merfolk_start_swimming();
        return;
    }

#ifdef JP
    strcpy(info, "あなたは");
#else
    strcpy(info, "You fall into the ");
#endif

#ifdef JP
    strcat(info, (terrain == DNGN_LAVA)       ? "溶岩" :
                 (terrain == DNGN_DEEP_WATER) ? "水"
                                              : "プログラムのヒビ");
#else
    strcat(info, (terrain == DNGN_LAVA)       ? "lava" :
                 (terrain == DNGN_DEEP_WATER) ? "water"
                                              : "programming rift");
#endif

#ifdef JP
    strcat(info, "の中に落ちた！");
#else
    strcat(info, "!");
#endif
    mpr(info);

    more();
    mesclr();

    if (terrain == DNGN_LAVA)
    {
        const int resist = player_res_fire();

        if (resist <= 0)
        {
#ifdef JP
            mpr( "溶岩があなたを黒焦げに焼き尽くした！" );
#else
            mpr( "The lava burns you to a cinder!" );
#endif
            ouch( -9999, 0, KILLED_BY_LAVA );
        }
        else
        {
            // should boost # of bangs per damage in the future {dlb}
#ifdef JP
            mpr( "溶岩があなたを火傷させた！" );
#else
            mpr( "The lava burns you!" );
#endif
            ouch( (10 + random2avg(100, 2)) / resist, 0, KILLED_BY_LAVA );
        }

        if (you.duration[DUR_CONDENSATION_SHIELD] > 0)
        {
#ifdef JP
            mpr("あなたの氷の盾は砕け散ってしまった！", MSGCH_DURATION);
#else
            mpr("Your icy shield dissipates!", MSGCH_DURATION);
#endif
            you.duration[DUR_CONDENSATION_SHIELD] = 0;
            you.redraw_armour_class = 1;
        }
    }

    // a distinction between stepping and falling from you.levitation
    // prevents stepping into a thin stream of lava to get to the other side.
    if (scramble())
    {
        if (place)
        {
            if (empty_surrounds(you.x_pos, you.y_pos, DNGN_FLOOR, false, empty))
            {
                you.x_pos = empty[0];
                you.y_pos = empty[1];
                escape = true;
            }
            else
                escape = false;
        }
        else
            escape = true;
    }
    else
    {
        // that is, don't display following when fall from levitating
        if (!place)
#ifdef JP
            mpr("あなたは脱け出ようとしたが、荷物に引きずり降ろされた！");
#else
            mpr("You try to escape, but your burden drags you down!");
#endif
    }

    if (escape)
    {
#ifdef JP
        mpr("あなたは這い上がることに成功した！");
#else
        mpr("You manage to scramble free!");
#endif

        if (terrain == DNGN_LAVA)
            scrolls_burn(10, OBJ_SCROLLS);

        return;
    }

#ifdef JP
    mpr("あなたは沈んでいった……。");
#else
    mpr("You drown...");
#endif

    if (terrain == DNGN_LAVA)
        ouch(-9999, 0, KILLED_BY_LAVA);
    else if (terrain == DNGN_DEEP_WATER)
        ouch(-9999, 0, KILLED_BY_WATER);

    // Okay, so you don't trigger a trap when you scramble onto it.
    //I really can't be bothered right now.
}                               // end fall_into_a_pool()

bool scramble(void)
{
    int max_carry = carrying_capacity();

    if ((max_carry / 2) + random2(max_carry / 2) <= you.burden)
        return false;
    else
        return true;
}                               // end scramble()

void weird_colours(unsigned char coll, char wc[30])
{
    unsigned char coll_div16 = coll / 16; // conceivable max is then 16 {dlb}

    // Must start with a consonant!
#ifdef JP
    strcpy(wc, (coll_div16 == 0 || coll_div16 ==  7) ? "目もあやな" :
               (coll_div16 == 1 || coll_div16 ==  8) ? "淡い" :
               (coll_div16 == 2 || coll_div16 ==  9) ? "まだら模様の" :
               (coll_div16 == 3 || coll_div16 == 10) ? "仄かな" :
               (coll_div16 == 4 || coll_div16 == 11) ? "まぶしい" :
               (coll_div16 == 5 || coll_div16 == 12) ? "不明瞭な" :
               (coll_div16 == 6 || coll_div16 == 13) ? "きらめく"
                                                     : "幽かな");
#else
    strcpy(wc, (coll_div16 == 0 || coll_div16 ==  7) ? "brilliant" :
               (coll_div16 == 1 || coll_div16 ==  8) ? "pale" :
               (coll_div16 == 2 || coll_div16 ==  9) ? "mottled" :
               (coll_div16 == 3 || coll_div16 == 10) ? "shimmering" :
               (coll_div16 == 4 || coll_div16 == 11) ? "bright" :
               (coll_div16 == 5 || coll_div16 == 12) ? "dark" :
               (coll_div16 == 6 || coll_div16 == 13) ? "shining"
                                                     : "faint");
    strcat(wc, " ");
#endif

    while (coll > 17)
        coll -= 10;

#ifdef JP
    strcat(wc, (coll ==  0) ? "赤" :
               (coll ==  1) ? "紫" :
               (coll ==  2) ? "緑" :
               (coll ==  3) ? "オレンジ" :
               (coll ==  4) ? "深紅" :
               (coll ==  5) ? "黒" :
               (coll ==  6) ? "灰色" :
               (coll ==  7) ? "銀色" :
               (coll ==  8) ? "金色" :
               (coll ==  9) ? "ピンク" :
               (coll == 10) ? "黄色" :
               (coll == 11) ? "白" :
               (coll == 12) ? "茶色" :
               (coll == 13) ? "紺色" :
               (coll == 14) ? "黄土色" :
               (coll == 15) ? "黄緑" :
               (coll == 16) ? "藤色" :
               (coll == 17) ? "空色"
                            : "無色");
#else
    strcat(wc, (coll ==  0) ? "red" :
               (coll ==  1) ? "purple" :
               (coll ==  2) ? "green" :
               (coll ==  3) ? "orange" :
               (coll ==  4) ? "magenta" :
               (coll ==  5) ? "black" :
               (coll ==  6) ? "grey" :
               (coll ==  7) ? "silver" :
               (coll ==  8) ? "gold" :
               (coll ==  9) ? "pink" :
               (coll == 10) ? "yellow" :
               (coll == 11) ? "white" :
               (coll == 12) ? "brown" :
               (coll == 13) ? "aubergine" :
               (coll == 14) ? "ochre" :
               (coll == 15) ? "leaf green" :
               (coll == 16) ? "mauve" :
               (coll == 17) ? "azure"
                            : "colourless");
#endif

    return;
}                               // end weird_colours()

bool go_berserk(bool intentional)
{
    if (you.berserker)
    {
        if (intentional)
#ifdef JP
            mpr("あなたはすでにバーサークしている！");
#else
            mpr("You're already berserk!");
#endif
        // or else you won't notice -- no message here.
        return false;
    }

    if (you.exhausted)
    {
        if (intentional)
#ifdef JP
            mpr("あなたはバーサークするには消耗しすぎている。");
#else
            mpr("You're too exhausted to go berserk.");
#endif
        // or else they won't notice -- no message here
        return false;
    }

    if (you.is_undead)
    {
        if (intentional)
#ifdef JP
            mpr("あなたは生命のない肉体に血の怒りをたぎらせることはできない。");
#else
            mpr("You cannot raise a blood rage in your lifeless body.");
#endif
        // or else you won't notice -- no message here
        return false;
    }

#ifdef JP
    //加速メッセージが重複するので変更。
    mpr("狂暴化によってあなたの視界は真っ赤に染まった！");
    mpr("あなたは狂戦士の力を呼び覚ました！");
#else
    mpr("A red film seems to cover your vision as you go berserk!");
    mpr("You feel yourself moving faster!");
    mpr("You feel mighty!");
#endif

    you.berserker += 20 + random2avg(19, 2);

    calc_hp();
    you.hp *= 15;
    you.hp /= 10;

    deflate_hp(you.hp_max, false);

    if (!you.might)
        modify_stat( STAT_STRENGTH, 5, true );

    you.might += you.berserker;
    haste_player( you.berserker );

    if (you.berserk_penalty != NO_BERSERK_PENALTY)
        you.berserk_penalty = 0;

    return true;
}                               // end go_berserk()

bool trap_item(char base_type, char sub_type, char beam_x, char beam_y)
{
    item_def  item;

    item.base_type = base_type;
    item.sub_type = sub_type;
    item.plus = 0;
    item.plus2 = 0;
    item.flags = 0;
    item.special = 0;
    item.quantity = 1;
    item.colour = LIGHTCYAN;

    if (base_type == OBJ_MISSILES)
    {
        if (sub_type == MI_NEEDLE)
        {
            set_item_ego_type( item, OBJ_MISSILES, SPMSL_POISONED );
            item.colour = WHITE;
        }
        else
        {
            set_item_ego_type( item, OBJ_MISSILES, SPMSL_NORMAL );
        }
    }
    else
    {
        set_item_ego_type( item, OBJ_WEAPONS, SPWPN_NORMAL );
    }

    if (igrd[beam_x][beam_y] != NON_ITEM)
    {
        if (items_stack( item, mitm[ igrd[beam_x][beam_y] ] ))
        {
            inc_mitm_item_quantity( igrd[beam_x][beam_y], 1 );
            return (false);
        }

        // don't want to go overboard here. Will only generate up to three
        // separate trap items, or less if there are other items present.
        if (mitm[ igrd[beam_x][beam_y] ].link != NON_ITEM)
        {
            if (mitm[ mitm[ igrd[beam_x][beam_y] ].link ].link != NON_ITEM)
                return (false);
        }
    }                           // end of if igrd != NON_ITEM

    return (!copy_item_to_grid( item, beam_x, beam_y, 1 ));
}                               // end trap_item()

// returns appropriate trap symbol for a given trap type {dlb}
unsigned char trap_category(unsigned char trap_type)
{
    switch (trap_type)
    {
    case TRAP_TELEPORT:
    case TRAP_AMNESIA:
    case TRAP_ZOT:
        return (DNGN_TRAP_MAGICAL);

    case TRAP_DART:
    case TRAP_ARROW:
    case TRAP_SPEAR:
    case TRAP_AXE:
    case TRAP_BLADE:
    case TRAP_BOLT:
    case TRAP_NEEDLE:
    default:                    // what *would* be the default? {dlb}
        return (DNGN_TRAP_MECHANICAL);
    }
}                               // end trap_category()

// returns index of the trap for a given (x,y) coordinate pair {dlb}
int trap_at_xy(int which_x, int which_y)
{

    for (int which_trap = 0; which_trap < MAX_TRAPS; which_trap++)
    {
        if (env.trap[which_trap].x == which_x
            && env.trap[which_trap].y == which_y)
        {
            if (env.trap[which_trap].type == TRAP_UNASSIGNED) continue;
            return (which_trap);
        }
    }

    // no idea how well this will be handled elsewhere: {dlb}
    return (-1);
}                               // end trap_at_xy()
