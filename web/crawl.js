/*
 * Linley's Dungeon Crawl in the browser: draws the game's regions
 * (source/libweb.cc calls Module.cr), keyboard and mouse input, tiling
 * windows, saves in IndexedDB. Layout code from ~/Games/zapm/web/zapm.js.
 * Loaded before crawl-core.js.
 */
(function () {
	'use strict';

	/* region roles (libweb.cc) and where they are drawn */
	var R_TILE = 0, R_MSG = 1, R_STAT = 2, R_MAP = 3, R_ITEM = 4, R_CRT = 5, R_ITEM2 = 6;
	var CANVAS = ['#t-map canvas', '#t-msg canvas', '#t-stat canvas', '#t-inv canvas.mini',
		'#t-items canvas', '#pop canvas.crt', '#pop canvas.items'];
	var TEXT_WIN = { 1: 'msg', 2: 'stat', 5: 'msg' };  /* text role -> window whose A-/A+ size it uses (lists follow the messages) */
	var WIN = ['map', 'msg', 'stat', 'inv', 'items', 'vis'], wm = null;
	var ROOT = '/crawl-linley', DIR = ROOT + '/save', LAYOUT_FILE = DIR + '/web-layout.json';
	var FONT = 'Web437_IBM_VGA_8x16';
	function face() { return L && L.face ? '"' + L.face + '", ' + FONT : FONT; }
	var GUT = 6, TITLE_H = 20, BORDER = 2;
	var ZOOM_STEPS = [1, 1.25, 1.5, 1.75, 2, 2.5, 3];  /* tiles, nearest-neighbour */
	/* libx11_init() colours */
	var PAL = ['#000', '#0000a0', '#64b946', '#00b4b4', '#a00000', '#ee5cee', '#a55b00', '#a2a2a2',
		'#525252', '#5266ff', '#52ff52', '#52ffff', '#ff5252', '#ff52ff', '#ffff52', '#fff'];
	/* X11 keysyms for libweb.cc's key handler */
	var KS = { Enter: 0xff0d, Escape: 0xff1b, Tab: 0xff09, Backspace: 0xff08, Delete: 0xffff,
		ArrowLeft: 0xff51, ArrowUp: 0xff52, ArrowRight: 0xff53, ArrowDown: 0xff54,
		Home: 0xff95, End: 0xff9c, PageUp: 0xff9a, PageDown: 0xff9b, Insert: 0xff9e };
	var KP = { NumpadEnter: 0xff8d, NumpadDecimal: 0xffae, NumpadAdd: 0xffab,
		NumpadSubtract: 0xffad, NumpadMultiply: 0xffaa, NumpadDivide: 0xffaf };

	var regs = [];             /* per role: state from the game + canvas */
	var events = [];
	var app, saveReq = false, layer = 0;
	var dpr = Math.max(1, Math.min(3, window.devicePixelRatio || 1));
	var L = null, rects = {};

	function $(id) { return document.getElementById(id); }
	function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }
	function fontPx(role) { if (!wm) makeWM(); return RvipWM.fontSize(TEXT_WIN[role]); }   /* WM-owned, default 16px (index.html) */

	/* ---------- drawing ---------- */

	function drawText(R) {
		var f = fontPx(R.role), cw = f / 2, ch = f;
		var w = R.w * cw, h = R.h * ch;
		if (R.cw !== cw || R.cv.width !== Math.round(w * dpr)) {
			R.cv.width = Math.round(w * dpr); R.cv.height = Math.round(h * dpr);
			R.cv.style.width = w + 'px'; R.cv.style.height = h + 'px';
			R.cw = cw; R.ch = ch;
		}
		var c = R.cv.getContext('2d'), H = Module.HEAPU8;
		c.setTransform(dpr, 0, 0, dpr, 0, 0);
		c.font = f + 'px ' + face();
		c.textBaseline = 'top';
		for (var y = 0; y < R.h; y++) {
			for (var x = 0; x < R.w; x++) {
				var i = y * R.w + x, k = H[R.a + i], at = H[R.b + i];
				c.fillStyle = PAL[at >> 4 & 15];
				c.fillRect(x * cw, y * ch, cw, ch);
				if (k > 32 && k < 127) {
					c.fillStyle = PAL[at & 15];
					c.fillText(String.fromCharCode(k), x * cw, y * ch);
				}
			}
		}
		if (R.role === R_CRT) cropPop(R);
	}

	/* the text layer shows only the cells in use (a box sized to its content) */
	function cropPop(R) {
		var H = Module.HEAPU8, x0 = R.w, y0 = R.h, x1 = -1, y1 = -1;
		for (var y = 0; y < R.h; y++)
			for (var x = 0; x < R.w; x++) {
				var i = y * R.w + x;
				if ((H[R.a + i] > 32 && H[R.a + i] < 127) || H[R.b + i] >> 4) {
					if (x < x0) x0 = x; if (x > x1) x1 = x;
					if (y < y0) y0 = y; if (y > y1) y1 = y;
				}
			}
		if (x1 < 0) { x0 = y0 = 0; x1 = y1 = 0; }
		var clip = R.cv.parentNode;
		clip.style.width = (x1 - x0 + 1) * R.cw + 'px';
		clip.style.height = (y1 - y0 + 1) * R.ch + 'px';
		R.cv.style.marginLeft = -x0 * R.cw + 'px';
		R.cv.style.marginTop = -y0 * R.ch + 'px';
		fitPop();
	}

	function drawImage(R) {
		if (R.cv.width !== R.w || R.cv.height !== R.h) { R.cv.width = R.w; R.cv.height = R.h; }
		var px = new Uint8ClampedArray(Module.HEAPU8.buffer, R.a, R.w * R.h * 4);
		R.cv.getContext('2d').putImageData(new ImageData(new Uint8ClampedArray(px), R.w, R.h), 0, 0);
		var z = R.role === R_TILE ? L.zoom : R.role === R_MAP ? L.mini : L.items;
		R.cv.style.width = R.w * z + 'px'; R.cv.style.height = R.h * z + 'px';
		R.z = z;
	}

	function drawCursor(R, cx, cy, cw) {
		var c = R.cv.getContext('2d');
		c.fillStyle = PAL[15];
		c.fillRect(cx * R.cw, (cy + 1) * R.ch - 2, Math.max(1, cw) * R.cw, 2);
	}

	/* ---------- called by the game (source/libweb.cc) ---------- */
	var lastCur = -1;
	var cr = {
		region: function (role, lay, flag, dirty, text, ox, oy, dx, dy, w, h, a, b) {
			var R = regs[role];
			if (!R) {
				R = regs[role] = { role: role, cv: document.querySelector(CANVAS[role]), force: true };
				R.cv.addEventListener('mousedown', function (e) { mouse(R, e, 2); });
				R.cv.addEventListener('mousemove', function (e) { mouse(R, e, 3); });
				R.cv.addEventListener('wheel', function (e) { mouse(R, e, 4); }, { passive: false });
				R.cv.addEventListener('contextmenu', function (e) { e.preventDefault(); });
			}
			R.layer = lay; R.ox = ox; R.oy = oy; R.dx = dx; R.dy = dy; R.text = text;
			var changed = R.w !== w || R.h !== h || R.a !== a;
			R.w = w; R.h = h; R.a = a; R.b = b;
			if (R.flag !== flag) { R.flag = flag; R.cv.style.display = flag ? '' : 'none'; if (lay === 1) fitPop(); }
			if (flag && (dirty || changed || R.force)) {
				R.force = false;
				if (text) drawText(R); else drawImage(R);
				R.drawn = true;
			}
		},
		present: function (lay, curRole, cx, cy, cw) {
			if ($('game').hidden) {
				$('game').hidden = false;
				if (L.auto) L.zoom = defaultLayout().zoom;
				redrawAll();
			}
			if (lay !== layer) { layer = lay; $('pop').hidden = layer !== 1; fitPop(); }
			/* the cursor is drawn over the text: redraw that region to move it */
			if (lastCur >= 0 && regs[lastCur] && lastCur !== curRole) drawText(regs[lastCur]);
			if (curRole >= 0 && regs[curRole]) { drawText(regs[curRole]); drawCursor(regs[curRole], cx, cy, cw); }
			lastCur = curRole;
		},
		vis: function (s) { RvipWM.visible($('vis'), s.replace(/\t(\d+)\t/gm, function (m, c) { return '\t' + PAL[+c || 7] + '\t'; }), visIcon); },
		event: function (atCmd) { RvipWM.prompt.wait(atCmd); return events.length ? events.shift() : null; },
		prompt: function (s) { RvipWM.prompt.text(s); },
		pending: function () { return events.length > 0 ? 1 : 0; },
		sync: function () { app.sync(); },
		requestSave: function () { saveReq = true; },   /* also for testing */
		wantSave: function () {
			if (!saveReq || !app.running) return 0;
			saveReq = false;
			return 1;
		},
		end: function () {
			app.running = false;
			var saved = !!newestSave();
			app.sync(function () {
				$('overlay-msg').textContent = saved ? 'Your game has been saved. Play again to continue it.'
					: 'The game is over.';
				$('overlay').hidden = false;
			});
		}
	};

	/* ---------- input ---------- */
	function mouse(R, e, type) {
		if (!app.running) return;
		var box = R.cv.getBoundingClientRect();
		var px = e.clientX - box.left, py = e.clientY - box.top, x, y;
		if (R.text) {
			x = R.ox + Math.floor(px / (box.width / R.w)) * R.dx + (R.dx >> 1);
			y = R.oy + Math.floor(py / (box.height / R.h)) * R.dy + (R.dy >> 1);
		} else {
			x = R.ox + Math.floor(px * R.w / box.width);
			y = R.oy + Math.floor(py * R.h / box.height);
		}
		var mods = (e.shiftKey ? 1 : 0) | (e.ctrlKey ? 2 : 0);
		if (type === 4) {
			events.push([2, x, y, e.deltaY < 0 ? 4 : 5, mods]);
			e.preventDefault();
		} else if (type === 2) {
			events.push([2, x, y, e.button === 2 ? 2 : e.button === 1 ? 3 : 1, mods]);
			e.preventDefault();
		} else if (!events.length || events[events.length - 1][0] !== 3) {
			events.push([3, x, y, 0, 0]);
		} else {
			events[events.length - 1] = [3, x, y, 0, 0];
		}
	}

	function onKey(e) {
		var t = e.target;
		if (t && (t.tagName === 'INPUT' || t.tagName === 'TEXTAREA' || t.isContentEditable)) return;
		if (!app.running || e.isComposing || e.metaKey) return;
		var k = e.key, code = e.code || '', m = /^Numpad(\d)$/.exec(code), ks = 0, ch = 0;
		var mods = (e.shiftKey ? 1 : 0) | (e.ctrlKey ? 2 : 0) | (e.altKey ? 4 : 0);
		if (m) ks = 0xffb0 + +m[1];
		else if (KP[code]) ks = KP[code];
		else if (/^F([1-9]|1[0-2])$/.test(k)) ks = 0xffbe + +k.slice(1) - 1;
		else if (KS[k]) ks = KS[k];
		else if (k.length === 1) {
			ch = k.charCodeAt(0);
			if (ch > 255) return;
			if (e.ctrlKey && !e.altKey) {
				var u = k.toUpperCase().charCodeAt(0);
				if (u >= 64 && u <= 95) { ks = k.toLowerCase().charCodeAt(0); ch = u & 0x1f; }
			}
			mods &= ~1;              /* the character already carries Shift */
		}
		else return;
		events.push([1, ks, ch, mods, 0]);
		e.preventDefault();
	}

	/* ---------- tiling layout ---------- */
	/*
	 *   +--------------------+--------+   side:   x of left part | right column
	 *   |                    | stats  |   bottom: y of map | messages (left part)
	 *   |     tile view      +--------+   stat:   y of stats | minimap
	 *   |                    |minimap |   items:  y of minimap | inventory
	 *   +--------------------+--------+
	 *   |      messages      | items  |
	 *   +--------------------+--------+
	 */
	var ITEMS_H = 8 * 32 + TITLE_H + BORDER;    /* the 8 x 8 item grid (libweb.cc) */

	function areaSize() {
		var g = $('game');
		return { w: g.clientWidth, h: g.clientHeight };
	}

	function defaultLayout() {
		var A = areaSize(), W = A.w, H = A.h;
		if (W < 400 || H < 300) { W = 1280; H = 720; }
		var font = 16, msgH = 9 * font + TITLE_H + BORDER, sideW = 40 * font / 2 + BORDER;
		var zoom = ZOOM_STEPS[0];
		ZOOM_STEPS.forEach(function (z) { if (544 * z + BORDER <= W - sideW - GUT && 544 * z + BORDER <= H - msgH - GUT) zoom = z; });
		var mapW = 544 * zoom + BORDER, mapH = 544 * zoom + BORDER;
		return { v: 1, zoom: zoom, mini: 2, items: 1, auto: true,
			split: { side: clamp((mapW + GUT / 2) / W, 0.3, 0.85), bottom: clamp((mapH + GUT / 2) / H, 0.3, 0.9),
				stat: clamp((18 * font + TITLE_H + BORDER + GUT / 2) / H, 0.2, 0.8),
				items: clamp(1 - (ITEMS_H + GUT / 2) / H, 0.3, 0.95) } };
	}

	function loadLayout() {
		var d = defaultLayout();
		try {
			var s = JSON.parse(Module.FS.readFile(LAYOUT_FILE, { encoding: 'utf8' }));
			if (s && s.v === 1) {
				if (!s.auto) {
					d.auto = false;
					if (ZOOM_STEPS.indexOf(s.zoom) >= 0) d.zoom = s.zoom;
				}
				if (s.wm) d.wm = s.wm;
				if (s.font && d.wm && !d.wm.fs) d.wm.fs = { msg: s.font.msg, stat: s.font.stat, vis: s.font.vis };   /* old layout: sizes were ours */
				if (typeof s.face === 'string') d.face = s.face;
			}
		} catch (err) { /* nothing saved yet */ }
		L = d;
	}

	var saveTimer = 0;
	function saveLayout() {
		clearTimeout(saveTimer);
		saveTimer = setTimeout(function () {
			try { Module.FS.writeFile(LAYOUT_FILE, JSON.stringify(L)); app.sync(); }
			catch (err) { console.warn('layout not saved', err); }
		}, 400);
	}

	function place(el, r) {
		el.style.left = r[0] + 'px'; el.style.top = r[1] + 'px';
		el.style.width = Math.max(0, r[2]) + 'px'; el.style.height = Math.max(0, r[3]) + 'px';
	}

	/* the text layer (menus, lists, help) as a box centred on the map body
	   (never over its title bar); it scrolls when larger */
	function fitPop() {
		var pop = $('pop');
		if (!pop.hidden) RvipWM.popup(pop, { center: true });
	}

	function applyDom() { if (!wm) makeWM(); wm.apply(); }
	/* windows: the shared tiling window manager (rvip-wm.js, RVIP.md 5b);
	 * the game's regions draw into them, Inventory is its 8 x 8 item grid */
	function makeWM() {
		var s = defaultLayout().split, A = areaSize(), line = 20;
		wm = RvipWM({
			area: $('game'), menu: $('btn-layout'),
			wins: [{ id: 'map', title: 'Map' }, { id: 'msg', title: 'Messages' }, { id: 'stat', title: 'Character' },
				{ id: 'items', title: 'Inventory' }, { id: 'vis', title: 'Visible' }, { id: 'inv', title: 'Level map' }],
			multi: { d: 'h', r: s.side, a: { d: 'v', r: s.bottom, a: 'map', b: 'msg' },
				b: { d: 'v', r: s.stat, a: 'stat', b: { d: 'v', r: (ITEMS_H + GUT / 2) / (A.h * (1 - s.stat)), a: 'items', b: 'vis' } } },
			single: { d: 'v', r: 1 - 3 * line / A.h, a: 'map', b: 'msg' },
			state: L.wm,
			save: function (st) { L.wm = st; saveLayout(); },
			layout: function (r) { rects = r; $('vis').style.fontFamily = L.face ? '"' + L.face + '", monospace' : ''; fitPop(); },
			zoom: {   /* text windows redraw their canvas at the new size; image windows step their own scale */
				map: function (px, d) { zoomMap(d); },
				msg: redrawAll, stat: redrawAll,
				items: function (px, d) { L.items = clamp(L.items + d * 0.5, 0.5, 4); redrawAll(); saveLayout(); },
				inv: function (px, d) { L.mini = clamp(L.mini + d * 0.5, 0.5, 4); redrawAll(); saveLayout(); }
			},
			onReset: resetLayout
		});
	}

	function redrawAll() {
		regs.forEach(function (R) { if (R && R.w) { if (R.text) drawText(R); else drawImage(R); } });
		applyDom();
	}

	function zoomMap(d) {
		L.zoom = ZOOM_STEPS[clamp(ZOOM_STEPS.indexOf(L.zoom) + d, 0, ZOOM_STEPS.length - 1)];
		L.auto = false;
		redrawAll(); saveLayout();
		app.status('Map tiles: ' + Math.round(32 * L.zoom) + ' px');
		setTimeout(function () { app.status(''); }, 1200);
	}

	function resetLayout() {
		var fc = L.face; L = defaultLayout(); L.face = fc; L.wm = wm.state();
		redrawAll(); saveLayout();
	}

	/* ---------- saves: IndexedDB (IDBFS) ---------- */
	function saveFiles() {
		return Module.FS.readdir(DIR).filter(function (f) { return f !== '.' && f !== '..' && f !== 'web-layout.json'; });
	}
	/* the character whose .sav was written last (saves are <name>0.sav: uid 0) */
	function newestSave() {
		var best = null, t = -1;
		saveFiles().forEach(function (f) {
			var m = /^(.*)0\.sav$/.exec(f);
			if (!m) return;
			var mt = Module.FS.stat(DIR + '/' + f).mtime.getTime();
			if (mt > t) { t = mt; best = m[1]; }
		});
		return best;
	}
	function clearSaves() { saveFiles().forEach(function (f) { Module.FS.unlink(DIR + '/' + f); }); }
	/* Export save: every save file as one JSON pack, written to /tmp for RvipApp to download */
	function packFile() {
		if (!newestSave()) return null;
		var pack = {};
		saveFiles().forEach(function (f) {
			var d = Module.FS.readFile(DIR + '/' + f), s = '';
			for (var i = 0; i < d.length; i++) s += String.fromCharCode(d[i]);
			pack[f] = btoa(s);
		});
		Module.FS.writeFile('/tmp/crawl-save.json', JSON.stringify({ crawl: 1, files: pack }));
		return '/tmp/crawl-save.json';
	}
	function putPack(file, data) {
		var pack;
		try { pack = JSON.parse(new TextDecoder().decode(data)); if (!pack.crawl) throw 0; }
		catch (e) { return 'That is not a save exported from this page.'; }
		Object.keys(pack.files).forEach(function (f) {
			if (/[\/]/.test(f)) return;
			var s = atob(pack.files[f]), d = new Uint8Array(s.length);
			for (var i = 0; i < s.length; i++) d[i] = s.charCodeAt(i);
			Module.FS.writeFile(DIR + '/' + f, d);
		});
	}

	/* Visible window icon: the game's tile (tiles/tile.png, 30 x 32 px per row) as a 16 px CSS sprite */
	function visIcon(t) {
		if (!(t >= 0)) return null;
		var e = document.createElement('i');
		e.className = 'wm-ic';
		e.style.cssText = 'background:url(tile.png) -' + (t % 30) * 16 + 'px -' + ((t / 30) | 0) * 16 + 'px / 480px auto';
		return e;
	}

	function loadFace(n) {
		if (!n) { redrawAll(); return; }
		var ff = new FontFace(n, 'url(../fonts/' + n + '.woff)');
		ff.load().then(function () { document.fonts.add(ff); if (!$('game').hidden) redrawAll(); }).catch(function () { app.status('Could not load the font ' + n + '.', true); });
	}

	app = RvipApp({ name: 'crawl', save: packFile, clear: clearSaves, put: putPack,
		flush: function (done) { saveReq = true; setTimeout(done, 1500); } });   /* the game saves at its next wantSave() poll */
	window.Module = {
		cr: cr,
		arguments: [],
		preRun: [function () {
			var FS = Module.FS;
			FS.mkdirTree(DIR);
			FS.mount(Module.IDBFS, {}, DIR);
			FS.chdir(ROOT);                      /* tiles/, init.txt, save/ are relative */
			Module.ENV.CRAWL_RC = 'init.txt';
			Module.ENV.CRAWL_DIR = 'save/';
			Module.addRunDependency('idbfs');
			FS.syncfs(true, function (err) {
				if (err) app.status('Could not read saved games from IndexedDB (' + err + '). Saving may not work in this browser mode.', true);
				var name = newestSave();
				if (name) Module.arguments.push('-name', name);   /* continue that character */
				loadLayout();
				if (L.face) loadFace(L.face);
				Module.removeRunDependency('idbfs');
			});
		}],
		onRuntimeInitialized: function () {
			app.running = true;
			app.status('');
		},
		print: function (s) { console.log(s); },
		printErr: function (s) { console.warn(s); },
		setStatus: function (s) { if (s && !app.running) app.status(s.replace(/\(\d+\/\d+\)/, '').trim() || 'Loading…'); },
		onAbort: function (what) { app.crashed(what); }
	};

	/* autosave: every 2 minutes and when the page is hidden */
	setInterval(function () { saveReq = true; }, 120000);
	document.addEventListener('visibilitychange', function () { if (document.hidden) { saveReq = true; app.sync(); } });
	window.addEventListener('pagehide', function () { app.sync(); });
	window.addEventListener('beforeunload', function (e) { if (app.running) { e.preventDefault(); e.returnValue = ''; } });

	document.addEventListener('keydown', onKey);
	document.addEventListener('DOMContentLoaded', function () {
		/* the canvases draw with the VGA font: fetch it now, redraw once it is there
		   (drawn before it loads, text falls back to a serif face until the next change) */
		document.fonts.load('16px ' + FONT).then(function () { if (!$('game').hidden) redrawAll(); });
		RvipWM.dropdown($('btn-file'), $('menu-file'));
		/* text font: a face from the index page's fonts/ (web/build.sh lists them) */
		var sel = $('sel-font');
		fetch('fonts.json').then(function (r) { return r.json(); }).then(function (list) {
			list.forEach(function (n) { var o = document.createElement('option'); o.value = n; o.textContent = n.replace(/^Web(Plus|437)_/, '').replace(/_/g, ' '); sel.appendChild(o); });
			sel.value = (L && L.face) || '';
		}).catch(function () { });
		sel.onchange = function () { if (!L) return; L.face = this.value; saveLayout(); loadFace(this.value); this.blur(); };
		$('btn-restart').onclick = function () { location.reload(); };
		document.querySelectorAll('button').forEach(function (b) {
			b.addEventListener('mousedown', function (e) { e.preventDefault(); });
		});
	});
	var resizeTimer = 0;
	window.addEventListener('resize', function () {
		if (!L) return;
		clearTimeout(resizeTimer);
		resizeTimer = setTimeout(function () {
			if (L.auto) {                        /* not customised: follow the window */
				L.zoom = defaultLayout().zoom;
			}
			redrawAll();
		}, 150);
	});
})();
