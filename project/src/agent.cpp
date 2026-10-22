#include <agent.h>
#include <agent_rules.h>

#include <parse.h>
#include <rules.h>
#include <iostream>

namespace nano_edr{
    Agent::Agent(std::size_t window_size, bool quiet) : engine_(BuildAgentRules()), window_(window_size), quiet_(quiet) {}

    void Agent::OnSkipped(){
        skipped_count_++;
    }

    void Agent::OnBroken(){
        broken_count_++;
    }

    void Agent::OnTick(){
        // ??????? метод пустой. а все работает
    }
    
    void Agent::HandleEvent(const Event& event){
        bool found = false;
        std::vector<const IRule*> fired;
        engine_.ProcessEvent(event, &fired);
        for (size_t i = 0; i < fired.size(); ++i) {
            found = true;
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(fired[i]->severity()), fired[i]->id(), event.raw_ts(), event.pid());
        }

        if (found && !quiet()){
            PrintContext();
        }

        window_.PushBack(event);
        
        if (window_.size() <= 2){
            context_pointer_ = window_.head();
        }
        else{
            if (context_pointer_ != nullptr){
                context_pointer_ = context_pointer_->next;
            }
        }
    }

    void Agent::PrintSummary(){
        std::print("правила, детектов:\n");
        for (size_t i = 0; i < engine_.size(); i++){
            std::print("{}: {} \n", engine_.rule(i).id(), engine_.rule(i).hits());
        }
        std::print("пропущено строк: {}, сломано строк: {}", skipped_count_, broken_count_);
    }

    void Agent::PrintContext(){
        const EventNode* curr = context_pointer_;
        size_t i = std::min(size_t{2}, size_t{window_.size()});
        while (curr != nullptr && i > 0){
            std::print("[CTX] {}: ts={} type={} pid={}\n", -static_cast<long long>(i), curr->event.raw_ts(), curr->event.type(), curr->event.pid());
            curr = curr->next;
            i--;
        }
    }

    bool Agent::quiet(){
        return quiet_;
    }
    
}