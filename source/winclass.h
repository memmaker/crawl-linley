#ifdef USE_X11
#include <X11/Xlib.h>
#include <X11/X.h>
#elif defined(WINDOWS)
#include <windows.h>
#include <commdlg.h>
#endif

#include <vector>

/*
 * 内部イメージ型の定義
 */

#ifdef USE_X11
/*********** X11 ********/
typedef XImage *img_type;
#define ImgWidth(img) (img->width)
#define ImgHeight(img) (img->height)

#elif defined(WINDOWS)
/********** Windows *****/
//DIB情報格納用の構造体
typedef struct dib_pack
{
    LPBITMAPINFO       pDib     ;    //タイルDIBのヘッダ+パレットへのポインタ
    HBITMAP            hDib     ;    //タイルをDIBとして保持
    HDC                hDC      ;    //タイルのデバインコンテキストハンドル
    LPBYTE             pDibBits ;    //DIBの先頭バイトへのポインタ
    LPBYTE             pDibZero ;    //DIBの(0,0)点のバイトへのポインタ
    int                Width    ;    //DIBの幅
    int                Height   ;    //DIBの高さ
} dib_pack;
typedef dib_pack *img_type;
#define ImgWidth(img) (img->Width)
#define ImgHeight(img) (img->Height)

#endif

/** 機種依存内部イメージ操作ルーチン  winclass-*.cc  **/
void ImgCopy(img_type src,  int sx, int sy, int wx, int wy,
             img_type dest, int dx, int dy, int copy);
//フチの色#101010 をマゼンタに変換 カーソル配置時のハイライト用
void ImgCopyH(img_type src,  int sx, int sy, int wx, int wy,
              img_type dest, int dx, int dy, int copy);
// 配列 mask でマスクアウト  QV の壁の透過用
void ImgCopyMasked(img_type src,  int sx, int sy, int wx, int wy,
                   img_type dest, int dx, int dy, char *mask);
// 配列 mask でマスクアウト＋マゼンタ変換
void ImgCopyMaskedH(img_type src,  int sx, int sy, int wx, int wy,
                    img_type dest, int dx, int dy, char *mask);

// メモリ領域だけ作成
img_type ImgCreateSimple(int wx, int wy);
// ファイルから作成
img_type ImgLoadFile(char *name);

// 開放
void ImgDestroy(img_type img);
// クリア
void ImgClear(img_type img);
// 透明色 pix_transparent #476c6c かどうか
bool ImgIsTransparentAt(img_type img, int x, int y);

#ifdef WINDOWS
//scr_mode
#define STAT_NORMAL     0
#define STAT_MAP        1
#define STAT_PAPER_DOLL 2
#define STAT_TITLE      3
#define STAT_END        4
#endif

/*
 * ウィンドウと内部領域(ステータス、メッセージ、ダンジョン) のクラス
 */

class WinClass
{
    public:
    int ox;  //Offset x in dots
    int oy;  //Offset y in dots

    int wx; //width in dots
    int wy; //height in dots

    std::vector<class RegionClass *> regions;
    std::vector<int> layers;

    // 同一ウィンドウでダンジョン/インベントリなど同じ場所で
    // 表示を切り替える場合に使用
    int active_layer;

#ifdef WINDOWS
    // 通常・[X]地図・着せ替えなど ゲームの操作モードの状態
    //int game_state;
#endif

    // Pointer to the window
#ifdef USE_X11
    Window win;
#elif defined(WINDOWS)
    HWND hWnd;
#endif

    // 初期化
    WinClass();

    // 機種依存追加処理
    void SysInit();
    void SysDeinit();

    // 終了
    ~WinClass();

    //黒で塗りつぶす Sys dep
    void clear();
    //作成 Sys dep
#ifdef USE_X11
    void create(char *name);
#elif defined(WINDOWS)
    BOOL create(const char *name, int sx, int sy);
#endif

    // Region を置く
    void placeRegion(class RegionClass *r, int layer, 
                     class RegionClass *neighbor,    
                     int pflag, int margin_top, int margin_left,
                     int margin_bottom, int margin_right);

    //長方形を塗りつぶす
    void fillrect(int left, int right, int top, int bottom, int color);

    //再描画
    void redraw(int x1, int y1, int x2, int y2);
    void redraw();
};

class RegionClass
{
    public:

    WinClass *win;
    int layer;

    // Geometry
    // <-----------------wx----------------------->
    // sx     ox                                ex
    // |margin| text/tile area            |margin|

    int ox;  //Offset x in dots
    int oy;  //Offset y in dots

    int dx;  //unit width
    int dy;  //unit height

    int mx;  //window width in dx
    int my;  //window height in dy

    int wx; //width in dots = dx*mx + margins
    int wy; //height in dots = dy*my + margins

    int sx; //Left edge pos
    int sy; //Top edge pos
    int ex; //right edge pos
    int ey; //bottom edge pos

    bool flag;// use or no

    // 再描画用内部イメージを使用する場合に使う
    // 内部イメージへのポインタ
    // Win では タイルとマップ、 X11 ではタイルで使用
    img_type backbuf;
#ifdef WINDOWS
    void init_backbuf(RGBQUAD *pPal, int ncol);
#else
    void init_backbuf();
#endif

