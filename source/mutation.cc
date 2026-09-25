/*
 *  File:       mutation.cc
 *  Summary:    Functions for handling player mutations.
 *  Written by: Linley Henzell
 *
 *  Change History (most recent first):
 *
 *      <5>      7/29/00        JDJ             Made give_cosmetic_mutation static
 *      <4>      9/25/99        CDL             linuxlib -> liblinux
 *      <3>      9/21/99        LRH             Added many new scales
 *      <2>      5/20/99        BWR             Fixed it so demonspwan should
 *                                              always get a mutation, 3 level
 *                                              perma_mutations now work.
 *      <1>      -/--/--        LRH             Created
 */

#include "AppHdr.h"
#include "mutation.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef DOS
#include <conio.h>
#endif

//#ifdef LINUX
//#include "liblinux.h"
//#endif

#include "externs.h"

#include "defines.h"
#include "effects.h"
#include "macro.h"
#include "ouch.h"
#include "player.h"
#include "skills2.h"
#include "stuff.h"
#include "transfor.h"
#include "view.h"

int how_mutated(void);
char body_covered(void);
bool perma_mutate(int which_mut, char how_much);

const char *mutation_descrip[][3] = {
#ifdef JP 
    {"あなたは強靭な皮膚を持っている。(AC +1)", "あなたは非常に強靭な皮膚を持っている。(AC +2)",
     "あなたは極めて強靭な皮膚を持っている。(AC +3)"},
#else
    {"You have tough skin (AC +1).", "You have very tough skin (AC +2).",
     "You have extremely tough skin (AC +3)."},
#endif

#ifdef JP 
    {"あなたの筋肉は強い。(STR +", "", ""},
    {"あなたの精神は鋭い。(INT +", "", ""},
    {"あなたは敏捷だ。(DEX +", "", ""},
#else
    {"Your muscles are strong (Str +", "", ""},
    {"Your mind is acute (Int +", "", ""},
    {"You are agile (Dex +", "", ""},
#endif

#ifdef JP 
    {"あなたは緑色の鱗で部分的に覆われている。(AC +1)",
     "あなたは緑色の鎧で大部分を覆われている。(AC +3)",
     "あなたが緑色の鎧で完全に覆われている。(AC +5)"},
#else
    {"You are partially covered in green scales (AC + 1).",
     "You are mostly covered in green scales (AC + 3).",
     "You are covered in green scales (AC + 5)."},
#endif

#ifdef JP 
    {"あなたは厚みのある黒い鱗で部分的に覆われている。(AC +3, DEX -1)",
     "あなたは厚みのある黒い鱗で大部分を覆われている。(AC +6, DEX -2)",
     "あなたは厚みのある黒い鱗で完全に覆われている。(AC +9, DEX -3)"},
#else
    {"You are partially covered in thick black scales (AC + 3, dex - 1).",
     "You are mostly covered in thick black scales (AC + 6, dex - 2).",
     "You are completely covered in thick black scales (AC + 9, dex - 3)."},
#endif

#ifdef JP 
    {"あなたは柔軟な灰色の鱗で部分的に覆われている。(AC +1)",
     "あなたは柔軟な灰色の鱗で大部分を覆われている。(AC +2)",
     "あなたは柔軟な灰色の鱗で完全に覆われている。(AC +3)"},
#else
    {"You are partially covered in supple grey scales (AC + 1).",
     "You are mostly covered in supple grey scales (AC + 2).",
     "You are completely covered in supple grey scales (AC + 3)."},
#endif

#ifdef JP 
    {"あなたは骨の装甲で護られている。(AC +2, DEX -1)",
     "あなたは骨の装甲で護られている。(AC +3, DEX -2)",
     "あなたは骨の装甲で護られている。(AC +4, DEX -3)"},
#else
    {"You are protected by plates of bone (AC + 2, dex - 1).",
     "You are protected by plates of bone (AC + 3, dex - 2).",
     "You are protected by plates of bone (AC + 4, dex - 3)."},
#endif

#ifdef JP 
    {"あなたは緩やかな斥力フィールドに覆われている。(EV +1)",
     "あなたは斥力フィールドに覆われている。(EV +3)",
     "あなたは強力な斥力フィールドに覆われている。(EV +5, 飛来物防御)"},
    {"あなたの肉体は毒に免疫がある。", "あなたの肉体は毒に免疫がある。",
     "あなたの肉体は毒に免疫がある。"},
#else
    {"You are surrounded by a mild repulsion field (ev + 1).",
     "You are surrounded by a moderate repulsion field (ev + 3).",
     "You are surrounded by a strong repulsion field (ev + 5; repel missiles)."},
    {"Your system is immune to poisons.", "Your system is immune to poisons.",
     "Your system is immune to poisons."},
#endif
// 10

#ifdef JP 
    {"あなたの消化器系統は肉の消化に特化している。",
     "あなたの消化器系統は肉の消化に特化している。",
     "あなたは本質的に肉食性だ。"},
#else
    {"Your digestive system is specialised to digest meat.",
     "Your digestive system is specialised to digest meat.",
     "You are primarily a carnivore."},
#endif

#ifdef JP 
    {"あなたは肉をうまく消化できない。", "あなたは肉をうまく消化できない。",
     "あなたは本質的に草食性だ。"},
#else
    {"You digest meat inefficiently.", "You digest meat inefficiently.",
     "You are primarily a herbivore."},
#endif

#ifdef JP 
    {"あなたの皮膚は熱に耐性がある。", "あなたの皮膚は熱にかなりの耐性がある。",
     "あなたの皮膚は熱の影響に対しほぼ免疫である。"},
#else
    {"Your flesh is heat resistant.", "Your flesh is very heat resistant.",
     "Your flesh is almost immune to the effects of heat."},
#endif

#ifdef JP 
    {"あなたの皮膚は冷気に耐性がある。", "あなたの皮膚は冷気にかなりの耐性がある。",
     "あなたの皮膚は冷気の影響に対しほぼ免疫である。"},
#else
    {"Your flesh is cold resistant.", "Your flesh is very cold resistant.",
     "Your flesh is almost immune to the effects of cold."},
#endif

#ifdef JP 
    {"あなたは電気ショックに免疫がある。", "あなたは電気ショックに免疫がある。",
     "あなたは電気ショックに免疫がある。"},
#else
    {"You are immune to electric shocks.", "You are immune to electric shocks.",
     "You are immune to electric shocks."},
#endif

#ifdef JP 
    {"あなたの自然治癒の速度は著しく速い。",
     "あなたは非常に速く治癒する。",
     "あなたは再生能力がある。"},
#else
    {"Your natural rate of healing is unusually fast.",
     "You heal very quickly.",
     "You regenerate."},
#endif

#ifdef JP 
    {"あなたの新陳代謝は速い。", "あなたの新陳代謝は非常に速い。",
     "あなたの新陳代謝はとてつもなく速い。"},
#else
    {"You have a fast metabolism.", "You have a very fast metabolism.",
     "Your metabolism is lightning-fast."},
#endif

#ifdef JP 
    {"あなたの新陳代謝は遅い。", "あなたの新陳代謝は遅い。",
     "あなたは食料を食べる必要がほとんどない。"},
#else
    {"You have a slow metabolism.", "You have a slow metabolism.",
     "You need consume almost no food."},
#endif

#ifdef JP 
    {"あなたは弱い。(STR -", "", ""},
    {"あなたは愚かだ。(INT -", "", ""},
#else
    {"You are weak (Str -", "", ""},
    {"You are dopey (Int -", "", ""},
#endif
// 20
#ifdef JP 
    {"あなたは不器用だ。(DEX -", "", ""},
#else
    {"You are clumsy (Dex -", "", ""},
#endif

#ifdef JP 
    {"あなたは空間転位を制御することができる。", "あなたは空間転位を制御することができる。",
     "あなたは空間転位を制御することができる。"},
#else
    {"You can control translocations.", "You can control translocations.",
     "You can control translocations."},
#endif

#ifdef JP 
    {"あなたの近くでたまに空間が歪む。",
     "あなたの近くで時々空間が歪む。",
     "あなたの近くで頻繁に空間が歪む。"},
#else
    {"Space occasionally distorts in your vicinity.",
     "Space sometimes distorts in your vicinity.",
     "Space frequently distorts in your vicinity."},
#endif

#ifdef JP 
    {"あなたは魔法に対して抵抗力がある。", "あなたは魔法に対して高い抵抗力がある。",
     "あなたは魔法の影響に対して強力な抵抗力がある。"},
#else
    {"You are resistant to magic.", "You are highly resistant to magic.",
     "You are extremely resistant to the effects of magic."},
#endif

#ifdef JP 
    {"あなたは素早く地面を動く。", "あなたは非常に素早く地面を動く。",
     "あなたは極めて素早く地面を動く。"},
#else
    {"You cover the ground quickly.", "You cover the ground very quickly.",
     "You cover the ground extremely quickly."},
#endif

#ifdef JP 
    {"あなたは超自然的に鋭い視力を持っている。",
     "あなたは超自然的に鋭い視力を持っている。",
     "あなたは超自然的に鋭い視力を持っている。"},
#else
    {"You have supernaturally acute eyesight.",
     "You have supernaturally acute eyesight.",
     "You have supernaturally acute eyesight."},
#endif

#ifdef JP 
    {"あなたの奇形の体には、鎧がうまく合わない。",
     "あなたのひどい奇形の体には、鎧がうまく合わない。",
     "あなたの恐ろしい奇形の体には、鎧がうまく合わない。"},
#else
    {"Armour fits poorly on your deformed body.",
     "Armour fits poorly on your badly deformed body.",
     "Armour fits poorly on your hideously deformed body."},
#endif

#ifdef JP 
    {"あなたは自分の意思でテレポートできる。", "あなたは自分の意思で上手にテレポートできる。",
     "あなたは自分の意思で即時のテレポートできる。"},
#else
    {"You can teleport at will.", "You are good at teleporting at will.",
     "You can teleport instantly at will."},
#endif

#ifdef JP 
    {"あなたは毒を吐き出すことができる。", "あなたは毒を吐き出すことができる。", "あなたは毒を吐き出すことができる。"},
#else
    {"You can spit poison.", "You can spit poison.", "You can spit poison."},
#endif

#ifdef JP 
    {"あなたはごく近くの周辺を感知することができる。",
     "あなたは周辺を感知することができる。",
     "あなたは広域に渡る周辺を感知できる。"},
#else
    {"You can sense your immediate surroundings.",
     "You can sense your surroundings.",
     "You can sense a large area of your surroundings."},
#endif

// 30

#ifdef JP 
    {"あなたは火のブレスを吐くことができる。", "あなたは火炎のブレスを吐くことができる。",
     "あなたは強烈な火炎のブレスを吐くことができる。"},
#else
    {"You can breathe flames.", "You can breathe fire.",
     "You can breathe blasts of fire."},
#endif

#ifdef JP 
    {"あなたは短い距離を瞬間移動することができる。",
     "あなたは短い距離を瞬間移動することができる。",
     "あなたは短い距離を瞬間移動することができる。"},
#else
    {"You can translocate small distances instantaneously.",
     "You can translocate small distances instantaneously.",
     "You can translocate small distances instantaneously."},
#endif

#ifdef JP 
    {"あなたの頭には一対の小さな角がある。",
     "あなたの頭には一対の角がある。",
     "あなたの頭には一対の大きな角がある。"},
#else
    {"You have a pair of small horns on your head.",
     "You have a pair of horns on your head.",
     "You have a pair of large horns on your head."},
#endif

#ifdef JP 
    {"あなたの筋肉は強い。(STR +1) しかし硬すぎる。(DEX -1)",
     "あなたの筋肉は非常に強い。(STR +2) しかし硬すぎる。(DEX -2)",
     "あなたの筋肉は極めて強い。(STR +3) しかし硬すぎる。(DEX -3)"},
#else
    {"Your muscles are strong (Str +1), but stiff (Dex -1).",
     "Your muscles are very strong (Str +2), but stiff (Dex -2).",
     "Your muscles are extremely strong (Str +3), but stiff (Dex -3)."},
#endif

#ifdef JP 
    {"あなたの筋肉は柔軟だ。(DEX +1) しかし弱々しい。(STR -1)",
     "あなたの筋肉は非常に柔軟だ。(DEX +2) しかし弱々しい。(STR -2)",
     "あなたの筋肉は極めて柔軟だ。(DEX +3) しかし弱々しい。(STR -3)"},
#else
    {"Your muscles are flexible (Dex +1), but weak (Str -1).",
     "Your muscles are very flexible (Dex +2), but weak (Str -2).",
     "Your muscles are extremely flexible (Dex +3), but weak (Str -3)."},
#endif

#ifdef JP 
    {"あなたはごくたまに自分のいる場所が分からなくなる。",
     "あなたはたまに自分のいる場所が分からなくなる。",
     "あなたは頻繁に自分のいる場所が分からなくなる。"},
#else
    {"You occasionally forget where you are.",
     "You sometimes forget where you are.",
     "You frequently forget where you are."},
#endif

#ifdef JP 
    {"あなたの意思は非常に明瞭だ。",
     "あなたの意思は不自然なほどに明瞭だ。",
     "あなたの意思は超自然的に明瞭だ。"},
#else
    {"You possess an exceptional clarity of mind.",
     "You possess an unnatural clarity of mind.",
     "You possess a supernatural clarity of mind."},
#endif

#ifdef JP 
    {"あなたは戦闘中に逆上する傾向がある。",
     "あなたはしばしば戦闘中に逆上する。",
     "あなたは逆上を抑えることができない。"},
#else
    {"You tend to lose your temper in combat.",
     "You often lose your temper in combat.",
     "You have an uncontrollable temper."},
#endif

#ifdef JP 
    {"あなたの肉体はゆっくりと能力を低下していく。", "あなたの肉体は能力を低下していく。",
     "あなたの肉体は急速に能力を低下していく。"},
#else
    {"Your body is slowly deteriorating.", "Your body is deteriorating.",
     "Your body is rapidly deteriorating."},
#endif

#ifdef JP 
    {"あなたの視界はぼんやりとしている。", "あなたの視界は非常にぼんやりしている。",
     "あなたの視界は極めてぼんやりしている。"},
#else
    {"Your vision is a little blurry.", "Your vision is quite blurry.",
     "Your vision is extremely blurry."},
#endif
// 40

#ifdef JP 
    {"あなたは突然変異の進行に幾らかの抵抗力がある。",
     "あなたは突然変異の進行と除去の両方に幾らかの抵抗力がある。",
     "あなたの突然変異は決定的に定着していて、これ以上変異することはない。"},
#else
    {"You are somewhat resistant to further mutation.",
     "You are somewhat resistant to both further mutation and mutation removal.",
     "Your current mutations are irrevocably fixed, and you can mutate no more."},
#endif

#ifdef JP 
    {"あなたは虚弱だ。(-10% HP)", "あなたは非常に虚弱だ。(-20% HP)",
     "あなたは極めて虚弱だ。(-30% HP)"},
#else
    {"You are frail (-10 percent hp).", "You are very frail (-20 percent hp).",
     "You are extremely frail (-30 percent hp)."},
#endif

#ifdef JP 
    {"あなたは強健だ。(+10% HP)",
     "あなたは非常に強健だ。(+20% HP)",
     "あなたは極めて強健だ。(+30% HP)"},
#else
    {"You are robust (+10 percent hp).",
     "You are very robust (+20 percent hp).",
     "You are extremely robust (+30 percent hp)."},
#endif

#ifdef JP 
    {"あなたは邪悪な苦痛に免疫がある。", "", ""},
#else
    {"You are immune to unholy pain and torment.", "", ""},
#endif

#ifdef JP 
    {"あなたは負のエネルギーに耐性がある。",
     "あなたは負のエネルギーに非常に耐性がある。",
     "あなたは負のエネルギーに免疫だ。"},
#else
    {"You resist negative energy.",
     "You are quite resistant to negative energy.",
     "You are immune to negative energy."},
#endif

    /* Use player_has_spell() to avoid duplication */
#ifdef JP 
    {"あなたは援軍として下級悪魔を召換できる。", "", ""},
    {"あなたは援軍として悪魔を召換できる。", "", ""},
    {"あなたは地獄の業火を放つことができる。", "", ""},
    {"あなたは地獄の苦悶を呼び起こすことができる。", "", ""},
    {"あなたは死体を従者として蘇生することができる。", "", ""},
#else
    {"You can summon minor demons to your aid.", "", ""},
    {"You can summon demons to your aid.", "", ""},
    {"You can hurl blasts of hellfire.", "", ""},
    {"You can call on the torments of Hell.", "", ""},
    {"You can raise the dead to walk for you.", "", ""},
#endif
// 50
#ifdef JP 
    {"あなたは悪魔を支配することができる。", "", ""},
    {"あなたはパンデモニウムに(片道だが)行くことができる。", "", ""},
    {"あなたは死と破壊から力を引き出すことができる。", "", ""},
#else
    {"You can control demons.", "", ""},
    {"You can travel to (but not from) Pandemonium at will.", "", ""},
    {"You can draw strength from death and destruction.", "", ""},
#endif

    /* Not worshippers of Vehumet */
#ifdef JP 
    {"あなたは地獄から魔法のエネルギーを引き出すことができる。", "", ""},
#else
    {"You can channel magical energy from Hell.", "", ""},
#endif

#ifdef JP 
    {"あなたは徒手での格闘で生命を衰弱させることができる。", "", ""},
#else
    {"You can drain life in unarmed combat.", "", ""},
#endif

    /* Not conjurers/worshippers of Makhleb */
#ifdef JP 
    {"あなたはゲヘナの炎を放射することができる。", "", ""},
#else
    {"You can throw forth the flames of Gehenna.", "", ""},
#endif

#ifdef JP 
    {"あなたはコキュートスの冷気を放射することができる。", "", ""},
#else
    {"You can throw forth the frost of Cocytus.", "", ""},
#endif

#ifdef JP 
    {"あなたはタルタロスの力を呼び起こし、生ける敵に一撃を加えることができる。", "", ""},
    {"あなたの爪は鋭い。", "あなたの爪は非常に鋭い。",
     "あなたは手に鉤爪が生えている。"},
#else
    {"You can invoke the powers of Tartarus to smite your living foes.", "", ""},
    {"You have sharp fingernails.", "Your fingernails are very sharp.",
     "You have claws for hands."},
#endif

#ifdef JP 
    {"あなたは足の替わりに蹄が生えている。", "", ""},
#else
    {"You have hooves in place of feet.", "", ""},
#endif
    // 60 - leave some space for more demonic powers...
#ifdef JP 
    {"あなたは毒の雲を吐き出すことができる。", "", ""},
#else
    {"You can exhale a cloud of poison.", "", ""},
#endif

#ifdef JP 
    {"あなたの尻尾の先には毒のある棘が生えている。",
     "あなたの尻尾の先には毒のある鋭い棘が生えている。",
     "あなたの尻尾の先には毒のある凶悪な棘が生えている。"}, //jmf: nagas & dracos
#else
    {"Your tail ends in a poisonous barb.",
     "Your tail ends in a sharp poisonous barb.",
     "Your tail ends in a wicked poisonous barb."}, //jmf: nagas & dracos
#endif

#ifdef JP 
    {"あなたの翼は大きくて力強い。", "", ""},       //jmf: dracos only
#else
    {"Your wings are large and strong.", "", ""},       //jmf: dracos only
#endif

    //jmf: these next two are for evil gods to mark their followers; good gods
    //     will never accept a 'marked' worhsipper

#ifdef JP 
    {"あなたの両手には青い印章が刻まれている。",
     "あなたの手から腕には幾つか青い印章が刻まれている。",
     "あなたは手から肩にかけて、複雑で神秘的な青い印章で覆われている。"},
#else
    {"There is a blue sigil on each of your hands.",
     "There are several blue sigils on your hands and arms.",
     "Your hands, arms and shoulders are covered in intricate, arcane blue writing."},
#endif

#ifdef JP 
    {"あなたの胸には緑の印章が刻まれている。",
     "あなたの胸から腹には幾つか緑の印章が刻まれている。",
     "あなたは首から腹にかけて、複雑で神秘的な緑の印章で覆われている。"},
#else
    {"There is a green sigil on your chest.",
     "There are several green sigils on your chest and abdomen.",
     "Your chest, abdomen and neck are covered in intricate, arcane green writing."},
#endif

    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    // 70

#ifdef JP 
    {"あなたは赤色の鱗で部分的に覆われている。(AC +1)",
     "あなたは赤色の鱗で大部分を覆われている。(AC +2)",
     "あなたは赤色の鱗で完全に覆われている。(AC +4)"},
#else
    {"You are partially covered in red scales (AC + 1).",
     "You are mostly covered in red scales (AC + 2).",
     "You are covered in red scales (AC + 4)."},
#endif

#ifdef JP 
    {"あなたはなめらかな真珠色の鱗で部分的に覆われている。(AC +1)",
     "あなたはなめらかな真珠色の鱗で大部分を覆われている。(AC +3)",
     "あなたはなめらかな真珠色の鱗で完全に覆われている。(AC +5)"},
#else
    {"You are partially covered in smooth nacreous scales (AC + 1).",
     "You are mostly covered in smooth nacreous scales (AC + 3).",
     "You are completely covered in smooth nacreous scales (AC + 5)."},
#endif

#ifdef JP 
    {"あなたは隆起した灰色の鱗で部分的に覆われている。(AC +2, DEX -1)",
     "あなたは隆起した灰色の鱗で大部分を覆われている。(AC +4, DEX -1)",
     "あなたは隆起した灰色の鱗で完全に覆われている。(AC +6, DEX -2)"},
#else
    {"You are partially covered in ridged grey scales (AC + 2, dex - 1).",
     "You are mostly covered in ridged grey scales (AC + 4, dex - 1).",
     "You are completely covered in ridged grey scales (AC + 6, dex - 2)."},
#endif

#ifdef JP 
    {"あなたは金属性の鱗で部分的に覆われている。(AC +3, DEX -2)",
     "あなたは金属性の鱗で大部分を覆われている。(AC +7, DEX -3)",
     "あなたは金属性の鱗で完全に覆われている。(AC +10, DEX -4)"},
#else
    {"You are partially covered in metallic scales (AC + 3, dex - 2).",
     "You are mostly covered in metallic scales (AC + 7, dex - 3).",
     "You are completely covered in metallic scales (AC + 10, dex - 4)."},
#endif

#ifdef JP 
    {"あなたは黒い鱗で部分的に覆われている。(AC +1)",
     "あなたは黒い鱗で大部分を覆われている。(AC +3)",
     "あなたは黒い鱗で完全に覆われている。(AC +5)"},
#else
    {"You are partially covered in black scales (AC + 1).",
     "You are mostly covered in black scales (AC + 3).",
     "You are completely covered in black scales (AC + 5)."},
#endif

#ifdef JP 
    {"あなたは白い鱗で部分的に覆われている。(AC +1)",
     "あなたは白い鱗で大部分を覆われている。(AC +3)",
     "あなたは白い鱗で完全に覆われている。(AC +5)"},
#else
    {"You are partially covered in white scales (AC + 1).",
     "You are mostly covered in white scales (AC + 3).",
     "You are completely covered in white scales (AC + 5)."},
#endif

#ifdef JP 
    {"あなたは黄色の鱗で部分的に覆われている。(AC +2)",
     "あなたは黄色の鱗で大部分を覆われている。(AC +4, DEX -1)",
     "あなたは黄色の鱗で完全に覆われている。(AC +6, DEX -2)"},
#else
    {"You are partially covered in yellow scales (AC + 2).",
     "You are mostly covered in yellow scales (AC + 4, dex - 1).",
     "You are completely covered in yellow scales (AC + 6, dex - 2)."},
#endif

#ifdef JP 
    {"あなたは茶色の鱗で部分的に覆われている。(AC +2)",
     "あなたは茶色の鱗で大部分を覆われている。(AC +4)",
     "あなたは茶色の鱗で完全に覆われている。(AC +5)"},
#else
    {"You are partially covered in brown scales (AC + 2).",
     "You are mostly covered in brown scales (AC + 4).",
     "You are completely covered in brown scales (AC + 5)."},
#endif

#ifdef JP 
    {"あなたは青い鱗で部分的に覆われている。(AC +1)",
     "あなたは青い鱗で大部分を覆われている。(AC +2)",
     "あなたは青い鱗で完全に覆われている。(AC +3)"},
#else
    {"You are partially covered in blue scales (AC + 1).",
     "You are mostly covered in blue scales (AC + 2).",
     "You are completely covered in blue scales (AC + 3)."},
#endif

#ifdef JP 
    {"あなたは紫色の鱗で部分的に覆われている。(AC +2)",
     "あなたは紫色の鱗で大部分を覆われている。(AC +4)",
     "あなたは紫色の鱗で完全に覆われている。(AC +6)"},
#else
    {"You are partially covered in purple scales (AC + 2).",
     "You are mostly covered in purple scales (AC + 4).",
     "You are completely covered in purple scales (AC + 6)."},
#endif

// 80

#ifdef JP 
    {"あなたはまだら模様の鱗で部分的に覆われている。(AC +1)",
     "あなたはまだら模様の鱗で大部分を覆われている。(AC +2)",
     "あなたはまだら模様の鱗で完全に覆われている。(AC +3)"},
#else
    {"You are partially covered in speckled scales (AC + 1).",
     "You are mostly covered in speckled scales (AC + 2).",
     "You are covered in speckled scales (AC + 3)."},
#endif

#ifdef JP 
    {"あなたはオレンジ色の鱗で部分的に覆われている。(AC +1)",
     "あなたはオレンジ色の鱗で大部分を覆われている。(AC +3)",
     "あなたはオレンジ色の鱗で完全に覆われている。(AC +4)"},
#else
    {"You are partially covered in orange scales (AC + 1).",
     "You are mostly covered in orange scales (AC + 3).",
     "You are completely covered in orange scales (AC + 4)."},
#endif

#ifdef JP 
    {"あなたは藍色の鱗で部分的に覆われている。(AC +2)",
     "あなたは藍色の鱗で大部分を覆われている。(AC +3)",
     "あなたは藍色の鱗で完全に覆われている。(AC +5)"},
#else
    {"You are partially covered in indigo scales (AC + 2).",
     "You are mostly covered in indigo scales (AC + 3).",
     "You are completely covered in indigo scales (AC + 5)."},
#endif

#ifdef JP 
    {"あなたは節くれ立った赤い鱗で部分的に覆われている。(AC +2)",
     "あなたは節くれ立った赤い鱗で大部分を覆われている。(AC +5, DEX -1)",
     "あなたは節くれ立った赤い鱗で完全に覆われている。(AC +7, DEX -2)"},
#else
    {"You are partially covered in knobbly red scales (AC + 2).",
     "You are mostly covered in knobbly red scales (AC + 5, dex - 1).",
     "You are completely covered in knobbly red scales (AC + 7, dex - 2)."},
#endif

#ifdef JP 
    {"あなたは玉虫色の鱗で部分的に覆われている。(AC +1)",
     "あなたは玉虫色の鱗で大部分を覆われている。(AC +2)",
     "あなたは玉虫色の鱗で完全に覆われている。(AC +3)"},
#else
    {"You are partially covered in iridescent scales (AC + 1).",
     "You are mostly covered in iridescent scales (AC + 2).",
     "You are completely covered in iridescent scales (AC + 3)."},
#endif

#ifdef JP 
    {"あなたは模様の浮き出した鱗で部分的に覆われている。(AC +1)",
     "あなたは模様の浮き出した鱗で大部分を覆われている。(AC +2)",
     "あなたは模様の浮き出した鱗で完全に覆われている。(AC +3)"},
#else
    {"You are partially covered in patterned scales (AC + 1).",
     "You are mostly covered in patterned scales (AC + 2).",
     "You are completely covered in patterned scales (AC + 3)."},
#endif
};

