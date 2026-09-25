#include "AppHdr.h"

// Windows ヘッダー ファイル:
#include <windows.h>
#include <commdlg.h>
#include <commctrl.h>

// C ランタイム ヘッダー ファイル
#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>

// acr.cc
extern bool game_has_started;

#include "version.h"
#include "defines.h"
#include "enum.h"
#include "externs.h"
#include "stuff.h"
#include "initfile.h"
#include "files.h"

#include "winclass.h"

#ifdef USE_TILE
#include "tiles.h"
#include "tiledef.h"
#include "player.h"
#include "tiledef-p.h"
#endif

int old_main();

HINSTANCE hInst;
int nCmdShow0;

#define KEYBUFMAX 1024
static int keybuf[KEYBUFMAX];
static int key_head, key_tail;

// Tip text
static TOOLINFO tiTip;
static HWND hTool;

// Window Preferences
static void load_wininit();
static void save_wininit();
// Font related
static int font_size = 16;
static unsigned char font_name[128];
// Window position
static int wintop = 0;
static int winleft = 0;

#define PAL_STD    0  //standard palette
#define PAL_BER    1  //berserk palette
#define PAL_SHA    2  //shadow palette
static int palette = PAL_STD;

//scr_mode
#define NORMAL     0
#define TITLE      1
#define PAPER_DOLL 2

// Windows and Regions
WinClass    *win_main = NULL;

TextRegionClass *region_crt = NULL;
TextRegionClass *region_msg = NULL;
TextRegionClass *region_stat = NULL;
MapRegionClass  *region_map = NULL;
TileRegionClass *region_tile = NULL;
#if 1 //Slot
TileRegionClass *region_item = NULL;
TileRegionClass *region_item2 = NULL;
#endif

    #define PIX_BLACK 0
    #define PIX_BLUE 1
    #define PIX_GREEN 2
    #define PIX_CYAN 3
    #define PIX_RED 4
    #define PIX_MAGENTA 5
    #define PIX_BROWN 6
    #define PIX_LIGHTGREY 7
    #define PIX_DARKGREY 8
    #define PIX_LIGHTBLUE 9
    #define PIX_LIGHTGREEN 10
    #define PIX_LIGHTCYAN 11
    #define PIX_LIGHTRED 12
    #define PIX_LIGHTMAGENTA 13
    #define PIX_YELLOW 14
    #define PIX_WHITE 15

COLORREF pix_col[16];
BYTE pix_transparent;
BYTE pix_black;
BYTE pix_white;
BYTE pix_magenta;
BYTE pix_rimcolor;

//libtile.cc
extern void init_tile_all();
extern int handle_mouse_motion(int mouse_x, int mouse_y);
extern int handle_mouse_button(int mx, int my,
                 int button, bool shift, bool ctrl);

// このコード モジュールに含まれる関数の前宣言:
ATOM   MyRegisterClass( HINSTANCE hInstance );
BOOL   InitInstance( HINSTANCE, int );
LRESULT CALLBACK    WndProc( HWND, UINT, WPARAM, LPARAM );

static void init_region_tile();
static void init_region_map(int fontsize);

//void draw_title();
//static void disp_title();

//LRESULT CALLBACK    About( HWND, UINT, WPARAM, LPARAM );

//以下、画像操作用の変数やポインタ
//DIB Image
#if 0
dib_pack TileDib;                    //タイルDIB
dib_pack TileQvDib;                    //タイルDIB
dib_pack BufDib;                     //表示用のバックバッファDIB
dib_pack DollDib;                    //着せ替えタイルDIB
//dib_pack MMapDib;                    //ミニマップのDIB
//dib_pack CacheDib[TCACHE_KIND];      //キャッシュDIB
//dib_pack DollCacheDib;               //着せ替えタイルキャッシュDIB
#endif

img_type TileImg;
img_type TileQvImg;
img_type PlayerImg;
img_type ScrBufImg;

LPBYTE lpBerPalette ;    //バーサーク時用のパレットへのポインタを格納
LPBYTE lpDarkPalette;    //影ランタン使用時用のパレットへのポインタを格納

/******** SMALL MAP *******/
#define MAP_XMAX GXM
#define MAP_YMAX GYM
#define MAP_XMARGIN 0
#define MAP_YMARGIN 0

