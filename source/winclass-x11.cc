#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <X11/Xlib.h>
#include <X11/X.h>
#include <X11/Xutil.h>
#include <X11/Xlocale.h>
#include <X11/keysym.h>
#include <X11/keysymdef.h>
#include <X11/Xmd.h>

#ifdef FULLDEBUG
 #define DEBUG 1
 #define DEBUG_DIAGNOSTICS 1
 #define DEBUG_ITEM_SCAN 1
#else
 #define NDEBUG
#endif

#include "winclass.h"
#include "defines.h"
#include "debug.h"

// From libx11.cc
extern Display *display;
extern int screen;
#define MAX_PIX_COLOR 16
extern GC xgc[MAX_PIX_COLOR];
extern unsigned long pix_col[MAX_PIX_COLOR];
extern unsigned long pix_transparent;
extern unsigned long pix_hilite;
extern unsigned long pix_black;
extern unsigned long pix_rimcolor;
extern int x11_byte_per_pixel_ximage();

void WinClass::SysInit()
{}

void WinClass::SysDeinit()
{}

void RegionClass::SysInit()
{
    font = NULL;
}


void RegionClass::SysDeinit()
{
#ifdef JP
    if (font != NULL) XFreeFontSet(display, font);
#else
    if (font != NULL) XFreeFont(display, font);
#endif
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

void RegionClass::init_font(const char *name){
  /*** Large part of this routine was copied from Hengband ***/
#ifdef JP
    char **missing_list;
    int missing_count;

    XCharStruct *cs;
    XFontStruct **fontinfo;
    char **fontname;
    int n_fonts;
    char *default_font;
#endif
    int ascent, descent, width;

#ifdef JP
    font = XCreateFontSet(display, name, &missing_list,
                          &missing_count, &default_font);
    if(missing_count > 0){
        printf("missing font(s): \n");
        while(missing_count-- > 0){
            printf("\t%s\n", missing_list[missing_count]);
        }
        XFreeStringList(missing_list);
        exit(1);
    }

    n_fonts = XFontsOfFontSet(font, &fontinfo, &fontname);

    ascent = descent = width = 0;
    while(n_fonts-- > 0){
        cs = &((*fontinfo)->max_bounds);
        if(ascent < (*fontinfo)->ascent) ascent = (*fontinfo)->ascent;
        if(descent < (*fontinfo)->descent) descent = (*fontinfo)->descent;
        if(((*fontinfo)->max_byte1) > 0){
            /* 多バイト文字の場合は幅半分(端数切り上げ)で評価する */
            if(width < (cs->width+1)/2) width = (cs->width+1)/2;
            }else{
            if(width < cs->width) width = cs->width;
        }
        fontinfo++;
        fontname++;
    }
#else
    font = XLoadQueryFont(display, name);
    if (!font)
    {
        fprintf(stderr,"Error! Can't load font %s\n",name);
        exit(1);
    }
    width   = font->max_bounds.width;
    ascent  = font->ascent;
    descent = font->descent;
#endif

    fx = dx = width;
    fy = dy = ascent + descent;
    asc =  ascent;
}

void RegionClass::init_backbuf()
{

}

void TextRegionClass::init_backbuf()
{

}

void TileRegionClass::init_backbuf()
{
    int x, y;
    backbuf = ImgCreateSimple(mx*dx, my*dy);
    for (x = 0; x < mx*dx; x++)
    for (y = 0; y < my*dy; y++)
        XPutPixel(backbuf, x, y, pix_black);
}

void MapRegionClass::init_backbuf()
{
    int x, y;
    backbuf = ImgCreateSimple(mx*dx, my*dy);
    for (x = 0; x < mx*dx; x++)
    for (y = 0; y < my*dy; y++)
        XPutPixel(backbuf, x, y, pix_black);
}

void TextRegionClass::addstr_aux(char *buffer, int len){
    int i, head, tail;
    WinClass    *w = win;

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
        if (x + i == mx) break;
        cbuf[adrs+x+i]=buffer[i];
        abuf[adrs+x+i]=col;
    }

    fillrect(head*dx, y*dy, tail*dx, (y+1)*dy, col>>4);
//    XFillRectangle(display, w->win, xgc[col>>4],
//                     head*dx + ox, y*dy +oy, (tail-head)*dx, dy);

#ifdef JP
    XmbDrawString(display, w->win, font,
         xgc[col&0xf], head*dx +ox, y*dy+asc+ oy,
         (char *)&cbuf[adrs+head], tail-head );
#else
    XSetFont(display, xgc[col&0x0f], font->fid);
    XDrawString(display, w->win,
                xgc[col&0x0f], head*dx+ ox, y*dy+asc+ oy,
                (char *)&cbuf[adrs+head], tail-head );
#endif

    cursor_x += len;
}

