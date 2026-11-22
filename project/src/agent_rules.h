#ifndef NANO_EDR_AGENT_RULES_H
#define NANO_EDR_AGENT_RULES_H

#pragma once

#include "rule_engine.h"


namespace nano_edr {
    RuleEngine BuildAgentRules(const EntityModel*);

}  // namespace nano_edr

#endif  // NANO_EDR_AGENT_RULES_H
