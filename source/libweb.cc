/*
 *  File:       libx11.cc
 *  Summary:    Functions for x11
 *  Written by: M.Itakura
 *
 *  Change History (most recent first):
 *
 *      <1>     03/08/11        Ita     copied from liblinux.cc and modified
 *
 */

#include "AppHdr.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <emscripten.h>

// X11 keysyms the key handler below uses (from X11/keysymdef.h)
#define XK_BackSpace 0xff08
#define XK_Tab 0xff09
#define XK_Return 0xff0d
#define XK_Escape 0xff1b
#define XK_Muhenkan 0xff22
#define XK_Henkan 0xff23
#define XK_Left 0xff51
#define XK_Up 0xff52
#define XK_Right 0xff53
#define XK_Down 0xff54
#define XK_KP_Enter 0xff8d
#define XK_KP_Home 0xff95
#define XK_KP_Left 0xff96
#define XK_KP_Up 0xff97
#define XK_KP_Right 0xff98
#define XK_KP_Down 0xff99
#define XK_KP_Page_Up 0xff9a
#define XK_KP_Page_Down 0xff9b
#define XK_KP_End 0xff9c
#define XK_KP_Insert 0xff9e
#define XK_KP_Delete 0xff9f
#define XK_KP_Decimal 0xffae
#define XK_KP_0 0xffb0
#define XK_KP_1 0xffb1
#define XK_KP_2 0xffb2
#define XK_KP_3 0xffb3
#define XK_KP_4 0xffb4
#define XK_KP_5 0xffb5
#define XK_KP_6 0xffb6
#define XK_KP_7 0xffb7
#define XK_KP_8 0xffb8
#define XK_KP_9 0xffb9
#define XK_F1 0xffbe
#define XK_F2 0xffbf
#define XK_F3 0xffc0
#define XK_F4 0xffc1
#define XK_F5 0xffc2
#define XK_F6 0xffc3
#define XK_F7 0xffc4
#define XK_F8 0xffc5
#define XK_F9 0xffc6
#define XK_F10 0xffc7
#define XK_F11 0xffc8
#define XK_F12 0xffc9
#define XK_Delete 0xffff

// the page: web/crawl.js (Module.cr)
EM_JS(int, js_event, (int *ev, int at_cmd), {
    var e = Module.cr.event(at_cmd);
    if (!e) return 0;
    for (var i = 0; i < 5; i++) HEAP32[(ev >> 2) + i] = e[i] | 0;
    return 1;
});
EM_JS(int, js_pending, (void), { return Module.cr.pending(); });

void web_present();   // winclass-web.cc
extern int rvip_at_cmd;   // rvip.cc


//Options
#include "externs.h"

//ウィンドウ＆領域のクラス
#include "winclass.h"
#include "rvip.h"
#include "itemname.h"
#include "mon-util.h"
#include "player.h"
#include "stuff.h"
#include "view.h"

/*
 * Tile related stuff
 */
#ifdef USE_TILE


// from libtile.cc
extern void init_tile_all();
extern int handle_mouse_motion(int mouse_x, int mouse_y);
extern int handle_mouse_button(int mx, int my, 
                 int button, bool shift, bool ctrl);

// Raw tile images
img_type TileImg, TileQvImg;
img_type PlayerImg;

// Screen buffer
img_type ScrBufImg;

/* X11 */ typedef unsigned long pix_type;
pix_type pix_transparent;
pix_type pix_hilite;
pix_type pix_black;
pix_type pix_rimcolor;
#define MAX_PIX_COLOR 16
pix_type pix_col[MAX_PIX_COLOR];

/* X11 */ int x11_byte_per_pixel_ximage();

#endif //USE_TILE

/*
 * Global variables
 */

WinClass *win_main = NULL;

TextRegionClass *region_crt = NULL;
TextRegionClass *region_msg = NULL;
TextRegionClass *region_stat = NULL;
MapRegionClass  *region_map = NULL;
TileRegionClass *region_tile = NULL;

#if 1 //Slot
TileRegionClass *region_item = NULL;
TileRegionClass *region_item2 = NULL;
#endif

static int tile_scr_width;
static int tile_scr_height;

#define KEYBUFMAX 1024
static int keybuf[KEYBUFMAX];
static int key_head, key_tail;

// a lookup table to convert keypresses to command enums
/* X11 */ static int key_to_command_table[256];

/******** GMAP *******/
#define GMAP_XMAX GXM
#define GMAP_YMAX GYM
#define GMAP_XMARGIN 5
#define GMAP_YMARGIN 4

#define GMAP_XDOTS 3
#define GMAP_YDOTS 3

/******* FONTS ***********/
#ifdef JP
/* X11 */ #define DEFAULT_FONT_MSG  "-alias-fixed-bold-r-normal--16-*"
/* X11 */ #define DEFAULT_FONT_STAT "-alias-fixed-bold-r-normal--16-*"
/* X11 */ #define DEFAULT_FONT_CRT  "-alias-fixed-bold-r-normal--16-*"
#else
/* X11 */ #define DEFAULT_FONT_MSG  "8x13"
/* X11 */ #define DEFAULT_FONT_STAT "8x13"
/* X11 */ #define DEFAULT_FONT_CRT  "8x13"
#endif

/******* Window SIZES ***********/
#define MSG_LINES 9

#define CRT_XMAX 80
#define CRT_YMAX 25