void TextRegionClass::draw_cursor(int x, int y){
    int i = 0;
    int head = 0;
    int tail = 0;

    int cx = x - cx_ofs;
    int cy = y - cy_ofs;

    int adrs;

    if(!flag)return;

    adrs = cy * mx;
    head = 0;
    tail = mx;
    i = 0;

    while (i<mx){
        if(i<=cx) head=i;
        if(i>cx){tail=i;break;}
        if (cbuf[adrs+i] & 0x80) i+=2;
        else i++;
    }
    old_cursor_x = head + cx_ofs;
    old_cursor_y = y + cy_ofs;
    old_cursor_width = tail-head;
    old_cursor_region = this;
    framerect(head*dx, y*dy, tail*dx-1, (y+1)*dy-1, PIX_WHITE);
    XFlush(display);
}

void TextRegionClass::erase_cursor(){
    WinClass    *w = win;

    int x0 = old_cursor_x;
    int y0 = old_cursor_y;
    int width = old_cursor_width;
    int adrs = y0 * mx + x0;
    int col = abuf[adrs];

    if(!flag)return;

    XDrawRectangle(display, w->win, xgc[col>>4],
                      x0*dx + ox, y0*dy +oy, width*dx-1, dy-1);

#ifdef JP
    XmbDrawString(display, w->win, font,
         xgc[col&0xf], x0*dx +ox, y0*dy+asc+ oy,
         (char *)&cbuf[adrs], width);
#else
    XSetFont(display, xgc[col&0x0f], font->fid);
    XDrawString(display, w->win,
                xgc[col&0x0f], x0*dx+ ox, y0*dy+asc+ oy,
                (char *)&cbuf[adrs], width );
#endif

}


void WinClass::clear()
{
    fillrect(0, 0, wx-1, wy-1, PIX_BLACK);
    XFlush(display);
}

void RegionClass::clear()
{
    fillrect(0, 0, wx-1, wy-1, PIX_BLACK);
    XFlush(display);
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

void WinClass::create(char *name)
{
    win = XCreateSimpleWindow(display, RootWindow(display,screen),
        10,10, wx, wy, 0,
        BlackPixel(display,screen), BlackPixel(display,screen));

    XMapWindow(display, win);
    XSelectInput(display, win, ExposureMask | KeyPressMask
                  | ButtonPressMask | PointerMotionMask );

    XMoveWindow(display, win, ox, oy);
    clear();
}

void TileRegionClass::redraw(int x1, int y1, int x2, int y2)
{
    int wwx = x2-x1+1;
    int wwy = y2-y1+1;

    if (x1<0)
    {
        wwx += x1;
        x1 = 0;
    }

    if (y1<0)
    {
        wwy += y1;
        y1 = 0;
    }

    if (x2 >= mx)
    {
        wwx -= x2-mx+1;
    }

    if (y2 >= my)
    {
        wwy -= y2-my+1;
    }

    XPutImage(display, win->win, xgc[0],
              backbuf, x1*dx, y1*dy,
              x1*dx+ox, y1*dy+oy,
              wwx*dx, wwy*dy);
}

void TileRegionClass::redraw()
{
    redraw(0, 0, mx-1, my-1);
}

void MapRegionClass::redraw(int x1, int y1, int x2, int y2)
{
    if(!flag)return;

    XPutImage(display, win->win, xgc[0],
              backbuf, x1*dx, y1*dy,
              x1*dx+ox, y1*dy+oy,
              (x2-x1+1)*dx, (y2-y1+1)*dy);
}

void MapRegionClass::redraw()
{
    redraw(0,0, mx-1, my-1);
}

void MapRegionClass::draw_data(unsigned char *buf){
    int x, y, xx, yy;

    if(!flag)return;

    if (qv_mode)
    {
        for (x = 0; x < mx2; x++){
        for (y = 0; y < my2; y++){

            int x0 = (my2 - 1 + x - y) * dx;
            int y0 = (x + y) * dy;
            int col=buf[(x_margin + x) + (y_margin + y)*(mx2 + x_margin*2)];
            // Hack :remember player position
            if (col == PIX_WHITE)
            {
                px = x;
                py = y;
            }
            //if(col != get_col(x, y) || force_redraw)
            {
                for(xx=0; xx<dx; xx++)
                for(yy=0; yy<dy*2; yy++)
                XPutPixel(backbuf, x0+xx, y0+yy, pix_col[col]);

                set_col(col, x, y);
            }
        }}
    }
    else
    {
        for (x = 0; x < mx; x++){
        for (y = 0; y < my; y++){
            int col=buf[(x_margin + x) + (y_margin + y)*(mx2 + x_margin*2)];
            // Hack :remember player position
            if (col == PIX_WHITE)
            {
                px = x;
                py = y;
            }
            //if(col != get_col(x, y) || force_redraw)
            {
                for(xx=0; xx<dx; xx++)
                for(yy=0; yy<dy; yy++)
                XPutPixel(backbuf, x*dx+xx, y*dy+yy, pix_col[col]);

                set_col(col, x, y);
            }
        }}
    }
    redraw();
    XFlush(display);
    force_redraw = false;
}

void TextRegionClass::redraw(int x1, int y1, int x2, int y2){
    int head, tail;
    WinClass    *w = win;
    int x,y,col,oldcol;

    if(!flag)return;

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

        while (x<=tail){
            oldcol = col;
            col = abuf[adrs+x];
            if (oldcol != col || x==tail)
            {
                fillrect(head*dx, y*dy, x*dx, (y+1)*dy,oldcol>>4);
#ifdef JP
                XmbDrawString(display, w->win, font,
                    xgc[oldcol&0x0f], ox+head*dx, oy+y*dy+asc,
                    (char *)&cbuf[adrs+head], x-head );
#else
                XSetFont(display, xgc[oldcol&0x0f], font->fid);
                XDrawString(display, w->win,
                    xgc[oldcol&0x0f], ox+head*dx, oy+y*dy+asc,
                    (char *)&cbuf[adrs+head], x-head );
#endif
                head = x;
            }
            if (cbuf[adrs+x] & 0x80) x+=2;
            else x++;
        }
    }

    if(old_cursor_region == this && cursor_flag == 1)
    {
        draw_cursor(old_cursor_x, old_cursor_y);
    }
    XFlush(display);
}

