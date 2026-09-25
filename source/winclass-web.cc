#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

// web: the platform half of Itakura's window classes (winclass-x11.cc with
// the drawing replaced). Images are RGBA buffers in memory; text and image
// regions are marked dirty and web/crawl.js draws them from memory.
typedef unsigned char CARD8;
typedef unsigned short CARD16;
typedef unsigned int CARD32;
#define XPutPixel(img, x, y, p) (((CARD32 *) (img)->data)[(y) * (img)->width + (x)] = (p))
#define XGetPixel(img, x, y)    (((CARD32 *) (img)->data)[(y) * (img)->width + (x)])

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

// From libweb.cc
#define MAX_PIX_COLOR 16
extern void web_dirty(RegionClass *r);
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
}


void RegionClass::SysDeinit()
{

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
    // web: the page draws text with the IBM VGA 8x16 font
    fx = dx = 8;
    fy = dy = 16;
    asc = 12;
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

    web_dirty(this);

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
    old_cursor_y = y;           // web: absolute, like old_cursor_x
    old_cursor_width = tail-head;
    old_cursor_region = this;
    web_dirty(this);
}

void TextRegionClass::erase_cursor(){
    WinClass    *w = win;

    int x0 = old_cursor_x;
    int y0 = old_cursor_y;
    int width = old_cursor_width;
    int adrs = y0 * mx + x0;
    int col = abuf[adrs];

    if(!flag)return;

    web_dirty(this);
}


void WinClass::clear()
{
    for (unsigned i = 0; i < regions.size(); i++)
        web_dirty(regions[i]);
}

void RegionClass::clear()
{
    if (backbuf)
    {
        CARD32 *p = (CARD32 *) backbuf->data;
        for (int i = 0; i < backbuf->width * backbuf->height; i++)
            p[i] = pix_black;
    }
    web_dirty(this);
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
    // web: the page lays out one window per region
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

    web_dirty(this);
}

void TileRegionClass::redraw()
{
    redraw(0, 0, mx-1, my-1);
}

void MapRegionClass::redraw(int x1, int y1, int x2, int y2)
{
    if(!flag)return;
    web_dirty(this);
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
    force_redraw = false;
}

void TextRegionClass::redraw(int x1, int y1, int x2, int y2){
    if(!flag)return;
    web_dirty(this);
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
    free(img->data);
    free(img);
}

img_type ImgCreateSimple(int wx, int wy)
{
    if (wx ==0 || wy == 0) return NULL;
    img_type res = (img_type) malloc(sizeof(*res));
    res->width = wx;
    res->height = wy;
    res->bytes_per_line = wx * 4;
    res->data = (char *) calloc(wx * wy, 4);
    return(res);
}

extern unsigned long create_pixel(unsigned int red, unsigned int green, unsigned int blue);

// libpng 1.6: the sheets are 8-bit paletted (same as libx11_png.cc)
img_type ImgLoadFile(char *name)
{
    FILE *ifp = fopen(name, "rb");
    if (!ifp) return NULL;

    png_structp png_ptr = png_create_read_struct (PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info_ptr = png_create_info_struct (png_ptr);
    if (setjmp (png_jmpbuf (png_ptr)))
    {
        fclose(ifp);
        return NULL;
    }
    png_init_io (png_ptr, ifp);
    png_read_info (png_ptr, info_ptr);
    if (png_get_bit_depth (png_ptr, info_ptr) < 8)
        png_set_packing (png_ptr);

    unsigned int w = png_get_image_width (png_ptr, info_ptr);
    unsigned int h = png_get_image_height (png_ptr, info_ptr);
    unsigned long pix_table[256];
    unsigned int i;

    if (png_get_color_type (png_ptr, info_ptr) == PNG_COLOR_TYPE_PALETTE)
    {
        png_colorp pal;
        int npal;
        png_get_PLTE (png_ptr, info_ptr, &pal, &npal);
        for (i = 0; i < (unsigned int) npal; i++)
            pix_table[i] = create_pixel(pal[i].red, pal[i].green, pal[i].blue);
    }
    else
        for (i = 0; i < 256; i++)
            pix_table[i] = create_pixel(i, i, i);

    png_bytep *rows = (png_bytep *) malloc (h * sizeof (png_bytep));
    for (i = 0; i < h; i++)
        rows[i] = (png_bytep) malloc (png_get_rowbytes (png_ptr, info_ptr));
    png_read_image (png_ptr, rows);
    png_read_end (png_ptr, info_ptr);

    img_type res = ImgCreateSimple(w, h);
    for (unsigned int y = 0; y < h; y++)
    {
        for (unsigned int x = 0; x < w; x++)
            XPutPixel(res, x, y, pix_table[rows[y][x]]);
        free (rows[y]);
    }
    free (rows);
    png_destroy_read_struct (&png_ptr, &info_ptr, NULL);
    fclose(ifp);
    return res;
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
    // web: only the text cursor used this; the page draws it
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
    clear();
}

void RegionClass::fillrect(int left, int top, int right, int bottom,
                            int color)
{
    // web: text regions keep their colours in abuf; images are cleared
    // by clear()
    web_dirty(this);
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
