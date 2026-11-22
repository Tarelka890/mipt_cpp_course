#include "rule.h"

namespace nano_edr {
    RuleBase::RuleBase(const std::string& id, Severity severity) : id_(id), severity_(severity) {}

    void RuleBase::AddAction(const std::string& action){
        actions_.push_back(action);
    }

    MatchRule::MatchRule(const std::string& id, Severity severity) : RuleBase(id, severity) {}

    void MatchRule::AddCondition(std::unique_ptr<ICondition> condition){
        conditions_.push_back(std::move(condition));
    }

    bool MatchRule::Check(const Event& event){
        for (size_t i=0;i<condition_count();i++){
            if (!(*conditions_[i])(event)) return false;
        }
        CountHit();
        return true;
    }

    SequenceRule::SequenceRule(const std::string& id, Severity severity, uint64_t window_ms) : RuleBase(id, severity), window_ms_(window_ms) {}
    
    void SequenceRule::SetFirst(std::unique_ptr<ICondition> ptr){
        first_ = std::move(ptr);
    }

    void SequenceRule::SetSecond(std::unique_ptr<ICondition> ptr){
        second_ = std::move(ptr);
    }
    
    bool SequenceRule::Check(const Event& event){
        if (first_ == nullptr || second_ == nullptr) return false;

        while (!pending_.empty() && (event.ts().ms - pending_.front().ts_ms > window_ms_)) {
            pending_.erase(pending_.begin()); 
        }

        if (second_->Matches(event)){
            for (size_t i=0;i<pending();i++){
                if (!Linked(pending_[i], event)){
                    continue;
                }
                pending_.erase(pending_.begin()+i);
                CountHit();
                return true;
            }
        }  

        if (first_->Matches(event)){
            pending_.push_back(Pending(event.ts().ms, event.pid()));
        }
        return false;
    }

    bool SequenceRule::Linked(const Pending& first, const Event& second) const{
        if (child_of_prev_){
            if (const auto* proc = model_ ? model_->FindProcess(second.pid()) : nullptr; proc && proc->ppid == first.pid) {
                return true;
            }
            return false;
        }

        if (same_pid_){
            return first.pid == second.pid();
        }
        return true;
    }

    LambdaRule::LambdaRule(const std::string& id, Severity severity, Predicate predicate) : RuleBase(id, severity), predicate_(std::move(predicate)){}
    
    bool LambdaRule::Check(const Event& event) {
        if (!predicate_){
            return false;
        }
        auto result = predicate_(event);
        if (result){
            CountHit();
            return true;
        }
        return false;
    }

}