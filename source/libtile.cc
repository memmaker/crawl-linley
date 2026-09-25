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

// MODE_STAT, etc
#include "defines.h"
//Options
#include "externs.h"
// random2, one_in, etc.
#include "stuff.h"

#include "tiles.h"
#include "tiledef.h"
#include "tiledef-p.h"
#include "tiledef-qv.h"

#include "winclass.h"

//extern

//direct.cc
extern char *look_directly(int cx, int cy);

// libx11.cc or libwt.cc
extern void update_tip_text(char *tip);
extern img_type ImgLoadFileSimple(const char *name);
extern void TileDrawDungeonAux();

// Raw tile images
extern img_type TileImg;
extern img_type TileQvImg;
extern img_type PlayerImg;
extern img_type ScrBufImg;

extern WinClass *win_main;
// Regions
extern TileRegionClass *region_tile;
extern TextRegionClass *region_crt;
extern TextRegionClass *region_stat;
extern TextRegionClass *region_msg;
extern MapRegionClass  *region_map;

#if 1 //Slot
extern TileRegionClass *region_item;
extern TileRegionClass *region_item2;
//itemname.cc
void in_name(int inn, char des, char buff[ITEMNAME_SIZE], bool terse = false);
void it_name(int itn, char des, char buff[ITEMNAME_SIZE], bool terse = false);

#define MAX_ITEMLIST 60
static int itemlist[MAX_ITEMLIST];
static int itemlist_num[MAX_ITEMLIST];
static int itemlist_idx[MAX_ITEMLIST];
static int itemlist_iflag[MAX_ITEMLIST];
static int itemlist_flag = -1;
#endif


//Internal
static img_type DollCacheImg;

static void tile_draw_grid(int kind, int xx, int yy);
static void clear_tcache();
static void init_tcache();
static void init_tileflag();

static void tcache_compose_qv(int kind, int ix, int *fg_dat, int *bg_dat);
static void tcache_compose_normal(int ix, int *fg, int *bg);

#if 1 // TILE_MON_EQUIP
static void mcache_init();
#endif

//Internal variables

static bool force_redraw_tile;
static unsigned short int t1buf[TILE_DAT_XMAX+2][TILE_DAT_YMAX+2],
                          t2buf[TILE_DAT_XMAX+2][TILE_DAT_YMAX+2];

//Tile cursor 表示されている位置、フラグ
static int tile_cursor_x;
static int tile_cursor_y;
static int tile_cursor_flag=0;
// 以前表示していた値。移動時のバックアップ
static int old_cursor_flag = 0;


#if 1  // MOUSE
//マウスポインタのある位置
static int tile_cursor_x0 = 0;
static int tile_cursor_y0 = 0;

//入力モードによりマウス挙動変化
//メイン側から set_keyin_mode(mode)で変更
static int keyin_mode = KEYIN_MODE_NONE;
extern bool player_monster_visible( struct monsters *mon );
extern void describe_monsters(int class_described, unsigned char which_mons);
#endif

/******** Cache buffer for transparency operation *****/
static int tcache_kind;

#if 1  // QV
#define TCACHE_KIND_QV 2
#define TCACHE0_QV 0
#define TCACHE1_QV 1
#endif

#define TCACHE_KIND_NORMAL 1
#define TCACHE0_NORMAL 0

//Internal Image buffer
static img_type *tcache_image;

//Start of a pointer string
static int *tcache_head;

typedef struct tile_cache
{
    unsigned int id[4];
    int idx;
    tile_cache *next;
    tile_cache *prev;
} tile_cache;

// number of tile grids
static int tile_xmax;
static int tile_ymax;

static int max_tcache;
static int tc_hash;
// [kind * tc_hash][max_tcache]
static tile_cache **tcache;
// [kind][x*y]
static int **screen_tcach_idx;

#if 1 //QV
/*
 screen grid arrangement. (x,y,kind)
 (0,0,B) (1,0,A) (2,0,B)
 (0,0,A) (1,0,B) (2,0,A) ...
 (0,1,B) (1,1,A) (2,1,B)
 (0,1,A) (1,1,B) (2,1,A)
       ....
*/
const int tcache_wx_qv[TCACHE_KIND_QV] = {TILE_X_EX_QV/2, TILE_X_EX_QV/2};
const int tcache_wy_qv[TCACHE_KIND_QV] = {TILE_Y_EX_QV/4, TILE_Y_EX_QV/4};
const int tcache_ox_qv[TCACHE_KIND_QV] = {0,0};
const int tcache_oy_qv[TCACHE_KIND_QV] = {0,TILE_Y_EX_QV/4};
const int tcache_nlayer_qv[TCACHE_KIND_QV] = {4,4};
#endif

const int tcache_wx_normal[TCACHE_KIND_NORMAL] = {TILE_X};
const int tcache_wy_normal[TCACHE_KIND_NORMAL] = {TILE_Y};
const int tcache_ox_normal[TCACHE_KIND_NORMAL] = {0};
const int tcache_oy_normal[TCACHE_KIND_NORMAL] = {0};
const int tcache_nlayer_normal[TCACHE_KIND_NORMAL] = {1};

unsigned char tile_flag[TILE_TOTAL2];
#define TFLAG_A 0x80
#define TFLAG_B 0x40
#define TFLAG_C 0x20
#define TFLAG_D 0x10
#define TFLAG_F 0x08
#define TFLAG_G 0x04
#define TFLAG_H 0x02
#define TFLAG_I 0x01

#define tile_check_blank(tile,region) \
    if( (tile_flag[tile & TILE_FLAG_MASK]&(region))==0) (tile) &= ~TILE_FLAG_MASK;

#if 1 //QV
#define TREGION_A_QV 0
#define TREGION_B_QV 1
#define TREGION_C_QV 2
#define TREGION_D_QV 3
#define TREGION_E_QV 4
#define TREGION_F_QV 5
#define TREGION_G_QV 6
#define TREGION_H_QV 7
/*
<-64->
 AAEE |
 BBFF 64
 CCGG |
 DDHH |
*/

const int region_sx_qv[8]=
    {0, 0, 0, 0,
    TILE_X_EX_QV/2, TILE_X_EX_QV/2, TILE_X_EX_QV/2, TILE_X_EX_QV/2};
const int region_sy_qv[8]=
    {0, TILE_Y_EX_QV/4, TILE_Y_EX_QV/2, (TILE_Y_EX_QV*3)/4,
     0, TILE_Y_EX_QV/4, TILE_Y_EX_QV/2, (TILE_Y_EX_QV*3)/4};
const int region_wx_qv[8]=
    {TILE_X_EX_QV/2, TILE_X_EX_QV/2, TILE_X_EX_QV/2, TILE_X_EX_QV/2,
     TILE_X_EX_QV/2, TILE_X_EX_QV/2, TILE_X_EX_QV/2, TILE_X_EX_QV/2};
const int region_wy_qv[8]=
    {TILE_Y_EX_QV/4, TILE_Y_EX_QV/4, TILE_Y_EX_QV/4, TILE_Y_EX_QV/4,
     TILE_Y_EX_QV/4, TILE_Y_EX_QV/4, TILE_Y_EX_QV/4, TILE_Y_EX_QV/4};

/* 32x32 の場合  タイルの(sx,sy)からwx*wy の領域を ox,oy ずらしてコピー */

#ifdef QV48
//大きいBMPが48x48 の場合
const int region_sx_small_qv[8]=
    {0, 0, 0, 0,
     TILE_X/2, TILE_X/2, TILE_X/2, TILE_X/2};
const int region_sy_small_qv[8]=
    {0, 2, 14, 26, 0, 2, 14, 26};
const int region_wx_small_qv[8]=
    {16,16,16,16, 16,16,16,16};
const int region_wy_small_qv[8]=
    {2, 12, 12, 6, 2, 12, 12, 6};
const int region_ox_small_qv[8]=
    {8,8,8, 8, 0, 0, 0, 0};
const int region_oy_small_qv[8]=
    {14,0,0,0, 14,0,0,0};
#else
//大きいBMPが64x64 の場合
const int region_sx_small_qv[8]=
    {0, 0, 0, 0,
     TILE_X/2, TILE_X/2, TILE_X/2, TILE_X/2};
const int region_sy_small_qv[8]=
    {0, 0, 8,24, 0, 0, 8,24};
const int region_wx_small_qv[8]={0,16,16,16, 0,16,16,16};
const int region_wy_small_qv[8]={0, 8,16, 8, 0, 8,16, 8};
const int region_ox_small_qv[8]={16,16,16, 16, 0, 0, 0, 0};
const int region_oy_small_qv[8]={0,8,0,0,0,8,0,0};
#endif

#endif // QV

#define TREGION_0_NORMAL 0
const int region_sx_normal[1]={0};
const int region_sy_normal[1]={0};
const int region_wx_normal[1]={TILE_X};
const int region_wy_normal[1]={TILE_Y};

// QV mode sink mask
static char *sink_mask;
static char *cursor_mask;

/* BG tile priority */
static unsigned char tile_prio[TILE_TOTAL2];
#define PRIO_WALL 10
#define tile_is_wall(tile) (tile_prio[(tile)&TILE_FLAG_MASK]==PRIO_WALL)

#ifdef USE_X11
#define DEBUGLOG( args...) {FILE *fp=fopen("log","a");fprintf(fp, ##args);\
                             fclose(fp);}
#endif


/********* subroutines ****************/

// TileImg macro
void ImgCopyFromTileImg(int idx, img_type dest, int dx, int dy, int copy,
                        char *mask = NULL, bool hilite = false)
{
    int sx = (idx % TILE_PER_ROW)*TILE_X;
    int sy = (idx / TILE_PER_ROW)*TILE_Y;
    if (hilite)
    {
        if (mask != NULL)
            ImgCopyMaskedH(TileImg, sx, sy, TILE_X, TILE_Y,
                           dest, dx, dy, mask);
        else
            ImgCopyH(TileImg, sx, sy, TILE_X, TILE_Y, dest, dx, dy, copy);
    }
    else
    {
        if (mask != NULL)
            ImgCopyMasked(TileImg, sx, sy, TILE_X, TILE_Y,
                           dest, dx, dy, mask);
        else
            ImgCopy(TileImg, sx, sy, TILE_X, TILE_Y, dest, dx, dy, copy);
    }
}

void ImgCopyToTileImg(int idx, img_type src, int sx, int sy, int copy,
                        char *mask = NULL, bool hilite = false)
{
    int dx = (idx % TILE_PER_ROW)*TILE_X;
    int dy = (idx / TILE_PER_ROW)*TILE_Y;
    if (hilite)
    {
        if (mask != NULL)
            ImgCopyMaskedH(src, sx, sy, TILE_X, TILE_Y,
                           TileImg, dx, dy, mask);
        else
            ImgCopyH(src, sx, sy, TILE_X, TILE_Y, TileImg, dx, dy, copy);
    }
    else
    {
        if (mask != NULL)
            ImgCopyMasked(src, sx, sy, TILE_X, TILE_Y,
                           TileImg, dx, dy, mask);
        else
            ImgCopy(src, sx, sy, TILE_X, TILE_Y, TileImg, dx, dy, copy);
    }
}

void init_tile_all()
{
    int x, y, k;
    textcolor(WHITE);

    if (!TileImg)
    {
    cprintf("Main tile not initialized\n");
    getch();
    end(-1);
    }

    if (ImgWidth(TileImg)!= TILE_X * TILE_PER_ROW ||
    ImgHeight(TileImg) < TILE_Y*( (TILE_TOTAL + TILE_PER_ROW -1)/TILE_PER_ROW))
    {
        cprintf("Main tile size invalid\n");
        getch();
        end(-1);
    }

    if (!PlayerImg)
    {
        cprintf("Player tile not initialized\n");
    getch();
        end(-1);
    }

    if (ImgWidth(PlayerImg)!= TILE_X * TILEP_PER_ROW)
    {
        cprintf("Player tile size invalid\n");
        getch();
        end(-1);
    }

    if (Options.use_qv_mode)
    {

        if (!TileQvImg)
        {
            cprintf("QV tile not initialized\n");
            getch();
            end(-1);
        }
        if (ImgWidth(TileQvImg)!= TILE_X_EX_QV * TILE_PER_ROW_EX ||
            ImgHeight(TileQvImg) < TILE_Y_EX_QV *
             ((TILE_TOTAL_EX + TILE_PER_ROW_EX -1)/TILE_PER_ROW_EX))
        {
            cprintf("QV tile size invalid\n");
            getch();
            end(-1);
        }

        tcache_kind = TCACHE_KIND_QV;
    tc_hash = 16;
        tile_xmax = TILE_XMAX_QV;
        tile_ymax = TILE_YMAX_QV;
    }
    else
    {
    tcache_kind = TCACHE_KIND_NORMAL;
    tc_hash = 1;
    tile_xmax = TILE_XMAX_NORMAL;
    tile_ymax = TILE_YMAX_NORMAL;
    }
    max_tcache = 4*tile_xmax*tile_ymax;

    screen_tcach_idx = (int **)malloc(sizeof(int *) * tcache_kind);

    tcache = (tile_cache **)malloc(sizeof(tile_cache *)*tcache_kind);

    tcache_head = (int *)malloc(sizeof(int)*tcache_kind*tc_hash);

    for(k=0;k<tcache_kind;k++)
    {
    screen_tcach_idx[k]= (int *)malloc(sizeof(int)* tile_xmax * tile_ymax);
    tcache[k] = (tile_cache *)malloc(sizeof(tile_cache)*max_tcache);

    for(x=0;x<tile_xmax * tile_ymax;x++)
    {
        screen_tcach_idx[k][x] = -1;
    }
    }

    init_tile_qv_table();
    if (Options.use_qv_mode)
    {
    sink_mask = (char *)malloc(TILE_UX_QV*TILE_UY_QV);
    cursor_mask = (char *)malloc(TILE_UX_QV*TILE_UY_QV);
    }
    else
    sink_mask = (char *)malloc(TILE_X*TILE_Y);

    init_tileflag();
    init_tcache();

    DollCacheImg = ImgCreateSimple(TILE_X, TILE_Y);

    for(x=0;x<TILE_DAT_XMAX+2;x++){
    for(y=0;y<TILE_DAT_YMAX+2;y++){
        t1buf[x][y]=0;
    t2buf[x][y]= simple_qv_tile(TILE_DNGN_UNSEEN)|TILE_FLAG_UNSEEN;
    }}

    force_redraw_tile = false;

#if 1 // TILE_MON_EQUIP
    mcache_init();
#endif

#if 1 //Slot
    for(x=0; x<MAX_ITEMLIST;x++)
    {
         itemlist[x]=itemlist_num[x]=itemlist_idx[x]=0;
    }
#endif

}


/*** Investigate a region of specific tile whether it contains any
     picture, or it is blank ***/
int init_tileflag_aux(int tile, int xs, int ys, int wx, int wy){
    int x,y, sx,sy;
    img_type src=TileImg;

    ASSERT(tile<TILE_TOTAL2);

    sx = tile % TILE_PER_ROW;
    sx *= TILE_X;
    sy = tile / TILE_PER_ROW;
    sy *= TILE_Y;

    if (tile >= TILE_NORMAL)
    {
      src = TileQvImg;
      sx = (tile - TILE_NORMAL) % TILE_PER_ROW_EX;
      sx *= TILE_X_EX_QV;
      sy = (tile - TILE_NORMAL) / TILE_PER_ROW_EX;
      sy *= TILE_Y_EX_QV;

    }

    for(x=xs;x<xs+wx;x++){
    for(y=ys;y<ys+wy;y++){
        if(!ImgIsTransparentAt(src, sx+x, sy+y))
            return 1;
    }}
    return 0;
}

