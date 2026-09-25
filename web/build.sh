#!/bin/sh
# Build Linley's Dungeon Crawl for the browser (Emscripten + Asyncify) into
# web/dist. source/libweb.cc + winclass-web.cc hand the regions to
# web/crawl.js, which draws them. Deploy with web/deploy.sh.
set -e
cd "$(dirname "$0")/.."
OUT=web/dist STAGE=web/stage
rm -rf "$OUT" "$STAGE" && mkdir -p "$OUT" "$STAGE/save"
cp -R tiles tips_e init.txt dolls.txt "$STAGE/"
# the objects of makefile.obj, with the web platform files
SRCS=$(tr -d '\r' < source/makefile.obj | grep -o '[A-Za-z0-9_-]*\.o' | sed 's/\.o$/.cc/' | sed 's|^|source/|')
em++ -O2 -std=gnu++98 -w -Isource -DLINUX -DV_FIX -DUSE_TILE -DUSE_X11 -DUSE_WEB \
	$SRCS source/rvip.cc source/tiles.cc source/libtile.cc source/winclass.cc \
	source/libweb.cc source/winclass-web.cc -o "$OUT/crawl-core.js" \
	-sUSE_LIBPNG=1 -sASYNCIFY -sASYNCIFY_STACK_SIZE=131072 -sSTACK_SIZE=1048576 \
	-sALLOW_MEMORY_GROWTH -sINITIAL_MEMORY=64MB \
	-sEXPORTED_FUNCTIONS=_main \
	-sEXPORTED_RUNTIME_METHODS=FS,IDBFS,ENV,HEAPU8,addRunDependency,removeRunDependency \
	-sFORCE_FILESYSTEM -lidbfs.js -sENVIRONMENT=web \
	--preload-file "$STAGE@/crawl-linley"
rm -rf "$STAGE"
cp web/index.html web/crawl.js "$HOME/Games/rvip-tools/web/rvip-wm.js" web/*.woff "$OUT/"
python3 web/make-help.py > "$OUT/help.html"
ls -la "$OUT"
