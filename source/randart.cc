/*
 *  File:       randart.cc
 *  Summary:    Random and unrandom artifact functions.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *   <8>     19 Jun 99   GDL    added IBMCPP support
 *   <7>     14/12/99    LRH    random2 -> random5
 *   <6>     11/06/99    cdl    random4 -> random2
 *
 *   <1>     -/--/--     LRH    Created
 */

#include "AppHdr.h"
#include "randart.h"

#include <string.h>
#include <stdio.h>

#include "externs.h"
#include "itemname.h"
#include "stuff.h"
#include "wpn-misc.h"

/*
   The initial generation of a randart is very simple - it occurs
   in dungeon.cc and consists of giving it a few random things - plus & plus2
   mainly.
*/
const char *rand_wpn_names[] = {
#ifdef JP
    "血流の",
    "死の",
    "惨殺の",
    "苦痛の",
    "悶死の",
    "死と苦痛の",
    "無限業罰の",
    "永劫痛苦の",
    "権能の",
    "憤怒の",
#else
    " of Blood",
    " of Death",
    " of Bloody Death",
    " of Pain",
    " of Painful Death",
    " of Pain & Death",
    " of Infinite Pain",
    " of Eternal Torment",
    " of Power",
    " of Wrath",
#endif
/* 10: */
#ifdef JP
    "破滅の",
    "かよわき慈悲の",
    "黙示録の",
    "道化師の",
    "指輪の",
    "愚者の",
    "神々の",
    "帝国の",
    "破壊の",
    "ハルマゲドンの",
#else
    " of Doom",
    " of Tender Mercy",
    " of the Apocalypse",
    " of the Jester",
    " of the Ring",
    " of the Fool",
    " of the Gods",
    " of the Imperium",
    " of Destruction",
    " of Armageddon",
#endif
/* 20: */
#ifdef JP
    "無慈悲なる正義の",
    "正義の怒りの",
    "膂力の",
    "宝珠の",
    "マクレブの",
    "トログの",
    "ゾムの",
    "古代の",
    "魔法力の",
    "ネメレクスの",
#else
    " of Cruel Justice",
    " of Righteous Anger",
    " of Might",
    " of the Orb",
    " of Makhleb",
    " of Trog",
    " of Xom",
    " of the Ancients",
    " of Mana",
    " of Nemelex Xobeh",
#endif
/* 30: */
#ifdef JP
    "魔道士の",
    "大魔道士の",
    "王の",
    "女王の",
    "天球の",
    "円環の",
    "血族の",
    "闘争の",
    "戦闘の",
    "誉れの",
#else
    " of the Magi",
    " of the Archmagi",
    " of the King",
    " of the Queen",
    " of the Spheres",
    " of Circularity",
    " of Linearity",
    " of Conflict",
    " of Battle",
    " of Honour",
#endif
/* 40: */
#ifdef JP
    "胡蝶の",
    "胡蜂の",
    "蛙の",
    "鼬の",
    "野蛮人の",
    "団子虫の",
    "原罪の",
    "復讐の",
    "処刑の",
    "調停の",
#else
    " of the Butterfly",
    " of the Wasp",
    " of the Frog",
    " of the Weasel",
    " of the Troglodytes",
    " of the Pill-Bug",
    " of Sin",
    " of Vengeance",
    " of Execution",
    " of Arbitration",
#endif
/* 50: */
#ifdef JP
    "探求者の",
    "真実の",
    "嘘の",
    "なすびの",
    "かぶらの",
    "好機の",
    "呪詛の",
    "地獄の怒りの",
    "不死者の",
    "混沌の",
#else
    " of the Seeker",
    " of Truth",
    " of Lies",
    " of the Eggplant",
    " of the Turnip",
    " of Chance",
    " of Curses",
    " of Hell's Wrath",
    " of the Undead",
    " of Chaos",
#endif
/* 60: */
#ifdef JP
    "秩序の",
    "生命の",
    "旧世界の",
    "新世界の",
    "中つ国の",
    "徘徊の",
    "不愉快の",
    "悲嘆の",
    "残忍なる報復の",
    "征服の",
#else
    " of Law",
    " of Life",
    " of the Old World",
    " of the New World",
    " of the Middle World",
    " of Crawl",
    " of Unpleasantness",
    " of Discomfort",
    " of Brutal Revenge",
    " of Triumph",
#endif
/* 70: */
#ifdef JP
    "解剖の",
    "寸断の",
    "戦慄の",
    "恐怖の",
    "傲慢の",
    "火山の",
    "血に飢えた",
    "両断の",
    "永遠調和の",
    "平安の",
#else
    " of Evisceration",
    " of Dismemberment",
    " of Terror",
    " of Fear",
    " of Pride",
    " of the Volcano",
    " of Blood-Lust",
    " of Division",
    " of Eternal Harmony",
    " of Peace",
#endif
/* 80: */
#ifdef JP
    "素早き死の",
    "即死の",
    "窮乏の",
    "鯢の",
    "ロブスターの",
    "巻貝の",
    "ペンギンの",
    "角目鳥の",
    "茸の",
    "毒茸の",
#else
    " of Quick Death",
    " of Instant Death",
    " of Misery",
    " of the Whale",
    " of the Lobster",
    " of the Whelk",
    " of the Penguin",
    " of the Puffin",
    " of the Mushroom",
    " of the Toadstool",
#endif
/* 90: */
#ifdef JP
    "民草の",
    "埃茸の",
    "胞子の",
    "最善の",
    "パレート最適の",
    "最大幸福の",
    "無秩序の",
    "旧き悪霊の",
    "革命の",
    "人民の",
#else
    " of the Little People",
    " of the Puffball",
    " of Spores",
    " of Optimality",
    " of Pareto-Optimality",
    " of Greatest Utility",
    " of Anarcho-Capitalism",
    " of Ancient Evil",
    " of the Revolution",
    " of the People",
#endif
/* 100: */
#ifdef JP
    "エルフ族の",
    "ドワーフ族の",
    "オーク族の",
    "ヒト族の",
    "汚泥の",
    "竜神の",
    "トロル族の",
    "人喰い鬼の",
    "公正なる再分配の",
    "分限者の",
#else
    " of the Elves",
    " of the Dwarves",
    " of the Orcs",
    " of the Humans",
    " of Sludge",
    " of the Naga",
    " of the Trolls",
    " of the Ogres",
    " of Equitable Redistribution",
    " of Wealth",
#endif
/* 110: */
#ifdef JP
    "文無しの",
    "再配分の",
    "儚き平和の",
    "強化の",
    "美貌の",
    "蛞蝓の",
    "蝸牛の",
    "巻貝の",
    "体刑の",
    "極刑の",
#else
    " of Poverty",
    " of Reapportionment",
    " of Fragile Peace",
    " of Reinforcement",
    " of Beauty",
    " of the Slug",
    " of the Snail",
    " of the Gastropod",
    " of Corporal Punishment",
    " of Capital Punishment",
#endif
/* 120: */
#ifdef JP
    "黙示録の獣の",
    "光芒の",
    "暗黒の",
    "陽光の",
    "昼つ方の",
    "夜闇の",
    "夜さり方の",
    "夕闇の",
    "黄昏の",
    "曉光の",
#else
    " of the Beast",
    " of Light",
    " of Darkness",
    " of Day",
    " of the Day",
    " of Night",
    " of the Night",
    " of Twilight",
    " of the Twilight",
    " of Dawn",
#endif
/* 130: */
#ifdef JP
    "暁の",
    "太陽の",
    "月の",
    "遠方世界の",
    "見えざる領域の",
    "パンデモニウムの",
    "アビスの",
    "因果の",
    "獄舎の",
    "十字軍の",
#else
    " of the Dawn",
    " of the Sun",
    " of the Moon",
    " of Distant Worlds",
    " of the Unseen Realm",
    " of Pandemonium",
    " of the Abyss",
    " of the Nexus",
    " of the Gulag",
    " of the Crusades",
#endif
/* 140: */
#ifdef JP
    "肉迫の",
    "損壊の",
    "危難の",
    "永遠の戦士の",
    "永遠の戦の",
    "邪悪の",
    "滅多打ちの",
    "膿漏の",
    "悪疫の",
    "災いの",
#else
    " of Proximity",
    " of Wounding",
    " of Peril",
    " of the Eternal Warrior",
    " of the Eternal War",
    " of Evil",
    " of Pounding",
    " of Oozing Pus",
    " of Pestilence",
    " of Plague",
#endif
/* 150: */
#ifdef JP
    "否定の",
    "救世主の",
    "感染の",
    "防御の",
    "護りの",
    "攻撃による防御の",
    "功利の",
    "条理の",
    "不条理の",
    "勇気の",
#else
    " of Negation",
    " of the Saviour",
    " of Infection",
    " of Defence",
    " of Protection",
    " of Defence by Offence",
    " of Expedience",
    " of Reason",
    " of Unreason",
    " of the Heart",
#endif
/* 160: */
#ifdef JP
    "攻勢の",
    "麻の葉の",
    "群葉の",
    "冬の",
    "夏の",
    "秋の",
    "春の",
    "真夏の",
    "真冬の",
    "明けざる夜の",
#else
    " of Offence",
    " of the Leaf",
    " of Leaves",
    " of Winter",
    " of Summer",
    " of Autumn",
    " of Spring",
    " of Midsummer",
    " of Midwinter",
    " of Eternal Night",
#endif
/* 170: */
#ifdef JP
    "叫喚地獄の",
    "蠢くものの",
    "這い寄るものの",
    "物体Ｘの",
    "『件』の",
    "海原の",
    "森林の",
    "木々の",
    "大地の",
    "森羅万象の",
#else
    " of Shrieking Terror",
    " of the Lurker",
    " of the Crawling Thing",
    " of the Thing",
    "\"Thing\"",
    " of the Sea",
    " of the Forest",
    " of the Trees",
    " of Earth",
    " of the World",
#endif
/* 180: */
#ifdef JP
    "聖餐の",
    "泡沫の",
    "アメーバの",
    "奇形の",
    "罪悪の",
    "無垢の",
    "栄達の",
    "零落の",
    "妙音の",
    "光明の",
#else
    " of Bread",
    " of Yeast",
    " of the Amoeba",
    " of Deformation",
    " of Guilt",
    " of Innocence",
    " of Ascent",
    " of Descent",
    " of Music",
    " of Brilliance",
#endif
/* 190: */
#ifdef JP
    "嫌悪の",
    "饗宴の",
    "太陽光の",
    "星芒の",
    "星々の",
    "塵芥の",
    "雲界の",
    "天空の",
    "灰燼の",
    "粘着の",
#else
    " of Disgust",
    " of Feasting",
    " of Sunlight",
    " of Starshine",
    " of the Stars",
    " of Dust",
    " of the Clouds",
    " of the Sky",
    " of Ash",
    " of Slime",
#endif
/* 200: */
#ifdef JP
    "清澄の",
    "絶えざる警戒の",
    "決意の",
    "蛾の",
    "生贄の",
    "堅忍の",
    "平衡の",
    "均衡の",
    "不均衡の",
    "調和の",
#else
    " of Clarity",
    " of Eternal Vigilance",
    " of Purpose",
    " of the Moth",
    " of the Goat",
    " of Fortitude",
    " of Equivalence",
    " of Balance",
    " of Unbalance",
    " of Harmony",
#endif
/* 210: */
#ifdef JP
    "不調和の",
    "火炎地獄の",
    "終末点の",
    "騰貴の",
    "低落の",
    "供給の",
    "需要の",
    "ＧＤＰの",
    "不当利得の",
    "不法留置の",
#else
    " of Disharmony",
    " of the Inferno",
    " of the Omega Point",
    " of Inflation",
    " of Deflation",
    " of Supply",
    " of Demand",
    " of Gross Domestic Product",
    " of Unjust Enrichment",
    " of Detinue",
#endif
/* 220: */
#ifdef JP
    "転換の",
    "アントンの",
    "勅書の",
    "挫折の",
    "侵害の",
    "違反の",
    "終結の",
    "根絶の",
    "贖罪の",
    "無主物の",
#else
    " of Conversion",
    " of Anton Piller",
    " of Mandamus",
    " of Frustration",
    " of Breach",
    " of Fundamental Breach",
    " of Termination",
    " of Extermination",
    " of Satisfaction",
    " of Res Nullius",
#endif
/* 230: */
#ifdef JP
    "封土の",
    "無主占有の",
    "脈絡の",
    "掟の",
    "自由保有の",
    "不法作為の",
    "不作為の",
    "過失の",
    "刑罰の",
    "汚名の",
#else
    " of Fee Simple",
    " of Terra Nullius",
    " of Context",
    " of Prescription",
    " of Freehold",
    " of Tortfeasance",
    " of Omission",
    " of Negligence",
    " of Pains",
    " of Attainder",
#endif
/* 240: */
#ifdef JP
    "敏活の",
    "休眠の",
    "廃止の",
    "放擲の",
    "砂漠の",
    "原生林の",
    "物狂いの",
    "不安の",
    "偏心の",
    "饗応の",
#else
    " of Action",
    " of Inaction",
    " of Truncation",
    " of Defenestration",
    " of Desertification",
    " of the Wilderness",
    " of Psychosis",
    " of Neurosis",
    " of Fixation",
    " of the Open Hand",
#endif
/* 250: */
#ifdef JP
    "牙の",
    "誠実の",
    "不実の",
    "神々の強制の",
    "見えざる手の",
    "自己決定の",
    "自由の",
    "隷属の",
    "独裁の",
    "緊迫の",
#else
    " of the Tooth",
    " of Honesty",
    " of Dishonesty",
    " of Divine Compulsion",
    " of the Invisible Hand",
    " of Freedom",
    " of Liberty",
    " of Servitude",
    " of Domination",
    " of Tension",
#endif
/* 260: */
#ifdef JP
    "一なる神の",
    "不敬の",
    "不可知の",
    "実存の",
    "善き",
    "相対の",
    "絶対の",
    "赦罪の",
    "禁欲の",
    "憎悪の",
#else
    " of Monotheism",
    " of Atheism",
    " of Agnosticism",
    " of Existentialism",
    " of the Good",
    " of Relativism",
    " of Absolutism",
    " of Absolution",
    " of Abstinence",
    " of Abomination",
#endif
/* 270: */
#ifdef JP
    "切除の",
    "鬱血の",
    "不可思議の",
    "濁音の",
    "薄明の",
    "光輝の",
    "不道徳の",
    "無道徳の",
    "精密手術の",
    "正教の",
#else
    " of Mutilation",
    " of Stasis",
    " of Wonder",
    " of Dullness",
    " of Dim Light",
    " of the Shining Light",
    " of Immorality",
    " of Amorality",
    " of Precise Incision",
    " of Orthodoxy",
#endif
/* 280: */
#ifdef JP
    "信義の",
    "虚偽の",
    "占い師の",
    "風水師の",
    "預言者の",
    "打擲の",
    "革新の",
    "硫黄の",
    "卵の",
    "天体の",
#else
    " of Faith",
    " of Untruth",
    " of the Augurer",
    " of the Water Diviner",
    " of the Soothsayer",
    " of Punishment",
    " of Amelioration",
    " of Sulphur",
    " of the Egg",
    " of the Globe",
#endif
/* 290: */
#ifdef JP
    "蝋燭の",
    "燭台の",
    "吸血鬼の",
    "オーク族の",
    "ホビット族の",
    "世の果ての",
    "蒼天の",
    "茜空の",
    "橙空の",
    "紫空の",
#else
    " of the Candle",
    " of the Candelabrum",
    " of the Vampires",
    " of the Orcs",
    " of the Halflings",
    " of World's End",
    " of Blue Skies",
    " of Red Skies",
    " of Orange Skies",
    " of Purple Skies",
#endif
/* 300: */
#ifdef JP
    "滑舌の",
    "理性の",
    "蜘蛛の",
    "八つ目鰻の",
    "原初の",
    "終末の",
    "断絶の",
    "追放の",
    "哀悼の",
    "死のとば口の",
#else
    " of Articulation",
    " of the Mind",
    " of the Spider",
    " of the Lamprey",
    " of the Beginning",
    " of the End",
    " of Severance",
    " of Sequestration",
    " of Mourning",
    " of Death's Door",
#endif
/* 310: */
#ifdef JP
    "鍵の",
    "地震の",
    "失敗の",
    "成功の",
    "強迫の",
    "蚊の",
    "虻の",
    "青蝿の",
    "海亀の",
    "陸亀の",
#else
    " of the Key",
    " of Earthquakes",
    " of Failure",
    " of Success",
    " of Intimidation",
    " of the Mosquito",
    " of the Gnat",
    " of the Blowfly",
    " of the Turtle",
    " of the Tortoise",
#endif
/* 320: */
#ifdef JP
    "黄泉の",
    "墓標の",
    "服従の",
    "支配の",
    "伝令の",
    "結晶の",
    "重力の",
    "浮力の",
    "泥濘の",
    "不意討ちの",
#else
    " of the Pit",
    " of the Grave",
    " of Submission",
    " of Dominance",
    " of the Messenger",
    " of Crystal",
    " of Gravity",
    " of Levity",
    " of the Slorg",
    " of Surprise",
#endif
/* 330: */
#ifdef JP
    "迷路の",
    "迷宮の",
    "神の調停の",
    "循環の",
    "糸疣の",
    "蠍の",
    "悪魔族の",
    "天賦の",
    "血玉髄の",
    "グロントルの",     //Grontolがどうしても分からなかったのでカタカナ。
#else
    " of the Maze",
    " of the Labyrinth",
    " of Divine Intervention",
    " of Rotation",
    " of the Spinneret",
    " of the Scorpion",
    " of Demonkind",
    " of the Genius",
    " of Bloodstone",
    " of Grontol",
#endif
/* 340: */
#ifdef JP
    "『グリムトゥース』",
    "『ウィドウメイカー』",
    "『ウィドワメイカー』",
    "『命の破滅』",
    "『介添人』",
    "『追放者』",
    "『拷問吏』",
    "『秘密兵器』",
    "『絞首刑』",
    "『八頭体』",
#else
    " \"Grim Tooth\"",
    " \"Widowmaker\"",
    " \"Widowermaker\"",
    " \"Lifebane\"",
    " \"Conservator\"",
    " \"Banisher\"",
    " \"Tormentor\"",
    " \"Secret Weapon\"",
    " \"String\"",
    " \"Stringbean\"",
#endif
/* 350: */
#ifdef JP
    "『ブロブ』",
    "『グロブルス』",
    "『ハルク』",
    "『皺くちゃ婆ア』",
    "『星月夜』",
    "『巨人の爪楊枝』",
    "『優柔不断』",
    "『おべっか使い』",
    "『鎧袖一触』",
    "『不平不満』",
#else
    " \"Blob\"",
    " \"Globulus\"",
    " \"Hulk\"",
    " \"Raisin\"",
    " \"Starlight\"",
    " \"Giant's Toothpick\"",
    " \"Pendulum\"",
    " \"Backscratcher\"",
    " \"Brush\"",
    " \"Murmur\"",
#endif
/* 360: */
#ifdef JP
    "『石棺』",
    "『調和』",
    "『龍の舌』",
    "『調停者』",
    "『グラム』",
    "『グロム』",
    "『グリム』",
    "『グルム』",
    "『虱潰し』",
    "『オムレット』",
#else
    " \"Sarcophage\"",
    " \"Concordance\"",
    " \"Dragon's Tongue\"",
    " \"Arbiter\"",
    " \"Gram\"",
    " \"Grom\"",
    " \"Grim\"",
    " \"Grum\"",
    " \"Rummage\"",
    " \"Omelette\"",
#endif
/* 370: */
#ifdef JP
    "『未熟者』",
    "『なすび』",
    "『Z』",
    "『X』",
    "『Q』",
    "『Ox』",
    "『臨終喘鳴』",
    "『告げ口屋』",
    "『淫売野郎』",
    "『袖の下』",
#else
    " \"Egg\"",
    " \"Aubergine\"",
    " \"Z\"",
    " \"X\"",
    " \"Q\"",
    " \"Ox\"",
    " \"Death Rattle\"",
    " \"Tattletale\"",
    " \"Fish\"",
    " \"Bung\"",
#endif
/* 380: */
#ifdef JP
    "『アルカナ』",
    "『死の泥饅頭』",
    "『魂の輪廻』",
    "『最後通牒』",
    "『蚯蚓』",
    "『虫けら』",
    "『虫けらの逆襲』",
    "『ゾムの恩寵』",
    "『大正解』",
    "『レビ記』",
#else
    " \"Arcanum\"",
    " \"Mud Pie of Death\"",
    " \"Transmigrator\"",
    " \"Ultimatum\"",
    " \"Earthworm\"",
    " \"Worm\"",
    " \"Worm's Wrath\"",
    " \"Xom's Favour\"",
    " \"Bingo\"",
    " \"Leviticus\"",
#endif
// Not yet possible...
/* 390: */
#ifdef JP
    "『快楽殺人者の』",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#else
    " of Joyful Slaughter",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#endif

/* 390: */
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",
    "\"\"",

/* 340: */
#ifdef JP
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#else
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#endif

/* 200: */
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
    "",
};

