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

// WinClass & RegionClass definitions
#include "winclass.h"

// Game core code constants
#include "defines.h"

// From libwin.cc
#define PAL_STD    0  //standard palette
#define PAL_BER    1  //berserk palette
#define PAL_SHA    2  //shadow palette

extern HINSTANCE hInst;
extern int       nCmdShow0;

// colors
extern COLORREF pix_col[16];
extern BYTE pix_transparent;
extern BYTE pix_black;
extern BYTE pix_white;
extern BYTE pix_magenta;
extern BYTE pix_rimcolor;

void WinClass::SysInit()
{
    hWnd = NULL;
}

void WinClass::SysDeinit()
{}

void RegionClass::SysInit()
{
    font = NULL;
}

void RegionClass::SysDeinit()
{
    if (font != NULL) DeleteObject(font);
}

void TextRegionClass::SysInit(int x, int y, int cx, int cy)
{}

void TextRegionClass::SysDeinit()
{}

void TileRegionClass::SysInit(int mx0, int my0, int dx0, int dy0)
{}

void TileRegionClass::SysDeinit()
{}

void MapRegionClass::SysInit(int x, int y, int o_x, int o_y)
{}

void MapRegionClass::SysDeinit()
{}

void RegionClass::init_font(const char *name, int height){
    int wid, hgt;
#ifdef JP
    LOGFONT lf;
    lf.lfHeight        = height;
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
    strcpy(lf.lfFaceName, name);
    font = CreateFontIndirect( &lf );
#else
    LOGFONT lf;
    lf.lfHeight        = height;
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
    strcpy(lf.lfFaceName, name);
    font = CreateFontIndirect( &lf );
#endif

    wid = lf.lfWidth;
    hgt = lf.lfHeight;
    /* This part is taken from angband */
    /* Hack -- Unknown size */
    if (!wid || !hgt)
    {
        HDC hdcDesktop;
        HFONT hfOld;
        TEXTMETRIC tm;

        /* all this trouble to get the cell size */
        hdcDesktop = GetDC(HWND_DESKTOP);
        hfOld = (HFONT)SelectObject(hdcDesktop, font);
        GetTextMetrics(hdcDesktop, &tm);
        SelectObject(hdcDesktop, hfOld);
        ReleaseDC(HWND_DESKTOP, hdcDesktop);

        /* Font size info */
        wid = tm.tmAveCharWidth;
        hgt = tm.tmHeight;
    }

    fx = dx = wid;
    fy = dy = hgt;
}

void RegionClass::init_backbuf(RGBQUAD *pPal, int ncolor)
{
    int i;

    backbuf = (dib_pack *)GlobalAlloc(GPTR, sizeof(dib_pack));

    //情報ヘッダ+パレットを入れる為のメモリ確保
    backbuf->pDib = (LPBITMAPINFO)GlobalAlloc(GPTR,
                (sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)) );

    backbuf->pDib->bmiHeader.biSize     = sizeof(BITMAPINFOHEADER);
    backbuf->pDib->bmiHeader.biWidth    = mx*dx;
    backbuf->pDib->bmiHeader.biHeight   = my*dy;
    backbuf->pDib->bmiHeader.biPlanes   = 1;
    backbuf->pDib->bmiHeader.biBitCount = 8;
    backbuf->pDib->bmiHeader.biCompression=BI_RGB;
    backbuf->pDib->bmiHeader.biSizeImage=0;
    backbuf->pDib->bmiHeader.biXPelsPerMeter=0;
    backbuf->pDib->bmiHeader.biYPelsPerMeter=0;
    backbuf->pDib->bmiHeader.biClrUsed=0;
    backbuf->pDib->bmiHeader.biClrImportant=0;

    //バックバッファ用パレットの作成
    for (i = 0; i < ncolor; i++)
    {
        //バックバッファDBIではカラーパレットをタイルから転写
        backbuf->pDib->bmiColors[i].rgbRed   = pPal[i].rgbRed  ;
        backbuf->pDib->bmiColors[i].rgbGreen = pPal[i].rgbGreen;
        backbuf->pDib->bmiColors[i].rgbBlue  = pPal[i].rgbBlue ;
    }

    //参照用の構造体に格納
    backbuf->Width = mx * dx;
    backbuf->Height= my * dy;
    backbuf->pDibZero = (backbuf->pDibBits) + (backbuf->Height -1) * backbuf->Width;

    if (win != NULL)
    {
        // ウィンドウが初期化してない場合はウィンドウのcreate時に一括処理
        if (win->hWnd != NULL)
        {
            HDC hdc1 = GetDC(0);
            HDC hdc2 = GetDC(win->hWnd);
            backbuf->hDib = CreateDIBSection(hdc1, backbuf->pDib, DIB_RGB_COLORS,
                               (VOID **)&(backbuf->pDibBits), NULL, 0);
            backbuf->hDC = CreateCompatibleDC(hdc2);
            SelectObject(backbuf->hDC, backbuf->hDib);
            ReleaseDC(win->hWnd, hdc2);
            ReleaseDC(0, hdc1);
        }
    }
}

