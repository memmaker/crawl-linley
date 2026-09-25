// Copied from libw32c.h and modified

#define WIN_NUMBER_OF_LINES     25
#include <string>

#include <stdlib.h>
#include <excpt.h>
#include <stdarg.h>

#define _NORMALCURSOR   true
#define _NOCURSOR       false

void init_libwin(void);
void deinit_libwin(void);

void print_timings(void);
void window(int x, int y, int lx, int ly);

int getch();
int getche(void);
int kbhit(void);
void delay(int ms);
#define textcolor_cake(col) textcolor((col)<<4 | (col))

void get_input_line_from_win(char *const buf, int len );

inline void srandom(unsigned int seed) { srand(seed); }
inline int random() { return rand(); }

#ifdef USE_TILE

void mpr_off();
#else
void _setcursortype(int curstype);
void gotoxy(int x, int y);
void textcolor(int c);
void cprintf( const char *format, ... );
//void setStringInput(bool value);
//bool setBuffering(bool value);
//int getConsoleString(char *buf, int maxlen);

int wherex(void);
int wherey(void);
void putch(char c);
void textbackground(int c);
void get_input_line_from_win(char *const buf, int len );
#ifdef JP
void writeWChar(unsigned char *ch);
#endif
#endif

void change_font();