//Debug output
#define DEBUGLOG( args...) {FILE *fp=fopen("log","a");fprintf(fp, ##args);\
                             fclose(fp);}

/*** Main body ***/
/* X11 */
unsigned long create_pixel(unsigned int red, unsigned int green, unsigned int blue)
{
    // web: RGBA bytes in memory, as the canvas wants them
    return 0xff000000UL | (blue << 16) | (green << 8) | red;
}

/* X11 */
const char *my_getenv(char *envname, const char *def){
    const char *result = getenv(envname);
    if (!result) result = def;
    return result;
}

/* X11 */
int my_getenv_int(char *envname, int def){
    const char *rstr = getenv(envname);
    if (!rstr) return def;
    return atoi(rstr);
}

/* X11 */
void region_modify_size(int *mx, int *my, const char *envname)
{
    char buf[1024];
    const char *val;
    int n = 0;
    sprintf(buf,"CRAWL_X11_WIDTH_%s",envname);
    val = getenv(buf);
    if (val != NULL) n=atoi(val);
    if (n> (*mx)/2 && n<= (*mx)*2) *mx=n;

    sprintf(buf,"CRAWL_X11_HEIGHT_%s",envname);
    val = getenv(buf);
    if (val != NULL) n=atoi(val);
    if (n> *my && n<= (*my)*3) *my=n;
}

void init_region_crt(){
    const char *fontname;
    int mx = CRT_XMAX;
    int my = CRT_YMAX;
    /* X11 */ region_modify_size(&mx, &my, "CRT");
    /* X11 */ fontname = my_getenv((char *)"CRAWL_X11_FONT_CRT", DEFAULT_FONT_CRT);
    region_crt = new TextRegionClass(mx, my, 0, 0);
    /* X11 */ region_crt->init_font(fontname);
}

void init_region_map(){
    int dx, dy;
    dx = dy = GMAP_XDOTS;
    /* X11 */ region_modify_size(&dx, &dy, "MAP");
    region_map = new MapRegionClass(GMAP_XMAX - GMAP_XMARGIN*2,
                                    GMAP_YMAX - GMAP_YMARGIN*2,
                                    GMAP_XMARGIN, GMAP_YMARGIN,
                                    Options.rotate_minimap && Options.use_qv_mode);
    region_map->dx = dx;
    region_map->dy = dy;

    // Mega Hack!
    if (region_map->qv_mode)
    {
        region_map->dx = 2;
        region_map->dy = 1;
    }
    region_map->init_backbuf();
}

#ifdef USE_TILE
void init_region_msg(){
    const char *fontname;
    int mx = 80;
    int my = MSG_LINES;
    /* X11 */ region_modify_size(&mx, &my, "MSG");
    /* X11 */ fontname = my_getenv((char *)"CRAWL_X11_FONT_MSG", DEFAULT_FONT_MSG);
    region_msg = new TextRegionClass(mx, my, 0, 17);
    /* X11 */ region_msg->init_font(fontname);
}

void init_region_stat(){
    const char *fontname;
    int mx = 40;
    int my = 18;
    //region_modify_size(&mx, &my, "STAT");
    /* X11 */ fontname = my_getenv((char *)"CRAWL_X11_FONT_STAT", DEFAULT_FONT_STAT);
    region_stat = new TextRegionClass(mx, my, 39, 0);
    /* X11 */ region_stat->init_font(fontname);
}

void init_region_tile(){
    tile_scr_width = TILE_SCR_WIDTH_NORMAL;
    tile_scr_height = TILE_SCR_HEIGHT_NORMAL;

    if (Options.use_qv_mode)
    {
        tile_scr_width = TILE_SCR_WIDTH_QV;
        tile_scr_height = TILE_SCR_HEIGHT_QV;
    }

    region_tile = new TileRegionClass(tile_scr_width, tile_scr_height, 1, 1);
    // region_tile のバックバッファを libtile.cc で使用
    region_tile -> init_backbuf();
    ScrBufImg = region_tile ->backbuf;
}

/* X11 */
img_type ImgLoadFileSimple(const char *name)
{
        char fname[512];
        sprintf(fname,"tiles/%s.png", name); // port: tiles live in tiles/
        return ImgLoadFile(fname);
}

void update_tip_text(char *tip)
{
    if (win_main->active_layer == 0)
    {
        mpr_on(MODE_STAT);
        gotoxy(40,18);
        cprintf("%s", tip);
        clear_to_end_of_line();
        mpr_on(MODE_CRT);
    }
    else
    {
        mpr_on(MODE_CRT);
        gotoxy(1,25);
        cprintf("%s", tip);
        clear_to_end_of_line();
    }
}

void TileDrawDungeonAux()
{
}

/****** Export ************/
int x11_byte_per_pixel_ximage(){
    return 4;
}

#endif  /* USE_TILE */


static void add_keypress(int ch)
{
    keybuf[key_tail]=ch;
    key_tail++;
    if(key_tail == KEYBUFMAX)
        key_tail=0;
}

static void add_keypresses(char *ch)
{
    int i;
    int n = strlen(ch);

    for(i=0;i<n;i++)
    {
        keybuf[key_tail]=ch[i];
        key_tail++;
        if(key_tail == KEYBUFMAX)
            key_tail=0;
    }
}