void TextRegionClass::init_backbuf()
{
  /* not use */
}

void TileRegionClass::init_backbuf(RGBQUAD *pPal)
{
    int i;
    RegionClass::init_backbuf(pPal, 256);

    //バックバッファ用パレット
    for (i = 0; i < 256; i++)
    {
        if ( (pPal[i].rgbRed   == 0)
           &&(pPal[i].rgbGreen == 0)
           &&(pPal[i].rgbBlue  == 0) )
            pix_black = i;

        //白(255, 255, 255)の番号を格納
        if ( (pPal[i].rgbRed   == 255)
           &&(pPal[i].rgbGreen == 255)
           &&(pPal[i].rgbBlue  == 255) )
            pix_white = i;

        //マゼンタ(255, 0, 255)の番号を格納
        if ( (pPal[i].rgbRed   == 255)
           &&(pPal[i].rgbGreen == 0  )
           &&(pPal[i].rgbBlue  == 255) )
            pix_magenta = i;

        //縁の色(1, 1, 1)の番号を格納
        if ( (pPal[i].rgbRed   == 1)
           &&(pPal[i].rgbGreen == 1)
           &&(pPal[i].rgbBlue  == 1) )
            pix_rimcolor = i;
    }

    //バックバッファの透明色は黒に変更する
    backbuf->pDib->bmiColors[pix_transparent].rgbRed   = 0;
    backbuf->pDib->bmiColors[pix_transparent].rgbGreen = 0;
    backbuf->pDib->bmiColors[pix_transparent].rgbBlue  = 0;

    for (i = 0; i< mx*dx*my*dy; i++)
        *(backbuf->pDibBits + i) = pix_black; //バックバッファを黒で埋める。

}

void MapRegionClass::init_backbuf()
{
    int i;

//ミニマップ用パレットの作成
    BYTE black;
    RGBQUAD scol[16] = {
    {  0,   0,   0, 0},
    {255,  82,   0, 0},
    { 70, 185, 100, 0},
    {180, 180,   0, 0},//!!!
    {  0,  48, 255, 0},
    {238,  92, 238, 0},
    {  0,  91, 165, 0},
    {162, 162, 162, 0},
    { 82,  82,  82, 0},
    {255, 102,  82, 0},
    { 82, 255,  82, 0},
    {255, 255,  82, 0},
    { 82,  82, 255, 0},
    {255,  82, 255, 0},
    { 82, 255, 255, 0},
    {255, 255, 255, 0}};

    RegionClass::init_backbuf(scol, 16);

    for (i = 0; i < 16; i++)
    {
        //黒(0, 0, 0)の番号を格納
        if ( (backbuf->pDib->bmiColors[i].rgbRed   == 0)
           &&(backbuf->pDib->bmiColors[i].rgbGreen == 0)
           &&(backbuf->pDib->bmiColors[i].rgbBlue  == 0) )
            black = i;
    }

    for (i = 0; i< mx*dx*my*dy; i++)
       *(backbuf->pDibBits + i) = black; //黒で埋める。
}