const char *rand_armour_names[] = {
/* 0: */
#ifdef JP
    "遮蔽の",
    "恩寵の",
    "不浸透の",
    "御回りの",
    "生命の",
    "防御の",
    "戯言の",
    "絶えざる警戒の",
    "戯謔の",
    "歓喜の",
#else
    " of Shielding",
    " of Grace",
    " of Impermeability",
    " of the Onion",
    " of Life",
    " of Defence",
    " of Nonsense",
    " of Eternal Vigilance",
    " of Fun",
    " of Joy",
#endif
/* 10: */
#ifdef JP
    "死のとば口の",
    "門の",
    "警護の",
    "完全性の",
    "全体調和の",
    "調和の",
    "不可触民の",
    "賤民の",
    "卑賤の",
    "汚濁の",
#else
    " of Death's Door",
    " of the Gate",
    " of Watchfulness",
    " of Integrity",
    " of Bodily Harmony",
    " of Harmony",
    " of the Untouchables",
    " of Grot",
    " of Grottiness",
    " of Filth",
#endif
/* 20: */
#ifdef JP
    "不思議の",
    "不思議な力の",
    "力の",
    "ヴラド公の",
    "霊の実の",
    "無敵の",
    "隠れん坊の",
    "臆病者の",
    "救世主の",
    "柔軟の",
#else
    " of Wonder",
    " of Wondrous Power",
    " of Power",
    " of Vlad",
    " of the Eternal Fruit",
    " of Invincibility",
    " of Hide-and-Seek",
    " of the Mouse",
    " of the Saviour",
    " of Plasticity",
#endif
/* 30: */
#ifdef JP
    "禿頭の",
    "戦慄の",
    "アルカナの",
    "死への抗戦の",
    "無痛覚の",
    "守護者の",
    "神聖不可侵の",
    "海亀の",
    "陸亀の",
    "鎧鼠の",
#else
    " of Baldness",
    " of Terror",
    " of the Arcane",
    " of Resist Death",
    " of Anaesthesia",
    " of the Guardian",
    " of Inviolability",
    " of the Tortoise",
    " of the Turtle",
    " of the Armadillo",
#endif
/* 40: */
#ifdef JP
    "針土竜の",
    "鎧われし者の",
    "超自然の",
    "激情の",
    "利運の",
    "損害の",
    "保険の",
    "賠償の",
    "制約の",
    "排斥の",
#else
    " of the Echidna",
    " of the Armoured One",
    " of Weirdness",
    " of Pathos",
    " of Serendipity",
    " of Loss",
    " of Hedging",
    " of Indemnity",
    " of Limitation",
    " of Exclusion",
#endif
/* 50: */
#ifdef JP
    "斥力の",
    "語られざる秘密の",
    "大地の",
    "雉鳩の",
    "有限責任の",
    "責務の",
    "ハジャムの",
    "栄光の",
    "維持の",
    "恒存の",
#else
    " of Repulsion",
    " of Untold Secrets",
    " of the Earth",
    " of the Turtledove",
    " of Limited Liability",
    " of Responsibility",
    " of Hadjma",
    " of Glory",
    " of Preservation",
    " of Conservation",
#endif
/* 60: */
#ifdef JP
    "保護拘束の",
    "黙秘の",
    "旧悪の",
    "鈍物の",
    "野蛮の",
    "強靭の",
    "空間の",
    "真空の",
    "加圧の",
    "減圧の",
#else
    " of Protective Custody",
    " of the Clam",
    " of the Barnacle",
    " of the Lobster",
    " of Hairiness",
    " of Supple Strength",
    " of Space",
    " of the Vacuum",
    " of Compression",
    " of Decompression",
#endif

/* 70: */
#ifdef JP
    "ヘチマの",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#else
    " of the Loofah",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
    " of ",
#endif
// Sarcophagus
};

