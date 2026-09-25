// insult generator
// Josh Fishman (c) 2001, All Rights Reserved
// This file is released under the GNU GPL, but special permission is granted
// to link with Linley Henzel's Dungeon Crawl (or Crawl) without change to
// Crawl's license.
//
// The goal of this stuff is catachronistic feel.

#include "AppHdr.h"

#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include "externs.h"
#include "insult.h"
#include "mon-util.h"
#include "stuff.h"

static const char* insults1(void);
static const char* insults2(void);
static const char* insults3(void);
static const char* run_away(void);
static const char* give_up(void);
static const char* meal(void);
static const char* whilst_thou_can(void);
static const char* important_body_part(void);
static const char* important_spiritual_part(void);

static void init_cap(char *);

void init_cap(char * str)
{
    if (str != NULL)
        str[0] = toupper( str[0] );
}

void imp_taunt( struct monsters *mons )
{
    char buff[80];
    const char *mon_name = ptr_monam( mons, DESC_CAP_THE );

    snprintf( buff, sizeof(buff), 
#ifdef JP 
              "%s、%sが！", 
#else
              "%s, thou %s!", 
#endif
              random2(7) ? run_away() : give_up(),
              generic_insult() );

    init_cap( buff );

#ifdef JP 
    // XXX: Not pretty, but stops truncation...
    if (strlen( mon_name ) + 11 + strlen( buff ) >= 76)
    {
        snprintf( info, INFO_SIZE, "%sは叫んだ。:", mon_name );
        mpr( info, MSGCH_TALK );
        snprintf( info, INFO_SIZE, "「%s」", buff );
        mpr( info, MSGCH_TALK );
#else
    // XXX: Not pretty, but stops truncation...
    if (strlen( mon_name ) + 11 + strlen( buff ) >= 79)
    {
        snprintf( info, INFO_SIZE, "%s shouts:", mon_name );
        mpr( info, MSGCH_TALK );
        mpr( buff, MSGCH_TALK );
#endif
    }
    else
    {
#ifdef JP 
        snprintf( info, INFO_SIZE, "%sは叫んだ。「%s」", mon_name, buff );
#else
        snprintf( info, INFO_SIZE, "%s shouts, \"%s\"", mon_name, buff );
#endif
        mpr( info, MSGCH_TALK );
    }
}

void demon_taunt( struct monsters *mons )
{
    static const char * sound_list[] = 
    {
#ifdef JP 
        "言った",         // actually S_SILENT
        "叫んだ", 
        "怒鳴った", 
        "叫んだ", 
        "喚いた", 
        "金切り声をあげた", 
        "吠えた", 
        "鋭く叫んだ", 
        "つぶやいた", 
        "唸り声をあげた", 
        "哀れっぽい声で言った", 
        "しわがれ声で言った", 
        "がみがみ言った", 
#else
        "says",         // actually S_SILENT
        "shouts", 
        "barks", 
        "shouts", 
        "roars", 
        "screams", 
        "bellows", 
        "screeches", 
        "buzzes", 
        "moans", 
        "whines", 
        "croaks", 
        "growls", 
#endif
    };
  
    char buff[80];
    const char *mon_name = ptr_monam( mons, DESC_CAP_THE );
    const char *voice = sound_list[ mons_shouts(mons->type) ];

    if (coinflip())
    {
        snprintf( buff, sizeof(buff), 
#ifdef JP 
                 "%s、この%s！",
#else
                 "%s, thou %s!",
#endif
                 random2(3) ? give_up() : run_away(),
                 generic_insult() );
    }
    else
    {
        switch( random2( 4 ) ) 
        {
        case 0:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "お前の%sを%sにしてやる！",
#else
                      "Thy %s shall be my %s!",
#endif
                      random2(4) ? important_body_part() 
                                 : important_spiritual_part(), meal() );
            break;
        case 1:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "%s、汝うまそうな%sよ！", give_up(), meal() );
#else
                      "%s, thou tasty %s!", give_up(), meal() );
#endif
            break;
        case 2:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "%s、%s！", run_away(), whilst_thou_can() );
