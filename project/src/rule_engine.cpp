#include "rule_engine.h"
#include "copy_stats.h"

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
        Detection detection;
        detection.rule = rule.id();
        detection.severity = rule.severity();
        detection.ts = event.ts();
        detection.pid = event.pid();
        detection.chain = (model_ == nullptr) ? "" : model_->DescribeChain(event.pid());
        detection.actions = rule.actions();
        detection.evidence.push_back(std::make_unique<Event>(event));
        ++copy_stats().evidence_copied;

        return detection;
    }
}