void init_tileflag(){
    int tile, flag;
    int region=0;

    for(tile=0;tile<TILE_TOTAL2;tile++){
        flag = 0;

        if (Options.use_qv_mode)
        {
        if (tile<TILE_NORMAL)
        {
            for(region=0; region<8; region++)
            {
            if (init_tileflag_aux(tile,
            region_sx_small_qv[region],
            region_sy_small_qv[region],
            region_wx_small_qv[region],
            region_wy_small_qv[region])) flag |= (1<<region);
            }
        }
        else
        {
                for(region=0; region<8; region++)
                {
                    if (init_tileflag_aux(tile,
                region_sx_qv[region],
                region_sy_qv[region],
                region_wx_qv[region],
                region_wy_qv[region])) flag |= 1<<region;
                }
        }
    }
        tile_flag[tile]=flag;
        tile_prio[tile]=0;
    }

    for(tile=0;tile<4;tile++)
    {
        tile_prio[simple_qv_tile(TILE_DNGN_LAVA)+tile]=2;
        tile_prio[simple_qv_tile(TILE_DNGN_SHALLOW_WATER)+tile]=2;
        tile_prio[simple_qv_tile(TILE_DNGN_DEEP_WATER)+tile]=3;
    }
#if 0    //Hack
    for(tile=TILE_MONS_SHADOW; tile<=TILE_MONS_WATER_ELEMENTAL; tile++)
        tile_prio[simple_qv_tile(tile)]=10;
#endif

    if (!Options.use_qv_mode) return;

    for(tile=simple_qv_tile(TILE_DNGN_ROCK_WALL_OFS); tile < simple_qv_tile(TILE_DNGN_ROCK_WALL_OFS) + 16; tile++)
        tile_prio[tile]=PRIO_WALL;
    for(tile=simple_qv_tile(TILE_DNGN_METAL_WALL); tile < simple_qv_tile(TILE_DNGN_METAL_WALL) + 16; tile++)
        tile_prio[tile]=PRIO_WALL;
    for(tile=simple_qv_tile(TILE_DNGN_STONE_WALL); tile < simple_qv_tile(TILE_DNGN_STONE_WALL) + 16; tile++)
        tile_prio[tile]=PRIO_WALL;
    for(tile=simple_qv_tile(TILE_DNGN_GREEN_CRYSTAL_WALL); tile < simple_qv_tile(TILE_DNGN_GREEN_CRYSTAL_WALL) + 16; tile++)
        tile_prio[tile]=PRIO_WALL;

    tile_prio[simple_qv_tile(TILE_DNGN_OPEN_DOOR)]=PRIO_WALL;
    tile_prio[simple_qv_tile(TILE_DNGN_OPEN_DOOR)+1]=PRIO_WALL;
    tile_prio[simple_qv_tile(TILE_DNGN_CLOSED_DOOR)]=PRIO_WALL;
    tile_prio[simple_qv_tile(TILE_DNGN_CLOSED_DOOR)+1]=PRIO_WALL;

    tile_prio[simple_qv_tile(TILE_DNGN_SILVER_STATUE)]=PRIO_WALL;
    tile_prio[simple_qv_tile(TILE_DNGN_GRANITE_STATUE)]=PRIO_WALL;
    tile_prio[simple_qv_tile(TILE_DNGN_ORANGE_CRYSTAL_STATUE)]=PRIO_WALL;
}

/**** Tile cache related *****/
void clear_tcache(){
    int i, k, h;

    for(k=0;k<tcache_kind;k++)
    {
        tile_cache *tc =  tcache[k];

    // Decompose pointer string tcache[k] into tc_hash segments
        for (h=0; h<tc_hash;h++)
        {
        int i_start = (max_tcache*h)/tc_hash;
        int i_end   = (max_tcache*(h+1))/tc_hash -1;
            tcache_head[k*tc_hash + h] = i_start;

            for(i=i_start;i<=i_end;i++)
            {
                tc[i].id[3] = tc[i].id[2] =
                tc[i].id[1] = tc[i].id[0] = 0;
            tc[i].idx = i;
                if (i==i_start) tc[i].prev =  NULL;
                   else tc[i].prev = &tc[i-1];
                if (i==i_end) tc[i].next = NULL;
                   else tc[i].next = &tc[i+1];
            }
        }
    }
}

void init_tcache(){
    int k;

    clear_tcache();

    tcache_image = (img_type *)malloc(sizeof(img_type)*tcache_kind);

    for(k=0;k<tcache_kind;k++)
    {
    int wx = tcache_wx_normal[k];
    int wy = tcache_wy_normal[k];
#if 1 //QV
    if (Options.use_qv_mode)
    {
        wx = tcache_wx_qv[k];
        wy = tcache_wy_qv[k];
    }
#endif
        tcache_image[k] = ImgCreateSimple(wx, max_tcache*wy);
    }

}

// Move a cache to the top of pointer string
// to shorten the search time
void lift_tcache(int ix, int kind, int hash){
    int kind_n_hash = kind * tc_hash + hash;
    int head_old=tcache_head[kind_n_hash];
    tile_cache *tc = tcache[kind];
    tile_cache *p  = tc[ix].prev;
    tile_cache *n  = tc[ix].next;

    ASSERT(ix<max_tcache);
    ASSERT(head_old<max_tcache);
    ASSERT(kind<tcache_kind);
    ASSERT(hash<tc_hash);

    if(ix == head_old) return;
    if(p!=NULL) p->next = n;
    if(n!=NULL) n->prev = p;

    tcache_head[kind_n_hash] = ix;
    tc[head_old].prev = &tc[ix];
    tc[ix].next       = &tc[head_old];
}

// Find cached image of fg+bg
// If not found, compose and cache it
static int tcache_find_id_normal(int kind, int *fg, int *bg, int *is_new){
    unsigned int id[4];
    int i;
    int hash = 0; // Don't use hash
    int kind_n_hash = kind*tc_hash + hash;
    int nlayer = tcache_nlayer_normal[kind];
    tile_cache *tc = tcache[kind];
    tile_cache *tc0 = &tc[tcache_head[kind_n_hash]];
#if DEBUG
    int search_count =0;
#endif

    *is_new=0;

    for(i=0;i<nlayer;i++) id[i]= (fg[i]<<16)+bg[i]+1;

    while(1){
        tile_cache *next = tc0->next;

        if(tc0->id[0] == id[0]) break;

        if(tc0->id[0]==0 || next == NULL)
        {
        //end of used cache
            *is_new = 1;
            tcache_compose_normal(tc0->idx, fg, bg);
            for(i=0;i<nlayer;i++) tc0->id[i] = id[i];
            break;
        }
        tc0 = next;
#if DEBUG
        search_count++;
#endif
    }
    lift_tcache(tc0->idx, kind, hash);

#if DEBUG
//DEBUGLOG("%d\n",search_count);
#endif

    return tc0->idx;
}

static int tcache_find_id_qv(int kind, int *fg, int *bg, int *is_new){
    unsigned int id[4];
    int i;
    int nlayer = tcache_nlayer_qv[kind];
    int hash = (fg[0]^fg[1]^fg[2]^fg[3]^bg[0]^bg[1]^bg[2]^bg[3])%tc_hash;
    int kind_n_hash = kind*tc_hash + hash;
    tile_cache *tc = tcache[kind];
    tile_cache *tc0 = &tc[tcache_head[kind_n_hash]];
    ASSERT(tcache_head[kind_n_hash]<max_tcache);

#if DEBUG
    int search_count =0;
#endif

    *is_new=0;

    for(i=0;i<nlayer;i++) id[i]= (fg[i]<<16)+bg[i]+1;

    while(1){
    ASSERT(tc0-tc <max_tcache);
        tile_cache *next = tc0->next;
        if((tc0->id[0] == id[0]) && (tc0->id[1] == id[1]) &&
           (tc0->id[2] == id[2]) && (tc0->id[3] == id[3])) break;

        if((tc0->id[0]|tc0->id[1]|tc0->id[2]|tc0->id[3])==0 || next == NULL)
        {
        //end of used cache
            *is_new = 1;
            tcache_compose_qv(kind, tc0->idx, fg, bg);
            for(i=0;i<nlayer;i++) tc0->id[i] = id[i];
            break;
        }
        tc0 = next;
#if DEBUG
        search_count++;
#endif
    }
    lift_tcache(tc0->idx, kind, hash);

#if DEBUG
    //DEBUGLOG("fid %d\n",search_count);
#endif

    return tc0->idx;
}

/*** overlay a tile onto an exsisting image with transpalency operation */
void tcache_overlay(img_type img, int idx,
                     int tile, int region, int *copy, char *mask)
{

    int x0, y0;
    int sx= region_sx_normal[region];
    int sy= region_sy_normal[region];
    int wx= region_wx_normal[region];
    int wy= region_wy_normal[region];
    int ox=0;
    int oy=0;
    img_type src = TileImg;
    int uy=wy;

    tile &= TILE_FLAG_MASK;

    x0 = (tile % TILE_PER_ROW)*TILE_X;
    y0 = (tile / TILE_PER_ROW)*TILE_Y;

    if (Options.use_qv_mode)
    {

    if (tile < TILE_NORMAL)
    {
    // 32x32 small tile
        sx= region_sx_small_qv[region];
        sy= region_sy_small_qv[region];
        wx= region_wx_small_qv[region];
        wy= region_wy_small_qv[region];
        ox= region_ox_small_qv[region];
        oy= region_oy_small_qv[region];
    }
    else
    {
    // 64x64 large tile
        sx= region_sx_qv[region];
        sy= region_sy_qv[region];
        wx= region_wx_qv[region];
        wy= region_wy_qv[region];
        x0 = ((tile - TILE_NORMAL) % TILE_PER_ROW_EX)*TILE_X_EX_QV;
        y0 = ((tile - TILE_NORMAL) / TILE_PER_ROW_EX)*TILE_Y_EX_QV;
    src = TileQvImg;
    }
    uy = region_wy_qv[region];

    }

    if (mask != NULL)
    {
        if(*copy ==2)
            ImgCopyMaskedH(src, x0 + sx, y0 + sy, wx, wy,
                          img, ox, oy + idx*uy, mask);
        else
            ImgCopyMasked(src, x0 + sx, y0 + sy, wx, wy,
                          img, ox, oy + idx*uy, mask);
    *copy = 0;
        return;
    }
    // Hack: hilite rim color
    else if(*copy ==2)
    {
        *copy=0;
        ImgCopyH(src, x0 + sx, y0 + sy, wx, wy,
                      img, ox, oy + idx*uy, *copy);
        return;
    } else
    {
        ImgCopy(src, x0 + sx, y0 + sy, wx, wy,
                      img, ox, oy + idx*uy, *copy);
    }
    *copy = 0;
}

void tcache_overlay_player(img_type img, int dx, int dy,
                        int part, int idx, int ymax, int *copy)
{
    int xs, ys;
    int tidx = tilep_parts_start[part];
    int nx = tilep_parts_nx[part];
    int ny = tilep_parts_ny[part];
    int ox = tilep_parts_ox[part];
    int oy = tilep_parts_oy[part];
    int wx = TILE_X/nx;
    int wy = TILE_Y/ny;

    if(!idx)return;
    idx--;

    tidx += idx/(nx*ny);

    if (oy+wy > ymax) wy -= oy + wy - ymax;
    if (wy<=0) return;

    xs = (tidx % TILEP_PER_ROW)*TILE_X;
    ys = (tidx / TILEP_PER_ROW)*TILE_Y;

    xs += (idx % nx)*(TILE_X/nx);
    ys += ((idx/nx) % ny)*(TILE_Y/ny);

    ImgCopy(PlayerImg, xs, ys, wx, wy,
                  img, dx+ox, dy+oy, *copy);

    *copy = 0;
}

/** overlay a tile onto an exsisting image with transpalency operation */
void register_mask(int tile, int region, int *copy, char *mask,
                    bool small=false)
{

    int x0, y0, x, y;
    int sx= region_sx_normal[region];
    int sy= region_sy_normal[region];
    int wx= region_wx_normal[region];
    int wy= region_wy_normal[region];
    int ox=0;
    int oy=0;
    img_type src = TileImg;
    int ux=wx;
    int uy=wy;

    tile &= TILE_FLAG_MASK;

    x0 = (tile % TILE_PER_ROW)*TILE_X;
    y0 = (tile / TILE_PER_ROW)*TILE_Y;

#if 1 //QV
    if (Options.use_qv_mode)
    {

    if (tile < TILE_NORMAL)
    {
        sx= region_sx_small_qv[region];
        sy= region_sy_small_qv[region];
        wx= region_wx_small_qv[region];
        wy= region_wy_small_qv[region];
        ox= region_ox_small_qv[region];
        oy= region_oy_small_qv[region];
    }
    else
    {
        sx= region_sx_qv[region];
        sy= region_sy_qv[region];
        wx= region_wx_qv[region];
        wy= region_wy_qv[region];
        x0 = ((tile - TILE_NORMAL) % TILE_PER_ROW_EX)*TILE_X_EX_QV;
        y0 = ((tile - TILE_NORMAL) / TILE_PER_ROW_EX)*TILE_Y_EX_QV;
    src = TileQvImg;
    }
    ux = region_wx_qv[region];
    uy = region_wy_qv[region];
    }
#endif

    if (*copy!=0) memset(mask, 0, ux*uy);

    if (*copy == 2)
    {
        if (small)
        {
            ux = wx;
        }
        for(x=0;x<wx;x++){
        for(y=0;y<wy;y++){
            if(! ImgIsTransparentAt(src, x0+sx+x, y0+sy+y))
              mask[(y+oy)*ux+(x+ox)]=1;
        }}
    }
    else
    {
        for(x=0;x<wx;x+=2){
        for(y=0;y<wy;y+=2){
            if(! ImgIsTransparentAt(src, x0+sx+x, y0+sy+y))
          mask[(y+oy)*ux+(x+ox)]=1;
        }}
    }
    *copy = 0;

}

void tile_draw_grid(int kind, int xx, int yy){
    int fg[4],bg[4],ix, ix_old, is_new;

    if (xx < 0 || yy < 0 || xx >= tile_xmax || yy>= tile_ymax) return;
#if 1 //QV

    if (Options.use_qv_mode)
    {

    const int region_of_layer[2][4] ={
    {TREGION_H_QV, TREGION_C_QV, TREGION_F_QV, TREGION_A_QV},
    {TREGION_D_QV, TREGION_G_QV, TREGION_B_QV, TREGION_E_QV}
    };
    const int dx_of_layer[2][4] ={
    {-2, -1, -1, 0},
    {-1, -1, 0, 0}
    };
    const int dy_of_layer[2][4] ={
    {-1, -1, 0, 0},
    {-1, 0, 0, 1}
    };
    int layer;

    for(layer=0; layer<4; layer++)
    {
    int ox = yy+(xx+1)/2-3;
    int oy = yy-(xx+1)/2+9;
    int region = region_of_layer[kind][layer];

    if(((xx&1)==0)&&(kind==0)) ox++;
    if(((xx&1)==0)&&(kind==1)) oy--;

    fg[layer]=0;
    bg[layer]=simple_qv_tile(TILE_DNGN_UNSEEN)|TILE_FLAG_UNSEEN;
    ox += dx_of_layer[kind][layer];
    oy += dy_of_layer[kind][layer];

    if((ox >= -1) && (oy >= -1) && (ox<18) && (oy<18))
    {
            fg[layer] = t1buf[ox+1][oy+1];
        bg[layer] = t2buf[ox+1][oy+1];
            tile_check_blank(fg[layer], (1<<region));
            tile_check_blank(bg[layer], (1<<region));
    }
    }
    }else
#endif
    {
        fg[0] = t1buf[xx+1][yy+1];
        bg[0] = t2buf[xx+1][yy+1];
    }

    ix_old = screen_tcach_idx[kind][xx+yy*tile_xmax];

    if (Options.use_qv_mode)
        ix = tcache_find_id_qv(kind, fg, bg, &is_new);
    else
        ix = tcache_find_id_normal(kind, fg, bg, &is_new);

    screen_tcach_idx[kind][xx+yy*tile_xmax]=ix;
    if(is_new || ix!=ix_old || force_redraw_tile)
    {
    int x_dest = tcache_ox_normal[kind]+xx* TILE_UX_NORMAL;
    int y_dest = tcache_oy_normal[kind]+yy* TILE_UY_NORMAL;
    int wx = tcache_wx_normal[kind];
    int wy = tcache_wy_normal[kind];

        if (Options.use_qv_mode)
    {
            x_dest = tcache_ox_qv[kind]+xx* TILE_UX_QV;
        y_dest = tcache_oy_qv[(kind+xx+1)&1]+yy*TILE_UY_QV;
            wx = tcache_wx_qv[kind];
            wy = tcache_wy_qv[kind];
    }

        //再描画用バッファへコピー
        ImgCopy(tcache_image[kind],
                   0, ix*wy,   wx, wy,
                   ScrBufImg, x_dest, y_dest, 1);
    }
}

void update_single_grid(int x, int y)
{
    int sx, sy, wx, wy;

#if 1 //QV
    if (Options.use_qv_mode)
    {
        sx = 12+(x-y)-1;
        sy = 6+(x+y)/2 - 8;
        if ( (x+y)&1 )
    {
            tile_draw_grid(TCACHE0_QV, sx, sy-1);
            tile_draw_grid(TCACHE1_QV, sx, sy);
            tile_draw_grid(TCACHE0_QV, sx, sy);
            tile_draw_grid(TCACHE1_QV, sx, sy+1);

            tile_draw_grid(TCACHE1_QV, sx+1, sy-1);
            tile_draw_grid(TCACHE0_QV, sx+1, sy);
            tile_draw_grid(TCACHE1_QV, sx+1, sy);
            tile_draw_grid(TCACHE0_QV, sx+1, sy+1);
        sx = sx*TILE_UX_QV;
        sy = sy*TILE_UY_QV-TILE_UY_QV/2;
    }else
    {
            tile_draw_grid(TCACHE0_QV, sx, sy-1);
            tile_draw_grid(TCACHE1_QV, sx, sy-1);
            tile_draw_grid(TCACHE0_QV, sx, sy);
            tile_draw_grid(TCACHE1_QV, sx, sy);

            tile_draw_grid(TCACHE1_QV, sx+1, sy-1);
            tile_draw_grid(TCACHE0_QV, sx+1, sy-1);
            tile_draw_grid(TCACHE1_QV, sx+1, sy);
            tile_draw_grid(TCACHE0_QV, sx+1, sy);
            sx = sx*TILE_UX_QV;
            sy = (sy-1)*TILE_UY_QV;
    }
    wx = TILE_UX_QV*2;
    wy = TILE_UY_QV*2;
    }else
#endif
    {
        sx = x;
        sy = y;
        tile_draw_grid(TCACHE0_NORMAL, sx, sy);
        wx = TILE_X;
        wy = TILE_Y;
    sx *= TILE_UX_NORMAL;
    sy *= TILE_UY_NORMAL;
    }
        region_tile->redraw(sx, sy, sx+wx-1, sy+wy-1);
}