#else
                      "%s %s!", run_away(), whilst_thou_can() );
#endif
            break;
        case 3:
            snprintf( buff, sizeof(buff), 
#ifdef JP 
                      "お前の%sを%s%s！",
                      random2(4) ? important_body_part() 
                                 : important_spiritual_part() ,
                      coinflip() ? "御馳走になる" : "むさぼり食う",
                      coinflip() ? "ぜ" : "ことにするぜ" );
#else
                      "I %s %s thy %s!",
                      coinflip() ? "will" : "shall",
                      coinflip() ? "feast upon" : "devour",
                      random2(4) ? important_body_part() 
                                 : important_spiritual_part() );
#endif
            break;
        default:
#ifdef JP 
            snprintf( buff, sizeof(buff), "汝、%sよ！", generic_insult() );
#else
            snprintf( buff, sizeof(buff), "Thou %s!", generic_insult() );
#endif
            break;
        }
    }

    init_cap( buff );

#ifdef JP 
    // XXX: Not pretty, but stops truncation...
    if (strlen(mon_name) + strlen(voice) + strlen(buff) + 5 >= 76)
    {
        snprintf( info, INFO_SIZE, "%sは%s:", mon_name, voice );
#else
    // XXX: Not pretty, but stops truncation...
    if (strlen(mon_name) + strlen(voice) + strlen(buff) + 5 >= 79)
    {
        snprintf( info, INFO_SIZE, "%s %s:", mon_name, voice );
#endif
        mpr( info, MSGCH_TALK );

        mpr( buff, MSGCH_TALK );
    }
    else
    {
#ifdef JP 
        snprintf( info, INFO_SIZE, "%sは%s。「%s」", mon_name, voice, buff );
#else
        snprintf( info, INFO_SIZE, "%s %s, \"%s\"", mon_name, voice, buff );
#endif
        mpr( info, MSGCH_TALK );
    }
}

const char * generic_insult(void)
{
    static char buffer[80]; //FIXME: use string objects or whatnot

    strcpy(buffer, insults1());
    strcat(buffer, "");
    strcat(buffer, insults2());
    strcat(buffer, "");
    strcat(buffer, insults3());

    return (buffer);
}

static const char * important_body_part(void)
{
    static const char * part_list[] = {
#ifdef JP 
        "頭",
        "脳味噌",
        "心臓",
        "はらわた",
        "目玉",
        "肺",
        "肝臓",
        "喉笛",
        "首",
        "頭蓋",
        "背骨",
#else
        "head",
        "brain",
        "heart",
        "viscera",
        "eyes",
        "lungs",
        "liver",
        "throat",
        "neck",
        "skull",
        "spine",
#endif
    };
  
    return (part_list[random2(sizeof(part_list) / sizeof(char *))]);
}

static const char * important_spiritual_part(void)
{
    static const char * part_list[] = {
#ifdef JP 
        "魂",
        "霊魂",
        "内なる光",
        "希望",
        "信仰",
        "意気",
        "勇気",
        "心",
        "正気",
        "根性",
        "生命力",
#else
        "soul",
        "spirit",
        "inner light",
        "hope",
        "faith",
        "will",
        "heart",
        "mind",
        "sanity",
        "fortitude",
        "life force",
#endif
    };

    return (part_list[random2(sizeof(part_list) / sizeof(char *))]);
}

static const char * meal(void)
{
    static const char * meal_list[] = {
#ifdef JP 
        //対応語がかぶる物は別の言葉に置き換え。
        "メシ",
        "朝メシ",
        "昼メシ",
        "晩メシ",
        "夕メシ",
        "ごちそう",
        "おやつ",
        "食糧",
        "軽食",
        "茶菓子",
        "弁当",
        "夜食",
        "おかず",
        "間食",
        "一口",
#else
        "meal",
        "breakfast",
        "lunch",
        "dinner",
        "supper",
        "repast",
        "snack",
        "victuals",
        "refection",
        "junket",
        "luncheon",
        "snackling",
        "curdle",
        "snacklet",
        "mouthful",
#endif
    };

    return (meal_list[random2(sizeof(meal_list) / sizeof(char *))]);
}

