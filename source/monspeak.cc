/*
 *  File:       monspeak.cc
 *  Summary:    Functions to handle speaking monsters
 *
 *  Change History (most recent first):
 *
 *      <1>    01/09/00        BWR     Created
 */

#include "AppHdr.h"
#include "monspeak.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#ifdef DOS
#include <conio.h>
#endif

#include "externs.h"

#include "beam.h"
#include "debug.h"
#include "fight.h"
#include "insult.h"
#include "itemname.h"
#include "misc.h"
#include "monplace.h"
#include "monstuff.h"
#include "mon-util.h"
#include "mstuff2.h"
#include "player.h"
#include "spells2.h"
#include "spells4.h"
#include "stuff.h"
#include "view.h"

// returns true if something is said
bool mons_speaks(struct monsters *monster)
{
    int temp_rand;              // probability determination

    // This function is a little bit of a problem for the message channels
    // since some of the messages it generates are "fake" warning to
    // scare the player.  In order to accomidate this intent, we're
    // falsely categorizing various things in the function as spells and
    // danger warning... everything else just goes into the talk channel -- bwr
    int msg_type = MSGCH_TALK;

    const char *m_name = ptr_monam(monster, DESC_CAP_THE);
    strcpy(info, m_name);

    if (mons_has_ench(monster, ENCH_INVIS))
        return false;
    // invisible monster tries to remain unnoticed

    //mv: if it's also invisible, program never gets here
    if (silenced(monster->x, monster->y))
    {
        if (!one_chance_in(3))
            return false;       // while silenced, don't bother so often

        if (mons_has_ench(monster, ENCH_CONFUSION))
        {
            temp_rand = random2(10);
#ifdef JP
            strcat(info, (temp_rand <  4) ? "はでたらめな身振りをした。" :
                         (temp_rand == 4) ? "は混乱しているようだ。" :
                         (temp_rand == 5) ? "は邪悪な笑みを浮かべた。" :
                         (temp_rand == 6) ? "は幸福な笑みを浮かべている。" :
                         (temp_rand == 7) ? "は泣いている。"
                             : " says something but you don't hear anything.");
#else
            strcat(info, (temp_rand <  4) ? " gestures wildly." :
                         (temp_rand == 4) ? " looks confused." :
                         (temp_rand == 5) ? " grins evilly." :
                         (temp_rand == 6) ? " smiles happily." :
                         (temp_rand == 7) ? " cries."
                             : " says something but you don't hear anything.");
#endif
        }
        else if (monster->behaviour == BEH_FLEE)
        {
            temp_rand = random2(10);
            strcat(info,
#ifdef JP
                     (temp_rand <  3) ? "は周りを盗み見た。" :
                     (temp_rand == 3) ? "は叫ぶかのように大口をあけている。" :
                     (temp_rand == 4) ? "は辺りを見回している。" :
                     (temp_rand == 5) ? "はなんとも煮え切らない様子だ。" :
                     (temp_rand == 6) ? "は状況について思いを巡らしている。"
                                      : "は何か言い出しそうな様子だ。");
#else
                     (temp_rand <  3) ? " glances furtively about." :
                     (temp_rand == 3) ? " opens its mouth, as if shouting." :
                     (temp_rand == 4) ? " looks around." :
                     (temp_rand == 5) ? " appears indecisive." :
                     (temp_rand == 6) ? " ponders the situation."
                                      : " seems to says something.");
#endif
        }
        // disregard charmed critters.. they're not too expressive
        else if (monster->attitude == ATT_FRIENDLY)
        {
            temp_rand = random2(10);
#ifdef JP
            strcat(info, (temp_rand <  3) ? "はあなたに親指を立てた。" :
                         (temp_rand == 3) ? "があなたの方を見た。" :
                         (temp_rand == 4) ? "があなたに手を振っている。" :
                         (temp_rand == 5) ? "は楽しげに微笑んでいる。":
                         (temp_rand == 6) ? "はあなたにウィンクした。"
                             : "が何か言ったが、あなたには聞き取れなかった。");
#else
            strcat(info, (temp_rand <  3) ? " gives you a thumbs up." :
                         (temp_rand == 3) ? " looks at you." :
                         (temp_rand == 4) ? " waves at you." :
                         (temp_rand == 5) ? " smiles happily.":
                         (temp_rand == 6) ? " winks at you."
                             : " says something you can't hear.");
#endif
        }
        else
        {
            temp_rand = random2(10);
#ifdef JP
            strcat(info, (temp_rand <  3) ? "が何か身振りを送ってきた。" :
                         (temp_rand == 3) ? "が淫らな身振りを送ってきた。" :
                         (temp_rand == 4) ? "がにたりと笑った。" :
                         (temp_rand == 5) ? "は腹を立てている様子だ。" :
                         (temp_rand == 6) ? "は耳をそばだてている。"
                             : "が何か言ったが、あなたには何も聞こえなかった。");
#else
            strcat(info, (temp_rand <  3) ? " gestures." :
                         (temp_rand == 3) ? " gestures obscenely." :
                         (temp_rand == 4) ? " grins." :
                         (temp_rand == 5) ? " looks angry." :
                         (temp_rand == 6) ? " seems to be listening."
                             : " says something but you don't hear anything.");
#endif
        }                       //end switch silenced monster's behaviour

        mpr(info, MSGCH_TALK);
        return true;
    }                           // end silenced monster

    // charmed monsters aren't too expressive
    if (mons_has_ench(monster, ENCH_CHARM))
        return false;

    if (mons_has_ench(monster, ENCH_CONFUSION))
    {
        if (mons_holiness( monster->type ) == MH_DEMONIC
            && monster->type != MONS_IMP)
        {
            return (false);
        }

        if (mons_friendly(monster))
        {
            switch (random2(18))        // speaks for friendly confused monsters
            {
            case 0:
#ifdef JP
                strcat(info, "は助けを請うた。");
#else
                strcat(info, " prays for help.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「助けて！」");
#else
                strcat(info, " screams, \"Help!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は叫んだ。「自分を押さえられない！」");
#else
                strcat(info, " shouts, \"I'm losing control!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は叫んだ。「一体何が起こっているんだ？」");
#else
                strcat(info, " shouts, \"What's happening?\"");
#endif
                break;
            case 4:
            case 5:
#ifdef JP
                strcat(info, "はデタラメな身振りを送ってきた。");
#else
                strcat(info, " gestures wildly.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は泣いている。");
#else
                strcat(info, " cries.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は叫んだ。「イェーィ！」");
#else
                strcat(info, " shouts, \"Yeah!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は歌っている。");
#else
                strcat(info, " sings.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は狂ったように笑っている。");
#else
                strcat(info, " laughs crazily.");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は状況について思いを巡らしている。");
#else
                strcat(info, " ponders the situation.");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は気違いのようにニタニタしている。");
#else
                strcat(info, " grins madly.");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "はとても混乱している様子だ。");
#else
                strcat(info, " looks very confused.");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は何か呟いている。");
#else
                strcat(info, " mumbles something.");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は狂ったようにクスクス笑っている。");
#else
                strcat(info, " giggles crazily.");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "は絶叫した。「");
#else
                strcat(info, " screams, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "！助けてくれ！」");
#else
                strcat(info, "! Help!\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "は絶叫した。「");
#else
                strcat(info, " screams, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "！一体何が起きてるんだ？」");
#else
                strcat(info, "! What's going on?\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "は言った。「");
#else
                strcat(info, " says, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "、俺はほんのちょっと混乱してるだけだ」");
#else
                strcat(info, ", I'm little bit confused.\"");
#endif
                break;
            }
        }
        else
        {
            switch (random2(23))  // speaks for unfriendly confused monsters
            {
            case 0:
#ifdef JP
                strcat(info, "は喚いた。「この俺から奪ってみろ！」");
#else
                strcat(info, " yells, \"Get them off of me!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「必ずお前を殺してやる！」");
#else
                strcat(info, " screams, \"I will kill you anyway!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は叫んだ。「一体何が起きてるんだ？」");
#else
                strcat(info, " shouts, \"What's happening?\"");
#endif
                break;
            case 3:
            case 4:
            case 5:
#ifdef JP
                strcat(info, "はでたらめな身振りをした。");
#else
                strcat(info, " gestures wildly.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は泣いている。");
#else
                strcat(info, " cries.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は叫んだ。「違う！」");
#else
                strcat(info, " shouts, \"NO!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は叫んだ。「そうだ！」");
#else
                strcat(info, " shouts, \"YES!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は狂ったように笑っている。");
#else
                strcat(info, " laughs crazily.");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は状況について思いを巡らしている。");
#else
                strcat(info, " ponders the situation.");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は気違いのようにニタニタしている。");
#else
                strcat(info, " grins madly.");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "は非常に混乱している様子だ。");
#else
                strcat(info, " looks very confused.");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は何か呟いている。");
#else
                strcat(info, " mumbles something.");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "が言った。「俺はほんのちょっと混乱してるだけだ」");
#else
                strcat(info, " says, \"I'm little bit confused.\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "は尋ねた。「俺はどこにいるんだ？」");
#else
                strcat(info, " asks, \"Where am I?\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "は震えている。");
#else
                strcat(info, " shakes.");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "は尋ねた。「お前は誰だっけ？」");
#else
                strcat(info, " asks, \"Who are you?\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "は尋ねた。「何をしてたんだっけ？ウムム、そうなのか……」");
#else
                strcat(info, " asks, \"What the hell are we doing here? Mmm, I see...\"");
#endif
                break;
            case 19:
#ifdef JP
                strcat(info, "は泣き叫んだ。「俺の頭が！頭が！！」");
#else
                strcat(info, " cries, \"My head! MY HEAD!!!\"");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "は言った。「世界が回ってる。なんでだ？」");
#else
                strcat(info, " says, \"Why is everything spinning?\"");
#endif
                break;
            case 21:
#ifdef JP
                strcat(info, "は絶叫した。「嫌だ！この雑音は我慢できない！」");
#else
                strcat(info, " screams, \"NO! I can't bear up that noise!\"");
#endif
                break;
            case 22:
#ifdef JP
                strcat(info, "は目を庇おうとしている。");
#else
                strcat(info, " is trying to cover his eyes.");
#endif
                break;
            }
        }

    }
    else if (monster->behaviour == BEH_FLEE)
    {
        if (mons_holiness( monster->type ) == MH_DEMONIC
            && monster->type != MONS_IMP)
        {
            return (false);
        }

        if (mons_friendly(monster))
        {
            switch (random2(11))
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「待ってくれ！『", m_name,
                        coinflip() ? "叫んだ" : "喚いた");
                strcat(info, "「");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"WAIT FOR ME!\"", m_name,
                        coinflip() ? "shouts" : "yells");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "』、助けてくれないのか？");
#else
                strcat(info, ", could you help me?\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「助けてくれ！」");
#else
                strcat(info, " screams, \"Help!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は叫んだ。「俺の身を庇え！」");
#else
                strcat(info, " shouts, \"Cover me!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は絶叫した。「");
#else
                strcat(info, " screams, \"");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "！助けてくれ！");
#else
                strcat(info, "! Help me!\"");
#endif
                break;
            case 4:
            case 5:
            case 6:
#ifdef JP
                strcat(info, "はどこかに身を隠そうとしている。");