#ifdef JP
    //50
    const char *rand_pre_names[] = {
    "紅き",      //色彩
    "緋色の",
    "蒼き",
    "紺碧の",
    "深緑の",
    "白き",
    "白亜の",
    "黒き",
    "漆黒の",
    "灰色の",
    "水晶の",    //材質
    "翡翠の",
    "琥珀の",
    "金剛の",
    "鋼鉄の",
    "白銀の",
    "黄金の",
    "血塗れの",  //状態・性質
    "旧き",
    "昏き",
    "猛き",
    "冷酷なる",
    "残忍なる",
    "静かなる",
    "絶望の",
    "苦悩の",
    "彷徨える",
    "待たれたる",
    "眠れる",
    "輝ける",
    "光の",      //属性・場所
    "星辰の",
    "冥界の",
    "黄泉の",
    "地獄の",
    "混沌の",
    "暗黒の",
    "天界の",
    "黙示録の",
    "宵闇の",
    "運命の",
    "封印の",
    "異界の",
    "予言の",
    "凶運の",
    "東方の",
    "西方の",
    "死の",
    "死せる",
    "不死の",
    };

    //50
    const char *rand_suf_names[] = {
    "瞳",    //肉体の一部など
    "単眼",
    "牙",
    "顎門",
    "爪",
    "心臓",
    "翼",
    "双翼",
    "蜘蛛",  //生物など
    "蛇",
    "鷹",
    "隼",
    "猟犬",
    "狼",
    "獅子",
    "虎",
    "豹",
    "王",    //役割など
    "公子",
    "盟主",
    "道化",
    "聖者",
    "咎人",
    "護り",
    "護り手",
    "語り部",
    "徴",    //モノなど
    "紋章",
    "徽章",
    "刻印",
    "碑文",
    "遺物",
    "鎖",
    "咆哮",
    "旋律",
    "頌歌",
    "凱旋歌",
    "悪疫",
    "聖餐",
    "言霊",  //意思や儀式など
    "祈祷",
    "誓願",
    "祝福",
    "呪詛",
    "兇状",
    "処罰",
    "審判",
    "告示",
    "秘蹟",
    "宣誓",
    };
#endif


// Remember: disallow unrandart creation in abyss/pan

/*
   The following unrandart bits were taken from $pellbinder's mon-util code
   (see mon-util.h & mon-util.cc) and modified (LRH). They're in randart.cc and
   not randart.h because they're only used in this code module.
*/

#if defined(MAC) || defined(__IBMCPP__) || defined(__BCPLUSPLUS__)
#define PACKED
#else
#define PACKED __attribute__ ((packed))
#endif

//int unranddatasize;

#ifdef __IBMCPP__
#pragma pack(push)
#pragma pack(1)
#endif

struct unrandart_entry
{
    const char *name;        // true name of unrandart (max 31 chars)
    const char *unid_name;   // un-id'd name of unrandart (max 31 chars)

    int ura_cl;        // class of ura
    int ura_ty;        // type of ura
    int ura_pl;        // plus of ura
    int ura_pl2;       // plus2 of ura
    int ura_col;       // colour of ura
    short prpty[RA_PROPERTIES];

    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip1;
    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip2;
    // special description added to 'v' command output (max 31 chars)
    const char *spec_descrip3;
};

#ifdef __IBMCPP__
#pragma pack(pop)
#endif

static struct unrandart_entry unranddata[] = {
#include "unrand.h"
};

char *art_n;
static FixedVector < char, NO_UNRANDARTS > unrandart_exist;

static int random5( int randmax );
static struct unrandart_entry *seekunrandart( const item_def &item );

static int random5( int randmax )
{
    if (randmax <= 0)
        return (0);

    //return rand() % randmax;
    return ((int) rand() / (RAND_MAX / randmax + 1));
    // must use random (not rand) for the predictable-results-from-known
    //  -srandom-seeds thing to work.
}

void set_unrandart_exist(int whun, char is_exist)
{
    unrandart_exist[whun] = is_exist;
}

char does_unrandart_exist(int whun)
{
    return (unrandart_exist[whun]);
}

// returns true is item is a pure randart or an unrandart
bool is_random_artefact( const item_def &item )
{
    return (item.flags & ISFLAG_ARTEFACT_MASK);
}

// returns true if item in an unrandart
bool is_unrandom_artefact( const item_def &item )
{
    return (item.flags & ISFLAG_UNRANDART);
}

// returns true if item is one of the origional fixed artefacts
bool is_fixed_artefact( const item_def &item )
{
    if (!is_random_artefact( item )
        && item.base_type == OBJ_WEAPONS
        && item.special >= SPWPN_SINGING_SWORD)
    {
        return (true);
    }

    return (false);
}

int get_unique_item_status( int base_type, int art )
{
    // Note: for weapons "art" is in item.special,
    //       for orbs it's the sub_type.
    if (base_type == OBJ_WEAPONS)
    {
        if (art >= SPWPN_SINGING_SWORD && art <= SPWPN_SWORD_OF_ZONGULDROK)
            return (you.unique_items[ art - SPWPN_SINGING_SWORD ]);
        else if (art >= SPWPN_SWORD_OF_POWER && art <= SPWPN_STAFF_OF_WUCAD_MU)
            return (you.unique_items[ art - SPWPN_SWORD_OF_POWER + 24 ]);
    }
    else if (base_type == OBJ_ORBS)
    {
        if (art >= 4 && art <= 19)
            return (you.unique_items[ art + 3 ]);

    }

    return (UNIQ_NOT_EXISTS);
}

void set_unique_item_status( int base_type, int art, int status )
{
    // Note: for weapons "art" is in item.special,
    //       for orbs it's the sub_type.
    if (base_type == OBJ_WEAPONS)
    {
        if (art >= SPWPN_SINGING_SWORD && art <= SPWPN_SWORD_OF_ZONGULDROK)
            you.unique_items[ art - SPWPN_SINGING_SWORD ] = status;
        else if (art >= SPWPN_SWORD_OF_POWER && art <= SPWPN_STAFF_OF_WUCAD_MU)
            you.unique_items[ art - SPWPN_SWORD_OF_POWER + 24 ] = status;
    }
    else if (base_type == OBJ_ORBS)
    {
        if (art >= 4 && art <= 19)
            you.unique_items[ art + 3 ] = status;

    }
}

static long calc_seed( const item_def &item )
{
    return (item.special & RANDART_SEED_MASK);
}

