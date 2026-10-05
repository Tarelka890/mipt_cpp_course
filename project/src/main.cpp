#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include <map>

#include <parse.h>
#include <event_list.h>
#include <rules.h>
#include <agent_rules.h>
#include <event.h>
#include <fields.h>

const size_t context_size = 2;

struct Config {
    std::string log_path;
    bool quiet = false;
    long long window_size = 64;
};

// nano_edr::Event SanitizeEvent(const nano_edr::Event& event){
//     nano_edr::Event copy = event;
//     for (auto& field : copy.fields){
//         if (field.key == "path" || field.key == "to" || field.key == "image" || field.key == "from") {
//             field.value = nano_edr::NormalizePath(field.value);
//         }
//     }
//     return copy;
// }

bool ParseArguments(int argc, char** argv, Config& config){
    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }

    for (int i=1;i<argc;i++){
        if (std::string_view(argv[i]) == "--quiet"){
            config.quiet = true;
        }
        else if (std::string_view(argv[i]) == "--window-size"){
            if (i+1 < argc){
                try{
                    config.window_size = std::stoll(argv[i+1]);
                }
                catch(const std::exception& e){
                    config.window_size = 64;
                }
                ++i;
            }
        }
        else if (std::string_view(argv[i])[0] != '-'){
            config.log_path = argv[i];
        }
    }

    if (config.log_path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }

    return true;
}

void PrintSummary(long long& lines, long long& comments, const std::map<std::string, int>& encounters){
    std::print("строк {}, из них комментариев {}\n", lines, comments);
    for (auto it = encounters.begin(); it != encounters.end(); it++){
        std::print("{}: {} \n", it->first, it->second);
    }
}

void PrintContext(nano_edr::EventList& events, nano_edr::EventNode* context_pointer){
    nano_edr::EventNode* curr = context_pointer;
    size_t i = std::min(context_size, size_t{events.size});
    while (curr != nullptr){
        std::print("[CTX] {}: ts={} type={} pid={}\n", -static_cast<long long>(i), curr->event.ts, curr->event.type, curr->event.pid);
        curr = curr->next;
        i--;
    }
}

void ProcessLogStream(std::ifstream& log, Config& config, long long& lines, long long& comments, std::map<std::string, int>& encounters){
    std::string line;
    nano_edr::EventList events;
    nano_edr::EventNode* context_pointer = nullptr;

    events.capacity = config.window_size;

    while (std::getline(log, line)) {
        ++lines;
        
        if (nano_edr::IsBlankOrComment(&line)) {
            ++comments;
            continue;
        }

        nano_edr::Event current;
        if (!nano_edr::ParseEventLine(&line, &current)){
            continue;
        }

        // current = SanitizeEvent(current);

        encounters[current.type]++;

        if (nano_edr::CheckRules(current, nano_edr::AgentRules(), nano_edr::AgentRuleCount())){
            if (!config.quiet){
                PrintContext(events, context_pointer);
            }
        }

        nano_edr::ListPushBack(&events, &current);
        
        if (events.size <= context_size){
            context_pointer = events.head;
        }
        else{
            if (context_pointer != nullptr){
                context_pointer = context_pointer->next;
            }
        }
    }
}


int main(int argc, char** argv) {
    try{
        Config config;

        if (!ParseArguments(argc, argv, config)){
            return 2;
        }

        std::ifstream log(config.log_path);

        if (!log){
            std::print(stderr, "Невозможно открыть журнал {}\n", config.log_path);
            return 2;
        }

        long long lines = 0;
        long long comments = 0;
        std::map<std::string, int> encounters;

        ProcessLogStream(log, config, lines, comments, encounters);

        if (!config.quiet) {
            PrintSummary(lines, comments, encounters);
        }
        
        return 0;
    }
    catch (const std::exception& error){
        std::print(stderr, "Прогон прерван из-за ошибки формата: {}\n", error.what());
        return 1;
    }

}