/* X11 */
// This routine is taken from Angband main-x11.c
// Called by the page (web/crawl.js) for every key: ks = X11 keysym for
// special keys, else 0; ch = the character; mods 1 shift, 2 ctrl, 4 alt.
// The rest is x11_keypress() from libx11.cc.
static void web_keypress(int ks, int ch, int mods){

#define IsSpecialKey(keysym) \
  ((unsigned)(keysym) >= 0xFF00)

    const unsigned char cmd_n[9]={'b','j','n','h','.','l','y','k','u'};
    const unsigned char cmd_s[9]={'B','J','N','H','5','L','Y','K','U'};
    const unsigned char cmd_c[9]={0x2, 0xa, 0xe, 0x8, 'X', 0xc, 0x19, 0xb, 0x15};
    //const unsigned char qv_table[9]={2, 3, 6, 1, 5, 9, 4, 7, 8};
    int dir, base;

    int n;
    bool mc, ms, ma;
    unsigned int ks1;
    char buf[4];

    n = ch ? 1 : 0;
    buf[0] = ch; buf[1] = 0;
    if (!ks) ks = ch;

    mc = (mods & 2) != 0;
    ms = (mods & 1) != 0;
    ma = (mods & 4) != 0;

    /* Normal keys */
    if (n &&  !IsSpecialKey(ks))
    {
        buf[n] = 0;

	//Hack Ctrl+[0-9] etc.
	if (mc && ((ks>='0' && ks<='9') || buf[0]>=' '))
	{
	    add_keypress(KEYFLAG_CTRL|ks);
	    return;
	}

	if (!ma)
        {   add_keypresses(buf);
            return;
	}

	/* Alt + ? */
	add_keypress(KEYFLAG_ALT|buf[0]);
	return;
    }

    /* Hack -- convert into an unsigned int */
    ks1 = (uint)(ks);


    /* Handle a few standard keys (bypass modifiers) XXX XXX XXX */
    base = dir = 0;
    switch (ks1)
        {
                case XK_Escape:
                    base = '\e';
                    break;
                case XK_Return:
                case XK_KP_Enter:       // port: keypad Enter
                    base = '\r';
                    break;
                case XK_Tab:
                    base = '\t';
                    break;
                case XK_Delete:
                case XK_BackSpace:
                    base = '\010';
                    break;

                case XK_F1:
		    base = CMD_TRIGGER_F1;
		    break;
                case XK_F2:
		    base = CMD_TRIGGER_F1 + 1;
		    break;
                case XK_F3:
		    base = CMD_TRIGGER_F1 + 2;
		    break;
                case XK_F4:
		    base = CMD_TRIGGER_F1 + 3;
		    break;
                case XK_F5:
		    base = CMD_TRIGGER_F1 + 4;
		    break;
                case XK_F6:
		    base = CMD_TRIGGER_F1 + 5;
		    break;
                case XK_F7:
		    base = CMD_TRIGGER_F1 + 6;
		    break;
                case XK_F8:
		    base = CMD_TRIGGER_F1 + 7;
		    break;
                case XK_F9:
		    base = CMD_TRIGGER_F1 + 8;
		    break;
                case XK_F10:
		    base = CMD_TRIGGER_F1 + 9;
		    break;
                case XK_F11:
		    base = CMD_TRIGGER_F1 + 10;
		    break;
                case XK_F12:
		    base = CMD_TRIGGER_F1 + 11;
		    break;

                case XK_Muhenkan:
                    base = CMD_TRIGGER_MUHENKAN;
                    break;

                case XK_Henkan:
                    base = CMD_TRIGGER_HENKAN;
                    break;

		/*
		 * Keypad
		 */

                case XK_KP_0:
                case XK_KP_Insert:
                    base = CMD_EXPLORE;
                    break;

                case XK_KP_Delete:
                case XK_KP_Decimal:
                    base = (int)'.';
                    break;

                case XK_KP_1:
                case XK_KP_End:
		    dir = 1;
		    break;

                case XK_KP_2:
                case XK_KP_Down:
                case XK_Down:          // port: cursor keys move too
                    dir = 2;
                    break;

                case XK_KP_3:
                case XK_KP_Page_Down:
                    dir = 3;
                    break;

                case XK_KP_6:
                case XK_KP_Right:
                case XK_Right:          // port: cursor keys move too
                    dir = 6;
                    break;

                case XK_KP_9:
                case XK_KP_Page_Up:
                    dir = 9;
                    break;

                case XK_KP_8:
                case XK_KP_Up:
                case XK_Up:          // port: cursor keys move too
                    dir = 8;
                    break;

                case XK_KP_7:
                case XK_KP_Home:
                    dir = 7;
                    break;

                case XK_KP_4:
                case XK_KP_Left:
                case XK_Left:          // port: cursor keys move too
                    dir = 4;
                    break;

                case XK_KP_5:
                    dir = 5;
                    break;
    }/* switch */

    //Handle keypad first
    if (dir != 0)
    {
	if (rvip_raw_dirs && !ms && !mc)   // RVIP: lists and menus
	{
	    add_keypress(RVIP_KEY_DIR(dir));
	    return;
	}
	dir --;
	//if ( (Options.use_qv_mode)&&(Options.rotate_numpad) )
        //dir = qv_table[dir]-1;

	if (ms) add_keypress(cmd_s[dir]);
	else
	if (mc) add_keypress(cmd_c[dir]);
	else
	add_keypress(cmd_n[dir]);
	return;
    }

    if (base != 0)
    {
	if (ms) base |= KEYFLAG_SHIFT;
	if (mc) base |= KEYFLAG_CTRL;
	if (ma) base |= KEYFLAG_ALT;

	add_keypress(base);
	return;
    }

    //Hack Special key
    if (ks1 >=0xff00)
	add_keypress(10000 + ks1 - 0xff00);
}

