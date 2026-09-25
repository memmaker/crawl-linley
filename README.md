# Linley's Dungeon Crawl 4.00b26 (tile version) for macOS

Upstream: Linley's Dungeon Crawl 4.00 beta 26 (Linley Henzell and the Crawl
DevTeam, `dc400b26-src.zip`, commit
[d5dff29](https://github.com/memmaker/crawl-linley/commit/d5dff29)) plus
Mitsuhiro Itakura's Dungeon Crawl Tile Version e070 (2005-12-24, commit
[fd103ab](https://github.com/memmaker/crawl-linley/commit/fd103ab)), both from
http://crawlj.sourceforge.jp/down_e.html via the Wayback Machine.

Our changes:
https://github.com/memmaker/crawl-linley/compare/fd103ab...main

- `port:` builds on macOS / Apple Silicon with the tile version's own X11
  frontend (XQuartz), libpng 1.6, 64-bit and AddressSanitizer fixes, saves
  in `save/`.
- `RVIP:` auto-explore stops on any message (Ctrl-O, keypad 0), `<` / `>`
  walk to the nearest known stairs, Enter opens a menu of all commands,
  the inventory and every item prompt use a list with a cursor and item
  menus (`source/rvip.cc`).

Build and play:

    cd source && make -f makefile.x11 && rm -f *.o
    ../play.sh

Tiles: RLTiles (http://rltiles.sourceforge.net/), public domain.
Licence of the game: `licence.txt`.
