/*
 *  File:       rvip.cc
 *  Summary:    RVIP additions: stair walking, Enter command menu,
 *              queued command keys.
 */

#include "AppHdr.h"

#include <string.h>

#include "externs.h"
#include "invent.h"
#include "itemname.h"
#include "items.h"
#include "macro.h"
#include "mon-util.h"
#include "player.h"
#include "rvip.h"
#include "stuff.h"
#include "travel.h"
#include "view.h"

// Itakura's item preselection (invent.cc)
void push_inven_idx(int idx);
void push_inven_count(int c);
int pop_inven_idx();
int pop_inven_count();

int rvip_raw_dirs = 0;
int rvip_at_cmd = 0;            // waiting for a command key (the web prompt line)

// keeps rvip_raw_dirs raised while a list or menu is open
struct raw_dirs
{
    raw_dirs()  { rvip_raw_dirs++; }
    ~raw_dirs() { rvip_raw_dirs--; }
};

/* ---------------- queued command keys ---------------- */

static int queue[8];
static int queue_len = 0;

void rvip_push_key(int key)
{
    if (queue_len < 8)
        queue[queue_len++] = key;
}

static bool monster_in_view()
{
    for (int i = 0; i < MAX_MONSTERS; i++)
    {
        struct monsters *m = &menv[i];

        if (m->type != -1 && mons_near(m) && player_monster_visible(m)
            && !mons_friendly(m) && !mons_is_mimic(m->type)
            && !mons_flag(m->type, M_NO_EXP_GAIN))
        {
            return true;
        }
    }
    return false;
}

/* ---------------- boxes ---------------- */

// A box sized to its content, centred on the text layer; rows beyond the
// screen scroll so that 'cur' stays visible.
struct box_row
{
    char text[ITEMNAME_SIZE + 8];
    int colour;         // 0 = header (yellow, not selectable)
    int key;            // what choosing the row returns
};

static void draw_box(const std::vector<box_row> &rows, int cur,
                     const char *title)
{
    int width = title ? strlen(title) : 0;
    for (unsigned i = 0; i < rows.size(); i++)
        if ((int) strlen(rows[i].text) > width)
            width = strlen(rows[i].text);

    const int extra = title ? 1 : 0;
    const int maxrows = get_number_of_lines() - 2 - extra;
    const int shown = ((int) rows.size() < maxrows) ? rows.size() : maxrows;
    const int left = (80 - (width + 4)) / 2 + 1;
    const int top = (get_number_of_lines() - (shown + 2 + extra)) / 2 + 1;

    int first = 0;
    if (cur >= shown)
        first = cur - shown + 1;
    if (first > 0 && cur == first && rows[cur - 1].colour == 0)
        first--;

    clrscr();
    textcolor(DARKGREY);
    for (int edge = 0; edge < 2; edge++)
    {
        gotoxy(left, edge ? top + shown + 1 + extra : top);
        cprintf("+");
        for (int x = 0; x < width + 2; x++)
            cprintf("-");
        cprintf("+");
    }
    if (title)
    {
        gotoxy(left, top + 1);
        cprintf("| ");
        textcolor(WHITE);
        cprintf("%-*s", width, title);
        textcolor(DARKGREY);
        cprintf(" |");
    }
    for (int r = 0; r < shown; r++)
    {
        const box_row &row = rows[first + r];

        gotoxy(left, top + 1 + extra + r);
        textcolor(DARKGREY);
        cprintf("| ");
        textcolor(row.colour == 0 ? YELLOW
                  : first + r == cur ? WHITE : row.colour);
        if (first + r == cur)
            textbackground(BLUE);
        cprintf("%-*s", width, row.text);
        textbackground(BLACK);
        textcolor(DARKGREY);
        cprintf(" |");
    }
    textcolor(LIGHTGREY);
    update_screen();
}

static int step_row(const std::vector<box_row> &rows, int cur, int step)
{
    for (int i = cur + step; i >= 0 && i < (int) rows.size(); i += step)
        if (rows[i].colour != 0)
            return i;
    return cur;
}

static bool key_up(int key)
{
    return key == RVIP_KEY_DIR(8) || key == RVIP_KEY_UP;
}

static bool key_down(int key)
{
    return key == RVIP_KEY_DIR(2) || key == RVIP_KEY_DOWN;
}

static bool key_close(int key)
{
    return key == ESCAPE || key == CMD_EXPLORE /* keypad 0 */ || key == '.';
}

