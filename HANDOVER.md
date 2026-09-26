# Linley's Dungeon Crawl 4.00b26: handover

See `~/Games/rvip-tools/RVIP.md` (O-Linley) for the port.

- Prompt line (RVIP step 5 / W4, 2026-09-26): the live message row is shown in a
  box over the map by `RvipWM.prompt` (rvip-wm.js). A key hides it only while
  the game waits for a command, so a question stays up until answered.
  Here: `rvip_at_cmd` (new global in `source/rvip.cc`, set around
  `getch_with_command_macros()` in `rvip_getkey()`), passed by `js_event(ev,
  at_cmd)`; `web_present()` in `source/libweb.cc` sends the message row the game
  writes to (`text_mode == region_msg`, `cursor_y`) via `js_prompt()`.