void TextRegionClass::addstr_aux(char *buffer, int len){
    int i, head, tail;
    RECT rc;
    HDC hdc;

    int x = cursor_x - cx_ofs;
    int y = cursor_y - cy_ofs;
    int col = text_col;
    int adrs = y * mx;

    if(!flag)return;

    head = 0;
    tail = mx;

    i = 0;
    while (i<mx){
        if(i<=x) head=i;
        if(i>x+len-1){tail=i;break;}
        if (cbuf[adrs+i] & 0x80) i+=2;
        else i++;
    }

    cbuf[adrs+head] = ' ';
    abuf[adrs+head] = col;

    cbuf[adrs+tail-1] = ' ';
    abuf[adrs+tail-1] = col;

    for (i=0;i<len;i++){
        cbuf[adrs+x+i]=buffer[i];
        abuf[adrs+x+i]=col;
    }

    rc.left = ox + x * dx;
    rc.right = rc.left + (tail-head) * dx;
    rc.top =  oy + y * dy;
    rc.bottom = rc.top + dy;

    hdc =GetDC(win->hWnd);
    SelectObject(hdc, font);
    SetBkColor(hdc, pix_col[col>>4]);
    SetTextColor(hdc, pix_col[col&0x0f]);
    ExtTextOut(hdc, rc.left, rc.top,  ETO_CLIPPED, &rc,
                       (char *)&cbuf[adrs+head], tail-head, NULL);
    ReleaseDC(win->hWnd, hdc);

    cursor_x += len;
}

// defined to object, not to class
void TextRegionClass::draw_cursor(int x, int y){
    RECT rc;
    HDC hdc;

    int cx = x - cx_ofs;
    int cy = y - cy_ofs;

    if(!flag)return;

    hdc =GetDC(win->hWnd);
    SelectObject(hdc, font);

    rc.left = ox + cx * dx ;
    rc.right = rc.left + (2 * dx);
    rc.top =  oy + cy * dy;
    rc.bottom = rc.top + dy;

    SetBkMode(hdc, TRANSPARENT);
    SetTextColor(hdc, pix_col[0x0f]);
    ExtTextOut(hdc, rc.left, rc.top,  ETO_CLIPPED, &rc,
                           "_ ", 2, NULL);
    ReleaseDC(win->hWnd, hdc);

    old_cursor_x = cx;
    old_cursor_y = cy;
    old_cursor_width = 1;
    old_cursor_region = this;
}

void TextRegionClass::erase_cursor(){

    int x0 = old_cursor_x;
    int y0 = old_cursor_y;
    int adrs = y0 * mx + x0;
    int col = abuf[adrs];

    if(!flag)return;

    RECT rc;
    HDC hdc =GetDC(win->hWnd);
    SelectObject(hdc, font);

    //redraw previous cursor cell
    rc.left = ox + x0 * dx;
    rc.right= rc.left + (2 * dx);
    rc.top  = oy + y0 * dy;
    rc.bottom = rc.top + dy;
    unsigned char rchar[3];

    SetBkColor(hdc, pix_col[col>>4]);
    SetTextColor(hdc, pix_col[col&0x0f]);
    rchar[0] = cbuf[adrs];
    if ( _ismbblead( rchar[0]) )
    {
        rchar[1] = cbuf[adrs+1];
        rchar[2] = '\0';
        ExtTextOut(hdc, rc.left, rc.top,  ETO_CLIPPED, &rc,
                           (char *)&rchar, 2, NULL);
    }
    else
    {
        rchar[1] = '\0';
        ExtTextOut(hdc, rc.left, rc.top,  ETO_CLIPPED, &rc,
                           (char *)&rchar, 1, NULL);
    }
    ReleaseDC(win->hWnd, hdc);
}


void WinClass::clear()
{
    fillrect(0, 0, wx-1, wy-1, PIX_BLACK);
}

void RegionClass::clear()
{
    fillrect(0, 0, wx-1, wy-1, PIX_BLACK);
}

void TileRegionClass::clear()
{
    RegionClass::clear();
}

void MapRegionClass::clear()
{
    int i;

    for (i=0; i<mx2*my2; i++)
    {
        mbuf[i]=PIX_BLACK;
    }
    RegionClass::clear();
}

void TextRegionClass::clear()
{
    int i;

    for (i=0; i<mx*my; i++)
    {
        cbuf[i]=' ';
        abuf[i]=0;
    }
    RegionClass::clear();
}