void randart_wpn_properties( const item_def &item,
                             FixedVector< char, RA_PROPERTIES > &proprt )
{
    ASSERT( is_random_artefact( item ) );

    const int aclass = item.base_type;
    const int atype  = item.sub_type;

    int i = 0;
    int power_level = 0;

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        for (i = 0; i < RA_PROPERTIES; i++)
            proprt[i] = unrand->prpty[i];

        return;
    }

    // long seed = aclass * adam + atype * (aplus % 100) + aplus2 * 100;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (aclass == OBJ_ARMOUR)
        power_level = item.plus / 2 + 2;
    else if (aclass == OBJ_JEWELLERY)
        power_level = 1 + random5(3) + random5(2);
    else // OBJ_WEAPON
        power_level = item.plus / 3 + item.plus2 / 3;

    if (power_level < 0)
        power_level = 0;

    for (i = 0; i < RA_PROPERTIES; i++)
        proprt[i] = 0;

    if (aclass == OBJ_WEAPONS)  /* Only weapons get brands, of course */
    {
        proprt[RAP_BRAND] = SPWPN_FLAMING + random5(15);        /* brand */

        if (random5(6) == 0)
            proprt[RAP_BRAND] = SPWPN_FLAMING + random5(2);

        if (random5(6) == 0)
            proprt[RAP_BRAND] = SPWPN_ORC_SLAYING + random5(4);

        if (random5(6) == 0)
            proprt[RAP_BRAND] = SPWPN_VORPAL;

        if (proprt[RAP_BRAND] == SPWPN_FLAME
            || proprt[RAP_BRAND] == SPWPN_FROST)
        {
            proprt[RAP_BRAND] = 0;      /* missile wpns */
        }

        if (proprt[RAP_BRAND] == SPWPN_PROTECTION)
            proprt[RAP_BRAND] = 0;      /* no protection */

        if (proprt[RAP_BRAND] == SPWPN_DISRUPTION
            && !(atype == WPN_MACE || atype == WPN_GREAT_MACE
                || atype == WPN_HAMMER))
        {
            proprt[RAP_BRAND] = SPWPN_NORMAL;
        }

        // is this happens, things might get broken -- bwr
        if (proprt[RAP_BRAND] == SPWPN_SPEED && atype == WPN_QUICK_BLADE)
            proprt[RAP_BRAND] = SPWPN_NORMAL;

        if (launches_things(atype))
        {
            proprt[RAP_BRAND] = SPWPN_NORMAL;

            if (random5(3) == 0)
            {
                int tmp = random5(20);

                proprt[RAP_BRAND] = (tmp >= 18) ? SPWPN_SPEED :
                                    (tmp >= 14) ? SPWPN_PROTECTION :
                                    (tmp >= 10) ? SPWPN_VENOM
                                                : SPWPN_FLAME + (tmp % 2);
            }
        }


        if (is_demonic(atype))
        {
            switch (random5(9))
            {
            case 0:
                proprt[RAP_BRAND] = SPWPN_DRAINING;
                break;
            case 1:
                proprt[RAP_BRAND] = SPWPN_FLAMING;
                break;
            case 2:
                proprt[RAP_BRAND] = SPWPN_FREEZING;
                break;
            case 3:
                proprt[RAP_BRAND] = SPWPN_ELECTROCUTION;
                break;
            case 4:
                proprt[RAP_BRAND] = SPWPN_VAMPIRICISM;
                break;
            case 5:
                proprt[RAP_BRAND] = SPWPN_PAIN;
                break;
            case 6:
                proprt[RAP_BRAND] = SPWPN_VENOM;
                break;
            default:
                power_level -= 2;
            }
            power_level += 2;
        }
        else if (random5(3) == 0)
            proprt[RAP_BRAND] = SPWPN_NORMAL;
        else
            power_level++;
    }

    if (random5(5) == 0)
        goto skip_mods;

    /* AC mod - not for armours or rings of protection */
    if (random5(4 + power_level) == 0
        && aclass != OBJ_ARMOUR
        && (aclass != OBJ_JEWELLERY || atype != RING_PROTECTION))
    {
        proprt[RAP_AC] = 1 + random5(3) + random5(3) + random5(3);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_AC] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* ev mod - not for rings of evasion */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_EVASION))
    {
        proprt[RAP_EVASION] = 1 + random5(3) + random5(3) + random5(3);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_EVASION] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* str mod - not for rings of strength */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_STRENGTH))
    {
        proprt[RAP_STRENGTH] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_STRENGTH] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* int mod - not for rings of intelligence */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_INTELLIGENCE))
    {
        proprt[RAP_INTELLIGENCE] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_INTELLIGENCE] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    /* dex mod - not for rings of dexterity */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_DEXTERITY))
    {
        proprt[RAP_DEXTERITY] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_DEXTERITY] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

  skip_mods:
    if (random5(15) < power_level
        || aclass == OBJ_WEAPONS
        || (aclass == OBJ_JEWELLERY && atype == RING_SLAYING))
    {
        goto skip_combat;
    }

    /* Weapons and rings of slaying can't get these */
    if (random5(4 + power_level) == 0)  /* to-hit */
    {
        proprt[RAP_ACCURACY] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_ACCURACY] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

    if (random5(4 + power_level) == 0)  /* to-dam */
    {
        proprt[RAP_DAMAGE] = 1 + random5(3) + random5(2);
        power_level++;
        if (random5(4) == 0)
        {
            proprt[RAP_DAMAGE] -= 1 + random5(3) + random5(3) + random5(3);
            power_level--;
        }
    }

  skip_combat:
    if (random5(12) < power_level)
        goto finished_powers;

/* res_fire */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY
            || (atype != RING_PROTECTION_FROM_FIRE
                && atype != RING_FIRE
                && atype != RING_ICE))
        && (aclass != OBJ_ARMOUR
            || (atype != ARM_DRAGON_ARMOUR
                && atype != ARM_ICE_DRAGON_ARMOUR
                && atype != ARM_GOLD_DRAGON_ARMOUR)))
    {
        proprt[RAP_FIRE] = 1;
        if (random5(5) == 0)
            proprt[RAP_FIRE]++;
        power_level++;
    }

    /* res_cold */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY
            || (atype != RING_PROTECTION_FROM_COLD
                && atype != RING_FIRE
                && atype != RING_ICE))
        && (aclass != OBJ_ARMOUR
            || (atype != ARM_DRAGON_ARMOUR
                && atype != ARM_ICE_DRAGON_ARMOUR
                && atype != ARM_GOLD_DRAGON_ARMOUR)))
    {
        proprt[RAP_COLD] = 1;
        if (random5(5) == 0)
            proprt[RAP_COLD]++;
        power_level++;
    }

    if (random5(12) < power_level || power_level > 7)
        goto finished_powers;

    /* res_elec */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_ARMOUR || atype != ARM_STORM_DRAGON_ARMOUR))
    {
        proprt[RAP_ELECTRICITY] = 1;
        power_level++;
    }

    /* res_poison */
    if (random5(5 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_POISON_RESISTANCE)
        && (aclass != OBJ_ARMOUR
            || atype != ARM_GOLD_DRAGON_ARMOUR
            || atype != ARM_SWAMP_DRAGON_ARMOUR))
    {
        proprt[RAP_POISON] = 1;
        power_level++;
    }

    /* prot_life - no necromantic brands on weapons allowed */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_TELEPORTATION)
        && proprt[RAP_BRAND] != SPWPN_DRAINING
        && proprt[RAP_BRAND] != SPWPN_VAMPIRICISM
        && proprt[RAP_BRAND] != SPWPN_PAIN)
    {
        proprt[RAP_NEGATIVE_ENERGY] = 1;
        power_level++;
    }

    /* res magic */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_PROTECTION_FROM_MAGIC))
    {
        proprt[RAP_MAGIC] = 20 + random5(40);
        power_level++;
    }

    /* see_invis */
    if (random5(4 + power_level) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_SEE_INVISIBLE))
    {
        proprt[RAP_EYESIGHT] = 1;
        power_level++;
    }

    if (random5(12) < power_level || power_level > 10)
        goto finished_powers;

    /* turn invis */
    if (random5(10) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_INVISIBILITY))
    {
        proprt[RAP_INVISIBLE] = 1;
        power_level++;
    }

    /* levitate */
    if (random5(10) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_LEVITATION))
    {
        proprt[RAP_LEVITATE] = 1;
        power_level++;
    }

    if (random5(10) == 0)       /* blink */
    {
        proprt[RAP_BLINK] = 1;
        power_level++;
    }

    /* teleport */
    if (random5(10) == 0
        && (aclass != OBJ_JEWELLERY || atype != RING_TELEPORTATION))
    {
        proprt[RAP_CAN_TELEPORT] = 1;
        power_level++;
    }

    /* go berserk */
    if (random5(10) == 0 && (aclass != OBJ_JEWELLERY || atype != AMU_RAGE))
    {
        proprt[RAP_BERSERK] = 1;
        power_level++;
    }

    if (random5(10) == 0)       /* sense surr */
    {
        proprt[RAP_MAPPING] = 1;
        power_level++;
    }


  finished_powers:
    /* Armours get less powers, and are also less likely to be
       cursed that wpns */
    if (aclass == OBJ_ARMOUR)
        power_level -= 4;

    if (random5(17) >= power_level || power_level < 2)
        goto finished_curses;

    switch (random5(9))
    {
    case 0:                     /* makes noise */
        if (aclass != OBJ_WEAPONS)
            break;
        proprt[RAP_NOISES] = 1 + random5(4);
        break;
    case 1:                     /* no magic */
        proprt[RAP_PREVENT_SPELLCASTING] = 1;
        break;
    case 2:                     /* random teleport */
        if (aclass != OBJ_WEAPONS)
            break;
        proprt[RAP_CAUSE_TELEPORTATION] = 5 + random5(15);
        break;
    case 3:   /* no teleport - doesn't affect some instantaneous teleports */
        if (aclass == OBJ_JEWELLERY && atype == RING_TELEPORTATION)
            break;              /* already is a ring of tport */
        if (aclass == OBJ_JEWELLERY && atype == RING_TELEPORT_CONTROL)
            break;              /* already is a ring of tport ctrl */
        proprt[RAP_BLINK] = 0;
        proprt[RAP_CAN_TELEPORT] = 0;
        proprt[RAP_PREVENT_TELEPORTATION] = 1;
        break;
    case 4:                     /* berserk on attack */
        if (aclass != OBJ_WEAPONS)
            break;
        proprt[RAP_ANGRY] = 1 + random5(8);
        break;
    case 5:                     /* susceptible to fire */
        if (aclass == OBJ_JEWELLERY
            && (atype == RING_PROTECTION_FROM_FIRE || atype == RING_FIRE
                || atype == RING_ICE))
            break;              /* already does this or something */
        if (aclass == OBJ_ARMOUR
            && (atype == ARM_DRAGON_ARMOUR || atype == ARM_ICE_DRAGON_ARMOUR
                || atype == ARM_GOLD_DRAGON_ARMOUR))
            break;
        proprt[RAP_FIRE] = -1;
        break;
    case 6:                     /* susceptible to cold */
        if (aclass == OBJ_JEWELLERY
            && (atype == RING_PROTECTION_FROM_COLD || atype == RING_FIRE
                || atype == RING_ICE))
            break;              /* already does this or something */
        if (aclass == OBJ_ARMOUR
            && (atype == ARM_DRAGON_ARMOUR || atype == ARM_ICE_DRAGON_ARMOUR
                || atype == ARM_GOLD_DRAGON_ARMOUR))
            break;
        proprt[RAP_COLD] = -1;
        break;
    case 7:                     /* speed metabolism */
        if (aclass == OBJ_JEWELLERY && atype == RING_HUNGER)
            break;              /* already is a ring of hunger */
        if (aclass == OBJ_JEWELLERY && atype == RING_SUSTENANCE)
            break;              /* already is a ring of sustenance */
        proprt[RAP_METABOLISM] = 1 + random5(3);
        break;
    case 8:   /* emits mutagenic radiation - increases magic_contamination */
        /* property is chance (1 in ...) of increasing magic_contamination */
        proprt[RAP_MUTAGENIC] = 2 + random5(4);
        break;
    }