static bool key_choose(int key)
{
    return key == '\r' || key == '\n' || key == RVIP_KEY_DIR(5);
}

/* ---------------- inventory ---------------- */

static bool class_wanted(int cls, int type_expect)
{
    return type_expect == -1 || cls == type_expect
        || (type_expect == OBJ_MISSILES && cls == OBJ_WEAPONS)
        || (type_expect == OBJ_WEAPONS
            && (cls == OBJ_STAVES || cls == OBJ_MISCELLANY))
        || (type_expect == OBJ_SCROLLS && cls == OBJ_BOOKS);
}

static const char *class_name(int cls)
{
    switch (cls)
    {
    case OBJ_WEAPONS:    return "Hand Weapons";
    case OBJ_MISSILES:   return "Missiles";
    case OBJ_ARMOUR:     return "Armour";
    case OBJ_WANDS:      return "Magical Devices";
    case OBJ_FOOD:       return "Comestibles";
    case OBJ_SCROLLS:    return "Scrolls";
    case OBJ_JEWELLERY:  return "Jewellery";
    case OBJ_POTIONS:    return "Potions";
    case OBJ_UNKNOWN_II: return "Gems";
    case OBJ_BOOKS:      return "Books";
    case OBJ_STAVES:     return "Magical Staves and Rods";
    case OBJ_ORBS:       return "Orbs of Power";
    case OBJ_MISCELLANY: return "Miscellaneous";
    case OBJ_CORPSES:    return "Carrion";
    default:             return "Other";
    }
}

static bool is_equipped(int idx)
{
    for (int i = 0; i < NUM_EQUIP; i++)
        if (you.equip[i] == idx)
            return true;
    return false;
}

struct item_action
{
    int key;
    const char *name;
};

// Every action that fits the item, the main one first.
static std::vector<item_action> item_actions(int idx)
{
    std::vector<item_action> acts;
    const item_def &item = you.inv[idx];
    const bool worn = is_equipped(idx);
    item_action a;

#define ACT(k, n) do { a.key = (k); a.name = (n); acts.push_back(a); } while (0)
    switch (item.base_type)
    {
    case OBJ_WEAPONS:
    case OBJ_STAVES:
        if (!worn) ACT('w', "wield");
        ACT('t', "throw");
        break;
    case OBJ_MISCELLANY:
        if (worn) ACT('E', "evoke");
        if (!worn) ACT('w', "wield");
        break;
    case OBJ_MISSILES:  ACT('t', "throw"); break;
    case OBJ_ARMOUR:
        if (worn) ACT('T', "take off");
        else ACT('W', "wear");
        break;
    case OBJ_WANDS:     ACT('z', "zap"); break;
    case OBJ_FOOD:      ACT('e', "eat"); break;
    case OBJ_SCROLLS:   ACT('r', "read"); break;
    case OBJ_BOOKS:
        ACT('r', "read");
        ACT('M', "memorise a spell");
        break;
    case OBJ_JEWELLERY:
        if (worn) ACT('R', "remove");
        else ACT('P', "put on");
        break;
    case OBJ_POTIONS:   ACT('q', "quaff"); break;
    }
    ACT('v', "examine");
    ACT('d', "drop");
#undef ACT
    return acts;
}

static int last_letter = 0;     // cursor comes back here on reopen

static void do_item_action(int idx, int key, bool reopen)
{
    last_letter = index_to_letter(idx);
    push_inven_idx(idx);
    if (key == 'd')
        push_inven_count(you.inv[idx].quantity);
    rvip_push_key(key);
    if (reopen)
        rvip_push_key(RVIP_REOPEN);
}

// Action menu for one item; returns the chosen action key or 0.
static int item_menu(int idx)
{
    raw_dirs guard;

    std::vector<item_action> acts = item_actions(idx);
    std::vector<box_row> rows;
    char title[ITEMNAME_SIZE];

    in_name(idx, DESC_INVENTORY_EQUIP, title);
    for (unsigned i = 0; i < acts.size(); i++)
    {
        box_row r;
        snprintf(r.text, sizeof(r.text), "%c  %s", acts[i].key, acts[i].name);
        r.colour = LIGHTGREY;
        r.key = acts[i].key;
        rows.push_back(r);
    }

    int cur = 0;
    for (;;)
    {
        draw_box(rows, cur, title);
        const int key = getch();

        if (key_close(key) || key == RVIP_KEY_DIR(4))
            return 0;
        if (key_up(key))
            cur = step_row(rows, cur, -1);
        else if (key_down(key))
            cur = step_row(rows, cur, 1);
        else if (key_choose(key) || key == RVIP_KEY_DIR(6) || key == ' ')
            return rows[cur].key;
        else
            for (unsigned i = 0; i < rows.size(); i++)
                if (rows[i].key == key)
                    return key;
    }
}