// One queued page event: key (ks, ch, mods), mouse button (x, y, button,
// mods) or motion (x, y), in window coordinates.
static void web_event(const int *e)
{
    enum { EV_KEY = 1, EV_BUTTON = 2, EV_MOTION = 3 };

    if (e[0] == EV_KEY)
        web_keypress(e[1], e[2], e[3]);
    else if (e[0] == EV_BUTTON)
    {
        int button = e[3];
        bool shift = (e[4] & 1) != 0;
        bool ctrl  = (e[4] & 2) != 0;
        int key;

        if (button == 4 || button == 5)
        {
            key = (button == 4) ? CMD_MOUSE_WHEEL_UP : CMD_MOUSE_WHEEL_DOWN;
            if (shift) key |= KEYFLAG_SHIFT;
            if (ctrl)  key |= KEYFLAG_CTRL;
            add_keypress(key);
        }
        else
        {
            key = handle_mouse_button(e[1], e[2], button, shift, ctrl);
            if (key) add_keypress(key);
        }
    }
    else if (e[0] == EV_MOTION)
    {
        int key = handle_mouse_motion(e[1], e[2]);
        if (key) add_keypress(key);
    }
}

int getch(){
    int ev[5];
    int kin;

    while(key_tail == key_head)
    {
        if (js_event(ev, rvip_at_cmd))
            web_event(ev);
        else
        {
            web_present();
            emscripten_sleep(10);
        }
    }

    kin = keybuf[key_head];
    key_head++;
    if(key_head == KEYBUFMAX)key_head = 0;

    return kin;
}

int kbhit(){
    static double last = 0;

    if(key_head !=  key_tail)return 1;
    if (js_pending()) return 1;

    // running and resting poll this: let the page paint now and then
    if (emscripten_get_now() - last > 50)
    {
        web_present();
        emscripten_sleep(0);
        last = emscripten_get_now();
    }
    return js_pending();
}

void libx11_init()
{
    int  i;

    pix_col[PIX_BLACK]       =create_pixel(  0,   0,   0);
    pix_col[PIX_BLUE]        =create_pixel(  0,  0, 160);
    pix_col[PIX_GREEN]       =create_pixel(100, 185,  70);
    pix_col[PIX_CYAN]        =create_pixel(  0, 180, 180);
    pix_col[PIX_RED]         =create_pixel(160,   0,   0);
    pix_col[PIX_MAGENTA]     =create_pixel(238,  92, 238);
    pix_col[PIX_BROWN]       =create_pixel(165,  91,   0);
    pix_col[PIX_LIGHTGREY]   =create_pixel(162, 162, 162);
    pix_col[PIX_DARKGREY]    =create_pixel( 82,  82,  82);
    pix_col[PIX_LIGHTBLUE]   =create_pixel( 82, 102, 255);
    pix_col[PIX_LIGHTGREEN]  =create_pixel( 82, 255,  82);
    pix_col[PIX_LIGHTCYAN]   =create_pixel( 82, 255, 255);
    pix_col[PIX_LIGHTRED]    =create_pixel(255,  82,  82);
    pix_col[PIX_LIGHTMAGENTA]=create_pixel(255,  82, 255);
    pix_col[PIX_YELLOW]      =create_pixel(255, 255,  82);
    pix_col[PIX_WHITE]       =create_pixel(255, 255, 255);

#ifdef USE_TILE
    pix_black = pix_col[PIX_BLACK] ;
    pix_hilite = pix_col[PIX_LIGHTMAGENTA] ;
    pix_rimcolor = create_pixel(1,1,1);
#endif


    /* X11 */ init_key_to_command();
    key_head = key_tail = 0;

    win_main = new WinClass();

    if (!Options.use_tile)
    {
        // Only LIST and MAP
        init_region_crt();
        TextRegionClass::text_mode = region_crt;
        win_main->placeRegion(region_crt, 0, NULL, PLACE_RIGHT, 0, 0, 0, 0);

        init_region_map();
        win_main->placeRegion(region_map, 0, region_crt, PLACE_RIGHT, 0, 0, 0, 0);

        win_main->create("MAIN");
        return;
    }

/******************
レイアウト
---------+--wx1-+
         |      |
DNGN     | STAT |
         |      |
         +--------+
         | MAP    |
---------+        |
         |        |
---------+------+-+
MSG             |
----------------+
*******************/

    TileImg = ImgLoadFileSimple("tile");
    PlayerImg = ImgLoadFileSimple("player");

    if (Options.use_qv_mode)
    {
#ifdef QV48
        TileQvImg = ImgLoadFileSimple("tile-qv48");
#else
        TileQvImg = ImgLoadFileSimple("tile-qv");
#endif
    }
    pix_transparent = ((unsigned int *) TileImg->data)[0];
    init_tile_all();

    init_region_tile();
    init_region_stat();
    init_region_map();
    init_region_crt();
    init_region_msg();

    TextRegionClass::text_mode = region_crt;

    int text_margin = region_crt->dx;

    if (Options.use_qv_mode)
    {
        win_main->placeRegion(region_tile, 0, NULL,        PLACE_FORCE,
            0, 0, 0, 0);
        win_main->placeRegion(region_msg, 0, region_tile, PLACE_BOTTOM, 
            text_margin, text_margin, text_margin, text_margin);
        win_main->placeRegion(region_stat, 0, region_msg, PLACE_RIGHT, 
            0, 0, 0, 0);
        
        if (region_tile->ex < region_stat->ex)
            win_main->placeRegion(region_map, 0, region_tile, PLACE_RIGHT, 
                0, 0, 0, 0);
        else
            win_main->placeRegion(region_map, 0, region_stat, PLACE_RIGHT, 
                0, 0, 0, 0);
        
        win_main->placeRegion(region_crt, 1, NULL, PLACE_FORCE, 
            text_margin, text_margin, text_margin, text_margin);
    }
    else
    {
        win_main->placeRegion(region_tile, 0, NULL,        PLACE_FORCE, 
             0, 0, 0, 0);
        win_main->placeRegion(region_msg, 0, region_tile, PLACE_BOTTOM,
             text_margin, text_margin, text_margin, text_margin);
        win_main->placeRegion(region_stat, 0, region_tile, PLACE_RIGHT, 
             text_margin, text_margin, 0, text_margin);
        win_main->placeRegion(region_map, 0, region_stat, PLACE_BOTTOM,    
            0, 0, 0, 0);
        win_main->placeRegion(region_crt, 1, NULL, PLACE_FORCE,
             text_margin, text_margin, text_margin, text_margin);
    }

#if 1 //Slot
    int item_x = region_crt->ex / TILE_X;
    int item_y = (52 + item_x -1) / item_x;

    region_item2 = new TileRegionClass(item_x, item_y, TILE_X, TILE_Y);
    win_main->placeRegion (region_item2, 1, region_crt, PLACE_BOTTOM,
                           0, 0, 0, 0);
    region_item2->init_backbuf();

    if (Options.show_items[0] != 0)
    {
        // web: the page shows the items in their own window, a fixed
        // grid (8 x 8 >= MAX_ITEMLIST) instead of the X11 leftover space
        region_item = new TileRegionClass(8, 8, TILE_X, TILE_Y);
        win_main->placeRegion (region_item, 0, region_map, PLACE_BOTTOM,
                           0, 0, 0, 0);
        region_item->init_backbuf();
    }
#endif

    win_main->create((char *)"MAIN");
}


