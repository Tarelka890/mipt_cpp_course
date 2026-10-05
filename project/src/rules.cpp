#include "agent_rules.h"
#include <rules.h>

#include <ostream>
#include <print>

namespace nano_edr {

// Важность детекта. enum class, а не обычный enum: с обычным `kLow` утёк бы
// в окружающую область имён, а сравнение с целым компилировалось бы молча.
// Строковое имя важности для печати. Возвращает "?" на значении вне
// перечисления — такое приходит только из битого приведения, но падать из-за
// него незачем.
const char* SeverityName(Severity severity){
    switch (severity){
        case Severity::kLow:
            return "low";
        case Severity::kMedium:
            return "medium";
        case Severity::kHigh:
            return "high";
        case Severity::kCritical:
            return "critical";
    }
    return "?";
}

// Проверка правила: одно событие на входе, «сработало или нет» на выходе.
//
// Указатель на функцию, а не std::function: нужен тип, который можно положить
// в constexpr-таблицу, и никакого захвата состояния здесь всё равно нет.
using RuleFn = bool (*)(const Event& event);

// Прогоняет событие по таблице и печатает сработавшие правила. В строке детекта
// после [DETECT] один пробел, дальше три поля через два пробела: важность,
// идентификатор и пара ts/pid, внутри которой пробел один:
//
//     [DETECT] high  script_host_from_temp  ts=1730000003000 pid=1101
//
// Возвращает число детектов. Таблица передаётся указателем и длиной, а не
// контейнером: контейнеры начинаются с лекции 13, а массив с длиной — то, что
// C++ умел с самого начала, и то, во что превращается любой контейнер на
// границе с C.
size_t CheckRules(const Event& event, const Rule* rules, size_t rule_count){
    size_t detected = 0;
    for (size_t i = 0; i < rule_count; i++){
        const Rule& rule = rules[i];
        if (rule.check(event)){
            detected++;
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rule.severity), rule.id, event.ts, event.pid);
        }
    }
    return detected;
}

}  // namespace nano_edr