// The inventory list with a cursor.  browse: the 'i' screen (letters run
// the item's main action).  Otherwise an item prompt: returns the chosen
// letter, ESCAPE, or any other key for the prompt to handle.
int rvip_item_list(int type_expect, bool browse, const char *prompt)
{
    raw_dirs guard;

    for (;;)
    {
        std::vector<box_row> rows;
        char title[80];
        const int cap = carrying_capacity();

        if (prompt)
            snprintf(title, sizeof(title), "%s", prompt);
        else
            snprintf(title, sizeof(title),
                     "Inventory: %d.%d aum (%d%% of %d.%d aum maximum)",
                     you.burden / 10, you.burden % 10,
                     (you.burden * 100) / cap, cap / 10, cap % 10);

        for (int cls = 0; cls < NUM_OBJECT_CLASSES; cls++)
        {
            if (!class_wanted(cls, type_expect))
                continue;

            bool header = false;
            for (int j = 0; j < ENDOFPACK; j++)
            {
                if (!is_valid_item(you.inv[j]) || you.inv[j].base_type != cls)
                    continue;

                box_row r;
                if (!header)
                {
                    snprintf(r.text, sizeof(r.text), "%s", class_name(cls));
                    r.colour = 0;
                    r.key = 0;
                    rows.push_back(r);
                    header = true;
                }
                in_name(j, DESC_INVENTORY_EQUIP, r.text);
                r.colour = LIGHTGREY;
                if (is_equipped(j))
                    r.colour = YELLOW;
                if (item_cursed(you.inv[j])
                    && item_ident(you.inv[j], ISFLAG_KNOW_CURSE))
                {
                    r.colour = LIGHTRED;
                }
                r.key = index_to_letter(j);
                rows.push_back(r);
            }
        }

        if (rows.empty())
        {
            if (type_expect != -1 && !browse)
            {
                type_expect = -1;       // nothing of that kind: show all
                continue;
            }
            mpr("You aren't carrying anything.");
            return ESCAPE;
        }

        int cur = step_row(rows, -1, 1);
        for (unsigned i = 0; browse && i < rows.size(); i++)
            if (rows[i].colour != 0 && rows[i].key == last_letter)
                cur = i;
        last_letter = 0;

        for (;;)
        {
            draw_box(rows, cur, title);
            const int key = getch();
            const int idx = letter_to_index(rows[cur].key);

            if (key_close(key))
            {
                redraw_screen();
                return ESCAPE;
            }
            if (key_up(key))
            {
                cur = step_row(rows, cur, -1);
                continue;
            }
            if (key_down(key))
            {
                cur = step_row(rows, cur, 1);
                continue;
            }

            if (!browse)
            {
                if (key == '*' || key == '?')
                {
                    type_expect = (key == '*') ? -1 : type_expect;
                    break;              // rebuild the list
                }
                redraw_screen();
                return key_choose(key) ? rows[cur].key : key;
            }

            // browse mode
            int act = 0, item = idx;

            if (key_choose(key) || key == ' ' || key == RVIP_KEY_DIR(6))
                act = item_menu(idx);
            else if (key == RVIP_KEY_KP_ADD)
                act = item_actions(idx)[0].key;
            else if (key == RVIP_KEY_KP_SUB)
                act = 'd';
            else if (key == RVIP_KEY_KP_MUL)
                act = 'v';
            else if (key >= 1 && key <= 26)             // Ctrl+letter
            {
                item = letter_to_index('a' + key - 1);
                act = 'v';
            }
            else if (isalpha(key))
            {
                item = letter_to_index(key);
                if (isupper(key) && !is_valid_item(you.inv[item]))
                {
                    // Shift+letter drops, unless a capital letter item exists
                    item = letter_to_index(tolower(key));
                    act = 'd';
                }
                else if (is_valid_item(you.inv[item]))
                    act = item_actions(item)[0].key;
            }
            else
            {
                redraw_screen();
                return key;             // a normal command
            }

            if (act && is_valid_item(you.inv[item]))
            {
                redraw_screen();
                do_item_action(item, act, true);
                return 0;
            }
        }
    }
}