/*
   If giving a mutation which must succeed (eg demonspawn), must add exception
   to the "resist mutation" mutation thing.
 */

const char *gain_mutation[][3] = {
#ifdef JP 
    {"あなたの皮膚は強靭になった。", "あなたの皮膚は強靭になった。", "あなたの皮膚は強靭になった。"},
#else
    {"Your skin toughens.", "Your skin toughens.", "Your skin toughens."},
#endif

    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}

#ifdef JP 
    {"緑色の鱗があなたの体の一部に生えてきた。",
     "緑色の鱗があなたの体を更に覆った。",
     "緑色の鱗があなたの体を完全に覆った。"},
#else
    {"Green scales grow over part of your body.",
     "Green scales spread over more of your body.",
     "Green scales cover you completely."},
#endif

#ifdef JP 
    {"厚みのある黒い鱗があなたの体の一部に生えてきた。",
     "厚みのある黒い鱗があなたの体を更に覆った。",
     "厚みのある黒い鱗があなたの体を完全に覆った。"},
#else
    {"Thick black scales grow over part of your body.",
     "Thick black scales spread over more of your body.",
     "Thick black scales cover you completely."},
#endif

#ifdef JP 
    {"柔軟な灰色の鱗があなたの体の一部に生えてきた。",
     "柔軟な灰色の鱗があなたの体を更に覆った。",
     "柔軟な灰色の鱗があなたの体を完全に覆った。"},
