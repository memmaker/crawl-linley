/*
 *  File:       command.cc
 *  Summary:    Misc commands.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>     10/12/99        BCR             BUILD_DATE is now used in version()
 *      <3>     6/13/99         BWR             New equipment listing commands
 *      <2>     5/20/99         BWR             Swapping inventory letters.
 *      <1>     -/--/--         LRH             Created
 */

#include "AppHdr.h"
#include "command.h"

#include <stdio.h>
#include <string.h>
#include <ctype.h>

#include "externs.h"

#include "abl-show.h"
#include "invent.h"
#include "itemname.h"
#include "item_use.h"
#include "items.h"
#include "ouch.h"
#include "spl-cast.h"
#include "spl-util.h"
#include "stuff.h"
#include "version.h"
#include "wpn-misc.h"

static void adjust_item(void);
static void adjust_spells(void);
static void adjust_ability(void);
static void adjust_fill(void);
static void swap_items(int from_slot, int to_slot);

void quit_game(void)
{
#ifdef JP 
    if (yesno("本当に今回のゲームを放棄しますか？", false))
#else
    if (yesno("Really quit?", false))
#endif
        ouch(-9999, 0, KILLED_BY_QUITTING);
}                               // end quit_game()

void version(void)
{
#ifdef JP 
    mpr( "Dungeon Crawl " VERSION " (Build " BUILD_DATE ")" );
#else
    mpr( "This is Dungeon Crawl " VERSION " (Last build " BUILD_DATE ")." );
#endif
}                               // end version()

void adjust(void)
{
#ifdef JP 
    mpr( "何を入れ替えますか？", MSGCH_PROMPT );
    mpr( "[i]アイテム [s]呪文 [a]特殊能力", MSGCH_PROMPT );
    mpr( "[c]指定文字までのアイテムを前方の空き文字に詰める", MSGCH_PROMPT );
#else
    mpr( "Adjust (i)tems, (c)ompact items, (s)pells, or (a)bilities?", MSGCH_PROMPT );
#endif

    unsigned char keyin = tolower( get_ch() );

    if (keyin == 'i')
        adjust_item();
    else if (keyin == 's')
        adjust_spells();
    else if (keyin == 'a')
        adjust_ability();
    else if (keyin == 'c')
        adjust_fill();
    else if (keyin == ESCAPE)
        canned_msg( MSG_OK );
    else
        canned_msg( MSG_HUH );
}                               // end adjust()

