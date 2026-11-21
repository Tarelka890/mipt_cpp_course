#pragma once

#include "os_handle.h"
#include "event_source.h"

namespace nano_edr {
    class OsSource : public IEventSource{
        public:
            explicit OsSource(const std::string& config_path);  // создаёт OsHandle
            void Run(IEventHandler* agent) override;  // Subscribe, Start, Wait в цикле
            static void Trampoline(const os_event* ev, void* ctx) noexcept;
            OsHandle* os() override { return &handle_; }

        private:
            OsHandle handle_;
    };

}