#else
    {"Supple grey scales grow over part of your body.",
     "Supple grey scales spread over more of your body.",
     "Supple grey scales cover you completely."},
#endif

#ifdef JP 
    {"あなたの体に骨の装甲が生えてきた。",
     "あなたの骨の装甲が更に成長した。",
     "あなたの骨の装甲が更に成長した。"},
#else
    {"You grow protective plates of bone.",
     "You grow more protective plates of bone.",
     "You grow more protective plates of bone."},
#endif

#ifdef JP 
    {"あなたは斥力を放射しはじめた。",
     "あなたの斥力が更に強くなった。",
     "あなたの斥力が更に強くなった。"},
#else
    {"You begin to radiate repulsive energy.",
     "Your repulsive radiation grows stronger.",
     "Your repulsive radiation grows stronger."},
#endif

#ifdef JP 
    {"あなたはより健全になったようだ。", "あなたはより健全になったようだ。",  "あなたはより健全になったようだ。"},
#else
    {"You feel healthy.", "You feel healthy.",  "You feel healthy."},
#endif
// 10
#ifdef JP 
    {"あなたは肉に飢えている。", "あなたは肉に飢えている。", "あなたは肉に飢えている。"},
#else
    {"You hunger for flesh.", "You hunger for flesh.", "You hunger for flesh."},
#endif

#ifdef JP 
    {"あなたは野菜に飢えている。", "あなたは野菜に飢えている。",
     "あなたは野菜に飢えている。"},
#else
    {"You hunger for vegetation.", "You hunger for vegetation.",
     "You hunger for vegetation."},
#endif

#ifdef JP 
    {"あなたは突然に肌寒さを感じた。", "あなたは突然に肌寒さを感じた。",
     "あなたは突然に肌寒さを感じた。"},
#else
    {"You feel a sudden chill.", "You feel a sudden chill.",
     "You feel a sudden chill."},
#endif

#ifdef JP 
    {"あなたは突然に暑さを感じた。", "あなたは突然に暑さを感じた。",
     "あなたは突然に暑さを感じた。"},
#else
    {"You feel hot for a moment.", "You feel hot for a moment.",
     "You feel hot for a moment."},
#endif

#ifdef JP 
    {"あなたは不電導体になったようだ。", "あなたは不電導体になったようだ。", "あなたは不電導体になったようだ。"},
#else
    {"You feel insulated.", "You feel insulated.", "You feel insulated."},
#endif

#ifdef JP 
    {"あなたはより速く傷が癒えるようになった。",
     "あなたはより速く傷が癒えるようになった。",
     "あなたは再生能力を身につけた。"},
#else
    {"You begin to heal more quickly.",
     "You begin to heal more quickly.",
     "You begin to regenerate."},
#endif

#ifdef JP 
    {"あなたは少し空腹を感じた。", "あなたは少し空腹を感じた。",
     "あなたは少し空腹を感じた。"},
#else
    {"You feel a little hungry.", "You feel a little hungry.",
     "You feel a little hungry."},
#endif

#ifdef JP 
    {"あなたの新陳代謝は遅くなった。", "あなたの新陳代謝は遅くなった。",
     "あなたの新陳代謝は遅くなった。"},
#else
    {"Your metabolism slows.", "Your metabolism slows.",
     "Your metabolism slows."},
#endif

#ifdef JP 
    {"あなたは弱くなったようだ。", "あなたは弱くなったようだ。", "あなたは弱くなったようだ。"},
#else
    {"You feel weaker.", "You feel weaker.", "You feel weaker."},
#endif

#ifdef JP 
    {"あなたは知力の衰えを感じた。", "あなたは知力の衰えを感じた。",
     "あなたは知力の衰えを感じた。"},
#else
    {"You feel less intelligent.", "You feel less intelligent",
     "You feel less intelligent"},
#endif
// 20
#ifdef JP 
    {"あなたは不器用になったようだ。", "あなたは不器用になったようだ。",
     "あなたは不器用になったようだ。"},
#else
    {"You feel clumsy.", "You feel clumsy.",
     "You feel clumsy."},
#endif

#ifdef JP 
    {"あなたは制御の力を感じた。", "あなたは制御の力を感じた。",
     "あなたは制御の力を感じた。"},
#else
    {"You feel controlled.", "You feel controlled.",
     "You feel controlled."},
#endif

#ifdef JP 
    {"あなたは奇妙な不確定性を意識した。",
     "あなたは更に、奇妙な不確定性を意識した。",
     "あなたは更に、奇妙な不確定性を意識した。"},
#else
    {"You feel weirdly uncertain.",
     "You feel even more weirdly uncertain.",
     "You feel even more weirdly uncertain."},
#endif

#ifdef JP 
    {"あなたは魔法への抵抗力を得た。",
     "あなたは更に魔法への抵抗力を得た。",
     "あなたは魔法の効果にほとんど影響を受けなくなった。"},
#else
    {"You feel resistant to magic.",
     "You feel more resistant to magic.",
     "You feel almost impervious to the effects of magic."},
#endif

#ifdef JP 
    {"あなたは素早くなった。", "あなたは素早くなった。", "あなたは素早くなった。"},
#else
    {"You feel quick.", "You feel quick.", "You feel quick."},
#endif

#ifdef JP 
    {"あなたの視力は鋭くなった。", "あなたの視力は鋭くなった。", "あなたの視力は鋭くなった。"},
#else
    {"Your vision sharpens.", "Your vision sharpens.", "Your vision sharpens."},
#endif

#ifdef JP 
    {"あなたの体は捩くれて奇形になった。", "あなたの体は捩くれて奇形になった。",
     "あなたの体は捩くれて奇形になった。"},
#else
    {"Your body twists and deforms.", "Your body twists and deforms.",
     "Your body twists and deforms."},
#endif

#ifdef JP 
    {"あなたは躍動感を感じた。", "あなたは躍動感を感じた。", "あなたは躍動感を感じた。"},
#else
    {"You feel jumpy.", "You feel more jumpy.", "You feel even more jumpy."},
#endif

#ifdef JP 
    {"一瞬あなた口の中で不快な味がした。",
     "一瞬あなた口の中で不快な味がした。",
     "一瞬あなた口の中で不快な味がした。"},
#else
    {"There is a nasty taste in your mouth for a moment.",
     "There is a nasty taste in your mouth for a moment.",
     "There is a nasty taste in your mouth for a moment."},
#endif

#ifdef JP 
    {"あなたは周囲の状況がわかるようになった。",
     "あなたは周囲の状況が更にわかるようになった。",
     "あなたは周囲の状況がより一層わかるようになった。"},
#else
    {"You feel aware of your surroundings.",
     "You feel more aware of your surroundings.",
     "You feel even more aware of your surroundings."},
#endif
// 30

#ifdef JP 
    {"あなたは喉が熱くなった。", "あなたは喉が熱くなった。",
     "あなたは喉が熱くなった。"},
#else
    {"Your throat feels hot.", "Your throat feels hot.",
     "Your throat feels hot."},
#endif

#ifdef JP 
    {"あなたは少し躍動感を感じた。", "あなたはさらに躍動感を感じた。",
     "あなたはより一層の躍動感を感じた。"},
#else
    {"You feel a little jumpy.", "You feel more jumpy.",
     "You feel even more jumpy."},
#endif

#ifdef JP 
    {"あなたの頭に一対の角が生えてきた！",
     "あなたの角が幾らか成長した。",
     "あなたの角が幾らか成長した。"},
#else
    {"A pair of horns grows on your head!",
     "The horns on your head grow some more.",
     "The horns on your head grow some more."},
#endif

#ifdef JP 
    {"あなたは筋肉痛を覚えた。", "あなたは筋肉痛を覚えた。",
     "あなたは筋肉痛を覚えた。"},
#else
    {"Your muscles feel sore.", "Your muscles feel sore.",
     "Your muscles feel sore."},
#endif

#ifdef JP 
    {"あなたの筋肉は柔軟になった。", "あなたの筋肉は柔軟になった。",
     "あなたの筋肉は柔軟になった。"},
#else
    {"Your muscles feel loose.", "Your muscles feel loose.",
     "Your muscles feel loose."},
#endif

#ifdef JP 
    {"あなたは軽い見当識喪失を覚えた。", "あなたは軽い見当識喪失を覚えた。",
     "あなたはどこにいるのだろう？"},
#else
    {"You feel a little disoriented.", "You feel a little disoriented.",
     "Where the Hells are you?"},
#endif

#ifdef JP 
    {"あなたは思考が明瞭になった。", "あなたは思考が明瞭になった。",
     "あなたは思考が明瞭になった。"},
#else
    {"Your thoughts seem clearer.", "Your thoughts seem clearer.",
     "Your thoughts seem clearer."},
#endif

#ifdef JP 
    {"あなたは少し腹立ちを覚えた。", "あなたは腹立ちを覚えた。",
     "あなたは全てに対して非常に怒りを覚えた！"},
#else
    {"You feel a little pissed off.", "You feel angry.",
     "You feel extremely angry at everything!"},
#endif

#ifdef JP 
    {"あなたは体が消耗していくのを感じた。", "あなたは体が消耗していくのを感じた。",
     "あなたは体がボロボロになっていくのを感じた。"},
#else
    {"You feel yourself wasting away.", "You feel yourself wasting away.",
     "You feel your body start to fall apart."},
#endif

#ifdef JP 
    {"あなたの視界はぼやけた。", "あなたの視界はぼやけた。", "あなたの視界はぼやけた。"},
#else
    {"Your vision blurs.", "Your vision blurs.", "Your vision blurs."},
#endif
// 40

#ifdef JP 
    {"あなたは遺伝子的に安定した。", "あなたは遺伝子的に安定した。",
     "あなたは遺伝子的に変化しなくなった。"},
#else
    {"You feel genetically stable.", "You feel genetically stable.",
     "You feel genetically immutable."},
#endif

#ifdef JP 
    {"あなたは虚弱になった。", "あなたは虚弱になった。",  "あなたは虚弱になった。"},
    {"あなたは強健になった。", "あなたは強健になった。", "あなたは強健になった。"},
    {"あなたは奇妙に無感覚になった。", "", ""},
    {"あなたは負の耐性を得た。", "あなたは負の耐性を得た。", "あなたは負の耐性を得た。"},
    {"千もの喋り声があなたに呼びかけてきた。", "", ""},
    {"助けがそう遠くないところにいる！", "", ""},
    {"あなたは火と硫黄の匂いを嗅いだ。", "", ""},
    {"あなたは恐怖すべき力を呼び起こすことができるようになった。", "", ""},
    {"あなたは死者に親近感をおぼえるようになった。", "", ""},
#else
    {"You feel frail.", "You feel frail.",  "You feel frail."},
    {"You feel robust.", "You feel robust.", "You feel robust."},
    {"You feel a strange anaesthesia.", "", ""},
    {"You feel negative.", "You feel negative.", "You feel negative."},
    {"A thousand chattering voices call out to you.", "", ""},
    {"Help is not far away!", "", ""},
    {"You smell fire and brimstone.", "", ""},
    {"You feel a terrifying power at your call.", "", ""},
    {"You feel an affinity for the dead.", "", ""},
#endif
// 50
#ifdef JP 
    {"あなたは悪魔全般に親近感をおぼえるようになった。", "", ""},
    {"あなたは何者かに奇怪で恐ろしい場所に呼び寄せられている。", "", ""},
    {"あなたは死に飢えている。", "", ""},
    {"あなたは魔法のエネルギーが流れ込むのを感じた。", "", ""},
    {"あなたの皮膚は奇妙で不愉快な感覚に疼く。", "", ""},
    {"あなたはゲヘナの火の匂いを嗅いだ。", "", ""},
    {"あなたはコキュートスの凍てつく冷気に魂の凍えを覚えた。", "", ""},
    {"あなたの周囲の世界を影が駆け抜けた。", "", ""},
#else
    {"You feel an affinity for all demonkind.", "", ""},
    {"You feel something pulling you to a strange and terrible place.", "", ""},
    {"You feel hungry for death.", "", ""},
    {"You feel a flux of magical energy.", "", ""},
    {"Your skin tingles in a strangely unpleasant way.", "", ""},
    {"You smell the fires of Gehenna.", "", ""},
    {"You feel the icy cold of Cocytus chill your soul.", "", ""},
    {"A shadow passes over the world around you.", "", ""},
