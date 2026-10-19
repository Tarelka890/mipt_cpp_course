#pragma once

#include <event.h>
#include <map>
#include <string>
#include <event_list.h>
#include <print>
#include <fstream>
#include <os.h>
#include <os_handle.h>
#include <rule.h>

namespace nano_edr{
    class Agent {
        public:
            Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
            static void Trampoline(const os_event* ev, void* ctx) noexcept;
            void HandleEvent(const Event& event);
            void PrintSummary();  // сводка в конце прогона
            void PrintContext();
            bool quiet();
        private:
            const EventNode* context_pointer_ = nullptr;
            EventList window_;
            bool quiet_;
            std::vector<MatchRule> rules_;
    };
}