#define DOLLS_MAX 11         //編集可能な着せ替え人形の数
#define PARTS_DISP_MAX 11    //画面に表示される部品数
#define PARTS_ITEMS 12       //編集可能な項目の数
#define TILEP_SELECT_DOLL 20 //メニューに使うダミー

typedef struct dolls_data
{
    int parts[TILEP_PARTS_TOTAL];
}dolls_data;

// Discard cache containing specific tile
void redraw_spx_tcache(int tile)
{
    int kind, idx, layer, redraw;
    int fg[4], bg[4];
    for(kind = 0; kind < tcache_kind; kind++)
    {
        int nlayer = tcache_nlayer_normal[kind];
#if 1 //QV
        if (Options.use_qv_mode)
        nlayer = tcache_nlayer_qv[kind];
#endif

        for(idx = 0; idx < max_tcache; idx++)
        {
            redraw = 0;
            for(layer = 0; layer < nlayer; layer++)
            {
                fg[layer] = (tcache[kind][idx].id[layer]-1)>>16;
                bg[layer] = (tcache[kind][idx].id[layer]-1)&0xffff;
                if((fg[layer]&TILE_FLAG_MASK) == tile) redraw = 1;
                if((bg[layer]&TILE_FLAG_MASK) == tile) redraw = 1;
            }
            if (redraw)
            {
#if 1 //QV
        if (Options.use_qv_mode)
                    tcache_compose_qv(kind, idx, fg, bg);
        else
#endif
                tcache_compose_normal(idx, fg, bg);
            }
        }
    }
}

static dolls_data current_doll;
static int current_parts[TILEP_PARTS_TOTAL];

static bool draw_doll(img_type img, dolls_data *doll)
{
    //合成順序
    const int p_order[TILEP_PARTS_TOTAL]=
      {
        TILEP_PART_SHADOW,
        TILEP_PART_CLOAK,
        TILEP_PART_BASE,
        TILEP_PART_BOOTS,
        TILEP_PART_LEG,
        TILEP_PART_BODY,
        TILEP_PART_ARM,
        TILEP_PART_HAND1,
        TILEP_PART_HAND2,
        TILEP_PART_HAIR,
        TILEP_PART_BEARD,
        TILEP_PART_HELM
      };
    int p_order2[TILEP_PARTS_TOTAL];

    int i;
    int flags[TILEP_PARTS_TOTAL];
    int parts2[TILEP_PARTS_TOTAL];
    int *parts = doll->parts;
    int c=1;
    bool changed = false;

    // convert TILEP_SHOW_EQUIP into real parts number
    for(i = 0; i < TILEP_PARTS_TOTAL; i++)
    {
        parts2[i] = parts[i];
        if (parts2[i] == TILEP_SHOW_EQUIP)
        {
            switch (i)
            {
                int item;
                case TILEP_PART_HAND1:
                    item = you.equip[EQ_WEAPON];
                    if (item == -1)
                        parts2[i] = 0;
                    else
                        parts2[i] = tilep_equ_weapon(you.inv[item]);
                   break;

                case TILEP_PART_HAND2:
                    item = you.equip[EQ_SHIELD];
                    if (item == -1)
                        parts2[i] = 0;
                    else
                        parts2[i] = tilep_equ_shield(you.inv[item]);
                    break;

                case TILEP_PART_BODY:
                    item = you.equip[EQ_BODY_ARMOUR];
                    if (item == -1)
                        parts2[i] = 0;
                else
                        parts2[i] = tilep_equ_armour(you.inv[item]);
                    break;

                case TILEP_PART_CLOAK:
                    item = you.equip[EQ_CLOAK];
                    if (item == -1)
                        parts2[i] = 0;
                    else
                        parts2[i] = tilep_equ_cloak(you.inv[item]);
                    break;

                default:
                   parts2[i] = 0;
            }
        }

        if (you.attribute[ATTR_TRANSFORMATION] == TRAN_BLADE_HANDS)
        {
            if (i== TILEP_PART_HAND1)
                parts2[i] = TILEP_HAND1_BLADEHAND;
            if (i== TILEP_PART_HAND2)
                parts2[i] = TILEP_HAND2_BLADEHAND;
        }

        if (parts2[i] != current_parts[i]) changed = true;
        current_parts[i] = parts2[i];
    }

    if (!changed)
    {
        return false;
    }

    tilep_calc_flags(parts2, flags);
    ImgClear(img);

    // Hack: change overlay order of boots/skirts
    for(i = 0; i < TILEP_PARTS_TOTAL; i++)
        p_order2[i]=p_order[i];
    if (parts2[TILEP_PART_LEG] < TILEP_LEG_SKIRT_OFS)
    {
        p_order2[3] = TILEP_PART_LEG;
        p_order2[4] = TILEP_PART_BOOTS;
    }

    for(i = 0; i < TILEP_PARTS_TOTAL; i++)
    {
        int p = p_order2[i];
        int ymax = TILE_Y;
        if (flags[p]==TILEP_FLAG_CUT_CENTAUR) ymax=18;
        if (flags[p]==TILEP_FLAG_CUT_NAGA) ymax=18;
        if(parts2[p] && flags[p])
        {
            tcache_overlay_player(img, 0, 0, p, parts2[p], ymax, &c);
        }
    }
    return true;
}

void TilePlayerRefresh()
{
    int parts0[TILEP_PARTS_TOTAL];

    // For draconian color update
    tilep_race_default(you.species, you.experience_level, parts0);
    current_doll.parts[TILEP_PART_BASE] = parts0[TILEP_PART_BASE];

    if (!draw_doll(DollCacheImg, &current_doll))
        return; // Not changed
    ImgCopyToTileImg(TILE_PLAYER, DollCacheImg, 0, 0, 1);

    redraw_spx_tcache(TILE_PLAYER);
    force_redraw_tile = true;

    update_single_grid(8,8);
}

//着せ替えコマンド
void TilePlayerEdit()
{
//初期化

    //ステータス画面に表示する部品番号の表示の順番
    const int p_lines[PARTS_ITEMS]=
      {
        TILEP_SELECT_DOLL, //ダミー
        TILEP_PART_BASE,
        TILEP_PART_HELM,
        TILEP_PART_HAIR,
        TILEP_PART_BEARD,
        TILEP_PART_HAND1,
        TILEP_PART_HAND2,
        TILEP_PART_ARM,
        TILEP_PART_BODY,
        TILEP_PART_LEG,
        TILEP_PART_BOOTS,
        TILEP_PART_CLOAK
      };

#define display_parts_idx(part) \
{                               \
    gotoxy(40+9 , part + 1);    \
    mpr_on(MODE_STAT);          \
    tilep_part_to_str(dolls[cur_doll].parts[ p_lines[part] ], ibuf);\
    cprintf(ibuf);\
}

#ifdef JP
    const char *gender_name[2] ={
        "男性", "女性"
    };
#else
    const char *gender_name[2] ={
        "Male", "Fem "
    };
#endif

    dolls_data dolls[DOLLS_MAX],
               undo_dolls[DOLLS_MAX];
    int copy_doll[TILEP_PARTS_TOTAL];

    //int parts[TILEP_PARTS_TOTAL];
    int i, j, k, x, y, kin, done = 0;
    int cur_doll = 0,   //どのドールをいじっているか
        pre_part = 0,   //どの部位をいじっていたか
        cur_part = 0,   //今どの部位をいじっているか
        mode     = TILEP_M_DEFAULT;  //dolls.txtを起動時に読み込むか否か
    int  d = 0;
    FILE *fp;
    char fbuf[80], ibuf[8];
    int tile_str, sx, sy;

    //ビットマップ領域の確保
    img_type PartsImg = ImgCreateSimple(TILE_X * PARTS_DISP_MAX, TILE_Y * 1);
    //部品リスト表示用ビットマップ

    img_type DollsListImg = ImgCreateSimple( TILE_X * DOLLS_MAX, TILE_Y * 1);
    //着せ替え人形ストックのリスト表示用ビットマップ

    //バックバッファを特定色で埋める。
    //windows版ではタイル地図バッファをそのまま着せ替え人形表示窓にする。
    ImgClear(ScrBufImg);

    //着せ替え配列の初期化
    tilep_gender = TILEP_GENDER_FEMALE;
    memset( copy_doll, 0, sizeof(dolls_data) );
    tilep_race_default(you.species, you.experience_level, copy_doll);
    for(j = 0; j < DOLLS_MAX; j++)
    {
        memset( dolls[j].parts, 0, sizeof(dolls_data) );
        memset( undo_dolls[j].parts, 0, sizeof(dolls_data) );
        tilep_race_default(you.species, you.experience_level, dolls[j].parts);
        tilep_race_default(you.species, you.experience_level, undo_dolls[j].parts);
    }
//ファイルから読みこみ
    if ( (fp = fopen("dolls.txt", "r")) == NULL )
    {
    }
    else
    {
        if (fscanf(fp, "%s", fbuf) != EOF)
        {
            if (strcmp(fbuf, "MODE=LOADING") == 0)
                mode = TILEP_M_LOADING;
        }
        if (fscanf(fp, "%s", fbuf) != EOF)
        {
            if (strncmp(fbuf, "NUM=", 4) == 0)
            {
                ibuf[0] = fbuf[4];
                ibuf[1] = fbuf[5];
                ibuf[2] = '\0';
                cur_doll = atoi(ibuf);
                if ( (cur_doll < 0)||(cur_doll > 10) )
                    cur_doll = 0;
            }
        }
        for(j = 0; j < DOLLS_MAX; j++)
        {
            if (fscanf(fp, "%s", fbuf) != EOF)
            {
                tilep_scan_parts(fbuf, dolls[j].parts);
                undo_dolls[j] = dolls[j];
            }
        }
        fclose(fp);
    }
//画面の消去
    clrscr();
    gotoxy(1,18);
    mpr_on(MODE_MPR);
//解説文表示
#ifdef JP
    mpr("部位を選択               : [j][k]or[↑][↓]");
    mpr("装備を選択/ページ移動    : [h][l]or[←][→]/[H][L]or[SHIFT]+[←][→]");
    mpr("職業準拠の衣服           : [CTRL]+[D]");
    mpr("装備依存表示 ON/OFF      : [*]");
    mpr("装備を脱ぐ/全て脱ぐ      : [t]/[CTRL]+[T]");
    mpr("コピー/ペースト/アンドゥ : [CTRL]+[C]/[CTRL]+[V]/[CTRL]+[Z]");
    mpr("ランダムな着せ替え       : [CTRL]+[R]");

    mpr_on(MODE_MPR);
    gotoxy(1, 25);
    cprintf("起動時の設定             : [m] (現在の設定:%s)",
        ( (mode == TILEP_M_LOADING) ? "起動時に服装を読み込む":"起動時は職業準拠の服装" ));
#else
    mpr("Select Part              : [j][k]or[up][down]");
    mpr("Change Part/Page         : [h][l]or[left][right]/[H][L]");
    mpr("Class-Default            : [CTRL]+[D]");
    mpr("Toggle Equipment Mode    : [*]");
    mpr("Take off/Take off All    : [t]/[CTRL]+[T]");
    mpr("Copy/Paste/Undo          : [CTRL]+[C]/[CTRL]+[V]/[CTRL]+[Z]");
    mpr("Randomize                : [CTRL]+[R]");

    mpr_on(MODE_MPR);
    gotoxy(1, 25);
    cprintf("Toggle Startup mode      : [m] (Current mode:%s)",
        ( (mode == TILEP_M_LOADING) ? "Load User's Settings":"Class Default" ));
#endif
//リストを表示
    mpr_on(MODE_STAT);
#ifdef JP
    gotoxy(40,  1);cprintf("  番号 :");
    gotoxy(40,  2);cprintf("  性別 :");
    gotoxy(40,  3);cprintf("  兜   :");
    gotoxy(40,  4);cprintf("  髪   :");
    gotoxy(40,  5);cprintf("  髭   :");
    gotoxy(40,  6);cprintf("  武器 :");
    gotoxy(40,  7);cprintf("  盾   :");
    gotoxy(40,  8);cprintf("  小手 :");
    gotoxy(40,  9);cprintf("  胴   :");
    gotoxy(40, 10);cprintf("  脚   :");
    gotoxy(40, 11);cprintf("  靴   :");
    gotoxy(40, 12);cprintf("  外套 :");
#else
    gotoxy(40,  1);cprintf("  Index:");
    gotoxy(40,  2);cprintf("  Gendr:");
    gotoxy(40,  3);cprintf("  Helm :");
    gotoxy(40,  4);cprintf("  Hair :");
    gotoxy(40,  5);cprintf("  Beard:");
    gotoxy(40,  6);cprintf("  Weapn:");
    gotoxy(40,  7);cprintf("  Shild:");
    gotoxy(40,  8);cprintf("  Glove:");
    gotoxy(40,  9);cprintf("  Armor:");
    gotoxy(40, 10);cprintf("  Legs :");
    gotoxy(40, 11);cprintf("  Boots:");
    gotoxy(40, 12);cprintf("  Cloak:");
#endif

#define PARTS_Y (TILE_Y*11)

//バックバッファへの文字の表示
#ifdef JP
    tile_str = TILE_TEXT_PARTS_J;
#else
    tile_str = TILE_TEXT_PARTS_E;
#endif
    sx = (tile_str % TILE_PER_ROW)*TILE_X;
    sy = (tile_str / TILE_PER_ROW)*TILE_Y;

    ImgCopy(TileImg, sx, sy, TILE_X, TILE_Y/2,
                ScrBufImg, (TILE_X * 3) - 8,(TILE_Y * 4) - 8-24, 0);
    ImgCopy(TileImg, sx, sy+ TILE_Y/2, TILE_X, TILE_Y/2,
                ScrBufImg, (TILE_X * 4) - 8,(TILE_Y * 4) - 8-24, 0);

#ifdef JP
    tile_str = TILE_TEXT_DOLLS_J;
#else
    tile_str = TILE_TEXT_DOLLS_E;
#endif
    sx = (tile_str % TILE_PER_ROW)*TILE_X;
    sy = (tile_str / TILE_PER_ROW)*TILE_Y;
    ImgCopy(TileImg, sx, sy, TILE_X, TILE_Y/2,
                ScrBufImg, (TILE_X * 3) - 8,PARTS_Y - 8 -24, 0);
    ImgCopy(TileImg, sx, sy+ TILE_Y/2, TILE_X, TILE_Y/2,
                ScrBufImg, (TILE_X * 4) - 8,PARTS_Y - 8 -24, 0);

//人形一覧画像の初回作成
    ImgClear(DollsListImg);
    for(j = 0; j < DOLLS_MAX; j++)
    {
        draw_doll(DollCacheImg, &dolls[j]);
        ImgCopy(DollCacheImg, 0, 0, ImgWidth(DollCacheImg), ImgHeight(DollCacheImg),
                       DollsListImg, j * TILE_X, 0, 1);
    }
//以下、部品選択ループ
    done = 0;

    while (!done)
    {
        //パーツリストを更新
        //clear_my_img(ScrBufImg, pix_transparent);
        //パネル描画
        ImgClear(PartsImg);

        region_tile->DrawPanel((TILE_X * 3) - 8, (TILE_Y * 4) - 8,
                   ImgWidth(PartsImg) + 16, ImgHeight(PartsImg) + 16);
        region_tile->DrawPanel((TILE_X * 3) - 8, PARTS_Y - 8,
                   ImgWidth(DollsListImg) + 16, ImgHeight(DollsListImg) + 16);
        region_tile->DrawPanel(8*TILE_X - 8, 8*TILE_Y - 8,
                   TILE_X + 16, TILE_Y + 16);

        if (p_lines[cur_part] == TILEP_SELECT_DOLL) //矢印は編集する人形の選択の位置にある
        {
            gotoxy(40+9 , 1);mpr_on(MODE_STAT);
#ifdef JP
            cprintf("%02d番", cur_doll);
#else
            cprintf("%02d", cur_doll);
#endif
            gotoxy(40+9 , 2);
            cprintf("%s", gender_name[ dolls[cur_doll].parts[TILEP_PART_BASE] % 2 ]);

            for (i = 2; i<PARTS_ITEMS; i++)
            {
                gotoxy(40+9 , i + 1);mpr_on(MODE_STAT);
                tilep_part_to_str(dolls[cur_doll].parts[ p_lines[i] ], ibuf);
                cprintf("%s / %03d", ibuf, tilep_parts_total[ p_lines[i] ]);
            }
        }
        else //矢印が部位のどこかを選択している場合。部品一覧画像を更新
        {
            for(i = 0; i < PARTS_DISP_MAX; i++)
            {
                int index;
                if (dolls[cur_doll].parts[ p_lines[cur_part] ] == TILEP_SHOW_EQUIP)
                    index = 0;
                else
                    index = (i + dolls[cur_doll].parts[ p_lines[cur_part] ] - (PARTS_DISP_MAX / 2) );
                if (index < 0)
                    index = index + tilep_parts_total[ p_lines[cur_part] ] + 1;
                if (index > tilep_parts_total[ p_lines[cur_part] ])
                    index = index - tilep_parts_total[ p_lines[cur_part] ] - 1;
                tcache_overlay_player(PartsImg, i * TILE_X, 0, p_lines[cur_part], index, TILE_Y, &d);
            }
            ImgCopy(PartsImg, 0, 0, ImgWidth(PartsImg), ImgHeight(PartsImg),
                          ScrBufImg, 3 * TILE_X, 4 * TILE_Y, 0);
            ImgCopyFromTileImg( TILE_CURSOR, ScrBufImg,
                  (3 + PARTS_DISP_MAX / 2) * TILE_X, 4 * TILE_Y, 0);
        }
        //編集中の人形を更新
        draw_doll(DollCacheImg, &dolls[cur_doll]);

        ImgCopy(DollCacheImg, 0, 0, TILE_X, TILE_Y,
                      DollsListImg, cur_doll*TILE_X, 0, 1);

        //画面の更新
        ImgCopyToTileImg(TILE_PLAYER, DollCacheImg, 0, 0, 1);
        ImgCopyFromTileImg(TILE_PLAYER, ScrBufImg, 8*TILE_X, 8*TILE_Y, 0);
        ImgCopy(DollsListImg, 0, 0,
                      ImgWidth(DollsListImg), ImgHeight(DollsListImg),
                      ScrBufImg, 3 * TILE_X, PARTS_Y, 0);
        ImgCopyFromTileImg( TILE_CURSOR, ScrBufImg,
                            (3 + cur_doll) * TILE_X, PARTS_Y, 0);
        region_tile->redraw();

        mpr_on(MODE_STAT);
        gotoxy(40+9 , cur_part + 1);
#ifdef JP
        if (cur_part == 0) //項目が編集中の人形番号なら番号の表示
            cprintf("%02d番", cur_doll);
#else
        if (cur_part == 0) //項目が編集中の人形番号なら番号の表示
            cprintf("%02d", cur_doll);
#endif
        else if (cur_part == 1) //項目が性別なら性別の表示
        {
            cprintf("%s", gender_name[ dolls[cur_doll].parts[p_lines[cur_part] ] % 2 ]);
        }
        else               //項目が性別以外なら数値の表示
        {
            tilep_part_to_str(dolls[cur_doll].parts[ p_lines[cur_part] ], ibuf);
            cprintf("%s", ibuf);
        }

        gotoxy(40, pre_part + 1);cprintf("  ");
#ifdef JP
        gotoxy(40, cur_part + 1);cprintf("＞");
#else
        gotoxy(40, cur_part + 1);cprintf("->");
#endif
        pre_part = cur_part;

        /* Get a key */
        kin = getch();

        /* Analyze the key */
        switch (kin)
        {
            case 0x1B: //ESC
            {
                done = 1;
                break;
            }

            case 'h': //部品変更:左
            case 'H': //部品変更:1ページ左
            {
                int *target = &dolls[cur_doll].parts[ p_lines[cur_part] ];

                if (cur_part == 0)      //項目が編集対象ドール選択
                {
                    if (cur_doll > 0)
                        cur_doll -= 1;
                    else
                        cur_doll = (DOLLS_MAX -1);
                }
                else if (cur_part == 1) //項目が性別なら性別のトグル
                {
                    if (*target % 2)
                        (*target)++;
                    else
                        (*target)--;
                }
                else
                {
                    if (*target == TILEP_SHOW_EQUIP) continue;

                    if (kin=='h')
                        (*target)--;
                    else
                        *target -= PARTS_DISP_MAX;
                   (*target) += tilep_parts_total[ p_lines[cur_part] ]+1;
                   (*target) %= tilep_parts_total[ p_lines[cur_part] ]+1;
                }
                break;
            }

            case 'l': //部品変更:右
            case 'L': //部品変更:1ページ右
            {
                int *target = &dolls[cur_doll].parts[ p_lines[cur_part] ];

                if (cur_part == 0)      //項目が編集対象ドール選択
                {
                    cur_doll ++;
                    cur_doll %= DOLLS_MAX;
                }
                else if (cur_part == 1) //項目が性別なら性別のトグル
                {
                    if (*target % 2)
                        (*target)++;
                    else
                        (*target)--;
                }
                else
                {
                    if (*target == TILEP_SHOW_EQUIP) continue;

                    if(kin=='l')
                        (*target)++;
                    else
                        *target +=PARTS_DISP_MAX;
                    *target %= tilep_parts_total[ p_lines[cur_part] ]+1;
                }
                break;
            }

            case 'j': //編集部位変更:下
            {
                if (cur_part < (PARTS_ITEMS-1) )
                    cur_part += 1;
                else
                    cur_part = 0;
                break;
            }

            case 'k': //編集部位変更:上
            {
                if (cur_part > 0)
                    cur_part -= 1;
                else
                    cur_part = (PARTS_ITEMS-1);
                break;
            }

            case 'm': //人形ファイル起動時読み込みについての設定トグル
            {
                if (mode == TILEP_M_LOADING)
                    mode = TILEP_M_DEFAULT;
                else
                    mode = TILEP_M_LOADING;

                gotoxy(1, 25);mpr_on(MODE_MPR);
#ifdef JP
                cprintf("起動時の設定             : [m] (現在の設定:%s)",
                    ( (mode == TILEP_M_LOADING) ? "起動時に服装を読み込む":"起動時は職業準拠の服装" ));
#else
                cprintf("Toggle Startup mode      : [m] (Current mode:%s)",
                    ( (mode == TILEP_M_LOADING) ? "Load User's Settings":"Class Default" ));
#endif
                break;
            }

            case 't': //選択中の部位の装備だけ脱ぐ
            {
                if (cur_part >= 2)
                {
                    dolls[cur_doll].parts[ p_lines[cur_part] ] = 0;
                    display_parts_idx(cur_part);
                }
                break;
            }

            case CONTROL('D'): //装備を全て現在の職業のデフォルトに戻す
            {
                tilep_job_default(you.char_class, dolls[cur_doll].parts);
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    display_parts_idx(i);
                }
                break;
            }

            case CONTROL('H'): //種族変更:左
            {
                if (dolls[cur_doll].parts[TILEP_PART_BASE] > 0)
                    dolls[cur_doll].parts[TILEP_PART_BASE] -= 1;
                else
                    dolls[cur_doll].parts[TILEP_PART_BASE] = tilep_parts_total[TILEP_PART_BASE];
                gotoxy(40+9 , 2);
        mpr_on(MODE_STAT);
                cprintf("%s", gender_name[ dolls[cur_doll].parts[TILEP_PART_BASE] % 2 ]);
                break;
            }

            case CONTROL('L'): //種族変更:右
            {
                if (dolls[cur_doll].parts[TILEP_PART_BASE] < tilep_parts_total[TILEP_PART_BASE])
                    dolls[cur_doll].parts[TILEP_PART_BASE] += 1;
                else
                    dolls[cur_doll].parts[TILEP_PART_BASE] = 0;
                gotoxy(40+9 , TILEP_PART_BASE + 2);
        mpr_on(MODE_STAT);
                cprintf("%s", gender_name[ dolls[cur_doll].parts[TILEP_PART_BASE] % 2 ]);
                break;
            }

            case CONTROL('T'): //装備を全て脱ぐ
            {
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    undo_dolls[cur_doll].parts[ p_lines[i] ] = dolls[cur_doll].parts[ p_lines[i] ];
                    dolls[cur_doll].parts[ p_lines[i] ] = 0;
                    display_parts_idx(i);
                }
                break;
            }

            case CONTROL('C'): //装備をコピー
            {
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    copy_doll[ p_lines[i] ] = dolls[cur_doll].parts[ p_lines[i] ];
                }
                break;
            }

            case CONTROL('V'): //装備をペースト
            {
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    undo_dolls[cur_doll].parts[ p_lines[i] ] = dolls[cur_doll].parts[ p_lines[i] ];
                    dolls[cur_doll].parts[ p_lines[i] ] = copy_doll[ p_lines[i] ];
                    display_parts_idx(i);
                }
                break;
            }

            case CONTROL('Z'): //装備をアンドゥ
            {
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    dolls[cur_doll].parts[ p_lines[i] ] = undo_dolls[cur_doll].parts[ p_lines[i] ];
                    display_parts_idx(i);
                }
                break;
            }

            case CONTROL('R'): //ランダムな着せ替え
            {
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    undo_dolls[cur_doll].parts[ p_lines[i] ] = dolls[cur_doll].parts[ p_lines[i] ];
                }
                dolls[cur_doll].parts[TILEP_PART_CLOAK] =
                    one_chance_in(2) * ( random2(tilep_parts_total[ TILEP_PART_CLOAK ]) + 1);
                dolls[cur_doll].parts[TILEP_PART_BOOTS] =
                    ( random2(tilep_parts_total[ TILEP_PART_BOOTS ] + 1) );
                dolls[cur_doll].parts[TILEP_PART_LEG] =
                    ( random2(tilep_parts_total[ TILEP_PART_LEG ] + 1) );
                dolls[cur_doll].parts[TILEP_PART_BODY] =
                    ( random2(tilep_parts_total[ TILEP_PART_BODY ] + 1) );
                dolls[cur_doll].parts[TILEP_PART_ARM] =
                    one_chance_in(2) * ( random2(tilep_parts_total[ TILEP_PART_ARM ]) + 1);
                dolls[cur_doll].parts[TILEP_PART_HAND1] =
                    ( random2(tilep_parts_total[ TILEP_PART_HAND1 ] + 1) );
                dolls[cur_doll].parts[TILEP_PART_HAND2] =
                    one_chance_in(2) * ( random2(tilep_parts_total[ TILEP_PART_HAND2 ]) + 1);
                dolls[cur_doll].parts[TILEP_PART_HAIR] =
                    ( random2(tilep_parts_total[ TILEP_PART_HAIR ] + 1) );
                dolls[cur_doll].parts[TILEP_PART_BEARD] =
                    ( (dolls[cur_doll].parts[TILEP_PART_BASE] + 1) % 2) *
                    one_chance_in(4) * ( random2(tilep_parts_total[ TILEP_PART_BEARD ]) + 1 );
                dolls[cur_doll].parts[TILEP_PART_HELM] =
                    one_chance_in(2) * ( random2(tilep_parts_total[ TILEP_PART_HELM ]) + 1 );
                for(i = 2; i < PARTS_ITEMS; i++)
                {
                    display_parts_idx(i);
                }
                break;
            }
            case '*':
            {
                int part = p_lines[cur_part];
                int *target = &dolls[cur_doll].parts[part];

                if (part != TILEP_PART_HAND2 && part != TILEP_PART_HAND1 &&
                    part != TILEP_PART_BODY && part != TILEP_PART_CLOAK)
                      continue;
                if (*target == TILEP_SHOW_EQUIP)
                    *target = 0;
                else
                    *target = TILEP_SHOW_EQUIP;

                display_parts_idx(cur_part);
            }

            default:
            {
            }
        }
        delay(20);
    }

    current_doll = dolls[cur_doll];
    draw_doll(DollCacheImg, &current_doll);
    ImgCopyToTileImg(TILE_PLAYER, DollCacheImg, 0, 0, 1);