/*
   26 - +to-hit (no wpns)
   27 - +to-dam (no wpns)
 */

finished_curses:
    if (random5(10) == 0
        && (aclass != OBJ_ARMOUR
            || atype != ARM_CLOAK
            || !cmp_equip_race( item, ISFLAG_ELVEN ))
        && (aclass != OBJ_ARMOUR
            || atype != ARM_BOOTS
            || !cmp_equip_race( item, ISFLAG_ELVEN )
        && get_armour_ego_type( item ) != SPARM_STEALTH))
    {
        power_level++;
        proprt[RAP_STEALTH] = 10 + random5(70);

        if (random5(4) == 0)
        {
            proprt[RAP_STEALTH] = -proprt[RAP_STEALTH] - random5(20);
            power_level--;
        }
    }

    if ((power_level < 2 && random5(5) == 0) || random5(30) == 0)
        proprt[RAP_CURSED] = 1;

    srand(randstore);

}

int randart_wpn_property( const item_def &item, char prop )
{
    FixedVector< char, RA_PROPERTIES > proprt;

    randart_wpn_properties( item, proprt );

    return (proprt[prop]);
}

const char *randart_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_WEAPONS );

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_ident(item, ISFLAG_KNOW_TYPE) ? unrand->name
                                                   : unrand->unid_name);
    }

    free(art_n);
    art_n = (char *) malloc(sizeof(char) * 80);

    if (art_n == NULL)
#ifdef JP
        return ("Malloc失敗のエラーです。");
#else
        return ("Malloc Failed Error");
#endif

    strcpy(art_n, "");

    // long seed = aclass + adam * (aplus % 100) + atype * aplus2;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (random5(21))
        {
#ifdef JP
        case  0: strcat(art_n, "明るく輝く"); break;
        case  1: strcat(art_n, "ルーンが刻まれた"); break;
        case  2: strcat(art_n, "煙を上げる"); break;
        case  3: strcat(art_n, "血に染まった"); break;
        case  4: strcat(art_n, "捻じ曲がった"); break;
        case  5: strcat(art_n, "チカチカ光る"); break;
        case  6: strcat(art_n, "歪んだ"); break;
        case  7: strcat(art_n, "水晶の"); break;
        case  8: strcat(art_n, "宝石で飾られた"); break;
        case  9: strcat(art_n, "透明な"); break;
        case 10: strcat(art_n, "装飾された"); break;
        case 11: strcat(art_n, "穴だらけの"); break;
        case 12: strcat(art_n, "ぬるぬるした"); break;
        case 13: strcat(art_n, "磨き上げられた"); break;
        case 14: strcat(art_n, "豪華な"); break;
        case 15: strcat(art_n, "粗末な"); break;
        case 16: strcat(art_n, "古ぼけた"); break;
        case 17: strcat(art_n, "膿のついた"); break;
        case 18: strcat(art_n, "弱く輝く"); break;
        case 19: strcat(art_n, "蒸気を上げる"); break;
        case 20: strcat(art_n, "光る"); break;
#else
        case  0: strcat(art_n, "brightly glowing "); break;
        case  1: strcat(art_n, "runed "); break;
        case  2: strcat(art_n, "smoking "); break;
        case  3: strcat(art_n, "bloodstained "); break;
        case  4: strcat(art_n, "twisted "); break;
        case  5: strcat(art_n, "shimmering "); break;
        case  6: strcat(art_n, "warped "); break;
        case  7: strcat(art_n, "crystal "); break;
        case  8: strcat(art_n, "jewelled "); break;
        case  9: strcat(art_n, "transparent "); break;
        case 10: strcat(art_n, "encrusted "); break;
        case 11: strcat(art_n, "pitted "); break;
        case 12: strcat(art_n, "slimy "); break;
        case 13: strcat(art_n, "polished "); break;
        case 14: strcat(art_n, "fine "); break;
        case 15: strcat(art_n, "crude "); break;
        case 16: strcat(art_n, "ancient "); break;
        case 17: strcat(art_n, "ichor-stained "); break;
        case 18: strcat(art_n, "faintly glowing "); break;
        case 19: strcat(art_n, "steaming "); break;
        case 20: strcat(art_n, "shiny "); break;
#endif
        }

        char st_p3[ITEMNAME_SIZE];

        standard_name_weap( item.sub_type, st_p3 );
        strcat(art_n, st_p3);
        srand(randstore);
        return (art_n);
    }

    char st_p[ITEMNAME_SIZE];
#ifdef JP
    //if (random5(5) >= 2)
    if (random5(2) == 0)
    {
        standard_name_weap( item.sub_type, st_p );
        strcpy(art_n, "" );

//日本語版ではアーティファクト修飾詞の語順を操作
        int rand_name_no;
        rand_name_no = random5(390);
//340番までは「○○の剣」それ以降は「剣『○○』」にする。
        if ( rand_name_no < 340 )
        {
        strcat(art_n, rand_wpn_names[rand_name_no]);
        strcat(art_n, st_p);
        }
        else
        {
        strcat(art_n, st_p);
        strcat(art_n, rand_wpn_names[rand_name_no]);
        }
    }
#else
    if (random5(2) == 0)
    {
        standard_name_weap( item.sub_type, st_p );
        strcat(art_n, st_p);
        strcat(art_n, rand_wpn_names[random5(390)]);
    }
#endif
    else
    {
        char st_p2[ITEMNAME_SIZE];
        strcpy(st_p2, "" );
#ifdef JP
        strcpy( st_p, rand_pre_names[ random5(50) ] );
        strcat( st_p, rand_suf_names[ random5(50) ] );
#else
        make_name(random5(250), random5(250), random5(250), 3, st_p);
#endif
        standard_name_weap( item.sub_type, st_p2 );
        strcpy(art_n, "" );

        if (random5(3) == 5) //(random5(3) == 0)
        {
#ifdef JP
            strcat(art_n, "『");
            strcat(art_n, st_p);
            strcat(art_n, "』");
            strcat(art_n, "の");
            strcat(art_n, st_p2);
#else
            strcat(art_n, " of ");
            strcat(art_n, st_p);
#endif

        }
        else
        {
            strcat(art_n, st_p2);
#ifdef JP
            strcat(art_n, "『");
#else
            strcat(art_n, " \"");
#endif
            strcat(art_n, st_p);
#ifdef JP
            strcat(art_n, "』");
#else
            strcat(art_n, "\"");
#endif
        }
    }

    srand(randstore);

    return (art_n);
}

