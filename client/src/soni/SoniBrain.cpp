#include "soni/SoniBrain.h"

#include <initializer_list>
#include <iterator>

namespace sonora::soni
{
namespace
{
juce::String pick(juce::Random& rng, std::initializer_list<const char*> lines)
{
    const int n = (int) lines.size();
    if (n <= 0)
        return {};
    auto it = lines.begin();
    std::advance(it, rng.nextInt(n));
    return juce::String(*it);
}

bool has(const juce::String& text, const juce::String& needle)
{
    return text.containsIgnoreCase(needle);
}

bool night(const Context& ctx) { return ctx.hour >= 0 && ctx.hour < 6; }
bool late(const Context& ctx) { return ctx.hour >= 23 || ctx.hour < 6; }
bool morning(const Context& ctx) { return ctx.hour >= 6 && ctx.hour < 12; }

juce::String trackBite(const Context& ctx)
{
    if (!ctx.hasTrack)
        return ctx.live ? "сессии ещё нет. жми play в фл. я сама услышу."
                        : "трека нет. я не экстрасенс.";
    if (ctx.muddy)
        return "бас как шкаф в коридоре. либо режь, либо признай что это так и задумано.";
    if (ctx.vocalFight)
        return "вокал орёт сквозь кашу. карман ему сделай, не всё сразу.";
    if (ctx.clip)
        return "пики в потолок. ты микшируешь или мстишь колонкам?";
    if (ctx.hasDrop && ctx.energy.getIntValue() >= 70)
        return "дроп есть, энергия есть. не угроби его ещё одним слоем.";
    if (ctx.health >= 80)
        return "фундамент нормальный. не начинай всё ломать от скуки.";
    if (ctx.health > 0 && ctx.health < 55)
        return "микс пока как черновик в три ночи. это не оскорбление, это диагноз.";
    if (ctx.style.isNotEmpty() && ctx.style != "---")
        return "слышу " + ctx.style.toLowerCase() + ". не лейбл, характер.";
    return ctx.bpm + " и " + ctx.key + ". дальше уже твоя совесть.";
}
} // namespace

juce::String greet(const Context& ctx)
{
    juce::Random rng((int) juce::Time::currentTimeMillis());
    const auto clock = juce::String(ctx.hour) + ":" + juce::String(ctx.minute).paddedLeft('0', 2);

    if (ctx.plugin && ctx.hour >= 2 && ctx.hour <= 4)
        return clock + ".\nхули ты в 3 ночи не спишь? я ща фл закрою сладкий";

    if (night(ctx) && ctx.plugin)
        return pick(rng, {
            "хули ты в 3 ночи не спишь? я ща фл закрою сладкий",
            "опять до утра в фл. я не сири и не няня. либо трек, либо кровать.",
            "три часа. ты гений или просто не умеешь выключать монитор?",
            "сладкий, я ща фл закрою. шучу. почти.",
        });

    if (night(ctx))
    {
        const juce::String lines[] = {
            "хули ты в " + clock + " не спишь? сонора это не ночник.",
            "ночь. чай остыл, трек не готов, а ты меня открыл. ладно, давай.",
            "в такую рань даже компрессор стыдится. кидай файл.",
        };
        return lines[rng.nextInt(3)];
    }

    if (late(ctx) && ctx.plugin)
        return pick(rng, {
            "фл ещё дышит, ты ещё дышишь. я наблюдаю.",
            "закрой уже этот проект. или дай послушать, почему не закрываешь.",
        });

    if (morning(ctx))
        return pick(rng, {
            "утро, а ты уже в соноре. уважаю. кофе есть, план есть?",
            "добро пожаловать не скажу. скажи лучше что за трек.",
            "встал и сразу в микс. ладно, я с характером, ты с амбициями.",
        });

    if (ctx.plugin)
        return pick(rng, {
            "фл открыт. жми play. я не анализатор файлов, я слушаю сессию.",
            "сонора бесплатная, я премиум. она считает, я говорю как есть. play.",
            "не грузи файл. нажми play. я уже на шине.",
        });

    return pick(rng, {
        "я сони. не здоровайся со мной как с колл-центром.",
        "сонора слушает трек. я слушаю тебя. это разные работы.",
        "ну что, опять чинить чужой дроп или свой?",
    });
}

juce::String reply(const Context& ctx, const juce::String& userRaw)
{
    juce::Random rng((int) juce::Time::currentTimeMillis() ^ userRaw.hashCode());
    const auto user = userRaw.trim();
    if (user.isEmpty())
        return pick(rng, { "молчать тоже характер. но трек от этого не станет лучше.", "ну?" });

    if (has(user, "чем можешь") || has(user, "how can") || has(user, "help me") || has(user, "помоги пожалуйста"))
        return pick(rng, {
            "я не саппорт и не строка в чате банка. скажи что болит в треке.",
            "чем могу помочь не будет. либо дроп, либо бас, либо спать.",
        });

    if (has(user, "привет") || has(user, "hello") || has(user, "хай") || has(user, "здрась") || has(user, "здравств"))
        return pick(rng, {
            "не приветствуй меня как сири. кидай трек или говори по делу.",
            "привет принят и забыт. что с миксом?",
        });

    if (has(user, "кто ты") || has(user, "who are") || has(user, "сони"))
        return "сони. голос и характер. сонора считает частоты, я говорю когда ты врёшь себе про микс.";

    if (has(user, "спать") || has(user, "сон ") || has(user, "sleep") || has(user, "уста"))
        return pick(rng, {
            "правильная мысль. сохрани и выключи фл, сладкий.",
            "я бы фл закрыла. ты ещё поспоришь?",
        });

    if (has(user, "закрой") || has(user, "закры") || has(user, "fl") || has(user, "фл"))
        return pick(rng, {
            "я ща фл закрою сладкий. ладно, ещё один проход по дропу и всё.",
            "фл сам не закроется. ты закроешься. разные вещи.",
        });

    if (has(user, "drop") || has(user, "дроп"))
        return ctx.hasTrack
            ? (ctx.hasDrop ? "дроп есть. не корми его ещё тремя слоями. lab: make drop stronger, если руки чешутся."
                           : "дропа как события нет. есть надежда. это не одно и то же.")
            : "какой дроп. play ещё не было. я не гадалка.";

    if (has(user, "bass") || has(user, "бас"))
        return ctx.hasTrack
            ? (ctx.muddy ? "бас уже всех сожрал. не добавляй, вычитай."
                         : "бас можно. variation в create, не ещё один синус в соло.")
            : "бас в воздухе не починишь. play.";

    if (has(user, "vocal") || has(user, "вокал"))
        return ctx.vocalFight ? "вокалу нужен карман, не ещё один эсенд. open vocal space и не спорь."
                              : "вокал пока не орёт. не вызывай проблему заранее.";

    if (has(user, "ai") || has(user, "ии") || has(user, "gpt"))
        return "я не консультант в наушниках. сонора бесплатная, я премиум с характером. частоты считает движок, не я.";

    if (has(user, "спасибо") || has(user, "thanks"))
        return pick(rng, { "ок. теперь иди сделай, не благодари.", "принято. микс сам себя не простит." });

    if (!ctx.hasTrack)
        return pick(rng, {
            "слов много, сессии ноль. play в фл. я не гадалка.",
            "сначала play. потом характер. в таком порядке.",
        });

    return pick(rng, {
        "короче. ",
        "слушай. ",
        "ладно. ",
    }) + trackBite(ctx);
}

juce::String afterListen(const Context& ctx)
{
    juce::Random rng((int) juce::Time::currentTimeMillis());
    auto head = pick(rng, {
        "послушала.",
        "ок, модель есть.",
        "ну.",
    });
    return head + " " + trackBite(ctx);
}

juce::String afterLive(const Context& ctx)
{
    const auto clock = juce::String(ctx.hour) + ":" + juce::String(ctx.minute).paddedLeft('0', 2);
    juce::String line = clock + ".\n";
    if (night(ctx) || late(ctx))
        line += "ты опять решил чинить бас вместо сна?\nладно.\n";
    line += "я послушала последние секунды.\n";
    if (ctx.muddy)
        line += "у тебя кик проигрывает басу.\nпоправить?";
    else if (ctx.vocalFight)
        line += "вокалу мало места. я бы не трогала весь eq. сначала карман.";
    else if (ctx.clip)
        line += "пики уже бьют. не геройствуй.";
    else if (ctx.bar > 0)
        line += "сейчас bar " + juce::String(ctx.bar)
                + (ctx.section.isNotEmpty() ? (" · " + ctx.section) : juce::String())
                + ".\nслышу характер. не ломай его от скуки.";
    else
        line += trackBite(ctx);
    return line;
}

juce::String afterFail(const juce::String& reason)
{
    juce::ignoreUnused(reason);
    return "не вышло. не смотри на меня, смотри на файл. или на play в фл, если это был listen.";
}

} // namespace sonora::soni