void delay(int ms)
{
   Sleep((DWORD)ms);
}

void window(int x, int y, int lx, int ly)
{
   // do nothing
}

void win_init_common(){

    pix_col[PIX_BLACK]       =PALETTERGB(  0,   0,   0);
    pix_col[PIX_BLUE]        =PALETTERGB(  0,  82, 255);
    pix_col[PIX_GREEN]       =PALETTERGB(100, 185,  70);
    pix_col[PIX_CYAN]        =PALETTERGB(  0, 180, 180);//!!!
    pix_col[PIX_RED]         =PALETTERGB(255,  48,   0);
    pix_col[PIX_MAGENTA]     =PALETTERGB(238,  92, 238);
    pix_col[PIX_BROWN]       =PALETTERGB(165,  91,   0);
    pix_col[PIX_LIGHTGREY]   =PALETTERGB(162, 162, 162);
    pix_col[PIX_DARKGREY]    =PALETTERGB( 82,  82,  82);
    pix_col[PIX_LIGHTBLUE]   =PALETTERGB( 82, 102, 255);
    pix_col[PIX_LIGHTGREEN]  =PALETTERGB( 82, 255,  82);
    pix_col[PIX_LIGHTCYAN]   =PALETTERGB( 82, 255, 255);
    pix_col[PIX_LIGHTRED]    =PALETTERGB(255,  82,  82);
    pix_col[PIX_LIGHTMAGENTA]=PALETTERGB(255,  82, 255);
    pix_col[PIX_YELLOW]      =PALETTERGB(255, 255,  82);
    pix_col[PIX_WHITE]       =PALETTERGB(255, 255, 255);

}

/********* REGION ******/
#if 0
void clrscr(){
    if (Options.use_tile)
    {
        win_main->clear();
    }
    region_crt->clear();

#ifdef USE_TILE
    if (Options.use_tile)
    {
        //msgバッファの消去
        region_msg->clear();
        //statusバッファの消去
        region_stat->clear();
    }
#endif
    gotoxy(1, 1);
}
#endif

void init_region_map(int fontsize)
{
    int x,y,i;
    int map_x = 4;
    int map_y = 4;

    //余剰領域を計算し、ミニマップのサイズと描画先を決める
    if ( (Options.use_tile)&&(!Options.use_qv_mode) )
    {
        if (fontsize <= 24)   map_x = 2;
        if (fontsize <= 20)   map_x = 3;
    }
    else
    {
        map_x = 4;
    }
    if (fontsize <= 16)   map_x = 4;
    if (fontsize <= 14)   map_x = 3;
    if (fontsize <= 10)   map_x = 2;
    map_y = map_x;

    region_map = new MapRegionClass(MAP_XMAX-MAP_XMARGIN*2,
                                    MAP_YMAX-MAP_YMARGIN*2,
                                    MAP_XMARGIN, MAP_YMARGIN,
                                    Options.rotate_minimap);
    region_map->dx = map_x;
    region_map->dy = map_y;

    // Mega Hack!
    if (region_map->qv_mode)
    {
        region_map->dx = 2;
        region_map->dy = 1;
    }

    //region_map->init_backbuf();
}


img_type ImgLoadFileSimple(const char *name)
{
    char fname[512];
    sprintf(fname,"%s.bmp", name);
    return ImgLoadFile(fname);
}