//ファイルに書き込み
    if ( (fp = fopen("dolls.txt", "w+")) == NULL )
    {
    }
    else
    {
        fprintf(fp, "MODE=%s\n", ( (mode == TILEP_M_LOADING) ? "LOADING":"DEFAULT" ) );
        fprintf(fp, "NUM=%02d\n", cur_doll);
        for(j = 0; j < DOLLS_MAX; j++)
        {
            tilep_print_parts(fbuf, dolls[j].parts);
            fprintf(fp, "%s\n", fbuf);
        }
        fclose(fp);
    }

//ビットマップ列に使用したメモリの解放
    ImgDestroy(PartsImg);
    ImgDestroy(DollsListImg);

//キャッシュ廃棄などゲームへの復帰処理
    ImgClear(ScrBufImg);
    redraw_spx_tcache(TILE_PLAYER);

    for(x=0;x<TILE_DAT_XMAX+2;x++){
    for(y=0;y<TILE_DAT_YMAX+2;y++){
        t1buf[x][y]=0;
    t2buf[x][y]=simple_qv_tile(TILE_DNGN_UNSEEN)|TILE_FLAG_UNSEEN;
    }}

    for(k=0;k<tcache_kind;k++)
        for(x=0;x<tile_xmax*tile_ymax;x++)
            screen_tcach_idx[k][x] = -1;

    clrscr();
    redraw_screen();
    mpr_on(MODE_CRT);
}

void TilePlayerInit()
{
    int i, j;
    int cur_doll = 0,          //どのドールが選択されていたか
        mode     = TILEP_M_DEFAULT;  //dolls.txtを起動時に読み込むか否か
    dolls_data doll;
    FILE *fp;
    char fbuf[80], ibuf[8];

//着せ替え配列の初期化
    for(i = 0; i < TILEP_PARTS_TOTAL; i++)
        doll.parts[i] = 0;
    tilep_gender = TILEP_GENDER_FEMALE;
    tilep_race_default(you.species, you.experience_level, doll.parts);
    tilep_job_default(you.char_class, doll.parts);

//ファイルから読みこみ
    if ( (fp = fopen("dolls.txt", "r")) == NULL )
    {
    }
    else
    {
        if (fscanf(fp, "%s", fbuf) != EOF)
        {
            if (strcmp(fbuf, "MODE=LOADING") == 0)
                mode = TILEP_M_LOADING;
        }
        if (fscanf(fp, "%s", fbuf) != EOF)
        {
            if (strncmp(fbuf, "NUM=", 4) == 0)
            {
                ibuf[0] = fbuf[4];
                ibuf[1] = fbuf[5];
                ibuf[2] = '\0';
                cur_doll = atoi(ibuf);
                if ( (cur_doll < 0)||(cur_doll > 10) )
                    cur_doll = 0;
            }
        }
        for(j = 0; j <= cur_doll; j++)
        {
            if ( (fscanf(fp, "%s", fbuf) != EOF) && (j == cur_doll) && (mode == TILEP_M_LOADING) )
            {
                tilep_scan_parts(fbuf, doll.parts);
            }
        }
        fclose(fp);
    }

    current_doll = doll;
    draw_doll(DollCacheImg, &doll);

    ImgCopyToTileImg(TILE_PLAYER, DollCacheImg, 0, 0, 1);

}

void TileGhostInit(struct ghost_struct &gs)
{
    dolls_data doll;
    int x, y;
    unsigned int pseudo_rand = ghost.values[GVAL_MAX_HP]*54321;
    char mask[TILE_X*TILE_Y];

    for (x=0; x<TILE_X; x++)
    for (y=0; y<TILE_X; y++)
        mask[x+y*TILE_X] = (x+y)&1;

    //着せ替え配列の初期化
    for(x = 0; x < TILEP_PARTS_TOTAL; x++)
    {
        doll.parts[x] = 0;
        current_parts[x] = 0;
    }
    tilep_race_default(gs.values[GVAL_SPECIES], gs.values[GVAL_EXP_LEVEL],
                       doll.parts);
    tilep_job_default (gs.values[GVAL_CLASS],   doll.parts);


    for(x = TILEP_PART_CLOAK; x < TILEP_PARTS_TOTAL; x++)
    {
        if (doll.parts[x] == TILEP_SHOW_EQUIP)
        {
            doll.parts[x] = 1 + (pseudo_rand % tilep_parts_total[x]);

            if (x == TILEP_PART_BODY)
            {
                int p = 0;
                int ac = ghost.values[GVAL_AC];
                ac *= (5 + (pseudo_rand/11) % 11);
                ac /= 10;

                if (ac > 25) p= TILEP_BODY_PLATE_BLACK;
                else
                if (ac > 20) p=  TILEP_BODY_BANDED;
                else
                if (ac > 15) p= TILEP_BODY_SCALEMAIL;
                else
                if (ac > 10) p= TILEP_BODY_CHAINMAIL;
                else
                if (ac > 5 ) p= TILEP_BODY_LEATHER_HEAVY;
                else
                             p= TILEP_BODY_ROBE_BLUE;
                doll.parts[x] = p;
            }
        }
    }

    int sk = ghost.values[GVAL_BEST_SKILL];
    int dam = ghost.values[GVAL_DAMAGE];
    int p = 0;

    dam *= (5 + pseudo_rand % 11);
    dam /= 10;

    if (sk == SK_MACES_FLAILS)
    {
        if (dam>30) p = TILEP_HAND1_GREAT_FRAIL;
        else
        if (dam>25) p = TILEP_HAND1_GREAT_MACE;
        else
        if (dam>20) p = TILEP_HAND1_SPIKED_FRAIL;
        else
        if (dam>15) p = TILEP_HAND1_MORNINGSTAR;
        else
        if (dam>10) p = TILEP_HAND1_FRAIL;
        else
        if (dam>5) p = TILEP_HAND1_MACE;
        else
        p = TILEP_HAND1_CLUB_SLANT;

        doll.parts[TILEP_PART_HAND1] = p;
    }
    else
    if (sk == SK_SHORT_BLADES)
    {
        if (dam>20) p = TILEP_HAND1_SABRE;
        else
        if (dam>10) p = TILEP_HAND1_SHORT_SWORD_SLANT;
        else
        p = TILEP_HAND1_DAGGER_SLANT;

        doll.parts[TILEP_PART_HAND1] = p;
    }
    else
    if (sk == SK_LONG_SWORDS)
    {
        if (dam>25) p = TILEP_HAND1_GREAT_SWORD_SLANT;
        else
        if (dam>20) p = TILEP_HAND1_KATANA_SLANT;
        else
        if (dam>15) p = TILEP_HAND1_SCIMITAR;
        else
        if (dam>10) p = TILEP_HAND1_LONG_SWORD_SLANT;
        else
        p = TILEP_HAND1_FALCHION;

        doll.parts[TILEP_PART_HAND1] = p;
    }
    else
    if (sk == SK_AXES)
    {
        if (dam>30) p = TILEP_HAND1_EXECUTIONERS_AXE;
        else
        if (dam>20) p = TILEP_HAND1_BATTLEAXE;
        else
        if (dam>15) p = TILEP_HAND1_BROAD_AXE;
        else
        if (dam>10) p = TILEP_HAND1_WAR_AXE;
        else
        p = TILEP_HAND1_HAND_AXE;

        doll.parts[TILEP_PART_HAND1] = p;
    }
    else
    if (sk == SK_POLEARMS)
    {
        if (dam>30) p =  TILEP_HAND1_GLAIVE;
        else
        if (dam>20) p = TILEP_HAND1_SCYTHE;
        else
        if (dam>15) p = TILEP_HAND1_HALBERD;
        else
        if (dam>10) p = TILEP_HAND1_TRIDENT2;
        else
        if (dam>10) p = TILEP_HAND1_HAMMER;
        else
        p = TILEP_HAND1_SPEAR;

        doll.parts[TILEP_PART_HAND1] = p;
    }
    else
    if (sk == SK_BOWS)
        doll.parts[TILEP_PART_HAND1] = TILEP_HAND1_BOW2;
    else
    if (sk == SK_CROSSBOWS)
        doll.parts[TILEP_PART_HAND1] = TILEP_HAND1_CROSSBOW;
    else
    if (sk == SK_SLINGS)
        doll.parts[TILEP_PART_HAND1] = TILEP_HAND1_SLING;
    else
    if (sk == SK_UNARMED_COMBAT)
        doll.parts[TILEP_PART_HAND1] = doll.parts[TILEP_PART_HAND2] = 0;

    ImgClear(DollCacheImg);
    // Clear
    ImgCopyToTileImg(TILE_MONS_PLAYER_GHOST, DollCacheImg, 0, 0, 1);

    draw_doll(DollCacheImg, &doll);
    ImgCopyToTileImg(TILE_MONS_PLAYER_GHOST, DollCacheImg, 0, 0, 1, mask, false);
    redraw_spx_tcache(TILE_MONS_PLAYER_GHOST);
}