static const char * run_away(void)
{
    static const char * run_away_list[] = {
#ifdef JP
        //意味が繋がりそうな形に意訳。
        "降参しろ",
        "失せろ",
        "立ち去れ",
        "逃げるがいい",
        "退き下がれ",
        "飛んで逃げろ",
        "バカ面下げて失せろ",
        "クセえんだよ失せろ",
        "行っちまえ。戻ってくんな",
        "死んじまえ",
        "汝よ退け",
        "汝よ立ち去れ",
        "尻尾を巻いて逃げるがいい",
        "去れ",
        "汝の来たりし処に帰れ",
        "今すぐ失せろ！",
        "逝っちまえ",
        "くたばれ",
        "這いずって失せろ",
        "這いずって引き返せ",
        "這いずれ",
        "這って帰れ",
        "とっとと引き返せ",
        "とっとと消えろ",
        "とっとと失せろ",
        "慌てて逃げるがいい",
        "こそこそ逃げてくがいい",
#else
        "give up",
        "quit",
        "run away",
        "escape",
        "flee",
        "fly",
        "take thy face hence",
        "remove thy stench",
        "go and return not",
        "get thee hence",
        "back with thee",
        "away with thee",
        "turn tail",
        "leave",
        "return whence thou came",
        "begone",
        "get thee gone",
        "get thee hence",
        "slither away",
        "slither home",
        "slither hence",
        "crawl home",
        "scamper home",
        "scamper hence",
        "scamper away",
        "bolt",
        "decamp",
#endif
    };

    return (run_away_list[random2(sizeof(run_away_list) / sizeof(char *))]);
}

static const char * give_up(void)
{
    static const char * give_up_list[] = {
#ifdef JP 
        "諦めろ",
        "降参しろ",
        "止まれ",
        "降伏しろ",
        "跪け",
        "慈悲を乞え",
        "絶望しろ",
        "服従しろ",
        "屈服しろ",
        "怖じ気づけ",
        "汝の過ちを認めよ",
        "汝の挫折を認めよ",
        "汝の運命を認めよ",
        "降伏せよ",
        "服従せよ",
        "汝の失敗を受け容れよ",
        "汝の挫折を受け容れよ",
        "汝の運命を受け容れよ",
        "抵抗をやめろ",
        "震え上がれ",
        "希望を棄てよ",
        "敗北を噛み締めろ",
        "うなだれろ",
        "その身を棄てよ",
        "望みを棄てよ",
        "汝の鎮魂歌を聴け",
        "汝の遁走曲を聴け",
        "敗北を認めろ",
        "もがき苦しめ",
#else
        "give up",
        "give in",
        "quit",
        "surrender",
        "kneel",
        "beg for mercy",
        "despair",
        "submit",
        "succumb",
        "quail",
        "embrace thy failure",
        "embrace thy fall",
        "embrace thy doom",
        "embrace thy dedition",
        "embrace submission",
        "accept thy failure",
        "accept thy fall",
        "accept thy doom",
        "capitulate",
        "tremble",
        "relinquish hope",
        "taste defeat",
        "despond",
        "disclaim thyself",
        "abandon hope",
        "face thy requiem",
        "face thy fugue",
        "admit defeat",
        "flounder",
#endif
    };

    return (give_up_list[random2(sizeof(give_up_list) / sizeof(char *))]);
}

static const char * whilst_thou_can(void)
{
    static const char * threat_list[] = {
#ifdef JP 
        "汝がそうすることのできるうち",
        "汝が許されているうちに",
        "汝がそうすることのあたううち",
        "もし汝に知恵のあらば",
        "汝の運の続くうち",
        "凶運が汝を捉える前に",
        "死が汝を見いだす前に",
        "汝の五体が満足なうちに",
        "汝に命あるうちに", //jmf: hmm. screen vs. this for undead?
#else
        "whilst thou can",
        "whilst thou may",
        "whilst thou are able",
        "if wit thou hast",
        "whilst thy luck holds",
        "before doom catcheth thee",
        "lest death find thee",
        "whilst thou art whole",
        "whilst life thou hast", //jmf: hmm. screen vs. this for undead?
#endif
    };

    return (threat_list[random2(sizeof(threat_list) / sizeof(char *))]);
}