BOOL WinClass::create(const char *name, int sx, int sy)
{

    int wintop = sy;
    int winleft = sx;
    RECT rc;
    rc.left   = 0;
    rc.right  = wx;
    rc.top    = 0;
    rc.bottom = wy;

    //game_state = STAT_NORMAL;

    if ( GetSystemMetrics(SM_CYSCREEN) < (wintop + rc.bottom) )
        wintop =0;
    if ( GetSystemMetrics(SM_CXSCREEN) < (winleft + rc.right) )
        winleft =0;

    AdjustWindowRectEx(&rc,
                       (WS_OVERLAPPED  | WS_SYSMENU |
                        WS_MINIMIZEBOX | WS_CAPTION |
                        WS_VISIBLE),
                       false, 0);
    hWnd = CreateWindowEx(0, "CrawlList",name,
                          (WS_OVERLAPPED  | WS_SYSMENU |
                           WS_MINIMIZEBOX | WS_CAPTION |
                           WS_VISIBLE),
                           winleft, wintop, //pos
                           rc.right - rc.left, rc.bottom - rc.top, //size
                           HWND_DESKTOP, NULL, hInst, NULL);
   if( !hWnd )
   {
      return FALSE;
   }

   ShowWindow( hWnd, nCmdShow0 );
   UpdateWindow( hWnd );
   clear();

   // Init pending backbuf of regions
   std::vector<RegionClass *>::iterator r;

   for (r = regions.begin();r != regions.end();r++)
   {
        if ( (*r)->backbuf != NULL)
        {
            img_type b = (*r)->backbuf;
            HDC hdc1 = GetDC(0);
            HDC hdc2 = GetDC(hWnd);
            b->hDib = CreateDIBSection(hdc1, b->pDib, DIB_RGB_COLORS,
                               (VOID **)&(b->pDibBits), NULL, 0);
            b->hDC = CreateCompatibleDC(hdc2);
            SelectObject(b->hDC, b->hDib);
            ReleaseDC(hWnd, hdc2);
            ReleaseDC(0, hdc1);
        }
   }

   return TRUE;
}

void TileRegionClass::redraw(int x1, int y1, int x2, int y2)
{
    if (!flag) return;
    if (!is_active()) return;

    HDC hdc = GetDC(win->hWnd);
    BitBlt(hdc, ox, oy, mx*dx, my*dy,
           backbuf->hDC, 0, 0, SRCCOPY);
    ReleaseDC(win->hWnd, hdc);
}

void TileRegionClass::redraw()
{
    redraw(0, 0, mx-1, my-1);
}

void MapRegionClass::redraw(int x1, int y1, int x2, int y2)
{
    if (!flag) return;
    if (!is_active()) return;
    HDC hdc =GetDC(win->hWnd);
    BitBlt(hdc, ox, oy, dx*mx, dy*my,
           backbuf->hDC, 0, 0, SRCCOPY);
    ReleaseDC(win->hWnd, hdc);
}

void MapRegionClass::redraw()
{
    redraw(0, 0, mx-1, my-1);
}

void TextRegionClass::redraw(int x1, int y1, int x2, int y2){
    RECT rc;
    HDC hdc;

    int head, tail;
    int x,y,col,oldcol;

    if (!flag) return;
    if (!is_active()) return;

    for(y=y1;y<=y2;y++){
        int adrs = y * mx;
        head = 0;
        tail = mx;
        x = 0;

        while (x<mx){
            if(x<=x1) head=x;
            if(x>x2){tail=x;break;}
            if (cbuf[adrs+x] & 0x80) x+=2;
            else x++;
        }

        x=head;
        col = abuf[adrs+x];

        while (x<=tail)
        {
            oldcol = col;
            col = abuf[adrs+x];
            if (oldcol != col || x==tail)
            {
                hdc =GetDC(win->hWnd);
                SelectObject(hdc, font);

                SetBkColor(hdc, pix_col[oldcol>>4]);
                SetTextColor(hdc, pix_col[oldcol&0x0f]);

                rc.left   = head * dx + ox;
                rc.right  = rc.left + (x-head) * dx;
                rc.top    = y * dy + oy;
                rc.bottom = rc.top + dy;

                ExtTextOut(hdc, rc.left, rc.top,  ETO_CLIPPED, &rc,
                          (char *)&cbuf[adrs+head], x-head, NULL);
                ReleaseDC(win->hWnd, hdc);
                head = x;
            }
            if (cbuf[adrs+x] & 0x80) x+=2;
            else x++;
        }
    }
}

void TextRegionClass::redraw()
{
    redraw(0, 0, mx-1, my-1);
}

