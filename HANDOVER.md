# Linley's Dungeon Crawl 4.00b26: handover

See `~/Games/rvip-tools/RVIP.md` (part 3, case O; 5.5) for the port.

- Prompt line (`RvipWM.prompt`, RVIP 5.9): `rvip_at_cmd` (new global in `source/rvip.cc`, set around
  `getch_with_command_macros()` in `rvip_getkey()`), passed by `js_event(ev,
  at_cmd)`; `web_present()` in `source/libweb.cc` sends the message row the game
  writes to (`text_mode == region_msg`, `cursor_y`) via `js_prompt()`.