void init_region_tile(){
    int i;

    int tile_scr_width = TILE_SCR_WIDTH_NORMAL;
    int tile_scr_height = TILE_SCR_HEIGHT_NORMAL;

    if (Options.use_qv_mode)
    {
        tile_scr_width = TILE_SCR_WIDTH_QV;
        tile_scr_height = TILE_SCR_HEIGHT_QV;
    }

    region_tile = new TileRegionClass(tile_scr_width, tile_scr_height, 1, 1);

//タイルの読み込み

    TileImg = ImgLoadFileSimple("tile");
    pix_transparent = (BYTE)*(TileImg->pDibZero);  //左上隅の1ピクセル

    if (Options.use_qv_mode)
    {
#ifdef QV48
        TileQvImg = ImgLoadFileSimple("tile-qv48");
#else
        TileQvImg = ImgLoadFileSimple("tile-qv");
#endif
    }

    PlayerImg = ImgLoadFileSimple("player");

    //バックバッファ用DBIの確保は後で
    //region_tile->init_backbuf(&(TileImg->pDib->bmiColors[0]));

    //透明色を黒に入れ替える
    TileImg->pDib->bmiColors[pix_transparent].rgbRed   = 0;
    TileImg->pDib->bmiColors[pix_transparent].rgbGreen = 0;
    TileImg->pDib->bmiColors[pix_transparent].rgbBlue  = 0;

    //バックバッファ用パレットの作成
    WORD chcol; //グレイスケール計算用
    lpBerPalette  = (LPBYTE)GlobalAlloc(GPTR, 256 * sizeof(RGBQUAD) ); //バーサークパレットを入れる為のメモリ確保
    lpDarkPalette = (LPBYTE)GlobalAlloc(GPTR, 256 * sizeof(RGBQUAD) ); //シャドウパレットを入れる為のメモリ確保
    for (i = 0; i < 256; i++)
    {
        //バーサーク用パレット
        chcol = TileImg->pDib->bmiColors[i].rgbRed   * 0.30
              + TileImg->pDib->bmiColors[i].rgbGreen * 0.59
              + TileImg->pDib->bmiColors[i].rgbBlue  * 0.11;
        *(lpBerPalette + i * sizeof(RGBQUAD) + 2) = (BYTE)chcol;
        *(lpBerPalette + i * sizeof(RGBQUAD) + 0) = (BYTE)( (chcol +1)/6 );
        *(lpBerPalette + i * sizeof(RGBQUAD) + 1) = (BYTE)( (chcol +1)/6 );

        //シャドウ用パレット
        *(lpDarkPalette + i * sizeof(RGBQUAD) + 2) = (BYTE)chcol;
        *(lpDarkPalette + i * sizeof(RGBQUAD) + 0) = (BYTE)chcol;
        *(lpDarkPalette + i * sizeof(RGBQUAD) + 1) = (BYTE)chcol;
    }

    init_tile_all();
}

#define CRT_XMAX 80
#define CRT_YMAX 25

BOOL init_win_all(HINSTANCE hInstance, int nCmdShow ){
    int text_margin = 0;

    // Read the init file
    read_init_file();

    load_wininit();

    //タイル版バイナリでの非タイルモードではqvオプションは抑止
    if (Options.use_tile == false)
    {
        Options.use_qv_mode =  false;
    }

    //非クオータービューではテンキー旋回オプションを抑止
    if (Options.use_qv_mode == false)
    {
        Options.rotate_numpad = false;
        Options.rotate_minimap = false;
    }

    win_main = new WinClass();

    hInst = hInstance;
    nCmdShow0 = nCmdShow;
    int i;
    //文字色の初期化
    win_init_common();

    // CRT 領域の初期化
    region_crt = new TextRegionClass(CRT_XMAX, CRT_YMAX, 0, 0);
    TextRegionClass::text_mode = region_crt;

    region_crt->init_font(font_name, font_size );
#ifdef JP
    if (region_crt->font == NULL)
    {
        region_crt->init_font("ＭＳ ゴシック", font_size );
        strcpy(font_name, "ＭＳ ゴシック");
    }
    unsigned char mw_caption[] = "Dungeon Crawl 日本語版";
#else
    if (region_crt->font == NULL)
    {
        region_crt->init_font("Courier", font_size );
        strcpy(font_name, "Courier");
    }
    unsigned char mw_caption[] = "Dungeon Crawl" VERSION;
#endif

    text_margin = region_crt->dx;

    //マップ領域初期化
    init_region_map(region_crt->dy);

    if (!Options.use_tile)
    {
        win_main->placeRegion(region_crt, 0, NULL, PLACE_RIGHT,
        text_margin, text_margin, text_margin, 0);
        win_main->placeRegion(region_map, 0, region_crt, PLACE_RIGHT, 0, 0, 0, 0);
    }
    else
    {
        region_msg = new TextRegionClass(80, 9, 0, 17);
        // Copy font id and size;
        region_msg->font = region_crt->font;
        region_msg->dx = region_crt->dx;
        region_msg->dy = region_crt->dy;
        region_msg->fx = region_crt->fx;
        region_msg->fy = region_crt->fy;
#if DEBUG_DIAGNOSTICS
        region_stat = new TextRegionClass(40, 17, 39, 0);
#else
        region_stat = new TextRegionClass(40, 16, 39, 0);
#endif
        // Copy font id and size;
        region_stat->font = region_crt->font;
        region_stat->dx = region_crt->dx;
        region_stat->dy = region_crt->dy;
        region_stat->fx = region_crt->fx;
        region_stat->fy = region_crt->fy;

        init_region_tile();

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
        }
#endif
    }

    if(! win_main->create(mw_caption, winleft, wintop)) return FALSE;
    mpr_off();

    // バックバッファ作成
    region_map->init_backbuf();

    if (Options.use_tile)
    {
        region_tile->init_backbuf(&(TileImg->pDib->bmiColors[0]));
        ScrBufImg = region_tile->backbuf;
#if 1 //Slot
        region_item->init_backbuf(&(TileImg->pDib->bmiColors[0]));
        region_item2->init_backbuf(&(TileImg->pDib->bmiColors[0]));
#endif
    }

    return TRUE;
}

