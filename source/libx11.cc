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

#ifdef USE_X11 //Sys dep
#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xmd.h>
#endif

//Options
#include "externs.h"

//ウィンドウ＆領域のクラス
#include "winclass.h"

/*
 * Tile related stuff
 */
#ifdef USE_TILE

/* X11 */ extern XImage *read_png (char *fname);

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
/* X11 */ Display *display=NULL;
/* X11 */ int screen;
/* X11 */ GC xgc[MAX_PIX_COLOR];

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
    Colormap cmap = DefaultColormapOfScreen(DefaultScreenOfDisplay(display));
    XColor xcolour;

    xcolour.red = red * 256;
    xcolour.green = green * 256;
    xcolour.blue = blue * 256;
    xcolour.flags = DoRed | DoGreen | DoBlue;

    XAllocColor(display, cmap, &xcolour);
    return (xcolour.pixel);
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
        sprintf(fname,"%s%s.png", SAVE_DIR_PATH, name);
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
    int i = 1;
    int j = (DefaultDepth(display, screen) - 1) >> 2;
    while (j >>= 1) i <<= 1;
    return i;
}

#endif  /* USE_TILE */

/* X11 */
void x11_check_exposure(XEvent *xev){
    int sx, sy, ex, ey;

    sx = xev->xexpose.x;
    ex = xev->xexpose.x + (xev->xexpose.width)-1;
    sy = xev->xexpose.y;
    ey = xev->xexpose.y + (xev->xexpose.height)-1;

    if (xev->xany.window != win_main->win) return;

    win_main->redraw(sx,sy,ex,ey);
}

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
static void x11_keypress(XKeyEvent *xev){

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

    XKeyEvent *ev = (XKeyEvent*)(xev);
    KeySym ks;
    char buf[256];

    n = XLookupString(ev, buf, 125, &ks, NULL);
    buf[n] = '\0';

    if (IsModifierKey(ks)) return;

    /* Extract "modifier flags" */
    mc = (ev->state & ControlMask) ? true : false;
    ms = (ev->state & ShiftMask) ? true : false;
    ma = (ev->state & Mod1Mask) ? true : false;

//DEBUGLOG("KS %x(%c, %x)  c%d s%d a%d\n",ks,ks,buf[0],mc,ms,ma);

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
                    dir = 2;
                    break;

                case XK_KP_3:
                case XK_KP_Page_Down:
                    dir = 3;
                    break;

                case XK_KP_6:
                case XK_KP_Right:
                    dir = 6;
                    break;

                case XK_KP_9:
                case XK_KP_Page_Up:
                    dir = 9;
                    break;

                case XK_KP_8:
                case XK_KP_Up:
                    dir = 8;
                    break;

                case XK_KP_7:
                case XK_KP_Home:
                    dir = 7;
                    break;

                case XK_KP_4:
                case XK_KP_Left:
                    dir = 4;
                    break;

                case XK_KP_5:
                    dir = 5;
                    break;
    }/* switch */

    //Handle keypad first
    if (dir != 0)
    {
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

/* X11 */
int getch(){
    XEvent xev;
    int kin;

    while(key_tail == key_head)
    {
        XNextEvent(display, &xev);

        if(xev.type==KeyPress)
	    x11_keypress(&(xev.xkey));
        else 
	if(xev.type==Expose) 
	    x11_check_exposure(&xev);
	else if(xev.type==ButtonPress)
        {
            int button = xev.xbutton.button;
            bool shift = (xev.xkey.state & ShiftMask)   ? true : false;
            bool ctrl  = (xev.xkey.state & ControlMask) ? true : false;
            int x = xev.xbutton.x;
            int y = xev.xbutton.y;
            int key;

            if (button == 4 || button == 5)
            {
                // Wheel mouse
                if (button == 4)
                    key = CMD_MOUSE_WHEEL_UP;
                else
                    key = CMD_MOUSE_WHEEL_DOWN;

                if (shift) key |= KEYFLAG_SHIFT;
                if (ctrl)  key |= KEYFLAG_CTRL;
                add_keypress(key); 
            }
            else
            {
                if (button == 3) button = 2;
                else if (button==2) button=3;

                key = handle_mouse_button(x, y, button, shift, ctrl);
                if (key) add_keypress(key);
            }
        }
	else if(xev.type==MotionNotify)
        {
            int x = xev.xbutton.x;
            int y = xev.xbutton.y;
            int key = handle_mouse_motion(x, y);
            if (key) add_keypress(key);
        }
    }/*while*/

    kin = keybuf[key_head];
    key_head++;
    if(key_head == KEYBUFMAX)key_head = 0;

    return kin;
}

int kbhit(){
    XEvent xev;

    if(key_head !=  key_tail)return 1;

    if (XCheckMaskEvent(display, 
        KeyPressMask | ButtonPressMask, &xev))
    {
        XPutBackEvent(display, &xev);
        return 1;
    }
    return 0;
}

void libx11_init()
{
    int  i;
    setlocale(LC_ALL, "");
    display= XOpenDisplay("");
    if (!display)
    {
        fprintf(stderr,"Cannot open display\n");
        exit(1);
    }
    screen=DefaultScreen(display);

    for(i=0;i<MAX_PIX_COLOR;i++)
	xgc[i]=XCreateGC(display,RootWindow(display,screen),0,0);

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

    for(i=0;i<MAX_PIX_COLOR;i++) XSetForeground(display,xgc[i], pix_col[i]);

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
    pix_transparent = XGetPixel(TileImg, 0, 0);
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

        if (Options.use_qv_mode)
        {
            item_x = (region_msg->ex) / TILE_X;
            item_y = (win_main->wy - region_msg->ey) / TILE_Y;
            while(item_x * item_y < 40) item_y++;
            region_item = new TileRegionClass(item_x, item_y, TILE_X, TILE_Y);
            win_main->placeRegion (region_item, 0, region_msg, PLACE_BOTTOM,
                               0, 0, 0, 0);
        }
        else
        if (region_msg->ex < region_tile->ex)
        {
            item_x = (win_main->wx - region_map->sx) / TILE_X;
            item_y = (win_main->wy - region_map->ey) / TILE_Y;
            while(item_x * item_y < 40) item_y++;

            region_item = new TileRegionClass(item_x, item_y, TILE_X, TILE_Y);
            win_main->placeRegion (region_item, 0, region_map, PLACE_BOTTOM,
                               0, 0, 0, 0);
        }
        else
        {
            item_x = (win_main->wx - region_msg->ex) / TILE_X;
            item_y = (win_main->wy - region_msg->sy) / TILE_Y;
            while(item_x * item_y < 40) item_y++;

            region_item = new TileRegionClass(item_x, item_y, TILE_X, TILE_Y);
            win_main->placeRegion (region_item, 0, region_msg, PLACE_RIGHT,
                               0, 0, 0, 0);
        }
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

    for(i=0;i<MAX_PIX_COLOR;i++)
        XFreeGC(display,xgc[i]);

    std::vector<RegionClass *>::iterator r;
    for (r = win_main->regions.begin();r != win_main->regions.end();r++)
        delete (*r);

    delete win_main;

    XCloseDisplay(display);
}

/* X11 */
void update_screen(){
    XFlush(display);
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
    usleep( time * 1000 );
}
