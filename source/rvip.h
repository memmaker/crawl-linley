#ifndef RVIP_H
#define RVIP_H

// X11 cursor keys arrive as 10000 + (keysym - 0xff00)
#define RVIP_KEY_UP     (10000 + 0x52)
#define RVIP_KEY_DOWN   (10000 + 0x54)
#define RVIP_KEY_KP_ADD (10000 + 0xab)
#define RVIP_KEY_KP_SUB (10000 + 0xad)
#define RVIP_KEY_KP_MUL (10000 + 0xaa)

// keypad / cursor directions while an RVIP list or menu is open
// (the frontend sends these instead of vi keys when rvip_raw_dirs > 0)
#define RVIP_KEY_DIR(d) (10100 + (d))
extern int rvip_raw_dirs;

// queued marker: reopen the inventory unless a monster is in view
#define RVIP_REOPEN     (-2)

int rvip_getkey();

#ifdef USE_WEB
// libweb.cc
void web_autosave();
extern "C" void web_sync_files();
void web_end(int code);
#endif
bool rvip_walk_stairs(int key);
void rvip_push_key(int key);
int rvip_item_list(int type_expect, bool browse, const char *prompt = NULL);

#endif