static const char * insults1(void)
{
    static const char * insults1_list[] = {
#ifdef JP 
//訳せる物は訳す。そうでない物はそれらしい言葉を捜して書く。
        "不細工な",             // artless
        "へまな",               // baffled
        "下劣な",               // bawdy
        "ベタベタな",           // beslubbering
        "役立たずな",           // bootless
        "尊大な",               // bumbling
        "口先だけで",           // canting
        "不作法な",             // churlish
        "甘ったれた",           // cockered
        "とんまな",             // clouted
        "意気地なしな",         // craven
        "卑しい",               // currish
        "気色悪い",             // dankish
        "しらばくれた",         // dissembling
        "退屈な",               // droning
        "無責任な",             // ducking
        "変態で",               // errant
        "卑屈な",               // fawning
        "無気力な",             // feckless
        "腰抜けな",             // feeble
        "嘘吐きな",             // fobbing
        "にやけた",             // foppish
        "つむじ曲がりな",       // froward
        "薄っぺらな",           // frothy
        "しつこい",             // fulsome
        "ペテン吐きな",         // gleeking
        "助平な",               // goatish
        "たるんだ",             // gorbellied
        "垢じみた",             // grime-gilt
        "おぞましい",           // horrid
        "憎むべき",             // hateful
        "出しゃばりな",         // impertinent
        "病気持ちで",           // infectious
        "癇に障る",             // jarring
        "強情張りな",           // loggerheaded
        "のろまな",             // lumpish
        "どもりで",             // mammering
        "ひしゃげた",           // mangled
        "めそめそした",         // mewling
        "いやらしい",           // odious
        "太鼓腹で",             // paunchy
        "臆病な",               // pribbling
        "むかつく",             // puking
        "ちんけな",             // puny
        "竦み上がった",         // qualling
        "震え上がった",         // quaking
        "プンプン臭う",         // rank
        "ポン引きで",           // pandering
        "猫かぶりで",           // pecksniffian
        "尾羽打ち枯らした",     // plume-plucked
        "底が浅い",             // pottle-deep
        "梅毒持ちで",           // pox-marked
        "酔いどれで",           // reeling-ripe
        "無教養な",             // rough-hewn
        "にやけた",             // simpering
        "スカスカな",           // spongy
        "ムッツリで",           // surly
        "よろよろで",           // tottering
        "ひねくれた",           // twisted
        "お世辞屋で",           // unctious
        "トチ狂った",           // unhinged
        "口の減らない",         // unmuzzled
        "見栄張りな",           // vain
        "嫌たらしい",           // venomed
        "極悪非道な",           // villainous
        "ひがみっぽい",         // warped
        "わがままな",           // wayward
        "貧弱な",               // weedy
        "無価値な",             // worthless
        "ふざけた",             // yeasty
#else
        "artless",
        "baffled",
        "bawdy",
        "beslubbering",
        "bootless",
        "bumbling",
        "canting",
        "churlish",
        "cockered",
        "clouted",
        "craven",
        "currish",
        "dankish",
        "dissembling",
        "droning",
        "ducking",
        "errant",
        "fawning",
        "feckless",
        "feeble",
        "fobbing",
        "foppish",
        "froward",
        "frothy",
        "fulsome",
        "gleeking",
        "goatish",
        "gorbellied",
        "grime-gilt",
        "horrid",
        "hateful",
        "impertinent",
        "infectious",
        "jarring",
        "loggerheaded",
        "lumpish",
        "mammering",
        "mangled",
        "mewling",
        "odious",
        "paunchy",
        "pribbling",
        "puking",
        "puny",
        "qualling",
        "quaking",
        "rank",
        "pandering",
        "pecksniffian",
        "plume-plucked", 
        "pottle-deep",
        "pox-marked",
        "reeling-ripe",
        "rough-hewn",
        "simpering",
        "spongy",
        "surly",
        "tottering",
        "twisted",
        "unctious", 
        "unhinged",
        "unmuzzled",
        "vain",
        "venomed",
        "villainous",
        "warped",
        "wayward",
        "weedy",
        "worthless",
        "yeasty",
#endif
    };

    return (insults1_list[random2(sizeof(insults1_list) / sizeof(char*))]);
}