/* X11 */
void libx11_shutdown()
{
    int i;

#ifdef USE_TILE
    if(ScrBufImg) ImgDestroy(ScrBufImg);
    if(PlayerImg) ImgDestroy(PlayerImg);
    if(TileQvImg) ImgDestroy(TileQvImg);
#endif


    std::vector<RegionClass *>::iterator r;
    for (r = win_main->regions.begin();r != win_main->regions.end();r++)
        delete (*r);

    delete win_main;

}

/* X11 */
void update_screen(){
    web_present();
}

/** from liblinux.cc **/
void init_key_to_command()
{
    int i;
    for (i=0; i < 256; i++) key_to_command_table[i] = CMD_NO_CMD;

    // lower case
    key_to_command_table[(int)'a'] = CMD_USE_ABILITY;
    key_to_command_table[(int)'b'] = CMD_MOVE_DOWN_LEFT;
    key_to_command_table[(int)'c'] = CMD_CLOSE_DOOR;
    key_to_command_table[(int)'d'] = CMD_DROP;
    key_to_command_table[(int)'e'] = CMD_EAT;
    key_to_command_table[(int)'f'] = CMD_FIRE;
    key_to_command_table[(int)'g'] = CMD_PICKUP;
    key_to_command_table[(int)'h'] = CMD_MOVE_LEFT;
    key_to_command_table[(int)'i'] = CMD_DISPLAY_INVENTORY;
    key_to_command_table[(int)'j'] = CMD_MOVE_DOWN;
    key_to_command_table[(int)'k'] = CMD_MOVE_UP;
    key_to_command_table[(int)'l'] = CMD_MOVE_RIGHT;
    key_to_command_table[(int)'m'] = CMD_DISPLAY_SKILLS;
    key_to_command_table[(int)'n'] = CMD_MOVE_DOWN_RIGHT;
    key_to_command_table[(int)'o'] = CMD_OPEN_DOOR;
    key_to_command_table[(int)'p'] = CMD_PRAY;
    key_to_command_table[(int)'q'] = CMD_QUAFF;
    key_to_command_table[(int)'r'] = CMD_READ;
    key_to_command_table[(int)'s'] = CMD_SEARCH;
    key_to_command_table[(int)'t'] = CMD_THROW;
    key_to_command_table[(int)'u'] = CMD_MOVE_UP_RIGHT;
    key_to_command_table[(int)'v'] = CMD_EXAMINE_OBJECT;
    key_to_command_table[(int)'w'] = CMD_WIELD_WEAPON;
    key_to_command_table[(int)'x'] = CMD_LOOK_AROUND;
    key_to_command_table[(int)'y'] = CMD_MOVE_UP_LEFT;
    key_to_command_table[(int)'z'] = CMD_ZAP_WAND;

    // upper case
    key_to_command_table[(int)'A'] = CMD_DISPLAY_MUTATIONS;
    key_to_command_table[(int)'B'] = CMD_RUN_DOWN_LEFT;
    key_to_command_table[(int)'C'] = CMD_EXPERIENCE_CHECK;
    key_to_command_table[(int)'D'] = CMD_BUTCHER;
    key_to_command_table[(int)'E'] = CMD_EVOKE;
    key_to_command_table[(int)'F'] = CMD_NO_CMD;
    key_to_command_table[(int)'G'] = CMD_NO_CMD;
    key_to_command_table[(int)'H'] = CMD_RUN_LEFT;
    key_to_command_table[(int)'I'] = CMD_OBSOLETE_INVOKE;
    key_to_command_table[(int)'J'] = CMD_RUN_DOWN;
    key_to_command_table[(int)'K'] = CMD_RUN_UP;
    key_to_command_table[(int)'L'] = CMD_RUN_RIGHT;
    key_to_command_table[(int)'M'] = CMD_MEMORISE_SPELL;
    key_to_command_table[(int)'N'] = CMD_RUN_DOWN_RIGHT;
    key_to_command_table[(int)'O'] = CMD_DISPLAY_OVERMAP;
    key_to_command_table[(int)'P'] = CMD_WEAR_JEWELLERY;
    key_to_command_table[(int)'Q'] = CMD_QUIT;
    key_to_command_table[(int)'R'] = CMD_REMOVE_JEWELLERY;
    key_to_command_table[(int)'S'] = CMD_SAVE_GAME;
    key_to_command_table[(int)'T'] = CMD_REMOVE_ARMOUR;
    key_to_command_table[(int)'U'] = CMD_RUN_UP_RIGHT;
    key_to_command_table[(int)'V'] = CMD_GET_VERSION;
    key_to_command_table[(int)'W'] = CMD_WEAR_ARMOUR;
    key_to_command_table[(int)'X'] = CMD_DISPLAY_MAP;
    key_to_command_table[(int)'Y'] = CMD_RUN_UP_LEFT;
    key_to_command_table[(int)'Z'] = CMD_CAST_SPELL;

    // control
    key_to_command_table[(int) CONTROL('A') ] = CMD_TOGGLE_AUTOPICKUP;
    key_to_command_table[(int) CONTROL('B') ] = CMD_OPEN_DOOR_DOWN_LEFT;
    key_to_command_table[(int) CONTROL('C') ] = CMD_NO_CMD;

#ifdef ALLOW_DESTROY_ITEM_COMMAND
    key_to_command_table[(int) CONTROL('D') ] = CMD_DESTROY_ITEM;
#else
    key_to_command_table[(int) CONTROL('D') ] = CMD_NO_CMD;
#endif

    key_to_command_table[(int) CONTROL('E') ] = CMD_FORGET_STASH;
    key_to_command_table[(int) CONTROL('F') ] = CMD_FIX_WAYPOINT;
    key_to_command_table[(int) CONTROL('G') ] = CMD_INTERLEVEL_TRAVEL;
    key_to_command_table[(int) CONTROL('H') ] = CMD_OPEN_DOOR_LEFT;
    key_to_command_table[(int) CONTROL('I') ] = CMD_NO_CMD;
    key_to_command_table[(int) CONTROL('J') ] = CMD_OPEN_DOOR_DOWN;
    key_to_command_table[(int) CONTROL('K') ] = CMD_OPEN_DOOR_UP;
    key_to_command_table[(int) CONTROL('L') ] = CMD_OPEN_DOOR_RIGHT;
    key_to_command_table[(int) CONTROL('M') ] = CMD_NO_CMD;
    key_to_command_table[(int) CONTROL('N') ] = CMD_OPEN_DOOR_DOWN_RIGHT;
    key_to_command_table[(int) CONTROL('O') ] = CMD_EXPLORE;
    key_to_command_table[(int) CONTROL('P') ] = CMD_REPLAY_MESSAGES;
    key_to_command_table[(int) CONTROL('Q') ] = CMD_NO_CMD;
    key_to_command_table[(int) CONTROL('R') ] = CMD_REDRAW_SCREEN;
    key_to_command_table[(int) CONTROL('S') ] = CMD_MARK_STASH;
    key_to_command_table[(int) CONTROL('T') ] = CMD_NO_CMD;
    key_to_command_table[(int) CONTROL('U') ] = CMD_OPEN_DOOR_UP_LEFT;
    key_to_command_table[(int) CONTROL('V') ] = CMD_NO_CMD;
    key_to_command_table[(int) CONTROL('W') ] = CMD_NO_CMD;
    key_to_command_table[(int) CONTROL('X') ] = CMD_SAVE_GAME_NOW;
    key_to_command_table[(int) CONTROL('Y') ] = CMD_OPEN_DOOR_UP_RIGHT;
    key_to_command_table[(int) CONTROL('Z') ] = CMD_SUSPEND_GAME;

    // other printables
    key_to_command_table[(int)'.'] = CMD_MOVE_NOWHERE;
    key_to_command_table[(int)'<'] = CMD_GO_UPSTAIRS;
    key_to_command_table[(int)'>'] = CMD_GO_DOWNSTAIRS;
    key_to_command_table[(int)'@'] = CMD_DISPLAY_CHARACTER_STATUS;
    key_to_command_table[(int)','] = CMD_PICKUP;
    key_to_command_table[(int)';'] = CMD_INSPECT_FLOOR;
    key_to_command_table[(int)'!'] = CMD_SHOUT;
    key_to_command_table[(int)'^'] = CMD_DISPLAY_RELIGION;
    key_to_command_table[(int)'#'] = CMD_CHARACTER_DUMP;
    key_to_command_table[(int)'='] = CMD_ADJUST_INVENTORY;
    key_to_command_table[(int)'?'] = CMD_DISPLAY_COMMANDS;
    key_to_command_table[(int)'`'] = CMD_MACRO_ADD;
    key_to_command_table[(int)'~'] = CMD_MACRO_SAVE;
    key_to_command_table[(int)'&'] = CMD_WIZARD;
    key_to_command_table[(int)'"'] = CMD_LIST_JEWELLERY;


    // I'm making this both, because I'm tried of changing it
    // back to '[' (the character that represents armour on the map) -- bwr
    key_to_command_table[(int)'['] = CMD_LIST_ARMOUR;
    key_to_command_table[(int)']'] = CMD_LIST_ARMOUR;

    // This one also ended up backwards this time, so it's also going on
    // both now -- should be ')'... the same character that's used to
    // represent weapons.
    key_to_command_table[(int)')'] = CMD_LIST_WEAPONS;
    key_to_command_table[(int)'('] = CMD_LIST_WEAPONS;

    key_to_command_table[(int)'\\'] = CMD_DISPLAY_KNOWN_OBJECTS;
    key_to_command_table[(int)'\''] = CMD_WEAPON_SWAP;

    // digits
    key_to_command_table[(int)'1'] = CMD_MOVE_DOWN_LEFT;
    key_to_command_table[(int)'2'] = CMD_MOVE_DOWN;
    key_to_command_table[(int)'3'] = CMD_MOVE_DOWN_RIGHT;
    key_to_command_table[(int)'4'] = CMD_MOVE_LEFT;
    key_to_command_table[(int)'5'] = CMD_REST;
    key_to_command_table[(int)'6'] = CMD_MOVE_RIGHT;
    key_to_command_table[(int)'7'] = CMD_MOVE_UP_LEFT;
    key_to_command_table[(int)'8'] = CMD_MOVE_UP;
    key_to_command_table[(int)'9'] = CMD_MOVE_UP_RIGHT;

    // these are invalid keys, but to help kludge running
    // pass them through unmolested
    key_to_command_table[(int)128] = 128;
    key_to_command_table[(int)'*'] = '*';
    key_to_command_table[(int)'/'] = '/';

    key_to_command_table[(int)'-'] = '-';

}