#define PDEMON_PARTS 3
static int PandemPartsOfs[PDEMON_PARTS]=
    { TILE_PANDEMO_WING_START,
      TILE_PANDEMO_BODY_START,
      TILE_PANDEMO_HEAD_START
    };

static int PandemPartsNum[PDEMON_PARTS]=
    { TILE_PANDEMO_WING_END-TILE_PANDEMO_WING_START+1,
      TILE_PANDEMO_BODY_END-TILE_PANDEMO_BODY_START+1,
      TILE_PANDEMO_HEAD_END-TILE_PANDEMO_HEAD_START+1
    };

static const char *PandemPartsName[PDEMON_PARTS]=
    { "Wing", "Body", "Head" };

void TilePandemInitAux(int *parts)
{
    if (!Options.use_qv_mode) return;

    // Assume 64x64 QV mode
    int sx, sy;
    int p;
    int tidx = simple_qv_tile(TILE_MONS_PANDEMONIUM_DEMON) - TILE_TOTAL;
    int dx = (tidx % TILE_PER_ROW_EX) * TILE_X_EX_QV;
    int dy = (tidx / TILE_PER_ROW_EX) * TILE_Y_EX_QV;

    for(p=0;p<PDEMON_PARTS;p++)
    {
        tidx = PandemPartsOfs[p] + parts[p];
        sx = (tidx % TILE_PER_ROW_EX) * TILE_X_EX_QV;
        sy = (tidx / TILE_PER_ROW_EX) * TILE_Y_EX_QV;

        ImgCopy(TileQvImg, sx, sy, TILE_X_EX_QV, TILE_Y_EX_QV,
            TileQvImg, dx, dy, ((p==0) ? 1:0) );
    }
    // Fix Cache
    redraw_spx_tcache(simple_qv_tile(TILE_MONS_PANDEMONIUM_DEMON));
}

void TilePandemInit(struct ghost_struct &gs)
{
    int parts[PDEMON_PARTS], p;
    unsigned long rnd = 0;
    for (unsigned int i = 0; i < strlen( gs.name ); i++)
        rnd += gs.name[i];

    rnd *= strlen( gs.name );

    for(p=0;p<PDEMON_PARTS;p++)
    {
        parts[p] = rnd / (0x7fffffff / PandemPartsNum[p] + 1);
        rnd *=54321;
        rnd &= 0x7fffffff;
    }
    TilePandemInitAux(parts);
}

void TileEditPandem()
{
    int parts[PDEMON_PARTS], p;
    char buf[128];

    for(p=0;p<PDEMON_PARTS;p++)
    {
        int max = PandemPartsNum[p]-1;
        sprintf(buf, "Pandem %s=(0-%d)", PandemPartsName[p], max);
        mpr(buf);
        mpr_on(MODE_MPR);
        get_input_line_tile(buf, 30);
        mpr_on(MODE_CRT);
        parts[p] = atoi(buf);
        if (parts[p] > max) parts[p]=max;
    }
    TilePandemInitAux(parts);
}

void TileInitItems()
{
#if 1
    for(int i=0; i<NUM_POTIONS; i++)
    {
        int special = you.item_description[IDESC_POTIONS][i];
        int tile0 = TILE_POTION_OFFSET + special % 14;
        int tile1 = TILE_POT_HEALING + i;

        ImgCopyFromTileImg(tile0, DollCacheImg, 0, 0, 1);
        ImgCopyFromTileImg(tile1, DollCacheImg, 0, 0, 0);
        ImgCopyToTileImg  (tile1, DollCacheImg, 0, 0, 1);
    }

    for(int i=0; i<NUM_WANDS; i++)
    {
        int special = you.item_description[IDESC_WANDS][i];
        int tile0 = TILE_WAND_OFFSET + special % 12;
        int tile1 = TILE_WAND_FLAME + i;

        ImgCopyFromTileImg(tile0, DollCacheImg, 0, 0, 1);
        ImgCopyFromTileImg(tile1, DollCacheImg, 0, 0, 0);
        ImgCopyToTileImg  (tile1, DollCacheImg, 0, 0, 1);
    }

    //Hack: call it again
    init_tileflag();
#endif
}

void get_bbg(int bg, int *new_bg, int *bbg)
{
    int bg0 = bg & TILE_FLAG_MASK;
    *bbg=simple_qv_tile(TILE_DNGN_FLOOR);
    *new_bg= bg;

#if 1 //QV
    if(Options.use_qv_mode)
    {
        if(bg0 >= simple_qv_tile(TILE_DNGN_UNSEEN) &&
           bg0 < simple_qv_tile(TILE_DNGN_ROCK_WALL_OFS))
        {
            *bbg = bg0;
            *new_bg = 0;
    }
    }else
#endif
    {
    if(bg0 == TILE_DNGN_UNSEEN ||
      (bg0 >= TILE_DNGN_ROCK_WALL_OFS && bg0 < TILE_DNGN_WAX_WALL) ||
       bg0 == 0)
        *bbg = 0;
    else
    if( bg0 >= TILE_DNGN_FLOOR && bg0 <= TILE_DNGN_SHALLOW_WATER)
    {
        *bbg = bg;
        *new_bg = 0;
    }
    }
}

int sink_mask_tile(int bg, int fg)
{
    int bg0 = bg & TILE_FLAG_MASK;
    if (fg == 0 || (fg & TILE_FLAG_FLYING)!=0) return 0;
/*
    if ( bg0 == simple_qv_tile(TILE_DNGN_SHALLOW_WATER))
        return TILE_MASK_SHALLOW_WATER;
    if ( bg0 == simple_qv_tile(TILE_DNGN_DEEP_WATER))
        return TILE_MASK_DEEP_WATER;
    if ( bg0 >= simple_qv_tile(TILE_DNGN_LAVA) &&
         bg0 <= simple_qv_tile(TILE_DNGN_LAVA)+3)
        return TILE_MASK_LAVA;
*/
    if ( bg0 >= simple_qv_tile(TILE_DNGN_LAVA) &&
         bg0 <= simple_qv_tile(TILE_DNGN_SHALLOW_WATER) + 3)
    {
        int result = TILE_SINK_MASK;//simple_qv_tile(TILE_SINK_MASK);
        if ((fg & TILE_FLAG_MASK) >= TILE_NORMAL)
            result = simple_qv_tile(TILE_SINK_MASK);
        //if ((fg & TILE_FLAG_MASK) >= TILE_NORMAL) result++;
        return result;
    }

    return 0;
}
#if 1 //QV
// Bubble sort back-background tiles according to their priority
void sort_bbg(int bbg[4], int bbg_region[4])
{
    int layer1, layer2;

    for(layer1 = 2; layer1>=0; layer1--)
    {
        for(layer2 = 0; layer2<=layer1; layer2++)
        {
            if ( tile_prio[bbg[layer2]] > tile_prio[bbg[layer2+1]] )
        {
        int tmp = bbg[layer2+1];
        bbg[layer2+1] = bbg[layer2];
        bbg[layer2] = tmp;

                tmp = bbg_region[layer2+1];
                bbg_region[layer2+1] = bbg_region[layer2];
                bbg_region[layer2] = tmp;
        }
    }
    }
}

void tcache_compose_qv(int kind, int ix, int *fg_dat, int *bg_dat){

  /*
   Compose tile image for a certain screen grid (32x16)
    from overlapping four 64x64 tiles (actually 4*fg and 4*bg)

   kind=0   kind=1
    /\         /\
   /0 \       /0 \     8 Regions of a 64x64 tile
   \H /\     /\D /     +-------+-------+
    \/1 \   /1 \/      |   A   |   E   |
    /\C /   \G /\      +-------+-------+
   /2 \/     \/2 \     |   B   |   F   |
   \F /\     /\B /     +-------+-------+
    \/3 \   /3 \/      |   C   |   G   |
     \A /   \E /       +-------+-------+
      \/     \/        |   D   |   H   |
                       +-------+-------+

  */

    const int region_of_layer[2][4] ={
    {TREGION_H_QV, TREGION_C_QV, TREGION_F_QV, TREGION_A_QV},
    {TREGION_D_QV, TREGION_G_QV, TREGION_B_QV, TREGION_E_QV}
    };

    int layer;
    int bbg[4], bbg0[4], new_bg[4], bbg_region[4];
    int cflag[4];
    int c=1;
    // 透過用マスク未使用のフラグ
    int bgflag = 1;
    static char mask[TILE_UX_QV*TILE_UY_QV];
    char *maskptr = NULL;

    char *fg_maskptr = NULL;
    // カーソル透過用マスク未使用のフラグ
    int  cursor_mask_flag = 1;

    bool do_floor_mask[4];
    bool can_see[4];

    int sink;
    img_type tc_img = tcache_image[kind];
    //int wx = tcache_wx_qv[kind];
    //int wy = tcache_wy_qv[kind];

    for (layer=0; layer<4; layer++)
    {
    int region = region_of_layer[kind][layer];
        bbg_region[layer] = region;
    get_bbg(bg_dat[layer], &new_bg[layer], &bbg[layer]);
#if 0 // WALLBG
        if (tile_is_wall(bg_dat[layer])) bbg[layer]++;
#endif
    tile_check_blank(bbg[layer], 1<<region);
    tile_check_blank(new_bg[layer], 1<<region);
        bbg0[layer]=bbg[layer]; // Backup unsorted data
    }

    // priority の高い水面などを後で描画するためソート
    // 水面が上下左右の床にしみだすエフェクトのための処理
    sort_bbg(bbg, bbg_region);

    // 床や水面は壁やモンスターなどより先に全てのレイヤを描画しておく。
    // こうしないと後ろのモンスターが床で上書きされる
    for (layer=0; layer<4; layer++)
    {
        if(bbg[layer])
    {
        tcache_overlay(tc_img, ix, bbg[layer], bbg_region[layer], &c, NULL);
    }
    }

    //視界外メッシュ＆カーソルの描画
    for (layer=0; layer<4; layer++)
    {
        int region = region_of_layer[kind][layer];
    cflag[layer] = bg_dat[layer]&TILE_FLAG_CURSOR;
        do_floor_mask[layer] = false;
    can_see[layer] = true;

    //if ((bbg0[layer]&TILE_FLAG_MASK)==simple_qv_tile(TILE_DNGN_FLOOR))
        //can_see[layer] = false;
        //Unseen mesh  視界外は床面に黒メッシュを上書き
        if((bg_dat[layer]&TILE_FLAG_UNSEEN) != 0)
        {
        can_see[layer]=false;
            tcache_overlay(tc_img, ix, simple_qv_tile(TILE_MESH),
                                region, &c, NULL);
        }

        //Draw Tile cursor
        // カーソルの位置では壁の透過処理を行うため床画像を mask 登録
        if(cflag[layer])
        {
            int type = TILE_CURSOR;
            if(cflag[layer] == TILE_FLAG_CURSOR2)
                type = TILE_CURSOR2;
            else
            if(cflag[layer] == TILE_FLAG_CURSOR3)
                type = TILE_CURSOR3;

            tcache_overlay(tc_img, ix, simple_qv_tile(type), region, &c, NULL);
            do_floor_mask[layer] = true;
        cflag[layer] = simple_qv_tile(type);
        }
    }


    for (layer=0; layer<4; layer++)
    {
    int region = region_of_layer[kind][layer];
    bool is_wall = false;
    int bg = new_bg[layer]&TILE_FLAG_MASK;
    int fg = fg_dat[layer]&TILE_FLAG_MASK;
    int bg0 =  bbg0[layer]&TILE_FLAG_MASK;
    int is_pet = fg_dat[layer]&TILE_FLAG_PET;
    int s_under = fg_dat[layer]&TILE_FLAG_S_UNDER;

        if (tile_is_wall(bg)) is_wall = true;

    if ((!is_wall) && can_see[layer])
        do_floor_mask[layer] = true;

    if (do_floor_mask[layer])
    {
            register_mask(bg0, region, &bgflag, mask);
            maskptr = mask;
    }

    //Background  (Walls, stairs, traps, statues, etc)
    if(bg)
    {
        // マスクがある場合は壁を透過処理
        if (is_wall)
            tcache_overlay(tc_img, ix, bg, region, &c, maskptr);
        else
            tcache_overlay(tc_img, ix, bg, region, &c, NULL);

        // 下り階段などが透過するようマスク登録  壁の場合(prio==10)は
        // 行わない
            if ((layer != 3) && !is_wall)
            {
                register_mask(bg,  region, &bgflag, mask);
                register_mask(bg0, region, &bgflag, mask);
                maskptr = mask;
            }
    }

    //Foreground
    if(fg_dat[layer])
    {
            if((cflag[layer] != 0) &&
               (cflag[layer] != simple_qv_tile(TILE_CURSOR3))) c = 2;

            //Sink mask (water, lava, etc)
            sink = sink_mask_tile(bg0, fg_dat[layer]);

            if(fg >=TILE_NORMAL)
        {
        if (sink)
        {
            // デカキャラ水没処理
            int flag=2;
                    register_mask(sink, region, &flag, sink_mask);
                    tcache_overlay(tc_img, ix, fg, region, &c, sink_mask);
        }
        else
        {
                    if(cflag[layer])
                    {
                        // カーソルを透過
                        register_mask(cflag[layer], region, &cursor_mask_flag,
                                        cursor_mask);
                        fg_maskptr = cursor_mask;
                    }
                    tcache_overlay(tc_img, ix, fg, region, &c, fg_maskptr);
        }
        }
        else
            {
                if (sink)
                {
                    int flag=2;
                    register_mask(sink, region, &flag, sink_mask, true);
                    tcache_overlay(tc_img, ix, fg, region, &c, sink_mask);
                }
                else
                // 32x32 normal
            tcache_overlay(tc_img, ix, fg, region, &c, NULL);
            }

        // 何か物があるときは透過処理のため床と物をマスク登録
            if (layer != 3)
        {
                register_mask(fg, region, &bgflag, mask);
            register_mask(bg0, region, &bgflag, mask);
                maskptr = mask;
        }

            if(s_under)
                tcache_overlay(tc_img, ix, TILE_SOMETHING_UNDER,
                               region, &c, NULL);
    }

    //Pet mark
    if(is_pet)
            tcache_overlay(tc_img, ix, simple_qv_tile(TILE_HEART),
                region, &c, NULL);

    }

}
#endif