static void adjust_fill(void)
{
    char str_pass[ ITEMNAME_SIZE ];
    int from_slot, to_slot;
#ifdef JP
    from_slot = prompt_invent_item( "どのアイテムまでを前方の空き文字に詰めますか？", -1, false );
#else
    from_slot = prompt_invent_item( "Upto which item letter do you want to compact?", -1, false );
#endif
    if (from_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    to_slot=Options.pick_items_start;
    for(;;)
    {
        while(to_slot < ENDOFPACK && is_valid_item( you.inv[to_slot] ))
            to_slot++;
        while(0 <= from_slot && !is_valid_item( you.inv[from_slot]) )
            from_slot--;
        if(from_slot <= to_slot)
            break;
        swap_items(from_slot, to_slot);
        in_name( to_slot, DESC_INVENTORY_EQUIP, str_pass );
        mpr( str_pass );
    }
}

static void adjust_item(void)
{
    int from_slot, to_slot;
    char str_pass[ ITEMNAME_SIZE ];

    if (inv_count() < 1)
    {
        canned_msg(MSG_NOTHING_CARRIED);
        return;
    }
/*
#ifdef JP
    if( yesno("前に詰めますか？", true))
#else
    if( yesno("Compact the item letters?", true))
#endif
    {
        adjust_fill();
        return;
    }
*/
#ifdef JP 
    from_slot = prompt_invent_item( "どのアイテムを入れ替えますか？", -1 ); 
#else
    from_slot = prompt_invent_item( "Adjust which item?", -1 ); 
#endif
    if (from_slot == PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    in_name( from_slot, DESC_INVENTORY_EQUIP, str_pass );
    mpr( str_pass );

#ifdef JP
    to_slot = prompt_invent_item( "どの文字に入れ替えますか？([SPACE]:前方に詰める)", -1, false, false, true, ' ');
#else
    to_slot = prompt_invent_item( "Adjust to which letter?", -1, false, false, true, ' ');
#endif
    if (to_slot ==  PROMPT_GOT_SPECIAL)
    {
        for (to_slot = Options.pick_items_start; to_slot < ENDOFPACK; to_slot++)
        {
            if (!is_valid_item( you.inv[to_slot] ) || to_slot == from_slot )
                break;
        }
    }
    if (to_slot ==  PROMPT_ABORT)
    {
        canned_msg( MSG_OK );
        return;
    }

    swap_items(from_slot, to_slot);

    in_name( to_slot, DESC_INVENTORY_EQUIP, str_pass );
    mpr( str_pass );

    if (is_valid_item( you.inv[from_slot] ))
    {
        in_name( from_slot, DESC_INVENTORY_EQUIP, str_pass );
        mpr( str_pass );
    }

}                               // end adjust_item()

static void swap_items(int from_slot, int to_slot)
{
    // swap items
    item_def tmp = you.inv[to_slot];
    you.inv[to_slot] = you.inv[from_slot];
    you.inv[from_slot] = tmp;

    you.inv[from_slot].link = from_slot;
    you.inv[to_slot].link = to_slot;

    for (int i = 0; i < NUM_EQUIP; i++)
    {
        if (you.equip[i] == from_slot)
            you.equip[i] = to_slot;
        else if (you.equip[i] == to_slot)
            you.equip[i] = from_slot;
    }
    if (to_slot == you.equip[EQ_WEAPON] || from_slot == you.equip[EQ_WEAPON])
        you.wield_change = true;
}

static void adjust_spells_cleanup(bool needs_redraw)
{
    if (needs_redraw)
        redraw_screen();
}

static void adjust_spells(void)
{
    unsigned char index_1, index_2;
    unsigned char nthing = 0;

    bool needs_redraw = false;

    if (!you.spell_no)
    {
#ifdef JP 
        mpr("あなたは呪文を知らない。");
#else
        mpr("You don't know any spells.");
#endif
        return;
    }

  query:
#ifdef JP 
    mpr("どの呪文を入れ替えますか？", MSGCH_PROMPT);
#else
    mpr("Adjust which spell?", MSGCH_PROMPT);
#endif

    unsigned char keyin = get_ch();

    if (keyin == '?' || keyin == '*')
    {
        if (keyin == '*' || keyin == '?')
        {
            nthing = list_spells();
            needs_redraw = true;
        }

        if (isalpha( nthing ) || nthing == ESCAPE)
            keyin = nthing;
        else
        {
            mesclr( true );
            goto query;
        }
    }

    if (keyin == ESCAPE)
    {
        adjust_spells_cleanup(needs_redraw);
        canned_msg( MSG_OK );
        return;
    }

    int input_1 = (int) keyin;

    if (!isalpha( input_1 ))
    {
        adjust_spells_cleanup(needs_redraw);
#ifdef JP 
        mpr("あなたはその呪文を知らない。");
#else
        mpr("You don't know that spell.");
#endif
        return;
    }

    index_1 = letter_to_index( input_1 );
    int spell = get_spell_by_letter( input_1 ); 

    if (spell == SPELL_NO_SPELL)
    {
        adjust_spells_cleanup(needs_redraw);
#ifdef JP 
        mpr("あなたはその呪文を知らない。");
#else
        mpr("You don't know that spell.");
#endif
        return;
    }

    // print out targeted spell:
#ifdef JP 
    snprintf( info, INFO_SIZE, "%c - %s", input_1, spell_title( spell ) );
#else
    snprintf( info, INFO_SIZE, "%c - %s", input_1, spell_title( spell ) );
#endif
    mpr(info);

#ifdef JP 
    mpr( "どの文字に入れ替えますか？", MSGCH_PROMPT );
#else
    mpr( "Adjust to which letter?", MSGCH_PROMPT );
#endif

    keyin = get_ch();

    if (keyin == '?' || keyin == '*')
    {
        if (keyin == '*' || keyin == '?')
        {
            nthing = list_spells();
            needs_redraw = true;
        }

        if (isalpha( nthing ) || nthing == ESCAPE)
            keyin = nthing;
        else
        {
            mesclr( true );
            goto query;
        }
    }

    if (keyin == ESCAPE)
    {
        adjust_spells_cleanup(needs_redraw);
        canned_msg( MSG_OK );
        return;
    }

    int input_2 = (int) keyin;

    if (!isalpha( input_2 ))
    {
        adjust_spells_cleanup(needs_redraw);
#ifdef JP 
        mpr("なんですって？");
#else
        mpr("What?");
#endif
        return;
    }

    adjust_spells_cleanup(needs_redraw);

    index_2 = letter_to_index( input_2 );

    // swap references in the letter table:
    int tmp = you.spell_letter_table[index_2];
    you.spell_letter_table[index_2] = you.spell_letter_table[index_1];
    you.spell_letter_table[index_1] = tmp;

    // print out spell in new slot (now at input_2)
#ifdef JP 
    snprintf( info, INFO_SIZE, "%c - %s", input_2, 
#else
    snprintf( info, INFO_SIZE, "%c - %s", input_2, 
#endif
              spell_title( get_spell_by_letter(input_2) ) );

    mpr(info);

    // print out other spell if one was involved (now at input_1)
    spell = get_spell_by_letter( input_1 );
    if (spell != SPELL_NO_SPELL)
    {
#ifdef JP 
        snprintf( info, INFO_SIZE, "%c - %s", input_1, spell_title(spell) );
#else
        snprintf( info, INFO_SIZE, "%c - %s", input_1, spell_title(spell) );
#endif
        mpr(info);
    }
}                               // end adjust_spells()

static void adjust_ability(void)
{
    unsigned char index_1, index_2;
    unsigned char nthing = 0;

    bool needs_redraw = false;

    if (!generate_abilities())
    {
#ifdef JP 
        mpr( "あなたは現在のところ特殊能力を持っていない。" );
#else
        mpr( "You don't currently have any abilities." );
#endif
        return;
    }

  query:
#ifdef JP 
    mpr( "どの特殊能力を入れ替えますか？", MSGCH_PROMPT );
#else
    mpr( "Adjust which ability?", MSGCH_PROMPT );
#endif

    unsigned char keyin = get_ch();

    if (keyin == '?' || keyin == '*')
    {
        if (keyin == '*' || keyin == '?')
        {
            nthing = show_abilities();
            needs_redraw = true;
        }

        if (isalpha( nthing ) || nthing == ESCAPE)
            keyin = nthing;
        else
        {
            mesclr( true );
            goto query;
        }
    }

    if (keyin == ESCAPE)
    {
        adjust_spells_cleanup(needs_redraw);
        canned_msg( MSG_OK );
        return;
    }

    int input_1 = (int) keyin;

    if (!isalpha( input_1 ))
    {
        adjust_spells_cleanup(needs_redraw);
#ifdef JP 
        mpr("あなたはその特殊能力を持っていない。");
#else
        mpr("You don't have that ability.");
#endif
        return;
    }

    index_1 = letter_to_index( input_1 );

    if (you.ability_letter_table[index_1] == ABIL_NON_ABILITY)
    {
        adjust_spells_cleanup(needs_redraw);
#ifdef JP 
        mpr("あなたはその特殊能力を持っていない。");
#else
        mpr("You don't have that ability.");
#endif
        return;
    }

    // print out targeted spell:
#ifdef JP 
    snprintf( info, INFO_SIZE, "%c - %s", input_1, 
#else
    snprintf( info, INFO_SIZE, "%c - %s", input_1, 
#endif
              get_ability_name_by_index( index_1 ) );

    mpr(info);

#ifdef JP 
    mpr( "どの文字に入れ替えますか？", MSGCH_PROMPT );
#else
    mpr( "Adjust to which letter?", MSGCH_PROMPT );
#endif

    keyin = get_ch();

    if (keyin == '?' || keyin == '*')
    {
        if (keyin == '*' || keyin == '?')
        {
            nthing = show_abilities();
            needs_redraw = true;
        }

        if (isalpha( nthing ) || nthing == ESCAPE)
            keyin = nthing;
        else
        {
            mesclr( true );
            goto query;
        }
    }

    if (keyin == ESCAPE)
    {
        adjust_spells_cleanup(needs_redraw);
        canned_msg( MSG_OK );
        return;
    }

    int input_2 = (int) keyin;

    if (!isalpha( input_2 ))
    {
        adjust_spells_cleanup(needs_redraw);
#ifdef JP 
        mpr("なんですって？");
#else
        mpr("What?");
#endif
        return;
    }

    adjust_spells_cleanup(needs_redraw);

    index_2 = letter_to_index( input_2 );

    // swap references in the letter table:
    int tmp = you.ability_letter_table[index_2];
    you.ability_letter_table[index_2] = you.ability_letter_table[index_1];
    you.ability_letter_table[index_1] = tmp;

    // Note:  the input_2/index_1 and input_1/index_2 here is intentional.
    // This is because nothing actually moves until generate_abilities is
    // called again... fortunately that has to be done everytime because
    // that's the silly way this system currently works.  -- bwr
#ifdef JP 
    snprintf( info, INFO_SIZE, "%c - %s", input_2, 
#else
    snprintf( info, INFO_SIZE, "%c - %s", input_2, 
#endif
              get_ability_name_by_index( index_1 ) );

    mpr(info);

    if (you.ability_letter_table[index_1] != ABIL_NON_ABILITY)
    {
#ifdef JP 
        snprintf( info, INFO_SIZE, "%c - %s", input_1, 
#else
        snprintf( info, INFO_SIZE, "%c - %s", input_1, 
#endif
                  get_ability_name_by_index( index_2 ) );
        mpr(info);
    }
}                               // end adjust_ability()

void list_armour(void)
{
    for (int i = EQ_CLOAK; i <= EQ_BODY_ARMOUR; i++)
    {
        int armour_id = you.equip[i];

        strcpy( info,
#ifdef JP 
                (i == EQ_CLOAK)       ? "外套   " :
                (i == EQ_HELMET)      ? "兜     " :
                (i == EQ_GLOVES)      ? "小手   " :
                (i == EQ_SHIELD)      ? "盾     " :
                (i == EQ_BODY_ARMOUR) ? "鎧     " :
#else
                (i == EQ_CLOAK)       ? "Cloak  " :
                (i == EQ_HELMET)      ? "Helmet " :
                (i == EQ_GLOVES)      ? "Gloves " :
                (i == EQ_SHIELD)      ? "Shield " :
                (i == EQ_BODY_ARMOUR) ? "Armour " :
#endif

                (i == EQ_BOOTS
                    && !(you.species == SP_CENTAUR || you.species == SP_NAGA))
#ifdef JP 
                                      ? "靴     " :
#else
                                      ? "Boots  " :
#endif
                (i == EQ_BOOTS
                    && (you.species == SP_CENTAUR || you.species == SP_NAGA))
#ifdef JP 
                                      ? "下肢   "
                                      : "不明   " );
#else
                                      ? "Barding"
                                      : "unknown" );
#endif

        strcat(info, " : ");

        if (armour_id != -1)
        {
            char str_pass[ ITEMNAME_SIZE ];
            in_name(armour_id, DESC_INVENTORY, str_pass);
            strcat(info, str_pass);
        }
        else
        {
#ifdef JP 
            strcat(info, "    なし");
#else
            strcat(info, "    none");
#endif
        }

        mpr( info, MSGCH_EQUIPMENT );
    }
}                               // end list_armour()

void list_jewellery(void)
{
    for (int i = EQ_LEFT_RING; i <= EQ_AMULET; i++)
    {
        int jewellery_id = you.equip[i];

#ifdef JP 
        strcpy( info, (i == EQ_LEFT_RING)  ? "左手の指輪" :
                      (i == EQ_RIGHT_RING) ? "右手の指輪" :
                      (i == EQ_AMULET)     ? "護符      "
                                           : "unknown   " );
#else
        strcpy( info, (i == EQ_LEFT_RING)  ? "Left ring " :
                      (i == EQ_RIGHT_RING) ? "Right ring" :
                      (i == EQ_AMULET)     ? "Amulet    "
                                           : "unknown   " );
#endif

        strcat(info, " : ");

        if (jewellery_id != -1)
        {
            char str_pass[ ITEMNAME_SIZE ];
            in_name(jewellery_id, DESC_INVENTORY, str_pass);
            strcat(info, str_pass);
        }
        else
        {
#ifdef JP 
            strcat(info, "    なし");
#else
            strcat(info, "    none");
#endif
        }

        mpr( info, MSGCH_EQUIPMENT );
    }
}                               // end list_jewellery()

void list_weapons(void)
{
    const int weapon_id = you.equip[EQ_WEAPON];

    // Output the current weapon
    //
    // Yes, this is already on the screen... I'm outputing it
    // for completeness and to avoid confusion.
#ifdef JP 
    strcpy(info, "現在の武器: ");
#else
    strcpy(info, "Current   : ");
#endif

    if (weapon_id != -1)
    {
        char str_pass[ ITEMNAME_SIZE ];
        in_name( weapon_id, DESC_INVENTORY_EQUIP, str_pass );
        strcat(info, str_pass);
    }
    else
    {
        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
#ifdef JP 
            strcat(info, "    刃の手     ");
#else
            strcat(info, "    blade hands");
#endif
        else
#ifdef JP 
            strcat(info, "    なし       ");
#else
            strcat(info, "    empty hands");
#endif
    }

    mpr(info);

    // Print out the swap slots
    for (int i = 0; i <= 1; i++)
    {
        // We'll avoid repeating the current weapon for these slots,
        // in order to keep things clean.
        if (weapon_id == i)
            continue;

#ifdef JP 
        strcpy(info, (i == 0) ? "第一の武器: " : "予備の武器: ");
#else
        strcpy(info, (i == 0) ? "Primary   : " : "Secondary : ");
#endif

        if (is_valid_item( you.inv[i] ))
        {
            char str_pass[ ITEMNAME_SIZE ];
            in_name(i, DESC_INVENTORY_EQUIP, str_pass);
            strcat(info, str_pass);
        }
        else
#ifdef JP 
            strcat(info, "    なし");
#else
            strcat(info, "    none");
#endif

        mpr(info);              // Output slot
    }

    // Now we print out the current default fire weapon
#ifdef JP 
    strcpy(info, "飛び道具  : ");
#else
    strcpy(info, "Firing    : ");
#endif

    const int item = get_fire_item_index();

    if (item == ENDOFPACK)
#ifdef JP 
        strcat(info, "       なし");
#else
        strcat(info, "    nothing");
#endif
    else
    {
        char str_pass[ ITEMNAME_SIZE ];
        in_name( item, DESC_INVENTORY_EQUIP, str_pass );
        strcat( info, str_pass );
    }

    mpr( info, MSGCH_EQUIPMENT );
}                               // end list_weapons()