int key_to_command(int keyin)
{
    if (keyin >= CMD_NO_CMD) return keyin;
    return (key_to_command_table[keyin]);
}

static inline short macro_colour( short col )
{
    return (Options.colour[ col ]);
}

/* Convert value to string */
int itoa(int value, char *strptr, int radix)
{
    unsigned int bitmask = 32768;
    int ctr = 0;
    int startflag = 0;

    if (radix == 10)
    {
        sprintf(strptr, "%i", value);
    }
    if (radix == 2)             /* int to "binary string" */
    {
        while (bitmask)
        {
            if (value & bitmask)
            {
                startflag = 1;
                sprintf(strptr + ctr, "1");
            }
            else
            {
                if (startflag)
                    sprintf(strptr + ctr, "0");
            }

            bitmask = bitmask >> 1;
            if (startflag)
                ctr++;
        }

        if (!startflag)         /* Special case if value == 0 */
            sprintf((strptr + ctr++), "0");

        strptr[ctr] = (char) NULL;
    }
    return (0);                /* Me? Fail? Nah. */
}


// Convert string to lowercase.
char *strlwr(char *str)
{
    unsigned int i;

    for (i = 0; i < strlen(str); i++)
        str[i] = tolower(str[i]);

    return (str);
}


int stricmp( const char *str1, const char *str2 )
{
    return (strcmp(str1, str2));
}