/**********************************************/
static void add_keypress(int ch)
{
    keybuf[key_tail]=ch;
    key_tail++;
    if(key_tail == KEYBUFMAX)
        key_tail=0;
}

int getch(){
    MSG msg;
    int ch;

    if(key_tail == key_head){
        while( GetMessage(&msg, NULL, 0, 0) )
        {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
            if(key_head != key_tail) break;
        }
    }

    //Pop a key
    ch = keybuf[key_head];
    key_head++;
    if(key_head == KEYBUFMAX)key_head = 0;

    return ch;
}

int kbhit()
{
    MSG msg;

    if(key_head !=  key_tail)return 1;

    if (PeekMessage(&msg, NULL, WM_CHAR, WM_CHAR, PM_NOREMOVE)
        || PeekMessage(&msg, NULL, WM_KEYDOWN, WM_KEYDOWN, PM_NOREMOVE)
     /* || PeekMessage(&msg, NULL, WM_MOUSEMOVE, WM_MOUSEMOVE, PM_NOREMOVE)*/ )
        return 1;
    else
        return 0;
}

FAR PASCAL WinMain(HINSTANCE hInstance,
                     HINSTANCE hPrevInstance,
                     LPSTR     lpCmdLine,
                     int       nCmdShow )
{
    MSG msg;
    int i;

    key_head=key_tail=0;

    MyRegisterClass( hInstance );

    // アプリケーションの初期化を行います:
    if( !init_win_all(hInstance, nCmdShow ) )
    {
        return FALSE;
    }

    //NumLock解除
    if ( (GetKeyState(VK_NUMLOCK) & 0x1) )
    {
        // キー押下
        keybd_event( VK_NUMLOCK, MapVirtualKey(VK_NUMLOCK, 0),
                     KEYEVENTF_EXTENDEDKEY | 0, 0 );
        // キー解放
        keybd_event( VK_NUMLOCK, MapVirtualKey(VK_NUMLOCK, 0),
                     KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
    }

    // ゲームメイン呼出し
    old_main();

    // メイン メッセージ ループ:
    while( GetMessage(&msg, NULL, 0, 0) )
    {
            TranslateMessage( &msg );
            DispatchMessage( &msg );
    }
    return msg.wParam;
}


//
//  関数: MyRegisterClass()
//
//  用途: ウィンドウ クラスの登録
//
//  コメント:
//
//    この関数およびその使用はこのコードを Windows 95 で先に追加された
//    'RegisterClassEx' 関数と Win32 システムの互換性を保持したい場合に
//    のみ必要となります。アプリケーションが、アプリケーションに関連付け
//    られたスモール アイコンを取得できるよう、この関数を呼び出すことは
//    重要です。
//
ATOM MyRegisterClass( HINSTANCE hInstance )
{
    WNDCLASSEX wcex;

    wcex.cbSize        = sizeof(WNDCLASSEX);

    wcex.style         = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc   = (WNDPROC)WndProc;
    wcex.cbClsExtra    = 0;
    wcex.cbWndExtra    = 0;
    wcex.hInstance     = hInstance;
    wcex.hIcon         = LoadIcon(hInstance, TEXT("CRAWL_ICON"));
    wcex.hCursor       = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
    wcex.lpszMenuName  = "CRAWL";
    wcex.lpszClassName = "CrawlList";
    wcex.hIconSm       = NULL;//LoadIcon(wcex.hInstance, (LPCTSTR)IDI_SMALL);

    return RegisterClassEx( &wcex );
}

#ifdef USE_TILE
void update_tip_text(char *tip)
{
    tiTip.lpszText = tip;
    SendMessage(hTool, TTM_UPDATETIPTEXT, 0, (LPARAM)&tiTip);
}
#endif

//ウィンドウ位置などをファイルから読みこみ
void load_wininit()
{
    FILE *fp;
    unsigned char buf[128];
#ifdef JP
    strcpy(font_name, "ＭＳ ゴシック");
#else
    strcpy(font_name, "Courier");
#endif

    if ( (fp = fopen("wininit.txt", "r")) != NULL )
    {
        fscanf(fp, "WindowTop=%d\n", &wintop);
        fscanf(fp, "WindowLeft=%d\n", &winleft);
        fscanf(fp, "FontSize=%d\n", &font_size);
        fgets((char *)buf, 120, fp);
        if (!feof(fp))
        {
            int i = 0;
            while(buf[i] >= 32 && i< 120) i++;
            buf[i] = 0;
            i = 0;
            while( buf[i] != '=' && buf[i] != 0) i++;
            if (buf[i] != 0)
                strncpy(font_name, &buf[i+1], 120);
        }
        fclose(fp);

        if (font_size <  10 ) font_size = 10;
        if (font_size >  24 ) font_size = 24;
        if (font_size % 2 == 1) font_size -= 1;
    }
}

//ウィンドウ位置などをファイルに書き込み
void save_wininit()
{
    WINDOWPLACEMENT wndpl;
    FILE *fp;

    wintop = winleft =0;
    //lengthセット
    wndpl.length = sizeof( WINDOWPLACEMENT );

    if( GetWindowPlacement( win_main->hWnd, &wndpl ) != 0 ) //ウィンドウ位置の取得
    {
        wintop  = wndpl.rcNormalPosition.top;
        winleft = wndpl.rcNormalPosition.left;
    }

    if ( (fp = fopen("wininit.txt", "w+")) == NULL )
    {
    }
    else
    {
        fprintf(fp, "WindowTop=%d\n", wintop);
        fprintf(fp, "WindowLeft=%d\n", winleft);
        fprintf(fp, "FontSize=%d\n", font_size);
        fprintf(fp, "FontName=%s\n", font_name);
        fprintf(fp, "\n");
        fclose(fp);
    }
}


//窓の位置を保存し窓を終了
void quit_wingame()
{
    //ウィンドウ情報の保存
    save_wininit();

    DestroyWindow( win_main->hWnd );
}


//
//  関数: WndProc(HWND, unsigned, WORD, LONG)
//
//  用途: メイン ウィンドウのメッセージを処理します。
//
//  WM_COMMAND    - アプリケーション メニューの処理
//  WM_PAINT    - メイン ウィンドウの描画
//  WM_DESTROY    - 終了メッセージの通知とリターン
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    int wmId, wmEvent;
    PAINTSTRUCT ps;
    HDC hdc;

//    static HWND hTool;
//    static TOOLINFO ti;

    // For keypad
    const unsigned char cmd_n[9]={'b','j','n','h','.','l','y','k','u'};
    const unsigned char cmd_s[9]={'B','J','N','H','5','L','Y','K','U'};
    const unsigned char cmd_c[9]={0x2, 0xa, 0xe, 0x8, 'X', 0xc, 0x19, 0xb, 0x15};
    //const unsigned char qv_table[9]={2, 3, 6, 1, 5, 9, 4, 7, 8};


    switch( message )
    {
#ifdef USE_TILE
// Example taken from
// http://black.sakura.ne.jp/~third/system/winapi/common10.html
        case WM_CREATE:
    {

        // TOOLTIP テキストの設定
            InitCommonControls();
            hTool = CreateWindowEx( 0 , TOOLTIPS_CLASS ,
                    NULL , TTS_ALWAYSTIP ,
                    CW_USEDEFAULT , CW_USEDEFAULT ,
                    CW_USEDEFAULT , CW_USEDEFAULT ,
                    hWnd , NULL , ((LPCREATESTRUCT)(lParam))->hInstance ,
                    NULL
            );
            GetClientRect(hWnd , &tiTip.rect);

            tiTip.cbSize = sizeof (TOOLINFO);
            tiTip.uFlags = TTF_SUBCLASS;
            tiTip.hwnd = hWnd;
#ifdef JP
            tiTip.lpszText = "ゲーム中、マウスカーソルを合わせることで対象の様々な情報を知ることができます";
#else
            tiTip.lpszText = "This text will tell you what you are pointing";
#endif
            SendMessage(hTool , TTM_ADDTOOL , 0 , (LPARAM)&tiTip);
            return 0;
    }

        case WM_RBUTTONDOWN:
        case WM_LBUTTONDOWN:
        {
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int button = 0;
            bool fs = ((GetKeyState(VK_SHIFT) & 0x80)!=0)? true:false;
            bool fc = ((GetKeyState(VK_CONTROL) & 0x80)!=0)? true:false;
            if (message == WM_RBUTTONDOWN) button = 2;

            int key = handle_mouse_button(x, y, button, fs, fc);
            if (key) add_keypress(key);
            return 0;
        }

        case WM_MOUSEWHEEL:
        {
            int z = (short)HIWORD(wParam);
            bool fs = ((GetKeyState(VK_SHIFT) & 0x80)!=0)? true:false;
            bool fc = ((GetKeyState(VK_CONTROL) & 0x80)!=0)? true:false;

            int key = (z>0)? CMD_MOUSE_WHEEL_UP:CMD_MOUSE_WHEEL_DOWN;

            add_keypress(key);
            return 0;
        }

        case WM_MOUSEMOVE:
        {
            char *str;
            int x = LOWORD(lParam);
            int y = HIWORD(lParam);
            int key = handle_mouse_motion(x, y);
            if (key) add_keypress(key);
/*
        if (str != NULL)
        {
            ti.lpszText = str;
            SendMessage(hTool, TTM_UPDATETIPTEXT, 0, (LPARAM)&ti);
        }
*/
            return 0;
        }
#endif
        case WM_KEYDOWN:
        {
            int ch=(int)wParam;
        int result = 0;
            BOOL fs = ((GetKeyState(VK_SHIFT)  & 0x80)!=0)? TRUE:FALSE;
            BOOL fc = ((GetKeyState(VK_CONTROL)& 0x80)!=0)? TRUE:FALSE;
            BOOL fa = ((GetKeyState(VK_MENU)   & 0x80)!=0)? TRUE:FALSE;

            if ( ( (0x21 <= ch)&&(ch <=0x28) )
               ||(ch == VK_CLEAR) )
            {
        int dir = 0;
                switch(ch)
                {
                    case VK_LEFT:   dir = 4; break;
                    case VK_RIGHT:  dir = 6; break;
                    case VK_UP:     dir = 8; break;
                    case VK_DOWN:   dir = 2; break;
                    case VK_HOME:   dir = 7; break;
                    case VK_PRIOR:  dir = 9; break;
                    case VK_NEXT:   dir = 3; break;
                    case VK_END:    dir = 1; break;
                    case VK_CLEAR:  dir = 5; break;
                }

        if (dir != 0)
        {
            dir --;

#if 0
            if (Options.rotate_numpad)
            {
                if (Options.use_qv_mode)
                {
                    if ( (get_keyin_mode() != KEYIN_MODE_MAP)
                       &&(get_keyin_mode() != KEYIN_MODE_PAPER_DOLL) )
                    {
                        // Rotate 45 degree
                        dir = qv_table[dir]-1;
                    }
                }
            }
#endif
            if (fc)
                ch = cmd_c[dir];
            else if (fs)
                ch = cmd_s[dir];
            else
                ch = cmd_n[dir];
            if (fa) ch |= KEYFLAG_ALT;
                add_keypress(ch);
            return 0;
        }
            }

            if (ch >= VK_F1 && ch <= VK_F24)
        result = CMD_TRIGGER_F1 + ch - VK_F1;

            if (ch == VK_INSERT) /* '0' */
                result = CMD_EXPLORE;
            if (ch == VK_DELETE) /* '.'*/
                result = CMD_MOVE_NOWHERE;
#if 0
            if (ch == VK_NONCONVERT)
                result = CMD_TRIGGER_MUHENKAN;

            if (ch == 0xE5)
                result = CMD_TRIGGER_HENKAN;
#endif

        if (result)
        {
        if (fs) result |= KEYFLAG_SHIFT;
        if (fc) result |= KEYFLAG_CTRL;
        if (fa) result |= KEYFLAG_ALT;
        add_keypress(result);
        return 0;
        }

            return 0;
        }

        case WM_CHAR:
        {
            int ch=(int)wParam;
        add_keypress(ch);
            return 0;
        }

        case WM_PAINT:
        {
            hdc = BeginPaint(hWnd, &ps);
            /*
            int x1 = ps.rcPaint.left;
            int x2 = ps.rcPaint.right;
            int y1 = ps.rcPaint.top;
            int y2 = ps.rcPaint.bottom;

            int dx = xwin.dx;
            int dy = xwin.dy;
            */
            win_main->redraw();
#if 0
            region_crt->redraw();
            if ( (get_keyin_mode() != KEYIN_MODE_MAP)
               &&(get_keyin_mode() != KEYIN_MODE_PAPER_DOLL)
               ||(!Options.use_tile) )
            {
                region_map->redraw();
            }

#ifdef USE_TILE
            if (Options.use_tile)
            {
                if (get_keyin_mode() == KEYIN_MODE_MAP)
                {
                    region_crt->make_active();
                    region_crt->redraw();
                }
                else
                {
                    region_tile->redraw();
                }
                if (game_has_started)
                    region_stat->redraw();
                region_msg->redraw();
            }
#endif
#endif
            EndPaint(hWnd, &ps);
            //ValidateRect(hWnd, NULL);
            return 0;
        }

        case WM_COMMAND:
        {
            wmId    = LOWORD(wParam);
            wmEvent = HIWORD(wParam);
            // メニュー選択の解析:
            switch( wmId )
            {
                default:
                    return DefWindowProc( hWnd, message, wParam, lParam );
            }
            return 0;
        }
        case WM_CLOSE:
            //ゲームが開始したら、コマンド受付状態以外では終了しない
            if ( (get_keyin_mode() == KEYIN_MODE_COMMAND)
               ||(!game_has_started) )
            {
                //キャラクター情報の保存
                if (game_has_started)
                {
                    save_game(true);
                }

                quit_wingame();
            }
            break;
        case WM_DESTROY:
        {
           //フォント解放
           //DeleteObject(xwin.font);
           //デストラクタで解放

            std::vector<RegionClass *>::iterator r;
            for (r = win_main->regions.begin();r != win_main->regions.end();r++)
                delete (*r);

#ifdef USE_TILE
            if (Options.use_tile)
            {
                //タイル画像
                if (TileImg)   ImgDestroy(TileImg);
                if (PlayerImg) ImgDestroy(PlayerImg);
                if (Options.use_qv_mode && TileQvImg)
                ImgDestroy(TileQvImg);

                //タイル地図バッファ画像
                //デストラクタで
                //GlobalFree(pBufDib);    //ビットマップ情報+パレット メモリ解放
                //DeleteDC  (hBufDC);     //デバインコンテキスト解放
                //DeleteObject(hBufDib);  //ビットマップ解放

                //ミニマップバッファ画像
                //デストラクタで
                //GlobalFree(pmmapDib);   //ビットマップ情報+パレット メモリ解放
                //DeleteDC  (hmmapDC);    //デバインコンテキスト解放
                //DeleteObject(hmmapDib); //ビットマップ解放

                //視覚効果用パレット
                GlobalFree(lpBerPalette);   //バーサーク用パレット メモリ解放
                GlobalFree(lpDarkPalette);  //シャドウ用パレット   メモリ解放
            }
#endif

            delete win_main;

            PostQuitMessage( 0 );
            exit(0);
            break;
        }
        default:
            return DefWindowProc( hWnd, message, wParam, lParam );
    }
    return 0;
}