#endif

#ifdef JP 
    {"あなたの爪は伸びていった。", "あなたの爪は鋭くなった。",
     "あなたの手は鉤爪に変化した。"},
#else
    {"Your fingernails lengthen.", "Your fingernails sharpen.",
     "Your hands twist into claws."},
#endif

#ifdef JP 
    {"あなたの足は先の割れた蹄に変化した。", "", ""},
#else
    {"Your feet shrivel into cloven hooves.", "", ""},
#endif
    // 60

#ifdef JP 
    {"あなたの口の中に不快な味がした。", "あなたの口の中に非常に不快な味がした。",
     "あなたの口の中に極めて不快な味がした。"},
#else
    {"You taste something nasty.", "You taste something very nasty.",
     "You taste something extremely nasty."},
#endif

#ifdef JP 
    {"あなたの尻尾の先に毒のある棘が生えてた。",
     "あなたの尻尾の棘は鋭くなった。",
     "あなたの尻尾の棘は非常に鋭くなった。"},
#else
    {"A poisonous barb forms on the end of your tail.",
     "The barb on your tail looks sharper.",
     "The barb on your tail looks very sharp."},
#endif

#ifdef JP 
    {"あなたの翼は大きく力強く成長した。", "", ""},
#else
    {"Your wings grow larger and stronger.", "", ""},
#endif

#ifdef JP 
    {"あなたは手がむず痒くなった。", "あなたは手から腕にかけてむず痒くなった。",
     "あなたは手から肩にかけてむず痒くなった。"},
#else
    {"Your hands itch.", "Your hands and forearms itch.",
     "Your arms, hands and shoulders itch."},
#endif

#ifdef JP 
    {"あなたは胸がむず痒くなった。", "あなたは胸から腹にかけてむず痒くなった。",
     "あなたは首から腹にかけてむず痒くなった。"},
#else
    {"Your chest itches.", "Your chest and abdomen itch.",
     "Your chest, abdomen and neck itch."},
#endif

    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    // 70

#ifdef JP 
    {"赤色の鱗があなたの体の一部に生えてきた。",
     "赤色の鱗があなたの体を更に覆った。",
     "赤色の鱗があなたの体を完全に覆った。"},
    {"なめらかな真珠色の鱗があなたの体の一部に生えてきた。",
     "なめらかな真珠色の鱗があなたの体を更に覆った。",
     "なめらかな真珠色の鱗があなたの体を完全に覆った。"},
    {"隆起した灰色の鱗があなたの体の一部に生えてきた。",
     "隆起した灰色の鱗があなたの体を更に覆った。",
     "隆起した灰色の鱗があなたの体を完全に覆った。"},
    {"金属性の鱗があなたの体の一部に生えてきた。",
     "金属性の鱗があなたの体を更に覆った。",
     "金属性の鱗があなたの体を完全に覆った。"},
    {"黒い鱗があなたの体の一部に生えてきた。",
     "黒い鱗があなたの体を更に覆った。",
     "黒い鱗があなたの体を完全に覆った。"},
    {"白い鱗があなたの体の一部に生えてきた。",
     "白い鱗があなたの体を更に覆った。",
     "白い鱗があなたの体を完全に覆った。"},
    {"黄色の鱗があなたの体の一部に生えてきた。",
     "黄色の鱗があなたの体を更に覆った。",
     "黄色の鱗があなたの体を完全に覆った。"},
    {"茶色の鱗があなたの体の一部に生えてきた。",
     "茶色の鱗があなたの体を更に覆った。",
     "茶色の鱗があなたの体を完全に覆った。"},
    {"青い鱗があなたの体の一部に生えてきた。",
     "青い鱗があなたの体を更に覆った。",
     "青い鱗があなたの体を完全に覆った。"},
    {"紫色の鱗があなたの体の一部に生えてきた。",
     "紫色の鱗があなたの体を更に覆った。",
     "紫色の鱗があなたの体を完全に覆った。"},
#else
    {"Red scales grow over part of your body.",
     "Red scales spread over more of your body.",
     "Red scales cover you completely."},
    {"Smooth nacreous scales grow over part of your body.",
     "Smooth nacreous scales spread over more of your body.",
     "Smooth nacreous scales cover you completely."},
    {"Ridged grey scales grow over part of your body.",
     "Ridged grey scales spread over more of your body.",
     "Ridged grey scales cover you completely."},
    {"Metallic scales grow over part of your body.",
     "Metallic scales spread over more of your body.",
     "Metallic scales cover you completely."},
    {"Black scales grow over part of your body.",
     "Black scales spread over more of your body.",
     "Black scales cover you completely."},
    {"White scales grow over part of your body.",
     "White scales spread over more of your body.",
     "White scales cover you completely."},
    {"Yellow scales grow over part of your body.",
     "Yellow scales spread over more of your body.",
     "Yellow scales cover you completely."},
    {"Brown scales grow over part of your body.",
     "Brown scales spread over more of your body.",
     "Brown scales cover you completely."},
    {"Blue scales grow over part of your body.",
     "Blue scales spread over more of your body.",
     "Blue scales cover you completely."},
    {"Purple scales grow over part of your body.",
     "Purple scales spread over more of your body.",
     "Purple scales cover you completely."},
#endif
    // 80

#ifdef JP 
    {"まだら模様の鱗があなたの体の一部に生えてきた。",
     "まだら模様の鱗があなたの体を更に覆った。",
     "まだら模様の鱗があなたの体を完全に覆った。"},
    {"オレンジ色の鱗があなたの体の一部に生えてきた。",
     "オレンジ色の鱗があなたの体を更に覆った。",
     "オレンジ色の鱗があなたの体を完全に覆った。"},
    {"藍色の鱗があなたの体の一部に生えてきた。",
     "藍色の鱗があなたの体を更に覆った。",
     "藍色の鱗があなたの体を完全に覆った。"},
    {"節くれ立った赤い鱗があなたの体の一部に生えてきた。",
     "節くれ立った赤い鱗があなたの体を更に覆った。",
     "節くれ立った赤い鱗があなたの体を完全に覆った。"},
    {"玉虫色の鱗があなたの体の一部に生えてきた。",
     "玉虫色の鱗があなたの体を更に覆った。",
     "玉虫色の鱗があなたの体を完全に覆った。"},
    {"模様の浮き出した鱗があなたの体の一部に生えてきた。",
     "模様の浮き出した鱗があなたの体を更に覆った。",
     "模様の浮き出した鱗があなたの体を完全に覆った。"},
#else
    {"Speckled scales grow over part of your body.",
     "Speckled scales spread over more of your body.",
     "Speckled scales cover you completely."},
    {"Orange scales grow over part of your body.",
     "Orange scales spread over more of your body.",
     "Orange scales cover you completely."},
    {"Indigo scales grow over part of your body.",
     "Indigo scales spread over more of your body.",
     "Indigo scales cover you completely."},
    {"Knobbly red scales grow over part of your body.",
     "Knobbly red scales spread over more of your body.",
     "Knobbly red scales cover you completely."},
    {"Iridescent scales grow over part of your body.",
     "Iridescent scales spread over more of your body.",
     "Iridescent scales cover you completely."},
    {"Patterned scales grow over part of your body.",
     "Patterned scales spread over more of your body.",
     "Patterned scales cover you completely."},
#endif
};

const char *lose_mutation[][3] = {

#ifdef JP 
    {"あなたの皮膚は柔らかくなった。", "あなたの皮膚は柔らかくなった。",
     "あなたの皮膚は柔らかくなった。"},
#else
    {"Your skin feels delicate.", "Your skin feels delicate.",
     "Your skin feels delicate."},
#endif

#ifdef JP 
    {"あなたは弱々しくなった。", "あなたは弱々しくなった。", "あなたは弱々しくなった。"},
#else
    {"You feel weaker.", "You feel weaker.", "You feel weaker."},
#endif

#ifdef JP 
    {"あなたは知力が衰えた。", "あなたは知力が衰えた。",
     "あなたは知力が衰えた。"},
#else
    {"You feel less intelligent.", "You feel less intelligent",
     "You feel less intelligent"},
#endif

#ifdef JP 
    {"あなたは不器用になった。", "あなたは不器用になった。", "あなたは不器用になった。"},
#else
    {"You feel clumsy.", "You feel clumsy.", "You feel clumsy."},
#endif

#ifdef JP 
    {"あなたの緑色の鱗が消えてしまった。",
     "あなたの緑色の鱗が幾らか減少した。",
     "あなたの緑色の鱗が幾らか減少した。"},
#else
    {"Your green scales disappear.",
     "Your green scales recede somewhat.",
     "Your green scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの黒い鱗が消えてしまった。", "あなたの黒い鱗が幾らか減少した。",
     "あなたの黒い鱗が幾らか減少した。"},
#else
    {"Your black scales disappear.", "Your black scales recede somewhat.",
     "Your black scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの灰色の鱗が消えてしまった。", "あなたの灰色の鱗が幾らか減少した。",
     "あなたの灰色の鱗が幾らか減少した。"},
#else
    {"Your grey scales disappear.", "Your grey scales recede somewhat.",
     "Your grey scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの骨の装甲は収縮してなくなった。", "あなたの骨の装甲は収縮した。",
     "あなたの骨の装甲は収縮した。"},
#else
    {"Your bony plates shrink away.", "Your bony plates shrink.",
     "Your bony plates shrink."},
#endif

#ifdef JP 
    {"あなたの引力が強くなった。", "あなたの引力が強くなった。", "あなたの引力が強くなった。"},
#else
    {"You feel attractive.", "You feel attractive.", "You feel attractive."},
#endif

#ifdef JP 
    {"あなたは少し健康が衰えたのを感じた。", "あなたは少し健康が衰えたのを感じた。",
     "あなたは少し健康が衰えたのを感じた。"},
#else
    {"You feel a little less healthy.", "You feel a little less healthy.",
     "You feel a little less healthy."},
#endif

#ifdef JP 
    {"あなたはよりバランス良い食事を取ることができるようになった。",
     "あなたはよりバランス良い食事を取ることができるようになった。",
     "あなたはよりバランス良い食事を取ることができるようになった。"},
#else
    {"You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet."},
#endif

#ifdef JP 
    {"あなたはよりバランス良い食事を取ることができるようになった。",
     "あなたはよりバランス良い食事を取ることができるようになった。",
     "あなたはよりバランス良い食事を取ることができるようになった。"},
#else
    {"You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet.",
     "You feel able to eat a more balanced diet."},
#endif

#ifdef JP 
    {"あなたは一瞬の暑さを感じた。", "あなたは一瞬の暑さを感じた。",
     "あなたは一瞬の暑さを感じた。"},
#else
    {"You feel hot for a moment.", "You feel hot for a moment.",
     "You feel hot for a moment."},
#endif

#ifdef JP 
    {"あなたは突然に寒気を感じた。", "あなたは突然に寒気を感じた。",
     "あなたは突然に寒気を感じた。"},
#else
    {"You feel a sudden chill.", "You feel a sudden chill.",
     "You feel a sudden chill."},
#endif

#ifdef JP 
    {"あなたは導電性を意識した。", "あなたは導電性を意識した。", "あなたは導電性を意識した。"},
#else
    {"You feel conductive.", "You feel conductive.", "You feel conductive."},
#endif

#ifdef JP 
    {"あなたの治癒速度は遅くなった。", "あなたの治癒速度は遅くなった。",
     "あなたの治癒速度は遅くなった。"},
#else
    {"Your rate of healing slows.", "Your rate of healing slows.",
     "Your rate of healing slows."},
#endif

#ifdef JP 
    {"あなたの新陳代謝は遅くなった。", "あなたの新陳代謝は遅くなった。",
     "あなたの新陳代謝は遅くなった。"},
#else
    {"Your metabolism slows.", "Your metabolism slows.",
     "Your metabolism slows."},
#endif

#ifdef JP 
    {"あなたは少し空腹を感じた。", "あなたは少し空腹を感じた。",
     "あなたは少し空腹を感じた。"},
#else
    {"You feel a little hungry.", "You feel a little hungry.",
     "You feel a little hungry."},
#endif

    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}
// 20
    {"", "", ""},  // replaced with player::modify_stat() handling {dlb}

#ifdef JP 
    {"あなたは制御の衰えを感じた。", "あなたは制御の衰えを感じた。", "あなたは制御の衰えを感じた。"},
    {"あなたは安定性を感じた。", "あなたは安定性を感じた。", "あなたは安定性を感じた。"},
#else
    {"You feel random.", "You feel uncontrolled.", "You feel uncontrolled."},
    {"You feel stable.", "You feel stable.", "You feel stable."},
#endif

#ifdef JP 
    {"あなたは魔法への抵抗力が衰えたのを感じた。", "あなたは魔法への抵抗力が衰えたのを感じた。",
     "あなたは再び魔法からの影響を受けるようになった。"},
#else
    {"You feel less resistant to magic.", "You feel less resistant to magic.",
     "You feel vulnerable to magic again."},
#endif

#ifdef JP 
    {"あなたは動きが緩慢になった。", "あなたは動きが緩慢になった。", "あなたは動きが緩慢になった。"},
#else
    {"You feel sluggish.", "You feel sluggish.", "You feel sluggish."},
#endif

#ifdef JP 
    {"あなたの視力は鈍くなった。", "あなたの視力は鈍くなった。",
     "あなたの視力は鈍くなった。"},
#else
    {"Your vision seems duller.", "Your vision seems duller.",
     "Your vision seems duller."},
#endif

#ifdef JP 
    {"あなたの肉体は奇形的でなくなった。",
     "あなたの肉体は幾らか奇形的でなくなった。",
     "あなたの肉体は幾らか奇形的でなくなった。"},
