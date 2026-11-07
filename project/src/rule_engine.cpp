#include "rule_engine.h"

namespace nano_edr {
    void RuleEngine::AddRule(std::unique_ptr<IRule> rule){
        rules_.push_back(std::move(rule));
    }

    std::size_t RuleEngine::ProcessEvent(const Event& event, std::vector<const IRule*>* fired){
        size_t counter = 0;
        for (size_t i=0;i<size();i++){
            if (rules_[i]->Check(event)){
                counter++;
                if (fired != nullptr){
                    fired->push_back(rules_[i].get());
                }
            }
        }
        return counter;
    }
}