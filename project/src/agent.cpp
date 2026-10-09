#include <agent.h>
#include <parse.h>
#include <rules.h>
#include <agent_rules.h>
#include <iostream>
#include <os_handle.h>

namespace nano_edr{
    Agent::Agent(std::size_t window_size, bool quiet) : window_(window_size), quiet_(quiet) {}

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
        catch(...){

        }
    }

    void Agent::HandleEvent(const Event& event){
        if (CheckRules(event, AgentRules(), AgentRuleCount())){
            if (!quiet()){
                PrintContext();
            }
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
        std::print("строк {}, из них комментариев {}\n", lines(), comments());
        for (auto it = encounters_.begin(); it != encounters_.end(); it++){
            std::print("{}: {} \n", it->first, it->second);
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

    const long long& Agent::lines(){
        return lines_;
    }

    const long long& Agent::comments(){
        return comments_;
    }

    void Agent::inc_comments(){
        comments_++;
    }

    void Agent::inc_lines(){
        lines_++;
    }

    void Agent::UpdateEncounters(const Event& current){
        encounters_[current.type()]++;
    }

    bool Agent::quiet(){
        return quiet_;
    }
    

    FileSource::FileSource(const std::string& path) : log_(path) {
        if (!log_.is_open()) {
            throw std::runtime_error(path);
        }
    }

    void FileSource::Run(Agent* agent){
        std::string line;

        while (std::getline(log_, line)) {
            agent->inc_lines();
            
            if (IsBlankOrComment(line)) {
                agent->inc_comments();
                continue;
            }

            EventParts parts;
            if (!ParseEventParts(line, &parts)){
                continue;
            }

            try{
                Event current(parts);
                agent->HandleEvent(current);
                agent->UpdateEncounters(current);
            }
            catch (...){
                continue;
            }

        }

        // end
        if (!agent->quiet()) {
            agent->PrintSummary();
        }
    }

    OsSource::OsSource(const std::string& config_path) : handle_(config_path) {}

    void OsSource::Run(Agent* agent){
        handle_.Subscribe(&Agent::Trampoline, agent);
        handle_.Start();

        os_status status = OS_TIMEOUT;
        while (status == OS_TIMEOUT) {
            status = handle_.Wait(100); 
        }

        if (status != OS_OK) {
            throw std::runtime_error("OsSource handle error");
        }
    }
}