const char *randart_armour_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_ARMOUR );

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_ident(item, ISFLAG_KNOW_TYPE) ? unrand->name
                                                   : unrand->unid_name);
    }

    free(art_n);
    art_n = (char *) malloc(sizeof(char) * 80);

    if (art_n == NULL)
    {
#ifdef JP
        return ("Malloc失敗のエラーです。");
#else
        return ("Malloc Failed Error");
#endif
    }

    strcpy(art_n, "");

    // long seed = aclass + adam * (aplus % 100) + atype * aplus2;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (random5(21))
        {
#ifdef JP
        case  0: strcat(art_n, "明るく輝く"); break;
        case  1: strcat(art_n, "ルーンが刻まれた"); break;
        case  2: strcat(art_n, "煙を上げる"); break;
        case  3: strcat(art_n, "血に染まった"); break;
        case  4: strcat(art_n, "捻じ曲がった"); break;
        case  5: strcat(art_n, "チカチカ光る"); break;
        case  6: strcat(art_n, "歪んだ"); break;
        case  7: strcat(art_n, "ルーンに埋め尽された"); break;
        case  8: strcat(art_n, "宝石で飾られた"); break;
        case  9: strcat(art_n, "透明な"); break;
        case 10: strcat(art_n, "装飾された"); break;
        case 11: strcat(art_n, "穴だらけの"); break;
        case 12: strcat(art_n, "ぬるぬるした"); break;
        case 13: strcat(art_n, "磨き上げられた"); break;
        case 14: strcat(art_n, "豪華な"); break;
        case 15: strcat(art_n, "粗末な"); break;
        case 16: strcat(art_n, "古ぼけた"); break;
        case 17: strcat(art_n, "膿のついた"); break;
        case 18: strcat(art_n, "弱く輝く"); break;
        case 19: strcat(art_n, "蒸気を上げる"); break;
        case 20: strcat(art_n, "光る"); break;
#else
        case  0: strcat(art_n, "brightly glowing "); break;
        case  1: strcat(art_n, "runed "); break;
        case  2: strcat(art_n, "smoking "); break;
        case  3: strcat(art_n, "bloodstained "); break;
        case  4: strcat(art_n, "twisted "); break;
        case  5: strcat(art_n, "shimmering "); break;
        case  6: strcat(art_n, "warped "); break;
        case  7: strcat(art_n, "heavily runed "); break;
        case  8: strcat(art_n, "jeweled "); break;
        case  9: strcat(art_n, "transparent "); break;
        case 10: strcat(art_n, "encrusted "); break;
        case 11: strcat(art_n, "pitted "); break;
        case 12: strcat(art_n, "slimy "); break;
        case 13: strcat(art_n, "polished "); break;
        case 14: strcat(art_n, "fine "); break;
        case 15: strcat(art_n, "crude "); break;
        case 16: strcat(art_n, "ancient "); break;
        case 17: strcat(art_n, "ichor-stained "); break;
        case 18: strcat(art_n, "faintly glowing "); break;
        case 19: strcat(art_n, "steaming "); break;
        case 20: strcat(art_n, "shiny "); break;
#endif
        }
        char st_p3[ITEMNAME_SIZE];

        standard_name_armour(item, st_p3);
        strcat(art_n, st_p3);
        srand(randstore);
        return (art_n);
    }

    char st_p[ITEMNAME_SIZE];

    if (random5(2) == 0)
    {
        standard_name_armour(item, st_p);
#ifdef JP
//日本語版ではアーティファクト修飾詞の語順を入れ替える
        strcpy(art_n, "" );
        strcat(art_n, rand_armour_names[random5(71)]);
        strcat(art_n, st_p );
#else
        strcpy(art_n, st_p);
        strcat(art_n, rand_armour_names[random5(71)]);
#endif
    }
    else
    {
        char st_p2[ITEMNAME_SIZE];
        strcpy(st_p2, "" );
#ifdef JP
        strcpy( st_p, rand_pre_names[ random5(50) ] );
        strcat( st_p, rand_suf_names[ random5(50) ] );
#else
        make_name(random5(250), random5(250), random5(250), 3, st_p);
#endif
        standard_name_armour(item, st_p2);
        strcpy(art_n, "" );

        if (random5(3) == 5) //(random5(3) == 0)
        {
#ifdef JP
            strcat(art_n, "『");
            strcat(art_n, st_p);
            strcat(art_n, "』");
            strcat(art_n, "の");
            strcat(art_n, st_p2);
#else
            strcat(art_n, " of ");
            strcat(art_n, st_p);
#endif

        }
        else
        {
            strcat(art_n, st_p2);
#ifdef JP
            strcat(art_n, "『");
#else
            strcat(art_n, " \"");
#endif
            strcat(art_n, st_p);
#ifdef JP
            strcat(art_n, "』");
#else
            strcat(art_n, "\"");
#endif
        }
    }

    srand(randstore);

    return (art_n);
}

const char *randart_ring_name( const item_def &item )
{
    ASSERT( item.base_type == OBJ_JEWELLERY );

    int temp_rand = 0;          // probability determination {dlb}

    if (is_unrandom_artefact( item ))
    {
        struct unrandart_entry *unrand = seekunrandart( item );

        return (item_ident(item, ISFLAG_KNOW_TYPE) ? unrand->name
                                                   : unrand->unid_name);
    }

    char st_p[ITEMNAME_SIZE];
#ifdef JP
    char chors[ITEMNAME_SIZE];
#endif
    free(art_n);
    art_n = (char *) malloc(sizeof(char) * 80);

    if (art_n == NULL)
#ifdef JP
        return ("Malloc失敗のエラーです。");
#else
        return ("Malloc Failed Error");
#endif

    strcpy(art_n, "");

    // long seed = aclass + adam * (aplus % 100) + atype * aplus2;
    long seed = calc_seed( item );
    long randstore = rand();
    srand( seed );

    if (item_not_ident( item, ISFLAG_KNOW_TYPE ))
    {
        temp_rand = random5(21);

#ifdef JP
        strcat(art_n,  (temp_rand == 0)  ? "明るく輝く" :
                       (temp_rand == 1)  ? "ルーンが刻まれた" :
                       (temp_rand == 2)  ? "煙を上げる" :
                       (temp_rand == 3)  ? "ルビーの" :
                       (temp_rand == 4)  ? "捻じ曲がった" :
                       (temp_rand == 5)  ? "チカチカ光る" :
                       (temp_rand == 6)  ? "歪んだ" :
                       (temp_rand == 7)  ? "水晶の" :
                       (temp_rand == 8)  ? "ダイアモンドの" :
                       (temp_rand == 9)  ? "透明な" :
                       (temp_rand == 10) ? "装飾された" :
                       (temp_rand == 11) ? "穴だらけの" :
                       (temp_rand == 12) ? "ぬるぬるした" :
                       (temp_rand == 13) ? "磨き上げられた" :
                       (temp_rand == 14) ? "豪華な" :
                       (temp_rand == 15) ? "粗末な" :
                       (temp_rand == 16) ? "古ぼけた" :
                       (temp_rand == 17) ? "エメラルドの" :
                       (temp_rand == 18) ? "弱く輝く" :
                       (temp_rand == 19) ? "蒸気を上げる"
                                         : "光る");
#else
        strcat(art_n,  (temp_rand == 0)  ? "brightly glowing" :
                       (temp_rand == 1)  ? "runed" :
                       (temp_rand == 2)  ? "smoking" :
                       (temp_rand == 3)  ? "ruby" :
                       (temp_rand == 4)  ? "twisted" :
                       (temp_rand == 5)  ? "shimmering" :
                       (temp_rand == 6)  ? "warped" :
                       (temp_rand == 7)  ? "crystal" :
                       (temp_rand == 8)  ? "diamond" :
                       (temp_rand == 9)  ? "transparent" :
                       (temp_rand == 10) ? "encrusted" :
                       (temp_rand == 11) ? "pitted" :
                       (temp_rand == 12) ? "slimy" :
                       (temp_rand == 13) ? "polished" :
                       (temp_rand == 14) ? "fine" :
                       (temp_rand == 15) ? "crude" :
                       (temp_rand == 16) ? "ancient" :
                       (temp_rand == 17) ? "emerald" :
                       (temp_rand == 18) ? "faintly glowing" :
                       (temp_rand == 19) ? "steaming"
                                         : "shiny");
#endif

#ifdef JP
        strcat(art_n, (item.sub_type < AMU_RAGE) ? "指輪" : "護符");
#else
        strcat(art_n, " ");
        strcat(art_n, (item.sub_type < AMU_RAGE) ? "ring" : "amulet");
#endif
        srand(randstore);
        return (art_n);
    }

    if (random5(5) == 0)
    {
        strcpy(art_n, "" );
#ifdef JP
        strcpy(st_p, (item.sub_type < AMU_RAGE) ? "指輪" : "護符");
#else
        strcpy(st_p, (item.sub_type < AMU_RAGE) ? "ring" : "amulet");
#endif
#ifdef JP
//日本語版ではアーティファクト修飾詞の語順を入れ替える
        strcpy(art_n, rand_armour_names[random5(71)]);
        strcat(art_n, st_p );
#else
        strcpy(art_n, st_p );
        strcat(art_n, rand_armour_names[random5(71)]);
#endif
    }
//ここから呪文風のランダム名
    else
    {
        char st_p2[ITEMNAME_SIZE];
        strcpy(st_p2, "" );
#ifdef JP
        strcpy( st_p, rand_pre_names[ random5(50) ] );
        strcat( st_p, rand_suf_names[ random5(50) ] );
        strcpy( st_p2, (item.sub_type < AMU_RAGE) ? "指輪" : "護符");
#else
        make_name(random5(250), random5(250), random5(250), 3, st_p);
        strcat(st_p2, (item.sub_type < AMU_RAGE) ? "ring" : "amulet");
#endif
        strcpy(art_n, "" );

        if (random5(3) == 5) //(random5(3) == 0)
        {
#ifdef JP
            strcat(art_n, "『");
            strcat(art_n, st_p);
            strcat(art_n, "』");
            strcat(art_n, "の");
            strcat(art_n, st_p2);
#else
            strcat(art_n, " of ");
            strcat(art_n, st_p);
#endif
        }
        else
        {
            strcat(art_n, st_p2);
#ifdef JP
            strcat(art_n, "『");
#else
            strcat(art_n, " \"");
#endif
            strcat(art_n, st_p);
#ifdef JP
            strcat(art_n, "』");
#else
            strcat(art_n, "\"");
#endif
        }
    }
    srand(randstore);
    return (art_n);
}
// end randart_ring_name()

static struct unrandart_entry *seekunrandart( const item_def &item )
{
    int x = 0;

    while (x < NO_UNRANDARTS)
    {
        if (unranddata[x].ura_cl == item.base_type
            && unranddata[x].ura_ty == item.sub_type
            && unranddata[x].ura_pl == item.plus
            && unranddata[x].ura_pl2 == item.plus2)
        {
            return (&unranddata[x]);
        }

        x++;
    }

    return (&unranddata[0]);  // Dummy object
}                               // end seekunrandart()
#if 1
int find_unrandart_index2(const item_def &item)
{
    static int cache_head = -1;
    static int cache_next[NO_UNRANDARTS];

    int x, prev;

    /* Init Cache */
    if (cache_head == -1)
    {
        cache_head = 0;
        for (x = 0; x <NO_UNRANDARTS-1; x++)
        {
            cache_next[x] = x + 1;
        }
        cache_next[NO_UNRANDARTS-1] = -1;
    }

    /* Search Cache */
    x = cache_head;
    prev = -1;

    while(x != -1)
    {
        int next = cache_next[x];

        if (unranddata[x].ura_cl == item.base_type
            && unranddata[x].ura_ty == item.sub_type
            && unranddata[x].ura_pl == item.plus
            && unranddata[x].ura_pl2 == item.plus2)
        {
            if (x != cache_head)
            {
                cache_next[x] = cache_head;
                cache_head = x;
                cache_next[prev] = next;
            }
            return (x);
        }
        prev = x;
        x = next;
    }

    return (-1);
}

int find_unrandart_index(int item_number)
{
  return find_unrandart_index2(mitm[item_number]);
}
#else

int find_unrandart_index(int item_number)
{
    int x;

    for(x=0; x < NO_UNRANDARTS; x++)
    {
        if (unranddata[x].ura_cl == mitm[item_number].base_type
            && unranddata[x].ura_ty == mitm[item_number].sub_type
            && unranddata[x].ura_pl == mitm[item_number].plus
            && unranddata[x].ura_pl2 == mitm[item_number].plus2)
        {
            return (x);
        }
    }

    return (-1);
}
#endif