/* ---------------- stairs ---------------- */

static int stairs_key = 0;          // '<' or '>' while walking to stairs
static int stairs_x, stairs_y;

static bool is_up_stair(int g)
{
    return (g >= DNGN_STONE_STAIRS_UP_I && g <= DNGN_ROCK_STAIRS_UP)
        || (g >= DNGN_RETURN_FROM_ORCISH_MINES && g < 150);
}

static bool is_down_stair(int g)
{
    return (g >= DNGN_STONE_STAIRS_DOWN_I && g <= DNGN_ROCK_STAIRS_DOWN);
}

// '<' / '>' off stairs: travel to the nearest known staircase of that kind,
// take it on arrival.  Returns false when none is known.
bool rvip_walk_stairs(int key)
{
    std::vector<coord_def> features;
    find_travel_pos(you.x_pos, you.y_pos, NULL, NULL, &features);

    for (unsigned i = 0; i < features.size(); i++)
    {
        const int g = grd[features[i].x][features[i].y];
        if (key == '<' ? is_up_stair(g) : is_down_stair(g))
        {
            stairs_key = key;
            stairs_x = features[i].x;
            stairs_y = features[i].y;
            start_travel(stairs_x, stairs_y);
            return true;
        }
    }
    return false;
}

/* ---------------- Enter menu ---------------- */

struct menu_entry
{
    int key;            // 0 = group header
    const char *name;   // key as shown
    const char *desc;
};

static const menu_entry commands[] = {
    { 0, NULL, "Movement" },
    { CONTROL('O'), "^O", "explore the level" },
    { '<', "<", "go up stairs (walks to nearest)" },
    { '>', ">", "go down stairs (walks to nearest)" },
    { CONTROL('G'), "^G", "travel to a level" },
    { CONTROL('F'), "^F", "add travel waypoint" },
    { 'o', "o", "open a door" },
    { 'c', "c", "close a door" },
    { 's', "s", "search adjacent tiles" },
    { '.', ".", "rest one turn" },
    { 0, NULL, "Items" },
    { 'i', "i", "inventory" },
    { ',', ",", "pick up items" },
    { 'd', "d", "drop items" },
    { 'w', "w", "wield an item" },
    { '\'', "'", "wield item a, or switch to b" },
    { 'W', "W", "wear armour" },
    { 'T', "T", "take off armour" },
    { 'P', "P", "put on jewellery" },
    { 'R', "R", "remove jewellery" },
    { 'e', "e", "eat food" },
    { 'q', "q", "quaff a potion" },
    { 'r', "r", "read a scroll or book" },
    { 'z', "z", "zap a wand" },
    { 'E', "E", "evoke power of wielded item" },
    { 't', "t", "throw/shoot an item" },
    { 'f', "f", "fire first available missile" },
    { 'D', "D", "dissect a corpse" },
    { 'v', "v", "view item description" },
    { CONTROL('D'), "^D", "destroy inventory item" },
    { '=', "=", "reassign inventory/spell letters" },
    { CONTROL('A'), "^A", "toggle autopickup" },
    { CONTROL('S'), "^S", "mark stash here" },
    { CONTROL('E'), "^E", "forget stash here" },
    { 0, NULL, "Magic and religion" },
    { 'Z', "Z", "cast a spell" },
    { 'M', "M", "memorise a spell" },
    { 'a', "a", "use special ability" },
    { 'p', "p", "pray" },
    { '!', "!", "shout or command allies" },
    { 0, NULL, "Information" },
    { 'x', "x", "examine surroundings" },
    { 'X', "X", "level map" },
    { 'O', "O", "dungeon overview" },
    { ']', "]", "worn armour" },
    { '"', "\"", "worn jewellery" },
    { '\\', "\\", "item knowledge" },
    { 'A', "A", "abilities and mutations" },
    { 'C', "C", "experience" },
    { 'm', "m", "skills" },
    { '@', "@", "status" },
    { '^', "^", "religion" },
    { CONTROL('P'), "^P", "old messages" },
    { '?', "?", "help" },
    { 0, NULL, "Game" },
    { '#', "#", "dump character to file" },
    { '`', "`", "add macro" },
    { '~', "~", "save macros" },
    { CONTROL('R'), "^R", "redraw screen" },
    { 'V', "V", "version" },
    { 'S', "S", "save game and exit" },
    { CONTROL('X'), "^X", "save game without query" },
    { 'Q', "Q", "quit without saving" },
};