void delay( unsigned long time )
{
    web_present();
    emscripten_sleep( time );
}

/*
 * web: regions -> page windows
 */

// roles as web/crawl.js knows them
enum { R_TILE, R_MSG, R_STAT, R_MAP, R_ITEM, R_CRT, R_ITEM2, R_COUNT };

static RegionClass *web_region(int role)
{
    switch (role)
    {
    case R_TILE:  return region_tile;
    case R_MSG:   return region_msg;
    case R_STAT:  return region_stat;
    case R_MAP:   return region_map;
    case R_ITEM:  return region_item;
    case R_CRT:   return region_crt;
    case R_ITEM2: return region_item2;
    }
    return NULL;
}

static bool dirty[R_COUNT];

void web_dirty(RegionClass *r)
{
    for (int i = 0; i < R_COUNT; i++)
        if (web_region(i) == r)
            dirty[i] = true;
}

EM_JS(void, js_region, (int role, int layer, int flag, int dirty, int text,
                        int ox, int oy, int dx, int dy,
                        int w, int h, void *a, void *b), {
    Module.cr.region(role, layer, flag, dirty, text, ox, oy, dx, dy, w, h, a, b);
});
EM_JS(void, js_prompt, (const char *s), { Module.cr.prompt(UTF8ToString(s)); });
EM_JS(void, js_present, (int layer, int cur_role, int cx, int cy, int cw), {
    Module.cr.present(layer, cur_role, cx, cy, cw);
});

