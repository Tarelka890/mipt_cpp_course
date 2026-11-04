#include <cstdio>
#include <cstring>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include <map>

#include <parse.h>
#include <event_list.h>
#include <rules.h>
#include <event.h>
#include <fields.h>
#include <os_handle.h>
#include <os.h>
#include <event_source.h>

#include <file_source.h>
#include <os_source.h>
#include <agent_rules.h>
#include <agent.h>

struct Config {
    std::string log_path;
    bool quiet = false;
    bool run_os = true;
    long long window_size = 64;
};

bool ParseArguments(int argc, char** argv, Config& config){
    if (argc < 2) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }

    bool path_set = false;

    for (int i=1;i<argc;i++){
        if (std::string_view(argv[i]) == "--quiet"){
            config.quiet = true;
        }
        else if (std::string_view(argv[i]) == "--window-size"){
            if (i+1 < argc){
                auto [ptr, ec] = std::from_chars(argv[i+1], argv[i+1] + std::strlen(argv[i+1]), config.window_size);
                if (ec != std::errc{}){
                    config.window_size = 64;
                }
                ++i;
            }
            else {
                return false;
            }
        }
        else if (std::string_view(argv[i]) == "--file"){
            if (i+1 < argc){
                if (path_set) return false;
                path_set = true;
                config.log_path = argv[i+1];
                config.run_os = false;
                ++i;
            }
            else{
                return false;
            }
        }
        else if (std::string_view(argv[i])[0] != '-'){
            if (path_set) return false;
            path_set = true;
            config.log_path = argv[i];
        }
        else{
            return false;
        }
    }

    if (config.log_path.empty()) {
        std::print(stderr, "использование: nano-edr <журнал.log>\n");
        return false;
    }

    return true;
}


int main(int argc, char** argv) {
    try{

        Config config;

        if (!ParseArguments(argc, argv, config)){
            return 2;
        }

        nano_edr::Agent agent(config.window_size, config.quiet);

        std::unique_ptr<nano_edr::IEventSource> source;

        if (config.run_os){
            source = std::make_unique<nano_edr::OsSource>(config.log_path);
        }
        else{
            source = std::make_unique<nano_edr::FileSource>(config.log_path);
        }
        source->Run(&agent);

        agent.PrintSummary();
    
        return 0;
    }
    catch (const std::exception& error){
        std::print(stderr, "Прогон прерван из-за ошибки формата: {}\n", error.what());
        return 1;
    }

}
