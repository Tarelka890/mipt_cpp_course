#ifndef NANO_EDR_AGENT_RULES_H
#define NANO_EDR_AGENT_RULES_H

#include "rules.h"

namespace nano_edr {

// Таблица правил агента и её длина — ровно то, что принимает CheckRules.
const Rule* AgentRules();

size_t AgentRuleCount();
// size_t FileChecksCount();

bool ScriptHostFromTemp(const Event& event);
bool LolbinDownload(const Event& event);
bool HiddenPowershell(const Event& event);
bool AutostartWrite(const Event& event);
bool RansomExtension(const Event& event);

bool IsFileCreate(const Event& event);
bool IsFileMove(const Event& event);

bool FieldEndsWith(const Event& event, const std::string& key, const std::string& suffix);
bool FieldContains(const Event& event, const std::string& key, const std::string& needle);
// bool FieldStartsWith(const Event& event, const std::string& key, const std::string& prefix);


}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_RULES_H
