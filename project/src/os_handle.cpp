#include <os.h>
#include <os_handle.h>
#include <stdexcept>
#include <format>

namespace nano_edr {

    OsHandle::OsHandle(const std::string& config_path){
        os_status status = os_init(config_path.c_str(), &handle_);

        if (status != OS_OK){
            std::string error = std::format("init failed. path:{} status:{}", config_path, os_status_str(status));
            throw std::runtime_error(error);
        }
    }

    // Зовёт os_free: тот сам останавливает поток и пишет отчёт. Не бросает.
    OsHandle::~OsHandle(){
        os_free(handle_);
    }

    void OsHandle::Subscribe(os_event_cb callback, void* context){
        os_status status = os_event_subscribe(handle_, callback, context);

        if (status != OS_OK){
            std::string error = std::format("cannot subscribe to callback");
            throw std::runtime_error(error);
        }
    }

    void OsHandle::Start(){
        os_status status = os_start(handle_);
        if (status != OS_OK){
            std::string error = std::format("cannot start");
            throw std::runtime_error(error);
        }
    }


    os_status OsHandle::Wait(uint32_t timeout_ms){
        return os_wait(handle_, timeout_ms);
    }

    void OsHandle::Stop(){
        os_stop(handle_);
    }

};