#else
                strcat(info, " tries to hide somewhere.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は助けを請うた。");
#else
                strcat(info, " prays for help.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は懇願するようにあなたを見た。");
#else
                strcat(info, " looks at you beseechingly.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は叫んだ。「俺の身を護れ！」");
#else
                strcat(info, " shouts, \"Protect me!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は懇願した。「友達だろ。見捨てないでくれ！」");
#else
                strcat(info, " cries, \"Don't forget your friends!\"");
#endif
                break;
            }
        }
        else
        {
            switch (random2(20))    // speaks for unfriendly fleeing monsters
            {
            case 0:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「助けてくれ！」", m_name,
                coinflip()? "喚いた" : "泣き叫んだ");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Help!\"", m_name, coinflip()? "yells" : "wails");
#endif
                break;
            case 1:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「助けてくれ！」", m_name,
                        coinflip() ? "懇願した" : "絶叫した"); break;
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Help!\"", m_name,
                        coinflip() ? "cries" : "screams"); break;
#endif
            case 2:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「仲良くやっていこうぜ？」",
                        m_name, coinflip() ? "慈悲を請うた" : "訴えかけた");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Why can't we all just get along?\"",
                        m_name, coinflip() ? "begs" : "pleads");
#endif
                break;
            case 3:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは逃げようとして%s転ぶところだった。", m_name,
                        coinflip() ? "危うく" : "もう少しで");
#else
                snprintf( info, INFO_SIZE, "%s %s trips in trying to escape.", m_name,
                        coinflip() ? "nearly" : "almost");
#endif
                break;
            case 4:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「ツキがなかったんだ！」", m_name,
                        coinflip() ? "呟いた" : "ブツブツ言った");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Of all the rotten luck!\"", m_name,
                        coinflip() ? "mutters" : "mumbles");
#endif
                break;
            case 5:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「あら！あらら！」", m_name,
                        coinflip() ? "うめいた" : "嘆いた");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Oh dear! Oh dear!\"", m_name,
                        coinflip() ? "moans" : "wails");
#endif
            case 6:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「畜生、しくじった！」", m_name,
                        coinflip() ? "呟いた" : "ブツブツ言った");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Damn and blast!\"", m_name,
                        coinflip() ? "mutters" : "mumbles");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は助けを請うた。");
#else
                strcat(info, " prays for help.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は叫んだ。「待ってくれ！もうあんなことはしない！」");
#else
                strcat(info, " shouts, \"No! I'll never do that again!\"");
#endif
                break;
            case 9:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s", m_name,
                        coinflip() ? "慈悲を請うた。" : "懇願した。「お慈悲を！」");
#else
                snprintf( info, INFO_SIZE, "%s %s", m_name,
                        coinflip() ? "begs for mercy." : "cries, \"Mercy!\"");
#endif
                break;
            case 10:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「%s」", m_name,
                        coinflip() ? "泣き喚いた" : "泣き叫んだ",
                        coinflip() ? "ママ！ママぁ！" : "パパ！パパぁ！");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"%s!\"", m_name,
                        coinflip() ? "blubbers" : "cries",
                        coinflip() ? "Mommeee" : "Daddeee");
#endif
                break;
            case 11:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「頼む、殺さないでくれ！」", m_name,
                        coinflip() ? "慈悲を請うた" : "訴えかけた");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Please don't kill me!\"", m_name,
                        coinflip() ? "begs" : "pleads");
#endif
                break;
            case 12:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「頼む、痛めつけないでくれ！」", m_name,
                        coinflip() ? "慈悲を請うた" : "訴えかけた");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Please don't hurt me!\"", m_name,
                        coinflip() ? "begs" : "pleads");
#endif
                break;
            case 13:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは%s。「頼む、俺には沢山の子供がいるんだ……」",
                        m_name, coinflip() ? "慈悲を請うた" : "訴えかけた");
#else
                snprintf( info, INFO_SIZE, "%s %s, \"Please, I have a lot of children...\"",
                        m_name, coinflip() ? "begs" : "pleads");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は失った勇気を取り戻そうと足掻いている。");
#else
                strcat(info, " tries to recover lost courage.");
#endif
                break;
            case 15:
            case 16:
            case 17:
#ifdef JP
                strcat(info, "は音をあげた。");
#else
                strcat(info, " gives up.");
#endif
                break;
            case 19:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは完全に%s様子だ。", m_name,
                        coinflip() ? "恐怖に竦んだ" : "弱り果てた");
#else
                snprintf( info, INFO_SIZE, "%s looks really %s.", m_name,
                        coinflip() ? "scared stiff" : "rattled");
#endif
                break;
            }
        }
    }
    else if (mons_friendly(monster))
    {
        if (mons_holiness( monster->type ) == MH_DEMONIC
            && monster->type != MONS_IMP)
        {
            return (false);
        }

        // friendly imps are too common so they speak very very rarely
        if ((monster->type == MONS_IMP) && (random2(10)))
            return (false);

        switch (random2(18))
        {
        case 0:
#ifdef JP
            strcat(info, "は喚いた。「行くぞ！俺が守ってやる！」");
#else
            strcat(info, " yells, \"Run! I'll cover you!\"");
#endif
            break;
        case 1:
#ifdef JP
            strcat(info, "は叫んだ。「死ね、怪物め！」");
#else
            strcat(info, " shouts, \"Die, monster!\"");
#endif
            break;
        case 2:
#ifdef JP
            strcat(info, "は言った。「仲間がいるって素晴らしいよな」");
#else
            strcat(info, " says, \"It's nice to have friends.\"");
#endif
            break;

        case 3:
#ifdef JP
            strcat(info, "はあなたを見つめている。");
#else
            strcat(info, " looks at you.");
#endif
            break;
        case 4:
#ifdef JP
            strcat(info, "はあなたに微笑みかけた。");
#else
            strcat(info, " smiles at you.");
#endif
            break;
        case 5:
#ifdef JP
            strcat(info, "は言った。「『");
#else
            strcat(info, " says, \"");
#endif
            strcat(info, you.your_name);
#ifdef JP
            strcat(info, "』、お前は俺の最高の友達だ」");
#else
            strcat(info, ", you are my only friend.\"");
#endif
            break;
        case 6:
#ifdef JP
            strcat(info, "は言った。「『");
#else
            strcat(info, " says, \"");
#endif
            strcat(info, you.your_name);
#ifdef JP
            strcat(info, "』、お前のことが好きだぜ」");
#else
            strcat(info, ", I like you.\"");
#endif
            break;

        case 7:
#ifdef JP
            strcat(info, "はあなたに手を振った。");
#else
            strcat(info, " waves at you.");
#endif
            break;
        case 8:
#ifdef JP
            strcat(info, "は言った。「用心しろ！」");
#else
            strcat(info, " says, \"Be careful!\"");
#endif
            break;
        case 9:
#ifdef JP
            strcat(info, "は言った。「心配すんな。俺がついてる」");
#else
            strcat(info, " says, \"Don't worry. I'm here with you.\"");
#endif
            break;
        case 10:
#ifdef JP
            strcat(info, "は幸福な笑みを浮かべている。");
#else
            strcat(info, " smiles happily.");
#endif
            break;
        case 11:
#ifdef JP
            strcat(info, "は叫んだ。「慈悲をかけるな！皆殺しだ！」");
#else
            strcat(info, " shouts, \"No mercy! Kill them all!");
#endif
            break;
        case 12:
#ifdef JP
            strcat(info, "はあなたにウィンクした。");
#else
            strcat(info, " winks at you.");
#endif
            break;
        case 13:
#ifdef JP
            strcat(info, "は言った。「俺とお前と。イカす響きだな」");
#else
            strcat(info, " says, \"Me and you. It sounds cool.\"");
#endif
            break;
        case 14:
#ifdef JP
            strcat(info, "は言った。「お前にずっとついてくぜ」");
#else
            strcat(info, " says, \"I'll never leave you.\"");
#endif
            break;
        case 15:
#ifdef JP
            strcat(info, "は言った。「お前のためなら死ねるぜ」");
#else
            strcat(info, " says, \"I would die for you.\"");
#endif
            break;
        case 16:
#ifdef JP
            strcat(info, "は叫んだ。「怪物に用心しろ！」");
#else
            strcat(info, " shouts, \"Beware of monsters!\"");
#endif
            break;
        case 17:
#ifdef JP
            strcat(info, "は友好的な様子だ。");
#else
            strcat(info, " looks friendly.");
#endif
            break;
        }
    }
    else
    {
        switch (monster->type)
        {
        case MONS_TERENCE:  // fighter who likes to kill
            switch (random2(15))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「殺してやる！」");
#else
                strcat(info, " screams, \"I'm going to kill you! \"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は叫んだ。「今からお前は死ぬ」");
#else
                strcat(info, " shouts, \"Now you die.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は言った。「安らかに眠れ」");
#else
                strcat(info, " says, \"Rest in peace.\"");
#endif
                break;
            case 3:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは叫んだ。「%s！！」",
                  m_name, coinflip() ? "行くぞ" : "くたばれ");
#else
                snprintf( info, INFO_SIZE, "%s shouts, \"%s!!!\"",
                  m_name, coinflip() ? "ATTACK" : "DIE");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は言った。「どうだ、楽しいか？」");
#else
                strcat(info, " says, \"How do you enjoy it?\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は叫んだ。「死ぬ準備を済ませろ！」");
#else
                strcat(info, " shouts, \"Get ready for death!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は言った。「お前は死んだも同然だ」");
#else
                strcat(info, " says, \"You are history.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は言った。「速やかに死にたいか？じわじわ行くか？」");
#else
                strcat(info, " says, \"Do you want it fast or slow?.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は言った。「遺言状は書いてあるか？必要になるぞ……」");
#else
                strcat(info, " says, \"Did you write a testament? You should...\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「サヨナラを言う頃合だ……」");
#else
                strcat(info, " says, \"Time to say good-bye...\"");
#endif
                break;
            case 10:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは言った。「%sな防御だな」",
                        m_name, coinflip() ? "無駄" : "無意味");
#else
                snprintf( info, INFO_SIZE, "%s says, \"Don't try to defend, it's %s.\"",
                        m_name, coinflip() ? "pointless" : "senseless");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は歯を剥き出した。");
#else
                strcat(info, " bares his teeth.");
#endif
                break;
            case 12:
#ifdef JP
                snprintf( info, INFO_SIZE, "%sは言った。「玄人の%sを見せてやる」",
                        m_name, coinflip() ? "技" : "妙手");
#else
                snprintf( info, INFO_SIZE, "%s says, \"I'll show you few %s.\"",
                        m_name, coinflip() ? "tricks" : "ploys.");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は絶叫した。「俺様はお前の血を所望だ」");
#else
                strcat(info, " screams, \"I want your blood.\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は軽蔑したようにあなたを一瞥した。");
#else
                strcat(info, " looks scornfully at you.");
#endif
                break;
            }
            break;          // end Terence

        case MONS_EDMUND:   // mercenaries guarding dungeon
        case MONS_LOUISE:   //Louise,Franchesは女性名
        case MONS_FRANCES:
        case MONS_DUANE:
        case MONS_ADOLF:
            switch (random2(17))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「今すぐ！殺してやる！」");
#else
                strcat(info, " screams, \"I'm going to kill you! Now!\"");