void MapRegionClass::draw_data(unsigned char *buf){
    int i, j, x, y, col;
    int dx2, dy2;

    if(!flag)return;

    LPBYTE ppix ,dpix;
    // 外側、内側の x, y ループでのアドレス増分
    int inc_x, inc_y, inc_x0, inc_y0;
    LPBYTE pmmapDibBits = backbuf->pDibBits;
    int mmapDibX = mx * dx;
    int mmapDibY = my * dy;

    if (qv_mode)
    {
        dx2 = dx;
        dy2 = dy * 2;
        ppix  = pmmapDibBits + mmapDibX * (mmapDibY - 1) + (my2 - 1) * dx;
        inc_x = dx - mmapDibX * dy;
        inc_x0 = 1;

        inc_y = -(mmapDibX * dy) - dx - mx2 * inc_x;
        inc_y0 = -(mmapDibX) - dx * inc_x0;
    }
    else
    {
        dx2 = dx;
        dy2 = dy;
        ppix  = pmmapDibBits + mmapDibX * (mmapDibY - 1);

        inc_x = dx;
        inc_x0 = 1;

        // 上にdy 左に mmapDibX
        inc_y = -(mmapDibX * dy) - mx2 * inc_x;
        // 上に1 左に dx
        inc_y0 = -(mmapDibX) - dx * inc_x0;
    }

    dpix  = ppix;
    for (j = 0; j < my2; j++)
    {
        for (i = 0; i < mx2; i++)
        {
            col=buf[(i+x_margin) + (j+y_margin)*GXM];
            if (col == PIX_WHITE)
            {
                px = i;
                py = j;
            }
            if ( (col != get_col(i,j)) || force_redraw)
            {
                dpix = ppix;
                for (y=0; y<dy2; y++)
                {
                    for (x=0; x<dx2; x++)
                    {
                        *dpix = col;
                        dpix += inc_x0;
                    }
                    dpix += inc_y0;
                }
                set_col(col, i,j);
            }
            ppix += inc_x;
        }
        ppix += inc_y;
    }

    redraw();
    force_redraw = false;
}

/* XXXXX
 * img_type related
 */

LPBYTE dib_ref_pixel(dib_pack* dib, int x, int y)
{
    int w = ((3 + dib->Width)/4)*4;
    LPBYTE ref = dib->pDibBits + x + (dib->Height -1 -y) * w;
    return ref;
}

bool ImgIsTransparentAt(img_type img, int x, int y)
{
    if (pix_transparent == *( dib_ref_pixel(img, x, y) )) return true;
    return false;
}

img_type ImgCreateSimple(int wx, int wy)
{
    if (wx ==0 || wy == 0) return NULL;
    dib_pack *ptr = (dib_pack *)GlobalAlloc(GPTR, sizeof(dib_pack));

    ptr->pDibBits = (LPBYTE)GlobalAlloc(GPTR, wx*wy );
    ptr->pDibZero = ptr->pDibBits + (wy -1)* wx;
    ptr->Width    = wx;
    ptr->Height   = wy;

    ptr->pDib = NULL;
    ptr->hDib = NULL;
    ptr->hDC  = NULL;

    return ptr;
}

void ImgDestroy(img_type img)
{
    if (!img) return;

    //if (img->pDibBits) GlobalFree(img->pDibBits);
    if (img->pDib) GlobalFree(img->pDib);
    if (img->hDC)  DeleteDC  (img->hDC);
    if (img->hDib) DeleteObject(img->hDib);

    GlobalFree(img);
}

