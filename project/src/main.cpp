#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include <map>

#include <parse.h>
#include <event_list.h>

const size_t context_size = 2;

int main(int argc, char** argv) {

    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return 2;
    }

    std::ifstream log(argv[1]);
    if (!log) {
        std::print(stderr, "не удалось открыть журнал: {}\n", argv[1]);
        return 2;
    }

    bool quiet = false;
    long long window_size = 64;

    for (int16_t i=0;i<argc;i++){
        if (std::string_view(argv[i]) == "--quiet") quiet = true;
        else if (std::string_view(argv[i]) == "--window-size"){
            if (i+1 < argc){
                try{
                    window_size = std::stoll(argv[i+1]);
                }
                catch(const std::exception& e){
                    window_size = 64;
                }
            }
        }
    }

    long long lines = 0;
    long long comments = 0;
    bool found = false;
    std::string line;
    std::map<std::string, int> encounters;
    std::vector<std::string> targets = {"wscript.exe", ".locked", "certutil.exe", "\\Startup\\"};
    nano_edr::EventList events;
    nano_edr::EventNode* context_pointer = nullptr;
    size_t last_size = 0;
    events.capacity = window_size;

    while (std::getline(log, line)) {
        ++lines;
        
        if (nano_edr::IsBlankOrComment(&line)) {
            ++comments;
            continue;
        }

        last_size = events.size;

        nano_edr::Event current;
        nano_edr::ParseEventLine(&line, &current);

        encounters[current.type]++;

        found = false;
        for (const auto& target : targets){
            for (const auto& field : current.fields){
                if (field.value.find(target) != std::string::npos){
                    std::print("[DETECT] строка {}, признак {}: {}\n", lines, target, line);
                    found = true;
                    break;
                }
            }
        }

        if (!quiet && found){
            nano_edr::EventNode* curr = context_pointer;
            size_t i = std::min(context_size, size_t{events.size});
            while (curr != nullptr){
                std::print("[CTX] {}: ts={} type={} pid={}\n", -static_cast<long long>(i), curr->event.ts, curr->event.type, curr->event.pid);
                curr = curr->next;
                i--;
            }
        }


        nano_edr::ListPushBack(&events, &current);
        if (!last_size){
            context_pointer = events.head;
        }
        else if (events.size <= context_size){
            context_pointer = events.head;
        }
        else{
            if (context_pointer != nullptr){
                context_pointer = context_pointer->next;
            }
        }
    }

    if (!quiet){
        std::print("строк {}, из них комментариев {}\n", lines, comments);
        for (auto it = encounters.begin(); it != encounters.end(); it++){
            std::print("{}: {} \n", it->first, it->second);
        }
    }

    return 0;
}