#endif
                break;
            case 1:
                strcat(info,
#ifdef JP
                       "は叫んだ。「直ちに引き返せ。さもなくば殺す！」");
#else
                       " shouts, \"Return immediately or I'll kill you!\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "は言った。「お前の旅もここでおしまいだ！」");
#else
                       " says, \"Now you've reached the end of your journey!\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "は叫んだ。「一歩間違えたらお前を殺してしまうよ！」");
#else
                       " screams, \"One false step and I'll kill you!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は言った。「盗んだ物を全部置いて、とっとと帰れ」");
#else
                strcat(info, " says, \"Drop everything you've found here and return home.\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は叫んだ。「オーブは決してお前の物にならない」");
#else
                strcat(info, " shouts, \"You will never get the Orb.\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は非常に敵意ある様子だ。");
#else
                strcat(info, " looks very unfriendly.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は非常に冷徹な様子だ。");
#else
                strcat(info, " looks very cold.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は叫んだ。「これでパーティーはお開きだ！」");
#else
                strcat(info, " shouts, \"It's the end of the party!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「盗んだアイテム全てを返せ！」");
#else
                strcat(info, " says, \"Return every stolen item!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は言った。「ここへの不法侵入は一切認められていない」");
#else
                strcat(info, " says, \"No trespassing is allowed here.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "は絶叫した。「お前は処罰されねばならない！」");
#else
                strcat(info, " screams, \"You must be punished!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は言った。「お前に恨みがあるわけじゃないが……」");
#else
                strcat(info, " says, \"It's nothing personal...\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は言った。「死せる冒険者のみが良き冒険者だ」");
#else
                strcat(info, " says, \"A dead adventurer is good adventurer.\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "は言った。「ここに来たのが運の尽きだ」");
#else
                strcat(info, " says, \"Coming here was your last mistake.\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "は叫んだ。「侵入者め！」");
#else
                strcat(info, " shouts, \"Intruder!\"");
#endif
                break;
            }
            break;          // end Edmund & Co

        case MONS_JOSEPH:
            switch (random2(16))
            {
            case 0:
#ifdef JP
                strcat(info, "は幸福な笑みを浮かべている。");
#else
                strcat(info, " smiles happily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は言った。「君に会えて嬉しいよ。君を殺せるのがまた嬉しい」");
#else
                strcat(info, " says, \"I'm happy to see you. And I'll be happy to kill you.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は言った。「この瞬間をどれほど待ちわびたことか」");
#else
                strcat(info, " says, \"I've waited for this moment for such a long time.\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "は言った。「君に恨みはないんだけど、殺さなくちゃいけなくてね」");
#else
                       " says, \"It's nothing personal but I have kill you.\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は言った。「悪いけど、オーブは決して君のものにならないよ」");
#else
                strcat(info, " says, \"You will never get the Orb, sorry.\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は叫んだ。「僕は戦いが！殺しが！！大好きなんだ！！」");
#else
                strcat(info, " shouts, \"I love to fight! I love killing!\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "は言った。「侵入者を殺すのが僕の役目さ。僕この仕事大好き」");
#else
                       " says, \"I'm here to kill trespassers. I like my job.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべようと努力した。");
#else
                strcat(info, " tries to grin evilly.");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "は言った。「君は処罰されないとね！僕がやりたいだけだけど！」");
#else
                       " says, \"You must be punished! Or... I want to punish you!\"");
#endif
                break;
            case 13:
                strcat(info,
#ifdef JP
                       "は嘆いた。「守衛の仕事って普段は退屈なんだよね……」");
#else
                       " sighs, \"Being guard is usually so boring...\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は叫んだ。「待ちに待った戦闘だ！」");
#else
                strcat(info, " shouts, \"At last some action!\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "は叫んだ。「ワォ！」");
#else
                strcat(info, " shouts, \"Wow!\"");
#endif
                break;
            }
            break;          // end Joseph

        case MONS_ORC_HIGH_PRIEST:  // priest, servants of dark ancient god
        case MONS_DEEP_ELF_HIGH_PRIEST:
            switch (random2(9))
            {
            case 0:
            case 1:
#ifdef JP
                strcat(info, "は祈りを上げた。");
#else
                strcat(info, " prays.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 2:
#ifdef JP
                strcat(info, "は何やら奇妙な祈祷の言葉を呟いた。");
#else
                strcat(info, " mumbles some strange prayers.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 3:
                strcat(info,
#ifdef JP
                       "は叫んだ。「お前のような異端者は殺されるべきだ」");
#else
                       " shouts, \"You are a heretic and must be destroyed.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は言った。「咎人みな等しく死すべし」");
#else
                strcat(info, " says, \"All sinners must die.\"");
#endif
                break;

            case 5:
#ifdef JP
                strcat(info, "は興奮した様子だ。");
#else
                strcat(info, " looks excited.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は言った。「お前は素晴らしい生贄になるだろう」");
#else
                strcat(info, " says, \"You will make a fine sacrifice.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は祈祷を吟じはじめた。");
#else
                strcat(info, " starts to sing a prayer.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は叫んだ。「お前は天罰を受けるべきだ」");
#else
                strcat(info, " shouts, \"You must be punished.\"");
#endif
                break;
            }
            break;          // end priests

        case MONS_ORC_SORCERER:   // hateful wizards, using strange powers
        case MONS_DEEP_ELF_SORCERER:
        case MONS_WIZARD:
            switch (random2(19))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                strcat(info, "は荒々しい身振りをした。");
#else
                strcat(info, " wildly gestures.");
#endif
                mpr( info, MSGCH_MONSTER_SPELL );
                if (coinflip())
                    canned_msg( MSG_NOTHING_HAPPENS );
                else
                    canned_msg( MSG_YOU_RESIST );
                return (true);

            case 3:
            case 4:
            case 5:
#ifdef JP
                strcat(info, "は何か奇妙な言葉で呟いた。");
#else
                strcat(info, " mumbles some strange words.");
#endif
                mpr( info, MSGCH_MONSTER_SPELL );
                if (coinflip())
                    canned_msg( MSG_NOTHING_HAPPENS );
                else
                    canned_msg( MSG_YOU_RESIST );
                return (true);

            case 6:
#ifdef JP
                strcat(info, "は叫んだ。「お前は我が力に抵抗できん」");
#else
                strcat(info, " shouts, \"You can't withstand my power!\"");
#endif
                break;

            case 7:
#ifdef JP
                strcat(info, "は叫んだ。「お前は死んだも同然だ」");
#else
                strcat(info, " shouts, \"You are history.\"");
#endif
                break;

            case 8:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "は一瞬だけ透明になった。");
