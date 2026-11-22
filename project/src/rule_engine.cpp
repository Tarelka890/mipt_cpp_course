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

                Detection get = MakeDetection(*rules_[i], event);

                for (const auto& sink : sinks_){
                    sink(get);
                }
            }
        }
        return counter;
    }

    void RuleEngine::Subscribe(AlertSink sink) {
        sinks_.push_back(std::move(sink));
    }

    Detection RuleEngine::MakeDetection(const IRule& rule, const Event& event) const{
        return Detection(
            rule.id(),
            rule.severity(),
            event.ts(),
            event.pid(),
            (model_ == nullptr) ? "" : model_->DescribeChain(event.pid()),
            rule.actions()
        );
    }
}