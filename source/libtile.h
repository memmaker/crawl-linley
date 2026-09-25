#ifdef USE_X11
#include <X11/Xlib.h>
#include <X11/X.h>
#elif defined(WINDOWS)
#include <windows.h>
#include <commdlg.h>
#endif
#include <vector>

/*
 * サイズ等の定義
 */

// メインから渡されるダンジョンデータのサイズ
#define TILE_DAT_XMAX 17
#define TILE_DAT_YMAX 17
//通常タイルのサイズ
#define TILE_X 32
#define TILE_Y 32

// Unit size
//表示画面のサイズ(単位グリッド数)
#define TILE_XMAX_QV 24
#define TILE_YMAX_QV 13
//拡張タイルのサイズ

#ifdef QV48
#define TILE_X_EX_QV 48
#define TILE_Y_EX_QV 48
#else
#define TILE_X_EX_QV 64
#define TILE_Y_EX_QV 64
#endif

//単位グリッドのサイズ
#define TILE_UX_QV (TILE_X_EX_QV/2)
#define TILE_UY_QV (TILE_X_EX_QV/2)
//タイル画面のサイズ
#define TILE_SCR_WIDTH_QV (TILE_UX_QV*TILE_XMAX_QV)
#define TILE_SCR_HEIGHT_QV (TILE_UY_QV*TILE_YMAX_QV)

//通常タイル＋拡張タイル数
#define TILE_TOTAL2 (TILE_TOTAL + TILE_TOTAL_EX)
//通常タイル数
#define TILE_NORMAL TILE_TOTAL

//通常モード
//表示画面のサイズ(単位グリッド数)
#define TILE_XMAX_NORMAL 17
#define TILE_YMAX_NORMAL 17
//単位グリッドのサイズ
#define TILE_UX_NORMAL TILE_X
#define TILE_UY_NORMAL TILE_Y
//タイル画面のサイズ
#define TILE_SCR_WIDTH_NORMAL (TILE_UX_NORMAL*TILE_XMAX_NORMAL)
#define TILE_SCR_HEIGHT_NORMAL (TILE_UY_NORMAL*TILE_YMAX_NORMAL)

/*
 * メインから呼ばれる機種共通ルーチン
 */
//タイル版カーソルを表示
void TileCursor(int x, int y, int flag);
//ボルトを表示
void TileDrawBolt(int x, int y, int fg);
//ダンジョンを表示。tileb は fg(0,0),bg(0,0),fg(1,0),bg(1,0), ..
//の順にデータを格納
void TileDrawDungeon(short unsigned int *tileb);

//プレイヤータイル更新
void TilePlayerRefresh();
//きせかえコマンド
void TilePlayerEdit();
//初期化
void TilePlayerInit();
//ゴーストのきせかえ
void TileGhostInit(struct ghost_struct &gs);
//パンデモニウムデーモン合成(QVモードのみ)
void TilePandemInit(struct ghost_struct &gs);
// デバグ用
void TileEditPandem();
// 鑑定済アイテム初期化
void TileInitItems();

//璧タイル変更
void TileLoadWall(bool wizard);

//ミニマップ
void TileDrawGmap(unsigned char *buf);
//タイトル
void TileDrawTitle();

#if 1 // TILE_MON_EQUIP
    //装備依存モンスター表示関連
    void TileMcacheUnlock();
    int  TileMcacheFind(int mon_tile, int eq_tile);
#endif

#if 1 //Slot
void TileDrawInven(int n, int flag, int *tiles, int *num, int *idx, int *iflags);
#endif

//マウス操作
#if 1 //MOUSE
  //メインからキー入力モード設定
  void set_keyin_mode(int mode);

  //キー入力モード問い合わせ
  int get_keyin_mode();

  //メインからマウス位置取得
  void mouse_get_cursor_info(int *x, int *y);
#endif

/* テキスト描画関連 */
void clrscr(void);
void mpr_on(int mode);
void textcolor(int color);
void gotoxy(int x, int y);
int wherex();
int wherey();
void cprintf(const char *format,...);    
void clear_to_end_of_line(void);
void clear_to_end_of_screen(void);
int get_number_of_lines_tile(void);
void get_input_line_tile(char *const buff, int len);
void _setcursortype(int curstype);
void textbackground(int bg);
void textcolor(int col);
void putch(unsigned char chr);
void writeWChar(unsigned char *ch);

void puttext(int x, int y, int lx, int ly, unsigned char *buf, 
               bool mono = false);
void ViewTextFile(const char *name);

void rotate_qv_key(int *key);