/* XXXXX
 * img_type related
 */

bool ImgIsTransparentAt(img_type img, int x, int y)
{
    ASSERT(x>=0);
    ASSERT(y>=0);
    ASSERT(x<(img->width));
    ASSERT(y<(img->height));

    return (pix_transparent == XGetPixel(img, x, y)) ? true:false;
}

void ImgDestroy(img_type img)
{
    XDestroyImage(img);
}

img_type ImgCreateSimple(int wx, int wy)
{
    if (wx ==0 || wy == 0) return NULL;
    char *buf = (char *)malloc(x11_byte_per_pixel_ximage()* wx * wy);

    img_type res= XCreateImage(display, DefaultVisual(display, screen),
                                  DefaultDepth(display, screen),
                                  ZPixmap, 0, buf, wx, wy, 8, 0);
    return(res);
}

img_type ImgLoadFile(char *name)
{
    extern XImage *read_png(char *fname);
    return read_png(name);
}

void ImgClear(img_type img)
{
    int x,y;
    ASSERT(img != NULL);
    for(y=0;y<img->height;y++)
    for(x=0;x<img->width;x++)
        XPutPixel(img, x, y, pix_transparent);
}

// Copy internal image to another internal image
void ImgCopy(img_type src,  int sx, int sy, int wx, int wy,
             img_type dest, int dx, int dy, int copy)
{
    int x, y;
    int bpp = src->bytes_per_line / src->width;
    int bpl_s = src->bytes_per_line;
    int bpl_d = dest->bytes_per_line;

    ASSERT(sx>=0);
    ASSERT(sy>=0);
    ASSERT(sx+wx<=(src->width));
    ASSERT(sy+wy<=(src->height));
    ASSERT(dx>=0);
    ASSERT(dy>=0);
    ASSERT(dx+wx<=(dest->width));
    ASSERT(dy+wy<=(dest->height));

    if(copy==1)
    {
        char *p_src = (char *)(src->data + bpl_s * sy + sx * bpp);
        char *p_dest = (char *)(dest->data + bpl_d * dy + dx * bpp);

        for(y=0;y<wy;y++){
            memcpy(p_dest, p_src, wx * bpp);
            p_src += bpl_s;
            p_dest += bpl_d;
        }
    }
    else if(bpp<=1)
    {
        CARD8 *p_src = (CARD8 *)(src->data + bpl_s * sy + sx * bpp);
        CARD8 *p_dest = (CARD8 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] != pix_transparent)
                    p_dest[x] = p_src[x];
            }
            p_src += bpl_s;
            p_dest += bpl_d;
        }
    }
    else if(bpp<=2)
    {
        CARD16 *p_src = (CARD16 *)(src->data + bpl_s * sy + sx * bpp);
        CARD16 *p_dest = (CARD16 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] != pix_transparent)
                    p_dest[x] = p_src[x];
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }
    else if(bpp<=4)
    {
        CARD32 *p_src = (CARD32 *)(src->data + bpl_s * sy + sx * bpp);
        CARD32 *p_dest = (CARD32 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] != pix_transparent)
                    p_dest[x] = p_src[x];
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }

}