//normal
void tcache_compose_normal(int ix, int *fg, int *bg){
    int bbg;
    int new_bg;
    int c=1;
    int fg0=fg[0];
    int bg0=bg[0];
    int sink;
    img_type tc_img = tcache_image[TCACHE0_NORMAL];

    get_bbg(bg0, &new_bg, &bbg);

    if(bbg) tcache_overlay(tc_img, ix, bbg, TREGION_0_NORMAL, &c, NULL);
    if(new_bg) tcache_overlay(tc_img, ix, new_bg, TREGION_0_NORMAL, &c, NULL);

    //Tile cursor
    if(bg0&TILE_FLAG_CURSOR)
    {
       int type = ((bg0&TILE_FLAG_CURSOR) == TILE_FLAG_CURSOR1) ?
            TILE_CURSOR : TILE_CURSOR2;
       tcache_overlay(tc_img, ix, type, TREGION_0_NORMAL, &c, NULL);
       c = 2;// Hilite
    }

    if(fg0)
    {
        sink = sink_mask_tile(bg0, fg0);
        if (sink)
        {
            int flag=2;
            register_mask(sink, TREGION_0_NORMAL, &flag, sink_mask);
            tcache_overlay(tc_img, ix, fg0, TREGION_0_NORMAL, &c, sink_mask);
        }
        else
            tcache_overlay(tc_img, ix, fg0, TREGION_0_NORMAL, &c, NULL);
    }
#if 0
    sink = sink_mask_tile(bg0, fg0);
    if(sink) tcache_overlay(tc_img, ix, sink, TREGION_0_NORMAL, &c, NULL);
#endif

    if(fg0 & TILE_FLAG_S_UNDER)
        tcache_overlay(tc_img, ix, TILE_SOMETHING_UNDER,
                        TREGION_0_NORMAL, &c, NULL);

    //Pet mark
    if(fg0&TILE_FLAG_PET)
         tcache_overlay(tc_img, ix, TILE_HEART, TREGION_0_NORMAL, &c, NULL);

    if(bg0&TILE_FLAG_UNSEEN)
         tcache_overlay(tc_img, ix, TILE_MESH, TREGION_0_NORMAL, &c, NULL);

}

// Tile cursor
void TileCursor(int x, int y, int flag)
{
    if (tile_cursor_x == x && tile_cursor_y == y && tile_cursor_flag == flag)
        return;
    // Redraw old cursor position
    if (tile_cursor_flag != 0 && (tile_cursor_x != x || tile_cursor_y != y|| flag==0))
    {
        t2buf[tile_cursor_x+1][tile_cursor_y+1] &= ~ TILE_FLAG_CURSOR;
        t2buf[tile_cursor_x+1][tile_cursor_y+1] |= old_cursor_flag;
    update_single_grid(tile_cursor_x, tile_cursor_y);
    }
    tile_cursor_x = x;
    tile_cursor_y = y;
    tile_cursor_flag = flag;

    if (flag)
    {
      // Draw cursor
        old_cursor_flag = t2buf[tile_cursor_x+1][tile_cursor_y+1] & TILE_FLAG_CURSOR;
    t2buf[tile_cursor_x+1][tile_cursor_y+1] &= ~ TILE_FLAG_CURSOR;
        if (flag == 1)
        t2buf[tile_cursor_x+1][tile_cursor_y+1] |= TILE_FLAG_CURSOR1;
    else
    if(flag == 2)
        t2buf[tile_cursor_x+1][tile_cursor_y+1] |= TILE_FLAG_CURSOR2;
        else
        if(flag == 3)
        t2buf[tile_cursor_x+1][tile_cursor_y+1] |= TILE_FLAG_CURSOR3;

        update_single_grid(tile_cursor_x, tile_cursor_y);

    }
}

void TileDrawBolt(int x, int y, int fg){

    t1buf[x+1][y+1]=fg | TILE_FLAG_FLYING;
    update_single_grid(x, y);
}

//Draw the tile screen once and for all
void TileDrawDungeon(short unsigned int *tileb){
    int count, x, y, kind;

    if(!TileImg)return;

    old_cursor_flag = 0; // Clear backup

    count = 0;
    for(y=0;y<TILE_DAT_YMAX;y++){
    for(x=0;x<TILE_DAT_XMAX;x++){
        if(tileb[count]==tileb[count+1]) tileb[count]=0;
        t1buf[x+1][y+1] = tileb[count];
        count ++;
        t2buf[x+1][y+1] = tileb[count];
        count ++;
    }}

    for(kind=0;kind<tcache_kind;kind++)
    {
        for(x=0;x<tile_xmax;x++){
        for(y=0;y<tile_ymax;y++){
            tile_draw_grid(kind, x, y);
        }}
    }

    force_redraw_tile = false;
    TileDrawDungeonAux();
    region_tile->redraw();
}

// Load optional wall tile
static void TileLoadWallAux(int idx_src, int idx_dst, img_type wall)
{
    int tile_per_row = ImgWidth(wall) / TILE_X;

    int sx = idx_src % tile_per_row;
    int sy = idx_src / tile_per_row;

    sx *= TILE_X;
    sy *= TILE_Y;

    ImgCopyToTileImg(idx_dst, wall, sx, sy, 1);
}

// Load optional wall tile
static void TileLoadWallAuxQV(int idx_src, int idx_dst, img_type wall)
{
    int tile_per_row = ImgWidth(wall) / TILE_X_EX_QV;

    int sx = idx_src % tile_per_row;
    int sy = idx_src / tile_per_row;
    int dx = idx_dst % TILE_PER_ROW_EX;
    int dy = idx_dst / TILE_PER_ROW_EX;

    sx *= TILE_X_EX_QV;
    sy *= TILE_Y_EX_QV;
    dx *= TILE_X_EX_QV;
    dy *= TILE_Y_EX_QV;

    ImgCopy(wall, sx, sy, TILE_X_EX_QV, TILE_Y_EX_QV,
            TileQvImg, dx, dy, 1);
}

const char *WallNameQV()
{
    if (you.level_type == LEVEL_PANDEMONIUM)
    {
         switch (mcolour[env.mons_alloc[8]])
         {
        case BLACK:
        case BLUE:
        case LIGHTBLUE:
        return "pandem1";

        case RED:
        case LIGHTRED:
                return "pandem2";

        case MAGENTA:
        case LIGHTMAGENTA:
                return "pandem3";

        case GREEN:
        case LIGHTGREEN:
                return "pandem4";

        case CYAN:
        case LIGHTCYAN:
        case WHITE:
                return "lapis";

        case BROWN:
        case YELLOW:
        return "pandem6";

        default:
        return "lapis";
         }// switch
    }
    else if (you.level_type == LEVEL_ABYSS)
    {
        return "normal";
    }
    else if (you.level_type == LEVEL_LABYRINTH)
    {
        return "normal";
    }
    else switch(you.where_are_you)
    {
        case BRANCH_HIVE:
        return "hive";

        case BRANCH_ELVEN_HALLS:
            return "lapis";

        case BRANCH_TARTARUS:
        case BRANCH_CRYPT:
            return "crypt";

        case BRANCH_TOMB:
            return "pyramid";

    case BRANCH_MAIN_DUNGEON:
            return "rock";

    case BRANCH_DIS:
        case BRANCH_HALL_OF_BLADES:
        case BRANCH_VAULTS:
        case BRANCH_ECUMENICAL_TEMPLE:
        case BRANCH_VESTIBULE_OF_HELL:
            return "normal";

    case BRANCH_GEHENNA:
        case BRANCH_INFERNO:
        case BRANCH_THE_PIT:
            return "gehena";

    case BRANCH_COCYTUS:
            return "cocutos";

    case BRANCH_ORCISH_MINES:
            return "tunnel";

    case BRANCH_LAIR:
            return "lair";

    case BRANCH_SLIME_PITS:
            return "moss";

        case BRANCH_SNAKE_PIT:
            return "mucus";

        case BRANCH_SWAMP:
            return "tunnel";

    case BRANCH_HALL_OF_ZOT:
            if (you.your_level - you.branch_stairs[7] <= 1)
            {
                return "pandem6";
            }
            else
            {
                switch (you.your_level - you.branch_stairs[7])
                {
                    case 2:
                        return "pandem4";
                    case 3:
                        return "lapis";
                    case 4:
                        return "pandem1";
                    case 5:
            default:
                        return "pandem3";
                }//switch
            }

    default:
        return "normal";
    }//switch
}

const char *WallName()
{
    if (you.level_type == LEVEL_PANDEMONIUM)
    {
        switch (mcolour[env.mons_alloc[8]])
        {
            case BLACK:
        case BLUE:
        case LIGHTBLUE:
        return "z_blue";

        case RED:
        case LIGHTRED:
               return "z_red";

            case MAGENTA:
        case LIGHTMAGENTA:
                return "z_magenta";

        case GREEN:
        case LIGHTGREEN:
                return "z_green";

        case CYAN:
        case LIGHTCYAN:
        case WHITE:
                return "z_cyan";

        case BROWN:
        case YELLOW:
            return "z_yellow";

        default:
            return "z_gray";
        }
    }
    else if (you.level_type == LEVEL_ABYSS)
    {
    return "undead";
    }
    else if (you.level_type == LEVEL_LABYRINTH)
    {
    return "undead";
    }
    else switch(you.where_are_you)
    {
        case BRANCH_MAIN_DUNGEON:
            return "normal";

        case BRANCH_HIVE:
        return "hive";

        case BRANCH_VAULTS:
            return "vault";

        case BRANCH_ELVEN_HALLS:
        case BRANCH_HALL_OF_BLADES:
        case BRANCH_ECUMENICAL_TEMPLE:
            return "hall";

        case BRANCH_TARTARUS:
        case BRANCH_CRYPT:
        case BRANCH_VESTIBULE_OF_HELL:
            return "undead";

        case BRANCH_TOMB:
            return "tomb";

        case BRANCH_DIS:
            return "z_cyan";

    case BRANCH_GEHENNA:
        case BRANCH_INFERNO:
        case BRANCH_THE_PIT:
            return "z_red";

        case BRANCH_COCYTUS:
            return "ice";

        case BRANCH_ORCISH_MINES:
            return "orc";

    case BRANCH_LAIR:
            return "lair";

    case BRANCH_SLIME_PITS:
            return "slime";

        case BRANCH_SNAKE_PIT:
            return "snake";

        case BRANCH_SWAMP:
            return "swamp";

        case BRANCH_HALL_OF_ZOT:
            if (you.your_level - you.branch_stairs[7] <= 1)
            {
                return "z_yellow";
            }
            else
            {
                switch (you.your_level - you.branch_stairs[7])
                {
                    case 2:
                        return "z_green";
                    case 3:
                        return "z_cyan";
                    case 4:
                        return "z_blue";
                    case 5:
                default:
                        return "z_magenta";
                }
              }

    default:
        return "normal";
    }
}
void TileLoadWall(bool wizard)
{
    static char buf[128];
    static const char *oldname = "normal";
    static const char *newname;
    img_type wall;

    const char *dirname = (Options.use_qv_mode) ? "walls":"walls2d";

    if (wizard)
    {
        oldname = "";
        mpr("BMP name of wall tile=");
        mpr_on(MODE_MPR);
        get_input_line_tile(buf, 40);
        newname = (const char *)buf;
        mpr_on(MODE_CRT);
    }
    else
    {
        if (Options.use_qv_mode)
            newname = WallNameQV();
        else
            newname = WallName();
    }

#if DEBUG_DIAGNOSTICS
    sprintf(info, "old wall name=%s",oldname);
    mpr( info, MSGCH_DIAGNOSTICS );
    sprintf(info, "new wall name=%s",newname);
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    if (strcmp(newname, oldname)!=0)
    {
        int i;
    int idx_src;
    int idx_dst;

        char fname[256];
#ifdef USE_X11
        sprintf(fname,"%s/%s", dirname, newname);
#elif defined(WINDOWS)
        sprintf(fname,"%s\\%s", dirname, newname);
#elif defined(MAC)
        not coded yet
#endif

#if DEBUG_DIAGNOSTICS
        sprintf(info, "Loading %s", fname);
        mpr( info, MSGCH_DIAGNOSTICS );
#endif

        wall = ImgLoadFileSimple(fname);
        if (!wall)
        {
            strcat(fname, " :Couldn't load it.");
            mpr(fname);
            return;
        }
        clear_tcache();
    force_redraw_tile = true;
    if (newname) oldname = newname;

        if (Options.use_qv_mode)
        {
            //Walls
            for (i=0;i<16;i++)
            {
                idx_src = i;
            idx_dst = simple_qv_tile(TILE_DNGN_ROCK_WALL_OFS) + i - TILE_NORMAL;
            TileLoadWallAuxQV(idx_src, idx_dst, wall);
            }

            // Closed Door
            for (i=0;i<2;i++)
            {
                idx_src = 16 + i;
                idx_dst = simple_qv_tile(TILE_DNGN_CLOSED_DOOR) + i - TILE_NORMAL;
                TileLoadWallAuxQV(idx_src, idx_dst, wall);
            }

            // Open Door
            for (i=0;i<2;i++)
            {
                idx_src = 18 + i;
                idx_dst = simple_qv_tile(TILE_DNGN_OPEN_DOOR) + i - TILE_NORMAL;
                TileLoadWallAuxQV(idx_src, idx_dst, wall);
            }

            // Floor
            for (i=0;i<4;i++)
            {
                idx_src = 20+i;
                idx_dst = simple_qv_tile(TILE_DNGN_FLOOR) + i - TILE_NORMAL;
                TileLoadWallAuxQV(idx_src, idx_dst, wall);
            }
        }
        else
        {
            for (i=0;i<4;i++)
            {
                idx_src = i;
                idx_dst = TILE_DNGN_ROCK_WALL_OFS + i;
                TileLoadWallAux(idx_src, idx_dst, wall);

                idx_src = 4 + i;
                idx_dst = TILE_DNGN_FLOOR + i;
                TileLoadWallAux(idx_src, idx_dst, wall);
            }
        }
        ImgDestroy(wall);
        return;
    }
}

#if 1 // TILE_MON_EQUIP
// Monster weapon tile
#define N_MCACHE (TILE_MCACHE_END - TILE_MCACHE_START +1)

typedef struct mcache mcache;
struct mcache
{
  bool lock_flag;
  mcache *next;
  int mon_tile;
  int equ_tile;
  int idx;
};

mcache mc_data[N_MCACHE];

mcache *mc_head = NULL;

static void mcache_compose(int tile_idx, int mon_tile, int equ_tile)
{
    int xs, ys;
    int tidx = tilep_parts_start[TILEP_PART_HAND1];
    int nx = tilep_parts_nx[TILEP_PART_HAND1];
    int ny = tilep_parts_ny[TILEP_PART_HAND1];
    int ox = tilep_parts_ox[TILEP_PART_HAND1];
    int oy = tilep_parts_oy[TILEP_PART_HAND1];
    int wx = TILE_X/nx;
    int wy = TILE_Y/ny;
    int idx = equ_tile -1;

    int ofs_x=0;
    int ofs_y=0;

    switch(mon_tile)
    {
        case TILE_MONS_ORC:
        case TILE_MONS_URUG:
        case TILE_MONS_BLORK_THE_ORC:
        case TILE_MONS_ORC_WARRIOR:
        case TILE_MONS_ORC_KNIGHT:
        case TILE_MONS_ORC_WARLORD: ofs_y = 2; break;

        case TILE_MONS_GOBLIN:
        case TILE_MONS_IJYB:  ofs_y = 4; break;
        case TILE_MONS_GNOLL: ofs_x = -1;break;
        case TILE_MONS_BOGGART: ofs_y = 2;break;

        case TILE_MONS_DEEP_ELF_FIGHTER:
        case TILE_MONS_DEEP_ELF_SOLDIER: ofs_y = 2;break;
        case TILE_MONS_DEEP_ELF_KNIGHT: ofs_y = 1; break;

        case TILE_MONS_MIDGE: ofs_y = -2;break;

        case TILE_MONS_NAGA:
        case TILE_MONS_NAGA_WARRIOR:
        case TILE_MONS_GREATER_NAGA: ofs_y = 1; break;

        case TILE_MONS_HELL_KNIGHT: ofs_y = 3; ofs_x = -1; break;

        case TILE_MONS_VAMPIRE_KNIGHT: break;
    }

    tidx += idx/(nx*ny);
    //Source pos
    xs = (tidx % TILEP_PER_ROW)*TILE_X;
    ys = (tidx / TILEP_PER_ROW)*TILE_Y;
    xs += (idx % nx)*(TILE_X/nx);
    ys += ((idx/nx) % ny)*(TILE_Y/ny);

    if (ofs_x < 0)
    {
        xs -= ofs_x;
        wx += ofs_x;
    }
    else
    if (ofs_x > 0)
    {
        ox += ofs_x;
        wx -= ofs_x;
    }

    if (ofs_y < 0)
    {
        ys -= ofs_y;
        wy += ofs_y;
    }
    else
    if (ofs_y > 0)
    {
        oy += ofs_y;
        wy -= ofs_y;
    }

    // Copy monster tile
    ImgCopyFromTileImg(mon_tile, DollCacheImg, 0, 0, 1);

    // Overlay weapon tile
    ImgCopy(PlayerImg, xs, ys, wx, wy,
                  DollCacheImg, ox, oy, 0);

    // Copy to the buffer
    ImgCopyToTileImg(tile_idx, DollCacheImg, 0, 0, 1);

    redraw_spx_tcache(tile_idx);
}

