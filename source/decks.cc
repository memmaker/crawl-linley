/*
 *  File:       decks.cc
 *  Summary:    Functions with decks of cards.
 *
 *  Change History (most recent first):
 *
 *               <1>     -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "decks.h"

#include <string.h>

#include "externs.h"

#include "effects.h"
#include "food.h"
#include "it_use2.h"
#include "items.h"
#include "misc.h"
#include "monplace.h"
#include "mutation.h"
#include "ouch.h"
#include "player.h"
#include "religion.h"
#include "spells1.h"
#include "spells3.h"
#include "spl-cast.h"
#include "stuff.h"

// array sizes -- see notes below {dlb}
#define DECK_WONDERS_SIZE 27
#define DECK_SUMMONING_SIZE 11
#define DECK_TRICKS_SIZE 11
#define DECK_POWER_SIZE 17
#define DECK_PUNISHMENT_SIZE 23

enum CARDS                      // (unsigned char) deck_of_foo[]
{
    CARD_BLANK = 0,             //    0
    CARD_BUTTERFLY,
    CARD_WRAITH,
    CARD_EXPERIENCE,
    CARD_WEALTH,
    CARD_INTELLIGENCE,          //    5
    CARD_STRENGTH,
    CARD_QUICKSILVER,
    CARD_STUPIDITY,
    CARD_WEAKNESS,
    CARD_SLOTH,                 //   10
    CARD_SHUFFLE,
    CARD_FREAK,
    CARD_DEATH,
    CARD_NORMALITY,
    CARD_SHADOW,                //   15
    CARD_GATE,
    CARD_STATUE,
    CARD_ACQUISITION,
    CARD_HASTEN,
    CARD_DEMON_LESSER,          //   20
    CARD_DEMON_COMMON,
    CARD_DEMON_GREATER,
    CARD_DEMON_SWARM,
    CARD_YAK,
    CARD_FIEND,                 //   25
    CARD_DRAGON,
    CARD_GOLEM,
    CARD_THING_FUGLY,
    CARD_LICH,
    CARD_HORROR_UNSEEN,         //   30
    CARD_BLINK,
    CARD_TELEPORT,
    CARD_TELEPORT_NOW,
    CARD_RAGE,
    CARD_LEVITY,                //   35
    CARD_VENOM,
    CARD_XOM,
    CARD_SLOW,
    CARD_DECAY,
    CARD_HEALING,               //   40
    CARD_HEAL_WOUNDS,
    CARD_TORMENT,
    CARD_FOUNTAIN,
    CARD_ALTAR,
    CARD_FAMINE,                //   45
    CARD_FEAST,
    CARD_WILD_MAGIC,
    CARD_VIOLENCE,
    CARD_PROTECTION,
    CARD_KNOWLEDGE,             //   50
    CARD_MAZE,
    CARD_PANDEMONIUM,
    CARD_IMPRISONMENT,
    CARD_RULES_FOR_BRIDGE,      //   54
    NUM_CARDS,                  // must remain last regular member {dlb}
    CARD_RANDOM = 255           // must remain final member {dlb}
};

static unsigned char deck_of_wonders[] =
{
    CARD_BLANK,
    CARD_BUTTERFLY,
    CARD_WRAITH,
    CARD_EXPERIENCE,
    CARD_WEALTH,
    CARD_INTELLIGENCE,
    CARD_STRENGTH,
    CARD_QUICKSILVER,
    CARD_STUPIDITY,
    CARD_WEAKNESS,
    CARD_SLOTH,
    CARD_SHUFFLE,
    CARD_FREAK,
    CARD_DEATH,
    CARD_NORMALITY,
    CARD_SHADOW,
    CARD_GATE,
    CARD_STATUE,
    CARD_ACQUISITION,
    CARD_HASTEN,
    CARD_LICH,
    CARD_XOM,
    CARD_DECAY,
    CARD_ALTAR,
    CARD_FOUNTAIN,
    CARD_MAZE,
    CARD_PANDEMONIUM
};

static unsigned char deck_of_summoning[] =
{
    CARD_STATUE,
    CARD_DEMON_LESSER,
    CARD_DEMON_COMMON,
    CARD_DEMON_GREATER,
    CARD_DEMON_SWARM,
    CARD_YAK,
    CARD_FIEND,
    CARD_DRAGON,
    CARD_GOLEM,
    CARD_THING_FUGLY,
    CARD_HORROR_UNSEEN
};

static unsigned char deck_of_tricks[] =
{
    CARD_BLANK,
    CARD_BUTTERFLY,
    CARD_BLINK,
    CARD_TELEPORT,
    CARD_TELEPORT_NOW,
    CARD_RAGE,
    CARD_LEVITY,
    CARD_HEALING,
    CARD_WILD_MAGIC,
    CARD_DEMON_LESSER,
    CARD_HASTEN
};

static unsigned char deck_of_power[] =
{
    CARD_BLANK,
    CARD_DEMON_COMMON,
    CARD_DEMON_GREATER,
    CARD_TELEPORT_NOW,
    CARD_VENOM,
    CARD_XOM,
    CARD_HEAL_WOUNDS,
    CARD_FAMINE,
    CARD_FEAST,
    CARD_WILD_MAGIC,
    CARD_VIOLENCE,
    CARD_PROTECTION,
    CARD_KNOWLEDGE,
    CARD_HASTEN,
    CARD_TORMENT,
    CARD_DEMON_SWARM,
    CARD_SLOW
};

// Supposed to be bad, small chance of OK... Nemelex wouldn't like a game
// that didn't have some chance of "losing".
static unsigned char deck_of_punishment[] =
{
    CARD_BLANK,
    CARD_BUTTERFLY,
    CARD_WRAITH,
    CARD_WEALTH,
    CARD_STUPIDITY,
    CARD_WEAKNESS,
    CARD_SLOTH,
    CARD_SHUFFLE,
    CARD_FREAK,
    CARD_DEATH,
    CARD_NORMALITY,
    CARD_SHADOW,
    CARD_GATE,
    CARD_DEMON_SWARM,
    CARD_RAGE,
    CARD_VENOM,
    CARD_SLOW,
    CARD_DECAY,
    CARD_TORMENT,
    CARD_FAMINE,
    CARD_WILD_MAGIC,
    CARD_MAZE,
    CARD_PANDEMONIUM
};

static void cards(unsigned char which_card);

void deck_of_cards(unsigned char which_deck)
{

    // I really am not fond of how all of this works, the
    // decks ought to be stored (possibly) in an array of
    // pointers to int or as discrete arrays of int using
    // the sizeof operator to determine upper bounds, and
    // not defines, which is a bit clumsy given that you
    // have to update two things presently (the array and
    // the corresponding define) in order to add things to
    // decks ... someone fix this, or I will {dlb}
    unsigned char *card = deck_of_wonders;
    unsigned char max_card = 0;
    int i = 0;
    int brownie_points = 0;     // for passing to done_good() {dlb}

#ifdef JP
    mpr("あなたはカードを引いた……。");
#else
    mpr("You draw a card...");
#endif

    switch (which_deck)
    {
    case DECK_OF_WONDERS:
        card = deck_of_wonders;
        max_card = DECK_WONDERS_SIZE;
        break;
    case DECK_OF_SUMMONING:
        card = deck_of_summoning;
        max_card = DECK_SUMMONING_SIZE;
        break;
    case DECK_OF_TRICKS:
        card = deck_of_tricks;
        max_card = DECK_TRICKS_SIZE;
        break;
    case DECK_OF_POWER:
        card = deck_of_power;
        max_card = DECK_POWER_SIZE;
        break;
    case DECK_OF_PUNISHMENT:
        card = deck_of_punishment;
        max_card = DECK_PUNISHMENT_SIZE;
        break;
    }

    i = (int) card[random2(max_card)];

    if (one_chance_in(250))
    {
#ifdef JP
        mpr("このカードはこのデッキの物ではないようだ。");
#else
        mpr("This card doesn't seem to belong here.");
#endif
        i = random2(NUM_CARDS);
    }

    if (i == CARD_BLANK && you.skills[SK_EVOCATIONS] > random2(30))
        i = (int) card[random2(max_card)];

    cards(i);

    // Decks of punishment aren't objects in the game,
    // its just Nemelex's form of punishment -- bwr
    if (which_deck != DECK_OF_PUNISHMENT)
    {
        you.inv[you.equip[EQ_WEAPON]].plus--;

        if (you.inv[you.equip[EQ_WEAPON]].plus == 0)
        {
#ifdef JP
            mpr("カードのデッキはひと吹きの煙となって消え去った。");
#else
            mpr("The deck of cards disappears in a puff of smoke.");
#endif

            unwield_item(you.equip[EQ_WEAPON]);

            dec_inv_item_quantity( you.equip[EQ_WEAPON], 1 );

#ifdef USE_TILE
            if (Options.use_tile)
                TilePlayerRefresh();
#endif

            // these bonuses happen only when the deck expires {dlb}:
            brownie_points = (coinflip()? 2 : 1);

            if (which_deck == DECK_OF_WONDERS)
                brownie_points += 2;
            else if (which_deck == DECK_OF_POWER)
                brownie_points++;
        }

        // this bonus happens with every use {dlb}:
        if (which_deck == DECK_OF_WONDERS || one_chance_in(3))
            brownie_points++;

        done_good(GOOD_CARDS, brownie_points);
    }

    return;
}                               // end deck_of_cards()

static void cards(unsigned char which_card)
{
    FixedVector < int, 5 > dvar;
    FixedVector < int, 5 > mvar;
    int dvar1 = 0;
    int loopy = 0;              // general purpose loop variable {dlb}
    bool success = false;       // for summoning messages {dlb}
    bool failMsg = true;
    int summ_dur, summ_beh, summ_num;

    if (which_card == CARD_BLANK && one_chance_in(10))
        which_card = CARD_RULES_FOR_BRIDGE;

    switch (which_card)
    {
    default:
    case CARD_BLANK:
#ifdef JP
        mpr("白紙のカードだ。");
#else
        mpr("It is blank.");
#endif
        break;

    case CARD_BUTTERFLY:
#ifdef JP
        mpr("あなたは蝶々のカードを引いた。");
#else
        mpr("You have drawn the Butterfly.");
#endif

        summ_dur = ENCH_ABJ_I + random2(3) + you.skills[SK_EVOCATIONS] / 2;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( MONS_BUTTERFLY, summ_dur, BEH_FRIENDLY,
                            you.x_pos, you.y_pos, MHITYOU, 250 ) != -1)
        {
#ifdef JP
            mpr("極彩色の蝶々がカードから飛び出した！");
#else
            mpr("A brightly coloured insect flies from the card!");
#endif
        }
        break;

    case CARD_WRAITH:
#ifdef JP
        mpr("あなたは亡霊のカードを引いた。");
#else
        mpr("You have drawn the Wraith.");
#endif

        lose_level();
        drain_exp();
        break;

    case CARD_EXPERIENCE:
#ifdef JP
        mpr( "あなたは経験のカードを引いた。" );
#else
        mpr( "You have drawn Experience." );
#endif
        potion_effect( POT_EXPERIENCE, 0 );
        break;

    case CARD_WEALTH:
#ifdef JP
        mpr("あなたは富のカードを引いた。");
#else
        mpr("You have drawn Wealth.");
#endif

        you.gold += roll_dice( 2, 20 * you.skills[SK_EVOCATIONS] );
        you.redraw_gold = 1;
        break;

    case CARD_INTELLIGENCE:
#ifdef JP
        mpr("あなたは頭脳のカードを引いた！");
#else
        mpr("You have drawn the Brain!");
#endif

        you.intel += 1 + random2( you.skills[SK_EVOCATIONS] ) / 7;

        if (you.max_intel < you.intel)
            you.max_intel = you.intel;

        you.redraw_intelligence = 1;
        break;

    case CARD_STRENGTH:
#ifdef JP
        mpr("あなたは膂力のカードを引いた！");
#else
        mpr("You have drawn Strength!");
#endif

        you.strength += 1 + random2( you.skills[SK_EVOCATIONS] ) / 7;

        if (you.max_strength < you.strength)
            you.max_strength = you.strength;

        you.redraw_strength = 1;
        break;

    case CARD_QUICKSILVER:
#ifdef JP
        mpr("あなたは機敏のカードを引いた。");
#else
        mpr("You have drawn the Quicksilver card.");
#endif

        you.dex += 1 + random2( you.skills[SK_EVOCATIONS] ) / 7;

        if (you.max_dex < you.dex)
            you.max_dex = you.dex;

        you.redraw_dexterity = 1;
        break;

    case CARD_STUPIDITY:
#ifdef JP
        mpr("あなたは愚昧のカードを引いた！");
#else
        mpr("You have drawn Stupidity!");
#endif

        you.intel -= (2 + random2avg(3, 2));
        if (you.intel < 4)
            you.intel = 0;

        if (you.skills[SK_EVOCATIONS] < random2(30))
            you.max_intel--;

        you.redraw_intelligence = 1;
        break;

    case CARD_WEAKNESS:
#ifdef JP
        mpr("あなたはひ弱のカードを引いた。");
#else
        mpr("You have drawn Weakness.");
#endif

        you.strength -= (2 + random2avg(3, 2));
        if (you.strength < 4)
            you.strength = 0;

        if (you.skills[SK_EVOCATIONS] < random2(30))
            you.max_strength--;

        you.redraw_strength = 1;
        break;

    case CARD_SLOTH:
#ifdef JP
        mpr("あなたは鈍間のカードを引いた。");
#else
        mpr("You have drawn the Slug.");
#endif

        you.dex -= (2 + random2avg(3, 2));
        if (you.dex < 4)
            you.dex = 0;

        if (you.skills[SK_EVOCATIONS] < random2(30))
            you.max_dex--;

        you.redraw_dexterity = 1;
        break;

    case CARD_SHUFFLE:          // shuffle stats
#ifdef JP
        mpr("あなたはシャッフルのカードを引いた！");
#else
        mpr("You have drawn the Shuffle card!");
#endif

        dvar[STAT_STRENGTH] = you.strength;
        dvar[STAT_DEXTERITY] = you.dex;
        dvar[STAT_INTELLIGENCE] = you.intel;

        mvar[STAT_STRENGTH] = you.max_strength;
        mvar[STAT_DEXTERITY] = you.max_dex;
        mvar[STAT_INTELLIGENCE] = you.max_intel;

        you.strength = 101;
        you.intel = 101;
        you.dex = 101;

        do
        {
            dvar1 = random2(NUM_STATS);

            if (dvar[dvar1] == 101)
                continue;

            if (you.strength == 101)
            {
                you.strength = dvar[dvar1];
                you.max_strength = mvar[dvar1];
            }
            else if (you.intel == 101)
            {
                you.intel = dvar[dvar1];
                you.max_intel = mvar[dvar1];
            }
            else if (you.dex == 101)
            {
                you.dex = dvar[dvar1];
                you.max_dex = mvar[dvar1];
            }

            dvar[dvar1] = 101;
        }
        while (dvar[STAT_STRENGTH] != 101 || dvar[STAT_DEXTERITY] != 101
                                           || dvar[STAT_INTELLIGENCE] != 101);

        you.redraw_strength = 1;
        you.redraw_intelligence = 1;
        you.redraw_dexterity = 1;
        burden_change();
        break;

    case CARD_FREAK:
#ifdef JP
        mpr("あなたは奇形のカードを引いた！");
#else
        mpr("You have drawn the Freak!");
#endif
        for (loopy = 0; loopy < 6; loopy++)
        {
            if (!mutate(100, failMsg))
                failMsg = false;
        }
        break;

    case CARD_DEATH:
#ifdef JP
        mpr("あなたは死神のカードを引いた！");
#else
        mpr("Oh no! You have drawn the Death card!");
#endif

        if (you.duration[DUR_TELEPORT])
            you_teleport();

        for (loopy = 0; loopy < 5; loopy++)
        {
            create_monster( MONS_REAPER, 0, BEH_HOSTILE, you.x_pos, you.y_pos,
                            MHITYOU, 250 );
        }
        break;

    case CARD_NORMALITY:
#ifdef JP
        mpr("あなたは凡庸のカードを引いた。");
#else
        mpr("You have drawn Normalisation.");
#endif
        for (loopy = 0; loopy < 6; loopy++)
        {
            delete_mutation(100);
        }
        break;

    case CARD_SHADOW:
#ifdef JP
        mpr("あなたは影のカードを引いた。");
#else
        mpr("You have drawn the Shadow.");
#endif
        create_monster( MONS_SOUL_EATER, 0, BEH_HOSTILE, you.x_pos, you.y_pos,
                        MHITYOU, 250 );
        break;

    case CARD_GATE:
#ifdef JP
        mpr("あなたは門のカードを引いた！");
#else
        mpr("You have drawn the Gate!");
#endif
        more();

        if (you.level_type == LEVEL_ABYSS)
            banished(DNGN_EXIT_ABYSS);
        else if (you.level_type == LEVEL_LABYRINTH)
            canned_msg(MSG_NOTHING_HAPPENS);
        else
            banished(DNGN_ENTER_ABYSS);
        break;

    case CARD_STATUE:
#ifdef JP
        mpr("あなたは水晶の像のカードを引いた！");
#else
        mpr("You have drawn the Crystal Statue.");
#endif
        create_monster( MONS_CRYSTAL_GOLEM, 0, BEH_FRIENDLY,
                        you.x_pos, you.y_pos, you.pet_target, 250 );
        break;

    case CARD_ACQUISITION:
#ifdef JP
        mpr( "あなたは獲得のカードを引いた！" );
        mpr( "カードが巻き物に変化した。" );
#else
        mpr( "You have drawn Acquisition!" );
        mpr( "The card unfolds to form a scroll of paper." );
#endif
        acquirement( OBJ_RANDOM );
        break;

    case CARD_HASTEN:
#ifdef JP
        mpr("あなたは加速のカードを引いた。");
#else
        mpr("You have drawn Haste.");
#endif
        potion_effect( POT_SPEED, 5 * you.skills[SK_EVOCATIONS] );
        break;

    case CARD_DEMON_LESSER:
#ifdef JP
        mpr("カードには小さな悪魔の絵が描かれている。");
#else
        mpr("On the card is a picture of a little demon.");
#endif

        summ_dur = ENCH_ABJ_I + random2(3) + you.skills[SK_EVOCATIONS] / 3;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( summon_any_demon( DEMON_LESSER ), summ_dur,
                            BEH_FRIENDLY, you.x_pos, you.y_pos, you.pet_target,
                            250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_DEMON_COMMON:
#ifdef JP
        mpr("カードには悪魔の絵が描かれている。");
#else
        mpr("On the card is a picture of a demon.");
#endif

        summ_dur = ENCH_ABJ_I + random2(3) + you.skills[SK_EVOCATIONS] / 4;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( summon_any_demon( DEMON_COMMON ), summ_dur,
                            BEH_FRIENDLY, you.x_pos, you.y_pos, you.pet_target,
                            250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_DEMON_GREATER:
#ifdef JP
        mpr("カードには巨大な悪魔の絵が描かれている。");
#else
        mpr("On the card is a picture of a huge demon.");
#endif

        summ_beh = (you.skills[SK_EVOCATIONS] > random2(30)) ? BEH_FRIENDLY
                                                             : BEH_CHARMED;

        if (summ_beh == BEH_CHARMED)
#ifdef JP
            mpr( "この存在にはあまり好感が持てない……。" );
#else
            mpr( "You don't feel so good about this..." );
#endif

        if (create_monster( summon_any_demon( DEMON_GREATER ), ENCH_ABJ_V,
                            summ_beh, you.x_pos, you.y_pos,
                            you.pet_target, 250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_DEMON_SWARM:
#ifdef JP
        mpr("カードには小さな悪魔の群れが描かれている。");
#else
        mpr("On the card is a picture of a swarm of little demons.");
#endif

        success = false;

        summ_num = 7 + random2(6);

        for (loopy = 0; loopy < summ_num; loopy++)
        {
            if (create_monster( summon_any_demon( DEMON_LESSER ), ENCH_ABJ_VI,
                                BEH_HOSTILE, you.x_pos, you.y_pos,
                                MHITYOU, 250 ) != -1)
            {
                 success = true;
            }
        }

        if (success)
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        break;

    case CARD_YAK:
#ifdef JP
        mpr("カードには巨大なヤクが描かれている。");
#else
        mpr("On the card is a picture of a huge shaggy yak.");
#endif

        summ_dur = ENCH_ABJ_II + you.skills[SK_EVOCATIONS] / 2;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( MONS_DEATH_YAK, summ_dur, BEH_FRIENDLY,
                            you.x_pos, you.y_pos, you.pet_target, 250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_FIEND:
#ifdef JP
        mpr("カードには鱗の生えた巨大な悪魔が描かれている。");
#else
        mpr("On the card is a picture of a huge scaly devil.");
#endif

        summ_dur = ENCH_ABJ_II + you.skills[SK_EVOCATIONS] / 6;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( MONS_FIEND, summ_dur, BEH_FRIENDLY,
                            you.x_pos, you.y_pos, you.pet_target, 250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_DRAGON:
#ifdef JP
        mpr("カードには巨大なドラゴンが描かれている。");
#else
        mpr("On the card is a picture of a huge scaly dragon.");
#endif

        summ_dur = ENCH_ABJ_III + you.skills[SK_EVOCATIONS] / 6;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( (coinflip() ? MONS_DRAGON : MONS_ICE_DRAGON),
                            summ_dur, BEH_FRIENDLY, you.x_pos, you.y_pos,
                            you.pet_target, 250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_GOLEM:
#ifdef JP
        mpr("カードには石像の絵が描かれている。");
#else
        mpr("On the card is a picture of a statue.");
#endif

        summ_dur = ENCH_ABJ_II + you.skills[SK_EVOCATIONS] / 4;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( MONS_CLAY_GOLEM + random2(6), summ_dur,
                            BEH_FRIENDLY, you.x_pos, you.y_pos,
                            you.pet_target, 250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_THING_FUGLY:
#ifdef JP
        mpr("カードにはとても醜い何かが描かれている。");
#else
        mpr("On the card is a picture of a very ugly thing.");
#endif

        summ_dur = ENCH_ABJ_II + you.skills[SK_EVOCATIONS] / 4;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( MONS_VERY_UGLY_THING, summ_dur, BEH_FRIENDLY,
                            you.x_pos, you.y_pos, you.pet_target, 250 ) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_LICH:
#ifdef JP
        mpr( "カードには非常に苛立った様子の骸骨が描かれている。" );
#else
        mpr( "On the card is a picture of a very irritated-looking "
             "skeletal thing." );
#endif

        if (create_monster( MONS_LICH, 0, BEH_HOSTILE, you.x_pos, you.y_pos,
                            MHITYOU, 250) != -1)
        {
#ifdef JP
            mpr("絵が本物のモンスターになった！");
#else
            mpr("The picture comes to life!");
#endif
        }
        break;

    case CARD_HORROR_UNSEEN:
        if (!player_see_invis())
#ifdef JP
            mpr("空白のカードだ！");
#else
            mpr("It is blank!");
#endif
        else
#ifdef JP
            mpr("カードにはひどく醜い嫌悪を催させるものが描かれている。");
#else
            mpr("On the card is a picture of a hideous abomination.");
#endif

        summ_dur = ENCH_ABJ_II + you.skills[SK_EVOCATIONS] / 4;
        if (summ_dur > ENCH_ABJ_VI)
            summ_dur = ENCH_ABJ_VI;

        if (create_monster( MONS_UNSEEN_HORROR, summ_dur, BEH_FRIENDLY,
                            you.x_pos, you.y_pos, you.pet_target, 250 ) != -1)
        {
            if (player_see_invis())
            {
#ifdef JP
                mpr("絵が本物のモンスターになった！");
#else
                mpr("The picture comes to life!");
#endif
            }
        }
        break;

    case CARD_BLINK:
#ifdef JP
        mpr("あなたは瞬間移動のカードを引いた。");
#else
        mpr("You have drawn Blink.");
#endif
        blink();
        // random_blink(true);
        break;

    case CARD_TELEPORT:
#ifdef JP
        mpr("あなたはテレポートのカードを引いた。");
#else
        mpr("You have drawn the Portal of Delayed Transposition.");
#endif
        you_teleport();
        break;

    case CARD_TELEPORT_NOW:
#ifdef JP
        mpr( "あなたは即時テレポートのカードを引いた。" );
#else
        mpr( "You have drawn the Portal of Instantaneous Transposition." );
#endif
        you_teleport2( true, true );  // in abyss, always to new area
        break;

    case CARD_RAGE:
#ifdef JP
        mpr("あなたは怒りのカードを引いた。");
#else
        mpr("You have drawn Rage.");
#endif

        if (!go_berserk(false))
            canned_msg(MSG_NOTHING_HAPPENS);
        else
            you.berserk_penalty = NO_BERSERK_PENALTY;
        break;

    case CARD_LEVITY:
#ifdef JP
        mpr("あなたは浮遊のカードを引いた。");
#else
        mpr("You have drawn Levity.");
#endif
        potion_effect( POT_LEVITATION, 5 * you.skills[SK_EVOCATIONS] );
        break;

    case CARD_VENOM:
#ifdef JP
        mpr("あなたは毒牙のカードを引いた。");
#else
        mpr("You have drawn Venom.");
#endif
        poison_player( 2 + random2( 7 - you.skills[SK_EVOCATIONS] / 5 ) );
        break;

    case CARD_XOM:
#ifdef JP
        mpr("あなたはゾムのカードを引いた！");
#else
        mpr("You have drawn the card of Xom!");
#endif
        Xom_acts( true, 5 + random2( you.skills[SK_EVOCATIONS] ), true );
        break;

    case CARD_SLOW:
#ifdef JP
        mpr("あなたは鈍重のカードを引いた。");
#else
        mpr("You have drawn Slowness.");
#endif
        potion_effect( POT_SLOWING, 100 - 2 * you.skills[SK_EVOCATIONS] );
        break;

    case CARD_DECAY:
#ifdef JP
        mpr("あなたは腐敗のカードを引いた。");
#else
        mpr("You have drawn Decay.");
#endif

        if (you.is_undead)
#ifdef JP
            mpr("あなたは恐怖を感じた。");
#else
            mpr("You feel terrible.");
#endif
        else
            rot_player( 2 + random2( 7 - you.skills[SK_EVOCATIONS] / 4 ) );
        break;

    case CARD_HEALING:
#ifdef JP
        mpr("あなたは癒しの霊薬のカードを引いた。");
#else
        mpr("You have drawn the Elixir of Health.");
#endif
        potion_effect( POT_HEALING, 5 * you.skills[SK_EVOCATIONS] );
        break;

    case CARD_HEAL_WOUNDS:
#ifdef JP
        mpr("あなたは即時再生の紋章のカードを引いた。");
#else
        mpr("You have drawn the Symbol of Immediate Regeneration.");
#endif
        potion_effect( POT_HEAL_WOUNDS, 5 * you.skills[SK_EVOCATIONS] );
        break;

    case CARD_TORMENT:
#ifdef JP
        mpr("あなたは苦痛の紋章のカードを引いた。");
#else
        mpr("You have drawn the Symbol of Torment.");
#endif
        torment( you.x_pos, you.y_pos );
        break;

// what about checking whether there are items there, too? {dlb}
    case CARD_FOUNTAIN:
#ifdef JP
        mpr("あなたは泉のカードを引いた。");
#else
        mpr("You have drawn the Fountain.");
#endif

        if (grd[you.x_pos][you.y_pos] == DNGN_FLOOR)
        {
#ifdef JP
            strcpy( info, "澄みきった泉が床から湧き出した。 " );
#else
            strcpy( info, "A beautiful fountain of clear blue water grows "
                          "from the floor " );
#endif

            strcat( info, (you.species == SP_NAGA || you.species == SP_CENTAUR)
#ifdef JP
                                ? "" : "" );
#else
                                ? "before you!" : "at your feet!" );
#endif
            mpr(info);
            grd[you.x_pos][you.y_pos] = DNGN_BLUE_FOUNTAIN;
        }
        else
        {
            canned_msg(MSG_NOTHING_HAPPENS);
        }
        break;

    case CARD_ALTAR:
#ifdef JP
        mpr("あなたは祭壇のカードを引いた。");
#else
        mpr("You have drawn the Altar.");
#endif

        if (you.religion == GOD_NO_GOD)
        {
            canned_msg(MSG_NOTHING_HAPPENS);
        }
        else
        {
            dvar1 = 179 + you.religion;

            if (grd[you.x_pos][you.y_pos] == DNGN_FLOOR)
            {
#ifdef JP
                strcpy(info, "祭壇が床から現れた。 ");
#else
                strcpy(info, "An altar grows from the floor ");
#endif
                strcat(info,
                        (you.species == SP_NAGA || you.species == SP_CENTAUR)
#ifdef JP
                                            ? "" : "");
#else
                                            ? "before you!" : "at your feet!");
#endif
                mpr(info);
                grd[you.x_pos][you.y_pos] = dvar1;
            }
            else
            {
                do
                {
                    dvar[0] = 10 + random2(GXM - 20);
                    dvar[1] = 10 + random2(GYM - 20);
                }
                while (grd[dvar[0]][dvar[1]] != DNGN_FLOOR);

                grd[dvar[0]][dvar[1]] = dvar1;

#ifdef JP
                mpr( "あなたは神聖な力を知覚した！" );
#else
                mpr( "You sense divine power!" );
#endif
            }
        }
        break;

    case CARD_FAMINE:
#ifdef JP
        mpr("あなたは飢餓のカードを引いた。");
#else
        mpr("You have drawn Famine.");
#endif

        if (you.is_undead == US_UNDEAD)
#ifdef JP
            mpr("あなたは涼しい顔をしている。");
#else
            mpr("You feel rather smug.");
#endif
        else
            set_hunger(500, true);
        break;

    case CARD_FEAST:
#ifdef JP
        mpr("あなたは御馳走のカードを引いた。");
#else
        mpr("You have drawn the Feast.");
#endif

        if (you.is_undead == US_UNDEAD)
#ifdef JP
            mpr("あなたは恐ろしい無益さを感じた。");
#else
            mpr("You feel a horrible emptiness.");
#endif
        else
            set_hunger(12000, true);
        break;

    case CARD_WILD_MAGIC:
#ifdef JP
        mpr( "あなたはでたらめ魔法のカードを引いた。" );
#else
        mpr( "You have drawn Wild Magic." );
#endif
        miscast_effect( SPTYP_RANDOM, random2(15) + 5, random2(250), 0 );
        break;

    case CARD_VIOLENCE:
#ifdef JP
        mpr("あなたは暴力のカードを引いた。");
#else
        mpr("You have drawn Violence.");
#endif
        acquirement( OBJ_WEAPONS );
        break;

    case CARD_PROTECTION:
#ifdef JP
        mpr("あなたは護りのカードを引いた。");
#else
        mpr("You have drawn Protection.");
#endif
        acquirement( OBJ_ARMOUR );
        break;

    case CARD_KNOWLEDGE:
#ifdef JP
        mpr("あなたは知識のカードを引いた。");
#else
        mpr("You have drawn Knowledge.");
#endif
        acquirement( OBJ_BOOKS );
        break;

    case CARD_MAZE:
#ifdef JP
        mpr("あなたは迷路のカードを引いた！");
#else
        mpr("You have drawn the Maze!");
#endif
        more();

        if (you.level_type == LEVEL_DUNGEON)
            banished( DNGN_ENTER_LABYRINTH );
        break;

    case CARD_PANDEMONIUM:
#ifdef JP
        mpr("あなたはパンデモニウムのカードを引いた！");
#else
        mpr("You have drawn the Pandemonium card!");
#endif
        more();

        if (you.level_type == LEVEL_PANDEMONIUM)
            banished(DNGN_EXIT_PANDEMONIUM);
        else if (you.level_type == LEVEL_LABYRINTH)
            canned_msg(MSG_NOTHING_HAPPENS);
        else
            banished(DNGN_ENTER_PANDEMONIUM);
        break;

    case CARD_RULES_FOR_BRIDGE:
#ifdef JP
        mpr("あなたはコントラクト・ブリッジのルールを引いた。");
        mpr("なんとも興味深い！");
#else
        mpr("You have drawn the rules for contract bridge.");
        mpr("How intriguing!");
#endif
        break;

    case CARD_IMPRISONMENT:
#ifdef JP
        mpr("あなたは監獄のカードを引いた！");
#else
        mpr("You have drawn the Prison!");
#endif
        entomb();
        break;
    }

    return;
}                               // end cards()