#define NCOMMANDS ((int) (sizeof(commands) / sizeof(commands[0])))

static int next_command(int i, int step)
{
    do
        i += step;
    while (i >= 0 && i < NCOMMANDS && commands[i].key == 0);

    return (i < 0 || i >= NCOMMANDS) ? -1 : i;
}

// Draws a boxed list sized to its content (scrolls when taller than the
// screen) and returns the chosen command key, or 0.
static int command_menu()
{
    raw_dirs guard;

    int width = 0;
    for (int i = 0; i < NCOMMANDS; i++)
    {
        const int w = commands[i].key ? strlen(commands[i].name) + 3
                                        + strlen(commands[i].desc)
                                      : strlen(commands[i].desc);
        if (w > width)
            width = w;
    }

    const int rows = get_number_of_lines() - 2;       // inside the border
    const int shown = (NCOMMANDS < rows) ? NCOMMANDS : rows;
    const int left = (80 - (width + 4)) / 2 + 1;
    const int top = (get_number_of_lines() - (shown + 2)) / 2 + 1;

    int cur = next_command(-1, 1);
    int first = 0;
    int result = 0;

    clrscr();
    for (;;)
    {
        if (cur < first + 1)
            first = (cur > 0 && commands[cur - 1].key == 0) ? cur - 1 : cur;
        if (cur >= first + shown)
            first = cur - shown + 1;

        textcolor(DARKGREY);
        gotoxy(left, top);
        cprintf("+");
        for (int x = 0; x < width + 2; x++)
            cprintf("-");
        cprintf("+");
        gotoxy(left, top + shown + 1);
        cprintf("+");
        for (int x = 0; x < width + 2; x++)
            cprintf("-");
        cprintf("+");

        for (int r = 0; r < shown; r++)
        {
            const int i = first + r;
            char line[100];

            if (commands[i].key)
                snprintf(line, sizeof(line), "%-2s  %s",
                         commands[i].name, commands[i].desc);
            else
                snprintf(line, sizeof(line), "%s", commands[i].desc);

            textcolor(DARKGREY);
            gotoxy(left, top + 1 + r);
            cprintf("| ");
            textcolor(commands[i].key == 0 ? YELLOW
                      : i == cur ? WHITE : LIGHTGREY);
            if (i == cur)
                textbackground(BLUE);
            cprintf("%-*s", width, line);
            textbackground(BLACK);
            textcolor(DARKGREY);
            cprintf(" |");
        }
        textcolor(LIGHTGREY);
        update_screen();

        int key = getch();
        int n;

        if (key == ESCAPE || key == ' ' || key == '0')
            break;
        else if (key_choose(key) || key == '5')
        {
            result = commands[cur].key;
            break;
        }
        else if ((key_down(key) || key == '2')
                 && (n = next_command(cur, 1)) >= 0)
            cur = n;
        else if ((key_up(key) || key == '8')
                 && (n = next_command(cur, -1)) >= 0)
            cur = n;
        else
        {
            for (int i = 0; i < NCOMMANDS; i++)
                if (commands[i].key && commands[i].key == key)
                    result = key;
            if (result)
                break;
        }
    }

    redraw_screen();
    return result;
}

/* ---------------- command key ---------------- */

// Replaces the plain key read at the command prompt.
int rvip_getkey()
{
#ifdef USE_WEB
    web_autosave();
#endif

    // arrived at the stairs we were walking to: take them
    if (stairs_key)
    {
        const int key = stairs_key;
        stairs_key = 0;
        if (you.x_pos == stairs_x && you.y_pos == stairs_y)
            return key;
    }

    while (queue_len)
    {
        const int key = queue[0];
        memmove(queue, queue + 1, --queue_len * sizeof(int));
        if (key != RVIP_REOPEN)
            return key;
        if (!monster_in_view())
            return 'i';
    }

    // an item pushed for a command that never asked for it
    pop_inven_idx();
    pop_inven_count();

    for (;;)
    {
        rvip_at_cmd = 1;
        int key = getch_with_command_macros();
        rvip_at_cmd = 0;

        if (key == CMD_MOUSE_WHEEL_UP || key == CMD_MOUSE_WHEEL_DOWN)
            continue;

        if (key == '\r')
        {
            key = command_menu();
            if (!key)
                continue;
        }
        return key;
    }
}