// Visible window (rvip-wm.js): "M<glyph><name>\t<colour>" per monster in
// sight, "I<glyph><name>\t<colour>" per item on a square in view (colours:
// libx11 palette indexes, crawl.js maps them)
EM_JS(void, js_vis, (const char *s), { Module.cr.vis(UTF8ToString(s)); });
static void send_visible()
{
    static char vis[16384];
    static const char GLYPH[] = ")([/%?? =!?+\\0}%$*";
    char *p = vis, *e = vis + sizeof vis - 200, name[ITEMNAME_SIZE];

    *p = 0;
    for (int i = 0; i < MAX_MONSTERS && p < e; i++)
    {
        monsters *m = &menv[i];
        if (m->type == -1 || mgrd[m->x][m->y] != i || mons_is_mimic(m->type) || !mons_near(m) || !player_monster_visible(m))
            continue;
        p += sprintf(p, "M%c%.60s\t%d\n", mons_char(m->type), ptr_monam(m, DESC_PLAIN), mons_colour(m->type) & 15);
    }
    for (int i = 0; i < MAX_ITEMS && p < e; i++)
    {
        item_def &it = mitm[i];
        if (it.base_type == OBJ_UNASSIGNED || it.quantity < 1 || it.x < 1 || !see_grid(it.x, it.y))
            continue;
        item_name(it, DESC_PLAIN, name);
        p += sprintf(p, "I%c%.80s\t%d\n", it.base_type < sizeof GLYPH - 1 ? GLYPH[it.base_type] : '*', name, it.colour & 15);
    }
    js_vis(vis);
}

// Hands every region's state to the page, which redraws the dirty ones.
void web_present()
{
    if (!win_main)
        return;
    send_visible();

    int cur_role = -1, cx = 0, cy = 0;
    for (int i = 0; i < R_COUNT; i++)
    {
        RegionClass *r = web_region(i);
        if (!r)
            continue;

        TextRegionClass *t = dynamic_cast<TextRegionClass *>(r);
        if (t)
        {
            js_region(i, r->layer, r->flag, dirty[i], 1, r->ox, r->oy, r->dx, r->dy,
                      t->mx, t->my, t->cbuf, t->abuf);
            if (TextRegionClass::old_cursor_region == t
                && TextRegionClass::cursor_flag == 1)
            {
                cur_role = i;       // cursor in the region's own cells
                cx = TextRegionClass::old_cursor_x - t->cx_ofs;
                cy = TextRegionClass::old_cursor_y - t->cy_ofs;
            }
        }
        else if (r->backbuf)
            js_region(i, r->layer, r->flag, dirty[i], 0, r->ox, r->oy, r->dx, r->dy,
                      r->backbuf->width, r->backbuf->height, r->backbuf->data, NULL);
        dirty[i] = false;
    }
    js_present(win_main->active_layer, cur_role, cx, cy,
               TextRegionClass::old_cursor_width);
    // the message row the game writes to (a question waits there): the prompt line over the map
    {
        TextRegionClass *t = TextRegionClass::text_mode;
        char line[256];
        int n = 0;
        if (t && t == region_msg && t->flag)
        {
            int row = TextRegionClass::cursor_y - t->cy_ofs;
            if (row >= 0 && row < t->my)
                for (int x = 0; x < t->mx && n < 255; x++)
                    line[n++] = t->cbuf[row * t->mx + x] & 0x7f ? t->cbuf[row * t->mx + x] & 0x7f : ' ';
        }
        line[n] = 0;
        js_prompt(line);
    }
}

/*
 * web: saves (IndexedDB via IDBFS, web/crawl.js)
 */
#include "files.h"
void save_level(int level_saved, bool was_a_labyrinth, char where_were_you);

EM_JS(void, web_sync_files, (void), { Module.cr.sync(); });
EM_JS(int, js_want_save, (void), { return Module.cr.wantSave(); });
EM_JS(void, js_end, (int code), { Module.cr.end(code); });

/* Run report (roguelikes-index/server/CONTRACT.md): fire-and-forget GET,
   never throws, offline just fails silently. Negative ints are omitted. */
EM_JS(void, js_beacon, (const char *g, const char *ev, const char *name, const char *killer, int depth, int score, int turns, int lvl), {
    try {
        var p = [['g', UTF8ToString(g)], ['ev', UTF8ToString(ev)], ['name', name ? UTF8ToString(name) : ''],
                 ['killer', killer ? UTF8ToString(killer) : ''], ['depth', depth], ['score', score], ['turns', turns], ['lvl', lvl]];
        var q = p.filter(function (a) { return a[1] !== '' && !(a[1] < 0); })
                 .map(function (a) { return a[0] + '=' + encodeURIComponent(a[1]); }).join('&');
        fetch('/roguelikes/beacon?' + q, { keepalive: true, mode: 'no-cors' }).catch(function () {});
    } catch (e) {}
});

// Called from end_game() (ouch.cc) with the finished scorefile entry.
void web_run_end(const struct scorefile_entry &se)
{
    const char *k = se.death_source_name, *ev = "death";
    if (se.death_type == KILLED_BY_WINNING) ev = "win", k = NULL;
    else if (se.death_type == KILLED_BY_QUITTING || se.death_type == KILLED_BY_LEAVING) ev = "quit", k = NULL;
    else if (!strncmp(k, "a ", 2)) k += 2;
    else if (!strncmp(k, "an ", 3)) k += 3;
    else if (!strncmp(k, "the ", 4)) k += 4;
    js_beacon("crawl-linley", ev, you.your_name, k, you.your_level + 1, (int)se.points, (int)se.num_turns, se.lvl);
}

// At the command prompt (rvip_getkey): autosave when the page asks.
void web_autosave()
{
    if (!js_want_save())
        return;
    save_level(you.your_level, (you.level_type != LEVEL_DUNGEON),
               you.where_are_you);
    save_game(false);
}

void web_end(int code)
{
    js_end(code);
}