#else
                strcat(info, " becomes transparent for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 9:
#ifdef JP
                strcat(info, "はあなたに向けて奇妙な力を投射した。");
#else
                strcat(info, " throws some strange powder towards you.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 10:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "は一瞬だけ明るく輝いた。");
#else
                strcat(info, " glows brightly for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 11:
#ifdef JP /* ?? */
                strcat(info, "は言った。「アルガタクス・ネトラノク・デルテクス」");
#else
                strcat(info, " says, \"argatax netranoch dertex\"");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 12:
#ifdef JP /* ?? */
                strcat(info, "は言った。「ドグロウ・ヌテウ・ベルグ」");
#else
                strcat(info, " says, \"dogrw nutew berg\"");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 13:
#ifdef JP /* ?? */
                strcat(info, "は叫んだ。「エントラム・モス・デグ・ウラグ！」");
#else
                strcat(info, " shouts, \"Entram moth deg ulag!\"");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 14:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);

                strcpy(info, m_name);
#ifdef JP
                strcat(info, "は一瞬だけ大きくなった。");
#else
                strcat(info, " becomes larger for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 15:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);

                strcpy(info, m_name);
#ifdef JP
                strcat(info, "の指先が輝きだした。");
#else
                strcat(info, "'s fingertips starts to glow.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 16:
#ifdef JP
                strcat(info, "の目は輝きだした。");
#else
                strcat(info, "'s eyes starts to glow.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 17:
#ifdef JP
                strcat(info, "はあなたを睨んで金縛りを試みた。");
#else
                strcat(info, " tries to paralyze you with his gaze.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 18:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
                canned_msg( MSG_YOU_RESIST );
                return (true);
            }
            break;          // end wizards

        case MONS_JESSICA:  // sorceress disturbed by player
            switch (random2(10))
            {
            case 0:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は言った。「私を本気で怒らせたな」");
#else
                strcat(info, " says, \"I'm really upset.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は言った。「私はお前みたいな野郎が大嫌いなんだ」");
#else
                strcat(info, " shouts, \"I don't like beings like you.\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "は言った。「私を煩わせるのをやめないなら殺すぞ！」");
#else
                       " shouts, \"Stop bothering me, or I'll kill you!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は冷ややかに言った。「私はお前の仲間が大嫌いなんだ」");
#else
                strcat(info, " very coldly says, \"I hate your company.\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "はおかしなことを呟いている。");
#else
                strcat(info, " mumbles something strange.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;
            case 6:
#ifdef JP
                strcat(info, "は怒り心頭の様子だ。");
#else
                strcat(info, " looks very angry.");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                        "は叫んだ。「お前は私を煩わせている。殺すことにした」");
#else
                        " shouts, \"You're disturbing me.  I'll have kill you.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「お前は恐ろしく鬱陶しい奴だな！」");
#else
                strcat(info, " screams, \"You are a ghastly nuisance!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "はでたらめな身振りをした。");
#else
                strcat(info, " gestures wildly.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;
            }
            break;          // end Jessica

        case MONS_SIGMUND:  // mad old wizard
            switch (random2(19))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                strcat(info, "は狂ったように笑っている。");
#else
                strcat(info, " laughs crazily.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は言った。「安心しろ。さっくり殺してやる」");
#else
                strcat(info, " says, \"Don't worry, I'll kill you fast.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は歯を軋ませた。");
#else
                strcat(info, " grinds his teeth.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は尋ねた。「私が好きなのか？」");
#else
                strcat(info, " asks, \"Do you like me?\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！怪物め！」");
#else
                strcat(info, " screams, \"Die, monster!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は言った。「すぐに何もかも忘れさせてやる」");
#else
                strcat(info, " says, \"You will soon forget everything.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「お前には無理だ……決してな！」");
#else
                strcat(info, " screams, \"You will never... NEVER!\"");
#endif
                break;

            case 9:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "の目が赤く輝きだした。");
#else
                strcat(info, "'s eyes starts to glow with a red light. ");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 10:
#ifdef JP
                strcat(info, "は言った。「私の目を見てみろ」");
#else
                strcat(info, " says, \"Look in to my eyes.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は言った。「私がお前の死神だ」");
#else
                strcat(info, " says, \"I'm your fate.\"");
#endif
                break;

            case 12:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "は突然に青白い光に包まれた。");
#else
                strcat(info, " is suddenly surrounded by pale blue light.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 13:
#ifdef JP
                strcat(info, "はあなたに噛みつこうとした。");
#else
                strcat(info, " tries to bite you.");
#endif
                break;

            case 14:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "は突然にぼやけた緑の光に包まれた。");
#else
                strcat(info, " is suddenly surrounded by pale green light.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 15:
#ifdef JP
                strcat(info, "は絶叫した。「この私が死の天使だ！」");
#else
                strcat(info, " screams, \"I am the angel of Death!\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "は絶叫した。「ただ死だけが、お前を自由にするのだ！」");
#else
                strcat(info, " screams, \"Only death can liberate you!\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "は囁いた。「すぐに死後の永劫を味わわせてやる」");
#else
                strcat(info, " whispers, \"You'll know eternity soon...\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "は絶叫した。「無駄な抵抗はやめろ！」");
#else
                strcat(info, " screams, \"Don't try to resist!\"");
#endif
                break;
            }
            break;          // end Sigmund

        case MONS_IMP:      // small demon
        case MONS_WHITE_IMP:
        case MONS_SHADOW_IMP:
            if (one_chance_in(3))
            {
                imp_taunt( monster );
                return (true);
            }
            else
            {
                switch (random2(11))
                {
                case 0:
#ifdef JP
                    strcat(info, "は狂ったように笑っている。");
#else
                    strcat(info, " laughs crazily.");
#endif
                    break;
                case 1:
#ifdef JP
                    strcat(info, "は邪悪な笑みを浮かべた。");
#else
                    strcat(info, " grins evilly.");
#endif
                    break;
                case 2:
#ifdef JP
                    strcat(info, "はあなたに煙を少し吹きかけた。");
#else
                    strcat(info, " breathes a bit of smoke at you.");
#endif
                    break;
                case 3:
#ifdef JP
                    strcat(info, "は尻尾を激しく振り下ろした。");
#else
                    strcat(info, " lashes with his tail.");
#endif
                    break;
                case 4:
#ifdef JP
                    strcat(info, "は歯を軋ませた。");
#else
                    strcat(info, " grinds his teeth.");
#endif
                    break;
                case 5:
#ifdef JP
                    strcat(info, "は口からつばを飛ばした。");
#else
                    strcat(info, " sputters.");
#endif
                    break;
                case 6:
#ifdef JP
                    strcat(info, "はあなたに蒸気を少し吹きかけた。");
#else
                    strcat(info, " breathes some steam toward you.");
#endif
                    break;
                case 7:
#ifdef JP
                    strcat(info, "はあなたにつばを吐きかけた。");
#else
                    strcat(info, " spits at you.");
#endif
                    break;
                case 8:
#ifdef JP
                    strcat(info, "は一瞬だけ姿を消した。");
#else
                    strcat(info, " disappears for a moment.");
#endif
                    break;
                case 9:
#ifdef JP
                    strcat(info, "はハエの群れを召換した。");
#else
                    strcat(info, " summons a swarm of flies.");
#endif
                    break;
                case 10:
#ifdef JP
                    strcat(info, "は甲虫を捕まえて食べた。");
#else
                    strcat(info, " picks up some beetle and eats it.");
#endif
                    break;
                }
            }
            break;          // end imp

        case MONS_TORMENTOR:        // cruel devil
            if (one_chance_in(10))
            {
                demon_taunt( monster );
                return (true);
            }
            else
            {
                switch (random2(18))
                {
                case 0:
#ifdef JP
                    strcat(info, "は狂ったように笑っている。");
#else
                    strcat(info, " laughs crazily.");
#endif
                    break;
                case 1:
#ifdef JP
                    strcat(info, "は邪悪な笑みを浮かべた。");
#else
                    strcat(info, " grins evilly.");
#endif
                    break;
                case 2:
#ifdef JP
                    strcat(info, "は言った。「我が輩は実体化せし貴様の悪夢だ」");
#else
                    strcat(info, " says, \"I am all your nightmares come true.\"");
#endif
                    break;
                case 3:
#ifdef JP
                    strcat(info, "は言った。「苦痛とは何かを、よおく教えてやろう」");
#else
                    strcat(info, " says, \"I will show you what pain is.\"");
#endif
                    break;
                case 4:
#ifdef JP
                    strcat(info, "は叫んだ。「バラバラに引き裂いてやろう」");
#else
                    strcat(info, " shouts, \"I'll tear you apart.\"");
#endif
                    break;
                case 5:
                    strcat(info,
#ifdef JP
                           "は言った。「我が拷問を受ければむしろ死を望むようになろう」");
#else
                           " says, \"You will wish to die when I get to you.\"");
#endif
                    break;
                case 6:
#ifdef JP
                    strcat(info, "は言った。「貴様自身の血の海で溺れさせてやろう」");
#else
                    strcat(info, " says, \"I will drown you in your own blood.\"");
#endif
                    break;
                case 7:
                    strcat(info,
#ifdef JP
                           "は叫んだ。「貴様は悲惨な死に方をすることになろう」");
#else
                           " screams, \"You will die horribly!\"");
#endif
                    break;
                case 8:
#ifdef JP
                    strcat(info, "は言った。「貴様の肝を食わせるがよい」");
#else
                    strcat(info, " says, \"I will eat your liver.\"");
#endif
                    break;
                case 9:
#ifdef JP
                    strcat(info, "は気違いのようにニタニタしている。");
#else
                    strcat(info, " grins madly.");
#endif
                    break;
                case 10:
#ifdef JP
                    strcat(info, "は叫んだ。「我が輩の千もの苦痛の針に覚悟したまえ」");
#else
                    strcat(info, " shouts, \"Prepare for my thousand needles of pain!\"");
#endif
                    break;
                case 11:
                    strcat(info,
#ifdef JP
                           "は言った。「貴様を殺すには千と一つものやり方があるのだ」");
#else
                           " says, \"I know thousand and one way to kill you.\"");
#endif
                    break;
                case 12:
                    strcat(info,
#ifdef JP
                           "は言った。「我が輩の拷問室をご覧に入れよう！」");
#else
                           " says, \"I'll show you my torture chamber!\"");
#endif
                    break;
                case 13:
                case 14:
                    strcat(info,
#ifdef JP
                           "は言った。「貴様の骨を一本づつ砕いてやろう」");
#else
                           " says, \"I'll crush your bones, one by one.\"");
#endif
                    break;
                case 15:
#ifdef JP
                    strcat(info, "は言った。「貴様の運命に待ち受けるのものは苦痛である」");
#else
                    strcat(info, " says, \"I know your fate. It's pain.\"");
#endif
                    break;
                case 16:
#ifdef JP
                    strcat(info, "は言った。「覚悟せよ！貴様には苦痛が待ち受けている」");
#else
                    strcat(info, " says, \"Get ready! Throes await you.\"");
#endif
                    break;
                case 17:
#ifdef JP
                    strcat(info, "は悪意を剥き出しにしている。");
#else
                    strcat(info, " grins malevolently.");
#endif
                    break;
                }
            }
            break;          // end tormentor

        case MONS_PANDEMONIUM_DEMON:    // named demons
        case MONS_GERYON:
        case MONS_ASMODEUS:
        case MONS_DISPATER:
        case MONS_ANTAEUS:
        case MONS_ERESHKIGAL:
        case MONS_MNOLEG:
        case MONS_LOM_LOBON:
        case MONS_CEREBOV:
        case MONS_GLOORX_VLOQ:
            demon_taunt( monster );
            return (true);

        case MONS_PLAYER_GHOST:     // ghost of unsuccesful player
            switch (random2(24))
            {
            case 0:
#ifdef JP
                strcat(info, "は狂ったように笑っている。");
#else
                strcat(info, " laughs crazily.");
#endif
                break;

            case 1:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;

            case 2:
#ifdef JP
                strcat(info, "は言った。「オーブは決してお前の物にならない！」");
#else
                strcat(info, " shouts, \"You will never get the ORB!\"");
#endif
                break;

            case 3: // mv: ghosts are usually wailing, aren't ?
            case 4:
            case 5:
            case 6:
            case 7:
            case 8:
            case 9:
            case 10:
            case 11:
#ifdef JP
                strcat(info, "は啜り泣いた。");
#else
                strcat(info, " wails.");
#endif
                break;

            case 12:
#ifdef JP
                strcat(info, "はあなたをじっと見つめた。");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは寒気を覚えた。", MSGCH_WARN);
#else
                mpr("You feel cold.", MSGCH_WARN);
#endif
                return (true);

            case 13:
#ifdef JP
                strcat(info, "は絶叫した。「お前ももうすぐ私の仲間だ！」");
#else
                strcat(info, " screams, \"You will join me soon!\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は嘆いた。「死んで、眠って、御仕舞いだ」");
#else
                strcat(info, " wails, \"To die, to sleep, no more.\"");
#endif
                break;      //Hamlet 暇ができたら対訳を当たる
            case 15:
                strcat(info,
#ifdef JP
                       "は絶叫した。「私が失敗したようにお前も失敗するのだ」");
#else
                       " screams, \"You must not succeed where I failed.\"");
#endif
                break;
            case 16:
                strcat(info,
#ifdef JP
                       "は絶叫した。「誰だろうがオーブを望む者は殺す」");
#else
                       " screams, \"I'll kill anyone who wants the ORB.\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "は囁いた。「死の虚ろの一部となるのだ！」");
#else
                strcat(info, " whispers, \"Meet emptiness of death!\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "は囁いた。「死こそ解放なのだ」");
#else
                strcat(info, " whispers, \"Death is liberation.\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "は囁いた。「永遠の静寂がお前を待っている」");
#else
                       " whispers, \"Everlasting silence awaits you.\"");
#endif
                break;
            case 20:
                strcat(info,
#ifdef JP
                       "は絶叫した。「防御をやめよ。お前に勝機などないのだ！」");
#else
                       " screams, \"Don't try to defend. You have no chance!\"");
#endif
                break;
            case 21:
                strcat(info,
#ifdef JP
                       "は囁いた。「死は何も損ないはしない」");
#else
                       " whispers, \"Death doesn't hurt. What you feel is life.\"");
#endif
                break;
            case 22:
#ifdef JP
                strcat(info, "は囁いた。「オーブなど存在しないのだ」");
#else
                strcat(info, " whispers, \"The ORB doesn't exist.\"");
#endif
                break;
            case 23:
#ifdef JP
                strcat(info, "は嘆いた。「お前はもう死ぬしかないのだ」");
#else
                strcat(info, " wails, \"Death is your only future.\"");
#endif
                break;
            }
            break;          // end players ghost

        case MONS_PSYCHE:   // insane girl
            switch (random2(20))
            {
            case 0:
#ifdef JP
                strcat(info, "は幸福な笑みを浮かべている。");
#else
                strcat(info, " smiles happily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は狂ったようにクスクス笑っている。");
#else
                strcat(info, " giggles crazily.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は泣いている。");
#else
                strcat(info, " cries.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は少しの間、あなたをじっと見つめた。");
#else
                strcat(info, " stares at you for a moment.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は歌っている。");
#else
                strcat(info, " sings.");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は言った。「お願い。ちょっと早いけど死んで貰える？」");
#else
                       " says, \"Please, could you die a little faster?\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "は言った。「私は悪い娘なの。でもどうにもならないのよ」");
#else
                       " says, \"I'm bad girl. But I can't do anything about it.\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "は絶叫した。「あなたは地域の安全を侵害しているわ！」");
#else
                       " screams, \"YOU ARE VIOLATING AREA SECURITY!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は泣き叫んだ。「私は血も暴力も大嫌いよ」");
#else
                strcat(info, " cries, \"I hate blood and violence.\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "は絶叫した。「平和！お花！自由！そして死せる冒険者！」");
#else
                       " screams, \"Peace! Flowers! Freedom! Dead adventurers!\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "は言った。「とても寂しいの。死体だけが私のお友達」");
#else
                       " says, \"I'm so lonely. Only corpses are my friends.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は泣き叫んだ。「あなた、私のペットを殺したでしょう」");
#else
                strcat(info, " cries, \"You've killed my pet.\"");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "は泣き叫んだ。「私のオーブコレクションが目当てなの！？」");