static const char * insults2(void)
{
    static const char * insults2_list[] = {
#ifdef JP 
//訳せる物は訳す。そうでない物はそれらしい言葉を捜して書く。
        "ご機嫌取りの",         // base-court
        "鳥刺しの",             // bat-fowling
        "阿呆の",               // beef-witted
        "痴呆の",               // beetle-headed
        "短気性の",             // boil-brained
        "鉤爪持ちの",           // clapper-clawed
        "低能の",               // clay-brained
        "キス魔の",             // common-kissing
        "クルクルパーの",       // crook-pated
        "鬱病の",               // dismal-dreaming
        "ドブ産まれの",         // ditch-delivered
        "乱視の",               // dizzy-eyed
        "冷血漢の",             // doghearted
        "いけ好かない",         // dread-bolted
        "社会の敵の",           // earth-vexing
        "エルフ殺しの",         // elf-skinned
        "腎肥大の",             // fat-kidneyed
        "泥啜りの",             // fen-sucked
        "垂れ唇の",             // flap-mouthed
        "蝿たかりの",           // fly-bitten
        "左巻きの",             // folly-fallen
        "天然馬鹿の",           // fool-born
        "馬鹿食いの",           // full-gorged
        "内臓病みの",           // guts-griping
        "ぴいぴいの",           // half-faced
        "短絡思考の",           // hasty-witted
        "下賎の",               // hedge-born
        "憎まれっ子の",         // hell-hated
        "うわごと言いの",       // idle-headed
        "粗忽者の",             // ill-breeding
        "がさつ者の",           // ill-nurtured
        "ゲテモノの",           // kobold-kissing
        "精神錯乱の",           // knotty-pated
        "びっこの",             // limp-willed
        "小心者の",             // milk-livered
        "月狂いの",             // moon-mazed
        "道化の",               // motley-minded
        "泣き虫の",             // onion-eyed
        "人でなしの",           // miscreant
        "不埒者の",             // roguish
        "カビだらけの",         // moldwarp
        "発情期の",             // ruttish
        "告げ口屋の",           // mumble-news
        "厚顔無知の",           // saucy
        "かっぱらいの",         // nut-hook
        "癇癪魔の",             // spleeny
        "薄バカの",             // pigeon-egg
        "野育ちの",             // rude-growing
        "カスの",               // rump-fed
        "木っ端の",             // shard-borne
        "万引きの",             // sheep-biting
        "ブス専の",             // sow-suckled
        "家畜の",               // spur-galled
        "腹ボテの",             // swag-bellied
        "鈍足の",               // tardy-gaited
        "呑兵衛の",             // tickle-brained
        "ヒキガエルの",         // toad-spotted
        "たかり屋の",           // toenail-biting
        "豚鼻の",               // unchin-snouted
        "けちの",               // weather-bitten
        "害虫の",               // weevil-witted
#else
        "base-court",
        "bat-fowling",
        "beef-witted",
        "beetle-headed",
        "boil-brained",
        "clapper-clawed",
        "clay-brained",
        "common-kissing",
        "crook-pated",
        "dismal-dreaming",
        "ditch-delivered",
        "dizzy-eyed",
        "doghearted",
        "dread-bolted",
        "earth-vexing",
        "elf-skinned",
        "fat-kidneyed",
        "fen-sucked",
        "flap-mouthed",
        "fly-bitten",
        "folly-fallen",
        "fool-born",
        "full-gorged",
        "guts-griping",
        "half-faced",
        "hasty-witted",
        "hedge-born",
        "hell-hated",
        "idle-headed",
        "ill-breeding",
        "ill-nurtured",
        "kobold-kissing",
        "knotty-pated",
        "limp-willed",
        "milk-livered",
        "moon-mazed",
        "motley-minded",
        "onion-eyed",
        "miscreant",
        "roguish",
        "moldwarp",
        "ruttish",
        "mumble-news",
        "saucy",
        "nut-hook",
        "spleeny",
        "pigeon-egg",
        "rude-growing",
        "rump-fed",
        "shard-borne",
        "sheep-biting",
        "sow-suckled",
        "spur-galled",
        "swag-bellied",
        "tardy-gaited",
        "tickle-brained",
        "toad-spotted",
        "toenail-biting",
        "unchin-snouted",
        "weather-bitten",
        "weevil-witted",
#endif
    };

    return (insults2_list[random2(sizeof(insults2_list) / sizeof(char*))]);
}