    // フォント関連
    // 着せ変えなどでタイル領域でも使用
    int fx; //フォントサイズ  dx,dy と異っても良い(行間マージンなど)
    int fy;
#ifdef USE_X11
    int asc; //font ascent
  #ifdef JP
    XFontSet font; //fontset
  #else
    XFontStruct *font;
  #endif
    void init_font(const char *name);
#elif defined(WINDOWS)
    HFONT font;
    void init_font(const char *name, int height);
#endif

    // 初期化
    RegionClass();

    // 機種依存追加処理
    void SysInit();
    void SysDeinit();

    //終了処理
    virtual ~RegionClass();

    // Sys indep
    bool is_active();
    void make_active();

    //以下はテキスト/マップ/タイル等の派生クラスで挙動を変える

    //Sys indep
    //マウス位置をダンジョン位置に変換、範囲内なら true を返す
    virtual bool mouse_pos(int mouse_x, int mouse_y, int *cx, int *cy);

    //矩形領域座標を region 内座標に
    bool convert_rect(int x1, int y1, int x2, int y2, 
                      int *rx1, int *ry1, int *rx2, int *ry2);

    //Sys dep
    // exposure 時などの再描画
    virtual void redraw(int x1, int y1, int x2, int y2);

    //低レベル描画  カーソル等用
    // TileRegionClass 等では 画面だけでなくバックバッファ内部イメージにも描画
    //長方形を塗りつぶす
    virtual void fillrect(int left, int right, int top, int bottom, int color);
    //長方形を描く
    virtual void framerect(int left, int right, int top, int bottom, int color);

    // Sys dep
    // 黒で塗る
    virtual void clear();
};


// 派生
class TextRegionClass :public RegionClass
{
    public:
    //初期化
    TextRegionClass(int x, int y, int cx, int cy);

    // 機種依存追加処理
    void SysInit(int x, int y, int cx, int cy);
    void SysDeinit();

    //終了処理
    ~TextRegionClass();

    //カーソル関連
    //現在の文字描画位置
    static int cursor_x;
    static int cursor_y;
    static int cursor_flag;
    //現在の文字描画 region
    static class TextRegionClass *text_mode;
    static int text_col;
    //前回表示したカーソルの region、位置
    static class TextRegionClass *old_cursor_region;
    static int old_cursor_x;
    static int old_cursor_y;
    static int old_cursor_width;

    // メソッド
    static void gotoxy(int x, int y);
    static int wherex();
    static int wherey();
    static int get_number_of_lines(void);
    static void _setcursortype(int curstype);
    static void textbackground(int bg);
    static void textcolor(int col);

    // Object's method
    void clear_to_end_of_line(void);
    void clear_to_end_of_screen(void);
    void putch(unsigned char chr);
    void writeWChar(unsigned char *ch);

    unsigned char *cbuf; //text backup
    unsigned char *abuf; //textcolor backup

    int cx_ofs; //cursor x offset
    int cy_ofs; //cursor y offset

    //Sys dep
    void addstr_aux(char *buffer, int len);
    void draw_cursor(int x, int y, int width);
    void erase_cursor();
    void draw_cursor(int x, int y);
    void clear();
    void init_backbuf();
    void redraw(int x1, int y1, int x2, int y2);
    void redraw();

    //Sys indep
    void addstr(char *buffer);
    void scroll();
    //bool mouse_pos(int mouse_x, int mouse_y, int *cx, int *cy);
};

class TileRegionClass :public RegionClass
{
    public:
    bool force_redraw;

    void DrawPanel(int left, int top, int width, int height);
    //バックバッファで長方形を塗りつぶす
    void fillrect(int left, int right, int top, int bottom, int color);
    //バックバッファで長方形を描く
    void framerect(int left, int right, int top, int bottom, int color);

    //bool mouse_pos(int mouse_x, int mouse_y, int *cx, int *cy);
    void redraw(int x1, int y1, int x2, int y2);
    void redraw();
    void clear();
    //Sys dep
#ifdef WINDOWS
    void init_backbuf(RGBQUAD *pPal);
#else
    void init_backbuf();
#endif

    // 初期化
    TileRegionClass(int mx0, int my0, int dx0, int dy0);

    // 機種依存追加処理
    void SysInit(int mx0, int my0, int dx0, int dy0);
    void SysDeinit();

    //終了処理 
    ~TileRegionClass();
};

class MapRegionClass  :public RegionClass
{
    public:
    int px; // remember player position
    int py; // remember player position
    int mx2;  //actual map size (mx!=mx2 when qv_mode is on
    int my2;
    int x_margin;
    int y_margin;
    bool qv_mode;
    unsigned char *mbuf;
    bool force_redraw;
    bool mouse_pos(int mouse_x, int mouse_y, int *cx, int *cy);
    void draw_data(unsigned char *buf);
    void redraw(int x1, int y1, int x2, int y2);
    void redraw();
    void clear();

    //Sys dep
    void init_backbuf();

    void set_col(int col, int x, int y);
    int get_col(int x, int y);

    // 初期化
    MapRegionClass(int x, int y, int o_x, int o_y, bool qv);

    // 機種依存追加処理
    void SysInit(int x, int y, int o_x, int o_y);
    void SysDeinit();

    //終了処理
    ~MapRegionClass();
};

#define PLACE_RIGHT 0
#define PLACE_BOTTOM 1
#define PLACE_FORCE 2

