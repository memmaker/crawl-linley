#ifdef USE_TILE

/* Flags for drawing routines */
#define TILE_FLAG_FLYING 0x8000 //flying object is always fg
#define TILE_FLAG_PET    0x4000 //pet is always fg
#define TILE_FLAG_S_UNDER    0x2000 //fg

#define TILE_FLAG_CURSOR 0xc000 //cursor is always bg
#define TILE_FLAG_CURSOR1 0x8000 //cursor is always bg
#define TILE_FLAG_CURSOR2 0x4000 //cursor is always bg
#define TILE_FLAG_CURSOR3 0xc000 //cursor is always bg
#define TILE_FLAG_CURSOR0 0x0000 //cursor is always bg

#define TILE_FLAG_UNSEEN 0x2000 //unseen flag is set to bg


#define TILE_FLAG_MASK   0x1fff

/* tiles.cc */
int tileidx(unsigned int object, int extra);
int tileidx_player(int job);
int tileidx_unseen(int ch);
int tileidx_item(const item_def &item);
int tileidx_item_throw(const item_def &item, int dx, int dy);
int tileidx_bolt(const bolt &bolt);

#if 1 //QV
void init_tile_qv_table();
int  simple_qv_tile(int tile);
#else
#define simple_qv_tile(x) (x)
#endif
// Modify wall tile index, etc.
void modifiy_qv_tile(int *tile, int wall_flag);

void tilep_race_default(int race, int level, int *parts);
void tilep_job_default(int job, int *parts);
void tilep_calc_flags(int parts[], int flag[]);

void tilep_part_to_str(int number, char *buf);
int tilep_str_to_part(char *str);

void tilep_scan_parts(char *fbuf, int *parts);
void tilep_print_parts(char *fbuf, int *parts);

int tilep_equ_weapon(const item_def &item);
int tilep_equ_shield(const item_def &item);
int tilep_equ_armour(const item_def &item);
int tilep_equ_cloak(const item_def &item);

/*************************************************************************
 Minimap related
 * *********************************************************************** */

/* ***********************************************************************
 * called from: misc
 * *********************************************************************** */
void init_gmap();

/* ***********************************************************************
 * called from: misc, spells2
 * *********************************************************************** */
void update_gmap(int x, int y, int what);

/*************************************************************************
 Tile display related
 * *********************************************************************** */
void viewwindow_tile(char draw_it, bool do_updates);
void tile_place_monster(int x, int y, int idx);
void tile_place_item(int x, int y, int idx);
void tile_place_cloud(int x, int y, int type, int decay);
void tile_clear_buf();
void tile_restore_buf();

#if 1 //Slot
// Tile Inventry display
void TileDrawInvenAux(int item_type, int flag);
// Multiple pickup
void TilePickMenu();

#define TILEI_FLAG_SELECT 0x100
#define TILEI_FLAG_TRIED  0x200
#define TILEI_FLAG_FLOOR  0x400
#endif

//性別
extern char tilep_gender;
#define TILEP_GENDER_MALE 0
#define TILEP_GENDER_FEMALE 1

//dolls.txt ロードのモード
#define TILEP_M_DEFAULT   0        //起動時にdolls.txtを自動ロード
#define TILEP_M_LOADING   1        //起動時にデフォルトからピックアップ

//装備依存パーツの予約定数
#define TILEP_SHOW_EQUIP 0x1000

//パーツ表示/非表示フラグ 種族によっては下をカットする
#define TILEP_FLAG_HIDE 0
#define TILEP_FLAG_NORMAL 1
#define TILEP_FLAG_CUT_CENTAUR 2
#define TILEP_FLAG_CUT_NAGA 3

#ifdef TILEP_DEBUG
//デバグ用
const char *get_ctg_name(int part);
int get_ctg_idx(char *name);
const char *get_parts_name(int part, int idx);
int get_parts_idx(int part, char *name);
#endif

#endif
