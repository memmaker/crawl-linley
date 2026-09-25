#ifndef RVIP_H
#define RVIP_H

// X11 cursor keys arrive as 10000 + (keysym - 0xff00)
#define RVIP_KEY_UP     (10000 + 0x52)
#define RVIP_KEY_DOWN   (10000 + 0x54)
#define RVIP_KEY_KP_ADD (10000 + 0xab)
#define RVIP_KEY_KP_SUB (10000 + 0xad)
#define RVIP_KEY_KP_MUL (10000 + 0xaa)

// queued marker: reopen the inventory unless a monster is in view
#define RVIP_REOPEN     (-2)

int rvip_getkey();
bool rvip_walk_stairs(int key);
void rvip_push_key(int key);
int rvip_item_list(int type_expect, bool browse, const char *prompt = NULL);

#endif
