#pragma once

#include <event.h>
#include <map>
#include <string>
#include <event_list.h>
#include <print>
#include <fstream>
#include <os_handle.h>
#include <rule.h>
#include <rule_engine.h>
#include <event_source.h>

namespace nano_edr{
    class Agent : public IEventHandler{
        public:
            Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
            // static void Trampoline(const os_event* ev, void* ctx) noexcept;

            void PrintSummary();  // сводка в конце прогона
            void PrintContext();
            bool quiet();

            // Оба источника: пришло событие. Тело — как на шаге 6.
            void HandleEvent(const Event& event) override;

            // FileSource: пустая строка или комментарий.
            void OnSkipped() override;

            // FileSource: из строки не вышло события.
            // Трамплин: событие от os.h не удалось построить.
            void OnBroken() override;

            // OsSource: Wait вернул OS_TIMEOUT, новых событий пока нет.
            void OnTick() override;
        private:
            const EventNode* context_pointer_ = nullptr;
            RuleEngine engine_;
            EventList window_;
            bool quiet_;
            size_t skipped_count_ = 0;
            size_t broken_count_ = 0;
    };
}