#ifndef LIBX11_H
#define LIBX11_H


// Some replacement routines missing in gcc

#define _NORMALCURSOR 1
#define _NOCURSOR 0
#define O_BINARY O_RDWR

/** Same as in liblinux.cc **/
char *strlwr(char *str);
int itoa(int value, char *strptr, int radix);
int key_to_command(int);
int stricmp(const char *str1, const char *str2);
void delay(unsigned long time);
void init_key_to_command();

/** Output/Input to WIN_MAIN **/
int kbhit(void);
int window(int x1, int y1, int x2, int y2);
void update_screen(void);
int getch();

#define textcolor_cake(col) textcolor((col)<<4 | (col))

/** X11 specific routines **/
void libx11_init();
void libx11_shutdown();

#endif