#else
                       " cries, \"You want to steal my orb collection?!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は奇妙な歌を歌っている。");
#else
                strcat(info, " sings some strange song.");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は涙を溢れさせた。");
#else
                strcat(info, " bursts in tears.");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "は親指をしゃぶっている。");
#else
                strcat(info, " sucks her thumb.");
#endif
                break;
            case 16:
                strcat(info,
#ifdef JP
                       "は囁いた。「私を抱いて。ゾクゾクさせて。キスして。殺して」");
#else
                       " whispers, \"Hold me, thrill me, kiss me, kill me.\"");
#endif
                break;      //(c) U2 ?
            case 17:
#ifdef JP
                strcat(info, "は言った。「あなたを殺して、おうちに帰してあげる」");
#else
                strcat(info, " says, \"I'll kill you and take you home.\"");
#endif
                break;
            case 18:
                strcat(info,
#ifdef JP
                       "は叫んだ。「ええ、わたしは気が変かも。でも誰が気にして？」");
#else
                       " shouts, \"Well, maybe I'm nutty, but who cares?\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "は叫んだ。「私、あなたが哀しんでくれるといいな」");
#else
                       " shouts, \"I hope that you are sorry for that.\"");
#endif
                break;
            }
            break;          // end Psyche

        case MONS_DONALD:   // adventurers hating competition
        case MONS_WAYNE:
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「おうちに帰んな！」");
#else
                strcat(info, " screams, \"Return home!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「オーブはこの俺のものだ！」");
#else
                strcat(info, " screams, \"The Orb is mine!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「お前の宝を俺に全部よこせ！」");
#else
                strcat(info, " screams, \"Give me all your treasure!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は絶叫した「オーブは決してお前なんかの物にならない！」");
#else
                strcat(info, " screams, \"You will never get the Orb!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は絶叫した。「俺がここに一番乗りしたんだ！」");
#else
                strcat(info, " screams, \"I was here first!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "はしかめ面をした。");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は非常に憤慨した様子だ。");
#else
                strcat(info, " looks very upset.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は絶叫した。「逃げないなら死ね！」");
#else
                strcat(info, " screams, \"Get away or die!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「俺を倒さなければ進めないぜ！」");
#else
                strcat(info, " screams, \"First you have to pass me!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「お前は虫が好かねえ！」");
#else
                strcat(info, " screams, \"I hate you!\"");
#endif
                break;
            }
            break;          // end Donald

        case MONS_MICHAEL:  // spellcaster who wanted to be alone
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は怒り心頭の様子だ。");
#else
                strcat(info, " looks very angry.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "はしかめ面をした。");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「俺は一人になりたいんだ！」");
#else
                strcat(info, " screams, \"I want to be alone!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は言った。「お前は本当に鬱陶しい奴だな」");
#else
                strcat(info, " says, \"You are really nuisance.\"");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺は一人になりたいんだ。だからお前は……」");
#else
                       " screams, \"I wanted to be alone. And you...\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は絶叫した。「立ち去れ！じゃなきゃいっそ死ね！」");
#else
                strcat(info, " screams, \"Get away! Or better yet, die!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は奇妙な言葉を呟いた。");
#else
                strcat(info, " mumbles some strange words.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 7:
#ifdef JP
                strcat(info, "はあなたを指差した。");
#else
                strcat(info, " points at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
                canned_msg(MSG_YOU_RESIST);
                return (true);

            case 8:
#ifdef JP
                strcat(info, "は怒りに震えている。");
#else
                strcat(info, " shakes with wrath.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "はポーションを飲んだ。");
#else
                strcat(info, " drinks a potion.");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "はでたらめな身振りをした。");
#else
                strcat(info, " gestures wildly.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;
            }
            break;          // end Michael

        case MONS_ERICA:    // wild tempered adventuress
            switch (random2(12))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「速やかに死ぬ？じわじわいく？」");
#else
                strcat(info, " screams, \"Do you want it fast or slow?\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は腹を立てている様子だ。");
#else
                strcat(info, " looks angry.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "はポーションを飲んだ。");
#else
                strcat(info, " drinks a potion.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は言った。「あたしはお前なんかよりずっと優れてる」");
#else
                strcat(info, " says, \"I'm so much better than you.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は言った。「あたしの殺しは迅速かつ完璧さ」");
#else
                       " says, \"Fast and perfect. Such is my way of killing.\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は絶叫した。「さっさとしな！死神が待ちかねているよ！」");
#else
                strcat(info, " screams, \"Hurry! Death awaits!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は荒々しい笑い声を上げた。");
#else
                strcat(info, " laughs wildly.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「あれのありかは言うもんか！」");
#else
                strcat(info, " screams, \"I'll never tell where it is!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「お前は絶対あれを手に入れられないよ！」");
#else
                strcat(info, " screams, \"You'll never get it!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「ここに来るなんて自殺同然だね！」");
#else
                strcat(info, " screams, \"Coming here was suicide!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "は言った。「あたしは戦いが好きだ。でも殺しはもっと好きだ」");
#else
                       " says, \"I love to fight,  but killing is better.\"");
#endif
                break;
            }
            break;          // end Erica

        case MONS_JOSEPHINE:        // ugly old witch looking for somone to kill
            switch (random2(13))
            {
            case 0:
            case 1:
            case 2:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 3:
            case 4:
#ifdef JP
                strcat(info, "は絶叫した。「殺すよ！」");
#else
                strcat(info, " screams, \"I will kill you!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は歯を軋ませた。");
#else
                strcat(info, " grinds her teeth.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は悪意を剥き出しにしている。");
#else
                strcat(info, " grins malevolently.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は気の触れたように笑った。");
#else
                strcat(info, " laughs insanely.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "は絶叫した。「お前のための取って置きがあるよ！」");
#else
                       " screams, \"I have something special for you!\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "は絶叫した。「お前の首を家の飾りにしてやる！」");
#else
                       " screams, \"I'll use your head as decoration in my hut!\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は言った。「お前の皮を敷物にしてやるよ」");
#else
                strcat(info, " says, \"I'll make a rug of your skin.\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "は言った。「断頭はいかが？」");
#else
                strcat(info, " says, \"How about some decapitation?\"");
#endif
                break;
            }
            break;          // end Josephine

        case MONS_HAROLD:  // middle aged man, hired to kill you. He is in a hurry.
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は苛立った様子だ。");
#else
                strcat(info, " looks nervous.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「さっさと死ね！」");
#else
                strcat(info, " screams, \"Hurry up!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「さっさと死んでくれないか？」");
#else
                strcat(info, " screams, \"Could you die faster?\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "は言った。「じっとしてろ。殺してやるから」");
#else
                       " says, \"Stand still. I'm trying to kill you.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は言った。「速やかに死ね！」");
#else
                strcat(info, " says, \"I hope you die soon!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "は言った。「２・３発で御仕舞いさ」");
#else
                       " says, \"Only few hits and it's over.\".");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は言った。「分かってるだろう。俺は急いでるんだ」");
#else
                strcat(info, " says, \"You know, I'm in a hurry.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「すぐに息の根を止めてやる！」");
#else
                strcat(info, " screams, \"I'll finish you soon!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「死を引き伸ばすのはやめろ！」");
#else
                strcat(info, " screams, \"Don't delay it.\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は言った。「理由は知ったことじゃない。俺は殺すだけだ」" );
#else
                strcat(info, " says, \"Mine is not to reason why.  Mine's to do, yours to die.\"" );
#endif
            }
            break;          // end Harold

        // skilled warrior looking for some fame. More deads = more fame
        case MONS_NORBERT:
            switch (random2(13))
            {
            case 0:
#ifdef JP
                strcat(info, "は幸福な笑みを浮かべている。");
#else
                strcat(info, " smiles happily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「死ね、怪物め！」");
#else
                strcat(info, " screams, \"Die, monster!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「俺はヒーローだ！」");
#else
                strcat(info, " screams, \"I'm a hero!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は叫んだ。「やった！新記録だ！」");
#else
                strcat(info, " shouts, \"YES! Another notch!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は言った。「気の毒だが、お前の首は不細工なトロフィーになる」");
#else
                strcat(info, " says, \"A pity your head will make such an ugly trophy.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は絶叫した。「祈るがいい。お前はすぐに死ぬのだから」");
#else
                       " screams, \"Pray, because you'll die soon!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                      "は尋ねた。「遺言は書いてあるか？必要になるぞ」");
#else
                      " asks \"Did you write a will? You should.\".");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "は言った。「俺は醜いモンスターを殺すのが大好きだ。お前とかな」");
#else
                       " says, \"I love killing ugly monsters like you.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「血と殺戮を！」");
#else
                strcat(info, " screams, \"Blood and destruction!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「知っての通り、俺の手にかかって死ぬのは名誉なことだ」");
#else
                strcat(info, " says, \"You know, it's honour to die by my hand.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は叫んだ。「死ぬ時間が来たぞ！」");
#else
                strcat(info, " shouts, \"Your time has come!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "は言った。「気の毒だが、お前には勝算などない」");
#else
                       " says, \"I'm sorry but you don't have a chance.\"");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "は言った。「また死にかけた怪物か……今日はツイている！」");
#else
                       " says, \"Another dead monster... It must be my lucky day!\"");
#endif
                break;
            }
            break;          // end Norbert

        case MONS_JOZEF:    // bounty hunter
            switch (random2(14))
            {
            case 0:
#ifdef JP
                strcat(info, "は満ち足りた様子だ。");
#else
                strcat(info, " looks satisfied.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「ようやくお前を見つけたぞ！」");
#else
                strcat(info, " screams, \"At last I found you!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は叫んだ。「お前の首には金貨500枚がかかっている！」");
#else
                strcat(info, " shouts, \"I'll get 500 for your head!\"");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "は言った。「お前はとても賞金に見合った奴には見えないな」");
#else
                       " says, \"You don't look worth for that money.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は言った。「お前に恨みはない。金のためだ！」");
#else
                       " says, \"It's nothing personal. I'm paid for it!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "は尋ねた。「遺言状は書いてあるか？必要になるぞ」");
#else
                       " asks \"Did you write a testament? You should.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は言った。「お前が『");
