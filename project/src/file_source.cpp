#include <headers/file_source.h>
#include <parse.h>

namespace nano_edr {
    FileSource::FileSource(const std::string& path) : log_(path) {
        if (!log_.is_open()) {
            throw std::runtime_error(path);
        }
    }

    void FileSource::Run(Agent* agent){
        std::string line;

        while (std::getline(log_, line)) {
            try{
                if (IsBlankOrComment(line)) {
                    continue;
                }

                EventParts parts;
                if (!ParseEventParts(line, &parts)){
                    continue;
                }
            
                Event current(parts);
                agent->HandleEvent(current);
            }
            catch (const std::exception& e){
                std::print(stderr, "got a bad line inside filesource::run {}", e.what());
            }

        }

    }
}