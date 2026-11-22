#pragma once

#include "os_handle.h"
#include "rule.h"
#include "rule_engine.h"
#include "event_source.h"
#include "event.h"
#include "event_list.h"
#include "ring_buffer.h"

#include <map>
#include <string>
#include <print>
#include <fstream>


namespace nano_edr{
    struct WindowEntry {
        Timestamp ts;
        std::string type;
        std::string pid;

        WindowEntry(Timestamp ts, std::string type, std::string pid): ts(ts), type(type), pid(pid) {};
        WindowEntry() = default;
    };

    class Agent : public IEventHandler{
        public:
            static constexpr std::size_t kWindowCapacity = 256;

            explicit Agent(bool quiet);

            void PrintSummary();  // сводка в конце прогона

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
            void PrintContext();
            
            RingBuffer<WindowEntry, kWindowCapacity> window_;
            RuleEngine engine_;
            bool quiet_;
            size_t skipped_count_ = 0;
            size_t broken_count_ = 0;
    };
}