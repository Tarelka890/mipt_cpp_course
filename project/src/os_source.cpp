#include "os_source.h"

#include <print>

namespace nano_edr {

    OsSource::OsSource(const std::string& config_path) : handle_(config_path) {}

    void OsSource::Trampoline(const os_event* ev, void* ctx) noexcept{
        try{

            IEventHandler* handler = static_cast<IEventHandler*>(ctx);

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
            handler->HandleEvent(event);
        }
        catch(const std::exception& e){
            std::print(stderr, "ooops something broke inside ossource::trampoline {}\n", e.what());
        }
    }

    void OsSource::Run(IEventHandler* handler){
        handle_.Subscribe(&OsSource::Trampoline, handler);
        handle_.Start();

        os_status status = OS_TIMEOUT;
        while (status == OS_TIMEOUT) {
            handler->OnTick();
            status = handle_.Wait(100); 
        }

        if (status != OS_OK) {
            throw std::runtime_error(std::format("OsSource handle error {}", os_status_str(status)));
        }
    }


}