/*
 *  WinClass and RegionClass system independent imprementaions
 *   see winclass-*.cc for  system dependent implementations
 */

// Class defines
#include "winclass.h"

//Hack
#ifndef DEBUG
#define NDEBUG
#endif

int TextRegionClass::cursor_x;
int TextRegionClass::cursor_y;
int TextRegionClass::cursor_flag = 0;
TextRegionClass *TextRegionClass::text_mode = NULL;
int TextRegionClass::text_col = 0;

TextRegionClass *TextRegionClass::old_cursor_region= NULL;
int TextRegionClass::old_cursor_x;
int TextRegionClass::old_cursor_y;
int TextRegionClass::old_cursor_width;

/*------------------------------------------*/

// 初期化
WinClass::WinClass()
{
    // Minimum;
    wx = 10;
    wy = 10;

    ox = 0;
    oy = 0;
    SysInit();
}

// 終了 
WinClass::~WinClass()
{
    SysDeinit();
    regions.clear();
    layers.clear();
}

void WinClass::placeRegion(RegionClass *r, int layer0,
     RegionClass *neighbor, int pflag,
     int margin_top, int margin_left,
     int margin_bottom, int margin_right)
{
    int sx0 =0;
    int sy0 =0;
    int ex0 =0;
    int ey0 =0;

    if (neighbor!=NULL)
    {
        sx0 = neighbor->sx;
        sy0 = neighbor->sy;
        ex0 = neighbor->ex;
        ey0 = neighbor->ey;
    }

    r->win = this;
    r->layer = layer0;
    r->flag = true;

    regions.push_back(r);
    layers.push_back(layer0);

    if (pflag == PLACE_RIGHT)
    {
	r->sx = ex0;
	r->sy = sy0;
	r->ox = ex0 + margin_left;
	r->oy = sy0 + margin_top;
    }
    else
    {
	r->sx = sx0;
	r->sy = ey0;
        r->ox = sx0 + margin_left;
        r->oy = ey0 + margin_top;
    }

    r->wx = r->dx * r->mx + margin_left + margin_right;
    r->wy = r->dy * r->my + margin_top + margin_bottom;
    r->ex = r->sx + r->wx;
    r->ey = r->sy + r->wy;
    if (r->ex > wx) wx = r->ex;
    if (r->ey > wy) wy = r->ey;
}

void WinClass::redraw(int x1, int y1, int x2, int y2)
{
    std::vector <RegionClass *>::iterator r;
    int cx1, cx2, cy1, cy2;

    for (r = regions.begin();r != regions.end();r++) 
    {
        if (!(*r)->is_active()) continue;
        if( (*r)->convert_rect(x1, y1, x2, y2, &cx1, &cy1, &cx2, &cy2))
        {
            (*r)->redraw(cx1, cy1, cx2, cy2);
        }
    }
}

void WinClass::redraw()
{
    redraw(0, 0, wx-1, wy-1);
}
/*------------------------------------------*/

/* 初期化の順序
 *  0 (親クラスの処理、自動)
 *  1 機種共通処理
 *  2 機種依存処理
 *
 * 終了処理の順序
 *  1 機種依存処理
 *  2 機種共通処理
 * (3) (親クラスの処理、自動)
 */


//初期化
RegionClass::RegionClass()
{
    flag = false;
    win = NULL;
    backbuf = NULL;
    SysInit();
    ox = oy = 0;
    dx = dy = 1;
}

//終了処理
RegionClass::~RegionClass()
{
    SysDeinit();
    // バックバッファ解放
    if (backbuf != NULL) ImgDestroy(backbuf);
}

// 初期化
TextRegionClass::TextRegionClass(int x, int y, int cx, int cy)
{
    int i;

    mx = x;
    my = y;
    cbuf = (unsigned char *)malloc(x*y);
    abuf = (unsigned char *)malloc(x*y);
    for (i=0; i<x*y; i++)
    {
        cbuf[i]=' ';
        abuf[i]=0;
    }

    // Cursor Offset
    cx_ofs = cx;
    cy_ofs = cy;

    //機種依存処理
    SysInit(x, y, cx, cy);
}

//終了処理
TextRegionClass::~TextRegionClass()
{
    SysDeinit();
    free(cbuf);
    free(abuf);
}

//初期化
TileRegionClass::TileRegionClass(int mx0, int my0, int dx0, int dy0)
{
    // Unit size
    dx = dx0;
    dy = dy0;

    mx = mx0;
    my = my0;
    force_redraw = false;

    //機種依存処理
    SysInit(mx0, my0, dx0, dy0);
}

//終了処理 
TileRegionClass::~TileRegionClass()
{
    SysDeinit();
}

// 初期化
MapRegionClass::MapRegionClass(int x, int y, int o_x, int o_y, bool qv)
{
    int i;

    mx2 = x;
    my2 = y;
    qv_mode = qv;

    if(qv)
    {
        /*      /\ my2
         *  mx2/  \
         *    /   /
         *    \  / mx2
         * my2 \/
         */

        mx = my = mx2 + my2;
    }
    else
    {
        mx = mx2;
        my = my2;
    }

    mbuf = (unsigned char *)malloc(mx2*my2);

    for (i=0; i<mx2*my2; i++)
    {
        mbuf[i]=0;
    }
    x_margin = o_x;
    y_margin = o_y;
    force_redraw = false;

    //機種依存処理
    SysInit(x, y, o_x, o_y);
}

//終了処理
MapRegionClass::~MapRegionClass()
{
    SysDeinit();
    free(mbuf);
}

/*------------------------------------------*/

bool RegionClass::is_active()
{
    if (!flag) return false;
    if (win->active_layer == layer)
        return true;
    else return false;
}