img_type ImgLoadFile(char *name)
{
    HANDLE fh;
    DWORD dummy;
    BITMAPFILEHEADER bmHead;
    int BitsSize;
    HDC hdc1;
    dib_pack *img;

    hdc1 = GetDC(0);

    //ファイルを開く
    fh=CreateFile(name, GENERIC_READ,0,NULL,OPEN_EXISTING,
        FILE_ATTRIBUTE_NORMAL,NULL);
    if (fh == INVALID_HANDLE_VALUE) return NULL;

    SetFilePointer(fh,0,NULL,FILE_BEGIN);

    //ファイルヘッダ読み込み
    if (!ReadFile(fh,&bmHead, sizeof(BITMAPFILEHEADER), &dummy, NULL))
    return NULL;

    //構造体を確保
    img = (dib_pack *) GlobalAlloc(GPTR, sizeof(dib_pack));
    if (!img) return  NULL;

    //情報ヘッダ+パレットを入れる為のメモリ確保
    img->pDib = (LPBITMAPINFO)GlobalAlloc(GPTR,
                (sizeof(BITMAPINFO) + 256 * sizeof(RGBQUAD)) );
    if (img->pDib == NULL)
    {
        GlobalFree(img);
        return NULL;
    }

    //パレットを含む情報ヘッダ読み込み
    SetFilePointer(fh, sizeof(BITMAPFILEHEADER), NULL, FILE_BEGIN);
    if (!ReadFile(fh,img->pDib, sizeof(BITMAPINFOHEADER)+ 256 * sizeof(RGBQUAD),
             &dummy, NULL))
    {
        GlobalFree(img->pDib);
        GlobalFree(img);
        return NULL;
    }

     //DIB作成
    img->hDib = CreateDIBSection(hdc1, img->pDib, DIB_RGB_COLORS,
                                (VOID **)&(img->pDibBits), NULL,0);
    if (img->hDib == NULL)
    {
        GlobalFree(img->pDib);
        GlobalFree(img);
        return NULL;
    }


    //画像のビット列のサイズ
    BitsSize = bmHead.bfSize-bmHead.bfOffBits;
    //ビット配列をポイント
    SetFilePointer(fh, bmHead.bfOffBits, NULL, FILE_BEGIN);

    //読み込み
    if (!ReadFile(fh, img->pDibBits, BitsSize, &dummy, NULL))
    {
        GlobalFree(img->hDib);
        GlobalFree(img->pDib);
        GlobalFree(img);
        return NULL;
    }

    CloseHandle(fh); //ファイルを閉じる

    //参照用の構造体に格納
    img->Width    = img->pDib->bmiHeader.biWidth ;
    img->Height   = img->pDib->bmiHeader.biHeight;
    img->pDibZero = img->pDibBits + (img->Height - 1) * img->Width;

    ReleaseDC(0, hdc1);
    return img;
}

void ImgClear(img_type img)
{
    int i;
    for (i = 0; i< (img->Width * img->Height); i++)
        *(img->pDibBits + i) = pix_transparent;
}

// Copy internal image to another internal image
void ImgCopy(img_type src,  int sx, int sy, int wx, int wy,
                    img_type dest, int dx, int dy, int copy)
{
    int x, y;
    BYTE pix;

    if(copy)
    {
        for(x=0;x<wx;x++){
        for(y=0;y<wy;y++){
            pix = *( dib_ref_pixel(src, sx+x, sy+y) );
            *( dib_ref_pixel(dest, dx+x, dy+y) ) = pix;
        }}
    }
    else
    {
        for(x=0;x<wx;x++){
        for(y=0;y<wy;y++){
            pix = *( dib_ref_pixel(src, sx+x, sy+y) );
            if(pix!=pix_transparent)
                *( dib_ref_pixel(dest, dx+x, dy+y) ) = pix;
        }}
    }
}

// Copy internal image to another internal image
void ImgCopyH(img_type src,  int sx, int sy, int wx, int wy,
                    img_type dest, int dx, int dy, int copy)
{
    int x, y;
    BYTE pix;

    if(copy)
    {
        for(x=0;x<wx;x++){
        for(y=0;y<wy;y++){
            pix = *( dib_ref_pixel(src, sx+x, sy+y) );
            if (pix == pix_rimcolor) pix = pix_magenta;
            *( dib_ref_pixel(dest, dx+x, dy+y) ) = pix;
        }}
    }
    else
    {
        for(x=0;x<wx;x++){
        for(y=0;y<wy;y++){
            pix = *( dib_ref_pixel(src, sx+x, sy+y) );
            if (pix == pix_rimcolor) pix = pix_magenta;
            if(pix!=pix_transparent)    *( dib_ref_pixel(dest, dx+x, dy+y) ) = pix;
        }}
    }
}


// Copy internal image to another internal image
void ImgCopyMasked(img_type src,  int sx, int sy, int wx, int wy,
                    img_type dest, int dx, int dy, char *mask)
{
    int x, y;
    BYTE pix;
    int count = 0;
    for(y=0;y<wy;y++){
    for(x=0;x<wx;x++){
        pix = *( dib_ref_pixel(src, sx+x, sy+y) );
        if (mask[count]==0 && pix != pix_transparent)
            *( dib_ref_pixel(dest, dx+x, dy+y) ) = pix;
        count++;
    }}
}