static void mcache_init()
{
    int i;

    for(i=0;i<N_MCACHE;i++)
    {
        mc_data[i].lock_flag = false;
        mc_data[i].next = NULL;
        if (i!=N_MCACHE-1)
            mc_data[i].next = &mc_data[i+1];
        mc_data[i].idx = TILE_MCACHE_START + i;
        mc_data[i].mon_tile = 0;
        mc_data[i].equ_tile = 0;
    }
    mc_head = &mc_data[0];
}

void TileMcacheUnlock()
{
    int i;

    for(i=0;i<N_MCACHE;i++)
    {
        mc_data[i].lock_flag = false;
    }
}

int TileMcacheFind(int mon_tile, int equ_tile)
{
    mcache *mc = mc_head;
    mcache *prev = NULL;
    mcache *empty = NULL;
#ifdef DEBUG_DIAGNOSTICS
    int count = 0;
    char info[40];
#endif

    while(1){
        if(mon_tile == mc->mon_tile && equ_tile == mc->equ_tile)
        {
             // 一致
             //キャシュを先頭に
             if (prev != NULL) prev->next = mc->next;
             if (mc != mc_head) mc->next = mc_head;
             mc_head = mc;

             //キャッシュをロック
             mc->lock_flag=true;
#ifdef DEBUG_DIAGNOSTICS
             snprintf( info, 39, "mcache (M %d, E %d) found count=%d",
                        mon_tile, equ_tile, count);
             mpr(info, MSGCH_DIAGNOSTICS );
#endif
             return mc->idx;
        }
        if (!mc->lock_flag) empty = mc;
        if (mc->next == NULL) break;
        prev = mc;
        mc = mc->next;

#ifdef DEBUG_DIAGNOSTICS
        count++;
#endif

    }//while

    // Not found
    if(empty == NULL)
    {
#ifdef DEBUG_DIAGNOSTICS
             snprintf( info, 39, "mcache (M %d, E %d) cache full",
                        mon_tile, equ_tile);
             mpr(info, MSGCH_DIAGNOSTICS );
#endif
        return mon_tile;
    }
    mc = empty;

#ifdef DEBUG_DIAGNOSTICS
             snprintf( info, 39, "mcache (M %d, E %d) newly composed",
                        mon_tile, equ_tile);
             mpr(info, MSGCH_DIAGNOSTICS );
#endif

    // タイルを合成
    mcache_compose(mc->idx, mon_tile, equ_tile);
    mc->mon_tile = mon_tile;
    mc->equ_tile = equ_tile;
    //キャシュを先頭に
    if (prev) prev->next = mc->next;
    if(mc != mc_head) mc->next = mc_head;
    mc_head = mc;
    mc->lock_flag=true;
    return mc->idx;
}
#endif

void TileDrawGmap(unsigned char *buf)
{
    region_map->flag = true;
    region_map->draw_data(buf);
}

void TileDrawTitle()
{
    int winx, winy, tx, ty, x, y;
    img_type TitleImg = ImgLoadFileSimple("title");
    img_type pBuf = region_tile->backbuf;
    if (!TitleImg || !pBuf) return;

    tx  = ImgWidth(TitleImg);
    ty  = ImgHeight(TitleImg);
    winx = ImgWidth(pBuf);
    winy = ImgHeight(pBuf);

    if (tx > winx)
    {
        x = 0;
        tx = winx;
    }
    else
    {
        x = (winx - tx)/2;
    }

    if (ty > winy)
    {
        y = 0;
        ty = winy;
    }
    else
    {
        y = (winy - ty)/2;
    }

    //パレットが同じと仮定
    ImgCopy(TitleImg, 0, 0, tx, ty, pBuf, x, y, 1);
    region_tile->make_active();
    region_tile->redraw();
    ImgDestroy(TitleImg);
}

#if 1 //Slot
void TilePutch(int c, img_type Dest, int dx, int dy)
{
    int tidx = TILE_CHAR00 + (c-32)/8;
    int tidx2 = c & 7;

    int sx = (tidx % TILE_PER_ROW)*TILE_X + (tidx2 % 4)*(TILE_X/4);
    int sy = (tidx / TILE_PER_ROW)*TILE_Y + (tidx2 / 4)*(TILE_Y/2);;

    ImgCopy(TileImg, sx, sy, TILE_X/4, TILE_Y/2,
              Dest, dx, dy, 0);
}

void TileDrawOneItem(TileRegionClass *r, int dx, int dy,
                     int tile, int num, bool floor, bool select,
                     bool tried)
{
    int sx, sy;

    if (tile == 0)
    {
        ImgCopyFromTileImg(TILE_DNGN_UNSEEN, r->backbuf, dx, dy, 1);
        return;
    }

    if (floor)
    {
        // Floor
        ImgCopyFromTileImg(TILE_DNGN_FLOOR, r->backbuf, dx, dy, 1);

        if (select)
        {
            ImgCopyFromTileImg(TILE_CURSOR, r->backbuf, dx, dy, 0);
        }
    }
    else
    {
        ImgCopyFromTileImg(TILE_ITEM_SLOT, r->backbuf, dx, dy, 1);

        if (select)
        {
            ImgCopyFromTileImg(TILE_ITEM_SLOT_EQUIP, r->backbuf, dx, dy, 0);
        }
    }

    ImgCopyFromTileImg(tile, r->backbuf, dx, dy, 0);

    if (num != -1)
    {
        int c1 = num/10;
        int c2 = num%10;
        if (c1) TilePutch('0'+ c1, r->backbuf, dx,dy);
        TilePutch('0'+ c2, r->backbuf, dx+TILE_X/4,dy);
    }

    if (tried)
    {
        TilePutch('?', r->backbuf, dx, dy + TILE_Y/2);
    }
}

void TileDrawInven(int n, int flag, int *tiles, int *num, int *idx, int *iflags)
{
    int i;
    TileRegionClass *r = region_item;
    if (flag == 2)
        r = region_item2;

    r->flag = true;

    int item_x = r->mx;
    int item_y = r->my;

    for (i=0;i<item_x*item_y;i++)
    {
        if (i==MAX_ITEMLIST) break;
        int dx = (i % item_x)*TILE_X;
        int dy = (i / item_x)*TILE_Y;

        if ( flag == itemlist_flag
             && tiles[i]  == itemlist[i]
             && num[i]  == itemlist_num[i]
             && idx[i]  == itemlist_idx[i]
             && iflags[i] == itemlist_iflag[i]) continue;

        itemlist[i] = tiles[i];
        itemlist_num[i] = num[i];
        itemlist_idx[i] = idx[i];
        itemlist_iflag[i] = iflags[i];

        TileDrawOneItem(r, dx, dy, tiles[i], num[i],
            (idx[i] >= ENDOFPACK),
            ((iflags[i]&TILEI_FLAG_SELECT) != 0),
            ((iflags[i]&TILEI_FLAG_TRIED) != 0));
    }

    r->make_active();
    r->redraw();
    itemlist_flag = flag;
}
#endif

#if 1  // MOUSE

#define MOUSE_IN_NONE 0
#define MOUSE_IN_TEXT_DUNGEON 1
#define MOUSE_IN_TILE_DUNGEON 2
#define MOUSE_IN_MINIMAP 3
#define MOUSE_IN_ITEMLIST1 4
#define MOUSE_IN_ITEMLIST2 5
#define MOUSE_IN_STAT 6

// Tiptext help info
typedef struct tip_info
{
    int sx;
    int ex;
    int sy;
    int ey;
    const char *tiptext;
    const char *tipfilename;
} tip_info;

const tip_info Tips[] ={
#ifdef JP
  {0,12,0,0, "あなたの名前と称号", NULL},
  {0,12,1,1, "あなたの種族", NULL},
  {0,15,2,2, "ヒットポイント", NULL},
  {20,39,2,2,
    "HPバー：右端が最大値、"
    "赤い部分は直前に受けたダメージを表します", NULL},
  {0,12,3,3, "マジックポイント", NULL},
  {0,12,4,4, "アーマークラス(右クリックで詳細)", "stat_acev"},
  {0,12,5,5, "回避力(右クリックで詳細)", "stat_acev"},
  {0,12,6,6,
    "腕力：戦闘時に与えるダメージ量に影響し、"
    "荷物を持ち運ぶ力にも影響します"
    , NULL},
  {0,12,7,7,
    "知力：魔法のアイテムを使用する能力、"
    "どのくらい良く呪文を唱えられるかに影響します"
    , NULL},
  {0,12,8,8,
    "器用さ：戦闘、投擲・射撃、攻撃回避、"
    "盗賊系スキルの使用に使われます"
    , NULL},
  {0,12,9,9,
    "所持金：所持金はスコアに合計されると同時に、店での買物にも使います"
    , NULL},
  {0,16,10,10, "レベル、経験値、未使用の技能経験値(右クリックで詳細)",
    "stat_exp"},
  {0,12,11,11, "ダンジョン内での位置", NULL},
  {0,12,12,12, "装備している武器(右クリックで詳細)", "stat_wep"},
  {-1, -1, -1, -1, NULL, NULL}
#else
  {0,12,0,0, "Your name and title", NULL},
  {0,12,1,1, "Your race", NULL},
  {0,12,2,2, "Hit points", NULL},
  {20,39,2,2,
    "HP Bar: When HP is full, the bar reaches the right edge."
    "Red part shows the damage you took from the latest attack", NULL},
  {0,12,3,3, "Magic points", NULL},
  {0,12,4,4, "Armour Class (R-Click for detail)", "stat_acev"},
  {0,12,5,5, "Evasion (R-Click for detail)", "stat_acev"},
  {0,12,6,6,
    "Strength: It affects the amount of damage you do in combat,"
    " as well as how much stuff you can carry."
    , NULL},
  {0,12,7,7,
    "Intelligence: It affects how well you can cast spells as well as"
    " your ability to use some magical items."
    , NULL},
  {0,12,8,8,
    "Dexterity: It affects your accuracy in combat, your general effectiveness"
    " with missile  weapons, your ability to dodge attacks aimed at you,"
    " and your ability to use thiefly skills such as backstabbing and"
    " disarming traps."
    , NULL},
  {0,12,9,9,
    "Gold: This is how much money you're carrying. Money adds to your final"
    " score, and can be used to purchase items in shops."
    , NULL},
  {0,16,10,10, "Your experience level, experience points,"
               " and pooled skill points (R-Click for detail)",
    "stat_exp"},
  {0,12,11,11, "Your position in the dungeon", NULL},
  {0,12,12,12, "Your weapon (R-Click for detail)", "stat_wep"},
  {-1, -1, -1, -1, NULL, NULL}
#endif
};

int convert_cursor_pos(int mx, int my, int *cx, int *cy)
{
    std::vector <RegionClass *>::iterator r;
    WinClass *w = win_main;
    RegionClass *r0 = NULL;
    int x, y;

    for (r = w->regions.begin();r != w->regions.end();r++)
    {
        if (! (*r)->is_active()) continue;
        if( (*r)->mouse_pos(mx, my, cx, cy))
        {
            r0 = *r;
            break;
        }
    }
    if (!r0) return MOUSE_IN_NONE;

    x = *cx;
    y = *cy;

/********************************************/

    if(r0 == region_crt)
    {
        if (Options.use_tile) return MOUSE_IN_NONE;

        if (x >= 39 && y < 18)
        {
            *cx = x - 39;
            return  MOUSE_IN_STAT;
        }
#ifdef JP
        if (Options.use_zenkaku)
        {
            x = (x-1)/2;
            if (x<0 || y<0 || x>= 17 || y>= 17) return MOUSE_IN_NONE;
        }
        else
#endif
        {
            x -= 9;
            if (x<-8 || y<0 || x>= 25 || y>= 17) return MOUSE_IN_NONE;
        }
        *cx = x;
        *cy = y;
        return MOUSE_IN_TEXT_DUNGEON;
    }

#if 1 //Slot
    if (r0 == region_item)
    {
        return MOUSE_IN_ITEMLIST1;
    }
    if (r0 == region_item2)
    {
        return MOUSE_IN_ITEMLIST2;
    }
#endif

    if (r0 == region_tile)
    {
        if (Options.use_qv_mode)
        {
            // QV Tile
            int tx = (2*y + x) / TILE_UX_QV;
            int ty = (2*y - x + TILE_UX_QV * 100) / TILE_UX_QV - 100;
            x = (tx)/2 -4;
            y = (ty + 100)/2 -50 + 8;
        }
        else
        {
            // Normal tile
            x /= TILE_X;
            y /= TILE_Y;
        }

        // Out of LOS range
        if (x<0 || y<0 || x>16 || y>16) return MOUSE_IN_NONE;
        *cx = x;
        *cy = y;
        return MOUSE_IN_TILE_DUNGEON;
    }

    if (r0 == region_map)
    {
        *cx = 8+x;
        *cy = 8+y;
        return MOUSE_IN_MINIMAP;
    }

    if (r0 == region_stat)
    {
        return MOUSE_IN_STAT;
    }

    return MOUSE_IN_NONE;
}