void mpr_off()
{
    TextRegionClass::text_mode = region_crt;
}


void TileDrawDungeonAux(){
    dib_pack *pBuf = region_tile->backbuf;
   //パレットに変更の必要があるか
   if ( ( (!you.berserker)&&(you.special_wield != SPWLD_SHADOW)&&(palette != PAL_STD ) )
      ||( (you.berserker)&&(palette != PAL_BER ) )
      ||( (you.special_wield == SPWLD_SHADOW)&&(palette == PAL_STD ) ) )
   {
       if ( (!you.berserker)&&(you.special_wield != SPWLD_SHADOW)&&(palette != PAL_STD ) )
       {
       //バーサークでも影のランタン装備でもないのなら、通常パレットに戻す
       SetDIBColorTable(pBuf->hDC,0,256,
            (RGBQUAD *)(&pBuf->pDib->bmiColors[0]) );
       palette = PAL_STD;
       }
       else if ( (you.berserker)&&(palette != PAL_BER ) )
       {
       //バーサークパレットに変える
       SetDIBColorTable(pBuf->hDC,0,256, (RGBQUAD *)lpBerPalette );
       palette = PAL_BER;
       }
       else if ( (you.special_wield == SPWLD_SHADOW)&&(palette == PAL_STD ) )
       {
       //シャドウパレットに変える
       SetDIBColorTable(pBuf->hDC,0,256, (RGBQUAD *)lpDarkPalette );
       palette = PAL_SHA;
       }
    }
}

