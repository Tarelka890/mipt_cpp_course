#include "agent.h"
#include "agent_rules.h"
#include "parse.h"
#include "rules.h"

#include <iostream>

namespace nano_edr{

    static constexpr size_t WINDOW_SIZE = 2;

    Agent::Agent(bool quiet) : engine_(BuildAgentRules()), quiet_(quiet) {};

    void Agent::OnSkipped(){
        skipped_count_++;
    }

    void Agent::OnBroken(){
        broken_count_++;
    }

    void Agent::OnTick(){
        
    }
    
    void Agent::HandleEvent(const Event& event){
        std::vector<const IRule*> fired;
        engine_.ProcessEvent(event, &fired);

        if (!fired.empty() && !quiet_){
            PrintContext();
        }

        for (size_t i = 0; i < fired.size(); ++i) {
            std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(fired[i]->severity()), fired[i]->id(), event.raw_ts(), event.pid());
        }

        WindowEntry entry(event.ts(), event.type(), event.pid());

        window_.PushBack(entry);
        
    }

    void Agent::PrintSummary(){
        if (!quiet_){
            std::print("правила, детектов:\n");
            for (size_t i = 0; i < engine_.size(); i++){
                std::print("{}: {} \n", engine_.rule(i).id(), engine_.rule(i).hits());
            }
            std::print("пропущено строк: {}, сломано строк: {}\n", skipped_count_, broken_count_);
            std::print("[STAT] в окне {} из {} мест\n", window_.size(), window_.capacity());
        }
    }

    void Agent::PrintContext(){
        if (window_.size() == 0) return;

        size_t start_index = window_.size() - std::min(WINDOW_SIZE, window_.size());

        for (size_t i=start_index;i<window_.size();i++){
            std::print("[CTX] {}: {} {} pid={}\n", -static_cast<long long>(window_.size()-i), window_[i].ts.ms, window_[i].type, window_[i].pid);
        }

    }
    
}