#include <event.h>
#include <map>
#include <string>
#include <event_list.h>
#include <print>
#include <fstream>
#include <os.h>
#include <os_handle.h>

namespace nano_edr{
    class Agent {
        public:
            Agent(std::size_t window_size, bool quiet);  // из --window-size и --quiet
            static void Trampoline(const os_event* ev, void* ctx) noexcept;
            void HandleEvent(const Event& event);
            void PrintSummary();  // сводка в конце прогона
            void PrintContext();

            const long long& lines();
            const long long& comments();

            void inc_comments();
            void inc_lines();
            void UpdateEncounters(const Event& current);
            bool quiet();
        private:
            const EventNode* context_pointer_ = nullptr;
            EventList window_;
            long long lines_;
            long long comments_;
            bool quiet_;
            std::map<std::string, long long> encounters_;
    };

    class FileSource {
        public:
            explicit FileSource(const std::string& path);
            void Run(Agent* agent);  // строка → EventParts → Event → HandleEvent
            
        private:
            std::ifstream log_;
    };

    class OsSource {
        public:
            explicit OsSource(const std::string& config_path);  // создаёт OsHandle
            void Run(Agent* agent);  // Subscribe, Start, Wait в цикле
        private:
            OsHandle handle_;
    };
}