#else
                strcat(info, " says, \"You are ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "』だな？違うか？");
#else
                strcat(info, ", aren't you?.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は言った。「お前『");
#else
                strcat(info, " says, \"I suppose that you are ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "』だろう。人違いならすまん」");
#else
                strcat(info, ". Sorry, if I'm wrong.\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「『");
#else
                strcat(info, " says, \"One dead ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "』を殺した者に金貨500枚。そういう約束だ」");
#else
                strcat(info, ", 500 gold pieces. It's in my contract.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は叫んだ。「死ぬ時が来たぞ！」");
#else
                strcat(info, " shouts, \"Your time has come!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "は言った。「俺の仕事は時には非常に面白い。時にはな……」");
#else
                       " says, \"My job is sometimes very exciting. Sometimes...\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "は言った。「俺は金に見合った働きはしてるぜ」");
#else
                strcat(info, " says, \"I think I deserve my money.\"");
#endif
                break;
            case 13:
                strcat(info,
#ifdef JP
                       "は絶叫した。「死ねよ！今日は他にも仕事を請け負ってるんだ」");
#else
                       " screams, \"Die! I've got more contracts today.\"");
#endif
                break;
            }
            break;          // end Jozef

        case MONS_AGNES:    // she is trying to get money and treasure
            switch (random2(10))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「有り金全部よこしな！」");
#else
                strcat(info, " screams, \"Give me all your money!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「宝は全部あたしの物だ！」");
#else
                strcat(info, " screams, \"All treasure is mine!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「あたしの金はやらないよ！」");
#else
                strcat(info, " screams, \"You'll never get my money!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "は絶叫した。「持ってる物全部よこして失せな！」");
#else
                       " screams, \"Give me everything and get away!\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は言った。「新しいローブが欲しいんだ。お前の金で買うよ」");
#else
                       " says, \"I need new robe. I'll buy it from your money.\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "は絶叫した。「お前の指輪が欲しい！首飾りも！あと……全部よこせ！」");
#else
                       " screams, \"I want your rings! And amulets! And... EVERYTHING!\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "は絶叫した。「あたし、お前みたいな薄汚い冒険者は大嫌いだよ」");
#else
                       " screams, \"I hate dirty adventurers like you.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は言った。「どうしたらそんな不恰好な着こなしができるのさ？」");
#else
                strcat(info, " says, \"How can you wear that ugly dress?\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「死ね、ケダモノ！」");
#else
                strcat(info, " screams, \"Die, beast!\"");
#endif
                break;
            }
            break;          // end Agnes

        case MONS_MAUD:     // warrior princess looking for sword "Entarex"
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「降伏か、さもなくば死を！」");
#else
                strcat(info, " screams, \"Submit or die!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「私に『エンタレクス』を渡せ！」");
#else
                strcat(info, " screams, \"Give me \"Entarex\"!\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "は絶叫した。「『エンタレクス』さえ渡せば生かしておいてやろう！」");
#else
                       " screams, \"If you give me \"Entarex\", I'll let you live!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "はしかめ面をした。");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は憤慨している様子だ。");
#else
                strcat(info, " looks upset.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は絶叫した。「お前では私の力に太刀打ちできまい！」");
#else
                strcat(info, " screams, \"You can't face my power!\"");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "は絶叫した。「私にかの剣をよこせ！今すぐにだ！」");
#else
                       " screams, \"Give me that sword! Immediately!\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "は絶叫した。「命か『エンタレクス』か！どちらかを差し出せ」");
#else
                       " screams, \"Your life or \"Entarex\"! You must choose.\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「それを所望だ！」");
#else
                strcat(info, " screams, \"I want it!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「死ぬがいい、この盗っ人め！」");
#else
                strcat(info, " screams, \"Die, you thief!\"");
#endif
                break;
            case 10:
                // needed at least one in here to tie to the amnesia
                // scroll reference -- bwr
#ifdef JP
                strcat(info, "は尋ねた。「死にたいなら私に任せるがよいぞ？」");
#else
                strcat(info, " asks \"Will you think of me as you die?\"");
#endif
                break;
            }
            break;          // end Maud

        // wizard looking for bodyparts as spell components
        case MONS_FRANCIS:
            switch (random2(15))
            {
            case 0:
                strcat(info,
#ifdef JP
                       "は言った。「いい眼を持ってるな。そいつをくれよ」");
#else
                       " says, \"You've nice eyes. I could use them.\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は言った。「すまないな、だが君の頭部が必要なんだ」");
#else
                strcat(info, " says, \"Excuse me, but I need your head.\"");
#endif
                    break;
            case 2:
#ifdef JP
                strcat(info, "は言った。「君の内臓が２～３個必要なだけだから！」");
#else
                strcat(info, " says, \"I only need a few of your organs!\"");
#endif
                    break;
            case 3:
#ifdef JP
                strcat(info, "は状況について思いを巡らしている。");
#else
                strcat(info, " ponders the situation.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は手術用具を探している。");
#else
                strcat(info, " looks for scalpel.");
#endif
                break;

            case 5:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "の手は柔らかい光に輝きだした。");
#else
                strcat(info, "'s hands started to glow with soft light.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 6:
#ifdef JP
                strcat(info, "は言った。「ちっとも痛くないからね」");
#else
                strcat(info, " says, \"This won't hurt a bit.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "はあなたに何かを投げた。");
#else
                strcat(info, " throws something at you.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は言った。「研究素材に君が必要なんだ！」");
#else
                strcat(info, " says, \"I want you in my laboratory!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「ちょっとした解剖はいかがかな？」");
#else
                strcat(info, " says, \"What about little dissection?\"");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "は言った。「君のための取って置きがあるんだ」");
#else
                       " says, \"I have something special for you.\"");
#endif
                    break;
            case 11:
                strcat(info,
#ifdef JP
                       "は絶叫した。「動かないで！今、耳を切り取るから！」");
#else
                       " screams, \"Don't move! I want to cut your ear!\"");
#endif
                break;
            case 12:
                strcat(info,
#ifdef JP
                       "は言った。「心臓ならどうだい？貰っていいかな？」");
#else
                       " says, \"What about your heart? Do you need it?\"");
#endif
                break;
            case 13:
                strcat(info,
#ifdef JP
                       "は言った。「知ってたかい？死体ってのは貴重な天然資源なんだ」");
#else
                       " says, \"Did you know that corpses are an important natural resource?\"");
#endif
                break;
            case 14:
                strcat(info,
#ifdef JP
                       "は言った。「安心してくれ。私は必要な部分しか切らないから」");
#else
                       " says, \"Don't worry, I'll only take what I need.\"");
#endif
                break;
            }
            break;          // end Francis

        case MONS_RUPERT:   // crazy adventurer
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は言った。「おまえ怪物だな？そうだろ？」");
#else
                strcat(info, " says, \"You are a monster, aren't you?\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「死ね、怪物め！」");
#else
                strcat(info, " screams, \"Die, monster!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「俺に聖杯をよこせ！」");
#else
                strcat(info, " screams, \"Give me Holy Grail!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は絶叫した。「赤を！青じゃなく！」");
#else
                strcat(info, " screams, \"Red!  No, blue!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は混乱している様子だ。");
#else
                strcat(info, " looks confused.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は興奮している様子だ。");
#else
                strcat(info, " looks excited.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は叫んだ。「俺は偉大なる最強のヒーローだ！」");
#else
                strcat(info, " shouts, \"I'm great and powerful hero!\"");
#endif
                break;
            case 7:
                strcat(info,
#ifdef JP
                       "は絶叫した。「覚悟しろ！お前を殺してやる！」");
#else
                       " screams, \"Get ready! I'll kill you! Or something like it...\"");
#endif
                break;
            case 8:
                strcat(info,
#ifdef JP
                       "は言った。「ママがいつも言ってた。皆殺しにしろって」");
#else
                       " says, \"My Mom always said, kill them all.\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "は絶叫した。「おまえが可愛い怪物たちを殺したのか、この人殺し！」");
#else
                       " screams, \"You killed all those lovely monsters, you murderer!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「バンザイ！」");
#else
                strcat(info, " screams, \"Hurray!\"");
#endif
                break;
            }
            break;          // end Rupert

        case MONS_NORRIS:   // enlighten but crazy man
            switch (random2(24))
            {
            case 0:
#ifdef JP
                strcat(info, "は吟じた。「ハーレラーマ、ハーレクリシュナ！」");
#else
                strcat(info, " sings \"Hare Rama, Hare Krishna!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "はあなたに笑いかけた。");
#else
                strcat(info, " smiles at you.");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "は言った。「死後にお前は内なる平安を見出すだろう」");
#else
                       " says, \"After death you'll find inner peace.\"");
#endif
                break;
            case 3:
                strcat(info,
#ifdef JP
                       "は言った。「生こそまさに苦しみだ。私がお前を救ってやろう」");
#else
                       " says, \"Life is just suffering. I'll help you.\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は平安の霊気に包まれている。");
#else
                strcat(info, " is surrounded with aura of peace.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は平穏な様子だ。");
#else
                strcat(info, " looks very balanced.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は言った。「抵抗はやめよ。お前のためにやっているのだ」");
#else
                strcat(info, " says, \"Don't resist. I'll do it for you.\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は絶叫した。「涅槃に入滅するのだ！今すぐに！」");
#else
                strcat(info, " screams, \"Enter NIRVANA! Now!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は言った。「死こそまさに解脱なのだ！」");
#else
                strcat(info, " says, \"Death is just a liberation!\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "は言った。「死による自由を味わえ。素晴らしいぞ」");
#else
                       " says, \"Feel free to die. It's great thing.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は言った。「ああ、蓮華の中の宝珠よ、永遠なれ！」");
#else
                strcat(info, " says, \"OHM MANI PADME HUM!\"");
#endif
                break;

            case 11:
#ifdef JP
                strcat(info, "は何か真言を呟いた。");
#else
                strcat(info, " mumbles some mantras.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 12:
#ifdef JP
                strcat(info, "は言った。「深く呼吸せよ」");
#else
                strcat(info, " says, \"Breath deeply.\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は絶叫した。「愛！永続にして普遍なる愛！」");
#else
                strcat(info, " screams, \"Love! Eternal love!\"");
#endif
                break;
            case 14:
                strcat(info,
#ifdef JP
                       "は絶叫した。「平安！お前に永遠の平安を授けよう！」");
#else
                       " screams, \"Peace! I bring you eternal peace!\"");
#endif
                break;
            case 15:
                strcat(info,
#ifdef JP
                       "は嘆いた。「啓発とはかような責務なのだ」");
#else
                       " sighs \"Enlightenment is such responsibility.\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "は寛いだ様子だ。");
#else
                strcat(info, " looks relaxed.");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "は絶叫した。「お前の魂を解き放て！死ぬのだ！」");
#else
                strcat(info, " screams, \"Free your soul! Die!\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "は絶叫した。「煩悩を振り払え！」");
#else
                strcat(info, " screams, \"Blow your mind!\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "は言った。「オーブなど伝説にすぎぬ。さような物は忘れよ」");
#else
                       " says, \"The Orb is only a myth. Forget about it.\"");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "は言った。「世の全ては幻だ」");
#else
                strcat(info, " says, \"It's all maya.\"");
#endif
                break;
            case 21:
#ifdef JP
                strcat(info, "は言った。「解脱せよ！」");
#else
                strcat(info, " says, \"Drop out!\"");
#endif
                break;
            case 22:
                strcat(info,
#ifdef JP
                       "は吟じた。「今こそ平安と自由を！平安と自由を今こそ！」");
#else
                       " sings, \"Peace now, freedom now! Peace now, freedom now!\"");
#endif
                break;
            case 23:
#ifdef JP
                strcat(info, "は言った。「これが戦闘瞑想と呼ばれるものだ」");
#else
                strcat(info, " says, \"This is called Combat Meditation.\"");
