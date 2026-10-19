#include <headers/os_source.h>

namespace nano_edr {

    OsSource::OsSource(const std::string& config_path) : handle_(config_path) {}

    void OsSource::Run(Agent* agent){
        handle_.Subscribe(&Agent::Trampoline, agent);
        handle_.Start();

        os_status status = OS_TIMEOUT;
        while (status == OS_TIMEOUT) {
            status = handle_.Wait(100); 
        }

        if (status != OS_OK) {
            throw std::runtime_error(std::format("OsSource handle error, error {}", os_status_str(status)));
        }
    }


}