#else
    {"Your body's shape seems more normal.",
     "Your body's shape seems slightly more normal.",
     "Your body's shape seems slightly more normal."},
#endif

#ifdef JP 
    {"あなたは停滞を感じた。", "あなたの躍動感は減少した。", "あなたの躍動感は減少した。"},
#else
    {"You feel static.", "You feel less jumpy.", "You feel less jumpy."},
#endif

#ifdef JP 
    {"あなたは喉の奥が痛むのを感じた。",
     "あなたは喉の奥が痛むのを感じた。", "あなたは喉の奥が痛むのを感じた。"},
#else
    {"You feel an ache in your throat.",
     "You feel an ache in your throat.", "You feel an ache in your throat."},
#endif

#ifdef JP 
    {"あなたは少し見当識を失った。", "あなたは少し見当識を失った。",
     "あなたは少し見当識を失った。"},
#else
    {"You feel slightly disorientated.", "You feel slightly disorientated.",
     "You feel slightly disorientated."},
#endif
// 30

#ifdef JP 
    {"あなたの喉の奥は冷たくなった。",
     "あなたの喉の奥は冷たくなった。",
     "あなたの喉の奥は冷たくなった。"},
#else
    {"A chill runs up and down your throat.",
     "A chill runs up and down your throat.",
     "A chill runs up and down your throat."},
#endif

#ifdef JP 
    {"あなたは躍動感をなくした。", "あなたは躍動感が減じた。",
     "あなたは躍動感が減じた。"},
#else
    {"You feel a little less jumpy.", "You feel less jumpy.",
     "You feel less jumpy."},
#endif

#ifdef JP 
    {"あなたの頭にある角が収縮して消えた。",
     "あなたの頭にある角が少し小さくなった。",
     "あなたの頭にある角が少し小さくなった。"},
#else
    {"The horns on your head shrink away.",
     "The horns on your head shrink a bit.",
     "The horns on your head shrink a bit."},
#endif

#ifdef JP 
    {"あなたの筋肉は柔らかくなったようだ。", "あなたの筋肉は柔らかくなったようだ。",
     "あなたの筋肉は柔らかくなったようだ。"},
#else
    {"Your muscles feel loose.", "Your muscles feel loose.",
     "Your muscles feel loose."},
#endif

#ifdef JP 
    {"あなたは筋肉痛を感じた。", "あなたは筋肉痛を感じた。",
     "あなたは筋肉痛を感じた。"},
#else
    {"Your muscles feel sore.", "Your muscles feel sore.",
     "Your muscles feel sore."},
#endif

#ifdef JP 
    {"あなたの見当識は幾分良くなった。", "あなたの見当識は幾分良くなった。",
     "あなたの見当識は幾分良くなった。"},
#else
    {"You feel less disoriented.", "You feel less disoriented.",
     "You feel less disoriented."},
#endif

#ifdef JP 
    {"あなたの思考は混濁した。", "あなたの思考は混濁した。",
     "あなたの思考は混濁した。"},
#else
    {"Your thinking seems confused.", "Your thinking seems confused.",
     "Your thinking seems confused."},
#endif

#ifdef JP 
    {"あなたは少し穏やかになった。", "あなたは少し苛立ちが減じた。",
     "あなたは少し苛立ちが減じた。"},
#else
    {"You feel a little more calm.", "You feel a little less angry.",
     "You feel a little less angry."},
#endif

#ifdef JP 
    {"あなたは健康体になった。", "あなたは少し健康体に近づいた。",
     "あなたは少し健康体に近づいた。"},
#else
    {"You feel healthier.", "You feel a little healthier.",
     "You feel a little healthier."},
#endif

#ifdef JP 
    {"あなたの視力は鋭くなった。", "あなたの視力は少し鋭くなった。",
     "あなたの視力は少し鋭くなった。"},
#else
    {"Your vision sharpens.", "Your vision sharpens a little.",
     "Your vision sharpens a little."},
#endif
// 40

#ifdef JP 
    {"あなたは遺伝子的に不安定になった。", "あなたは遺伝子的に不安定になった。",
     "あなたは遺伝子的に不安定になった。"},
#else
    {"You feel genetically unstable.", "You feel genetically unstable.",
     "You feel genetically unstable."},
#endif

#ifdef JP 
    {"あなたは頑丈になった。", "あなたは頑丈になった。", "あなたは頑丈になった。"},
    {"あなたは脆弱になった。", "あなたは脆弱になった。", "あなたは脆弱になった。"},
#else
    {"You feel robust.", "You feel robust.", "You feel robust."},
    {"You feel frail.", "You feel frail.", "You feel frail."},
#endif

/* Some demonic powers (which can't be lost) start here... */
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
// 50
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},

#ifdef JP 
    {"あなたの爪は普通の大きさに縮んでしまった。",
     "あなたの爪は鋭さを失った。", "あなたの鉤爪は指に変化した。"},
#else
    {"Your fingernails shrink to normal size.",
     "Your fingernails look duller.", "Your hands feel fleshier."},
#endif

#ifdef JP 
    {"あなたの蹄が普通の足に変化した！", "", ""},
#else
    {"Your hooves expand and flesh out into feet!", "", ""},
#endif
    // 60
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
    {"", "", ""},
// 70

#ifdef JP 
    {"あなたの赤色の鱗が消えてしまった。", "あなたの赤色の鱗が幾らか減少した。",
     "あなたの赤色の鱗が幾らか減少した。"},
#else
    {"Your red scales disappear.", "Your red scales recede somewhat.",
     "Your red scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたのなめらかな真珠色の鱗が消えてしまった。",
     "あなたのなめらかな真珠色の鱗が幾らか減少した。",
     "あなたのなめらかな真珠色の鱗が幾らか減少した。"},
#else
    {"Your smooth nacreous scales disappear.",
     "Your smooth nacreous scales recede somewhat.",
     "Your smooth nacreous scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの隆起した灰色の鱗が消えてしまった。",
     "あなたの隆起した灰色の鱗が幾らか減少した。",
     "あなたの隆起した灰色の鱗が幾らか減少した。"},
#else
    {"Your ridged grey scales disappear.",
     "Your ridged grey scales recede somewhat.",
     "Your ridged grey scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの金属性の鱗が消えてしまった。",
     "あなたの金属性の鱗が幾らか減少した。",
     "あなたの金属性の鱗が幾らか減少した。"},
#else
    {"Your metallic scales disappear.",
     "Your metallic scales recede somewhat.",
     "Your metallic scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの黒い鱗が消えてしまった。",
     "あなたの黒い鱗が幾らか減少した。",
     "あなたの黒い鱗が幾らか減少した。"},
#else
    {"Your black scales disappear.", "Your black scales recede somewhat.",
     "Your black scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの白い鱗が消えてしまった。",
     "あなたの白い鱗が幾らか減少した。",
     "あなたの白い鱗が幾らか減少した。"},
#else
    {"Your white scales disappear.", "Your white scales recede somewhat.",
     "Your white scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの黄色の鱗が消えてしまった。",
     "あなたの黄色の鱗が幾らか減少した。",
     "あなたの黄色の鱗が幾らか減少した。"},
#else
    {"Your yellow scales disappear.", "Your yellow scales recede somewhat.",
     "Your yellow scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの茶色の鱗が消えてしまった。",
     "あなたの茶色の鱗が幾らか減少した。",
     "あなたの茶色の鱗が幾らか減少した。"},
#else
    {"Your brown scales disappear.", "Your brown scales recede somewhat.",
     "Your brown scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの青い鱗が消えてしまった。",
     "あなたの青い鱗が幾らか減少した。",
     "あなたの青い鱗が幾らか減少した。"},
#else
    {"Your blue scales disappear.", "Your blue scales recede somewhat.",
     "Your blue scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの紫色の鱗が消えてしまった。",
     "あなたの紫色の鱗が幾らか減少した。",
     "あなたの紫色の鱗が幾らか減少した。"},
#else
    {"Your purple scales disappear.", "Your purple scales recede somewhat.",
     "Your purple scales recede somewhat."},
#endif
// 80

#ifdef JP 
    {"あなたのまだら模様の鱗が消えてしまった。",
     "あなたのまだら模様の鱗が幾らか減少した。",
     "あなたのまだら模様の鱗が幾らか減少した。"},
#else
    {"Your speckled scales disappear.",
     "Your speckled scales recede somewhat.",
     "Your speckled scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたのオレンジ色の鱗が消えてしまった。",
     "あなたのオレンジ色の鱗が幾らか減少した。",
     "あなたのオレンジ色の鱗が幾らか減少した。"},
#else
    {"Your orange scales disappear.", "Your orange scales recede somewhat.",
     "Your orange scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの藍色の鱗が消えてしまった。",
     "あなたの藍色の鱗が幾らか減少した。",
     "あなたの藍色の鱗が幾らか減少した。"},
#else
    {"Your indigo scales disappear.", "Your indigo scales recede somewhat.",
     "Your indigo scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの節くれ立った赤い鱗が消えてしまった。",
     "あなたの節くれ立った赤い鱗が幾らか減少した。",
     "あなたの節くれ立った赤い鱗が幾らか減少した。"},
#else
    {"Your knobbly red scales disappear.",
     "Your knobbly red scales recede somewhat.",
     "Your knobbly red scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの玉虫色の鱗が消えてしまった。",
     "あなたの玉虫色の鱗が幾らか減少した。",
     "あなたの玉虫色の鱗が幾らか減少した。"},
#else
    {"Your iridescent scales disappear.",
     "Your iridescent scales recede somewhat.",
     "Your iridescent scales recede somewhat."},
#endif

#ifdef JP 
    {"あなたの模様の浮き出した鱗が消えてしまった。",
     "あなたの模様の浮き出した鱗が幾らか減少した。",
     "あなたの模様の浮き出した鱗が幾らか減少した。"},
#else
    {"Your patterned scales disappear.",
     "Your patterned scales recede somewhat.",
     "Your patterned scales recede somewhat."},
#endif
};

/*
   Chance out of 10 that mutation will be given/removed randomly. 0 means never.
 */
const char mutation_rarity[] = {
    10,                         // tough skin
    8,                          // str
    8,                          // int
    8,                          // dex
    2,                          // gr scales
    1,                          // bl scales
    2,                          // grey scales
    1,                          // bone
    1,                          // repuls field
    4,                          // res poison
// 10
    5,                          // carn
    5,                          // herb
    4,                          // res fire
    4,                          // res cold
    2,                          // res elec
    3,                          // regen
    10,                         // fast meta
    7,                          // slow meta
    10,                         // abil loss
    10,                         // ""
// 20
    10,                         // ""
    2,                          // tele control
    3,                          // teleport
    5,                          // res magic
    1,                          // run
    2,                          // see invis
    8,                          // deformation
    2,                          // teleport at will
    8,                          // spit poison
    3,                          // sense surr
// 30
    4,                          // breathe fire
    3,                          // blink
    7,                          // horns
    10,                         // strong/stiff muscles
    10,                         // weak/loose muscles
    6,                          // forgetfulness
    6,                          // clarity (as the amulet)
    7,                          // berserk/temper
    10,                         // deterioration
    10,                         // blurred vision
// 40
    4,                          // resist mutation
    10,                         // frail
    5,                          // robust
/* Some demonic powers start here: */
    0,
    0,
    0,
    0,
    0,
    0,
    0,
// 50
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    2,                          //jmf: claws
    1,                          //jmf: hooves
// 60
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
    0,
// 70
    2,                          // red scales
    1,                          // nac scales
    2,                          // r-grey scales
    1,                          // metal scales
    2,                          // black scales
    2,                          // wh scales
    2,                          // yel scales
    2,                          // brown scales
    2,                          // blue scales
    2,                          // purple scales
// 80
    2,                          // speckled scales
    2,                          // orange scales
    2,                          // indigo scales
    1,                          // kn red scales
    1,                          // irid scales
    1,                          // pattern scales
    0,                          //
    0,                          //
    0,                          //
    0                           //
};