int find_okay_unrandart(unsigned char aclass, unsigned char atype)
{
    int x, count;
    int ret = -1;

    for (x = 0, count = 0; x < NO_UNRANDARTS; x++)
    {
        if (unranddata[x].ura_cl == aclass
            && does_unrandart_exist(x) == 0
            && (atype == OBJ_RANDOM || unranddata[x].ura_ty == atype))
        {
            count++;

            if (random5(count) == 0)
                ret = x;
        }
    }

    return (ret);
}                               // end find_okay_unrandart()

// which == 0 (default) gives random fixed artefact.
// Returns true if successful.
bool make_item_fixed_artefact( item_def &item, bool in_abyss, int which )
{
    bool  force = true;  // we force any one asked for specifically

    if (!which)
    {
        // using old behaviour... try only once. -- bwr
        force = false;

        which = SPWPN_SINGING_SWORD + random2(12);
        if (which >= SPWPN_SWORD_OF_CEREBOV)
            which += 3; // skip over Cerebov's, Dispater's, and Asmodeus' weapons
    }

    int status = get_unique_item_status( OBJ_WEAPONS, which );

    if ((status == UNIQ_EXISTS
            || (in_abyss && status == UNIQ_NOT_EXISTS)
            || (!in_abyss && status == UNIQ_LOST_IN_ABYSS))
        && !force)
    {
        return (false);
    }

    switch (which)
    {
    case SPWPN_SINGING_SWORD:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_LONG_SWORD;
        item.plus  = 7;
        item.plus2 = 6;
        break;

    case SPWPN_WRATH_OF_TROG:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_BATTLEAXE;
        item.plus  = 3;
        item.plus2 = 11;
        break;

    case SPWPN_SCYTHE_OF_CURSES:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_SCYTHE;
        item.plus  = 13;
        item.plus2 = 13;
        break;

    case SPWPN_MACE_OF_VARIABILITY:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_MACE;
        item.plus  = random2(16) - 4;
        item.plus2 = random2(16) - 4;
        break;

    case SPWPN_GLAIVE_OF_PRUNE:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GLAIVE;
        item.plus  = 0;
        item.plus2 = 12;
        break;

    case SPWPN_SCEPTRE_OF_TORMENT:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_MACE;
        item.plus  = 7;
        item.plus2 = 6;
        break;

    case SPWPN_SWORD_OF_ZONGULDROK:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_LONG_SWORD;
        item.plus  = 9;
        item.plus2 = 9;
        break;

    case SPWPN_SWORD_OF_POWER:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GREAT_SWORD;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_KNIFE_OF_ACCURACY:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_DAGGER;
        item.plus  = 27;
        item.plus2 = -1;
        break;

    case SPWPN_STAFF_OF_OLGREB:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_VAMPIRES_TOOTH:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_DAGGER;
        item.plus  = 3;
        item.plus2 = 4;
        break;

    case SPWPN_STAFF_OF_WUCAD_MU:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 0; // set on wield
        item.plus2 = 0; // set on wield
        break;

    case SPWPN_SWORD_OF_CEREBOV:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_GREAT_SWORD;
        item.plus  = 6;
        item.plus2 = 6;
        item.colour = YELLOW;
        do_curse_item( item );
        break;

    case SPWPN_STAFF_OF_DISPATER:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 4;
        item.plus2 = 4;
        item.colour = YELLOW;
        break;

    case SPWPN_SCEPTRE_OF_ASMODEUS:
        item.base_type = OBJ_WEAPONS;
        item.sub_type = WPN_QUARTERSTAFF;
        item.plus  = 7;
        item.plus2 = 7;
        item.colour = RED;
        break;

    default:
#ifdef JP
        DEBUGSTR( "非合法に改造されたアーティファクトを製造します！" );
#else
        DEBUGSTR( "Trying to create illegal fixed artefact!" );
#endif
        return (false);
    }

    // If we get here, we've made the artefact
    item.special = which;
    item.quantity = 1;

    // Items originally generated in the abyss and not found will be
    // shifted to "lost in abyss", and will only be found there. -- bwr
    set_unique_item_status( OBJ_WEAPONS, which, UNIQ_EXISTS );

    return (true);
}

bool make_item_randart( item_def &item )
{
    if (item.base_type != OBJ_WEAPONS
        && item.base_type != OBJ_ARMOUR
        && item.base_type != OBJ_JEWELLERY)
    {
        return (false);
    }

    item.flags |= ISFLAG_RANDART;
    item.special = (random() & RANDART_SEED_MASK);

    return (true);
}

// void make_item_unrandart( int x, int ura_item )
bool make_item_unrandart( item_def &item, int unrand_index )
{
    item.base_type = unranddata[unrand_index].ura_cl;
    item.sub_type  = unranddata[unrand_index].ura_ty;
    item.plus      = unranddata[unrand_index].ura_pl;
    item.plus2     = unranddata[unrand_index].ura_pl2;
    item.colour    = unranddata[unrand_index].ura_col;

    item.flags |= ISFLAG_UNRANDART;
    item.special = unranddata[ unrand_index ].prpty[ RAP_BRAND ];

    if (unranddata[ unrand_index ].prpty[ RAP_CURSED ])
        do_curse_item( item );

    set_unrandart_exist( unrand_index, 1 );

    return (true);
}                               // end make_item_unrandart()

const char *unrandart_descrip( char which_descrip, const item_def &item )
{
/* Eventually it would be great to have randomly generated descriptions for
   randarts. */
    struct unrandart_entry *unrand = seekunrandart( item );

    return ((which_descrip == 0) ? unrand->spec_descrip1 :
            (which_descrip == 1) ? unrand->spec_descrip2 :
#ifdef JP
            (which_descrip == 2) ? unrand->spec_descrip3 : "未判明。");
#else
            (which_descrip == 2) ? unrand->spec_descrip3 : "Unknown.");
#endif

}                               // end unrandart_descrip()

void standard_name_weap(unsigned char item_typ, char glorg[ITEMNAME_SIZE])
{
#ifdef JP
//装備の名前は8文字におさめたい。トライデントと悪魔の三叉戟の整合性は我慢。
    strcpy(glorg,  (item_typ == WPN_CLUB) ? "棍棒" :
                   (item_typ == WPN_MACE) ? "メイス" :
                   (item_typ == WPN_FLAIL) ? "フレイル" :
                   (item_typ == WPN_KNIFE) ? "ナイフ" :
                   (item_typ == WPN_DAGGER) ? "ダガー" :
                   (item_typ == WPN_MORNINGSTAR) ? "モーニングスター" :
                   (item_typ == WPN_SHORT_SWORD) ? "ショートソード" :
                   (item_typ == WPN_LONG_SWORD) ? "ロングソード" :
                   (item_typ == WPN_GREAT_SWORD) ? "グレートソード" :
                   (item_typ == WPN_SCIMITAR) ? "シミター" :
                   (item_typ == WPN_HAND_AXE) ? "ハンドアックス" :
                   (item_typ == WPN_BATTLEAXE) ? "バトルアックス" :
                   (item_typ == WPN_SPEAR) ? "スピア" :
                   (item_typ == WPN_TRIDENT) ? "トライデント" :
                   (item_typ == WPN_HALBERD) ? "ハルバード" :
                   (item_typ == WPN_SLING) ? "スリング" :
                   (item_typ == WPN_BOW) ? "弓" :
                   (item_typ == WPN_BLOWGUN) ? "吹き矢筒" :
                   (item_typ == WPN_CROSSBOW) ? "クロスボウ" :
                   (item_typ == WPN_HAND_CROSSBOW) ? "小型クロスボウ" :
                   (item_typ == WPN_GLAIVE) ? "グレイブ" :
                   (item_typ == WPN_QUARTERSTAFF) ? "六尺棒" :
                   (item_typ == WPN_SCYTHE) ? "大鎌" :
                   (item_typ == WPN_EVENINGSTAR) ? "イブニングスター" :
                   (item_typ == WPN_QUICK_BLADE) ? "クイックブレード" :
                   (item_typ == WPN_KATANA) ? "カタナ" :
                   (item_typ == WPN_EXECUTIONERS_AXE) ? "処刑人の斧" :
                   (item_typ == WPN_DOUBLE_SWORD) ? "ダブルソード" :
                   (item_typ == WPN_TRIPLE_SWORD) ? "トリプルソード" :
                   (item_typ == WPN_HAMMER) ? "ハンマー" :
                   (item_typ == WPN_ANCUS) ? "アンクス" :
                   (item_typ == WPN_WHIP) ? "鞭" :
                   (item_typ == WPN_SABRE) ? "サーベル" :
                   (item_typ == WPN_DEMON_BLADE) ? "悪魔の刃" :
                   (item_typ == WPN_DEMON_WHIP) ? "悪魔の鞭" :
                   (item_typ == WPN_DEMON_TRIDENT) ? "悪魔の三叉戟" :
                   (item_typ == WPN_BROAD_AXE) ? "ブロードアックス" :
                   (item_typ == WPN_WAR_AXE) ? "ウォーアックス" :
                   (item_typ == WPN_SPIKED_FLAIL) ? "釘つきフレイル" :
                   (item_typ == WPN_GREAT_MACE) ? "大型メイス" :
                   (item_typ == WPN_GREAT_FLAIL) ? "大型フレイル" :
                   (item_typ == WPN_FALCHION) ? "ファルシオン" :

           (item_typ == WPN_GIANT_CLUB)
                           ? (SysEnv.board_with_nail ? "角材"
                                                     : "巨大棍棒") :

           (item_typ == WPN_GIANT_SPIKED_CLUB)
                           ? (SysEnv.board_with_nail ? "釘つき角材"
                                                     : "釘つき巨大棍棒")

                                   : "unknown weapon");
#else
    strcpy(glorg,  (item_typ == WPN_CLUB) ? "club" :
                   (item_typ == WPN_MACE) ? "mace" :
                   (item_typ == WPN_FLAIL) ? "flail" :
                   (item_typ == WPN_KNIFE) ? "knife" :
                   (item_typ == WPN_DAGGER) ? "dagger" :
                   (item_typ == WPN_MORNINGSTAR) ? "morningstar" :
                   (item_typ == WPN_SHORT_SWORD) ? "short sword" :
                   (item_typ == WPN_LONG_SWORD) ? "long sword" :
                   (item_typ == WPN_GREAT_SWORD) ? "great sword" :
                   (item_typ == WPN_SCIMITAR) ? "scimitar" :
                   (item_typ == WPN_HAND_AXE) ? "hand axe" :
                   (item_typ == WPN_BATTLEAXE) ? "battleaxe" :
                   (item_typ == WPN_SPEAR) ? "spear" :
                   (item_typ == WPN_TRIDENT) ? "trident" :
                   (item_typ == WPN_HALBERD) ? "halberd" :
                   (item_typ == WPN_SLING) ? "sling" :
                   (item_typ == WPN_BOW) ? "bow" :
                   (item_typ == WPN_BLOWGUN) ? "blowgun" :
                   (item_typ == WPN_CROSSBOW) ? "crossbow" :
                   (item_typ == WPN_HAND_CROSSBOW) ? "hand crossbow" :
                   (item_typ == WPN_GLAIVE) ? "glaive" :
                   (item_typ == WPN_QUARTERSTAFF) ? "quarterstaff" :
                   (item_typ == WPN_SCYTHE) ? "scythe" :
                   (item_typ == WPN_EVENINGSTAR) ? "eveningstar" :
                   (item_typ == WPN_QUICK_BLADE) ? "quick blade" :
                   (item_typ == WPN_KATANA) ? "katana" :
                   (item_typ == WPN_EXECUTIONERS_AXE) ? "executioner's axe" :
                   (item_typ == WPN_DOUBLE_SWORD) ? "double sword" :
                   (item_typ == WPN_TRIPLE_SWORD) ? "triple sword" :
                   (item_typ == WPN_HAMMER) ? "hammer" :
                   (item_typ == WPN_ANCUS) ? "ancus" :
                   (item_typ == WPN_WHIP) ? "whip" :
                   (item_typ == WPN_SABRE) ? "sabre" :
                   (item_typ == WPN_DEMON_BLADE) ? "demon blade" :
                   (item_typ == WPN_DEMON_WHIP) ? "demon whip" :
                   (item_typ == WPN_DEMON_TRIDENT) ? "demon trident" :
                   (item_typ == WPN_BROAD_AXE) ? "broad axe" :
                   (item_typ == WPN_WAR_AXE) ? "war axe" :
                   (item_typ == WPN_SPIKED_FLAIL) ? "spiked flail" :
                   (item_typ == WPN_GREAT_MACE) ? "great mace" :
                   (item_typ == WPN_GREAT_FLAIL) ? "great flail" :
                   (item_typ == WPN_FALCHION) ? "falchion" :

           (item_typ == WPN_GIANT_CLUB)
                           ? (SysEnv.board_with_nail ? "two-by-four"
                                                     : "giant club") :

           (item_typ == WPN_GIANT_SPIKED_CLUB)
                           ? (SysEnv.board_with_nail ? "board with nail"
                                                     : "giant spiked club")

                                   : "unknown weapon");
#endif
}                               // end standard_name_weap()