// Copy internal image to another internal image
void ImgCopyMaskedH(img_type src,  int sx, int sy, int wx, int wy,
                    img_type dest, int dx, int dy, char *mask)
{
    int x, y;
    BYTE pix;
    int count = 0;
    for(y=0;y<wy;y++){
    for(x=0;x<wx;x++){
        pix = *( dib_ref_pixel(src, sx+x, sy+y) );
        if (pix == pix_rimcolor) pix = pix_magenta;
        if (mask[count]==0 && pix != pix_transparent)
            *( dib_ref_pixel(dest, dx+x, dy+y) ) = pix;
        count++;
    }}
}

void WinClass::fillrect(int left, int top, int right, int bottom, int color)
{
    HDC hdc = GetDC(hWnd);

    HBRUSH curbrush;
    HDC    curbrushhdc;
    RECT   currect;
    curbrush = CreateSolidBrush(pix_col[color]);
    currect.left  = left;
    currect.right = right;
    currect.top   = top;
    currect.bottom= bottom;
    SelectObject(curbrushhdc, curbrush);
    FillRect(hdc, &currect, curbrush);
    DeleteObject(curbrush);
    DeleteDC(curbrushhdc);

    ReleaseDC(hWnd, hdc);
}


void TileRegionClass::DrawPanel(int left, int top, int width, int height)
{
    framerect(left, top , left + width, top + height, PIX_WHITE);
    framerect(left + 1, top + 1, left + width, top + height, PIX_DARKGREY);
    fillrect (left + 1, top + 1, left + width - 1, top + height -1, PIX_LIGHTGREY);
}

void RegionClass::framerect(int left, int top, int right, int bottom, int color)
{
    HDC hdc = GetDC(win->hWnd);

    HBRUSH curbrush;
    HDC    curbrushhdc;
    RECT   currect;
    curbrush = CreateSolidBrush(pix_col[color]);
    currect.left  = left;
    currect.right = right;
    currect.top   = top;
    currect.bottom= bottom;
    SelectObject(curbrushhdc, curbrush);
    FrameRect(hdc, &currect, curbrush);
    DeleteObject(curbrush);
    DeleteDC(curbrushhdc);

    ReleaseDC(win->hWnd, hdc);
}

void TileRegionClass::framerect(int left, int top, int right, int bottom,
                    int color)
{
    HDC dhdc = backbuf->hDC;
    HBRUSH curbrush;
    HDC    curbrushhdc;
    RECT   currect;
    curbrush = CreateSolidBrush(pix_col[color]);
    currect.left  = left;
    currect.right = right;
    currect.top   = top;
    currect.bottom= bottom;
    SelectObject(curbrushhdc, curbrush);
    FrameRect(dhdc, &currect, curbrush);
    DeleteObject(curbrush);
    DeleteDC(curbrushhdc);
}

void RegionClass::fillrect(int left, int top, int right, int bottom, int color)
{
    HDC hdc = GetDC(win->hWnd);

    HBRUSH curbrush;
    HDC    curbrushhdc;
    RECT   currect;
    curbrush = CreateSolidBrush(pix_col[color]);
    currect.left  = left;
    currect.right = right;
    currect.top   = top;
    currect.bottom= bottom;
    SelectObject(curbrushhdc, curbrush);
    FillRect(hdc, &currect, curbrush);
    DeleteObject(curbrush);
    DeleteDC(curbrushhdc);

    ReleaseDC(win->hWnd, hdc);
}

void  TileRegionClass::fillrect(int left, int top, int right, int bottom,
                    int color)
{
    HDC dhdc = backbuf->hDC;
    HBRUSH curbrush;
    HDC    curbrushhdc;
    RECT   currect;
    curbrush = CreateSolidBrush(pix_col[color]);
    currect.left  = left;
    currect.right = right;
    currect.top   = top;
    currect.bottom= bottom;
    SelectObject(curbrushhdc, curbrush);
    FillRect(dhdc, &currect, curbrush);
    DeleteObject(curbrush);
    DeleteDC(curbrushhdc);
}

