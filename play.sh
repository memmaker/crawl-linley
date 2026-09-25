#!/bin/sh
# Linley's Dungeon Crawl 4.00b26 with Itakura's tile version, X11 (XQuartz).
# One window: tile view, stats, minimap, messages. Options in init.txt;
# saves, character dumps and macros in save/.
cd "$(dirname "$0")" || exit 1
mkdir -p save
export CRAWL_RC=init.txt CRAWL_DIR=save/
exec ./source/crawl "$@"