// Copy internal image to another internal image
void ImgCopyH(img_type src,  int sx, int sy, int wx, int wy,
              img_type dest, int dx, int dy, int copy)
{
    int x, y;
    int bpp = src->bytes_per_line / src->width;
    int bpl_s = src->bytes_per_line;
    int bpl_d = dest->bytes_per_line;

    ASSERT(sx>=0);
    ASSERT(sy>=0);
    ASSERT(sx+wx<=(src->width));
    ASSERT(sy+wy<=(src->height));
    ASSERT(dx>=0);
    ASSERT(dy>=0);
    ASSERT(dx+wx<=(dest->width));
    ASSERT(dy+wy<=(dest->height));

    if(copy==1)
    {
        char *p_src = (char *)(src->data + bpl_s * sy + sx * bpp);
        char *p_dest = (char *)(dest->data + bpl_d * dy + dx * bpp);

        for(y=0;y<wy;y++){
            memcpy(p_dest, p_src, wx * bpp);
            p_src += bpl_s;
            p_dest += bpl_d;
        }
    }
    else if(bpp<=1)
    {
        CARD8 *p_src = (CARD8 *)(src->data + bpl_s * sy + sx * bpp);
        CARD8 *p_dest = (CARD8 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] == pix_rimcolor)
                    p_dest[x] = pix_hilite;

                else if(p_src[x] != pix_transparent)
                    p_dest[x] = p_src[x];
            }
            p_src += bpl_s;
            p_dest += bpl_d;
        }
    }
    else if(bpp<=2)
    {
        CARD16 *p_src = (CARD16 *)(src->data + bpl_s * sy + sx * bpp);
        CARD16 *p_dest = (CARD16 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] == pix_rimcolor)
                    p_dest[x] = pix_hilite;

                else if(p_src[x] != pix_transparent)
                    p_dest[x] = p_src[x];
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }
    else if(bpp<=4)
    {
        CARD32 *p_src = (CARD32 *)(src->data + bpl_s * sy + sx * bpp);
        CARD32 *p_dest = (CARD32 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] == pix_rimcolor)
                    p_dest[x] = pix_hilite;

                else if(p_src[x] != pix_transparent)
                    p_dest[x] = p_src[x];
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }

}


// Copy internal image to another internal image
void ImgCopyMasked(img_type src,  int sx, int sy, int wx, int wy,
                   img_type dest, int dx, int dy, char *mask)
{
    int x, y, count;
    int bpp = src->bytes_per_line / src->width;
    int bpl_s = src->bytes_per_line;
    int bpl_d = dest->bytes_per_line;
    ASSERT(sx>=0);
    ASSERT(sy>=0);
    ASSERT(sx+wx<=(src->width));
    ASSERT(sy+wy<=(src->height));
    ASSERT(dx>=0);
    ASSERT(dy>=0);
    ASSERT(dx+wx<=(dest->width));
    ASSERT(dy+wy<=(dest->height));


    count = 0;

    if(bpp<=1)
    {
        CARD8 *p_src = (CARD8 *)(src->data + bpl_s * sy + sx * bpp);
        CARD8 *p_dest = (CARD8 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] != pix_transparent && mask[count]==0)
                    p_dest[x] = p_src[x];
        count++;
            }
            p_src += bpl_s;
            p_dest += bpl_d;
        }
    }
    else if(bpp<=2)
    {
        CARD16 *p_src = (CARD16 *)(src->data + bpl_s * sy + sx * bpp);
        CARD16 *p_dest = (CARD16 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] != pix_transparent && mask[count]==0)
                    p_dest[x] = p_src[x];
        count++;
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }
    else if(bpp<=4)
    {
        CARD32 *p_src = (CARD32 *)(src->data + bpl_s * sy + sx * bpp);
        CARD32 *p_dest = (CARD32 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] != pix_transparent && mask[count]==0)
                    p_dest[x] = p_src[x];
        count++;
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }

}