void standard_name_armour( const item_def &item, char glorg[ITEMNAME_SIZE] )
{
    short helm_type;

    glorg[0] = '\0';

    switch (item.sub_type)
    {
    case ARM_ROBE:
#ifdef JP
        strcat(glorg, "ローブ");
#else
        strcat(glorg, "robe");
#endif
        break;

    case ARM_LEATHER_ARMOUR:
#ifdef JP
        strcat(glorg, "レザーアーマー");
#else
        strcat(glorg, "leather armour");
#endif
        break;

    case ARM_RING_MAIL:
#ifdef JP
        strcat(glorg, "リングメイル");
#else
        strcat(glorg, "ring mail");
#endif
        break;

    case ARM_SCALE_MAIL:
#ifdef JP
        strcat(glorg, "スケイルメイル");
#else
        strcat(glorg, "scale mail");
#endif
        break;

    case ARM_CHAIN_MAIL:
#ifdef JP
        strcat(glorg, "鎖かたびら");
#else
        strcat(glorg, "chain mail");
#endif
        break;

    case ARM_SPLINT_MAIL:
#ifdef JP
        strcat(glorg, "スプリントメイル");
#else
        strcat(glorg, "splint mail");
#endif
        break;

    case ARM_BANDED_MAIL:
#ifdef JP
        strcat(glorg, "バンディドメイル");
#else
        strcat(glorg, "banded mail");
#endif
        break;

    case ARM_PLATE_MAIL:
#ifdef JP
        strcat(glorg, "プレートメイル");
#else
        strcat(glorg, "plate mail");
#endif
        break;

    case ARM_SHIELD:
#ifdef JP
        strcat(glorg, "盾");
#else
        strcat(glorg, "shield");
#endif
        break;

    case ARM_CLOAK:
#ifdef JP
        strcat(glorg, "クローク");
#else
        strcat(glorg, "cloak");
#endif
        break;

    case ARM_HELMET:
        if (cmp_helmet_type( item, THELM_HELM )
                    || cmp_helmet_type( item, THELM_HELMET ))
        {
            short dhelm = get_helmet_desc( item );

            if (dhelm != THELM_DESC_PLAIN)
            {
#ifdef JP
                strcat( glorg,
                        (dhelm == THELM_DESC_WINGED)   ? "羽飾り" :
                        (dhelm == THELM_DESC_HORNED)   ? "角つき" :
                        (dhelm == THELM_DESC_CRESTED)  ? "天頂飾り" :
                        (dhelm == THELM_DESC_PLUMED)   ? "飾りつき" :
                        (dhelm == THELM_DESC_SPIKED)   ? "棘つき" :
                        (dhelm == THELM_DESC_VISORED)  ? "庇しつき" :
                        (dhelm == THELM_DESC_JEWELLED) ? "宝飾"
                                                       : "バグの" );
#else
                strcat( glorg,
                        (dhelm == THELM_DESC_WINGED)   ? "winged " :
                        (dhelm == THELM_DESC_HORNED)   ? "horned " :
                        (dhelm == THELM_DESC_CRESTED)  ? "crested " :
                        (dhelm == THELM_DESC_PLUMED)   ? "plumed " :
                        (dhelm == THELM_DESC_SPIKED)   ? "spiked " :
                        (dhelm == THELM_DESC_VISORED)  ? "visored " :
                        (dhelm == THELM_DESC_JEWELLED) ? "jeweled "
                                                       : "buggy " );
#endif
            }
        }

        helm_type = get_helmet_type( item );
        if (helm_type == THELM_HELM)
#ifdef JP
            strcat(glorg, "兜");
#else
            strcat(glorg, "helm");
#endif
        else if (helm_type == THELM_CAP)
#ifdef JP
            strcat(glorg, "帽子");
#else
            strcat(glorg, "cap");
#endif
        else if (helm_type == THELM_WIZARD_HAT)
#ifdef JP
            strcat(glorg, "魔法帽");
#else
            strcat(glorg, "wizard's hat");
#endif
        else
#ifdef JP
            strcat(glorg, "鉄兜");
#else
            strcat(glorg, "helmet");
#endif
        break;

    case ARM_GLOVES:
#ifdef JP
        strcat(glorg, "グローブ");
#else
        strcat(glorg, "gloves");
#endif
        break;

    case ARM_BOOTS:
        if (item.plus2 == TBOOT_NAGA_BARDING)
#ifdef JP
            strcat(glorg, "ナーガの具装");
#else
            strcat(glorg, "naga barding");
#endif
        else if (item.plus2 == TBOOT_CENTAUR_BARDING)
#ifdef JP
            strcat(glorg, "セントールの馬甲");
#else
            strcat(glorg, "centaur barding");
#endif
        else
#ifdef JP
            strcat(glorg, "ブーツ");
#else
            strcat(glorg, "boots");
#endif
        break;

    case ARM_BUCKLER:
#ifdef JP
        strcat(glorg, "バックラー");
#else
        strcat(glorg, "buckler");
#endif
        break;

    case ARM_LARGE_SHIELD:
#ifdef JP
        strcat(glorg, "大盾");
#else
        strcat(glorg, "large shield");
#endif
        break;

    case ARM_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "ドラゴンの皮");
#else
        strcat(glorg, "dragon hide");
#endif
        break;

    case ARM_TROLL_HIDE:
#ifdef JP
        strcat(glorg, "トロルの皮");
#else
        strcat(glorg, "troll hide");
#endif
        break;

    case ARM_CRYSTAL_PLATE_MAIL:
#ifdef JP
        strcat(glorg, "クリスタルメイル");
#else
        strcat(glorg, "crystal plate mail");
#endif
        break;

    case ARM_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "ドラゴンの鎧");
#else
        strcat(glorg, "dragon armour");
#endif
        break;

    case ARM_TROLL_LEATHER_ARMOUR:
#ifdef JP
        strcat(glorg, "トロル革の鎧");
#else
        strcat(glorg, "troll leather armour");
#endif
        break;

    case ARM_ICE_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "アイスドラゴンの皮");
#else
        strcat(glorg, "ice dragon hide");
#endif
        break;

    case ARM_ICE_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "アイスドラゴンの鎧");
#else
        strcat(glorg, "ice dragon armour");
#endif
        break;

    case ARM_STEAM_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "蒸気ドラゴンの皮");
#else
        strcat(glorg, "steam dragon hide");
#endif
        break;

    case ARM_STEAM_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "蒸気ドラゴンの鎧");
#else
        strcat(glorg, "steam dragon armour");
#endif
        break;

    case ARM_MOTTLED_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "斑紋ドラゴンの皮");
#else
        strcat(glorg, "mottled dragon hide");
#endif
        break;

    case ARM_MOTTLED_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "斑紋ドラゴンの鎧");
#else
        strcat(glorg, "mottled dragon armour");
#endif
        break;

    case ARM_STORM_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "ストームドラゴンの皮");
#else
        strcat(glorg, "storm dragon hide");
#endif
        break;

    case ARM_STORM_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "ストームドラゴンの鎧");
#else
        strcat(glorg, "storm dragon armour");
#endif
        break;

    case ARM_GOLD_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "ゴールドドラゴンの皮");
#else
        strcat(glorg, "gold dragon hide");
#endif
        break;

    case ARM_GOLD_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "ゴールドドラゴンの鎧");
#else
        strcat(glorg, "gold dragon armour");
#endif
        break;

    case ARM_ANIMAL_SKIN:
#ifdef JP
        strcat(glorg, "獣の皮");
#else
        strcat(glorg, "animal skin");
#endif
        break;

    case ARM_SWAMP_DRAGON_HIDE:
#ifdef JP
        strcat(glorg, "沼ドラゴンの皮");
#else
        strcat(glorg, "swamp dragon hide");
#endif
        break;

    case ARM_SWAMP_DRAGON_ARMOUR:
#ifdef JP
        strcat(glorg, "沼ドラゴンの鎧");
#else
        strcat(glorg, "swamp dragon armour");
#endif
        break;
    }
}                               // end standard_name_armour()