void display_mutations(void)
{
    int i;
    int j = 0;
#ifdef JP  
    const char *mut_title = "先天的/超自然的/もしくは突然変異の能力";
#else
    const char *mut_title = "Innate abilities, Weirdness & Mutations";
#endif
    const int num_lines = get_number_of_lines(); 

#ifdef DOS_TERM
    char buffer[4800];

    window(1, 1, 80, 25);
    gettext(1, 1, 80, 25, buffer);
#endif

    clrscr();
    textcolor(WHITE);

    // center title
    i = 40 - strlen(mut_title) / 2;
    if (i<1) i=1;
    gotoxy(i, 1);
    cprintf(mut_title);
    gotoxy(1,3);
    textcolor(LIGHTBLUE);  //textcolor for inborn abilities and weirdness

    switch (you.species)   //mv: following code shows innate abilities - if any
    {
    case SP_MERFOLK:
#ifdef JP 
        cprintf("あなたは水中では本来の形態に戻る。" EOL);
#else
        cprintf("You revert to your normal form in water." EOL);
#endif
        j++;
        break;

    case SP_NAGA:
        // breathe poison replaces spit poison:
        if (!you.mutation[MUT_BREATHE_POISON])
#ifdef JP 
            cprintf("あなたは毒液を吐くことができる。" EOL);
#else
            cprintf("You can spit poison." EOL);
#endif
        else
#ifdef JP 
            cprintf("あなたは毒の雲を吐き出すことができる。" EOL);
#else
            cprintf("You can exhale a cloud of poison." EOL);
#endif

#ifdef JP 
        cprintf("あなたの肉体は毒に耐性がある。" EOL);
        cprintf("あなたは見えざるものを見ることができる。" EOL);
#else
        cprintf("Your system is immune to poisons." EOL);
        cprintf("You can see invisible." EOL);
#endif
        j += 3;
        break;

    case SP_GNOME:
#ifdef JP 
        cprintf("あなたは周辺を感知することができる。" EOL);
#else
        cprintf("You can sense your surroundings." EOL);
#endif
        j++;
        break;

    case SP_TROLL:
#ifdef JP 
        cprintf("あなたの肉体は高速に再生する。" EOL);
#else
        cprintf("Your body regenerates quickly." EOL);
#endif
        j++;
        break;

    case SP_GHOUL:
#ifdef JP 
        cprintf("あなたの体は腐り落ちていく。" EOL);
        cprintf("あなたは肉食性だ。" EOL);
#else
        cprintf("Your body is rotting away." EOL);
        cprintf("You are carnivorous." EOL);
#endif
        j += 2;
        break;

    case SP_KOBOLD:
#ifdef JP 
        cprintf("あなたは肉食性だ。" EOL);
#else
        cprintf("You are carnivorous." EOL);
#endif
        j++;
        break;

    case SP_GREY_ELF:
        if (you.experience_level > 4)
        {
#ifdef JP 
            cprintf("あなたはとても魅力的だ。" EOL);
#else
            cprintf("You are very charming." EOL);
#endif
            j++;
        }
        break;

    case SP_KENKU:
        if (you.experience_level > 4)
        {
#ifdef JP 
            cprintf("あなたは");
            cprintf((you.experience_level > 14) ? "絶え間なく飛ぶことができる。" EOL : "飛ぶことができる。"
                    EOL);
#else
            cprintf("You can fly");
            cprintf((you.experience_level > 14) ? " continuously." EOL : "."
                    EOL);
#endif
            j++;
        }
        break;

    case SP_MUMMY:
#ifdef JP 
        cprintf("あなたは");
        cprintf((you.experience_level > 25) ? "非常に強く" :
                ((you.experience_level > 12) ? "強く" : ""));
        cprintf("死の力との接触を持っている。" EOL);
#else
        cprintf("You are");
        cprintf((you.experience_level > 25) ? " very strongly" :
                ((you.experience_level > 12) ? " strongly" : ""));
        cprintf(" in touch with the powers of death." EOL);
#endif
        j++;

        if (you.experience_level >= 12)
        {
#ifdef JP 
            cprintf("あなたは腐敗した肉体を回復するために肉体に魔力を充填できる。" EOL);
#else
            cprintf("You can restore your body by infusing magical energy." EOL);
#endif
            j++;
        }
        break;

    case SP_GREEN_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは毒に耐性がある。" EOL);
            cprintf("あなたは毒のブレスを吐くことができる。" EOL);
#else
            cprintf("You are resistant to poison." EOL);
            cprintf("You can breathe poison." EOL);
#endif
            j += 2;
        }
        break;

    case SP_RED_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは火のブレスを吐くことができる。" EOL);
#else
            cprintf("You can breathe fire." EOL);
#endif
            j++;
        }
        if (you.experience_level > 17)
        {
#ifdef JP 
            cprintf("あなたは火に耐性がある。" EOL);
#else
            cprintf("You are resistant to fire." EOL);
#endif
            j++;
        }
        break;

    case SP_WHITE_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは冷気のブレスを吐くことができる。" EOL);
#else
            cprintf("You can breathe frost." EOL);
#endif
            j++;
        }
        if (you.experience_level > 17)
        {
#ifdef JP 
            cprintf("あなたは冷気に耐性がある。" EOL);
#else
            cprintf("You are resistant to cold." EOL);
#endif
            j++;
        }
        break;

    case SP_BLACK_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは稲妻のブレスを吐くことができる。" EOL);
#else
            cprintf("You can breathe lightning." EOL);
#endif
            j++;
        }
        if (you.experience_level > 17)
        {
#ifdef JP 
            cprintf("あなたは稲妻に耐性がある。" EOL);
#else
            cprintf("You are resistant to lightning." EOL);
#endif
            j++;
        }
        break;

    case SP_GOLDEN_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは強酸を吐くことができる。" EOL);
#else
            cprintf("You can spit acid." EOL);
#endif
            j++;
        }
        break;

    case SP_PURPLE_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは力のブレスを吐くことができる。" EOL);
#else
            cprintf("You can breathe power." EOL);
#endif
            j++;
        }
        break;

    case SP_MOTTLED_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは焼夷の炎のブレスを吐くことができる。" EOL);
#else
            cprintf("You can breathe sticky flames." EOL);
#endif
            j++;
        }
        break;

    case SP_PALE_DRACONIAN:
        if (you.experience_level > 6)
        {
#ifdef JP 
            cprintf("あなたは蒸気のブレスを吐くことができる。" EOL);
#else
            cprintf("You can breathe steam." EOL);
#endif
            j++;
        }
        break;
    }                           //end switch - innate abilities

    textcolor(LIGHTGREY);

    for (i = 0; i < 100; i++)
    {
        if (you.mutation[i] != 0)
        {
            // this is already handled above:
            if (you.species == SP_NAGA && i == MUT_BREATHE_POISON)
                continue;

            j++;
            textcolor(LIGHTGREY);

            if (j > num_lines - 4)
            {
                gotoxy( 1, num_lines - 1 );
#ifdef JP 
                cprintf("-続く-");
#else
                cprintf("-more-");
#endif

                if (getch() == 0)
                    getch();

                clrscr();

                // center title
                int x = 40 - strlen(mut_title) / 2;
                if (x < 1) 
                    x = 1;

                gotoxy(x, 1);
                textcolor(WHITE);
                cprintf(mut_title);
                textcolor(LIGHTGREY);
                gotoxy(1,3);
                j = 1;
            }

            /* mutation is actually a demonic power */
            if (you.demon_pow[i] != 0)
                textcolor(RED);

            /* same as above, but power is enhanced by mutation */
            if (you.demon_pow[i] != 0 && you.demon_pow[i] < you.mutation[i])
                textcolor(LIGHTRED);

            cprintf( mutation_name( i ) );
            cprintf(EOL);
        }
    }

    if (j == 0)
#ifdef JP 
        cprintf( "あなたは変異を蒙っていない。" EOL );
#else
        cprintf( "You are not a mutant." EOL );
#endif

#ifndef USE_MULTIWIN // skip getch
    if (getch() == 0)
        getch();
#endif

#ifdef DOS_TERM
    puttext(1, 1, 80, 25, buffer);
#endif

    //cprintf("xxxxxxxxxxxxx");
    //last_requested = 0;

    return;
}                               // end display_mutations()

