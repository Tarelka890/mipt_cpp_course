#include <headers/agent.h>
#include <headers/agent_rules.h>

#include <parse.h>
#include <rules.h>
#include <iostream>

namespace nano_edr{
    Agent::Agent(std::size_t window_size, bool quiet) : window_(window_size), quiet_(quiet), rules_(BuildAgentRules()) {}

    void Agent::Trampoline(const os_event* ev, void* ctx) noexcept{
        try{

            Agent* agent = static_cast<Agent*>(ctx);

            if (ctx == nullptr || ev == nullptr) return;

            EventParts parts;
            parts.ts = std::to_string(ev->ts);
            parts.type = ev->type;

            if (ev->pid != 0){
                parts.pid = std::to_string(ev->pid);
            }
            
            for (size_t i=0;i<ev->field_count;i++){
                parts.fields.push_back(Field(ev->fields[i].key, ev->fields[i].value));
            }

            Event event(parts);
            agent->HandleEvent(event);
        }
        catch(const std::exception& e){
            std::print(stderr, "ooops something broke inside agent::trampoline {}", e.what());
        }
    }

    void Agent::HandleEvent(const Event& event){
        bool found = false;

        for (size_t i = 0; i < rules_.size(); ++i) {
            if (rules_[i].Check(event)) {
                found = true;
                std::print("[DETECT] {}  {}  ts={} pid={}\n", SeverityName(rules_[i].severity()), rules_[i].id(), event.raw_ts(), event.pid());
            }
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
        std::print("правила, детектов:");
        for (size_t i = 0; i < rules_.size(); i++){
            std::print("{}: {} \n", rules_[i].id(), rules_[i].hits());
        }
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