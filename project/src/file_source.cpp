#include "file_source.h"
#include "parse.h"

#include <print>

namespace nano_edr {
    FileSource::FileSource(const std::string& path) : log_(path) {
        if (!log_.is_open()) {
            throw std::runtime_error(path);
        }
    }

    void FileSource::Run(IEventHandler* handler){
        std::string line;

        while (std::getline(log_, line)) {
            try{
                if (IsBlankOrComment(line)) {
                    handler->OnSkipped();
                    continue;
                }

                EventParts parts;
                if (!ParseEventParts(line, &parts)){
                    handler->OnBroken();
                    continue;
                }
            
                Event current(parts);
                handler->HandleEvent(current);
            }
            catch (const std::exception& e){
                std::print(stderr, "got a bad line inside filesource::run {}\n", e.what());
            }

        }

    }
}