bool mutate(int which_mutation, bool failMsg)
{
    char mutat = which_mutation;
    bool force_mutation = false;        // is mutation forced?
    int  i;

    if (which_mutation >= 1000) // must give mutation without failure
    {
        force_mutation = true;
        mutat -= 1000;
        which_mutation -= 1000;
    }

    // Undead bodies don't mutate, they fall apart. -- bwr
    if (you.is_undead) 
    {
        if (force_mutation 
            || (!wearing_amulet(AMU_RESIST_MUTATION) && coinflip()))
        {
#ifdef JP 
            mpr( "あなたの肉体は腐敗した！" );
#else
            mpr( "Your body decomposes!" );
#endif

            if (coinflip())
                lose_stat( STAT_RANDOM, 1 );
            else
            {
                ouch( 3, 0, KILLED_BY_ROTTING );
                rot_hp( roll_dice( 1, 3 ) );
            }

            return (true);
        }

        if (failMsg)
#ifdef JP 
            mpr("あなたは一瞬だけ奇妙な感覚になった。");
#else
            mpr("You feel odd for a moment.");
#endif

        return (false);
    }

    if (wearing_amulet(AMU_RESIST_MUTATION)
        && !force_mutation && !one_chance_in(10))
    {
        if (failMsg)
#ifdef JP 
            mpr("あなたは一瞬だけ奇妙な感覚になった。");
#else
            mpr("You feel odd for a moment.");
#endif

        return (false);
    }

    if (you.mutation[MUT_MUTATION_RESISTANCE]
        && !force_mutation
        && (you.mutation[MUT_MUTATION_RESISTANCE] == 3 || !one_chance_in(3)))
    {
        if (failMsg)
#ifdef JP 
            mpr("あなたは一瞬だけ奇妙な感覚になった。");
#else
            mpr("You feel odd for a moment.");
#endif

        return (false);
    }

    if (which_mutation == 100 && random2(15) < how_mutated())
    {
        if (!force_mutation && !one_chance_in(3))
            return (false);
        else
            return (delete_mutation(100));
    }

    if (which_mutation == 100)
    {
        do
        {
            mutat = random2(NUM_MUTATIONS);

            if (one_chance_in(1000))
                return false;
        }
        while ((you.mutation[mutat] >= 3
                && (mutat != MUT_STRONG && mutat != MUT_CLEVER
                    && mutat != MUT_AGILE) && (mutat != MUT_WEAK
                                               && mutat != MUT_DOPEY
                                               && mutat != MUT_CLUMSY))
               || you.mutation[mutat] > 13
               || random2(10) >= mutation_rarity[mutat] + you.demon_pow[mutat]);
    }

    if (you.mutation[mutat] >= 3
        && (mutat != MUT_STRONG && mutat != MUT_CLEVER && mutat != MUT_AGILE)
        && (mutat != MUT_WEAK && mutat != MUT_DOPEY && mutat != MUT_CLUMSY))
    {
        return false;
    }

    if (you.mutation[mutat] > 13 && !force_mutation)
        return false;

    // These can be forced by demonspawn
    if ((mutat == MUT_TOUGH_SKIN
         || (mutat >= MUT_GREEN_SCALES && mutat <= MUT_BONEY_PLATES)
         || (mutat >= MUT_RED_SCALES && mutat <= MUT_PATTERNED_SCALES))
        && body_covered() >= 3 && !force_mutation)
    {
        return false;
    }

    if (mutat == MUT_HORNS && you.species == SP_MINOTAUR)
        return false;

    // nagas have see invis and res poison and can spit poison
    if (you.species == SP_NAGA)
    {
        if (mutat == MUT_ACUTE_VISION || mutat == MUT_POISON_RESISTANCE)
            return false;

        // gdl: spit poison 'upgrades' to breathe poison.  Why not..
        if (mutat == MUT_SPIT_POISON)
        {
            if (coinflip())
                return false;
            {
                mutat = MUT_BREATHE_POISON;

                // breathe poison replaces spit poison (so it takes the slot)
                for (i = 0; i < 52; i++)
                {
                    if (you.ability_letter_table[i] == ABIL_SPIT_POISON)
                        you.ability_letter_table[i] = ABIL_BREATHE_POISON;
                }
            }
        }
    }

    // gnomes can already sense surroundings
    if (you.species == SP_GNOME && mutat == MUT_MAPPING)
        return false;

    // spriggans already run at max speed (centaurs can get a bit faster)
    if (you.species == SP_SPRIGGAN && mutat == MUT_FAST)
        return false;

    // this might have issues if we allowed it -- bwr
    if (you.species == SP_KOBOLD 
        && (mutat == MUT_CARNIVOROUS || mutat == MUT_HERBIVOROUS))
    {
        return (false);
    }

    // This one can be forced by demonspawn
    if (mutat == MUT_REGENERATION
        && you.mutation[MUT_SLOW_METABOLISM] > 0 && !force_mutation)
    {
        return false;           /* if you have a slow metabolism, no regen */
    }

    if (mutat == MUT_SLOW_METABOLISM && you.mutation[MUT_REGENERATION] > 0)
        return false;           /* if you have a slow metabolism, no regen */

    // This one can be forced by demonspawn
    if (mutat == MUT_ACUTE_VISION
        && you.mutation[MUT_BLURRY_VISION] > 0 && !force_mutation)
    {
        return false;
    }

    if (mutat == MUT_BLURRY_VISION && you.mutation[MUT_ACUTE_VISION] > 0)
        return false;           /* blurred vision/see invis */

    //jmf: added some checks for new mutations
    if (mutat == MUT_STINGER
        && !(you.species == SP_NAGA || player_genus(GENPC_DRACONIAN)))
    {
        return false;
    }

    // putting boots on after they are forced off. -- bwr
    if (mutat == MUT_HOOVES
        && (you.species == SP_NAGA || you.species == SP_CENTAUR
            || you.species == SP_KENKU || player_genus(GENPC_DRACONIAN)))
    {
        return false;
    }

    if (mutat == MUT_BIG_WINGS && !player_genus(GENPC_DRACONIAN))
        return false;

    //jmf: added some checks for new mutations
#ifdef JP 
    mpr("あなたは突然変異した。", MSGCH_MUTATION);
#else
    mpr("You mutate.", MSGCH_MUTATION);
#endif

    // find where these things are actually changed
    // -- do not globally force redraw {dlb}
    you.redraw_hit_points = 1;
    you.redraw_magic_points = 1;
    you.redraw_armour_class = 1;
    you.redraw_evasion = 1;
    you.redraw_experience = 1;
    you.redraw_gold = 1;
    //you.redraw_hunger = 1;

    switch (mutat)
    {
    case MUT_STRONG:
        if (you.mutation[MUT_WEAK] > 0)
        {
            delete_mutation(MUT_WEAK);
            return true;
        }
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_STRENGTH, 1, false);
        break;

    case MUT_CLEVER:
        if (you.mutation[MUT_DOPEY] > 0)
        {
            delete_mutation(MUT_DOPEY);
            return true;
        }
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_INTELLIGENCE, 1, false);
        break;

    case MUT_AGILE:
        if (you.mutation[MUT_CLUMSY] > 0)
        {
            delete_mutation(MUT_CLUMSY);
            return true;
        }
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_DEXTERITY, 1, false);
        break;

    case MUT_WEAK:
        if (you.mutation[MUT_STRONG] > 0)
        {
            delete_mutation(MUT_STRONG);
            return true;
        }
        modify_stat(STAT_STRENGTH, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_DOPEY:
        if (you.mutation[MUT_CLEVER] > 0)
        {
            delete_mutation(MUT_CLEVER);
            return true;
        }
        modify_stat(STAT_INTELLIGENCE, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_CLUMSY:
        if (you.mutation[MUT_AGILE] > 0)
        {
            delete_mutation(MUT_AGILE);
            return true;
        }
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_REGENERATION:
        if (you.mutation[MUT_SLOW_METABOLISM] > 0)
        {
            // Should only get here from demonspawn, where our innate
            // ability will clear away the counter-mutation.
            while (delete_mutation(MUT_SLOW_METABOLISM))
                ;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_ACUTE_VISION:
        if (you.mutation[MUT_BLURRY_VISION] > 0)
        {
            // Should only get here from demonspawn, where our inate
            // ability will clear away the counter-mutation.
            while (delete_mutation(MUT_BLURRY_VISION))
                ;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_CARNIVOROUS:
        if (you.mutation[MUT_HERBIVOROUS] > 0)
        {
            delete_mutation(MUT_HERBIVOROUS);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_HERBIVOROUS:
        if (you.mutation[MUT_CARNIVOROUS] > 0)
        {
            delete_mutation(MUT_CARNIVOROUS);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_SHOCK_RESISTANCE:
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_FAST_METABOLISM:
        if (you.mutation[MUT_SLOW_METABOLISM] > 0)
        {
            delete_mutation(MUT_SLOW_METABOLISM);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_SLOW_METABOLISM:
        if (you.mutation[MUT_FAST_METABOLISM] > 0)
        {
            delete_mutation(MUT_FAST_METABOLISM);
            return true;
        }
        //if (you.mutation[mutat] == 0 || you.mutation[mutat] == 2)
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_TELEPORT_CONTROL:
        you.attribute[ATTR_CONTROL_TELEPORT]++;
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;


    case MUT_HOOVES:            //jmf: like horns
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        if (you.equip[EQ_BOOTS] != -1)
        {
            FixedVector < char, 8 > removed;

            for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
            {
                removed[i] = 0;
            }

            removed[EQ_BOOTS] = 1;
            remove_equipment(removed);
        }
        break;

    case MUT_CLAWS:
        mpr( gain_mutation[ mutat ][ you.mutation[mutat] ], MSGCH_MUTATION );

        // gloves aren't prevented until level three
        if (you.mutation[ mutat ] >= 3 && you.equip[ EQ_GLOVES ] != -1)
        {
            FixedVector < char, 8 > removed;

            for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
            {
                removed[i] = 0;
            }

            removed[ EQ_GLOVES ] = 1;
            remove_equipment( removed );
        }
        break;

    case MUT_HORNS:             // horns force your helmet off
        {
            mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);

            if (you.equip[EQ_HELMET] != -1
                && you.inv[you.equip[EQ_HELMET]].plus2 > 1)
            {
                break;          // horns don't push caps/wizard hats off
            }

            FixedVector < char, 8 > removed;

            for (int i = EQ_WEAPON; i < EQ_RIGHT_RING; i++)
            {
                removed[i] = 0;
            }

            removed[EQ_HELMET] = 1;
            remove_equipment(removed);
        }
        break;

    case MUT_STRONG_STIFF:
        if (you.mutation[MUT_FLEXIBLE_WEAK] > 0)
        {
            delete_mutation(MUT_FLEXIBLE_WEAK);
            return true;
        }
        modify_stat(STAT_STRENGTH, 1, true);
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FLEXIBLE_WEAK:
        if (you.mutation[MUT_STRONG_STIFF] > 0)
        {
            delete_mutation(MUT_STRONG_STIFF);
            return true;
        }
        modify_stat(STAT_STRENGTH, -1, true);
        modify_stat(STAT_DEXTERITY, 1, true);
        mpr(gain_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FRAIL:
        if (you.mutation[MUT_ROBUST] > 0)
        {
            delete_mutation(MUT_ROBUST);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        you.mutation[mutat]++;
        calc_hp();
        return true;

    case MUT_ROBUST:
        if (you.mutation[MUT_FRAIL] > 0)
        {
            delete_mutation(MUT_FRAIL);
            return true;
        }
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        you.mutation[mutat]++;
        calc_hp();
        return true;

    case MUT_BLACK_SCALES:
    case MUT_BONEY_PLATES:
        modify_stat(STAT_DEXTERITY, -1, true);
        // deliberate fall-through
    default:
        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_GREY2_SCALES:
        if (you.mutation[mutat] != 1)
            modify_stat(STAT_DEXTERITY, -1, true);

        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_METALLIC_SCALES:
        if (you.mutation[mutat] == 0)
            modify_stat(STAT_DEXTERITY, -2, true);
        else
            modify_stat(STAT_DEXTERITY, -1, true);

        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;

    case MUT_RED2_SCALES:
    case MUT_YELLOW_SCALES:
        if (you.mutation[mutat] != 0)
            modify_stat(STAT_DEXTERITY, -1, true);

        mpr(gain_mutation[mutat][you.mutation[mutat]], MSGCH_MUTATION);
        break;
    }

    you.mutation[mutat]++;

    /* remember, some mutations don't get this far (eg frail) */
    return true;
}                               // end mutation()

int how_mutated(void)
{
    int j = 0;

    for (int i = 0; i < 100; i++)
    {
        if (you.mutation[i] && you.demon_pow[i] < you.mutation[i])
        {
            // these allow for 14 levels:
            if (i == MUT_STRONG || i == MUT_CLEVER || i == MUT_AGILE
                || i == MUT_WEAK || i == MUT_DOPEY || i == MUT_CLUMSY)
            {
                j += (you.mutation[i] / 5 + 1);
            }
            else 
            {
                j += you.mutation[i];
            }
        }
    }

#if DEBUG_DIAGNOSTICS
#ifdef JP 
    snprintf( info, INFO_SIZE, "levels: %d", j );
#else
    snprintf( info, INFO_SIZE, "levels: %d", j );
#endif
    mpr( info, MSGCH_DIAGNOSTICS );
#endif

    return (j);
}                               // end how_mutated()

bool delete_mutation(char which_mutation)
{
    char mutat = which_mutation;
    int i;

    if (you.mutation[MUT_MUTATION_RESISTANCE] > 1
        && (you.mutation[MUT_MUTATION_RESISTANCE] == 3 || coinflip()))
    {
#ifdef JP 
        mpr("あなたは一瞬、どうにも奇妙な感覚を覚えた。");
#else
        mpr("You feel rather odd for a moment.");
#endif
        return false;
    }

    if (which_mutation == 100)
    {
        do
        {
            mutat = random2(NUM_MUTATIONS);
            if (one_chance_in(1000))
                return false;
        }
        while ((you.mutation[mutat] == 0
                   && (mutat != MUT_STRONG && mutat != MUT_CLEVER && mutat != MUT_AGILE) 
                   && (mutat != MUT_WEAK && mutat != MUT_DOPEY && mutat != MUT_CLUMSY))
               || random2(10) >= mutation_rarity[mutat]
               || you.demon_pow[mutat] >= you.mutation[mutat]);
    }

    if (you.mutation[mutat] == 0)
        return false;

    if (you.demon_pow[mutat] >= you.mutation[mutat])
        return false;

#ifdef JP 
    mpr("あなたは突然変異した。", MSGCH_MUTATION);
#else
    mpr("You mutate.", MSGCH_MUTATION);
#endif

    switch (mutat)
    {
    case MUT_STRONG:
        modify_stat(STAT_STRENGTH, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_CLEVER:
        modify_stat(STAT_INTELLIGENCE, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_AGILE:
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_WEAK:
        modify_stat(STAT_STRENGTH, 1, false);
        break;

    case MUT_DOPEY:
        modify_stat(STAT_INTELLIGENCE, 1, false);
        break;

    case MUT_CLUMSY:
        // replaces earlier, redundant code - 12mar2000 {dlb}
        modify_stat(STAT_DEXTERITY, 1, false);
        break;

    case MUT_SHOCK_RESISTANCE:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_FAST_METABOLISM:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_SLOW_METABOLISM:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_TELEPORT_CONTROL:
        you.attribute[ATTR_CONTROL_TELEPORT]--;
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_STRONG_STIFF:
        modify_stat(STAT_STRENGTH, -1, true);
        modify_stat(STAT_DEXTERITY, 1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FLEXIBLE_WEAK:
        modify_stat(STAT_STRENGTH, 1, true);
        modify_stat(STAT_DEXTERITY, -1, true);
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        break;

    case MUT_FRAIL:
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        if (you.mutation[mutat] > 0)
            you.mutation[mutat]--;
        calc_hp();
        return true;

    case MUT_ROBUST:
        mpr(lose_mutation[mutat][0], MSGCH_MUTATION);
        if (you.mutation[mutat] > 0)
            you.mutation[mutat]--;
        calc_hp();
        return true;

    case MUT_BLACK_SCALES:
    case MUT_BONEY_PLATES:
        modify_stat(STAT_DEXTERITY, 1, true);

    default:
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_GREY2_SCALES:
        if (you.mutation[mutat] != 2)
            modify_stat(STAT_DEXTERITY, 1, true);
        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_METALLIC_SCALES:
        if (you.mutation[mutat] == 1)
            modify_stat(STAT_DEXTERITY, 2, true);
        else
            modify_stat(STAT_DEXTERITY, 1, true);

        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_RED2_SCALES:
    case MUT_YELLOW_SCALES:
        if (you.mutation[mutat] != 1)
            modify_stat(STAT_DEXTERITY, 1, true);

        mpr(lose_mutation[mutat][you.mutation[mutat] - 1], MSGCH_MUTATION);
        break;

    case MUT_BREATHE_POISON:
        // can't be removed yet, but still covered:
        if (you.species == SP_NAGA)
        {
            // natural ability to spit poison retakes the slot
            for (i = 0; i < 52; i++)
            {
                if (you.ability_letter_table[i] == ABIL_BREATHE_POISON)
                    you.ability_letter_table[i] = ABIL_SPIT_POISON;
            }
        }
        break;
    }

    // find where these things are actually altered
    /// -- do not globally force redraw {dlb}
    you.redraw_hit_points = 1;
    you.redraw_magic_points = 1;
    you.redraw_armour_class = 1;
    you.redraw_evasion = 1;
    you.redraw_experience = 1;
    you.redraw_gold = 1;
    //you.redraw_hunger = 1;

    if (you.mutation[mutat] > 0)
        you.mutation[mutat]--;

    return true;
}                               // end delete_mutation()

char body_covered(void)
{
    /* checks how much of your body is covered by scales etc */
    char covered = 0;

    if (you.species == SP_NAGA)
        covered++;

    if (player_genus(GENPC_DRACONIAN))
        return 3;

    covered += you.mutation[MUT_TOUGH_SKIN];
    covered += you.mutation[MUT_GREEN_SCALES];
    covered += you.mutation[MUT_BLACK_SCALES];
    covered += you.mutation[MUT_GREY_SCALES];
    covered += you.mutation[MUT_BONEY_PLATES];
    covered += you.mutation[MUT_RED_SCALES];
    covered += you.mutation[MUT_NACREOUS_SCALES];
    covered += you.mutation[MUT_GREY2_SCALES];
    covered += you.mutation[MUT_METALLIC_SCALES];
    covered += you.mutation[MUT_BLACK2_SCALES];
    covered += you.mutation[MUT_WHITE_SCALES];
    covered += you.mutation[MUT_YELLOW_SCALES];
    covered += you.mutation[MUT_BROWN_SCALES];
    covered += you.mutation[MUT_BLUE_SCALES];
    covered += you.mutation[MUT_PURPLE_SCALES];
    covered += you.mutation[MUT_SPECKLED_SCALES];
    covered += you.mutation[MUT_ORANGE_SCALES];
    covered += you.mutation[MUT_INDIGO_SCALES];
    covered += you.mutation[MUT_RED2_SCALES];
    covered += you.mutation[MUT_IRIDESCENT_SCALES];
    covered += you.mutation[MUT_PATTERNED_SCALES];

    return covered;
}

const char *mutation_name( char which_mutat, int level )
{
    static char mut_string[INFO_SIZE];

    // level == -1 means default action of current level
    if (level == -1)
        level = you.mutation[ which_mutat ];

    if (which_mutat == MUT_STRONG || which_mutat == MUT_CLEVER
        || which_mutat == MUT_AGILE || which_mutat == MUT_WEAK
        || which_mutat == MUT_DOPEY || which_mutat == MUT_CLUMSY)
    {
#ifdef JP 
        snprintf( mut_string, sizeof( mut_string ), "%s%d)", 
#else
        snprintf( mut_string, sizeof( mut_string ), "%s%d).", 
#endif
                  mutation_descrip[ which_mutat ][0], level );

        return (mut_string);
    }

    // Some mutations only have one "level", and it's better
    // to show the first level description than a blank description.
    if (mutation_descrip[ which_mutat ][ level - 1 ][0] == '\0')
        return (mutation_descrip[ which_mutat ][ 0 ]);
    else 
        return (mutation_descrip[ which_mutat ][ level - 1 ]);
}                               // end mutation_name()

/* Use an attribute counter for how many demonic mutations a dspawn has */
void demonspawn(void)
{
    int whichm = -1;
    char howm = 1;
    int counter = 0;

    const int scale_levels = body_covered();

    you.attribute[ATTR_NUM_DEMONIC_POWERS]++;

#ifdef JP 
    mpr("あなたの悪魔的な血統が顕在化した……。", MSGCH_INTRINSIC_GAIN);
#else
    mpr("Your demonic ancestry asserts itself...", MSGCH_INTRINSIC_GAIN);
#endif

    // Merged the demonspawn lists into a single loop.  Now a high level
    // character can potentially get mutations from the low level list if 
    // its having trouble with the high level list.
    do
    {
        if (you.experience_level >= 10)
        {
            if (you.skills[SK_CONJURATIONS] < 5)
            {                       // good conjurers don't get bolt of draining
                whichm = MUT_SMITE;
                howm = 1;
            }

            if (you.skills[SK_CONJURATIONS] < 10 && one_chance_in(4))
            {                       // good conjurers don't get hellfire
                whichm = MUT_HURL_HELLFIRE;
                howm = 1;
            }

            if (you.skills[SK_SUMMONINGS] < 5 && one_chance_in(3))
            {                       // good summoners don't get summon demon
                whichm = MUT_SUMMON_DEMONS;
                howm = 1;
            }

            if (one_chance_in(8))
            {
                whichm = MUT_MAGIC_RESISTANCE;
                howm = (coinflip() ? 2 : 3);
            }

            if (one_chance_in(12))
            {
                whichm = MUT_FAST;
                howm = 1;
            }

            if (one_chance_in(7))
            {
                whichm = MUT_TELEPORT_AT_WILL;
                howm = 2;
            }

            if (one_chance_in(10))
            {
                whichm = MUT_REGENERATION;
                howm = (coinflip() ? 2 : 3);
            }

            if (one_chance_in(12))
            {
                whichm = MUT_SHOCK_RESISTANCE;
                howm = 1;
            }

            if (!you.mutation[MUT_CALL_TORMENT] && one_chance_in(15))
            {
                whichm = MUT_TORMENT_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(12))
            {
                whichm = MUT_NEGATIVE_ENERGY_RESISTANCE;
                howm = 1 + random2(3);
            }

            if (!you.mutation[MUT_TORMENT_RESISTANCE] && one_chance_in(20))
            {
                whichm = MUT_CALL_TORMENT;
                howm = 1;
            }

            if (you.skills[SK_SUMMONINGS] < 5 && you.skills[SK_NECROMANCY] < 5
                && one_chance_in(12))
            {
                whichm = MUT_CONTROL_DEMONS;
                howm = 1;
            }

            if (you.skills[SK_TRANSLOCATIONS] < 5 && one_chance_in(15))
            {
                whichm = MUT_PANDEMONIUM;
                howm = 1;
            }

            if (you.religion != GOD_VEHUMET && one_chance_in(11))
            {
                whichm = MUT_DEATH_STRENGTH;
                howm = 1;
            }

            if (you.religion != GOD_VEHUMET && one_chance_in(11))
            {
                whichm = MUT_CHANNEL_HELL;
                howm = 1;
            }

            if (you.skills[SK_SUMMONINGS] < 3 && you.skills[SK_NECROMANCY] < 3
                && one_chance_in(10))
            {
                whichm = MUT_RAISE_DEAD;
                howm = 1;
            }

            if (you.skills[SK_UNARMED_COMBAT] > 5 && one_chance_in(14))
            {
                whichm = MUT_DRAIN_LIFE;
                howm = 1;
            }
        }

        // check here so we can see if we need to extent our options:
        if (whichm != -1 && you.mutation[whichm] != 0)
            whichm = -1;

        if (you.experience_level < 10 || (counter > 0 && whichm == -1))
        {
            if ((!you.mutation[MUT_THROW_FROST]         // only one of these
                    && !you.mutation[MUT_THROW_FLAMES]
                    && !you.mutation[MUT_BREATHE_FLAMES])
                && (!you.skills[SK_CONJURATIONS]        // conjurers seldomly
                    || one_chance_in(5))
                && (!you.skills[SK_ICE_MAGIC]           // already ice & fire?
                    || !you.skills[SK_FIRE_MAGIC]))
            {
                // try to give the flavour the character doesn't have:
                if (!you.skills[SK_FIRE_MAGIC])
                    whichm = MUT_THROW_FLAMES;
                else if (!you.skills[SK_ICE_MAGIC])
                    whichm = MUT_THROW_FROST;
                else
                    whichm = (coinflip() ? MUT_THROW_FLAMES : MUT_THROW_FROST);

                howm = 1;
            }

            if (!you.skills[SK_SUMMONINGS] && one_chance_in(3))
            {                           /* summoners don't get summon imp */
                whichm = (you.experience_level < 10) ? MUT_SUMMON_MINOR_DEMONS
                                                     : MUT_SUMMON_DEMONS;
                howm = 1;
            }

            if (one_chance_in(4))
            {
                whichm = MUT_POISON_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(4))
            {
                whichm = MUT_COLD_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(4))
            {
                whichm = MUT_HEAT_RESISTANCE;
                howm = 1;
            }

            if (one_chance_in(5))
            {
                whichm = MUT_ACUTE_VISION;
                howm = 1;
            }

            if (!you.skills[SK_POISON_MAGIC] && one_chance_in(7))
            {
                whichm = MUT_SPIT_POISON;
                howm = (you.experience_level < 10) ? 1 : 3;
            }

            if (one_chance_in(10))
            {
                whichm = MUT_MAPPING;
                howm = 3;
            }

            if (one_chance_in(12))
            {
                whichm = MUT_TELEPORT_CONTROL;
                howm = 1;
            }

            if (!you.mutation[MUT_THROW_FROST]         // not with these
                && !you.mutation[MUT_THROW_FLAMES]
                && !you.mutation[MUT_BREATHE_FLAMES]
                && !you.skills[SK_FIRE_MAGIC]          // or with fire already
                && one_chance_in(5))
            {
                whichm = MUT_BREATHE_FLAMES;
                howm = 2;
            }

            if (!you.skills[SK_TRANSLOCATIONS] && one_chance_in(12))
            {
                whichm = (you.experience_level < 10) ? MUT_BLINK
                                                     : MUT_TELEPORT_AT_WILL;
                howm = 2;
            }

            if (scale_levels < 3 && one_chance_in( 1 + scale_levels * 5 ))
            {
                const int bonus = (you.experience_level < 10) ? 0 : 1;
                int levels = 0;

                if (one_chance_in(10))
                {
                    whichm = MUT_TOUGH_SKIN;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(24))
                {
                    whichm = MUT_GREEN_SCALES;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(24))
                {
                    whichm = MUT_BLACK_SCALES;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(24))
                {
                    whichm = MUT_GREY_SCALES;
                    levels = (coinflip() ? 2 : 3);
                }

                if (one_chance_in(12))
                {
                    whichm = MUT_RED_SCALES + random2(16);

                    switch (whichm)
                    {
                    case MUT_RED_SCALES:
                    case MUT_NACREOUS_SCALES:
                    case MUT_BLACK2_SCALES:
                    case MUT_WHITE_SCALES:
                    case MUT_BLUE_SCALES:
                    case MUT_SPECKLED_SCALES:
                    case MUT_ORANGE_SCALES:
                    case MUT_IRIDESCENT_SCALES:
                    case MUT_PATTERNED_SCALES:
                        levels = (coinflip() ? 2 : 3);
                        break;

                    default:
                        levels = (coinflip() ? 1 : 2);
                        break;
                    }
                }

                if (one_chance_in(30))
                {
                    whichm = MUT_BONEY_PLATES;
                    levels = (coinflip() ? 1 : 2);
                }

                if (levels)
                    howm = MINIMUM( 3 - scale_levels, levels + bonus );
            }

            if (one_chance_in(25))
            {
                whichm = MUT_REPULSION_FIELD;
                howm = (coinflip() ? 2 : 3);
            }

            if (one_chance_in( (you.experience_level < 10) ? 5 : 20 ))
            {
                whichm = MUT_HORNS;
                howm = (coinflip() ? 1 : 2);

                if (you.experience_level > 4 || one_chance_in(5))
                    howm++;
            }
        }

        if (whichm != -1 && you.mutation[whichm] != 0)
            whichm = -1;

        counter++;
    }
    while (whichm == -1 && counter < 5000);

    if (whichm == -1 || !perma_mutate( whichm, howm ))
    {
        /* unlikely but remotely possible */
        /* I know this is a cop-out */
        modify_stat(STAT_STRENGTH, 1, true);
        modify_stat(STAT_INTELLIGENCE, 1, true);
        modify_stat(STAT_DEXTERITY, 1, true);
#ifdef JP 
        mpr("あなたは非常に体調が良くなった。", MSGCH_INTRINSIC_GAIN);
#else
        mpr("You feel much better now.", MSGCH_INTRINSIC_GAIN);
#endif
    }
}                               // end demonspawn()

bool perma_mutate(int which_mut, char how_much)
{
    char levels = 0;

    if (mutate(which_mut + 1000))
        levels++;

    if (how_much >= 2 && mutate(which_mut + 1000))
        levels++;

    if (how_much >= 3 && mutate(which_mut + 1000))
        levels++;

    you.demon_pow[which_mut] = levels;

    return (levels > 0);
}                               // end perma_mutate()

bool give_good_mutation(bool failMsg)
{
    int temp_rand = 0;          // probability determination {dlb}
    int which_good_one = 0;

    temp_rand = random2(25);

    which_good_one = ((temp_rand >= 24) ? MUT_TOUGH_SKIN :
                      (temp_rand == 23) ? MUT_STRONG :
                      (temp_rand == 22) ? MUT_CLEVER :
                      (temp_rand == 21) ? MUT_AGILE :
                      (temp_rand == 20) ? MUT_HEAT_RESISTANCE :
                      (temp_rand == 19) ? MUT_COLD_RESISTANCE :
                      (temp_rand == 18) ? MUT_SHOCK_RESISTANCE :
                      (temp_rand == 17) ? MUT_REGENERATION :
                      (temp_rand == 16) ? MUT_TELEPORT_CONTROL :
                      (temp_rand == 15) ? MUT_MAGIC_RESISTANCE :
                      (temp_rand == 14) ? MUT_FAST :
                      (temp_rand == 13) ? MUT_ACUTE_VISION :
                      (temp_rand == 12) ? MUT_GREEN_SCALES :
                      (temp_rand == 11) ? MUT_BLACK_SCALES :
                      (temp_rand == 10) ? MUT_GREY_SCALES :
                      (temp_rand ==  9) ? MUT_BONEY_PLATES :
                      (temp_rand ==  8) ? MUT_REPULSION_FIELD :
                      (temp_rand ==  7) ? MUT_POISON_RESISTANCE :
                      (temp_rand ==  6) ? MUT_TELEPORT_AT_WILL :
                      (temp_rand ==  5) ? MUT_SPIT_POISON :
                      (temp_rand ==  4) ? MUT_MAPPING :
                      (temp_rand ==  3) ? MUT_BREATHE_FLAMES :
                      (temp_rand ==  2) ? MUT_BLINK :
                      (temp_rand ==  1) ? MUT_CLARITY
                                        : MUT_ROBUST);

    return (mutate(which_good_one, failMsg));
}                               // end give_good_mutation()

bool give_bad_mutation(bool forceMutation, bool failMsg)
{
    int temp_rand = 0;          // probability determination {dlb}
    int which_bad_one = 0;

    temp_rand = random2(12);

    which_bad_one = ((temp_rand >= 11) ? MUT_CARNIVOROUS :
                     (temp_rand == 10) ? MUT_HERBIVOROUS :
                     (temp_rand ==  9) ? MUT_FAST_METABOLISM :
                     (temp_rand ==  8) ? MUT_WEAK :
                     (temp_rand ==  7) ? MUT_DOPEY :
                     (temp_rand ==  6) ? MUT_CLUMSY :
                     (temp_rand ==  5) ? MUT_TELEPORT :
                     (temp_rand ==  4) ? MUT_DEFORMED :
                     (temp_rand ==  3) ? MUT_LOST :
                     (temp_rand ==  2) ? MUT_DETERIORATION :
                     (temp_rand ==  1) ? MUT_BLURRY_VISION
                                       : MUT_FRAIL);

    if (forceMutation)
        which_bad_one += 1000;

    return (mutate(which_bad_one), failMsg);
}                               // end give_bad_mutation()

//jmf: might be useful somewhere (eg Xom or transmigration effect)
bool give_cosmetic_mutation()
{
    int mutation = -1;
    int how_much = 0;
    int counter = 0;

    do
    {
        mutation = MUT_DEFORMED;
        how_much = 1 + random2(3);

        if (one_chance_in(6))
        {
            mutation = MUT_ROBUST;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(6))
        {
            mutation = MUT_FRAIL;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(5))
        {
            mutation = MUT_TOUGH_SKIN;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(4))
        {
            mutation = MUT_CLAWS;
            how_much = 1 + random2(3);
        }

        if (you.species != SP_CENTAUR && you.species != SP_NAGA
            && you.species != SP_KENKU && !player_genus(GENPC_DRACONIAN)
            && one_chance_in(5))
        {
            mutation = MUT_HOOVES;
            how_much = 1;
        }

        if (player_genus(GENPC_DRACONIAN) && one_chance_in(5))
        {
            mutation = MUT_BIG_WINGS;
            how_much = 1;
        }

        if (one_chance_in(5))
        {
            mutation = MUT_CARNIVOROUS;
            how_much = 1 + random2(3);
        }

        if (one_chance_in(6))
        {
            mutation = MUT_HORNS;
            how_much = 1 + random2(3);
        }

        if ((you.species == SP_NAGA || player_genus(GENPC_DRACONIAN))
            && one_chance_in(4))
        {
            mutation = MUT_STINGER;
            how_much = 1 + random2(3);
        }

        if (you.species == SP_NAGA && one_chance_in(6))
        {
            mutation = MUT_BREATHE_POISON;
            how_much = 1;
        }

        if (!(you.species == SP_NAGA || player_genus(GENPC_DRACONIAN))
            && one_chance_in(7))
        {
            mutation = MUT_SPIT_POISON;
            how_much = 1;
        }

        if (!(you.species == SP_NAGA || player_genus(GENPC_DRACONIAN))
            && one_chance_in(8))
        {
            mutation = MUT_BREATHE_FLAMES;
            how_much = 1 + random2(3);
        }

        if (you.mutation[mutation] > 0)
            how_much -= you.mutation[mutation];

        if (how_much < 0)
            how_much = 0;
    }
    while (how_much == 0 && counter++ < 5000);

    if (how_much != 0)
        return mutate(mutation);
    else
        return false;
}                               // end give_cosmetic_mutation()
