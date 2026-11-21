#pragma once

#include "event_source.h"

#include <fstream>

namespace nano_edr {
    class FileSource : public IEventSource{
        public:
            explicit FileSource(const std::string& path);
            void Run(IEventHandler* agent);  // строка → EventParts → Event → HandleEvent
            
        private:
            std::ifstream log_;
    };
}