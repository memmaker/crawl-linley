#!/usr/bin/env python3
"""Writes the in-page game guide (dist/help.html) for the web build.

The game content comes from the desktop key guides in
~/Desktop/Games/Roguelikes/Docs (build-docs.py + guides.py), so both guides
stay in sync; only the saving and "playing in the browser" parts are
written here, because they differ on the web."""
import html, importlib.util, os, sys

DOCS = os.path.expanduser('~/Desktop/Games/Roguelikes/Docs')
PAGE = 'linleys-crawl.html'

sys.path.insert(0, DOCS)
spec = importlib.util.spec_from_file_location('build_docs', os.path.join(DOCS, 'build-docs.py'))
docs = importlib.util.module_from_spec(spec)
spec.loader.exec_module(docs)
from guides import GUIDES   # noqa: E402

game = next(g for g in docs.GAMES if g['file'] == PAGE)
guide = dict(GUIDES[PAGE])
info = dict(game['info'])
kbd = docs.kbd
esc = html.escape

SAVING = '''<ul>
<li><strong>Saving is automatic.</strong> Your character and the current level are stored in this browser (IndexedDB) every two minutes while the game waits for your next command, when you switch to another tab or window, and on every staircase. Reloading the page continues the last character you played.</li>
<li><kbd>S</kbd> (answer with a capital <kbd>Y</kbd>) or <kbd>Ctrl+X</kbd> saves and ends the session, as in the original. <em>Play again</em> or a reload continues.</li>
<li>When your character dies, its save is deleted: death is final.</li>
<li>To start another character, press <em>New game</em> (it deletes the saves in this browser) or type a new name at the start.</li>
<li><em>Export save</em> downloads all save files as one <code>.json</code> file; <em>Import save</em> loads one (browser saves only; the Mac version's saves are separate).</li>
<li>Window layout and zoom are stored in the same browser storage.</li>
<li>Private/incognito windows and "clear site data" delete the stored game. Export first if it matters.</li>
</ul>'''

WEB = '''<ul>
<li><strong>Windows:</strong> the tile view top left (32×32 tiles from the 2005 tile version), Messages under it, the character on the right, the level map below it and your inventory (the item types from <code>show_items</code> in init.txt) at the bottom right. Lists, menus and help appear as a box over the game, in the IBM VGA 8×16 font (<a href="https://int10h.org/oldschool-pc-fonts/" target="_blank" rel="noopener">The Oldschool PC Font Resource</a>, CC BY-SA 4.0).</li>
<li><strong>Resize windows</strong> by dragging the gaps between them. <em>Reset windows</em> puts everything back.</li>
<li><strong>Inventory window:</strong> click an item to use it (eat, wield, drop…), right-click to describe it. Which item classes it shows is <code>show_items</code> in init.txt; every class is on by default.</li>
<li><strong>Zoom:</strong> <em>Zoom −</em> / <em>Zoom +</em> change the size of the tiles (scaled without blurring). Hover over a text window's title to show its <em>A−</em> / <em>A+</em> buttons; the text is sharpest at 16 and 32 px.</li>
<li><strong>No sound:</strong> the original game has none.</li>
<li><strong>Keys:</strong> the arrow keys, the numeric keypad or the vi keys (<kbd>h</kbd><kbd>j</kbd><kbd>k</kbd><kbd>l</kbd><kbd>y</kbd><kbd>u</kbd><kbd>b</kbd><kbd>n</kbd>) move you; Shift+direction runs.</li>
<li>Browsers keep a few shortcuts for themselves (<kbd>Ctrl+W</kbd>, <kbd>Ctrl+T</kbd>, <kbd>Ctrl+N</kbd>, and <kbd>Cmd</kbd> shortcuts on a Mac), so those never reach the game.</li>
<li>If the game ever crashes, a message appears at the top; reload the page to continue from the last autosave.</li>
</ul>'''

KEY_HINTS = [
    ('?', 'In-game help'),
    ('Ctrl+O', 'Auto-explore (also keypad 0)'),
    ('Enter', 'Menu of all commands'),
    ('i', 'Inventory with a cursor: letter = main action, Enter = all actions'),
    ('<', 'Go up (walks to the nearest known staircase)'),
    ('>', 'Go down (walks to the nearest known staircase)'),
    ('S', 'Save and end the session'),
]


def dl(items):
    return '<dl>' + ''.join(f'<dt>{kbd(k)}</dt><dd>{esc(d)}</dd>' for k, d in items) + '</dl>'


def section(anchor, title, body):
    return f'<h2 id="h-{anchor}">{esc(title)}</h2>{body}'


parts = []
toc = [('about', 'About the game'), ('keys', 'Keyboard controls'), ('saving', 'Saving your game'),
       ('tips', 'Tips'), ('guide', "New player's guide"), ('web', 'Playing in the browser')]
parts.append('<p>' + esc(game['tagline']) + '</p>' + info['About the game'] + '<ul class="toc">' +
             ''.join(f'<li><a href="#h-{a}">{esc(t)}</a></li>' for a, t in toc) + '</ul>')

parts.append(section('about', 'About the game',
                     guide.pop('How it differs from Stone Soup')))

ess = ''.join(f'<div class="box"><h3>{esc(cat)}</h3>{dl(items)}</div>' for cat, items in game['essentials'])
all_keys = game['all']() if callable(game['all']) else game['all']
full = ''.join(f'<div>{kbd(k)}<span>{esc(d)}</span></div>' for k, d in all_keys)
parts.append(section('keys', 'Keyboard controls',
                     '<div class="box key"><h3>The keys to remember</h3>' + dl(KEY_HINTS) + '</div>'
                     '<h3>Essential keys</h3><div class="grid">' + ess + '</div>'
                     '<details><summary>Complete key list (' + str(len(all_keys)) + ' commands)</summary>'
                     '<div class="all">' + full + '</div></details>'))

parts.append(section('saving', 'Saving your game', SAVING))
parts.append(section('tips', 'Tips', info['Tips']))
parts.append(section('guide', "New player's guide",
                     ''.join(f'<h3>{esc(t)}</h3>{b}' for t, b in guide.items())))
parts.append(section('web', 'Playing in the browser', WEB))

# RVIP: About this version (rogue2wasm.md: Source and changes)
parts.append('<h2 id="h-version">About this version</h2><ul>'
             '<li>Based on <strong>Linley\'s Dungeon Crawl 4.00 beta 26</strong> (Linley Henzell and the Crawl DevTeam) '
             'with the <strong>Dungeon Crawl Tile Version e070</strong> (Mitsuhiro Itakura, 2005).</li>'
             '<li>Original source: <a href="https://github.com/memmaker/crawl-linley/tree/d5dff29" target="_blank" rel="noopener">dc400b26-src.zip</a> '
             'and <a href="https://github.com/memmaker/crawl-linley/tree/fd103ab" target="_blank" rel="noopener">the tile version patch</a> '
             '(both from crawlj.sourceforge.jp via the Wayback Machine).</li>'
             '<li>Our changes (port, auto-explore stops, stair walking, command menu, inventory menus, web build): '
             '<a href="https://github.com/memmaker/crawl-linley/compare/fd103ab...main" target="_blank" rel="noopener">memmaker/crawl-linley</a></li></ul>')
print('\n'.join(parts))
