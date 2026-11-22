#include "agent.h"
#include "agent_rules.h"
#include "parse.h"
#include "dedup_sink.h"

#include <iostream>

namespace nano_edr{

    static constexpr size_t WINDOW_SIZE = 2;

    Agent::Agent(bool quiet, uint64_t dedup_period_ms) : engine_(BuildAgentRules(&model_)), quiet_(quiet), dedup_period_ms_(dedup_period_ms) {
        engine_.SetModel(&model_);
    };

    void Agent::OnSkipped(){
        skipped_count_++;
    }

    void Agent::OnBroken(){
        broken_count_++;
    }

    void Agent::OnTick(){
        
    }
    
    void Agent::HandleEvent(const Event& event){
        context_printed_ = false;
        std::vector<const IRule*> fired;
        model_.Observe(event);

        WindowEntry entry;
        entry.ts = event.ts();
        entry.type = event.type();
        entry.pid = event.pid();
        
        if (responder_ != nullptr && responder_->Recognize(event)) {
            window_.PushBack(std::move(entry));
            return;  
        }
        engine_.ProcessEvent(event, &fired);
        window_.PushBack(std::move(entry));
    }

    void Agent::PrintSummary(){
        if (!quiet_){
            std::print("правила, детектов:\n");
            for (size_t i = 0; i < engine_.size(); i++){
                std::print("{}: {} \n", engine_.rule(i).id(), engine_.rule(i).hits());
            }
            std::print("пропущено строк: {}, сломано строк: {}\n", skipped_count_, broken_count_);
            std::print("[STAT] в окне {} из {} мест\n", window_.size(), window_.capacity());
            std::print("[STAT] в модели процессов {}, файлов {}\n", model_.process_count(), model_.file_count());
            std::print("[STAT] подавлено дубликатов {} (срок {} мс)\n", suppressed_ , dedup_period_ms_);
            if (responder_){
                std::print("[STAT] действий запланировано {}, повторов отброшено {}\n", responder_->stats().planned, responder_->stats().skipped_repeat);
                std::print("[STAT] завершено процессов {}, изолировано файлов {}, удалено файлов {}\n", responder_->stats().killed, responder_->stats().quarantined, responder_->stats().deleted);
                std::print("[STAT] своих событий {}\n", responder_->stats().own_events);
                
            }
        }
    }

    void Agent::PrintContext(){
        if (window_.size() == 0) return;

        size_t start_index = window_.size() - std::min(WINDOW_SIZE, window_.size());

        for (size_t i=start_index;i<window_.size();i++){
            std::print("[CTX] {}: {} {} pid={}\n", -static_cast<long long>(window_.size()-i), window_[i].ts.ms, window_[i].type, window_[i].pid);
        }

    }

    void Agent::Subscribe(AlertSink sink) {
        FanOutSink all;
        all.Add([this](const Detection&) { PrintContextOnce(); });
        all.Add(std::move(sink));
        engine_.Subscribe(DedupSink(std::move(all), dedup_period_ms_, &suppressed_));
    }

    void Agent::PrintContextOnce() {
        if (!quiet_ && !context_printed_){
            PrintContext();
            context_printed_ = true;
        }
    }
    
    void Agent::EnableResponse(OsHandle* os, bool dry_run, const std::string& quarantine_dir) {
        planner_ = std::make_unique<ResponsePlanner>(&model_, quarantine_dir);
        responder_ = std::make_unique<Responder>(os, dry_run);
        engine_.Subscribe([this](const Detection& detection) {
            responder_->Execute(planner_->Plan(detection));
        });
    }
    
}