void test_bmp()
{
    HDC hdc;
    hdc =GetDC(win_main->hWnd);
    BitBlt(hdc, 0, 0, 512, 512, region_tile->backbuf->hDC, 0, 0, SRCCOPY);
    ReleaseDC(win_main->hWnd, hdc);
    return;
}

void change_font()
{
    char info[INFO_SIZE];
    CHOOSEFONT cf;

    memset(&cf, 0, sizeof(cf));
    cf.lStructSize = sizeof(cf);
    cf.iPointSize = font_size;
    cf.nSizeMin = 8;
    cf.nSizeMax = 24;
    cf.Flags = CF_SCREENFONTS | CF_FIXEDPITCHONLY | CF_NOVERTFONTS | CF_INITTOLOGFONTSTRUCT
             | CF_LIMITSIZE | CF_FORCEFONTEXIST;

    LOGFONT lf;
#ifdef JP
    lf.lfHeight        = 21;
    lf.lfWidth         = 0;
    lf.lfEscapement    = 0;
    lf.lfOrientation   = lf.lfEscapement;
    lf.lfWeight        = FW_NORMAL;
    lf.lfItalic        = FALSE;
    lf.lfUnderline     = FALSE;
    lf.lfStrikeOut     = FALSE;
    lf.lfCharSet       = SHIFTJIS_CHARSET;
    lf.lfOutPrecision  = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality       = DEFAULT_QUALITY;
    lf.lfPitchAndFamily= FF_DONTCARE|FIXED_PITCH;
    strcpy(lf.lfFaceName, font_name);
#else
    lf.lfHeight        = 21;
    lf.lfWidth         = 0;
    lf.lfEscapement    = 0;
    lf.lfOrientation   = lf.lfEscapement;
    lf.lfWeight        = FW_NORMAL;
    lf.lfItalic        = FALSE;
    lf.lfUnderline     = FALSE;
    lf.lfStrikeOut     = FALSE;
    lf.lfCharSet       = ANSI_CHARSET;
    lf.lfOutPrecision  = OUT_DEFAULT_PRECIS;
    lf.lfClipPrecision = CLIP_DEFAULT_PRECIS;
    lf.lfQuality       = DEFAULT_QUALITY;
    lf.lfPitchAndFamily= FF_MODERN|FIXED_PITCH;
    strcpy(lf.lfFaceName, font_name);
#endif
    cf.lpLogFont = &lf;

#ifdef JP
        snprintf( info, INFO_SIZE, "現在のフォント: %s / %d",
        font_name, font_size);
#else
        snprintf( info, INFO_SIZE, "Current font: %s / %d",
        font_name, font_size);
#endif
    mpr( info );

    if (ChooseFont(&cf))
    {
        font_size = (cf.iPointSize / 10);
        strcpy(font_name, lf.lfFaceName);
#ifdef JP
        snprintf( info, INFO_SIZE, "新しいフォント: %s / %d",
        font_name, font_size);
        mpr( info );
        mpr( "新しいフォント設定はCrawlの再起動後に反映されます。" );
#else
        snprintf( info, INFO_SIZE, "New font: %s / %d",
        font_name, font_size);
        mpr( info );
        mpr( "Restart the Dungeon Crawl to enable the new font settings." );
#endif
    }
}