static const char * insults3(void)
{
    static const char * insults3_list[] = {
#ifdef JP 
//訳せる物は訳す。そうでない物はそれらしい言葉を捜して書く。
        "萎び野郎",               // apple-john
        "邪魔者",                 // baggage
        "ひったくり",             // bandersnitch
        "偏執狂",                 // barnacle
        "物乞い",                 // beggar
        "いびり屋",               // bladder
        "イノブタ",               // boar-pig
        "食わせ者",               // bounder
        "化け物",                 // bugbear
        "浮浪者",                 // bum-bailey
        "ド腐れ",                 // canker-blossom
        "おしゃべり",             // clack-dish
        "だんまり屋",             // clam
        "ボッキ野郎",             // clotpole
        "気取り屋",               // coxcomb
        "玉袋",                   // codpiece
        "死に損ない",             // death-token
        "しみったれ",             // dewberry
        "とんま",                 // dingleberry
        "浮かれ野郎",             // flap-bat
        "淫売",                   // flax-wench
        "お調子者",               // flirt-gill
        "べんちゃら野郎",         // foot-licker
        "ほら吹き",               // fustilarian
        "甘えたガキ",             // giglet
        "三下",                   // gnoll-tail
        "カモ",                   // gudgeon
        "よた公",                 // guttersnipe
        "やせっぽち",             // haggard
        "業突く張り",             // harpy
        "豚公",                   // hedge-pig
        "ケダモノ",               // horn-beast
        "こそ泥",                 // hugger-mugger
        "脳たりん",               // joithead
        "下劣漢",                 // lewdster
        "田舎者",                 // lout
        "蛆虫",                   // maggot-pie
        "害虫",                   // malt-worm
        "デク人形",               // mammet
        "貧弱野郎",               // measle
        "物乞い",                 // mendicant
        "クソガキ",               // minnow reeky
        "金づち頭",               // mule
        "クソ",                   // nightsoil
        "名無し",                 // nobody
        "クズ",                   // nothing
        "薄のろ",                 // pigeon-egg
        "豚の餌",                 // pignut
        "ニキビ野郎",             // pimple
        "イボ野郎",               // pustule
        "ハゲタカ",               // puttock
        "ぼんくら",               // pumpion
        "害悪",                   // ratsbane
        "ゴミあさり",             // scavenger
        "若造",                   // scut
        "奴隷野郎",               // serf
        "間抜け",                 // simpleton
        "ゴロツキ",               // skainsmate
        "スライムモルド",         // slime mold
        "盗っ人",                 // snaffler
        "空威張り野郎",           // snake-molt
        "売女",                   // strumpet
        "山師",                   // surfacer
        "無宿者",                 // tinkerer
        "ちびすけ",               // tiddler
        "悪ガキ",                 // urchin
        "悪党",                   // varlet
        "下僕",                   // vassal
        "ハゲワシ",               // vulture
        "ろくでなし",             // wastrel
        "ごますり",               // wagtail
        "生白野郎",               // whey-face
        "ナメクジ野郎",           // wormtrail
        "ヤクの糞",               // yak-dropping
        "ゾンビのエサ",           // zombie-fodder
#else
        "apple-john",
        "baggage",
        "bandersnitch",
        "barnacle",
        "beggar",
        "bladder",
        "boar-pig",
        "bounder",
        "bugbear",
        "bum-bailey",
        "canker-blossom",
        "clack-dish",
        "clam",
        "clotpole",
        "coxcomb",
        "codpiece",
        "death-token",
        "dewberry",
        "dingleberry",
        "flap-bat",
        "flax-wench",
        "flirt-gill",
        "foot-licker",
        "fustilarian",
        "giglet",
        "gnoll-tail",
        "gudgeon",
        "guttersnipe",
        "haggard",
        "harpy",
        "hedge-pig",
        "horn-beast",
        "hugger-mugger",
        "joithead",
        "lewdster",
        "lout",
        "maggot-pie",
        "malt-worm",
        "mammet",
        "measle",
        "mendicant",
        "minnow reeky",
        "mule",
        "nightsoil",
        "nobody",
        "nothing",
        "pigeon-egg",
        "pignut",
        "pimple",
        "pustule",
        "puttock",
        "pumpion",
        "ratsbane",
        "scavenger",
        "scut",
        "serf",
        "simpleton",
        "skainsmate",
        "slime mold",
        "snaffler",
        "snake-molt",
        "strumpet",
        "surfacer",
        "tinkerer",
        "tiddler",
        "urchin",
        "varlet",
        "vassal",
        "vulture",
        "wastrel",
        "wagtail",
        "whey-face",
        "wormtrail",
        "yak-dropping",
        "zombie-fodder",
#endif
    };

    return (insults3_list[random2(sizeof(insults3_list) / sizeof(char*))]);
}