void RegionClass::make_active()
{
    if (!flag) return;
    win->active_layer = layer;
} 

void RegionClass::redraw(int x1, int y1, int x2, int y2)
{
}

void MapRegionClass::set_col(int col, int x, int y)
{
    mbuf[x + y * mx2] = col;
}

int MapRegionClass::get_col(int x, int y)
{
    return mbuf[x + y * mx2];
}

/*------------------------------------------*/
bool RegionClass::convert_rect(int x1, int y1, int x2, int y2, 
                  int *rx1, int *ry1, int *rx2, int *ry2)
{
    int cx1 = x1-ox;
    int cy1 = y1-oy;
    int cx2 = x2-ox;
    int cy2 = y2-oy;

    if ( (cx2 < 0) || (cy2 < 0) || (cx1 >= dx*mx) || (cy1 >=dy*my))
        return false;

    cx1 /= dx;
    cy1 /= dy;
    cx2 /= dx;
    cy2 /= dy;

    if(cx2>=mx-1)cx2=mx-1;
    if(cy2>=my-1)cy2=my-1;
    if(cx1<0) cx1=0;
    if(cy1<0) cy1=0;

    *rx1 = cx1;
    *ry1 = cy1;
    *rx2 = cx2;
    *ry2 = cy2;

    return true;
}

bool RegionClass::mouse_pos(int mouse_x, int mouse_y, int *cx, int *cy)
{
    int x = mouse_x - ox;
    int y = mouse_y - oy;
    if (!is_active()) return false;
    if ( x < 0 || y < 0 ) return false;
    x /= dx;
    y /= dy;
    if (x >= mx || y >= my) return false;
    *cx = x;
    *cy = y;
    return true;
}

bool MapRegionClass::mouse_pos(int mouse_x, int mouse_y, int *cx, int *cy)
{
    int x = mouse_x - ox;
    int y = mouse_y - oy;
    if ( x < 0 || y < 0 ) return false;
    x /= dx;
    y /= dy;
    if (x >= mx || y >= my) return false;
    if (!is_active()) return false;

    if (qv_mode)
    {
        int xx = x - (my2 - 1);
        int yy = (mouse_y - oy + (xx * dy)) / (dy*2);
        x = yy;
        y = yy - xx;
        if ( x < 0 || y < 0 ) return false;
        if (x >= mx2 || y >= my2) return false;
    }

    *cx = x - px;
    *cy = y - py;
    return true;
}

/*
 * Text related 
 */

void TextRegionClass::scroll(){
    int idx;

    if(!flag)return;

    for(idx=0; idx<mx*(my-1);idx++)
    {
            cbuf[idx] = cbuf[idx + mx];
            abuf[idx] = abuf[idx + mx];
    }

    for(idx=mx*(my-1);idx<mx*my;idx++)
    {
        cbuf[idx] = ' ';
        abuf[idx] = 0;
    }
    redraw(0, 0, mx-1, my-1);
}

void TextRegionClass::addstr(char *buffer){
    int i,j;
    char buf2[1024];
    int len = strlen(buffer);

    if(!flag)return;

    j=0;

    for(i=0;i<len+1;i++){
        char c = buffer[i];
        char nl=0;
        if (c== '\n' || c== '\r')
        {
            c=0;
            nl=1;
            if (buffer[i+1]=='\n' || buffer[i+1]=='\r')
                i++;
        }
        buf2[j] = c;
        j++;
        if(c==0){
            if (j-1 != 0) addstr_aux(buf2, j - 1);
            if (nl)
            {
                cursor_x = cx_ofs;
                cursor_y++;
                j=0;

                if(cursor_y - cy_ofs == my)
                {
                        scroll();
                        cursor_y--;
                }
            }
        }
    }
}

void TextRegionClass::clear_to_end_of_line()
{
    int i;
    int cx = cursor_x - cx_ofs;
    int cy = cursor_y - cy_ofs;
    int col = text_col;
    int adrs = cy * mx;

    if(!flag)return;

    for(i=cx; i<mx; i++){
        cbuf[adrs+i]=' ';
        abuf[adrs+i]=col;
    }
    redraw(cx, cy, mx-1, cy);
}

void TextRegionClass::clear_to_end_of_screen()
{
    int i;
    int cy = cursor_y - cy_ofs;
    int col = text_col;

    if(!flag)return;

    for(i=cy*mx; i<mx*my; i++){
        cbuf[i]=' ';
        abuf[i]=col;
    }
    redraw(0, cy, mx-1, my-1);
}

void TextRegionClass::putch(unsigned char ch)
{
    if (ch==0) ch=32;
    addstr_aux((char *)&ch, 1);
}

void TextRegionClass::writeWChar(unsigned char *ch)
{
    addstr_aux((char *)ch, 2);
}

void TextRegionClass::textcolor(int color)
{
    text_col = color;
}

void TextRegionClass::textbackground(int col)
{
    textcolor(col*16 + (text_col & 0xf));
}

void TextRegionClass::gotoxy(int x, int y)
{
    if (old_cursor_region != NULL)
        old_cursor_region ->erase_cursor();
    old_cursor_region = NULL;
    cursor_x = x-1;    
    cursor_y = y-1;    

    if (cursor_flag /*&& !Options.use_tile*/) text_mode->draw_cursor(x-1, y-1);
}

int  TextRegionClass::wherex()
{
    return cursor_x + 1;
}

int  TextRegionClass::wherey()
{
    return cursor_y + 1;
}

void TextRegionClass::_setcursortype(int curstype)
{
    cursor_flag = curstype;
}

int  TextRegionClass::get_number_of_lines()
{
    return (text_mode->cx_ofs + text_mode->my);
}
