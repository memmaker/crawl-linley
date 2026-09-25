/*
 *  File:       describe.cc
 *  Summary:    Functions used to print information about various game objects.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <4>      10/14/99     BCR     enummed describe_god()
 *      <3>      10/13/99     BCR     Added GOD_NO_GOD case in describe_god()
 *      <2>      5/20/99      BWR     Replaced is_artifact with
 *                                    is_dumpable_artifact
 *      <1>      4/20/99      JDJ     Reformatted, uses string objects,
 *                                    split out 10 new functions from
 *                                    describe_item(), added
 *                                    get_item_description and
 *                                    is_artifact.
 */

#include "AppHdr.h"
#include "describe.h"

#include <stdlib.h>
#include <stdio.h>
#include <string>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "abl-show.h"
#include "debug.h"
#include "fight.h"
#include "itemname.h"
#include "macro.h"
#include "mon-util.h"
#include "player.h"
#include "randart.h"
#include "religion.h"
#include "skills2.h"
#include "stuff.h"
#include "wpn-misc.h"
#include "spl-util.h"

// ========================================================================
//      Internal Functions
// ========================================================================

//---------------------------------------------------------------
//
// append_value
//
// Appends a value to the string. If plussed == 1, will add a + to
// positive values (itoa always adds - to -ve ones).
//
//---------------------------------------------------------------
static void append_value( std::string & description, int valu, bool plussed )
{
    if (valu >= 0 && plussed == 1)
        description += "+";

    char value_str[80];

    itoa( valu, value_str, 10 );

    description += value_str;
}                               // end append_value()

//---------------------------------------------------------------
//
// print_description
//
// Takes a descpr string filled up with stuff from other functions,
// and displays it with minor formatting to avoid cut-offs in mid
// word and such. The character $ is interpreted as a CR.
//
//---------------------------------------------------------------
static void print_description( const std::string &d )
{
    unsigned int  nextLine = std::string::npos;
    unsigned int  currentPos = 0;

#ifdef DOS
    const unsigned int lineWidth = 52;
#else
    const unsigned int lineWidth = 70;
#endif

    bool nlSearch = true;       // efficiency

    textcolor(LIGHTGREY);

    while(currentPos < d.length())
    {
        if (currentPos != 0)
        {
#ifdef PLAIN_TERM
            gotoxy(1, wherey() + 1);
#endif
#ifdef DOS_TERM
            cprintf(EOL);
#endif
        }

        // see if $ sign is within one lineWidth
        if (nlSearch)
        {
            nextLine = d.find('$', currentPos);

            if (nextLine >= currentPos && nextLine < currentPos + lineWidth)
            {
                cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
                currentPos = nextLine + 1;
                continue;
            }

            if (nextLine == std::string::npos)
                nlSearch = false;       // there are no newlines, don't search again.
        }

        // no newline -- see if rest of string will fit.
        if (currentPos + lineWidth >= d.length())
        {
            cprintf((d.substr(currentPos)).c_str());
            return;
        }


        // ok.. try to truncate at space.
        nextLine = d.rfind(' ', currentPos + lineWidth);

        if (nextLine != std::string::npos)
        {
            cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
            currentPos = nextLine + 1;
            continue;
        }

        // oops.  just truncate.
        nextLine = currentPos + lineWidth;

        if (nextLine > d.length())
            nextLine = d.length();

        cprintf((d.substr(currentPos, nextLine - currentPos)).c_str());
        currentPos = nextLine;
    }
}

//!!!! description += "xxxxxxx "; は文字列末尾に半角スペースを入れないとフリーズを起こすことがある。
//     必ず改行したい場合には$を入れるが、" $"とすることにしておく。

//---------------------------------------------------------------
//
// randart_descpr
//
// Appends the various powers of a random artefact to the description
// string.
//
//---------------------------------------------------------------
static void randart_descpr( std::string &description, const item_def &item )
{
    unsigned int old_length = description.length();

    FixedVector< char, RA_PROPERTIES > proprt;
    randart_wpn_properties( item, proprt );

    if (proprt[ RAP_AC ])
    {
#ifdef JP
        description += "$これはあなたのACに影響する。(";
#else
        description += "$It affects your AC (";
#endif
        append_value(description, proprt[ RAP_AC ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_EVASION ])
    {
#ifdef JP
        description += "$これはあなたの回避力に影響する。(";
#else
        description += "$It affects your evasion (";
#endif
        append_value(description, proprt[ RAP_EVASION ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_STRENGTH ])
    {
#ifdef JP
        description += "$これはあなたの腕力に影響する。(";
#else
        description += "$It affects your strength (";
#endif
        append_value(description, proprt[ RAP_STRENGTH ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_INTELLIGENCE ])
    {
#ifdef JP
        description += "$これはあなたの知力に影響する。(";
#else
        description += "$It affects your intelligence (";
#endif
        append_value(description, proprt[ RAP_INTELLIGENCE ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_DEXTERITY ])
    {
#ifdef JP
        description += "$これはあなたの器用さに影響する。(";
#else
        description += "$It affects your dexterity (";
#endif
        append_value(description, proprt[ RAP_DEXTERITY ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_ACCURACY ])
    {
#ifdef JP
        description += "$これはあなたの命中率に影響する。(";
#else
        description += "$It affects your accuracy (";
#endif
        append_value(description, proprt[ RAP_ACCURACY ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_DAMAGE ])
    {
#ifdef JP
        description += "$これはあなたがダメージを与える能力に影響する。(";
#else
        description += "$It affects your damage-dealing abilities (";
#endif
        append_value(description, proprt[ RAP_DAMAGE ], true);
#ifdef JP
        description += ")";
#else
        description += ").";
#endif
    }

    if (proprt[ RAP_FIRE ] < -2)
#ifdef JP
        description += "$これはあなたを極度に火から弱くする。";
#else
        description += "$It makes you highly vulnerable to fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == -2)
#ifdef JP
        description += "$これはあなたを非常に火から弱くする。";
#else
        description += "$It makes you greatly susceptible to fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == -1)
#ifdef JP
        description += "$これはあなたを火から弱くする。";
#else
        description += "$It makes you susceptible to fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == 1)
#ifdef JP
        description += "$これはあなたを火から保護する。";
#else
        description += "$It protects you from fire. ";
#endif
    else if (proprt[ RAP_FIRE ] == 2)
#ifdef JP
        description += "$これはあなたを火から強力に保護する。";
#else
        description += "$It greatly protects you from fire. ";
#endif
    else if (proprt[ RAP_FIRE ] > 2)
#ifdef JP
        description += "$これはあなたを火からほぼ免疫にする。";
#else
        description += "$It renders you almost immune to fire. ";
#endif

    if (proprt[ RAP_COLD ] < -2)
#ifdef JP
        description += "$これはあなたを極度に冷気から弱くする。";
#else
        description += "$It makes you highly susceptible to cold. ";
#endif
    else if (proprt[ RAP_COLD ] == -2)
#ifdef JP
        description += "$これはあなたを非常に冷気から弱くする。";
#else
        description += "$It makes you greatly susceptible to cold. ";
#endif
    else if (proprt[ RAP_COLD ] == -1)
#ifdef JP
        description += "$これはあなたを冷気から弱くする。";
#else
        description += "$It makes you susceptible to cold. ";
#endif
    else if (proprt[ RAP_COLD ] == 1)
#ifdef JP
        description += "$これはあなたを冷気から保護する。";
#else
        description += "$It protects you from cold. ";
#endif
    else if (proprt[ RAP_COLD ] == 2)
#ifdef JP
        description += "$これはあなたを冷気から強力に保護する。";
#else
        description += "$It greatly protects you from cold. ";
#endif
    else if (proprt[ RAP_COLD ] > 2)
#ifdef JP
        description += "$これはあなたを冷気からほぼ免疫にする。";
#else
        description += "$It renders you almost immune to cold. ";
#endif

    if (proprt[ RAP_ELECTRICITY ])
#ifdef JP
        description += "$これはあなたを電気から絶縁する。";
#else
        description += "$It insulates you from electricity. ";
#endif

    if (proprt[ RAP_POISON ])
#ifdef JP
        description += "$これはあなたを毒から保護する。";
#else
        description += "$It protects you from poison. ";
#endif

    if (proprt[ RAP_NEGATIVE_ENERGY ] == 1)
#ifdef JP
        description += "$これはあなたを負のエネルギーからある程度保護する。";
#else
        description += "$It partially protects you from negative energy. ";
#endif
    else if (proprt[ RAP_NEGATIVE_ENERGY ] == 2)
#ifdef JP
        description += "$これはあなたを負のエネルギーから保護する。";
#else
        description += "$It protects you from negative energy. ";
#endif
    else if (proprt[ RAP_NEGATIVE_ENERGY ] > 2)
#ifdef JP
        description += "$これはあなたを負のエネルギーからほぼ免疫にする。";
#else
        description += "$It renders you almost immune negative energy. ";
#endif

    if (proprt[ RAP_MAGIC ])
#ifdef JP
        description += "$これはあなたを魔法から保護する。";
#else
        description += "$It protects you from magic. ";
#endif

    if (proprt[ RAP_STEALTH ] < 0)
    {
        if (proprt[ RAP_STEALTH ] < -20)
#ifdef JP
            description += "$これはあなたの隠密性を非常に悪化させる。";
#else
            description += "$It makes you much less stealthy. ";
#endif
        else
#ifdef JP
            description += "$これはあなたの隠密性を悪化させる。";
#else
            description += "$It makes you less stealthy. ";
#endif
    }
    else if (proprt[ RAP_STEALTH ] > 0)
    {
        if (proprt[ RAP_STEALTH ] > 20)
#ifdef JP
            description += "$これはあなたの隠密性を非常に高める。";
#else
            description += "$It makes you much more stealthy. ";
#endif
        else
#ifdef JP
            description += "$これはあなたの隠密性を高める。";
#else
            description += "$It makes you more stealthy. ";
#endif
    }

    if (proprt[ RAP_EYESIGHT ])
#ifdef JP
        description += "$これはあなたの視力を強化する。";
#else
        description += "$It enhances your eyesight. ";
#endif

    if (proprt[ RAP_INVISIBLE ])
#ifdef JP
        description += "$これはあなたに透明化の発動を可能とさせる。";
#else
        description += "$It lets you turn invisible. ";
#endif

    if (proprt[ RAP_LEVITATE ])
#ifdef JP
        description += "$これはあなたに浮力の発動を可能とさせる。";
#else
        description += "$It lets you levitate. ";
#endif

    if (proprt[ RAP_BLINK ])
#ifdef JP
        description += "$これはあなたに瞬間移動の発動を可能とさせる。";
#else
        description += "$It lets you blink. ";
#endif

    if (proprt[ RAP_CAN_TELEPORT ])
#ifdef JP
        description += "$これはあなたにテレポートの発動を可能とさせる。";
#else
        description += "$It lets you teleport. ";
#endif

    if (proprt[ RAP_BERSERK ])
#ifdef JP
        description += "$これはあなたにバーサークの発動を可能とさせる。";
#else
        description += "$It lets you go berserk. ";
#endif

    if (proprt[ RAP_MAPPING ])
#ifdef JP
        description += "$これはあなたに周辺感知の発動を可能とさせる。";
#else
        description += "$It lets you sense your surroundings. ";
#endif

    if (proprt[ RAP_NOISES ])
#ifdef JP
        description += "$これは騒音を発生する。";
#else
        description += "$It makes noises. ";
#endif

    if (proprt[ RAP_PREVENT_SPELLCASTING ])
#ifdef JP
        description += "$これは呪文の詠唱を阻害する。";
#else
        description += "$It prevents spellcasting. ";
#endif

    if (proprt[ RAP_CAUSE_TELEPORTATION ])
#ifdef JP
        description += "$これはテレポートを引き起こす。";
#else
        description += "$It causes teleportation. ";
#endif

    if (proprt[ RAP_PREVENT_TELEPORTATION ])
#ifdef JP
        description += "$これはほとんどのタイプのテレポートを阻害する。";
#else
        description += "$It prevents most forms of teleportation. ";
#endif

    if (proprt[ RAP_ANGRY ])
#ifdef JP
        description += "$これはあなたに怒りをもたらす。";
#else
        description += "$It makes you angry. ";
#endif

    if (proprt[ RAP_METABOLISM ] >= 3)
#ifdef JP
        description += "$これはあなたの新陳代謝を非常に加速する。";
#else
        description += "$It greatly speeds your metabolism. ";
#endif
    else if (proprt[ RAP_METABOLISM ])
#ifdef JP
        description += "$これはあなたの新陳代謝を加速する。";
#else
        description += "$It speeds your metabolism. ";
#endif

    if (proprt[ RAP_MUTAGENIC ] > 3)
#ifdef JP
        description += "$これは突然変異を誘発する放射エネルギーにぎらぎらしている。";
#else
        description += "$It glows with mutagenic radiation.";
#endif
    else if (proprt[ RAP_MUTAGENIC ])
#ifdef JP
        description += "$これは突然変異を誘発する放射エネルギーを出している。";
#else
        description += "$It emits mutagenic radiations.";
#endif

    if (old_length != description.length())
        description += "$";

    if (is_unrandom_artefact( item ))
    {
        const char *desc = unrandart_descrip( 0, item );
        if (strlen( desc ) > 0)
        {
            description += desc;
            description += "$";
        }
    }
}


//---------------------------------------------------------------
//
// describe_demon
//
// Describes the random demons you find in Pandemonium.
//
//---------------------------------------------------------------
static std::string describe_demon(void)
{
    long globby = 0;

    for (unsigned int i = 0; i < strlen( ghost.name ); i++)
        globby += ghost.name[i];

    globby *= strlen( ghost.name );

    srand( globby );

#ifdef JP
    std::string description = "強力なデーモンだ。 $";
    description += "『";
#else
    std::string description = "A powerful demon, ";
#endif

    description += ghost.name;
#ifdef JP
    description += "』は";
#else
    description += " has a";
#endif

    switch (random2(31))
    {
    case 0:
#ifdef JP
        description += "巨大な樽のような";
#else
        description += " huge, barrel-shaped ";
#endif
        break;
    case 1:
#ifdef JP
        description += "か細く実体のない";
#else
        description += " wispy, insubstantial ";
#endif
        break;
    case 2:
#ifdef JP
        description += "ひょろりとした";
#else
        description += " spindly ";
#endif
        break;
    case 3:
#ifdef JP
        description += "骸骨の";
#else
        description += " skeletal ";
#endif
        break;
    case 4:
#ifdef JP
        description += "恐ろしい奇形の";
#else
        description += " horribly deformed ";
#endif
        break;
    case 5:
#ifdef JP
        description += "棘だらけの";
#else
        description += " spiny ";
#endif
        break;
    case 6:
#ifdef JP
        description += "痩せさらばえた";
#else
        description += " waif-like ";
#endif
        break;
    case 7:
#ifdef JP
        description += "鱗の生えた";
#else
        description += " scaly ";
#endif
        break;
    case 8:
#ifdef JP
        description += "おぞましい奇形の";
#else
        description += " sickeningly deformed ";
#endif
        break;
    case 9:
#ifdef JP
        description += "傷つき血を滲ませた";
#else
        description += " bruised and bleeding ";
#endif
        break;
    case 10:
#ifdef JP
        description += "病的な";
#else
        description += " sickly ";
#endif
        break;
    case 11:
#ifdef JP
        description += "蠢く無数の触手が生えた";
#else
        description += " mass of writhing tentacles for a ";
#endif
        break;
    case 12:
#ifdef JP
        description += "無数の粘つく蔓が生えた";
#else
        description += " mass of ropey tendrils for a ";
#endif
        break;
    case 13:
#ifdef JP
        description += "木の幹のような";
#else
        description += " tree trunk-like ";
#endif
        break;
    case 14:
#ifdef JP
        description += "毛深い";
#else
        description += " hairy ";
#endif
        break;
    case 15:
#ifdef JP
        description += "毛皮の生えた";
#else
        description += " furry ";
#endif
        break;
    case 16:
#ifdef JP
        description += "綿毛の生えた";
#else
        description += " fuzzy ";
#endif
        break;
    case 17:
#ifdef JP
        description += "肥満体の";
#else
        description += "n obese ";
#endif
        break;
    case 18:
#ifdef JP
        description += "太った";
#else
        description += " fat ";
#endif
        break;
    case 19:
#ifdef JP
        description += "ぬるぬるした";
#else
        description += " slimy ";
#endif
        break;
    case 20:
#ifdef JP
        description += "皺が寄った";
#else
        description += " wrinkled ";
#endif
        break;
    case 21:
#ifdef JP
        description += "金属質の";
#else
        description += " metallic ";
#endif
        break;
    case 22:
#ifdef JP
        description += "ガラス質の";
#else
        description += " glassy ";
#endif
        break;
    case 23:
#ifdef JP
        description += "結晶質の";
#else
        description += " crystalline ";
#endif
        break;
    case 24:
#ifdef JP
        description += "筋骨逞しい";
#else
        description += " muscular ";
#endif
        break;
    case 25:
#ifdef JP
        description += "べたつく";
#else
        description += "n icky ";
#endif
        break;
    case 26:
#ifdef JP
        description += "膨れあがった";
#else
        description += " swollen ";
#endif
        break;
    case 27:
#ifdef JP
        description += "瘤だらけの";
#else
        description += " lumpy ";
#endif
        break;
    case 28:
#ifdef JP
        description += "装甲された";
#else
        description += " armoured ";
#endif
        break;
    case 29:
#ifdef JP
        description += "甲殻に包まれた";
#else
        description += " carapaced ";
#endif
        break;
    case 30:
#ifdef JP
        description += "細身の";
#else
        description += " slender ";
#endif
        break;
    }

#ifdef JP
    description += "体をして";
#else
    description += "body";
#endif


    switch (ghost.values[GVAL_DEMONLORD_FLY])
    {
    case 1: // proper flight
        switch (random2(10))
        {
        case 0:
#ifdef JP
            description += "おり、 $昆虫に似た小さな羽根を持つ。 $";
#else
            description += " with small insectoid wings";
#endif
            break;
        case 1:
#ifdef JP
            description += "おり、 $昆虫に似た大きな羽根を持つ。 $";
#else
            description += " with large insectoid wings";
#endif
            break;
        case 2:
#ifdef JP
            description += "おり、 $蛾のような羽根を持つ。 $";
#else
            description += " with moth-like wings";
#endif
            break;
        case 3:
#ifdef JP
            description += "おり、 $蝶のような羽根を持つ。 $";
#else
            description += " with butterfly wings";
#endif
            break;
        case 4:
#ifdef JP
            description += "おり、 $蝙蝠のような巨大な翼を持つ。 $";
#else
            description += " with huge, bat-like wings";
#endif
            break;
        case 5:
#ifdef JP
            description += "おり、 $肉質の翼を持つ。 $";
#else
            description += " with fleshy wings";
#endif
            break;
        case 6:
#ifdef JP
            description += "おり、 $蝙蝠のような小さな翼を持つ。 $";
#else
            description += " with small, bat-like wings";
#endif
            break;
        case 7:
#ifdef JP
            description += "おり、 $剛毛の生えた翼を持つ。 $";
#else
            description += " with hairy wings";
#endif
            break;
        case 8:
#ifdef JP
            description += "おり、 $美しい羽毛の生えた翼を持つ。 $";
#else
            description += " with great feathered wings";
#endif
            break;
        case 9:
#ifdef JP
            description += "おり、 $輝く金属の翼を持つ。 $";
#else
            description += " with shiny metal wings";
#endif
            break;
        default:
            break;
        }
        break;

    case 2: // levitation
        if (coinflip())
#ifdef JP
            description += "おり、 $宙に浮かんでいる。 $";
#else
            description += " which hovers in mid-air";
#endif
        else
#ifdef JP
            description += "おり、 $その背のガス嚢で宙に浮いている。 $";
#else
            description += " with sacs of gas hanging from its back";
#endif
        break;

    default:  // does not fly
        switch (random2(40))
        {
        default:
#ifdef JP
            description += "いる。 $";
#endif
            break;
        case 12:
#ifdef JP
            description += "這いまわる小さな蜘蛛に覆われている。 $";
#else
            description += " covered in tiny crawling spiders";
#endif
            break;
        case 13:
#ifdef JP
            description += "這いまわる小さな昆虫に覆われている。 $";
#else
            description += " covered in tiny crawling insects";
#endif
            break;
        case 14:
#ifdef JP
            description += "ワニの頭をしている。 $";
#else
            description += " and the head of a crocodile";
#endif
            break;
        case 15:
#ifdef JP
            description += "カバの頭をしている。 $";
#else
            description += " and the head of a hippopotamus";
#endif
            break;
        case 16:
#ifdef JP
            description += "恐ろしい湾曲した嘴を持っている。 $";
#else
            description += " and a cruel curved beak for a mouth";
#endif
            break;
        case 17:
#ifdef JP
            description += "まっすぐで鋭利な嘴を持っている。 $";
#else
            description += " and a straight sharp beak for a mouth";
#endif
            break;
        case 18:
#ifdef JP
            description += "頭がついていない。 $";
#else
            description += " and no head at all";
#endif
            break;
        case 19:
#ifdef JP
            description += "口から生えた気色悪い触手が絡み合っている。 $";
#else
            description += " and a hideous tangle of tentacles for a mouth";
#endif
            break;
        case 20:
#ifdef JP
            description += "象の鼻を持っている。 $";
#else
            description += " and an elephantine trunk";
#endif
            break;
        case 21:
#ifdef JP
            description += "凶悪な顎を持っている。 $";
#else
            description += " and an evil-looking proboscis";
#endif
            break;
        case 22:
#ifdef JP
            description += "数十もの眼を持っている。 $";
#else
            description += " and dozens of eyes";
#endif
            break;
        case 23:
#ifdef JP
            description += "二つの醜悪な頭を持っている。 $";
#else
            description += " and two ugly heads";
#endif
            break;
        case 24:
#ifdef JP
            description += "長大な蛇の尾を持っている。 $";
#else
            description += " and a long serpentine tail";
#endif
            break;
        case 25:
#ifdef JP
            description += "顎から一対の巨大な牙を生やしている。 $";
#else
            description += " and a pair of huge tusks growing from its jaw";
#endif
            break;
        case 26:
            description +=
#ifdef JP
                "額の中央に巨大な単眼を持っている。 $";
#else
                " and a single huge eye, in the centre of its forehead";
#endif
            break;
        case 27:
#ifdef JP
            description += "黒い金属の牙を生やしている。 $";
#else
            description += " and spikes of black metal for teeth";
#endif
            break;
        case 28:
#ifdef JP
            description += "頭に円形の吸盤がついている。 $";
#else
            description += " and a disc-shaped sucker for a head";
#endif
            break;
        case 29:
#ifdef JP
            description += "巨大なはためく耳を持っている。 $";
#else
            description += " and huge, flapping ears";
#endif
            break;
        case 30:
#ifdef JP
            description += "胸の中央に巨大な、牙を持った顎を持っている。 $";
#else
            description += " and a huge, toothy maw in the centre of its chest";
#endif
            break;
        case 31:
#ifdef JP
            description += "背に巨大な巻き貝状の殻を持っている。 $";
#else
            description += " and a giant snail shell on its back";
#endif
            break;
        case 32:
#ifdef JP
            description += "多数の頭を持っている。 $";
#else
            description += " and a dozen heads";
#endif
            break;
        case 33:
#ifdef JP
            description += "ハイエナの頭を持っている。 $";
#else
            description += " and the head of a jackal";
#endif
            break;
        case 34:
#ifdef JP
            description += "狒々の頭を持っている。 $";
#else
            description += " and the head of a baboon";
#endif
            break;
        case 35:
#ifdef JP
            description += "巨大な、涎を流す舌を持っている。 $";
#else
            description += " and a huge, slobbery tongue";
#endif
            break;
        case 36:
#ifdef JP
            description += "全身がジクジクした裂傷で覆われている。 $";
#else
            description += " which is covered in oozing lacerations";
#endif
            break;
        case 37:
#ifdef JP
            description += "カエルの頭を持っている。 $";
#else
            description += " and the head of a frog";
#endif
            break;
        case 38:
#ifdef JP
            description += "ヤク牛の頭を持っている。 $";
#else
            description += " and the head of a yak";
#endif
            break;
        case 39:
#ifdef JP
            description += "柄の先に付いた眼を持っている。 $";
#else
            description += " and eyes out on stalks";
#endif
            break;
        }
        break;
    }

#ifdef JP
    //description += ".";
#else
    description += ".";
#endif

    switch (random2(40) + (you.species == SP_MUMMY ? 3 : 0))
    {
    case 0:
#ifdef JP
        description += "このデーモンは硫黄の臭いを放っている。";
#else
        description += " It stinks of brimstone.";
#endif
        break;
    case 1:
#ifdef JP
        description += "このデーモンは腐肉の臭いを放っている。";
#else
        description += " It smells like rotting flesh";
#endif
        if (you.species == SP_GHOUL)
#ifdef JP
            description += "うまそうだ！ $";
#else
            description += " - yum!";
#endif
        else
#ifdef JP
            description += " $";
#else
            description += ".";
#endif
        break;
    case 2:
#ifdef JP
        description += "このデーモンは吐き気を催す悪臭に包まれている。 $";
#else
        description += " It is surrounded by a sickening stench.";
#endif
        break;
    case 3:
#ifdef JP
        description += "このデーモンは生ある者への憎悪に怒り狂っている。 $";
#else
        description += " It seethes with hatred of the living.";
#endif
        break;
    case 4:
#ifdef JP
        description += "小さなオレンジの炎がその周囲に踊っている。 $";
#else
        description += " Tiny orange flames dance around it.";
#endif
        break;
    case 5:
#ifdef JP
        description += "小さな紫の炎がその周囲に踊っている。 $";
#else
        description += " Tiny purple flames dance around it.";
#endif
        break;
    case 6:
#ifdef JP
        description += "このデーモンは気味悪い薄霧に包まれている。 $";
#else
        description += " It is surrounded by a weird haze.";
#endif
        break;
    case 7:
#ifdef JP
        description += "このデーモンは邪悪な光に輝いている。 $";
#else
        description += " It glows with a malevolent light.";
#endif
        break;
    case 8:
#ifdef JP
        description += "このデーモンは途方もないほどに激昂している。 $";
#else
        description += " It looks incredibly angry.";
#endif
        break;
    case 9:
#ifdef JP
        description += "このデーモンは粘液を滲出させている。 $";
#else
        description += " It oozes with slime.";
#endif
        break;
    case 10:
#ifdef JP
        description += "このデーモンは絶え間なく涎を垂れ流している。 $";
#else
        description += " It dribbles constantly.";
#endif
        break;
    case 11:
#ifdef JP
        description += "このデーモンは全身いたる所にカビを生やしている。 $";
#else
        description += " Mould grows all over it.";
#endif
        break;
    case 12:
#ifdef JP
        description += "このデーモンは病的な様子だ。 $";
#else
        description += " It looks diseased.";
#endif
        break;
    case 13:
#ifdef JP
        description += "このデーモンはあなたが彼に脅えているのと同じ位、 $"
                       "あなたに対して脅えあがっているように見える。 ";
#else
        description += " It looks as frightened of you as you are of it.";
#endif
        break;
    case 14:
#ifdef JP
        description += "このデーモンはひどい痙攣を起こしながら動く。 $";
#else
        description += " It moves in a series of hideous convulsions.";
#endif
        break;
    case 15:
#ifdef JP
        description += "このデーモンは信じ難いほど優雅に動く。 $";
#else
        description += " It moves with an unearthly grace.";
#endif
        break;
    case 16:
#ifdef JP
        description += "このデーモンはあなたの魂を渇望している！ $";
#else
        description += " It hungers for your soul!";
#endif
        break;
    case 17:
#ifdef JP
        description += "このデーモンは光る油の足跡を残していく。 $";
#else
        description += " It leaves a glistening oily trail.";
#endif
        break;
    case 18:
#ifdef JP
        description += "このデーモンの姿は明滅している。 $";
#else
        description += " It shimmers before your eyes.";
#endif
        break;
    case 19:
#ifdef JP
        description += "このデーモンは輝く光に取り巻かれている。 $";
#else
        description += " It is surrounded by a brilliant glow.";
#endif
        break;
    case 20:
#ifdef JP
        description += "このデーモンは凄まじい力の霊気を放っている。 $";
#else
        description += " It radiates an aura of extreme power.";
#endif
        break;
    default:
        break;
    }

    return description;
}                               // end describe_demon()


//---------------------------------------------------------------
//
// describe_weapon
//
//---------------------------------------------------------------
static std::string describe_weapon( const item_def &item, char verbose)
{
    std::string description;

    description.reserve(200);

    description = "";

    if (is_fixed_artefact( item ))
    {
        if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
        {
            description += "$";

            switch (item.special)
            {
            case SPWPN_SINGING_SWORD:
#ifdef JP
                description += "この剣は所有者に歌いかけることを何より愛す。 $"
                               "……所有者の望むと望まざるに関らず。 ";
#else
                description += "This blessed weapon loves nothing more "
                    "than to sing to its owner, "
                    "whether they want it to or not. ";
#endif
                break;
            case SPWPN_WRATH_OF_TROG:
#ifdef JP
                description += "この剣は旧き神トログが失くしたもので、彼のお気に入りの品だった。 $"
                               "これは振るう者全てに血に飢えた狂戦士の怒りを誘発する。 ";
#else
                description += "This was the favourite weapon of "
                    "the old god Trog, before he lost it one day. "
                    "It induces a bloodthirsty berserker rage in "
                    "anyone who uses it to strike another. ";
#endif
                break;
            case SPWPN_SCYTHE_OF_CURSES:
#ifdef JP
                description += "この武器には恐ろしく非常に腹立たしい呪いがかかっている。 ";
#else
                description += "This weapon carries a "
                    "terrible and highly irritating curse. ";
#endif
                break;
            case SPWPN_MACE_OF_VARIABILITY:
#ifdef JP
                description += "この武器はどうも当てにならない。 ";
#else
                description += "It is rather unreliable. ";
#endif
                break;
            case SPWPN_GLAIVE_OF_PRUNE:
#ifdef JP
                description += "この武器は狂った神の創造物で、 $"
                               "所有者をプルーンに変える呪いがかかっている。 $"
                               "幸いなことに呪いはゆっくり進行するので、 $"
                               "短期間使うだけであれば皮膚が少し紫色になり、 $"
                               "皺が寄ってくるという以上の実害はない。 ";
#else
                description += "It is the creation of a mad god, and "
                    "carries a curse which transforms anyone "
                    "possessing it into a prune. Fortunately, "
                    "the curse works very slowly, and one can "
                    "use it briefly with no consequences "
                    "worse than slightly purple skin and a few wrinkles. ";
#endif
                break;
            case SPWPN_SCEPTRE_OF_TORMENT:
#ifdef JP
                description += "この忌まわしい武器は、地獄の拷問具だ。 ";
#else
                description += "This truly accursed weapon is "
                    "an instrument of Hell. ";
#endif
                break;
            case SPWPN_SWORD_OF_ZONGULDROK:
#ifdef JP
                description += "この恐るべき武器は使用者を危険に晒す。 ";
#else
                description += "This dreadful weapon is used "
                    "at the user's peril. ";
#endif
                break;
            case SPWPN_SWORD_OF_CEREBOV:
#ifdef JP
                description += "薄気味の悪い炎がこの捩じれた刃を包んでいる。 ";
#else
                description += "Eerie flames cover its twisted blade. ";
#endif
                break;
            case SPWPN_STAFF_OF_DISPATER:
#ifdef JP
                description += "この伝説的な品は、地獄の猛威を放つことができる。 ";
#else
                description += "This legendary item can unleash "
                    "the fury of Hell. ";
#endif
                break;
            case SPWPN_SCEPTRE_OF_ASMODEUS:
#ifdef JP
                description += "これは悪魔の首領であるアスモデウスの力を幾らか孕んでいる。 ";
#else
                description += "It carries some of the powers of "
                    "the arch-fiend Asmodeus. ";
#endif
                break;
            case SPWPN_SWORD_OF_POWER:
#ifdef JP
                description += "この武器は力ある者には激しく、 $"
                               "力なき者には穏やかに応える。 ";
#else
                description += "It rewards the powerful with power "
                    "and the meek with weakness. ";
#endif
                break;
            case SPWPN_KNIFE_OF_ACCURACY:
#ifdef JP
                description += "この武器はほとんど的を外さないほどに正確だ。 ";
#else
                description += "It is almost unerringly accurate. ";
#endif
                break;
            case SPWPN_STAFF_OF_OLGREB:
#ifdef JP
                description += "これは力ある魔法使いオルグレブが帯びていた魔法の武器だ。 $"
                               "彼はダンジョンでの探索中に命を落とした。 $"
                               "これは手にした者に毒への耐性を与え、毒の魔術を操る能力を増大させ、 $"
                               "更にはこの杖に固有な魔法の力を喚起する能力を与える。 ";
#else
                description += "It was the magical weapon wielded by the "
                    "mighty wizard Olgreb before he met his "
                    "fate somewhere within these dungeons. It "
                    "grants its wielder resistance to the "
                    "effects of poison and increases their "
                    "ability to use venomous magic, and "
                    "carries magical powers which can be evoked. ";
#endif
                break;
            case SPWPN_VAMPIRES_TOOTH:
#ifdef JP
                description += "これは致命的な吸血の武器だ。 ";
#else
                description += "It is lethally vampiric. ";
#endif
                break;
            case SPWPN_STAFF_OF_WUCAD_MU:
#ifdef JP
                description += "この武器は手にした者の知力に応じて威力が変化する。 $"
                               "これを扱うには少しだけ危険が伴う。 ";
#else
                description += "Its power varies in proportion to "
                    "its wielder's intelligence. "
                    "Using it can be a bit risky. ";
#endif
                break;
            }

            description += "$";
        }
        else if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            // We know it's an artefact type weapon, but not what it does.
#ifdef JP
            description += "$この武器には幾つかの隠された特性があるかもしれない。$";
#else
            description += "$This weapon may have some hidden properties.$";
#endif
        }
    }
    else if (is_unrandom_artefact( item )
        && strlen(unrandart_descrip(1, item)) != 0)
    {
        description += unrandart_descrip(1, item);
        description += "$";
    }
    else
    {
        if (verbose == 1)
        {
            switch (item.sub_type)
            {
            case WPN_CLUB:
#ifdef JP
                description += "重量がある木の棍棒だ。 ";
#else
                description += "A heavy piece of wood. ";
#endif
                break;

            case WPN_MACE:
#ifdef JP
                description += "長い柄の先に重量のある金属塊がついた武器だ。 ";
#else
                description += "A long handle "
                    "with a heavy lump on one end. ";
#endif
                break;

            case WPN_FLAIL:
#ifdef JP
                description += "メイスに似ているが、柄と金属塊が鎖で繋がれている。 ";
#else
                description += "Like a mace, but with a length of chain "
                    "between the handle and the lump of metal. ";
#endif
                break;

            case WPN_DAGGER:
#ifdef JP
                description += "長いナイフ、あるいは非常に小さな剣だ。 $"
                               "手で振るうことも投げることもできる。 ";
#else
                description += "A long knife or a very short sword, "
                    "which can be held or thrown. ";
#endif
                break;

            case WPN_KNIFE:
#ifdef JP
                description += "簡素な野営用ナイフで、戦闘よりは道具として用いるために造られている。 $"
                               "これは肉を捌くのに向いている。 ";
#else
                description += "A simple survival knife. "
                    "Designed more for utility than combat, "
                    "it looks quite capable of butchering a corpse. ";
#endif
                break;

            case WPN_MORNINGSTAR:
#ifdef JP
                description += "スパイクに覆われたメイスだ。 ";
#else
                description += "A mace covered in spikes. ";
#endif
                break;

            case WPN_SHORT_SWORD:
#ifdef JP
                description += "切ることを目的とした短めの刃の剣だ。 ";
#else
                description += "A sword with a short, slashing blade. ";
#endif
                break;

            case WPN_LONG_SWORD:
#ifdef JP
                description += "切ることを目的とした長い刃の剣だ。 ";
#else
                description += "A sword with a long, slashing blade. ";
#endif
                break;

            case WPN_GREAT_SWORD:
#ifdef JP
                description += "非常に長くて重い刃と、長い柄を持つ剣だ。 ";
#else
                description += "A sword with a very long, heavy blade "
                    "and a long handle. ";
#endif
                break;

            case WPN_SCIMITAR:
#ifdef JP
                description += "湾曲した刃を持つ長剣だ。 ";
#else
                description += "A long sword with a curved blade. ";
#endif
                break;

            case WPN_HAND_AXE:
#ifdef JP
                description += "白兵戦にも投擲にも適すように造られた小振りの斧だ。 ";
#else
                description += "An small axe designed for either hand combat "
                               "or throwing. ";
#endif
                               // "It might also make a good tool.";
                break;

            case WPN_BATTLEAXE:
#ifdef JP
                description += "大きな両刃の斧だ。 ";
#else
                description += "A large axe with a double-headed blade. ";
#endif
                break;

            case WPN_SPEAR:
#ifdef JP
                description += "先端に刺すための穂先がついた長い棒だ。 $"
                               "手で振るうことも投げることもできる。 ";
#else
                description += "A long stick with a pointy blade on one end, "
                    "to be held or thrown. ";
#endif
                break;

            case WPN_TRIDENT:
                description +=
#ifdef JP
                    "三叉の穂先がついた長柄の武器だ。 ";
#else
                    "A hafted weapon with three points at one end. ";
#endif
                break;

            case WPN_HALBERD:
                description +=
#ifdef JP
                    "斧と穂先がついた長柄の武器だ。 ";
#else
                    "A long pole with a spiked axe head on one end. ";
#endif
                break;

            case WPN_SLING:
                description +=
#ifdef JP
                    "布切れと皮紐でできた投石器だ。 $"
                    "それほどのダメージは期待できない。 ";
#else
                    "A piece of cloth and leather for launching stones, "
                    "which do a small amount of damage on impact. ";
#endif
                break;

            case WPN_BOW:
#ifdef JP
                description += "湾曲した木材と弦からなり、矢を射るのに用いる。 $"
                               "戦闘において大きなダメージを与えることを期待でき、 $"
                               "とりわけ熟練者が手にした場合は恐るべき威力を持つ。 ";
#else
                description += "A curved piece of wood and string, "
                    "for shooting arrows. It does good damage in combat, "
                    "and a skilled user can use it to great effect. ";
#endif
                break;

            case WPN_BLOWGUN:
#ifdef JP
                description += "両側に穴が空いた長くて軽い管だ。非常に小さなダメージしか $"
                               "期待できないため、主な用途は遠くから毒の塗られた針を射つことである。 $"
                               "吹き矢は射つ時にほとんど音を立てない。 ";
#else
                description += "A long, light tube, open at both ends.  Doing "
                    "very little damage,  its main use is to fire poisoned "
                    "needles from afar.  It makes very little noise. ";
#endif
                break;

            case WPN_CROSSBOW:
#ifdef JP
                description += "矢を射るための機械的な装置で、 $"
                               "矢を装填するのに若干の時間を必要とする。 $"
                               "戦闘において非常に大きなダメージを与える。 ";
#else
                description += "A piece of machinery used for firing bolts, "
                    "which takes some time to load and fire. "
                    "It does very good damage in combat. ";
#endif
                break;

            case WPN_HAND_CROSSBOW:
#ifdef JP
                description += "投げ矢を射るのに用いる小型のクロスボウだ。 ";
#else
                description += "A small crossbow, for firing darts. ";
#endif
                break;

            case WPN_GLAIVE:
                description +=
#ifdef JP
                    "長くて重い刃のついた長柄武器だ。 ";
#else
                    "A pole with a large, heavy blade on one end. ";
#endif
                break;

            case WPN_QUARTERSTAFF:
#ifdef JP
                description += "頑丈な木製の棒だ。 ";
#else
                description += "A sturdy wooden pole. ";
#endif
                break;

            case WPN_SCYTHE:
                description +=
#ifdef JP
                    "これは農機具であり、一般的には戦闘にあまり適さない。 ";
#else
                    "A farm implement, usually unsuited to combat. ";
#endif
                break;

            case WPN_GIANT_CLUB:
#ifdef JP
                description += "巨大な木の棍棒だ。 $"
                               "オーガの手で振るわれるために造られている。 ";
#else
                description += "A giant lump of wood, "
                    "shaped for an ogre's hands. ";
#endif
                break;

            case WPN_GIANT_SPIKED_CLUB:
                description +=
#ifdef JP
                    "先端に鋭いスパイクが植えられた巨大な木の棍棒だ。 ";
#else
                    "A giant lump of wood with sharp spikes at one end. ";
#endif
                break;

            case WPN_EVENINGSTAR:
#ifdef JP
                description += "モーニングスターの対極となる武器だ。 ";
#else
                description += "The opposite of a morningstar. ";
#endif
                break;

            case WPN_QUICK_BLADE:
#ifdef JP
                description += "魔術的に鋭い、小さな剣だ。 ";
#else
                description += "A small and magically quick sword. ";
#endif
                break;

            case WPN_KATANA:
#ifdef JP
                description += "非常に珍しく、また非常に切れ味鋭い異国の武器だ。 $"
                               "長い片刃の刀身を特徴とする。 ";
#else
                description += "A very rare and extremely effective "
                    "imported weapon, featuring a long "
                    "single-edged blade. ";
#endif
                break;

            case WPN_EXECUTIONERS_AXE:
#ifdef JP
                description += "巨大な斧だ。 ";
#else
                description += "A huge axe. ";
#endif
                break;

            case WPN_DOUBLE_SWORD:
                description +=
#ifdef JP
                    "二本の鋭利な刃を持つ魔法の武器だ。 ";
#else
                    "A magical weapon with two razor-sharp blades. ";
#endif
                break;

            case WPN_TRIPLE_SWORD:
#ifdef JP
                description += "三本の非常に鋭利な刃を持つ魔法の武器だ。 ";
#else
                description += "A magical weapon with three "
                    "great razor-sharp blades. ";
#endif
                break;

            case WPN_HAMMER:
#ifdef JP
                description += "戦闘用に改良された金槌だ。 ";
#else
                description += "The kind of thing you hit nails with, "
                    "adapted for battle. ";
#endif
                break;

            case WPN_ANCUS:
#ifdef JP
                description += "邪悪な歯が植え込まれた大きな棍棒だ。 ";
#else
                description += "A large and vicious toothed club. ";
#endif
                break;

            case WPN_WHIP:
#ifdef JP
                description += "普通の鞭だ。 ";
#else
                description += "A whip. ";
#endif
                break;

            case WPN_SABRE:
#ifdef JP
                description += "切ることを目的とした程よい長さの刃の剣だ。 ";
#else
                description += "A sword with a medium length slashing blade. ";
#endif
                break;

            case WPN_DEMON_BLADE:
                description +=
#ifdef JP
                    "地獄の火で鍛えられた恐るべき武器だ。 ";
#else
                    "A terrible weapon, forged in the fires of Hell. ";
#endif
                break;

            case WPN_DEMON_WHIP:
#ifdef JP
                description += "地獄の深層で造られた恐るべき武器だ。 ";
#else
                description += "A terrible weapon, woven "
                    "in the depths of the inferno. ";
#endif
                break;

            case WPN_DEMON_TRIDENT:
                description +=
#ifdef JP
                    "地獄の業火で鍛えられた恐るべき武器だ。 ";
#else
                    "A terrible weapon, molded by fire and brimstone. ";
#endif
                break;

            case WPN_BROAD_AXE:
#ifdef JP
                description += "大きな刃の斧だ。 ";
#else
                description += "An axe with a large blade. ";
#endif
                break;

            case WPN_WAR_AXE:
#ifdef JP
                description += "白兵戦を目的に造られた斧だ。 ";
#else
                description += "An axe intended for hand to hand combat. ";
#endif
                break;

            case WPN_SPIKED_FLAIL:
                description +=
#ifdef JP
                    "先端の金属塊に大きなスパイクが幾つも植え込まれたフレイルだ。 ";
#else
                    "A flail with large spikes on the metal lump. ";
#endif
                break;

            case WPN_GREAT_MACE:
#ifdef JP
                description += "特に大きくて重量があるメイスだ。 ";
#else
                description += "A large and heavy mace. ";
#endif
                break;

            case WPN_GREAT_FLAIL:
#ifdef JP
                description += "特に大きくて重量があるフレイルだ。 ";
#else
                description += "A large and heavy flail. ";
#endif
                break;

            case WPN_FALCHION:
#ifdef JP
                description += "切ることを目的とした広刃の剣だ。 ";
#else
                description += "A sword with a broad slashing blade. ";
#endif
                break;

            default:
#ifdef JP
                DEBUGSTR("知られざる武器");
#else
                DEBUGSTR("Unknown weapon");
#endif
            }

            description += "$";
        }
    }

    if (verbose == 1 && !launches_things( item.sub_type ))
    {
#ifdef JP
        description += "$ダメージレート: ";
#else
        description += "$Damage rating: ";
#endif
        append_value(description, property( item, PWPN_DAMAGE ), false);

#ifdef JP
        description += "$命中レート: ";
#else
        description += "$Accuracy rating: ";
#endif
        append_value(description, property( item, PWPN_HIT ), true);

#ifdef JP
        description += "$基本攻撃速度: ";
#else
        description += "$Base attack delay: ";
#endif
        append_value(description, property( item, PWPN_SPEED ) * 10, false);
        description += "%%";
    }
    description += "$";

    if (!is_fixed_artefact( item ))
    {
        int spec_ench = get_weapon_brand( item );

        if (!is_random_artefact( item ) && verbose == 0)
            spec_ench = SPWPN_NORMAL;

        // special weapon descrip
        if (spec_ench != SPWPN_NORMAL && item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            description += "$";

            switch (spec_ench)
            {
            case SPWPN_FLAMING:
#ifdef JP
                description += "これは炎を放っており、大抵の敵に追加ダメージを与える。 $"
                               "炎が苦手な敵には倍の追加ダメージを与える。 ";
#else
                description += "It emits flame when wielded, "
                    "causing extra injury to most foes "
                    "and up to double damage against "
                    "particularly susceptible opponents. ";
#endif
                break;
            case SPWPN_FREEZING:
#ifdef JP
                description += "これは命中すると相手を凍らせ、大抵の敵に追加ダメージを与える。 $"
                               "冷気が苦手な敵には倍の追加ダメージを与える。 ";
#else
                description += "It has been specially enchanted to "
                    "freeze those struck by it, causing "
                    "extra injury to most foes and "
                    "up to double damage against "
                    "particularly susceptible opponents. ";
#endif
                break;
            case SPWPN_HOLY_WRATH:
#ifdef JP
                description += "これは『輝けるもの』の祝福を受けている。 $"
                               "特にアンデットに効果があり、また地獄とパンデモニウムの邪悪な存在にも $"
                               "大きなダメージを与える。 ";
#else
                description += "It has been blessed by the Shining One "
                    "to harm undead and cause great damage to "
                    "the unholy creatures of Hell or Pandemonium. ";
#endif
                break;
            case SPWPN_ELECTROCUTION:
#ifdef JP
                description += "これは敵に命中した際に時として電気エネルギーを放出し、 $"
                               "恐ろしい損害を与える。 ";
#else
                description += "Occasionally upon striking a foe "
                    "it will discharge some electrical energy "
                    "and cause terrible harm. ";
#endif
                break;
            case SPWPN_ORC_SLAYING:
#ifdef JP
                description += "これはオークの血族に対して特に効果的だ。 ";
#else
                description += "It is especially effective against "
                    "all of orcish descent. ";
#endif
                break;
            case SPWPN_VENOM:
                if (launches_things( item.sub_type ))
#ifdef JP
                    description += "これは発射する矢や弾に毒を付与する。 ";
#else
                    description += "It poisons the unbranded ammo it fires. ";
#endif
                else
#ifdef JP
                    description += "これは命中した相手を毒に冒す。 ";
#else
                    description += "It poisons the flesh of those it strikes. ";
#endif
                break;
            case SPWPN_PROTECTION:
#ifdef JP
                description += "これは手にした者を負傷から保護する。(+5 AC) ";
#else
                description += "It protects the one who wields it against "
                    "injury (+5 to AC). ";
#endif
                break;
            case SPWPN_DRAINING:
#ifdef JP
                description += "実に恐るべき武器である。 $"
                               "これは命中すると相手の生命力を衰弱させる。 ";
#else
                description += "A truly terrible weapon, "
                    "it drains the life of those it strikes. ";
#endif
                break;
            case SPWPN_SPEED:
                if (launches_things( item.sub_type ))
                {
#ifdef JP
                    description += "これは手にした者に２倍の速さでの射撃を可能とさせる。 ";
#else
                    description += "It allows its wielder to fire twice when "
                           "they would otherwise have fired only once. ";
#endif
                }
                else
                {
#ifdef JP
                    description += "これは手にした者に２倍の速さでの攻撃を可能とさせる。 ";
#else
                    description += "It allows its wielder to attack twice when "
                           "they would otherwise have struck only once. ";
#endif
                }
                break;
            case SPWPN_VORPAL:
#ifdef JP
                description += "これは敵に対して強烈なダメージを与える。 ";
#else
                description += "It inflicts extra damage upon your enemies. ";
#endif
                break;
            case SPWPN_FLAME:
#ifdef JP
                description += "これは発射された矢や弾を炎で包む。 ";
#else
                description += "It turns projectiles fired from it into "
                    "bolts of fire. ";
#endif
                break;
            case SPWPN_FROST:
#ifdef JP
                description += "これは発射された矢や弾を凍気で包む。 ";
#else
                description += "It turns projectiles fired from it into "
                    "bolts of frost. ";
#endif
                break;
            case SPWPN_VAMPIRICISM:
#ifdef JP
                description += "これは追加のダメージは与えないが、 $"
                               "生命ある敵を攻撃した際に、装備した者を幾らか癒す。 ";
#else
                description += "It inflicts no extra harm, "
                    "but heals its wielder somewhat when "
                    "he or she strikes a living foe. ";
#endif
                break;
            case SPWPN_DISRUPTION:
#ifdef JP
                description += "これは『ジン』の祝福を受けた武器だ。 $"
                               "アンデッドに対して最大で４倍までのダメージを与えることができる。 ";
#else
                description += "It is a weapon blessed by Zin, "
                    "and can inflict up to fourfold damage "
                    "when used against the undead. ";
#endif
                break;
            case SPWPN_PAIN:
#ifdef JP
                description += "これは死霊術の熟練者の手に振るわれる時、 $"
                               "生命あるモンスターに追加のダメージを及ぼす。 ";
#else
                description += "In the hands of one skilled in "
                    "necromantic magic it inflicts "
                    "extra damage on living creatures. ";
#endif
                break;
            case SPWPN_DISTORTION:
#ifdef JP
                description += "これは周囲の空間を捻じ曲げ歪ませる。 ";
#else
                description += "It warps and distorts space around it. ";
#endif
                break;
            case SPWPN_REACHING:
#ifdef JP
                description += "これは発動によって射程を伸ばすことができる。 ";
#else
                description += "It can be evoked to extend its reach. ";
#endif
                break;
            }
        }

        if (is_random_artefact( item ))
        {
            if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
            {
                unsigned int old_length = description.length();
                randart_descpr( description, item );

                if (description.length() == old_length)
                    description += "$";
            }
            else if (item_ident( item, ISFLAG_KNOW_TYPE ))
            {
#ifdef JP
                description += "$この武器には幾つかの隠された特性があるかもしれない。$";
#else
                description += "$This weapon may have some hidden properties.$";
#endif
            }
        }
        else if (spec_ench != SPWPN_NORMAL && item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            description += "$";
        }
    }

    if (item_known_cursed( item ))
    {
#ifdef JP
        description += "$これには呪いがかかっている。 ";
#else
        description += "$It has a curse placed upon it.";
#endif
    }

    if (verbose == 1 && !launches_things( item.sub_type ))
    {
#ifdef USE_NEW_COMBAT_STATS
        const int str_weight = weapon_str_weight( item.base_type, item.sub_type );

        if (str_weight >= 8)
#ifdef JP
            description += "$この武器は腕力が強い者に最適だ。";
#else
            description += "$This weapon is best used by the strong.";
#endif
        else if (str_weight > 5)
#ifdef JP
            description += "$この武器は腕力が強い者に向いている。";
#else
            description += "$This weapon is better for the strong.";
#endif
        else if (str_weight <= 2)
#ifdef JP
            description += "$この武器は動きが敏捷な者に最適だ。";
#else
            description += "$This weapon is best used by the dexterous.";
#endif
        else if (str_weight < 5)
#ifdef JP
            description += "$この武器は動きが敏捷な者に向いている。";
#else
            description += "$This weapon is better for the dexterous.";
#endif
#endif

        switch (hands_reqd_for_weapon(item.base_type, item.sub_type))
        {
        case HANDS_ONE_HANDED:
#ifdef JP
            description += "$これは片手武器だ。";
#else
            description += "$It is a one handed weapon.";
#endif
            break;
        case HANDS_ONE_OR_TWO_HANDED:
#ifdef JP
            description += "$これは片手で振るうことができるが、 $"
                           "両手で扱えば(つまり盾を使わなければ)更に効果的だ。";
#else
            description += "$It can be used with one hand, or more "
                    "effectively with two (i.e. when not using a shield).";
#endif
            break;
        case HANDS_TWO_HANDED:
#ifdef JP
            description += "$これは両手武器だ。 ";
#else
            description += "$It is a two handed weapon.";
#endif
            break;
        }
    }

    if (!is_random_artefact( item ))
    {
        switch (get_equip_race( item ))
        {
        case ISFLAG_DWARVEN:
#ifdef JP
            description += "$これは巧みに造られていて、非常に耐久力がある。";
#else
            description += "$It is well-crafted and very durable.";
#endif
            break;
        }

        if (launches_things( item.sub_type ))
        {
            switch (get_equip_race( item ))
            {
            case ISFLAG_DWARVEN:
#ifdef JP
                description += "$これはドワーフの矢や弾を放つ時に、 "
                               "最高の効果を発揮する。";
#else
                description += "$It is most deadly when used with "
                    "dwarven ammunition.";
#endif
                break;
            case ISFLAG_ELVEN:
#ifdef JP
                description += "$これはエルフの矢や弾を放つ時に、 "
                               "最高の効果を発揮する。";
#else
                description += "$It is most deadly when used with "
                    "elven ammunition.";
#endif
                break;
            case ISFLAG_ORCISH:
#ifdef JP
                description += "$これはオークの矢や弾を放つ時に、 "
                               "最高の効果を発揮する。";
#else
                description += "$It is most deadly when used with "
                    "orcish ammunition.";
#endif
                break;
            }
        }
    }

    if (verbose == 1)
    {
#ifdef JP
        description += "$これは";
#else
        description += "$It falls into the";
#endif

        switch (item.sub_type)
        {
        case WPN_SLING:
#ifdef JP
            description += "スリングに分類される。 ";
#else
            description += " 'slings' category. ";
#endif
            break;
        case WPN_BOW:
#ifdef JP
            description += "弓に分類される。 ";
#else
            description += " 'bows' category. ";
#endif
            break;
        case WPN_HAND_CROSSBOW:
        case WPN_CROSSBOW:
#ifdef JP
            description += "クロスボウに分類される。 ";
#else
            description += " 'crossbows' category. ";
#endif
            break;
        case WPN_BLOWGUN:
#ifdef JP
            description += "投げ矢に分類される。 ";
#else
            description += " 'darts' category. ";
#endif
            break;
        default:
            // Melee weapons
            switch (weapon_skill(item.base_type, item.sub_type))
            {
            case SK_SHORT_BLADES:
#ifdef JP
                description += "短剣に分類される。 ";
#else
                description += " 'short blades' category. ";
#endif
                break;
            case SK_LONG_SWORDS:
#ifdef JP
                description += "長剣に分類される。 ";
#else
                description += " 'long swords' category. ";
#endif
                break;
            case SK_AXES:
#ifdef JP
                description += "斧に分類される。 ";
#else
                description += " 'axes' category. ";
#endif
                break;
            case SK_MACES_FLAILS:
#ifdef JP
                description += "鈍器に分類される。 ";
#else
                description += " 'maces and flails' category. ";
#endif
                break;
            case SK_POLEARMS:
#ifdef JP
                description += "長柄武器に分類される。 ";
#else
                description += " 'pole-arms' category. ";
#endif
                break;
            case SK_STAVES:
#ifdef JP
                description += "棒杖に分類される。 ";
#else
                description += " 'staves' category. ";
#endif
                break;
            default:
#ifdef JP
                description += "バグに分類される ";
                DEBUGSTR("Unknown weapon type");
#else
                description += " 'bug' category. ";
                DEBUGSTR("Unknown weapon type");
#endif
                break;
            }
        }
    }

    return (description);
}


//---------------------------------------------------------------
//
// describe_ammo
//
//---------------------------------------------------------------
static std::string describe_ammo( const item_def &item )
{
    std::string description;

    description.reserve(64);

    switch (item.sub_type)
    {
    case MI_STONE:
#ifdef JP
        description += "これは小石だ。 ";
#else
        description += "A stone. ";
#endif
        break;
    case MI_ARROW:
#ifdef JP
        description += "これは矢だ。 ";
#else
        description += "An arrow. ";
#endif
        break;
    case MI_NEEDLE:
#ifdef JP
        description += "これは吹き矢針だ。 ";
#else
        description += "A needle. ";
#endif
        break;
    case MI_BOLT:
#ifdef JP
        description += "これはクロスボウの矢だ。 ";
#else
        description += "A crossbow bolt. ";
#endif
        break;
    case MI_DART:
#ifdef JP
        description += "これは小さな手投げ武器だ。 ";
#else
        description += "A small throwing weapon. ";
#endif
        break;
    case MI_LARGE_ROCK:
#ifdef JP
        description += "これは岩だ。巨人によって飛び道具として使われる。 ";
#else
        description += "A rock, used by giants as a missile. ";
#endif
        break;
    case MI_EGGPLANT:
#ifdef JP
        description += "紫色の野菜だ $"
                       "ゲーム中にこれが存在したらそれはバグを示す。 $"
                       "(もしくはあなたの側でチートしているのかもしれないが) ";
#else
        description += "A purple vegetable. "
            "The presence of this object in the game "
            "indicates a bug (or some kind of cheating on your part). ";
#endif
        break;
    default:
#ifdef JP
        DEBUGSTR("知られざる飛び道具");
#else
        DEBUGSTR("Unknown ammo type");
#endif
        break;
    }

    if (item.special != 0 && item_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (item.special)
        {
        case 1:
#ifdef JP
            description += "$然るべき手段で撃ち出されたなら、これは炎の矢に変化する。 $";
#else
            description += "$When fired from an appropriate launcher, "
                "it turns into a bolt of flame. ";
#endif
            break;
        case 2:
#ifdef JP
            description += "$然るべき手段で撃ち出されたなら、これは氷の矢に変化する。 $";
#else
            description += "$When fired from an appropriate launcher, "
                "it turns into a bolt of ice. ";
#endif
            break;
        case 3:
        case 4:
#ifdef JP
            description += "$これには毒が塗られている。 ";
#else
            description += "$It is coated with poison. ";
#endif
            break;
        }
    }

    description += "$";

    return (description);
}


//---------------------------------------------------------------
//
// describe_armour
//
//---------------------------------------------------------------
static std::string describe_armour( const item_def &item, char verbose )
{
    std::string description;

    description.reserve(200);

    if (is_unrandom_artefact( item )
        && strlen(unrandart_descrip(1, item)) != 0)
    {
        description += "$";
        description += unrandart_descrip(1, item);
        description += "$$";
    }
    else
    {
        if (verbose == 1)
        {
            switch (item.sub_type)
            {
            case ARM_ROBE:
#ifdef JP
                description += "布製のローブだ。 ";
#else
                description += "A cloth robe. ";
#endif
                break;
            case ARM_LEATHER_ARMOUR:
#ifdef JP
                description += "硬くなめされた革の鎧だ。 ";
#else
                description += "A suit made of hardened leather. ";
#endif
                break;
            case ARM_RING_MAIL:
#ifdef JP
                description += "革の上着にたくさんの金属の環が縫い付けられた鎧だ。 ";
#else
                description += "A leather suit covered in little rings. ";
#endif
                break;
            case ARM_SCALE_MAIL:
                description +=
#ifdef JP
                    "革の上着にたくさんの金属の小板が縫い付けられた鎧だ。 ";
#else
                    "A leather suit covered in little metal plates. ";
#endif
                break;
            case ARM_CHAIN_MAIL:
#ifdef JP
                description += "金属の鎖で組み上げられた鎧だ。 ";
#else
                description += "A suit made of interlocking metal rings. ";
#endif
                break;
            case ARM_SPLINT_MAIL:
#ifdef JP
                description += "金属の板で作られた鎧だ。 ";
#else
                description += "A suit made of splints of metal. ";
#endif
                break;
            case ARM_BANDED_MAIL:
#ifdef JP
                description += "金属の帯で作られた鎧だ。 ";
#else
                description += "A suit made of bands of metal. ";
#endif
                break;
            case ARM_PLATE_MAIL:
#ifdef JP
                description += "広範囲を板金で覆った鎧だ。 ";
#else
                description += "A suit of mail and large plates of metal. ";
#endif
                break;
            case ARM_SHIELD:
                description +=
#ifdef JP
                    "敵の武器を受けとめるための金属の盾だ。 $"
                    "装備するとその重量のため、あなたの攻撃の速度は $"
                    "少し遅くなるだろう。 ";
#else
                    "A piece of metal, to be strapped on one's arm. "
                    "It is cumbersome to wear, and slightly slows "
                    "the rate at which you may attack. ";
#endif
                break;
            case ARM_CLOAK:
#ifdef JP
                description += "布製のクロークだ。 ";
#else
                description += "A cloth cloak. ";
#endif
                break;

            case ARM_HELMET:
                switch (get_helmet_type( item ))
                {
                case THELM_HELMET:
                case THELM_HELM:
#ifdef JP
                    description += "金属製の被り物だ。 ";
#else
                    description += "A piece of metal headgear. ";
#endif
                    break;
                case THELM_CAP:
#ifdef JP
                    description += "布もしくは革でできた帽子だ。 ";
#else
                    description += "A cloth or leather cap. ";
#endif
                    break;
                case THELM_WIZARD_HAT:
#ifdef JP
                    description += "円錐形をした布の帽子だ。 ";
#else
                    description += "A conical cloth hat. ";
#endif
                    break;
                }
                break;

            case ARM_GLOVES:
#ifdef JP
                description += "一対の小手だ。 ";
#else
                description += "A pair of gloves. ";
#endif
                break;
            case ARM_BOOTS:
                if (item.plus2 == TBOOT_NAGA_BARDING)
#ifdef JP
                    description += "ナーガが尾に着用するための特別な鎧だ。 ";
#else
                    description += "A special armour made for Nagas, "
                        "to wear over their tails. ";
#endif
                else if (item.plus2 == TBOOT_CENTAUR_BARDING)
#ifdef JP
                    description += "セントールのために造られた鎧だ。 $"
                                   "下半身の馬の部分に装着する。 ";
#else
                    description += "An armour made for centaurs, "
                        "to wear over their equine half. ";
#endif
                else
#ifdef JP
                    description += "一足の頑丈なブーツだ。 ";
#else
                    description += "A pair of sturdy boots. ";
#endif
                break;
            case ARM_BUCKLER:
#ifdef JP
                description += "小さな盾だ。 ";
#else
                description += "A small shield. ";
#endif
                break;
            case ARM_LARGE_SHIELD:
#ifdef JP
                description += "この盾は普通の盾と同じような形状だがより大きい。 ";
#else
                description += "Like a normal shield, only larger. ";
#endif
                if (you.species == SP_TROLL || you.species == SP_OGRE
                    || you.species == SP_OGRE_MAGE
                    || player_genus(GENPC_DRACONIAN))
                {
#ifdef JP
                    description += "これはあなたが使うのにちょうど良さそうだ。 ";
#else
                    description += "It looks like it would fit you well. ";
#endif
                }
                else
                {
#ifdef JP
                    description += "これはあまりにも重いので、装備すると攻撃速度が犠牲になる。 ";
#else
                    description += "It is very cumbersome to wear, and "
                        "slows the rate at which you may attack. ";
#endif
                }
                break;
            case ARM_DRAGON_HIDE:
#ifdef JP
                description += "ドラゴンの鱗が生えた皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The scaly skin of a dragon. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_TROLL_HIDE:
#ifdef JP
                description += "トロルの硬くて節くれ立った皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The stiff and knobbly hide of a troll. "
                    "I suppose you could wear it "
                    "if you really wanted to. ";
#endif
                break;
            case ARM_CRYSTAL_PLATE_MAIL:
#ifdef JP
                description += "信じ難いほどの重さだが、極めて防御力の高い結晶質の鎧だ。 $"
                               "これは腐蝕に対してある程度の抵抗力がある。 ";
#else
                description += "An incredibly heavy but extremely effective "
                    "suit of crystalline armour. "
                    "It is somewhat resistant to corrosion. ";
#endif
                break;
            case ARM_DRAGON_ARMOUR:
#ifdef JP
                description += "火を吐くドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "これは火に対する強力な保護効果をもたらすが、 $"
                               "着用者は冷気の影響によって傷つきやすくなってしまう。 ";
#else
                description += "A magical armour, made from the scales of "
                    "a fire-breathing dragon. It provides "
                    "great protection from the effects of fire, "
                    "but renders its wearer more susceptible to "
                    "the effects of cold. ";
#endif
                break;
            case ARM_TROLL_LEATHER_ARMOUR:
#ifdef JP
                description += "一般的なトロルの、硬くて節くれ立った皮で造られた魔法の鎧だ。 $"
                               "それは(着用者がもとよりトロルでないなら)魔法的な再生能力をもたらす。 ";
#else
                description += "A magical armour, made from the stiff and "
                    "knobbly skin of a common troll. It magically regenerates "
                    "its wearer's flesh at a fairly slow rate "
                    "(unless already a troll). ";
#endif
                break;
            case ARM_ICE_DRAGON_HIDE:
#ifdef JP
                description += "ドラゴンの鱗が生えた皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The scaly skin of a dragon. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_ICE_DRAGON_ARMOUR:
#ifdef JP
                description += "冷気を吐くドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "これは冷気に対する強力な保護効果をもたらすが、 $"
                               "着用者は火の影響によって傷つきやすくなってしまう。 ";
#else
                description += "A magical armour, made from the scales of "
                    "a cold-breathing dragon. It provides "
                    "great protection from the effects of cold, "
                    "but renders its wearer more susceptible to "
                    "the effects of fire and heat. ";
#endif
                break;
            case ARM_STEAM_DRAGON_HIDE:
#ifdef JP
                description += "蒸気ドラゴンの柔らかくてしなやかな、鱗が生えた皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The soft and supple scaley skin of "
                    "a steam dragon. I suppose you could "
                    "wear it if you really wanted to. ";
#endif
                break;
            case ARM_STEAM_DRAGON_ARMOUR:
#ifdef JP
                description += "蒸気を吐くドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "これは他の大型ドラゴンの鎧のように魔法的な保護は $"
                               "もたらさないが、極めて軽量で布のようにしなやかだ。 ";
#else
                description += "A magical armour, made from the scales of "
                    "a steam-breathing dragon. Although unlike "
                    "the armour made from the scales of some "
                    "larger dragons it does not provide its wearer "
                    "with much in the way of special magical "
                    "protection, it is extremely light and "
                    "as supple as cloth. ";
#endif
                break;          /* Protects from steam */
            case ARM_MOTTLED_DRAGON_HIDE:
#ifdef JP
                description += "斑紋ドラゴンの奇妙な模様の鱗が生えた皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The weirdly-patterned scaley skin of "
                    "a mottled dragon. I suppose you could "
                    "wear it if you really wanted to. ";
#endif
                break;
            case ARM_MOTTLED_DRAGON_ARMOUR:
#ifdef JP
                description += "斑紋ドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "これは他の大型ドラゴンの鎧のように魔法的な保護はもたらさないが、 $"
                               "レザーアーマーのように軽くて動きを妨げない。 ";
#else
                description += "A magical armour made from the scales of a "
                    "mottled dragon. Although unlike the armour "
                    "made from the scales of some larger dragons "
                    "it does not provide its wearer with much in "
                    "the way of special magical protection, it is "
                    "as light and relatively uncumbersome as "
                    "leather armour. ";
#endif
                break;          /* Protects from napalm */
            case ARM_STORM_DRAGON_HIDE:
#ifdef JP
                description += "ストームドラゴンの極めて硬い鱗が生えた皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The hide of a storm dragon, covered in "
                    "extremely hard blue scales. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_STORM_DRAGON_ARMOUR:
#ifdef JP
                description += "稲妻を吐くドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "この鎧はドラゴンの鎧の中では最も重い部類に入るが、 $"
                               "着用者に放電に対する強力な耐性を授ける。 ";
#else
                description += "A magical armour made from the scales of "
                    "a lightning-breathing dragon. It is heavier "
                    "than most dragon scale armours, but gives "
                    "its wearer great resistance to "
                    "electrical discharges. ";
#endif
                break;
            case ARM_GOLD_DRAGON_HIDE:
#ifdef JP
                description += "ゴールドドラゴンの、輝く金色の鱗に包まれた $"
                               "極めて丈夫で重量のある皮だ。 $"
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The extremely tough and heavy skin of a "
                    "golden dragon, covered in shimmering golden "
                    "scales. I suppose you could wear it if "
                    "you really wanted to. ";
#endif
                break;
            case ARM_GOLD_DRAGON_ARMOUR:
#ifdef JP
                description += "ゴールドドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "これは極めて重く扱いが困難だが、 $"
                               "着用者に火と冷気と毒に対する耐性を授ける。 ";
#else
                description += "A magical armour made from the golden scales "
                    "of a golden dragon. It is extremely heavy and "
                    "cumbersome, but confers resistances to fire, "
                    "cold, and poison on its wearer. ";
#endif
                break;
            case ARM_ANIMAL_SKIN:
#ifdef JP
                description += "動物の皮だ。 ";
#else
                description += "The skins of several animals. ";
#endif
                break;
            case ARM_SWAMP_DRAGON_HIDE:
#ifdef JP
                description += "ぬるぬる";
                if (you.species != SP_MUMMY)
                    description += "していて嫌な臭いの";
                description += "する沼ドラゴンの皮だ。 "
                               "着ようと思えば着られないこともない。 ";
#else
                description += "The slimy";
                if (you.species != SP_MUMMY)
                    description += ", smelly";
                description += " skin of a swamp-dwelling dragon. I suppose "
                    "you could wear it if you really wanted to. ";
#endif
                break;
            case ARM_SWAMP_DRAGON_ARMOUR:
#ifdef JP
                description += "沼ドラゴンの鱗で造られた魔法の鎧だ。 $"
                               "これは着用者に毒への耐性を授ける。 ";
#else
                description += "A magical armour made from the scales of "
                    "a swamp dragon. It confers resistance to "
                    "poison on its wearer. ";
#endif
                break;
            default:
#ifdef JP
                DEBUGSTR("知られざる鎧");
#else
                DEBUGSTR("Unknown armour");
#endif
            }

            description += "$";
        }
    }

    if (verbose == 1
            && item.sub_type != ARM_SHIELD
            && item.sub_type != ARM_BUCKLER
            && item.sub_type != ARM_LARGE_SHIELD)
    {
#ifdef JP
        description += "$アーマーレート: ";
#else
        description += "$Armour rating: ";
#endif

        if (item.sub_type == ARM_HELMET
            && (get_helmet_type( item ) == THELM_CAP
                || get_helmet_type( item ) == THELM_WIZARD_HAT))
        {
            // caps and wizard hats don't have a base AC
            append_value(description, 0, false);
        }
        else if (item.sub_type == ARM_BOOTS && item.plus2 != TBOOT_BOOTS)
        {
            // Barding has AC value 4.
            append_value(description, 4, false);
        }
        else
        {
            append_value(description, property( item, PARM_AC ), false);
        }

#ifdef JP
        description += "$回避力修正値: ";
#else
        description += "$Evasion modifier: ";
#endif
        append_value(description, property( item, PARM_EVASION ), true);
        description += "$";
    }

    int ego = get_armour_ego_type( item );
    if (ego != SPARM_NORMAL
        && item_ident( item, ISFLAG_KNOW_TYPE )
        && verbose == 1)
    {
        description += "$";

        switch (ego)
        {
        case SPARM_RUNNING:
#ifdef JP
            description += "これは着用者に非常な速さで走ることを可能とさせる。 ";
#else
            description += "It allows its wearer to run at a great speed. ";
#endif
            break;
        case SPARM_FIRE_RESISTANCE:
#ifdef JP
            description += "これは着用者を熱と火から保護する。 ";
#else
            description += "It protects its wearer from heat and fire. ";
#endif
            break;
        case SPARM_COLD_RESISTANCE:
#ifdef JP
            description += "これは着用者を冷気から保護する。 ";
#else
            description += "It protects its wearer from cold. ";
#endif
            break;
        case SPARM_POISON_RESISTANCE:
#ifdef JP
            description += "これは着用者を毒から保護する。 ";
#else
            description += "It protects its wearer from poison. ";
#endif
            break;
        case SPARM_SEE_INVISIBLE:
#ifdef JP
            description += "これは着用者が見えざる者を見ることを可能とさせる。 ";
#else
            description += "It allows its wearer to see invisible things. ";
#endif
            break;
        case SPARM_DARKNESS:
#ifdef JP
            description += "これは発動することで着用者を一時的に透明にするが、 $"
                           "透明化の最中には代謝エネルギーの消費が大幅に増加する。 ";
#else
            description += "When activated it hides its wearer from "
                "the sight of others, but also increases "
                "their metabolic rate by a large amount. ";
#endif
            break;
        case SPARM_STRENGTH:
#ifdef JP
            description += "これは着用者の物理的な力を増大させる。(+3 腕力) ";
#else
            description += "It increases the physical power of its wearer (+3 to strength). ";
#endif
            break;
        case SPARM_DEXTERITY:
#ifdef JP
            description += "これは着用者の器用さを増大させる。(+3 器用さ) ";
#else
            description += "It increases the dexterity of its wearer (+3 to dexterity). ";
#endif
            break;
        case SPARM_INTELLIGENCE:
#ifdef JP
            description += "これは着用者の知力を増大させる。(+3 知力) ";
#else
            description += "It makes you more clever (+3 to intelligence). ";
#endif
            break;
        case SPARM_PONDEROUSNESS:
#ifdef JP
            description += "これは非常に扱いづらい。(-2 EV, 移動力低下) ";
#else
            description += "It is very cumbersome (-2 to EV, slows movement). ";
#endif
            break;
        case SPARM_LEVITATION:
#ifdef JP
            description += "これは発動することで着用者を浮遊させることができる。 $"
                           "浮遊は着用者が停止させるまで効果が続く。 ";
#else
            description += "It can be activated to allow its wearer to "
                "float above the ground and remain so indefinitely. ";
#endif
            break;
        case SPARM_MAGIC_RESISTANCE:
#ifdef JP
            description += "これは着用者の魔法への抵抗力を増大させる。 ";
#else
            description += "It increases its wearer's resistance "
                "to enchantments. ";
#endif
            break;
        case SPARM_PROTECTION:
#ifdef JP
            description += "これは着用者を負傷から保護する。(+3 AC) ";
#else
            description += "It protects its wearer from harm (+3 to AC). ";
#endif
            break;
        case SPARM_STEALTH:
#ifdef JP
            description += "これは着用者の隠密性を高める。 ";
#else
            description += "It enhances the stealth of its wearer. ";
#endif
            break;
        case SPARM_RESISTANCE:
#ifdef JP
            description += "これは着用者を熱と冷気の両方の効果から保護する。 ";
#else
            description += "It protects its wearer from the effects "
                "of both cold and heat. ";
#endif
            break;

        // these two are robes only:
        case SPARM_POSITIVE_ENERGY:
#ifdef JP
            description += "これは着用者を負のエネルギーの影響からある程度保護する。 ";
#else
            description += "It partially protects its wearer from "
                "the effects of negative energy. ";
#endif
            break;
        case SPARM_ARCHMAGI:
#ifdef JP
            description += "これは着用者の呪文の力を増大させる。 $"
                           "もはや学び残したことがない者だけが身に着けるべき防具だ。 ";
#else
            description += "It greatly increases the power of its "
                "wearer's magical spells, but is only "
                "intended for those who have " "very little left to learn. ";
#endif
            break;

        case SPARM_PRESERVATION:
#ifdef JP
            description += "これは着用者の所持品を損害や破壊から保護する。 ";
#else
            description += "It protects its wearer's possessions "
                "from damage and destruction. ";
#endif
            break;
        }
        description += "$";
    }

    if (is_random_artefact( item ))
    {
        if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
            randart_descpr( description, item );
        else if (item_ident( item, ISFLAG_KNOW_TYPE ))
#ifdef JP
            description += "$この防具には幾つかの隠された特性があるかもしれない。 ";
#else
            description += "$This armour may have some hidden properties.$";
#endif
    }
    else
    {
        switch (get_equip_race( item ))
        {
        case ISFLAG_ELVEN:
            //jmf: not light
#ifdef JP
            description += "$これは巧みに造られていて、体の動きを妨げない。 ";
#else
            description += "$It is well-crafted and unobstructive";
#endif

            if (item.sub_type == ARM_CLOAK || item.sub_type == ARM_BOOTS)
#ifdef JP
                description += "そして着用者の隠密行動の助けになる。 ";
#else
                description += ", and helps its wearer avoid being noticed";
#endif

#ifdef JP
            //description += ".";
#else
            description += ".";
#endif
            break;

        case ISFLAG_DWARVEN:
#ifdef JP
            description += "$これは巧みに造られていて、非常に耐久力がある。 ";
#else
            description += "$It is well-crafted and very durable.";
#endif
            break;

        case ISFLAG_ORCISH:
        default:
            break;
        }
    }

    if (item_known_cursed( item ))
    {
#ifdef JP
        description += "$これには呪いがかかっている。 ";
#else
        description += "$It has a curse placed upon it.";
#endif
    }

#ifdef JP
    if ( (verbose == 1)
       &&(item.sub_type != ARM_SHIELD)&&(item.sub_type != ARM_CLOAK)&&(item.sub_type != ARM_HELMET)
       &&(item.sub_type != ARM_GLOVES)&&(item.sub_type != ARM_BOOTS)&&(item.sub_type != ARM_BUCKLER)
       &&(item.sub_type != ARM_LARGE_SHIELD) )
    {
        if ( is_light_armour(item) )
            description += "$これは軽量鎧に分類される。 ";
        else
            description += "$これは鎧に分類される。 ";
    }
#else
#endif

    return description;
}

//---------------------------------------------------------------
//
// describe_stick
//
//---------------------------------------------------------------
static std::string describe_stick( const item_def &item )
{
    std::string description;

    description.reserve(64);

    if (get_ident_type( OBJ_WANDS, item.sub_type ) != ID_KNOWN_TYPE)
#ifdef JP
        description += "これは杖だ。魔法の力を秘めているかもしれない。 $";
#else
        description += "A stick. Maybe it's magical. ";
#endif
    else
    {
#ifdef JP
        description += "この杖は魔法の道具だ。 $";
#else
        description += "A magical device which ";
#endif
        switch (item.sub_type)
        {
        case WAND_FLAME:
#ifdef JP
            description += "これは少量の炎を放射する。 $";
#else
            description += "throws little bits of flame. ";
#endif
            break;

        case WAND_FROST:
#ifdef JP
            description += "これは少量の冷気を放射する。 $";
#else
            description += "throws little bits of frost. ";
#endif
            break;

        case WAND_SLOWING:
#ifdef JP
            description += "これは狙った対象に、動きが遅くなる魔法をかける。 $";
#else
            description += "casts enchantments to slow down the actions of "
                "a creature at which it is directed. ";
#endif
            break;

        case WAND_HASTING:
#ifdef JP
            description += "これは狙った対象に、動きを加速する魔法をかける。 $";
#else
            description += "casts enchantments to speed up the actions of "
                "a creature at which it is directed. ";
#endif
            break;

        case WAND_MAGIC_DARTS:
#ifdef JP
            description += "これは小さな破壊エネルギーの矢を放つ。 $";
#else
            description += "throws small bolts of destructive energy. ";
#endif
            break;

        case WAND_HEALING:
#ifdef JP
            description += "これは対象の傷を治療する。 $";
#else
            description += "can heal a creature's wounds. ";
#endif
            break;

        case WAND_PARALYSIS:
#ifdef JP
            description += "これは対象を麻痺させる。 $";
#else
            description += "can render a creature immobile. ";
#endif
            break;

        case WAND_FIRE:
#ifdef JP
            description += "これは強力な炎の矢を放つ。 $";
#else
            description += "throws great bolts of fire. ";
#endif
            break;

        case WAND_COLD:
#ifdef JP
            description += "これは強力な冷気の矢を放つ。 $";
#else
            description += "throws great bolts of cold. ";
#endif
            break;

        case WAND_CONFUSION:
#ifdef JP
            description += "これは対象に混乱と狼狽を引き起こす。 $";
#else
            description += "induces confusion and bewilderment in "
                "a target creature. ";
#endif
            break;

        case WAND_INVISIBILITY:
#ifdef JP
            description += "これは対象を他者の視覚から隠蔽する。 $";
#else
            description += "hides a creature from the view of others. ";
#endif
            break;

        case WAND_DIGGING:
#ifdef JP
            description += "これは強化を受けていない岩を掘削し通路にする。 $";
#else
            description += "drills tunnels through unworked rock. ";
#endif
            break;

        case WAND_FIREBALL:
#ifdef JP
            description += "これは炎の爆風を放つ。 $";
#else
            description += "throws exploding blasts of flame. ";
#endif
            break;

        case WAND_TELEPORTATION:
#ifdef JP
            description += "これは対象に無作為な空間転位を引き起こす。 $";
#else
            description += "causes a creature to be randomly translocated. ";
#endif
            break;

        case WAND_LIGHTNING:
#ifdef JP
            description += "これは強力な稲妻の矢を放つ。 $";
#else
            description += "throws great bolts of lightning. ";
#endif
            break;

        case WAND_POLYMORPH_OTHER:
#ifdef JP
            description += "これは対象を別の形態に変化させる。 $"
                           "このワンドはあなた自身には働かないので、試してはならない。 $";
#else
            description += "causes a creature to be transmogrified into "
                "another form. "
                "It doesn't work on you, so don't even try. ";
#endif
            break;

        case WAND_ENSLAVEMENT:
#ifdef JP
            description += "これは対象に奴隷的な服従をさせる。 $";
#else
            description += "causes slavish obedience in a creature. ";
#endif
            break;

        case WAND_DRAINING:
#ifdef JP
            description += "これは対象に負のエネルギーの矢を放つ。 $"
                           "矢は生命あるモンスターの生命力を衰弱させるが、 $"
                           "アンデッドに対しては効果がない。 $";
#else
            description += "throws a bolt of negative energy which "
                "drains the life essences of living creatures, "
                "but is useless against the undead. ";
#endif
            break;

        case WAND_RANDOM_EFFECTS:
#ifdef JP
            description += "これは変化に富んだ効果を引き起こす。 $";
#else
            description += "can produce a variety of effects. ";
#endif
            break;

        case WAND_DISINTEGRATION:
#ifdef JP
            description += "これは物体の構造、特にモンスターの肉体を崩壊させる $";
#else
            description += "disrupts the physical structure of "
                "an object, especially a creature's body. ";
#endif
            break;

        default:
#ifdef JP
            DEBUGSTR("知られざるワンド");
#else
            DEBUGSTR("Unknown stick");
#endif
        }

        if (item_ident( item, ISFLAG_KNOW_PLUSES ) && item.plus == 0)
#ifdef JP
            description += "残念ながら、これには魔力が残っていない。 ";
#else
            description += "Unfortunately, it has no charges left. ";
#endif
    }

    return description;
}


//---------------------------------------------------------------
//
// describe_food
//
//---------------------------------------------------------------
static std::string describe_food( const item_def &item )
{
    std::string description;

    description.reserve(100);

    switch (item.sub_type)
    {
    // rations
    case FOOD_MEAT_RATION:
    case FOOD_BREAD_RATION:
#ifdef JP
        description += "食べ応えのある";
#else
        description += "A filling ration of ";
#endif
        switch (item.sub_type)
        {
        case FOOD_MEAT_RATION:
#ifdef JP
            description += "乾し肉の保存食だ。 ";
#else
            description += "dried and preserved meats";
#endif
            break;
        case FOOD_BREAD_RATION:
#ifdef JP
            description += "パンの保存食だ。 ";
#else
            description += "breads";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        break;

    // fruits
    case FOOD_PEAR:
    case FOOD_APPLE:
    case FOOD_APRICOT:
    case FOOD_ORANGE:
    case FOOD_BANANA:
    case FOOD_STRAWBERRY:
    case FOOD_RAMBUTAN:
    case FOOD_LEMON:
    case FOOD_GRAPE:
    case FOOD_LYCHEE:
    case FOOD_SULTANA:
#ifdef JP
        //description += "A";
#else
        description += "A";
#endif
        switch (item.sub_type)
        {
        case FOOD_PEAR:
#ifdef JP
            description += "瑞々しくて美味しい";
#else
            description += " delicious juicy";
#endif
            break;
        case FOOD_APPLE:
#ifdef JP
            description += "赤あるいは青の、美味しい";
#else
            description += " delicious red or green";
#endif
            break;
        case FOOD_APRICOT:
#ifdef JP
            description += "美味しいオレンジ色の";
#else
            description += " delicious orange";
#endif
            break;
        case FOOD_ORANGE:
#ifdef JP
            description += "オレンジ色の瑞々しくて美味しい";
#else
            description += " delicious juicy orange";
#endif
            break;
        case FOOD_BANANA:
#ifdef JP
            description += "黄色の美味しい";
#else
            description += " delicious yellow";
#endif
            break;
        case FOOD_STRAWBERRY:
#ifdef JP
            description += "小さいが美味しい、赤い";
#else
            description += " small but delicious red";
#endif
            break;
        case FOOD_RAMBUTAN:
#ifdef JP
            description += "小さいが美味しい、南国の";
#else
            description += " small but delicious tropical";
#endif
            break;
        case FOOD_LEMON:
#ifdef JP
            description += "黄色の";
#else
            description += " yellow";
#endif
            break;
        case FOOD_GRAPE:
#ifdef JP
            description += "小粒の";
#else
            description += " small";
#endif
            break;
        case FOOD_LYCHEE:
#ifdef JP
            description += "南国の";
#else
            description += " tropical";
#endif
            break;
        case FOOD_SULTANA:
#ifdef JP
            description += "乾燥された";
#else
            description += " dried";
#endif
            break;
        }

#ifdef JP
        description += "果物だ。 $";
#else
        description += " fruit";
#endif

        switch (item.sub_type)
        {
        case FOOD_BANANA:
#ifdef JP
            description +=
                "恐らくは不平等な貿易協定の元で、 $"
                "道徳観念のない多国籍企業によって $"
                "栽培と輸入をされた。 ";
#else
            description += ", probably grown and imported by "
                "some amoral multinational as the "
                "result of a corrupt trade deal";
#endif
            break;
        case FOOD_RAMBUTAN:
#ifdef JP
            description +=
                "これがどうやってダンジョンに持ちこまれたかは、 $"
                "誰にとっても謎だ。 ";
#else
            description += ". How it got into this dungeon "
                "is anyone's guess";
#endif
            break;
        case FOOD_SULTANA:
#ifdef JP
            description += "おそらくはブドウの一種だと思われる。 ";
#else
            description += " of some sort, possibly a grape";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        break;

    // vegetables
    case FOOD_CHOKO:
    case FOOD_SNOZZCUMBER:
#ifdef JP
        //description += "A";
#else
        description += "A";
#endif
        switch (item.sub_type)
        {
        case FOOD_CHOKO:
#ifdef JP
            description += "ほとんど味がない緑色の野菜だ。 $";
#else
            description += "n almost tasteless green";
#endif
            break;
        case FOOD_SNOZZCUMBER:
#ifdef JP
            description += "吐き気を催す味の、キュウリに似た形の野菜だ。 $";
            description += "オ・ヤサシ巨人が食べ物としている。 $";
#else
            description += " repulsive cucumber-shaped";
#endif
            break;
        }
#ifdef JP
        description += "";
#else
        description += " vegetable";
#endif
        switch (item.sub_type)
        {
        case FOOD_CHOKO:
#ifdef JP
            description += "これは蔓に実がなる。 ";
#else
            description += ", which grows on a vine";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        break;

    // lumps, slices, chunks, and strips
    case FOOD_HONEYCOMB:
    case FOOD_ROYAL_JELLY:
    case FOOD_PIZZA:
    case FOOD_CHEESE:
    case FOOD_BEEF_JERKY:
    case FOOD_SAUSAGE:
    case FOOD_CHUNK:
#ifdef JP
        //description += "A";
#else
        description += "A";
#endif
        switch (item.sub_type)
        {
        case FOOD_SAUSAGE:
#ifdef JP
            description += "";
#else
            description += "n elongated";
#endif
            break;
        }
        switch (item.sub_type)
        {
        //case FOOD_HONEYCOMB:
        //case FOOD_ROYAL_JELLY:
        case FOOD_CHEESE:
        //case FOOD_SAUSAGE:
#ifdef JP
            description += "一塊の";
#else
            description += " lump";
#endif
            break;
        case FOOD_PIZZA:
#ifdef JP
            description += "一切れの";
#else
            description += " slice";
#endif
            break;
        case FOOD_BEEF_JERKY:
#ifdef JP
            description += "細長く切られた";
#else
            description += " strip";
#endif
            break;
        case FOOD_CHUNK:
#ifdef JP
            description += "";
#else
            description += " piece";
#endif
        }
#ifdef JP
        //description += " of ";
#else
        description += " of ";
#endif
        switch (item.sub_type)
        {
        case FOOD_SAUSAGE:
#ifdef JP
            description +=
                "軟骨、内臓、穀物などを腸に詰めた食べ物だ。 ";
#else
            description += "low-grade gristle, entrails and "
                "cereal products encased in an intestine";
#endif
            break;
        case FOOD_HONEYCOMB:
#ifdef JP
            description += "巨大なミツバチによって作られた、美味しい蜂の巣だ。 ";
#else
            description += "the delicious honeycomb made by giant bees";
#endif
            break;
        case FOOD_ROYAL_JELLY:
#ifdef JP
            description += "巨大なミツバチによって、その女王を養うために作られた $"
                           "神秘的な物質だ。 ";
#else
            description += "the magical substance produced by giant bees "
                "to be fed to their queens";
#endif
            break;
        case FOOD_PIZZA:
#ifdef JP
            description += "ピザだ。 ";
#else
            description += "pizza";
#endif
            break;
        case FOOD_CHEESE:
#ifdef JP
            description += "チーズだ。 ";
#else
            description += "cheese";
#endif
            break;
        case FOOD_BEEF_JERKY:
#ifdef JP
            description += "保存加工済みの牛肉だ。 ";
#else
            description += "preserved dead cow or bull";
#endif
            break;
        case FOOD_CHUNK:
#ifdef JP
            description += "ダンジョンで手に入れた肉だ。 ";
#else
            description += "dungeon meat";
#endif
            break;
        }
#ifdef JP
        //description += ". ";
#else
        description += ". ";
#endif
        switch (item.sub_type)
        {
        case FOOD_SAUSAGE:
#ifdef JP
            description += "$これはうまそうだ！ ";
#else
            description += "Yum! ";
#endif
            break;
        case FOOD_PIZZA:
#ifdef JP
            description += "$そんな食べ物聞いたことがないなどとは言ってはならない！ ";
#else
            description += "Don't tell me you don't know what that is! ";
#endif
            break;
        case FOOD_CHUNK:
            if (you.species != SP_GHOUL)
#ifdef JP
                description += "$この肉はどちらかというと不味そうだ。 ";
#else
                description += "It looks rather unpleasant. ";
#endif

            if (item.special < 100)
            {
                if (you.species == SP_GHOUL)
#ifdef JP
                    description += "$この肉は素晴らしく熟成された様子だ。 ";
#else
                    description += "It looks nice and ripe. ";
#endif
                else if (you.species != SP_MUMMY)
                {
#ifdef JP
                    description +=
                        "$この肉は腐敗が進行している。 $"
                        "これを食べるのは愚かというものだろう。 ";
#else
                    description += "In fact, it is "
                        "rotting away before your eyes. "
                        "Eating it would probably be unwise. ";
#endif
                }
            }
            break;
        }
        break;

    default:
#ifdef JP
        DEBUGSTR("知られざる食料");
#else
        DEBUGSTR("Unknown food");
#endif
    }

    description += "$";

    return (description);
}

//---------------------------------------------------------------
//
// describe_potion
//
//---------------------------------------------------------------
static std::string describe_potion( const item_def &item )
{
    std::string description;

    description.reserve(64);

    if (get_ident_type( OBJ_POTIONS, item.sub_type ) != ID_KNOWN_TYPE)
#ifdef JP
        description += "液体の入った小さな瓶だ。 ";
#else
        description += "A small bottle of liquid.";
#endif
    else
    {
#ifdef JP
        description += "これは";
#else
        description += "A";
#endif

        switch (item.sub_type)
        {
        case POT_HEALING:
#ifdef JP
            description += "祝福された";
#else
            description += " blessed";
#endif
            break;
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "魔法の治癒の";
#else
            description += " magical healing";
#endif
            break;
        case POT_SPEED:
#ifdef JP
            description += "魔力の込められた";
#else
            description += "n enchanted";
#endif
            break;
        case POT_MIGHT:
#ifdef JP
            description += "魔法の";
#else
            description += " magic";
#endif
            break;
        case POT_POISON:
#ifdef JP
            description += "危険な毒物の";
#else
            description += " nasty poisonous";
#endif
            break;
        case POT_PORRIDGE:
#ifdef JP
            description += "満腹をもたらす";
#else
            description += " filling";
#endif
            break;
        case POT_DEGENERATION:
#ifdef JP
            description += "有害な";
#else
            description += " noxious";
#endif
            break;
        case POT_DECAY:
#ifdef JP
            description += "衰弱と腐敗の呪いがかかった";
#else
            description += " vile and putrid cursed";
#endif
            break;
        case POT_WATER:
#ifdef JP
            description += "かけがえのない";
#else
            description += " unique";
#endif
            break;
        case POT_EXPERIENCE:
#ifdef JP
            description += "実に素晴らしく非常に貴重な";
#else
            description += " truly wonderful and very rare";
#endif
            break;
        case POT_MAGIC:
#ifdef JP
            description += "とても有用な";
#else
            description += " valuable";
#endif
            break;
        case POT_STRONG_POISON:
#ifdef JP
            description += "恐ろしい猛毒の";
#else
            description += " terribly venomous";
#endif
            break;
        }

        description += "";

        switch (item.sub_type)
        {
        case POT_MIGHT:
        case POT_GAIN_STRENGTH:
        case POT_GAIN_DEXTERITY:
        case POT_GAIN_INTELLIGENCE:
        case POT_LEVITATION:
        case POT_SLOWING:
        case POT_PARALYSIS:
        case POT_CONFUSION:
        case POT_INVISIBILITY:
        case POT_PORRIDGE:
        case POT_MAGIC:
        case POT_RESTORE_ABILITIES:
        case POT_STRONG_POISON:
        case POT_BERSERK_RAGE:
        case POT_CURE_MUTATION:
        case POT_MUTATION:
#ifdef JP
            description += "水薬だ。 $";
#else
            description += "potion";
#endif
            break;
        case POT_HEALING:
#ifdef JP
            description += "薬液だ。 $";
#else
            description += "fluid";
#endif
            break;
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "霊液だ。 $";
#else
            description += "elixir";
#endif
            break;
        case POT_SPEED:
#ifdef JP
            description += "飲料だ。 $";
#else
            description += "beverage";
#endif
            break;
        case POT_POISON:
        case POT_DECAY:
#ifdef JP
            description += "液体だ。 $";
#else
            description += "liquid";
#endif
            break;
        case POT_DEGENERATION:
#ifdef JP
            description += "調合物だ。 $";
#else
            description += "concoction";
#endif
            break;
        case POT_WATER:
#ifdef JP
            description += "物質だ。 $";
#else
            description += "substance";
#endif
            break;
        case POT_EXPERIENCE:
#ifdef JP
            description += "飲み物だ。 $";
#else
            description += "drink";
#endif
            break;
        }

        switch (item.sub_type)
        {
        case POT_HEALING:
        case POT_HEAL_WOUNDS:
        case POT_SPEED:
        case POT_MIGHT:
        case POT_LEVITATION:
        case POT_SLOWING:
        case POT_PARALYSIS:
        case POT_CONFUSION:
        case POT_INVISIBILITY:
        case POT_DEGENERATION:
        case POT_DECAY:
        case POT_MAGIC:
        case POT_RESTORE_ABILITIES:
        case POT_BERSERK_RAGE:
        case POT_CURE_MUTATION:
        case POT_MUTATION:
#ifdef JP
            description += "これは";
#else
            description += " which ";
#endif
            break;
        case POT_GAIN_STRENGTH:
        case POT_GAIN_DEXTERITY:
        case POT_GAIN_INTELLIGENCE:
        case POT_PORRIDGE:
#ifdef JP
            description += "";
#else
            description += " of ";
#endif
            break;
        }

        switch (item.sub_type)
        {
        case POT_HEALING:
#ifdef JP
            description += "傷を幾らか癒し、精神を浄化し、更には病気を治す。 ";
#else
            description += "heals some wounds, clears the mind, "
                "and cures diseases";
#endif
            break;
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "ほぼ即座に傷口を塞いで癒してしまう。 ";
#else
            description += "causes wounds to close and heal "
                "almost instantly";
#endif
            break;
        case POT_SPEED:
#ifdef JP
            description += "飲んだ者の動作を加速する。 ";
#else
            description += "speeds the actions of anyone who drinks it";
#endif
            break;
        case POT_MIGHT:
#ifdef JP
            description += "飲んだ者の腕力と物理的な力を著しく増加させる。 ";
#else
            description += "greatly increases the strength and "
                "physical power of one who drinks it";
#endif
            break;
        case POT_GAIN_STRENGTH:
        case POT_GAIN_DEXTERITY:
        case POT_GAIN_INTELLIGENCE:
#ifdef JP
            description += "有益な突然変異を引き起こす。 ";
#else
            description += "beneficial mutation";
#endif
            break;
        case POT_LEVITATION:
#ifdef JP
            description += "飲んだ者に大きな浮力を与える。 ";
#else
            description += "confers great buoyancy on one who consumes it";
#endif
            break;
        case POT_SLOWING:
#ifdef JP
            description += "あなたの動作を遅くする。 ";
#else
            description += "slows your actions";
#endif
            break;
        case POT_PARALYSIS:
#ifdef JP
            description += "あなたの全身を麻痺させる。 ";
#else
            description += "eliminates your control over your own body";
#endif
            break;
        case POT_CONFUSION:
#ifdef JP
            description += "あなたの認知能力を混乱させ、 "
                           "あなた自身の行動をまともに制御できなくしてしまう。 ";
#else
            description += "confuses your perceptions and reduces "
                "your control over your own actions";
#endif
            break;
        case POT_INVISIBILITY:
#ifdef JP
            description += "他の者の視覚からあなたを隠蔽する。 ";
#else
            description += "hides you from the sight of others";
#endif
            break;
        case POT_PORRIDGE:
#ifdef JP
            description += "穀物繊維をたくさん含んだ粥だ。 ";
#else
            description += "sludge, high in cereal fibre";
#endif
            break;
        case POT_DEGENERATION:
#ifdef JP
            description += "あなたの肉体と頭脳と反射神経に "
                           "恐ろしい障害をもたらす。 ";
#else
            description += "can do terrible things to your "
                "body, brain and reflexes";
#endif
            break;
        case POT_DECAY:
#ifdef JP
            description += "あなたの肉体に急速な腐敗を引き起こす。 ";
#else
            description += "causes your flesh to decay "
                "before your very eyes";
#endif
            break;
        case POT_WATER:
#ifdef JP
            description += "大抵の生命体にとって必要不可欠である。 ";
#else
            description += ", vital for the existence of most life";
#endif
            break;
        case POT_MAGIC:
#ifdef JP
            description += "飲んだ者に魔法のエネルギーを注ぎ込む。 ";
#else
            description += "grants a person with an "
                "infusion of magical energy";
#endif
            break;
        case POT_RESTORE_ABILITIES:
#ifdef JP
            description += "飲んだ者の能力を回復させる。 ";
#else
            description += "restores the abilities of one who drinks it";
#endif
            break;
        case POT_BERSERK_RAGE:
#ifdef JP
            description += "飲んだ者に突発性の激怒を発生させる。 ";
#else
            description += "can send one into an incoherent rage";
#endif
            break;
        case POT_CURE_MUTATION:
#ifdef JP
            description += "あなたの突然変異を幾つか、あるいは全て取り除く。 ";
#else
            description += "removes some or all of any mutations "
                "which may be afflicting you";
#endif
            break;
        case POT_MUTATION:
#ifdef JP
            description += "あなたに非常に奇妙な作用を及ぼす。 ";
#else
            description += "does very strange things to you";
#endif
            break;
        }

#ifdef JP
        description += " ";
#else
        description += ". ";
#endif

        switch (item.sub_type)
        {
        case POT_HEALING:
        case POT_HEAL_WOUNDS:
#ifdef JP
            description += "もしもこの薬を体調が万全か、ほぼ万全の状態で飲めば、 "
                           "永続的な負傷を";
#else
            description += "If one uses it when they are "
                "at or near full health, it can also ";
#endif

            if (item.sub_type == POT_HEALING)
#ifdef JP
                description += "少しだけ";
            description += "治癒する。 ";
#else
                description += "slightly ";
            description += "repair permanent injuries. ";
#endif
            break;
        }

        //default:
        //    DEBUGSTR("Unknown potion");          // I had no idea where to put this back 16jan2000 {dlb}
    }

    description += "$";

    return (description);
}


//---------------------------------------------------------------
//
// describe_scroll
//
//---------------------------------------------------------------
static std::string describe_scroll( const item_def &item )
{
    std::string description;

    description.reserve(64);

    if (get_ident_type( OBJ_SCROLLS, item.sub_type ) != ID_KNOWN_TYPE)
#ifdef JP
        description += "魔法の文章が書き込まれた紙の巻物だ。 ";
#else
        description += "A scroll of paper covered in magical writing.";
#endif
    else
    {
        switch (item.sub_type)
        {
        case SCR_IDENTIFY:
#ifdef JP
            description += "これはいかなる品物でも鑑定することを可能とする $"
                           "有用な魔法の巻き物だ。 ";
#else
            description += "This useful magic scroll allows you to "
                "determine the properties of any object. ";
#endif
            break;

        case SCR_TELEPORTATION:
#ifdef JP
            description += "この巻物を読み上げると、あなたは無作為な地点に転送される。 ";
#else
            description += "Reading the words on this scroll "
                "translocates you to a random position. ";
#endif
            break;

        case SCR_FEAR:
#ifdef JP
            description += "この巻き物は読み上げた者の視界内にいる相手全てに $"
                           "恐怖を抱かせる。 ";
#else
            description += "This scroll causes great fear in those "
                "who see the one who reads it. ";
#endif
            break;

        case SCR_NOISE:
#ifdef JP
            description += "この悪戯の巻き物は、しばしば邪悪な実習生によって $"
                           "魔法使いの荷物の中に忍び込まされて、うるさい騒音を引き起す。 $"
                           "その他にはこれといって有用な使い途などない。 ";
#else
            description += "This prank scroll, often slipped into a wizard's "
                "backpack by a devious apprentice, causes a loud noise. "
                "It is not otherwise noted for its usefulness. ";
#endif
            break;

        case SCR_REMOVE_CURSE:
#ifdef JP
            description += "この巻物を読み上げると、あなたの装備中のアイテムから $"
                           "呪いが取り除かれる。 ";
#else
            description += "Reading this scroll removes curses from "
                "the items you are using. ";
#endif
            break;

        case SCR_DETECT_CURSE:
#ifdef JP
            description += "この巻物を読み上げると、あなたの持ち物の中から $"
                           "呪われたアイテムの存在を識別できる。 ";
#else
            description += "This scroll allows you to detect the presence "
                "of cursed items among your possessions. ";
#endif
            break;

        case SCR_SUMMONING:
#ifdef JP
            description += "この巻物を読み上げると、アビスへの経路が開いて $"
                           "恐るべき獣を限られた時間だけこの世界に呼び寄せる。 ";
#else
            description += "This scroll opens a conduit to the Abyss "
                "and draws a terrible beast to this world "
                "for a limited time. ";
#endif
            break;

        case SCR_ENCHANT_WEAPON_I:
#ifdef JP
            description += "この巻き物は武器に魔法をかけて、戦闘でより正確であるようにする。 $"
                           "この強化はすでに高い性能を持つ武器に対しては失敗する場合がある。 ";
#else
            description += "This scroll places an enchantment on a weapon, "
                "making it more accurate in combat. It may fail "
                "to affect weapons already heavily enchanted. ";
#endif
            break;

        case SCR_ENCHANT_ARMOUR:
#ifdef JP
            description += "この巻き物は防具に魔法をかけて強化する。 ";
#else
            description += "This scroll places an enchantment "
                "on a piece of armour. ";
#endif
            break;

        case SCR_TORMENT:
#ifdef JP
            description += "この巻き物は凄まじい苦痛をもたらす地獄の力を呼び出して $"
                           "辺りにいる生物を責め苛む。 $"
                           "もちろん、あなたも含めて！ ";
#else
            description += "This scroll calls on the powers of Hell to "
                "inflict great pain on any nearby creature - "
                "including you! ";
#endif
            break;

        case SCR_RANDOM_USELESSNESS:
#ifdef JP
            description += "この巻き物は何らかの機能を持っているかのように振る舞い、 $"
                           "その完全な無用性をあなたから隠蔽する。 ";
#else
            description += "It is easy to be blinded to the essential "
                "uselessness of this scroll by the sense of achievement "
                "you get from getting it to work at all.";
#endif
                // -- The Hitchhiker's Guide to the Galaxy (paraphrase)
            break;

        case SCR_CURSE_WEAPON:
#ifdef JP
            description += "この巻き物は武器に呪いをかける。 ";
#else
            description += "This scroll places a curse on a weapon. ";
#endif
            break;

        case SCR_CURSE_ARMOUR:
#ifdef JP
            description += "この巻き物は防具に呪いをかける。 ";
#else
            description += "This scroll places a curse "
                "on a piece of armour. ";
#endif
            break;

        case SCR_IMMOLATION:
#ifdef JP
            description += "巻物の裏に何かが小さく書かれている。 $ $"
                           "『警告:内部に圧力がかかっているので取り扱い注意。 $"
                           "　可燃物のそばでは使用しないでください』 ";
#else
            description += "Small writing on the back of the scroll reads: "
                "\"Warning: contents under pressure.  Do not use near"
                " flammable objects.\"";
#endif
            break;

        case SCR_BLINKING:
#ifdef JP
            description += "この巻き物は読んだ者に、正確に制御可能な短距離の $"
                           "テレポートをもたらす。 $"
                           "ただし制御されたテレポートは使用者を魔法のエネルギーで $"
                           "汚染することがあるので注意しなくてはならない。 ";
#else
            description += "This scroll allows its reader to teleport "
                "a short distance, with precise control.  Be wary that "
                "controlled teleports will cause the subject to "
                "become contaminated with magical energy. ";
#endif
            break;

        case SCR_PAPER:
#ifdef JP
            description += "ラベルを別としたら、この巻き物には何も書かれていない。 ";
#else
            description += "Apart from a label, this scroll is blank. ";
#endif
            break;

        case SCR_MAGIC_MAPPING:
#ifdef JP
            description += "この巻き物は読んだ者の周辺の地形を明らかにする。 ";
#else
            description += "This scroll reveals the nearby surroundings "
                "of one who reads it. ";
#endif
            break;

        case SCR_FORGETFULNESS:
#ifdef JP
            description += "この巻き物は腹立たしい見当室喪失を引き起こす。 ";
#else
            description += "This scroll induces "
                "an irritating disorientation. ";
#endif
            break;

        case SCR_ACQUIREMENT:
#ifdef JP
            description += "この素晴らしい巻き物は読んだ者の目の前に価値あるアイテムを $"
                           "生成する。 $"
                           "これはとりわけ魔法のみに特化した魔術師に重要視される。 $"
                           "なぜなら、この巻物を使って強力な呪文書を手に入れることも $"
                           "可能だからである。 ";
#else
            description += "This wonderful scroll causes the "
                "creation of a valuable item to "
                "appear before the reader. "
                "It is especially treasured by specialist "
                "magicians, as they can use it to obtain "
                "the powerful spells of their specialty. ";
#endif
            break;

        case SCR_ENCHANT_WEAPON_II:
#ifdef JP
            description += "この巻き物は武器に魔法をかけて、 $"
                           "戦闘でより高いダメージをもたらすようにする。 $"
                           "この強化はすでに高い性能を持つ武器に対しては失敗する場合がある。 ";
#else
            description += "This scroll places an enchantment on a weapon, "
                "making it inflict greater damage in combat. "
                "It may fail to affect weapons already "
                "heavily enchanted. ";
#endif
            break;

        case SCR_VORPALISE_WEAPON:
#ifdef JP
            description += "この巻き物は威力を飛躍的に強化する魔力を武器に与える。 $"
                           "武器がすでに何らかの魔法的な属性を帯びている場合には、 $"
                           "(つまりエゴつきの武器やアーティファクトには) $"
                           "この巻物を使うことはお奨めできない。 ";
#else
            description += "This scroll enchants a weapon so as to make "
                "it far more effective at inflicting harm on "
                "its wielder's enemies. Using it on a weapon "
                "already affected by some kind of special "
                "enchantment (other than that produced by a "
                "normal scroll of enchant weapon) is not advised. ";
#endif
            break;

        case SCR_RECHARGING:
#ifdef JP
            description += "この巻き物は読んだ者が手に持っているワンドの使用回数を回復させる。 ";
#else
            description += "This scroll restores the charges of "
                "any magical wand wielded by its reader. ";
#endif
            break;

        case SCR_ENCHANT_WEAPON_III:
#ifdef JP
            description += "この巻き物は武器に魔法をかけて、 $"
                           "戦闘でより高い性能を発揮するようにする。 $"
                           "この強化はすでに高い性能を持つ武器に対しては失敗する場合がある。 ";
#else
            description += "This scroll enchants a weapon to be "
                "far more effective in combat. Although "
                "it can be used in the creation of especially "
                "enchanted weapons, it may fail to affect those "
                "already heavily enchanted. ";
#endif
            break;

        default:
#ifdef JP
            DEBUGSTR("知られざる巻物");
#else
            DEBUGSTR("Unknown scroll");
#endif
        }
    }

    description += "$";

    return (description);
}


//---------------------------------------------------------------
//
// describe_jewellery
//
//---------------------------------------------------------------
static std::string describe_jewellery( const item_def &item, char verbose)
{
    std::string description;

    description.reserve(200);

    if (is_unrandom_artefact( item ) && strlen(unrandart_descrip(1, item)) != 0)
    {
        description += "$";
        description += unrandart_descrip(1, item);
        description += "$$";
    }
    else if ((!is_random_artefact( item )
            && get_ident_type( OBJ_JEWELLERY, item.sub_type ) != ID_KNOWN_TYPE)
            || (is_random_artefact( item )
            && item_not_ident( item, ISFLAG_KNOW_TYPE )))
    {
#ifdef JP
        description += "これは装身具だ。 ";
#else
        description += "A piece of jewellery.";
#endif
    }
    else if (verbose == 1 || is_random_artefact( item ))
    {
        switch (item.sub_type)
        {
        case RING_REGENERATION:
#ifdef JP
            description += "この驚くべき指輪は装着者の回復能力を増強するが、 "
                           "同様に代謝エネルギーの消費速度も加速する。 ";
#else
            description += "This wonderful ring greatly increases the "
                "recuperative powers of its wearer, but also "
                "considerably speeds his or her metabolism. ";
#endif
            break;

        case RING_PROTECTION:
            description +=
#ifdef JP
                "この指輪は装着者を負傷から保護するか、あるいは負傷を受けやすくする。 "
                "効果がどちらであるかは指輪の数値に依存している。 ";
#else
                "This ring either protects its wearer from harm or makes "
                "them more vulnerable to injury, to a degree dependent "
                "on its power. ";
#endif
            break;

        case RING_PROTECTION_FROM_FIRE:
            description +=
#ifdef JP
                "この指輪は熱と火からの保護をもたらす。 ";
#else
                "This ring provides protection from heat and fire. ";
#endif
            break;

        case RING_POISON_RESISTANCE:
            description +=
#ifdef JP
                "この指輪は毒からの保護をもたらす。 ";
#else
                "This ring provides protection from the effects of poisons and venom. ";
#endif
            break;

        case RING_PROTECTION_FROM_COLD:
#ifdef JP
            description += "この指輪は冷気からの保護をもたらす。 ";
#else
            description += "This ring provides protection from cold. ";
#endif
            break;

        case RING_STRENGTH:
            description +=
#ifdef JP
                "この指輪は装着者の腕力を増加させるか、あるいは減退させる。 "
                "効果がどちらであるかは指輪の数値に依存している。 ";
#else
                "This ring increases or decreases the physical strength "
                "of its wearer, to a degree dependent on its power. ";
#endif
            break;

        case RING_SLAYING:
            description +=
#ifdef JP
                "この指輪は装着者の白兵戦と射撃に関する能力を増大させる。 ";
#else
                "This ring increases the hand-to-hand and missile combat "
                "skills of its wearer.";
#endif
            break;

        case RING_SEE_INVISIBLE:
            description +=
#ifdef JP
                "この指輪は装着者に、魔法によって視界から隠れているものを "
                "視ることを可能にする。 ";
#else
                "This ring allows its wearer to see those things hidden "
                "from view by magic. ";
#endif
            break;

        case RING_INVISIBILITY:
            description +=
#ifdef JP
                "この指輪は発動することで装着者を一時的に透明にするが、 "
                "透明化の最中には代謝エネルギーの消費が大幅に増加する。 ";
#else
                "This powerful ring can be activated to hide its wearer "
                "from the view of others, but increases the speed of his "
                "or her metabolism greatly while doing so. ";
#endif
            break;

        case RING_HUNGER:
            description +=
#ifdef JP
                "この忌まわしい指輪は装着者に本来よりもかなり速く飢えをもたらす。 ";
#else
                "This accursed ring causes its wearer to hunger "
                "considerably more quickly. ";
#endif
            break;

        case RING_TELEPORTATION:
            description +=
#ifdef JP
                "この指輪は時折、装着者を無作為な場所に転位させる力を及ぼす。 "
                "また、発動することで同様の転位を意図的に引き起こすことができる。 ";
#else
                "This ring occasionally exerts its power to randomly "
                "translocate its wearer to another place, and can be "
                "deliberately activated for the same effect. ";
#endif
            break;

        case RING_EVASION:
            description +=
#ifdef JP
                "この指輪は装着者の回避能力を上昇させるか、あるいは低下させる。 "
                "効果がどちらであるかは指輪の数値に依存している。 ";
#else
                "This ring makes its wearer either more or less capable "
                "of avoiding attacks, depending on its degree "
                "of enchantment. ";
#endif
            break;

        case RING_SUSTAIN_ABILITIES:
            description +=
#ifdef JP
                "この指輪は装着者を腕力、知力、器用さの損失から保護する。 ";
#else
                "This ring protects its wearer from the loss of their "
                "strength, dexterity and intelligence. ";
#endif
            break;

        case RING_SUSTENANCE:
            description +=
#ifdef JP
                "この指輪は装着者にエネルギーを供給するため、 "
                "装着者は食事を必要とする頻度が少なくなる。 ";
#else
                "This ring provides energy to its wearer, so that they "
                "need eat less often. ";
#endif
            break;

        case RING_DEXTERITY:
            description +=
#ifdef JP
                "この指輪は装着者の器用さを増加させるか、あるいは減退させる。 "
                "効果がどちらであるかは指輪の数値に依存している。 ";
#else
                "This ring increases or decreases the dexterity of its "
                "wearer, depending on the degree to which it has been "
                "enchanted. ";
#endif
            break;

        case RING_INTELLIGENCE:
            description +=
#ifdef JP
                "この指輪は装着者の知力を増加させるか、あるいは減退させる。 "
                "効果がどちらであるかは指輪の数値に依存している。 ";
#else
                "This ring increases or decreases the mental ability of "
                "its wearer, depending on the degree to which it has "
                "been enchanted. ";
#endif
            break;

        case RING_WIZARDRY:
            description +=
#ifdef JP
                "この指輪は装着者が魔法の呪文を使う能力を増強する。 ";
#else
                "This ring increases the ability of its wearer to use "
                "magical spells. ";
#endif
            break;

        case RING_MAGICAL_POWER:
            description +=
#ifdef JP
                "この指輪は装着者の魔力の量を増加させる。 ";
#else
                "This ring increases its wearer's reserves of magical "
                "power. ";
#endif
            break;

        case RING_LEVITATION:
            description +=
#ifdef JP
                "この指輪は装着者に浮力の発動を可能とさせる。 ";
#else
                "This ring allows its wearer to hover above the floor. ";
#endif
            break;

        case RING_LIFE_PROTECTION:
            description +=
#ifdef JP
                "この祝福された指輪は負のエネルギーから装着者を保護し、 "
                "アンデッドと死霊術による生命力の衰弱に対してある程度の免疫を授ける。 ";
#else
                "This blessed ring protects the life-force of its wearer "
                "from negative energy, making them partially immune to "
                "the draining effects of undead and necromantic magic. ";
#endif
            break;

        case RING_PROTECTION_FROM_MAGIC:
            description +=
#ifdef JP
                "この指輪は敵対的な魔法に対する装着者の抵抗力を強化する。 ";
#else
                "This ring increases its wearer's resistance to "
                "hostile enchantments. ";
#endif
            break;

        case RING_FIRE:
            description +=
#ifdef JP
                "この指輪は火の力との更なる接触を装着者にもたらす。 "
                "装着者は熱に対する耐性を得ると共に、 "
                "火の魔法をより効果的に扱うことが可能になる。 "
                "ただし冷気の影響によって傷つきやすくなってしまう。 ";
#else
                "This ring brings its wearer more in contact with "
                "the powers of fire. He or she gains resistance to "
                "heat and can use fire magic more effectively, but "
                "becomes more vulnerable to the effects of cold. ";
#endif
            break;

        case RING_ICE:
            description +=
#ifdef JP
                "この指輪は冷気と氷の力との更なる接触を装着者にもたらす。 "
                "装着者は冷気に対する耐性を得ると共に、 "
                "冷気の魔法をより効果的に扱うことが可能になる。 "
                "ただし火の影響によって傷つきやすくなってしまう。 ";
#else
                "This ring brings its wearer more in contact with "
                "the powers of cold and ice. He or she gains resistance "
                "to cold and can use ice magic more effectively, but "
                "becomes more vulnerable to the effects of fire. ";
#endif
            break;

        case RING_TELEPORT_CONTROL:
#ifdef JP
            description += "この指輪は装着者がどのようなテレポートについてでも目的地を "
                           "制御することを可能にする。ただし完全な正確さは望めない。 "
                           "固体物の中にテレポートを試みた場合には通常のランダムテレポートとなる。 "
                           "また、制御されたテレポートは残留した魔法のエネルギーによって "
                           "使用者に悪影響を与えることに用心するべきである。 ";
#else
            description += "This ring allows its wearer to control the "
                "destination of any teleportation, although without "
                "perfect accuracy.  Trying to teleport into a solid "
                "object will result in a random teleportation, at "
                "least in the case of a normal teleportation.  Also "
                "be wary that controlled teleports will contaminate "
                "the subject with residual magical energy.";
#endif
            break;

        case AMU_RAGE:
            description +=
#ifdef JP
                "この護符は装着者がバーサークの激昂状態になることを可能とし、 "
                "またバーサークへの移行を成功させるチャンスを増加させる。 "
                "さらに装着者がバーサークから醒めた際に気絶してしまう確率を減らす。 ";
#else
                "This amulet enables its wearer to attempt to enter "
                "a state of berserk rage, and increases their chance "
                "of successfully doing so.  It also partially protects "
                "the user from passing out when coming out of that rage. ";
#endif
            break;

        case AMU_RESIST_SLOW:
            description +=
#ifdef JP
                "この護符は装着者を魔術的に引き起こされる減速から保護する。 "
                "また装着者を加速する魔術の持続時間を増加させる。 ";
#else
                "This amulet protects its wearer from some magically "
                "induced forms of slowness, and increases the duration "
                "of enchantments which speed his or her actions. ";
#endif
            break;

        case AMU_CLARITY:
            description +=
#ifdef JP
                "この護符は装着者を精神の混乱から保護する。 ";
#else
                "This amulet protects its wearer from some forms of "
                "mental confusion. ";
#endif
            break;

        case AMU_WARDING:
            description +=
#ifdef JP
                "この護符は魔法で召換されたモンスターからの攻撃を防ぐ。 ";
#else
                "This amulet repels some of the attacks of creatures "
                "which have been magically summoned. ";
#endif
            break;

        case AMU_RESIST_CORROSION:
            description +=
#ifdef JP
                "この護符は装着者の防具と武器を酸による腐蝕から保護する。 "
                "ただし保護は絶対確実というわけではない。 ";
#else
                "This amulet protects the armour and weaponry of its "
                "wearer from corrosion caused by acids, although not "
                "infallibly so. ";
#endif
            break;

        case AMU_THE_GOURMAND:
            description +=
#ifdef JP
                "この護符は着用者が腐敗した肉を問題なく食べることを可能とする。 "
                "ただし有毒あるいは呪われた肉はとりもなおさず危険である。 ";
#else
                "This amulet allows its wearer to consume meat in "
                "various states of decay without suffering unduly as "
                "a result. Poisonous or cursed flesh is still not "
                "recommended. ";
#endif
            break;

        case AMU_CONSERVATION:
            description +=
#ifdef JP
                "この護符は着用者の所持品を破壊から保護する。 "
                "ただし保護は絶対確実というわけではない。 ";
#else
                "This amulet protects some of the possessions of "
                "its wearer from outright destruction, but not "
                "infallibly so. ";
#endif
            break;

        case AMU_CONTROLLED_FLIGHT:
            description +=
#ifdef JP
                "この護符の着用者は魔術的な力で宙に浮いている時に "
                "飛行運動をある程度制御することができる。 "
                "これは例えば、着用者が空中浮揚を停止することなしに "
                "階段を降りたり床にあるアイテムを拾ったりすることを可能にする。 ";
#else
                "Should the wearer of this amulet be levitated "
                "by magical means, he or she will be able to exercise "
                "some control over the resulting motion. This allows "
                "the descent of staircases and the retrieval of items "
                "lying on the ground, for example, but does not "
                "deprive the wearer of the benefits of levitation. ";
#endif
            break;

        case AMU_INACCURACY:
            description +=
#ifdef JP
                "この護符は白兵戦での着用者の攻撃を不正確なものにする。 ";
#else
                "This amulet makes its wearer less accurate in hand combat. ";
#endif
            break;

        case AMU_RESIST_MUTATION:
            description +=
#ifdef JP
                "この護符は着用者を突然変異から保護する。 "
                "ただし保護は絶対確実というわけではない。 ";
#else
                "This amulet protects its wearer from mutations, "
                "although not infallibly so. ";
#endif
            break;

        default:
#ifdef JP
            DEBUGSTR("知られざる宝石");
#else
            DEBUGSTR("Unknown jewellery");
#endif
        }

        description += "$";
    }

    if ((verbose == 1 || is_random_artefact( item ))
        && item_ident( item, ISFLAG_KNOW_PLUSES ))
    {
        // Explicit description of ring power (useful for randarts)
        // Note that for randarts we'll print out the pluses even
        // in the case that its zero, just to avoid confusion. -- bwr
        if (item.plus != 0
            || (item.sub_type == RING_SLAYING && item.plus2 != 0)
            || is_random_artefact( item ))
        {
            switch (item.sub_type)
            {
            case RING_PROTECTION:
#ifdef JP
                description += "$これはあなたのACに影響する。(";
#else
                description += "$It affects your AC (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_EVASION:
#ifdef JP
                description += "$これはあなたの回避力に影響する。(";
#else
                description += "$It affects your evasion (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_STRENGTH:
#ifdef JP
                description += "$これはあなたの腕力に影響する。(";
#else
                description += "$It affects your strength (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_INTELLIGENCE:
#ifdef JP
                description += "$これはあなたの知力に影響する。(";
#else
                description += "$It affects your intelligence (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_DEXTERITY:
#ifdef JP
                description += "$これはあなたの器用さに影響する。(";
#else
                description += "$It affects your dexterity (";
#endif
                append_value( description, item.plus, true );
#ifdef JP
                description += ")";
#else
                description += ").";
#endif
                break;

            case RING_SLAYING:
                if (item.plus != 0 || is_random_artefact( item ))
                {
#ifdef JP
                    description += "$これはあなたの命中率に影響する。(";
#else
                    description += "$It affects your accuracy (";
#endif
                    append_value( description, item.plus, true );
#ifdef JP
                    description += ")";
#else
                    description += ").";
#endif
                }

                if (item.plus2 != 0 || is_random_artefact( item ))
                {
#ifdef JP
                    description += "$これはあなたがダメージを与える能力に影響する。(";
#else
                    description += "$It affects your damage-dealing abilities (";
#endif
                    append_value( description, item.plus2, true );
#ifdef JP
                    description += ")";
#else
                    description += ").";
#endif
                }
                break;

            default:
                break;
            }
        }
    }

    // randart properties
    if (is_random_artefact( item ))
    {
        if (item_ident( item, ISFLAG_KNOW_PROPERTIES ))
            randart_descpr( description, item );
        else if (item_ident( item, ISFLAG_KNOW_TYPE ))
        {
            if (item.sub_type >= AMU_RAGE)
#ifdef JP
                description += "$この護符には幾つかの隠された特性があるかもしれない。$";
#else
                description += "$This amulet may have hidden properties.$";
#endif
            else
#ifdef JP
                description += "$この指輪には幾つかの隠された特性があるかもしれない。$";
#else
                description += "$This ring may have hidden properties.$";
#endif
        }
    }

    if (item_known_cursed( item ))
    {
#ifdef JP
        description += "$$これには呪いがかかっている。 ";
#else
        description += "$It has a curse placed upon it.";
#endif
    }

    return (description);
}                               // end describe_jewellery()

//---------------------------------------------------------------
//
// describe_staff
//
//---------------------------------------------------------------
static std::string describe_staff( const item_def &item )
{
    std::string description;

    description.reserve(200);

    if (item_ident( item, ISFLAG_KNOW_TYPE ))
    {
        // NB: the leading space is here {dlb}
#ifdef JP
        description += "この" + std::string( item_is_staff( item ) ? "杖は"
                                                                    : "ロッドは" );
#else
        description += "This " + std::string( item_is_staff( item ) ? "staff "
                                                                    : "rod " );
#endif

        switch (item.sub_type)
        {
        case STAFF_WIZARDRY:
            description +=
#ifdef JP
                "手にしている者の魔法の熟練度を大幅に増やし、 $"
                "呪文の威力を強化する。 ";
#else
                "increases the magical proficiency of its wielder by "
                "a considerable degree, increasing the power of their spells. ";
#endif
            break;

        case STAFF_POWER:
            description +=
#ifdef JP
                "手にしている者に追加の魔力をもたらす。 ";
#else
                "provides a reservoir of magical power to its wielder. ";
#endif
            break;

        case STAFF_FIRE:
            description +=
#ifdef JP
                "手にしている者の炎の魔法の威力を強化し、 $"
                "更には炎の影響から保護する。 $"
                "この杖は攻撃した相手を燃やすこともできる。 ";
#else
                "increases the power of fire spells cast by its wielder, "
                "and protects him or her from the effects of heat and fire. "
                "It can burn those struck by it. ";
#endif
            break;

        case STAFF_COLD:
            description +=
#ifdef JP
                "手にしている者の氷の魔法の威力を強化し、 $"
                "更には氷の影響から保護する。 $"
                "この杖は攻撃した相手を凍らせることができる。 ";
#else
                "increases the power of ice spells cast by its wielder, "
                "and protects him or her from the effects of cold. It can "
                "freeze those struck by it. ";
#endif
            break;

        case STAFF_POISON:
            description +=
#ifdef JP
                "手にしている者の毒の魔法の威力を強化し、 $"
                "更には毒の影響から保護する。 $"
                "この杖は攻撃した相手を毒に冒すことができる。 ";
#else
                "increases the power of poisoning spells cast by its "
                "wielder, and protects him or her from the effects of "
                "poison. It can poison those struck by it. ";
#endif
            break;

        case STAFF_ENERGY:
            description +=
#ifdef JP
                "手にしている者が栄養を消耗せずに魔法の呪文を $"
                "唱えることを可能とする。 ";
#else
                "allows its wielder to cast magical spells without "
                "hungering as a result. ";
#endif
            break;

        case STAFF_DEATH:
            description +=
#ifdef JP
                "手にしている者の死霊術の呪文の威力を強化する。 $"
                "この杖は攻撃した相手に凄まじい苦痛を与えることができる。 ";
#else
                "increases the power of necromantic spells cast by its "
                "wielder. It can cause great pain in those living souls "
                "its wielder strikes. ";
#endif
            break;

        case STAFF_CONJURATION:
            description +=
#ifdef JP
                "手にしている者が唱えた妖術の呪文の威力を強化する。 ";
#else
                "increases the power of conjurations cast by its wielder. ";
#endif
            break;

        case STAFF_ENCHANTMENT:
            description +=
#ifdef JP
                "手にしている者が唱えた呪術の呪文の威力を強化する。 ";
#else
                "increases the power of enchantments cast by its wielder. ";
#endif
            break;

        case STAFF_SUMMONING:
            description +=
#ifdef JP
                "手にしている者が唱えた召換術の呪文の威力を強化する。 ";
#else
                "increases the power of summonings cast by its wielder. ";
#endif
            break;

        case STAFF_SMITING:
            description +=
#ifdef JP
                "手にしている者が遠距離から敵を強打することを可能とする。 $"
                "この能力を使うには最低でもレベル4に達していなければならず、 $"
                "4ポイントの魔力を消費する。 ";
#else
                "allows its wielder to smite foes from afar. The wielder "
                "must be at least level four to safely use this ability, "
                "which costs 4 magic points. ";
#endif
            break;

        case STAFF_STRIKING:
#ifdef JP
            description += "手にしている者が遠距離から敵を打つことを可能とする。 ";
#else
            description += "allows its wielder to strike foes from afar. ";
#endif
            break;

        case STAFF_SPELL_SUMMONING:
#ifdef JP
            description += "召換の呪文群を収めている。 ";
#else
            description += "contains spells of summoning. ";
#endif
            break;

        case STAFF_WARDING:
            description +=
#ifdef JP
                "敵を退けるための呪文群を収めている。 ";
#else
                "contains spells designed to repel one's enemies. ";
#endif
            break;

        case STAFF_DISCOVERY:
            description +=
#ifdef JP
                "周辺の状況を見破るための呪文群を収めている。 ";
#else
                "contains spells which reveal various aspects of "
                "an explorer's surroundings to them. ";
#endif
            break;

        case STAFF_AIR:
            description +=
#ifdef JP
                "手にしている者の大気の呪文の威力を強化する。 $"
                "この杖は攻撃した相手に電撃を与えることができる。 ";
#else
                "increases the power of air spells cast by its wielder. "
                "It can shock those struck by it. ";
#endif
            break;

        case STAFF_EARTH:
            description +=
#ifdef JP
                "手にしている者の大地の呪文の威力を強化する。 $"
                "この杖は攻撃した相手を圧砕することができる。 ";
#else
                "increases the power of earth spells cast by its wielder. "
                "It can crush those struck by it. ";
#endif
            break;

        case STAFF_CHANNELING:
            description +=
#ifdef JP
                "周囲の魔法エネルギーを取りこんで、 $"
                "魔力を回復することを可能にする。 ";
#else
                "allows its caster to channel ambient magical energy for "
                "his or her own purposes. ";
#endif
            break;

        default:
            description +=
#ifdef JP
                "破壊と荒廃の呪文群を収めている。 ";
#else
                "contains spells of mayhem and destruction. ";
#endif
            break;
        }

        if (item_is_rod( item ))
        {
            description +=
#ifdef JP
                "$収められた呪文は栄養を消耗せず、失敗もせず唱えることができる。 $";
#else
                "Casting a spell from it consumes no food, and will not fail.$";
#endif
        }
        else
        {
            description +=
#ifdef JP
                "$$ダメージレート: 7 $命中レート: +6 $基本攻撃速度: 120%%";
#else
                "$$Damage rating: 7 $Accuracy rating: +6 $Attack delay: 120%%";
#endif

#ifdef JP
            description += "$$これは棒杖に分類される。 ";
#else
            description += "$$It falls into the 'staves' category. ";
#endif
        }
    }
    else
    {
#ifdef JP
        description += "魔力が封入された杖だ。 $";
#else
        description += "A stick imbued with magical properties.$";
#endif
    }

    return (description);
}


//---------------------------------------------------------------
//
// describe_misc_item
//
//---------------------------------------------------------------
static std::string describe_misc_item( const item_def &item )
{
    std::string description;

    description.reserve(100);

    if (item_ident( item, ISFLAG_KNOW_TYPE ))
    {
        switch (item.sub_type)
        {
        case MISC_BOTTLED_EFREET:
            description +=
#ifdef JP
                "いずこかの魔術師に捕らえられ、青銅の瓶に封印された力あるイフリートだ。 $"
                "瓶の封印を破ればイフリートは解き放たれ破壊をもたらすだろう。 $"
                "……もしかするとあなた自身に対して。 ";
#else
                "A mighty efreet, captured by some wizard and bound into "
                "a bronze flask. Breaking the flask's seal will release it "
                "to wreak havoc - possibly on you. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_SEEING:
            description +=
#ifdef JP
                "周囲の地形を見ることを可能にする魔法の道具だ。 $"
                "これを的確に扱うにはある程度の魔法的な技量が必要だ。 $"
                "さもなければこれは予測不能な、そして恐らくは有害な結果を招く。 ";
#else
                "A magical device which allows one to see the layout of "
                "their surroundings. It requires a degree of magical "
                "ability to be used reliably, otherwise it can produce "
                "unpredictable and possibly harmful results. ";
#endif
            break;
        case MISC_AIR_ELEMENTAL_FAN:
#ifdef JP
            description +=
                "大気の精霊を召換するための魔法の道具だ。 $"
                "これは幾分信頼性に欠ける品物で、うまく発動するまで $"
                "何度も試みなくてはならない場合が多い。 $"
                "使用には危険を伴うが、使用者が然るべき精霊魔法に $"
                "習熟していればその危険は減少する。 ";
#else
            description += "A magical device for summoning air "
                "elementals. It is rather unreliable, and usually requires "
                "several attempts to function correctly. Using it carries "
                "an element of risk, which is reduced if one is skilled in "
                "the appropriate elemental magic. ";
#endif
            break;
        case MISC_LAMP_OF_FIRE:
#ifdef JP
            description +=
                "炎の精霊を召換するための魔法の道具だ。 $"
                "これは幾分信頼性に欠ける品物で、うまく発動するまで $"
                "何度も試みなくてはならない場合が多い。 $"
                "使用には危険を伴うが、使用者が然るべき精霊魔法に $"
                "習熟していればその危険は減少する。 ";
#else
            description += "A magical device for summoning fire "
                "elementals. It is rather unreliable, and usually "
                "requires several attempts to function correctly. Using "
                "it carries an element of risk, which is reduced if one "
                "is skilled in the appropriate elemental magic.";
#endif
            break;
        case MISC_STONE_OF_EARTH_ELEMENTALS:
#ifdef JP
            description +=
                "大地の精霊を召換するための魔法の道具だ。 $"
                "これは幾分信頼性に欠ける品物で、うまく発動するまで $"
                "何度も試みなくてはならない場合が多い。 $"
                "使用には危険を伴うが、使用者が然るべき精霊魔法に $"
                "習熟していればその危険は減少する。 ";
#else
            description += "A magical device for summoning earth "
                "elementals. It is rather unreliable, and usually "
                "requires several attempts to function correctly. "
                "Using it carries an element of risk, which is reduced "
                "if one is skilled in the appropriate elemental magic.";
#endif
            break;
        case MISC_LANTERN_OF_SHADOWS:
            description +=
#ifdef JP
                "この邪悪な品は、小さな代償と引き換えに $"
                "使用者に手を貸す闇の力を召換する。 ";
#else
                "An unholy device which calls on the powers of darkness "
                "to assist its user, with a small cost attached. ";
#endif
            break;
        case MISC_HORN_OF_GERYON:
            description +=
#ifdef JP
                "地獄の入り口の番人であるゲリュオンの角笛だ。 $"
                "伝説によると、いずれかの地獄に訪れることを欲する定命の者は $"
                "中に入るためにこれを使わなければならないという。 ";
#else
                "The horn belonging to Geryon, guardian of the Vestibule "
                "of Hell. Legends say that a mortal who desires access "
                "into one of the Hells must use it in order to gain entry. ";
#endif
            break;
        case MISC_BOX_OF_BEASTS:
            description +=
#ifdef JP
                "沢山の野生の獣が収納された魔法の匣だ。 $"
                "匣の蓋を開けば、獣を解き放つことができるかもしれない ";
#else
                "A magical box containing many wild beasts. One may "
                "allow them to escape by opening the box's lid. ";
#endif
            break;
        case MISC_DECK_OF_WONDERS:
            description +=
#ifdef JP
                "非常に神秘的な魔法のカードの一組だ。 $"
                "ただしカードを引くのなら、起こり得る結果への覚悟が必要だ！ ";
#else
                "A deck of highly mysterious and magical cards. One may "
                "draw a random card from it, but should be prepared to "
                "suffer the possible consequences! ";
#endif
            break;
        case MISC_DECK_OF_SUMMONINGS:
            description +=
#ifdef JP
                "魔法のカードの一組だ。 $"
                "奇怪で驚くべき生物が描かれている。 ";
#else
                "A deck of magical cards, depicting a range of weird and "
                "wondrous creatures. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_ENERGY:
            description +=
#ifdef JP
                "使用することで魔力を回復させることができる魔法の道具だ。$"
                "しかし使用には魔力を完全に吸い取られてしまう危険を伴う。 $"
                "魔力を奪われる危険の率は使用者の魔力の最大値と現在値の比に $"
                "反比例する。 $"
                "つまり、魔力が最大に近い状態で使用するのが最も有効だ。 ";
#else
                "A magical device which can be used to restore one's "
                "reserves of magical energy, but the use of which carries "
                "the risk of draining all of those energies completely. "
                "This risk varies inversely with the proportion of their "
                "maximum energy which the user possesses; a user near his "
                "or her full potential will find this item most beneficial. ";
#endif
            break;
        case MISC_EMPTY_EBONY_CASKET:
#ifdef JP
            description += "力を使い果たした魔法の匣だ。 ";
#else
            description += "A magical box after its power is spent. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_FIXATION:
            description +=
#ifdef JP
                "危険な品物だ。 $"
                "この水晶球はこれを覗き込むような無思慮な者を催眠にかけて、 $"
                "長時間にわたって無力な状態にしてしまう。 ";
#else
                "A dangerous item which hypnotises anyone so unwise as "
                "to gaze into it, leaving them helpless for a significant "
                "length of time. ";
#endif
            break;
        case MISC_DISC_OF_STORMS:
            description +=
#ifdef JP
                "この極めて強力なアイテムは、破壊的な雷電の嵐を解き放つ。 $"
                "これは大気の精霊魔法の熟練者に扱われると特に効果的だ。 $"
                "ただし電気を絶縁した状態では使用することができない。 ";
#else
                "This extremely powerful item can unleash a destructive "
                "storm of electricity. It is especially effective in the "
                "hands of one skilled in air elemental magic, but cannot "
                "be used by one who is not a conductor. ";
#endif
            break;
        case MISC_RUNE_OF_ZOT:
            description +=
#ifdef JP
                "ゾットの領域に訪れることを可能にする御守りだ。 ";
#else
                "A talisman which allows entry into Zot's domain. ";
#endif
            break;
        case MISC_DECK_OF_TRICKS:
            description +=
#ifdef JP
                "愉快なトリックでいっぱいの魔法のカードの一組だ。 ";
#else
                "A deck of magical cards, full of amusing tricks. ";
#endif
            break;
        case MISC_DECK_OF_POWER:
#ifdef JP
            description += "強力な魔法のカードの一組だ。 ";
#else
            description += "A deck of powerful magical cards. ";
#endif
            break;
        case MISC_PORTABLE_ALTAR_OF_NEMELEX:
            description +=
#ifdef JP
                "簡単に分解・組み立てができるよう造られたネメレクス・ソベーの祭壇だ。 $"
                "この祭壇は床に何もない場所でならどこでも組み立て可能だ。 $"
                "そして祈り終わったら再び持ち運ぶことができる。 ";
#else
                "An altar to Nemelex Xobeh, built for easy assembly and "
                "disassembly.  Evoke it to place it on a clear patch of floor, "
                "then pick it up again when you've finished. ";
#endif
            break;
        default:
#ifdef JP
            DEBUGSTR("知られざるガラクタアイテム(2)");
#else
            DEBUGSTR("Unknown misc item (2)");
#endif
        }
    }
    else
    {
        switch (item.sub_type)
        {
        case MISC_BOTTLED_EFREET:
#ifdef JP
            description += "重い青銅の瓶だ。触ると暖かい。 ";
#else
            description += "A heavy bronze flask, warm to the touch. ";
#endif
            break;
        case MISC_CRYSTAL_BALL_OF_ENERGY:
        case MISC_CRYSTAL_BALL_OF_FIXATION:
        case MISC_CRYSTAL_BALL_OF_SEEING:
#ifdef JP
            description += "透き通った水晶球だ。 ";
#else
            description += "A sphere of clear crystal. ";
#endif
            break;
        case MISC_AIR_ELEMENTAL_FAN:
#ifdef JP
            description += "これは扇だ。 ";
#else
            description += "A fan. ";
#endif
            break;
        case MISC_LAMP_OF_FIRE:
#ifdef JP
            description += "これはランプだ。 ";
#else
            description += "A lamp. ";
#endif
            break;
        case MISC_STONE_OF_EARTH_ELEMENTALS:
#ifdef JP
            description += "これは岩の塊だ。 ";
#else
            description += "A lump of rock. ";
#endif
            break;
        case MISC_LANTERN_OF_SHADOWS:
#ifdef JP
            description += "古びた骨で作られた奇怪なランタンだ。 ";
#else
            description += "A strange lantern made out of ancient bones. ";
#endif
            break;
        case MISC_HORN_OF_GERYON:
#ifdef JP
            description += "邪悪なエネルギーに輝く、立派な銀の角笛だ。 ";
#else
            description += "A great silver horn, radiating unholy energies. ";
#endif
            break;
        case MISC_BOX_OF_BEASTS:
        case MISC_EMPTY_EBONY_CASKET:
#ifdef JP
            description += "小さな黒い匣だ。中がとても気になる。 ";
#else
            description += "A small black box. I wonder what's inside? ";
#endif
            break;
        case MISC_DECK_OF_WONDERS:
        case MISC_DECK_OF_TRICKS:
        case MISC_DECK_OF_POWER:
        case MISC_DECK_OF_SUMMONINGS:
#ifdef JP
            description += "一組のカードだ。 ";
#else
            description += "A deck of cards. ";
#endif
            break;
        case MISC_RUNE_OF_ZOT:
#ifdef JP
            description += "ある種の御守りだ。 ";
#else
            description += "A talisman of some sort. ";
#endif
            break;
        case MISC_DISC_OF_STORMS:
#ifdef JP
            description += "灰色の円盤だ。 ";
#else
            description += "A grey disc. ";
#endif
            break;
        case MISC_PORTABLE_ALTAR_OF_NEMELEX:
            description +=
#ifdef JP
                "簡単に分解・組み立てができるよう造られたネメレクス・ソベーの祭壇だ。 $"
                "この祭壇は床に何もない場所でならどこでも組み立て可能だ。 $"
                "そして祈り終わったら再び持ち運ぶことができる。 ";
#else
                "An altar to Nemelex Xobeh, built for easy assembly and "
                "disassembly.  Evoke it to place on a clear patch of floor, "
                "then pick it up again when you've finished. ";
#endif
            break;
        default:
#ifdef JP
            DEBUGSTR("知られざるガラクタアイテム");
#else
            DEBUGSTR("Unknown misc item");
#endif
        }
    }

    description += "$";

    return (description);
}

#if MAC
#pragma mark -
#endif

// ========================================================================
//      Public Functions
// ========================================================================

bool is_dumpable_artifact( const item_def &item, char verbose)
{
    bool ret = false;

    if (is_random_artefact( item ) || is_fixed_artefact( item ))
    {
        ret = item_ident( item, ISFLAG_KNOW_PROPERTIES );
    }
    else if (item.base_type == OBJ_ARMOUR
        && (verbose == 1 && item_ident( item, ISFLAG_KNOW_TYPE )))
    {
        const int spec_ench = get_armour_ego_type( item );
        ret = (spec_ench >= SPARM_RUNNING && spec_ench <= SPARM_PRESERVATION);
    }
    else if (item.base_type == OBJ_JEWELLERY
        && (verbose == 1
            && get_ident_type(OBJ_JEWELLERY, item.sub_type) == ID_KNOWN_TYPE))
    {
        ret = true;
    }

    return (ret);
}                               // end is_dumpable_artifact()


//---------------------------------------------------------------
//
// get_item_description
//
// Note that the string will include dollar signs which should
// be interpreted as carriage returns.
//
//---------------------------------------------------------------
std::string get_item_description( const item_def &item, char verbose, bool dump )
{
    std::string description;
    description.reserve(500);

    if (!dump)
    {
        char str_pass[ ITEMNAME_SIZE ];
        item_name( item, DESC_INVENTORY_EQUIP, str_pass );
        description += std::string(str_pass);
    }

    description += "$$";

#if DEBUG_DIAGNOSTICS
    if (!dump)
    {
        snprintf( info, INFO_SIZE,
#ifdef JP
                  "base: %d; sub: %d; plus: %d; plus2: %d; special: %ld$"
                  "quant: %d; colour: %d; flags: 0x%08lx$"
                  "x: %d; y: %d; link: %d$ident_type: %d$$",
                  item.base_type, item.sub_type, item.plus, item.plus2,
                  item.special, item.quantity, item.colour, item.flags,
                  item.x, item.y, item.link,
                  get_ident_type( item.base_type, item.sub_type ) );
#else
                  "base: %d; sub: %d; plus: %d; plus2: %d; special: %ld$"
                  "quant: %d; colour: %d; flags: 0x%08lx$"
                  "x: %d; y: %d; link: %d$ident_type: %d$$",
                  item.base_type, item.sub_type, item.plus, item.plus2,
                  item.special, item.quantity, item.colour, item.flags,
                  item.x, item.y, item.link,
                  get_ident_type( item.base_type, item.sub_type ) );
#endif

        description += info;
    }
#endif

    switch (item.base_type)
    {
    case OBJ_WEAPONS:
        description += describe_weapon( item, verbose );
        break;
    case OBJ_MISSILES:
        description += describe_ammo( item );
        break;
    case OBJ_ARMOUR:
        description += describe_armour( item, verbose );
        break;
    case OBJ_WANDS:
        description += describe_stick( item );
        break;
    case OBJ_FOOD:
        description += describe_food( item );
        break;
    case OBJ_SCROLLS:
        description += describe_scroll( item );
        break;
    case OBJ_JEWELLERY:
        description += describe_jewellery( item, verbose );
        break;
    case OBJ_POTIONS:
        description += describe_potion( item );
        break;
    case OBJ_STAVES:
        description += describe_staff( item );
        break;

    case OBJ_BOOKS:
        switch (item.sub_type)
        {
        case BOOK_DESTRUCTION:
#ifdef JP
            description += "極めて強力ではあるが予測がつかない魔法の書だ。 ";
#else
            description += "An extremely powerful but unpredictable book "
                "of magic. ";
#endif
            break;

        case BOOK_MANUAL:
#ifdef JP
            description +=
                "特定のスキルを大いに学ぶことが可能な貴重な魔法の本だ。 $"
                "使うたびに次第に崩れて行き、しまいには完全に崩れ去る。 ";
#else
            description += "A valuable book of magic which allows one to "
                "practise a certain skill greatly. As it is used, it gradually "
                "disintegrates and will eventually fall apart. ";
#endif
            break;

        default:
#ifdef JP
            description +=
                "魔法の呪文についての書物だ。 $"
                "強力な魔術には危険が伴うので注意が必要だ。 ";
#else
            description += "A book of magic spells. Beware, for some of the "
                "more powerful grimoires are not to be toyed with. ";
#endif
            break;
        }
        break;

    case OBJ_ORBS:
#ifdef JP
        description +=
            "このかけがえのないアーティファクトを手に地上に脱出すれば、 $"
            "あなたの冒険の旅は達成される。 ";
#else
        description += "Once you have escaped to the surface with "
            "this invaluable artefact, your quest is complete. ";
#endif
        break;

    case OBJ_MISCELLANY:
        description += describe_misc_item( item );
        break;

    case OBJ_CORPSES:
        description +=
#ifdef JP
            ((item.sub_type == CORPSE_BODY) ? "モンスターの死体だ。 $"
                                        : "肉が朽ちた骨だ。 $");
#else
            ((item.sub_type == CORPSE_BODY) ? "A corpse. "
                                        : "A decaying skeleton. ");
#endif
        break;

    default:
#ifdef JP
        DEBUGSTR("Bad item class");
        description += "このアイテムは存在しない。メーデー！メーデー！ $";
#else
        DEBUGSTR("Bad item class");
        description += "This item should not exist. Mayday! Mayday! ";
#endif
    }

    if (verbose == 1)
    {
#ifdef JP
        description += "$重量 ";
#else
        description += "$It weighs around ";
#endif

        const int mass = mass_item( item );

        char item_mass[16];
        itoa( mass / 10, item_mass, 10 );

        for (int i = 0; i < 14; i++)
        {
            if (item_mass[i] == '\0')
            {
                item_mass[i] = '.';
                item_mass[i+1] = (mass % 10) + '0';
                item_mass[i+2] = '\0';
                break;
            }
        }

        description += item_mass;
#ifdef JP
        description += " aum. ";        // arbitrary unit of mass
#else
        description += " aum. ";        // arbitrary unit of mass
#endif
    }

    return (description);
}                               // end get_item_description()


//---------------------------------------------------------------
//
// describe_item
//
// Describes all items in the game.
//
//---------------------------------------------------------------
void describe_item( const item_def &item )
{
#ifdef DOS_TERM
    char buffer[3400];

    gettext(25, 1, 80, 25, buffer);

    window(25, 1, 80, 25);
#endif

    clrscr();

    std::string description = get_item_description( item, 1 );

    print_description(description);

    set_keyin_mode(KEYIN_MODE_MORE);
    if (getch() == 0)
        getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM
    puttext(25, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}                               // end describe_item()


//---------------------------------------------------------------
//
// describe_spell
//
// Describes (most) every spell in the game.
//
//---------------------------------------------------------------
void describe_spell(int spelled)
{
    std::string description;

    description.reserve(500);

#ifdef DOS_TERM
    char buffer[3400];

    gettext(25, 1, 80, 25, buffer);
    window(25, 1, 80, 25);
#endif

    clrscr();
    description += spell_title( spelled );
#ifdef JP
    description += "$$この呪文は";   // NB: the leading space is here {dlb}
#else
    description += "$$This spell ";   // NB: the leading space is here {dlb}
#endif

    switch (spelled)
    {
    case SPELL_IDENTIFY:
#ifdef JP
        description += "唱えた者に魔法の品物の鑑定を可能とさせる。 ";
#else
        description += "allows the caster to determine the properties of "
            "an otherwise inscrutable magic item. ";
#endif
        break;

    case SPELL_TELEPORT_SELF:
#ifdef JP
        description += "唱えた者を無作為な地点に転送する。 ";
#else
        description += "teleports the caster to a random location. ";
#endif
        break;

    case SPELL_CAUSE_FEAR:
#ifdef JP
        description += "唱えた者の周辺にいる存在に恐怖を抱かせる。 ";
#else
        description += "causes fear in those near to the caster. ";
#endif
        break;

    case SPELL_CREATE_NOISE:
#ifdef JP
        description += "やかましい騒音を響かせる。 ";
#else
        description += "causes a loud noise to be heard. ";
#endif
        break;

    case SPELL_REMOVE_CURSE:
#ifdef JP
        description += "唱えた者の身につけているアイテムから呪いを取り除く。 ";
#else
        description += "removes curses from any items which are "
            "being used by the caster. ";
#endif
        break;

    case SPELL_MAGIC_DART:
#ifdef JP
        description += "魔法のエネルギーによる小さな矢を放つ。 ";
#else
        description += "hurls a small bolt of magical energy. ";
#endif
        break;

    case SPELL_FIREBALL:
#ifdef JP
        description += "爆発する火の玉を放つ。 $"
                       "もしもこの呪文を学ぶ者がすでに遅延式ファイアボールを $"
                       "習得している場合、この呪文は呪文レベルを消費せずに記憶できる。 ";
#else
        description += "hurls an exploding bolt of fire.  This spell "
            "does not cost additional spell levels if the learner already "
            "knows Delayed Fireball. ";
#endif
        break;

    case SPELL_DELAYED_FIREBALL:
#ifdef JP
        description = "$$詠唱に成功すると、術者は好きな時に瞬時にしてファイアボールを $"
                      "解き放つことができる。 $"
                      "この呪文を知っている場合、ファイアボールの呪文を呪文レベルの $"
                      "消費なしで記憶できる。 ";
#else
        description = "$$Successfully casting this spell gives the caster "
            "the ability to instantaneously release a fireball at a later "
            "time.  Knowing this spell allows the learner to memorise "
            "Fireball for no additional spell levels. ";
#endif
        break;

    case SPELL_BOLT_OF_MAGMA:
#ifdef JP
        description += "煮えたぎる溶岩の矢を放つ。 ";
#else
        description += "hurls a sizzling bolt of molten rock. ";
#endif
        break;

// spells 7 through 12 ??? {dlb}

    case SPELL_CONJURE_FLAME:
#ifdef JP
        description += "唸りをあげる炎の円柱を生成する。 ";
#else
        description += "creates a column of roaring flame. ";
#endif
        break;

    case SPELL_DIG:
#ifdef JP
        description += "強化されていない壁に横穴を穿つ。 ";
#else
        description += "digs a tunnel through unworked rock. ";
#endif
        break;

    case SPELL_BOLT_OF_FIRE:
#ifdef JP
        description += "強力な炎の矢を放つ。 ";
#else
        description += "hurls a great bolt of flames. ";
#endif
        break;

    case SPELL_BOLT_OF_COLD:
#ifdef JP
        description += "強力な氷と冷気の矢を放つ。 ";
#else
        description += "hurls a great bolt of ice and frost. ";
#endif
        break;

    case SPELL_LIGHTNING_BOLT:
#ifdef JP
        description += "強力な稲妻の矢を放つ。 $"
                       "この呪文は炎や氷の同格の呪文と比べるとダメージが小さいが、 $"
                       "列をなすモンスター全員を貫いて攻撃することができる。 ";
#else
        description += "hurls a mighty bolt of lightning. "
            "Although this spell inflicts less damage than "
            "similar fire and ice spells, it can at once "
            "rip through whole rows of creatures. ";
#endif
        break;

// spells 18 and 19 ??? {dlb}

    case SPELL_POLYMORPH_OTHER:
#ifdef JP
        description += "対象を無作為な種類のモンスターに変異させる。 ";
#else
        description += "randomly alters the form of another creature. ";
#endif
        break;

    case SPELL_SLOW:
#ifdef JP
        description += "対象の動きを遅くする。 ";
#else
        description += "slows the actions of a creature. ";
#endif
        break;

    case SPELL_HASTE:
#ifdef JP
        description += "対象の動きを加速する。 ";
#else
        description += "speeds the actions of a creature. ";
#endif
        break;

    case SPELL_PARALYZE:
#ifdef JP
        description += "対象を動くことができないようにする。 ";
#else
        description += "prevents a creature from moving. ";
#endif
        break;

    case SPELL_CONFUSING_TOUCH:
#ifdef JP
        description += "唱えた者の手に魔法のエネルギーを付与する。 $"
                       "このエネルギーはモンスターに素手で触れることで解放され、 $"
                       "モンスターに混乱状態をもたらす。 ";
#else
        description += "enchants the casters hands with magical energy. "
            "This energy is released when the caster touches "
            "a monster with their bare hands, and may induce "
            "a state of confusing in the monster. ";
#endif
        break;

    case SPELL_CONFUSE:
#ifdef JP
        description += "対象の精神に当惑と混乱をもたらす。 ";
#else
        description += "induces a state of bewilderment and confusion "
            "in a creature's mind. ";
#endif
        break;

    case SPELL_SURE_BLADE:
#ifdef JP
        description += "詠唱者とその手にした短剣との間に魔法の結合をもたらし、 $"
                       "短剣を非常に扱いやすくする。 ";
#else
        description += "forms a mystical bond between the caster and "
            "a wielded short blade, making the blade much " "easier to use. ";
#endif
        break;

    case SPELL_INVISIBILITY:
#ifdef JP
        description += "対象を他の存在の視覚から隠す。 ";
#else
        description += "hides a creature from the sight of others. ";
#endif
        break;

    case SPELL_THROW_FLAME:
#ifdef JP
        description += "小さな炎の矢を放つ。 ";
#else
        description += "throws a small bolt of flame. ";
#endif
        break;

    case SPELL_THROW_FROST:
#ifdef JP
        description += "小さな冷気の矢を放つ。 ";
#else
        description += "throws a small bolt of frost. ";
#endif
        break;

    case SPELL_CONTROLLED_BLINK:
        description +=
#ifdef JP
            "正確に制御可能な短距離のテレポートをもたらす。 $"
            "ただし制御されたテレポートは使用者を魔法のエネルギーで $"
            "汚染することがあるので注意しなくてはならない。 ";
#else
            "allows short-range translocation, with precise control. "
            "Be wary that controlled teleports will cause the subject to "
            "become contaminated with magical energy. ";
#endif
        break;

    case SPELL_FREEZING_CLOUD:
#ifdef JP
        description += "致命的な冷気の雲を出現させる。 ";
#else
        description += "conjures up a large cloud of lethally cold vapour. ";
#endif
        break;

    case SPELL_MEPHITIC_CLOUD:
        description +=
#ifdef JP
                        "広範囲だが短命な汚濁の雲を出現させる。 ";
#else
            "conjures up a large but short-lived cloud of vile fumes. ";
#endif
        break;

    case SPELL_RING_OF_FLAMES:
#ifdef JP
        description += "術者を、共に移動する猛火の環で取り巻き、 $"
                       "他の術者による炎の雲などから保護する。 $"
                       "この呪文は術者を炎の力と同調させ、炎の魔法を強化し $"
                       "炎からの保護をもたらす。 $"
                       "しかしながら、これは同じく氷による損傷を受けやすくしてしまう。 ";
                       // well, if it survives the fire wall it's a risk -- bwr
#else
        description += "surrounds the caster with a mobile ring of searing "
            "flame, and keeps other fire clouds away from the caster.  "
            "This spell attunes the caster to the forces of fire, "
            "increasing their fire magic and giving protection from fire.  "
            "However, it also makes them much more susceptible to the forces "
            "of ice. "; // well, if it survives the fire wall it's a risk -- bwr
#endif
        break;

    case SPELL_RESTORE_STRENGTH:
#ifdef JP
        description += "唱えた者の肉体的な強さを回復する。 ";
#else
        description += "restores the physical strength of the caster. ";
#endif
        break;

    case SPELL_RESTORE_INTELLIGENCE:
#ifdef JP
        description += "唱えた者の知力を回復する。 ";
#else
        description += "restores the intelligence of the caster. ";
#endif
        break;

    case SPELL_RESTORE_DEXTERITY:
#ifdef JP
        description += "唱えた者の器用さを回復する。 ";
#else
        description += "restores the dexterity of the caster. ";
#endif
        break;

    case SPELL_VENOM_BOLT:
#ifdef JP
        description += "毒の矢を放つ。 ";
#else
        description += "throws a bolt of poison. ";
#endif
        break;

    case SPELL_POISON_ARROW:
        description +=
#ifdef JP
            "最も不浄で有害な毒素による魔法的な矢を放つ。 $"
            "生命のない存在はこの魔法の影響を全く受けない。 ";
#else
            "hurls a magical arrow of the most vile and noxious toxin.  "
            "No living thing is completely immune to it's effects. ";
#endif
        break;

    case SPELL_OLGREBS_TOXIC_RADIANCE:
        description +=
#ifdef JP
            "術者の周囲に有毒な緑の光を浴びせる。 ";
#else
            "bathes the caster's surroundings in poisonous green light. ";
#endif
        break;

    case SPELL_TELEPORT_OTHER:
#ifdef JP
        description += "対象に無作為な場所への転位をもたらす。 ";
#else
        description += "randomly translocates another creature. ";
#endif
        break;

    case SPELL_LESSER_HEALING:
        description +=
#ifdef JP
            "術者が肉体に負ったダメージを少しだけ癒す。 ";
#else
            "heals a small amount of damage to the caster's body. ";
#endif
        break;

    case SPELL_GREATER_HEALING:
        description +=
#ifdef JP
            "術者が肉体に負ったダメージを大いに癒す。 ";
#else
            "heals a large amount of damage to the caster's body. ";
#endif
        break;

    case SPELL_CURE_POISON_I:
#ifdef JP
        description += "術者の体内から毒を取り除く。 ";
#else
        description += "removes poison from the caster's system. ";
#endif
        break;

    case SPELL_PURIFICATION:
#ifdef JP
        description += "術者の肉体を浄化し、毒と病気と悪意ある魔法を取り除く。 ";
#else
        description += "purifies the caster's body, removing "
            "poison, disease, and certain malign enchantments. ";
#endif
        break;

    case SPELL_DEATHS_DOOR:
#ifdef JP
        description += "極めて強力ではあるが、かなりの危険を伴う。 $"
                       "この呪文は術者を短時間だけ損害からほぼ無敵にする。 $"
                       "しかし術者を危険なまでに死に接近させもする。 $"
                       "(どの程度危険であるかは、死霊術の能力による) $"
                       "この呪文はいかなる時でも何らかの治癒効果を受けることで $"
                       "取り消すことができる。 $"
                       "また、呪文の効果時間が切れる直前に術者は一回警告を受ける。 $"
                       "アンデッドはこの呪文を使用することができない。 ";
#else
        description += "is extremely powerful, but carries a degree of risk. "
            "It renders living casters nigh invulnerable to harm "
            "for a brief period, but can bring them dangerously "
            "close to death (how close depends on one's necromantic "
            "abilities). The spell can be cancelled at any time by "
            "any healing effect, and the caster will receive one "
            "warning shortly before the spell expires. "
            "Undead cannot use this spell. ";
#endif
        break;

    case SPELL_SELECTIVE_AMNESIA:
#ifdef JP
        description += "術者が記憶から選択的に呪文を消去し、 $"
                       "その呪文に占有されていた分の魔力を取り戻すことを可能とする。 $"
                       "魔法使いは精神が呪文で埋め尽くされている場合でも残呪文レベルさえ $"
                       "足りるならこの呪文を記憶することができる。 $"
                       "(つまり、すでに上限数まで呪文を覚えてしまっていても記憶できる) ";
#else
        description += "allows the caster to selectively erase one spell "
            "from memory to recapture the magical energy bound "
            "up with it. Casters will be able to memorise this "
            "spell should even their minds be otherwise full of "
            "magic (i.e., already possessing the maximum number "
            "of spells). ";
#endif
        break;

    case SPELL_MASS_CONFUSION:
#ifdef JP
        description += "術者の姿を見た全ての者を混乱させる。 ";
#else
        description += "causes confusion in all who gaze upon the caster. ";
#endif
        break;

    case SPELL_STRIKING:
#ifdef JP
        description += "小さな力場の矢を放つ。 ";
#else
        description += "hurls a small bolt of force. ";
#endif
        break;

    case SPELL_SMITING:
#ifdef JP
        description += "術者の選んだ対象を打ち据える。 ";
#else
        description += "smites one creature of the caster's choice. ";
#endif
        break;

    case SPELL_REPEL_UNDEAD:
#ifdef JP
        description += "不浄な者を退ける神聖な力を呼び起こす。 ";
#else
        description += "calls on a divine power to repel the unholy. ";
#endif
        break;

    case SPELL_HOLY_WORD:
#ifdef JP
        description += "力ある言葉の詠唱によって邪悪なモンスターを $"
                       "退散させ破壊することができる。 ";
#else
        description += "involves the intonation of a word of power "
            "which repels and can destroy unholy creatures. ";
#endif
        break;

    case SPELL_DETECT_CURSE:
#ifdef JP
        description += "術者に所有物の中の呪いの存在を警告する。 ";
#else
        description += "alerts the caster to the presence of curses "
            "on his or her possessions. ";
#endif
        break;

    case SPELL_SUMMON_SMALL_MAMMAL:
#ifdef JP
        description += "術者の助けとなる小動物を1匹から数匹召換する。 ";
#else
        description += "summons one or more "
            "small creatures to the caster's aid. ";
#endif
        break;

    case SPELL_ABJURATION_I:
#ifdef JP
        description += "召換された敵対的なモンスターを元いた場所に送還しようと試みる。 $"
                       "少なくとも対象の存在可能な時間を短くする。 ";
#else
        description += "attempts to send hostile summoned creatures to "
            "the place from whence they came, or at least "
            "shorten their stay in the caster's locality. ";
#endif
        break;

    case SPELL_SUMMON_SCORPIONS:
#ifdef JP
        description += "術者の助けとなる大サソリを1匹から数匹召換する。 ";
#else
        description += "summons one or more "
            "giant scorpions to the caster's assistance. ";
#endif
        break;

    case SPELL_LEVITATION:
#ifdef JP
        description += "術者を宙に浮かせる。 ";
#else
        description += "allows the caster to float in the air. ";
#endif
        break;

    case SPELL_BOLT_OF_DRAINING:
#ifdef JP
        description += "負のエネルギーの致命的な矢を放つ。 $"
                       "命中すると生きているモンスターの生命力を衰弱させる。 ";
#else
        description += "hurls a deadly bolt of negative energy, "
            "which drains the life from any living creature " "it strikes. ";
#endif
        break;

    case SPELL_LEHUDIBS_CRYSTAL_SPEAR:
#ifdef JP
        description += "致命的なまでに鋭い水晶の矢を放つ。 ";
#else
        description += "hurls a lethally sharp bolt of crystal. ";
#endif
        break;

    case SPELL_BOLT_OF_INACCURACY:
#ifdef JP
        description += "白熱するエネルギーの矢を実体化させ、命中した対象に $"
                       "大ダメージを与える。 $"
                       "不幸にもこの呪文は狙いをつけることが非常に難しく、 $"
                       "極々稀にしか命中しない。ご愁傷様である。 ";
#else
        description += "inflicts enormous damage upon any creature struck "
            "by the bolt of incandescent energy conjured into "
            "existence. Unfortunately, it is very difficult to "
            "aim and very rarely hits anything. Pity, that. ";
#endif
        break;

    case SPELL_POISONOUS_CLOUD:
#ifdef JP
        description += "致命的なガスによる巨大な雲を発生させる。 ";
#else
        description += "conjures forth a great cloud of lethal gasses. ";
#endif
        break;

    case SPELL_FIRE_STORM:
#ifdef JP
        description += "唸りをあげる強力な炎の嵐を造り出す。 ";
#else
        description += "creates a mighty storm of roaring flame. ";
#endif
        break;

    case SPELL_DETECT_TRAPS:
#ifdef JP
        description += "術者の付近にある罠を明らかにする。 ";
#else
        description += "reveals traps in the caster's vicinity. ";
#endif
        break;

    case SPELL_BLINK:
#ifdef JP
        description += "術者に無作為な座標への短距離の瞬間移動をもたらす。 ";
#else
        description += "randomly translocates the caster a short distance. ";
#endif
        break;

    case SPELL_ISKENDERUNS_MYSTIC_BLAST:
#ifdef JP
        description += "バチバチと爆ぜる破壊的なエネルギーの球を放つ。 ";
#else
        description += "throws a crackling sphere of destructive energy. ";
#endif
        break;

    case SPELL_SWARM:
#ifdef JP
        description += "有害な昆虫の群れを召換する。 ";
#else
        description += "summons forth a pestilential swarm. ";
#endif
        break;

    case SPELL_SUMMON_HORRIBLE_THINGS:
#ifdef JP
        description += "アビスへの門を開き、かの恐るべき場所から $"
                       "1体から数体のひどく忌まわしい者を召換する。 $"
                       "この召換に応える者たちは、奉仕の代償として $"
                       "術者の知力の一部を要求する。 ";
#else
        description += "opens a gate to the Abyss and calls through "
            "one or more hideous abominations from that dreadful place."
            "  The powers who answer this invocation require of casters "
            "a portion of their intellect in exchange for this service.";
#endif
        break;

    case SPELL_ENSLAVEMENT:
#ifdef JP
        description += "敵対的なモンスターをしばらくの間あなたの味方にする。 ";
#else
        description += "causes an otherwise hostile creature "
            "to fight on your side for a while. ";
#endif
        break;

    case SPELL_MAGIC_MAPPING:
#ifdef JP
        description += "術者の周辺の地形を明らかにする。 ";
#else
        description += "reveals details about the caster's surroundings. ";
#endif
        break;

    case SPELL_HEAL_OTHER:
#ifdef JP
        description += "離れた場所からモンスターを回復できる。 ";
#else
        description += "heals another creature from a distance. ";
#endif
        break;

    case SPELL_ANIMATE_DEAD:
#ifdef JP
        description += "死体を蘇生し、術者のしもべとする。 $"
                       "術者から一定距離にある全ての死体が対象となる。 $"
                       "つまりこの呪文によって、力ある術者は魂なきアンデッドの $"
                       "軍隊を編成することができる。 ";
#else
        description += "causes the dead to rise up and serve the caster; "
            "every corpse within a certain distance of the caster "
            "is affected. By means of this spell, powerful casters "
            "could press into service an army of the mindless undead. ";
#endif
        break;

    case SPELL_PAIN:
#ifdef JP
        description += "生命のあるモンスターに苦痛に満ちた損傷を与える。 $";
#else
        description += "inflicts an extremely painful injury "
            "upon one living creature. ";
#endif
        break;

    case SPELL_EXTENSION:
        description +=
#ifdef JP
            "術者に影響を与えている大抵の有益な魔法について、 $"
            "持続時間を延長する。 ";
#else
            "extends the duration of most beneficial enchantments "
            "affecting the caster. ";
#endif
        break;

    case SPELL_CONTROL_UNDEAD:
        description +=
#ifdef JP
            "術者の近辺にいるアンデッドを奴隷化しようと試みる。 ";
#else
            "attempts to enslave any undead in the vicinity of the caster. ";
#endif
        break;

    case SPELL_ANIMATE_SKELETON:
#ifdef JP
        description += "モンスターの骨を反生命の存在として蘇生する。 ";
#else
        description += "raises an inert skeleton to a state of unlife. ";
#endif
        break;

    case SPELL_VAMPIRIC_DRAINING:
#ifdef JP
        description += "生きたモンスターから生命力を奪い、術者に与える。 $"
                       "ただし生命力は術者が吸収できる量を越えては奪えない。 ";
#else
        description += "steals the life of a living creature and grants it "
            "to the caster. Life will not be drained in excess of "
            "what the caster can capably absorb. ";
#endif
        break;

    case SPELL_SUMMON_WRAITHS:
        description +=
#ifdef JP
            "術者を手助けするアンデッドの勢力を召換する。 ";
#else
            "calls on the powers of the undead to aid the caster. ";
#endif
        break;

    case SPELL_DETECT_ITEMS:
        description +=
#ifdef JP
            "術者の近辺に存在する全てのアイテムを発見する。 ";
#else
            "detects any items lying about the caster's general vicinity. ";
#endif
        break;

    case SPELL_BORGNJORS_REVIVIFICATION:
#ifdef JP
        description += "術者とそのしもべの全ての傷を瞬時に治癒するが、 $"
                       "代償として永久的に負傷への耐久力が減少する。 $"
                       "代償の大きさは魔法の技能に反比例する。 ";
#else
        description += "instantly heals any and all wounds suffered by the "
            "caster with an attendant, but also permanently lessens his or her "
            "resilience to injury -- the severity of which is dependent on "
            "(and inverse to) magical skill. ";
#endif
        break;

    case SPELL_BURN:
#ifdef JP
        description += "モンスターを燃焼させる。 ";
#else
        description += "burns a creature. ";
#endif
        break;

    case SPELL_FREEZE:
#ifdef JP
        description += "モンスターを凍えさせる。 $"
                       "これは一時的に冷血動物の代謝を遅くする場合がある。 ";
#else
        description += "freezes a creature. This may temporarily slow the "
            "metabolism of a cold-blooded creature. ";
#endif
        break;

    case SPELL_SUMMON_ELEMENTAL:
#ifdef JP
        description +=
            "元素界から精霊を召換し術者の助けとする。 $"
            "召換には対応する元素が大量に利用可能でなくてはならない。 $"
            "元素の確保は大地と大気の場合にはほとんどの場合 $"
            "問題にならないが、火と水の場合には問題となるだろう。 $"
            "精霊たちは大抵は術者に友好的である。 $"
            "……術者が該当する精霊魔法に熟達している場合にはだが。 ";
#else
        description += "calls forth "
            "a spirit from the elemental planes to aid the caster. "
            "A large quantity of the desired element must be "
            "available; this is rarely a problem for earth and air, "
            "but may be for fire or water. The elemental will usually "
            "be friendly to casters -- especially those skilled in "
            "the appropriate form of elemental magic.";
#endif
        break;

    case SPELL_OZOCUBUS_REFRIGERATION:
#ifdef JP
        description += "術者とその周辺から熱を奪い取り、冷気に耐性のない存在 $"
                       "全てに損害を与える。 ";
#else
        description += "drains the heat from the caster and her "
            "surroundings, causing harm to all creatures not resistant to "
            "cold. ";
#endif
        break;

    case SPELL_STICKY_FLAME:
#ifdef JP
        description += "液体の炎の塊を作り出す。 $"
                       "この液体は命中したモンスターに粘着して焼き尽くす。 ";
#else
        description += "conjures a sticky glob of liquid fire, which will "
            "adhere to and burn any creature it strikes. ";
#endif
        break;

    case SPELL_SUMMON_ICE_BEAST:
#ifdef JP
        description += "氷の獣を召換して術者のしもべとする。 ";
#else
        description += "calls forth " "a beast of ice to serve the caster. ";
#endif
        break;

    case SPELL_OZOCUBUS_ARMOUR:
#ifdef JP
        description +=
            "氷の保護層で術者の体を覆う。 $"
            "この強度は術者の氷の魔術の技能に依存している。 $"
            "術者とその装備は冷気から保護されるが、 $"
            "術者が重量のある鎧を着用している場合にはこの呪文は機能しない。 $"
            "術者が氷の躯である時には、この呪文の効果は増大する。 ";
#else
        description += "encases the caster's body in a protective layer "
            "of ice, the power of which depends on his or her "
            "skill with Ice magic. The caster and the caster's "
            "equipment are protected from the cold, but the "
            "spell will not function for casters already wearing "
            "heavy armour.  The effects of this spell are boosted "
            "if the caster is in Ice Form. ";
#endif
        break;

    case SPELL_CALL_IMP:
#ifdef JP
        description += "地獄の穴から下級の悪魔を召換する。 ";
#else
        description += "calls forth " "a minor demon from the pits of Hell. ";
#endif
        break;

    case SPELL_REPEL_MISSILES:
#ifdef JP
        description += "投射された攻撃が術者に命中する確率を減少させる。 $"
                       "その効果は小さな飛来物に非常に大きな影響を与え、 $"
                       "稲妻やドラゴンのブレスなどの強力な攻撃にも効果を及ぼす。 ";
#else
        description += "reduces the chance of projectile attacks striking "
            "the caster. Even powerful attacks such as "
            "lightning bolts or dragon breath are affected, "
            "although smaller missiles are repelled to a "
            "much greater extent. ";
#endif
        break;

    case SPELL_BERSERKER_RAGE:
#ifdef JP
        description += "術者を一時的な狂気の激怒に駆り立てる。 ";
#else
        description += "sends the caster into a temporary psychotic rage. ";
#endif
        break;

    case SPELL_DISPEL_UNDEAD:
        description +=
#ifdef JP
            "アンデッドモンスターに非常に大きなダメージを与える。 ";
#else
            "inflicts a great deal of damage on an undead creature. ";
#endif
        break;

        // spell  86 - Guardian
        // spell  87 - Pestilence
        // spell  99 - Thunderbolt
        // spell 100 - Flame of Cleansing
        // spell 101 - Shining Light
        // spell 102 - Summon Daeva
        // spell 103 - Abjuration II

    case SPELL_TWISTED_RESURRECTION:
#ifdef JP
        description += "術者が死肉の山に魔法の生命力を吹き込むことを可能とする。 $"
                       "この呪文を唱えるには結合させるために幾つかの死体を必要とする。 $"
                       "材料となる死体の山が大きいほど成功の確率は上昇する。 ";
#else
        description += "allows its caster to imbue a mass of deceased flesh "
            "with a magical life force. Casting this spell involves "
            "the assembling several corpses together; the greater "
            "the combined mass of flesh available, the greater the "
            "chances of success. ";
#endif
        break;

    case SPELL_REGENERATION:
#ifdef JP
        description += "術者の再生能力を一定時間劇的に強化する。 $"
                       "この間は栄養の消耗も同様に増加する。 ";
#else
        description += "dramatically but temporarily increases the caster's "
            "recuperative abilities, while also increasing the rate "
            "of food consumption. ";
#endif
        break;

    case SPELL_BONE_SHARDS:
#ifdef JP
        description += "骸骨または昆虫の外骨格などから切り裂く破片による $"
                       "致命的な爆風を作り出す。 $"
                       "この呪文は死霊術単体で使用でき妖術を必要としないので、 $"
                       "低レベルに於ける強力な攻撃魔法として使うことができる。 $"
                       "大きくて重い骨を手に持って使うことで、この呪文の威力は $"
                       "増大する。 ";
#else
        description += "uses the bones of a skeleton (or similar materials: "
            "the rigid exoskeleton of an insect, for example) to "
            "dispense a lethal spray of slicing fragments, allowing "
            "its caster to dispense with conjurations in favour of "
            "necromancy alone to provide a low-level yet very "
            "powerful offensive spell. The use of a large and "
            "heavy skeleton (by wielding it) amplifies this spell's "
            "effect. ";
#endif
        break;

    case SPELL_BANISHMENT:
#ifdef JP
        description += "モンスターをアビスに追放する。 $"
                       "自らあの不愉快な場所を訪れたいのなら、 $"
                       "術者は自身にこの呪文をかけることができる。 ";
#else
        description += "banishes one creature to the Abyss. Those wishing "
            "to visit that unpleasant place in person may always "
            "banish themselves. ";
#endif
        break;

    case SPELL_CIGOTUVIS_DEGENERATION:
        description +=
#ifdef JP
            "モンスターを脈打つ肉塊に変異させる。 ";
#else
            "mutates one creature into a pulsating mass of flesh. ";
#endif
        break;

    case SPELL_STING:
#ifdef JP
        description += "有毒な魔法の投げ矢を放つ。 ";
#else
        description += "throws a magical dart of poison. ";
#endif
        break;

    case SPELL_SUBLIMATION_OF_BLOOD:
#ifdef JP
        description += "肉、血、その他体液を魔法のエネルギーに変換する。 $"
                       "術者はこの呪文を自分の肉体に向けて使うこともできる。 $"
                       "(これは危険ではあるが即死するほどのことはない) $"
                       "もしくは殺したての生肉を手に持った状態で魔力を $"
                       "引き出すこともできる。 ";
#else
        description += "converts flesh, blood, and other bodily fluids "
            "into magical energy. Casters may focus this spell "
            "on their own bodies (which can be dangerous but "
            "never directly lethal) or can wield freshly butchered "
            "flesh in order to draw power into themselves. ";
#endif
        break;

    case SPELL_TUKIMAS_DANCE:
#ifdef JP
        description += "術者が手に持っていた剣を空中で舞わせて、 $"
                       "敵を攻撃させる。 $"
                       "この呪文は魔法の杖と意思を持つアーティファクトには "
                       "作用しない。 ";
#else
        description += "causes a weapon held in the caster's hand to dance "
            "into the air and strike the caster's enemies. It will "
            "not function on magical staves and certain "
            "willful artefacts. ";
#endif
        break;

    case SPELL_HELLFIRE:        // basically, a debug message {dlb}
#ifdef JP
        description += "ディスペイターの杖のみで使用可能であるはずだ。 $"
                       "どうしてあなたはこれを読んでいるのだ？(describe.cc)";
#else
        description += "should only be available from Dispater's staff. "
            "So how are you reading this? ";
#endif
        break;

    case SPELL_SUMMON_DEMON:
#ifdef JP
        description += "パンデモニウムの領域への門を開き、 $"
                       "その住人の一人を術者のしもべとして一定時間召換する。 ";
#else
        description += "opens a gate to the realm of Pandemonium "
            "and draws forth one of its inhabitants "
            "to serve the caster for a time. ";
#endif
        break;

    case SPELL_DEMONIC_HORDE:
#ifdef JP
        description += "術者の敵と戦う小さな悪魔の一群を召換する。 ";
#else
        description += "calls forth "
            "a small swarm of small demons "
            "to do battle with the caster's foes. ";
#endif
        break;

    case SPELL_SUMMON_GREATER_DEMON:
#ifdef JP
        description += "パンデモニウムの偉大なデーモンを呼び出し、 $"
                       "術者のしもべとする。注意すべきは、 $"
                       "デーモンに服従を強いている呪文の効果が、デーモンがこの世界に $"
                       "まだ存在しているうちに切れるかもしれないことだ！ ";
#else
        description += "calls forth one of the greater demons of Pandemonium "
            "to serve the caster. Beware, for the spell binding it "
            "to service may not outlast "
            "that which binds it to this world! ";
#endif
        break;

    case SPELL_CORPSE_ROT:
#ifdef JP
        description += "術者の周囲に転がる死体を急速に腐敗させ、 $"
                       "その課程で不浄極まる瘴気を発生する。 $"
                       "この瘴気は触れたモンスターの生命力を奪い取って行く。 ";
#else
        description += "rapidly accelerates the decomposition of any "
            "corpses lying around the caster, emitting in"
            "process a foul miasmic vapour, which eats away "
            "at the life force of any creature it envelops. ";
#endif
        break;

    case SPELL_TUKIMAS_VORPAL_BLADE:
#ifdef JP
        description += "術者の手にした剣に一時的な鋭さを与える。 $"
                       "これは既に何らかの魔力を帯びている武器には効果がない。 ";
#else
        description += "bestows a lethal but temporary sharpness "
            "on a sword held by the caster. It will not affect "
            "weapons otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_FIRE_BRAND:
#ifdef JP
        description += "術者の手にした武器に炎を纏わせる。 $"
                       "これは既に何らかの魔力を帯びている武器には効果がない。 ";
#else
        description += "sets a weapon held by the caster ablaze. It will not "
            "affect weapons otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_FREEZING_AURA:
        description +=
#ifdef JP
            "術者の手にした武器に凍てつく冷気を纏わせる。 $"
            "これは既に何らかの魔力を帯びている武器には効果がない。 ";
#else
            "surrounds a weapon held by the caster with an aura of "
            "freezing cold. It will not affect weapons which are "
            "otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_LETHAL_INFUSION:
#ifdef JP
        description += "術者の手にした武器に邪悪な力を充填する。 $"
                       "これは既に何らかの魔力を帯びている武器には効果がない。 ";
#else
        description += "infuses a weapon held by the caster with unholy "
            "energies. It will not affect weapons which are "
            "otherwise subject to special enchantments. ";
#endif
        break;

    case SPELL_CRUSH:           // a theory of gravity in Crawl? {dlb}
#ifdef JP
        description += "重力波によって周囲のモンスターを押し潰す。 ";
#else
        description += "crushes a nearby creature with waves of "
            "gravitational force. ";
#endif
        break;

    case SPELL_BOLT_OF_IRON:
#ifdef JP
        description += "大きくて重い金属の矢を術者の敵に放つ。 ";
#else
        description += "hurls "
            "a large and heavy metal bolt " "at the caster's foes. ";
#endif
        break;

    case SPELL_STONE_ARROW:
#ifdef JP
        description += "鋭く尖った岩の杭を放つ。 ";
#else
        description += "hurls "
            "a sharp spine of rock outward from the caster. ";
#endif
        break;

    case SPELL_TOMB_OF_DOROKLOHE:
#ifdef JP
        description += "術者を岩の壁による柩に囲む。 $"
                       "この壁は大抵の物質を破壊して作り出されるが、 $"
                       "モンスターの存在する地点には作られない。 $"
                       "注意:岩から出る手段なしにこの呪文を唱えるのは馬鹿者だけだ。 ";
#else
        description += "entombs the caster within four walls of rock. These "
            "walls will destroy most objects in their way, but "
            "their growth is obstructed by the presence of any "
            "creature. Beware - only the unwise cast this spell "
            "without reliable means of escape. ";
#endif
        break;

    case SPELL_STONEMAIL:
#ifdef JP
        description += "術者を分厚い石の鱗で覆う。 $"
                       "鱗の耐久性は術者の大地の魔法の技能に依存する。 $"
                       "この鱗は他の防具と共存できるが、非常に重くて動作を妨げる $"
                       "術者が石像の形態を取っている時、この呪文の効果は強化される ";
#else
        description += "covers the caster with chunky scales of stone, "
            "the durability of which depends on his or her "
            "skill with Earth magic. These scales can coexist "
            "with other forms of armour, but are in and of "
            "themselves extremely heavy and cumbersome.  The effects "
            "of this spell are increased if the caster is in Statue Form. ";
#endif
        break;

    case SPELL_SHOCK:
#ifdef JP
        description += "電撃の矢を放つ。 ";
#else
        description += "throws a bolt of electricity. ";
#endif
        break;

    case SPELL_SWIFTNESS:
#ifdef JP
        description += "術者に卓越した移動速度をもたらす。 $"
                       "この呪文は空を飛んでいる場合にも効果がある。 ";
#else
        description += "imbues its caster with the ability to achieve "
            "great movement speeds.  Flying spellcasters can move even "
            "faster.";
#endif
        break;

    case SPELL_FLY:
        description +=
#ifdef JP
            "術者に空中を飛ぶ能力を与える。 ";
#else
            "grants to the caster the ability to fly through the air. ";
#endif
        break;

    case SPELL_INSULATION:
#ifdef JP
        description += "電気ショックから術者を保護する。 ";
#else
        description += "protects the caster from electric shocks. ";
#endif
        break;

    case SPELL_ORB_OF_ELECTROCUTION:
#ifdef JP
        description += "バチバチと爆ぜる電気エネルギーの球体を放つ。 $"
                       "これは強烈な衝撃を伴う爆発を起こす。 ";
#else
        description += "hurls "
            "a crackling orb of electrical energy "
            "which explodes with immense force on impact. ";
#endif
        break;

    case SPELL_DETECT_CREATURES:
#ifdef JP
        description += "術者が一定範囲内のモンスターを感知することを可能にする。 ";
#else
        description += "allows the caster to detect any creatures "
            "within a certain radius. ";
#endif
        break;

    case SPELL_CURE_POISON_II:
        description +=
#ifdef JP
            "術者の体内から毒素を取り除く。 ";
#else
            "removes some or all toxins from the caster's system. ";
#endif
        break;

    case SPELL_CONTROL_TELEPORT:
#ifdef JP
        description += "術者に制御されたテレポートをもたらす。 $"
                       "ただし制御されたテレポートは使用者を魔法のエネルギーで $"
                       "汚染することがあるので注意しなくてはならない。 ";
#else
        description += "allows the caster to control translocations.  Be "
            "wary that controlled teleports will cause the subject to "
            "become contaminated with magical energy. ";
#endif
        break;

    case SPELL_POISON_AMMUNITION:
#ifdef JP
        description += "術者が手に持った矢や弾に毒を塗る。 ";
#else
        description += "envenoms missile ammunition held by the caster. ";
#endif
        break;

    case SPELL_POISON_WEAPON:
        description +=
#ifdef JP
            "鋭い刃を持つ武器を一時的に毒で覆う。 $"
            "これは既に何らかの魔力を帯びている武器には効果がない。 ";
#else
            "temporarily coats any sharp bladed weapon with poison.  Will only "
            "work on weapons without an existing enchantment.";
#endif
        break;

    case SPELL_RESIST_POISON:
#ifdef JP
        description += "術者を一定時間あらゆる毒の被害から保護する。 ";
#else
        description += "protects the caster from exposure to all poisons "
            "for a period of time. ";
#endif
        break;

    case SPELL_PROJECTED_NOISE:
#ifdef JP
        description += "術者が選んだ地点から発生する騒音を作り出す。 ";
#else
        description += "produces a noise emanating "
            "from a place of the caster's own choosing. ";
#endif
        break;

    case SPELL_ALTER_SELF:
#ifdef JP
        description += "術者の肉体に変異をもたらす。 $"
                       "これは術者に衰弱状態をもたらす。 $"
                       "(ただしこの衰弱は直接には死をもたらさない) $"
                       "この呪文はすでに重度の突然変異を蒙っている者には $"
                       "効果を及ぼさないかもしれない。 ";
#else
        description += "causes aberrations to form in the caster's body, "
            "leaving the caster in a weakened state "
            "(though it is not fatal in and of itself). "
            "It may fail to affect those who are already "
            "heavily mutated. ";
#endif
        break;

// spell 145 - debugging ray

    case SPELL_RECALL:
#ifdef JP
        description += "召換士と死霊術師にとって非常に有用である。 $"
                       "唱えると仲間にしている全てのモンスターを $"
                       "術者の付近に呼び寄せる。 ";
#else
        description += "is greatly prized by summoners and necromancers, "
            "as it allows the caster to recall any friendly "
            "creatures nearby to a position adjacent to the caster. ";
#endif
        break;

    case SPELL_PORTAL:
#ifdef JP
        description += "数階の移動が可能な門を開く。 $"
                       "通常のダンジョンでしか呪文は働かない。 $"
                       "門は術者やモンスターが中に入ることができるだけの $"
                       "充分な時間、存続している。 $"
                       "この呪文でダンジョンの上限あるいは下限を越えて $"
                       "移動することはできない ";
#else
        description += "creates a gate allowing long-distance travel "
            "in relatively ordinary environments "
            "(i.e., the Dungeon only). The portal lasts "
            "long enough for the caster and nearby creatures "
            "to enter. Casters are never taken past the level "
            "limits of the current area. ";
#endif
        break;

    case SPELL_AGONY:
#ifdef JP
        description += "決して直接の死は引き起こさないが、 $"
                       "対象の回復力を半分に切り下げてしまう。 ";
#else
        description += "cuts the resilience of a target creature in half, "
            "although it will never cause death directly. ";
#endif
        break;

    case SPELL_SPIDER_FORM:
#ifdef JP
        description += "術者を一時的に、毒を持つ蜘蛛型のモンスターに変化させる。 $"
                       "この形態を取っている間には呪文の詠唱がやや困難になる。 $"
                       "この呪文は呪われた装備が脱げない状況では効果を発揮できない。 ";
#else
        description += "temporarily transforms the caster into a venomous, "
            "spider-like creature.  Spellcasting is slightly more difficult "
            "in this form.  This spell is not powerful enough to allow "
            "the caster to slip out of cursed equipment. ";
#endif
        break;

    case SPELL_DISRUPT:
#ifdef JP
        description += "モンスターを取り巻く空間に断裂を起こして負傷させる。 ";
#else
        description += "disrupts space around another creature, "
            "causing injury.";
#endif
        break;

    case SPELL_DISINTEGRATE:
#ifdef JP
        description += "特定の空間の中にある全ての物を徹底的に砕く。 $"
                       "これはモンスターに手酷い損害を引き起こすことができる。 ";
#else
        description += "violently rends apart anything in a small volume of "
            "space.  Can be used to cause severe damage.";
#endif
        break;

    case SPELL_BLADE_HANDS:
#ifdef JP
        description += "術者の手を大鎌に似た長大な刃に変化させる。 $"
                       "この状態では呪文の詠唱は幾分難しくなる。 $"
                       "この呪文は呪われた武器が手から離れない状態では成功しない。 ";
#else
        description += "causes long, scythe-shaped blades to grow "
            "from the caster's hands.  It makes spellcasting somewhat "
            "difficult.  This spell is not powerful enough to force "
            "a cursed weapon from the caster's hands.";
#endif
        break;

    case SPELL_STATUE_FORM:
#ifdef JP
        description += "術者を一時的に鈍重な、しかし非常に強靭な石像に $"
                       "変化させる。 ";
#else
        description += "temporarily transforms the caster into a "
            "slow-moving (but extremely robust) stone statue. ";
#endif
        break;

    case SPELL_ICE_FORM:
#ifdef JP
        description += "一時的に術者の肉体を、凍てつく氷のモンスターに変化させる。 ";
#else
        description += "temporarily transforms the caster's body into a "
            "frozen ice-creature. ";
#endif
        break;

    case SPELL_DRAGON_FORM:
#ifdef JP
        description += "術者を一時的に炎を吐くドラゴンに変化させる。 ";
#else
        description += "temporarily transforms the caster into a "
            "great, fire-breathing dragon. ";
#endif
        break;

    case SPELL_NECROMUTATION:
#ifdef JP
        description += "まず術者を負の力と親和性のある半物質的な $"
                       "幽体に変化させ、次にその体を死の力で満たす。 $"
                       "術者は冷気、毒、魔力、そして敵対的な負の力への $"
                       "抵抗力を得る。 $";
#else
        description += "first transforms the caster into a "
            "semi-corporeal apparition receptive to negative energy, "
            "then infuses that form with the powers of Death. "
            "The caster becomes resistant to "
            "cold, poison, magic and hostile negative energies. ";
#endif
        break;

    case SPELL_DEATH_CHANNEL:
#ifdef JP
        description += "術者が殺した生き物を幽鬼のしもべとして甦らせる。 ";
#else
        description += "raises living creatures slain by the caster "
            "into a state of unliving slavery as spectral horrors. ";
#endif
        break;

    case SPELL_SYMBOL_OF_TORMENT:
#ifdef JP
        description += "地獄の力を呼び起こし、術者の周囲のあらゆる生命ある $"
                       "存在に苦痛に満ちた損傷を引き起こす。 $"
                       "この力を呼び起こすのは危険を伴う。 $"
                       "なぜならこの力は術者にも被害を与え、術者が苦痛に $"
                       "免疫を持つ場合には作用しないためである。 $"
                       "その恐ろしい力にも関らず、この呪文は即死を引き起こしはしない。 ";
#else
        description += "calls on the powers of Hell to cause agonising "
            "injury to any living thing in the caster's vicinity. "
            "It carries within itself a degree of danger, "
            "for any brave enough to invoke it, for the Symbol "
            "also affects its caller and indeed will not function "
            "if he or she is immune to its terrible effects. "
            "Despite its ominous power, this spell is never lethal. ";
#endif
        break;

    case SPELL_DEFLECT_MISSILES:
#ifdef JP
        description += "投射された攻撃から術者を守る。 $"
                       "その効果は小さな飛来物に非常に大きな影響を与え、 $"
                       "稲妻やドラゴンのブレスなどの強力な攻撃にも効果を及ぼす。 ";
#else
        description += "protects the caster from "
            "any kind of projectile attack, "
            "although particularly powerful attacks "
            "(lightning bolts, etc.) are deflected "
            "to a lesser extent than lighter missiles. ";
#endif
        break;

    case SPELL_ORB_OF_FRAGMENTATION:
#ifdef JP
        description += "爆発して致命的な破片の雨を降らせる金属の塊を放つ。 $"
                       "破片はモンスターを切り刻むが、重装甲の対象にはあまり $"
                       "効果的ではない。 ";
#else
        description += "throws a heavy sphere of metal "
            "which explodes on impact into a rain of "
            "deadly, jagged fragments. "
            "It can rip a creature to shreds, "
            "but proves ineffective against heavily-armoured targets. ";
#endif
        break;

    case SPELL_ICE_BOLT:
#ifdef JP
        description += "巨大な氷塊を放つ。 $"
                       "これは凍結への耐性のないモンスターには特に有効だが、 $"
                       "呪文の破壊力の半分を占める重量と断面による切断からは $"
                       "冷気に耐性のあるモンスターでも損害を免れない。 ";
#else
        description += "throws forth a chunk of ice. "
            "It is particularly effective against "
            "those creatures not immune to the effects of freezing, "
            "but the half of its destructive potential that comes from "
            "its weight and cutting edges "
            "cannot be ignored by even cold-resistant creatures. ";
#endif
        break;

    case SPELL_ICE_STORM:
#ifdef JP
        description += "氷とみぞれ、そして凍気による嵐を発生させる。 ";
#else
        description += "conjures forth "
            "a raging blizzard of ice, sleet and freezing gasses. ";
#endif
        break;

    case SPELL_ARC:
#ifdef JP
        description += "強力な電流で無作為に選んだ周囲のモンスター一体を $"
                       "攻撃する。 ";
#else
        description += "zaps at random a nearby creature with a powerful "
            "electrical current.";
#endif
        break;

    case SPELL_AIRSTRIKE:       // jet planes in Crawl ??? {dlb}
        description +=
#ifdef JP
            "モンスターを取り巻く大気に強烈な竜巻を引き起こす。 ";
#else
            "causes the air around a creature to twist itself into "
            "a whirling vortex of meteorological fury. ";
#endif
        break;

    case SPELL_SHADOW_CREATURES:
#ifdef JP
        description +=
            "影と深淵の物質からモンスターを編み上げる。 $"
            "創り出された存在は術者の近くで何らかのモンスターへと $"
            "実体化する $"
            "この呪文はモンスターの装備までも生成する。 $"
            "モンスターに生成された装備は強固な現実性を持つため、 $"
            "永続的に存在する。 ";
#else
        description += "weaves a creature from shadows and threads of "
            "Abyssal matter. The creature thus brought into "
            "existence will recreate some type of creature "
            "found in the caster's immediate vicinity. "
            "The spell even creates appropriate equipment for "
            "the creature, which are given a lasting substance "
            //jmf: if also conjuration:
            //"by the spell's conjuration component. ";
            //jmf: else:
            "by their firm contact with reality. ";
#endif
        break;

        //jmf: new spells
    case SPELL_FLAME_TONGUE:
#ifdef JP
        description += "炎を短く迸らせる。 ";
#else
        description += "creates a short burst of flame.";
#endif
        break;

    case SPELL_PASSWALL:
#ifdef JP
        description += "ごく短い時間、術者の肉体が岩の中を通ることができるように $"
                       "変化させる。 $"
                       "岩を通過中に呪文の効果が切れる可能性があり、 $"
                       "また体が岩を透過する状態に変化するまでに $"
                       "無防備な状態となるので、この呪文には危険が伴う。 ";
#else
        description += "tunes the caster's body such that it can instantly "
            "pass through solid rock. This can be dangerous, "
            "since it is possible for the spell to expire while "
            "the caster is en route, and it also takes time for the "
            "caster to attune to the rock, during which time they will "
            "be helpless. ";
#endif
        break;

    case SPELL_IGNITE_POISON:
#ifdef JP
        description += "術者の視界にあるあらゆる毒素を焼夷の炎に変化させる。 $"
                       "これは毒性のあるモンスターや毒のポーションを運んでいる $"
                       "モンスターに対して非常に効果的だ。 $"
                       "この呪文は術者自身の体内の毒にも作用し、壮絶な苦痛と共に $"
                       "毒を焼き尽くす。 ";
#else
        description += "attempts to convert all poison within the caster's "
            "view into liquid flame. It is very effective against "
            "poisonous creatures or those carrying poison potions. "
            "It is also an amazingly painful way to eliminate "
            "poison from one's own system. ";
#endif
        break;

    case SPELL_STICKS_TO_SNAKES:        // FIXME: description sucks
#ifdef JP
        description += "術者の手にした木製のアイテムを材料にして $"
                       "強力な召換を行う。 $"
                       "魔術師の杖などの強力な魔法のかかったアイテムは $"
                       "影響を受けない。 ";
#else
        description += "uses wooden items in the caster's grasp as raw "
            "material for a powerful summoning. Note that highly "
            "enchanted items, such as wizard's staves, will not be "
            "affected. ";
#endif
        // "Good examples of sticks include arrows, quarterstaves and clubs.";
        break;

    case SPELL_SUMMON_LARGE_MAMMAL:
#ifdef JP
        description += "術者の助けとなる犬族を召換する。 ";
#else
        description += "summons a canine to the caster's aid.";
#endif
        break;

    case SPELL_SUMMON_DRAGON:   //jmf: reworking, currently unavailable
#ifdef JP
        description += "強力なドラゴンを召換して、術者の命令に従うように支配する。 $"
                       "注意すべきは、召換のみ成功し支配は失敗する可能性があることである。 ";
#else
        description += "summons and binds a powerful dragon to perform the "
            "caster's bidding. Beware, for the summons may succeed "
            "even as the binding fails. ";
#endif
        break;

    case SPELL_TAME_BEASTS:
#ifdef JP
        description += "術者の近くにいる動物の支配を試みる。 $"
                       "もともと飼い馴らすのが容易な動物に最も良く作用する。 ";
#else
        description += "attempts to tame animals in the caster's vicinity. "
            "It works best on animals amenable to domestication. ";
#endif
        break;

    case SPELL_SLEEP:
#ifdef JP
        description += "対象の代謝率を低下させ、低体温による冬眠状態を $"
                       "誘発する。 $"
                       "これは冷血動物には追加の効果を及ぼすかもしれない。 ";
#else
        description += "tries to lower its target's metabolic rate, "
            "inducing hypothermic hibernation. It may have side effects "
            "on cold-blooded creatures. ";
#endif
        break;

    case SPELL_MASS_SLEEP:
#ifdef JP
        description += "術者の視界内全てのモンスターの対象の代謝率を低下させ、 $"
                       "低体温による冬眠状態を誘発する。 $"
                       "これは冷血動物には追加の効果を及ぼすかもしれない。 ";
#else
        description += "tries to lower the metabolic rate of every creature "
            "within the caster's view enough to induce hypothermic hibernation. "
            "It may have side effects on cold-blooded creatures. ";
#endif
        break;

/* ******************************************************************
// not implemented {dlb}:
    case SPELL_DETECT_MAGIC:
      description += "probes one or more items lying nearby for enchantment. "
         "An experienced diviner may glean additional information. ";
      break;
****************************************************************** */

    case SPELL_DETECT_SECRET_DOORS:
#ifdef JP
        description += "探索にかかる時間を大幅に減らすことができるので、 $"
                       "世界中の怠け者なダンジョン野郎に愛されている。 ";
#else
        description += "is beloved by lazy dungeoneers everywhere, for it can "
            "greatly reduce time-consuming searches. ";
#endif
        break;

    case SPELL_SEE_INVISIBLE:
#ifdef JP
        description += "術者に通常の視覚からは隠された存在を見ることを $"
                       "可能にする。 ";
#else
        description += "enables the caster to perceive things that are "
            "shielded from ordinary sight. ";
#endif
        break;

    case SPELL_FORESCRY:
#ifdef JP
        description += "ごく短時間後までの未来を術者に予知させる。 $"
                       "戦闘の結果を見通すほどの未来はわからないが、 $"
                       "敵の一撃を回避するのには(反射神経が許す限り) $"
                       "充分な程度までの時間を予知できる。 ";
#else
        description += "makes the caster aware of the immediate future; "
            "while not far enough to predict the result of a "
            "fight, it does give the caster ample time to get "
            "out of the way of a punch (reflexes allowing). ";
#endif
        break;

    case SPELL_SUMMON_BUTTERFLIES:
        description +=
#ifdef JP
            "カラフルな蝶の群れを作り出す。なんて綺麗なんだろう！ ";
#else
            "creates a shower of colourful butterflies. How pretty!";
#endif
        break;

    case SPELL_WARP_BRAND:
#ifdef JP
        description += "一時的に術者の武器に、局所的な歪曲空間を結わえ付ける。 $"
                       "この空間は術者にも同様に影響を及ぼす可能性があるので、 $"
                       "この呪文の詠唱は非常に危険である。 ";
#else
        description += "temporarily binds a localized warp field to the "
            "invoker's weapon. This spell is very dangerous to cast, "
            "as the field is likely to effect the caster as well. ";
#endif
        break;

    case SPELL_SILENCE:
#ifdef JP
        description += "術者の周辺のあらゆる音を消し去る。 $"
                       "これは術者の周辺で巻物を読み上げること、呪文の詠唱、 $"
                       "祈り、叫びなどを不可能にする。術者もこの効果を蒙る。 $"
                       "この呪文があなたの存在を隠すことはない。 $"
                       "なぜならこの極端に不自然な効果は、モンスターにまず間違いなく $"
                       "何かが非常におかしいと警告を与えることになるからである。 ";
#else
        description += "eliminates all sound near the caster. This makes "
            "reading scrolls, casting spells, praying or yelling "
            "in the caster's vicinity impossible. (Applies to "
            "caster too, of course.)  This spell will not hide your "
            "presence, since its oppressive, unnatural effect "
            "will almost certainly alert any living creature that something "
            "is very wrong. ";
#endif
        break;

    case SPELL_SHATTER:
        description +=
#ifdef JP
            "術者の周囲に振動性の爆発を起こす。 $"
            "これは大抵のモンスターにダメージを与えるが、 $"
            "とりわけ石、金属、結晶などの弾力のない物質でできた $"
            "モンスターに効果がある。 $"
            "この魔法は壁に対しても効果があることで知られている。 ";
#else
            "causes a burst of concussive force around the caster, "
            "which will damage most creatures, although those "
            "composed of stone, metal or crystal, or otherwise "
            "brittle, will particularly suffer. The magic has been "
            "known to adversely affect walls. ";
#endif
        break;

    case SPELL_DISPERSAL:
#ifdef JP
        description += "術者に隣接するモンスターをどこかにテレポートさせる。 ";
#else
        description += "tries to teleport away any monsters directly beside "
                       "the caster. ";
#endif
        break;

    case SPELL_DISCHARGE:
#ifdef JP
        description += "術者に隣接するモンスター全てに稲妻を放つ。 $"
                       "稲妻は地面に落ちて消えるまで、隣接した者(術者を含む)に $"
                       "連鎖して行く。 ";
#else
        description += "releases electric charges against those "
                       "next to the caster.  These may arc to "
                       "adjacent monsters (or even the caster) before "
                       "they eventually ground out. ";
#endif
        break;

    case SPELL_BEND:
        description +=
#ifdef JP
            "局地的な空間歪曲を用いて周囲のモンスターに損傷を与える。 ";
#else
            "applies a localized spatial distortion to the detriment"
            " of some nearby creature. ";
#endif
        break;

    case SPELL_BACKLIGHT:
#ifdef JP
        description += "モンスターを取り囲む光輪を発生させ、その姿を $"
                       "鮮明に浮かび上がらせる。 $"
                       "この光はダンジョンの薄暗さを相殺するので、 $"
                       "この呪文を受けた敵に攻撃を命中させるのはかなり容易になる。 ";
#else
        description += "causes a halo of glowing light to surround and "
            "effectively outline a creature. This glow offsets "
            "the dark, musty atmosphere of the dungeon, and "
            "thereby makes the affected creature appreciably easier to hit.";
#endif
        break;

    case SPELL_INTOXICATE:
#ifdef JP
        description += "脳内物質のごく一部をアルコールに変換する。 $"
                       "これは術者の視界内の全ての知的な人型生物 $"
                       "(大抵は術者も含む)に作用する。 $"
                       "この呪文はしばしば魔術師の宴会の景気づけに唱えられる。 ";
#else
        description += "works by converting a small portion of brain matter "
            "into alcohol. It affects all intelligent humanoids within "
            "the caster's view (presumably including the caster). It "
            "is frequently used as an icebreaker at wizard parties. ";
#endif
        break;

    case SPELL_GLAMOUR: // intended only as Grey Elf ability
#ifdef JP
        description += "エルフの魔法である。 $"
                       "これは視覚を持つモンスターの軽信性と術者の美貌を引きだし、 $"
                       "モンスターを魅了して、混乱させるか昏睡状態にする。 ";
#else
        description += "is an Elvish magic, which draws upon the viewing "
            "creature's credulity and the caster's comeliness "
            "to charm, confuse or render comatose. ";
#endif
        break;

    case SPELL_EVAPORATE:
#ifdef JP
        description += "投げつけると爆発して雲になるようにポーションを加熱する。 $"
                       "この呪文は呪文の動作の一部として、ポーションをその場で $"
                       "投げなくてはならない。 ";
#else
        description += "heats a potion causing it to explode into a large "
            "cloud when thrown.  The potion must be thrown immediately, "
            "as part of the spell, for this to work. ";
#endif
        break;

    case SPELL_FULSOME_DISTILLATION:
#ifdef JP
        description += "死体から不浄で有毒なエッセンスを抽出する。 $"
                       "腐敗した死体からはより強力な毒物を抽出できるかもしれない。 $"
                       "$$この結果できる液体は、大抵は飲もうとは思えない代物だろう。 ";
#else
        description += "extracts the vile and poisonous essences from a "
            "corpse.  A rotten corpse may produce a stronger potion."
            "$$You probably don't want to drink the results. ";
#endif
        break;

/* ******************************************************************
// not implemented {dlb}:
    case SPELL_ERINGYAS_SURPRISING_BOUQUET:
      description += "transmutes any wooden items in the caster's grasp "
                     "into a bouquet of beautiful flowers. ";
      break;
****************************************************************** */

    case SPELL_FRAGMENTATION:
        description +=
#ifdef JP
            "岩もしくはその他の固い材質でできた物体の中から $"
            "衝撃性の爆発を発生させ、そばにいた者に損害を与える。 ";
#else
            "creates a concussive explosion within a large body of "
            "rock (or other hard material), to the detriment of "
            "any who happen to be standing nearby. ";
#endif
        break;

    case SPELL_AIR_WALK:
#ifdef JP
        description += "術者の体を実体のない雲に変化させる。 $"
                       "実体のなくなった術者は物理的な損傷に対しては $"
                       "ほぼ免疫となるが、魔法の炎と氷に対して傷つきやすくなる。 $"
                       "術者は実体がない間は、当然のごとく物体を持ち上げることが $"
                       "できない。ただし呪文を唱えることはできる。 ";
#else
        description += "transforms the caster's body into an insubstantial "
            "cloud. The caster becomes immaterial and nearly immune "
            "to physical harm, but is vulnerable to magical fire "
            "and ice. While insubstantial the caster is, of course, "
            "unable to interact with physical objects (but may still "
            "cast spells). ";
#endif
        break;

    case SPELL_SANDBLAST:
#ifdef JP
        description += "細かい粒子の混じった高速な突風を引き起こす。 $"
                       "例えば術者が小石を手に持つなど触媒を利用する時 $"
                       "この呪文は最もよく働く。しかし地面の砂だけを利用して $"
                       "呪文をかけることも可能である。 ";
#else
        description += "creates a short blast of high-velocity particles. "
            "It works best when the caster provides some source "
            "(by wielding a stone), but will do what it can with "
            "whatever ambient grit is available. ";
#endif
        break;

    case SPELL_ROTTING:
#ifdef JP
        description += "術者の近くにある全ての肉を腐敗させる。 $"
                       "これは生きている者と実体を持つアンデッドに $"
                       "作用する。 ";
#else
        description += "causes the flesh of all those near the caster to "
            "rot. It will affect the living and many of the "
            "corporeal undead. ";
#endif
        break;

    case SPELL_SHUGGOTH_SEED:
#ifdef JP
        description += "生きた生物を宿主にショゴスの種子を埋め込む。 $"
                       "これは恐るべきショゴスの幼生形態である寄生体だ。 $"
                       "ショゴスの種子は宿主から生命力を吸いとって孵化する。 $"
                       "そしてショゴスの成体は不運な宿主の胸を裂いて $"
                       "飛び出すだろう。 ";
#else
        description += "implants a shuggoth seed, the larval parasitic form "
            "of the fearsome shuggoth, in a living host. The "
            "shuggoth seed will draw life from its host and then "
            "hatch, whereupon a fully grown shuggoth will burst "
            "from the unfortunate host's chest. ";
#endif
        break;

    case SPELL_MAXWELLS_SILVER_HAMMER:
#ifdef JP
        description += "一定時間、術者の持った鈍器に致命的な重力場を付与する。 $"
                       "この呪文はすでに何らかの魔力を持った武器には効果を持たない。 ";
#else
        description += "bestows a lethal but temporary gravitic field "
            "to a crushing implement held by the caster. "
            "It will not affect weapons otherwise subject to "
            "special enchantments. ";
#endif
        break;

    case SPELL_CONDENSATION_SHIELD:
#ifdef JP
        description += "術者を取り巻く大気から圧縮した大気の円盤を作り出す $"
                       "これは通常の盾と同じように扱うことができるが、 $"
                       "その密度、つまり攻撃を止める力は氷の魔法の技能に依存する。 $"
                       "円盤は術者の精神でコントロールされるので、術者が $"
                       "両手持ちの武器を振るうことを妨げない。 ";
#else
        description += "causes a disc of dense vapour to condense out of the "
            "air surrounding the caster. It acts like a normal "
            "shield, but its density (and therefore stopping power) "
            "depends upon the caster's skill with Ice Magic. The "
            "disc is controlled by the caster's mind and thus will "
            "not conflict with the wielding of a two-handed weapon. ";
#endif
        break;

    case SPELL_STONESKIN:
#ifdef JP
        description += "術者の皮膚に大地の魔術の技能に応じた強靭さを与える。 $"
                       "この呪文は比較的正常な肉体にしか作用しない。 $"
                       "つまり、アンデッドも変身している者にも作用しない。 $"
                       "ただし、もしも術者が石像の形態を取っている場合には $ "
                       "この呪文の効果は強化される。 ";
#else
        description += "hardens the one's skin to a degree determined "
            "by one's skill in Earth Magic. This only works on relatively "
            "normal flesh; it will aid neither the undead nor the bodily "
            "transformed.  The effects of this spell are boosted if the "
            "caster is in Statue Form. ";
#endif
        break;

    case SPELL_SIMULACRUM:
#ifdef JP
        description += "手にしたオリジナルの肉片から、氷のレプリカを作り出す。 $"
                       "この呪文は不安定なので、レプリカはいずれ気体に $"
                       "昇華してしまう。それよりも先に斬り裂かれたり、 $"
                       "水溜りに溶けてしまったりしなければであるが。 $";
#else
        description += "uses a piece of a flesh in hand to create a replica "
                       "of the original being out of ice. This magic is "
                       "unstable so eventually the replica will sublimate "
                       "into a freezing cloud, if it isn't hacked or melted "
                       "into a small puddle of water first. ";
#endif
        break;

    case SPELL_CONJURE_BALL_LIGHTNING:
#ifdef JP
        description += "球雷を造り出す。 $"
                       "この呪文には危険がないとは言えない。 $"
                       "なぜなら、球雷の制御は非常に難しいのだ。 ";
#else
        description += "allows the conjurer to create ball lightning.  "
                        "Using the spell is not without risk - ball lighting "
                        "can be difficult to control. ";
#endif
        break;

    case SPELL_TWIST:
#ifdef JP
        description += "術者の視線の範囲に軽く空間の歪曲を起こし傷を負わせる。 ";
#else
        description += "causes a slight spatial distortion around a monster "
                       "in line of sight of the caster, causing injury. ";
#endif
        break;

    case SPELL_FAR_STRIKE:
#ifdef JP
        description += "視界内の敵に、術者が手にした武器の力で離れた地点から $"
                       "攻撃することを可能とする。 $"
                       "この魔法は目標に打撃力だけを伝達する。 $"
                       "武器や付与魔術の追加効果を伝達しない。 $"
                       "その威力は武器の技能や腕力、転位、魔法技能などからは $"
                       "ほとんど影響を受けない。 ";
#else
        description += "allows the caster to transfer the force of a "
                       "weapon strike to any target the caster can see.  "
                       "This spell will only deliver the impact of the blow; "
                       "magical side-effects and enchantments cannot be "
                       "transferred in this way.  The force transferred by "
                       "this spell has little to do with one's skill with "
                       "weapons, and more to do with personal strength, "
                       "translocation skill, and magic ability. ";
#endif
        break;

    case SPELL_SWAP:
#ifdef JP
        description += "術者が隣接したモンスターと位置交換を行うことが可能とする。 ";
#else
        description += "allows the caster to swap positions with an adjacent "
                       "being. ";
#endif
        break;

    case SPELL_APPORTATION:
#ifdef JP
        description += "術者が少し離れたアイテムの山から、一番上にある $"
                       "アイテムを引き寄せることを可能にする。 $"
                       "質量の大きなアイテムは呪文の成功を難しくする。 $"
                       "ある種のアイテムはこの呪文で動かすには重過ぎる。 $"
                       "この呪文をグループ化された(矢などの)アイテムに使うのは $"
                       "リスクを伴う。力の不足によってアイテムの一部が虚空に $"
                       "消える可能性があるからである。 ";
#else
        description += "allows the caster to pull the top item or group of "
                       "similar items from a distant pile to the floor "
                       "near the caster.  The mass of the target item(s) will "
                       "make the task more difficult, with some items too "
                       "massive to ever be moved by this spell.   Using this "
                       "spell on a group of items can be risky;  insufficient "
                       "power will cause some of the items to be lost in the "
                       "infinite void.";
#endif
        break;

    default:
#ifdef JP
        DEBUGSTR("間違った呪文");
        description += "存在しない呪文のようだ。 $ "
                       "従って、これを唱えるのは賢明ではあるまい。 "
#if DEBUG
            "というか、修正したまえよ。 ";
#else
            "地下迷宮技術保安部に "
            "詳細を報告して頂けると幸いである。 ";
#endif // DEBUG
#else
        DEBUGSTR("Bad spell");
        description += "apparently does not exist. "
            "Casting it may therefore be unwise. "
#if DEBUG
            "Instead, go fix it. ";
#else
            "Please contact Dungeon Tech Support "
            "at /dev/null for details. ";
#endif // DEBUG
#endif
    }

    print_description(description);

    set_keyin_mode(KEYIN_MODE_MORE);
    if (getch() == 0)
        getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM
    puttext(25, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}                               // end describe_spell()


//---------------------------------------------------------------
//
// describe_monsters
//
// Contains sketchy descriptions of every monster in the game.
//
//---------------------------------------------------------------
void describe_monsters(int class_described, unsigned char which_mons)
{
    std::string description;

    description.reserve(200);

#ifdef DOS_TERM
    char buffer[3400];

    gettext(25, 1, 80, 25, buffer);
    window(25, 1, 80, 25);
#endif

    clrscr();
    description = std::string( ptr_monam( &(menv[ which_mons ]), DESC_CAP_A ) );
    description += "$$";

    switch (class_described)
    {
        // (missing) case 423 - MONS_ANOTHER_LAVA_THING ??? 15jan2000 {dlb}
        //                      no entry in m_list.h 17jan200 {dlb}
        //          monster has no stats!
        // mv: changed ANOTHER_LAVA_THING to SALAMANDER, added stats and
        //     description
        // (missing) case 250 - MONS_PROGRAM_BUG ??? 16jan2000 {dlb}
    case MONS_KILLER_BEE_LARVA:
#ifdef JP
        description += "小さくて無力な殺人蜂の幼虫だ。 ";
#else
        description += "A small, powerless larva of killer bee.";
#endif
        break;

    case MONS_QUASIT:
#ifdef JP
        description += "長くて鋭利な尾の先を持つ、不気味なデーモンだ。 ";
#else
        description += "A small twisted demon with long sharply pointed tail.";
#endif
        break;

    case MONS_ANGEL:
#ifdef JP
        description += "超自然的な美しさをもつ有翼の聖なる存在だ。 $"
                       "煌く金色の霊気に包まれている。 ";
#else
        description += "A winged holy being of unnatural beauty. "
            "It's surrounded by aura of brilliant golden light. ";
#endif
        break;

    case MONS_HUMAN:
        // These should only be possible from polymorphing or shapeshifting.
#ifdef JP
        description += "全く特徴のない人物。むしろ奇妙だ！ ";
#else
        description += "A remarkably nondescript person.  How odd!";
#endif
        break;

    case MONS_GIANT_ANT:
#ifdef JP
        description += "有毒な大顎をもつ、大型犬ほどの大きさの黒い蟻だ。 ";
#else
        description += "A black ant with poisonous pincers,"
            " about the size of a large dog.";
#endif
        break;

    case MONS_SOLDIER_ANT:
#ifdef JP
        description += "大きな下顎と邪悪な針を持つ巨大な蟻だ。 ";
#else
        description += "A giant ant with large mandibles and a vicious sting.";
#endif
        break;

    case MONS_QUEEN_ANT:
#ifdef JP
        description += "厚いキチン質の鎧に覆われた、膨れ上がった昆虫だ $"
                       "今やあなたは全ての蟻がどこから来るかを知った！ ";
#else
        description += "A bloated insect, covered in thick chitinous armour."
            "Now you know where all those ants keep coming from!";
#endif
        break;

    case MONS_ANT_LARVA:
#ifdef JP
        description += "蟻の赤ん坊だ。可愛くないだろうか？ ";
#else
        description += "A baby ant. Isn't it cute?";
#endif
        break;

    case MONS_GIANT_BAT:
#ifdef JP
        description += "巨大な黒いコウモリだ。 ";
#else
        description += "A huge black bat.";
#endif
        break;

    case MONS_CENTAUR:
    case MONS_CENTAUR_WARRIOR:
#ifdef JP
        description += "大きな馬の体の上に人間の胴体がついた混成種だ。 ";
#else
        description += "A hybrid with the torso of a "
            "human atop the body of a large horse. ";
#endif
        if (class_described == MONS_CENTAUR_WARRIOR)
#ifdef JP
            description += "$これは力強く攻撃的に見える。 " ;
#else
            description += "It looks strong and aggressive. ";
#endif
        break;

    case MONS_YAKTAUR:
    case MONS_YAKTAUR_CAPTAIN:
#ifdef JP
        description += "セントールに似ているが、その下半身はヤク牛だ。 ";
#else
        description += "Like a centaur, but half yak. ";
#endif
        if (class_described == MONS_YAKTAUR_CAPTAIN)
#ifdef JP
            description += "$これは非常に力強く攻撃的に見える。 ";
#else
            description += "It looks very strong and aggressive. ";
#endif
        break;

    case MONS_RED_DEVIL:
#ifdef JP
        description += "紅の悪魔は人間よりやや背が低いが、 $"
                       "筋骨たくましく刺と角で覆われている。 $"
                       "背中には一対の小さな翼が生えている。 ";
#else
        description += "The Red Devil is slightly shorter than a human, "
            "but muscular and covered in spikes and horns. Two "
            "short wings sprout from its shoulders.";
#endif
        break;

    case MONS_ROTTING_DEVIL:
#ifdef JP
        description += "激しく腐敗している人の形をした存在だ。 ";
#else
        description += "A hideous decaying form.";
#endif
        if (you.species == SP_GHOUL)
#ifdef JP
            description += "$とてもうまそうな匂いがする！ ";
#else
            description += "$It smells great!";
#endif
        else if (you.species != SP_MUMMY)
#ifdef JP
            description += "$これは悪臭を放っている。 ";
#else
            description += "$It stinks.";
#endif
        break;

    case MONS_HAIRY_DEVIL:
#ifdef JP
        description +=
            "茶色の毛に覆われた小さな人間型のデーモンだ。 $"
            "注意せよ:これは蚤を持っているかもしれない。 ";
#else
        description += "A small humanoid demon covered in brown hair. "
            "Watch out - it may have fleas!";
#endif
        break;

    case MONS_ICE_DEVIL:
#ifdef JP
        description += "光り輝く氷に覆われた人間サイズのデーモンだ。 ";
#else
        description += "A man-sized demon covered in glittering ice.";
#endif
        break;

    case MONS_BLUE_DEVIL:
#ifdef JP
        description += "奇怪でおぞましい蒼い存在だ。これは低温に見える。 ";
#else
        description += "A strange and nasty blue thing. It looks cold.";
#endif
        break;

    case MONS_IRON_DEVIL:
#ifdef JP
        description += "金属の皮膚をした見るもおぞましい人間型の存在だ。 ";
#else
        description += "A hideous humanoid figure with metal skin.";
#endif
        break;

    case MONS_ETTIN:
#ifdef JP
        description +=
            "巨大な双頭の人間型生物だ。 $"
            "二つの武器を振り回している姿が頻繁に見られる。 $"
            "二つの頭部で言い争いが起きないようにするためだろう。 ";
#else
        description += "A large, two headed humanoid. Most often seen "
            "wielding two weapons, so that the heads will have one less "
            "thing to bicker about.";
#endif
        break;

    case MONS_FUNGUS:
#ifdef JP
        description +=
            "塊になった灰色の菌類だ。 $"
            "暗く湿った地下迷宮で活発に生育する。 ";
#else
        description += "A lumpy grey fungus, "
            "growing well in the dank underground dungeon.";
#endif
        break;

    case MONS_GOBLIN:
#ifdef JP
        description += "チビで醜くて非友好的な人間型生物だ。 ";
#else
        description += "A race of short, ugly and unfriendly humanoids.";
#endif
        break;

    case MONS_HOUND:
#ifdef JP
        description += "恐ろしい猟犬だ。 ";
#else
        description += "A fearsome hunting dog.";
#endif
        break;

    case MONS_HELL_HOUND:
#ifdef JP
        description += "赤く輝く目をして牙の生えた口から煙を放つ、巨大な黒犬だ。 ";
#else
        description += "A huge black dog, with glowing red eyes and "
            "smoke pouring from its fanged mouth.";
#endif
        break;

    case MONS_WAR_DOG:
#ifdef JP
        description +=
            "殺しの訓練を受けた獰猛な犬だ。 $"
            "その喉はスパイク付の強固な首輪に守られている。 ";
#else
        description += "A vicious dog, trained to kill."
            "Its neck is protected by massive spiked collar.";
#endif
        break;

    case MONS_IMP:
#ifdef JP
        description += "小さくて醜い下級デーモンだ。 ";
#else
        description += "A small, ugly minor demon.";
#endif
        break;

    case MONS_JACKAL:
#ifdef JP
        description +=
            "小振りな犬型の腐食動物だ。 $"
            "この生き物の群れは、ダンジョンの中を死肉を貪るために徘徊する。 ";
#else
        description += "A small, dog-like scavenger. Packs of these creatures "
            "roam the underworld, searching for carrion to devour.";
#endif
        break;

    case MONS_KILLER_BEE:
#ifdef JP
        description +=
            "巨大な蜂だ。繰り返し刺すことができる致命的な毒針を持っている。 ";
#else
        description += "A giant bee, bearing a deadly barb which can sting "
            "repeatedly.";
#endif
        break;

    case MONS_QUEEN_BEE:
#ifdef JP
        description +=
            "その子供である殺人蜂よりも更に大きく、より恐ろしげな外見をしている。 $"
            "今、このモンスターはあなたを欲して巣穴から這い出して来た！ ";
#else
        description += "Even larger and more dangerous-looking than its "
            "offspring, this creature wants you out of its hive. Now!";
#endif
        break;

    case MONS_BUMBLEBEE:
#ifdef JP
        description += "非常に大きく丸々した毛深い蜂だ。 ";
#else
        description += "A very large and fat hairy bee.";
#endif
        break;

    case MONS_MANTICORE:
#ifdef JP
        description +=
            "ひどく醜い交雑生物だ。人間の頭と、ライオンの銅と、 $"
            "とても大きな蝙蝠の羽とを持つ。 $"
            "その尾には刺が密生しており、獲物を狙って発射することができる。 ";
#else
        description += "A hideous cross-breed, bearing the features of a "
            "human and a lion, with great bat-like wings. Its tail "
            "bristles with spikes, which can be loosed at potential prey.";
#endif
        break;

    case MONS_NECROPHAGE:
#ifdef JP
        description += "最も邪悪で不浄なアンデッド被造物で、 $"
                       "腐敗した人間型生物の死体から作成される。 $"
                       "彼らは疫病と腐敗を撒き散らしすために存在し、 $"
                       "他の生き物の腐乱死体から活力を得る。 ";
#else
        description += "A vile undead creation of the most unholy necromancy,"
            " these creatures are made from the decaying corpses "
            "of humanoid creatures.  They exist to spread disease "
            "and decay, and gain power from the decaying corpses "
            "of other beings.";
#endif
        break;

    case MONS_GHOUL:
#ifdef JP
        description +=
            "死霊術の邪悪な魔法によって腐乱死体から作られた、 $"
            "人間型のアンデッドモンスターだ。 $"
            "彼らは疫病と腐敗を撒き散らしすために存在し、 $"
            "屍肉喰らいと同じく他の生き物の腐乱死体から活力を得る。 ";
#else
        description += "An undead humanoid creature created from the decaying "
            "corpse by some unholy means of necromancy. It "
            "exists to spread disease and decay, and gains power"
            "from the decaying corpses same way as necrophage does.";
#endif
        break;

    case MONS_ORC:
#ifdef JP
        description +=
            "オークは醜い地下の種族であり、豚と人間、そして $"
            "その他不快な生物との最悪の結合物である。 ";
#else
        description += "An ugly subterranean race, orcs combine the"
            " worst features of humans, pigs, and several"
            " other unpleasant creatures.";
#endif
        break;

    case MONS_ORC_KNIGHT:
#ifdef JP
        description +=
            "重装備をしたオークだ。過去の多くの戦いによる古傷に覆われている。 $";
#else
        description += "A heavily armoured orc, covered in scars from many "
            "past battles.";
#endif
        break;

    case MONS_ORC_PRIEST:
#ifdef JP
        description +=
            "オークの崇める旧く残忍な神のしもべだ。長いローブを着ている。 $"
            "彼は奇怪な祈りを呟いている。 $"
            "その祈りが届かないように願うべきだろう。 ";
#else
        description += "A servant of the ancient and cruel gods of the orcs,"
            " dressed in long robe. he's mumbling some strange prayers. "
            "Hope that they will remain unheard.";
#endif
        break;

    case MONS_ORC_HIGH_PRIEST:
#ifdef JP
        description += "オークの崇める神の、高位のしもべだ。 ";
#else
        description += "An exalted servant of the orc god.";
#endif
        break;

    case MONS_ORC_SORCERER:
#ifdef JP
        description += "地獄から引き出した魔力を操るオークだ。 ";
#else
        description += "An orc who draws magical power from Hell.";
#endif
        break;

    case MONS_ORC_WARLORD:
#ifdef JP
        description += "非常に大柄で強そうな外見のオークだ。 ";
#else
        description += "A very large and strong looking orc.";
#endif
        break;

    case MONS_ORC_WARRIOR:
#ifdef JP
        description +=
            "武装したオークだ。明らかに他の者をブツ切りにしてしまう方法に $"
            "熟練している。 ";
#else
        description += "An armoured orc, obviously experienced in the ways of "
            "hacking other creatures apart.";
#endif
        break;

    case MONS_ORC_WIZARD:
#ifdef JP
        description +=
            "オークは一般的には極めて低能だが、時折は魔法の能力を $"
            "発達させる者もいる。 ";
#else
        description += "While orcs are generally quite stupid, occasionally"
            " one develops an aptitude for magic.";
#endif
        break;

    case MONS_PHANTOM:
#ifdef JP
        description += "半透明な人間らしき形をした精神体のアンデッドだ。 ";
#else
        description += "A transparent man-like undead spirit.";
#endif
        break;

    case MONS_RAT:
#ifdef JP
        description +=
            "過酷なダンジョンの環境で大きく、攻撃的になったネズミだ。 ";
#else
        description += "Rats which have grown large and aggressive in "
            "the pestilential dungeon environment.";
#endif
        break;

    case MONS_GREY_RAT:
#ifdef JP
        description += "非常に大きな灰色のネズミだ。 ";
#else
        description += "A very large grey rat.";
#endif
        break;

    case MONS_GREEN_RAT:
#ifdef JP
        description +=
            "非常に大きなネズミだ。 $"
            "この上なく奇妙な緑色の毛皮をしている。 ";
#else
        description += "A very large rat, with hair and skin of a "
            "most peculiar green colour.";
#endif
        break;

    case MONS_ORANGE_RAT:
#ifdef JP
        description +=
            "巨大なネズミだ。不気味な瘤だらけの皮はオレンジ色をしており、 $"
            "邪悪なエネルギーに輝いている。 ";
#else
        description += "A huge rat, with weird knobbly orange skin."
            "It glows with unholy energies. ";
#endif
        break;

    case MONS_SCORPION:
#ifdef JP
        description +=
            "巨大な黒いサソリだ。その体は厚い殻の鎧で覆われ、 $"
            "その尾の先には危険な毒を分泌する針がついている。 ";
#else
        description += "A giant black scorpion, its body covered in thick"
            " armour plating, and its tail tipped by a nasty "
            "venomous sting.";
#endif
        break;

/* ******************************************************************
// the tunneling worm is no more ...
// not until it can be re-implemented safely {dlb}
    case MONS_TUNNELING_WORM:
    case MONS_WORM_TAIL:
        description += "A gargantuan worm, its huge maw capable of crushing rock into dust with little trouble.";
        break;
****************************************************************** */

    case MONS_BRAIN_WORM:
#ifdef JP
        description += "ねばねばした藤色のワームだ。頭部がとてつもなく膨らんでいる。 ";
#else
        description += "A slimy mauve worm with a greatly distended head.";
#endif
        break;

    case MONS_LAVA_WORM:
#ifdef JP
        description += "溶岩の中を泳ぐ危険な赤いワームだ。 ";
#else
        description += "A vicious red worm which swims through molten rock.";
#endif
        break;

    case MONS_SPINY_WORM:
#ifdef JP
        description +=
            "大きな黒いワームだ。たくさんの節がある胴体は刺の生えたキチン質の $"
            "装甲によって覆われている。 $"
            "鋭い歯の並んだ口からは酸性の毒液を滴らせている。 ";
#else
        description += "A great black worm, its many-segmented body covered "
            "in spiky plates of chitinous armour. Acidic venom drips "
            "from its toothy maw.";
#endif
        break;

    case MONS_SWAMP_WORM:
#ifdef JP
        description +=
            "大きなぬるぬるしたワームだ。 $"
            "この不浄な沼の泥の中を泳ぐのに特化している。 ";
#else
        description += "A large slimy worm, adept at swimming through the "
            "muck of this foul swamp.";
#endif
        break;

    case MONS_WORM:
#ifdef JP
        description += "巨大なワームだ。異常なまでに大きな歯が生えている。 ";
#else
        description += "A giant worm, with unusually large teeth.";
#endif
        break;

    case MONS_UGLY_THING:
#ifdef JP
        description += "醜い存在だ。胸が悪くなる。 ";
#else
        description += "An ugly thing. Yuck.";
#endif
        break;

    case MONS_VERY_UGLY_THING:
#ifdef JP
        description += "非常に醜い存在だ。とてつもなく胸が悪くなる。 ";
#else
        description += "A very ugly thing. Double yuck.";
#endif
        break;

    case MONS_FIRE_VORTEX:
#ifdef JP
        description += "渦巻く炎の雲だ。 ";
#else
        description += "A swirling cloud of flame.";
#endif
        break;

    case MONS_SPATIAL_VORTEX:
#ifdef JP
        description += "狂ったように転位していく現実構造の歪みだ。 ";
#else
        description += "A crazily shifting twist in the fabric of reality.";
#endif
        break;

    case MONS_ABOMINATION_SMALL:
        description +=
#ifdef JP
            "見るもおぞましい姿をしたものだ。秘術によって創造または召換された。 ";
#else
            "A hideous form, created or summoned by some arcane process.";
#endif
        break;

    case MONS_ABOMINATION_LARGE:
#ifdef JP
        description +=
            "巨大で見るもおぞましい姿をしたものだ。 $"
            "秘術によって創造または召換された。 ";
#else
        description += "A huge and hideous form, created or summoned "
            "by some arcane process.";
#endif
        break;

    case MONS_YELLOW_WASP:
#ifdef JP
        description +=
            "巨大なススメバチだ。 $"
            "厚いキチン質でできた黄色い装甲に覆われている。 ";
#else
        description += "A giant wasp covered with thick plates of yellow "
            "chitinous armour.";
#endif
        break;

    case MONS_RED_WASP:
#ifdef JP
        description += "巨大な赤いススメバチだ。邪悪なかかりのついた毒針を持つ。 ";
#else
        description += "A huge red wasp with a viciously barbed stinger.";
#endif
        break;

    case MONS_ZOMBIE_SMALL:
#ifdef JP
        description += "死霊術によって甦った死体だ。 ";
#else
        description += "A corpse raised to undeath by necromancy. ";
#endif
        break;
    case MONS_ZOMBIE_LARGE:
#ifdef JP
        description += "死霊術によって甦った大きな死体だ。 ";
#else
        description += "A large corpse raised to undeath by necromancy. ";
#endif
        break;

    case MONS_SIMULACRUM_LARGE:
    case MONS_SIMULACRUM_SMALL:
#ifdef JP
        description +=
            "モンスターの氷による複製だ。 $"
            "死霊術の力で操られている。 ";
#else
        description += "An ice replica of a monster, that's animated by "
            "the powers of necromancy. ";
#endif
        break;

    case MONS_CYCLOPS:
#ifdef JP
        description +=
            "額の中央に一つだけ目を持つ巨人だ。 $"
            "両眼による立体視の欠如にも関らず、 $"
            "サイクロプスは恐るべき正確さで巨石を投げつける。 ";
#else
        description += "A giant with one eye in the centre of its forehead."
            " Despite their lack of binocular vision, cyclopes "
            "throw boulders with fearsomely accuracy.";
#endif
        break;

    case MONS_DRAGON:
#ifdef JP
        description +=
            "巨大な爬虫類の怪獣だ。 $"
            "全身が分厚い緑の鱗に覆われ、一対のコウモリに似た翼を持つ。 $"
            "牙の並んだ顎からはかすかな煙がたなびいている。 ";
#else
        description += "A great reptilian beast, covered in thick green "
            "scales and with two huge bat-like wings. Little trails "
            "of smoke spill from its toothy maw.";
#endif
        break;

    case MONS_GOLDEN_DRAGON:
#ifdef JP
        description += "輝く黄金の鱗に覆われた巨大なドラゴンだ。 ";
#else
        description += "A great dragon covered in shining golden scales. ";
#endif
        break;

    case MONS_ICE_DRAGON:
        description +=
#ifdef JP
            "通常のドラゴンに似ているが、全身は白くて寒気を纏っている。 ";
#else
            "Like a normal dragon, only white and covered in frost.";
#endif
        break;

    case MONS_IRON_DRAGON:
#ifdef JP
        description += "非常な重量のため、明らかに飛行能力がないドラゴンだ。 ";
#else
        description += "A very heavy and apparently flightless dragon.";
#endif
        break;

    case MONS_MOTTLED_DRAGON:
#ifdef JP
        description += "奇妙な斑紋の鱗を持つ、小型のドラゴンだ。 ";
#else
        description += "A small dragon with strangely mottled scales.";
#endif
        break;

    case MONS_QUICKSILVER_DRAGON:
#ifdef JP
        description +=
            "長い体を曲がりくねらせたドラゴンだ。 $"
            "蛇に似た姿をしている。 $"
            "皮膚は水銀のように輝き、鋭い鼻先は魔力の放電を放っている。 ";
#else
        description += "A long and sinuous dragon, seemingly more neck and "
            "tail than anything else. Its skin shines like molten mercury, "
            "and magical energies arc from its pointed snout.";
#endif
        break;

    case MONS_SHADOW_DRAGON:
#ifdef JP
        description += "巨大な影のような姿をしている。邪悪と死をの気配を放っている。 ";
#else
        description += "A great shadowy shape, radiating evil and death.";
#endif
        break;

    case MONS_SKELETAL_DRAGON:
#ifdef JP
        description +=
            "忌まわしい巨大なアンデッドだ。 $"
            "たくさんのドラゴンの砕けた骨を継ぎ合わせて造られている。 ";
#else
        description += "A huge undead abomination, pieced together from "
            "the broken bones of many dragons.";
#endif
        break;

    case MONS_STEAM_DRAGON:
#ifdef JP
        description +=
            "比較的小型のドラゴンだ。 $"
            "その顎からは蒸気が立ち昇っている。 ";
#else
        description += "A relatively small grey dragon, with steam pouring "
            "from its mouth.";
#endif
        break;

    case MONS_STORM_DRAGON:
#ifdef JP
        description +=
            "巨大で強力なドラゴンだ。 $"
            "その巨大な翼に沿って放電の火花が音を立てている。 ";
#else
        description += "A huge and very powerful dragon. "
            "Sparks crackle along its enormous scaly wings.";
#endif
        break;

    case MONS_SWAMP_DRAGON:
#ifdef JP
        description +=
            "沼の泥に覆われた、ぬめぬめしたドラゴンだ。 $"
            "有毒なガスがその鼻先から流れ出ている。 ";
#else
        description += "A slimy dragon, covered in swamp muck. "
            "Poisonous gasses dribble from its snout.";
#endif
        break;

    case MONS_SERPENT_OF_HELL:
#ifdef JP
        description += "地獄の業火に燃えて赤く輝く、巨大なドラゴンだ。 ";
#else
        description += "A huge red glowing dragon, burning with hellfire. ";
#endif
        break;

    case MONS_SWAMP_DRAKE:
#ifdef JP
        description += "沼の泥に覆われた、ぬめぬめした小さなドラゴンだ。 $";
#else
        description += "A small and slimy dragon, covered in swamp muck. ";
#endif
        if (you.species != SP_MUMMY)
#ifdef JP
            description += "凄まじい匂いを放っている。 ";
#else
            description += "It smells horrible.";
#endif
        break;

    case MONS_FIREDRAKE:
#ifdef JP
        description += "小型のドラゴンだ。煙を吹いている。 ";
#else
        description += "A small dragon, puffing clouds of smoke.";
#endif
        break;

    case MONS_TWO_HEADED_OGRE:
#ifdef JP
        description +=
            "肥大した体に二つの頭を持つ、巨大なオーガだ。 $"
            "大きな両方の手に、それぞれ武器を持つことができる。 ";
#else
        description += "A huge ogre with two heads on top of a "
            "bloated ogre body. It is capable of holding a weapon "
            "in each giant hand.";
#endif
        break;

    case MONS_FIEND:
#ifdef JP
        description +=
            "地獄に棲むものの中でも最も恐ろしい存在の一つだ。 $"
            "地獄の炎に包まれた巨大で強力なデーモンである。 $"
            "鱗の生えた大きな翼を持っている。 ";
#else
        description += "One of the most fearsome denizens of any Hell. "
            "A huge and powerful demon wreathed in hellfire,"
            " with great scaly wings.";
#endif
        break;

    case MONS_ICE_FIEND:
#ifdef JP
        description +=
            "地獄に棲むものの中でも最も恐ろしい存在の一つだ。 $"
            "氷の悪魔は巨大な氷の体を持ち、氷と凍気を身に纏っている。 ";
#else
        description += "One of the most terrible denizens of the "
            "many Hells, the Ice Fiend is a huge icy figure, "
            "covered in frost and wreathed in freezing air.";
#endif
        break;

    case MONS_SHADOW_FIEND:
#ifdef JP
        description +=
            "地獄に棲むものの中でも最も恐ろしい存在の一つだ。 $"
            "このおぞましい存在は巨大な蠢く影として出現し、 $"
            "ごくたまに角のある骸骨の姿が剥き出しになる。 ";
#else
        description += "One of the most terrible denizens of the many Hells, "
            "this horrible being appears as a great mass of "
            "writhing shadows which occasionally reveal a huge, "
            "horned skeleton.";
#endif
        break;

    case MONS_GIANT_SPORE:
#ifdef JP
        description +=
            "宙に浮いた胞子の球で、爆発しやすい。 $"
            "瘤だらけの茎に覆われている。 ";
#else
        description += "A volatile floating ball of spores, "
            "covered in knobbly rhizome growths.";
#endif
        break;

    case MONS_HOBGOBLIN:
#ifdef JP
        description += "ゴブリンの近縁種で、より大きくて強い。 ";
#else
        description += "A larger and stronger relatives of goblins.";
#endif
        break;

    case MONS_ICE_BEAST:
        description +=
#ifdef JP
            "恐ろしいモンスターだ。雪と透き通る氷でできた体を持つ。 $"
            "その歩みは地面に冷たい水溜りを残していく。 ";
#else
            "A terrible creature, formed of snow and crystalline ice. "
            "Its feet leave puddles of icy water on the floor.";
#endif
        break;

    case MONS_KOBOLD:
#ifdef JP
        description +=
            "コボルドはゴブリンに似た小さなモンスターで、犬の頭を持つ。 $"
            "話によると、古代の魔神の被造物であるという。 ";
#else
        description += "Reputedly the creation of an ancient demon-god, "
            "kobolds are small goblin-like creatures with canine heads.";
#endif
        break;

    case MONS_BIG_KOBOLD:
#ifdef JP
        description += "異常に大きなコボルドだ。 ";
#else
        description += "An unusually large kobold.";
#endif
        break;

    case MONS_KOBOLD_DEMONOLOGIST:
#ifdef JP
        description += "悪魔を召換し、指揮することを覚えたコボルドだ。 ";
#else
        description += "A kobold who has learned to summon and direct demons.";
#endif
        break;

    case MONS_LICH:
        description +=
#ifdef JP
            "死を拒絶した魔法使いだ。リッチは骸骨の乾いた死体であり、 $"
            "死霊術の強力な作用で紛いものの生命を保っている。 $"
            "このアンデッドモンスターは強力な魔法の遣い手であり、 $"
            "よほどの自信がない限り対峙することは避けるのが最良だ。 ";
#else
            "A wizard who didn't want to die, a Lich is a skeletal,"
            " desiccated corpse kept alive by a mighty exercise of "
            "necromancy.  These undead creatures can wield great "
            "magic and are best avoided by all but the most confident.";
#endif
        break;

    case MONS_ANCIENT_LICH:
#ifdef JP
        description += "数え切れない年月を経て、更に強力になったリッチだ。 ";
#else
        description += "A lich who has grown mighty over countless years. ";
#endif
        break;

    case MONS_MUMMY:
#ifdef JP
        description +=
            "防腐処理を施され包帯に覆われた体を持つアンデッドだ。 $"
            "古代の魔術によって使役されている。 $"
            "その領土を侵害した者に対し、有害な霊気を放射する。 ";
#else
        description += "An undead figure covered in "
            "bandages and embalming fluids, "
            "compelled to walk by an ancient curse. "
            "It radiates a malign aura to those who intrude on its domain. ";
#endif
        break;

    case MONS_GUARDIAN_MUMMY:
#ifdef JP
        description +=
            "古代の戦士だ。死体に防腐処理を施され、 $"
            "呪術によって永遠に死ぬこともなく使役されている。 ";
#else
        description += "An ancient warrior, embalmed "
            "and cursed to walk in undeath for eternity.";
#endif
        break;

    case MONS_GREATER_MUMMY:
    case MONS_MUMMY_PRIEST:
#ifdef JP
        description += "防腐処理を施されたアンデッドだ。 $";
#else
        description += "The embalmed and undead corpse of an ancient ";
#endif
        if (class_described == MONS_GREATER_MUMMY)
#ifdef JP
            description += "この死体はかつては古代の君主だった。 ";
#else
            description += "ruler";
#endif
        else
#ifdef JP
            description += "この死体はかつては闇のしもべだった。 ";
#else
            description += "servant of darkness";
#endif
#ifdef JP
        //description += ".";
#else
        description += ".";
#endif
        break;

    case MONS_NAGA:
    case MONS_NAGA_MAGE:
    case MONS_NAGA_WARRIOR:
    case MONS_GUARDIAN_NAGA:
    case MONS_GREATER_NAGA:
        if (you.species == SP_NAGA)
#ifdef JP
            description = "魅力的な";
#else
            description = "An attractive";
#endif
        else
#ifdef JP
            description = "奇怪な";
#else
            description = "A strange";
#endif

#ifdef JP
        description +=
            "混成体だ。 $"
            "胸から上は人間だが、たくましい胴体は鱗のある蛇の $"
            "体に繋がっている。 ";
#else
        description += " hybrid; human from the chest up,"
            " with a scaly, muscular torso trailing off like "
            " that of a snake.  ";
#endif

        switch (class_described)
        {
        case MONS_GUARDIAN_NAGA:
#ifdef JP
            description +=
                "この種のナーガはしばしば力ある者によって $"
                "番人として用いられている。 ";
#else
            description += "These nagas are "
                "often used as guardians by powerful creatures.";
#endif
            break;
        case MONS_GREATER_NAGA:
#ifdef JP
            description += "力強く獰猛な様子だ。 ";
#else
            description += "It looks strong and aggressive.";
#endif
            break;
        case MONS_NAGA_MAGE:
#ifdef JP
            description += "薄気味の悪い後光を纏っている。 ";
#else
            description += "An eldritch nimbus trails its motions. ";
#endif
            break;
        case MONS_NAGA_WARRIOR:
#ifdef JP
            description += "過去の多くの戦いによる古傷を帯びている。 ";
#else
            description += "It bears scars of many past battles. ";
#endif
            break;
        }
        break;

    case MONS_OGRE:
#ifdef JP
        description +=
            "オークやゴブリンの遠縁だが、遥かに大きく、遥かに醜悪で、 $"
            "遥かに太っている。 ";
#else
        description += "A larger, uglier and fatter relative "
            "of orcs and goblins.";
#endif
        break;

    case MONS_OGRE_MAGE:
#ifdef JP
        description += "稀少な血統のオーガで、魔法の使用に長けている。 ";
#else
        description += "A rare breed of ogre, skilled in the use of magic.";
#endif
        break;

    case MONS_PLANT:
#ifdef JP
        description +=
            "ほとんどの植物はダンジョンの不快な環境では生育しないが、 $"
            "ある品種は太陽光の欠如した地下に順応し、繁栄さえしている。 ";
#else
        description += "Few plants can grow in the unpleasant dungeon "
            "environment, but some have managed to adapt and even thrive "
            "underground in the absence of the sun.";
#endif
        break;

    case MONS_OKLOB_PLANT:
#ifdef JP
        description += "硫酸を滴らせる危険な植物だ。 ";
#else
        description += "A vicious plant, dripping with vitriol.";
#endif
        break;

    case MONS_RAKSHASA:
    case MONS_RAKSHASA_FAKE:
#ifdef JP
        description +=
            "力と知識を求めて物質界に訪れた、デーモンの一種だ。 $"
            "ラクシャーサは幻影を操る技にとりわけ秀でている。 ";
#else
        description += "A type of demon who comes to the material world in "
            "search of power and knowledge. Rakshasas are experts"
            " in the art of illusion, among other things.";
#endif
        break;

    case MONS_SNAKE:
#ifdef JP
        description += "ダンジョンに生息する蛇としては一般的なものだ。 ";
#else
        description += "The common dungeon snake. ";
#endif
        break;

    case MONS_BLACK_SNAKE:
#ifdef JP
        description += "大きな黒い蛇だ。 ";
#else
        description += "A large black snake. ";
#endif
        break;

    case MONS_BROWN_SNAKE:
#ifdef JP
        description += "大きな茶色の蛇だ。 ";
#else
        description += "A large brown snake.";
#endif
        break;

    case MONS_GREY_SNAKE:
#ifdef JP
        description += "非常な大きさの灰色の大蛇だ。 ";
#else
        description += "A very large grey python.";
#endif
        break;

    case MONS_LAVA_SNAKE:
#ifdef JP
        description +=
            "灼熱する赤い蛇だ。溶岩の池から鎌首をもたげて $"
            "あなたに噛みつこうとしている。 ";
#else
        description += "A burning red snake which rears up from pools "
            "of lava and tries to bite you.";
#endif
        break;

    case MONS_SMALL_SNAKE:
#ifdef JP
        description += "ダンジョンに生息する小さな蛇だ。 ";
#else
        description += "The lesser dungeon snake.";
#endif
        break;

    case MONS_YELLOW_SNAKE:
#ifdef JP
        description += "大きな黄色の蛇族の爬虫類だ。 ";
#else
        description += "A large yellow tubular reptile.";
#endif
        break;

    case MONS_GIANT_NEWT:
#ifdef JP
        description +=
            "通常のイモリの数倍のサイズがある。 $"
            "しかし目を瞠るほどに大きいとは言えない。 ";
#else
        description += "Several times the size of a normal newt, but still "
            "not really impressive.";
#endif
        break;

    case MONS_GIANT_GECKO:
#ifdef JP
        description +=
            "足指の先に吸盤を持ち、壁や天井を上ることができる $"
            "トカゲだ。 ";
#else
        description += "A lizard with pads on its toes allowing it to cling "
            "to walls and ceilings.  It's much larger than a normal gecko... "
            "perhaps it's something in the water?";
#endif
        break;

    case MONS_GIANT_IGUANA:
    case MONS_GIANT_LIZARD:
#ifdef JP
        description += "噛み砕く力の強い大顎を持った、巨大なトカゲだ。 ";
#else
        description += "A huge lizard with great crunching jaws.";
#endif
        break;

    case MONS_GILA_MONSTER:
#ifdef JP
        description +=
            "明るい色彩の縞や斑のある大トカゲだ。 ";
#else
        description += "A large lizard with brightly coloured stripes and "
            "splotches.";
#endif
        break;

    case MONS_KOMODO_DRAGON:
#ifdef JP
        description +=
            "非常に巨大なオオトカゲだ。 $"
            "このオオトカゲは大型動物など簡単に餌食にしてしまう。 $"
            "その歯には前回の餌食の悪臭を放つ腐った肉の欠片がくっついている。 ";
#else
        description += "An enormous monitor lizard.  It's more than capable "
            "of preying on large animals.  Bits of fetid and rotting flesh "
            "from its last few meals are stuck in its teeth.";
#endif
        break;

    case MONS_LINDWURM:
#ifdef JP
        description +=
            "力強い一対の前肢を持つ、蛇のような姿の小型ドラゴンだ。 $"
            "その厚い鱗は、不気味な緑色に発光している。 ";
#else
        description += "A small serpentine dragon with a pair of strong "
            "forelimbs.  Its thick scales give off an eerie green glow.";
#endif
        break;

    case MONS_TROLL:
        description +=
#ifdef JP
            "嫌悪を催す外見をした巨大なモンスターだ。その分厚くて $"
            "節くれ立った皮膚は、大抵の傷を即座に再生してしまう。 ";
#else
            "A huge, nasty-looking creature. Its thick and knobbly hide "
            "seems to heal almost instantly from most wounds.";
#endif
        break;

    case MONS_DEEP_TROLL:
#ifdef JP
        description += "背の曲がったトロルだ。 ";
#else
        description += "A stooped troll.";
#endif
        break;

    case MONS_IRON_TROLL:
        description +=
#ifdef JP
            "錆びた鉄の厚い鱗に覆われた、大きなトロルだ。 ";
#else
            "A great troll, plated with thick scales of rusty iron.";
#endif
        // you can't see its hide, but think it's thick and kobbly, too :P {dlb}
        //jmf: I thought its skin *was* the rusty iron. If so, ought to change
        //     shatter_monsters in spells4.cc.
        break;

    case MONS_ROCK_TROLL:
        description +=
#ifdef JP
            "嫌悪を催す外見をした非常に巨大な人間型のモンスターだ。 $"
            "その岩のような皮膚は、大抵の傷を即座に再生してしまう。 ";
#else
            "An enormous and very nasty-looking humanoid creature. Its "
            "rocky hide seems to heal almost instantaneously from most wounds.";
#endif
        break;

    case MONS_UNSEEN_HORROR:
        description +=
#ifdef JP
            "このモンスターは通常は目に見えない。 $"
            "そしてその姿を見てしまった者は後悔するだろう。 ";
#else
            "These creatures are usually unseen by the eyes of most,"
            " and those few who have seen them would rather not have.";
#endif
        break;

    case MONS_VAMPIRE:
#ifdef JP
        description += "強力なアンデッドだ。 $";
#else
        description += "A powerful undead.";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "彼はあなたの血を求めている！ ";
#else
            description += " It wants to drink your blood! ";
#endif
        break;

    case MONS_VAMPIRE_KNIGHT:
        description +=
#ifdef JP
            "強力な戦士だ。その技量はアンデッドとなっても健在だ。 ";
#else
            "A powerful warrior, with skills undiminished by undeath.";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "彼はあなたの血を求めている！ ";
#else
            description += " It wants to drink your blood! ";
#endif
        break;

    case MONS_VAMPIRE_MAGE:
#ifdef JP
        description += "強力な魔術師だ。その力はアンデッドとなっても健在だ。 ";
#else
        description += "Undeath has not lessened this powerful mage.";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "彼はあなたの血を求めている！ ";
#else
            description += " It wants to drink your blood! ";
#endif
        break;

    case MONS_WRAITH:
#ifdef JP
        description +=
            "精神体のアンデッドだ。実体のない骸骨の体に黒い霧を纏っている。 $"
            "その両眼は途方もない悪意に燃えあがっている。 ";
#else
        description += "This undead spirit appears as a cloud of black mist "
            "surrounding an insubstantial skeletal form. Its eyes "
            "burn bright with unholy malevolence.";
#endif
        break;

    case MONS_FREEZING_WRAITH:
#ifdef JP
        description +=
            "実体のない骸骨の体に凍気を纏っている。 ";
#else
        description += "A cloud of freezing air surrounding an incorporeal "
            "skeletal form.";
#endif
        break;

    case MONS_SHADOW_WRAITH:
#ifdef JP
        description +=
            "霧を纏った骸骨の影だ。宙に浮いている。 $"
            "このモンスターは強化された視力をもってしてもほぼ不可視だ。 ";
#else
        description += "A mist-wreathed skeletal shadow hanging in mid-air, "
            "this creature is almost invisible even to your enhanced sight. ";
#endif
        // assumes: to read this message, has see invis
        break;

    case MONS_YAK:
#ifdef JP
        description +=
            "ダンジョンに生息するヤクとしては普通のものだ。 $"
            "ぼさぼさした毛皮に覆われていて、一対の危険な角を持つ。 ";
#else
        description += "The common dungeon yak, covered in shaggy yak hair "
            "and bearing a nasty pair of yak horns.";
#endif
        break;

    case MONS_DEATH_YAK:
#ifdef JP
        description +=
            "普通のダンジョンヤクより大きくて逞しい近縁種だ。 $"
            "その小さな赤い目は、生き物の肉への飢えに輝いている。 ";
#else
        description += "A larger and beefier relative of the common dungeon "
            "yak. Its little red eyes gleam with hunger for living flesh.";
#endif
        break;

    case MONS_WYVERN:
#ifdef JP
        description +=
            "鋭い突起がついた長い尾を持つ、ドラゴンに似たモンスターだ。 $"
            "ワーバーンは本物の竜よりは小型で手強さも劣るが、とりもなおさず $"
            "侮り難い敵である。 ";
#else
        description += "A dragon-like creature with long sharply pointed tail."
            " Although smaller and less formidable than true dragons, "
            "wyverns are nonetheless a foe to be reckoned with.";
#endif
        break;

    case MONS_GIANT_EYEBALL:
#ifdef JP
        description += "呪縛する凝視を向けてくる、巨大な眼球だ。 ";
#else
        description += "A giant eyeball, with a captivating stare.";
#endif
        break;

    case MONS_GREAT_ORB_OF_EYES:
#ifdef JP
        description += "表面が悪意ある目玉で埋め尽くされた、宙に浮く球体だ。 ";
#else
        description += "A levitating ball, covered in malignant eyes.";
#endif
        break;

    case MONS_EYE_OF_DEVASTATION:
#ifdef JP
        description +=
            "宙に浮く巨大な眼球だ。 $"
            "白熱するエネルギーの球体に包まれている。 $";
#else
        description += "A huge eyeball, encased in a levitating globe of "
            "incandescent energy. ";
#endif
        break;

    case MONS_SHINING_EYE:
#ifdef JP
        description +=
            "光を明滅させている巨大な奇形の眼球だ。 $"
            "このビホルダーには美しいところなど何もない。 ";
#else
        description += "A huge and strangely deformed eyeball, "
            "pulsating with light. "
            "Beauty is certainly nowhere to be found " "in this beholder. ";
#endif
        break;

    case MONS_EYE_OF_DRAINING:
        description +=
#ifdef JP
            "この宙に浮く恐怖の存在は、特に魔術師に忌み嫌われる。 ";
#else
            "These hovering horrors are especially loathed by wizards.";
#endif
        break;

    case MONS_WIGHT:
#ifdef JP
        description += "生への妄執によってアンデッドとなった、古代の戦士だ。 ";
#else
        description += "An ancient warrior, kept in a state of undeath "
            "by its will to live.";
#endif
        break;

    case MONS_WOLF_SPIDER:
#ifdef JP
        description +=
            "危険な大顎を持った毛深い大蜘蛛だ。 $"
            "食餌を求めてダンジョンを徘徊している。 ";
#else
        description += "A large hairy spider with vicious mandibles, "
            "roaming the dungeon in search of food.";
#endif
        break;

    case MONS_REDBACK:
#ifdef JP
        description +=
            "膨れ上がった腹部の背に赤い飛沫模様のある、獰猛な黒い蜘蛛だ。 $"
            "その下顎からは致命的な毒が滴っている。 ";
#else
        description += "A vicious black spider with a splash of red on its "
            "swollen abdomen. Its mandibles drip with lethal poison.";
#endif
        break;

    case MONS_SHADOW:
        description +=
#ifdef JP
            "視界の隅を横切る、生命なき一筋の影だ。 ";
#else
            "An wisp of unliving shadow, drifting on the edge of vision.";
#endif
        break;

    case MONS_HUNGRY_GHOST:
#ifdef JP
        description +=
            "飢えで死んだ何者かのアンデッドだ。 $"
            "このモンスターはあなたも飢え死にさせてやろうと考えている！ ";
#else
        description += "The undead form of someone who died of starvation,"
            " this creature wants the same thing to happen to you!";
#endif
        break;

    case MONS_BUTTERFLY:
#ifdef JP
        description +=
            "大きな多色の蝶々だ。その羽は美しい模様をしている。 ";
#else
        description += "A large multicoloured butterfly with beautifully "
            "patterned wings.";
#endif
        break;

    case MONS_WANDERING_MUSHROOM:
#ifdef JP
        description += "大きく肥大したキノコだ。 ";
#else
        description += "A large, fat mushroom.";
#endif
        break;

    case MONS_EFREET:
        description +=
#ifdef JP
            "灼熱の炎に包まれた、巨大で筋骨逞しい存在だ。 $";
#else
            "A huge and muscular figure engulfed in a cloud of searing flame.";
#endif
        break;

    case MONS_GIANT_ORANGE_BRAIN:
#ifdef JP
        description +=
            "床から少し浮き上がっている、巨大で皺が寄った脳だ。 $"
            "時々脈動している。 ";
#else
        description += "A huge wrinkled brain, floating just off the floor."
            " Every now and then it seems to pulsate.";
#endif
        break;

    case MONS_GIANT_BEETLE:
#ifdef JP
        description +=
            "噛み砕く力の強い下顎を持つ、巨大な黒い甲虫だ。 $"
            "非常に固いキチン質の殻を持つ。 ";
#else
        description += "A huge black beetle with great crunching mandibles "
            "and very hard chitinous armour.";
#endif
        break;

    case MONS_BORING_BEETLE:
        description +=
#ifdef JP
            "岩を砕く巨大な下顎を持つ、大きな甲虫だ。 ";
#else
            "A large brown beetle with huge, rock-crushing mandibles.";
#endif
        break;

    case MONS_BOULDER_BEETLE:
        description +=
#ifdef JP
            "貫くことがほとんど不可能な岩のような甲殻を持つ、 $"
            "灰色の甲虫だ。 ";
#else
            "A huge grey beetle with an almost impenetrable rocky carapace.";
#endif
        break;

    case MONS_FLYING_SKULL:
        description +=
#ifdef JP
            "邪悪な魔法によって宙に浮く、頭蓋だけのアンデッドだ。 $"
            "その歯は非常に危険な代物だ。 ";
#else
            "Unholy magic keeps a disembodied undead skull hovering "
            "above the floor. It has a nasty set of teeth.";
#endif
        break;

    case MONS_MINOTAUR:
#ifdef JP
        description +=
            "大きく筋肉質な人間の体に雄牛の頭部を持つモンスターだ。 $"
            "隔離された迷宮を棲み処としている。 ";
#else
        description += "A large muscular human with the head of a bull. "
            "It makes its home in secluded labyrinths.";
#endif
        break;

    case MONS_SLIME_CREATURE:
        description +=
#ifdef JP
            "地面を這うベタベタした粘液の塊だ。 ";
#else
            "An icky glob of slime, which slithers along the ground.";
#endif
        break;

    case MONS_HELLION:
#ifdef JP
        description += "唸りをあげる地獄の業火に包まれた、恐ろしいデーモンだ。 ";
#else
        description += "A frightful demon, covered in roaring hellfire.";
#endif
        break;

    case MONS_TORMENTOR:
#ifdef JP
        description +=
            "この凶悪な悪魔はあらゆる種類の鉤爪と棘と鉤で $"
            "全身が覆われている。 ";
#else
        description += "This malign devil is covered in all manner "
            "of claws, spines and cruel hooks.";
#endif
        break;

    case MONS_REAPER:
#ifdef JP
        description += "巨大な鎌を手にした、骸骨のような姿の存在だ。 ";
#else
        description += "A skeletal form wielding a giant scythe. ";
#endif
        if (you.is_undead == US_ALIVE)
#ifdef JP
            description += "$彼はあなたの魂を迎えに来た！ ";
#else
            description += "It has come for your soul!";
#endif
        break;

    case MONS_SOUL_EATER:
        description +=
#ifdef JP
            "この強大な悪魔は、あなたに向かって宙を滑り寄る影のように見える。 $"
            "この存在は負のエネルギーの激しい霊気を放っている。 ";
#else
            "This greater demon looks like a shadow gliding through "
            "the air towards you. It radiates an intense aura of negative power.";
#endif
        break;

    case MONS_BEAST:
#ifdef JP
        description += "人間と獣の異様でおぞましい交雑物だ。 ";
#else
        description += "A weird and hideous cross between beast and human.";
#endif
        break;

    case MONS_GLOWING_SHAPESHIFTER:
#ifdef JP
        description +=
            "変身の制御を失った変身能力者だ。 $"
            "常に変化を続けている。 ";
#else
        description += "A shapeshifter who has lost control over its "
            "transformations, and is constantly changing form.";
#endif
        break;

    case MONS_SHAPESHIFTER:
#ifdef JP
        description +=
            "形態を変化させる能力を持つ奇怪なモンスターだ。 $"
            "このモンスターが本来の形態で目撃されることは極めて稀だ。 ";
#else
        description += "A weird creature with the power to change its form. "
            "It is very rarely observed alive in its natural state.";
#endif
        break;

    case MONS_GIANT_MITE:
#ifdef JP
        description += "巨大なクモ形類動物だ。その顎には危険な毒がある。 ";
#else
        description += "A large arachnid with vicious poisoned mouth-parts.";
#endif
        break;

    case MONS_GRIFFON:
    case MONS_HIPPOGRIFF:
#ifdef JP
        description += "大鷲の頭と鉤爪と翼とを持ち、";
#else
        description += "A large creature with the hindquarters of a ";
#endif
        if (class_described == MONS_HIPPOGRIFF)
#ifdef JP
            description += "馬の";
#else
            description += "horse";
#endif
        else
#ifdef JP
            description += "ライオンの";
#else
            description += "lion";
#endif
#ifdef JP
        description += "後躯を持つ巨大なモンスターだ。 ";
#else
        description += " and the wings, head, and talons of a great eagle. ";
#endif
        break;

    case MONS_HYDRA:
        description +=
#ifdef JP
            "ドラゴンの遠縁である、強大な爬虫類の怪獣だ。 $"
            "これは複数の頭を持ち、更に頭を殖やす能力がある！ ";
#else
            "A great reptilian beast, distantly related to the dragon."
            " It has many heads, and the potential to grow many more!";
#endif
        break;

    case MONS_SKELETON_SMALL:   //MONS_SMALL_SKELETON:
    case MONS_SKELETON_LARGE:   //MONS_LARGE_SKELETON:
        description +=
#ifdef JP
            "死霊術の働きによってアンデッドにされた骸骨だ。 ";
#else
            "A skeleton compelled to unlife by the exercise of necromancy.";
#endif
        break;

    case MONS_SKELETAL_WARRIOR:
#ifdef JP
        description +=
            "重武装した凶悪な、人型生物の骸骨だ。 $"
            "これは邪悪な力によって造られた。 ";
#else
        description += "The vicious and heavily armed skeleton of a humanoid "
            "creature, animated by unholy power.";
#endif
        break;

    case MONS_HELL_KNIGHT:
#ifdef JP
        description +=
            "地獄の勢力と盟約した、重武装の戦士だ。 ";
#else
        description += "A heavily armoured warrior, in league with the powers"
            " of Hell.";
#endif
        break;

    case MONS_WIZARD:
#ifdef JP
        description +=
            "様々な秘術に手を染めている、風変わりな人物だ。 ";
#else
        description += "An rather eccentric person, dabbling in all sorts of"
            " arcanities.";
#endif
        break;

    case MONS_NECROMANCER:
        description +=
#ifdef JP
            "死霊術の魔法を専門的に扱う魔術師だ。 ";
#else
            "A wizard specializing in the practices of necromantic magic.";
#endif
        break;

    case MONS_GNOLL:
        description +=
#ifdef JP
            "ゴブリンとオークの遠縁種だ。 $"
            "上背があり、良い装備をしている。 ";
#else
            "A taller and better equipt relative of goblins and orcs.";
#endif
        break;

    case MONS_CLAY_GOLEM:
#ifdef JP
        description += "かりそめの生命を与えられた巨大な土の像だ。 ";
#else
        description += "A huge animated clay statue.";
#endif
        break;

    case MONS_WOOD_GOLEM:
#ifdef JP
        description += "かりそめの生命を与えられた巨大な木の像だ。 ";
#else
        description += "An animated wooden statue.";
#endif
        break;

    case MONS_STONE_GOLEM:
#ifdef JP
        description += "かりそめの生命を与えられた巨大な石の像だ。 ";
#else
        description += "A huge animated stone statue.";
#endif
        break;

    case MONS_IRON_GOLEM:
#ifdef JP
        description += "かりそめの生命を与えられた巨大な金属の像だ。 ";
#else
        description += "A huge animated metal statue.";
#endif
        break;

    case MONS_CRYSTAL_GOLEM:
#ifdef JP
        description += "かりそめの生命を与えられた巨大な水晶の像だ。 ";
#else
        description += "A huge animated crystal statue.";
#endif
        break;

    case MONS_TOENAIL_GOLEM:
#ifdef JP
        description +=
            "かりそめの生命を与えられた巨大な像だ。 $"
            "爪切りで切った足の爪だけで造られている。 $"
            "非常に時間の有り余った誰かが造ったのだろう。 ";
#else
        description += "A huge animated statue made entirely from toenail "
            "clippings. Some people just have too much time on their hands.";
#endif
        break;

    case MONS_ELECTRIC_GOLEM:
#ifdef JP
        description += "かりそめの生命を与えられた巨大な像だ。 $"
                       "純粋に電気だけで造られている。 ";
#else
        description += "An animated figure made completely of electricity. ";
#endif
        break;

    case MONS_EARTH_ELEMENTAL:
#ifdef JP
        description +=
            "大地の精霊界から召換された精霊だ。 $"
            "土と岩の塊に宿ることでこの世界に存在している。 ";
#else
        description += "A spirit drawn from the elemental plane of earth, "
            "which exists in this world by inhabiting a lump of earth and rocks.";
#endif
        break;

    case MONS_FIRE_ELEMENTAL:
#ifdef JP
        description +=
            "火の精霊界から召換された精霊だ。 $"
            "輝く猛烈な炎の柱に宿ることでこの世界に存在している。 ";
#else
        description += "A spirit drawn from the elemental plane of fire, "
            "which exists in this world as a brilliant column of raging flames.";
#endif
        break;

    case MONS_AIR_ELEMENTAL:
#ifdef JP
        description +=
            "大気の精霊界から召換された精霊だ。 $"
            "頻繁に離散集合する竜巻に宿ることでこの世界に存在している。 ";
#else
        description += "A spirit drawn from the elemental plane of air. "
            "It exists in this world as a swirling vortex of air, "
            "often dissipating and reforming.";
#endif
        break;

    case MONS_WATER_ELEMENTAL:
#ifdef JP
        description +=
            "水の精霊界から召換された精霊だ。 $"
            "水域の一部分に宿ることでこの世界に存在している。 ";
#else
        description += "A spirit drawn from the elemental plane of water. "
            "It exists on this world as part of a body of water.";
#endif
        break;

    case MONS_SPECTRAL_WARRIOR: // spectre
#ifdef JP
        description += "見るもおぞましい、半透明な緑色をしたアンデッド精神体だ。 ";
#else
        description += "A hideous translucent green undead spirit.";
#endif
        break;

    case MONS_CURSE_TOE:
#ifdef JP
        description +=
            "切り離された足の指だ。 "
            "宙に浮いていて、負のエネルギーの強烈な場を放射している。 ";
#else
        description += "A disembodied toe, hanging in the air and"
            " radiating an intense field of negative energy.";
#endif
        break;

    case MONS_PULSATING_LUMP:
#ifdef JP
        description += "胸の悪くなるような震える肉塊だ。 ";
#else
        description += "A revolting glob of writhing flesh.";
#endif
        break;

    case MONS_OOZE:
#ifdef JP
        description += "気色の悪い灰色のヘドロの塊だ。 ";
#else
        description += "A disgusting glob of grey sludge.";
#endif
        break;

    case MONS_BROWN_OOZE:
#ifdef JP
        description +=
            "粘着性の液体だ。 $"
            "腐蝕させる有機物を探して床を這っている。 ";
#else
        description += "A viscous liquid, flowing along the floor "
            "in search of organic matter to corrode. ";
#endif
        break;

    case MONS_DEATH_OOZE:
#ifdef JP
        description += "悪臭を放つ腐敗した肉の塊だ。 ";
#else
        description += "A putrid mass of decaying flesh. ";
#endif
        break;

    case MONS_GIANT_AMOEBA:
#ifdef JP
        description += "脈動する原形質の塊だ。 ";
#else
        description += "A pulsating lump of protoplasm. ";
#endif
        break;

    case MONS_JELLY:
#ifdef JP
        description +=
            "脈動する酸性の原形質の塊だ。これは何でも消化してしまう。 $"
            "そして短時間のうちに増殖していく……。 ";
#else
        description += "A pulsating mass of acidic protoplasm. It can and "
            "will eat almost anything, and grows a little each time...";
#endif
        break;

    case MONS_AZURE_JELLY:
#ifdef JP
        description += "空色をした冷たい細胞質の塊だ。 ";
#else
        description += "A frosty blob of bright blue cytoplasm. ";
#endif
        break;

    case MONS_ACID_BLOB:
        description +=
#ifdef JP
            "致命的な酸を滲ませる、不健康な緑色をした肉塊だ。 ";
#else
            "A lump of sickly green flesh, dripping with lethal acid.";
#endif
        break;

    case MONS_JELLYFISH:
#ifdef JP
        description +=
             "脈動する透明な肉塊だ。 $"
             "そのたくさんの触手であなたを刺そうと、水面で待ち伏せている。 ";
#else
        description += "A pulsating glob of transparent flesh, waiting just "
            "below the surface to sting you with its many tentacles.";
#endif
        break;

    case MONS_ROYAL_JELLY:
#ifdef JP
        description += "とりわけ豪華な、金色をしたゼリー状の存在だ。 ";
#else
        description += "A particularly rich and golden gelatinous thing. ";
#endif
        break;

    case MONS_FIRE_GIANT:
#ifdef JP
        description += "輝く頭髪を持つ赤色の巨人だ。 ";
#else
        description += "A huge ruddy humanoid with bright hair. ";
#endif
        break;

    case MONS_FROST_GIANT:
#ifdef JP
        description += "氷の頭髪を持つ青色の巨人だ。 ";
#else
        description += "A huge blue humanoid with hoarfrost hair.";
#endif
        break;

    case MONS_HILL_GIANT:
        description +=
#ifdef JP
            "巨人種の中では小柄な部類だが、この巨人の大きさは充分に危険だ。 ";
#else
            "Although one of the smaller giant varieties, this hill giant is still big enough to be dangerous.";
#endif
        break;

    case MONS_STONE_GIANT:
        description +=
#ifdef JP
            "岩のように固い灰色の皮膚を持つ巨大な人型生物だ。 $"
            "数個の巨石を運んでいる。……キャッチボールはいかがかな？ ";
#else
            "A gigantic humanoid with grey skin almost as hard as rock. "
            "It carries several boulders - are you up for a game of 'catch'?";
#endif
        break;

    case MONS_TITAN:
#ifdef JP
        description +=
            "この稲妻に照らし出された人間型生物は、 $"
            "巨人族の中にあってさえ著しく巨大で力強い。 ";
#else
        description += "This lightning-limned humanoid is unusually large "
            "and powerful, even among giants.";
#endif
        break;

    case MONS_FLAYED_GHOST:
#ifdef JP
        description +=
            "痩せさらばえた体から裂けた皮をぶら下げた、 $"
            "おぞましいアンデッドモンスターだ。 ";
#else
        description += "A hideous undead creature, with torn skin hanging "
            "from an emaciated body.";
#endif
        break;

    case MONS_INSUBSTANTIAL_WISP:
#ifdef JP
        description += "浮遊するガスの稀薄な一塊だ。 ";
#else
        description += "A thin wisp of floating gas.";
#endif
        break;

    case MONS_VAPOUR:
#ifdef JP
        description += "通常は不可視の、気味の悪い蒸気による雲だ。 ";
#else
        description += "A normally invisible cloud of weird-looking vapour.";
#endif
        break;

    case MONS_DANCING_WEAPON:
#ifdef JP
        description += "宙を舞っている武器だ。 ";
#else
        description += "A weapon dancing in the air. ";
#endif
        break;

    case MONS_ELEPHANT_SLUG:
#ifdef JP
        description += "皺が寄った襞を持つ、巨大な灰色のナメクジだ。 ";
#else
        description += "A huge grey slug with folds of wrinkled skin. ";
#endif
        break;

    case MONS_GIANT_SLUG:
#ifdef JP
        description += "巨大で気色の悪い腹足類だ。 ";
#else
        description += "A huge and disgusting gastropod. ";
#endif
        break;

    case MONS_GIANT_SNAIL:
        description +=
#ifdef JP
            "明るい緑色の殻を持つ、巨大で気色の悪い腹足類だ。 ";
#else
            "A huge and disgusting gastropod with light green shell. ";
#endif
        break;

    case MONS_SHEEP:
#ifdef JP
        description += "間抜けな毛皮動物だ。その両眼に殺意を湛えている。 ";
#else
        description += "A stupid woolly animal, with murder in its eyes. ";
#endif
        break;

    case MONS_HOG:
#ifdef JP
        description += "大きくて太った、とてつもなく醜い豚だ。 ";
#else
        description += "A large, fat and very ugly pig. ";
#endif
        break;

    case MONS_HELL_HOG:
#ifdef JP
        description +=
            "地獄の窖で飼育された、 "
            "大きく太った、とてつもなく醜い豚だ。 ";
#else
        description += "A large, fat and very ugly pig, suckled "
            "in the pits of Hell. ";
#endif
        break;

    case MONS_GIANT_MOSQUITO:
#ifdef JP
        description += "巨大に肥大化した蚊だ。病的な様子だ。 ";
#else
        description += "A huge, bloated mosquito. It looks diseased.";
#endif
        break;

    case MONS_GIANT_CENTIPEDE:
#ifdef JP
        description += "このモンスターはたくさんの脚を持っている。 ";
#else
        description += "It has a lot of legs.";
#endif
        break;

    case MONS_GIANT_BLOWFLY:
#ifdef JP
        description += "巨大で鬱陶しい蝿だ。 ";
#else
        description += "A huge and irritating fly.";
#endif
        break;

    case MONS_GIANT_FROG:
        description +=
#ifdef JP
            "この蛙が小さな昆虫を主食にしているだけなら、 $"
            "こうも巨大にはならなかっただろう。 ";
#else
            "It probably didn't get this big by eating little insects.";
#endif
        break;

    case MONS_GIANT_BROWN_FROG:
#ifdef JP
        description +=
            "非常に大きく邪悪な外見をした、肉食の蛙だ。 $"
            "この蛙の瘤だらけの茶色の皮膚は、周囲の凸凹の岩に溶けこんで見える。 ";
#else
        description += "A very large and vicious-looking carnivorous frog. "
            "Its knobbly brown skin blends in with the rough rock of your surroundings.";
#endif
        break;

    case MONS_SPINY_FROG:
        description +=
#ifdef JP
            "その従兄弟より少しだけ小さな茶色の蛙だ。 $"
            "この棘だらけの蛙は、凶悪な返しのついた棘と突起によって $"
            "大きさの不足を補っている。 ";
#else
            "Although slightly smaller than its cousin, the giant brown"
            " frog, the spiny frog makes up for lack of size by being"
            " covered in wickedly barbed spines and spurs.";
#endif
        break;

    case MONS_BLINK_FROG:
        description +=
#ifdef JP
            "薄気味の悪い外見の蛙だ。常に実体と非実体に点滅している。 ";
#else
            "A weird-looking frog, constantly blinking in and out of reality.";
#endif
        break;

    case MONS_GIANT_COCKROACH:
#ifdef JP
        description += "巨大な茶色のゴキブリだ。 ";
#else
        description += "A large brown cockroach.";
#endif
        break;

    case MONS_PIT_FIEND:
#ifdef JP
        description += "巨大な翼ある悪魔だ。その皮膚は信じ難いほど強靭だ。 ";
#else
        description += "A huge winged fiend with incredibly tough skin.";
#endif
        break;

    case MONS_GARGOYLE:
#ifdef JP
        description += "生命を持ったおぞましい石像だ。 ";
#else
        description += "A hideous stone statue come to life.";
#endif
        break;

    case MONS_METAL_GARGOYLE:
#ifdef JP
        description += "生命を持ったおぞましい金属の像だ。 ";
#else
        description += "A hideous metal statue come to life.";
#endif
        break;

    case MONS_MOLTEN_GARGOYLE:
#ifdef JP
        description += "生命を持ったおぞましい溶岩の像だ。 ";
#else
        description += "A hideous melting stone statue come to life.";
#endif
        break;

    case MONS_ELF:
    case MONS_DEEP_ELF_SOLDIER:
    case MONS_DEEP_ELF_FIGHTER:
    case MONS_DEEP_ELF_KNIGHT:
    case MONS_DEEP_ELF_MAGE:
    case MONS_DEEP_ELF_SUMMONER:
    case MONS_DEEP_ELF_CONJURER:
    case MONS_DEEP_ELF_PRIEST:
    case MONS_DEEP_ELF_HIGH_PRIEST:
    case MONS_DEEP_ELF_DEMONOLOGIST:
    case MONS_DEEP_ELF_ANNIHILATOR:
    case MONS_DEEP_ELF_SORCERER:
    case MONS_DEEP_ELF_DEATH_MAGE:
        description +=
#ifdef JP
            "この陰鬱な洞窟に棲みついたエルフ族だ。 $";
#else
            "One of the race of elves which inhabits this dreary cave.$";
#endif
        switch (class_described)
        {

        case MONS_DEEP_ELF_SOLDIER:
#ifdef JP
            description += "このエルフは一般的な戦士だ。 ";
#else
            description += "This one is just common soldier.";
#endif
            break;

        case MONS_DEEP_ELF_FIGHTER:
#ifdef JP
            description += "この戦士は幾つか魔法を習い覚えている。 ";
#else
            description += "This soldier has learned some magic.";
#endif
            break;

        case MONS_DEEP_ELF_KNIGHT:
#ifdef JP
            description += "このエルフは過去の戦いによる傷跡を帯びている。 ";
#else
            description += "This one bears the scars of battles past.";
#endif
            break;

        case MONS_DEEP_ELF_MAGE:
#ifdef JP
            description += "このエルフの長い指の間には魔力が弾けている。 ";
#else
            description += "Mana crackles between this one's long fingers.";
#endif
            break;

        case MONS_DEEP_ELF_SUMMONER:
        case MONS_DEEP_ELF_CONJURER:
#ifdef JP
            description += "このエルフは古代の";
#else
            description += "This one is a mage specialized in the ancient art ";
#endif
            if (class_described == MONS_DEEP_ELF_SUMMONER)
#ifdef JP
                description += "召換の術に特化した魔術師だ。 ";
#else
                description += "of summoning servants";
#endif
            else
#ifdef JP
                description += "破壊の力を放つ術に特化した魔術師だ。 ";
#else
                description += "of hurling energies";
#endif
#ifdef JP
            //description += " of destruction.";
#else
            description += " of destruction.";
#endif
            break;

        case MONS_DEEP_ELF_PRIEST:
#ifdef JP
            description += "このエルフは闇エルフの神に仕えるしもべだ。 ";
#else
            description += "This one is a servant of the deep elves' god.";
#endif
            break;

        case MONS_DEEP_ELF_HIGH_PRIEST:
            description +=
#ifdef JP
                "このエルフは闇エルフの神に仕える高位のしもべだ。 ";
#else
                "This one is an exalted servant of the deep elves' god.";
#endif
            break;

        case MONS_DEEP_ELF_DEMONOLOGIST:
            description +=
#ifdef JP
                "この魔術師は悪魔術に特化しており、特筆すべきは $"
                "超自然的な悪魔の軍勢と長期に渡って接触していることだ。 ";
#else
                "This mage specialized in demonology, and is marked heavily "
                "from long years in contact with unnatural demonic forces.";
#endif
            break;

        case MONS_DEEP_ELF_ANNIHILATOR:
#ifdef JP
            description +=
                "このエルフは破壊の魔術を何より好み、また熟達している。 ";
#else
            description += "This one likes destructive magics more than most, "
                "and is better at them.";
#endif
            break;

        case MONS_DEEP_ELF_SORCERER:
#ifdef JP
            description += "この強力な魔法使いは地獄から力を引き出している。 ";
#else
            description += "This mighty spellcaster draws power from Hell.";
#endif
            break;

        case MONS_DEEP_ELF_DEATH_MAGE:
#ifdef JP
            description += "このエルフは強力な負の霊気を纏っている。 ";
#else
            description += "A strong negative aura surrounds this one.";
#endif
            break;

        case MONS_ELF:
            // These are only possible from polymorphing or shapeshifting.
#ifdef JP
            description += "このエルフは非常に普通の外見をしている。 ";
#else
            description += "This one is remarkably plain looking.";
#endif
            break;
        }
        break;

    case MONS_WHITE_IMP:
#ifdef JP
        description += "小さくて悪戯好きな下級デーモンだ。 ";
#else
        description += "A small and mischievous minor demon. ";
#endif
        break;

    case MONS_LEMURE:
#ifdef JP
        description += "かすかに人型をとどめた腐敗した白い肉の塊だ。 ";
#else
        description += "A vaguely humanoid blob of putrid white flesh. ";
#endif
        break;

    case MONS_UFETUBUS:
#ifdef JP
        description += "金切り声でペチャクチャ喋る下級デーモンだ。 ";
#else
        description += "A chattering and shrieking minor demon. ";
#endif
        break;

    case MONS_MANES:
#ifdef JP
        description += "醜悪で不気味な小型の下級デーモンだ。 ";
#else
        description += "An ugly, twisted little minor demon. ";
#endif
        break;

    case MONS_MIDGE:
#ifdef JP
        description += "小さな空飛ぶデーモンだ。 ";
#else
        description += "A small flying demon. ";
#endif
        break;

    case MONS_NEQOXEC:
#ifdef JP
        description += "奇怪な姿をしたデーモンだ。 ";
#else
        description += "A weirdly shaped demon. ";
#endif
        break;

    case MONS_ORANGE_DEMON:
#ifdef JP
        description += "危険な毒針を持つ、明るいオレンジ色のデーモンだ。 ";
#else
        description += "A bright orange demon with a venomous stinger. ";
#endif
        break;

    case MONS_HELLWING:
        description +=
#ifdef JP
            "おぞましい骸骨のデーモンだ。古代の干からびた皮の羽を持つ。 ";
#else
            "A hideous skeletal demon, with wings of ancient withered skin. ";
#endif
        break;

    case MONS_SMOKE_DEMON:
#ifdef JP
        description += "宙に浮いた蠢く煙の塊だ。 ";
#else
        description += "A writhing cloud of smoke hanging in the air. ";
#endif
        break;

    case MONS_YNOXINUL:
#ifdef JP
        description += "輝く金属の鱗を持つデーモンだ。 ";
#else
        description += "A demon with shiny metallic scales. ";
#endif
        break;

    case MONS_EXECUTIONER:
#ifdef JP
        description += "見るもおぞましい強力なデーモンだ。 ";
#else
        description += "A horribly powerful demon. ";
#endif
        break;

    case MONS_GREEN_DEATH:
        description +=
#ifdef JP
            "膨れあがった体をしたデーモンだ。 $"
            "全身が爛れた腫れ物に覆われ、致命的な毒の雲を放っている。 ";
#else
            "A bloated form covered in oozing sores and exhaling clouds of lethal poison. ";
#endif
        break;

    case MONS_BLUE_DEATH:
#ifdef JP
        description += "強大な青いデーモンだ。 ";
#else
        description += "A blue greater demon. ";
#endif
        break;

    case MONS_BALRUG:
        description +=
#ifdef JP
            "巨大で非常に強力なデーモンだ。炎と影とに包まれている。 ";
#else
            "A huge and very powerful demon, wreathed in fire and shadows. ";
#endif
        break;

    case MONS_CACODEMON:
#ifdef JP
        description += "見るもおぞましい醜悪な怒りのデーモンだ。 $"
                       "伝説的なまでの力を持つ。 ";
#else
        description += "A hideously ugly demon of rage and legendary power. ";
#endif
        break;

    case MONS_DEMONIC_CRAWLER:
#ifdef JP
        description +=
            "膨れあがった長い体を多数の短い足で支えていて、 $"
            "天辺には凶悪な外見の頭部がついている。 ";
#else
        description += "A long and bloated body, supported by "
            "dozens of short legs and topped with an evil-looking head. ";
#endif
        break;

    case MONS_SUN_DEMON:
        description +=
#ifdef JP
            "地に墜ちた星の光と激怒とに輝く、悪魔的な姿をした存在だ。 ";
#else
            "A demonic figure shining with the light and fury of a fallen star.";
#endif
        break;

    case MONS_SHADOW_IMP:
#ifdef JP
        description += "小さくて影のような下級デーモンだ。 ";
#else
        description += "A small and shadowy minor demon.";
#endif
        break;

    case MONS_SHADOW_DEMON:
#ifdef JP
        description +=
            "不可思議な悪魔的存在だ。 $"
            "常に自身の複数の影の中に霞んでいる。 ";
#else
        description += "A mysterious demonic figure,"
            " constantly blurring into multiple shadows of itself.";
#endif
        break;

    case MONS_LOROCYPROCA:
#ifdef JP
        description +=
            "背が高く痩せさらばえた人型の存在だ。 $"
            "生きているかのようにたなびく長いローブを羽織っている。 ";
#else
        description += "A tall and gaunt figure, "
            "draped in long robes which flow as if alive.";
#endif
        break;

    case MONS_GERYON:
        description +=
#ifdef JP
            "地獄の門を守っている、這い寄る巨大な大悪魔だ。 ";
#else
            "A huge and slithery arch-demon, guarding the gates of Hell. ";
#endif
        break;

    case MONS_DISPATER:
#ifdef JP
        description += "鉄の都ディースの支配者だ。 ";
#else
        description += "The lord of the Iron City of Dis. ";
#endif
        break;

    case MONS_ASMODEUS:
        description +=
#ifdef JP
            "地獄の深層に住まうデーモンの首領の一体だ。 ";
#else
            "One of the arch-demons who dwell in the depths of Hell. ";
#endif
        break;

    case MONS_ANTAEUS:
#ifdef JP
        description += "コキュートスの最深部に住まう強大なタイタンだ。 ";
#else
        description += "A great titan who lives in the depths of Cocytus. ";
#endif
        break;

    case MONS_ERESHKIGAL:
        description +=
#ifdef JP
            "タルタロスの死の地獄を支配する恐るべき大悪魔だ。 ";
#else
            "A fearsome arch-fiend who rules the deathly netherworld of Tartarus. ";
#endif
        break;

    case MONS_VAULT_GUARD:
#ifdef JP
        description += "重武装重装甲を備えた宝物庫の番人だ。 ";
#else
        description += "A heavily armed and armoured guardian of the Vaults. ";
#endif
        break;

    case MONS_CURSE_SKULL:
        description +=
#ifdef JP
            "空中に浮いてゆっくりと回転している、黒焦げの頭蓋骨だ。 $"
            "その黒ずんだ表面に刻まれた神秘的な記号が、ほとんどあらゆる $"
            "攻撃に対しての耐性を暗示している。 ";
#else
            "A charred skull floating in the air and rotating slowly. "
            "Mystic symbols carved into its blackened surface indicate "
            "its resistance to almost any form of attack. ";
#endif
        break;

    case MONS_ORB_GUARDIAN:
        description +=
#ifdef JP
            "巨大で輝いている、紫色のモンスターだ。 $"
            "オーブが自分自身を護らせるために創造したものだ。 ";
#else
            "A huge and glowing purple creature, created by the Orb to "
            "defend itself. ";
#endif
        break;

    case MONS_DAEVA:
        description +=
#ifdef JP
            "『輝けるもの』の神聖な使徒だ。 $"
            "輝く黄金の光に包まれた翼ある者として顕現している。 ";
#else
            "A divine agent of the Shining One. It manifests as a winged "
            "figure obscured by an aura of brilliant golden light. ";
#endif
        break;

    case MONS_SPECTRAL_THING:
#ifdef JP
        description += "おぞましい輝く幽霊だ。 ";
#else
        description += "A hideous glowing apparition.";
#endif
        break;

    case MONS_TENTACLED_MONSTROSITY:
        description +=
#ifdef JP
            "蠢く触手の集合体だ。悪臭を放つ粘液に覆われている。 ";
#else
            "A writhing mass of tentacles, all covered in putrid mucous.";
#endif
        break;

    case MONS_SPHINX:
        description +=
#ifdef JP
            "人間の頭にライオンの胴体、そして大きな鳥の翼を持つ $"
            "巨大なモンスターだ。 ";
#else
            "A large creature with a human head, the body of a lion, and "
            "the wings of a huge bird.";
#endif
        break;

    case MONS_ROTTING_HULK:
#ifdef JP
        description += "グールに近い、よろめき歩くアンデッドだ。 ";
#else
        description += "A shambling undead, related to the ghoul.";
#endif
        break;

    case MONS_KILLER_KLOWN:
#ifdef JP
        description +=
            "活気と笑いに溢れた滑稽な姿をした存在だ。 $"
            "あなたに会えてとても嬉しそうにしている……。 $"
            "あなたはその容貌にかすかな悪意の視線を向けていないだろうか？ $"
            "赤い化粧やその姿だけでは充分に楽しくはない？ $"
            "だったら一緒になって遊んでみるといい。 $"
            "きっとあなたにも楽しさがわかるだろう！ ";
#else
        description += "A comical figure full of life and laughter.  It"
            " looks very happy to see you... but is there a slightly malicious"
            " cast to its features?  Is that red facepaint or something"
            " altogether less pleasant?  Join in the fun, and maybe you'll"
            " find out!";
#endif
        break;

    case MONS_MOTH_OF_WRATH:
#ifdef JP
        description += "巨大な蛾だ。その毛深さと同じくらいに、凄まじく獰猛だ。 ";
#else
        description += "A huge moth, as violent as it is hairy.";
#endif
        break;

    case MONS_DEATH_COB:
#ifdef JP
        description += "恐ろしいトウモロコシの軸のアンデッドだ。 ";
#else
        description += "A dreadful undead cob of corn.";
#endif
        break;

    case MONS_BOGGART:
        description +=
#ifdef JP
            "不気味な小柄のゴブリン妖精だ。その魔法の悪戯に注意せよ！ ";
#else
            "A twisted little sprite-goblin. Beware of its magical tricks!";
#endif
        break;

    case MONS_LAVA_FISH:
#ifdef JP
        description += "溶岩の中で棲息する魚だ。 ";
#else
        description += "A fish which lives in lava.";
#endif
        break;

    case MONS_BIG_FISH:
#ifdef JP
        description += "異常な大きさの魚だ。 ";
#else
        description += "A fish of unusual size.";
#endif
        break;

    case MONS_GIANT_GOLDFISH:
        description +=
#ifdef JP
            "ペットに餌を与えすぎると、このような怪物になってしまう！ ";
#else
            "This is what happens when you give your pet goldfish too much food!";
#endif
        break;

    case MONS_ELECTRICAL_EEL:
        description +=
#ifdef JP
            "小さくてぬめぬめしたウナギだ。放電でバチバチいっている。 ";
#else
            "A small and slimy eel, crackling with electrical discharge.";
#endif
        break;

    case MONS_PLAYER_GHOST:
#ifdef JP
        description += ghost_description();
#else
        description += "The apparition of ";
        description += ghost_description();
        description += ".$";
#endif
        break;

    case MONS_PANDEMONIUM_DEMON:
        description += describe_demon();
        break;

    // mimics -- I'm not considering these descriptions a bug. -- bwr
    case MONS_GOLD_MIMIC:
        description +=
#ifdef JP
            "見たところは無害な金貨の山だが、厄介で危険な $"
            "変身する捕食モンスターの擬態だ。 ";
#else
            "An apparently harmless pile of gold coins hides a nasty "
            "venomous shapechanging predator.";
#endif
        break;

    case MONS_WEAPON_MIMIC:
        description +=
#ifdef JP
            "見たところは放置された武器だが、実は危険な小型の獣の $"
            "擬態した姿だ。 ";
#else
            "An apparently abandoned weapon, actually a vicious little "
            "beast in disguise.";
#endif
        break;

    case MONS_ARMOUR_MIMIC:
        description +=
#ifdef JP
            "見たところ放置された良い造りの鎧だが、 $"
            "実は危険な小型の獣の擬態した姿だ。 ";
#else
            "An apparently abandoned suit of finely-made armour, actually "
            "a vicious little beast in disguise.";
#endif
        break;

    case MONS_SCROLL_MIMIC:
        description +=
#ifdef JP
            "秘術のルーンが書き込まれた古びた羊皮紙だ。 $"
            "今、少し動かなかっただろうかか？ ";
#else
            "An ancient parchment covered in arcane runes. Did it just twitch?";
#endif
        break;

    case MONS_POTION_MIMIC:
#ifdef JP
        description += "美味しそうな外見の魔法の飲み物だ。拾おう！ ";
#else
        description += "A delicious looking magical drink. Go on, pick it up!";
#endif
        break;

    case MONS_BALL_LIGHTNING:
#ifdef JP
        description +=
                      "自然の怪奇だ。 $"
                      "球雷のそこら中を跳ねまわる挙動は、 $"
                      "通常の稲妻とはまるで異なっている。 ";
#else
        description += "An oddity of nature, ball lightning bounces around "
                      "behaving almost, but not quite, entirely unlike "
                      "regular lightning. ";
#endif
        break;

    case MONS_ORB_OF_FIRE:
#ifdef JP
        description += "原初の炎そのものの球体だ。 $"
                       "壮麗な爆発を起こす。 ";
#else
        description += "A globe of raw primordial fire, capable of "
                       "impressive pyrotechnics.";
#endif
        break;

    // the quokka is no more ... {dlb}
    // the quokka is back, without cyberware -- bwr
    case MONS_QUOKKA:
#ifdef JP
        description += "小さな有袋類だ。ネズミではない。 ";
#else
        description += "A small marsupial.  Don't call it a rat.";
#endif
        break;

    // uniques
    case MONS_MNOLEG:           // was: Nemelex Xobeh - and wrong! {dlb}
#ifdef JP
        description +=
            "不可思議に輝く人影だ。 $"
            "パンデモニウムの不気味な大気の中を踊っている。 ";
#else
        description += "A weirdly glowing figure, "
            "dancing through the twisted air of Pandemonium. ";
#endif
        break;

    case MONS_LOM_LOBON:        // was: Sif Muna - and wrong! {dlb}
#ifdef JP
        description +=
            "奇妙なまでに平穏な古代のデーモンだ。 $"
            "額の中央に輝く巨大な単眼が、 $"
            "あなたを冷ややかに見つめている ";
#else
        description += "An ancient and strangely serene demon. "
            "It regards you coldly from "
            "the huge glowing eye in the centre of its forehead. ";
#endif
        break;

    case MONS_CEREBOV:          // was: Okawaru - and wrong! {dlb}
#ifdef JP
        description +=
            "激怒した凶暴なデーモンだ。 $"
            "セレボブは輝く黄金の鎧を身につけ、 $"
            "巨大な捻れた剣を帯びた巨人として現れる。 ";
#else
        description += "A violent and wrathful demon, "
            "Cerebov appears as a giant human "
            "covered in shining golden armour "
            "and wielding a huge twisted sword. ";
#endif
        break;

    case MONS_GLOORX_VLOQ:      // was: Kikubaaqudgha - and wrong! {dlb}
#ifdef JP
        description += "深淵の暗闇を身に纏った影のような姿の存在だ。 ";
#else
        description += "A shadowy figure clothed in profound darkness. ";
#endif
        break;

    case MONS_TERENCE:
#ifdef JP
        description += "邪悪な人間の戦士だ。 ";
#else
        description += "An evil human fighter.";
#endif
        break;

    case MONS_JESSICA:
#ifdef JP
        description += "邪悪な魔女実習生だ。 ";
#else
        description += "An evil apprentice sorceress.";
#endif
        break;

    case MONS_SIGMUND:
#ifdef JP
        description +=
               "邪悪で精力的な年老いた人間だ。 $"
               "その目は狂気に輝いている。 $"
               "シグムンドは凶悪な外見の鎌を手にしている。 ";
#else
        description += "An evil and spry old human, whose eyes "
               "twinkle with madness.  Sigmund wields a nasty looking scythe.";
#endif
        break;

    case MONS_EDMUND:
#ifdef JP
        description += "軽い鎧を身に着けた戦士だ。 ";
#else
        description += "A lightly armoured warrior.";
#endif
        break;

    case MONS_PSYCHE:
#ifdef JP
        description += "金髪の女魔術師だ。 ";
#else
        description += "A fair-haired magess.";
#endif
        break;

    case MONS_DONALD:
#ifdef JP
        description += "あなたと同じく、オーブを探す冒険者だ。 ";
#else
        description += "An adventurer like you, trying to find the Orb.";
#endif
        break;

    case MONS_MICHAEL:
#ifdef JP
        description += "長いローブを纏った、強力な魔法使いだ。 ";
#else
        description += "A powerful spellcaster, dressed in a long robe.";
#endif
        break;

    case MONS_JOSEPH:
#ifdef JP
        description += "傭兵のようだ。 ";
#else
        description += "Looks like a mercenary.";
#endif
        break;

    case MONS_ERICA:
#ifdef JP
        description += "美貌の女呪術師だ。 ";
#else
        description += "A comely spellweaver.";
#endif
        break;

    case MONS_JOSEPHINE:
#ifdef JP
        description += "ドルイドの衣服を着た、醜い年配の人物だ。 ";
#else
        description += "An ugly elderly figure, dressed in Druidic clothes.";
#endif
        break;

    case MONS_HAROLD:
#ifdef JP
        description += "邪悪な人間の賞金稼ぎだ。 ";
#else
        description += "An evil human bounty hunter.";
#endif
        break;

    case MONS_NORBERT:
#ifdef JP
        description += "熟練した戦士だ。 ";
#else
        description += "A skilled warrior.";
#endif
        break;

    case MONS_JOZEF:
#ifdef JP
        description += "上背のある賞金稼ぎだ。 ";
#else
        description += "A tall bounty hunter.";
#endif
        break;

    case MONS_AGNES:
#ifdef JP
        description += "ほっそりした戦士だ。 ";
#else
        description += "A lanky warrior.";
#endif
        break;

    case MONS_MAUD:
#ifdef JP
        description += "邪悪な戦士だ。不可解なことだが、齧歯類のように見える。 ";
#else
        description += "An evil warrior who looks inexplicably like a rodent.";
#endif
        break;

    case MONS_LOUISE:
#ifdef JP
        description += "非常な重装甲を纏った魔法使いだ。 ";
#else
        description += "An unusually heavily armoured spellcaster.";
#endif
        break;

    case MONS_FRANCIS:
#ifdef JP
        description += "皺くちゃの魔法使いだ。 ";
#else
        description += "A wizened spellcaster.";
#endif
        break;

    case MONS_FRANCES:
#ifdef JP
        description += "体格の良い戦士だ。顔に深い古傷を帯びている。 ";
#else
        description += "A stout warrior, bearing a deep facial scar.";
#endif
        break;

    case MONS_RUPERT:
#ifdef JP
        description += "邪悪な狂戦士だ。 ";
#else
        description += "An evil berserker.";
#endif
        break;

    case MONS_WAYNE:
#ifdef JP
        description += "邪悪な太っちょドワーフだ。馬鹿みたいな帽子を被っている。 ";
#else
        description += "A fat, evil dwarf in a stupid looking hat.";
#endif
        break;

    case MONS_DUANE:
#ifdef JP
        description += "異常に大きな耳をした、邪悪な傭兵だ。 ";
#else
        description += "An evil mercenary with unusually large ears.";
#endif
        break;

    case MONS_NORRIS:
#ifdef JP
        description += "褐色に日焼けした、邪悪そのもののサーファーだ。 ";
#else
        description += "A tan, fit and thoroughly evil surfer.";
#endif
        break;

    case MONS_ADOLF:
#ifdef JP
        description += "洗練された物腰の、戦士にして魔術師だ。 $"
                       "似合わない髭を生やしている。 ";
#else
        description += "A svelte fighter-mage with unfortunate facial hair.";
#endif
        break;

    case MONS_MARGERY:
#ifdef JP
        description += "身軽な魔法使いだ。 ";
#else
        description += "A lithe spellcaster.";
#endif
        break;

    case MONS_IJYB:
#ifdef JP
        description += "小さくて不細工なゴブリンだ。何やら青いぼろを着ている。 ";
#else
        description += "A small and twisted goblin, wearing some ugly blue rags.";
#endif
        break;

    case MONS_BLORK_THE_ORC:
#ifdef JP
        description += "著しく太った醜悪なオークだ。 ";
#else
        description += "A particularly fat and ugly orc.";
#endif
        break;

    case MONS_EROLCHA:
#ifdef JP
        description += "極めて狡猾なオーガの女魔術師だ。 ";
#else
        description += "An especially cunning ogre magess.";
#endif
        break;

    case MONS_URUG:
#ifdef JP
        description += "粗野で";
#else
        description += "A rude";
#endif
        if (you.species != SP_MUMMY)
#ifdef JP
            description += "臭い";
#else
            description += ", smelly";
#endif
#ifdef JP
        description += "オークだ。 ";
#else
        description += " orc.";
#endif
        break;

    case MONS_SNORG:
#ifdef JP
        description += "毛深いトロルだ。 ";
#else
        description += "A hairy troll.";
#endif
        break;

    case MONS_XTAHUA:
#ifdef JP
        description += "古代の強力なドラゴンだ。 ";
#else
        description += "An ancient and mighty dragon.";
#endif
        break;

    case MONS_BORIS:
        description +=
#ifdef JP
            "古代のリッチだ。彼のぼやけた姿の周辺の大気は、 $"
            "負の力に弾ける音を立てている。 ";
#else
            "An ancient lich. The air around his shrouded form crackles with evil energy. ";
#endif
        break;

    case MONS_SHUGGOTH:
#ifdef JP
        description +=
            "細長い頭部と棘の生えた尾、そして危険な六本指の爪を持つ $"
            "邪悪なモンスターだ。 $"
            "その驚異的な強さはこの辺境の次元に放逐されるだけのことはある。 ";
#else
        description += "A vile creature with an elongated head, spiked tail "
            "and wicked six-fingered claws. Its awesome strength is matched by "
            "its umbrage at being transported to this backwater dimension. ";
#endif
        break;

    case MONS_WOLF:
#ifdef JP
        description += "大きくて力強い灰色の犬科の動物だ。 ";
#else
        description += "A large and strong grey canine.";
#endif
        break;

    case MONS_WARG:
#ifdef JP
        description +=
            "殊更に大きく凶悪な外見をした狼だ。 $"
            "大抵の場合はオークと一緒にいる。 ";
#else
        description += "A particularly large and evil looking wolf, usually "
            "found in the company of orcs.";
#endif
        break;

    case MONS_BEAR:
#ifdef JP
        description += "ダンジョンに棲息する熊としては一般的なものだ。 ";
#else
        description += "The common dungeon bear.";
#endif
        break;

    case MONS_GRIZZLY_BEAR:
#ifdef JP
        description += "大きくて危険な、灰色の毛皮の熊だ。 ";
#else
        description += "A large, nasty bear with grey fur.";
#endif
        break;

    case MONS_POLAR_BEAR:
#ifdef JP
        description +=
            "大きくて非常に力の強い熊だ。 $"
            "白く輝く毛皮に包まれている。 ";
#else
        description += "A large and very strong bear covered in glistening "
            "white fur. ";
#endif
        break;

    case MONS_BLACK_BEAR:
#ifdef JP
        description += "小型の黒い熊だ。 ";
#else
        description += "A small black bear.";
#endif
        break;

    case MONS_SALAMANDER:   // mv: was ANOTHER_LAVA_THING
#ifdef JP
        description +=
            "奇怪な半人半蛇のモンスターだ。 $"
            "厚い赤色の鱗と棘とで覆われている。 ";
#else
        description += "A strange half-human half-snake creature "
            "covered in thick red scales and thorns.";
#endif
        break;

    case MONS_PROGRAM_BUG:
    default:
#ifdef JP
        description +=
            "もしもこのモンスターが『プログラムバグ』なら、 $"
            "このゲームを一旦セーブして再起動することが推奨される。 $"
            "プログラムバグに化けるモンスターを報告するか、 $"
            "当局に報告せずダンジョンを駆け巡るかはお好きなように。 ";
#else
        description += "If this monster is a \"program bug\", then it's "
            "recommended that you save your game and reload.  Please report "
            "monsters who masquerade as program bugs or run around the "
            "dungeon without a proper description to the authorities.";
#endif
        break;
        // onocentaur - donkey
    }

#if DEBUG_DIAGNOSTICS

    if (mons_flag( menv[ which_mons ].type, M_SPELLCASTER ))
    {
        int hspell_pass[6] = { MS_NO_SPELL, MS_NO_SPELL, MS_NO_SPELL,
                               MS_NO_SPELL, MS_NO_SPELL, MS_NO_SPELL };

        int msecc = ((class_described == MONS_HELLION)    ? MST_BURNING_DEVIL :
                     (class_described == MONS_PANDEMONIUM_DEMON) ? MST_GHOST
                                                 : menv[ which_mons ].number);

        mons_spell_list(msecc, hspell_pass);

        bool found_spell = false;

        for (int i = 0; i < 6; i++)
        {
            if (hspell_pass[i] != MS_NO_SPELL)
            {
                if (!found_spell)
                {
#ifdef JP
                    description += "$Monster Spells:$";
#else
                    description += "$Monster Spells:$";
#endif
                    found_spell = true;
                }

#ifdef JP
                snprintf( info, INFO_SIZE, "    %d: %s$", i,
#else
                snprintf( info, INFO_SIZE, "    %d: %s$", i,
#endif
                         mons_spell_name( hspell_pass[i] ) );

                description += info;
            }
        }
    }

    bool has_item = false;
    for (int i = 0; i < NUM_MONSTER_SLOTS; i++)
    {
        if (menv[ which_mons ].inv[i] != NON_ITEM)
        {
            if (!has_item)
            {
#ifdef JP
                description += "$Monster Inventory:$";
#else
                description += "$Monster Inventory:$";
#endif
                has_item = true;
            }

            char buff[ ITEMNAME_SIZE ];

            item_def item = mitm[ menv[which_mons].inv[i] ];
            set_ident_flags( item, ISFLAG_IDENT_MASK );

            item_name( item, DESC_NOCAP_A, buff );
#ifdef JP
            snprintf( info, INFO_SIZE, "    %d: %s$", i, buff );
#else
            snprintf( info, INFO_SIZE, "    %d: %s$", i, buff );
#endif
            description += info;
        }
    }

#endif

    print_description(description);

    set_keyin_mode(KEYIN_MODE_MORE);
    if (getch() == 0)
        getch();
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM
    puttext(25, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}                               // end describe_monsters

//---------------------------------------------------------------
//
// ghost_description
//
// Describes the current ghost's previous owner. The caller must
// prepend "The apparition of" or whatever and append any trailing
// punctuation that's wanted.
//
//---------------------------------------------------------------
std::string ghost_description(bool concise)
{
    char tmp_buff[ INFO_SIZE ];

    // We're fudging stats so that unarmed combat gets based off
    // of the ghost's species, not the player's stats... exact
    // stats are required anyways, all that matters is whether
    // dex >= str. -- bwr
    const int dex = 10;
    int str;
    switch (ghost.values[GVAL_SPECIES])
    {
      case SP_HILL_DWARF:
      case SP_MOUNTAIN_DWARF:
      case SP_TROLL:
      case SP_OGRE:
      case SP_OGRE_MAGE:
      case SP_MINOTAUR:
      case SP_HILL_ORC:
      case SP_CENTAUR:
      case SP_NAGA:
      case SP_MUMMY:
      case SP_GHOUL:
      str = 15;
      break;

      case SP_HUMAN:
      case SP_DEMIGOD:
      case SP_DEMONSPAWN:
      str = 10;
      break;

      default:
      str = 5;
      break;
    }

#ifdef JP
            snprintf( tmp_buff, sizeof(tmp_buff),
                  (concise ? "%s『%s』の亡霊(%s%s-%s)" :
                  "%s『%s』の亡霊だ。 $かつては%s%sの%sであった。 $") ,
                  skill_title( ghost.values[GVAL_BEST_SKILL],
                               ghost.values[GVAL_SKILL_LEVEL],
                               ghost.values[GVAL_SPECIES],
                               str, dex, GOD_NO_GOD ),
                  ghost.name,

                  (ghost.values[GVAL_EXP_LEVEL] <  4) ? "弱小な" :
                  (ghost.values[GVAL_EXP_LEVEL] <  7) ? "平凡な" :
                  (ghost.values[GVAL_EXP_LEVEL] < 11) ? "経験を積んだ" :
                  (ghost.values[GVAL_EXP_LEVEL] < 16) ? "力ある" :
                  (ghost.values[GVAL_EXP_LEVEL] < 22) ? "卓越した" :
                  (ghost.values[GVAL_EXP_LEVEL] < 26) ? "偉大な" :
                  (ghost.values[GVAL_EXP_LEVEL] < 27) ? "恐るべき強さの"
                                                      : "伝説的な",

            ( concise? get_species_abbrev(ghost.values[GVAL_SPECIES]) :
                species_name( ghost.values[GVAL_SPECIES],
                    ghost.values[GVAL_EXP_LEVEL] ) ),

            ( concise? get_class_abbrev(ghost.values[GVAL_CLASS]) :
                get_class_name( ghost.values[GVAL_CLASS] ) ) );

#else
    snprintf( tmp_buff, sizeof(tmp_buff),
            "%s the %s, a%s %s %s",
            ghost.name,

            skill_title( ghost.values[GVAL_BEST_SKILL],
                ghost.values[GVAL_SKILL_LEVEL],
                ghost.values[GVAL_SPECIES],
                str, dex, GOD_NO_GOD ),

            (ghost.values[GVAL_EXP_LEVEL] <  4) ? " weakling" :
            (ghost.values[GVAL_EXP_LEVEL] <  7) ? "n average" :
            (ghost.values[GVAL_EXP_LEVEL] < 11) ? "n experienced" :
            (ghost.values[GVAL_EXP_LEVEL] < 16) ? " powerful" :
            (ghost.values[GVAL_EXP_LEVEL] < 22) ? " mighty" :
            (ghost.values[GVAL_EXP_LEVEL] < 26) ? " great" :
            (ghost.values[GVAL_EXP_LEVEL] < 27) ? "n awesomely powerful"
            : " legendary",

            ( concise? get_species_abbrev(ghost.values[GVAL_SPECIES]) :
                species_name( ghost.values[GVAL_SPECIES],
                    ghost.values[GVAL_EXP_LEVEL] ) ),

            ( concise? get_class_abbrev(ghost.values[GVAL_CLASS]) :
                get_class_name( ghost.values[GVAL_CLASS] ) ) );
#endif
    return std::string(tmp_buff);
}

static void print_god_abil_desc( int abil )
{
    const ability_def &abil_info = get_ability_def( abil );

#ifdef JP
    const std::string cost = "(" + make_cost_description( abil_info ) + ")";
#else
    const std::string cost = "(" + make_cost_description( abil_info ) + ")";
#endif

    // Produce a 79 character string with cost right justified:
    std::string str( abil_info.name );
#ifdef JP
    str += std::string( 77 - str.length() - cost.length(), ' ' ) + cost + EOL;
#else
    str += std::string( 79 - str.length() - cost.length(), ' ' ) + cost + EOL;
#endif
    cprintf( str.c_str() );
}


//---------------------------------------------------------------
//
// describe_god
//
// Describes all gods. Accessible through altars (by praying), or
// by the ^ key if player is a worshipper.
//
//---------------------------------------------------------------

void describe_god( int which_god, bool give_title )
{

    const char *description; // mv: tmp string used for printing description
    int         colour;      // mv: colour used for some messages

#ifdef DOS_TERM
    char buffer[4000];
    gettext( 1, 1, 80, 25, buffer );
    window( 1, 1, 80, 25 );
#endif

    clrscr();

    if (give_title)
    {
        textcolor( WHITE );
#ifdef JP
        cprintf( "                                    信仰" EOL );
#else
        cprintf( "                                  Religion" EOL );
#endif
        textcolor( LIGHTGREY );
    }

    if (which_god == GOD_NO_GOD) //mv:no god -> say it and go away
    {
#ifdef JP
        cprintf( EOL "あなたは信仰を持っていない。 " );
#else
        cprintf( EOL "You are not religious." );
#endif
        goto end_god_info;
    }

    colour = god_colour(which_god);

    //mv: print god's name and title - if you can think up better titles
    //I have nothing against
    textcolor(colour);
    cprintf (god_name(which_god,true)); //print long god's name
    cprintf (EOL EOL);

    //mv: print god's description
    textcolor (LIGHTGRAY);

    switch (which_god)
    {
    case GOD_ZIN:
#ifdef JP
        description = "ジンは古くより崇められてきた神であり、" EOL
                      "秩序の確立と、夜と混沌の勢力の破壊に力を尽くしてきた。" EOL
                      "価値を認められた信徒は邪悪な存在と対峙するにあたって有用な様々な力を" EOL
                      "得ることができる。" EOL
                      "ただし死霊術その他の不浄な魔術の行使は差し控えなくてはならない。" EOL
                      "ジンは価値ある品の捧げ物と同様に、長期にわたる信仰の継続を高く評価する。";
#else
        description = "Zin is an ancient and revered God, dedicated to the establishment of order" EOL
                      "and the destruction of the forces of chaos and night. Valued worshippers " EOL
                      "can gain a variety of powers useful in the fight against the evil, but must" EOL
                      "abstain from the use of necromancy and other forms of unholy magic." EOL
                      "Zin appreciates long-standing faith as well as sacrifices of valued objects." EOL;
#endif
        break;

    case GOD_SHINING_ONE:
#ifdef JP
        description = "『輝けるもの』は力ある聖戦の神であり、悪との戦いに於いてジンと同盟関係にある。" EOL
                      "信徒は天界の怒りを速やかに振り下ろす能力を与えられるが、" EOL
                      "決して悪しき性質の魔術を使ってはならず、また名誉をもって戦わなくてはならない。" EOL
                      "『輝けるもの』は邪悪なモンスターの撲滅と同様に、" EOL
                      "長期にわたる信仰の継続を高く評価する。";
#else
        description = "The Shining One is a powerful crusading deity, allied with Zin in the fight" EOL
                      "against evil. Followers may be granted with the ability to summarily dispense" EOL
                      "the wrath of heaven, but must never use any form of evil magic and should" EOL
                      "fight honourably. The Shining One appreciates long-standing persistence in " EOL
                      "the endless crusade, as well as the dedicated destruction of unholy creatures.";
#endif
        break;

    case GOD_KIKUBAAQUDGHA:
#ifdef JP
        description = "キクバークッグァは恐るべき悪魔の神であり、" EOL
                      "死の力についての知識を探求する者たちが仕えている。" EOL
                      "信徒はアンデッドに関する特別な力を獲得し、" EOL
                      "特に寵愛された信徒は敵を殺すために力ある悪魔を呼び出すことができる。" EOL
                      "キクバークッグァは生ける存在の殺戮を常に要求している。" EOL
                      "しかし祭壇で捧げるのでない限りは、死体そのものには興味を示さない。";
#else
        description = "Kikubaaqudgha is a terrible Demon-God, served by those who seek knowledge of" EOL
                      "the powers of death. Followers gain special powers over the undead, and " EOL
                      "especially favoured servants can call on mighty demons to slay their foes." EOL
                      "Kikubaaqudgha requires the deaths of living creatures as often as possible," EOL
                      "but is not interested in the offering of corpses except at an appropriate" EOL
                      "altar.";
#endif
        break;

    case GOD_YREDELEMNUL:
#ifdef JP
        description = "イレデレンヌルは死霊術を学ぶことなしに死とアンデッドへの支配力を" EOL
                      "探求している者たちによって崇拝されている。" EOL
                      "信徒は奴隷化したアンデッドの軍勢を呼び起すことができ、" EOL
                      "他にも多くの(不快ではあるが)有用な力を獲得する。" EOL
                      "イレデレンヌルは殺しを喜ぶが、死体については捧げられるよりも" EOL
                      "アンデッドとして利用することなどをより好む。";
#else
        description = "Yredelemnul is worshipped by those who seek powers over death and the undead" EOL
                      "without having to learn to use necromancy. Followers can raise legions of " EOL
                      "servile undead and gain a number of other useful (if unpleasant) powers." EOL
                      "Yredelemnul appreciates killing, but prefers corpses to be put to use rather" EOL
                      "than sacrificed.";
#endif
        break;

    case GOD_XOM:
#ifdef JP
        description = "ゾムは狂気じみて気まぐれなカオスの神である。" EOL
                      "彼は崇拝者ではなく、玩ぶべきおもちゃを欲している。" EOL
                      "多くの者が素晴らしい褒美や強い力を賜ることを望んでゾムに跪拝するが、" EOL
                      "ゾムは信者をひたすら気まぐれに扱う。";
#else
        description = "Xom is a wild and unpredictable God of chaos, who seeks not worshippers but" EOL
                      "playthings to toy with. Many choose to follow Xom in the hope of receiving" EOL
                      "fabulous rewards and mighty powers, but Xom is nothing if not capricious. ";
#endif
        break;

    case GOD_VEHUMET:
#ifdef JP
        description = "ヴェフメットは破壊的な魔法の力を司る神である。" EOL
                      "信徒は秘術を駆使する能力を強化する様々な力を得る。" EOL
                      "そして最も寵愛を受けた者は、ヴェフメットの蔵書に記された " EOL
                      "恐るべき呪文を利用する資格を与えられる。" EOL
                      "ヴェフメットへの献身はあたう限りの虐殺と破壊とを引き起こすことをもって" EOL
                      "証明することができる。";
#else
        description = "Vehumet is a God of the destructive powers of magic. Followers gain various" EOL
                      "useful powers to enhance their command of the hermetic arts, and the most" EOL
                      "favoured stand to gain access to some of the fearsome spells in Vehumet's" EOL
                      "library. One's devotion to Vehumet can be proved by the causing of as much" EOL
                      "carnage and destruction as possible.";
#endif
        break;

    case GOD_OKAWARU:
#ifdef JP
        description = "オカワルは好戦的で強力な戦いの神である。" EOL
                      "信徒は戦いにおいて有用な数々の力と、褒美の品を得る。" EOL
                      "しかし信者は戦いと死体と価値ある品との献上をもって、" EOL
                      "常に信仰を証明しなくてはならない。";
#else
        description = "Okawaru is a dangerous and powerful God of battle. Followers can gain a " EOL
                      "number of powers useful in combat as well as various rewards, but must " EOL
                      "constantly prove themselves through battle and the sacrifice of corpses" EOL
                      "and valuable items.";
#endif
        break;

    case GOD_MAKHLEB:
#ifdef JP
        description = "破壊者マクレブは混沌と暴力による死とを司る恐るべき神である。" EOL
                      "信者は常に流血を捧げることでマクレブを満たさなければならないが、" EOL
                      "死と破壊の様々な力を得ることができる。" EOL
                      "この破壊神は、死体と価値ある品の献上を評価する。";
#else
        description = "Makhleb the Destroyer is a fearsome God of chaos and violent death. Followers," EOL
                      "who must constantly appease Makhleb with blood, stand to gain various powers " EOL
                      "of death and destruction. The Destroyer appreciates sacrifices of corpses and" EOL
                      "valuable items.";
#endif
        break;

    case GOD_SIF_MUNA:
#ifdef JP
        description = "シフ・ムーナは瞑想的ではあるが強力な神であり、" EOL
                      "魔法の知識を探求する者たちが仕えている。" EOL
                      "シフ・ムーナは価値ある品の献上と、頻繁に呪文の力を使うことを評価する。";
#else
        description = "Sif Muna is a contemplative but powerful deity, served by those who seek" EOL
                      "magical knowledge. Sif Muna appreciates sacrifices of valuable items, and" EOL
                      "the casting of spells as often as possible.";
#endif
        break;

    case GOD_TROG:
#ifdef JP
        description = "トログは怒りと暴力を司る古代の神である。" EOL
                      "信徒はトログの名に於いて殺すことと、死体を奉納することを期待される。" EOL
                      "そして見返りとして戦闘における能力と、時には褒美の品を得る。" EOL
                      "トログは魔術師を憎んでいるので、信者は魔法の使用を禁止される。";
#else
        description = "Trog is an ancient God of anger and violence. Followers are expected to kill" EOL
                      "in Trog's name and sacrifice the dead, and in return gain power in battle and" EOL
                      "occasional rewards. Trog hates wizards, and followers are forbidden the use" EOL
                      "of spell magic. ";
#endif
        break;

    case GOD_NEMELEX_XOBEH:
#ifdef JP
        description = "ネメレクスは不可思議で気まぐれなトリックスターの神である。" EOL
                      "その力はネメレクスが悪魔の血を用いて描いた魔法のカードのデッキを通して" EOL
                      "呼び出すことができる。" EOL
                      "信徒が特別な賜り物を受けた場合、それを可能な限り使いこなすことを求められる。" EOL
                      "ネメレクスはどのような種類の捧げ物でも喜んで受け容れる。";
#else
        description = "Nemelex is a strange and unpredictable trickster God, whose powers can be" EOL
                      "invoked through the magical packs of cards which Nemelex paints in the ichor" EOL
                      "of demons. Followers receive occasional gifts, and should use these gifts as" EOL
                      "as much as possible. Offerings of any type of item are also appreciated.";
#endif
        break;

    case GOD_ELYVILON:
#ifdef JP
        description = "癒し手エリヴィロンはとりわけ治療者によって崇拝される。" EOL
                      "信徒は長きに渡る崇拝と献身によって癒しの力を与えられる。" EOL
                      "エリヴィロンは平和主義の教義を掲げてはいるが、" EOL
                      "邪悪との聖戦を行う者については許容する。" EOL
                      "エリヴィロンは武器の奉納を評価する。";
#else
        description = "Elyvilon the Healer is worshipped by the healers (among others), who gain" EOL
                      "their healing powers by long worship and devotion. Although Elyvilon prefers" EOL
                      "a creed of pacifism, those who crusade against evil are not excluded. Elyvilon" EOL
                      "appreciates the offering of weapons. ";
#endif
        break;

    default:
#ifdef JP
        description = "プログラムバグの神は不気味で危険な神であり、" EOL
                      "彼らの存在は開発チームに報告されるべきである。";
#else
        description = "God of Program Bugs is a weird and dangerous God and his presence should" EOL
                      "be reported to dev-team.";
#endif
    }

    cprintf(description);
    //end of printing description

    // title only shown for our own god
    if (you.religion == which_god)
    {
        //mv: print title based on piety
#ifdef JP
        cprintf( EOL EOL "称号  - " );
#else
        cprintf( EOL EOL "Title - " );
#endif
        textcolor(colour);

        // mv: if your piety is high enough you get title
        // based on your god
        if (you.piety > 160)
        {
#ifdef JP
            cprintf((which_god == GOD_SHINING_ONE) ? "秩序の代行者" :
                    (which_god == GOD_ZIN) ? "聖なる戦士" :
                    (which_god == GOD_ELYVILON) ? "光の代行者" :
                    (which_god == GOD_OKAWARU) ? "千の戦の支配者" :
                    (which_god == GOD_YREDELEMNUL) ? "永遠なる死の支配者" :
                    (which_god == GOD_KIKUBAAQUDGHA) ? "暗黒の領主" :
                    (which_god == GOD_MAKHLEB) ? "混沌の代行者" :
                    (which_god == GOD_VEHUMET) ? "破壊の王" :
                    (which_god == GOD_TROG) ? "偉大なる殺戮者" :
                    (which_god == GOD_NEMELEX_XOBEH) ? "偉大なるトリックスター" :
                    (which_god == GOD_SIF_MUNA) ? "秘術の支配者" :
                    (which_god == GOD_XOM) ? "テディベア" :
                        "バグの王ボギー"); // Xom and no god is handled before
#else
            cprintf((which_god == GOD_SHINING_ONE) ? "Champion of Law" :
                    (which_god == GOD_ZIN) ? "Divine Warrior" :
                    (which_god == GOD_ELYVILON) ? "Champion of Light" :
                    (which_god == GOD_OKAWARU) ? "Master of Thousand Battles" :
                    (which_god == GOD_YREDELEMNUL) ? "Master of Eternal Death" :
                    (which_god == GOD_KIKUBAAQUDGHA) ? "Lord of Darkness" :
                    (which_god == GOD_MAKHLEB) ? "Champion of Chaos" :
                    (which_god == GOD_VEHUMET) ? "Lord of Destruction" :
                    (which_god == GOD_TROG) ? "Great Slayer" :
                    (which_god == GOD_NEMELEX_XOBEH) ? "Great Trickster" :
                    (which_god == GOD_SIF_MUNA) ? "Master of Arcane" :
                    (which_god == GOD_XOM) ? "Teddy Bear" :
                        "Bogy the Lord of the Bugs"); // Xom and no god is handled before
#endif
        }
        else
        {
            //mv: most titles are still universal - if any one wants to
            //he might write specific titles for all gods or rewrite current
            //ones (I know they are not perfect)
            //btw. titles are divided according to piety levels on which you get
            //new abilities.In the main it means - new ability = new title
            switch (which_god)
            {
            case GOD_ZIN:
            case GOD_SHINING_ONE:
            case GOD_KIKUBAAQUDGHA:
            case GOD_YREDELEMNUL:
            case GOD_VEHUMET:
            case GOD_OKAWARU:
            case GOD_MAKHLEB:
            case GOD_SIF_MUNA:
            //mv: what about
            //sinner, believer, apprentice, disciple, adept, scholar, oracle
            case GOD_TROG:
            case GOD_NEMELEX_XOBEH:
            case GOD_ELYVILON:
#ifdef JP
                cprintf ( (you.piety >= 120) ? "大祭司" :
                          (you.piety >= 100) ? "長老職" :
                          (you.piety >=  75) ? "司祭" :
                          (you.piety >=  50) ? "助祭" :
                          (you.piety >=  30) ? "修練者" :
                          (you.piety >    5) ? "信者"
                                             : "咎人" );
#else
                cprintf ( (you.piety >= 120) ? "High Priest" :
                          (you.piety >= 100) ? "Elder" :
                          (you.piety >=  75) ? "Priest" :
                          (you.piety >=  50) ? "Deacon" :
                          (you.piety >=  30) ? "Novice" :
                          (you.piety >    5) ? "Believer"
                                             : "Sinner" );
#endif
                break;

            case GOD_XOM:
#ifdef JP
                cprintf( (you.experience_level >= 20) ? "ゾムのお気に入り玩具"
                                                      : "玩具" );
#else
                cprintf( (you.experience_level >= 20) ? "Xom's favourite toy"
                                                      : "Toy" );
#endif
            break;

            default:
#ifdef JP
                cprintf ("バグ");
#else
                cprintf ("Bug");
#endif
            }
        }
    }
    // end of print title

    // mv: now let's print favor as Brent suggested
    // I know these messages aren't perfect so if you can
    // think up something better, do it

    textcolor(LIGHTGRAY);
#ifdef JP
    cprintf(EOL EOL "好意  - ");
#else
    cprintf(EOL EOL "Favour - ");
#endif
    textcolor(colour);

    //mv: player is praying at altar without appropriate religion
    //it means player isn't checking his own religion and so we only
    //display favour and will go out
    if (you.religion != which_god)
    {
        textcolor (colour);
        snprintf( info, INFO_SIZE,
#ifdef JP
                 (you.penance[which_god] >= 50) ? "%sはあなたに激怒している！" :
                 (you.penance[which_god] >= 20) ? "%sはあなたに苛立っている。" :
                 (you.penance[which_god] >=  5) ? "%sはあなたの罪をはっきりと覚えている。" :
                 (you.penance[which_god] >   0) ? "%sはあなたの罪を赦そうとしている。" :
                 (you.worshipped[which_god])    ? "%sはあなたに曖昧な感情を抱いている。"
                                                : "%sはあなたに対して中立である。",
                 god_name(which_god) );
#else
                 (you.penance[which_god] >= 50) ? "%s's wrath is upon you!" :
                 (you.penance[which_god] >= 20) ? "%s is annoyed with you." :
                 (you.penance[which_god] >=  5) ? "%s well remembers your sins." :
                 (you.penance[which_god] >   0) ? "%s is ready to forgive your sins." :
                 (you.worshipped[which_god])    ? "%s is ambivalent towards you."
                                                : "%s is neutral towards you.",
                 god_name(which_god) );
#endif

        cprintf(info);
    }
    else
    {
        if (player_under_penance()) //mv: penance check
        {
#ifdef JP
            cprintf( (you.penance[which_god] >= 50) ? "神はあなたに激怒している！" :
                     (you.penance[which_god] >= 20) ? "あなたは著しく道を踏み外した。懺悔せよ！" :
                     (you.penance[which_god] >= 5 ) ? "あなたは懺悔の途上にある。"
                                                    : "あなたは更なる自制を示すべきだ。" );
#else
            cprintf( (you.penance[which_god] >= 50) ? "Godly wrath is upon you!" :
                     (you.penance[which_god] >= 20) ? "You've transgressed heavily! Be penitent!" :
                     (you.penance[which_god] >= 5 ) ? "You are under penance."
                                                    : "You should show more discipline." );
#endif

        }
        else
        {
            if (which_god == GOD_XOM)
#ifdef JP
                cprintf("あなたは無視されている。");
#else
                cprintf("You are ignored.");
#endif
            else
            {
                snprintf( info, INFO_SIZE,

#ifdef JP
                         (you.piety > 130) ? "あなたは%sの第一の化身だ。":
                         (you.piety > 100) ? "あなたは%sの寵臣だ。" :
                         (you.piety >  70) ? "あなたは%sの期待の星だ。" :
                         (you.piety >  40) ? "%sはあなたにかなり満足している。" :
                         (you.piety >  20) ? "%sはあなたの存在を気に留めている。" :
                         (you.piety >   5) ? "%sはあなたに何も明言しない。"
                                           : "あなたは注目されてない。",
#else
                         (you.piety > 130) ? "A prized avatar of %s.":
                         (you.piety > 100) ? "A shining star in the eyes of %s." :
                         (you.piety >  70) ? "A rising star in the eyes of %s." :
                         (you.piety >  40) ? "%s is most pleased with you." :
                         (you.piety >  20) ? "%s has noted your presence." :
                         (you.piety >   5) ? "%s is noncommittal."
                                           : "You are beneath notice.",
#endif

                         god_name(which_god)
                       );

                cprintf(info);
            }
        }
        //end of favour

        //mv: following code shows abilities given from god (if any)


        textcolor(LIGHTGRAY);
#ifdef JP
        cprintf(EOL EOL "授けられた能力 :                                                     (コスト)" EOL);
#else
        cprintf(EOL EOL "Granted powers :                                                         (Cost)" EOL);
#endif
        textcolor(colour);


        // mv: these gods protects you during your prayer (not mentioning XOM)
        // chance for doing so is (random2(you.piety) >= 30)
        // Note that it's not depending on penance.
        // Btw. I'm not sure how to explain such divine protection
        // because god isn't really protecting player - he only sometimes
        // saves his life (probably it shouldn't be displayed at all).
        // What about this ?
        if ((which_god == GOD_ZIN
                || which_god == GOD_SHINING_ONE
                || which_god == GOD_ELYVILON
                || which_god == GOD_OKAWARU
                || which_god == GOD_YREDELEMNUL)
            && you.piety >= 30)
        {
            snprintf( info, INFO_SIZE,
#ifdef JP
                      "%sは祈りの間、%sあなたを見守っている。" EOL,
                      god_name(which_god),
                      (you.piety >= 150) ? "入念に":   // > 4/5
                      (you.piety >=  90) ? "大抵は": // > 2/3
                                           "時おり"    // less than 2:3
#else
                      "%s %s watches over you during prayer." EOL,
                      god_name(which_god),
                      (you.piety >= 150) ? "carefully":   // > 4/5
                      (you.piety >=  90) ? "often" :      // > 2/3
                                           "sometimes"    // less than 2:3
#endif
                    );

            cprintf(info);
        }

        // mv: No abilities (except divine protection)
        // under penance (fix me if I'm wrong)
        if (player_under_penance())
        {
#ifdef JP
            cprintf( "なし" EOL );
#else
            cprintf( "None." EOL );
#endif
        }
        else
        {
            switch (which_god) //mv: finaly let's print abilities
            {
            case GOD_ZIN:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_ZIN_REPEL_UNDEAD );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_ZIN_HEALING );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_ZIN_PESTILENCE );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_ZIN_HOLY_WORD );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_ZIN_SUMMON_GUARDIAN );
                break;

            case GOD_SHINING_ONE:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_TSO_REPEL_UNDEAD );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_TSO_SMITING );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_TSO_ANNIHILATE_UNDEAD );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_TSO_THUNDERBOLT );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_TSO_SUMMON_DAEVA );
                break;

            case GOD_KIKUBAAQUDGHA:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_KIKU_RECALL_UNDEAD_SLAVES );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
#ifdef JP
                    cprintf("あなたは死の魔術の副作用からの保護を与えられている。" EOL);

#else
                    cprintf("You are protected from some of the side-effects of death magic." EOL);

#endif
                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_KIKU_ENSLAVE_UNDEAD );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_KIKU_INVOKE_DEATH );
                break;

            case GOD_YREDELEMNUL:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_YRED_ANIMATE_CORPSE );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_YRED_RECALL_UNDEAD );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_YRED_ANIMATE_DEAD );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_YRED_DRAIN_LIFE );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_YRED_CONTROL_UNDEAD );
                break;


            case GOD_VEHUMET:
                if (you.piety >= 30)
                {
#ifdef JP
                    cprintf( "あなたはヴェフメットの名に於いての殺しによって力を得られる。" EOL
                             "                        あなたのしもべによる殺しでも同様だ。" EOL );
#else
                    cprintf( "You can gain power from the those you kill " EOL
                             "   in Vehumet's name, or those slain by your servants." EOL );
#endif
                }
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
#ifdef JP
                    cprintf( "ヴェフメットは祈りの間に破壊の魔術の手助けをする。" EOL );
#else
                    cprintf( "Vehumet assists with destructive magics during prayer." EOL );
#endif

                if (you.piety >= 75)
#ifdef JP
                    cprintf( "あなたは祈りの間、召換されたモンスターからの護りを得る。" EOL );
#else
                    cprintf( "During prayer you have some protection from summoned creatures." EOL );
#endif

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_VEHUMET_CHANNEL_ENERGY );
                break;


            case GOD_OKAWARU:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_OKAWARU_MIGHT );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_OKAWARU_HEALING );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_OKAWARU_HASTE );
                break;

            case GOD_MAKHLEB:
                if (you.piety >= 30)
                {
#ifdef JP
                    cprintf( "マクレブの名に於いての殺しによって力を得ることができる。" EOL );
#else
                    cprintf( "You can gain power from the deaths " EOL
                             "   of those you kill in Makhleb's name." EOL );
#endif
                }
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_MAKHLEB_MINOR_DESTRUCTION );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_MAKHLEB_LESSER_SERVANT_OF_MAKHLEB );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_MAKHLEB_MAJOR_DESTRUCTION );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_MAKHLEB_GREATER_SERVANT_OF_MAKHLEB );
                break;

            case GOD_SIF_MUNA:
                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_SIF_MUNA_FORGET_SPELL );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 100)
#ifdef JP
                    cprintf( "あなたは魔術の副作用からの保護を与えられている。" EOL );
#else
                    cprintf( "You are protected from some side-effects of spellcasting." EOL );
#endif
                break;

            case GOD_TROG:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_TROG_BERSERK );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_TROG_MIGHT );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_TROG_HASTE_SELF );
                break;

            case GOD_ELYVILON:
                if (you.piety >= 30)
                    print_god_abil_desc( ABIL_ELYVILON_LESSER_HEALING );
                else
#ifdef JP
                    cprintf( "なし" EOL );
#else
                    cprintf( "None." EOL );
#endif

                if (you.piety >= 50)
                    print_god_abil_desc( ABIL_ELYVILON_PURIFICATION );

                if (you.piety >= 75)
                    print_god_abil_desc( ABIL_ELYVILON_HEALING );

                if (you.piety >= 100)
                    print_god_abil_desc( ABIL_ELYVILON_RESTORATION );

                if (you.piety >= 120)
                    print_god_abil_desc( ABIL_ELYVILON_GREATER_HEALING );
                break;

            default:   //mv: default is Xom, Nemelex and all bugs.
#ifdef JP
                cprintf( "なし" EOL );
#else
                cprintf( "None." EOL );
#endif
            } //end of printing abilities
        }
    }


end_god_info: //end of everything (life, world, universe etc.)

    set_keyin_mode(KEYIN_MODE_MORE);
    getch(); // wait until keypressed
    set_keyin_mode(KEYIN_MODE_NONE);

#ifdef DOS_TERM //mv: if DOS_TERM is defined than buffer is returned to screen
                //if not redraw_screen() is called everytime when this function is
                //called
    puttext(1, 1, 80, 25, buffer);
    window(1, 1, 80, 25);
#endif
}          //mv: That's all folks.