#endif
                break;
            }
            break;          // end Norris

        case MONS_MARGERY:  // powerful sorceress, guarding the ORB
            switch (random2(22))
            {
            case 0:
#ifdef JP
                strcat(info, "は言った。「あなたは死にました」");
#else
                strcat(info, " says, \"You are dead.\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は非常に自信に満ちた様子だ。");
#else
                strcat(info, " looks very self-confident.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「あなたは処罰されるべきです！」");
#else
                strcat(info, " screams, \"You must be punished!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は絶叫した。「私の力に太刀打ちできまい！」");
#else
                strcat(info, " screams, \"You can't withstand my power!\"");
#endif
                break;

            case 4:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "は力の霊気に包まれている。");
#else
                strcat(info, " is surrounded with aura of power.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 5:
#ifdef JP
                strcat(info, "の目は赤い光に輝きだした。");
#else
                strcat(info, "'s eyes starts to glow with a red light.");
#endif
                break;
            case 6:
                strcat(info,
#ifdef JP
                       "の目は緑の光に輝きだした。");
#else
                       "'s eyes starts to glow with a green light.");
#endif
                    break;
            case 7:
#ifdef JP
                strcat(info, "の目は青い光に輝きだした。");
#else
                strcat(info, "'s eyes starts to glow with a blue light.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「全ての侵入者に死を！」");
#else
                strcat(info, " screams, \"All trespassers must die!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「死になさい！」");
#else
                strcat(info, " says, \"Die!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「あなたには私をやり過ごすしかないでしょう！」");
#else
                strcat(info, " screams, \"You'll have to get past me!\"");
#endif
                break;

            case 11:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "は一瞬だけ透明になった。");
#else
                strcat(info, " becomes transparent for a moment.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 12:
#ifdef JP
                strcat(info, "は手を振りかざした。");
#else
                strcat(info, " gestures.");
#endif
                msg_type = MSGCH_MONSTER_SPELL;
                break;

            case 13:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat(info, "の手は輝きだした。");
#else
                strcat(info, "'s hands start to glow.");
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;

            case 14:
#ifdef JP
                strcat(info, "は絶叫した。「エルギカンテグ・レズタハゥ！」");
#else
                strcat(info, " screams, \"Ergichanteg reztahaw!\"");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは気分が非常に悪くなった。", MSGCH_WARN);
#else
                mpr("You feel really bad.", MSGCH_WARN);
#endif
                return (true);

            case 15:
#ifdef JP
                strcat(info, "は絶叫した。「あなたはもうお仕舞いです！」");
#else
                strcat(info, " screams, \"You are doomed!\"");
#endif
                break;
            case 16:
#ifdef JP
                strcat(info, "は絶叫した。「あなたに助かる途はありません」");
#else
                strcat(info, " screams, \"Nothing can help you.\"");
#endif
                break;
            case 17:
#ifdef JP
                strcat(info, "は絶叫した。「『死神』が私の通り名です！」");
#else
                strcat(info, " screams, \"Death is my middle name!\"");
#endif
                break;

            case 18:
#ifdef JP
                strcat(info, "は手を振りかざした。");
#else
                strcat(info, " gestures.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは絶望的な気分になった。", MSGCH_WARN);
#else
                mpr("You feel doomed.", MSGCH_WARN);
#endif
                return (true);

            case 19:
#ifdef JP
                strcat(info, "は手を振りかざした。");
#else
                strcat(info, " gestures.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは弱くなったような気がした。", MSGCH_WARN);
#else
                mpr("You feel weakened.", MSGCH_WARN);
#endif
                return (true);

            case 20:
#ifdef JP
                strcat(info, "はあなたに向けて紫色の粉を投げた。");
#else
                strcat(info, " throws some purple powder towards you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは呪われたような気がした。", MSGCH_WARN);
#else
                mpr("You feel cursed.", MSGCH_WARN);
#endif
                return (true);

            case 21:
                strcat(info,
#ifdef JP
                       "は絶叫した。「オーブなど御伽噺！どの道あなたは殺しますが！」");
#else
                       " screams, \"The ORB is only a tale, but I will kill you anyway!");
#endif
                break;
            }
            break;          // end Margery

        case MONS_IJYB:     // twisted goblin
            switch (random2(14))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「しね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「おれ、おまえころす！」");
#else
                strcat(info, " screams, \"Me kill you!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「おれ、おまえよりつよい！」");
#else
                strcat(info, " screams, \"Me stronger than you!\"");
#endif
                break;
            case 3:
            case 4:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は絶叫した。「それ、ぜんぶおれの！」");
#else
                strcat(info, " screams, \"It's all mine!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は絶叫した。「あっちいけ！」");
#else
                strcat(info, " screams, \"Get away!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は絶叫した。「ここおれの！ぜんぶおれの！」");
#else
                strcat(info, " screams, \"Level is mine! All mine!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「おまえのあたま、きる！」");
#else
                strcat(info, " screams, \"I cut your head off!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「おれ、おまえころして、おおよろこび！」");
#else
                strcat(info, " screams, \"I dance on your bones!\"");
#endif
                    break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「おれ、すごくおこってる！」");
#else
                strcat(info, " screams, \"Me very upset!\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は絶叫した。「おまえいじわる！おおいじわる！」");
#else
                strcat(info, " screams, \"You nasty! Big nasty!\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "は絶叫した。「だめ！だめ、だめ、だめ、だめ！」");
#else
                strcat(info, " screams, \"No! No, no, no, no!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は絶叫した。「おれ、おまえすきくない！」");
#else
                strcat(info, " screams, \"I no like you!\"");
#endif
                break;
            }
            break;          // end IJYB

        case MONS_BLORK_THE_ORC:    // unfriendly orc
            switch (random2(21))
            {
            case 0:
#ifdef JP
                strcat(info, "は絶叫した。「俺、お前みたいなの、嫌い！」");
#else
                strcat(info, " screams, \"I don't like you!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「俺、お前を殺す！」");
#else
                strcat(info, " screams, \"I'm going to kill you!\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺、お前よりずっと強い！」");
#else
                       " screams, \"I'm much stronger than you!\"");
#endif
                break;
            case 3:
            case 4:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "はしかめ面をした。");
#else
                strcat(info, " frowns.");
#endif
                break;
            case 6:
            case 7:
            case 8:
            case 9:
#ifdef JP
                strcat(info, "は腹を立てている様子だ。");
#else
                strcat(info, " looks angry.");
#endif
                break;
            case 10:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺、お前の脳みそ食う！そしたら吐いちまう！」");
#else
                       " screams, \"I'll eat your brain! And then I'll vomit it back up!\"");
#endif
                break;
            case 11:
                strcat(info,
#ifdef JP
                       "は絶叫した。「お前、俺が見たモンスターで、一番ブサイク！」");
#else
                       " screams, \"You are the ugliest creature I've ever seen!\"");
#endif
                break;
            case 12:
#ifdef JP
                strcat(info, "は絶叫した。「俺、お前の頭、切り取る！」");
#else
                strcat(info, " screams, \"I'll cut your head off!\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は絶叫した。「俺、お前の足、折る！」");
#else
                strcat(info, " screams, \"I'll break your legs!\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は絶叫した。「俺、お前の腕、折る！」");
#else
                strcat(info, " screams, \"I'll break your arms!\"");
#endif
                break;
            case 15:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺、お前のあばら、みんな折る！一本ずつ！」");
#else
                       " screams, \"I'll crush all your ribs! One by one!\"");
#endif
                break;
            case 16:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺、お前の皮で、クローク作る！」");
#else
                       " screams, \"I'll make a cloak from your skin!\"");
#endif
                    break;
            case 17:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺、お前のはらわた、うちに飾る！」");
#else
                       " screams, \"I'll decorate my home with your organs!\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 19:
                strcat(info,
#ifdef JP
                       "は絶叫した。「俺、お前の血で、そこらじゅう塗りたくる！」");
#else
                       " screams, \"I'll cover the dungeon with your blood!\"");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "は絶叫した。「俺、お前の血飲む！すぐに！」");
#else
                strcat(info, " screams, \"I'll drink your blood! Soon!\"");
#endif
                break;
            }
            break;          // end Blork

        case MONS_EROLCHA:  // ugly ogre
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべようと努力した。");
#else
                strcat(info, " tries to grin evilly.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「食べる！」");
#else
                strcat(info, " screams, \"Eat!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「立ってろ！エロルカがぶつ！」");
#else
                strcat(info, " screams, \"Stand! Erolcha hit you!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は絶叫した。「血ぃ！」");
#else
                strcat(info, " screams, \"Blood!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は絶叫した。「エロルカ、お前殺す！」");
#else
                strcat(info, " screams, \"Erolcha kill you!\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は絶叫した。「エロルカ、お前の頭つぶす！」");
#else
                       " screams, \"Erolcha crush your head!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は咆哮した。");
#else
                strcat(info, " roars.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は腹をグゥグゥ鳴らしている。");
#else
                strcat(info, " growls.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「昼メシ！」");
#else
                strcat(info, " screams, \"Lunch!\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は絶叫した。「エロルカ、お前殺せてしあわせ！」");
#else
                strcat(info, " screams, \"Erolcha happy to kill you!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「エロルカ怒った！」");
#else
                strcat(info, " screams, \"Erolcha angry!\"");
#endif
                break;
            }
            break;          // end Erolcha

        case MONS_URUG:     // orc hired to kill you
            switch (random2(11))
            {
            case 0:
#ifdef JP
                strcat(info, "は邪悪な笑みを浮かべた。");
#else
                strcat(info, " grins evilly.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は絶叫した。「死ね！」");
#else
                strcat(info, " screams, \"Die!\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は絶叫した。「お前を殺す！今！」");
#else
                strcat(info, " screams, \"I'm going to kill you! Now!\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は絶叫した。「血と殺戮を！」");
#else
                strcat(info, " screams, \"Blood and destruction!\"");
#endif
                break;
            case 4:
                strcat(info,
#ifdef JP
                       "は嘲った。「無実だと？どっちみちお前は殺すんだよ」");
#else
                       " sneers, \"Innocent? I'll kill you anyway.\"");
#endif
                break;
            case 5:
                strcat(info,
#ifdef JP
                       "は絶叫した。「お前を始末して、銀貨30枚頂きたぜ！」");
#else
                       " screams, \"I'll get 30 silver pieces for your head!\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は咆哮した。");
#else
                strcat(info, " roars.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は血に飢えた雄たけびをあげた。");
#else
                strcat(info, " howls with blood-lust.");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は絶叫した。「お前はもう死んでいるのさ」");
#else
                strcat(info, " screams, \"You are already dead.\"");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「お前は『");
#else
                strcat(info, " says, \"Maybe you aren't ");
#endif
                strcat(info, you.your_name);
#ifdef JP
                strcat(info, "』じゃないかもな。まあどうでもいいが」");
#else
                strcat(info, ". It doesn't matter.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「俺は血を見るのが大好きだ！」");
#else
                strcat(info, " screams, \"I love blood!\"");
#endif
                break;
            }
            break;          // end Urug

        case MONS_SNORG:    // troll
            switch (random2(16))
            {
            case 0:
#ifdef JP
                strcat(info, "は歯を剥いてにたりとした。");
#else
                strcat(info, " grins.");
#endif
                break;
            case 1:
            case 2:
            case 3:
#ifdef JP
                strcat(info, "は凄まじい体臭を放っている。");
#else
                strcat(info, " smells terrible.");
#endif
                break;
            case 4:
            case 5:
            case 6:
#ifdef JP
                strcat(info, "は非常に腹を空かせた様子だ。");
#else
                strcat(info, " looks very hungry.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は絶叫した。「おやつ！」");
#else
                strcat(info, " screams, \"Snack!\"");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "は咆哮した。");
