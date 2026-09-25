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
		'#t-inv canvas.items', '#pop canvas.crt', '#pop canvas.items'];
	var TEXT_WIN = { 1: 'msg', 2: 'stat', 5: 'pop' };  /* text role -> font setting */
	var WIN = ['map', 'msg', 'stat', 'inv'];
	var ROOT = '/crawl-linley', DIR = ROOT + '/save', LAYOUT_FILE = DIR + '/web-layout.json';
	var FONT = 'Web437_IBM_VGA_8x16';
	var GUT = 6, TITLE_H = 20, BORDER = 2;
	var ZOOM_STEPS = [1, 1.25, 1.5, 1.75, 2, 2.5, 3];  /* tiles, nearest-neighbour */
	var FONT_STEPS = [12, 14, 16, 20, 24, 28, 32];     /* crisp at 16 and 32 */
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
	var running = false, saveReq = false, layer = 0;
	var dpr = Math.max(1, Math.min(3, window.devicePixelRatio || 1));
	var L = null, rects = {};

	function $(id) { return document.getElementById(id); }
	function clamp(v, lo, hi) { return Math.max(lo, Math.min(hi, v)); }
	function status(msg, isError) {
		var s = $('status');
		s.textContent = msg;
		s.className = isError ? 'error' : '';
		s.hidden = !msg;
	}
	function fontPx(role) { return role === R_CRT ? L.font.pop : L.font[TEXT_WIN[role]]; }

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
		c.font = f + 'px ' + FONT;
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
				if (L.auto) { var d = defaultLayout(); L.split = d.split; L.zoom = d.zoom; }
				redrawAll();
			}
			if (lay !== layer) { layer = lay; $('pop').hidden = layer !== 1; fitPop(); }
			/* the cursor is drawn over the text: redraw that region to move it */
			if (lastCur >= 0 && regs[lastCur] && lastCur !== curRole) drawText(regs[lastCur]);
			if (curRole >= 0 && regs[curRole]) { drawText(regs[curRole]); drawCursor(regs[curRole], cx, cy, cw); }
			lastCur = curRole;
		},
		event: function () { return events.length ? events.shift() : null; },
		pending: function () { return events.length > 0 ? 1 : 0; },
		sync: function () { syncFiles(); },
		requestSave: function () { saveReq = true; },   /* also for testing */
		wantSave: function () {
			if (!saveReq || !running) return 0;
			saveReq = false;
			return 1;
		},
		end: function () {
			running = false;
			var saved = !!newestSave();
			syncFiles(function () {
				$('overlay-msg').textContent = saved ? 'Your game has been saved. Play again to continue it.'
					: 'The game is over.';
				$('overlay').hidden = false;
			});
		}
	};

	/* ---------- input ---------- */
	function mouse(R, e, type) {
		if (!running) return;
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
		if (!$('help').hidden) {
			if (e.key === 'Escape') { $('help').hidden = true; e.preventDefault(); }
			return;
		}
		var t = e.target;
		if (t && (t.tagName === 'INPUT' || t.tagName === 'TEXTAREA' || t.isContentEditable)) return;
		if (!running || e.isComposing || e.metaKey) return;
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
	 *   |     tile view      +--------+   stat:   y of stats | minimap and items
	 *   |                    |minimap |
	 *   +--------------------+ items  |
	 *   |      messages      |        |
	 *   +--------------------+--------+
	 */
	var SPLITS = ['bottom', 'side', 'stat'];

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
		return { v: 1, zoom: zoom, mini: 2, items: 1, auto: true, font: { msg: font, stat: font, pop: font },
			split: { side: clamp((mapW + GUT / 2) / W, 0.3, 0.85), bottom: clamp((mapH + GUT / 2) / H, 0.3, 0.9),
				stat: clamp((18 * font + TITLE_H + BORDER + GUT / 2) / H, 0.2, 0.8) } };
	}

	function loadLayout() {
		var d = defaultLayout();
		try {
			var s = JSON.parse(Module.FS.readFile(LAYOUT_FILE, { encoding: 'utf8' }));
			if (s && s.v === 1) {
				if (!s.auto) {
					d.auto = false;
					SPLITS.forEach(function (k) { if (s.split[k] > 0 && s.split[k] < 1) d.split[k] = s.split[k]; });
					if (ZOOM_STEPS.indexOf(s.zoom) >= 0) d.zoom = s.zoom;
				}
				Object.keys(d.font).forEach(function (k) {
					if (s.font && FONT_STEPS.indexOf(s.font[k]) >= 0) d.font[k] = s.font[k];
				});
			}
		} catch (err) { /* nothing saved yet */ }
		L = d;
	}

	var saveTimer = 0;
	function saveLayout() {
		clearTimeout(saveTimer);
		saveTimer = setTimeout(function () {
			try { Module.FS.writeFile(LAYOUT_FILE, JSON.stringify(L)); syncFiles(); }
			catch (err) { console.warn('layout not saved', err); }
		}, 400);
	}

	function computeRects() {
		var A = areaSize(), W = A.w, H = A.h, h = GUT / 2, s = L.split;
		var xs = clamp(Math.round(W * s.side), 200, W - 100);
		var yb = clamp(Math.round(H * s.bottom), 100, H - 60);
		var ys = clamp(Math.round(H * s.stat), 60, H - 60);
		return {
			map: [0, 0, xs - h, yb - h],
			msg: [0, yb + h, xs - h, H - yb - h],
			stat: [xs + h, 0, W - xs - h, ys - h],
			inv: [xs + h, ys + h, W - xs - h, H - ys - h],
			split: { side: [xs - h, 0, GUT, H], bottom: [0, yb - h, xs - h, GUT], stat: [xs + h, ys - h, W - xs - h, GUT] }
		};
	}

	function place(el, r) {
		el.style.left = r[0] + 'px'; el.style.top = r[1] + 'px';
		el.style.width = Math.max(0, r[2]) + 'px'; el.style.height = Math.max(0, r[3]) + 'px';
	}

	/* the text layer (menus, lists, help) as a box over the game */
	function fitPop() {
		var pop = $('pop'), A = areaSize();
		if (pop.hidden) return;
		pop.style.left = Math.max(0, (A.w - pop.offsetWidth) / 2) + 'px';
		pop.style.top = Math.max(0, (A.h - pop.offsetHeight) / 2) + 'px';
	}

	function applyDom() {
		rects = computeRects();
		WIN.forEach(function (id) { place($('t-' + id), rects[id]); });
		SPLITS.forEach(function (k) { place($('split-' + k), rects.split[k]); });
		fitPop();
	}

	function redrawAll() {
		regs.forEach(function (R) { if (R && R.w) { if (R.text) drawText(R); else drawImage(R); } });
		applyDom();
	}

	function startDrag(k, e) {
		var el = $('split-' + k);
		el.setPointerCapture(e.pointerId);
		el.classList.add('drag');
		function move(ev) {
			var g = $('game').getBoundingClientRect(), H = g.height;
			if (k === 'bottom') L.split.bottom = clamp((ev.clientY - g.top) / H, 0.1, 0.9);
			if (k === 'side') L.split.side = clamp((ev.clientX - g.left) / g.width, 0.1, 0.9);
			if (k === 'stat') L.split.stat = clamp((ev.clientY - g.top) / H, 0.1, 0.9);
			L.auto = false;
			applyDom();
		}
		function up() {
			el.classList.remove('drag');
			el.removeEventListener('pointermove', move);
			el.removeEventListener('pointerup', up);
			saveLayout();
		}
		el.addEventListener('pointermove', move);
		el.addEventListener('pointerup', up);
		e.preventDefault();
	}

	function zoomMap(d) {
		L.zoom = ZOOM_STEPS[clamp(ZOOM_STEPS.indexOf(L.zoom) + d, 0, ZOOM_STEPS.length - 1)];
		L.auto = false;
		redrawAll(); saveLayout();
		status('Map tiles: ' + Math.round(32 * L.zoom) + ' px');
		setTimeout(function () { status(''); }, 1200);
	}

	function zoomText(id, d) {
		L.font[id] = FONT_STEPS[clamp(FONT_STEPS.indexOf(L.font[id]) + d, 0, FONT_STEPS.length - 1)];
		if (id === 'msg') L.font.pop = L.font.msg;      /* lists follow the messages */
		redrawAll(); saveLayout();
	}

	function resetLayout() {
		L = defaultLayout();
		redrawAll(); saveLayout();
	}

	/* ---------- saves: IndexedDB (IDBFS) ---------- */
	var syncing = false, syncAgain = false, pendingCbs = [];
	function syncFiles(cb) {
		if (!Module.FS) { if (cb) cb(); return; }
		if (typeof cb === 'function') pendingCbs.push(cb);
		if (syncing) { syncAgain = true; return; }
		syncing = true;
		var cbs = pendingCbs; pendingCbs = [];
		Module.FS.syncfs(false, function (err) {
			syncing = false;
			if (err) status('Saving to browser storage (IndexedDB) failed: ' + err + '. Use "Export save" to keep a copy.', true);
			cbs.forEach(function (f) { f(err); });
			if (syncAgain) { syncAgain = false; syncFiles(); }
		});
	}
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
	function exportSave() {
		if (running) saveReq = true;
		setTimeout(function () {
			var files = saveFiles();
			if (!newestSave()) { status('There is no saved game yet.', true); setTimeout(function () { status(''); }, 2000); return; }
			var pack = {};
			files.forEach(function (f) {
				var d = Module.FS.readFile(DIR + '/' + f), s = '';
				for (var i = 0; i < d.length; i++) s += String.fromCharCode(d[i]);
				pack[f] = btoa(s);
			});
			var a = document.createElement('a');
			a.href = URL.createObjectURL(new Blob([JSON.stringify({ crawl: 1, files: pack })], { type: 'application/json' }));
			a.download = 'crawl-save.json';
			document.body.appendChild(a); a.click();
			setTimeout(function () { URL.revokeObjectURL(a.href); a.remove(); }, 1000);
		}, running ? 1500 : 0);
	}
	function importSave(file) {
		var r = new FileReader();
		r.onload = function () {
			var pack;
			try { pack = JSON.parse(r.result); if (!pack.crawl) throw 0; }
			catch (e) { status('That is not a save exported from this page.', true); return; }
			if (!confirm('Replace the saved games in this browser with "' + file.name + '"?')) return;
			running = false;
			saveFiles().forEach(function (f) { Module.FS.unlink(DIR + '/' + f); });
			Object.keys(pack.files).forEach(function (f) {
				if (/[\/]/.test(f)) return;
				var s = atob(pack.files[f]), d = new Uint8Array(s.length);
				for (var i = 0; i < s.length; i++) d[i] = s.charCodeAt(i);
				Module.FS.writeFile(DIR + '/' + f, d);
			});
			syncFiles(function (err) { if (!err) location.reload(); });
		};
		r.readAsText(file);
	}
	function newGame() {
		if (!confirm('Delete the saved games in this browser and start a new character?')) return;
		running = false;
		saveFiles().forEach(function (f) { Module.FS.unlink(DIR + '/' + f); });
		syncFiles(function (err) { if (!err) location.reload(); });
	}

	/* ---------- help ---------- */
	var helpLoaded = false;
	function toggleHelp() {
		var h = $('help');
		h.hidden = !h.hidden;
		if (!h.hidden && !helpLoaded) {
			helpLoaded = true;
			fetch('help.html').then(function (r) { if (!r.ok) throw new Error(r.status); return r.text(); })
				.then(function (t) { $('help-body').innerHTML = t; })
				.catch(function (err) { helpLoaded = false; $('help-body').textContent = 'Could not load the guide (' + err + '). Press ? in the game for its own help.'; });
		}
		if (!h.hidden) $('help-body').focus();
	}

	/* ---------- startup ---------- */
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
				if (err) status('Could not read saved games from IndexedDB (' + err + '). Saving may not work in this browser mode.', true);
				var name = newestSave();
				if (name) Module.arguments.push('-name', name);   /* continue that character */
				loadLayout();
				Module.removeRunDependency('idbfs');
			});
		}],
		onRuntimeInitialized: function () {
			running = true;
			status('');
		},
		print: function (s) { console.log(s); },
		printErr: function (s) { console.warn(s); },
		setStatus: function (s) { if (s && !running) status(s.replace(/\(\d+\/\d+\)/, '').trim() || 'Loading…'); },
		onAbort: function (what) { crashed(what); }
	};

	function crashed(err) {
		if (!running) return;
		running = false;
		var msg = (err && (err.message || err.reason && err.reason.message)) || String(err);
		console.error('[crawl] crash:', err);
		status('The game crashed (' + msg + '). Reload the page to continue from the last autosave.', true);
	}
	window.addEventListener('unhandledrejection', function (e) {
		if (e.reason && e.reason.name === 'ExitStatus') return;   /* exit() is the normal end */
		crashed(e.reason);
	});
	window.addEventListener('error', function (e) {
		if (e.error && e.error.name === 'ExitStatus') return;
		if (e.error instanceof WebAssembly.RuntimeError || /crawl-core/.test(e.filename || '')) crashed(e.error || e.message);
	});

	/* autosave: every 2 minutes and when the page is hidden */
	setInterval(function () { saveReq = true; }, 120000);
	document.addEventListener('visibilitychange', function () { if (document.hidden) saveReq = true; });
	window.addEventListener('beforeunload', function (e) { if (running) { e.preventDefault(); e.returnValue = ''; } });

	document.addEventListener('keydown', onKey);
	document.addEventListener('DOMContentLoaded', function () {
		$('btn-export').onclick = exportSave;
		$('btn-import').onclick = function () { $('import-file').click(); };
		$('import-file').onchange = function () { if (this.files[0]) importSave(this.files[0]); this.value = ''; };
		$('btn-new').onclick = newGame;
		$('btn-help').onclick = toggleHelp;
		$('help-close').onclick = toggleHelp;
		$('btn-zoom-in').onclick = function () { zoomMap(1); };
		$('btn-zoom-out').onclick = function () { zoomMap(-1); };
		$('btn-layout').onclick = resetLayout;
		$('btn-restart').onclick = function () { location.reload(); };
		document.querySelectorAll('button').forEach(function (b) {
			b.addEventListener('mousedown', function (e) { e.preventDefault(); });
		});
		SPLITS.forEach(function (k) { $('split-' + k).addEventListener('pointerdown', function (e) { startDrag(k, e); }); });
		['msg', 'stat'].forEach(function (id) {
			var w = $('t-' + id);
			w.querySelector('.zin').addEventListener('click', function () { zoomText(id, 1); });
			w.querySelector('.zout').addEventListener('click', function () { zoomText(id, -1); });
		});
	});
	var resizeTimer = 0;
	window.addEventListener('resize', function () {
		if (!L) return;
		clearTimeout(resizeTimer);
		resizeTimer = setTimeout(function () {
			if (L.auto) {                        /* not customised: follow the window */
				var d = defaultLayout();
				L.split = d.split; L.zoom = d.zoom;
			}
			redrawAll();
		}, 150);
	});
})();
