#include <cstdio>
#include <fstream>
#include <print>
#include <string>
#include <vector>
#include <map>

#include <agent.h>
#include <parse.h>
#include <event_list.h>
#include <rules.h>
#include <agent_rules.h>
#include <event.h>
#include <fields.h>
#include <os_handle.h>
#include <os.h>

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
                try{
                    config.window_size = std::stoll(argv[i+1]);
                }
                catch(const std::exception& e){
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

        if (config.run_os){
            nano_edr::OsSource os(config.log_path);
            os.Run(&agent);

            return 0;
        }

        
        std::ifstream log(config.log_path);

        if (!log){
            std::print(stderr, "Невозможно открыть журнал {}\n", config.log_path);
            return 1;
        }

        nano_edr::FileSource file(config.log_path);
        file.Run(&agent);

        return 0;
    }
    catch (const std::exception& error){
        std::print(stderr, "Прогон прерван из-за ошибки формата: {}\n", error.what());
        return 1;
    }

}