// Copy internal image to another internal image
void ImgCopyMaskedH(img_type src,  int sx, int sy, int wx, int wy,
                   img_type dest, int dx, int dy, char *mask)
{
    int x, y, count;
    int bpp = src->bytes_per_line / src->width;
    int bpl_s = src->bytes_per_line;
    int bpl_d = dest->bytes_per_line;
    ASSERT(sx>=0);
    ASSERT(sy>=0);
    ASSERT(sx+wx<=(src->width));
    ASSERT(sy+wy<=(src->height));
    ASSERT(dx>=0);
    ASSERT(dy>=0);
    ASSERT(dx+wx<=(dest->width));
    ASSERT(dy+wy<=(dest->height));


    count = 0;

    if(bpp<=1)
    {
        CARD8 *p_src = (CARD8 *)(src->data + bpl_s * sy + sx * bpp);
        CARD8 *p_dest = (CARD8 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] == pix_rimcolor)
                    p_dest[x] = pix_hilite;
                else if(p_src[x] != pix_transparent && mask[count]==0)
                    p_dest[x] = p_src[x];

        count++;
            }
            p_src += bpl_s;
            p_dest += bpl_d;
        }
    }
    else if(bpp<=2)
    {
        CARD16 *p_src = (CARD16 *)(src->data + bpl_s * sy + sx * bpp);
        CARD16 *p_dest = (CARD16 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] == pix_rimcolor)
                    p_dest[x] = pix_hilite;
                else if(p_src[x] != pix_transparent && mask[count]==0)
                    p_dest[x] = p_src[x];
        count++;
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }
    else if(bpp<=4)
    {
        CARD32 *p_src = (CARD32 *)(src->data + bpl_s * sy + sx * bpp);
        CARD32 *p_dest = (CARD32 *)(dest->data + bpl_d * dy + dx * bpp);
        for(y=0;y<wy;y++){
            for(x=0;x<wx;x++){
                //X11 specific
                if(p_src[x] == pix_rimcolor)
                    p_dest[x] = pix_hilite;
                else if(p_src[x] != pix_transparent && mask[count]==0)
                    p_dest[x] = p_src[x];
        count++;
            }
            p_src += bpl_s/bpp;
            p_dest += bpl_d/bpp;
        }
    }

}

void TileRegionClass::DrawPanel(int left, int top, int width, int height)
{
    framerect(left, top , left + width, top + height, PIX_WHITE);
    framerect(left + 1, top + 1, left + width, top + height, PIX_DARKGREY);
    fillrect (left + 1, top + 1, left + width - 1, top + height -1, PIX_LIGHTGREY);
}

void RegionClass::framerect(int left, int top, int right, int bottom,
                            int color)
{
    XDrawRectangle(display, win->win, xgc[color&0xf],
                   ox+left, oy+top, right-left, bottom-top);
}

void TileRegionClass::framerect(int left, int top, int right, int bottom,
                    int color)
{
    int x,y;
    int pix = pix_col[color];

    for (x=left; x<=right; x++){
        XPutPixel(backbuf, x, top, pix);
        XPutPixel(backbuf, x, bottom, pix);
    }

    for (y=top+1; y< bottom; y++){
        XPutPixel(backbuf, left, y, pix);
        XPutPixel(backbuf, right, y, pix);
    }
}

void WinClass::fillrect(int left, int top, int right, int bottom,
                            int color)
{
    XFillRectangle(display, win, xgc[color&0xf],
                   top, left, right-left+1, bottom-top+1);
}

void RegionClass::fillrect(int left, int top, int right, int bottom,
                            int color)
{
    XFillRectangle(display, win->win, xgc[color&0xf],
                   ox+left, oy+top, right-left, bottom-top);
}

void  TileRegionClass::fillrect(int left, int top, int right, int bottom,
                    int color)
{
    int x,y;
    int pix = pix_col[color];

    ASSERT(left>=0);
    ASSERT(top>=0);
    ASSERT(right<mx);
    ASSERT(bottom<my);

    for (x=left; x<=right; x++){
    for (y=top; y<= bottom; y++){
        XPutPixel(backbuf, x, y, pix);
    }}
}