#else
                strcat(info, " roars.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った。「食いもの！」");
#else
                strcat(info, " says, \"Food!\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は絶叫した。「スノーグ、腹減った！」");
#else
                strcat(info, " screams, \"Snorg hungry!\"");
#endif
                break;
            case 11:
#ifdef JP
                strcat(info, "は絶叫した。「スノーグ、すごく、すごく腹減った！」");
#else
                strcat(info, " screams, \"Snorg very, very hungry!\"");
#endif
            case 12:
#ifdef JP
                strcat(info, "は言った。「スノーグ、お前食らう」");
#else
                strcat(info, " says, \"Snorg eat you.\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は言った。「お前、食いもん？」");
#else
                strcat(info, " says, \"You food?\"");
#endif
                break;
            case 14:
#ifdef JP
                strcat(info, "は言った。「うまそう、うまそう」");
#else
                strcat(info, " says, \"Yum, yum.\"");
#endif
                break;
            case 15:
#ifdef JP
                strcat(info, "はげっぷした。");
#else
                strcat(info, " burps.");
#endif
                break;
            }
            break;          // end Snorg

        case MONS_XTAHUA:   // ancient dragon
            switch (random2(13))
            {
            case 0:
#ifdef JP
                strcat(info, "は咆哮した。「死ね、小さき者よ！」");
#else
                strcat(info, " roars, \"DIE,  PUNY ONE!\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は唸った。「お前にはもううんざりだ」");
#else
                strcat(info, " growls, \"YOU BORE ME SO.\"");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "は怒号した。「お前ではおやつにしかならんな」");
#else
                strcat(info, " rumbles, \"YOU'RE BARELY A SNACK.\"");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "は咆哮した。「煩わしいのは我慢がならん！」");
#else
                strcat(info, " roars, \"I HATE BEING BOTHERED!\"");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は咆哮した。「お前が美味だと良いのだが」");
#else
                strcat(info, " roars, \"I HOPE YOU'RE TASTY!\"");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "は咆哮した。「ハハン！残忍な冒険者というわけか」");
#else
                strcat(info, " roars, \"BAH!  BLOODY ADVENTURERS.\"");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は咆哮した。「我が怒りを受けよ！」");
#else
                strcat(info, " roars, \"FACE MY WRATH!\"");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "はあなたをギロリと睨んだ。");
#else
                strcat(info, " glares at you.");
#endif
                break;
            case 8:
                strcat(info,
#ifdef JP
                       "は咆哮した。「ここに来たのが運の尽きだ！」");
#else
                       " roars, \"COMING HERE WAS YOUR LAST MISTAKE!\"");
#endif
                break;
            case 9:
                strcat(info,
#ifdef JP
                       "は咆哮した。「私は今まで何百もの冒険者を屠ってきたのだ！」");
#else
                       " roars, \"I'VE KILLED HUNDREDS OF ADVENTURERS!\"");
#endif
                break;
            case 10:
            case 11:
            case 12:
#ifdef JP
                strcat(info, "は凄まじい咆哮をあげた。");
#else
                strcat(info, " roars horribly.");
#endif
                mpr(info, MSGCH_TALK);
#ifdef JP
                mpr("あなたは恐ろしくなった。", MSGCH_WARN);
#else
                mpr("You are afraid.", MSGCH_WARN);
#endif
                return (true);
            }
            break;          // end Xtahua

        case MONS_BORIS:    // ancient lich
            switch (random2(24))
            {
            case 0:
#ifdef JP
                strcat(info, "は言った。「お前を招待した覚えはないが」");
#else
                strcat(info, " says, \"I didn't invite you.\"");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は言った。「お前には我が力が想像もできまい」");
#else
                strcat(info, " says, \"You can't imagine my power.\"");
#endif
                break;
            case 2:
                strcat(info,
#ifdef JP
                       "は言った。「オーブ？お前オーブが望みか？お前の手には入るまいよ」");
#else
                       " says, \"Orb? You want the Orb? You'll never get it.\"");
#endif
                break;

            case 3:
#ifdef JP
                strcat(info, "は言った。「世界と、肉体と、悪魔」");
#else
                strcat(info, " says, \"The world, the flesh, and the devil.\"");
#endif
                break;

            case 4:
#ifdef JP
                strcat(info, "は手を振りかざした。");
#else
                strcat(info, " gestures.");
#endif
                break;

            case 5:
#ifdef JP
                strcat(info, "はあなたを凝視した。");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは衰弱感を味わった。", MSGCH_WARN);
#else
                mpr("You feel drained.", MSGCH_WARN);
#endif
                return (true);

            case 6:
#ifdef JP
                strcat(info, "はあなたを凝視した。");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは弱くなったような気がした。", MSGCH_WARN);
#else
                mpr("You feel weakened.", MSGCH_WARN);
#endif
                return (true);

            case 7:
#ifdef JP
                strcat(info, "はあなたを凝視した。");
#else
                strcat(info, " stares at you.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは恐怖を感じた。", MSGCH_WARN);
#else
                mpr("You feel troubled.", MSGCH_WARN);
#endif
                return (true);

            case 8:
#ifdef JP
                strcat(info, "は言った。「魔術。お前等はこいつを何もわかっちゃいない」");
#else
                strcat(info, " says \"Magic. You know nothing about it.\"");
#endif
                break;

            case 9:
#ifdef JP
                strcat(info, "は言った。「我が力は無限だ」");
#else
                strcat(info, " says, \"My power is unlimited.\"");
#endif
                break;
            case 10:
#ifdef JP
                strcat(info, "は言った。「お前に私は殺せん。私は不滅だ」");
#else
                strcat(info, " says, \"You can't kill me. I'm immortal.\"");
#endif
                break;

            case 11:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたの装備が突然に重量を増したように思われた。", MSGCH_WARN);
#else
                mpr("Your equipment suddenly seems to weigh more.", MSGCH_WARN);
#endif
                return (true);

            case 12:
                strcat(info,
#ifdef JP
                       "は言った。「私は永遠の命の秘密を知っている。お前はどうかね？」");
#else
                       " says, \"I know the secret of eternal life.  Do you?\"");
#endif
                break;
            case 13:
#ifdef JP
                strcat(info, "は言った。「また戻ってくるぞ」");
#else
                strcat(info, " says, \"I'll be back.\"");
#endif
                break;

            case 14:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
                canned_msg( MSG_YOU_RESIST );
                return (true);

            case 15:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたは突然、ぼやけた緑の光に包まれた。", MSGCH_WARN);
#else
                mpr("Suddenly you are surrounded with pale green light.", MSGCH_WARN);
#endif
                return (true);

            case 16:
#ifdef JP
                strcat(info, "は呪文を唱えた。");
#else
                strcat(info, " casts a spell.");
#endif
                mpr(info, MSGCH_MONSTER_SPELL);
#ifdef JP
                mpr("あなたはひどい頭痛を覚えた。", MSGCH_WARN);
#else
                mpr("You have terrible head-ache.", MSGCH_WARN);
#endif
                return (true);

            case 17:
                strcat(info,
#ifdef JP
                       "は言った。「お前の運命が見えるぞ。お前には死が待ち受けている」");
#else
                       " says, \"I know your future. Your future is death.\"");
#endif
                break;
            case 18:
#ifdef JP
                strcat(info, "は言った。「誰が永遠に生きる事など望むのかだと？私がだ」");
#else
                strcat(info, " says, \"Who wants to live forever?  Me.\"");
#endif
                break;
            case 19:
#ifdef JP
                strcat(info, "は嘲りの笑いをあげた。");
#else
                strcat(info, " laughs.");
#endif
                break;
            case 20:
#ifdef JP
                strcat(info, "は言った。「わがしもべの軍勢に加わるがいい」");
#else
                strcat(info, " says, \"Join the legion of my servants.\"");
#endif
                break;
            case 21:
#ifdef JP
                strcat(info, "は言った。「お前にはたった一つの結末しかない。即ち死だ」");
#else
                strcat(info, " says, \"There's only one solution for you. To die.\"");
#endif
                break;
            case 22:
#ifdef JP
                strcat(info, "は言った。「お前が勝利することはあり得ない」");
#else
                strcat(info, " says, \"You can never win.\"");
#endif
                break;
            case 23:
#ifdef JP
                simple_monster_message( monster, "は呪文を唱えた。",
#else
                simple_monster_message( monster, " casts a spell.",
#endif
                                        MSGCH_MONSTER_SPELL );

#ifdef JP
                strcat( info, "は加速した。" );
#else
                strcat( info, " speeds up." );
#endif
                msg_type = MSGCH_MONSTER_ENCHANT;
                break;
            }
            break;          // end BORIS


        case MONS_DEATH_COB:
            if (one_chance_in(2000))
            {
#ifdef JP
                mpr("殺人トウモロコシは駄洒落を吐いた。", MSGCH_TALK);
#else
                mpr("The death cob makes a corny joke.", MSGCH_TALK);
#endif
                return (true);
            }
            return (false);

        case MONS_KILLER_KLOWN:     // Killer Klown - guess!
            switch (random2(10))
            {
            case 0:
#ifdef JP
                strcat(info, "は狂ったようにクスクス笑っている。");
#else
                strcat(info, " giggles crazily.");
#endif
                break;
            case 1:
#ifdef JP
                strcat(info, "は陽気に笑った。");
#else
                strcat(info, " laughs merrily.");
#endif
                break;
            case 2:
#ifdef JP
                strcat(info, "はあなたを差し招いた。");
#else
                strcat(info, " beckons to you.");
#endif
                break;
            case 3:
#ifdef JP
                strcat(info, "はとんぼ返りをした。");
#else
                strcat(info, " does a flip.");
#endif
                break;
            case 4:
#ifdef JP
                strcat(info, "は宙返りをした。");
#else
                strcat(info, " does a somersault.");
#endif
                break;
            case 5:
#ifdef JP
                strcat(info, "はあなたに微笑みかけた。");
#else
                strcat(info, " smiles at you.");
#endif
                break;
            case 6:
#ifdef JP
                strcat(info, "は陽気な気ままさを湛えてにっこり笑った。");
#else
                strcat(info, " grins with merry abandon.");
#endif
                break;
            case 7:
#ifdef JP
                strcat(info, "は血に飢えた雄たけびをあげた。");
#else
                strcat(info, " howls with blood-lust!");
#endif
                break;
            case 8:
#ifdef JP
                strcat(info, "はその舌を突き出した。");
#else
                strcat(info, " pokes out its tongue.");
#endif
                break;
            case 9:
#ifdef JP
                strcat(info, "は言った「こっちへ来て一緒に遊ぼう！」");
#else
                strcat(info, " says, \"Come and play with me!\"");
#endif
                break;
            }
            break;          // end Killer Klown

        default:
            strcat(info,
#ifdef JP
                   "は言った。「君が何を言いたいかわからないんだ。これはバグだよ」");
#else
                   " says, \"I don't know what to say. It's a bug.\"");
#endif
            break;
        }                   // end monster->type - monster type switch
    }                       // end default

    mpr(info, msg_type);
    return true;
}                               // end mons_speaks = end of routine