// currently unused:
#if 0
const char * racial_insult(void)
{
    static const char * food3[] = {
        "snackling",
        "crunchlet",
        "half-meal",
        "supper-setting",
        "snacklet",
        "noshlet",
        "morsel",
        "mug-up",
        "bite-bait",
        "crunch-chow",
        "snack-pap",
        "grub",
    };

    static const char * elf1[] = {
        "weakly",
        "sickly",
        "frail",
        "delicate",
        "fragile",
        "brittle",
        "tender",
        "mooning",
        "painted",
        "lily-hearted",
        "dandy",
        "featherweight",
        "flimsy",
        "rootless",
        "spindly",
        "puny",
        "shaky",
        "prissy",
    };

    static const char * halfling3[] = {
        "half-pint",
        "footstool",
        "munchkin",
        "side-stool",
        "pudgelet",
        "groundling",
        "burrow-snipe",
        "hole-bolter",
        "low-roller",
        "runt",
        "peewee",
        "mimicus",
        "manikin",
        "hop-o-thumb",
        "knee-biter",
        "burrow-botch",
        "hole-pimple",
        "hovel-pustule",
    };

    static const char * spriggan3[] = {
        "rat-rider",
        "mouthfull",
        "quarter-pint",
        "nissette",
        "fizzle-flop",
        "spell-botch",
        "feeblet",
        "weakling",
        "pinchbeck-pixie",
        "ankle-biter",
        "bootstain",
        "nano-nebbish",
        "sopling",
        "shrunken violet",
        "sissy-prig",
        "pussyfoot",
        "creepsneak",
    };

    static const char * dwarf2[] = {
        "dirt-grubbing",
        "grit-sucking",
        "muck-plodding",
        "stone-broke",
        "pelf-dandling",
        "fault-botching",
        "gravel-groveling",
        "boodle-bothering",
        "cabbage-coddling",
        "rhino-raveling",
        "thigh-biting",
        "dirt-delving",
    };

    static const char * kenku2[] = {
        "hollow-boned",
        "feather-brained",
        "beak-witted",
        "hen-pecked",
        "lightweight",
        "frail-limbed",
        "bird-brained",
        "featherweight",
        "pigeon-toed",
        "crow-beaked",
        "magpie-eyed",
        "mallardish",
    };

    static const char * minotaur3[] = {
        "bull-brain",
        "cud-chewer",
        "calf-wit",
        "bovine",
        //"mooer", // of Venice
        "cow",
        "cattle",
        "meatloaf",
        "veal",
        "meatball",
        "rump-roast",
        "briscut",
        "cretin",
        "walking sirloin",
    };

    switch (you.species) 
    {
    default:
    break;
    }
}
#endif