int handle_mouse_motion(int mouse_x, int mouse_y)
{
    static int oldcx = -1;
    static int oldcy = -1;
    static int oldmode = 0;
    int flag;
    int cx,cy;
    int mode;

    if(keyin_mode==KEYIN_MODE_NONE) return 0;
    if(keyin_mode==KEYIN_MODE_MORE) return 0;
    if(keyin_mode==KEYIN_MODE_MAP) return 0;
    if(keyin_mode==KEYIN_MODE_PAPER_DOLL) return 0;
    if(keyin_mode==KEYIN_MODE_END) return 0;

    mode = convert_cursor_pos(mouse_x, mouse_y, &cx, &cy);
    if (oldcx == cx && oldcy == cy && oldmode == mode) return 0;

#if 1 //Slot
    if (mode == MOUSE_IN_ITEMLIST1 || mode == MOUSE_IN_ITEMLIST2)
    {
        TileRegionClass *r = region_item;
        if (mode == MOUSE_IN_ITEMLIST2)
            r = region_item2;

        oldcx = cx;
        oldcy = cy;
        oldmode = mode;
        int pos = cx + cy * (r->mx);
        int ix = itemlist_idx[pos];
        static char desc[200];
        if (ix != -1)
        {
            if (ix >= ENDOFPACK)
                it_name(ix - ENDOFPACK, DESC_NOCAP_A, desc);
            else
                in_name(ix, DESC_INVENTORY_EQUIP, desc);
            update_tip_text(desc);
        }
        return 0;
    }
#endif

    if (mode==MOUSE_IN_TEXT_DUNGEON || mode==MOUSE_IN_TILE_DUNGEON)
    {
        //if (cx<0 || cx>=17 || cy<0 || cy>=17) return 0;
    oldcx = cx;
    oldcy = cy;
        oldmode = mode;
        flag = 1;

        if(keyin_mode==KEYIN_MODE_TARGET || keyin_mode==KEYIN_MODE_TARGET_PRE)
        {
            tile_cursor_x0 = cx;
            tile_cursor_y0 = cy;
            return CMD_MOUSE_MOVE;
        }

        if(keyin_mode==KEYIN_MODE_COMMAND ||
           keyin_mode == KEYIN_MODE_TARGET_DIR)
        {
            int map_x = cx + you.x_pos - 9;
            int map_y = cy + you.y_pos - 9;
            if (keyin_mode == KEYIN_MODE_TARGET_DIR &&
                 (cx <7 || cx > 9 || cy < 7 || cy > 9))
            {
                 if (Options.use_tile) TileCursor(0, 0, 0);
                 return 0;
            }

            tile_cursor_x0 = cx;
            tile_cursor_y0 = cy;

            if (map_x<0 || map_y<0 || map_x>= GXM || map_y >= GYM) return 0;
            unsigned char ch = env.map[ map_x ][ map_y ];

            if ( ch==0 || ch==' ' || ch == 127 || (ch >= 137 && ch <= 140))
                flag = 2;
        }

        if(mode==MOUSE_IN_TILE_DUNGEON)
            TileCursor(cx, cy, flag);

        if(mode==MOUSE_IN_TEXT_DUNGEON)
        {
#ifdef JP
            if (Options.use_zenkaku)
                gotoxy(cx*2+2, cy+1);
            else
#endif
            gotoxy(cx+10, cy+1);
        }

        if (cx<0 || cx>=17 || cy<0 || cy>=17) return 0;

        // Tip Text
        int ch = env.show[cx+1][cy+1];

        if (ch==DNGN_FLOOR || ch == DNGN_UNDISCOVERED_TRAP ||
            ch==DNGN_ROCK_WALL || ch == DNGN_SECRET_DOOR || ch==0)return 0;

        static char desc[200];
        strncpy(desc, look_directly(cx+1, cy+1), 199);
        update_tip_text(desc);
        return 0;
    }

#if 1 // Help
    if (mode == MOUSE_IN_STAT && keyin_mode == KEYIN_MODE_COMMAND)
    {
        static int oldtip = -1;
    oldcx = cx;
    oldcy = cy;
        oldmode = mode;

        int i = 0;
        while( Tips[i].tiptext != NULL)
        {
            if (cx >= Tips[i].sx && cx <= Tips[i].ex &&
                cy >= Tips[i].sy && cy <= Tips[i].ey)
            break;
            i++;
        }
        if (Tips[i].tiptext == NULL) return 0;
        if (i == oldtip) return 0;
        oldtip = i;
        update_tip_text((char *)Tips[i].tiptext);
        return 0;
    }
#endif
    return 0;
}
int handle_mouse_button(int mx, int my, int button, bool shift, bool ctrl)
{
    int dir, ch;
    const int dx[9]={-1,0,1, -1,0,1, -1,0,1};
    const int dy[9]={1,1,1,0,0,0,-1,-1,-1};
#if 1
    const int cmd_n[9]={
        CMD_MOVE_DOWN_LEFT,
        CMD_MOVE_DOWN,
        CMD_MOVE_DOWN_RIGHT,
        CMD_MOVE_LEFT,
        CMD_MOVE_NOWHERE,
        CMD_MOVE_RIGHT,
        CMD_MOVE_UP_LEFT,
        CMD_MOVE_UP,
        CMD_MOVE_UP_RIGHT
    };
    const int cmd_s[9]={
        CMD_RUN_DOWN_LEFT,
        CMD_RUN_DOWN,
        CMD_RUN_DOWN_RIGHT,
        CMD_RUN_LEFT,
        '5',
        CMD_RUN_RIGHT,
        CMD_RUN_UP_LEFT,
        CMD_RUN_UP,
        CMD_RUN_UP_RIGHT
    };
    const int cmd_c[9]={
        CMD_OPEN_DOOR_DOWN_LEFT,
        CMD_OPEN_DOOR_DOWN,
        CMD_OPEN_DOOR_DOWN_RIGHT,
        CMD_OPEN_DOOR_LEFT,
        'X',
        CMD_OPEN_DOOR_RIGHT,
        CMD_OPEN_DOOR_UP_LEFT,
        CMD_OPEN_DOOR_UP,
        CMD_OPEN_DOOR_UP_RIGHT
    };
    const int cmd_dir[9]={'1','2','3','4','5','6','7','8','9'};
#else
    const int cmd_n[9]={'b','j','n','h','.','l','y','k','u'};
    const int cmd_s[9]={'B','J','N','H','5','L','Y','K','U'};
    const int cmd_c[9]={0x2, 0xa, 0xe, 0x8, 'X', 0xc, 0x19, 0xb, 0x15};
#endif

    if(keyin_mode==KEYIN_MODE_NONE) return 0;
    if(keyin_mode==KEYIN_MODE_MORE) return '\r';
    if(keyin_mode==KEYIN_MODE_MAP) return 0;
    if(keyin_mode==KEYIN_MODE_PAPER_DOLL) return 0;
    if(keyin_mode==KEYIN_MODE_END) return 0;

    if(keyin_mode==KEYIN_MODE_ITEMLIST_COMMAND)
    {
        // Use rclick as "use the item"
        if (button == 2) return CMD_MOUSE_RCLICK;
        return 0;
    }

    int cx,cy;
    int mode;

    mode = convert_cursor_pos(mx, my, &cx, &cy);

    if ( (mode == MOUSE_IN_ITEMLIST1 || mode == MOUSE_IN_ITEMLIST2) &&
        (keyin_mode==KEYIN_MODE_COMMAND || keyin_mode == KEYIN_MODE_INVENT
          || keyin_mode == KEYIN_MODE_MULTIPICK))
    {
        TileRegionClass *r = region_item;
        if (mode == MOUSE_IN_ITEMLIST2)
            r = region_item2;

        int j = cx + cy * (r->mx);

        if (keyin_mode == KEYIN_MODE_MULTIPICK)
        {
             if (j < 23)
                 return 'a' + j;
             else
                 return 'A' + j - 23;
        }

        int ix = itemlist_idx[j];
        // Floor item
        if (ix >= ENDOFPACK) return 0;
        if (ix != -1)
        {
            if (keyin_mode == KEYIN_MODE_INVENT)
            {
                // inventry prompt
                return index_to_letter(ix);
            }

            if (button == 2)
            {
                return CMD_VIEW_ITEM + ix;
            }
            else
            {
                return CMD_USE_ITEM + ix;
            }
        }
        return 0;
    }

    if (mode == MOUSE_IN_STAT && keyin_mode == KEYIN_MODE_COMMAND)
    {
        int i = 0;
        while( Tips[i].tiptext != NULL)
        {
            if (cx >= Tips[i].sx && cx <= Tips[i].ex &&
                cy >= Tips[i].sy && cy <= Tips[i].ey)
            break;
            i++;
        }

        if (Tips[i].tipfilename)
        {
            char fname[256];
#ifdef JP
            snprintf(fname, 250, "tips_j/%s.txt", Tips[i].tipfilename);
#else
            snprintf(fname, 250, "tips_e/%s.txt", Tips[i].tipfilename);
#endif
            ViewTextFile(fname);
            redraw_screen();
            return CMD_DO_NOTHING;
        }
        return 0;
    }

    if(keyin_mode==KEYIN_MODE_COMMAND &&
        (mode == MOUSE_IN_TILE_DUNGEON || mode == MOUSE_IN_TEXT_DUNGEON))
    {
        if (button == 2)
        {
            // Right Click
            // Out of LOS
            if (cx<0 || cy<0 || cx>16 || cy>16 ) return 0;
            if (env.show[cx+1][cy+1] == 0) return CMD_EXPLORE;
            int mx = you.x_pos + cx - 8;
            int my = you.y_pos + cy - 8;
            int mid = mgrd[mx][my];
            if (!player_monster_visible( &menv[mid] )
                 || mid == NON_MONSTER ) return CMD_EXPLORE;

            describe_monsters( menv[ mid ].type, mid );
            redraw_screen();
            mesclr( true );
            return CMD_DO_NOTHING;
        }

        for(dir=0;dir<9;dir++)
        {
            if( 8+dx[dir] == cx && 8+dy[dir]==cy)
            {
                if (shift) return cmd_s[dir];
                else
                    if (ctrl) return cmd_c[dir];
                    else
                        return cmd_n[dir];
            }
        }
        cx = cx + you.x_pos - 8;
        cy = cy + you.y_pos - 8;

        if (cx<0 || cy<0 || cx>= GXM || cy >= GYM) return 0;
        ch = env.map[cx-1][cy-1];

        if ( ch==0 || ch==' ' || ch == 127 || (ch >= 137 && ch <= 140))
            return 0;

        // Activate travel
        you.running = -1;
        you.run_x   = cx;
        you.run_y   = cy;
        return CMD_DO_NOTHING;
    }

    if(keyin_mode==KEYIN_MODE_COMMAND && mode == MOUSE_IN_MINIMAP)
    {
        cx = cx + you.x_pos - 8;
        cy = cy + you.y_pos - 8;

        if (cx<0 || cy<0 || cx>= GXM || cy >= GYM) return 0;
        ch = env.map[cx-1][cy-1];

        if ( ch==0 || ch==' ' || ch == 127 || (ch >= 137 && ch <= 140))
            return 0;

        // Activate travel
        you.running = -1;
        you.run_x   = cx;
        you.run_y   = cy;
        return CMD_DO_NOTHING;
    }

    if(keyin_mode==KEYIN_MODE_TARGET &&
       (mode == MOUSE_IN_TILE_DUNGEON || mode == MOUSE_IN_TEXT_DUNGEON))
    {
        if (cx<0 || cy<0 || cx>=17 || cy >= 17) return 0;
        tile_cursor_x0 = cx;
        tile_cursor_y0 = cy;
        return CMD_MOUSE_LCLICK;
    }

    if(keyin_mode==KEYIN_MODE_TARGET_DIR &&
       (mode == MOUSE_IN_TILE_DUNGEON || mode == MOUSE_IN_TEXT_DUNGEON))
    {
        if (cx < 7 || cy < 7 || cx > 9 || cy > 9) return 0;
        for(dir=0;dir<9;dir++)
        {
            if( 8+dx[dir] == cx && 8+dy[dir]==cy)
            {
                return cmd_dir[dir];
            }
        }
        return 0;
    }

    if(keyin_mode==KEYIN_MODE_TARGET_PRE &&
       (mode == MOUSE_IN_TILE_DUNGEON || mode == MOUSE_IN_TEXT_DUNGEON))
    {
        if (cx<0 || cy<0 || cx>=17 || cy >= 17) return 0;
        tile_cursor_x0 = cx;
        tile_cursor_y0 = cy;
        return CMD_MOUSE_LCLICK;
    }

    return 0;
}

void set_keyin_mode(int mode){
    keyin_mode = mode;
}

int get_keyin_mode(){
    return keyin_mode;
}

void mouse_get_cursor_info(int *x, int *y){
    *x=tile_cursor_x0;
    *y=tile_cursor_y0;
}
#endif


/*
 *  Wrapper
 */

void clrscr()
{
    win_main->clear();
    TextRegionClass::old_cursor_region = NULL;

    // clear Text regions
    region_crt->clear();
    //msgバッファの消去
    if (region_msg) region_msg->clear();
    //statusバッファの消去
    if (region_stat) region_stat->clear();

    // Hack: Do not erase the backbuffer. Instead just hide it.
    if (region_map)
        region_map->flag = false;
    if (region_item)
        region_item->flag = false;
    if (region_item2)
        region_item2->flag = false;

    gotoxy(1, 1);
}

void mpr_on(int mode)
{
    if (!Options.use_tile) return;
    if(mode==MODE_CRT) TextRegionClass::text_mode = region_crt;
    if(mode==MODE_MPR) TextRegionClass::text_mode = region_msg;
    if(mode==MODE_STAT) TextRegionClass::text_mode = region_stat;

    TextRegionClass::text_mode->flag = true;
}

void putch(unsigned char chr)
{
    // object's method
    TextRegionClass::text_mode->putch(chr);
}

void writeWChar(unsigned char *ch)
{
    // object's method
    TextRegionClass::text_mode->writeWChar(ch);
}

void clear_to_end_of_line()
{
    // object's method
    TextRegionClass::text_mode->clear_to_end_of_line();
}

void clear_to_end_of_screen()
{
    // object's method
    TextRegionClass::text_mode->clear_to_end_of_screen();
}

void get_input_line_tile(char *const buff, int len)
{
    int y, x;
    int k = 0;
    int prev = 0;
    int done = 0;
    int kin;
    TextRegionClass *r = TextRegionClass::text_mode;

    if(! r->flag)return;

    /* Locate the cursor */
    x = wherex();
    y = wherey();

    /* Paranoia -- check len */
    if (len < 1) len = 1;

    /* Restrict the length */
    if (x + len > r->mx) len = r->mx - x;
    if (len > 40) len = 40;

    /* Paranoia -- Clip the default entry */
    buff[len] = '\0';
    buff[0] = '\0';

    gotoxy(x, y);
    putch('_');

    /* Process input */
    while (!done)
    {
        /* Get a key */
        kin = getch();

        /* Analyze the key */
        switch (kin)
        {
            case 0x1B:
                k = 0;
                done = 1;
                break;

            case '\n':
            case '\r':
                k = strlen(buff);
                done = 1;
                break;

            case 0x7F:
            case '\010':
                k = prev;
                break;

            // Escape conversion. (for ^H, etc)
            case CONTROL('V'):
                kin = getch();
                // fallthrough

            default:
            {
                if (k < len &&
                     (
                      isprint(kin)
                      || (kin >=  CONTROL('A') && kin <= CONTROL('Z'))
                      || (kin >= 0x80 && kin <=0xff)
                     )
                   )
                     buff[k++] = kin;
                break;
            }

        }
        /* Terminate */
        buff[k] = '\0';

        /* Update the entry */
        gotoxy(x, y);
        int i;

        //addstr(buff);
        for(i=0;i<k;i++)
        {
            prev = i;
            int c = (unsigned char)buff[i];
            if (c>=0x80)
            {
                if (buff[i+1]==0) break;
                writeWChar((unsigned char *)&buff[i]);
                i++;
            }
            else
            if (c >= CONTROL('A') && c<= CONTROL('Z'))
            {
                putch('^');
                putch(c + 'A' - 1);
            }
            else
                putch(c);
        }
        r->addstr((char *)"_             ");
    }/* while */
}

void cprintf(const char *format,...)
{
    char buffer[2048];          // One full screen if no control seq...
    va_list argp;
    va_start(argp, format);
    vsprintf(buffer, format, argp);
    va_end(argp);
    // object's method
    TextRegionClass::text_mode->addstr(buffer);
    TextRegionClass::text_mode->make_active();
}

void textcolor(int color)
{
    TextRegionClass::textcolor(color);
}

void textbackground(int bg)
{
    TextRegionClass::textbackground(bg);
}

void gotoxy(int x, int y)
{
    TextRegionClass::gotoxy(x, y);
}

void _setcursortype(int curstype)
{
    if (!Options.use_tile)
     TextRegionClass::_setcursortype(curstype);
}

int wherex()
{
    return TextRegionClass::wherex();
}

int wherey()
{
    return TextRegionClass::wherey();
}

int get_number_of_lines_tile()
{
    return TextRegionClass::get_number_of_lines();
}

void puttext(int x, int y, int lx, int ly, unsigned char *buf, bool mono)
{
    TextRegionClass *r = region_crt;
    int xx, yy;
    unsigned char *ptr = buf;

    mpr_on(MODE_CRT);

    for(yy= y-1; yy< y-1+ly; yy++)
    {
        unsigned char *c = &(r->cbuf[yy*(r->mx)+x-1]);
        unsigned char *a = &(r->abuf[yy*(r->mx)+x-1]);
        for(xx= x-1; xx< x-1+lx; xx++)
        {
            *c = *ptr++;
            if (*c==0) *c=32;
            c++;

            if (mono)
                *a = WHITE;
            else
                *a = *ptr++;
            a++;
        }
    }
    r->make_active();
    r->redraw(x-1, y-1, x-2+lx, y-2+ly);
}

void ViewTextFile(const char *name)
{
    FILE *fp = fopen(name, "r");
    int max = get_number_of_lines_tile();

    // Hack
#define MAXTEXTLINES 100

#ifdef JP
#define DELIMITER_END "-------------------------------------------------------------------------------"
#define DELIMITER_MORE "...(続く)...                                                                   "
#else
#define DELIMITER_END "-------------------------------------------------------------------------------"
#define DELIMITER_MORE "...(more)...                                                                   "
#endif
    unsigned char buf[80*MAXTEXTLINES], buf2[84];
    int nlines = 0;
    int cline = 0;
    int i;

    if (!fp)
    {
        mpr("Couldn't open file.");
        return;
    }

    for (i=0; i<80*MAXTEXTLINES; i++) buf[i] = ' ';

    while(nlines<MAXTEXTLINES)
    {
        fgets((char *)buf2, 82, fp);
        if (feof(fp)) break;
        i = 0;
        while(i<79 && buf2[i] !=13 && buf2[i] != 10) i++;
        memcpy(&buf[nlines*80], buf2, i);
        nlines++;
    }
    fclose(fp);
    clrscr();

    while(1)
    {
        gotoxy(1, 1);
        if (cline == 0)
            cprintf(DELIMITER_END);
        else
            cprintf(DELIMITER_MORE);

#ifdef USE_TILE
        puttext(1, 2, 80, max-2, &buf[cline*80], true);
#else
        textcolor(LIGHTGRAY);
        for (i=0; i < max-2; i++)
        {
            char tmp[81];
            memcpy(tmp, &buf[(cline + i) * 80], 80);
            tmp[80] = 0;
            gotoxy(1, 2 + i);
            cprintf((const char *)tmp);
        }
#endif
        gotoxy(1, max);

        if (cline + max-2 >= nlines)
            cprintf(DELIMITER_END);
        else
            cprintf(DELIMITER_MORE);

        gotoxy(14, max);
#ifdef JP
        cprintf("[j/k/+/-/SPACE/b: スクロール  q/ESC/RETURN: 終了]");
#else
        cprintf("[j/k/+/-/SPACE/b: scroll   q/ESC/RETURN: exit]");
#endif
        set_keyin_mode(KEYIN_MODE_MORE);
        int key = getch();
        set_keyin_mode(KEYIN_MODE_NONE);
        if (key == 'q' || key == ESCAPE || key =='\r') break;
        if (key == '-' || key == 'b') cline -= max-2;
        if (key == 'k') cline --;
        if (key == '+' || key == ' ') cline += max-2;
        if (key == 'j') cline ++;
#ifdef USE_TILE
        if (key == CMD_MOUSE_WHEEL_UP) cline--;
        if (key == CMD_MOUSE_WHEEL_DOWN) cline++;
#endif

        if (cline + max-2 > nlines) cline = nlines-max + 2;
        if (cline < 0) cline = 0;
    }
}

void rotate_qv_key(int *key)
{
    static const char table[9*3+1] = {
        'b',  'j',  'n',  'l',  'u',  'k',  'y',  'h',  'b',
        'B',  'J',  'N',  'L',  'U',  'K',  'Y',  'H',  'B',
        0x02, 0x0a, 0x0e, 0x0c, 0x15, 0x0b, 0x19, 0x08, 0x02,
        0
    };

    char *ptr;
    if (*key > 127) return;

    ptr = strchr((char *)table, (char)(*key));
    if (ptr != NULL)
        (*key) = (int) *(ptr + 1);
}
